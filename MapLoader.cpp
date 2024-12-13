#include "MapLoader.h"
#include "FileUtils.h"
#include "Utils.h"
#include <filesystem>
#include "GraphicsEngine.h"
#include "IGraphicsEngine.h"
#include "AObject.h"
#include "AMesh.h"

std::vector<AObject*> MapLoader::LoadMapData(std::string _path, IGraphicsEngine* _igp)
{
	std::string modelPath = "../AssimpData/Models/" + _path + ".map.dir";
	std::string texturePath = "../AssimpData/Models/" + _path + "/Textures";

	std::vector<AObject*> result;

	std::shared_ptr<FileUtils> file = std::make_shared<FileUtils>();
	file->Open(Utils::ToWString(modelPath + "/map.data"), FileMode::Read);

	std::map<std::string, uint32> meshMap;
	std::map<uint32, std::vector<Matrix>> mapData;

	uint32 meshCount = file->Read<uint32>();

	/// 매쉬의 이름, 인덱스, 월드 행렬 매핑 값을 가져온다.
	for (uint32 i = 0; i < meshCount; i++)
	{
		uint32 index = file->Read<uint32>();
		std::string name = file->Read<std::string>();
		meshMap[name] = index;
		uint32 size = file->Read<uint32>();
		mapData[index].reserve(size);
		for (uint32 j = 0; j < size; j++)
		{
			Matrix wTM = file->Read<Matrix>();
			mapData[index].push_back(wTM);
		}
	}

	file->Close();

	std::filesystem::path p(modelPath + "/meshes/");

	std::filesystem::directory_iterator itr(p);

	/// 마지막으로 맵에 있는 mesh 데이터를 읽고 AObject로 만든다
	for (const std::filesystem::directory_entry& entry : itr)
	{
		std::string fileName = entry.path().filename().string();
		size_t pos = fileName.find_last_of('.');
		fileName = fileName.substr(0, pos);
		std::string filePath = p.string() + fileName;

		uint32 meshCount = static_cast<uint32>(mapData[meshMap[fileName]].size());

		for (uint32 i = 0; i < meshCount; i++)
		{
			AObject* obj = new AObject();
			obj->isVisible = true;
			obj->hasBone = false;
			obj->hasAnimaiton = false;
			obj->gp = dynamic_cast<GraphicsEngine*>(_igp);
			obj->modelPath = filePath;
			obj->ReadMesh(true);
			obj->meshes[0]->ori = mapData[meshMap[fileName]][i];
			obj->CreateMeshBuffer();

			obj->texturePath = modelPath + "/materials/";
			obj->filePath = obj->meshes[0]->materialName;
			obj->ReadMaterial();

			obj->position = std::vector<float>{ 0.0f, 0.0f, 0.0f };
			obj->rotation = std::vector<float>{ 0.0f, 0.0f, 0.0f };
			obj->scale = std::vector<float>{ 1.0f, 1.0f, 1.0f };
			obj->finalPos = std::vector<float>{ 1.0f, 1.0f, 1.0f };

			obj->BindData();
			obj->DecomposeMeshMatrix();
			result.push_back(obj);
		}
	}

	return result;
}
