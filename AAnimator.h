#pragma once
#include "pch.h"

class AObject;

class AAnimator
{
public:
	// animatirx[a][f][b]
	// a 번째 animation
	// f 번째 boneframe
	// b 번째 bone
	std::vector<std::vector<std::vector<Matrix>>> aniMatrix;

public:
	void CreateAniMatrix(AObject* _obj);
};

