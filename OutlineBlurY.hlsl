Texture2D OutlineTexture : register(t0);

SamplerState fSampler : register(s0);

struct PS_POSTFX_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

float4 PS(PS_POSTFX_INPUT input) :SV_Target
{
	float4 outline = OutlineTexture.Sample(fSampler, input.Tex);
    
    float _BlurSize = 0.001;
	
    //위의 선에 대해서 블러처리합시다~
    float invAspect = 1024.f / 1080.f;
    
    //init color variable
    float4 col = float4(0,0,0,0);

    for(float index=0; index<10; index++)
    {
        //get uv coordinate of sample
        float2 uv = input.Tex + float2(0, (index/9 - 0.5) * _BlurSize );
        //add color at position to color
        
        //col = max(col, OutlineTexture.Sample(fSampler, uv));
        col += 80.0 * OutlineTexture.Sample(fSampler, uv); // Use Sample function instead of tex2D
    }
    
    col = col / 10.f;
    
    
    return col;
}