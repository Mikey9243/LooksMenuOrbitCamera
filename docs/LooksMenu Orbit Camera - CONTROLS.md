# Controls

## Default

| | Mouse / keyboard | Controller |
|---|---|---|
| Rotate around the character | hold **right mouse button** and move the mouse | **right stick** |
| Zoom | **right mouse button + mouse wheel** | hold **R3** + right stick up/down |
| Camera up / down | **right mouse button + Q / E**, or **Page Up / Page Down** | hold **R3** + **D-pad up/down** |
| Reset | **middle mouse button** | **tap R3** (press and release quickly without moving the stick) |

Mouse and controller can be used at the same time; nothing has to be switched.

## Why these buttons
LooksMenu already uses many keys and buttons (checked in its own interface file, LooksMenu.swf 1.6.19):

| Input | What LooksMenu does with it |
|---|---|
| Mouse movement | moves the cursor, sculpting |
| Mouse wheel | scrolls lists, "Slide" in sculpt mode |
| E | Accept / Select / Extras |
| Q | Skin (body mode) |
| LT / RT | face preset, clothes on/off, skin, slider steps |
| D-pad | sculpt slide and scale, overlay order, slider values |
| Right stick, R3, right/middle mouse button, Page Up/Down | not used |

So the camera uses the free inputs (right mouse button, middle mouse button, right stick, R3, Page Up/Down) on their own, and the busy ones (mouse movement, wheel, Q/E, D-pad) only while the right mouse button or R3 is held. Whatever the camera uses is not passed on to LooksMenu, so the cursor, lists and sliders do not move while you orbit.

## Changing them (LooksMenuOrbitCamera.ini)
- `iRotateMouseButton` – 1 left, 2 right (default), 3 middle, 0 = mouse movement always rotates (the cursor then moves too)
- `bWheelZoomOnlyWhileRotating=0` – the wheel always zooms (LooksMenu then no longer gets the wheel)
- `iKeyCameraUp` / `iKeyCameraDown` – keys used together with the rotate button (default Q / E)
- `iKeyCameraUpAlways` / `iKeyCameraDownAlways` – keys that work on their own (default Page Up / Page Down)
- `iKeyReset` – an extra reset key (0 = none)
- `bTriggerZoom=1` – LT / RT zoom (LT out, RT in). LooksMenu then no longer gets LT / RT.
- `iDpadRaiseLower` – 0 off, 1 = while R3 is held (default), 2 = always (LooksMenu then no longer gets D-pad up/down)
- `bLeftStickPositioning=1` – left stick up/down moves the camera height (LooksMenu then no longer gets the left stick)
- `bInvertVertical`, `bInvertHorizontal`
- `bBlockUsedInputFromLooksMenu=0` – LooksMenu also receives everything the camera uses

Key codes are Windows virtual-key codes in hex, for example 0x51 = Q, 0x45 = E, 0x21 = Page Up, 0x22 = Page Down, 0x24 = Home, 0x23 = End.
