#pragma once
#include "IRenderer.h"
#include "Vertex.h"
#include "pipeline.h"
#include <vector>
#include <wrl\client.h>
using Microsoft::WRL::ComPtr;

class GraphicsEngine;
class AMesh;
class ABone;
class AMaterial;

/// <summary>
/// ������ �������� ������ ������
/// ��� ������ ��ü�� �׸��� ���Ѱ�����
/// ������ ��ü�� �������� ���� ����
/// 2024.01.29
/// </summary>


class ForwardRenderer
	:public IRenderer
{
public:
	ForwardRenderer(ID3D11Device* _d3d11Device, ID3D11DeviceContext* _d3d11DeviceContext, ID3D11DepthStencilView* _depthStancilView, const int _windowWidth, const int _windowHeight);
	~ForwardRenderer();

	virtual void Initailze(GraphicsEngine* _gp) override;
	virtual void Finalize() override;


	virtual void BeginRender() override;
	virtual void EndRender() override;

	void GetData(
		std::vector<std::shared_ptr<AMesh>> _nowMesh
		, std::vector<std::shared_ptr<ABone>> _nowBone
		, std::vector<float> _nowPostion
		, std::vector<float> _nowRotation
		, std::vector<float> _nowScale
	);

	void UpdateDeferredTexture();

	void CreateSecondPipeLine();
	void BeginTwoPass();

	void UnBindingPS();

private:
	GraphicsEngine* gp = nullptr;

	//D3D ����̽��� ����̽� ���ؽ�Ʈ
	ComPtr<ID3D11Device> d3d11Device;
	ComPtr<ID3D11DeviceContext> d3d11DeviceContext;

	ComPtr<ID3D11DepthStencilView> depthStancilView;

	const int windowWidth;
	const int windowHeight;

	//�ȿ��� �����Ǵ� �͵�
	std::vector<ID3D11ShaderResourceView*> fSRV;
	std::vector<ID3D11Texture2D*> fTexture;
	std::vector<ID3D11RenderTargetView*> fRenderTargets;

	ComPtr<ID3D11ShaderResourceView> f2SRV;
	ComPtr<ID3D11Texture2D> f2Texture;
	ComPtr<ID3D11RenderTargetView> f2RenderTargets;

	//������ �� ����� �ҷ��ͼ� �����صд�.
	std::vector<std::vector<std::shared_ptr<AMesh>>> tpData;
	std::vector<std::vector<std::shared_ptr<ABone>>> tpBone;
	std::vector<std::vector<float>> tpPosition;
	std::vector<std::vector<float>> tpRotation;
	std::vector<std::vector<float>> tpScale;



	//Depth������ �ذ��ϱ� ���� ������
	ComPtr<ID3D11ShaderResourceView> dSRV;	  //�ؽ���
	ComPtr<ID3D11ShaderResourceView> dSRV2;  //����

	//�ᱹ ���� ���� �ٽ� ���н��Դϴ�.
	PipeLine FPipeline;
	VertexD::Data FVdata[4];
	UINT FIdata[6];
	int fBufferSize = 2;



};

