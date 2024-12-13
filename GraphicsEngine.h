#pragma once
#include "pch.h"
#include "IGraphicsEngine.h"
#include "Vertex.h"
#include "color.h"
#include "pipeline.h"
#include "LightHelper.h"
#include "DirectXTex.h"
#include "ComResourceManager.h"
#include <queue>

/// <summary>
/// D3D 그래픽 엔진
/// 작성자 : 김형환
/// 최초 작성일 : 2023/09/06
/// 최종 수정일 : 2024/02/07
/// 
/// Dx11을 이용한 3D 그래픽 엔진
/// </summary>

class DXTKFont;
class RenderObject;
class FObject;
class UObject;
class Camera;
class ICamera;
class Axes;
class LineObject;
class DeferredRenderer;
class UIRenderer;
class ForwardRenderer;
class PostRenderer;
class BackgroundRenderer;
class ABone;

class GraphicsEngine
	: public IGraphicsEngine
{
private:

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX wvp;
		DirectX::XMMATRIX worldInversTranspose;
		Material material;
	};

	struct LightingBufferType
	{
		// 직사광선 (3종류)
		DirectionalLight dirLights[3];
		UINT lightCount;
		DirectX::XMFLOAT3 eyePosW;
	};

	struct CameraBufferType
	{
		DirectX::XMFLOAT4 lookDir;
		DirectX::XMFLOAT4 pos;
	};

	struct BonesBufferType
	{
		DirectX::XMMATRIX bones[1000];
	};

	// D3D 기능 레벨
	D3D_FEATURE_LEVEL featureLevel;

	// D3D 디바이스와 디바이스 컨텍스트
	ComPtr<ID3D11Device> d3d11Device;
	ComPtr<ID3D11DeviceContext> d3d11DeviceContext;

	// 디바이스 플래그
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG;

	// 윈도우 핸들러
	HWND hwnd;

	int windowWidth;
	int windowHeight;

	// 스왑 체인
	ComPtr<IDXGISwapChain> swapChain;

	// 렌더 타겟 뷰
	ComPtr<ID3D11RenderTargetView> renderTargetView;

	// m4xMass 수준
	UINT m4xMsaaQuality;

	//DepthStencil
	ComPtr<ID3D11Texture2D> depthStancilBuffer;
	ComPtr<ID3D11DepthStencilView> depthStancilView;

	ComPtr<ID3D11Buffer> matrixBuffer;
	ComPtr<ID3D11Buffer> lightBuffer;
	ComPtr<ID3D11Buffer> boneBuffer;
	ComPtr<ID3D11Buffer> cameraBuffer;

	bool useMSAA;

	DXTKFont* writer;
	ComPtr<ID3D11DepthStencilState> writerDSS;
	ComPtr<ID3D11RasterizerState> writerRS;

	Camera* mainCamera;

	std::unique_ptr<Axes> dAxes;
	std::unique_ptr<LineObject> dLine;

	//Effect bool
	bool flashOnOff;
	int flashCount;
	bool pixelOnOff;
	bool whiteOutOnOff;
	int whiteOutCount;

	float timer;

	std::unique_ptr<ComResourceManager<ID3D11ShaderResourceView>> textureResourceManager;
	std::unique_ptr<ComResourceManager<ID3D11VertexShader>> VSResourceManager;
	std::unique_ptr<ComResourceManager<ID3D11PixelShader>> PSResourceManager;
	std::unique_ptr<ComResourceManager<ID3D11InputLayout>> IAResourceManager;

	std::map<std::string, ComPtr<ID3D11ShaderResourceView>> screenImageMap;
	std::queue<std::pair<ComPtr<ID3D11ShaderResourceView>, RECT>> queueImage;

public:
	//Deferred
	DeferredRenderer* deferredRenderer;
	ComPtr<ID3D11ShaderResourceView> deferredSRV;		 //DeferredTexture
	ComPtr<ID3D11ShaderResourceView> deferredSRVDepth;	 //Depth
	ComPtr<ID3D11ShaderResourceView> deferredSRVNormal; //Normal
	ComPtr<ID3D11ShaderResourceView> deferredSRVTexture;//texture
	ID3D11Texture2D* dFinalTexture; //texture

	//UI
	UIRenderer* uiRenderer;

	//Forward
	ForwardRenderer* forwardRenderer;

	//PostProcessing
	PostRenderer* postRenderer;

	//Background
	BackgroundRenderer* backgroundRenderer;

	//Sampler
	ComPtr<ID3D11SamplerState> defaultSampler;

	// 이거 하나를 돌려쓰는거 같아서 하나로 통합함.
	ComPtr<ID3D11RasterizerState> rasterizerState;

	//최종 그림을 위한 ShaderResource
	ComPtr <ID3D11RenderTargetView> finalRenderTartView;
	ComPtr <ID3D11ShaderResourceView> finalSRV;
	ComPtr <ID3D11Texture2D> finalTexture2D;
	PipeLine finalPipeline;

	ID3D11BlendState* defaultBlend;

	VertexD::Data FVdata[4];
	UINT FIdata[6];

	//Delta Time
	float dt;

	// test
	bool screenshotTest = false;
	std::string testScreen = "";

public:
	GraphicsEngine();
	~GraphicsEngine();

	virtual void Initialize(HWND _hwnd) override;

	virtual void Render(PipeLine& _pipline, int _indexSize) override;
	virtual void endDraw() override;
	virtual void begineDraw() override;

	virtual void BindPipeline(PipeLine& _pipline) override;

	virtual void WriteText(int _x, int _y, float _rgba[4], TCHAR* _text);

	virtual void CreateCamera(ICamera** _camera, float _w, float _h) override;
	virtual void SetMainCamera(ICamera* _camera) override;
	virtual bool IsMainCamera(ICamera* _camera) const override;

	Camera* GetCamera();

	virtual void DrawDefaultAxes() override;
	virtual void DrawDefaultLine() override;

	void RenderClearView();
	void RenderTestThing(PipeLine& _pipline);

	void ClearRenderTargetView();
	void ClearDepthStencilView();
	void ClearFinalRenderTargetView();

	void CreateInputLayer(ComPtr<ID3D11InputLayout>& _inputLayout, D3D11_INPUT_ELEMENT_DESC* _defaultInputLayerDECS, UINT _numberOfElement, ComPtr<ID3D11VertexShader>& _vs, std::wstring _path);
	void CreatePixelShader(ComPtr<ID3D11PixelShader>& _ps, std::wstring _path);

	template<typename V> void CreateVertexBuffer(V* _verteies, UINT _size, ComPtr<ID3D11Buffer>& _vertexbuffer, std::string _name);

	void CreateIndexBuffer(UINT* _indices, UINT _size, ComPtr<ID3D11Buffer>& _indexbuffer);
	void CreateRasterizerState(ComPtr<ID3D11RasterizerState>& _rasterizerState);

	void CreateMatrixBuffer();
	void BindMatrixParameter(DirectX::XMMATRIX _w);

	void CreateBoneBuffer();
	void BindBonesData(std::vector<RenderObject*>& bones, DirectX::XMMATRIX _worldTM);
	void BindBonesData(std::vector<std::shared_ptr<ABone>>& bones, DirectX::XMMATRIX _worldTM);

	void CreateLightingBffer();
	void BindLightingParameter(DirectionalLight _directionLight[], UINT _lightCount);

	void SetTexture(UINT _start, UINT _viewNumbers, ComPtr<ID3D11ShaderResourceView>& _resourceView);

	void BindView();

	void BindFinalView();

	void LoadTexture(std::wstring _path, ComPtr<ID3D11ShaderResourceView>& _resourceView);
	void LoadTexture(std::vector<std::wstring> _path, ComPtr<ID3D11ShaderResourceView>& _resourceView);

	void UIRender(PipeLine& _pipline, int _indexSize);
	void BindCameraAtPS();

	void CreateFinalView();

	void BindSamplerState();

	ComPtr<ID3D11Device>& GetDevice() { return this->d3d11Device; };
	ComPtr<ID3D11DeviceContext>& GetDeviceContext() { return this->d3d11DeviceContext; };

	int GetWindowWidth() { return this->windowWidth; }
	int GetWindowHeight() { return this->windowHeight; };

	std::vector<float> PickingWorldPoint(float _x, float _y) override;

	virtual void Resize() override;

	virtual void GetDT(float _dt) override;

	virtual void ChaptuerScreen(std::string _name) override;
	virtual void ShowChaptueredImage(std::string _name, RECT _rect) override;
	/// Effect OnOff functions
public:
	virtual void SetFlashEffect(float deltaTime, bool _isOnOff) override;
	void PlayFlashEffect();
	virtual void SetPixelateEffect() override;
	virtual void SetWhiteOutEffect(float deltaTime, bool _isOnOff) override;
	void PlayWhiteOutEffect();
	

private:
	void CreateD3D11DeviceContext();
	void CreateChainValue();
	void CreateRenderTargetView();
	void CreateDepthStencilBufferAndView();
	void CreateViewport();
	void CreateRasterizerState();

	void CreateWriter();

	void CreateTextureDataFromDDS(std::wstring _path, ScratchImage* _image);
	void CreateTextureDataFromTGA(std::wstring _path, ScratchImage* _image);
	void CreateTextureDataFromWIC(std::wstring _path, ScratchImage* _image);

	void CreateCameraBuffer();
	void CreateFinalPipeline();

	std::vector<byte> Read(std::string File);
};

template<typename V> void GraphicsEngine::CreateVertexBuffer(V* _verteies, UINT _size, ComPtr<ID3D11Buffer>& _vertexbuffer, std::string _name)
{
	HRESULT hr = S_OK;

// 	_vertexbuffer = this->bufferResourceManager->GetResource(_name + "VB");
// 
// 	if (_vertexbuffer != nullptr)
// 	{
// 		return;
// 	}

	D3D11_BUFFER_DESC vb = {};

	vb.Usage = D3D11_USAGE_IMMUTABLE;
	vb.ByteWidth = _size;
	vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb.CPUAccessFlags = 0;
	vb.MiscFlags = 0;
	vb.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = _verteies;

	hr = this->d3d11Device->CreateBuffer(
		&vb,
		&initData,
		_vertexbuffer.GetAddressOf()
	);

	// this->bufferResourceManager->StoreResource(_name + "VB", _vertexbuffer);

	assert(SUCCEEDED(hr) && "cannot create vertex buffer");
}