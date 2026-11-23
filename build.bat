@echo off
rem Builds src\LooksMenuOrbitCamera.dll on Windows with LLVM (https://github.com/llvm/llvm-project/releases, 18.1.x).
rem Needs clang.exe, lld-link.exe, llvm-dlltool.exe and llvm-rc.exe on PATH. No Visual Studio and no C runtime are used.
cd /d "%~dp0src" || exit /b 1
for %%d in (k32 shell32) do llvm-dlltool -m i386:x86-64 -d %%d.def -l %%d.lib || exit /b 1
llvm-rc /fo version.res version.rc || exit /b 1
clang -target x86_64-pc-windows-msvc -O2 -ffreestanding -fno-builtin -fno-stack-protector -mno-stack-arg-probe -Wall -Wextra -Wno-unused-parameter -c lmoc.c -o lmoc.obj || exit /b 1
lld-link /Brepro /dll /entry:DllMain /nodefaultlib /subsystem:windows /out:LooksMenuOrbitCamera.dll lmoc.obj version.res k32.lib shell32.lib || exit /b 1
echo Built src\LooksMenuOrbitCamera.dll
