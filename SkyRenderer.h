#pragma once
#include "pipeline.h"

/// <summary>
/// SkyRenderer
/// 일단 스카이박스는 하나만 사용된다는 전제하에
/// Renderer 하나에 박아둔다. 
/// </summary>

class GraphicsEngine;
class FbxData;
class FbxMeshData;

class SkyRenderer
{
public:
	SkyRenderer();
	~SkyRenderer();

	void Initailze(GraphicsEngine* _gp);
	void Finalize();

	void BeginRender();
	void EndRender();

	void Render();
	void SkyRenderClearView();


private:
	ID3D10EffectShaderResourceVariable* skySRViariable;
	ID3D10ShaderResourceView* skySRV;
	ID3D10EffectTechnique* skyET;

public:
	void CreateSphere(int LatLines, int LongLines);
};

