#include "../common.hlsli"

#define USE_HARDCODED_VALUES 1

#if USE_HARDCODED_VALUES

struct PSInput
{
    float4 Pos : SV_POSITION;
};

float4 main(PSInput input) : SV_TARGET
{
// Hardcoded color for testing
    float4 color = float4(1.0f, 0.0f, 0.0f, 1.0f); // Red color
    return color;
}

#else

struct PSInput
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

cbuffer ViewParam : register(b0, space0)
{
    ViewUniformBuffer ViewParam;
}

cbuffer RenderObjectParam : register(b0, space1)
{
    RenderObjectUniformBuffer RenderObjectParam;
}

float4 main(PSInput input) : SV_TARGET
{
    float4 finalColor = input.Color;
    return finalColor;
}

#endif