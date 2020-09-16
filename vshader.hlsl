struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 ShadeColor : COLOR;
};
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 ShadeColor : COLOR;
};
PS_INPUT siderVS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = input.Pos;
    output.ShadeColor = input.ShadeColor;
    return output;
}
