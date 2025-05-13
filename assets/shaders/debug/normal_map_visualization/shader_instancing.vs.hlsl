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
    [[vk::location(0)]] float3 Position   : POSITION0;
    [[vk::location(1)]] float2 TexCoord   : TEXCOORD1; 
    [[vk::location(2)]] float4x4 Instance : INSTANCE2;  
#else
    float3 Position : POSITION0;
    float2 TexCoord : TEXCOORD1;
    float4x4 Instance : INSTANCE2;
#endif
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

    float4 modelPos = mul(ModelParam.ModelMatrix, float4(input.Position, 1.0));
#ifdef __spirv__
    //float4 modelPos = mul(float4(input.Position, 1.0), ModelParam.ModelMatrix);
    output.Position = mul(modelPos, input.Instance);
#else
    output.Position = mul(input.Instance, modelPos);
#endif
    output.Position = mul(ViewParam.VP, output.Position);

    output.TexCoord = input.TexCoord;

    return output;
}
