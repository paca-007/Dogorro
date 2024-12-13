struct VIN
{
	float3 PosL    : POSITION;
	float2 Tex     : TEXCOORD;
};

struct VOUT
{
	float4 pos    : SV_POSITION;
	float2 Tex     : TEXCOORD;
};

cbuffer uiPosition : register(b0)
{
	float dx;
	float dy;
	float2 padding;
}

VOUT VS(VIN _vin)
{
	VOUT vout;
	vout.pos = float4(_vin.PosL, 1.0f);
	vout.pos.x += dx;
	vout.pos.y += dy;
	vout.Tex = _vin.Tex;

    return vout;
}