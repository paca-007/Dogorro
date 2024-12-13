#pragma once
#include "pch.h"
#include "Vertex.h"
#include "color.h"
#include "pipeline.h"

class GraphicsEngine;
class DemoProcess;

class Axes
{
private:
	VertexC::Data vertexes[6] =
	{
		{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), COLORS::Red  },	// x축 (빨강)
		{ DirectX::XMFLOAT3(10.0f, 0.0f, 0.0f), COLORS::Red  },

		{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), COLORS::Green},	// y축 (초록)
		{ DirectX::XMFLOAT3(0.0f, 10.0f, 0.0f), COLORS::Green},

		{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), COLORS::Blue	},	// z축 (파랑)
		{ DirectX::XMFLOAT3(0.0f, 0.0f, 10.0f), COLORS::Blue }
	};

	UINT indexes[6] = {
		// x축
		0, 1,

		// y축
		2, 3,

		// z축
		4, 5,
	};

	PipeLine pipeline;

	GraphicsEngine* graphicsEngine;

	DemoProcess* scene;

	std::wstring path[2]
	{
		L"../Shader/compiled/VertexShader.cso",
		L"../Shader/compiled/PixelShader.cso",
	};

public:
	Axes(GraphicsEngine* _graphicsEngine);
	~Axes();

	void Update(float _dt);
	void Render(GraphicsEngine* ge);
};