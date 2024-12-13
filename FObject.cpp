#include "FObject.h"
#include "FbxData.h"
#include "FbxLoader.h"
#include "GraphicsEngine.h"
#include "IGraphicsEngine.h"
#include "fbxVertex.h"
#include "FbxMeshData.h"
#include "LightHelper.h"
#include "pipeline.h"
#include "DeferredRenderer.h"
#include "FbxMaterialData.h"

const std::wstring FObject::defaultPath[4] =
{
	 L"..\\Model\\",
	 L"..\\Model\\defaultNormalMap.png",
	 L"..\\Model\\",
	 L"..\\Model\\"
};

FObject::FObject()
	: fData(nullptr)
	, path(nullptr)
	, gp(nullptr)
	, texturePath{}
	, deferredRenderer(nullptr)
{
	position = std::vector<float>{ 0.0f, 0.0f, 0.0f };
	rotation = std::vector<float>{ 0.0f, 0.0f, 0.0f };
	scale = std::vector<float>{ 1.0f, 1.0f, 1.0f };
}

FObject::~FObject()
{
	ReleasePipeline(this->fData->mesh);

	delete this->fData;
}

/// <summary>
/// 밖에서 접근하는 렌더 함수
/// </summary>
void FObject::Render()
{
	RenderMesh(this->fData->mesh);
}

/// <summary>
/// 밖에서 접근하는 업데이트 함수
/// </summary>
void FObject::Update()
{
	UpdataMatirx(this->fData->mesh);
}

/// <summary>
/// 초기화
/// </summary>
/// <param name="_gp">그래픽 엔진</param>
/// <param name="_sPath">셰이더 경로</param>
/// <param name="_texturePath">텍스쳐 경로</param>
/// <param name="_nowData">데이터</param>
void FObject::Initalize(IGraphicsEngine* _gp, std::wstring _sPath[], std::wstring _texturePath, FbxData* _nowData)
{
	this->gp = dynamic_cast<GraphicsEngine*>(_gp);
	this->path = _sPath;
	this->texturePath = _texturePath;
	CreatePipeline(this->fData->mesh);

	this->deferredRenderer = this->gp->deferredRenderer;
	// this->deferredRenderer->Initailze(this->gp);
}

/// <summary>
/// 위치 조정
/// </summary>
/// <param name="_position">위치</param>
void FObject::SetPosition(std::vector<float> _position)
{
	if (_position.size() == 3)
	{
		position = _position;
	}
}

void FObject::SetPosition(float _x, float _y, float _z)
{
	position[0] = _x;
	position[1] = _y;
	position[2] = _z;
}

/// <summary>
/// 위치 누산
/// </summary>
/// <param name="_position">위치</param>
void FObject::AddPosition(std::vector<float> _position)
{
	if (_position.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			position[i] += _position[i];
		}
	}
}

void FObject::AddPosition(float _x, float _y, float _z)
{
	position[0] += _x;
	position[1] += _y;
	position[2] += _z;
}

void FObject::SetRotation(std::vector<float> _rotation)
{
	if (_rotation.size() == 3)
	{
		rotation = _rotation;
	}
}

void FObject::SetRotation(float _x, float _y, float _z)
{
	rotation[0] = _x;
	rotation[1] = _y;
	rotation[2] = _z;
}

void FObject::AddRotation(std::vector<float> _rotation)
{
	if (_rotation.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			rotation[i] += _rotation[i];
		}
	}
}

void FObject::AddRotation(float _x, float _y, float _z)
{
	rotation[0] += _x;
	rotation[1] += _y;
	rotation[2] += _z;
}

void FObject::SetScale(std::vector<float> _scale)
{
	if (_scale.size() == 3)
	{
		scale = _scale;
	}
}

void FObject::SetScale(float _x, float _y, float _z)
{
	scale[0] = _x;
	scale[1] = _y;
	scale[2] = _z;
}

void FObject::AddScale(std::vector<float> _scale)
{
	if (_scale.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			scale[i] += _scale[i];
		}
	}
}

void FObject::AddScale(float _x, float _y, float _z)
{
	scale[0] += _x;
	scale[1] += _y;
	scale[2] += _z;
}

void FObject::CreatePipeline(FbxMeshData* _nowMesh)
{
	if (!_nowMesh->vertexData.size() == 0)
	{
		_nowMesh->pipeline = new PipeLine();
		this->gp->CreateInputLayer(_nowMesh->pipeline->inputLayout, VertexF::defaultInputLayerDECS, 6, _nowMesh->pipeline->vertexShader, this->path[0]);
		this->gp->CreatePixelShader(_nowMesh->pipeline->pixelShader, this->path[1]);
		this->gp->CreateRasterizerState(_nowMesh->pipeline->rasterizerState);
		_nowMesh->pipeline->primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		size_t matCount = _nowMesh->matrialList.size();
		for (size_t i = 0; i < matCount; i++)
		{
			std::vector<std::wstring> wTextruePath;
			int factorCount = this->fData->materialMap[_nowMesh->matrialList[i]]->materialTypeCount;
			for (int j = 0; j < factorCount; j++)
			{
				std::wstring sTexturePath;
				sTexturePath.assign(this->fData->materialMap[_nowMesh->matrialList[i]]->filePath[j].begin(),
					this->fData->materialMap[_nowMesh->matrialList[i]]->filePath[j].end());
				if (sTexturePath != L"")
				{
					wTextruePath.push_back(this->texturePath + sTexturePath);
				}
				else
				{
					wTextruePath.push_back(defaultPath[j]);
				}
			}
			_nowMesh->textuers.push_back(ComPtr<ID3D11ShaderResourceView>());
			this->gp->LoadTexture(wTextruePath, _nowMesh->textuers[i]);
		}

		_nowMesh->vertexBufferData = new VertexF::Data[_nowMesh->vertexData.size()];
		memcpy(_nowMesh->vertexBufferData, &_nowMesh->vertexData[0], (sizeof(VertexF::Data) * _nowMesh->vertexData.size()));
		this->gp->CreateVertexBuffer(_nowMesh->vertexBufferData, static_cast<UINT>(_nowMesh->vertexData.size()) * VertexF::Size(), _nowMesh->pipeline->vertexBuffer, _nowMesh->name);

		size_t indexCount = _nowMesh->indexData.size();
		_nowMesh->indexBufferData.resize(indexCount);
		_nowMesh->indexBuffer.resize(indexCount);
		for (size_t i = 0; i < _nowMesh->indexData.size(); i++)
		{
			_nowMesh->indexBufferData[i] = new UINT[_nowMesh->indexData[i].size()];
			memcpy(_nowMesh->indexBufferData[i], &_nowMesh->indexData[i][0], (sizeof(UINT) * _nowMesh->indexData[i].size()));
			this->gp->CreateIndexBuffer(_nowMesh->indexBufferData[i], static_cast<UINT>(_nowMesh->indexData[i].size()), _nowMesh->indexBuffer[i]);
		}

	}

	for (auto& c : _nowMesh->children)
	{
		CreatePipeline(c);
	}
}

void FObject::ReleasePipeline(FbxMeshData* _nowMesh)
{
	if (_nowMesh->pipeline)
	{
		delete _nowMesh->pipeline;
	}
	for (auto& c : _nowMesh->children)
	{
		ReleasePipeline(c);
	}
}

void FObject::RenderMesh(FbxMeshData* _nowData)
{
	gp->BindMatrixParameter(_nowData->globalTM);
	for (size_t i = 0; i < _nowData->indexBuffer.size(); i++)
	{
		_nowData->pipeline->IndexBuffer = _nowData->indexBuffer[i];
		gp->BindPipeline(*_nowData->pipeline);
		gp->SetTexture(0, 4, _nowData->textuers[i]);
		gp->Render(*_nowData->pipeline, static_cast<int>(_nowData->indexData[i].size()));
	}

	for (auto& child : _nowData->children)
	{
		RenderMesh(child);
	}
}

void FObject::UpdataMatirx(FbxMeshData* _nowData)
{
	if (_nowData->parent == nullptr)
	{
		DirectX::XMMATRIX newTM = DirectX::XMMatrixIdentity();

		newTM *= DirectX::XMMatrixScaling(this->scale[0], this->scale[1], this->scale[2]);

		newTM *= DirectX::XMMatrixRotationX(this->rotation[0]);
		newTM *= DirectX::XMMatrixRotationY(this->rotation[1]);
		newTM *= DirectX::XMMatrixRotationZ(this->rotation[2]);

		newTM *= DirectX::XMMatrixTranslation(this->position[0], this->position[1], this->position[2]);
		_nowData->localTM = newTM;
		_nowData->globalTM = newTM;
	}
	else
	{
		_nowData->globalTM = _nowData->parent->globalTM * _nowData->localTM;
	}

	for (auto& child : _nowData->children)
	{
		UpdataMatirx(child);
	}
}
