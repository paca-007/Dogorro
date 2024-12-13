#include "RenderObject.h"
#include "Mesh.h"
#include "GraphicsEngine.h"

RenderObject::RenderObject()
	: name{}
	, nodeTM(DirectX::XMMatrixIdentity())
	, isHelper(false)
	, meshes(std::vector<Mesh*>())
	, parent(nullptr)
	, children(std::vector<RenderObject*>())
	, localTM(DirectX::XMMatrixIdentity())
	, animationPositionTrack(std::vector<std::pair<int, DirectX::XMFLOAT3>>())
	, animationRotateTrack(std::vector<std::pair<int, DirectX::XMFLOAT4>>())
	, positionTrackIndex(0)
	, rotateTrackIndex(0)
	, nowTick(0)
	, oneTick(0.0001f * 1)
	, accTick(0.0f)
	, maxTick(0)
	, isAnimation(false)
	, localScale{}
	, localRotate{}
	, localPosition{}
	, animationTM(DirectX::XMMatrixIdentity())
	, nodePosition{}
	, nodeRotate{}
	, nodeScale{}
	, isNegative(false)
	, fileScale{}
	, fileRotate{}
	, filePosition{}
	, type(RENDER_OBJECT_TYPE::GEOMOBJCT)
{
	this->demoMat.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	this->demoMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	this->demoMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

void RenderObject::AddMesh(Mesh* _mesh)
{
	meshes.push_back(_mesh);
}

void RenderObject::AddChild(RenderObject* _child)
{
	children.push_back(_child);
}

void RenderObject::SetParent(RenderObject* _parent)
{
	this->parent = _parent;
}

void RenderObject::Render(GraphicsEngine* _graphicsEngine)
{
	for (auto& m : this->meshes)
	{
		m->Render(_graphicsEngine, this->nodeTM);
	}
	for (auto& c : this->children)
	{
		c->Render(_graphicsEngine);
	}
}

void RenderObject::Initalize(GraphicsEngine* _graphicsEngine, std::wstring _path)
{
	this->originalNodeTM = this->nodeTM;
	for (auto& m : this->meshes)
	{
		m->SetVertexesData();
	}

	this->Localize(_graphicsEngine, _path);

	for (auto& c : this->children)
	{
		c->Initalize(_graphicsEngine);
	}
}

void RenderObject::Localize(GraphicsEngine* _graphicsEngine, std::wstring _path)
{
	assert(DirectX::XMMatrixDecompose(&this->nodeScale, &this->nodeRotate, &this->nodePosition, this->nodeTM) &&
		"cannot decompose node Tm");

	DirectX::XMMATRIX nodeTemp = this->nodeTM;
	nodeTemp *= DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranslationFromVector(this->filePosition));

	if (!DirectX::XMVector3Equal(fileRotate, DirectX::XMVectorZero()))
	{
		this->fileRotate = DirectX::XMQuaternionRotationAxis(this->fileRotate, this->fileRotate.m128_f32[3]);
		nodeTemp *= DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixRotationQuaternion(this->fileRotate));
	}

	if (nodeTemp.r[1].m128_f32[1] - (-1.0f) < abs(0.1f))
	{
		this->hasNegativeScale = true;
		if (this->parent && this->parent->hasNegativeScale != true)
		{
			this->isNegative = true;
		}
	}
	else
	{
		if (this->parent && this->parent->hasNegativeScale == true)
		{
			this->isNegative = true;
		}
	}

	if (this->parent)
	{
		DirectX::XMMATRIX pInvers;
		pInvers = DirectX::XMMatrixInverse(nullptr, this->parent->nodeTM);
		this->localTM = this->nodeTM * pInvers;
	}
	else
	{
		localTM = nodeTM;
	}

	assert(DirectX::XMMatrixDecompose(&this->localScale, &this->localRotate, &this->localPosition, this->localTM) &&
		"cannot decompose local Tm");


	for (size_t i = 0; i < this->animationRotateTrack.size(); i++)
	{
		DirectX::XMFLOAT4& data = this->animationRotateTrack[i].second;

		data.x = (float)sin(data.w / 2.0f) * data.x;
		data.y = (float)sin(data.w / 2.0f) * data.y;
		data.z = (float)sin(data.w / 2.0f) * data.z;
		data.w = (float)cos(data.w / 2.0f);

		if (i != 0)
		{
			DirectX::XMVECTOR dataVector = DirectX::XMLoadFloat4(&data);
			DirectX::XMVECTOR prevVector = DirectX::XMLoadFloat4(&this->animationRotateTrack[i - 1].second);
			dataVector = DirectX::XMQuaternionMultiply(prevVector, dataVector);
			DirectX::XMStoreFloat4(&data, dataVector);
		}
	}

	DirectX::XMMATRIX invers = DirectX::XMMatrixInverse(nullptr, nodeTM);
	DirectX::XMMATRIX inversTarnspose = DirectX::XMMatrixTranspose(invers);

	for (auto& m : this->meshes)
	{
		for (int i = 0; i < m->vertexList.size(); i++)
		{
			DirectX::XMVECTOR position = DirectX::XMLoadFloat3(&m->vertexes[i].position);
			position.m128_f32[3] = 1.0f;
			position = DirectX::XMVector4Transform(position, invers);
			DirectX::XMStoreFloat3(&m->vertexes[i].position, position);
		}
		m->CreatePipeline(_graphicsEngine, this->path, _path);
	}
}

void RenderObject::Update(float _dt)
{
	UpdateAnimation(_dt);

	if (this->parent)
	{
		this->nodeTM = this->localTM * this->parent->nodeTM;
	}
	else
	{
		this->nodeTM = this->localTM;
	}

	for (auto& c : this->children)
	{
		c->Update(_dt);
	}

}

void RenderObject::Translate(float _x, float _y, float _z)
{
	this->localTM *= DirectX::XMMatrixTranslation(_x, _y, _z);
}

void RenderObject::RoateBaseAxis(float _x, float _y, float _z)
{
	this->localTM *= DirectX::XMMatrixRotationX(_x);
	this->localTM *= DirectX::XMMatrixRotationY(_y);
	this->localTM *= DirectX::XMMatrixRotationZ(_z);
}

void RenderObject::Scale(float _x, float _y, float _z)
{

}

void RenderObject::UpdateAnimation(float _dt)
{
	this->accTick += _dt * 1;
	while (this->accTick >= this->oneTick)
	{
		this->accTick -= this->oneTick;
		this->nowTick++;
		if (this->nowTick > this->maxTick)
		{
			this->nowTick = 0;
			this->positionTrackIndex = 0;
			this->rotateTrackIndex = 0;
		}
	}

	this->localTM = DirectX::XMMatrixIdentity();

	if (this->isNegative && !this->animationRotateTrack.empty())
	{
		this->localTM *= DirectX::XMMatrixScalingFromVector(DirectX::XMVECTOR{ -1.0f, -1.0f, -1.0f });
	}
	else
	{
		this->localTM *= DirectX::XMMatrixScalingFromVector(this->localScale);
	}



	if (!this->animationRotateTrack.empty())
	{
		if (this->rotateTrackIndex < (int)animationRotateTrack.size())
		{
			if (this->animationRotateTrack[this->rotateTrackIndex].first <= this->nowTick)
			{
				this->rotateTrackIndex++;
				if (rotateTrackIndex == animationRotateTrack.size())
				{
					this->rotateTrackIndex = (int)this->animationRotateTrack.size() - 1;
				}
			}
		}

		const DirectX::XMFLOAT4 nowRotate = this->animationRotateTrack[this->rotateTrackIndex].second;
		DirectX::XMVECTOR rotateNowVector = DirectX::XMLoadFloat4(&nowRotate);

		if (this->rotateTrackIndex != 0)
		{
			const DirectX::XMFLOAT4 prevRotate = this->animationRotateTrack[this->rotateTrackIndex - 1].second;
			DirectX::XMVECTOR rotatePrevVector = DirectX::XMLoadFloat4(&prevRotate);

			float t1 = (float)this->animationRotateTrack[this->rotateTrackIndex - 1].first;
			float t2 = (float)this->animationRotateTrack[this->rotateTrackIndex].first;
			float lerp = (nowTick - t1) / (t2 - t1);
			if (lerp >= 1.0f)
			{
				lerp = 1.0f;
			}
			rotateNowVector = DirectX::XMQuaternionSlerp(rotatePrevVector, rotateNowVector, lerp);
		}
		this->localTM *= DirectX::XMMatrixRotationQuaternion(rotateNowVector);
	}
	else
	{
		this->localTM *= DirectX::XMMatrixRotationQuaternion(this->localRotate);
	}

	if (this->animationPositionTrack.size() > 0)
	{
		if (this->positionTrackIndex < animationPositionTrack.size())
		{
			if (this->animationPositionTrack[this->positionTrackIndex].first <= this->nowTick)
			{
				this->positionTrackIndex++;
				if (positionTrackIndex == animationPositionTrack.size())
				{
					this->positionTrackIndex = (int)this->animationPositionTrack.size() - 1;
				}
			}
		}
		DirectX::XMFLOAT3 nowPosition = this->animationPositionTrack[this->positionTrackIndex].second;

		if (this->positionTrackIndex != 0)
		{
			const DirectX::XMFLOAT3 prevPosition = this->animationPositionTrack[this->positionTrackIndex - 1].second;
			float t1 = (float)this->animationPositionTrack[this->positionTrackIndex - 1].first;
			float t2 = (float)this->animationPositionTrack[this->positionTrackIndex].first;
			float lerp = (nowTick - t1) / (t2 - t1);

			if (lerp >= 1.0f)
			{
				lerp = 1.0f;
			}

			DirectX::XMVECTOR nowVector = DirectX::XMLoadFloat3(&nowPosition);
			DirectX::XMVECTOR prevVector = DirectX::XMLoadFloat3(&prevPosition);
			nowVector = DirectX::XMVectorLerp(prevVector, nowVector, lerp);

			DirectX::XMStoreFloat3(&nowPosition, nowVector);
		}
		this->localTM *= DirectX::XMMatrixTranslation(nowPosition.x, nowPosition.y, nowPosition.z);
	}
	else
	{
		this->localTM *= DirectX::XMMatrixTranslationFromVector(this->localPosition);
	}
}

