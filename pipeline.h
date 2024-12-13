#pragma once
#include "pch.h"

struct PipeLine
{
	ComPtr<ID3D11InputLayout> inputLayout = nullptr;
	ComPtr<ID3D11VertexShader> vertexShader = nullptr;
	ComPtr<ID3D11PixelShader> pixelShader = nullptr;
	ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
	ComPtr<ID3D11Buffer> IndexBuffer = nullptr;
	ComPtr<ID3D11RasterizerState> rasterizerState = nullptr;
	ComPtr<ID3D11ShaderResourceView> textureView = nullptr;

	UINT vertexStructSize = 0;
	D3D_PRIMITIVE_TOPOLOGY primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

};
