#include "AssimpType.h"

void asSkeleton::ReadFromAssimp(const aiScene* _scene)
{
	int numNode = 0;
	CountNode(numNode, _scene->mRootNode);
	this->bones.reserve(numNode);
	CreateBone(_scene, _scene->mRootNode);
}

asBone* asSkeleton::CreateBone(const aiScene* _scene, const aiNode* _node)
{
	asBone& bone = this->bones.emplace_back();
	bone.Set(_node);

	int boneIndex = static_cast<int>(this->bones.size()) - 1;
	this->boneMappingTable[bone.name] = boneIndex;

	if (_node->mNumMeshes > 0)
	{
		bone.meshName.resize(_node->mNumMeshes);
		for (uint32 i = 0; i < _node->mNumMeshes; i++)
		{
			uint32 meshIndex = _node->mMeshes[i];
			aiMesh* mesh = _scene->mMeshes[meshIndex];

			std::string meshName = mesh->mName.C_Str();
			meshMappingTable[meshName] = boneIndex;
		}
	}

	for (uint32 i = 0; i < _node->mNumChildren; i++)
	{
		asBone* child = CreateBone(_scene, _node->mChildren[i]);
		child->parentIndex = boneIndex;
	}
	return &(this->bones[boneIndex]);
}


asBone* asSkeleton::FindBone(const std::string& _name)
{
	auto iter = this->boneMappingTable.find(_name);
	if (iter == this->boneMappingTable.end())
	{
		return nullptr;
	}
	return &(this->bones[iter->second]);
}

asBone* asSkeleton::GetBone(int _index)
{
	if (_index < 0 || _index >= this->bones.size())
	{
		return nullptr;
	}
	return &(this->bones[_index]);
}

int asSkeleton::GetBoneIndexByBoneName(const std::string& _boneName)
{
	auto iter = this->boneMappingTable.find(_boneName);
	if (iter == this->boneMappingTable.end())
	{
		return -1;
	}
	return iter->second;
}

int asSkeleton::GetBoneIndexByMeshName(const std::string& _meshName)
{
	auto iter = this->meshMappingTable.find(_meshName);
	if (iter == this->meshMappingTable.end())
	{
		return -1;
	}
	return iter->second;
}

void asSkeleton::CountNode(int& _count, const aiNode* _node)
{
	_count++;
	std::string name = _node->mName.C_Str();
	this->boneMappingTable[name] = _count;

	for (uint32 i = 0; i < _node->mNumChildren; i++)
	{
		CountNode(_count, _node->mChildren[i]);
	}
}