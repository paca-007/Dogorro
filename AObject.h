#pragma once
#include <string>
#include <vector>
#include <memory.h>
#include <memory>
#include <map>


class AMesh;
class ABone;
class AMaterial;
class AAnimation;
class GraphicsEngine;
class IGraphicsEngine;
class AAnimator;
class DeferredRenderer;
class ForwardRenderer;
class BackgroundRenderer;

struct ABoundingBox
{
	float xLen;
	float yLen;
	float zLen;

	float cx, cy, cz;
};

class AObject
{
public:

	std::vector<float> position;
	std::vector<float> rotation;
	std::vector<float> scale;

	std::vector<float> finalPos;

	bool hasBone = false;
	bool hasAnimaiton = false;
	bool isOutline = false;
	bool isVisible = false;

private:
	std::vector<std::shared_ptr<AMesh>> meshes;
	std::shared_ptr<ABone> rootBone;
	std::vector<std::shared_ptr<AMaterial>> materials;
	std::vector<std::shared_ptr<AAnimation>> animations;
	std::vector<std::shared_ptr<ABone>> bones;

	std::map<std::string, int> boneMap;

	std::map<std::string, int> animationMap;

	std::shared_ptr<AAnimator> animator;

	std::string filePath;
	std::string modelPath = "../AssimpData/Models/";
	std::string texturePath = "../AssimpData/Textures/";
	std::string name;

	GraphicsEngine* gp = nullptr;

	DeferredRenderer* deferredRenderer = nullptr;
	ForwardRenderer* forwardRenderer = nullptr;
	BackgroundRenderer* backgroundRenderer = nullptr;

	std::weak_ptr<ABone> attachedObject;
	std::string attachBoneName;
	
private:
	AObject() {};


public:
	AObject(std::string _path, IGraphicsEngine* _igp, bool _hasBone = true, bool _hasAnimation = true, bool _isTransparent = true);
	~AObject();

	void Render();
	void ApplyAnimation(std::string _name, float _dt);

	void SetPosition(float _x, float _y, float _z);
	void AddPosition(float _x, float _y, float _z);

	void SetRotation(float _x, float _y, float _z);
	void AddRotation(float _x, float _y, float _z);

	void SetScale(float _x, float _y, float _z);
	void AddScale(float _x, float _y, float _z);

	void SetOutline(bool _val);

	std::string GetName(int _index = 0);
	
	void Attach(AObject* _other, std::string _where);
	void AttachTo(AObject* _other, std::string _where);

	void Detach();
	void Detach(AObject* _other);

	ABoundingBox GetBoundingBox(int _index = 0);

private:
	void ReadMaterial();
	void ReadMesh(bool _isMap = false);
	void ReadAnimation();

	void DecomposeMeshMatrix();

	std::shared_ptr<AMaterial> GetMaterial(std::string _name);
	std::shared_ptr<ABone> GetBone(int _index);

	void BindData();
	void CreateMeshBuffer();

	friend class MapLoader;
	friend class AAnimator;
};

