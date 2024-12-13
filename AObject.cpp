#include "AObject.h"
#include "FileUtils.h"
#include "Utils.h"
#include "ABone.h"
#include "AMesh.h"
#include "AMaterial.h"
#include "AAnimation.h"
#include "AAnimator.h"
#include <memory>
#include "VertexType.h"
#include <filesystem>
#include "tinyxml2.h"
#include "GraphicsEngine.h"
#include "DeferredRenderer.h"
#include "ForwardRenderer.h"
#include "BackgroundRenderer.h"
 
AObject::AObject(std::string _path, IGraphicsEngine* _igp, bool _hasBone, bool _hasAnimation, bool _isVisible)
	: filePath(_path)
	, hasAnimaiton(_hasAnimation)
	, hasBone(_hasBone)
	, isVisible(_isVisible)
	, attachedObject{}
{
	gp = dynamic_cast<GraphicsEngine*>(_igp);
	position = std::vector<float>{ 0.0f, 0.0f, 0.0f };
	finalPos = std::vector<float>{ 0.0f, 0.0f, 0.0f };
	rotation = std::vector<float>{ 0.0f, 0.0f, 0.0f };
	scale = std::vector<float>{ 1.0f, 1.0f, 1.0f };
	ReadMaterial();
	ReadMesh();
	if (_hasAnimation)
	{
		ReadAnimation();
		animator = std::make_shared<AAnimator>();
		animator->CreateAniMatrix(this);
	}
	CreateMeshBuffer();
	BindData();
	DecomposeMeshMatrix();

	this->forwardRenderer = gp->forwardRenderer;
}

AObject::~AObject()
{

}

void AObject::Render()
{
	//Forward, 즉 투명한 물체는 forward로 그려져야 한다.
	//여기서 안 그리고 ForwardRenderer vector에 저장해두고 그려야한다.
	if (isVisible == false)
	{
		forwardRenderer->GetData
		(
			this->meshes,
			this->bones,
			this->position,
			this->rotation,
			this->scale
		);
	}
	//Background는 deferred로 그리면 안됨니다. 이름에 넣고 싶은데
	else if (GetName().find("floor") != std::string::npos || GetName().find("wall") != std::string::npos || GetName().find("window") != std::string::npos)
	{
		this->backgroundRenderer = gp->backgroundRenderer;
		backgroundRenderer->GetData
		(
			this->meshes,
			this->position,
			this->rotation,
			this->scale
		);
	}
	else
	{
		for (auto& m : this->meshes)
		{
			Matrix movement = Matrix::Identity;

			Vector3 objectRotation = { this->rotation[0], this->rotation[1], this->rotation[2] };
			Vector3 objectPosition = { this->position[0], this->position[1] , this->position[2] };
			Vector3 objectScale = { this->scale[0], this->scale[1] , this->scale[2] };

			Quaternion quaternionRotation = Quaternion::CreateFromYawPitchRoll(objectRotation);

			if (!this->attachedObject.expired())
			{
				Vector3 bonePosition;
				Quaternion boneQuaternion;
				Vector3 tempScale;

				Matrix temp = this->attachedObject.lock()->transform;
				temp.Decompose(tempScale, boneQuaternion, bonePosition);

				auto& temprotVec = this->attachedObject.lock()->obj->rotation;
				Vector3 tempRotation = { temprotVec[0], temprotVec[1], temprotVec[2] };
				Quaternion objQuaternion = Quaternion::CreateFromYawPitchRoll(tempRotation);

				auto& tempposVec = this->attachedObject.lock()->obj->position;
				Vector3 tempPosition = { tempposVec[0], tempposVec[1], tempposVec[2] };

				auto& objRot = this->attachedObject.lock()->obj->rotation;
				Vector3 objRotVec = { objRot[0], objRot[1], objRot[2] };
				Quaternion objrotQuaternion = Quaternion::CreateFromYawPitchRoll(objRotVec);

				auto& objScale = this->attachedObject.lock()->obj->scale;
				tempScale = { objScale[0], objScale[1], objScale[2] };


				movement *= Matrix::CreateScale(objectScale);
				movement *= Matrix::CreateFromQuaternion(boneQuaternion * objQuaternion);
				movement *= Matrix::CreateTranslation(bonePosition);
				movement *= Matrix::CreateScale(tempScale);
				movement *= Matrix::CreateFromQuaternion(objrotQuaternion);
				movement *= Matrix::CreateTranslation(tempPosition);

				Vector3 fianlPosition;
				Quaternion finalQuaternion;
				Vector3 finalScale;

				movement.Decompose(finalScale, finalQuaternion, fianlPosition);
				movement = Matrix::CreateScale(objectScale);
				movement *= Matrix::CreateFromQuaternion(finalQuaternion);
				movement *= Matrix::CreateTranslation(fianlPosition);
			}
			else
			{
				movement *= Matrix::CreateScale(objectScale);
				movement *= Matrix::CreateFromQuaternion(quaternionRotation);
				movement *= Matrix::CreateTranslation(objectPosition);
			}

			Vector3 fPos = { 0.0f, 0.0f, 0.0f };
			fPos = Vector3::Transform(fPos, movement);
			this->finalPos[0] = fPos.x;
			this->finalPos[1] = fPos.y;
			this->finalPos[2] = fPos.z;
			this->gp->BindBonesData(this->bones, m->ori);
			this->gp->BindMatrixParameter(movement);
			this->gp->BindPipeline(m->pip);
			this->gp->SetTexture(0, 1, m->material.lock()->diffusMap);
			this->gp->Render(m->pip, static_cast<int>(m->indexData.size()));
		}
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="_name"></param>
/// <param name="_dt"></param>
void AObject::ApplyAnimation(std::string _name, float _dt)
{
	int aniIndex = this->animationMap[_name];

	std::weak_ptr<AAnimation> nowAni = this->animations[aniIndex];

	// _dt /= 10;

	if (_dt < 0)
	{
		_dt *= -1;
		_dt = ::fmod(_dt, nowAni.lock()->duration);
		_dt = nowAni.lock()->duration - _dt;
	}
	else
	{
		_dt = ::fmod(_dt, (nowAni.lock()->duration / nowAni.lock()->frameCount));
	}

	uint32 nowFrame = static_cast<uint32>(_dt * nowAni.lock()->frameCount);
	uint32 nextFrame = nowFrame + 1;
	float ratio = nextFrame - (_dt * nowAni.lock()->frameCount);

	if (nowFrame < 0)
	{
		nowFrame = 0;
	}
	if (nextFrame >= nowAni.lock()->frameCount)
	{
		nextFrame = nowFrame;
	}

	for (auto& bone : this->bones)
	{
		// 		if (bone->index >= 0)
		// 		{
		// 			bone->transform = Matrix::Identity;
		// 
		// 			AFrameData nowData;
		// 			AFrameData nextData;
		// 			AFrameData result;
		// 
		// 			nowData = this->animations[aniIndex]->keyframes[bone->name]->transforms[nowFrame];
		// 			nextData = this->animations[aniIndex]->keyframes[bone->name]->transforms[nextFrame];
		// 
		// 			result.translation = DirectX::SimpleMath::Vector3::Lerp(nextData.translation, nowData.translation, ratio);
		// 			result.scale = DirectX::SimpleMath::Vector3::Lerp(nextData.scale, nowData.scale, ratio);
		// 			result.rotation = DirectX::SimpleMath::Quaternion::Slerp(nextData.rotation, nowData.rotation, ratio);
		// 			Matrix resultMatrix = Matrix::CreateScale(result.scale);
		// 			resultMatrix *= Matrix::CreateFromQuaternion(result.rotation);
		// 			resultMatrix *= Matrix::CreateTranslation(result.translation);
		// 
		// 			if (bone->parentIndex >= 0)
		// 			{
		// 				bone->transform = resultMatrix * bone->parent.lock()->transform;
		// 			}
		// 			else
		// 			{
		// 				bone->transform = resultMatrix;
		// 			}
		// 		}
		bone->transform = DirectX::SimpleMath::Matrix::Lerp(
			animator->aniMatrix[aniIndex][nextFrame][bone->index],
			animator->aniMatrix[aniIndex][nowFrame][bone->index],
			ratio
		);
		// bone->transform = animator->aniMatrix[aniIndex][nowFrame][bone->index];
	}
}

void AObject::SetPosition(float _x, float _y, float _z)
{
	this->position[0] = _x;
	this->position[1] = _y;
	this->position[2] = _z;
}

void AObject::AddPosition(float _x, float _y, float _z)
{
	this->position[0] += _x;
	this->position[1] += _y;
	this->position[2] += _z;
}

void AObject::SetRotation(float _x, float _y, float _z)
{
	this->rotation[0] = _x;
	this->rotation[1] = _y;
	this->rotation[2] = _z;
}

void AObject::AddRotation(float _x, float _y, float _z)
{
	this->rotation[1] += _x;
	this->rotation[0] += _y;
	this->rotation[2] += _z;
}

void AObject::SetScale(float _x, float _y, float _z)
{
	this->scale[0] = _x;
	this->scale[1] = _y;
	this->scale[2] = _z;
}

void AObject::AddScale(float _x, float _y, float _z)
{
	this->scale[0] += _x;
	this->scale[1] += _y;
	this->scale[2] += _z;
}

void AObject::SetOutline(bool _val)
{
	this->isOutline = _val;
}

std::string AObject::GetName(int _index)
{
	if (_index < 0 || _index >= this->meshes.size())
	{
		return "";
	}
	return meshes[_index]->name;
}


/// <summary>
/// 나한테 어떤 오브젝트를 붙인다.
/// </summary>
/// <param name="_other">붙을 오브젝트</param>
/// <param name="_where">붙을 본 이름</param>
void AObject::Attach(AObject* _other, std::string _where)
{
	if (this->boneMap.find(_where) == this->boneMap.end())
	{
		return;
	}

	_other->attachBoneName = _where;
	_other->attachedObject = this->bones[this->boneMap[_where]];
}

/// <summary>
/// 내가 다른 오브젝트에 붙는다.
/// </summary>
/// <param name="_other">붙을 오브젝트</param>
/// <param name="_where">붙을 본 이름</param>
void AObject::AttachTo(AObject* _other, std::string _where)
{
	if (_other->boneMap.find(_where) == _other->boneMap.end())
	{
		return;
	}

	this->attachBoneName = _where;
	this->attachedObject = _other->bones[_other->boneMap[_where]];
	this->meshes[0]->ori = Matrix::Identity;
	this->meshes[0]->oriPos = Vector3{ 0.0f, 0.0f, 0.0f };
	this->meshes[0]->oriRot = Quaternion::CreateFromRotationMatrix(Matrix::Identity);
}
/// <summary>
/// 연결되어 있다면 떨군다.
/// </summary>
void AObject::Detach()
{
	if (this->attachedObject.expired())
	{
		return;
	}
	Vector3 bonePosition;
	Quaternion boneQuaternion;
	Vector3 tempScale;

	Matrix temp = this->attachedObject.lock()->transform;
	temp.Decompose(tempScale, boneQuaternion, bonePosition);

	auto& temprotVec = this->attachedObject.lock()->obj->rotation;
	Vector3 tempRotation = { temprotVec[0], temprotVec[1], temprotVec[2] };
	Quaternion objQuaternion = Quaternion::CreateFromYawPitchRoll(tempRotation);

	auto& tempposVec = this->attachedObject.lock()->obj->position;
	Vector3 tempPosition = { tempposVec[0], tempposVec[1], tempposVec[2] };

	Quaternion finalQuaternion = boneQuaternion * objQuaternion;
	Vector3 finalRotation = finalQuaternion.ToEuler();
	Vector3 finalPos = bonePosition + tempPosition;

	this->position[0] = finalPos.x;
	this->position[1] = finalPos.y;
	this->position[2] = finalPos.z;

	this->rotation[0] = finalRotation.x;
	this->rotation[1] = finalRotation.y;
	this->rotation[2] = finalRotation.z;

	this->attachedObject.reset();
}

/// <summary>
/// 나한테 연결 되어 있는 누군가를 떨군다.
/// </summary>
/// <param name="_other">연결되어 있다고 생각하는 물체</param>
void AObject::Detach(AObject* _other)
{
	_other->Detach();
}

ABoundingBox AObject::GetBoundingBox(int _index /*= 0*/)
{
	Vector3 maxAABB = this->meshes[_index]->maxAABB;
	Vector3 minAABB = this->meshes[_index]->minAABB;

	ABoundingBox result;
	result.xLen = maxAABB.x - minAABB.x;
	result.yLen = maxAABB.y - minAABB.y;
	result.zLen = maxAABB.z - minAABB.z;

	result.cx = maxAABB.x - result.xLen + this->position[0];
	result.cy = maxAABB.y - result.yLen + this->position[1];
	result.cz = maxAABB.z - result.zLen + this->position[2];

	return result;
}

void AObject::ReadMaterial()
{
	std::string fullPath = this->texturePath + this->filePath + ".xml";
	auto parentPath = std::filesystem::path(Utils::ToWString(fullPath)).parent_path();

	tinyxml2::XMLDocument* document = new tinyxml2::XMLDocument();
	tinyxml2::XMLError error = document->LoadFile((fullPath).c_str());
	assert(error == tinyxml2::XML_SUCCESS);

	tinyxml2::XMLElement* root = document->FirstChildElement();
	tinyxml2::XMLElement* materialNode = root->FirstChildElement();


	while (materialNode)
	{
		std::shared_ptr<AMaterial> material = std::make_shared<AMaterial>();

		tinyxml2::XMLElement* node = nullptr;
		node = materialNode->FirstChildElement();

		std::string name = node->GetText();
		material->name = name;
		// RESOURCES->Add(name, material);

		// Diffuse Texture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			std::string textureStr = node->GetText();
			if (textureStr.length() > 0)
			{
				std::wstring path = Utils::ToWString(this->texturePath);
				path += Utils::ToWString(textureStr);
				gp->LoadTexture(path, material->diffusMap);
			}
		}

		if (material->diffusMap.Get() == nullptr)
		{
			std::wstring defalutTetuer = L"../AssimpData/Textures/top.jpg";
			gp->LoadTexture(defalutTetuer, material->diffusMap);
		}

		// Specular Texture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			wstring texture = Utils::ToWString(node->GetText());
			if (texture.length() > 0)
			{
				std::string textureStr = node->GetText();
				if (textureStr.length() > 0)
				{
					std::wstring path = Utils::ToWString(this->texturePath);
					path += Utils::ToWString(textureStr);
					gp->LoadTexture(path, material->specularMap);
				}
			}
		}

		// Normal Texture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			std::string textureStr = node->GetText();
			if (textureStr.length() > 0)
			{
				std::wstring path = Utils::ToWString(this->texturePath);
				path += Utils::ToWString(textureStr);
				gp->LoadTexture(path, material->normalMap);
			}
		}

		// Ambient
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->ambient = color;
		}

		// Diffuse
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->diffuse = color;
		}

		// Specular
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->specular = color;
		}

		// Emissive
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			material->emissive = color;
		}

		this->materials.push_back(material);

		// Next Material
		materialNode = materialNode->NextSiblingElement();
	}

	delete document;
}

void AObject::ReadMesh(bool _isMap)
{
	std::string fullPath = this->modelPath + this->filePath + ".mesh";

	size_t lastSlahPosition = this->filePath.find_last_of('/');
	this->name = this->filePath.substr(lastSlahPosition + 1);

	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(Utils::ToWString(fullPath), FileMode::Read);

	/// Read Bone
	if (!_isMap)
	{
		const uint32 boneCount = file->Read<uint32>();
		for (uint32 i = 0; i < boneCount; i++)
		{
			std::shared_ptr<ABone> bone = std::make_shared<ABone>();

			bone->index = file->Read<int32>();
			bone->name = file->Read<std::string>();
			bone->parentIndex = file->Read<int32>();
			bone->transform = file->Read<Matrix>();
			bone->offsetTrasform = file->Read<Matrix>();
			bone->originalLocalTransform = bone->transform;
			this->bones.push_back(bone);
		}
	}

	/// Read Mesh
	const uint32 meshCount = file->Read<uint32>();
	for (uint32 i = 0; i < meshCount; i++)
	{
		std::shared_ptr<AMesh> mesh = std::make_shared<AMesh>();
		mesh->name = file->Read<std::string>();
		mesh->boneIndex = file->Read<int32>();

		mesh->materialName = file->Read<std::string>();

		/// Read Mesh Vertex
		const uint32 vertexCount = file->Read<uint32>();
		std::vector<ModelVertexType> vertices(vertexCount);

		void* vData = vertices.data();
		file->Read(&vData, sizeof(ModelVertexType) * vertexCount);

		mesh->vertexData = std::move(vertices);

		/// Read Index Data
		const uint32 indexCount = file->Read<uint32>();

		std::vector<uint32> indices(indexCount);

		void* iData = indices.data();
		file->Read(&iData, sizeof(uint32) * indexCount);
		mesh->indexData = std::move(indices);

		mesh->maxAABB = file->Read<Vector3>();
		mesh->minAABB = file->Read<Vector3>();

		this->meshes.push_back(mesh);
	}

	file->Close();
}

void AObject::ReadAnimation()
{
	std::string fullPath = this->modelPath + this->filePath + ".clip";
	if (!std::filesystem::exists(fullPath))
	{
		return;
	}
	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(Utils::ToWString(fullPath), FileMode::Read);

	uint32 aniCount = file->Read<uint32>();

	for (uint32 i = 0; i < aniCount; i++)
	{
		std::shared_ptr<AAnimation> animation = std::make_shared<AAnimation>();

		animation->name = file->Read<string>();
		size_t lastComma = animation->name.find_last_of('|');
		this->animationMap[animation->name.substr(lastComma + 1)] = i;
		animation->duration = file->Read<float>();
		animation->frameRate = file->Read<float>();
		animation->frameCount = file->Read<uint32>();

		uint32 keyframesCount = file->Read<uint32>();

		for (uint32 i = 0; i < keyframesCount; ++i)
		{
			std::shared_ptr<AKeyFrame> keyframe = std::make_shared<AKeyFrame>();
			keyframe->boneName = file->Read<string>();

			uint32 size = file->Read<uint32>();

			if (size > 0)
			{
				keyframe->transforms.resize(size);
				void* ptr = &keyframe->transforms[0];
				file->Read(&ptr, sizeof(AFrameData) * size);
			}

			// 		for (size_t j = 1; j < keyframe->transforms.size(); j++)
			// 		{
			// 			keyframe->transforms[j].rotation *= keyframe->transforms[j - 1].rotation;
			// 		}

			animation->keyframes[keyframe->boneName] = keyframe;
		}

		this->animations.push_back(animation);
	}
	file->Close();
}

void AObject::DecomposeMeshMatrix()
{
	// Mesh 에 Bone 캐싱
	for (const auto& mesh : this->meshes)
	{
		mesh->ori.Decompose(mesh->oriSca, mesh->oriRot, mesh->oriPos);
		mesh->ori = Matrix::Identity;

		this->position[0] = mesh->oriPos.x;
		this->position[1] = mesh->oriPos.y;
		this->position[2] = mesh->oriPos.z;

		this->scale[0] = mesh->oriSca.x;
		this->scale[1] = mesh->oriSca.y;
		this->scale[2] = mesh->oriSca.z;

		Vector3 tmepRot = mesh->oriRot.ToEuler();
		this->rotation[1] = tmepRot.x;
		this->rotation[0] = tmepRot.y;
		this->rotation[2] = tmepRot.z;
	}
}

std::shared_ptr<AMaterial> AObject::GetMaterial(std::string _name)
{
	for (auto& material : this->materials)
	{
		if (material->name == _name)
		{
			return material;
		}
	}
	return nullptr;
}

std::shared_ptr<ABone> AObject::GetBone(int _index)
{
	if (_index > 0 && _index < this->bones.size())
	{
		return this->bones[_index];
	}
	return nullptr;
}

void AObject::BindData()
{
	// Mesh 에 Material 캐싱
	for (const auto& mesh : this->meshes)
	{
		// 이미 찾았으면 스킵
		if (!mesh->material.expired())
		{
			continue;
		}

		mesh->material = GetMaterial(mesh->materialName);
	}



	// Bone 계층 정보 채우기
	if (this->rootBone == nullptr && this->bones.size() > 0)
	{
		this->rootBone = this->bones[0];

		this->boneMap[this->rootBone->name] = 0;

		for (auto& bone : this->bones)
		{
			this->boneMap[bone->name] = bone->index;
			bone->obj = this;
			if (bone->parentIndex >= 0)
			{
				bone->parent = this->bones[bone->parentIndex];
				bone->parent.lock()->children.push_back(bone);

				bone->transform *= bone->parent.lock()->transform;
				bone->oriinalWorldTransform = bone->transform;
			}
			else
			{
				bone->parent.reset(); // nullptr
			}
		}
	}

	// Mesh 에 Bone 캐싱
	for (const auto& mesh : this->meshes)
	{
		// 이미 찾았으면 스킵
		if (!mesh->bone.expired())
		{
			continue;
		}

		mesh->bone = GetBone(mesh->boneIndex);
		if (mesh->bone.expired())
		{
			continue;
		}
		mesh->ori = mesh->bone.lock()->transform;
	}
}

void AObject::CreateMeshBuffer()
{
	for (auto& mesh : this->meshes)
	{
		gp->CreateVertexBuffer<ModelVertexType>(mesh->vertexData.data(), static_cast<UINT>(sizeof(ModelVertexType) * mesh->vertexData.size()), mesh->pip.vertexBuffer, mesh->name);
		gp->CreateIndexBuffer(mesh->indexData.data(), static_cast<UINT>(mesh->indexData.size()), mesh->pip.IndexBuffer);

		if (isVisible == true)
		{
			if (this->hasBone)
			{
				mesh->SetVS(gp, "../Shader/compiled/AssimpVS.cso");
			}
			else
			{
				mesh->SetVS(gp, "../Shader/compiled/AssimpVSNoneBone.cso");
			}
			mesh->SetPS(gp, "../Shader/compiled/AssimpPSPass1.cso");
		}

		//background도 따로 들어가야한다.
		else if (GetName().find("floor") == std::string::npos|| GetName().find("wall") == std::string::npos || GetName().find("window") == std::string::npos)
		{
			mesh->SetVS(gp, "../Shader/compiled/BackgroundVS.cso");
			mesh->SetPS(gp, "../Shader/compiled/BackgroundPS.cso");
		}

		//위에와 마찬가지로 투명한 물체에 대해서는 PS가 따로 들어가야 한다.
		if(isVisible == false)
		{
			if (this->hasBone)
			{
				mesh->SetVS(gp, "../Shader/compiled/AssimpVS.cso");
			}
			else
			{
				mesh->SetVS(gp, "../Shader/compiled/AssimpVSNoneBone.cso");
			}

			mesh->SetPS(gp, "../Shader/compiled/ForwardPixelShader.cso");
		}

	}
}
