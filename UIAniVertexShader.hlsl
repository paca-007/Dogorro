cbuffer AnimationData : register(b0)
{
    float width;        // ���� ������ ��ü ����
    float aniWidth;      // �� �������� ���� 
    float totalFrames;  // ��ü ������ ��
    float frameIndex;   // ���� ������ �ε���
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

    // ��ǥ ���    
    float uvLeft = left / width;  
    float uvRight = right / width;

    //����
    float uvTop = 0.f;
    float uvBottom = 1.f;

    // ��Ȯ�� �߰� �� ���
    float u = input.Tex.x * (uvRight - uvLeft) + uvLeft;
    float v = input.Tex.y * (uvBottom - uvTop) + uvTop;

    // �� �������� �ؽ�ó ��ǥ ���
    output.Tex = float2(u, v);

    return output;
}