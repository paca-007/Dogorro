// 텍스처 선언
Texture2D dTexture : register(t0);
Texture2D dDepth : register(t1);
Texture2D fTexture : register(t2);
Texture2D fDepth : register(t3);

SamplerState f_Sampler : register(s0);

// 쉐이더 입력 구조체
struct PIN
{
    float4 pos    : SV_POSITION;
	float2 Tex     : TEXCOORD;
};

float4 PS(PIN pin) : SV_Target0
{
 
    //DeferredTexture
    float4 DTexture = dTexture.Sample(f_Sampler, pin.Tex);    
    
    //DeferredDepth
    float4 DDepth = dDepth.Sample(f_Sampler, pin.Tex);
    float dBuffer = DDepth.z; 
        
    
    //ForwardTexture
    float4 FTexture = fTexture.Sample(f_Sampler, pin.Tex);
    
    //ForwardDepth
    float4 FDepth = fDepth.Sample(f_Sampler, pin.Tex);    
    float fBuffer = FDepth.z;

    
    
    if(fBuffer < dBuffer)
		return DTexture;
	else
		return FTexture;

    //return color;
}
