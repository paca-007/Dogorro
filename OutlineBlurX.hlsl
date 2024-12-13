Texture2D OutlineBlurY : register(t0);

SamplerState fSampler : register(s0);

struct PS_POSTFX_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

float4 PS(PS_POSTFX_INPUT input) :SV_Target
{
	float4 outline = OutlineBlurY.Sample(fSampler, input.Tex);
	float _BlurSize = 0.001;
	//위의 선에 대해서 블러처리합시다~
    float invAspect = 1024.f / 1080.f;

    //init color variable
    float4 col = float4(0,0,0,0);

    //iterate over blur samples
    for(float index = 0; index < 10; index++){
        //get uv coordinate of sample
        float2 uv = input.Tex + float2((index/9 - 0.5) * _BlurSize * invAspect, 0);
        //add color at position to color
        //col = max(col, OutlineBlurY.Sample(fSampler, uv));
        col +=  10 * OutlineBlurY.Sample(fSampler, uv); // Use Sample function instead of tex2D
    }
    //divide the sum of values by the amount of samples
    col = col / 10.f;
    
    return col;
}