#include "pch.h"
#include "GraphicsEngine.h"
#include "IGraphicsEngine.h"
#include "Converter.h"
#include <filesystem>
#include "VertexType.h"
#include "FileUtils.h"
#include "Utils.h"
#include "tinyxml2.h"
#include "AssimpType.h"

#include <algorithm>


Converter::Converter(IGraphicsEngine* _gp)
	:gp(dynamic_cast<GraphicsEngine*>(_gp))
{
	this->importer = std::make_shared<Assimp::Importer>();
	importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);
}

Converter::~Converter()
{

}

/// <summary>
/// 에셋 ( fbx, ase obj등)을 읽어서 메모리에 올린다.
/// </summary>
/// <param name="_file">경로</param>
void Converter::ReadAssetFile(std::string _file)
{
	std::string fileStr = this->assetPath + _file;

	auto p = std::filesystem::path(fileStr);
	assert(std::filesystem::exists(p) && "not exsists file path");

	this->scene = this->importer->ReadFile(
		fileStr,
		aiProcess_ConvertToLeftHanded |  // 왼손 좌표계 로드, uv 시작점 좌상단, cw 로드 (시계 방향)
		aiProcess_MakeLeftHanded |
		//aiProcess_FlipUVs |
		//aiProcess_FlipWindingOrder |
		//aiProcess_JoinIdenticalVertices | // 동일한 위치의 정점을 하나의 정점으로 병합 (정점 데이터의 중복을 제거하고 모델 최적화)
		//aiProcess_PreTransformVertices | // 계층구조와 애니메이션 정보를 제거하고 정점 정보를 구성
		aiProcess_Triangulate | // 삼각형 메쉬 형태로 구성
		aiProcess_GenUVCoords | // UV 정보 생성
		aiProcess_GenNormals | // Normal 정보 생성
		aiProcess_CalcTangentSpace  // TangentSpace 정보 생성 
	);

	assert(this->scene && "cannot read file");
}

/// <summary>
/// 에세스이 정보를 커스텀 데이터 형식으로 바꾼다.
/// </summary>
/// <param name="_savePath">저장 될 경로</param>
void Converter::ExportModelData(std::string _savePath)
{
	std::string finalPath = this->modelPath + _savePath + ".mesh";

	// this->skel.ReadFromAssimp(this->scene);
	this->scene->mMeshes[0]->mAABB;
	ReadModelData(this->scene->mRootNode, 0, -1);
	ReadSkinData();

	WriteCSVFile(_savePath);
	WriteModelFile(finalPath);
}

/// <summary>
/// 메테리얼 데이터 로드
/// </summary>
/// <param name="_savePath">저장 경로</param>
void Converter::ExportMaterialData(std::string _savePath)
{
	std::string finalPath = this->texturePath + _savePath + ".xml";

	ReadMaterialData();
	WriteMaterialData(finalPath);
}

/// <summary>
/// animation 데이터를 추출해서 바이너리 형식으로 저장
/// </summary>
/// <param name="_savePath">저장 경로</param>
/// <param name="_index">인덱스</param>
void Converter::ExportAnimationData(std::string _savePath, uint32 _index /*= 0*/)
{
	std::string finalPath = this->modelPath + _savePath;
	// assert(_index < this->scene->mNumAnimations && "no animation exist");

	for (uint32 i = 0; i < this->scene->mNumAnimations; i++)
	{
		std::shared_ptr<asAnimation> animation = ReadAnimationData(this->scene->mAnimations[i]);
		animations.push_back(animation);
	}
	WriteAnimationData(finalPath + ".clip");
	WriteAnimationNameData(finalPath + ".name");
}

/// <summary>
/// fbx 데이터를 이용한 맵을 추출하는 함수
/// 매쉬의 이름 = 게임의 클래스
/// 좀 더 다양한 방법으로 가공하면 애니메이션 데이터나
/// 더 다양한 데이터를 가져와서 
/// 진짜 맵 툴 엔진으로 사용할 수 있지 않을까?
/// 물론 자체 엔진 툴이 있으면 더 좋겠지만
/// </summary>
/// made by hedwig
/// <param name="_file">경로</param>
void Converter::ExportMapData(std::string _file)
{
	ExportMaterialData(_file);
	std::string finalPath = this->modelPath + _file + ".map";
	std::filesystem::path p = this->modelPath + _file + ".map";


	ReadMapData(this->scene->mRootNode, 0, -1);

	MappingMatrixAtMesh();

	WriteMapCSVFile(finalPath);
	WriteMapFile(finalPath);
}

void Converter::ConvertAllAesset()
{

}

/// <summary>
/// 계층 구조인 모델 데이터를 읽는다.
/// </summary>
/// <param name="_node">현 노드</param>
/// <param name="_index">현 본의 인덱스</param>
/// <param name="_parent">부모 본의 인덱스</param>
void Converter::ReadModelData(aiNode* _node, int32 _index, int32 _parent)
{
	// 일단 노드를 전부 본 이라고 가정한다.
	std::shared_ptr<asBone> bone = std::make_shared<asBone>();
	bone->index = _index;
	bone->parentIndex = _parent;
	bone->name = _node->mName.C_Str();

	// 부모 기준 행렬
	Matrix transform(_node->mTransformation[0]);

	// DX는 행 우선, FBX, ASE는 열 우선
	bone->Set(_node);
	_node->mMetaData;

	// 부모 행렬 가져오기
	// 부모가 없다면 단위행렬
// 	Matrix matParent = Matrix::Identity;
// 		if (_parent >= 0)
// 		{
// 			matParent = this->bones[_parent]->relativeTransform;
// 		}
// 	
// 		// 부모 기준의 행렬을 루트 기준의 행렬로 바꿔준다.
// 		// bone->relativeTransform = bone->relativeTransform * matParent;

	this->bones.push_back(bone);

	// Mesh 정보 읽기
	ReadMeshData(_node, _index);

	// 재귀
	for (uint32 i = 0; i < _node->mNumChildren; i++)
	{
		ReadModelData(_node->mChildren[i], static_cast<uint32>(this->bones.size()), _index);
	}
}

/// <summary>
/// 매쉬 데이터 읽기
/// </summary>
/// <param name="_node">노드</param>
/// <param name="_bone">본</param>
void Converter::ReadMeshData(aiNode* _node, int32 _bone)
{
	// 매쉬가 없는 노드의 경우
	if (_node->mNumMeshes < 1)
	{
		return;
	}

	// 매쉬가 여러개 일 수도 있다.
	for (uint32 i = 0; i < _node->mNumMeshes; i++)
	{
		std::shared_ptr<asMesh> mesh = std::make_shared<asMesh>();

		mesh->name = _node->mName.C_Str();
		mesh->boneIndex = _bone;

		// scene 에서 매핑되는 매쉬의 인덱스
		uint32 index = _node->mMeshes[i];
		const aiMesh* srcMesh = this->scene->mMeshes[index];

		// 한 물체에 여려 매쉬가 있을 수 있다.
		// 그 매쉬마다 다른 매테리얼을 사용 할 수 있다.
		// 그 때 사용할 매핑 정보
		const aiMaterial* material = this->scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str();
		mesh->maxAABB = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		mesh->minAABB = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		// 정점 순회
		for (uint32 v = 0; v < srcMesh->mNumVertices; v++)
		{
			VertexType vertex;

			/// Position
			::memcpy(&vertex.position, &srcMesh->mVertices[v], sizeof(Vector3));

			mesh->maxAABB.x = max(mesh->maxAABB.x, srcMesh->mVertices[v].x);
			mesh->maxAABB.y = max(mesh->maxAABB.y, srcMesh->mVertices[v].y);
			mesh->maxAABB.z = max(mesh->maxAABB.z, srcMesh->mVertices[v].z);

			mesh->minAABB.x = (((mesh->maxAABB.x) < (srcMesh->mVertices[v].x)) ? (mesh->maxAABB.x) : (srcMesh->mVertices[v].x));
			mesh->minAABB.y = (((mesh->maxAABB.y) < (srcMesh->mVertices[v].y)) ? (mesh->maxAABB.y) : (srcMesh->mVertices[v].y));
			mesh->minAABB.z = (((mesh->maxAABB.z) < (srcMesh->mVertices[v].z)) ? (mesh->maxAABB.z) : (srcMesh->mVertices[v].z));

			/// UV
			// 만일 UV 정보가 있다면 받아온다.
			if (srcMesh->HasTextureCoords(0))
			{
				::memcpy(&vertex.uv, &srcMesh->mTextureCoords[0][v], sizeof(Vector2));
			}

			/// Normal
			if (srcMesh->HasNormals())
			{
				::memcpy(&vertex.normal, &srcMesh->mNormals[v], sizeof(Vector3));
			}

			/// Tangent & BiTangent
			if (srcMesh->HasTangentsAndBitangents())
			{
				::memcpy(&vertex.tangent, &srcMesh->mTangents[v], sizeof(Vector3));
				::memcpy(&vertex.bitangent, &srcMesh->mBitangents[v], sizeof(Vector3));
			}

			mesh->vertices.push_back(vertex);
		}

		/// index
		for (uint32 f = 0; f < srcMesh->mNumFaces; f++)
		{
			aiFace& face = srcMesh->mFaces[f];

			for (uint32 k = 0; k < face.mNumIndices; k++)
			{
				mesh->indices.push_back(face.mIndices[k]);
			}
		}

		this->mapData[static_cast<int>(this->meshes.size())] = std::vector<Matrix>();
		this->mapData[static_cast<int>(this->meshes.size())].push_back(bones[_bone]->relativeTransform);
		this->mapMeshData[mesh->name] = static_cast<int>(this->meshes.size());
		this->meshes.push_back(mesh);
	}
}

/// <summary>
/// 정점마다 본의 가중치를 읽어온다.
/// </summary>
void Converter::ReadSkinData()
{
	// 모든 매쉬 순회
	for (uint32 i = 0; i < this->scene->mNumMeshes; i++)
	{
		aiMesh* srcMesh = this->scene->mMeshes[i];

		// 본이 없다면 
		if (!srcMesh->HasBones())
		{
			continue;
		}

		std::shared_ptr<asMesh> mesh = this->meshes[i];

		// 본 번호와 가중치를 임시 저장할 벡터
		std::vector<asBoneWeights> tempData;
		tempData.resize(mesh->vertices.size());

		for (uint32 b = 0; b < srcMesh->mNumBones; b++)
		{
			aiBone* srcMeshBone = srcMesh->mBones[b];

			uint32 boneIndex = GetBoneIndex(srcMeshBone->mName.C_Str());
			this->bones[boneIndex]->offsetMatrix = Matrix(srcMeshBone->mOffsetMatrix[0]).Transpose();
			for (uint32 w = 0; w < srcMeshBone->mNumWeights; w++)
			{
				// 해당 본에 영향을 받는 정점 번호
				uint32 index = srcMeshBone->mWeights[w].mVertexId;

				float weight = srcMeshBone->mWeights[w].mWeight;

				tempData[index].AddWeights(boneIndex, weight);
			}
		}

		// 결과 계산
		for (uint32 v = 0; v < tempData.size(); v++)
		{
			tempData[v].Normalize();
			asBlendWeight blendWeight = tempData[v].GetBlendWeights();
			mesh->vertices[v].blendIndices[0] = blendWeight.indices[0];
			mesh->vertices[v].blendIndices[1] = blendWeight.indices[1];
			mesh->vertices[v].blendIndices[2] = blendWeight.indices[2];
			mesh->vertices[v].blendIndices[3] = blendWeight.indices[3];

			mesh->vertices[v].blendWeights = blendWeight.weights;
		}
	}
}

/// <summary>
/// 정점 정보를 저장하는 파일
/// 텍스트 파일 형식으로 데이터를 저장하여 간편하게 디버깅 가능
/// </summary>
void Converter::WriteCSVFile(std::string _savePath)
{
	// csv 파일 경로
	size_t lastSlashPos = _savePath.find_last_of(L'/');
	std::string fileName = _savePath.substr(lastSlashPos + 1);
	std::string csvFilePath = "../AssimpData/PersonalSpace/" + fileName + ".csv";

	FILE* file;
	::fopen_s(&file, csvFilePath.c_str(), "w");

	if (file == NULL)
		assert(false && "CSV 파일 생성 오류");

	for (std::shared_ptr<asBone>& bone : this->bones)
	{
		std::string name = bone->name;
		::fprintf(file, "%d,%s\n", bone->index, bone->name.c_str());

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				::fprintf(file, "%f", bone->relativeTransform.m[i][j]);
				if (j != 3)
				{
					::fprintf(file, ",");

				}
			}
			::fprintf(file, "\n");
		}
		::fprintf(file, "\n");

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				::fprintf(file, "%f", bone->offsetMatrix.m[i][j]);
				if (j != 3)
				{
					::fprintf(file, ",");

				}
			}
			::fprintf(file, "\n");
		}
	}

	::fprintf(file, "\n\n");
	for (std::shared_ptr<asMesh>& mesh : this->meshes)
	{
		std::string name = mesh->name;
		::fprintf(file, "%s\n", name.c_str());
		::fprintf(file, "%s\n", mesh->materialName.c_str());
		for (UINT i = 0; i < mesh->vertices.size(); i++)
		{
			Vector3 p = mesh->vertices[i].position;
			uint32 indices[4];
			indices[0] = mesh->vertices[i].blendIndices[0];
			indices[1] = mesh->vertices[i].blendIndices[1];
			indices[2] = mesh->vertices[i].blendIndices[2];
			indices[3] = mesh->vertices[i].blendIndices[3];

			Vector4 weights = mesh->vertices[i].blendWeights;

			::fprintf(file, "%f,%f,%f,", p.x, p.y, p.z);
			::fprintf(file, "%d,%d,%d,%d,", indices[0], indices[1], indices[2], indices[3]);
			::fprintf(file, "%f,%f,%f,%f\n", weights.x, weights.y, weights.z, weights.w);
		}

		::fprintf(file, "\n\n");
	}

	::fclose(file);
}

/// <summary>
/// 추출한 데이터로 바이너리 형태 파일을 만든다.
/// (데이터를 납작하게 만들기)
/// </summary>
void Converter::WriteModelFile(std::string _finalPath)
{
	auto path = std::filesystem::path(_finalPath);

	// 폴더가 없으면 만든다.
	std::filesystem::create_directory(path.parent_path());

	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(Utils::ToWString(_finalPath), FileMode::Write);

	Matrix rtm = Matrix(this->scene->mRootNode->mTransformation[0]);

	/// Bone Data
	file->Write<uint32>(static_cast<uint32>(this->bones.size()));
	for (std::shared_ptr<asBone>& bone : this->bones)
	{
		file->Write<int32>(bone->index);
		file->Write<string>(bone->name);
		file->Write<int32>(bone->parentIndex);
		file->Write<Matrix>(bone->relativeTransform);
		file->Write<Matrix>(bone->offsetMatrix);
	}

	/// Mesh Data
	file->Write<uint32>(static_cast<uint32>(this->meshes.size()));
	for (std::shared_ptr<asMesh>& meshData : this->meshes)
	{
		file->Write<string>(meshData->name);
		file->Write<int32>(meshData->boneIndex);
		file->Write<string>(meshData->materialName);

		/// Vertex Data
		file->Write<uint32>(static_cast<uint32>(meshData->vertices.size()));
		file->Write(&meshData->vertices[0], static_cast<uint32>(sizeof(VertexType) * meshData->vertices.size()));

		/// Index Data
		file->Write<uint32>(static_cast<uint32>(meshData->indices.size()));
		file->Write(&meshData->indices[0], static_cast<uint32>(sizeof(uint32) * meshData->indices.size()));

		file->Write<Vector3>(meshData->maxAABB);
		file->Write<Vector3>(meshData->minAABB);
	}


	file->Close();
}

/// <summary>
/// 맵 데이터 파일 읽기
/// </summary>
/// <param name="_node">시작 노드</param>
void Converter::ReadMapData(aiNode* _node, int32 _index, int32 _parent)
{
	// 현 위치를 기억하기 위한 데이터를 저장 하는 본
	std::shared_ptr<asBone> bone = std::make_shared<asBone>();
	bone->index = _index;
	bone->parentIndex = _parent;
	bone->name = _node->mName.C_Str();

	// DX는 행 우선, FBX, ASE는 열 우선
	bone->Set(_node);

	// 부모 행렬 가져오기
	// 부모가 없다면 단위행렬
	Matrix matParent = Matrix::Identity;
	if (_parent >= 0)
	{
		matParent = this->bones[_parent]->relativeTransform;
	}

	// 부모 기준의 행렬을 루트 기준의 행렬로 바꿔준다.
	bone->relativeTransform = bone->relativeTransform * matParent;

	this->bones.push_back(bone);

	// Mesh 정보 읽기
	ReadMapModelData(_node, _index);

	// 재귀
	for (uint32 i = 0; i < _node->mNumChildren; i++)
	{
		ReadMapData(_node->mChildren[i], static_cast<uint32>(this->bones.size()), _index);
	}
}

void Converter::ReadMapModelData(aiNode* _node, int32 _bone)
{
	// 매쉬가 없는 노드의 경우
	if (_node->mNumMeshes < 1)
	{
		return;
	}

	// 매쉬가 여러개 일 수도 있다.
	for (uint32 i = 0; i < _node->mNumMeshes; i++)
	{
		std::shared_ptr<asMesh> mesh = std::make_shared<asMesh>();

		std::string tempName = _node->mName.C_Str();
// 		int pointPos = (int)(tempName.find_last_of('.'));
// 		if (pointPos != -1)
// 		{
// 			tempName = tempName.substr(0, pointPos);
// 		}
// 
// 		if (this->mapMeshData.find(tempName) != this->mapMeshData.end())
// 		{
// 			this->mapData[this->mapMeshData[tempName]].push_back(bones[_bone]->relativeTransform);
// 			continue;
// 		}

		mesh->name = tempName;

		// scene 에서 매핑되는 매쉬의 인덱스
		uint32 index = _node->mMeshes[i];
		const aiMesh* srcMesh = this->scene->mMeshes[index];

		// 한 물체에 여려 매쉬가 있을 수 있다.
		// 그 매쉬마다 다른 매테리얼을 사용 할 수 있다.
		// 그 때 사용할 매핑 정보
		const aiMaterial* material = this->scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str();
		mesh->maxAABB = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		mesh->minAABB = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		// 정점 순회
		for (uint32 v = 0; v < srcMesh->mNumVertices; v++)
		{
			VertexType vertex;

			/// Position
			::memcpy(&vertex.position, &srcMesh->mVertices[v], sizeof(Vector3));

			mesh->maxAABB.x = max(mesh->maxAABB.x, srcMesh->mVertices[v].x);
			mesh->maxAABB.y = max(mesh->maxAABB.y, srcMesh->mVertices[v].y);
			mesh->maxAABB.z = max(mesh->maxAABB.z, srcMesh->mVertices[v].z);

			mesh->minAABB.x = std::min(mesh->minAABB.x, srcMesh->mVertices[v].x);
			mesh->minAABB.y = std::min(mesh->minAABB.y, srcMesh->mVertices[v].y);
			mesh->minAABB.z = std::min(mesh->minAABB.z, srcMesh->mVertices[v].z);


			/// UV
			// 만일 UV 정보가 있다면 받아온다.
			if (srcMesh->HasTextureCoords(0))
			{
				::memcpy(&vertex.uv, &srcMesh->mTextureCoords[0][v], sizeof(Vector2));
			}

			/// Normal
			if (srcMesh->HasNormals())
			{
				::memcpy(&vertex.normal, &srcMesh->mNormals[v], sizeof(Vector3));
			}

			/// Tangent & BiTangent
			if (srcMesh->HasTangentsAndBitangents())
			{
				::memcpy(&vertex.tangent, &srcMesh->mTangents[v], sizeof(Vector3));
				::memcpy(&vertex.bitangent, &srcMesh->mBitangents[v], sizeof(Vector3));
			}

			mesh->vertices.push_back(vertex);
		}

		/// index
		for (uint32 f = 0; f < srcMesh->mNumFaces; f++)
		{
			aiFace& face = srcMesh->mFaces[f];

			for (uint32 k = 0; k < face.mNumIndices; k++)
			{
				mesh->indices.push_back(face.mIndices[k]);
			}
		}

		this->mapData[static_cast<int>(this->meshes.size())] = std::vector<Matrix>();
		this->mapData[static_cast<int>(this->meshes.size())].push_back(bones[_bone]->relativeTransform);
		this->mapMeshData[mesh->name] = static_cast<int>(this->meshes.size());
		this->meshes.push_back(mesh);
	}
}

void Converter::MappingMatrixAtMesh()
{
	for (auto& m : this->meshes)
	{
		m->worldTM = this->bones[m->boneIndex]->relativeTransform;
	}
}

/// <summary>
/// 맵 파일 데이터 생성
/// </summary>
/// <param name="_savePath">경로</param>
void Converter::WriteMapFile(std::string _savePath)
{
	_savePath += ".dir/";
	auto pathe = std::filesystem::path(_savePath + "map.data");
	std::filesystem::create_directory(pathe.parent_path());
	/// Mesh
	for (auto& m : this->meshes)
	{
		auto path = std::filesystem::path(_savePath + "meshes/" + m->name + ".mesh");
		std::filesystem::create_directory(path.parent_path());

		std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
		file->Open(Utils::ToWString(_savePath + "meshes/" + m->name + ".mesh"), FileMode::Write);

		file->Write<uint32>(1);

		file->Write<string>(m->name);
		file->Write<int32>(m->boneIndex);
		file->Write<string>(m->materialName);

		/// Vertex Data
		file->Write<uint32>(static_cast<uint32>(m->vertices.size()));
		file->Write(&m->vertices[0], static_cast<uint32>(sizeof(VertexType) * m->vertices.size()));

		/// Index Data
		file->Write<uint32>(static_cast<uint32>(m->indices.size()));
		file->Write(&m->indices[0], static_cast<uint32>(sizeof(uint32) * m->indices.size()));

		file->Write<Vector3>(m->maxAABB);
		file->Write<Vector3>(m->minAABB);

		file->Close();
	}

	/// Material

	WriteMaterialMapFile(_savePath + "materials/");


	/// MapData
	auto path = std::filesystem::path(_savePath + "map.data");
	std::filesystem::create_directory(path.parent_path());
	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(Utils::ToWString(_savePath + "map.data"), FileMode::Write);

	file->Write<uint32>(static_cast<uint32>(this->mapData.size()));
	for (auto& d : this->mapData)
	{
		file->Write<uint32>(d.first);
		file->Write<std::string>(this->meshes[d.first]->name);
		file->Write<uint32>(static_cast<uint32>(d.second.size()));
		for (size_t i = 0; i < d.second.size(); i++)
		{
			file->Write<Matrix>(d.second[i]);
		}
	}
}

void Converter::WriteMaterialMapFile(std::string finalPath)
{
	for (std::shared_ptr<asMaterial> material : this->materials)
	{
		auto path = std::filesystem::path(finalPath);

		// 폴더가 없으면 만든다. (House/House.xml 에서 상위 폴더인 House 를 만듬) 
		std::filesystem::create_directory(path.parent_path());

		string folder = path.parent_path().string();

		// 문서 생성
		std::shared_ptr<tinyxml2::XMLDocument> document = std::make_shared<tinyxml2::XMLDocument>();

		// xml 포맷으로 읽겠다고 선언
		tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
		document->LinkEndChild(decl);

		// <Materials>
		// 
		// </Materials>
		tinyxml2::XMLElement* root = document->NewElement("Materials");
		document->LinkEndChild(root);

		tinyxml2::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);

		tinyxml2::XMLElement* element = nullptr;

		element = document->NewElement("Name");
		element->SetText(material->name.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("DiffuseFile");
		element->SetText(WriteTexture(folder, material->diffuseFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		element->SetText(WriteTexture(folder, material->specularFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		element->SetText(WriteTexture(folder, material->normalFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("Ambient");
		element->SetAttribute("R", material->ambient.x);
		element->SetAttribute("G", material->ambient.y);
		element->SetAttribute("B", material->ambient.z);
		element->SetAttribute("A", material->ambient.w);
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		element->SetAttribute("R", material->diffuse.x);
		element->SetAttribute("G", material->diffuse.y);
		element->SetAttribute("B", material->diffuse.z);
		element->SetAttribute("A", material->diffuse.w);
		node->LinkEndChild(element);

		element = document->NewElement("Specular");
		element->SetAttribute("R", material->specular.x);
		element->SetAttribute("G", material->specular.y);
		element->SetAttribute("B", material->specular.z);
		element->SetAttribute("A", material->specular.w);
		node->LinkEndChild(element);

		element = document->NewElement("Emissive");
		element->SetAttribute("R", material->emissive.x);
		element->SetAttribute("G", material->emissive.y);
		element->SetAttribute("B", material->emissive.z);
		element->SetAttribute("A", material->emissive.w);
		node->LinkEndChild(element);
		document->SaveFile((finalPath + material->name + ".xml").c_str());
	}
}

/// <summary>
/// 맵 파일 디버깅 데이터 생성
/// </summary>
/// <param name="_savePath">경로</param>
void Converter::WriteMapCSVFile(std::string _savePath)
{
	// csv 파일 경로
	size_t lastSlashPos = _savePath.find_last_of(L'/');
	std::string fileName = _savePath.substr(lastSlashPos + 1);
	std::string csvFilePath = "../AssimpData/PersonalSpace/" + fileName + ".csv";

	FILE* file;
	::fopen_s(&file, csvFilePath.c_str(), "w");

	if (file == NULL)
		assert(false && "CSV 파일 생성 오류");

	::fprintf(file, "\n\n");
	for (std::shared_ptr<asMesh>& mesh : this->meshes)
	{
		std::string name = mesh->name;
		::fprintf(file, "%s\n", name.c_str());
		::fprintf(file, "%s\n", mesh->materialName.c_str());
		for (UINT i = 0; i < mesh->vertices.size(); i++)
		{
			Vector3 p = mesh->vertices[i].position;
			::fprintf(file, "%f,%f,%f,\n", p.x, p.y, p.z);
		}

		::fprintf(file, "\n\n");
	}

	::fclose(file);
}

/// <summary>
/// Scene 에서 들고있는 MaterialData를 읽어들이고 메모리에 저장한다.
/// </summary>
void Converter::ReadMaterialData()
{
	for (uint32 i = 0; i < this->scene->mNumMaterials; ++i)
	{
		// Assimp 가 로드한 원본 머테리얼 데이터
		aiMaterial* srcMaterial = this->scene->mMaterials[i];

		std::shared_ptr<asMaterial> material = std::make_shared<asMaterial>();
		material->name = srcMaterial->GetName().C_Str();


		aiColor3D color;

		// Ambient
		srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material->ambient = Color(color.r, color.g, color.b, 1.f);

		// Diffuse
		srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->diffuse = Color(color.r, color.g, color.b, 1.f);

		// Specular
		srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material->specular = Color(color.r, color.g, color.b, 1.f);
		srcMaterial->Get(AI_MATKEY_SHININESS, material->specular.w);

		// Emissive
		srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		material->emissive = Color(color.r, color.g, color.b, 1.f);

		srcMaterial->mProperties[0];

		aiString file;
		
		// Diffuse Texture
		srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
		material->diffuseFile = file.C_Str();
		std::wstring temp = Utils::ToWString(material->diffuseFile);
		// Specular Texture
		srcMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
		material->specularFile = file.C_Str();

		// Normal Texture
		srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
		material->normalFile = file.C_Str();

		this->materials.push_back(material);
	}
}

/// <summary>
/// 추출한 데이터로 xml 문서를 만든다.
/// </summary>
void Converter::WriteMaterialData(std::string finalPath)
{
	auto path = std::filesystem::path(finalPath);

	// 폴더가 없으면 만든다. (House/House.xml 에서 상위 폴더인 House 를 만듬) 
	std::filesystem::create_directory(path.parent_path());

	string folder = path.parent_path().string();
	std::wstring wfolder = path.parent_path().wstring();


	// 문서 생성
	std::shared_ptr<tinyxml2::XMLDocument> document = std::make_shared<tinyxml2::XMLDocument>();

	// xml 포맷으로 읽겠다고 선언
	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	// <Materials>
	// 
	// </Materials>
	tinyxml2::XMLElement* root = document->NewElement("Materials");
	document->LinkEndChild(root);


	// <Materials>
	//		<Material>
	//			<> ~~ </>
	//			...
	// 			...
	//		</Material>
	// </Materials>
	for (std::shared_ptr<asMaterial> material : this->materials)
	{
		tinyxml2::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);

		tinyxml2::XMLElement* element = nullptr;

		element = document->NewElement("Name");
		element->SetText(material->name.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("DiffuseFile");
		element->SetText(WriteTexture(folder, material->diffuseFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		element->SetText(WriteTexture(folder, material->specularFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		element->SetText(WriteTexture(folder, material->normalFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("Ambient");
		element->SetAttribute("R", material->ambient.x);
		element->SetAttribute("G", material->ambient.y);
		element->SetAttribute("B", material->ambient.z);
		element->SetAttribute("A", material->ambient.w);
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		element->SetAttribute("R", material->diffuse.x);
		element->SetAttribute("G", material->diffuse.y);
		element->SetAttribute("B", material->diffuse.z);
		element->SetAttribute("A", material->diffuse.w);
		node->LinkEndChild(element);

		element = document->NewElement("Specular");
		element->SetAttribute("R", material->specular.x);
		element->SetAttribute("G", material->specular.y);
		element->SetAttribute("B", material->specular.z);
		element->SetAttribute("A", material->specular.w);
		node->LinkEndChild(element);

		element = document->NewElement("Emissive");
		element->SetAttribute("R", material->emissive.x);
		element->SetAttribute("G", material->emissive.y);
		element->SetAttribute("B", material->emissive.z);
		element->SetAttribute("A", material->emissive.w);
		node->LinkEndChild(element);
	}

	document->SaveFile(finalPath.c_str());
}


/// <summary>
/// 텍스쳐가 다운로드 받은 FBX 파일 자체에 있다면 내용을 추출하고,
/// 텍스쳐를 따로 받았다면 특정 위치로 복사 생성해준다.
/// 
/// 프로젝트마다 경로에 따라서 수정이 필요.
/// </summary>
std::string Converter::WriteTexture(string saveFolder, string file)
{
	string fileName = std::filesystem::path(file).filename().string();
	string folderName = std::filesystem::path(saveFolder).filename().string();

	// FBX 파일에 텍스쳐가 포함이 되서 들어가 있는 경우가 있다.
	// 일반적으로는 따로 빠져있는 경우가 많고 그렇게 관리하는 걸 추천하는 듯?
	const aiTexture* srcTexture = this->scene->GetEmbeddedTexture(file.c_str());
	if (srcTexture)
	{
		string pathStr = (std::filesystem::path(saveFolder) / fileName).string();

		if (srcTexture->mHeight == 0)
		{
			std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
			file->Open(Utils::ToWString(pathStr), FileMode::Write);
			file->Write(srcTexture->pcData, srcTexture->mWidth);
		}
		else
		{
			D3D11_TEXTURE2D_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
			desc.Width = srcTexture->mWidth;
			desc.Height = srcTexture->mHeight;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA subResource = { 0 };
			subResource.pSysMem = srcTexture->pcData;

			ComPtr<ID3D11Texture2D> texture;
			HRESULT hr = gp->GetDevice()->CreateTexture2D(&desc, &subResource, texture.GetAddressOf());

			DirectX::ScratchImage img;
			::CaptureTexture(gp->GetDevice().Get(), gp->GetDeviceContext().Get(), texture.Get(), img);
			assert(SUCCEEDED(hr) && "cannot create texture data at load assimp texture");
			// Save To File
			hr = DirectX::SaveToDDSFile(*img.GetImages(), DirectX::DDS_FLAGS_NONE, Utils::ToWString(fileName).c_str());
		}
	}

	// 텍스쳐를 따로 받았다면 새로운 경로로 복사생성
	else
	{
		// 완성된 하나의 원본 경로
		string originStr = (std::filesystem::path(this->assetPath) / folderName / file).string();
		Utils::Replace(originStr, "\\", "/");

		// 복사해서 저장되는 파일 경로
		string pathStr = (std::filesystem::path(saveFolder) / fileName).string();
		Utils::Replace(pathStr, "\\", "/");

		// 파일 복사
		::CopyFileA(originStr.c_str(), pathStr.c_str(), false);
	}

	return fileName;
}

/// <summary>
/// Animation Data 를 읽어내고, ReadKeyframeData 를 재귀적으로 호출하여
/// 모든 Keyframe data 를 얻어낸다.
/// </summary>
std::shared_ptr<asAnimation> Converter::ReadAnimationData(aiAnimation* srcAnimation)
{
	std::shared_ptr<asAnimation> animation = std::make_shared<asAnimation>();
	animation->name = srcAnimation->mName.C_Str();
	animation->frameRate = static_cast<float>(srcAnimation->mTicksPerSecond);

	// 애니메이션 데이터를 프레임으로 분할할 때, 프레임당 1개의 데이터를 가지게 된다.
	// 이 애니메이션을 1초 단위로 나누고자 할 때, 1초를 더해서 소수점 단위를 없애준다.
	// ex : mDuration 값이 3.5 라면 3.5초 동안의 애니메이션을 나타내는 것 -> 1초를 더해서 4.5초로 만든다음 0.5초를 날려서 4초를 저장시킨다.
	animation->frameCount = static_cast<uint32>(srcAnimation->mDuration) + 1;
	animation->duration = static_cast<float>(srcAnimation->mDuration);

	std::map<string, std::shared_ptr<asAnimationNode>> cacheAnimNodes;

	// 애니메이션이 들고있는 bone 채널
	for (uint32 i = 0; i < srcAnimation->mNumChannels; ++i)
	{
		aiNodeAnim* srcNode = srcAnimation->mChannels[i];

		// 애니메이션 노드 데이터 파싱 (bone 하나의 매 초마다 들고 있는 transform 값이 들어있는 node)
		std::shared_ptr<asAnimationNode> node = ParseAnimationNode(animation, srcNode);

		// 현재 찾은 노드 중에 제일 긴 시간으로 애니메이션 시간 갱신 (모든 bone 중에서 가장 긴 time 값으로 애니메이션 길이를 설정) 
		animation->duration = max(animation->duration, node->keyframe.back().time);

		cacheAnimNodes[srcNode->mNodeName.C_Str()] = node;
	}

	ReadKeyframeData(animation, this->scene->mRootNode, cacheAnimNodes);

	return animation;
}

/// <summary>
/// Bone 노드 하나의 데이터를 파싱한다.
/// time 값에 해당하는 transform 정보를 들고 있다.
/// </summary>
std::shared_ptr<asAnimationNode> Converter::ParseAnimationNode(std::shared_ptr<asAnimation> animation, aiNodeAnim* srcNode)
{
	std::shared_ptr<asAnimationNode> node = std::make_shared<asAnimationNode>();
	node->name = srcNode->mNodeName;

	// 가장 많은 키프레임 개수를 가져온다 (for 문으로 데이터를 얻어오기 위함)
	uint32 keyCount = max(max(srcNode->mNumPositionKeys, srcNode->mNumScalingKeys), srcNode->mNumRotationKeys);

	// 각 키프레임에 대해서 position, scale, rotation 데이터를 저장 
	for (uint32 k = 0; k < keyCount; ++k)
	{
		asKeyframeData frameData;

		bool found = false;

		// 현재까지 처리된 키프레임의 수
		uint32 t = static_cast<uint32>(node->keyframe.size());
		while (static_cast<float>(srcNode->mPositionKeys[k].mTime) > static_cast<float>(t))
		{
			node->keyframe.push_back(node->keyframe.back());
			t = static_cast<uint32>(node->keyframe.size());
		}
		// 애니메이션 데이터는 연속적이지 않을 수 있으며, 시간 값이 중복되거나 누락된 경우가 발생할 수 있다.
		// 무결성과 정확성을 보장하기 위해 조건문 추가 
		// 
		// Position  
		if (::fabsf(static_cast<float>(srcNode->mPositionKeys[k].mTime) - static_cast<float>(t)) <= 0.0001f)
		{
			aiVectorKey key = srcNode->mPositionKeys[k];
			frameData.time = static_cast<float>(key.mTime);
			::memcpy_s(&frameData.translation, sizeof(Vector3), &key.mValue, sizeof(aiVector3D));

			found = true;
		}

		// Rotation
		if (::fabsf(static_cast<float>(srcNode->mRotationKeys[k].mTime) - static_cast<float>(t)) <= 0.0001f)
		{
			aiQuatKey key = srcNode->mRotationKeys[k];
			frameData.time = static_cast<float>(key.mTime);

			frameData.rotation.x = key.mValue.x;
			frameData.rotation.y = key.mValue.y;
			frameData.rotation.z = key.mValue.z;
			frameData.rotation.w = key.mValue.w;

			found = true;
		}

		// Scale
		if (::fabsf(static_cast<float>(srcNode->mScalingKeys[k].mTime) - static_cast<float>(t)) <= 0.0001f)
		{
			aiVectorKey key = srcNode->mScalingKeys[k];
			frameData.time = static_cast<float>(key.mTime);
			::memcpy_s(&frameData.scale, sizeof(Vector3), &key.mValue, sizeof(aiVector3D));

			found = true;
		}

		if (found == true)
		{
			node->keyframe.push_back(frameData);
		}
	}

	// Keyframe 늘려주기
	// 키프레임 데이터(keyCount)의 수를 기반으로 루프를 돌더라도, 이것만으로 애니메이션의 총 프레임 수를 보장하지 않는다.
	// 애니메이션 총 프레임 수보다 keyframe 이 적다면 마지막 keyframe 을 복사해서 채워넣어준다.
	if (node->keyframe.size() < animation->frameCount)
	{
		uint32 count = animation->frameCount - static_cast<uint32>(node->keyframe.size());
		asKeyframeData keyFrame = node->keyframe.back();

		for (uint32 n = 0; n < count; ++n)
			node->keyframe.push_back(keyFrame);
	}

	return node;
}

/// <summary
/// 앞서 ParseAnimationNode 함수를 사용하여 모든 bone 의 data 를 뽑아서 cache 해두었고
/// 이 정보를 바탕으로 Keyframe 을 생성하고 KeyframeData 를 집어넣는다.
/// fineNode 를 활용하여 캐시된 데이터가 없을 때에만 다시 데이터를 생성하고,
/// 캐시된 데이터가 있는 경우에만 그 데이터를 사용한다.
/// 즉 중복된 데이터 생성을 방지한다. 
/// 재귀적으로 호출하여 트리구조 형태의 모든 노드 데이터를 분석한다.
/// </summary>
void Converter::ReadKeyframeData(std::shared_ptr<asAnimation> animation, aiNode* srcNode, std::map<string, std::shared_ptr<asAnimationNode>>& cache)
{
	std::shared_ptr<asKeyframe> keyframe = std::make_shared<asKeyframe>();
	keyframe->boneName = srcNode->mName.C_Str();

	// 이미 cache 된 노드인지 검색
	std::shared_ptr<asAnimationNode> findNode = cache[srcNode->mName.C_Str()];

	// 애니메이션의 총 frame 만큼 데이터를 복사한다.
	for (uint32 i = 0; i < animation->frameCount; i++)
	{
		asKeyframeData frameData;

		if (findNode == nullptr)
		{
			Matrix transform(srcNode->mTransformation[0]);
			transform = transform.Transpose();
			frameData.time = (float)i;
			transform.Decompose(frameData.scale, frameData.rotation, frameData.translation);
		}
		else
		{
			frameData = findNode->keyframe[i];
		}

		keyframe->transforms.push_back(frameData);
	}

	// 애니메이션 키프레임 채우기
	animation->keyframes.push_back(keyframe);

	// 재귀적 호출
	for (uint32 i = 0; i < srcNode->mNumChildren; i++)
		ReadKeyframeData(animation, srcNode->mChildren[i], cache);
}

/// <summary>
/// 추출한 데이터로 바이너리 형태 파일을 만든다.
/// (데이터를 납작하게 만들기)
/// </summary>
void Converter::WriteAnimationData(std::string _finalPath)
{
	auto path = std::filesystem::path(_finalPath);

	// 폴더가 없으면 만든다.
	std::filesystem::create_directory(path.parent_path());

	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(Utils::ToWString(_finalPath), FileMode::Write);

	file->Write<uint32>(this->scene->mNumAnimations);

	if (this->scene->mNumAnimations == 0)
	{
		file->Close();
		return;
	}

	for (auto& animation : this->animations)
	{

		file->Write<string>(animation->name);
		file->Write<float>(animation->duration);
		file->Write<float>(animation->frameRate);
		file->Write<uint32>(animation->frameCount);

		file->Write<uint32>(static_cast<uint32>(animation->keyframes.size()));

		for (std::shared_ptr<asKeyframe> keyframe : animation->keyframes)
		{
			file->Write<string>(keyframe->boneName);

			file->Write<uint32>(static_cast<uint32>(keyframe->transforms.size()));
			file->Write(&keyframe->transforms[0], static_cast<uint32>(sizeof(asKeyframeData) * keyframe->transforms.size()));
		}
	}
}

void Converter::WriteAnimationNameData(std::string _finalPath)
{
	_finalPath += ".csv";
	FILE* file;
	::fopen_s(&file, _finalPath.c_str(), "w");
	assert(file);
	for (size_t i = 0; i < this->animations.size(); i++)
	{
		size_t lastComma = this->animations[i]->name.find_last_of('|');
		string s = this->animations[i]->name.substr(lastComma + 1);
		::fprintf(file, "%s,%d\n", s.c_str(), (int)i);
	}

	::fclose(file);
}

/// <summary>
/// bone 이름으로 index 알아내기
/// </summary>
uint32 Converter::GetBoneIndex(const string& name)
{
	for (std::shared_ptr<asBone>& bone : this->bones)
	{
		if (bone->name == name)
			return bone->index;
	}

	assert(false && "Bone 이름 확인");
	return 0;
}
