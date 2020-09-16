struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Tex : TEXCOORD0;
};
struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Tex : TEXCOORD0;
};
PS_INPUT siderTexVS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = input.Pos;
    output.Tex = input.Tex;
    return output;
};
