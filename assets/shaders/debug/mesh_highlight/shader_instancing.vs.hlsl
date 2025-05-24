struct VSInput
{
#ifdef __spirv__
    [[vk::location(0)]] float3 Position   : POSITION0;
    [[vk::location(1)]] float3 Normal     : NORMAL1;  
    [[vk::location(2)]] float4x4 Instance : INSTANCE2;  
#else
    float3 Position : POSITION0;
    float3 Normal : NORMAL1;
    float4x4 Instance : INSTANCE2;
#endif
};

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

struct HighlightParamsBuffer
{
    float4 Color;
    float Thickness;
    float3 Padding;
};
cbuffer HighlightParams : register(b0, space2)
{
    HighlightParamsBuffer HighlightParams;
}

struct VSOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;

#ifdef __spirv__
    float4x4 worldMatrix = mul(ModelParam.ModelMatrix, input.Instance);
    float4 worldPos = mul(float4(input.Position, 1.0), worldMatrix);
#else
    float4x4 worldMatrix = mul(input.Instance, ModelParam.ModelMatrix);
    float4 worldPos = mul(worldMatrix, float4(input.Position, 1.0));
#endif
    
    // Calculate normal in world space
    float3x3 normalMat =
    {
        normalize(worldMatrix[0].xyz),
        normalize(worldMatrix[1].xyz),
        normalize(worldMatrix[2].xyz)
    };

#ifdef __spirv__
    float3 worldNormal = normalize(mul(input.Normal,    normalMat));
#else
    float3 worldNormal = normalize(mul(normalMat, input.Normal));
#endif

    worldPos.xyz += worldNormal * HighlightParams.Thickness;

    output.Position = mul(ViewParam.VP, worldPos);

    output.Color = HighlightParams.Color;
    
    return output;
}