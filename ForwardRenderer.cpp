#include "ForwardRenderer.h"
#include "GraphicsEngine.h"
#include "AMesh.h"
#include "ABone.h"
#include "AMaterial.h"
#include <memory>

ForwardRenderer::ForwardRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext, ID3D11DepthStencilView* _depthStancilView, const int _windowWidth, const int _windowHeight)
	: d3d11Device(_d3d11Device)
	, d3d11DeviceContext(_d3d11DeviceContext)
	, depthStancilView(_depthStancilView)
	, windowWidth(_windowWidth)
	, windowHeight(_windowHeight)
{

}

ForwardRenderer::~ForwardRenderer()
{
	
}

void ForwardRenderer::Initailze(GraphicsEngine* _gp)
{
	//잘 받아오는지 확인해주는 변수
	HRESULT hr = S_OK;

	this->gp = _gp;

	//앞뒤 렌더링용 정점 구조체 초기화
	FVdata[1] = { DirectX::XMFLOAT3{1.0f, 1.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 0.0f} };
	FVdata[0] = { DirectX::XMFLOAT3{-1.0f, 1.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 0.0f} };
	FVdata[2] = { DirectX::XMFLOAT3{1.0f, -1.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 1.0f} };
	FVdata[3] = { DirectX::XMFLOAT3{-1.0f, -1.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 1.0f} };
	
	FIdata[0] = 0;
	FIdata[1] = 1;
	FIdata[2] = 3;
	FIdata[3] = 1;
	FIdata[4] = 2;
	FIdata[5] = 3;

	//각 데이터들에 대해 크기 재설정
	fTexture.resize(fBufferSize);
	fRenderTargets.resize(fBufferSize);
	fSRV.resize(fBufferSize);

	//새로운 렌더타겟과 쉐이더리소스뷰 생성
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc{};
	renderTargetTextureDesc.Width = static_cast<UINT>(windowWidth);
	renderTargetTextureDesc.Height = static_cast<UINT>(windowHeight);
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	////Texture
	for (auto& ft : fTexture)
	{
		this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, &ft);
		assert(SUCCEEDED(hr) && "cannot create Texture at Forward renderer");
	}

	this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, &f2Texture);
	assert(SUCCEEDED(hr) && "cannot create Texture at Forward renderer");

	////RenderTargetView
	D3D11_RENDER_TARGET_VIEW_DESC rendertargetViewDesc;
	ZeroMemory(&rendertargetViewDesc, sizeof(rendertargetViewDesc));
	rendertargetViewDesc.Format = renderTargetTextureDesc.Format;
	rendertargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	for (int i = 0; i < fBufferSize; ++i)
	{
		this->d3d11Device->CreateRenderTargetView(fTexture[i], &rendertargetViewDesc, &this->fRenderTargets[i]);
		assert(SUCCEEDED(hr) && "cannot create RenderTargetView at Forward renderer");
	}
	this->d3d11Device->CreateRenderTargetView(f2Texture.Get(), &rendertargetViewDesc, &this->f2RenderTargets);
	assert(SUCCEEDED(hr) && "cannot create RenderTargetView at Forward renderer");

	// 2. ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = renderTargetTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < fBufferSize; i++)
	{
		this->d3d11Device->CreateShaderResourceView(fTexture[i], &shaderResourceViewDesc, &this->fSRV[i]);
		assert(SUCCEEDED(hr) && "cannot create ShaderResourceView at Forward renderer");
	}

	this->d3d11Device->CreateShaderResourceView(f2Texture.Get(), nullptr, &this->f2SRV);
	assert(SUCCEEDED(hr) && "cannot create ShaderResourceView at Forward renderer");
	
	CreateSecondPipeLine();

	//벡터 크기 초기화
	this->tpData.reserve(10);
	this->tpBone.reserve(10);
	this->tpPosition.reserve(10);
	this->tpRotation.reserve(10);
	this->tpScale.reserve(10);
}

void ForwardRenderer::Finalize()
{
	//사용햇던 자원들 초기화
	for (int i = 0; i < fBufferSize; i++)
	{
		this->fTexture[i]->Release();
		this->fSRV[i]->Release();
		this->fRenderTargets[i]->Release();
	}
}

void ForwardRenderer::BeginRender()
{
	//앞에서 썻던거 자원 언바인딩 -> 근데 이건 각 EndRender에서 해줍시다.

	//beginRender에서 강아지 그린 한 장을 뽑아냅시다. 
	//메인이랑 Binding안하고 내 렌더타겟에 그려서 한 장 뽑아낼거임.

	//픽셀쉐이더 초기화
	ID3D11ShaderResourceView* pSRV[2] = { nullptr, nullptr };
	this->d3d11DeviceContext->PSSetShaderResources(0, 2, pSRV);

	//렌더 타겟 설정
	this->d3d11DeviceContext->OMSetRenderTargets(fBufferSize, this->fRenderTargets.data(), this->depthStancilView.Get());
	
	//렌더 타겟 클리어
	float bgRed[4] = { 1.0f, 1.0f, 1.0f, 0.0f };
	for (auto& rt : this->fRenderTargets)
	{
		// 임시 색 ( R G B A )

		// 렌더 타겟을 지정한 색으로 초기화
		this->d3d11DeviceContext->ClearRenderTargetView(
			rt,
			bgRed
		);
	}

	//뎁스스텐실 클리어
	gp->ClearDepthStencilView();
	
	//진짜 춘식님이 보면 혼날 코드다..
	for (int i = 0; i < tpData.size(); i++)
	{
		Matrix movement = Matrix::Identity;
		
		Vector3 rot = { this->tpRotation[i][0], this->tpRotation[i][1], this->tpRotation[i][2] };
		Vector3 pos = { this->tpPosition[i][0], this->tpPosition[i][1] , this->tpPosition[i][2] };
		Vector3 sca = { this->tpScale[i][0], this->tpScale[i][1] , this->tpScale[i][2] };

		Quaternion quaternionRotation = Quaternion::CreateFromYawPitchRoll(rot);
		
	
		movement *= Matrix::CreateScale(sca);
		movement *= Matrix::CreateFromYawPitchRoll(rot);
		movement *= Matrix::CreateTranslation(pos);

		for (int j = 0; j < tpData[i].size(); j++)
		{
			
			this->gp->BindBonesData(tpBone[i], tpData[i][j]->ori);
			this->gp->BindMatrixParameter(tpData[i][j]->ori * movement);
			this->gp->BindPipeline(tpData[i][j]->pip);
			this->gp->SetTexture(0, 1, tpData[i][j]->material.lock()->diffusMap);
			this->gp->Render(tpData[i][j]->pip, static_cast<int>(tpData[i][j]->indexData.size()));
			
		}
		
	}
	
	
}

void ForwardRenderer::EndRender()
{
	
	//0. 그리기 준비
	BeginTwoPass();

	//알맞은 처리가 끝났음 그리자!
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);
	
	//다 그렷음 바인딩 해제!
	//UnBindingPS();
	
	//다시 그리기 위해 깨끗하게 비우기
	tpData.clear();
	tpBone.clear();
	tpPosition.clear();
	tpRotation.clear();
	tpScale.clear();

}

void ForwardRenderer::GetData
(
	  std::vector<std::shared_ptr<AMesh>> _nowMesh
	, std::vector<std::shared_ptr<ABone>> _nowBone
	, std::vector<float> _nowPostion
	, std::vector<float> _nowRotation
	, std::vector<float> _nowScale
)
{

	this->tpData.push_back(_nowMesh);
	this->tpBone.push_back(_nowBone);
	this->tpPosition.push_back(_nowPostion);
	this->tpRotation.push_back(_nowRotation);
	this->tpScale.push_back(_nowScale);
	
	/*tpData.emplace_back(std::move(frameMeshes));
	tpBone.emplace_back(std::move(frameBones));
	tpPosition.push_back(_nowPostion);
	tpRotation.push_back(_nowRotation);
	tpScale.push_back(_nowScale);*/
}

void ForwardRenderer::UpdateDeferredTexture()
{
	this->dSRV = gp->deferredSRV;
	this->dSRV2 = gp->deferredSRVDepth;
}

void ForwardRenderer::BeginTwoPass()
{
	//0. 일단 픽셀쉐이더 자원 초기화
	ID3D11ShaderResourceView* pSRV = NULL;
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &pSRV);
	this->d3d11DeviceContext->PSSetShaderResources(1, 1, &pSRV);

	gp->BindView();
	gp->BindPipeline(this->FPipeline);

	//this->d3d11DeviceContext->PSSetShaderResources(0, 1, &f2SRV);	//그려질 texture
	//Deferred Data
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &dSRV);	//texture인데 그거 안된거
	this->d3d11DeviceContext->PSSetShaderResources(1, 1, &dSRV2);	//depth
	//Forward Data
	this->d3d11DeviceContext->PSSetShaderResources(2, 1, &fSRV[0]);	//texture
	this->d3d11DeviceContext->PSSetShaderResources(3, 1, &fSRV[1]);	//depth

	//랜더 클리어 뷰
	float bgRed[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	this->d3d11DeviceContext->ClearRenderTargetView(this->f2RenderTargets.Get(), bgRed);

	gp->BindView();
	gp->BindPipeline(this->FPipeline);

}

void ForwardRenderer::UnBindingPS()
{
	//앞에서 썻던거 자원 언바인딩
	ID3D11ShaderResourceView* pSRV = NULL;

	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &pSRV);
	this->d3d11DeviceContext->PSSetShaderResources(1, 1, &pSRV);
	this->d3d11DeviceContext->PSSetShaderResources(2, 1, &pSRV);
	this->d3d11DeviceContext->PSSetShaderResources(3, 1, &pSRV);
	this->d3d11DeviceContext->PSSetShaderResources(4, 1, &pSRV);
}

void ForwardRenderer::CreateSecondPipeLine()
{
	std::wstring vsPath = L"../Shader/compiled/FPass2VS.cso";
	std::wstring psPath = L"../Shader/compiled/FPass2.cso";
	
	gp->CreateInputLayer(this->FPipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->FPipeline.vertexShader, vsPath);
	gp->CreatePixelShader(this->FPipeline.pixelShader, psPath);
	gp->CreateVertexBuffer<VertexD::Data>(this->FVdata, (UINT)(4 * VertexD::Size()), this->FPipeline.vertexBuffer, "ForwardSecondVB");
	gp->CreateIndexBuffer(this->FIdata, 6, this->FPipeline.IndexBuffer);
	this->FPipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->FPipeline.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(this->FPipeline.rasterizerState);
}

