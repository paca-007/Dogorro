#include "DXTKFont.h"
#include <tchar.h>

#define SafeDelete(x) { delete x; x = 0; }
#define SAFE_RELEASE(x) { if(x){ x->Release(); x = 0; } }	// 예전 스타일

#include <DirectXMath.h> // XMFLOAT
#include "SimpleMath.h"

DXTKFont::DXTKFont()
	: m_pSpriteBatch(nullptr), m_pSpriteFont(m_pSpriteFont), m_RasterizerState(nullptr), m_DepthStencilState(nullptr)
{

}

DXTKFont::~DXTKFont()
{
	SafeDelete(m_pSpriteFont);
	SafeDelete(m_pSpriteBatch);
}

void DXTKFont::Create(ID3D11Device* pDevice, ID3D11RasterizerState* rs, ID3D11DepthStencilState* ds)
{
	ID3D11DeviceContext* pDC = nullptr;
	pDevice->GetImmediateContext(&pDC);

	m_pSpriteBatch = new DirectX::SpriteBatch(pDC);

	TCHAR* _fileName = (TCHAR*)L"../Font/gulim9k.spritefont";

	m_pSpriteFont = new DirectX::SpriteFont(pDevice, _fileName);
	m_pSpriteFont->SetLineSpacing(14.0f);
	//m_pSpriteFont->SetDefaultCharacter(' ');

	SAFE_RELEASE(pDC);

	m_RasterizerState = rs;
	m_DepthStencilState = ds;
}

void DXTKFont::DrawTest()
{
	m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, L"Hello, world! 한글은 안되냐? 똠방각하 펲시 콜라 뾸뾸", DirectX::XMFLOAT2(10.f, 10.f));
	m_pSpriteBatch->End();
}

void DXTKFont::DrawTextColor(int x, int y, DirectX::XMFLOAT4 color, TCHAR* text, ...)
{
	TCHAR _buffer[1024] = L"";
	va_list vl;
	va_start(vl, text);
	_vstprintf(_buffer, 1024, text, vl);
	va_end(vl);

	// SpriteBatch에 여러 렌더 스테이트를 지정할 수 있다.
	// 문제는, 그냥 Begin만 하면 뎁스스탠실버퍼 옵션이 D3D11_DEPTH_WRITE_MASK_ZERO가 되는 듯. DSS를 다루지 않는 포반에는 문제가 될 수 있다.
	// 아래처럼 여기에 옵션을 ALL로 넣어줘서 ZERO가 되는 것을 막을 수도 있고, 다른 쪽 오브젝트를 그릴 때 렌더스테이트를 지정 해 줄 수도 있다.
	// DX12에서 렌더스테이트등을 그렇게 가져가는 것도 이해는 간다. 별로 맘에 안들었겠지..
	m_pSpriteBatch->Begin(DirectX::SpriteSortMode::SpriteSortMode_Deferred, nullptr, nullptr, m_DepthStencilState/*, m_RasterizerState*/);
	//m_pSpriteBatch->Begin();
	m_pSpriteFont->DrawString(m_pSpriteBatch, _buffer, DirectX::XMFLOAT2((float)x, (float)y), DirectX::SimpleMath::Vector4(color));
	m_pSpriteBatch->End();
}
