#include "FMesh.h"
#include "FbxData.h"
#include "FbxLoader.h"
#include "GraphicsEngine.h"
#include "fbxVertex.h"
FMesh::FMesh()
	: fData(nullptr)
	, demoMat{}
{
	this->demoMat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	this->demoMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	this->demoMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

FMesh::~FMesh()
{

}

void FMesh::Render(GraphicsEngine* _gp, DirectX::XMMATRIX _viewTM, DirectX::XMMATRIX _projTM)
{

	FbxData* _nowData = this->fData;
	for (auto& c : _nowData->children)
	{
		_gp->BindMatrixParameter(
			c->globalTM,
			_viewTM,
			_projTM,
			this->demoMat
		);
		if (c->vertexData.size() != 0)
		{
			_gp->BindPipeline(*c->pipeline);
			_gp->SetTexture(0, 1, c->pipeline->textureView);
			_gp->RenderByIndex(*c->pipeline, static_cast<int>(c->indexData.size()));
		}
	}

}

void FMesh::CreatePipeline(GraphicsEngine* _gp, std::wstring _sPath[], std::wstring _texturePath, FbxData* _nowData)
{
	if (!_nowData->vertexData.size() == 0)
	{
		_nowData->pipeline = new PipeLine();
		_gp->CreateInputLayer(*_nowData->pipeline, VertexF::defaultInputLayerDECS, _sPath, 6);
		_gp->CreateRasterizerState(&_nowData->pipeline->rasterizerState);
		_nowData->pipeline->primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		_nowData->pipeline->vertexStructSize = VertexF::Size();


		std::vector<std::wstring> path(_nowData->textureFileName.size());
		for (int i = 0; i < _nowData->textureFileName.size(); i++)
		{
			path[i].assign(_nowData->textureFileName[i].begin(), _nowData->textureFileName[i].end());
			path[i] = _texturePath + path[i];
		}

		if (!path.empty())
		{
			_nowData->pipeline->textureView = new ID3D11ShaderResourceView * [path.size()];
			_gp->CreateTextureDataFromTGA(path, _nowData->pipeline->textureView);
		}

		_nowData->vertexBufferData = new VertexF::Data[_nowData->vertexData.size()];
		memcpy(_nowData->vertexBufferData, &_nowData->vertexData[0], (sizeof(VertexF::Data) * _nowData->vertexData.size()));
		_gp->CreateVertexBuffer(_nowData->vertexBufferData, static_cast<UINT>(_nowData->vertexData.size()) * VertexF::Size(), &_nowData->pipeline->vertexBuffer);

		_nowData->indexBufferData = new UINT[_nowData->indexData.size()];
		memcpy(_nowData->indexBufferData, &_nowData->indexData[0], (sizeof(UINT) * _nowData->indexData.size()));

		_gp->CreateIndexBuffer(_nowData->indexBufferData, static_cast<UINT>(_nowData->indexData.size()), &_nowData->pipeline->IndexBuffer);
	}

	for (auto& c : _nowData->children)
	{
		CreatePipeline(_gp, _sPath, _texturePath, c);
	}
}
