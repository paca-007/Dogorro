#pragma once
#include "IRenderer.h"
#include "Vertex.h"
#include "pipeline.h"
#include <vector>
#include <map>
#include <wrl\client.h>

using Microsoft::WRL::ComPtr;

/// <summary>
/// UI렌더러가 될 친구
/// 모든 렌더가 된 이후 화면에 마지막으로 UI를 그려주는 역할을 한다
/// UObject와 함께합니다.
/// </summary>


class GraphicsEngine;
class FbxData;
class FbxMeshData;


struct AnimationData
{
	float width;        // 가로 프레임 전체 넓이
	float aniWidth;      // 각 프레임의 넓이 
	float totalFrames;  // 전체 프레임 수
	float frameIndex;   // 현재 프레임 인덱스
	
};

struct KeyFrame
{
	float TimePos = 0.f;
	Vector3 Translation = { 0.f, 0.f, 0.f };
};

class UIRenderer
	: public IRenderer
{
public:
	UIRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext, ID3D11DepthStencilView* _depthStancilView, const int _windowWidth, const int _windowHeight);
	~UIRenderer();

	virtual void Initailze(GraphicsEngine* _gp) override;
	virtual void Finalize() override;


	virtual void BeginRender() override;
	virtual void EndRender() override;


	void GetMesh(std::string _name);

	void CreateMesh(std::string _name, float _width, float _height, float _startPointX, float _startPointY, std::wstring _texturePath);
	void CreatePipeline(std::string _name, std::wstring* _path);

	void CreateAnimationMesh(std::string _name, float _width, float _height, float _startPointX, float _startPointY, float _aniWidth, std::wstring _texturePath, int _aniCount);
	void CreateAnimationPipeline(std::string _name);
	
	void UpdateAnimationBuffer(std::string _name);

	void SetPosition(float _x, float _y, std::string _name);

	void RenderChapturedImage(std::pair<ComPtr<ID3D11ShaderResourceView>, RECT>& _renderTings);

private:
	void CreateMoveBuffer();
	void BindMoveBuffer(float _x, float _y);

private:
	struct MovementBuffer
	{
		float dx;
		float dy;
		Vector2	padding;
	};

	GraphicsEngine* gp = nullptr;

	// D3D 디바이스와 디바이스 컨텍스트
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DeviceContext;
	ID3D11DepthStencilView* depthStancilView;

	ComPtr<ID3D11Buffer> movementBuffer;

	//vertex를 object에서 만들수가 없어요..
	VertexD::Data UVdata[4];
	UINT UIdata[6];
	ComPtr<ID3D11ShaderResourceView> uPipeTexture;
	PipeLine* UPipeline;
	float fps;
	
	//각 객체들을 저장해둘 필요가 생김! 이것도 vector로 저장해둘까요 아님 맵?
	//name 생긴김에 map으로 저장굿
	std::map<std::string, PipeLine*> uiPipeline;
	std::map<std::string, ComPtr<ID3D11ShaderResourceView>> uiTexture;
	std::map<std::string, std::pair<float, float>> uiPosition;
	std::map<std::string, AnimationData> uiAnimation;
	std::map<std::string, ID3D11Buffer*> uiAnimationBuffer;

	const int windowWidth;
	const int windowHeight;

	//Animation을 위한 데이터들
	ID3D11Buffer* animationBuffer;
	AnimationData animationData;

	//안에서 생성되는 것들
	// 
	//UI로 렌더링 될 대상을 불러와 저장해둔다.
	std::vector<PipeLine*> uData;
	std::vector<ComPtr<ID3D11ShaderResourceView>> uTexture;
	std::vector<std::string> uName;
};

