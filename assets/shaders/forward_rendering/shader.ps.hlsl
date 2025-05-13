
struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT0;
    float3 Bitangent : BITANGENT0;
    float4 Color : COLOR0;
};

Texture2D DiffuseTexture : register(t0, space1);
SamplerState DiffuseTextureSampler : register(s0, space1);
Texture2D NormalTexture : register(t1, space1);
SamplerState NormalTextureSampler : register(s1, space1);
Texture2D RoughnessTexture : register(t2, space1); 
SamplerState RoughnessTextureSampler : register(s2, space1);
Texture2D MetalicTexture : register(t3, space1);
SamplerState MetailcTextureSampler : register(s3, space1);

float4 main(PSInput input) : SV_TARGET
{
    //return float4(1.0, 0.0, 0.0, 1.0);
    //return input.Color;
    
    float4 diffuseColor = DiffuseTexture.Sample(DiffuseTextureSampler, input.TexCoord);
    
    float3 normalColor = NormalTexture.Sample(NormalTextureSampler, input.TexCoord).rgb;

    float roughnessValue = RoughnessTexture.Sample(RoughnessTextureSampler, input.TexCoord).r;

    float metalicValue = MetalicTexture.Sample(MetailcTextureSampler, input.TexCoord).r;

    return diffuseColor;
    return float4(diffuseColor);
}
