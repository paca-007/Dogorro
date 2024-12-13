// 픽셀 쉐이더 입력 구조체
struct PS_POSTFX_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


// 텍스처 리소스 선언
Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);


// 픽셀 쉐이더 함수
float4 PS(PS_POSTFX_INPUT pin) : SV_TARGET
{

	// 텍스처에서 색상을 샘플링
	float4 texColor = g_Texture.Sample(g_Sampler, pin.Tex);

	float4 whiteColor = float4(1.f, 1.f, 1.f, 1.f);


	
	return texColor;

}