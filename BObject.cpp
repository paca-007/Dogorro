#include "BObject.h"
#include "FbxData.h"
#include "FbxLoader.h"
#include "FbxMaterialData.h"
#include "GraphicsEngine.h"
#include "IGraphicsEngine.h"
#include "fbxVertex.h"
#include "FbxMeshData.h"
#include "pipeline.h"
#include "BackgroundRenderer.h"

BObject::BObject()
	: bData(nullptr)
	, texturePath{}
	, path(nullptr)
	, gp(nullptr)
	, backgroundRenderer(nullptr)
{
	position = std::vector<float>{ 0.0f, 0.0f, 0.0f };
	rotation = std::vector<float>{ 0.0f, 0.0f, 0.0f };
	scale = std::vector<float>{ 1.0f, 1.0f, 1.0f };
}

BObject::~BObject()
{
	ReleasePipeline(this->bData->mesh);
	delete this->bData;
}

/// <summary>
/// �ۿ��� �����ϴ� ���� �Լ�
/// </summary>
void BObject::Render()
{
	//���⼭ �׸��� ���� renderer���� �����͸� �Ѱ��ִ°�?
	backgroundRenderer->GetData(this->bData->mesh);

	//RenderMesh(this->tpData->mesh);
}

/// <summary>
/// �ۿ��� �����ϴ� ������Ʈ �Լ�
/// </summary>
void BObject::Update()
{
	UpdateMatrix(this->bData->mesh);
}

void BObject::Initalize(IGraphicsEngine* _gp, std::wstring _sPath[], std::wstring _texturePath, FbxData* _nowData)
{
	//�׷��Ƚ����� �޾ƿ��� �� ���̴� ��� ����
	this->gp = dynamic_cast<GraphicsEngine*>(_gp);
	this->path = _sPath;
	this->texturePath = _texturePath;

	//������ ��������
	this->backgroundRenderer = this->gp->backgroundRenderer;

	//�� ������������ �� ����� �׸����� �ϴ�
	CreatePipeline(this->bData->mesh);
}

/// <summary>
/// ��ġ ����
/// </summary>
/// <param name="_position">��ġ</param>
void BObject::SetPosition(std::vector<float> _position)
{
	if (_position.size() == 3)
	{
		position = _position;
	}
}

void BObject::SetPosition(float _x, float _y, float _z)
{
	position[0] = _x;
	position[1] = _y;
	position[2] = _z;
}

/// <summary>
/// ��ġ ����
/// </summary>
/// <param name="_position">��ġ</param>
void BObject::AddPosition(std::vector<float> _position)
{
	if (_position.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			position[i] += _position[i];
		}
	}
}

void BObject::AddPosition(float _x, float _y, float _z)
{
	position[0] += _x;
	position[1] += _y;
	position[2] += _z;
}

void BObject::SetRotation(std::vector<float> _rotation)
{
	if (_rotation.size() == 3)
	{
		rotation = _rotation;
	}
}

void BObject::SetRotation(float _x, float _y, float _z)
{
	rotation[0] = _x;
	rotation[1] = _y;
	rotation[2] = _z;
}

void BObject::AddRotation(std::vector<float> _rotation)
{
	if (_rotation.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			rotation[i] += _rotation[i];
		}
	}
}

void BObject::AddRotation(float _x, float _y, float _z)
{
	rotation[0] += _x;
	rotation[1] += _y;
	rotation[2] += _z;
}

void BObject::SetScale(std::vector<float> _scale)
{
	if (_scale.size() == 3)
	{
		scale = _scale;
	}
}

void BObject::SetScale(float _x, float _y, float _z)
{
	scale[0] = _x;
	scale[1] = _y;
	scale[2] = _z;
}

void BObject::AddScale(std::vector<float> _scale)
{
	if (_scale.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			scale[i] += _scale[i];
		}
	}
}

void BObject::AddScale(float _x, float _y, float _z)
{
	scale[0] += _x;
	scale[1] += _y;
	scale[2] += _z;
}

void BObject::CreatePipeline(FbxMeshData* _nowMesh)
{
	if (!_nowMesh->vertexData.size() == 0)
	{
		_nowMesh->pipeline = new PipeLine();
		this->gp->CreateInputLayer(_nowMesh->pipeline->inputLayout, VertexF::forwardInputLayderDECS, 2, _nowMesh->pipeline->vertexShader, this->path[0]);
		this->gp->CreatePixelShader(_nowMesh->pipeline->pixelShader, this->path[1]);
		this->gp->CreateRasterizerState(_nowMesh->pipeline->rasterizerState);
		_nowMesh->pipeline->primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		_nowMesh->pipeline->vertexStructSize = VertexF::Size();

		size_t matCount = _nowMesh->matrialList.size();
		for (size_t i = 0; i < matCount; i++)
		{
			std::vector<std::wstring> wTextruePath;
			int factorCount = this->bData->materialMap[_nowMesh->matrialList[i]]->materialTypeCount;
			for (int j = 0; j < factorCount; j++)
			{
				std::wstring sTexturePath;
				sTexturePath.assign(this->bData->materialMap[_nowMesh->matrialList[i]]->filePath[0].begin(),
					this->bData->materialMap[_nowMesh->matrialList[i]]->filePath[0].end());
				wTextruePath.push_back(this->texturePath + sTexturePath);
			}
			_nowMesh->textuers.push_back(ComPtr<ID3D11ShaderResourceView>());
			this->gp->LoadTexture(wTextruePath, _nowMesh->textuers[i]);
		}

		_nowMesh->vertexBufferData = new VertexF::Data[_nowMesh->vertexData.size()];
		memcpy(_nowMesh->vertexBufferData, &_nowMesh->vertexData[0], (sizeof(VertexF::Data) * _nowMesh->vertexData.size()));
		this->gp->CreateVertexBuffer(_nowMesh->vertexBufferData, static_cast<UINT>(_nowMesh->vertexData.size()) * VertexF::Size(), _nowMesh->pipeline->vertexBuffer, _nowMesh->name + "VB");

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

void BObject::ReleasePipeline(FbxMeshData* _nowMesh)
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

void BObject::RenderMesh(FbxMeshData* _nowData)
{
	//��� ���⼭ ���� �Ͼ�� �ʴ´�
}

void BObject::UpdateMatrix(FbxMeshData* _nowData)
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
		UpdateMatrix(child);
	}
}
