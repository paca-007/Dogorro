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
/// ���ȭ���� ����� ������
/// Outline�� ����� ���� ���� ����� �Ǿ���.
/// ����� outline�� �׷����� �ʾƵ� �� ģ������ �� �̰ɷ� �׸��� �ɵ�!
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

	//D3D ����̽��� ����̽� ���ؽ�Ʈ
	ID3D11Device* d3d11Device;
	ID3D11DeviceContext* d3d11DeviceContext;

	ID3D11DepthStencilView* depthStancilView;

	const int windowWidth;
	const int windowHeight;

	//�ȿ��� �����Ǵ� �͵�
	ID3D11Texture2D* bTexture;
	ID3D11ShaderResourceView* bSRV;
	ID3D11RenderTargetView* bRenderTarget;

	//UI�� ������ �� ����� �ҷ��� �����صд�.
	//std::vector<FbxMeshData*> bData;
	std::vector<std::vector<std::shared_ptr<AMesh>>> bData;
	std::vector<std::vector<float>> bPosition;
	std::vector<std::vector<float>> bRotation;
	std::vector<std::vector<float>> bScale;
};

