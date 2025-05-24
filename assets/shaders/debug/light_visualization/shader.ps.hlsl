struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD1; 
    float3 Normal : NORMAL2;
    float3 Tangent : TANGENT3;
    float3 Bitangent : BITANGENT4;
    float3 WorldPos : TEXCOORD0;
};

struct ViewUniformBuffer
{
    float4x4 V;
    float4x4 P;
    float4x4 VP;
    float4x4 InvV;
    float4x4 InvP;
    float4x4 InvVP;
    float3 EyeWorld;
    float padding0;
};

cbuffer ViewParam : register(b0, space0)
{
    ViewUniformBuffer ViewParam;
}

struct DirectionalLightData
{
    float3 color;
    float intensity;
    float3 direction;
    float padding;
};
struct PointLightData
{
    float3 color;
    float intensity;
    float range;
    float3 position;
};
struct SpotLightData
{
    float3 color;
    float intensity;
    float range;
    float innerConeAngle;
    float outerConeAngle;
    float padding1;
    float3 position;
    float padding2;
    float3 direction;
    float padding3;
};

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

Texture2D<float4> NormalTexture : register(t0, space3);
SamplerState DefaultSampler : register(s0, space4);

float4 main(PSInput input) : SV_TARGET
{
    float3 Nmap = NormalTexture.Sample(DefaultSampler, input.TexCoord).rgb * 2.0 - 1.0;
    float3 T = normalize(input.Tangent);
    float3 B = normalize(input.Bitangent);
    float3 N = normalize(input.Normal);
    float3x3 TBN = float3x3(T, B, N);
    N = normalize(mul(Nmap, TBN));
    
    float3 color = float3(0.0, 0.0, 0.0);

    /* Directional */
    for (uint i = 0; i < directionalLightCount; ++i)
    {
        float3 L = normalize(-directionalLights[i].direction);
        float NdotL = saturate(dot(N, L));
        color += directionalLights[i].color * directionalLights[i].intensity * NdotL;
    }

    /* Point */
    for (uint j = 0; j < pointLightCount; ++j)
    {
        float3 Lvec = pointLights[j].position - input.WorldPos;
        float dist = length(Lvec);
        if (dist < pointLights[j].range)
        {
            float3 L = Lvec / dist;
            float atten = pow(1.0 - saturate(dist / pointLights[j].range), 2.0);
            float NdotL = saturate(dot(N, L));
            color += pointLights[j].color * pointLights[j].intensity * NdotL * atten;
        }
    }

    /* Spot */
    for (uint k = 0; k < spotLightCount; ++k)
    {
        float3 Lvec = spotLights[k].position - input.WorldPos;
        float dist = length(Lvec);
        if (dist < spotLights[k].range)
        {
            float3 L = Lvec / dist;
            float NdotL = saturate(dot(N, L));

            float cosDir = dot(-L, normalize(spotLights[k].direction));
            float innerCos = cos(radians(spotLights[k].innerConeAngle));
            float outerCos = cos(radians(spotLights[k].outerConeAngle));
            float spot = smoothstep(outerCos, innerCos, cosDir);

            float atten = pow(1.0 - saturate(dist / spotLights[k].range), 2.0) * spot;

            color += spotLights[k].color * spotLights[k].intensity * NdotL * atten;
        }
    }

    return float4(color, 1.0);
}
