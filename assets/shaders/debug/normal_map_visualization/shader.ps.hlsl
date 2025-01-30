struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT0;
    float3 Bitangent : BITANGENT0;
    float4 Color : COLOR0;
};

#if !TODO
Texture2D<float4> NormalTexture : register(t0, space1);
SamplerState NormalTextureSampler : register(s0, space1);
#endif

float4 main(PSInput input) : SV_TARGET
{
    float3 normalColor = NormalTexture.Sample(NormalTextureSampler, input.TexCoord).xyz;
    return float4(normalColor, 1.0);
}
