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

    float3x3 modelMatrix3x3 = (float3x3) ModelParam.ModelMatrix;
    float3x3 instanceMatrix3x3 = (float3x3) input.Instance;
    
    float4 modelPos = mul(ModelParam.ModelMatrix, float4(input.Position, 1.0));
    //float3x3 worldMatrix3x3 = mul(instanceMatrix3x3, modelMatrix3x3);
    float3x3 worldMatrix3x3 = mul(modelMatrix3x3, instanceMatrix3x3);
    //float3x3 worldMatrix3x3 = instanceMatrix3x3;
    
#ifdef __spirv__
    //float3x3 worldMatrix3x3 = mul(modelMatrix3x3, instanceMatrix3x3);

    //float4 modelPos = mul(ModelParam.ModelMatrix, float4(input.Position, 1.0));
    output.Position = mul(modelPos, input.Instance);
    
    output.Normal    = normalize(mul(input.Normal,    worldMatrix3x3));
    output.Tangent   = normalize(mul(input.Tangent,   worldMatrix3x3));
    output.Bitangent = normalize(mul(input.Bitangent, worldMatrix3x3));
#else
    //float3x3 worldMatrix3x3 = mul(instanceMatrix3x3, modelMatrix3x3);
    //float3x3 worldMatrix3x3 = mul(modelMatrix3x3, instanceMatrix3x3);
    
    //float4 modelPos = mul(float4(input.Position, 1.0), ModelParam.ModelMatrix);
    output.Position = mul(input.Instance, modelPos);
    
    output.Normal =    normalize(mul(worldMatrix3x3, input.Normal));
    output.Tangent =   normalize(mul(worldMatrix3x3, input.Tangent));
    output.Bitangent = normalize(mul(worldMatrix3x3, input.Bitangent));
#endif


    //output.Normal = normalize(mul(worldMatrix3x3, input.Normal));
    //output.Tangent = normalize(mul(worldMatrix3x3, input.Tangent));
    //output.Bitangent = normalize(mul(worldMatrix3x3, input.Bitangent));
    return output;
}
