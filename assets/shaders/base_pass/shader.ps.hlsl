struct PSInput
{
    float4 Position  : SV_POSITION;  
    float2 TexCoord  : TEXCOORD1;    
    float3 Normal    : NORMAL2;        
    float3 Tangent   : TANGENT3;      
    float3 Bitangent : BITANGENT4;  
    float4 Color     : COLOR5;          
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

// TODO: Currently only one type of light (consider in future to be able to dynamically add lights) - use StructuredBuffer
//cbuffer DirectionalLightBuffer : register(b0, space2)
//{
//    DirectionalLightData directionalLight;
//};

//cbuffer PointLightBuffer : register(b0, space3)
//{
//    PointLightData pointLight;
//};

//cbuffer SpotLightBuffer : register(b0, space4)
//{
//    SpotLightData spotLight;
//};

cbuffer LightCounts : register(b0, space2)
{
    uint directionalLightCount;
    uint pointLightCount;
    uint spotLightCount;
    uint padding;
}

StructuredBuffer<DirectionalLightData> directionalLights : register(t1, space2);
StructuredBuffer<PointLightData> pointLights : register(t2, space2);
StructuredBuffer<SpotLightData> spotLights : register(t3, space2);

struct MaterialParams
{
    float4 baseColor;
    float metallic;
    float roughness;
    float opacity;
    float padding;
};

cbuffer MaterialBuffer : register(b0, space3) 
{
    MaterialParams material;
}

Texture2D<float4> DiffuseTexture : register(t1, space3);
Texture2D<float4> NormalTexture : register(t2, space3);
Texture2D<float4> MetallicRoughnessTexture : register(t3, space3);
//Texture2D<float4> RoughnessTexture : register(t2, space5);
//Texture2D<float4> MetalicTexture : register(t3, space5);

SamplerState DefaultSampler : register(s0, space4);



float4 main(PSInput input) : SV_TARGET
{
    
    //return float4(1.0, 0.0, 0.0, 1.0);
    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    
    //color += float4(directionalLight.color, 0);
    //color += float4(pointLight.color, 0);
    //color += float4(spotLight.color, 0);

    
    
    //return color;
    
    float4 diffuseColor = DiffuseTexture.Sample(DefaultSampler, input.TexCoord);

    color += diffuseColor;
    
    float3 normalColor = NormalTexture.Sample(DefaultSampler, input.TexCoord).rgb;

    color += float4(normalColor, 0);
    
    float2 metallicRoughnessValue = MetallicRoughnessTexture.Sample(DefaultSampler, input.TexCoord).gb;

    color += float4(metallicRoughnessValue, 0, 0);
    
    //float roughnessValue = RoughnessTexture.Sample(DefaultSampler, input.TexCoord).r;

    //color += float4(roughnessValue, 0, 0, 0);
    
    //float metalicValue = MetalicTexture.Sample(DefaultSampler, input.TexCoord).r;

    //color += float4(metalicValue, 0, 0, 0);
    
    //return color;
    
    for(uint i = 0; i < directionalLightCount; i++)
    {
        color += float4(directionalLights[i].color, 0);
    }
    
    for (uint j = 0; j < pointLightCount; j++)
    {
        color += float4(pointLights[j].color, 0);
    }
    
    for (uint k = 0; k < spotLightCount; k++)
    {
      color += float4(spotLights[k].color, 0);
    }
    
    //return color;
    
    return float4(directionalLightCount, spotLightCount, pointLightCount , 0);
    return material.baseColor;
    //return input.Color;
    //return float4(normalColor, 1.0);
    return diffuseColor;
    //return float4(diffuseColor.rgb, 1);
    return float4(metallicRoughnessValue, 0, 1);
    return float4(metallicRoughnessValue.r, 0, 0, 1);
    return float4(metallicRoughnessValue.g, 0, 0, 1);
}
