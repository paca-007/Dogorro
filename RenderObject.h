#pragma once
#include "pch.h"
#include "LightHelper.h"
class Mesh;
class GraphicsEngine;

enum class RENDER_OBJECT_TYPE
{
	GEOMOBJCT,
	HELPEROBJECT,
	SHAPEOBJECT,
	BONE,
};

class RenderObject
{
public:
	std::string name;

	std::vector<Mesh*> meshes;

	RenderObject* parent;
	std::vector<RenderObject*> children;

	bool isAnimation;
	int nowTick;
	float oneTick;
	float accTick;
	int maxTick;
	std::vector<std::pair<int, DirectX::XMFLOAT3>> animationPositionTrack;
	int positionTrackIndex;
	std::vector<std::pair<int, DirectX::XMFLOAT4>> animationRotateTrack;
	int rotateTrackIndex;

	DirectX::XMVECTOR localScale;
	DirectX::XMVECTOR localRotate;
	DirectX::XMVECTOR localPosition;

	DirectX::XMVECTOR nodeScale;
	DirectX::XMVECTOR nodeRotate;
	DirectX::XMVECTOR nodePosition;

	DirectX::XMVECTOR fileScale;
	DirectX::XMVECTOR fileRotate;
	DirectX::XMVECTOR filePosition;

	RENDER_OBJECT_TYPE type;

public:
	bool isHelper;
	DirectX::XMMATRIX nodeTM;
	DirectX::XMMATRIX originalNodeTM;
	DirectX::XMMATRIX localTM;
	DirectX::XMMATRIX animationTM;

	bool isNegative;
	bool hasNegativeScale;

	std::wstring path[2]
	{
		L"../Shader/VertexShader2.hlsl",
		L"../Shader/PixelShader2.hlsl",
	};

	Material demoMat;

public:
	RenderObject();

	void AddMesh(Mesh* _mesh);
	void AddChild(RenderObject* _child);
	void SetParent(RenderObject* _parent);

	std::string GetName() const { return name; }
	void SetName(std::string val) { name = val; }

	void Render(GraphicsEngine* _graphicsEngine);

	void Initalize(GraphicsEngine* _graphicsEngine, std::wstring _path = L" ");

	void Localize(GraphicsEngine* _graphicsEngine, std::wstring _path = L" ");

	void Update(float _dt);

	void Translate(float _x, float _y, float _z);
	void RoateBaseAxis(float _x, float _y, float _z);
	void Scale(float _x, float _y, float _z);

	void UpdateAnimation(float _dt);
};

