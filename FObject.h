#pragma once
#include <string>
#include <vector>

class FbxData;
class GraphicsEngine;
class IGraphicsEngine;
class FbxMeshData;
class DeferredRenderer;

enum class TEXTURE_USAGE
{
	BASE_COLOR = 0,
	NORMAL,
	METALIC,
	ROUGHNESS,
	END,
};

class FObject
{
public:
	FbxData* fData;

private:
	GraphicsEngine* gp;
	std::wstring texturePath;
	std::wstring* path;
	DeferredRenderer* deferredRenderer;

	std::vector<float> position;
	std::vector<float> rotation;
	std::vector<float> scale;

	static const std::wstring defaultPath[4];

public:
	FObject();
	~FObject();

	void Render();
	void Update();

	void Initalize(IGraphicsEngine* gp, std::wstring _sPath[], std::wstring _texturePath, FbxData* _nowData);

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

// 	void RunAnimation(std::string _name, float _dt);
// 	void ResetAnimation(std::string _name);

private:
	void CreatePipeline(FbxMeshData* _nowMesh);
	void ReleasePipeline(FbxMeshData* _nowMesh);

	void RenderMesh(FbxMeshData* _nowData);
	void UpdataMatirx(FbxMeshData* _nowData);
};