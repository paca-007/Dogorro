#pragma once
#include "IRenderer.h"
#include "Vertex.h"
#include "pipeline.h"

/// <summary>
/// ���۵� �������� �����ô�~
/// �ϴ��� ���۵尡 �ǵ��� �ϰ� 
/// GameEngine���� �޴°� �ι�°�� �����սô�.
/// 
/// </summary>

class GraphicsEngine;

class DeferredRenderer 
	: public IRenderer
{
public:
	DeferredRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext, ID3D11DepthStencilView* _depthStancilView, const int _windowWidth, const int _windowHeight);
	~DeferredRenderer();

	virtual void Initailze(GraphicsEngine* _gp) override;
	virtual void Finalize() override;


	virtual void BeginRender() override;
	virtual void EndRender() override;

	void BindDeferredView();
	void DeferredRenderClearView();

	void CreateDeferredPipeline();
	void CreateDeferredFinalPipeline();
	void CreateSubView();
	

	void UpdateTexture();

private:
	GraphicsEngine* gp = nullptr;

	// D3D ����̽��� ����̽� ���ؽ�Ʈ
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DeviceContext;

	ID3D11DepthStencilView* depthStancilView;

	const int windowWidth;
	const int windowHeight;

	//�ȿ��� �����Ǵ°͵�
	const int gBufferSize = 6;
	std::vector<ID3D11Texture2D*> dTexture;
	std::vector<ID3D11ShaderResourceView*> dSRV;
	std::vector<ID3D11RenderTargetView*> dRenderTargets;
	
	//���������� �׷��� ģ����
	ID3D11Texture2D* dFinalTexture;
	ID3D11ShaderResourceView* dFinalSRV;
	ID3D11RenderTargetView* dFinalRenderTargets;

	PipeLine DPipeline;
	PipeLine DFinalPipeline;
	VertexD::Data DVdata[4];
	UINT DIdata[6];

	PipeLine DSubPipeline[6];
	VertexD::Data DSubVdata[6][4];

	//���� ������ �ذ��ϱ� ���� ���������� �̾Ƴ� ���̴����ҽ���
	//ID3D11ShaderResourceView* deferredTexture;
};

