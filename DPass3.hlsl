Texture2D TextureFinal : register(t0);

SamplerState fSampler : register(s0);

struct PS_POSTFX_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};


float4 PS(PS_POSTFX_INPUT input) : SV_Target
{
	float4 finalDeferredResult = TextureFinal.Sample(fSampler, input.Tex);

    return finalDeferredResult;
}
