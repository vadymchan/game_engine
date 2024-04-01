#include "../common.hlsli"

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSITION0;
    [[vk::location(1)]] float4 Color : COLOR0;
    [[vk::location(2)]] float3 Normal : NORMAL0;
    [[vk::location(3)]] float3 Tangent : NORMAL1;
    [[vk::location(4)]] float2 TexCoord : TEXCOORD0;
};

cbuffer ViewParam : register(b0, space0)
{
    ViewUniformBuffer ViewParam;
}

cbuffer DirectionalLight : register(b0, space1)
{
    DirectionalLightUniformBuffer DirectionalLight;
}
Texture2D DirectionalLightShadowMap : register(t1, space1);
SamplerState DirectionalLightShadowMapSampler : register(s1, space1);

//cbuffer PointLight : register(b0, space2) { PointLightUniformBufferData PointLight; }
//TextureCube PointLightShadowCubeMap : register(t1, space2);
//SamplerState PointLightShadowMapSampler : register(s1, space2);

cbuffer SpotLight : register(b0, space3)
{
    SpotLightUniformBufferData SpotLight;
}
TextureCube SpotLightShadowCubeMap : register(t1, space3);
SamplerState SpotLightShadowMapSampler : register(s1, space3);

cbuffer RenderObjectParam : register(b0, space4)
{
    RenderObjectUniformBuffer RenderObjectParam;
}


struct VSOutput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL0;
    float4 DirectionalLightShadowPosition : TEXCOORD1;
    float4 SpotLightShadowPosition : TEXCOORD2;
    float4 WorldPos : TEXCOORD3;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    output.WorldPos = mul(RenderObjectParam.M, float4(input.Position, 1.0));
    output.Pos = mul(ViewParam.VP, output.WorldPos);
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;

    output.Normal = normalize(mul((float3x3) RenderObjectParam.M, input.Normal));

    output.DirectionalLightShadowPosition = mul(DirectionalLight.ShadowVP, output.WorldPos);
    output.SpotLightShadowPosition = mul(SpotLight.ShadowVP, output.WorldPos);
    return output;
}
