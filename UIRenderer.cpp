#include "UIRenderer.h"
#include "GraphicsEngine.h"

UIRenderer::UIRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext,
	ID3D11DepthStencilView* _depthStancilView,
	const int _windowWidth, const int _windowHeight)
	: gp(nullptr)
	, d3d11Device(_d3d11Device)
	, d3d11DeviceContext(_d3d11DeviceContext)
	, depthStancilView(_depthStancilView)
	, windowWidth(_windowWidth)
	, windowHeight(_windowHeight)
	, fps(0)
{

}

UIRenderer::~UIRenderer()
{

}

void UIRenderer::Initailze(GraphicsEngine* _gp)
{
	//그래픽 엔진 초기화
	this->gp = _gp;
	CreateMoveBuffer();
}



void UIRenderer::Finalize()
{
	for (auto& p : this->uiPipeline)
	{
		delete p.second;
	}
}

void UIRenderer::BeginRender()
{
	gp->BindView();
}

void UIRenderer::EndRender()
{
	int i = 0;

	for (auto p : uData)
	{
		gp->BindPipeline(*p);
		BindMoveBuffer(uiPosition[uName[i]].first, uiPosition[uName[i]].second);
		gp->ClearRenderTargetView();
		//UI에 맞는 텍스처 바인드.
		gp->SetTexture(0, 1, uTexture[i]);

		//애니메이션을 위한 데이터
		fps += gp->dt;

		//애니메이션이 잇을 경우 상수버퍼를 업데이트 시킨다.
		if (uName[i].find("Ani") != std::string::npos)
		{
			UpdateAnimationBuffer(uName[i]);
			AnimationData _nowData = uiAnimation[uName[i]];
			int _nowIndex = static_cast<int>(_nowData.frameIndex);

			//애니메이션 변환
			if (fps > 0.5f)
			{
				if (_nowIndex > _nowData.totalFrames - 2)
					_nowData.frameIndex = 0;
				else
					_nowData.frameIndex++;

				uiAnimation[uName[i]] = _nowData;

				fps = 0;
			}
		}

		this->d3d11DeviceContext->DrawIndexed(6, 0, 0);

		i++;
	}

	//다시 그리기 위해 깨끗하게 비우기
	uData.clear();
	uTexture.clear();
	uName.clear();

}

void UIRenderer::GetMesh(std::string _name)
{
	PipeLine* _nowPipeline = uiPipeline[_name];
	ID3D11ShaderResourceView* _nowTexture = uiTexture[_name].Get();

	uData.push_back(_nowPipeline);
	uTexture.push_back(_nowTexture);
	uName.push_back(_name);
}

void UIRenderer::CreateMesh(std::string _name, float _width, float _height, float _startPointX, float _startPointY, std::wstring _texturePath)
{
	//원하는 자리에 넣기 위한 계산..
	_width = 2 * _width / windowWidth;
	_height = 2 * _height / windowHeight;

	_startPointX = (2 * _startPointX / windowWidth) - 1;
	_startPointY = (2 * _startPointY / windowHeight) - 1;

	UVdata[0] = { DirectX::XMFLOAT3{0.0f, 0.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 0.0f} };
	UVdata[1] = { DirectX::XMFLOAT3{0.0f + _width, 0.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 0.0f} };
	UVdata[2] = { DirectX::XMFLOAT3{0.0f + _width, 0.0f - _height, 0.0f}, DirectX::XMFLOAT2{1.0f, 1.0f} };
	UVdata[3] = { DirectX::XMFLOAT3{0.0f, 0.0f - _height, 0.0f}, DirectX::XMFLOAT2{0.0f, 1.0f} };

	UIdata[0] = 0;
	UIdata[1] = 1;
	UIdata[2] = 3;
	UIdata[3] = 1;
	UIdata[4] = 2;
	UIdata[5] = 3;

	//텍스처를 로딩해봅시다
	std::wstring wTexturePath = _texturePath;
	gp->LoadTexture(wTexturePath, uPipeTexture);
	uiTexture[_name] = uPipeTexture;
	uiPosition[_name] = std::make_pair(_startPointX, _startPointY);
}

void UIRenderer::CreatePipeline(std::string _name, std::wstring* _path)
{
	/*std::wstring path[2]
	{
		L"../Shader/compiled/UIVertexShader.cso",
		L"../Shader/compiled/UIPixelShader.cso",
	};*/

	UPipeline = new PipeLine();
	gp->CreateInputLayer(UPipeline->inputLayout, VertexD::defaultInputLayerDECS, 2, UPipeline->vertexShader, _path[0]);
	gp->CreatePixelShader(UPipeline->pixelShader, _path[1]);
	gp->CreateVertexBuffer<VertexD::Data>(this->UVdata, (UINT)(4 * VertexD::Size()), UPipeline->vertexBuffer, _name);
	gp->CreateIndexBuffer(this->UIdata, 6, UPipeline->IndexBuffer);
	UPipeline->primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UPipeline->vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(UPipeline->rasterizerState);

	uiPipeline[_name] = UPipeline;
}

void UIRenderer::CreateAnimationMesh(std::string _name, float _width, float _height, float _startPointX, float _startPointY, float _aniWidth, std::wstring _texturePath, int _aniCount)
{
	//원하는 자리에 넣기 위한 계산..
	_width = 2 * _width / windowWidth;
	_height = 2 * _height / windowHeight;

	_startPointX = (2 * _startPointX / windowWidth) - 1;
	_startPointY = (2 * _startPointY / windowHeight) - 1;

	UVdata[0] = { DirectX::XMFLOAT3{_startPointX, _startPointY, 0.0f}, DirectX::XMFLOAT2{0.0f, 0.0f} };
	UVdata[1] = { DirectX::XMFLOAT3{_startPointX + _width, _startPointY, 0.0f}, DirectX::XMFLOAT2{1.0f, 0.0f} };
	UVdata[2] = { DirectX::XMFLOAT3{_startPointX + _width, _startPointY - _height, 0.0f}, DirectX::XMFLOAT2{1.0f, 1.0f} };
	UVdata[3] = { DirectX::XMFLOAT3{_startPointX, _startPointY - _height, 0.0f}, DirectX::XMFLOAT2{0.0f, 1.0f} };

	UIdata[0] = 0;
	UIdata[1] = 1;
	UIdata[2] = 3;
	UIdata[3] = 1;
	UIdata[4] = 2;
	UIdata[5] = 3;

	//텍스처를 로딩해봅시다
	std::wstring wTexturePath = _texturePath;
	gp->LoadTexture(wTexturePath, uPipeTexture);
	uiTexture[_name] = uPipeTexture;

	// 상수 버퍼 생성
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(AnimationData);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	this->d3d11Device->CreateBuffer(&bufferDesc, NULL, &animationBuffer);

	animationData.width = _aniCount * _aniWidth;	//1200
	animationData.aniWidth = _aniWidth;	//300
	animationData.totalFrames = static_cast<float>(_aniCount);//4
	animationData.frameIndex = 0;//0

	uiAnimation[_name] = animationData;
	uiAnimationBuffer[_name] = animationBuffer;
}

void UIRenderer::CreateAnimationPipeline(std::string _name)
{
	std::wstring path[2]
	{
		L"../Shader/compiled/UIAniVertexShader.cso",
		L"../Shader/compiled/UIPixelShader.cso",
	};

	UPipeline = new PipeLine();
	gp->CreateInputLayer(UPipeline->inputLayout, VertexD::defaultInputLayerDECS, 2, UPipeline->vertexShader, path[0]);
	gp->CreatePixelShader(UPipeline->pixelShader, path[1]);
	gp->CreateVertexBuffer<VertexD::Data>(this->UVdata, (UINT)(4 * VertexD::Size()), UPipeline->vertexBuffer, _name);
	gp->CreateIndexBuffer(this->UIdata, 6, UPipeline->IndexBuffer);
	UPipeline->primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UPipeline->vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(UPipeline->rasterizerState);

	uiPipeline[_name] = UPipeline;
}

void UIRenderer::UpdateAnimationBuffer(std::string _name)
{
	AnimationData _nowData = uiAnimation[_name];
	ID3D11Buffer* _nowBuffer = uiAnimationBuffer[_name];

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = (this->d3d11DeviceContext->Map(_nowBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	AnimationData* bufferData = static_cast<AnimationData*>(mappedResource.pData);
	*bufferData = _nowData;

	this->d3d11DeviceContext->Unmap(_nowBuffer, 0);

	this->d3d11DeviceContext->VSSetConstantBuffers(0, 1, &_nowBuffer);
}

void UIRenderer::SetPosition(float _x, float _y, std::string _name)
{
	std::pair<float, float>& objDelta = this->uiPosition[_name];
	objDelta.first = (2 * _x / windowWidth) - 1;
	objDelta.second = (2 * _y / windowHeight) - 1;
}

void UIRenderer::RenderChapturedImage(std::pair<ComPtr<ID3D11ShaderResourceView>, RECT>& _renderTings)
{
// 	_renderTings.second.top = this->windowHeight - _renderTings.second.top;
// 	_renderTings.second.bottom = this->windowWidth - _renderTings.second.bottom;

	PipeLine pip;
	//원하는 자리에 넣기 위한 계산..
	float width = static_cast<float>(_renderTings.second.right - _renderTings.second.left);
	float height = static_cast<float>(_renderTings.second.bottom - _renderTings.second.top);

	width = 2 * width / static_cast<float>(windowWidth);
	height = 2 * height / static_cast<float>(windowHeight);

	float startX = static_cast<float>((2.0f * static_cast<float>(_renderTings.second.left) / static_cast<float>(windowWidth)) - 1);
	float startY = static_cast<float>((2.0f * static_cast<float>(_renderTings.second.top) / static_cast<float>(windowHeight)) - 1);

	VertexD::Data UVdata[4];

	UVdata[0] = { DirectX::XMFLOAT3{startX, startY, 0.0f}, DirectX::XMFLOAT2{0.0f, 1.0f} };
	UVdata[1] = { DirectX::XMFLOAT3{startX + width, startY, 0.0f}, DirectX::XMFLOAT2{1.0f, 1.0f} };
	UVdata[2] = { DirectX::XMFLOAT3{startX + width, startY + height, 0.0f}, DirectX::XMFLOAT2{1.0f, 0.0f} };
	UVdata[3] = { DirectX::XMFLOAT3{startX, startY + height, 0.0f}, DirectX::XMFLOAT2{0.0f, 0.0f} };

	UINT UIdata[6];

	UIdata[0] = 3;
	UIdata[1] = 2;
	UIdata[2] = 1;
	UIdata[3] = 3;
	UIdata[4] = 1;
	UIdata[5] = 0;

	std::wstring path[2]
	{
		L"../Shader/compiled/UIVertexShader.cso",
		L"../Shader/compiled/UIPixelShader.cso",
	};

	gp->CreateInputLayer(pip.inputLayout, VertexD::defaultInputLayerDECS, 2, pip.vertexShader, path[0]);
	gp->CreatePixelShader(pip.pixelShader, path[1]);
	gp->CreateVertexBuffer<VertexD::Data>(UVdata, (UINT)(4 * VertexD::Size()), pip.vertexBuffer, "");
	gp->CreateIndexBuffer(UIdata, 6, pip.IndexBuffer);
	pip.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	pip.vertexStructSize = VertexD::Size();
	gp->CreateRasterizerState(pip.rasterizerState);

	gp->BindView();

	gp->BindPipeline(pip);
	BindMoveBuffer(0.0f, 0.0f);
	gp->ClearRenderTargetView();
	gp->SetTexture(0, 1, _renderTings.first);

	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);
}

void UIRenderer::CreateMoveBuffer()
{
	D3D11_BUFFER_DESC mbd = {};
	mbd.Usage = D3D11_USAGE_DYNAMIC;
	mbd.ByteWidth = sizeof(MovementBuffer);
	mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbd.MiscFlags = 0;
	mbd.StructureByteStride = 0;

	this->d3d11Device->CreateBuffer(&mbd, nullptr, this->movementBuffer.GetAddressOf());
}

void UIRenderer::BindMoveBuffer(float _x, float _y)
{

	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = this->d3d11DeviceContext->Map(this->movementBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(SUCCEEDED(hr));

	MovementBuffer* dataptr = static_cast<MovementBuffer*>(mappedResource.pData);

	dataptr->dx = _x;
	dataptr->dy = _y;

	this->d3d11DeviceContext->Unmap(this->movementBuffer.Get(), 0);
	this->d3d11DeviceContext->VSSetConstantBuffers(0, 1, movementBuffer.GetAddressOf());
}

