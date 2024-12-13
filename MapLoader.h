#pragma once
#include "AObject.h"

class MapLoader
{
public:
	static std::vector<AObject*> LoadMapData(std::string _path, IGraphicsEngine* _igp);
};

