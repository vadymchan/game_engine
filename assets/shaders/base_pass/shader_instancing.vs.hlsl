
struct VSInput
{
#ifdef __spirv__
    [[vk::location(0)]] float3   Position  : POSITION0;
    [[vk::location(1)]] float2   TexCoord  : TEXCOORD1;  
    [[vk::location(2)]] float3   Normal    : NORMAL2;  
    [[vk::location(3)]] float3   Tangent   : TANGENT3;  
    [[vk::location(4)]] float3   Bitangent : BITANGENT4;  
    [[vk::location(5)]] float4   Color     : COLOR5;  
    [[vk::location(6)]] float4x4 Instance  : INSTANCE6;  
#else
    float3   Position  : POSITION0;
    float2   TexCoord  : TEXCOORD1;
    float3   Normal    : NORMAL2;
    float3   Tangent   : TANGENT3;
    float3   Bitangent : BITANGENT4;
    float4   Color     : COLOR5;
    float4x4 Instance  : INSTANCE6;
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

struct ModelUniformBuffer
{
    float4x4 ModelMatrix; 
};

cbuffer ModelParam : register(b0, space1) 
{
    ModelUniformBuffer ModelParam;
}

struct VSOutput
{
    float4 Position  : SV_POSITION;
    float2 TexCoord  : TEXCOORD1;
    float3 Normal    : NORMAL2;
    float3 Tangent   : TANGENT3;
    float3 Bitangent : BITANGENT4;
    float4 Color     : COLOR5;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

    float4 modelPos = mul(ModelParam.ModelMatrix, float4(input.Position, 1.0));
#ifdef __spirv__
    output.Position = mul(modelPos, input.Instance);
#else
    output.Position = mul(input.Instance, modelPos);
#endif
    
    output.Position = mul(ViewParam.VP, output.Position);
    
    // TODO: add model matrix multiplication
    //output.Normal = normalize(mul((float3x3) input.Instance, input.Normal));
    //output.Tangent = normalize(mul((float3x3) input.Instance, input.Tangent));
    //output.Bitangent = normalize(mul((float3x3) input.Instance, input.Bitangent));
    
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    
   

    return output;
}
