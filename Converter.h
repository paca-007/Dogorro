#pragma once
#include <map>
#include <Memory>
#include "pch.h"
#include "AssimpType.h"
class IGraphicsEngine;
class GraphicsEngine;

struct asBone;
struct asMesh;
struct asMaterial;
struct asAnimation;
struct asAnimationNode;

class Converter
{
private:
	std::string assetPath = "../AssimpData/Assets/";
	std::string modelPath = "../AssimpData/Models/";
	std::string texturePath = "../AssimpData/Textures/";
	GraphicsEngine* gp;

private:
	std::shared_ptr<Assimp::Importer> importer;
	const aiScene* scene;

private:
	Vector3 worldPosi;
	Vector3 worldRota;
	Vector3 worldScal;


	// 데이터 파싱하고 들고 있는 정보
	std::vector<std::shared_ptr<asBone>> bones; // 계층구조 
	std::vector<std::shared_ptr<asMesh>> meshes; // vertex, index, material 어떻게 연동하고 그려야할지 정보
	std::vector<std::shared_ptr<asMaterial>> materials; // 어떠한 texture, lighting 수치로 그려야 할지 정보 
	std::vector<std::shared_ptr<asAnimation>> animations;

	// 매쉬의 인덱스, 매쉬의 월드 매트릭스
	std::map<int, std::vector<Matrix>> mapData;
	std::map<std::string, int> mapMeshData;

	std::map<std::string, int> boneMapping;

public:
	Converter(IGraphicsEngine* _gp);
	~Converter();

public:
	// Asset 추출 -> CustomData File 생성
	void ReadAssetFile(std::string _file);
	void ExportModelData(std::string _savePath);
	void ExportMaterialData(std::string _savePath);
	void ExportAnimationData(std::string _savePath, uint32 _index = 0);


	void ExportMapData(std::string _file);

	static void ConvertAllAesset();

private:
	void ReadModelData(aiNode* _node, int32 _index, int32 _parent);
	void ReadMeshData(aiNode* _node, int32 _bone);
	void ReadSkinData();

	void WriteCSVFile(std::string _savePath);
	void WriteModelFile(std::string _finalPath);

	void WriteMapFile(std::string _savePath);
	void WriteMaterialMapFile(std::string finalPath);
	void WriteMapCSVFile(std::string _savePath);

	void ReadMapData(aiNode* _node, int32 _index, int32 _parent);
	void ReadMapModelData(aiNode* _node, int32 _bone);

	void MappingMatrixAtMesh();

private:
	void ReadMaterialData();
	void WriteMaterialData(std::string _finalPath);
	std::string WriteTexture(std::string _saveFolder, std::string _file);

private:
	std::shared_ptr<asAnimation> ReadAnimationData(aiAnimation* _srcAnimation);
	std::shared_ptr<asAnimationNode> ParseAnimationNode(std::shared_ptr<asAnimation> _animation, aiNodeAnim* _srcNode);
	void ReadKeyframeData(std::shared_ptr<asAnimation> _animation, aiNode* _srcNode, std::map<std::string, std::shared_ptr<asAnimationNode>>& _cache);
	void WriteAnimationData(std::string _finalPath);
	void WriteAnimationNameData(std::string _finalPath);

private:
	uint32 GetBoneIndex(const std::string& _name);
};

