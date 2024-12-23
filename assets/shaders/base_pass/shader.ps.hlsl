//#include "../common.hlsli"

//#ifndef USE_VARIABLE_SHADING_RATE
//#define USE_VARIABLE_SHADING_RATE 0
//#endif

//struct VSOutput
//{
//    float4 Pos : SV_POSITION;
//    float4 Color : COLOR0;
//    float2 TexCoord : TEXCOORD0;
//    float3 Normal : NORMAL0;
//    float4 DirectionalLightShadowPosition : TEXCOORD1;
//    float4 SpotLightShadowPosition : TEXCOORD2;
//    float4 WorldPos : TEXCOORD3;
//};

//cbuffer ViewParam : register(b0, space0)
//{
//    ViewUniformBuffer ViewParam;
//}

//cbuffer DirectionalLight : register(b0, space1)
//{
//    DirectionalLightUniformBuffer DirectionalLight;
//}
//Texture2D DirectionalLightShadowMap : register(t1, space1);
//SamplerComparisonState DirectionalLightShadowMapSampler : register(s1, space1);

//cbuffer PointLight : register(b0, space2)
//{
//    PointLightUniformBufferData PointLight;
//}
//TextureCube PointLightShadowCubeMap : register(t1, space2);
//SamplerComparisonState PointLightShadowMapSampler : register(s1, space2);

//cbuffer SpotLight : register(b0, space3)
//{
//    SpotLightUniformBufferData SpotLight;
//}
//Texture2D SpotLightShadowMap : register(t1, space3);
//SamplerComparisonState SpotLightShadowMapSampler : register(s1, space3);

//cbuffer RenderObjectParam : register(b0, space4)
//{
//    RenderObjectUniformBuffer RenderObjectParam;
//}
//Texture2D DiffuseTexture : register(t1, space4);
//SamplerState DiffuseTextureSampler : register(s1, space4);

//struct PushConsts
//{
//    float4 Color;
//    bool ShowVRSArea;
//    bool ShowGrid;
//};
//[[vk::push_constant]] PushConsts pushConsts;

//float WindowingFunction(float value, float maxValue)
//{
//    return pow(max(0.0, 1.0 - pow(value / maxValue, 4.0)), 2.0);
//}

//float DistanceAttenuation(float distance, float maxDistance)
//{
//    const float refDistance = 50.0;
//    float attenuation = (refDistance * refDistance) / ((distance * distance) + 1.0);
//    return attenuation * WindowingFunction(distance, maxDistance);
//}

//float Square(float x)
//{
//    return x * x;
//}

//float DistanceAttenuation2(float DistanceSqr, float InvMaxDistance)
//{
//    return Square(saturate(1 - Square(DistanceSqr * Square(InvMaxDistance))));
//}

//float DiretionalFalloff(float lightRadian, float penumbraRadian, float umbraRadian)
//{
//    float t = clamp((cos(lightRadian) - cos(umbraRadian)) / (cos(penumbraRadian) - cos(umbraRadian)), 0.0, 1.0);
//    return t * t;
//}

//float3 GetDirectionalLightDiffuse(DirectionalLightUniformBuffer light, float3 normal)
//{
//    return light.Color * clamp(dot(-light.Direction, normal), 0.0, 1.0) * light.DiffuseIntensity;
//}

//float3 GetDirectionalLightSpecular(DirectionalLightUniformBuffer light, float3 reflectLightDir, float3 viewDir)
//{
//    return light.Color * pow(clamp(dot(reflectLightDir, viewDir), 0.0, 1.0), light.SpecularPow) * light.SpecularIntensity;
//}

//float3 GetDirectionalLight(DirectionalLightUniformBuffer light, float3 normal, float3 viewDir)
//{
//    float3 lightDir = normalize(-light.Direction);
//    float3 reflectLightDir = 2.0 * clamp(dot(lightDir, normal), 0.0, 1.0) * normal - lightDir;
//    return (GetDirectionalLightDiffuse(light, normal) + GetDirectionalLightSpecular(light, reflectLightDir, viewDir));
//}

//float3 GetPointLightDiffuse(PointLightUniformBufferData light, float3 normal, float3 lightDir)
//{
//    return light.Color * clamp(dot(lightDir, normal), 0.0, 1.0) * light.DiffuseIntensity;
//}

//float3 GetPointLightSpecular(PointLightUniformBufferData light, float3 reflectLightDir, float3 viewDir)
//{
//    return light.Color * pow(clamp(dot(reflectLightDir, viewDir), 0.0, 1.0), light.SpecularPow) * light.SpecularIntensity;
//}

//float3 GetPointLight(PointLightUniformBufferData light, float3 normal, float3 pixelPos, float3 viewDir)
//{
//    float3 lightDir = light.Position - pixelPos;
//    float distanceSqr = dot(lightDir, lightDir);
//    lightDir = normalize(lightDir);
//    float3 reflectLightDir = 2.0 * clamp(dot(lightDir, normal), 0.0, 1.0) * normal - lightDir;

//    return (GetPointLightDiffuse(light, normal, lightDir) + GetPointLightSpecular(light, reflectLightDir, viewDir)) * DistanceAttenuation2(distanceSqr, 1.0f / light.MaxDistance);
//}

//float3 GetSpotLightDiffuse(SpotLightUniformBufferData light, float3 normal, float3 lightDir)
//{
//    return light.Color * clamp(dot(lightDir, normal), 0.0, 1.0) * light.DiffuseIntensity;
//}

//float3 GetSpotLightSpecular(SpotLightUniformBufferData light, float3 reflectLightDir, float3 viewDir)
//{
//    return light.Color * pow(clamp(dot(reflectLightDir, viewDir), 0.0, 1.0), light.SpecularPow) * light.SpecularIntensity;
//}

//float3 GetSpotLight(SpotLightUniformBufferData light, float3 normal, float3 pixelPos, float3 viewDir)
//{
//    float3 lightDir = light.Position - pixelPos;
//    float distanceSqr = dot(lightDir, lightDir);
//    lightDir = normalize(lightDir);
//    float3 reflectLightDir = 2.0 * clamp(dot(lightDir, normal), 0.0, 1.0) * normal - lightDir;

//    float lightRadian = acos(dot(lightDir, -light.Direction));

//    return (GetSpotLightDiffuse(light, normal, lightDir)
//        + GetSpotLightSpecular(light, reflectLightDir, viewDir))
//        * DistanceAttenuation2(distanceSqr, 1.0f / light.MaxDistance)
//        * DiretionalFalloff(lightRadian, light.PenumbraRadian, light.UmbraRadian);
//}

//float4 main(VSOutput input
//#if USE_VARIABLE_SHADING_RATE
//    , uint shadingRate : SV_ShadingRate
//#endif
//) : SV_TARGET
//{
//    float3 PointLightLit = 0.0f;
//    float3 ViewWorld = normalize(ViewParam.EyeWorld - input.WorldPos.xyz);

//    // Point light shadow map
//    float3 LightDir = input.WorldPos.xyz - PointLight.Position;
//    float DistanceToLight = length(LightDir);
//    if (DistanceToLight <= PointLight.MaxDistance)
//    {
//        float NormalizedDistance = DistanceToLight / PointLight.MaxDistance;

//        const float Bias = 0.005f;
//        float Shadow = PointLightShadowCubeMap.SampleCmpLevelZero(PointLightShadowMapSampler, LightDir.xyz, NormalizedDistance - Bias);
//        if (Shadow > 0.0f)
//        {
//            PointLightLit = Shadow * GetPointLight(PointLight, input.Normal, input.WorldPos.xyz, ViewWorld);
//        }
//    }

//    // Directional light shadow map
//    float3 DirectionalLightShadowPosition = input.DirectionalLightShadowPosition.xyz / input.DirectionalLightShadowPosition.w;
//    DirectionalLightShadowPosition.y = -DirectionalLightShadowPosition.y;

//    float3 DirectionalLightLit = GetDirectionalLight(DirectionalLight, input.Normal, ViewWorld);
//    if (-1.0 <= DirectionalLightShadowPosition.z && DirectionalLightShadowPosition.z <= 1.0)
//    {
//        const float Bias = 0.01f;
//        float Shadow = DirectionalLightShadowMap.SampleCmpLevelZero(
//            DirectionalLightShadowMapSampler, (DirectionalLightShadowPosition.xy * 0.5 + 0.5), (DirectionalLightShadowPosition.z - Bias));
//        DirectionalLightLit *= Shadow;
//    }

//    // Spot light shadow map
//    float3 SpotLightShadowPosition = input.SpotLightShadowPosition.xyz / input.SpotLightShadowPosition.w;
//    SpotLightShadowPosition.y = -SpotLightShadowPosition.y;

//    float3 SpotLightLit = 0.0f;
//    if (-1.0 <= SpotLightShadowPosition.z && SpotLightShadowPosition.z <= 1.0)
//    {
//        const float Bias = 0.01f;
//#if USE_REVERSEZ
//		float Shadow = 1.0f - SpotLightShadowMap.SampleCmpLevelZero(SpotLightShadowMapSampler, SpotLightShadowPosition.xy * 0.5 + 0.5, SpotLightShadowPosition.z + Bias);
//#else
//        float Shadow = SpotLightShadowMap.SampleCmpLevelZero(SpotLightShadowMapSampler, SpotLightShadowPosition.xy * 0.5 + 0.5, SpotLightShadowPosition.z - Bias);
//#endif
//        if (Shadow > 0.0f)
//        {
//            SpotLightLit = Shadow * GetSpotLight(SpotLight, input.Normal, input.WorldPos.xyz, ViewWorld);
//        }
//    }

//    float3 DiffuseColor = DiffuseTexture.Sample(DiffuseTextureSampler, input.TexCoord.xy).xyz; // *input.Color.xyz;

//    float4 color = (1.0 / 3.14) * float4(pushConsts.Color.rgb * DiffuseColor * (PointLightLit + DirectionalLightLit + SpotLightLit), 1.0);

//#if USE_VARIABLE_SHADING_RATE
//    if (pushConsts.ShowVRSArea)
//    {
//        const uint SHADING_RATE_PER_PIXEL = 0x0;
//        const uint SHADING_RATE_PER_2X1_PIXELS = 6;
//        const uint SHADING_RATE_PER_1X2_PIXELS = 7;
//        const uint SHADING_RATE_PER_2X2_PIXELS = 8;
//        const uint SHADING_RATE_PER_4X2_PIXELS = 9;
//        const uint SHADING_RATE_PER_2X4_PIXELS = 10;
    
//        switch (shadingRate)
//        {
//            case SHADING_RATE_PER_PIXEL:
//                return color * float4(0.0, 0.8, 0.4, 1.0);
//            case SHADING_RATE_PER_2X1_PIXELS:
//                return color * float4(0.2, 0.6, 1.0, 1.0);
//            case SHADING_RATE_PER_1X2_PIXELS:
//                return color * float4(0.0, 0.4, 0.8, 1.0);
//            case SHADING_RATE_PER_2X2_PIXELS:
//                return color * float4(1.0, 1.0, 0.2, 1.0);
//            case SHADING_RATE_PER_4X2_PIXELS:
//                return color * float4(0.8, 0.8, 0.0, 1.0);
//            case SHADING_RATE_PER_2X4_PIXELS:
//                return color * float4(1.0, 0.4, 0.2, 1.0);
//            default:
//                return color * float4(0.8, 0.0, 0.0, 1.0);
//        }
//    }
//#endif

//    // Draw Grid by using shadow position
//    if (pushConsts.ShowGrid)
//    {
//        float2 center = (input.DirectionalLightShadowPosition.xy * 0.5 + 0.5);
        
//        float resolution = 100.0;
//        float cellSpace = 1.0;
//        float2 cells = abs(frac(center * resolution / cellSpace) - 0.5);
        
//        float distToEdge = (0.5 - max(cells.x, cells.y)) * cellSpace;
        
//        float lineWidth = 0.1;
//        float lines = smoothstep(0.0, lineWidth, distToEdge);
//        color = lerp(float4(0.0, 1.0, 0.0, 1.0), color, lines);
//    }

//    return color;
//}

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT0;
    float3 Bitangent : BITANGENT0;
    float4 Color : COLOR0;
};



#if !TODO
Texture2D DiffuseTexture : register(t0, space4);
SamplerState DiffuseTextureSampler : register(s0, space4);
Texture2D NormalTexture : register(t1, space4);
SamplerState NormalTextureSampler : register(s1, space4);
Texture2D RoughnessTexture : register(t2, space4); 
SamplerState RoughnessTextureSampler : register(s2, space4);
Texture2D MetalicTexture : register(t3, space4);
SamplerState MetailcTextureSampler : register(s3, space4);
#endif

//struct ViewUniformBuffer1
//{
//    float4x4 V;
//    float4x4 P;
//    float4x4 VP;
//    float3 EyeWorld;
//    float padding0;
//};

//cbuffer ViewParam : register(b0, space0)
//{
//    ViewUniformBuffer1 ViewParam;
//}

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

    // From Transform
    float3 position;
    float3 direction;
    
    float3 padding;
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


float4 main(PSInput input) : SV_TARGET
{
    //float4 color = float4(0.0, 0.0, 0.0, 1.0);
    
    //color += float4(directionalLight.color, 0);
    //color += float4(pointLight.color, 0);
    //color += float4(spotLight.color, 0);
    //return color;

    float4 diffuseColor = DiffuseTexture.Sample(DiffuseTextureSampler, input.TexCoord);

    float3 normalColor = NormalTexture.Sample(NormalTextureSampler, input.TexCoord).rgb;

    float roughnessValue = RoughnessTexture.Sample(RoughnessTextureSampler, input.TexCoord).r;

    float metalicValue = MetalicTexture.Sample(MetailcTextureSampler, input.TexCoord).r;

    return input.Color;
    return float4(normalColor, 1.0);
    return diffuseColor;
    return float4(roughnessValue, 0, 0, 0);
    return float4(metalicValue, 0, 0, 0);
}
