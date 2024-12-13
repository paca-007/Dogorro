#pragma once
#include "pch.h"
#include "VertexType.h"
#include "pipeline.h"

class AMaterial;
class ABone;
class GraphicsEngine;
struct PipeLine;

class AMesh
{
public:
	std::string name;

	std::vector<ModelVertexType> vertexData;
	std::vector<uint32> indexData;

	/// pipline
	PipeLine pip;
// 	ComPtr<ID3D11Buffer> vBuffer;
// 	ComPtr<ID3D11Buffer> iBuffer;
// 	ComPtr<ID3D11InputLayout> inputLayout;
// 	ComPtr<ID3D11VertexShader> vs;
// 	ComPtr<ID3D11PixelShader> ps;

	std::string materialName;
	std::weak_ptr<AMaterial> material;

	uint32 boneIndex;
	std::weak_ptr<ABone> bone;

	Matrix ori;

	Vector3 oriPos;
	Quaternion oriRot;
	Vector3 oriSca;

	Vector3 maxAABB;
	Vector3 minAABB;

public:
	void SetVS(GraphicsEngine* _gp, std::string _path);
	void SetPS(GraphicsEngine* _gp, std::string _path);
};

