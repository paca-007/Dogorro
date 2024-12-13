cbuffer AnimationData : register(b0)
{
    float width;        // 가로 프레임 전체 넓이
    float aniWidth;      // 각 프레임의 넓이 
    float totalFrames;  // 전체 프레임 수
    float frameIndex;   // 현재 프레임 인덱스
};

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

VOUT VS(VIN input)
{
    VOUT output;
    output.pos = float4(input.PosL, 1.0f);

    float left = frameIndex * aniWidth;     
    float right = (frameIndex + 1) * aniWidth;
    float top = 0.0f;
    float bottom = 1.0f;

    // 좌표 계산    
    float uvLeft = left / width;  
    float uvRight = right / width;

    //고정
    float uvTop = 0.f;
    float uvBottom = 1.f;

    // 정확한 중간 값 계산
    float u = input.Tex.x * (uvRight - uvLeft) + uvLeft;
    float v = input.Tex.y * (uvBottom - uvTop) + uvTop;

    // 각 프레임의 텍스처 좌표 계산
    output.Tex = float2(u, v);

    return output;
}