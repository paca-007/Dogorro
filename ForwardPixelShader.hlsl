// 텍스처 선언
Texture2D fTexture : register(t0);
SamplerState f_Sampler : register(s0);

// 쉐이더 입력 구조체
struct VIN
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float2 Texf : TEXCOORD; 
};

struct POUT
{
    float4 Color : SV_TARGET1;
    float4 zDepth: SV_TARGET2;
  
};

POUT PS(VIN pin) : SV_Target
{
    POUT result;
    
    // 텍스처 샘플링
    float4 color = fTexture.Sample(f_Sampler, pin.Texf);
 
    // 투명도 조절 
    color.a *= 0.5; // 투명도 값을 조절하여 투명한 효과를 얻을 수 있음
    result.Color = color;
    
    //내 깊이 계산 
	float d = pin.PosH.z / pin.PosH.w;
	result.zDepth = float4(d, d, d, 0);
    
    return result;
}
