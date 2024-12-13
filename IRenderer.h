#pragma once
#include "pch.h"

class GraphicsEngine;

class IRenderer abstract
{
public:
	GraphicsEngine* gp = nullptr;

	virtual ~IRenderer() {};

	virtual void Initailze(GraphicsEngine* _gp) abstract;
	virtual void Finalize() abstract;
	
	virtual void BeginRender() abstract;
	virtual void EndRender() abstract;
};

