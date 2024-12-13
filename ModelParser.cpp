#include "ModelParser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "color.h"
#include "ASEFile.h"
void GetVertexAndIndex(std::vector<VertexC::Data>& _vertexes, std::vector<UINT>& _indexes, std::wstring _filePath)
{
	std::string line;
	std::ifstream file(_filePath);

	if (file.is_open())
	{
		while (getline(file, line))
		{
			std::vector<std::string> parsed = split(line, ' ');

			if (parsed[0] == "v")
			{
				VertexC::Data input = { DirectX::XMFLOAT3{std::stof(parsed[1]) / 10.0f, std::stof(parsed[2]) / 10.0f ,std::stof(parsed[3]) / 10.0f}, COLORS::White };
				_vertexes.push_back(input);
			}
			else if (parsed[0] == "f")
			{
				for (int i = 1; i < parsed.size(); i++)
				{
					std::vector<std::string> indexData = split(parsed[i], '/');
					_indexes.push_back((UINT)(std::stoi(indexData[0])));
				}
			}
		}
	}
	else
	{
		assert(false && "cannot read 3d model object");
	}
}

std::vector<RenderObject*> AseParser(std::wstring _filePath)
{
	std::string line;
	std::ifstream file(_filePath);

	std::vector<RenderObject*> result;

	std::map<std::string, RenderObject*> dict;

	std::vector<std::string> s;

	std::vector<Mesh*> hasBoneMesh;

	int optimizeSize = 0;
	int optimizeIndex = 0;

	int maxTick = 0;
	int firatFrame = 0;
	int lastFrame = 0;
	int tickPerFrame = 0;

	int meshWeight = 0;

	Mesh* nowMesh = nullptr;
	RenderObject* nowRenderObject = nullptr;
	RenderObject* parentObj = nullptr;

	if (file.is_open())
	{
		while (getline(file, line))
		{
			s.clear();
			ASEToken(line, s);

			/// 매쉬 생성
			if (s[0] == Token[_ASEToken::TOKENR_MESH])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowMesh = new Mesh();
				nowRenderObject->AddMesh(nowMesh);
				optimizeIndex = 0;
				optimizeSize = 0;
			}
			else if (
				s[0] == Token[_ASEToken::TOKENR_GEOMOBJECT] ||
				s[0] == Token[_ASEToken::TOKENR_HELPEROBJECT] ||
				s[0] == Token[_ASEToken::TOKENR_SHAPEOBJECT]
				)
			{
				nowRenderObject = new RenderObject();
				nowRenderObject->maxTick = lastFrame * tickPerFrame;
				parentObj = nullptr;

				if (s[0] == Token[_ASEToken::TOKENR_GEOMOBJECT])
				{
					nowRenderObject->type = RENDER_OBJECT_TYPE::GEOMOBJCT;
				}
				else if (s[0] == Token[_ASEToken::TOKENR_HELPEROBJECT])
				{
					nowRenderObject->type = RENDER_OBJECT_TYPE::HELPEROBJECT;
				}
				else if (s[0] == Token[_ASEToken::TOKENR_SHAPEOBJECT])
				{
					nowRenderObject->type = RENDER_OBJECT_TYPE::SHAPEOBJECT;
				}
			}
			else if (s[0] == Token[_ASEToken::TOKENR_NODE_NAME])
			{
				assert(nowRenderObject && "no geomatry object\n");
				std::string fullName = "";

				for (size_t i = 1; i < s.size(); i++)
				{
					fullName += s[i];
				}
				nowRenderObject->SetName(fullName);

				dict[fullName] = nowRenderObject;
			}
			else if (s[0] == Token[_ASEToken::TOKENR_NODE_PARENT])
			{
				std::string fullName = "";
				for (size_t i = 1; i < s.size(); i++)
				{
					fullName += s[i];
				}
				parentObj = dict[fullName];
			}
			else if (s[0] == Token[_ASEToken::TOKENR_NODE_TM])
			{
				nowRenderObject->SetParent(parentObj);
				if (parentObj)
				{
					parentObj->AddChild(nowRenderObject);
				}
				else
				{
					result.push_back(nowRenderObject);
				}

			}
			/// TM 데이터
			else if (s[0] == Token[_ASEToken::TOKENR_TM_ROW0])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->nodeTM.r[0] = DirectX::XMVECTOR{ std::stof(s[1]), std::stof(s[3]), std::stof(s[2]) , 0.0f };
			}
			else if (s[0] == Token[_ASEToken::TOKENR_TM_ROW1])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->nodeTM.r[2] = DirectX::XMVECTOR{ std::stof(s[1]), std::stof(s[3]), std::stof(s[2]) , 0.0f };
			}
			else if (s[0] == Token[_ASEToken::TOKENR_TM_ROW2])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->nodeTM.r[1] = DirectX::XMVECTOR{ std::stof(s[1]), std::stof(s[3]), std::stof(s[2]) , 0.0f };
			}
			else if (s[0] == Token[_ASEToken::TOKENR_TM_ROW3])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->nodeTM.r[3] = DirectX::XMVECTOR{ std::stof(s[1]), std::stof(s[3]), std::stof(s[2]) , 1.0f };
			}
			else if (s[0] == Token[_ASEToken::TOKENR_TM_POS])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->filePosition = DirectX::XMVECTOR{ std::stof(s[1]), std::stof(s[3]), std::stof(s[2]) };
			}
			else if (s[0] == Token[_ASEToken::TOKENR_TM_ROTAXIS])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->fileRotate = DirectX::XMVECTOR{ std::stof(s[1]), std::stof(s[3]), std::stof(s[2]) };
			}
			else if (s[0] == Token[_ASEToken::TOKENR_TM_ROTANGLE])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->fileRotate.m128_f32[3] = std::stof(s[1]);
			}
			else if (s[0] == Token[_ASEToken::TOKENR_TM_SCALEAXIS])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->fileScale = DirectX::XMVECTOR{ std::stof(s[1]), std::stof(s[3]), std::stof(s[2]) };
			}
			else if (s[0] == Token[_ASEToken::TOKENR_TM_SCALEAXISANG])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->fileScale.m128_f32[3] = std::stof(s[1]);
			}
			/// 정점과 면의 갯수
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_NUMVERTEX])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				nowMesh->position = std::vector<DirectX::XMFLOAT3>(std::stoi(s[1]));
			}
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_NUMTVERTEX])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				nowMesh->texture = std::vector<DirectX::XMFLOAT2>(std::stoi(s[1]));
			}
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_NUMTVFACES])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				nowMesh->textureIndex = std::vector<int>(std::stoi(s[1]) * 3);
			}
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_NUMFACES])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				optimizeSize = std::stoi(s[1]) * 3;
				nowMesh->indexList = std::vector<UINT>(optimizeSize);
				nowMesh->vertexList = std::vector<VertexT::Data>(optimizeSize);
			}
			/// 정점 데이터 파싱
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_VERTEX])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				nowMesh->position[std::stoi(s[1])] = DirectX::XMFLOAT3{ std::stof(s[2]), std::stof(s[4]), std::stof(s[3]) };
			}
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_FACE])
			{
			}
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_VERTEXNORMAL])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				nowMesh->normalIndex.push_back(std::stoi(s[1]));
				nowMesh->normal.push_back(DirectX::XMFLOAT3{ std::stof(s[2]), std::stof(s[4]), std::stof(s[3]) });
			}
			/// 텍스쳐 데이터 파싱
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_TVERT])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				DirectX::XMFLOAT2 input = { std::stof(s[2]), 1 - std::stof(s[3]) };
				nowMesh->texture[std::stoi(s[1])] = std::move(input);
			}
			else if (s[0] == Token[TOKENR_MESH_TFACE])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				int startIndex = std::stoi(s[1]) * 3;
				nowMesh->textureIndex[startIndex] = std::stoi(s[2]);
				nowMesh->textureIndex[startIndex + 1] = std::stoi(s[3]);
				nowMesh->textureIndex[startIndex + 2] = std::stoi(s[4]);
			}
			/// 에니메이션 데이터
			else if (s[0] == Token[_ASEToken::TOKENR_CONTROL_POS_SAMPLE])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->animationPositionTrack.push_back(
					std::make_pair(
						std::stoi(s[1]),
						DirectX::XMFLOAT3{ std::stof(s[2]), std::stof(s[4]), std::stof(s[3]) }
					)
				);
			}
			else if (s[0] == Token[_ASEToken::TOKENR_CONTROL_ROT_SAMPLE])
			{
				assert(nowRenderObject && "no geomatry object\n");
				nowRenderObject->animationRotateTrack.push_back(
					std::make_pair(
						std::stoi(s[1]),
						DirectX::XMFLOAT4{ std::stof(s[2]), std::stof(s[4]), std::stof(s[3]), std::stof(s[5]) }
					)
				);
			}
			else if (s[0] == Token[_ASEToken::TOKENR_SCENE_LASTFRAME])
			{
				lastFrame = std::stoi(s[1]);
			}
			else if (s[0] == Token[_ASEToken::TOKENR_SCENE_TICKSPERFRAME])
			{
				tickPerFrame = std::stoi(s[1]);
			}
			/// Bone 데이터
			else if (s[0] == Token[_ASEToken::TOKENR_BONE_LIST])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				nowMesh->weight.resize(nowMesh->position.size());
				nowMesh->boneIndex.resize(nowMesh->position.size());
				hasBoneMesh.push_back(nowMesh);
			}
			else if (s[0] == Token[_ASEToken::TOKENR_BONE_NAME])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				std::string fullName = "";
				for (size_t i = 1; i < s.size(); i++)
				{
					fullName += s[i];
				}
				nowMesh->boneNames.push_back(fullName);
			}
			else if (s[0] == Token[_ASEToken::TOKENR_MESH_WEIGHT])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				meshWeight = std::stoi(s[1]);
			}
			else if (s[0] == Token[_ASEToken::TOKENR_BONE_BLENGING_WEIGHT])
			{
				assert(nowMesh && "Ase parser error. no mesh in data");
				nowMesh->boneIndex[meshWeight].push_back(std::stoi(s[1]));
				nowMesh->weight[meshWeight].push_back(std::stof(s[2]));
			}
		}
	}
	else
	{
		assert(false && "cannot read 3d model object");
	}

	for(auto& m : hasBoneMesh) 
	{
		m->bones.resize(m->boneNames.size());
		for (int i = 0; i < (int)m->boneNames.size(); i++)
		{
			m->bones[i] = dict[m->boneNames[i]];
		}
	}

	return result;
}

std::vector<std::string> split(std::string& input, char delimiter)
{
	std::vector<std::string> answer;
	std::stringstream ss(input);
	std::string temp;

	while (getline(ss, temp, delimiter)) {
		answer.push_back(temp);
	}

	return answer;
}

void ASEToken(std::string& _input, std::vector<std::string>& _tokens)
{
	std::string temp = "";

	for (auto& c : _input)
	{
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != ':')
		{
			temp += c;
		}
		else
		{
			if (temp != "")
			{
				_tokens.push_back(temp);
			}
			temp = "";
		}
	}
	_tokens.push_back(temp);
}
