#include "LineObject.h"
#include "GraphicsEngine.h"

LineObject::LineObject(GraphicsEngine* _graphicsEngine)
	: graphicsEngine(_graphicsEngine)
{
	for (int i = 0; i < 100; i++)
	{
		vertexes[i].position = DirectX::XMFLOAT3((float)(i % 10) - 5.0f, 0.0f, (float)(i / 10) - 5.0f);
		vertexes[i].color = COLORS::White;
	}

	for (int i = 0; i < 10; i++)
	{
		indexes[i * 2] = i;
		indexes[i * 2 + 1] = i + 90;
	}

	for (int i = 0; i < 10; i++)
	{
		indexes[20 + (i * 2)] = i * 10;
		indexes[20 + (i * 2) + 1] = i * 10 + 9;
	}

	this->graphicsEngine->CreateInputLayer(this->pipeline.inputLayout, VertexC::defaultInputLayerDECS, 2, this->pipeline.vertexShader, path[0]);
	this->graphicsEngine->CreatePixelShader(this->pipeline.pixelShader, path[1]);
	this->graphicsEngine->CreateVertexBuffer(this->vertexes, sizeof(this->vertexes), this->pipeline.vertexBuffer, "DefaultLineVB");
	this->graphicsEngine->CreateIndexBuffer(indexes, sizeof(indexes), this->pipeline.IndexBuffer);
	this->graphicsEngine->CreateRasterizerState(this->pipeline.rasterizerState);
	this->pipeline.primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	this->pipeline.vertexStructSize = VertexC::Size();
}

LineObject::~LineObject()
{
}

void LineObject::Update(float _dt)
{

}

void LineObject::Render(GraphicsEngine* ge)
{
	this->graphicsEngine->BindPipeline(pipeline);
	this->graphicsEngine->BindMatrixParameter(
		DirectX::XMMatrixIdentity()
	);
	this->graphicsEngine->Render(pipeline, sizeof(this->indexes) / sizeof(this->indexes[0]));
}
