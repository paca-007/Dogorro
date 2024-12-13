#pragma once
#include <string>
#include <memory.h>
#include <memory>

class GraphicsEngine;
class IGraphicsEngine;
class UIRenderer;
class AObject;

class UObject
{
private:
	GraphicsEngine* gp;
	
	std::wstring* path;
	std::wstring texturePath;
	UIRenderer* uiRenderer;
	

	float startPointX;
	float startPointY;
	float width;
	float height;

	float aniWidth;
	int aniCount;
	
	int windowWidth;
	int windowHeight;

	float dx;
	float dy;

	float offsetDx;
	float offsetDy;

	AObject* attachedObject;

public:
	UObject();
	~UObject();

	void Render();
	void AddPosition(float _x, float _y);
	void SetPosition(float _x, float _y);

	void Initalize(IGraphicsEngine* _gp, std::string name, std::wstring* _path, std::wstring _texturePath, float _startPointX, float _startPointY, float _width, float _height);
	void AnimationInitailze(IGraphicsEngine* _gp, std::string _name, std::wstring _texturePath, float _startPointX, float _startPointY, float _width, float _height, float _aniWidth, int _aniCount);

	void AttachTo(AObject* _obj, float _offsetX = 0 , float _offsetY = 0);
	void DetachTo();

	std::string name;
private:
	void CreateMesh();
	void CreateAnimationMesh();
};

