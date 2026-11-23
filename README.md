# LooksMenu Orbit Camera – source

F4SE plugin for Fallout 4. While LooksMenu is open, the camera orbits the character being edited (the player, or an NPC opened with `slm`); the character is held still while the menu is open.

Release: LooksMenu Orbit Camera 1.4.5 on Nexus Mods.

## What the DLL does
- Runs only inside the Fallout 4 process, loaded by F4SE.
- Hooks (pointer replacement only, no code patching):
  - the camera states' virtual `Update` function (camera placement while LooksMenu is open);
  - the game's import-table entries for `GetRawInputData` (USER32) and `XInputGetState` (XINPUT1_3), so mouse/controller input used by the camera does not also move LooksMenu's sliders.
- Reads keys only from the game's own raw input that passes through the hook above (no key polling, no window polling), settings with `GetPrivateProfileStringA`, writes one log file (`Documents\My Games\Fallout4\F4SE\LooksMenuOrbitCamera.log`).
- No network access, no other files written, no other processes touched.
- Imports only KERNEL32 and SHELL32 (SHGetFolderPathA for the Documents folder). Carries a VERSIONINFO resource (src/version.rc).

## Files
- `src/lmoc.c` – the whole plugin (freestanding C, no C runtime)
- `src/*.def` – import definitions for KERNEL32 / SHELL32 (turned into import libraries at build time)
- `src/LooksMenuOrbitCamera.ini` – default settings
- `src/version.rc` – the DLL's version information
- `build.sh` (Linux) / `build.bat` (Windows) – build scripts
- `docs/` – the user documentation shipped with the mod

See BUILD.md for how to build it and check the release DLL.
