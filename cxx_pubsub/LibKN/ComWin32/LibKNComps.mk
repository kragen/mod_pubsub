
LibKNComps.dll: dlldata.obj LibKNCom_p.obj LibKNCom_i.obj
	link /dll /out:LibKNComps.dll /def:LibKNComps.def /entry:DllMain dlldata.obj LibKNCom_p.obj LibKNCom_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del LibKNComps.dll
	@del LibKNComps.lib
	@del LibKNComps.exp
	@del dlldata.obj
	@del LibKNCom_p.obj
	@del LibKNCom_i.obj
