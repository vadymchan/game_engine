#define PI 3.1415926535897932384626433832795028841971693993751058209749

struct RenderObjectUniformBuffer
{
    float4x4 M;
    float4x4 InvM;
    // currently not stated in cpu side
    //float Metallic;
    //float Roughness;
    //float padding0;
    //float padding1;
};

struct ViewUniformBuffer
{
    float4x4 V;
    float4x4 P;
    float4x4 VP;
    float3 EyeWorld;
    float padding0;
};

struct DirectionalLightUniformBuffer
{
    float3 Direction;
    float SpecularPow;

    float3 Color;
    float padding0;

    float3 DiffuseIntensity;
    float padding1;

    float3 SpecularIntensity;
    float padding2;

    float4x4 ShadowVP;
    float4x4 ShadowV;
		
    float3 LightPos;
    float padding3;

    float2 ShadowMapSize;
    float Near;
    float Far;
};

struct PointLightUniformBufferData
{
    float3 Position;
    float SpecularPow;

    float3 Color;
    float MaxDistance;

    float3 DiffuseIntensity;
    float padding0;

    float3 SpecularIntensity;
    float padding1;

    float4x4 ShadowVP[6];
};

struct SpotLightUniformBufferData
{
    float3 Position;
    float MaxDistance;

    float3 Direction;
    float PenumbraRadian;

    float3 Color;
    float UmbraRadian;

    float3 DiffuseIntensity;
    float SpecularPow;

    float3 SpecularIntensity;
    float padding0;

    float4x4 ShadowVP;
};

