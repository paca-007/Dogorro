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
/// 강아지 렌더링용 포워드 렌더러
/// 사실 투명한 물체를 그리기 위한거지만
/// 투명한 물체는 강아지만 있을 예정
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

	//D3D 디바이스와 디바이스 컨텍스트
	ComPtr<ID3D11Device> d3d11Device;
	ComPtr<ID3D11DeviceContext> d3d11DeviceContext;

	ComPtr<ID3D11DepthStencilView> depthStancilView;

	const int windowWidth;
	const int windowHeight;

	//안에서 생성되는 것들
	std::vector<ID3D11ShaderResourceView*> fSRV;
	std::vector<ID3D11Texture2D*> fTexture;
	std::vector<ID3D11RenderTargetView*> fRenderTargets;

	ComPtr<ID3D11ShaderResourceView> f2SRV;
	ComPtr<ID3D11Texture2D> f2Texture;
	ComPtr<ID3D11RenderTargetView> f2RenderTargets;

	//렌더링 될 대상을 불러와서 저장해둔다.
	std::vector<std::vector<std::shared_ptr<AMesh>>> tpData;
	std::vector<std::vector<std::shared_ptr<ABone>>> tpBone;
	std::vector<std::vector<float>> tpPosition;
	std::vector<std::vector<float>> tpRotation;
	std::vector<std::vector<float>> tpScale;



	//Depth문제를 해결하기 위한 데이터
	ComPtr<ID3D11ShaderResourceView> dSRV;	  //텍스쳐
	ComPtr<ID3D11ShaderResourceView> dSRV2;  //뎁스

	//결국 돌고 돌아 다시 투패스입니다.
	PipeLine FPipeline;
	VertexD::Data FVdata[4];
	UINT FIdata[6];
	int fBufferSize = 2;



};

