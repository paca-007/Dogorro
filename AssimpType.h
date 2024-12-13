#pragma once
#include "pch.h"
#include "VertexType.h"

using VertexType = VertexTextureNormalTangentBlendData;

struct asMesh;

/// <summary>
/// Assimp 를 통해서 얻어낸 정보들을 메모리에 저장해둔다.
/// 이 정보들로 customData 파일을 만든다.
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
	// 뼈의 목록은 4개
	// index : indices
	// 목록에서 몇 번째인지
	// boneIndex : 무슨 뼈를 가리키는지
	// weight : 가중치 (영향 받는 비율)
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

// 정점마다 -> (관절번호, 가중치) 를 들고있는다.
// 이곳에서 작업을 끝내면 asBlendWeight struct로 넘겨준다.
struct asBoneWeights
{
	// 가중치가 높은 순서대로 boneWeights에 insert된다. 
	// ex : 3 7 1 5 -> 7 5 3 1
	//
	// 정점에 영향을 주는 bone이 많을 때, 해당 정점에
	// 가장 영향력 있는 bone을 가중치가 가장 높은 순서대로 적용함으로써
	// 정점의 움직임을 더 자연스럽게 보이게 할 수 있다.
	//
	// 현재 짜여진 skinning 코드는 모든 가중치에 대한 transform을 더해주고 있기에 별 다른 효과는 없을 듯
	// 이라고 생각했지만 normalize 하는 부분에서 영향을 주는 bone의 수가 4 이상일 경우 사이즈를 4로
	// 재조정함으로써 의미 있다고 생각됨
	void AddWeights(uint32 boneIndex, float weight)
	{
		// 이상한 값
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

	// 최종적인 가중치를 보정해서 1로 만들기 위한 함수
	// ex (1, 0.3) (2, 0.2) -> (1, 0.6) (2, 0.4)
	void Normalize()
	{
		// 파이프라인의 효율성과 하드웨어의 제약때문에
		// 일반적으로 가중치는 보통 최대 4개로 제한된다.
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

// Keyframe 이 들고있는 실질적인 bone transform 데이터
// Animation -> Keyframe -> KeyframeData로 이루어져 있으며 
// 3중 배열로 생각하면 된다.
// Animation 은 배열로 관리될 것이고 Animation[0] 안에는 frame 단위의 정보들이 들어있고
// frame[0] 단위의 정보들 속에는 실질적인 bone transform 데이터가 들어있을 것
struct asKeyframeData
{
	float time = 0.f;
	Vector3 scale;
	Quaternion rotation;
	Vector3 translation;
};

// 애니메이션이 들고있는 Keyframe 정보
struct asKeyframe
{
	std::string boneName;
	std::vector<asKeyframeData> transforms;
};

// 애니메이션 (idle, run, slash 등 각각 하나의 애니메이션이다.)
struct asAnimation
{
	std::string name;
	uint32 frameCount = 0; // 총 프레임 개수
	float frameRate = 0.f; // 초당 화면에 출력되는 프레임 개수 (fps)
	float duration = 0.f;  // 애니메이션의 총 시간 (길이) 
	std::vector<std::shared_ptr<asKeyframe>> keyframes;
};

// Cache
struct asAnimationNode
{
	aiString name;
	std::vector<asKeyframeData> keyframe;
};