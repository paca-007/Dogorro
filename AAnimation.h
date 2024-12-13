#pragma once
#include "pch.h"


struct AFrameData
{
	float time = 0.f;
	Vector3 scale = {};
	Quaternion rotation = {};
	Vector3 translation = {};
};

struct AKeyFrame
{
	std::string boneName;
	std::vector<AFrameData> transforms;
};

class AAnimation
{
public:
	std::string name = "";
	float duration = 0.0f;
	float frameRate = 0.0f;
	uint32 frameCount = 0;

	std::map<std::string, std::shared_ptr<AKeyFrame>> keyframes;
};

