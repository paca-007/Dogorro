#include "GraphicsEngine.h"
#include <minwinbase.h>
#include "DXTKFont.h"
#include "DDSTextureLoader.h"
#include "LightHelper.h"
#include "RenderObject.h"
#include "UObject.h"
#include "Camera.h"
#include "Axes.h"
#include "LineObject.h"
#include "DeferredRenderer.h"
#include "ForwardRenderer.h"
#include "PostRenderer.h"
#include "UIRenderer.h"
#include "BackgroundRenderer.h"

#include <algorithm>
#include "ABone.h"
#include "Utils.h"

GraphicsEngine::GraphicsEngine()
	: featureLevel{}
	, d3d11Device(nullptr)
	, d3d11DeviceContext(nullptr)
	, hwnd(nullptr)
	, swapChain(nullptr)
	, renderTargetView(nullptr)
	, m4xMsaaQuality(-1)
	, depthStancilBuffer(nullptr)
	, depthStancilView(nullptr)
	, matrixBuffer(nullptr)
	, useMSAA(true)
	, writerDSS(nullptr)
	, writerRS(nullptr)
	, writer(nullptr)
	, lightBuffer(nullptr)
	, boneBuffer(nullptr)
	, dAxes(nullptr)
	, dLine(nullptr)
	, windowHeight(0)
	, windowWidth(0)
	, mainCamera(nullptr)
	, pixelOnOff(false)
	, flashOnOff(false)
	, whiteOutOnOff(false)
{
}

GraphicsEngine::~GraphicsEngine()
{
	this->textureResourceManager->UnLoadResource();
	this->VSResourceManager->UnLoadResource();
	this->PSResourceManager->UnLoadResource();
	this->IAResourceManager->UnLoadResource();

	delete this->writer;

	this->deferredRenderer->Finalize();
	//this->deferredTexture->Release();
	delete this->deferredRenderer;

	this->forwardRenderer->Finalize();
	delete this->forwardRenderer;

	this->postRenderer->Finalize();
	delete this->postRenderer;

	this->uiRenderer->Finalize();
	delete this->uiRenderer;

	this->backgroundRenderer->Finalize();
	delete this->backgroundRenderer;

	ID3D11RenderTargetView* nullViews[] = { nullptr };
	this->d3d11DeviceContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);
	this->d3d11DeviceContext->Flush();

#if defined(DEBUG) || defined(_DEBUG)
	ID3D11Debug* dxgiDebug;

	if (SUCCEEDED(this->d3d11Device->QueryInterface(IID_PPV_ARGS(&dxgiDebug))))
	{
		dxgiDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		dxgiDebug = nullptr;
	}
#endif

}

/// <summary>
/// 그래픽 엔진 초기화
/// </summary>
/// <param name="_hwnd">윈도우 핸들</param>
void GraphicsEngine::Initialize(HWND _hwnd)
{
	HRESULT hr = S_OK;
	this->hwnd = _hwnd;

	this->textureResourceManager = std::make_unique<ComResourceManager<ID3D11ShaderResourceView>>();
	this->VSResourceManager = std::make_unique<ComResourceManager<ID3D11VertexShader>>();
	this->PSResourceManager = std::make_unique<ComResourceManager<ID3D11PixelShader>>();
	this->IAResourceManager = std::make_unique<ComResourceManager<ID3D11InputLayout>>();

	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	CreateD3D11DeviceContext();
	CreateChainValue();
	CreateRenderTargetView();
	CreateDepthStencilBufferAndView();
	CreateViewport();
	CreateWriter();
	CreateMatrixBuffer();
	CreateLightingBffer();
	CreateBoneBuffer();
	CreateCameraBuffer();
	CreateRasterizerState();

	RECT windowSize = {0, 0, 0, 0};
	GetWindowRect(hwnd, &windowSize);
	assert(GetWindowRect(hwnd, &windowSize) && "cannot get window rectangle");

	this->windowWidth = windowSize.right - windowSize.left;
	this->windowHeight = windowSize.bottom - windowSize.top;

	//BackgroundRenderer
	this->backgroundRenderer = new BackgroundRenderer(d3d11Device.Get(), d3d11DeviceContext.Get(), depthStancilView.Get(), windowWidth, windowHeight);
	this->backgroundRenderer->Initailze(this);

	//defferedRenderer
	this->deferredRenderer = new DeferredRenderer(d3d11Device.Get(), d3d11DeviceContext.Get(), depthStancilView.Get(), windowWidth, windowHeight);
	this->deferredRenderer->Initailze(this);

	//UIRenderer
	this->uiRenderer = new UIRenderer(d3d11Device.Get(), d3d11DeviceContext.Get(), depthStancilView.Get(), windowWidth, windowHeight);
	this->uiRenderer->Initailze(this);

	//forwardRenderer
	this->forwardRenderer = new ForwardRenderer(d3d11Device.Get(), d3d11DeviceContext.Get(), depthStancilView.Get(), windowWidth, windowHeight);
	this->forwardRenderer->Initailze(this);

	//postRenderer
	this->postRenderer = new PostRenderer(d3d11Device.Get(), d3d11DeviceContext.Get(), depthStancilView.Get(), windowWidth, windowHeight);
	this->postRenderer->Initailze(this);

	dAxes = std::make_unique<Axes>(this);
	dLine = std::make_unique<LineObject>(this);

	//모든 그림을 그릴 마지막 SRV를 만든다.
	FVdata[0] = { DirectX::XMFLOAT3{-1.0f, 1.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 0.0f} };
	FVdata[1] = { DirectX::XMFLOAT3{1.0f, 1.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 0.0f} };
	FVdata[2] = { DirectX::XMFLOAT3{1.0f, -1.0f, 0.0f}, DirectX::XMFLOAT2{1.0f, 1.0f} };
	FVdata[3] = { DirectX::XMFLOAT3{-1.0f, -1.0f, 0.0f}, DirectX::XMFLOAT2{0.0f, 1.0f} };

	FIdata[0] = 0;
	FIdata[1] = 1;
	FIdata[2] = 3;
	FIdata[3] = 1;
	FIdata[4] = 2;
	FIdata[5] = 3;

	D3D11_BLEND_DESC bd;
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	this->d3d11Device->CreateBlendState(&bd, &defaultBlend);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = 1.0f;
	sampDesc.BorderColor[1] = 1.0f;
	sampDesc.BorderColor[2] = 1.0f;
	sampDesc.BorderColor[3] = 1.0f;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = this->d3d11Device->CreateSamplerState(&sampDesc, defaultSampler.GetAddressOf());
	assert(SUCCEEDED(hr));

	CreateFinalView();
	CreateFinalPipeline();

	BindSamplerState();


}

/// <summary>
/// 화면을 특정 색으로 초기화
/// </summary>
void GraphicsEngine::RenderClearView()
{
	ClearRenderTargetView();
	ClearFinalRenderTargetView();
	ClearDepthStencilView();
}

/// <summary>
/// 임시 오브젝트 렌더
/// </summary>
void GraphicsEngine::RenderTestThing(PipeLine& _pipline)
{
	this->d3d11DeviceContext->DrawIndexed(36, 0, 0);
}

void GraphicsEngine::Render(PipeLine& _pipline, int _indexSize)
{
	this->d3d11DeviceContext->DrawIndexed(_indexSize, 0, 0);
}

/// <summary>
/// D3D11 디바이스와 디바이스 컨텍스트 생성
/// </summary>
void GraphicsEngine::CreateD3D11DeviceContext()
{
	HRESULT hr;
	// D3D11 디바이스 생성
	hr = D3D11CreateDevice(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		this->createDeviceFlags,
		0,
		0,
		D3D11_SDK_VERSION,
		this->d3d11Device.GetAddressOf(),
		&this->featureLevel,
		this->d3d11DeviceContext.GetAddressOf()
	);
	assert(hr == S_OK && "cannot create d3d11 device");
}

/// <summary>
/// 교환 사슬 구조체의 생성
/// </summary>
void GraphicsEngine::CreateChainValue()
{
	RECT windowSize = {};
	GetWindowRect(hwnd, &windowSize);

	assert(GetWindowRect(hwnd, &windowSize) && "cannot get window rectangle");

	HRESULT hr = S_OK;

	// 스왑  체인 구조체
	DXGI_SWAP_CHAIN_DESC chainDesc = {};

	chainDesc.BufferDesc.Width = windowSize.right - windowSize.left;
	chainDesc.BufferDesc.Height = windowSize.bottom - windowSize.top;
	chainDesc.BufferDesc.RefreshRate.Numerator = 60;
	chainDesc.BufferDesc.RefreshRate.Denominator = 1;
	chainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	chainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	chainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	chainDesc.SampleDesc.Count = 1;
	chainDesc.SampleDesc.Quality = 0;



	chainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	chainDesc.BufferCount = 1;
	chainDesc.OutputWindow = this->hwnd;
	chainDesc.Windowed = true;
	chainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	chainDesc.Flags = 0;

	// 디바이스 인터페이스 가져오기
	IDXGIDevice* dxgiDevice = nullptr;
	hr = (d3d11Device->QueryInterface(
		__uuidof(IDXGIDevice),
		reinterpret_cast<void**>(&dxgiDevice)));
	assert(hr == S_OK && "cannot get DXGI device");

	// 디바이스 어뎁터 가져오기
	IDXGIAdapter* dxgiAdapter = nullptr;
	hr = (dxgiDevice->GetParent(
		__uuidof(IDXGIAdapter),
		reinterpret_cast<void**>(&dxgiAdapter)));
	assert(hr == S_OK && "cannot get DXGI adapter");

	// 디바이스 펙토리 가져오기
	IDXGIFactory* dxgiFactory = nullptr;
	hr = (dxgiAdapter->GetParent(
		__uuidof(IDXGIFactory),
		reinterpret_cast<void**>(&dxgiFactory)));
	assert(hr == S_OK && "cannot get DXGI factory");

	// 스왑 체인 생성
	hr = (dxgiFactory->CreateSwapChain(this->d3d11Device.Get(), &chainDesc, this->swapChain.GetAddressOf()));
	assert(hr == S_OK && "cannot create swapChain");

	// 사용한 인터페이스 제거
	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();
}

/// <summary>
/// 렌더 뷰 생성
/// </summary>
void GraphicsEngine::CreateRenderTargetView()
{
	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT hr;

	// 스왑 체인에서 버퍼를 가져옴
	hr = this->swapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&backBuffer)
	);
	assert(hr == S_OK && "cannot get buffer");

	// 렌더 타겟 뷰 생성
	hr = this->d3d11Device->CreateRenderTargetView(
		backBuffer,
		0,
		this->renderTargetView.GetAddressOf()
	);
	assert(hr == S_OK && "cannot create RenderTargetView");

	// 사용한 백 버퍼 인터페이스 반환	
	hr = backBuffer->Release();
	assert(hr == S_OK && "cannot release backBuffer");
}

/// <summary>
/// 깊이 스텐실 버퍼와 뷰 생성 생성
/// </summary>
void GraphicsEngine::CreateDepthStencilBufferAndView()
{
	HRESULT hr = S_OK;

	RECT windowSize = {};
	GetWindowRect(hwnd, &windowSize);

	assert(GetWindowRect(hwnd, &windowSize) && "cannot get window rectangle");

	// 구조체 값 채우기
	D3D11_TEXTURE2D_DESC depthStancilDesc = {};
	depthStancilDesc.Width = windowSize.right - windowSize.left;
	depthStancilDesc.Height = windowSize.bottom - windowSize.top;
	depthStancilDesc.MipLevels = 1;
	depthStancilDesc.ArraySize = 1;
	depthStancilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	depthStancilDesc.SampleDesc.Count = 1;
	depthStancilDesc.SampleDesc.Quality = 0;

	depthStancilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStancilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStancilDesc.CPUAccessFlags = 0;
	depthStancilDesc.MiscFlags = 0;

	// 깊이 스텐실 버퍼 생성
	hr = this->d3d11Device->CreateTexture2D(
		&depthStancilDesc,
		nullptr,
		this->depthStancilBuffer.GetAddressOf()
	);
	assert(hr == S_OK && "cannot create depth-stancil buffer");

	// 깊이 스텐실 뷰 생성
	hr = d3d11Device->CreateDepthStencilView(
		this->depthStancilBuffer.Get(),
		0,
		this->depthStancilView.GetAddressOf()
	);
	assert(hr == S_OK && "cannot create depth-stancil view");
}

/// <summary>
/// 뷰포트 생성
/// </summary>
void GraphicsEngine::CreateViewport()
{
	RECT windowSize = {};
	GetWindowRect(hwnd, &windowSize);

	assert(GetWindowRect(hwnd, &windowSize) && "cannot get window rectangle");

	D3D11_VIEWPORT vp = {};
	vp.Width = static_cast<FLOAT>(windowSize.right - windowSize.left);
	vp.Height = static_cast<FLOAT>(windowSize.bottom - windowSize.top);
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	this->d3d11DeviceContext->RSSetViewports(1, &vp);
}

/// <summary>
/// 생성된 뷰를 렌더링 파이프라인에 묶는다.
/// </summary>
void GraphicsEngine::BindView()
{
	this->d3d11DeviceContext->OMSetRenderTargets(
		1,
		this->finalRenderTartView.GetAddressOf(),
		this->depthStancilView.Get()
	);

}

void GraphicsEngine::BindFinalView()
{
	CreateViewport();
	// 뷰를 렌더링 파이프라인에 바인드
	this->d3d11DeviceContext->OMSetRenderTargets(
		1,
		this->renderTargetView.GetAddressOf(),
		this->depthStancilView.Get()
	);
}

/// <summary>
/// 단일 텍스쳐 로드
/// </summary>
/// <param name="_path">경로</param>
/// <param name="_resourceView">output</param>
void GraphicsEngine::LoadTexture(std::wstring _path, ComPtr<ID3D11ShaderResourceView>& _resourceView)
{
	HRESULT hr;

	string sPath = Utils::ToString(_path);
	_resourceView = this->textureResourceManager->GetResource(sPath);

	if (_resourceView != nullptr)
	{
		return;
	}

	std::wstring format = _path.substr(_path.length() - 3, _path.length());
	std::transform(format.begin(), format.end(), format.begin(), ::tolower);

	DirectX::ScratchImage image;
	ID3D11Resource* texResource = nullptr;

	if (format == L"tga")
	{
		CreateTextureDataFromTGA(_path, &image);
	}
	else if (format == L"dds")
	{
		CreateTextureDataFromDDS(_path, &image);
	}
	else if (format == L"png" || format == L"jpg" || format == L"jpge" || format == L"tiff" || format == L"bmp")
	{
		CreateTextureDataFromWIC(_path, &image);
	}

	hr = DirectX::CreateTexture(this->d3d11Device.Get(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), &texResource);
	assert(SUCCEEDED(hr) && "cannot create image when load TGA data");

	hr = this->d3d11Device->CreateShaderResourceView(texResource, nullptr, _resourceView.GetAddressOf());
	assert(SUCCEEDED(hr) && "cannot create image when load TGA data");

	this->textureResourceManager->StoreResource(sPath, _resourceView);

	texResource->Release();
}

/// <summary>
/// 텍스쳐 배열 로드
/// </summary>
/// <param name="_path">경로</param>
/// <param name="_resourceView">output</param>
void GraphicsEngine::LoadTexture(std::vector<std::wstring> _path, ComPtr<ID3D11ShaderResourceView>& _resourceView)
{
	HRESULT hr = S_OK;

	UINT arraySize = static_cast<UINT>(_path.size());

	D3D11_SUBRESOURCE_DATA* texture = new D3D11_SUBRESOURCE_DATA[arraySize];

	ID3D11Resource** texResource = new ID3D11Resource * [arraySize];
	DirectX::ScratchImage* image = new DirectX::ScratchImage[arraySize];
	*texResource = nullptr;
	for (UINT i = 0; i < arraySize; i++)
	{
		std::wstring format = _path[i].substr(_path[i].length() - 3, _path[i].length());
		std::transform(format.begin(), format.end(), format.begin(), ::tolower);

		if (format == L"tga")
		{
			CreateTextureDataFromTGA(_path[i], &image[i]);
		}
		else if (format == L"dds")
		{
			CreateTextureDataFromDDS(_path[i], &image[i]);
		}
		else if (format == L"png" || format == L"jpg" || format == L"jpge" || format == L"tiff" || format == L"bmp")
		{
			CreateTextureDataFromWIC(_path[i], &image[i]);
		}
		else
		{
			_resourceView.GetAddressOf()[i] = nullptr;
			continue;
		}

		hr = DirectX::CreateTexture(
			this->d3d11Device.Get(),
			image[i].GetImages(),
			image[i].GetImageCount(),
			image[i].GetMetadata(),
			&texResource[i]
		);
		assert(SUCCEEDED(hr) && "cannot create texture");

	}
	assert(*texResource);
	hr = this->d3d11Device->CreateShaderResourceView(*texResource, nullptr, _resourceView.GetAddressOf());
	assert(SUCCEEDED(hr) && "cannot create shader resource view");
}

void GraphicsEngine::UIRender(PipeLine& _pipline, int _indexSize)
{
	this->d3d11DeviceContext->DrawIndexed(_indexSize, 0, 0);
}

/// <summary>
/// 카메라 생성
/// </summary>
/// <param name="_camera">output</param>
/// <param name="_w">넓이</param>
/// <param name="_h">높이</param>
void GraphicsEngine::CreateCamera(ICamera** _camera, float _w, float _h)
{
	(*_camera) = new Camera(_w, _h);
}

/// <summary>
/// 렌더 할 카메라 설정
/// </summary>
/// <param name="_camera"></param>
void GraphicsEngine::SetMainCamera(ICamera* _camera)
{
	this->mainCamera = dynamic_cast<Camera*>(_camera);
}

/// <summary>
/// 주어진 카메라가 그래픽 엔진에서 렌더링 되는 카메라인지
/// </summary>
/// <param name="_camera">카메라</param>
/// <returns>결과</returns>
bool GraphicsEngine::IsMainCamera(ICamera* _camera) const
{
	return this->mainCamera == _camera;
}

Camera* GraphicsEngine::GetCamera()
{
	return this->mainCamera;
}

/// <summary>
/// 디버깅용 기본 축 그리기
/// </summary>
void GraphicsEngine::DrawDefaultAxes()
{
	this->dAxes->Render(this);
}

/// <summary>
/// 디버깅용 기본 선 그리기
/// </summary>
void GraphicsEngine::DrawDefaultLine()
{
	this->dLine->Render(this);
}

void GraphicsEngine::CreateWriter()
{
	// 폰트용 DSS
	D3D11_DEPTH_STENCIL_DESC equalsDesc;
	ZeroMemory(&equalsDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	equalsDesc.DepthEnable = true;
	equalsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;		// 깊이버퍼에 쓰기는 한다
	equalsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	this->d3d11Device->CreateDepthStencilState(&equalsDesc, writerDSS.GetAddressOf());

	// Render State 중 Rasterizer State
	D3D11_RASTERIZER_DESC solidDesc;
	ZeroMemory(&solidDesc, sizeof(D3D11_RASTERIZER_DESC));
	solidDesc.FillMode = D3D11_FILL_SOLID;
	solidDesc.CullMode = D3D11_CULL_BACK;
	solidDesc.FrontCounterClockwise = false;
	solidDesc.DepthClipEnable = true;

	this->d3d11Device->CreateRasterizerState(&solidDesc, writerRS.GetAddressOf());
	this->writer = new DXTKFont();
	this->writer->Create(this->d3d11Device.Get(), this->writerRS.Get(), this->writerDSS.Get());
}

/// <summary>
/// 파일을 바이트로 읽기
/// </summary>
/// <param name="File">경로</param>
/// <returns>바이트 벡터</returns>
std::vector<byte> GraphicsEngine::Read(std::string File)
{
	std::vector<byte> Text;
	std::fstream file(File, std::ios::in | std::ios::ate | std::ios::binary);

	if (file.is_open()) {
		Text.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(reinterpret_cast<char*>(&Text[0]), Text.size());
		file.close();
	}

	return Text;
}

/// <summary>
/// input layer와 vertexShader를 생성한다.
/// </summary>
/// <param name="_inputLayout">IL</param>
/// <param name="_defaultInputLayerDECS">요소 구조체</param>
/// <param name="_numberOfElement">요소 갯수</param>
/// <param name="_vs">VS</param>
/// <param name="_path">VS의 경로</param>
void GraphicsEngine::CreateInputLayer(ComPtr<ID3D11InputLayout>& _inputLayout, D3D11_INPUT_ELEMENT_DESC* _defaultInputLayerDECS, UINT _numberOfElement, ComPtr<ID3D11VertexShader>& _vs, std::wstring _path)
{
	HRESULT hr = S_OK;
	std::string vs = Utils::ToString(_path);

	_vs = this->VSResourceManager->GetResource(vs);
	if (_vs != nullptr)
	{
		_inputLayout = this->IAResourceManager->GetResource(vs);
		return;
	}

	auto vsByteCode = Read(vs);

	hr = this->d3d11Device->CreateVertexShader(vsByteCode.data(), vsByteCode.size(), nullptr, _vs.GetAddressOf());
	assert(SUCCEEDED(hr) && "Cannot Read VertexShader");

	hr = this->d3d11Device->CreateInputLayout(
		_defaultInputLayerDECS,
		_numberOfElement,
		vsByteCode.data(),
		vsByteCode.size(),
		_inputLayout.GetAddressOf()
	);
	assert(SUCCEEDED(hr) && "cannot create input layer");

	this->VSResourceManager->StoreResource(vs, _vs);
	this->IAResourceManager->StoreResource(vs, _inputLayout);
}

/// <summary>
/// 픽셸 셰이더 로드
/// </summary>
/// <param name="_pipline">픽셀 셰이더가 있는 파이프라인</param>
/// <param name="_path">경로</param>
void GraphicsEngine::CreatePixelShader(ComPtr<ID3D11PixelShader>& _ps, std::wstring _path)
{
	HRESULT hr = S_OK;
	std::string ps = "";
	ps = Utils::ToString(_path);
	auto psByteCode = Read(ps);

	hr = this->d3d11Device->CreatePixelShader(psByteCode.data(), psByteCode.size(), nullptr, _ps.GetAddressOf());
	assert(SUCCEEDED(hr) && "Cannot Read PixelShader");
}

/// <summary>
/// 렌더 타겟 뷰 초기화
/// </summary>
void GraphicsEngine::ClearRenderTargetView()
{
	// 임시 색 ( R G B A )
	float bgRed[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// 렌더 타겟을 지정한 색으로 초기화
	this->d3d11DeviceContext->ClearRenderTargetView(
		this->renderTargetView.Get(),
		bgRed
	);
}

/// <summary>
/// 깊이 스텐실 뷰 초기화
/// </summary>
void GraphicsEngine::ClearDepthStencilView()
{
	// 깊이 스텐실 뷰 초기화
	this->d3d11DeviceContext->ClearDepthStencilView(
		this->depthStancilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0
	);
}
/// <summary>
/// 최종 그림을 그리는 렌더 타겟 뷰 클리어
/// </summary>
void GraphicsEngine::ClearFinalRenderTargetView()
{
	// 임시 색 ( R G B A )
	float bgRed[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// 렌더 타겟을 지정한 색으로 초기화
	this->d3d11DeviceContext->ClearRenderTargetView(
		this->finalRenderTartView.Get(),
		bgRed
	);
}

/// <summary>
/// 레스터라이저 생성
/// </summary>
/// <param name="_rasterizerState">반환 받을 레스터라이저</param>
void GraphicsEngine::CreateRasterizerState(ComPtr<ID3D11RasterizerState>& _rasterizerState)
{
	HRESULT hr = S_OK;

	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = true;

	hr = this->d3d11Device->CreateRasterizerState(&rsDesc, _rasterizerState.GetAddressOf());
	assert(SUCCEEDED(hr) && "cannot create Rasterizser State");
}

void GraphicsEngine::CreateRasterizerState()
{
	CreateRasterizerState(this->rasterizerState);
}

void GraphicsEngine::CreateMatrixBuffer()
{
	D3D11_BUFFER_DESC mbd = {};
	mbd.Usage = D3D11_USAGE_DYNAMIC;
	mbd.ByteWidth = sizeof(MatrixBufferType);
	mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbd.MiscFlags = 0;
	mbd.StructureByteStride = 0;

	this->d3d11Device->CreateBuffer(&mbd, nullptr, matrixBuffer.GetAddressOf());
}

/// <summary>
/// 파라미터 설정
/// </summary>
/// <param name="_w">월드 TM</param>
/// <param name="_v">뷰포트 TM</param>
/// <param name="_p">프로젝션 TM</param>
void GraphicsEngine::BindMatrixParameter(DirectX::XMMATRIX _w)
{
	HRESULT hr;

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = this->d3d11DeviceContext->Map(this->matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(SUCCEEDED(hr));

	MatrixBufferType* dataptr = (MatrixBufferType*)mappedResource.pData;

	dataptr->world = _w;
	dataptr->wvp = _w * this->mainCamera->GetViewTM() * this->mainCamera->GetProjectionTM();
	dataptr->worldInversTranspose = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, _w));

	dataptr->world = DirectX::XMMatrixTranspose(_w);
	dataptr->wvp = DirectX::XMMatrixTranspose(dataptr->wvp);
	dataptr->worldInversTranspose = DirectX::XMMatrixTranspose(dataptr->worldInversTranspose);

	this->d3d11DeviceContext->VSSetConstantBuffers(0, 1, this->matrixBuffer.GetAddressOf());
	this->d3d11DeviceContext->Unmap(matrixBuffer.Get(), 0);
}


void GraphicsEngine::CreateBoneBuffer()
{
	D3D11_BUFFER_DESC mbd = {};
	mbd.Usage = D3D11_USAGE_DYNAMIC;
	mbd.ByteWidth = sizeof(BonesBufferType);
	mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbd.MiscFlags = 0;
	mbd.StructureByteStride = 0;

	this->d3d11Device->CreateBuffer(&mbd, nullptr, this->boneBuffer.GetAddressOf());
}

void GraphicsEngine::BindBonesData(std::vector<RenderObject*>& _bones, DirectX::XMMATRIX _worldTM)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = this->d3d11DeviceContext->Map(this->boneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(SUCCEEDED(hr));

	BonesBufferType* dataptr = static_cast<BonesBufferType*>(mappedResource.pData);
	for (int i = 0; i < (int)_bones.size(); i++)
	{
		DirectX::XMMATRIX boneWorldTM = _bones[i]->nodeTM;
		DirectX::XMMATRIX boneNodeTM = _bones[i]->originalNodeTM;

		DirectX::XMMATRIX skinWorldTM = _worldTM;
		DirectX::XMMATRIX skinWorldTMInverse = DirectX::XMMatrixInverse(nullptr, skinWorldTM);

		DirectX::XMMATRIX boneoffsetTM = boneNodeTM * skinWorldTMInverse;
		DirectX::XMMATRIX boneoffsetTM_Inverse = DirectX::XMMatrixInverse(nullptr, boneoffsetTM);

		DirectX::XMMATRIX finalboneTM = boneoffsetTM_Inverse * boneWorldTM;

		dataptr->bones[i] = DirectX::XMMatrixTranspose(finalboneTM);
	}
	this->d3d11DeviceContext->Unmap(this->boneBuffer.Get(), 0);
	this->d3d11DeviceContext->VSSetConstantBuffers(2, 1, boneBuffer.GetAddressOf());
}

void GraphicsEngine::BindBonesData(std::vector<std::shared_ptr<ABone>>& _bones, DirectX::XMMATRIX _worldTM)
{
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = this->d3d11DeviceContext->Map(this->boneBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(SUCCEEDED(hr));

	BonesBufferType* dataptr = static_cast<BonesBufferType*>(mappedResource.pData);

	// 	std::vector<DirectX::SimpleMath::Matrix> toRootTransforms;
	// 	toRootTransforms.reserve(128);
	// 
	// 	toRootTransforms.push_back(_bones[0]->transform);
	// 	for (size_t i = 1; i < _bones.size(); ++i)
	// 	{
	// 		auto& parent = toRootTransforms[_bones[i]->parentIndex];
	// 		toRootTransforms.push_back(_bones[i]->transform * parent);
	// 	}

	for (int i = 0; i < (int)_bones.size(); i++)
	{
		DirectX::XMMATRIX boneWorldTM = _bones[i]->transform;
		DirectX::XMMATRIX boneNodeTM = _bones[i]->oriinalWorldTransform;

		DirectX::XMMATRIX skinWorldTM = _worldTM;
		DirectX::XMMATRIX skinWorldTMInverse = DirectX::XMMatrixInverse(nullptr, skinWorldTM);

		DirectX::XMMATRIX boneoffsetTM = boneNodeTM * skinWorldTMInverse;
		DirectX::XMMATRIX boneoffsetTM_Inverse = DirectX::XMMatrixInverse(nullptr, boneoffsetTM);

		DirectX::XMMATRIX finalboneTM = boneoffsetTM_Inverse * boneWorldTM;

		dataptr->bones[i] = DirectX::XMMatrixTranspose(_bones[i]->offsetTrasform * _bones[i]->transform); //  _bones[i]->transform);
	}
	this->d3d11DeviceContext->Unmap(this->boneBuffer.Get(), 0);
	this->d3d11DeviceContext->VSSetConstantBuffers(1, 1, boneBuffer.GetAddressOf());
}

void GraphicsEngine::CreateLightingBffer()
{
	D3D11_BUFFER_DESC mbd = {};
	mbd.Usage = D3D11_USAGE_DYNAMIC;
	mbd.ByteWidth = sizeof(LightingBufferType);
	mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbd.MiscFlags = 0;
	mbd.StructureByteStride = 0;

	this->d3d11Device->CreateBuffer(&mbd, nullptr, this->lightBuffer.GetAddressOf());
}

void GraphicsEngine::BindLightingParameter(DirectionalLight _directionLight[], UINT _lightCount)
{
	HRESULT hr;

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = this->d3d11DeviceContext->Map(this->lightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(SUCCEEDED(hr));

	LightingBufferType* dataptr = static_cast<LightingBufferType*>(mappedResource.pData);

	dataptr->dirLights[0] = _directionLight[0];
	dataptr->dirLights[1] = _directionLight[1];
	dataptr->dirLights[2] = _directionLight[2];

	dataptr->lightCount = _lightCount;

	dataptr->eyePosW = this->mainCamera->GetPositoin();

	this->d3d11DeviceContext->Unmap(this->lightBuffer.Get(), 0);
	this->d3d11DeviceContext->PSSetConstantBuffers(1, 1, this->lightBuffer.GetAddressOf());
}

/// <summary>
/// 파이프라인 바인딩
/// </summary>
/// <param name="_pipline"></param>
void GraphicsEngine::BindPipeline(PipeLine& _pipline)
{
	this->d3d11DeviceContext->IASetInputLayout(_pipline.inputLayout.Get());
	this->d3d11DeviceContext->IASetPrimitiveTopology(_pipline.primitiveTopology);

	UINT stride = _pipline.vertexStructSize;
	UINT offset = 0;

	this->d3d11DeviceContext->IASetIndexBuffer(_pipline.IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	this->d3d11DeviceContext->IASetVertexBuffers(0, 1, _pipline.vertexBuffer.GetAddressOf(), &stride, &offset);
	this->d3d11DeviceContext->RSSetState(this->rasterizerState.Get());
	this->d3d11DeviceContext->VSSetShader(_pipline.vertexShader.Get(), nullptr, 0);
	this->d3d11DeviceContext->PSSetShader(_pipline.pixelShader.Get(), nullptr, 0);
}

void GraphicsEngine::WriteText(int _x, int _y, float _rgba[4], TCHAR* _text)
{
	DirectX::XMFLOAT4 color = { _rgba[0], _rgba[1] , _rgba[2] , _rgba[3] };
	this->writer->DrawTextColor(_x, _y, color, _text);
}

/// <summary>
/// DDS 파일에서 이미지 데이터 생성
/// </summary>
/// <param name="_path">경로</param>
/// <param name="_image">output</param>
void GraphicsEngine::CreateTextureDataFromDDS(std::wstring _path, ScratchImage* _image)
{
	HRESULT hr = S_OK;

	hr = DirectX::LoadFromDDSFile(_path.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, *_image);
	assert(SUCCEEDED(hr) && "cannot create image when load DDS data");
}

/// <summary>
/// TGA 파일에서 이미지 데이터 생성
/// </summary>
/// <param name="_path">경로</param>
/// <param name="_image">output</param>
void GraphicsEngine::CreateTextureDataFromTGA(std::wstring _path, ScratchImage* _image)
{
	HRESULT hr = S_OK;

	hr = DirectX::LoadFromTGAFile(_path.c_str(), nullptr, *_image);
	assert(SUCCEEDED(hr) && "cannot create image when load TGA data");

	// 	hr = DirectX::CreateTexture(this->d3d11Device, image.GetImages(), image.GetImageCount(), image.GetMetadata(), &texResource);
	// 	assert(SUCCEEDED(hr) && "cannot create image when load TGA data");
	// 
	// 	hr = this->d3d11Device->CreateShaderResourceView(texResource, nullptr, _resourceView);
	// 	assert(SUCCEEDED(hr) && "cannot create image when load TGA data");
}

/// <summary>
/// WIC (PNG, JPG 등) 파일에서 이미지 데이터 생성
/// </summary>
/// <param name="_path">경로</param>
/// <param name="_image">output</param>
void GraphicsEngine::CreateTextureDataFromWIC(std::wstring _path, ScratchImage* _image)
{
	HRESULT hr = S_OK;

	hr = DirectX::LoadFromWICFile(_path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, *_image);
	assert(SUCCEEDED(hr) && "cannot create image when load WIC data");
}

/// <summary>
/// 카메라 버퍼 생성
/// </summary>
void GraphicsEngine::CreateCameraBuffer()
{
	D3D11_BUFFER_DESC mbd = {};
	mbd.Usage = D3D11_USAGE_DYNAMIC;
	mbd.ByteWidth = sizeof(CameraBufferType);
	mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mbd.MiscFlags = 0;
	mbd.StructureByteStride = 0;

	HRESULT hr;

	hr = this->d3d11Device->CreateBuffer(&mbd, nullptr, this->cameraBuffer.GetAddressOf());
	return;
}

/// <summary>
/// 최종 그림을 그리게 될 SRV를 받아서 화면에 띄우는 역할을 하게 될 파이프라인
/// </summary>
void GraphicsEngine::CreateFinalPipeline()
{
	std::wstring vsPath = L"../Shader/compiled/FinalVertexShader.cso";
	std::wstring psPath = L"../Shader/compiled/FinalPixelShader.cso";

	this->CreateInputLayer(this->finalPipeline.inputLayout, VertexD::defaultInputLayerDECS, 2, this->finalPipeline.vertexShader, vsPath);
	this->CreatePixelShader(this->finalPipeline.pixelShader, psPath);
	this->CreateVertexBuffer<VertexD::Data>(this->FVdata, (UINT)(4 * VertexD::Size()), this->finalPipeline.vertexBuffer, "FinalDrawVB");
	this->CreateIndexBuffer(this->FIdata, 6, this->finalPipeline.IndexBuffer);
	this->finalPipeline.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	this->finalPipeline.vertexStructSize = VertexD::Size();
	this->CreateRasterizerState(this->finalPipeline.rasterizerState);
}

/// <summary>
/// 카메라를 픽셀 셰이더에 바인딩
/// </summary>
void GraphicsEngine::BindCameraAtPS()
{
	HRESULT hr;

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	hr = this->d3d11DeviceContext->Map(this->cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(SUCCEEDED(hr));

	CameraBufferType* dataptr = static_cast<CameraBufferType*>(mappedResource.pData);
	DirectX::XMFLOAT3 dir = this->mainCamera->GetLook();
	DirectX::XMFLOAT3 p = this->mainCamera->GetPositoin();
	dataptr->lookDir = DirectX::XMFLOAT4(dir.x, dir.y, dir.z, 1.0f);
	dataptr->pos = DirectX::XMFLOAT4(p.x, p.y, p.z, 1.0f);

	this->d3d11DeviceContext->Unmap(this->cameraBuffer.Get(), 0);
	this->d3d11DeviceContext->PSSetConstantBuffers(0, 1, this->cameraBuffer.GetAddressOf());
}

void GraphicsEngine::CreateFinalView()
{
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc{};
	renderTargetTextureDesc.Width = static_cast<UINT>(windowWidth);
	renderTargetTextureDesc.Height = static_cast<UINT>(windowHeight);
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, this->finalTexture2D.GetAddressOf());

	this->d3d11Device->CreateRenderTargetView(this->finalTexture2D.Get(), 0, this->finalRenderTartView.GetAddressOf());

	// 2. ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = renderTargetTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	this->d3d11Device->CreateShaderResourceView(this->finalTexture2D.Get(), &shaderResourceViewDesc, this->finalSRV.GetAddressOf());
}

void GraphicsEngine::BindSamplerState()
{

	this->d3d11DeviceContext->PSSetSamplers(0, 1, defaultSampler.GetAddressOf());
}

std::vector<float> GraphicsEngine::PickingWorldPoint(float _x, float _y)
{
	return std::vector<float>();
}

void GraphicsEngine::Resize()
{

}

void GraphicsEngine::GetDT(float _dt)
{
	dt = _dt;
}

void GraphicsEngine::ChaptuerScreen(std::string _name)
{
	ComPtr <ID3D11RenderTargetView> nowRenderTartView;
	ComPtr <ID3D11ShaderResourceView> nowSRV;
	ComPtr <ID3D11Texture2D> nowTexture2D;

	D3D11_TEXTURE2D_DESC renderTargetTextureDesc{};
	renderTargetTextureDesc.Width = static_cast<UINT>(windowWidth);
	renderTargetTextureDesc.Height = static_cast<UINT>(windowHeight);
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	this->d3d11Device->CreateTexture2D(&renderTargetTextureDesc, nullptr, nowTexture2D.GetAddressOf());

	this->d3d11DeviceContext->CopyResource(nowTexture2D.Get(), this->finalTexture2D.Get());

	this->d3d11Device->CreateRenderTargetView(nowTexture2D.Get(), 0, nowRenderTartView.GetAddressOf());

	// 2. ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = renderTargetTextureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	this->d3d11Device->CreateShaderResourceView(nowTexture2D.Get(), &shaderResourceViewDesc, nowSRV.GetAddressOf());

	this->screenImageMap[_name] = nowSRV;
}

void GraphicsEngine::ShowChaptueredImage(std::string _name, RECT _rect)
{
	if (this->screenImageMap.find(_name) == this->screenImageMap.end())
	{
		return;
	}
	this->queueImage.push(std::make_pair(this->screenImageMap[_name], _rect));
}

void GraphicsEngine::SetFlashEffect(float deltaTime, bool _isOnOff)
{
	flashOnOff = _isOnOff;
	flashCount = 0;
}

void GraphicsEngine::PlayFlashEffect()
{
	if (flashOnOff == true)
	{
		float nowTime = this->postRenderer->GetTime();
		nowTime += 100;
		flashCount++;
		if (flashCount == 10)
		{
			this->postRenderer->SetTime(nowTime);
			flashCount = 0;
		}

		if (nowTime >= 4000)
		{
			flashOnOff = false;
			flashCount = 0;
			this->postRenderer->SetTime(0);
		}
	}
}

void GraphicsEngine::SetPixelateEffect()
{
	if (pixelOnOff == true)
		pixelOnOff = false;
	else
		pixelOnOff = true;
}

void GraphicsEngine::SetWhiteOutEffect(float deltaTime, bool _isOnOff)
{
	whiteOutOnOff = _isOnOff;
	whiteOutCount = 0;
}

void GraphicsEngine::PlayWhiteOutEffect()
{
	if (whiteOutOnOff == true)
	{
		float nowTime = this->postRenderer->GetTime();
		nowTime += 400;
		flashCount++;
		if (flashCount == 100)
		{
			this->postRenderer->SetTime(nowTime);
			flashCount = 0;
		}

		if (nowTime >= 4000)
		{
			whiteOutOnOff = false;
			flashCount = 0;
			this->postRenderer->SetTime(0);
		}
	}
}

/// <summary>
/// 텍스쳐 리소스를 픽셀 셰이더에 적용
/// </summary>
/// <param name="_start">리소스 슬릇</param>
/// <param name="_viewNumbers">리소스 갯수</param>
/// <param name="_resourceView">리소스 뷰 포인터</param>
void GraphicsEngine::SetTexture(UINT _start, UINT _viewNumbers, ComPtr<ID3D11ShaderResourceView>& _resourceView)
{
	this->d3d11DeviceContext->PSSetShaderResources(_start, _viewNumbers, _resourceView.GetAddressOf());
}

/// <summary>
/// 인덱스 버퍼 생성
/// </summary>
/// <param name="_indices">인덱스 배열</param>
/// <param name="_size">배열의 사이즈</param>
/// <param name="_indexbuffer">버퍼를 반환받을 포인터</param>
void GraphicsEngine::CreateIndexBuffer(UINT* _indices, UINT _size, ComPtr<ID3D11Buffer>& _indexbuffer)
{
	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * _size;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA iinitData = {};
	iinitData.pSysMem = _indices;

	hr = this->d3d11Device->CreateBuffer(&ibd, &iinitData, _indexbuffer.GetAddressOf());
	assert(SUCCEEDED(hr) && "cannot create Index Buffer");
}

/// <summary>
/// 그리기 작업을 종료하고 출력
/// </summary>
void GraphicsEngine::endDraw()
{


	this->deferredRenderer->EndRender();

	//여기서 디퍼드텍스쳐를 받아와야한다. 그리고 포워드친구한테 던져줘서 깊이계산을 해야 됨.
	this->deferredRenderer->UpdateTexture();
	this->forwardRenderer->UpdateDeferredTexture();

	//여기서 Forward가 렌더링 되었음 좋것다.
	//this->forwardRenderer->ForwardRenderClearView();
	this->forwardRenderer->BeginRender();
	this->forwardRenderer->EndRender();

	//여기서 postProcessing이 될 예정이다..
	this->postRenderer->BeginRender();
	this->postRenderer->EndRender();

	//postRendering중 Pixelate는 모든게 그려진 이후에 잘라야한다.
	//일단은 꺼둡시다.
	if (pixelOnOff == true)
	{
		this->postRenderer->FirstpassPixel();
	}

	//여기서 UI가 렌더링 되었음 좋것다
	// BindSamplerState();
	this->uiRenderer->BeginRender();
	this->uiRenderer->EndRender();
	
	FLOAT temp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	
	//최종적으로 받아서 그려둔 친구를 그립시다!
	//화면 전환 처리는 UI와 함께 되는게 맞다.
	if (flashOnOff == true)
	{
		PlayFlashEffect();
		this->postRenderer->FirstPassFlash();
	}

	//최종적으로 받아서 그려둔 친구를 그립시다!
	//화면 전환 처리는 UI와 함께 되는게 맞다.
	if (whiteOutOnOff == true)
	{
		PlayWhiteOutEffect();
		this->postRenderer->FirstPassFlash();
	}

	this->d3d11DeviceContext->OMSetBlendState(nullptr, temp, 0xffffffff);
	
	while (!this->queueImage.empty())
	{
		auto& nowImage = this->queueImage.front();
		this->uiRenderer->RenderChapturedImage(nowImage);
		this->queueImage.pop();
	}

	this->BindFinalView();
	this->ClearDepthStencilView();
	this->BindPipeline(finalPipeline);

	this->d3d11DeviceContext->PSSetShaderResources(0, 1, this->finalSRV.GetAddressOf());
	this->d3d11DeviceContext->DrawIndexed(6, 0, 0);

	this->swapChain->Present(0, 0);
}

/// <summary>
/// 그리기 작업 시작시 초기화
/// </summary>
void GraphicsEngine::begineDraw()
{
	// BindView();
	RenderClearView();

	std::wstring dt = L" ";
	float w[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	WriteText(200, 12, w, const_cast<TCHAR*>(dt.c_str()));

	FLOAT temp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	this->d3d11DeviceContext->OMSetBlendState(defaultBlend, temp, 0xffffffff);

	this->backgroundRenderer->BeginRender();
	this->backgroundRenderer->EndRender();

	this->deferredRenderer->BeginRender();
}

