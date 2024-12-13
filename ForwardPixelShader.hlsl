// �ؽ�ó ����
Texture2D fTexture : register(t0);
SamplerState f_Sampler : register(s0);

// ���̴� �Է� ����ü
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
    
    // �ؽ�ó ���ø�
    float4 color = fTexture.Sample(f_Sampler, pin.Texf);
 
    // ���� ���� 
    color.a *= 0.5; // ���� ���� �����Ͽ� ������ ȿ���� ���� �� ����
    result.Color = color;
    
    //�� ���� ��� 
	float d = pin.PosH.z / pin.PosH.w;
	result.zDepth = float4(d, d, d, 0);
    
    return result;
}
