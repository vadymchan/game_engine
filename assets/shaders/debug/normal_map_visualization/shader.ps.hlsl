struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD1;
};

#if !TODO
Texture2D<float4> NormalTexture : register(t0, space1);
#endif

SamplerState DefaultSampler : register(s0, space2);

float4 main(PSInput input) : SV_TARGET
{
    float3 normalColor = NormalTexture.Sample(DefaultSampler, input.TexCoord).xyz;
    return float4(normalColor, 1.0);
}
