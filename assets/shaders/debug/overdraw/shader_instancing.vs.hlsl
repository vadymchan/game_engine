struct VSInput
{
#ifdef __spirv__
    [[vk::location(0)]] float3 Position   : POSITION0;
    [[vk::location(1)]] float4x4 Instance : INSTANCE1;  
#else
    float3 Position : POSITION0;
    float4x4 Instance : INSTANCE1;
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
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
#ifdef __spirv__
    output.Position = mul(float4(input.Position, 1.0), input.Instance);
#else
    output.Position = mul(input.Instance, float4(input.Position, 1.0));
#endif
    output.Position = mul(ViewParam.VP, output.Position);

    return output;
}
