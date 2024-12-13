#include "TObject.h"
#include "FbxData.h"
#include "FbxLoader.h"
#include "FbxMaterialData.h"
#include "GraphicsEngine.h"
#include "IGraphicsEngine.h"
#include "fbxVertex.h"
#include "FbxMeshData.h"
#include "pipeline.h"
#include "ForwardRenderer.h"

TObject::TObject()
	: tpData(nullptr)
	, texturePath{}
	, path(nullptr)
	, gp(nullptr)
	, forwardRenderer(nullptr)
{
	position = std::vector<float>{ 100.0f, 0.0f, 0.0f };
	rotation = std::vector<float>{ 0.0f, 0.0f, 0.0f };
	scale = std::vector<float>{ 1.0f, 1.0f, 1.0f };
}

TObject::~TObject()
{
	ReleasePipeline(this->tpData->mesh);
	delete this->tpData;
}
/// <summary>
/// 밖에서 접근하는 렌더 함수
/// </summary>
void TObject::Render()
{
	//여기서 그리지 말고 renderer한테 데이터를 넘겨주는건?
	forwardRenderer->GetData(this->tpData->mesh);

	//RenderMesh(this->tpData->mesh);
}

/// <summary>
/// 밖에서 접근하는 업데이트 함수
/// </summary>
void TObject::Update()
{
	UpdateMatrix(this->tpData->mesh);
}

void TObject::Initalize(IGraphicsEngine* _gp, std::wstring _sPath[], std::wstring _texturePath, FbxData* _nowData)
{
	//그래픽스엔진 받아오기 및 쉐이더 경로 지정
	this->gp = dynamic_cast<GraphicsEngine*>(_gp);
	this->path = _sPath;
	this->texturePath = _texturePath;

	//렌더러 가져오기
	this->forwardRenderer = this->gp->forwardRenderer;

	//왜 파이프라인을 안 만들고 그릴려고 하니
	CreatePipeline(this->tpData->mesh);

}

/// <summary>
/// 위치 조정
/// </summary>
/// <param name="_position">위치</param>
void TObject::SetPosition(std::vector<float> _position)
{
	if (_position.size() == 3)
	{
		position = _position;
	}
}

void TObject::SetPosition(float _x, float _y, float _z)
{
	position[0] = _x;
	position[1] = _y;
	position[2] = _z;
}

/// <summary>
/// 위치 누산
/// </summary>
/// <param name="_position">위치</param>
void TObject::AddPosition(std::vector<float> _position)
{
	if (_position.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			position[i] += _position[i];
		}
	}
}

void TObject::AddPosition(float _x, float _y, float _z)
{
	position[0] += _x;
	position[1] += _y;
	position[2] += _z;
}

void TObject::SetRotation(std::vector<float> _rotation)
{
	if (_rotation.size() == 3)
	{
		rotation = _rotation;
	}
}

void TObject::SetRotation(float _x, float _y, float _z)
{
	rotation[0] = _x;
	rotation[1] = _y;
	rotation[2] = _z;
}

void TObject::AddRotation(std::vector<float> _rotation)
{
	if (_rotation.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			rotation[i] += _rotation[i];
		}
	}
}

void TObject::AddRotation(float _x, float _y, float _z)
{
	rotation[0] += _x;
	rotation[1] += _y;
	rotation[2] += _z;
}

void TObject::SetScale(std::vector<float> _scale)
{
	if (_scale.size() == 3)
	{
		scale = _scale;
	}
}

void TObject::SetScale(float _x, float _y, float _z)
{
	scale[0] = _x;
	scale[1] = _y;
	scale[2] = _z;
}

void TObject::AddScale(std::vector<float> _scale)
{
	if (_scale.size() == 3)
	{
		for (int i = 0; i < 3; i++)
		{
			scale[i] += _scale[i];
		}
	}
}

void TObject::AddScale(float _x, float _y, float _z)
{
	scale[0] += _x;
	scale[1] += _y;
	scale[2] += _z;
}

void TObject::CreatePipeline(FbxMeshData* _nowMesh)
{
	if (!_nowMesh->vertexData.size() == 0)
	{
		_nowMesh->pipeline = new PipeLine();
		this->gp->CreateInputLayer(&_nowMesh->pipeline->inputLayout, VertexF::forwardInputLayderDECS, 2, &_nowMesh->pipeline->vertexShader, this->path[0]);
		this->gp->CreatePixelShader(&_nowMesh->pipeline->pixelShader, this->path[1]);
		this->gp->CreateRasterizerState(&_nowMesh->pipeline->rasterizerState);
		_nowMesh->pipeline->primitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		_nowMesh->pipeline->vertexStructSize = VertexF::Size();

		size_t matCount = _nowMesh->matrialList.size();
		for (size_t i = 0; i < matCount; i++)
		{
			std::vector<std::wstring> wTextruePath;
			int factorCount = this->tpData->materialMap[_nowMesh->matrialList[i]]->materialTypeCount;
			for (int j = 0; j < factorCount; j++)
			{
				std::wstring sTexturePath;
				sTexturePath.assign(this->tpData->materialMap[_nowMesh->matrialList[i]]->filePath[0].begin(),
					this->tpData->materialMap[_nowMesh->matrialList[i]]->filePath[0].end());
				wTextruePath.push_back(this->texturePath + sTexturePath);
			}
			_nowMesh->textuers.push_back(new ID3D11ShaderResourceView * [factorCount]);
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
			this->gp->CreateIndexBuffer(_nowMesh->indexBufferData[i], static_cast<UINT>(_nowMesh->indexData[i].size()), &_nowMesh->indexBuffer[i]);
		}

	}

	for (auto& c : _nowMesh->children)
	{
		CreatePipeline(c);
	}
}

void TObject::ReleasePipeline(FbxMeshData* _nowMesh)
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

void TObject::RenderMesh(FbxMeshData* _nowData)
{
	/*gp->BindMatrixParameter(_nowData->globalTM);
	for (size_t i = 0; i < _nowData->indexBuffer.size(); i++)
	{
		_nowData->pipeline->IndexBuffer = _nowData->indexBuffer[i];
		gp->BindPipeline(*_nowData->pipeline);
		gp->SetTexture(0, 1, _nowData->textuers[i]);
		gp->Render(*_nowData->pipeline, static_cast<int>(_nowData->indexData[i].size()));
	}

	for (auto& child : _nowData->children)
	{
		RenderMesh(child);
	}*/
}

void TObject::UpdateMatrix(FbxMeshData* _nowData)
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
