#pragma once
#include "IRenderer.h"
#include "Vertex.h"
#include "pipeline.h"
#include <vector>
#include <map>
#include <wrl\client.h>

using Microsoft::WRL::ComPtr;

/// <summary>
/// UI�������� �� ģ��
/// ��� ������ �� ���� ȭ�鿡 ���������� UI�� �׷��ִ� ������ �Ѵ�
/// UObject�� �Բ��մϴ�.
/// </summary>


class GraphicsEngine;
class FbxData;
class FbxMeshData;


struct AnimationData
{
	float width;        // ���� ������ ��ü ����
	float aniWidth;      // �� �������� ���� 
	float totalFrames;  // ��ü ������ ��
	float frameIndex;   // ���� ������ �ε���
	
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

	// D3D ����̽��� ����̽� ���ؽ�Ʈ
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DeviceContext;
	ID3D11DepthStencilView* depthStancilView;

	ComPtr<ID3D11Buffer> movementBuffer;

	//vertex�� object���� ������� �����..
	VertexD::Data UVdata[4];
	UINT UIdata[6];
	ComPtr<ID3D11ShaderResourceView> uPipeTexture;
	PipeLine* UPipeline;
	float fps;
	
	//�� ��ü���� �����ص� �ʿ䰡 ����! �̰͵� vector�� �����صѱ�� �ƴ� ��?
	//name ����迡 map���� �����
	std::map<std::string, PipeLine*> uiPipeline;
	std::map<std::string, ComPtr<ID3D11ShaderResourceView>> uiTexture;
	std::map<std::string, std::pair<float, float>> uiPosition;
	std::map<std::string, AnimationData> uiAnimation;
	std::map<std::string, ID3D11Buffer*> uiAnimationBuffer;

	const int windowWidth;
	const int windowHeight;

	//Animation�� ���� �����͵�
	ID3D11Buffer* animationBuffer;
	AnimationData animationData;

	//�ȿ��� �����Ǵ� �͵�
	// 
	//UI�� ������ �� ����� �ҷ��� �����صд�.
	std::vector<PipeLine*> uData;
	std::vector<ComPtr<ID3D11ShaderResourceView>> uTexture;
	std::vector<std::string> uName;
};

