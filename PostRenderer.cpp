#include "PostRenderer.h"
#include "GraphicsEngine.h"

PostRenderer::PostRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext, ID3D11DepthStencilView* _depthStancilView, const int _windowWidth, const int _windowHeight) : d3d11Device(_d3d11Device)
	, d3d11DeviceContext(_d3d11DeviceContext)
	, depthStancilView(_depthStancilView)
	, windowWidth(_windowWidth)
	, windowHeight(_windowHeight)
{

}

PostRenderer::~PostRenderer()
{

}

void PostRenderer::Initailze(GraphicsEngine* _gp)
{
	//잘 받아오는지 확인해주는 변수
	HRESULT hr = S_OK;

	//그래픽스 받아오고
	this->gp = _gp;

	//스케치북 만들어주고
	PVdata[0] = { DirectX::XMFLOAT3{-1.0f, 1.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 0.0f} };
	PVdata[1] = { DirectX::XMFLOAT3{1.0f, 1.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 0.0f} };
	PVdata[2] = { DirectX::XMFLOAT3{1.0f, -1.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 1.0f} };
	PVdata[3] = { DirectX::XMFLOAT3{-1.0f, -1.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 1.0f} };
	
	PIdata[0] = 0;
	PIdata[1] = 1;
	PIdata[2] = 3;
	PIdata[3] = 1;
	PIdata[4] = 2;
	PIdata[5] = 3;


	///Outline
	//새로운 렌더타겟과 쉐이더소스뷰 생성
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc{};
	renderTargetTextureDesc.Width = static_cast<UINT>(windowWidth);
	renderTargetTextureDesc.Height = static_cast<UINT>(windowHeight);
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	//texture
	this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, outlineTexture.GetAddressOf());
	assert(SUCCEEDED(hr) && "cannot create Texture at PostProcessing renderer");

	//RenderTargetView
	this->d3d11Device->CreateRenderTargetView(outlineTexture.Get(), 0, this->outlineRenderTarget.GetAddressOf());
	assert(SUCCEEDED(hr) && "cannot create RenderTargetView at PostProcessing renderer");

	//ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = renderTargetTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	this->d3d11Device->CreateShaderResourceView(outlineTexture.Get(), nullptr, this->outlineSRV.GetAddressOf());
	assert(SUCCEEDED(hr) && "cannot create ShaderResourceView at PostProcessing renderer");

	this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, this->outlineBlurYTexture.GetAddressOf());
	this->d3d11Device->CreateRenderTargetView(outlineBlurYTexture.Get(), 0, this->outlineBlurYRenderTarget.GetAddressOf());
	this->d3d11Device->CreateShaderResourceView(outlineBlurYTexture.Get(), nullptr, this->outlineBlurYSRV.GetAddressOf());


	//아웃라인 그려줄 파이프라인 생성
	CreateOutlinePipeline();
	//놀랍게도 블러도 따로 파이프라인을 타야됩니다..
	CreateOutlineBlurYPipeline();
	//심지어 두번 타야됩니다.
	CreateOutlineBlurXPipeline();
	//아웃라인 한번으로 안되것구나.. 파이프 라인 한번 더 가자
	CreateSecondOutlinePipeline();

	//pixelate pipeline
	CreatePixelatePipeline();

	//flash pipeline
	CreateFlashPipeline();
}

void PostRenderer::Finalize()
{
}

void PostRenderer::BeginRender()
{
	//필요한 리소스 받아오기     
	UpdateShaderResources();

	//뎁스 스텐실 초기화
	gp->ClearDepthStencilView();

	//Outline
	FirstpassOutline();
}

void PostRenderer::EndRender()
{
	SecondPassOutline();
}

void PostRenderer::UpdateShaderResources()
{
	this->deferredNormal = gp->deferredSRVNormal;
	this->deferredDepth = gp->deferredSRVDepth;
	this->deferredColor = gp->deferredSRV;
	this->deferredTexture = gp->deferredSRVTexture;
	//this->finalTexture = gp->finalSRV;
}

void PostRenderer::CreateOutlinePipeline()
{
	std::wstring vsPath = L"../Shader/compiled/OutlineVertexShader.cso";
	std::wstring psPath = L"../Shader/compiled/OutlinePixelShader.cso";

	gp->CreateInputLayer(this->OutlinePipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->OutlinePipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->OutlinePipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->PVdata, (UINT)(4 * VertexD::Size()), this->OutlinePipeline.vertexBuffer, "PostProcessVB");
	gp->CreateIndexBuffer(this->PIdata, 6, this->OutlinePipeline.IndexBuffer);
	this->OutlinePipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->OutlinePipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->OutlinePipeline.rasterizerState);
}

void PostRenderer::CreateOutlineBlurYPipeline()
{
	std::wstring vsPath = L"../Shader/compiled/OutlineVertexShader.cso";
	std::wstring psPath = L"../Shader/compiled/OutlineBlurY.cso";

	gp->CreateInputLayer(this->OutlineBlurYPipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->OutlineBlurYPipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->OutlineBlurYPipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->PVdata, (UINT)(4 * VertexD::Size()), this->OutlineBlurYPipeline.vertexBuffer, "OutLineBluryVB");
	gp->CreateIndexBuffer(this->PIdata, 6, this->OutlineBlurYPipeline.IndexBuffer);
	this->OutlineBlurYPipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->OutlineBlurYPipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->OutlineBlurYPipeline.rasterizerState);
}

void PostRenderer::CreateOutlineBlurXPipeline()
{
	std::wstring vsPath = L"../Shader/compiled/OutlineVertexShader.cso";
	std::wstring psPath = L"../Shader/compiled/OutlineBlurX.cso";

	gp->CreateInputLayer(this->OutlineBluerXPipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->OutlineBluerXPipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->OutlineBluerXPipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->PVdata, (UINT)(4 * VertexD::Size()), this->OutlineBluerXPipeline.vertexBuffer, "OutLineBlurXVB");
	gp->CreateIndexBuffer(this->PIdata, 6, this->OutlineBluerXPipeline.IndexBuffer);
	this->OutlineBluerXPipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->OutlineBluerXPipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->OutlineBluerXPipeline.rasterizerState);

}

void PostRenderer::CreateSecondOutlinePipeline()
{
	std::wstring vsPath = L"../Shader/compiled/OutlineVertexShader.cso";
	std::wstring psPath = L"../Shader/compiled/Outline2Pass.cso";

	gp->CreateInputLayer(this->Pass2OutlinePipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->Pass2OutlinePipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->Pass2OutlinePipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->PVdata, (UINT)(4 * VertexD::Size()), this->Pass2OutlinePipeline.vertexBuffer, "OutLineSecoundVB");
	gp->CreateIndexBuffer(this->PIdata, 6, this->Pass2OutlinePipeline.IndexBuffer);
	this->Pass2OutlinePipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->Pass2OutlinePipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->Pass2OutlinePipeline.rasterizerState);
}


void PostRenderer::FirstpassOutline()
{
	
	///Pass1 diffuse를 이용해 outline을 따준다.
	//렌더 타겟 설정
	this->d3d11DeviceContext->OMSetRenderTargets(1, this->outlineRenderTarget.GetAddressOf(), this->depthStancilView);
	//클리어 렌더 타겟
	// 임시 색 ( R G B A )
	float bgRed[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	// 렌더 타겟을 지정한 색으로 초기화
	this->d3d11DeviceContext->ClearRenderTargetView(this->outlineRenderTarget.Get(), bgRed);
	
	//바인드 파이프라인
	gp->BindPipeline(this->OutlinePipeline);

	//픽셀 쉐이더 연결
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, this->deferredTexture.GetAddressOf());	//텍스쳐값 보내기
	
	//그리기
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);

	//언바인딩
	ID3D11ShaderResourceView* nSRV = NULL;
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &nSRV);
	
	///Pass2 Blur를 이용해 선을 굵게 만들어준다.
	this->d3d11DeviceContext->OMSetRenderTargets(1, this->outlineBlurYRenderTarget.GetAddressOf(), this->depthStancilView);
	this->d3d11DeviceContext->ClearRenderTargetView(this->outlineBlurYRenderTarget.Get(), bgRed);
	gp->BindView();
	gp->BindPipeline(this->OutlineBlurYPipeline);
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, this->outlineSRV.GetAddressOf());	//outline값 보내주기
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &nSRV);

	///Pass3 BlurX를 이용해 다시 한번 더 선을 굵게 만든다
	gp->BindView();
	//gp->ClearRenderTargetView();
	gp->BindPipeline(this->OutlineBluerXPipeline);
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, this->outlineSRV.GetAddressOf());	//outline값 보내주기
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &nSRV);
}

void PostRenderer::SecondPassOutline()
{
	//픽셀 쉐이더 연결
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, this->deferredColor.GetAddressOf());
	this->d3d11DeviceContext->PSSetShaderResources(1, 1, this->outlineSRV.GetAddressOf());	//텍스쳐 보내기

	//바인드 뷰
	gp->BindView();
	//gp->ClearRenderTargetView();
	//바인드 파이프라인
	gp->BindPipeline(this->Pass2OutlinePipeline);
	
	//그리기
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);

	//언바인딩
	ID3D11ShaderResourceView* nSRV = NULL;
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &nSRV);
	this->d3d11DeviceContext->PSSetShaderResources(1, 1, &nSRV);

}

void PostRenderer::CreatePixelatePipeline()
{
	std::wstring vsPath = L"../Shader/compiled/OutlineVertexShader.cso";
	std::wstring psPath = L"../Shader/compiled/PixelatePixelShader.cso";

	gp->CreateInputLayer(this->pixelatePipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->pixelatePipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->pixelatePipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->PVdata, (UINT)(4 * VertexD::Size()), this->pixelatePipeline.vertexBuffer, "PostPixelVB");
	gp->CreateIndexBuffer(this->PIdata, 6, this->pixelatePipeline.IndexBuffer);
	this->pixelatePipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->pixelatePipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->pixelatePipeline.rasterizerState);
}

void PostRenderer::FirstpassPixel()
{	
	ID3D11ShaderResourceView* nowSRV;
	nowSRV = gp->finalSRV.Get();
	
	//바인드 뷰
	gp->BindView();
	//바인드 파이프라인
	gp->BindPipeline(this->pixelatePipeline);
	//픽셀 쉐이더 연결

	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &nowSRV);	//텍스쳐 보내기

	//그리기
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);

	//언바인딩
	ID3D11ShaderResourceView* nSRV = NULL;
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &nSRV);

}

void PostRenderer::CreateFlashPipeline()
{
	std::wstring vsPath = L"../Shader/compiled/OutlineVertexShader.cso";
	std::wstring psPath = L"../Shader/compiled/FlashPS.cso";


	gp->CreateInputLayer(this->flashPipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->flashPipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->flashPipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->PVdata, (UINT)(4 * VertexD::Size()), this->flashPipeline.vertexBuffer, "FlashPiplineVB");
	gp->CreateIndexBuffer(this->PIdata, 6, this->flashPipeline.IndexBuffer);
	this->flashPipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->flashPipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->flashPipeline.rasterizerState);


	// 상수 버퍼 생성
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(ConstantBufferData);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	this->d3d11Device->CreateBuffer(&bufferDesc, NULL, this->flashBuffer.GetAddressOf());
	flashData.Time = 0;
}

void PostRenderer::FirstPassFlash()
{
	//바인드 뷰
	gp->BindView();
	//바인드 파이프라인
	gp->BindPipeline(this->flashPipeline);


	//픽셀 쉐이더 연결
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, this->finalTexture.GetAddressOf());	//텍스쳐 보내기

	//상수 버퍼 업데이트
	UpdateFlashBuffer();

	//그리기
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);

	//언바인딩
	ID3D11ShaderResourceView* nSRV = NULL;
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &nSRV);
}

void PostRenderer::UpdateFlashBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = (this->d3d11DeviceContext->Map(flashBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	ConstantBufferData* bufferData = static_cast<ConstantBufferData*>(mappedResource.pData);
	*bufferData = flashData;
	
	// 상수 버퍼 데이터 복사
	//memcpy(mappedResource.pData, &cbData, sizeof(ConstantBufferData));

	this->d3d11DeviceContext->Unmap(flashBuffer.Get(), 0);
	
	this->d3d11DeviceContext->PSSetConstantBuffers(0, 1, this->flashBuffer.GetAddressOf());
	
}

