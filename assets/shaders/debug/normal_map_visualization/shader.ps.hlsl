struct PSInput
{
    float4 Position  : SV_POSITION;
    float2 TexCoord  : TEXCOORD1;
    float3 Normal    : NORMAL2;
    float3 Tangent   : TANGENT3;
    float3 Bitangent : BITANGENT4;
};

Texture2D<float4> NormalTexture : register(t0, space2);

SamplerState DefaultSampler : register(s0, space3);

float4 main(PSInput input) : SV_TARGET
{
    float3 normal = input.Normal * 0.5 + 0.5;  
    //return float4(normal, 1);
    
    float3 Nmap = NormalTexture.Sample(DefaultSampler, input.TexCoord).rgb * 2.0 - 1.0;
    float3 T = normalize(input.Tangent);
    float3 B = normalize(input.Bitangent);
    float3 N = normalize(input.Normal);
    float3x3 TBN = float3x3(T, B, N);
    
    N = normalize(mul(Nmap, TBN));
    
    return float4(N, 1.0);
}
