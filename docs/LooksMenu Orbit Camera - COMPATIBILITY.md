# Compatibility

## Game and tools
| | |
|---|---|
| 1.10.163 + F4SE 0.6.23 | supported, tested in game |
| 1.10.980, 1.10.984 + F4SE 0.7.0 – 0.7.3 | supported, not tested in game |
| 1.11.137, 1.11.159, 1.11.169, 1.11.191, 1.11.221, 1.11.240 + F4SE 0.7.4 – 0.7.9 | supported, not tested in game |
| Any other version | the plugin does nothing (it writes "Unknown game version" in its log) |
| GOG / Epic builds | addresses are the Steam ones; the plugin checks the camera and menu structures before using them and stays off if they do not match |
| LooksMenu (1.6.19 for 1.10.163, 1.7.x for next-gen) | supported; LooksMenu's files are not changed |

Before it changes anything, the plugin checks that the camera is where it expects (13 camera states, the third-person one with the right id, all inside the game) and that the menu list looks right. If a check fails, it writes "check FAILED" in the log and leaves the game alone.
| Windows 10 / 11 | yes |
| Vortex / manual install | yes |

## Controllers
- **Xbox controllers and other XInput controllers:** supported. The plugin reads the same controller input as the game (XInput, first controller).
- **PlayStation controllers:** Fallout 4 itself only reads XInput controllers, so a PlayStation controller works only when something presents it as an Xbox controller: Steam Input (Steam → Settings → Controller → PlayStation Configuration Support), DS4Windows or similar. Then the orbit camera works with it exactly like an Xbox controller (R3 = right stick click). Without such a tool neither the game nor this plugin sees the controller.

## Other mods
- **Camera mods** (True Third Person, Commonwealth Camera, Comprehensive 3rd Person Camera Tweaks, Full Body First Person …): these were installed during testing. Outside LooksMenu the plugin does nothing to the camera, so they work as before. Inside LooksMenu the orbit camera decides the view.
- **Wall collision:** the orbit camera does not test for walls itself. During testing the camera was sometimes pulled closer when zoomed far out into a ceiling; that came from the game or another camera mod and is harmless. In a small room the camera can still end up behind a wall: zoom in or move round.
- **Other plugins that read the mouse or controller:** the plugin only takes inputs while LooksMenu is open and only the ones listed in CONTROLS.md. Outside LooksMenu every input reaches the game untouched.
- **Editing NPCs with LooksMenu** (`slm`): supported. The camera circles the NPC LooksMenu is editing (or the character selected in the console), the NPC is frozen, and the player's first-person arms and weapon are hidden while the menu is open. If no NPC can be found when the menu opens, the camera waits until it is loaded (written in the log).

## How it works (technical)
- After the game has placed the camera for the frame (the active camera state's Update), the plugin sets the camera node's position and rotation and lets the node update its children, as the game does itself. No game code is patched; only the camera states' function tables are changed.
- Mouse and keyboard input are read from the game's own raw input (USER32 GetRawInputData) and the controller from XInputGetState (XINPUT1_3), through the game's import table.
- Addresses per game version come from the F4SE source at the matching release tag (0.6.23 and 0.7.0 – 0.7.9): GameCamera.cpp g_playerCamera, GameMenus.cpp g_ui, GameReferences.cpp g_player, GameCustomization.cpp g_characterCreation / g_characterIndex (1.2.0). The structure offsets used are the same in all versions (F4SE headers; checked against the 1.11.191 exe, whose camera code uses exactly these offsets and the same camera function table).
- F4SE 0.7.x reads the plugin's version data (F4SEPlugin_Version) and loads it on 1.10.980 – 1.11.240; F4SE 0.6.23 calls F4SEPlugin_Query on 1.10.163.
- On next-gen the log goes to the My Games folder F4SE reports for that build.
- Log: `Documents\My Games\Fallout4\F4SE\LooksMenuOrbitCamera.log`

## Freezing the character (1.3.0)
- The freeze holds the position and rotation of every node of the character's 3D, every frame. Mods that move bones by themselves while LooksMenu is open (physics such as HDT-SMP hair or cloth, head tracking) are held still too.
- LooksMenu body and face sliders change the mesh, not the bones, so they still show. Bone scale changes still show; bone position/rotation edits made inside LooksMenu (if a LooksMenu version offers them) would be held at the opening pose until the menu closes.
- Set `bFreezeCharacter=0` if another mod should keep animating the character.
