struct PS_POSTFX_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

cbuffer MyConstantBuffer : register(b0)
{
    float Time;
	float Padding[15];
};


float4 PS(PS_POSTFX_INPUT input) : SV_Target
{
	float x = 1.0 * abs(sin(Time * 1000.0));
	
    return float4(x, x, x, x);

}

