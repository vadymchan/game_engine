#define PI 3.14159265

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD1;
    float3 Normal : NORMAL2;
    float3 Tangent : TANGENT3;
    float3 Bitangent : BITANGENT4;
    float4 Color : COLOR5;
    float3 WorldPos : TEXCOORD0;
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
SamplerState DefaultSampler : register(s0, space4);

float D_GGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; 
    float ggx1 = GeometrySchlickGGX(saturate(dot(N, V)), k);
    float ggx2 = GeometrySchlickGGX(saturate(dot(N, L)), k);
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 CalcDirectional(DirectionalLightData light, float3 N, float3 V,
                       float3 albedo, float metallic, float roughness)
{
    float3 L = normalize(-light.direction);
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(N, L));
    if (NdotL <= 0.0)
        return 0;

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 F = FresnelSchlick(saturate(dot(H, V)), F0);
    float D = D_GGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 spec = D * G * F / max(4.0 * saturate(dot(N, V)) * NdotL, 1e-4);

    float3 kd = (1.0 - F) * (1.0 - metallic);
    float3 diff = kd * albedo / PI;

    float3 radiance = light.color * light.intensity;
    return (diff + spec) * radiance * NdotL;
}

float3 CalcPoint(PointLightData light, float3 N, float3 V, float3 worldPos,
                 float3 albedo, float metallic, float roughness)
{
    float3 Lvec = light.position - worldPos;
    float dist = length(Lvec);
    if (dist >= light.range)
        return 0;

    float attenuation = pow(1.0 - saturate(dist / light.range), 2.0);
    float3 L = Lvec / dist;
    float3 H = normalize(V + L);

    float NdotL = saturate(dot(N, L));
    if (NdotL <= 0.0)
        return 0;

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 F = FresnelSchlick(saturate(dot(H, V)), F0);
    float D = D_GGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 spec = D * G * F / max(4.0 * saturate(dot(N, V)) * NdotL, 1e-4);

    float3 kd = (1.0 - F) * (1.0 - metallic);
    float3 diff = kd * albedo / PI;

    float3 radiance = light.color * light.intensity * attenuation;
    return (diff + spec) * radiance * NdotL;
}

float3 CalcSpot(SpotLightData light, float3 N, float3 V, float3 worldPos,
                float3 albedo, float metallic, float roughness)
{
    float3 Lvec = light.position - worldPos;
    float dist = length(Lvec);
    if (dist >= light.range)
        return 0;

    float3 L = Lvec / dist;
    float cosOuter = cos(radians(light.outerConeAngle));
    float cosInner = cos(radians(light.innerConeAngle));
    float cosDir = dot(-L, normalize(light.direction));
    if (cosDir < cosOuter)
        return 0;

    float spot = smoothstep(cosOuter, cosInner, cosDir);
    float attenuation = pow(1.0 - saturate(dist / light.range), 2.0) * spot;

    float3 H = normalize(V + L);
    float NdotL = saturate(dot(N, L));
    if (NdotL <= 0.0)
        return 0;

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);
    float3 F = FresnelSchlick(saturate(dot(H, V)), F0);
    float D = D_GGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 spec = D * G * F / max(4.0 * saturate(dot(N, V)) * NdotL, 1e-4);

    float3 kd = (1.0 - F) * (1.0 - metallic);
    float3 diff = kd * albedo / PI;

    float3 radiance = light.color * light.intensity * attenuation;
    return (diff + spec) * radiance * NdotL;
}


// PBR
float4 main(PSInput input) : SV_TARGET
{

    float3 albedo = DiffuseTexture.Sample(DefaultSampler, input.TexCoord).rgb *
                    material.baseColor.rgb;
    float2 mr = MetallicRoughnessTexture.Sample(DefaultSampler, input.TexCoord).gb;
    float metallic = saturate(mr.x * material.metallic);
    float roughness = saturate(mr.y * material.roughness);

    float3 Nmap = NormalTexture.Sample(DefaultSampler, input.TexCoord).rgb * 2.0 - 1.0;
    float3 T = normalize(input.Tangent);
    float3 B = normalize(input.Bitangent);
    float3 N = normalize(input.Normal);
    float3x3 TBN = float3x3(T, B, N);
    N = normalize(mul(Nmap, TBN));

    float3 V = normalize(ViewParam.EyeWorld - input.WorldPos);

    float3 color = float3(0, 0, 0);

    // Directional
    for (uint i = 0; i < directionalLightCount; ++i)
        color += CalcDirectional(directionalLights[i], N, V, albedo, metallic, roughness);

    // Point
    for (uint k = 0; k < pointLightCount; ++k)
        color += CalcPoint(pointLights[k], N, V, input.WorldPos, albedo, metallic, roughness);

    // Spot
    for (uint j = 0; j < spotLightCount; ++j)
        color += CalcSpot(spotLights[j], N, V, input.WorldPos, albedo, metallic, roughness);

    color += albedo * 0.03;

    return float4(color, material.opacity);
}


//float4 main(PSInput input) : SV_TARGET
//{
//    float3 Nmap = NormalTexture.Sample(DefaultSampler, input.TexCoord).rgb * 2.0 - 1.0;
//    float3 T = normalize(input.Tangent);
//    float3 B = normalize(input.Bitangent);
//    float3 N = normalize(input.Normal);
//    float3x3 TBN = float3x3(T, B, N);
//    N = normalize(mul(Nmap, TBN));
    
//    float3 color = float3(0.0, 0.0, 0.0);

//    /* Directional */
//    for (uint i = 0; i < directionalLightCount; ++i)
//    {
//        float3 L = normalize(-directionalLights[i].direction);
//        float NdotL = saturate(dot(N, L));
//        color += directionalLights[i].color * directionalLights[i].intensity * NdotL;
//    }

//    /* Point */
//    for (uint i = 0; i < pointLightCount; ++i)
//    {
//        float3 Lvec = pointLights[i].position - input.WorldPos;
//        float dist = length(Lvec);
//        if (dist < pointLights[i].range)
//        {
//            float3 L = Lvec / dist;
//            float atten = pow(1.0 - saturate(dist / pointLights[i].range), 2.0);
//            float NdotL = saturate(dot(N, L));
//            color += pointLights[i].color * pointLights[i].intensity * NdotL * atten;
//        }
//    }

//    /* Spot */
//    for (uint i = 0; i < spotLightCount; ++i)
//    {
//        float3 Lvec = spotLights[i].position - input.WorldPos;
//        float dist = length(Lvec);
//        if (dist < spotLights[i].range)
//        {
//            float3 L = Lvec / dist;
//            float NdotL = saturate(dot(N, L));

//            float cosDir = dot(-L, normalize(spotLights[i].direction));
//            float innerCos = cos(radians(spotLights[i].innerConeAngle));
//            float outerCos = cos(radians(spotLights[i].outerConeAngle));
//            float spot = smoothstep(outerCos, innerCos, cosDir);

//            float atten = pow(1.0 - saturate(dist / spotLights[i].range), 2.0) * spot;

//            color += spotLights[i].color * spotLights[i].intensity * NdotL * atten;
//        }
//    }

//    return float4(color, 1.0);
//}
