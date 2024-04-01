#include "../common.hlsli"

#define USE_HARDCODED_VALUES 1

#if USE_HARDCODED_VALUES

struct VSInput
{
    uint VertexID : SV_VertexID;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

    // Hardcoded triangle vertices
    float4 positions[3] =
    {
        float4(-0.5f, -0.5f, 0.0f, 1.0f),
        float4(0.5f, -0.5f, 0.0f, 1.0f),
        float4(0.0f, 0.5f, 0.0f, 1.0f)
    };

    output.Pos = positions[input.VertexID];

    return output;
}

#else

struct VSInput
{
    [[vk::location(0)]] float3 Position : POSITION;
    [[vk::location(1)]] float4 Color : COLOR;
    [[vk::location(2)]] float3 Normal : NORMAL;
    [[vk::location(3)]] float3 Tangent : TANGENT;
    [[vk::location(4)]] float2 TexCoord : TEXCOORD;
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
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

    output.Pos = mul(ViewParam.VP, mul(RenderObjectParam.M, float4(input.Position, 1.0)));
    output.Color = input.Color;
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;
    output.TexCoord = input.TexCoord;

    return output;
}

#endif