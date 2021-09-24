@echo off

set DIRECTX_LIB_PATH="C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x64"
set DIRECTX_INCLUDE_PATH="C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include"

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build
cl -nologo /Zi -wd4005 ..\code\win32_platform.cpp /I%DIRECTX_INCLUDE_PATH% /link /LIBPATH:%DIRECTX_LIB_PATH% User32.lib Gdi32.lib Kernel32.lib Winmm.lib d3d11.lib d3dx11.lib dxerr.lib d3dcompiler.lib Ole32.lib Comctl32.lib
popd
