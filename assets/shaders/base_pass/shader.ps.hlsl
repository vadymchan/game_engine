struct PSInput
{
    float4 Position : SV_POSITION;  
    float3 Normal : NORMAL1;        
    float2 TexCoord : TEXCOORD2;    
    float3 Tangent : TANGENT3;      
    float3 Bitangent : BITANGENT4;  
    float4 Color : COLOR5;          
};

struct ViewUniformBuffer
{
    float4x4 V;
    float4x4 P;
    float4x4 VP;
    float3 EyeWorld;
    float padding0;
};

cbuffer ViewParam : register(b0, space0)
{
    ViewUniformBuffer ViewParam;
}

#if !TODO 
struct DirectionalLightData
{
    // From Light
    float3 color;
    float intensity;

    // From DirectionalLight
    float3 direction;
    
    float padding;
};

struct PointLightData
{
    // From Light
    float3 color;
    float intensity;

    // From PointLight
    float range;

    // From Transform
    float3 position;
};

struct SpotLightData
{
    // From Light
    float3 color;
    float intensity;

    // From PointLight
    float range;
    float innerConeAngle;
    float outerConeAngle;
    float padding1;

    // From Transform
    float3 position;
    float padding2;
    float3 direction;
    float padding3;
};


// TODO: Currently only one type of light (consider in future to be able to dynamically add lights)
cbuffer DirectionalLightBuffer : register(b0, space1)
{
    DirectionalLightData directionalLight;
};

cbuffer PointLightBuffer : register(b0, space2)
{
    PointLightData pointLight;
};

cbuffer SpotLightBuffer : register(b0, space3)
{
    SpotLightData spotLight;
};
#endif

#if !TODO
Texture2D<float4> DiffuseTexture : register(t0, space4);
//SamplerState DiffuseTextureSampler : register(s0, space4);

Texture2D<float4> NormalTexture : register(t1, space4);
//SamplerState NormalTextureSampler : register(s1, space4);

Texture2D<float4> RoughnessTexture : register(t2, space4);
//SamplerState RoughnessTextureSampler : register(s2, space4);

Texture2D<float4> MetalicTexture : register(t3, space4);
//SamplerState MetailcTextureSampler : register(s3, space4);

#endif

SamplerState DefaultSampler : register(s0, space5);



float4 main(PSInput input) : SV_TARGET
{
    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    
    color += float4(directionalLight.color, 0);
    color += float4(pointLight.color, 0);
    color += float4(spotLight.color, 0);
    return color;

    float4 diffuseColor = DiffuseTexture.Sample(DefaultSampler, input.TexCoord);

    float3 normalColor = NormalTexture.Sample(DefaultSampler, input.TexCoord).rgb;

    float roughnessValue = RoughnessTexture.Sample(DefaultSampler, input.TexCoord).r;

    float metalicValue = MetalicTexture.Sample(DefaultSampler, input.TexCoord).r;

    return input.Color;
    return float4(normalColor, 1.0);
    return diffuseColor;
    return float4(roughnessValue, 0, 0, 0);
    return float4(metalicValue, 0, 0, 0);
}
