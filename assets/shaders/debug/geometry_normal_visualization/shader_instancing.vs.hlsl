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

struct ModelUniformBuffer
{
    float4x4 ModelMatrix;
};

cbuffer ModelParam : register(b0, space1)
{
    ModelUniformBuffer ModelParam;
}

struct VSInput
{
#ifdef __spirv__
    [[vk::location(0)]] float3   Position  : POSITION0;
    [[vk::location(1)]] float3   Normal    : NORMAL1;  
    [[vk::location(2)]] float3   Tangent   : TANGENT2;  
    [[vk::location(3)]] float3   Bitangent : BITANGENT3;  
    [[vk::location(4)]] float4x4 Instance  : INSTANCE4;  
#else
    float3   Position  : POSITION0;
    float3   Normal    : NORMAL1;
    float3   Tangent   : TANGENT2;
    float3   Bitangent : BITANGENT3;
    float4x4 Instance  : INSTANCE4;
#endif
};


struct VSOutput
{
    float4 Position  : SV_POSITION;
    float3 Normal    : NORMAL1;
    float3 Tangent   : TANGENT2;
    float3 Bitangent : BITANGENT3;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

#ifdef __spirv__
    float4x4 worldMatrix = mul(ModelParam.ModelMatrix, input.Instance);
    
    output.Position = mul(float4(input.Position, 1.0), worldMatrix);
    
    output.Normal = normalize(mul(input.Normal, (float3x3) worldMatrix));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) worldMatrix));
    output.Bitangent = normalize(mul(input.Bitangent, (float3x3) worldMatrix));
#else
    float4x4 worldMatrix = mul(input.Instance, ModelParam.ModelMatrix);
    
    output.Position = mul(worldMatrix, float4(input.Position, 1.0));
    
    output.Normal = normalize(mul((float3x3) worldMatrix, input.Normal));
    output.Tangent = normalize(mul((float3x3) worldMatrix, input.Tangent));
    output.Bitangent = normalize(mul((float3x3) worldMatrix, input.Bitangent));
#endif
    
    return output;
}
