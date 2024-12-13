#pragma once
#include "pch.h"

/// <summary>
/// 정점 구조체
/// </summary>
namespace VertexC
{
	struct Data
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	static UINT Size()
	{
		return (UINT)28;
	}

	static D3D11_INPUT_ELEMENT_DESC defaultInputLayerDECS[2] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
}

/// <summary>
/// 디퍼드 렌더링용 정점 구조체
/// </summary>
namespace VertexD
{
	struct Data
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 tex;
	};

	static UINT Size()
	{
		return (UINT)(sizeof(Data));
	}

	static D3D11_INPUT_ELEMENT_DESC defaultInputLayerDECS[2] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
}

namespace VertexT
{
	struct Data
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 weight;
		UINT32 BoneIndices[4];
	};

	static UINT Size()
	{
		return static_cast<UINT>(sizeof(Data));
	}

	static D3D11_INPUT_ELEMENT_DESC defaultInputLayerDECS[5] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"WEIGHT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
}
