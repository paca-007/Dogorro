Texture2D FinalTexture : register(t0);

SamplerState fSampler : register(s0);

struct PS_POSTFX_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

// Fragment Shader�� �ȼ� ������ ����Ǹ�,�ȼ�ȭ ȿ���� ������. 
// ���� �ȼ��� ���ø��Ͽ� ���� ����� ���� �� ���� ���� ã�Ƴ��� ���� �ȼ��� ������.
float4 PS(PS_POSTFX_INPUT input) : SV_Target
{
    float4 result;
    //float4 inputTexture = FinalTexture.Sample(fSampler, input.Tex);    

    float Pixels = 7000.0;
    float dx = 15.0 * (1.0 / Pixels);
    float dy = 10.0 * (1.0 / Pixels);
    float2 Coord = float2(dx * floor(input.Tex.x / dx), dy * floor(input.Tex.y / dy));
    result = FinalTexture.Sample(fSampler, Coord);

    return result; 
}

