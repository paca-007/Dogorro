#include "BackgroundRenderer.h"
#include "GraphicsEngine.h"
#include "AMesh.h"
#include "AMaterial.h"

BackgroundRenderer::BackgroundRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext, ID3D11DepthStencilView* _depthStancilView, const int _windowWidth, const int _windowHeight)
	: d3d11Device(_d3d11Device)
	, d3d11DeviceContext(_d3d11DeviceContext)
	, depthStancilView(_depthStancilView)
	, windowWidth(_windowWidth)
	, windowHeight(_windowHeight)
{

}

BackgroundRenderer::~BackgroundRenderer()
{

}

void BackgroundRenderer::Initailze(GraphicsEngine* _gp)
{
	//잘 받아오는지 확인해주는 변수
	HRESULT hr = S_OK;

	this->gp = _gp;

	//새로운 렌더타겟과 쉐이더리소스뷰 생성
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc{};
	renderTargetTextureDesc.Width = static_cast<UINT>(windowWidth);
	renderTargetTextureDesc.Height = static_cast<UINT>(windowHeight);
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // 스펙큘러, 디퓨즈는 8비트로 괜찮을 것 같은데 포지션 노말은?
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, &bTexture);
	//RenderTargetView
	this->d3d11Device->CreateRenderTargetView(bTexture, 0, &this->bRenderTarget);

	//ShaderResourceView
	this->d3d11Device->CreateShaderResourceView(bTexture, nullptr, &this->bSRV);

	this->d3d11DeviceContext->OMSetRenderTargets(1, &this->bRenderTarget, this->depthStancilView);

	//벡터 크기 초기화
	this->bData.reserve(10);
	this->bPosition.reserve(10);
	this->bRotation.reserve(10);
	this->bScale.reserve(10);

}

void BackgroundRenderer::Finalize()
{
	this->bTexture->Release();
	this->bSRV->Release();
	this->bRenderTarget->Release();
}

void BackgroundRenderer::BeginRender()
{
	//1. 렌더 타겟 설정
	SetRenderTarget();
	//2. 렌더 타겟 클리어
	ClearRenderTarget();
	//3. 픽셀쉐이더 초기화
	ClearShaderResources();
	//4. 뎁스 스텐실 초기화
	gp->ClearDepthStencilView();
	gp->BindView();


}

void BackgroundRenderer::EndRender()
{
	//여기서 저장해둔 데이터들을 싹 그린다.
	for (int i = 0; i < bData.size(); i++)
	{
		Matrix movement = Matrix::Identity;

		Vector3 rot = { this->bRotation[i][0], this->bRotation[i][1], this->bRotation[i][2] };
		Vector3 pos = { this->bPosition[i][0], this->bPosition[i][1] , this->bPosition[i][2] };
		Vector3 sca = { this->bScale[i][0], this->bScale[i][1] , this->bScale[i][2] };

		movement *= Matrix::CreateScale(sca);
		movement *= Matrix::CreateFromYawPitchRoll(rot);
		movement *= Matrix::CreateTranslation(pos);

		for (int j = 0; j < bData[i].size(); j++)
		{

			this->gp->BindMatrixParameter(/*bData[i][j]->ori * */movement);
			this->gp->BindPipeline(bData[i][j]->pip);
			this->gp->SetTexture(0, 1, bData[i][j]->material.lock()->diffusMap);
			this->gp->Render(bData[i][j]->pip, static_cast<int>(bData[i][j]->indexData.size()));

		}

	}
	//다 쓴 자원 언바인딩
	ClearShaderResources();

	//다시 그리기 위해 깨끗하게 비우기
	bData.clear();
	bPosition.clear();
	bRotation.clear();
	bScale.clear();
}

void BackgroundRenderer::GetData
(
	std::vector<std::shared_ptr<AMesh>> _nowMesh, 
	std::vector<float> _nowPostion, 
	std::vector<float> _nowRotation, 
	std::vector<float> _nowScale
)
{
	this->bData.push_back(_nowMesh);
	this->bPosition.push_back(_nowPostion);
	this->bRotation.push_back(_nowRotation);
	this->bScale.push_back(_nowScale);
}

void BackgroundRenderer::SetRenderTarget()
{
	this->d3d11DeviceContext->OMSetRenderTargets(1, &this->bRenderTarget, this->depthStancilView);
}

void BackgroundRenderer::ClearRenderTarget()
{
	// 임시 색 ( R G B A )
	float bgRed[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// 렌더 타겟을 지정한 색으로 초기화
	this->d3d11DeviceContext->ClearRenderTargetView(
		bRenderTarget,
		bgRed
	);
}

void BackgroundRenderer::ClearShaderResources()
{
	ID3D11ShaderResourceView* pSRV = NULL;
	this->d3d11DeviceContext->PSSetShaderResources(0, 1, &pSRV);
}
