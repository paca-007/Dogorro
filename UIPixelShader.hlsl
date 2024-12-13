// �ȼ� ���̴� �Է� ����ü
struct PS_POSTFX_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


// �ؽ�ó ���ҽ� ����
Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);


// �ȼ� ���̴� �Լ�
float4 PS(PS_POSTFX_INPUT pin) : SV_TARGET
{

	// �ؽ�ó���� ������ ���ø�
	float4 texColor = g_Texture.Sample(g_Sampler, pin.Tex);

	float4 whiteColor = float4(1.f, 1.f, 1.f, 1.f);


	
	return texColor;

}