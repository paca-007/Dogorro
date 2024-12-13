#pragma once
#include "pipeline.h"

/// <summary>
/// SkyRenderer
/// �ϴ� ��ī�̹ڽ��� �ϳ��� ���ȴٴ� �����Ͽ�
/// Renderer �ϳ��� �ھƵд�. 
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

