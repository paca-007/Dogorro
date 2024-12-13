#pragma once
#include "IRenderer.h"
#include "Vertex.h"
#include "pipeline.h"

/// <summary>
/// 디퍼드 렌더러를 만들어봅시다~
/// 일단은 디퍼드가 되도록 하고 
/// GameEngine에서 받는건 두번째로 생각합시다.
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

	// D3D 디바이스와 디바이스 컨텍스트
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DeviceContext;

	ID3D11DepthStencilView* depthStancilView;

	const int windowWidth;
	const int windowHeight;

	//안에서 생성되는것들
	const int gBufferSize = 6;
	std::vector<ID3D11Texture2D*> dTexture;
	std::vector<ID3D11ShaderResourceView*> dSRV;
	std::vector<ID3D11RenderTargetView*> dRenderTargets;
	
	//마지막으로 그려질 친구들
	ID3D11Texture2D* dFinalTexture;
	ID3D11ShaderResourceView* dFinalSRV;
	ID3D11RenderTargetView* dFinalRenderTargets;

	PipeLine DPipeline;
	PipeLine DFinalPipeline;
	VertexD::Data DVdata[4];
	UINT DIdata[6];

	PipeLine DSubPipeline[6];
	VertexD::Data DSubVdata[6][4];

	//뎁스 문제를 해결하기 위해 최종적으로 뽑아낼 쉐이더리소스뷰
	//ID3D11ShaderResourceView* deferredTexture;
};

