#include "Axes.h"
#include "GraphicsEngine.h"

Axes::Axes(GraphicsEngine* _graphicsEngine)
	: graphicsEngine(_graphicsEngine)
{

	this->graphicsEngine->CreateInputLayer(this->pipeline.inputLayout, VertexC::defaultInputLayerDECS, 2, this->pipeline.vertexShader, path[0]);
	this->graphicsEngine->CreatePixelShader(this->pipeline.pixelShader, path[1]);
	this->graphicsEngine->CreateVertexBuffer(this->vertexes, sizeof(this->vertexes), this->pipeline.vertexBuffer, "AxesVB");
	this->graphicsEngine->CreateIndexBuffer(indexes, sizeof(indexes), this->pipeline.IndexBuffer);
	this->graphicsEngine->CreateRasterizerState(this->pipeline.rasterizerState);
	this->pipeline.primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	this->pipeline.vertexStructSize = VertexC::Size();
}

Axes::~Axes()
{
}

void Axes::Update(float _dt)
{

}

void Axes::Render(GraphicsEngine* ge)
{
	this->graphicsEngine->BindPipeline(pipeline);
	this->graphicsEngine->BindMatrixParameter(
		DirectX::XMMatrixIdentity()
	);
	this->graphicsEngine->Render(pipeline, sizeof(this->indexes) / sizeof(this->indexes[0]));
}
