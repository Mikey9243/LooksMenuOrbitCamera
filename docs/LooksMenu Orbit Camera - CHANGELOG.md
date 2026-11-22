# Changelog

## 1.4.1
- Fixed: at the start of a new game, after switching to the other character at the mirror (for example from Nate to Nora), the other character stood in the way of the camera. The newly chosen character was frozen at the moment of the switch, before the game had walked it to the mirror. The freeze now waits until the character has stopped moving, the camera follows it while it walks, and then puts itself in front of it.

## 1.4.0
- Release of the NPC support built in 1.2.0 - 1.3.6:
  - The camera circles whoever LooksMenu is editing: your own character, or an NPC opened with `slm` (companions, settlers, named and generic NPCs). Same controls, settings and smoothing as for the player.
  - The edited character is frozen while LooksMenu is open and moves normally again when it closes (`bFreezeCharacter`).
  - While an NPC is edited, the player's first-person arms and weapon are hidden from the camera and restored on closing.

## 1.3.6
- Fixed: the player's first-person arms still showed while circling an NPC (1.3.5 only set the hidden flag, which the game ignored). The first-person model is now also shrunk to nothing while LooksMenu is open on an NPC, and restored exactly when it closes.

## 1.3.5
- The character selected in the console is used for `slm`: when LooksMenu opens on an NPC, the camera circles the NPC LooksMenu is editing, or else the character selected in the console.
- Only real characters are used: a selection with almost no 3D (a marker or placeholder - the 1.3.3 log's stale selection had 3 nodes) is ignored.
- If no NPC can be found when LooksMenu opens, the camera waits and starts as soon as the NPC is loaded, instead of circling the wrong thing.
- The player's own LooksMenu (third-person camera) always circles the player.

## 1.3.4
- Fixed: while an NPC was edited, the player's first-person arms and weapon kept getting in the way of the camera (the game keeps the camera in first person for NPCs). They are now hidden while LooksMenu is open and shown again when it closes.

## 1.3.3
- The edited character is now watched for the whole time LooksMenu is open. If the game fills in the NPC a moment after the menu opens, the camera and the freeze switch to that NPC at once.
- The log records every change of the game's character-creation slots while LooksMenu is open.

## 1.3.2
- Fixed: with an NPC (`slm`) the camera turned sideways and flew around - the same mistake as 0.1.x. With `slm` the game leaves the camera in first person, pointed away from the NPC, and the plugin worked out the camera's axes from that view. It now only does that when the camera is clearly aimed at the character, and otherwise uses the standard axes every session has shown. The camera starts in front of the NPC at chest height, like with the player.
- Diagnostics: if the edited character's 3D looks too small to freeze, its node tree is written to the log.

## 1.3.1
- Fixed: NPCs edited with `slm` were not found. The game's character-creation slot still holds the player then, and the camera stays in first person at the player's head. The orbit target is now chosen from the console's selected reference, all character-creation slots and the player - whichever character LooksMenu's view is pointed at. The freeze uses the same character.
- The log lists every candidate and why the target was chosen.

## 1.3.0
- The character being edited - the player or an NPC - is frozen while LooksMenu is open: no idle animation, no turning, no walking off. It holds the pose and spot it had when the menu opened and moves normally again as soon as LooksMenu closes.
- Nothing is written to the character's data: only the displayed pose is held. Scale is not held, so LooksMenu's own changes still show.
- New INI setting `bFreezeCharacter` (1 = on, the default; 0 = off).

## 1.2.0
- NPC support: the camera now orbits whoever LooksMenu is editing - the player, companions, settlers, named or generic NPCs (for example with the console command `slm`). The character is read from the game's own character-creation data, the same data LooksMenu edits, so it works for every way LooksMenu is opened.
- When an NPC is edited, the orbit centre, the reset view and the starting view use that NPC's position, facing and size (a scaled character gets a scaled distance and focus height).
- If the camera is not aimed at the edited NPC when LooksMenu opens, the camera moves to it instead of switching off.
- Same camera, controls, settings and smoothing as before; nothing new to configure. The log says which character is orbited.

## 1.1.1
- Fixed: 1.1.0 did not work at all. Its new start-up check ran before the game had created its camera states, decided the camera "did not look right" and switched the orbit camera off. The check now waits until the camera states exist.

## 1.1.0
- Works on every Fallout 4 version F4SE exists for: 1.10.163, 1.10.980, 1.10.984, 1.11.137, 1.11.159, 1.11.169, 1.11.191, 1.11.221 and 1.11.240 (per-version addresses from the F4SE source, version data for F4SE 0.7.x).
- New safety checks: the camera and the menu list are checked before anything is changed; if they do not look right the plugin stays off and says so in the log.
- On next-gen the log is written to the My Games folder that build uses.

## 1.0.0
- Release. Same camera as 0.2.0; extra diagnostic lines in the log are now off by default (`bDebugLog=1` turns them on).

## 0.2.0 (test)
- Fixed: the camera faced the wrong way from the sides (it pointed at walls). The game stores node rotations in the opposite layout to the one 0.1.x wrote; the turn was mirrored except straight in front and behind.

## 0.1.1 (test)
- Diagnostics: camera node tree, and a check whether anything moves the camera after the plugin.

## 0.1.0 (test)
- First version: orbit, zoom, height, reset, mouse/keyboard and XInput controller, INI.
