all: vsimple.h vclip.h vempty.h psimple.h pclip.h gsimple.h gclip.h

vsimple.h: vsimple.hlsl
	fxc /E simpleVS /Ges /T vs_4_0 /Fh vsimple.h vsimple.hlsl

vclip.h: vclip.hlsl
	fxc /E clipVS /Ges /T vs_4_0 /Fh vclip.h vclip.hlsl

vempty.h: vempty.hlsl
	fxc /E emptyVS /Ges /T vs_4_0 /Fh vempty.h vempty.hlsl

psimple.h: psimple.hlsl
	fxc /E simplePS /Ges /T ps_4_0 /Fh psimple.h psimple.hlsl

pclip.h: pclip.hlsl
	fxc /E clipPS /Ges /T ps_4_0 /Fh pclip.h pclip.hlsl

gsimple.h: gsimple.hlsl
	fxc /E simpleGS /Ges /T gs_4_0 /Fh gsimple.h gsimple.hlsl

gclip.h: gclip.hlsl
	fxc /E clipGS /Ges /T gs_4_0 /Fh gclip.h gclip.hlsl

