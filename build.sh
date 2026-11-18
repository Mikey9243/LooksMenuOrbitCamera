#!/bin/sh
# Builds LooksMenuOrbitCamera.dll on Linux (clang + lld-link + llvm-dlltool, no Visual Studio, no C runtime).
set -e
cd "$(dirname "$0")/src"
for d in k32 user32 shell32; do llvm-dlltool -m i386:x86-64 -d $d.def -l $d.lib; done
clang -target x86_64-pc-windows-msvc -O2 -ffreestanding -fno-builtin -fno-stack-protector -mno-stack-arg-probe -Wall -Wextra -Wno-unused-parameter -c lmoc.c -o lmoc.obj
lld-link /Brepro /dll /entry:DllMain /nodefaultlib /subsystem:windows /out:LooksMenuOrbitCamera.dll lmoc.obj k32.lib user32.lib shell32.lib
