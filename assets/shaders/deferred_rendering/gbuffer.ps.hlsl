#include "../common.hlsli"

#ifndef USE_VERTEX_COLOR
#define USE_VERTEX_COLOR 0
#endif

#ifndef USE_FLIP_NORMALMAP_Y
#define USE_FLIP_NORMALMAP_Y 1      // todo : apply it at loading time of the normal map
#endif

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

cbuffer ViewParam : register(b0, space0)
{
    ViewUniformBuffer ViewParam;
}
cbuffer RenderObjectParam : register(b0, space1)
{
    RenderObjectUniformBuffer RenderObjectParam;
}
#if USE_ALBEDO_TEXTURE
Texture2D DiffuseTexture : register(t0, space2);
SamplerState DiffuseTextureSampler : register(s0, space2);
Texture2D NormalTexture : register(t1, space2);
SamplerState NormalTextureSampler : register(s1, space2);
Texture2D RMTexture : register(t2, space2);     // Metalic, Roughness
SamplerState RMTextureSampler : register(s2, space2);
TextureCube EnvTexture : register(t3, space2);
SamplerState EnvextureSampler : register(s3, space2);
#endif

struct PushConsts
{
    float4 Color;
    bool ShowVRSArea;
    bool ShowGrid;
};
[[vk::push_constant]] PushConsts pushConsts;

struct FSOutput
{
    float4 GBuffer0 : SV_TARGET0; // Position.xyz
    float4 GBuffer1 : SV_TARGET1; // Normal.xyz
    float4 GBuffer2 : SV_TARGET2; // Albedo.xyz
};

FSOutput main(VSOutput input
#if USE_VARIABLE_SHADING_RATE
    , uint shadingRate : SV_ShadingRate
#endif
)
{
    float4 color = 1;
#if USE_VERTEX_COLOR
    color *= input.Color;
#endif

#if USE_ALBEDO_TEXTURE
    color *= DiffuseTexture.Sample(DiffuseTextureSampler, input.TexCoord.xy);
    if (color.w < 0.5f)
    {
        discard;
    }

#if USE_SRGB_ALBEDO_TEXTURE
    // Convert to linear space
    color.rgb = pow(color.rgb, 2.2);
#endif // USE_SRGB_ALBEDO_TEXTURE

    float3 normal = NormalTexture.Sample(NormalTextureSampler, input.TexCoord.xy).xyz;
#if USE_FLIP_NORMALMAP_Y
    normal.y = 1.0 - normal.y;
#endif // USE_FLIP_NORMALMAP_Y
    normal = normal * 2.0f - 1.0f;
    float3 WorldNormal = normalize(mul(input.TBN, normal));

#else // USE_ALBEDO_TEXTURE
    float3 WorldNormal = normalize(input.Normal);
#endif // USE_ALBEDO_TEXTURE

#if USE_VARIABLE_SHADING_RATE
    if (pushConsts.ShowVRSArea)
    {
        const uint SHADING_RATE_PER_PIXEL = 0x0;
        const uint SHADING_RATE_PER_2X1_PIXELS = 6;
        const uint SHADING_RATE_PER_1X2_PIXELS = 7;
        const uint SHADING_RATE_PER_2X2_PIXELS = 8;
        const uint SHADING_RATE_PER_4X2_PIXELS = 9;
        const uint SHADING_RATE_PER_2X4_PIXELS = 10;

        switch (shadingRate)
        {
            case SHADING_RATE_PER_PIXEL:
                color *= float4(0.0, 0.8, 0.4, 1.0);
            case SHADING_RATE_PER_2X1_PIXELS:
                color *= float4(0.2, 0.6, 1.0, 1.0);
            case SHADING_RATE_PER_1X2_PIXELS:
                color *= float4(0.0, 0.4, 0.8, 1.0);
            case SHADING_RATE_PER_2X2_PIXELS:
                color *= float4(1.0, 1.0, 0.2, 1.0);
            case SHADING_RATE_PER_4X2_PIXELS:
                color *= float4(0.8, 0.8, 0.0, 1.0);
            case SHADING_RATE_PER_2X4_PIXELS:
                color *= float4(1.0, 0.4, 0.2, 1.0);
            default:
                color *= float4(0.8, 0.0, 0.0, 1.0);
        }
    }
#endif

    //if (pushConsts.ShowGrid)
    //{
    //    float2 center = (input.DirectionalLightShadowPosition.xy * 0.5 + 0.5);

    //    float resolution = 100.0;
    //    float cellSpace = 1.0;
    //    float2 cells = abs(frac(center * resolution / cellSpace) - 0.5);

    //    float distToEdge = (0.5 - max(cells.x, cells.y)) * cellSpace;

    //    float lineWidth = 0.1;
    //    float lines = smoothstep(0.0, lineWidth, distToEdge);
    //    color = lerp(float4(0.0, 1.0, 0.0, 1.0), color, lines);
    //}

    FSOutput output = (FSOutput) 0;

    output.GBuffer0.xyz = input.WorldPos.xyz / input.WorldPos.w;
#if USE_PBR
#if USE_ALBEDO_TEXTURE
    float roughness = RMTexture.Sample(RMTextureSampler, input.TexCoord.xy).y;
    float metallic = RMTexture.Sample(RMTextureSampler, input.TexCoord.xy).z;
    output.GBuffer0.w = metallic;
#else
    output.GBuffer0.w = RenderObjectParam.Metallic;
#endif
#endif
    
    output.GBuffer1.xyz = WorldNormal;
#if USE_PBR
#if USE_ALBEDO_TEXTURE
    output.GBuffer1.w = roughness;
#else
    output.GBuffer1.w = RenderObjectParam.Roughness;
#endif
#endif
    
    output.GBuffer2.xyz = color.xyz;

    return output;
}
