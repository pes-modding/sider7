struct GSIn {
	float3 PositionIndex : POSITIONINDEX;
	float4 GlyphColor : GLYPHCOLOR;
};

GSIn emptyVS(GSIn Input) {
	return Input;
}
