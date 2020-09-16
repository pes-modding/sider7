cbuffer ShaderConstants : register(b0) {
	float4x4 TransformMatrix : packoffset(c0);
	float4 ClipRect : packoffset(c4);
};

struct VSIn {
	float4 Position : POSITION;
	float4 GlyphColor : GLYPHCOLOR;
};

struct VSOut {
	float4 Position : SV_Position;
	float4 GlyphColor : COLOR;
	float2 TexCoord : TEXCOORD;
	float4 ClipDistance : CLIPDISTANCE;
};

VSOut clipVS(VSIn Input) {
	VSOut Output;
	
	Output.Position = mul(TransformMatrix, float4(Input.Position.xy, 0.0f, 1.0f));
	Output.GlyphColor = Input.GlyphColor;
	Output.TexCoord = Input.Position.zw;
	Output.ClipDistance = ClipRect + float4(Input.Position.xy, -Input.Position.xy);
	
	return Output;
}
