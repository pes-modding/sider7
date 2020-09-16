# For debug builds, use "debug=1" in command line. For instance,
# to build DLLs in debug mode: nmake dlls debug=1

CC=cl
LINK=link
RC=rc

!if "$(debug)"=="1"
EXTRA_CFLAGS=/DDEBUG
!else
EXTRA_CFLAGS=/DMYDLL_RELEASE_BUILD
!endif

!if "$(perf)"=="1"
EXTRA_CFLAGS=$(EXTRA_CFLAGS) /DPERF_TESTING
!endif

LPZLIB=soft\zlib-1.2.11
ZLIBINC=/I soft\zlib-1.2.11
ZLIBLIB=zlib.lib

# 4731: warning about ebp modification
#CFLAGS=/nologo /Od /EHsc /wd4731 /MT /D_WIN32_WINNT=0x601 /DMA_NO_AVX2 /DMA_NO_AVX512 $(EXTRA_CFLAGS)
CFLAGS=/nologo /Od /EHsc /wd4731 /MT /D_WIN32_WINNT=0x601 $(EXTRA_CFLAGS)
LFLAGS=/NOLOGO /DEFAULTLIB:"LIBCMT"
LIBS=user32.lib gdi32.lib comctl32.lib version.lib ole32.lib shell32.lib

#LUAINC=/I soft\LuaJIT\src
#LUALIBPATH=soft\LuaJIT\src
#LUAINC=/I soft\LuaJIT-2.0.5\src
#LUALIBPATH=soft\LuaJIT-2.0.5\src
LUAINC=/I soft\LuaJIT-2.1.0-beta3\src
LUALIBPATH=soft\LuaJIT-2.1.0-beta3\src
LUALIB=lua51.lib
LUADLL=lua51.dll
LUAJIT=luajit.exe

FW1INC=/I soft\FW1FontWrapper\Source
FW1LIB=FW1FontWrapper.lib
FW1LIBPATH=soft\FW1FontWrapper\Release\x64
FW1LIBROOT=soft\FW1FontWrapper

MAINC=/I soft\miniaudio

all: sider.exe sider.dll

sider.res: sider.rc
	$(RC) -r -fo sider.res sider.rc
sider_main.res: sider_main.rc sider.ico
	$(RC) -r -fo sider_main.res sider_main.rc

simple_playback.obj: simple_playback.c
audio.obj: audio.cpp audio.h sider.h common.h config.h
kmp.obj: kmp.cpp kmp.h
common.obj: common.cpp common.h
imageutil.obj: imageutil.cpp imageutil.h
version.obj: version.cpp
libz.obj: libz.cpp libz.h common.h
kitinfo.obj: kitinfo.cpp kitinfo.h
memlib.obj: memlib.h memlib_lua.h memlib.cpp
memlib_lua.h: memory.lua makememlibhdr.exe
	makememlibhdr.exe
makememlibhdr.exe: makememlibhdr.c
	$(CC) makememlibhdr.c

$(LPZLIB)\$(ZLIBLIB):
    cd $(LPZLIB) && nmake -f win32\Makefile.msc

$(LUALIBPATH)\$(LUALIB):
	cd $(LUALIBPATH) && msvcbuild.bat gc64 static

$(FW1LIBPATH)\$(FW1LIB):
	cd $(FW1LIBROOT)\Source && nmake -f ..\Makefile
	cd $(FW1LIBROOT) && msbuild /p:Configuration=Release

DDSTextureLoader.obj: DDSTextureLoader.cpp DDSTextureLoader.h
WICTextureLoader.obj: WICTextureLoader.cpp WICTextureLoader.h

util.obj: util.asm
	ml64 /c util.asm

vshader.h: vshader.hlsl
	fxc /E siderVS /Ges /T vs_4_0 /Fh vshader.h vshader.hlsl

vtexshader.h: vtexshader.hlsl
	fxc /E siderTexVS /Ges /T vs_4_0 /Fh vtexshader.h vtexshader.hlsl

pshader.h: pshader.hlsl
	fxc /E siderPS /Ges /T ps_4_0 /Fh pshader.h pshader.hlsl

ptexshader.h: ptexshader.hlsl
	fxc /E siderTexPS /Ges /T ps_4_0 /Fh ptexshader.h ptexshader.hlsl

sider.obj: sider.cpp sider.h patterns.h common.h config.h audio.h imageutil.h vshader.h vtexshader.h pshader.h ptexshader.h libz.h kitinfo.h utf8.h
sider.dll: sider.obj util.obj imageutil.obj version.obj common.obj kmp.obj memlib.obj libz.obj audio.obj kitinfo.obj DDSTextureLoader.obj WICTextureLoader.obj sider.res $(LUALIBPATH)\$(LUALIB) $(FW1LIBPATH)\$(FW1LIB) $(LPZLIB)\$(ZLIBLIB)
	$(LINK) $(LFLAGS) /out:sider.dll /DLL sider.obj util.obj imageutil.obj version.obj common.obj kmp.obj memlib.obj libz.obj audio.obj kitinfo.obj DDSTextureLoader.obj WICTextureLoader.obj sider.res $(ZLIBLIB) /LIBPATH:$(LUALIBPATH) /LIBPATH:$(FW1LIBPATH) $(LIBS) $(LUALIB) $(FW1LIB) /LIBPATH:$(LPZLIB) /LIBPATH:"$(LIB)"

sider.exe: main.obj sider.dll sider_main.res
	$(LINK) $(LFLAGS) /out:sider.exe main.obj sider_main.res $(LIBS) sider.lib /LIBPATH:"$(LIB)"

simple_playback.exe: simple_playback.obj
	$(LINK) $(LFLAGS) /out:simple_playback.exe simple_playback.obj

$(LUAJIT): $(LUALIBPATH)\$(LUALIB)
	copy $(LUALIBPATH)\$(LUAJIT) .

.cpp.obj:
	$(CC) $(CFLAGS) -c $(INC) $(LUAINC) $(FW1INC) $(ZLIBINC) $(MAINC) $<

clean:
	del *.obj *.dll *.exp *.res *.lib *.exe *~ memlib_lua.h vshader.h vtexshader.h pshader.h ptexshader.h

clean-all: clean
	cd $(LUALIBPATH) && del /Q lua51.exp lua51.lib lua51.dll luajit.exe
	cd $(FW1LIBROOT) && del /Q /S Debug Release
	cd $(FW1LIBROOT)\Source && del /Q /S vsimple.h vclip.h vempty.h gsimple.h gclip.h psimple.h pclip.h
	cd $(LPZLIB) && nmake -f win32\Makefile.msc clean

