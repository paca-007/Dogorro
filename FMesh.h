#pragma once
#include "pch.h"
#include "pipeline.h"
#include "LightHelper.h"


class FbxData;
class GraphicsEngine;
namespace VertexF
{
	struct Data;
};

class FMesh
{
public:
	FbxData* fData;

	Material demoMat;

public:
	FMesh();
	~FMesh();

	void Render(GraphicsEngine* _gp, DirectX::XMMATRIX _viewTM, DirectX::XMMATRIX _projTM);
	void CreatePipeline(GraphicsEngine* gp, std::wstring _sPath[], std::wstring _texturePath, FbxData* _nowData);
};

