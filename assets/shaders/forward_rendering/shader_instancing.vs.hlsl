
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
    
    
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TANGENT0;
    float3 Bitangent : BITANGENT0;
    float4 Color : COLOR0;
    
    // instancing data
    float4x4 Instance : INSTANCE0;
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

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    //float4x4 Instance = float4x4(input.Instance0, input.Instance1, input.Instance2, input.Instance3);
    
    
    
    //output.Position = mul(input.Instance, float4(input.Position.x, input.Position.y, -input.Position.z, 1.0));
    output.Position = mul(input.Instance, float4(input.Position, 1.0));
    output.Position = mul(ViewParam.VP, output.Position);
    
    output.Normal = normalize(mul((float3x3) input.Instance, input.Normal));
    output.Tangent = normalize(mul((float3x3) input.Instance, input.Tangent));
    output.Bitangent = normalize(mul((float3x3) input.Instance, input.Bitangent));
    
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    
   

    return output;
}
