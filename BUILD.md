# Building

Toolchain: LLVM 18.1.x (`clang`, `lld-link`, `llvm-dlltool`, `llvm-rc`). No Visual Studio, no Windows SDK, no C runtime.

## Linux (how the release was built)
Ubuntu 24.04: `sudo apt install clang-18 lld-18 llvm-18` (or `clang lld llvm`), then:

    sh build.sh

## Windows
Install LLVM 18.1.x from https://github.com/llvm/llvm-project/releases and make sure its `bin` folder is on PATH, then:

    build.bat

## Output
`src/LooksMenuOrbitCamera.dll`. Install it as `Data\F4SE\Plugins\LooksMenuOrbitCamera.dll` together with `src/LooksMenuOrbitCamera.ini`.

## Reproducible build
The link uses `/Brepro`, so the same compiler version produces a byte-identical DLL. Built with Ubuntu clang 18.1.3 / LLD 18.1.3, the 1.4.5 release DLL has

    SHA-256 a7d7dee86bcdc0f8e6eb7a7f987febd81248b1e2c8c108861383fd398d945631

(Other LLVM builds may produce a different but equivalent binary.)
