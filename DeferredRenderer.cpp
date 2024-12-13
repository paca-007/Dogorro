#include "DeferredRenderer.h"
#include "GraphicsEngine.h"
#include "pipeline.h"



DeferredRenderer::DeferredRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext,
	ID3D11DepthStencilView* _depthStancilView,
	const int _windowWidth, const int _windowHeight)
	: d3d11Device(_d3d11Device)
	, d3d11DeviceContext(_d3d11DeviceContext)
	, depthStancilView(_depthStancilView)
	, windowWidth(_windowWidth)
	, windowHeight(_windowHeight)
{

}

DeferredRenderer::~DeferredRenderer()
{

}

void DeferredRenderer::Initailze(GraphicsEngine* _gp)
{
	HRESULT hr = S_OK;
	//그래픽 엔진 받아와서 초기화 해주기
	this->gp = _gp;

	//디퍼드 렌더링용 정점 구조체 초기화
	DVdata[0] = { DirectX::XMFLOAT3{-1.0f, 1.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 0.0f} };
	DVdata[1] = { DirectX::XMFLOAT3{1.0f, 1.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 0.0f} };
	DVdata[2] = { DirectX::XMFLOAT3{1.0f, -1.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 1.0f} };
	DVdata[3] = { DirectX::XMFLOAT3{-1.0f, -1.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 1.0f} };

	CreateSubView();

	DIdata[0] = 0;
	DIdata[1] = 1;
	DIdata[2] = 3;
	DIdata[3] = 1;
	DIdata[4] = 2;
	DIdata[5] = 3;


	//각 데이터들에 대해 크기 재설정
	dTexture.resize(gBufferSize);
	dRenderTargets.resize(gBufferSize);
	dSRV.resize(gBufferSize);

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc{};
	renderTargetTextureDesc.Width = static_cast<UINT>(windowWidth);
	renderTargetTextureDesc.Height = static_cast<UINT>(windowHeight);
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;  // 스펙큘러, 디퓨즈는 8비트로 괜찮을 것 같은데 포지션 노말은?
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	//renderTargetTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;	//죽인다.
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	for (auto& dt : dTexture)
	{
		this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, &dt);
		assert(SUCCEEDED(hr) && "cannot create Texture at Deffered renderer");
	}
	
	// 1-2. RenderTargetView
	D3D11_RENDER_TARGET_VIEW_DESC rendertargetViewDesc;
	ZeroMemory(&rendertargetViewDesc, sizeof(rendertargetViewDesc));
	rendertargetViewDesc.Format = renderTargetTextureDesc.Format;
	rendertargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	for (int i = 0; i < gBufferSize; ++i)
	{
		this->d3d11Device->CreateRenderTargetView(this->dTexture[i], &rendertargetViewDesc, &this->dRenderTargets[i]);
		assert(SUCCEEDED(hr) && "cannot create RenderTargetView at Deffered renderer");
	}


	// 2. ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = renderTargetTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < gBufferSize; i++)
	{
		this->d3d11Device->CreateShaderResourceView(this->dTexture[i], &shaderResourceViewDesc, &this->dSRV[i]);
		assert(SUCCEEDED(hr) && "cannot create ShaderResourceView at Deffered renderer");

	}

	CreateDeferredPipeline();

	this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, &dFinalTexture);
	this->d3d11Device->CreateRenderTargetView(this->dFinalTexture, &rendertargetViewDesc, &this->dFinalRenderTargets);
	this->d3d11Device->CreateShaderResourceView(this->dFinalTexture, &shaderResourceViewDesc, &this->dFinalSRV);
	
	CreateDeferredFinalPipeline();

}

void DeferredRenderer::Finalize()
{
	for (int i = 0; i < gBufferSize; i++)
	{
		this->dTexture[i]->Release();
		this->dSRV[i]->Release();
		this->dRenderTargets[i]->Release();
	}
}

void DeferredRenderer::BeginRender()
{
	ID3D11ShaderResourceView* pSRV[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	this->d3d11DeviceContext->PSSetShaderResources(0, 5, pSRV);
	BindDeferredView();
	DeferredRenderClearView();
	gp->ClearDepthStencilView();
}

void DeferredRenderer::EndRender()
{
	float bgRed[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	this->d3d11DeviceContext->OMSetRenderTargets(1, &this->dFinalRenderTargets, this->depthStancilView);
	this->d3d11DeviceContext->ClearRenderTargetView(this->dFinalRenderTargets, bgRed);
	gp->ClearDepthStencilView();
	gp->BindPipeline(this->DPipeline);
	gp->BindCameraAtPS();

	this->d3d11DeviceContext->PSSetShaderResources(0, this->gBufferSize, dSRV.data());
	
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);
	
	for (auto& pipe : this->DSubPipeline)
	{
		gp->BindPipeline(pipe);
		this->d3d11DeviceContext->DrawIndexed(6, 0, 0);
	}

	//렌더 타겟 자원 언바인딩
	for (int i = 0; i < 6; i++)
	{
		ID3D11ShaderResourceView* pSRV = NULL;
		this->d3d11DeviceContext->PSSetShaderResources(i, 1, &pSRV);
	}

	//this->d3d11DeviceContext->OMSetRenderTargets(1, &gp->finalRenderTartView, this->depthStancilView);
	//gp->BindPipeline(this->DFinalPipeline);
	//this->d3d11DeviceContext->PSSetShaderResources(0, 1, &this->dFinalSRV);
	//this->d3d11DeviceContext->DrawIndexed(6, 0, 0);

	//마지막으로 한번 더 그려야 됨!
	/*gp->BindView();
	gp->ClearDepthStencilView();
	gp->BindPipeline(this->DFinalPipeline);
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &this->dFinalSRV);

	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);*/
}


void DeferredRenderer::BindDeferredView()
{
	this->d3d11DeviceContext->OMSetRenderTargets(gBufferSize, this->dRenderTargets.data(), this->depthStancilView);
}

void DeferredRenderer::DeferredRenderClearView()
{
	float bgRed[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	for (auto& rt : this->dRenderTargets)
	{
		// 임시 색 ( R G B A )

		// 렌더 타겟을 지정한 색으로 초기화
		this->d3d11DeviceContext->ClearRenderTargetView(
			rt,
			bgRed
		);
	}
}

void DeferredRenderer::CreateDeferredPipeline()
{
	std::wstring vsPath = L"../Shader/compiled/DPass2VS.cso";
	std::wstring psPath = L"../Shader/compiled/DPass2.cso";
	gp->CreateInputLayer(this->DPipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->DPipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->DPipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->DVdata, (UINT)(4 * VertexD::Size()), this->DPipeline.vertexBuffer, "DefferedVB");
	gp->CreateIndexBuffer(this->DIdata, 6, this->DPipeline.IndexBuffer);
	this->DPipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->DPipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->DPipeline.rasterizerState);

	std::wstring psSubPath[6] =
	{
		L"../Shader/compiled/DPass2Textuer.cso",
		L"../Shader/compiled/DPass2Normal.cso",
		L"../Shader/compiled/DPass2Depth.cso",
		L"../Shader/compiled/DPass2Tangent.cso",
		L"../Shader/compiled/DPass2Bitangent.cso",
		L"../Shader/compiled/DPass2rou.cso"
	};
	int pindx = 0;
	for (auto& pipe : this->DSubPipeline)
	{
		gp->CreateInputLayer(pipe.inputLayout, VertexD::defaultInputLayerDECS, 2, pipe.vertexShader, vsPath);
		gp->CreatePixelShader(pipe.pixelShader, psSubPath[pindx]);
		gp->CreateIndexBuffer(this->DIdata, 6, pipe.IndexBuffer);
		gp->CreateVertexBuffer<VertexD::Data>(this->DSubVdata[pindx], (UINT)(4 * VertexD::Size()), pipe.vertexBuffer, "DefferedSubVB" + std::to_string(pindx));
		pipe.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		pipe.vertexStructSize = VertexD::Size();
		gp->CreateRasterizerState(pipe.rasterizerState);
		pindx++;
	}
}

void DeferredRenderer::CreateDeferredFinalPipeline()
{
	std::wstring vsPath = L"../Shader/compiled/DPass2VS.cso";
	std::wstring psPath = L"../Shader/compiled/DPass3.cso";
	gp->CreateInputLayer(this->DFinalPipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->DFinalPipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->DFinalPipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->DVdata, (UINT)(4 * VertexD::Size()), this->DFinalPipeline.vertexBuffer, "DefferdFinalVB");
	gp->CreateIndexBuffer(this->DIdata, 6, this->DFinalPipeline.IndexBuffer);
	this->DFinalPipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->DFinalPipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->DFinalPipeline.rasterizerState);
}

void DeferredRenderer::CreateSubView()
{
	float distance = (2 / static_cast<float>(this->gBufferSize));
	float lx = -1.0f + (distance * (static_cast<float>(this->gBufferSize) - 1.0f));
	float rx = 1;
	float dis = rx - lx;
	for (int i = 0; i < this->gBufferSize; i++)
	{
		float uy = 1 - ((1 - lx) * i);
		float dy = uy - distance;
		DSubVdata[i][0] = { DirectX::XMFLOAT3{lx, uy, 0.0f}, DirectX::XMFLOAT2{0.0f, 0.0f} };
		DSubVdata[i][1] = { DirectX::XMFLOAT3{rx, uy, 0.0f}, DirectX::XMFLOAT2{1.0f, 0.0f} };
		DSubVdata[i][2] = { DirectX::XMFLOAT3{rx, dy, 0.0f}, DirectX::XMFLOAT2{1.0f, 1.0f} };
		DSubVdata[i][3] = { DirectX::XMFLOAT3{lx, dy, 0.0f}, DirectX::XMFLOAT2{0.0f, 1.0f} };
	}
}

void DeferredRenderer::UpdateTexture()
{
	gp->deferredSRV = this->dFinalSRV;
	gp->deferredSRVTexture = this->dSRV[0];
	gp->deferredSRVNormal = this->dSRV[1];			//Normal
	gp->deferredSRVDepth = this->dSRV[2];			//Depth
	gp->dFinalTexture = this->dFinalTexture;
}

