Texture2D FinalTexture : register(t0);

SamplerState fSampler : register(s0);

struct PS_POSTFX_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

// Fragment Shader는 픽셀 단위로 실행되며,픽셀화 효과를 적용함. 
// 인접 픽셀을 샘플링하여 가장 가까운 색상 및 깊이 값을 찾아내어 현재 픽셀에 적용함.
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

