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
    [[vk::location(0)]] float3 Position   : POSITION0;
    [[vk::location(1)]] float4 Color      : COLOR1;  
    [[vk::location(2)]] float4x4 Instance : INSTANCE2;  
#else
    float3   Position : POSITION0;
    float4   Color    : COLOR1;
    float4x4 Instance : INSTANCE2;
#endif
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color    : COLOR1;
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
    
    output.Color = input.Color;

    return output;
}
