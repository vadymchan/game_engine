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

struct GSInput
{
    float4 PositionWS : SV_POSITION;
    float3 Normal : NORMAL1;
    float3 Tangent : TANGENT2;
    float3 Bitangent : BITANGENT3;
};

struct GSOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};


// 3 lines (2 points each) for each vertex (3 vertices of a triangle)
//[maxvertexcount(3 * 2 * 3)]
[maxvertexcount(3 * 2 * 3)]
void main(triangle GSInput inputTri[3], inout LineStream<GSOutput> lineStream)
{
    // lenght of the lines
    float scale = 0.1f;
    // for each triangle vertex
    [unroll]
    for (int i = 0; i < 3; i++)
    {
        float3 pos = float3(inputTri[i].PositionWS.xyz);

        // 1) Tangent (red)
        {
            float3 endPos = pos + inputTri[i].Tangent * scale;
            float4 endPosClip = mul(ViewParam.VP, float4(endPos, 1));

            GSOutput vStart, vEnd;

            vStart.position = mul(ViewParam.VP, inputTri[i].PositionWS);
            //vEnd.position = vStart.position;
            vEnd.position = endPosClip;
            
            vStart.color = vEnd.color = float4(1, 0, 0, 1);

            lineStream.Append(vStart);
            lineStream.Append(vEnd);
            lineStream.RestartStrip();
        }

        // 2) Bitangent (green)
        {
            float3 endPos = pos + inputTri[i].Bitangent * scale;
            float4 endPosClip = mul(ViewParam.VP, float4(endPos, 1));
            
            GSOutput vStart, vEnd;
            
            vStart.position = mul(ViewParam.VP, inputTri[i].PositionWS);
            //vEnd.position = vStart.position;
            vEnd.position = endPosClip;
            
            vStart.color = vEnd.color = float4(0, 1, 0, 1);

            lineStream.Append(vStart);
            lineStream.Append(vEnd);
            lineStream.RestartStrip();
        }

        // 3) Normal (blue)
        {
            float3 endPos = pos + inputTri[i].Normal * scale;
            float4 endPosClip = mul(ViewParam.VP, float4(endPos, 1));

            GSOutput vStart, vEnd;

            vStart.position = mul(ViewParam.VP, inputTri[i].PositionWS);
            //vEnd.position = vStart.position;
            vEnd.position = endPosClip;
            
            vStart.color = vEnd.color = float4(0, 0, 1, 1);

            lineStream.Append(vStart);
            lineStream.Append(vEnd);
            lineStream.RestartStrip();
        }
    }
}
