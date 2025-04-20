
struct VSInput
{
#ifdef __spirv__
    [[vk::location(0)]] float3 Position   : POSITION0;
    [[vk::location(1)]] float3 Normal     : NORMAL1;  
    [[vk::location(2)]] float3 Tangent    : TANGENT2;  
    [[vk::location(3)]] float3 Bitangent  : BITANGENT3;  
    [[vk::location(4)]] float4x4 Instance : INSTANCE4;  
#else
    float3 Position : POSITION0;
    float3 Normal : NORMAL1;
    float3 Tangent : TANGENT2;
    float3 Bitangent : BITANGENT3;
    float4x4 Instance : INSTANCE4;
#endif
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
    float3 Normal : NORMAL1;
    float3 Tangent : TANGENT2;
    float3 Bitangent : BITANGENT3;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

#ifdef __spirv__
    output.Position = mul(float4(input.Position, 1.0), input.Instance);
#else
    output.Position = mul(input.Instance, float4(input.Position, 1.0));
#endif
    output.Normal = normalize(mul((float3x3) input.Instance, input.Normal));
    output.Tangent = normalize(mul((float3x3) input.Instance, input.Tangent));
    output.Bitangent = normalize(mul((float3x3) input.Instance, input.Bitangent));
    
    //output.Color = input.Color;
    //output.TexCoord = input.TexCoord;

    return output;
}
