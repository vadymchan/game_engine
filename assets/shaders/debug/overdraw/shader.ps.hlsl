struct PSInput
{
    float4 Position : SV_POSITION;
};


float4 main(PSInput input) : SV_TARGET
{
    return float4(1, 0, 0, 0.25);
}
