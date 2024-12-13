#pragma once
#include <string>
#include <vector>

class FbxData;
class GraphicsEngine;
class IGraphicsEngine;
class FbxMeshData;
class BackgroundRenderer;
class FbxMeshData;

/// <summary>
/// outline을 위한 배경 따로 그리기 renderer
/// 이론상 배경은 어떤 object들이 그려지든 제일 뒤에 위치하게 된다.
/// 그러니 뭘 그리게 되든 일단 젤 뒤에 그려버리겟다는게 시작점.
/// 교수님 말씀대로 pass는 자꾸만 늘어간다~
/// </summary>

class BObject
{
public:
	FbxData* bData;

private:
	GraphicsEngine* gp;
	std::wstring texturePath;
	std::wstring* path;
	BackgroundRenderer* backgroundRenderer;

	std::vector<float> position;
	std::vector<float> rotation;
	std::vector<float> scale;

public:
	BObject();
	~BObject();

	void Render();
	void Update();

	void Initalize(IGraphicsEngine* _gp, std::wstring _sPath[], std::wstring _texturePath, FbxData* _nowData);

	void SetPosition(std::vector<float> _position);
	void SetPosition(float _x, float _y, float _z);

	void AddPosition(std::vector<float> _position);
	void AddPosition(float _x, float _y, float _z);

	void SetRotation(std::vector<float> _rotation);
	void SetRotation(float _x, float _y, float _z);

	void AddRotation(std::vector<float> _rotation);
	void AddRotation(float _x, float _y, float _z);

	void SetScale(std::vector<float> _scale);
	void SetScale(float _x, float _y, float _z);

	void AddScale(std::vector<float> _scale);
	void AddScale(float _x, float _y, float _z);

private:
	void CreatePipeline(FbxMeshData* _nowMesh);
	void ReleasePipeline(FbxMeshData* _nowMesh);

	void RenderMesh(FbxMeshData* _nowData);
	void UpdateMatrix(FbxMeshData* _nowData);
};

