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

struct VSOut
{
    float4 pos : SV_POSITION;
};

//VSOutput main(uint vertexId : SV_VertexID)
//{
//    VSOutput output;
    
//    // Generate fullscreen triangle
//    float2 texCoord = float2((vertexId << 1) & 2, vertexId & 2);
//    float2 pos = texCoord * 2.0 - 1.0;
    
//    output.Position = float4(pos, 0.0, 1.0);
    
//    // Transform from clip space to world space using inverse VP
//    //float4 worldPos = mul(ViewParam.InvVP, float4(pos.x, pos.y, 1.0, 1.0));
//    //float3 worldPosNear = worldPos.xyz / worldPos.w;
//    //float4 worldPosNearH = mul(ViewParam.InvVP, float4(pos.x, pos.y, 0.0, 1.0)); // z = 0
//    float4 worldPosNearH = mul(ViewParam.InvVP, float4(pos.x, pos.y, 0.0, 1.0)); // z = 0
//    float3 worldPosNear = worldPosNearH.xyz / worldPosNearH.w;
    
//    // Far plane position
//    //float4 worldPosFar = mul(ViewParam.InvVP, float4(pos.x, pos.y, 0.0, 1.0));
//    //float3 worldPosFarNorm = worldPosFar.xyz / worldPosFar.w;
//    float4 worldPosFarH = mul(ViewParam.InvVP, float4(pos.x, pos.y, 1.0, 1.0)); // z = 1
//    float3 worldPosFar = worldPosFarH.xyz / worldPosFarH.w;
    
//    // Ray from camera through this pixel
//    float3 rayOrigin = ViewParam.EyeWorld;
//    //float3 rayDir = normalize(worldPosFarNorm - worldPosNear);
//    float3 rayDir = normalize(worldPosFar - worldPosNear);
    
//    // Ray-plane intersection with Y=0 plane
//    float t = -rayOrigin.y / rayDir.y;
    
//    if (t > 0.0)
//    {
//        output.WorldPos = rayOrigin + rayDir * t;
//        output.DistanceToCamera = length(output.WorldPos - rayOrigin);
//    }
//    else
//    {
//        // Ray doesn't intersect the plane or goes backward
//        output.WorldPos = float3(0, 0, 0);
//        output.DistanceToCamera = 0.0;
//        output.Position = float4(0, 0, 0, 0); // Discard this vertex
//    }
    
//    return output;
//}

//VSOutput main(uint vertexId : SV_VertexID)
//{
//    VSOutput o;

//    float2 tc = float2((vertexId << 1) & 2, vertexId & 2);
//    float2 pos = tc * 2.0 - 1.0;
//    o.Position = float4(pos, 0.0, 1.0);

//    float4 nearH = mul(ViewParam.InvVP, float4(pos, 0.0, 1.0));
//    float3 near = nearH.xyz / nearH.w;

//    float4 farH = mul(ViewParam.InvVP, float4(pos, 1.0, 1.0));
//    float3 farP = farH.xyz / farH.w;

//    float3 rayO = ViewParam.EyeWorld;
//    float3 rayDir = normalize(farP - near);

//    float t = -rayO.y / rayDir.y;

//    o.WorldPos = rayO + rayDir * t;
//    o.DistanceToCamera = t; 
//    return o;
//}

VSOut main(uint id : SV_VertexID)
{
    float2 tc = float2((id << 1) & 2, id & 2);
    VSOut o;
    o.pos = float4(tc * 2.0 - 1.0, 0.0, 1.0);
    return o;
}