#pragma once
#include "pch.h"

struct VertexDefaultData
{
	Vector3 position = {};
};

struct VertexTextureData
{
	Vector3 position = {};
	Vector2 uv = {};
};

struct VertexColorData
{
	Vector3 position = {};
	Color color = {};
};

struct VertexTextureNormalData
{
	Vector3 position = {};
	Vector2 uv = {};
	Vector3 normal = {};
};

struct VertexTextureNormalTangentData
{
	Vector3 position = {};
	Vector2 uv = {};
	Vector3 normal = {};
	Vector3 tangent = {};
};

struct VertexTextureNormalTangentBlendData
{
	Vector3 position = {};
	Vector3 normal = {};
	Vector2 uv = {};
	Vector3 bitangent = {};
	Vector3 tangent = {};
	uint32 blendIndices[4] = {0, 0, 0, 0};
	Vector4 blendWeights = {0.0f, 0.0f, 0.0f, 0.0f};

	static const D3D11_INPUT_ELEMENT_DESC inputLayerDECS[7];
};

using ModelVertexType = VertexTextureNormalTangentBlendData;

