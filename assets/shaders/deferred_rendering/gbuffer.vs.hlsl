#include "../common.hlsli"

#if USE_SPHERICAL_MAP
#include "../shperical_map.hlsl"
#endif

#ifndef USE_VERTEX_COLOR
#define USE_VERTEX_COLOR 0
#endif

#ifndef USE_VERTEX_BITANGENT
#define USE_VERTEX_BITANGENT 0
#endif

struct VSInput
{
    [[vk::location((0))]] float3 Position : POSITION0;
#if USE_VERTEX_COLOR
    [[vk::location((1))]] float4 Color : COLOR0;
#endif
    [[vk::location((1+USE_VERTEX_COLOR))]] float3 Normal : NORMAL0;
    [[vk::location((2+USE_VERTEX_COLOR))]] float3 Tangent : TANGENT0;
#if USE_VERTEX_BITANGENT
    [[vk::location((3+USE_VERTEX_COLOR))]] float3 Bitangent : BITANGENT0;
#endif
    [[vk::location((3+USE_VERTEX_COLOR+USE_VERTEX_BITANGENT))]] float2 TexCoord : TEXCOORD0;
};

cbuffer ViewParam : register(b0, space0)
{
    ViewUniformBuffer ViewParam;
}
cbuffer RenderObjectParam : register(b0, space1)
{
    RenderObjectUniformBuffer RenderObjectParam;
}

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float3 LocalPos : POSITION0;
#if USE_VERTEX_COLOR
    float4 Color : COLOR0;
#endif
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL0;
    float4 WorldPos : TEXCOORD1;
    float3x3 TBN : TEXCOORD2;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    output.LocalPos = input.Position;
    output.WorldPos = mul(RenderObjectParam.M, float4(input.Position, 1.0));
    output.Pos = mul(ViewParam.VP, output.WorldPos);
#if USE_VERTEX_COLOR
    output.Color = input.Color;
#endif

    float2 UV = 0;
#if USE_SPHERICAL_MAP
    output.TexCoord = GetSphericalMap_TwoMirrorBall(normalize(input.Position));
#else
    output.TexCoord = input.TexCoord;
#endif
    output.Normal = mul((float3x3) RenderObjectParam.M, input.Normal);
    
#if USE_ALBEDO_TEXTURE
#if USE_VERTEX_BITANGENT
    float3 Bitangent = input.Bitangent;
#else
    float3 Bitangent = cross(input.Normal, input.Tangent);
#endif // USE_VERTEX_BITANGENT

    float3 T = normalize(mul((float3x3)RenderObjectParam.M, input.Tangent).xyz);
    float3 B = normalize(mul((float3x3)RenderObjectParam.M, Bitangent).xyz);
    float3 N = normalize(mul((float3x3)RenderObjectParam.M, input.Normal).xyz);
    output.TBN = float3x3(T, B, N);
    output.TBN = transpose(output.TBN);
#endif

    return output;
}
