#pragma once
#include "SpriteFont.h"
#include "SpriteBatch.h"

/// <summary>
/// DXDK 이용한 글자 출력
/// 일단은 교수님 코드 복붙
/// 나중에 내 마음대로 바꿀 예정
/// </summary>
class DXTKFont
{
public:
	DXTKFont();
	~DXTKFont();

	void Create(ID3D11Device* pDevice, ID3D11RasterizerState* rs, ID3D11DepthStencilState* ds);

	void DrawTest();
	void DrawTextColor(int x, int y, DirectX::XMFLOAT4 color, TCHAR* text, ...);

private:
	DirectX::DX11::SpriteBatch* m_pSpriteBatch;
	DirectX::DX11::SpriteFont* m_pSpriteFont;


	// 그냥 사용하면 뎁스 문제가 생긴다.
	ID3D11RasterizerState* m_RasterizerState;
	ID3D11DepthStencilState* m_DepthStencilState;
};

