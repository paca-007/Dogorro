#pragma once
#include "pch.h"
#include "VertexType.h"

using VertexType = VertexTextureNormalTangentBlendData;

struct asMesh;

/// <summary>
/// Assimp �� ���ؼ� �� �������� �޸𸮿� �����صд�.
/// �� ������� customData ������ �����.
/// </summary>
struct asBone
{
	std::string name = "";
	int32 numChildren = 0;
	int32 index = -1;
	int32 parentIndex = -1;
	Matrix relativeTransform = {};
	std::vector<std::string> meshName;

	void Set(const aiNode* _node)
	{
		this->name = _node->mName.C_Str();
		this->relativeTransform = Matrix(&_node->mTransformation.a1).Transpose();
		this->numChildren = _node->mNumChildren;
	}


	Matrix offsetMatrix;
};

struct asSkeleton
{
public:
	std::vector<asBone> bones;
	std::map<std::string, int> boneMappingTable;
	std::map<std::string, int> meshMappingTable;

	void ReadFromAssimp(const aiScene* _scene);
	asBone* CreateBone(const aiScene* _scene, const aiNode* _node);
	asBone* FindBone(const std::string& _name);
	asBone* GetBone(int _index);
	int GetBoneIndexByBoneName(const std::string& _boneName);
	int GetBoneIndexByMeshName(const std::string& _meshName);
	int GetBoneCount() { return (int)bones.size(); }
	const std::string& GetBoneName(int _index) { return bones[_index].name; }
	void CountNode(int& _count, const aiNode* _node);
};

struct asMesh
{
	std::string name;
	aiMesh* mesh = nullptr;
	std::vector<VertexType> vertices;
	std::vector<uint32> indices;

	int32 boneIndex = 0;
	std::string materialName;

	Matrix worldTM;

	Vector3 maxAABB;
	Vector3 minAABB;
};

struct asMaterial
{
	std::string name;
	Color ambient = Color(0.f, 0.f, 0.f, 1.f);
	Color diffuse = Color(1.f, 1.f, 1.f, 1.f);
	Color specular = Color(0.f, 0.f, 0.f, 1.f);
	Color emissive = Color(0.f, 0.f, 0.f, 1.f);
	std::string diffuseFile;
	std::string specularFile;
	std::string normalFile;
};

// Animation
struct asBlendWeight
{
	// ���� ����� 4��
	// index : indices
	// ��Ͽ��� �� ��°����
	// boneIndex : ���� ���� ����Ű����
	// weight : ����ġ (���� �޴� ����)
	void Set(uint32 index, uint32 boneIndex, float weight)
	{
		float w = weight;

		switch (index)
		{
		case 0: indices[0] = boneIndex; weights.x = w; break;
		case 1: indices[1] = boneIndex; weights.y = w; break;
		case 2: indices[2] = boneIndex; weights.z = w; break;
		case 3: indices[3] = boneIndex; weights.w = w; break;
		}
	}

	uint32 indices[4] = { 0, 0, 0, 0 };
	Vector4 weights = Vector4(0.f, 0.f, 0.f, 0.f);
};

// �������� -> (������ȣ, ����ġ) �� ����ִ´�.
// �̰����� �۾��� ������ asBlendWeight struct�� �Ѱ��ش�.
struct asBoneWeights
{
	// ����ġ�� ���� ������� boneWeights�� insert�ȴ�. 
	// ex : 3 7 1 5 -> 7 5 3 1
	//
	// ������ ������ �ִ� bone�� ���� ��, �ش� ������
	// ���� ����� �ִ� bone�� ����ġ�� ���� ���� ������� ���������ν�
	// ������ �������� �� �ڿ������� ���̰� �� �� �ִ�.
	//
	// ���� ¥���� skinning �ڵ�� ��� ����ġ�� ���� transform�� �����ְ� �ֱ⿡ �� �ٸ� ȿ���� ���� ��
	// �̶�� ���������� normalize �ϴ� �κп��� ������ �ִ� bone�� ���� 4 �̻��� ��� ����� 4��
	// �����������ν� �ǹ� �ִٰ� ������
	void AddWeights(uint32 boneIndex, float weight)
	{
		// �̻��� ��
		if (weight <= 0.f)
			return;

		auto findIt = std::find_if(boneWeights.begin(), boneWeights.end(),
			[weight](const Pair& p) {return weight > p.second; });

		boneWeights.insert(findIt, Pair(boneIndex, weight));
	}

	asBlendWeight GetBlendWeights()
	{
		asBlendWeight blendWeights;

		for (uint32 i = 0; i < boneWeights.size(); ++i)
		{
			if (i >= 4)
				break;

			blendWeights.Set(i, boneWeights[i].first, boneWeights[i].second);
		}

		return blendWeights;
	}

	// �������� ����ġ�� �����ؼ� 1�� ����� ���� �Լ�
	// ex (1, 0.3) (2, 0.2) -> (1, 0.6) (2, 0.4)
	void Normalize()
	{
		// ������������ ȿ������ �ϵ������ ���ට����
		// �Ϲ������� ����ġ�� ���� �ִ� 4���� ���ѵȴ�.
		if (boneWeights.size() >= 4)
			boneWeights.resize(4);

		float totalWeight = 0.f;
		for (const auto& item : boneWeights)
			totalWeight += item.second;

		float scale = 1.f / totalWeight;
		for (auto& item : boneWeights)
			item.second *= scale;
	}

	using Pair = std::pair<uint32, float>;
	std::vector<Pair> boneWeights;
};

// Keyframe �� ����ִ� �������� bone transform ������
// Animation -> Keyframe -> KeyframeData�� �̷���� ������ 
// 3�� �迭�� �����ϸ� �ȴ�.
// Animation �� �迭�� ������ ���̰� Animation[0] �ȿ��� frame ������ �������� ����ְ�
// frame[0] ������ ������ �ӿ��� �������� bone transform �����Ͱ� ������� ��
struct asKeyframeData
{
	float time = 0.f;
	Vector3 scale;
	Quaternion rotation;
	Vector3 translation;
};

// �ִϸ��̼��� ����ִ� Keyframe ����
struct asKeyframe
{
	std::string boneName;
	std::vector<asKeyframeData> transforms;
};

// �ִϸ��̼� (idle, run, slash �� ���� �ϳ��� �ִϸ��̼��̴�.)
struct asAnimation
{
	std::string name;
	uint32 frameCount = 0; // �� ������ ����
	float frameRate = 0.f; // �ʴ� ȭ�鿡 ��µǴ� ������ ���� (fps)
	float duration = 0.f;  // �ִϸ��̼��� �� �ð� (����) 
	std::vector<std::shared_ptr<asKeyframe>> keyframes;
};

// Cache
struct asAnimationNode
{
	aiString name;
	std::vector<asKeyframeData> keyframe;
};