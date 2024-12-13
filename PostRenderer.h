#pragma once

#include "IRenderer.h"
#include "Vertex.h"
#include "pipeline.h"
#include <wrl\client.h>
using Microsoft::WRL::ComPtr;

class GraphicsEngine;
class FbxData;
class FbxMeshData;

/// <summary>
/// ��� ����Ʈ���μ����� ���ư��� �� ������
/// �ؽ��ĸ� �̿��� ���� ó���ϰ� �� ���̴�
/// �Ƹ���.
/// 2024.02.01
/// </summary>

struct ConstantBufferData
{
	float Time;
	float Padding[15];	//������ 15����Ʈ�� ä��� ���� �е�
};

class PostRenderer 
	: public IRenderer
{
public:
	PostRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext, ID3D11DepthStencilView* _depthStancilView, const int _windowWidth, const int _windowHeight);
	~PostRenderer();

	virtual void Initailze(GraphicsEngine* _gp) override;
	virtual void Finalize() override;

	virtual void BeginRender() override;
	virtual void EndRender() override;
	
	void UpdateShaderResources();

	ComPtr<ID3D11ShaderResourceView> finalSRV;

//Outline 
private:
	void CreateOutlinePipeline();
	void CreateOutlineBlurYPipeline();
	void CreateOutlineBlurXPipeline();
	void CreateSecondOutlinePipeline();
	void FirstpassOutline();
	void SecondPassOutline();


//Pixelate
public:
	void CreatePixelatePipeline();
	void FirstpassPixel();


//Flash Effect
public:
	void CreateFlashPipeline();
	void FirstPassFlash();
	void UpdateFlashBuffer();

private:
	GraphicsEngine* gp = nullptr;

	//D3D ����̽��� ����̽� ���ؽ�Ʈ
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DeviceContext;

	ID3D11DepthStencilView* depthStancilView;

	const int windowWidth;
	const int windowHeight;

	//�ȿ��� �����Ǵ°͵�.
	HRESULT hr;

	//�⺻ ����ġ���� �� ģ��
	VertexD::Data PVdata[4];
	UINT PIdata[6];

	//�޾ƿͼ� ���� ��
	ComPtr<ID3D11ShaderResourceView> deferredNormal;	//normal
	ComPtr<ID3D11ShaderResourceView> deferredColor;	//deferredFinal
	ComPtr<ID3D11ShaderResourceView> deferredDepth;	//depth
	ComPtr<ID3D11ShaderResourceView> deferredTexture;	//diffuse
	ComPtr<ID3D11ShaderResourceView> finalTexture;		//FinalSRV

	///--------Outline----------
	PipeLine OutlinePipeline;
	PipeLine Pass2OutlinePipeline;
	PipeLine OutlineBlurYPipeline;
	PipeLine OutlineBluerXPipeline;

	ComPtr<ID3D11ShaderResourceView> outlineSRV;
	ComPtr<ID3D11Texture2D> outlineTexture;
	ComPtr<ID3D11RenderTargetView> outlineRenderTarget;
	
	ComPtr<ID3D11ShaderResourceView> outlineBlurYSRV;
	ComPtr<ID3D11Texture2D> outlineBlurYTexture;
	ComPtr<ID3D11RenderTargetView> outlineBlurYRenderTarget;

	///--------Pixelate----------
	PipeLine pixelatePipeline;

	///--------Flash-----------
	PipeLine flashPipeline;
	ComPtr<ID3D11Buffer> flashBuffer;
	ConstantBufferData flashData;
public:
	float GetTime() { return flashData.Time; }
	void SetTime(float _time) { flashData.Time = _time; }

	
};

