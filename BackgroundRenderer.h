#pragma once
#include "IRenderer.h"
#include "Vertex.h"
#include "pipeline.h"
#include <vector>
using Microsoft::WRL::ComPtr;

class GraphicsEngine;
class AMesh;
class AMaterial;

/// <summary>
/// 배경화면을 담당할 렌더러
/// Outline의 존재로 인해 냅다 생기게 되엇다.
/// 어찌보면 outline이 그려지지 않아도 될 친구들을 다 이걸로 그리게 될듯!
/// 2024.02.05
/// </summary>

class BackgroundRenderer
	:public IRenderer
{
public:
	BackgroundRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext, ID3D11DepthStencilView* _depthStancilView, const int _windowWidth, const int _windowHeight);
	~BackgroundRenderer();

	virtual void Initailze(GraphicsEngine* _gp) override;
	virtual void Finalize() override;


	virtual void BeginRender() override;
	virtual void EndRender() override;

	void GetData(
		  std::vector<std::shared_ptr<AMesh>> _nowMesh
		, std::vector<float> _nowPostion
		, std::vector<float> _nowRotation
		, std::vector<float> _nowScale
	);
	void SetRenderTarget();
	void ClearRenderTarget();
	void ClearShaderResources();
	
private:
	GraphicsEngine* gp = nullptr;

	//D3D 디바이스와 디바이스 컨텍스트
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DeviceContext;

	ID3D11DepthStencilView* depthStancilView;

	const int windowWidth;
	const int windowHeight;

	//안에서 생성되는 것들
	ID3D11Texture2D* bTexture;
	ID3D11ShaderResourceView* bSRV;
	ID3D11RenderTargetView* bRenderTarget;

	//UI로 렌더링 될 대상을 불러와 저장해둔다.
	//std::vector<FbxMeshData*> bData;
	std::vector<std::vector<std::shared_ptr<AMesh>>> bData;
	std::vector<std::vector<float>> bPosition;
	std::vector<std::vector<float>> bRotation;
	std::vector<std::vector<float>> bScale;
};

