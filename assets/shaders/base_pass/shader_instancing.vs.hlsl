
struct VSInput
{
    //[[vk::location(0)]] float3 Position : POSITION0;
    //[[vk::location(1)]] float3 Normal : NORMAL0;
    //[[vk::location(2)]] float2 TexCoord : TEXCOORD0;
    //[[vk::location(3)]] float3 Tangent : TANGENT0;
    //[[vk::location(4)]] float3 Bitangent : BITANGENT0;
    //[[vk::location(5)]] float4 Color : COLOR0;
    
    //// instancing data
    //[[vk::location(6)]] float4x4 Instance : INSTANCE0;
  
#ifdef __spirv__
    [[vk::location(0)]] float3 Position   : POSITION0;
    [[vk::location(1)]] float3 Normal     : NORMAL0;
    [[vk::location(2)]] float2 TexCoord   : TEXCOORD0;
    [[vk::location(3)]] float3 Tangent    : TANGENT0;
    [[vk::location(4)]] float3 Bitangent  : BITANGENT0;
    [[vk::location(5)]] float4 Color      : COLOR0;
    [[vk::location(6)]] float4x4 Instance : INSTANCE0;
#else
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT0;
    float3 Bitangent : BITANGENT0;
    float4 Color : COLOR0;
    float4x4 Instance : INSTANCE0;
#endif
    
    
    //float3 Position : POSITION0;
    //float3 Normal : NORMAL0;
    //float2 TexCoord : TEXCOORD0;
    //float3 Tangent : TANGENT0;
    //float3 Bitangent : BITANGENT0;
    //float4 Color : COLOR0;
    
    //// instancing data
    //float4x4 Instance : INSTANCE0;
    //float4 Instance0 : INSTANCE0;
    //float4 Instance1 : INSTANCE1;
    //float4 Instance2 : INSTANCE2;
    //float4 Instance3 : INSTANCE3;
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

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT0;
    float3 Bitangent : BITANGENT0;
    float4 Color : COLOR0;
};

#if TODO 
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

cbuffer PointLight : register(b0, space2)
{
    PointLightData pointLight;
};

cbuffer SpotLight : register(b0, space3)
{
    SpotLightData spotLight;
};
#endif

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

#ifdef __spirv__
    output.Position = mul(float4(input.Position, 1.0), input.Instance);
#else
    output.Position = mul(input.Instance, float4(input.Position, 1.0));
#endif
    output.Position = mul(ViewParam.VP, output.Position);
    
    output.Normal = normalize(mul((float3x3) input.Instance, input.Normal));
    output.Tangent = normalize(mul((float3x3) input.Instance, input.Tangent));
    output.Bitangent = normalize(mul((float3x3) input.Instance, input.Bitangent));
    
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    
   

    return output;
}
