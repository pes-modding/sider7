SamplerState sampler0 : register(s0);
Texture2D<float> tex0 : register(t0);

struct PSIn {
	float4 Position : SV_Position;
	float4 GlyphColor : COLOR;
	float2 TexCoord : TEXCOORD;
};

float4 simplePS(PSIn Input) : SV_Target {
	float a = tex0.Sample(sampler0, Input.TexCoord);
	
	if(a == 0.0f)
		discard;
	
	return (a * Input.GlyphColor.a) * float4(Input.GlyphColor.rgb, 1.0f);
}
