Texture2D NormalBuffer : register(t0);

SamplerState f_Sampler : register(s0);

struct PS_POSTFX_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};

float4 PS( PS_POSTFX_INPUT input) : SV_Target
{
    //임계값 
    const float threshold = 0.05;//0.05;
    const float edgeThickness = 0.99; // 두께 조절 가능

    const int2 texAddrOffsets[8] = {
            int2(-1, -1), 
            int2( 0, -1),
            int2( 1, -1),
            int2(-1,  0),
            int2( 1,  0),
            int2(-1,  1),
            int2( 0,  1),
            int2( 1,  1),
    };

    float lum[8];
    int i;

    float3 LuminanceConv = { 0.3f, 0.59f, 0.11f };

    uint width, height, levels;

    NormalBuffer.GetDimensions(0, width, height, levels);

    for (i=0; i < 8; i++) {
      float3 colour = NormalBuffer.Load( int3(int2(input.Tex * float2(width, height)) + texAddrOffsets[i], 0));
      lum[i] = dot(colour, LuminanceConv);
    }

    float x = lum[0] + 2 * lum[3] + lum[5] - lum[2] - 2 * lum[4] - lum[7];
    float y = lum[0] + 2 * lum[1] + lum[2] - lum[5] - 2 * lum[6] - lum[7];
    float edge = sqrt(x*x + y*y);

    float4 DTexture = NormalBuffer.Sample(f_Sampler, input.Tex);

    return (edge > threshold) ? float4(1, 1, 1, 1) : float4(0,0,0,0);
    
}
