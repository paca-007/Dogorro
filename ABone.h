#pragma once
#include "pch.h"

class AObject;

class ABone
{
public:
	int32 index;
	std::string name;
	int32 parentIndex;
	Matrix transform;
	Matrix originalLocalTransform;
	Matrix oriinalWorldTransform;
	Matrix offsetTrasform;

	AObject* obj;
	std::weak_ptr<ABone> parent;
	std::vector<std::weak_ptr<ABone>> children;
};

