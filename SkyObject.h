#pragma once
#include <string>
#include <vector>

class FbxData;
class GraphicsEngine;
class IGraphicsEngine;
class FbxMeshData;

class SkyObject
{
public:
	FbxData* skyData;

private:
	GraphicsEngine* gp;
	std::wstring texturePath;
	std::wstring* path;

public:
	SkyObject();
	~SkyObject();

	void Render();
	void Update();

	void Initalize(IGraphicsEngine* gp, std::wstring _sPath[], std::wstring _texturePath, FbxData* _nowData);

};

