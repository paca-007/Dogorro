#pragma once
#include "pch.h"
#include "Vertex.h"
#include "pipeline.h"
#include "LightHelper.h"

class GraphicsEngine;
class RenderObject;

class Mesh
{
public:
	std::vector<VertexT::Data> vertexList;
	std::vector<UINT> indexList;

	std::vector<DirectX::XMFLOAT3> position;

	std::vector<int> normalIndex;
	std::vector<DirectX::XMFLOAT3> normal;

	std::vector<int> textureIndex;
	std::vector<DirectX::XMFLOAT2> texture;

	std::vector<std::vector<float>> weight;
	std::vector<std::vector<int>> boneIndex;

	std::vector<std::string> boneNames;
	std::vector<RenderObject*> bones;


	PipeLine pipeline;

	VertexT::Data* vertexes;
	UINT* indexes;

	bool isLocal;


	Material demoMat;

public:
	Mesh();
	~Mesh();

	void Render(GraphicsEngine* gp, const DirectX::XMMATRIX& _worldTM);
	void CreatePipeline(GraphicsEngine* gp, std::wstring _sPath[], std::wstring _texturePath);
	void SetVertexesData();
};

