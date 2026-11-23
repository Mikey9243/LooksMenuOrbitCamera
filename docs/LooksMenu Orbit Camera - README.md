# LooksMenu Orbit Camera 1.4.5

Lets you walk the camera all the way around the character you are editing in LooksMenu - your own character or any NPC: front, sides, back, from above and from below, zoomed in or out. The camera moves; your character, her pose and where she stands stay exactly as they are.

- Starts from the view LooksMenu gives you, with no jump.
- 360° left/right, up/down with limits, smooth zoom, smooth movement.
- The camera height can be moved, so you can look at the face, the clothes or the boots.
- One button puts the camera back in front of the character.
- Works for NPCs too: open LooksMenu on an NPC with the console (`slm`) and the camera circles that NPC.
- The character being edited (player or NPC) is frozen while LooksMenu is open and moves again when you close it (`bFreezeCharacter`).
- Keeps its position when you switch between LooksMenu's categories.
- Closing LooksMenu gives you the normal game camera back.
- Mouse/keyboard and Xbox controller both work at the same time, with no switching.
- Everything can be changed in `F4SE\Plugins\LooksMenuOrbitCamera.ini`.

## Requirements
- Fallout 4, any version F4SE exists for:
  - **1.10.163** (pre-next-gen) with F4SE 0.6.23
  - **1.10.980 / 1.10.984** (next-gen) with F4SE 0.7.0 – 0.7.3
  - **1.11.137, 1.11.159, 1.11.169, 1.11.191, 1.11.221, 1.11.240** (anniversary) with F4SE 0.7.4 – 0.7.9
- LooksMenu for your game version

No ESP. Nothing in the game or in LooksMenu is overwritten.

## Controls (default)

| | Mouse / keyboard | Controller |
|---|---|---|
| Rotate around the character | hold **right mouse button** and move the mouse | **right stick** |
| Zoom | **right mouse button + mouse wheel** | hold **R3** + right stick up/down |
| Camera up / down | **right mouse button + Q / E**, or **Page Up / Page Down** | hold **R3** + **D-pad up/down** |
| Reset | **middle mouse button** | **tap R3** |

See CONTROLS.md for the reasons behind these and how to change them.

## Troubleshooting
- **Nothing happens:** check that `Documents\My Games\Fallout4\F4SE\LooksMenuOrbitCamera.log` exists. If it does not, F4SE did not load the plugin (start the game through F4SE, check that the DLL is in `Data\F4SE\Plugins`). If it says "Unknown game version", your game is a version F4SE does not exist for. If it says "camera check FAILED" or "menu list check FAILED", the plugin did not recognise the game's memory layout and stayed off to be safe; send the log.
- **Editing an NPC orbits the wrong character:** send the log; its "orbit target" line says which character was chosen and why.
- **The camera drifts on its own (controller):** raise `fDeadZone` in the INI (for example 0.25).
- **Too fast / too slow:** `fCameraSensitivity` (mouse), `fRightStickSensitivity` (controller), `fZoomSpeed`, `fHeightAdjustSpeed`.
- **Too floaty / too stiff:** `fCameraSmoothing` (higher = snappier, 0 = none).
- **The camera goes through a wall:** the plugin has no wall collision of its own (see COMPATIBILITY.md). Zoom in or move round.
- **Something else:** set `bDebugLog=1`, reproduce it, and send the log.

## Files
- `F4SE\Plugins\LooksMenuOrbitCamera.dll` – the plugin
- `F4SE\Plugins\LooksMenuOrbitCamera.ini` – settings
