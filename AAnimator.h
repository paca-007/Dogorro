#pragma once
#include "pch.h"

class AObject;

class AAnimator
{
public:
	// animatirx[a][f][b]
	// a ��° animation
	// f ��° boneframe
	// b ��° bone
	std::vector<std::vector<std::vector<Matrix>>> aniMatrix;

public:
	void CreateAniMatrix(AObject* _obj);
};

