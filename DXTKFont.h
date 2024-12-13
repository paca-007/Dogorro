#pragma once
#include "SpriteFont.h"
#include "SpriteBatch.h"

/// <summary>
/// DXDK �̿��� ���� ���
/// �ϴ��� ������ �ڵ� ����
/// ���߿� �� ������� �ٲ� ����
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


	// �׳� ����ϸ� ���� ������ �����.
	ID3D11RasterizerState* m_RasterizerState;
	ID3D11DepthStencilState* m_DepthStencilState;
};

