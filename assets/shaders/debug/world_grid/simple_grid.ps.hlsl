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

//struct PSInput
//{
//    float4 Position : SV_POSITION;
//    float3 WorldPos : TEXCOORD0;
//    float DistanceToCamera : TEXCOORD1;
//};

//float4 main(PSInput input) : SV_TARGET
//{
//    // Discard if we didn't hit the plane
//    if (input.DistanceToCamera < 0.0)
//        discard;
    
//    float3 worldPos = input.WorldPos;
//    float gridSize = 1.0; // 1 unit grid
    
//    // Calculate grid lines
//    float2 gridPos = worldPos.xz / gridSize;
//    float2 grid = abs(frac(gridPos - 0.5) - 0.5) / fwidth(gridPos);
//    float minorLine = min(grid.x, grid.y);
    
//    // Grid intensity (1.0 on lines, 0.0 between)
//    float gridIntensity = 1.0 - min(minorLine, 1.0);
    
//    // Major grid lines every 10 units
//    float2 gridPosMajor = worldPos.xz / (gridSize * 10.0);
//    float2 gridMajor = abs(frac(gridPosMajor - 0.5) - 0.5) / fwidth(gridPosMajor);
//    float majorLine = min(gridMajor.x, gridMajor.y);
//    float majorIntensity = 1.0 - min(majorLine, 1.0);
    
//    // Distance fade
//    float fadeDistance = 100.0;
//    float fade = saturate(1.0 - input.DistanceToCamera / fadeDistance);
//    fade = fade * fade; // Quadratic falloff
    
//    // Combine grid lines
//    float3 gridColor = float3(0.3, 0.3, 0.3); // Minor grid color
//    float3 majorColor = float3(0.6, 0.6, 0.6); // Major grid color
    
//    // Axis lines
//    float axisLineWidth = 0.02;
//    float xAxis = 1.0 - smoothstep(0.0, axisLineWidth, abs(worldPos.x));
//    float zAxis = 1.0 - smoothstep(0.0, axisLineWidth, abs(worldPos.z));

    
//    float3 finalColor = gridColor;
    
//    // Apply major grid
//    finalColor = lerp(finalColor, majorColor, majorIntensity);
    
//    // Apply axis colors
//    if (xAxis > 0.01)
//        finalColor = float3(1.0, 0.0, 0.0); // X
//    if (zAxis > 0.01)
//        finalColor = float3(0.0, 0.0, 1.0); // Z

//    float alpha = max(gridIntensity, majorIntensity);
//    alpha = max(alpha, max(xAxis, zAxis));
//    alpha *= fade;
    
//    // TODO: make optimization
//    if (alpha < 0.01)
//        discard;
    
//    return float4(finalColor, alpha);
//}


struct PSIn
{
    float4 pos : SV_POSITION; 
};

float4 main(PSIn inp) : SV_TARGET
{

    float2 ndc = inp.pos.xy / inp.pos.w;
    
    float4 nearH = mul(ViewParam.InvVP, float4(ndc, 0.0, 1.0));
    float3 nearW = nearH.xyz / nearH.w;

    float4 farH = mul(ViewParam.InvVP, float4(ndc, 1.0, 1.0));
    float3 farW = farH.xyz / farH.w;

    float3 rayDir = normalize(farW - nearW);

    float t = -ViewParam.EyeWorld.y / rayDir.y;
    if (t < 0.0)
        discard; 

    float3 wPos = ViewParam.EyeWorld + rayDir * t;


    float gridSize = 1.0;
    float2 g = abs(frac(wPos.xz / gridSize - 0.5) - 0.5) / fwidth(wPos.xz / gridSize);
    float minor = 1.0 - saturate(min(g.x, g.y));

    float2 gM = abs(frac(wPos.xz / (gridSize * 10) - 0.5) - 0.5) / fwidth(wPos.xz / (gridSize * 10));
    float major = 1.0 - saturate(min(gM.x, gM.y));

    float axisW = 0.02;
    float xAxis = 1.0 - smoothstep(0.0, axisW, abs(wPos.x));
    float zAxis = 1.0 - smoothstep(0.0, axisW, abs(wPos.z));

    float3 color = float3(0.3, 0.3, 0.3);
    color = lerp(color, float3(0.6, 0.6, 0.6), major);
    if (xAxis > 0.01)
        color = float3(1, 0, 0);
    if (zAxis > 0.01)
        color = float3(0, 0, 1); 

    float alpha = max(max(minor, major), max(xAxis, zAxis));
    alpha *= saturate(1.0 - t / 150.0); 

    if (alpha < 0.01)
        discard;
    return float4(color, alpha);
}