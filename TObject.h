#pragma once
#include <string>
#include <vector>

class FbxData;
class GraphicsEngine;
class IGraphicsEngine;
class FbxMeshData;
class ForwardRenderer;
class FbxMeshData;

class TObject
{

public:
	FbxData* tpData;

private:
	GraphicsEngine* gp;
	std::wstring texturePath;
	std::wstring* path;
	ForwardRenderer* forwardRenderer;

	std::vector<float> position;
	std::vector<float> rotation;
	std::vector<float> scale;

public:
	TObject();
	~TObject();

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

