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
/// ���� ( fbx, ase obj��)�� �о �޸𸮿� �ø���.
/// </summary>
/// <param name="_file">���</param>
void Converter::ReadAssetFile(std::string _file)
{
	std::string fileStr = this->assetPath + _file;

	auto p = std::filesystem::path(fileStr);
	assert(std::filesystem::exists(p) && "not exsists file path");

	this->scene = this->importer->ReadFile(
		fileStr,
		aiProcess_ConvertToLeftHanded |  // �޼� ��ǥ�� �ε�, uv ������ �»��, cw �ε� (�ð� ����)
		aiProcess_MakeLeftHanded |
		//aiProcess_FlipUVs |
		//aiProcess_FlipWindingOrder |
		//aiProcess_JoinIdenticalVertices | // ������ ��ġ�� ������ �ϳ��� �������� ���� (���� �������� �ߺ��� �����ϰ� �� ����ȭ)
		//aiProcess_PreTransformVertices | // ���������� �ִϸ��̼� ������ �����ϰ� ���� ������ ����
		aiProcess_Triangulate | // �ﰢ�� �޽� ���·� ����
		aiProcess_GenUVCoords | // UV ���� ����
		aiProcess_GenNormals | // Normal ���� ����
		aiProcess_CalcTangentSpace  // TangentSpace ���� ���� 
	);

	assert(this->scene && "cannot read file");
}

/// <summary>
/// �������� ������ Ŀ���� ������ �������� �ٲ۴�.
/// </summary>
/// <param name="_savePath">���� �� ���</param>
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
/// ���׸��� ������ �ε�
/// </summary>
/// <param name="_savePath">���� ���</param>
void Converter::ExportMaterialData(std::string _savePath)
{
	std::string finalPath = this->texturePath + _savePath + ".xml";

	ReadMaterialData();
	WriteMaterialData(finalPath);
}

/// <summary>
/// animation �����͸� �����ؼ� ���̳ʸ� �������� ����
/// </summary>
/// <param name="_savePath">���� ���</param>
/// <param name="_index">�ε���</param>
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
/// fbx �����͸� �̿��� ���� �����ϴ� �Լ�
/// �Ž��� �̸� = ������ Ŭ����
/// �� �� �پ��� ������� �����ϸ� �ִϸ��̼� �����ͳ�
/// �� �پ��� �����͸� �����ͼ� 
/// ��¥ �� �� �������� ����� �� ���� ������?
/// ���� ��ü ���� ���� ������ �� ��������
/// </summary>
/// made by hedwig
/// <param name="_file">���</param>
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
/// ���� ������ �� �����͸� �д´�.
/// </summary>
/// <param name="_node">�� ���</param>
/// <param name="_index">�� ���� �ε���</param>
/// <param name="_parent">�θ� ���� �ε���</param>
void Converter::ReadModelData(aiNode* _node, int32 _index, int32 _parent)
{
	// �ϴ� ��带 ���� �� �̶�� �����Ѵ�.
	std::shared_ptr<asBone> bone = std::make_shared<asBone>();
	bone->index = _index;
	bone->parentIndex = _parent;
	bone->name = _node->mName.C_Str();

	// �θ� ���� ���
	Matrix transform(_node->mTransformation[0]);

	// DX�� �� �켱, FBX, ASE�� �� �켱
	bone->Set(_node);
	_node->mMetaData;

	// �θ� ��� ��������
	// �θ� ���ٸ� �������
// 	Matrix matParent = Matrix::Identity;
// 		if (_parent >= 0)
// 		{
// 			matParent = this->bones[_parent]->relativeTransform;
// 		}
// 	
// 		// �θ� ������ ����� ��Ʈ ������ ��ķ� �ٲ��ش�.
// 		// bone->relativeTransform = bone->relativeTransform * matParent;

	this->bones.push_back(bone);

	// Mesh ���� �б�
	ReadMeshData(_node, _index);

	// ���
	for (uint32 i = 0; i < _node->mNumChildren; i++)
	{
		ReadModelData(_node->mChildren[i], static_cast<uint32>(this->bones.size()), _index);
	}
}

/// <summary>
/// �Ž� ������ �б�
/// </summary>
/// <param name="_node">���</param>
/// <param name="_bone">��</param>
void Converter::ReadMeshData(aiNode* _node, int32 _bone)
{
	// �Ž��� ���� ����� ���
	if (_node->mNumMeshes < 1)
	{
		return;
	}

	// �Ž��� ������ �� ���� �ִ�.
	for (uint32 i = 0; i < _node->mNumMeshes; i++)
	{
		std::shared_ptr<asMesh> mesh = std::make_shared<asMesh>();

		mesh->name = _node->mName.C_Str();
		mesh->boneIndex = _bone;

		// scene ���� ���εǴ� �Ž��� �ε���
		uint32 index = _node->mMeshes[i];
		const aiMesh* srcMesh = this->scene->mMeshes[index];

		// �� ��ü�� ���� �Ž��� ���� �� �ִ�.
		// �� �Ž����� �ٸ� ���׸����� ��� �� �� �ִ�.
		// �� �� ����� ���� ����
		const aiMaterial* material = this->scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str();
		mesh->maxAABB = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		mesh->minAABB = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		// ���� ��ȸ
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
			// ���� UV ������ �ִٸ� �޾ƿ´�.
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
/// �������� ���� ����ġ�� �о�´�.
/// </summary>
void Converter::ReadSkinData()
{
	// ��� �Ž� ��ȸ
	for (uint32 i = 0; i < this->scene->mNumMeshes; i++)
	{
		aiMesh* srcMesh = this->scene->mMeshes[i];

		// ���� ���ٸ� 
		if (!srcMesh->HasBones())
		{
			continue;
		}

		std::shared_ptr<asMesh> mesh = this->meshes[i];

		// �� ��ȣ�� ����ġ�� �ӽ� ������ ����
		std::vector<asBoneWeights> tempData;
		tempData.resize(mesh->vertices.size());

		for (uint32 b = 0; b < srcMesh->mNumBones; b++)
		{
			aiBone* srcMeshBone = srcMesh->mBones[b];

			uint32 boneIndex = GetBoneIndex(srcMeshBone->mName.C_Str());
			this->bones[boneIndex]->offsetMatrix = Matrix(srcMeshBone->mOffsetMatrix[0]).Transpose();
			for (uint32 w = 0; w < srcMeshBone->mNumWeights; w++)
			{
				// �ش� ���� ������ �޴� ���� ��ȣ
				uint32 index = srcMeshBone->mWeights[w].mVertexId;

				float weight = srcMeshBone->mWeights[w].mWeight;

				tempData[index].AddWeights(boneIndex, weight);
			}
		}

		// ��� ���
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
/// ���� ������ �����ϴ� ����
/// �ؽ�Ʈ ���� �������� �����͸� �����Ͽ� �����ϰ� ����� ����
/// </summary>
void Converter::WriteCSVFile(std::string _savePath)
{
	// csv ���� ���
	size_t lastSlashPos = _savePath.find_last_of(L'/');
	std::string fileName = _savePath.substr(lastSlashPos + 1);
	std::string csvFilePath = "../AssimpData/PersonalSpace/" + fileName + ".csv";

	FILE* file;
	::fopen_s(&file, csvFilePath.c_str(), "w");

	if (file == NULL)
		assert(false && "CSV ���� ���� ����");

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
/// ������ �����ͷ� ���̳ʸ� ���� ������ �����.
/// (�����͸� �����ϰ� �����)
/// </summary>
void Converter::WriteModelFile(std::string _finalPath)
{
	auto path = std::filesystem::path(_finalPath);

	// ������ ������ �����.
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
/// �� ������ ���� �б�
/// </summary>
/// <param name="_node">���� ���</param>
void Converter::ReadMapData(aiNode* _node, int32 _index, int32 _parent)
{
	// �� ��ġ�� ����ϱ� ���� �����͸� ���� �ϴ� ��
	std::shared_ptr<asBone> bone = std::make_shared<asBone>();
	bone->index = _index;
	bone->parentIndex = _parent;
	bone->name = _node->mName.C_Str();

	// DX�� �� �켱, FBX, ASE�� �� �켱
	bone->Set(_node);

	// �θ� ��� ��������
	// �θ� ���ٸ� �������
	Matrix matParent = Matrix::Identity;
	if (_parent >= 0)
	{
		matParent = this->bones[_parent]->relativeTransform;
	}

	// �θ� ������ ����� ��Ʈ ������ ��ķ� �ٲ��ش�.
	bone->relativeTransform = bone->relativeTransform * matParent;

	this->bones.push_back(bone);

	// Mesh ���� �б�
	ReadMapModelData(_node, _index);

	// ���
	for (uint32 i = 0; i < _node->mNumChildren; i++)
	{
		ReadMapData(_node->mChildren[i], static_cast<uint32>(this->bones.size()), _index);
	}
}

void Converter::ReadMapModelData(aiNode* _node, int32 _bone)
{
	// �Ž��� ���� ����� ���
	if (_node->mNumMeshes < 1)
	{
		return;
	}

	// �Ž��� ������ �� ���� �ִ�.
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

		// scene ���� ���εǴ� �Ž��� �ε���
		uint32 index = _node->mMeshes[i];
		const aiMesh* srcMesh = this->scene->mMeshes[index];

		// �� ��ü�� ���� �Ž��� ���� �� �ִ�.
		// �� �Ž����� �ٸ� ���׸����� ��� �� �� �ִ�.
		// �� �� ����� ���� ����
		const aiMaterial* material = this->scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str();
		mesh->maxAABB = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		mesh->minAABB = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		// ���� ��ȸ
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
			// ���� UV ������ �ִٸ� �޾ƿ´�.
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
/// �� ���� ������ ����
/// </summary>
/// <param name="_savePath">���</param>
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

		// ������ ������ �����. (House/House.xml ���� ���� ������ House �� ����) 
		std::filesystem::create_directory(path.parent_path());

		string folder = path.parent_path().string();

		// ���� ����
		std::shared_ptr<tinyxml2::XMLDocument> document = std::make_shared<tinyxml2::XMLDocument>();

		// xml �������� �аڴٰ� ����
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
/// �� ���� ����� ������ ����
/// </summary>
/// <param name="_savePath">���</param>
void Converter::WriteMapCSVFile(std::string _savePath)
{
	// csv ���� ���
	size_t lastSlashPos = _savePath.find_last_of(L'/');
	std::string fileName = _savePath.substr(lastSlashPos + 1);
	std::string csvFilePath = "../AssimpData/PersonalSpace/" + fileName + ".csv";

	FILE* file;
	::fopen_s(&file, csvFilePath.c_str(), "w");

	if (file == NULL)
		assert(false && "CSV ���� ���� ����");

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
/// Scene ���� ����ִ� MaterialData�� �о���̰� �޸𸮿� �����Ѵ�.
/// </summary>
void Converter::ReadMaterialData()
{
	for (uint32 i = 0; i < this->scene->mNumMaterials; ++i)
	{
		// Assimp �� �ε��� ���� ���׸��� ������
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
/// ������ �����ͷ� xml ������ �����.
/// </summary>
void Converter::WriteMaterialData(std::string finalPath)
{
	auto path = std::filesystem::path(finalPath);

	// ������ ������ �����. (House/House.xml ���� ���� ������ House �� ����) 
	std::filesystem::create_directory(path.parent_path());

	string folder = path.parent_path().string();
	std::wstring wfolder = path.parent_path().wstring();


	// ���� ����
	std::shared_ptr<tinyxml2::XMLDocument> document = std::make_shared<tinyxml2::XMLDocument>();

	// xml �������� �аڴٰ� ����
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
/// �ؽ��İ� �ٿ�ε� ���� FBX ���� ��ü�� �ִٸ� ������ �����ϰ�,
/// �ؽ��ĸ� ���� �޾Ҵٸ� Ư�� ��ġ�� ���� �������ش�.
/// 
/// ������Ʈ���� ��ο� ���� ������ �ʿ�.
/// </summary>
std::string Converter::WriteTexture(string saveFolder, string file)
{
	string fileName = std::filesystem::path(file).filename().string();
	string folderName = std::filesystem::path(saveFolder).filename().string();

	// FBX ���Ͽ� �ؽ��İ� ������ �Ǽ� �� �ִ� ��찡 �ִ�.
	// �Ϲ������δ� ���� �����ִ� ��찡 ���� �׷��� �����ϴ� �� ��õ�ϴ� ��?
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

	// �ؽ��ĸ� ���� �޾Ҵٸ� ���ο� ��η� �������
	else
	{
		// �ϼ��� �ϳ��� ���� ���
		string originStr = (std::filesystem::path(this->assetPath) / folderName / file).string();
		Utils::Replace(originStr, "\\", "/");

		// �����ؼ� ����Ǵ� ���� ���
		string pathStr = (std::filesystem::path(saveFolder) / fileName).string();
		Utils::Replace(pathStr, "\\", "/");

		// ���� ����
		::CopyFileA(originStr.c_str(), pathStr.c_str(), false);
	}

	return fileName;
}

/// <summary>
/// Animation Data �� �о��, ReadKeyframeData �� ��������� ȣ���Ͽ�
/// ��� Keyframe data �� ����.
/// </summary>
std::shared_ptr<asAnimation> Converter::ReadAnimationData(aiAnimation* srcAnimation)
{
	std::shared_ptr<asAnimation> animation = std::make_shared<asAnimation>();
	animation->name = srcAnimation->mName.C_Str();
	animation->frameRate = static_cast<float>(srcAnimation->mTicksPerSecond);

	// �ִϸ��̼� �����͸� ���������� ������ ��, �����Ӵ� 1���� �����͸� ������ �ȴ�.
	// �� �ִϸ��̼��� 1�� ������ �������� �� ��, 1�ʸ� ���ؼ� �Ҽ��� ������ �����ش�.
	// ex : mDuration ���� 3.5 ��� 3.5�� ������ �ִϸ��̼��� ��Ÿ���� �� -> 1�ʸ� ���ؼ� 4.5�ʷ� ������� 0.5�ʸ� ������ 4�ʸ� �����Ų��.
	animation->frameCount = static_cast<uint32>(srcAnimation->mDuration) + 1;
	animation->duration = static_cast<float>(srcAnimation->mDuration);

	std::map<string, std::shared_ptr<asAnimationNode>> cacheAnimNodes;

	// �ִϸ��̼��� ����ִ� bone ä��
	for (uint32 i = 0; i < srcAnimation->mNumChannels; ++i)
	{
		aiNodeAnim* srcNode = srcAnimation->mChannels[i];

		// �ִϸ��̼� ��� ������ �Ľ� (bone �ϳ��� �� �ʸ��� ��� �ִ� transform ���� ����ִ� node)
		std::shared_ptr<asAnimationNode> node = ParseAnimationNode(animation, srcNode);

		// ���� ã�� ��� �߿� ���� �� �ð����� �ִϸ��̼� �ð� ���� (��� bone �߿��� ���� �� time ������ �ִϸ��̼� ���̸� ����) 
		animation->duration = max(animation->duration, node->keyframe.back().time);

		cacheAnimNodes[srcNode->mNodeName.C_Str()] = node;
	}

	ReadKeyframeData(animation, this->scene->mRootNode, cacheAnimNodes);

	return animation;
}

/// <summary>
/// Bone ��� �ϳ��� �����͸� �Ľ��Ѵ�.
/// time ���� �ش��ϴ� transform ������ ��� �ִ�.
/// </summary>
std::shared_ptr<asAnimationNode> Converter::ParseAnimationNode(std::shared_ptr<asAnimation> animation, aiNodeAnim* srcNode)
{
	std::shared_ptr<asAnimationNode> node = std::make_shared<asAnimationNode>();
	node->name = srcNode->mNodeName;

	// ���� ���� Ű������ ������ �����´� (for ������ �����͸� ������ ����)
	uint32 keyCount = max(max(srcNode->mNumPositionKeys, srcNode->mNumScalingKeys), srcNode->mNumRotationKeys);

	// �� Ű�����ӿ� ���ؼ� position, scale, rotation �����͸� ���� 
	for (uint32 k = 0; k < keyCount; ++k)
	{
		asKeyframeData frameData;

		bool found = false;

		// ������� ó���� Ű�������� ��
		uint32 t = static_cast<uint32>(node->keyframe.size());
		while (static_cast<float>(srcNode->mPositionKeys[k].mTime) > static_cast<float>(t))
		{
			node->keyframe.push_back(node->keyframe.back());
			t = static_cast<uint32>(node->keyframe.size());
		}
		// �ִϸ��̼� �����ʹ� ���������� ���� �� ������, �ð� ���� �ߺ��ǰų� ������ ��찡 �߻��� �� �ִ�.
		// ���Ἲ�� ��Ȯ���� �����ϱ� ���� ���ǹ� �߰� 
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

	// Keyframe �÷��ֱ�
	// Ű������ ������(keyCount)�� ���� ������� ������ ������, �̰͸����� �ִϸ��̼��� �� ������ ���� �������� �ʴ´�.
	// �ִϸ��̼� �� ������ ������ keyframe �� ���ٸ� ������ keyframe �� �����ؼ� ä���־��ش�.
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
/// �ռ� ParseAnimationNode �Լ��� ����Ͽ� ��� bone �� data �� �̾Ƽ� cache �صξ���
/// �� ������ �������� Keyframe �� �����ϰ� KeyframeData �� ����ִ´�.
/// fineNode �� Ȱ���Ͽ� ĳ�õ� �����Ͱ� ���� ������ �ٽ� �����͸� �����ϰ�,
/// ĳ�õ� �����Ͱ� �ִ� ��쿡�� �� �����͸� ����Ѵ�.
/// �� �ߺ��� ������ ������ �����Ѵ�. 
/// ��������� ȣ���Ͽ� Ʈ������ ������ ��� ��� �����͸� �м��Ѵ�.
/// </summary>
void Converter::ReadKeyframeData(std::shared_ptr<asAnimation> animation, aiNode* srcNode, std::map<string, std::shared_ptr<asAnimationNode>>& cache)
{
	std::shared_ptr<asKeyframe> keyframe = std::make_shared<asKeyframe>();
	keyframe->boneName = srcNode->mName.C_Str();

	// �̹� cache �� ������� �˻�
	std::shared_ptr<asAnimationNode> findNode = cache[srcNode->mName.C_Str()];

	// �ִϸ��̼��� �� frame ��ŭ �����͸� �����Ѵ�.
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

	// �ִϸ��̼� Ű������ ä���
	animation->keyframes.push_back(keyframe);

	// ����� ȣ��
	for (uint32 i = 0; i < srcNode->mNumChildren; i++)
		ReadKeyframeData(animation, srcNode->mChildren[i], cache);
}

/// <summary>
/// ������ �����ͷ� ���̳ʸ� ���� ������ �����.
/// (�����͸� �����ϰ� �����)
/// </summary>
void Converter::WriteAnimationData(std::string _finalPath)
{
	auto path = std::filesystem::path(_finalPath);

	// ������ ������ �����.
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
/// bone �̸����� index �˾Ƴ���
/// </summary>
uint32 Converter::GetBoneIndex(const string& name)
{
	for (std::shared_ptr<asBone>& bone : this->bones)
	{
		if (bone->name == name)
			return bone->index;
	}

	assert(false && "Bone �̸� Ȯ��");
	return 0;
}
