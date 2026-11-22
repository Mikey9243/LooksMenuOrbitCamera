/*
 * LooksMenu Orbit Camera 1.4.0 - F4SE plugin for every Fallout 4 version F4SE supports:
 *   1.10.163 (F4SE 0.6.23) and next-gen / anniversary 1.10.980 - 1.11.240 (F4SE 0.7.0 - 0.7.9).
 *
 * While the LooksMenu menu is open, the camera orbits around the character being edited (the player or an NPC
 * opened with slm); the character is held still, and for NPCs the player's first-person model is hidden.
 *
 * How it works (no game code is patched, only pointer tables):
 *  - Camera: after the game has placed the camera for this frame (the active camera state's virtual Update,
 *    TESCameraState vtable slot 0x0B), the plugin overwrites the camera root node's local transform and runs the
 *    node's own update (NiAVObject vtable slot 0x30, UpdateDownwardPass) - exactly what the game's ThirdPersonState
 *    does at the end of its Update. When the orbit is off, nothing is written, so the game's camera comes back by
 *    itself on the next frame.
 *  - Input: the game's import-table entries for USER32 GetRawInputData (mouse) and XINPUT1_3 XInputGetState
 *    (ordinal 2, controller) are redirected. While the orbit camera uses an input (right-mouse drag, wheel while
 *    dragging, right stick, triggers, R3, ...) that input is removed before the game sees it, so LooksMenu's
 *    sliders and lists do not react to it. Keys (Q/E etc.) are read with GetAsyncKeyState.
 *  - Edited character: the game's character-creation slots and the console's selected reference (F4SE
 *    g_characterCreation / g_characterIndex, g_consoleHandle + LookupREFRByHandle).
 *  - Freeze: the position/rotation of each node of the character's 3D is put back every frame while the menu is open.
 *  - Addresses for each game version are listed in k_versions, taken from the F4SE source at each release tag.
 *    Structure layouts: F4SE headers / CommonLibF4.
 *  - No files are written except the log; no network access; no other process is touched.
 *
 * Log:    Documents\My Games\Fallout4\F4SE\LooksMenuOrbitCamera.log
 * Config: Data\F4SE\Plugins\LooksMenuOrbitCamera.ini
 * Freestanding C (no CRT), built with clang -target x86_64-pc-windows-msvc + lld-link (see build.sh).
 */
typedef unsigned char u8; typedef unsigned short u16; typedef unsigned int u32; typedef unsigned long long u64;
typedef signed short s16; typedef signed int s32; typedef long long s64; typedef void* HANDLE; typedef int BOOL;
#define WINAPI __stdcall
#define NULL ((void*)0)
__declspec(dllimport) HANDLE WINAPI GetModuleHandleA(const char*);
__declspec(dllimport) u32 WINAPI GetModuleFileNameA(HANDLE, char*, u32);
__declspec(dllimport) HANDLE WINAPI CreateFileA(const char*, u32, u32, void*, u32, u32, HANDLE);
__declspec(dllimport) BOOL WINAPI WriteFile(HANDLE, const void*, u32, u32*, void*);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) BOOL WINAPI VirtualProtect(void*, u64, u32, u32*);
__declspec(dllimport) u32 WINAPI GetPrivateProfileStringA(const char*, const char*, const char*, char*, u32, const char*);
__declspec(dllimport) BOOL WINAPI QueryPerformanceCounter(s64*);
__declspec(dllimport) BOOL WINAPI QueryPerformanceFrequency(s64*);
__declspec(dllimport) u32 WINAPI GetCurrentProcessId(void);
__declspec(dllimport) BOOL WINAPI IsBadReadPtr(const void*, u64);
__declspec(dllimport) s16 WINAPI GetAsyncKeyState(int);
__declspec(dllimport) HANDLE WINAPI GetForegroundWindow(void);
__declspec(dllimport) u32 WINAPI GetWindowThreadProcessId(HANDLE, u32*);
__declspec(dllimport) long WINAPI SHGetFolderPathA(void*, int, HANDLE, u32, char*);
int _fltused = 0;
void* memset(void* d, int c, u64 n) { u8* p = d; while (n--) *p++ = (u8)c; return d; }
void* memcpy(void* d, const void* s, u64 n) { u8* p = d; const u8* q = s; while (n--) *p++ = *q++; return d; }

#define LMOC_VERSION_STR "1.4.4"
#define LMOC_VERSION_NUM 0x01040400
/* 1.1.0: every Fallout 4 version F4SE exists for. Addresses (offsets from the exe base) of the three game singletons
   used, taken from the F4SE source at each release tag: GameCamera.cpp g_playerCamera, GameMenus.cpp g_ui,
   GameReferences.cpp g_player. The structure offsets used are the same in all of them (F4SE headers 0.6.23 - 0.7.9;
   checked against the 1.11.191 exe) and are also checked at run time before anything is hooked. */
/* 1.2.0: + the character-creation slots (GameCustomization.cpp g_characterCreation, an array of CharacterCreation*, and
   g_characterIndex, the slot in use). CharacterCreation +0x58 is the Actor being edited - the actor LooksMenu edits,
   the player or an NPC (slm). Same layout in F4SE 0.6.23 - 0.7.9. */
typedef struct { u32 runtime; u32 camera, ui, player; const char* name; u32 cc, ccIndex, consoleHandle, lookupHandle; } VersionEntry;
static const VersionEntry k_versions[] = {
    { 0x010A0A30, 0x058CEB28, 0x058D0898, 0x05AA4388, "1.10.163", 0x05A67420, 0x05A67440, 0x05ADB4A8, 0x0000AB60 },   /* F4SE 0.6.23 */
    { 0x010A3D40, 0x02E639D8, 0x02E65400, 0x03039CA0, "1.10.980", 0x02FF7180, 0x02FF7158, 0x0306EEA0, 0x0024C210 },   /* F4SE 0.7.0 / 0.7.1 */
    { 0x010A3D80, 0x02E649D8, 0x02E66400, 0x0303ACA0, "1.10.984", 0x02FF8180, 0x02FF8158, 0x0306FEA0, 0x0024C240 },   /* F4SE 0.7.2 / 0.7.3 */
    { 0x010B0890, 0x030CAAD8, 0x030CC5B0, 0x032BC250, "1.11.137", 0x0325E518, 0x0325E4F0, 0x032F1720, 0x0029D5E0 },   /* F4SE 0.7.4 */
    { 0x010B09F0, 0x030CBA58, 0x030CD530, 0x032BD1F0, "1.11.159", 0x0325F498, 0x0325F470, 0x032F26A0, 0x0029D720 },   /* F4SE 0.7.5 */
    { 0x010B0A90, 0x030D0C58, 0x030D2730, 0x032C23F0, "1.11.169", 0x03264698, 0x03264670, 0x032F78A0, 0x0029DB10 },   /* F4SE 0.7.6 */
    { 0x010B0BF0, 0x030DBD58, 0x030DD830, 0x032D2260, "1.11.191", 0x0326F798, 0x0326F770, 0x03307720, 0x002A0540 },   /* F4SE 0.7.7 */
    { 0x010B0DD0, 0x030DBDD8, 0x030DD8B0, 0x032D22E0, "1.11.221", 0x0326F818, 0x0326F7F0, 0x033077A0, 0x002A0540 },   /* F4SE 0.7.8 */
    { 0x010B0F00, 0x030E6E58, 0x030E8930, 0x032DD370, "1.11.240", 0x0327A8A8, 0x0327A880, 0x03312820, 0x002A0860 },   /* F4SE 0.7.9 */
};
#define NUM_VERSIONS (sizeof(k_versions) / sizeof(k_versions[0]))
static u32 RVA_PLAYER_CAMERA, RVA_UI, RVA_PLAYER, RVA_CC, RVA_CC_INDEX, RVA_CONSOLE, RVA_LOOKUP;
static const VersionEntry* find_version(u32 runtime) {
    u32 i;
    for (i = 0; i < NUM_VERSIONS; i++) if (k_versions[i].runtime == (runtime & ~0xFu)) return &k_versions[i];   /* low 4 bits: Steam/GOG/Epic build type */
    return 0;
}

/* F4SE 0.7.x (next-gen / anniversary) reads this export instead of calling F4SEPlugin_Query */
typedef struct {
    u32 dataVersion, pluginVersion; char name[256]; char author[256];
    u32 addressIndependence, structureIndependence; u32 compatibleVersions[16];
    u32 seVersionRequired, reservedNonBreaking, reservedBreaking; u8 reserved[512];
} F4SEPluginVersionData;
__declspec(dllexport) F4SEPluginVersionData F4SEPlugin_Version = {
    1, 0x01010100, "LooksMenuOrbitCamera", "LooksMenu Orbit Camera",
    0,               /* hard-coded per-version addresses (table above), so the version list below decides */
    (1<<1)|(1<<2),   /* the structures used have the same layout in the 1.10.980 and 1.11.137 builds */
    { 0x010A3D40, 0x010A3D80, 0x010B0890, 0x010B09F0, 0x010B0A90, 0x010B0BF0, 0x010B0DD0, 0x010B0F00, 0 },
    0, 0, 0, {0}
};

/* ------------------------------------------------------------------ small helpers */
static u32 slen(const char* s) { u32 n = 0; while (s && s[n]) n++; return n; }
static int lower(int c) { return (c >= 'A' && c <= 'Z') ? c + 32 : c; }
static int streq_i(const char* a, const char* b) { while (*a && *b) { if (lower(*a) != lower(*b)) return 0; a++; b++; } return *a == *b; }
static char g_logPath[340], g_iniPath[340];
static int g_logLines;
static void log_raw(const char* s) {
    u32 w; HANDLE h;
    if (!g_logPath[0] || g_logLines > 4000) return;
    g_logLines++;
    h = CreateFileA(g_logPath, 4, 3, 0, 4, 0x80, 0);
    if (h == (HANDLE)(long long)-1) return;
    WriteFile(h, s, slen(s), &w, 0); WriteFile(h, "\r\n", 2, &w, 0); CloseHandle(h);
}
/* tiny printf: %s %d %u %x %p %f (3 decimals) */
static void logf(const char* fmt, ...) {
    char buf[600]; u32 n = 0; __builtin_va_list ap; __builtin_va_start(ap, fmt);
    for (; *fmt && n < sizeof(buf) - 40; fmt++) {
        if (*fmt != '%') { buf[n++] = *fmt; continue; }
        fmt++;
        if (*fmt == 's') { const char* s = __builtin_va_arg(ap, const char*); if (!s) s = "(null)"; while (*s && n < sizeof(buf) - 40) buf[n++] = *s++; }
        else if (*fmt == 'd' || *fmt == 'u') {
            s64 v = (*fmt == 'd') ? (s64)__builtin_va_arg(ap, int) : (s64)__builtin_va_arg(ap, u32); char t[24]; int k = 0;
            if (v < 0) { buf[n++] = '-'; v = -v; }
            do { t[k++] = (char)('0' + v % 10); v /= 10; } while (v);
            while (k) buf[n++] = t[--k];
        } else if (*fmt == 'x' || *fmt == 'p') {
            u64 v = (*fmt == 'x') ? (u64)__builtin_va_arg(ap, u32) : (u64)__builtin_va_arg(ap, void*); int k, started = 0;
            for (k = 60; k >= 0; k -= 4) { int d = (int)((v >> k) & 15); if (d || started || k == 0) { buf[n++] = "0123456789ABCDEF"[d]; started = 1; } }
        } else if (*fmt == 'f') {
            double v = __builtin_va_arg(ap, double); s64 ip; s64 fp; char t[24]; int k = 0;
            if (v != v) { buf[n++] = 'n'; buf[n++] = 'a'; buf[n++] = 'n'; continue; }
            if (v < 0) { buf[n++] = '-'; v = -v; }
            if (v > 1e12) v = 1e12;
            ip = (s64)v; fp = (s64)((v - (double)ip) * 1000.0 + 0.5); if (fp >= 1000) { ip++; fp -= 1000; }
            do { t[k++] = (char)('0' + ip % 10); ip /= 10; } while (ip);
            while (k) buf[n++] = t[--k];
            buf[n++] = '.'; buf[n++] = (char)('0' + fp / 100); buf[n++] = (char)('0' + (fp / 10) % 10); buf[n++] = (char)('0' + fp % 10);
        } else buf[n++] = *fmt;
    }
    __builtin_va_end(ap); buf[n] = 0; log_raw(buf);
}

/* ------------------------------------------------------------------ math (no CRT) */
#define PI 3.14159265358979f
static float fsqrt(float x) { float r; if (x <= 0) return 0; __asm__("sqrtss %1, %0" : "=x"(r) : "x"(x)); return r; }
static float fabsf_(float x) { return x < 0 ? -x : x; }
static float wrap_pi(float a) { while (a > PI) a -= 2 * PI; while (a < -PI) a += 2 * PI; return a; }
static float fsin(float x) {   /* range-reduced Taylor/minimax, |err| < 2e-6 */
    float x2, s = 1;
    x = wrap_pi(x);
    if (x > PI / 2) x = PI - x; else if (x < -PI / 2) x = -PI - x;
    x2 = x * x; (void)s;
    return x * (1 - x2 / 6 * (1 - x2 / 20 * (1 - x2 / 42 * (1 - x2 / 72 * (1 - x2 / 110)))));
}
static float fcos(float x) { return fsin(x + PI / 2); }
static float fatan(float x) {   /* |x| <= 1, minimax, err ~1e-5 */
    float x2 = x * x;
    return x * (0.99997726f + x2 * (-0.33262347f + x2 * (0.19354346f + x2 * (-0.11643287f + x2 * (0.05265332f + x2 * -0.01172120f)))));
}
static float fatan2(float y, float x) {
    float ax = fabsf_(x), ay = fabsf_(y), r;
    if (ax < 1e-9f && ay < 1e-9f) return 0;
    if (ax >= ay) { r = fatan(ay / ax); } else { r = PI / 2 - fatan(ax / ay); }
    if (x < 0) r = PI - r;
    if (y < 0) r = -r;
    return r;
}
static float fasin(float x) { if (x > 1) x = 1; if (x < -1) x = -1; return fatan2(x, fsqrt(1 - x * x)); }
static float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
/* 1 - e^(-k*dt) without exp(): stable for any k*dt >= 0 */
static float smooth_alpha(float k, float dt) {
    float x = k * dt, e;
    if (k <= 0) return 1;
    if (x > 10) return 1;
    e = 1 + x / 8; e = e * e; e = e * e; e = e * e;   /* (1 + x/8)^8 ~ e^x */
    return 1 - 1 / e;
}
typedef struct { float x, y, z; } V3;
static V3 v3(float x, float y, float z) { V3 r; r.x = x; r.y = y; r.z = z; return r; }
static V3 vsub(V3 a, V3 b) { return v3(a.x - b.x, a.y - b.y, a.z - b.z); }
static V3 vadd(V3 a, V3 b) { return v3(a.x + b.x, a.y + b.y, a.z + b.z); }
static V3 vmul(V3 a, float s) { return v3(a.x * s, a.y * s, a.z * s); }
static float vdot(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static V3 vcross(V3 a, V3 b) { return v3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }
static float vlen(V3 a) { return fsqrt(vdot(a, a)); }
static V3 vnorm(V3 a) { float l = vlen(a); return l > 1e-6f ? vmul(a, 1 / l) : v3(0, 1, 0); }

/* ------------------------------------------------------------------ configuration */
typedef struct {
    int enable, enableController, enableMouseKeyboard, autoInput, startFromLooksMenuView, debugLog, freeze;
    float distance, minZoom, maxZoom, sensitivity, smoothing, vertSensitivity, focusHeight, minPitch, maxPitch;
    float mouseZoomStep, heightSpeed, minFocus, maxFocus;
    int invertY, invertX, rotateButton, wheelNeedsRotateButton, keyUp, keyDown, keyReset;
    float rStickSens, lStickSens, deadZone, zoomSpeed;
    int enableReset, enableVertical, blockGameInput, dpadHeight, leftStickHeight, triggerZoom, keyUpAlways, keyDownAlways;
} Config;
static Config C;
static float ini_f(const char* sec, const char* key, float def) {
    char b[64]; const char* p = b; float v = 0, sc = 0.1f; int neg = 0, any = 0;
    GetPrivateProfileStringA(sec, key, "", b, sizeof(b), g_iniPath);
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '-') { neg = 1; p++; } else if (*p == '+') p++;
    while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); p++; any = 1; }
    if (*p == '.') { p++; while (*p >= '0' && *p <= '9') { v += sc * (*p - '0'); sc *= 0.1f; p++; any = 1; } }
    if (!any) return def;
    return neg ? -v : v;
}
static int ini_i(const char* sec, const char* key, int def) {
    char b[64]; const char* p = b; int v = 0, neg = 0, any = 0, base = 10;
    GetPrivateProfileStringA(sec, key, "", b, sizeof(b), g_iniPath);
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '-') { neg = 1; p++; }
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) { base = 16; p += 2; }
    for (;; p++) {
        int d = (*p >= '0' && *p <= '9') ? *p - '0' : (base == 16 && lower(*p) >= 'a' && lower(*p) <= 'f') ? lower(*p) - 'a' + 10 : -1;
        if (d < 0) break; v = v * base + d; any = 1;
    }
    if (!any) return def;
    return neg ? -v : v;
}
static void load_config(void) {
    C.enable              = ini_i("General", "bEnableOrbitCamera", 1);
    C.enableController    = ini_i("General", "bEnableController", 1);
    C.enableMouseKeyboard = ini_i("General", "bEnableMouseKeyboard", 1);
    C.autoInput           = ini_i("General", "bAutomaticInputDetection", 1);
    C.startFromLooksMenuView = ini_i("General", "bStartFromLooksMenuView", 1);
    C.debugLog            = ini_i("General", "bDebugLog", 0);
    C.blockGameInput      = ini_i("General", "bBlockUsedInputFromLooksMenu", 1);
    C.freeze              = ini_i("General", "bFreezeCharacter", 1);
    C.distance        = clampf(ini_f("Camera", "fCameraDistance", 150.0f), 10, 2000);
    C.minZoom         = clampf(ini_f("Camera", "fMinimumZoom", 40.0f), 5, 2000);
    C.maxZoom         = clampf(ini_f("Camera", "fMaximumZoom", 400.0f), C.minZoom + 1, 4000);
    C.sensitivity     = clampf(ini_f("Camera", "fCameraSensitivity", 1.0f), 0.01f, 20);
    C.smoothing       = clampf(ini_f("Camera", "fCameraSmoothing", 8.0f), 0, 100);
    C.vertSensitivity = clampf(ini_f("Camera", "fVerticalSensitivity", 1.0f), 0.01f, 20);
    C.focusHeight     = clampf(ini_f("Camera", "fFocusHeight", 90.0f), -50, 400);
    C.minFocus        = ini_f("Camera", "fMinimumFocusHeight", 0.0f);
    C.maxFocus        = ini_f("Camera", "fMaximumFocusHeight", 200.0f);
    if (C.maxFocus < C.minFocus + 1) C.maxFocus = C.minFocus + 1;
    C.minPitch        = clampf(ini_f("Camera", "fMinimumPitch", -60.0f), -89, 89) * PI / 180;
    C.maxPitch        = clampf(ini_f("Camera", "fMaximumPitch", 80.0f), -89, 89) * PI / 180;
    if (C.maxPitch < C.minPitch) C.maxPitch = C.minPitch;
    C.mouseZoomStep   = clampf(ini_f("Camera", "fMouseWheelZoomStep", 0.12f), 0.01f, 0.9f);
    C.heightSpeed     = clampf(ini_f("Camera", "fHeightAdjustSpeed", 60.0f), 1, 1000);
    C.invertY         = ini_i("Camera", "bInvertVertical", 0);
    C.invertX         = ini_i("Camera", "bInvertHorizontal", 0);
    C.rStickSens      = clampf(ini_f("Controller", "fRightStickSensitivity", 2.0f), 0.01f, 20);
    C.lStickSens      = clampf(ini_f("Controller", "fLeftStickSensitivity", 1.0f), 0.01f, 20);
    C.deadZone        = clampf(ini_f("Controller", "fDeadZone", 0.18f), 0, 0.9f);
    C.zoomSpeed       = clampf(ini_f("Controller", "fZoomSpeed", 5.0f), 0.1f, 100);
    C.dpadHeight      = ini_i("Controller", "iDpadRaiseLower", 1);       /* 0 off, 1 while R3 is held, 2 always */
    C.triggerZoom     = ini_i("Controller", "bTriggerZoom", 0);
    C.leftStickHeight = ini_i("Controller", "bLeftStickPositioning", 0);
    C.enableReset     = ini_i("Controls", "bEnableResetCamera", 1);
    C.enableVertical  = ini_i("Controls", "bEnableVerticalAdjustment", 1);
    C.rotateButton    = ini_i("Controls", "iRotateMouseButton", 2);   /* 1 left, 2 right, 3 middle, 0 = always rotate */
    C.wheelNeedsRotateButton = ini_i("Controls", "bWheelZoomOnlyWhileRotating", 1);
    C.keyUp           = ini_i("Controls", "iKeyCameraUp", 0x51);      /* Q */
    C.keyDown         = ini_i("Controls", "iKeyCameraDown", 0x45);    /* E */
    C.keyReset        = ini_i("Controls", "iKeyReset", 0);            /* 0 = none (middle mouse resets) */
    C.keyUpAlways     = ini_i("Controls", "iKeyCameraUpAlways", 0x21);   /* Page Up */
    C.keyDownAlways   = ini_i("Controls", "iKeyCameraDownAlways", 0x22); /* Page Down */
}

/* ------------------------------------------------------------------ game structures (1.10.163) */
static u64 g_base, g_modEnd;
static int in_module(const void* p) { return (u64)p >= g_base && (u64)p < g_modEnd; }
#define PTR(p, off, T) (*(T*)((u8*)(p) + (off)))
typedef struct { float m[3][4]; V3 t; float s; } NiTransform;   /* F4SE NiTransform: NiMatrix43 rot, pos, scale */
static void* player_camera(void) { return *(void**)(g_base + RVA_PLAYER_CAMERA); }
static void* player_ref(void) { return *(void**)(g_base + RVA_PLAYER); }

/* is a menu with this name on the UI menu stack? (UI +0x190 tArray<IMenu*>: entries +0x190, count +0x1A0; IMenu +0x50 name) */
static const char* entry_str(void* e) { return e ? (const char*)e + 0x18 : 0; }
static int g_lastStackHash, g_uiChecked;
static int menu_open(const char* name, int logStack) {
    u8* ui = *(u8**)(g_base + RVA_UI); void** entries; u32 count, i; int found = 0, hash = 0;
    char line[512]; u32 n = 0;
    if (!ui) return 0;
    entries = *(void***)(ui + 0x190); count = *(u32*)(ui + 0x1A0);
    if (!entries || count > 64) return 0;
    if (!g_uiChecked) {   /* first time: check the menu stack is where expected */
        g_uiChecked = (!IsBadReadPtr(entries, 8 * (u64)count) && in_module(*(void**)ui)) ? 1 : -1;
        logf("menu list check %s", g_uiChecked == 1 ? "passed" : "FAILED - LooksMenu cannot be detected on this game build");
    }
    if (g_uiChecked != 1) return 0;
    for (i = 0; i < count; i++) {
        void* menu = entries[i]; const char* s;
        if (!menu) continue;
        s = entry_str(*(void**)((u8*)menu + 0x50));
        if (!s) continue;
        if (streq_i(s, name)) found = 1;
        if (logStack) { const char* q = s; hash = hash * 31 + (int)(u64)menu; if (n < 400) { while (*q && n < 400) line[n++] = *q++; line[n++] = ' '; } }
    }
    if (logStack && C.debugLog && hash != g_lastStackHash) { g_lastStackHash = hash; line[n] = 0; logf("menu stack: %s", line); }
    return found;
}

/* ------------------------------------------------------------------ input (shared between hooks and the camera update) */
static volatile s32 g_mouseDX, g_mouseDY, g_wheel;   /* accumulated since the last camera frame */
static volatile int g_rotateHeld, g_resetRequest, g_keyUpHeld, g_keyDownHeld, g_sawKeyboard, g_sawMouse;
static volatile int g_orbitActive;                   /* camera override running this frame */
typedef struct { u16 buttons; u8 lt, rt; s16 lx, ly, rx, ry; } Pad;
static volatile int g_padValid; static Pad g_pad;
static s64 g_lastPadTime;

static int game_has_focus(void) {
    u32 pid = 0; HANDLE w = GetForegroundWindow();
    if (!w) return 0;
    GetWindowThreadProcessId(w, &pid);
    return pid == GetCurrentProcessId();
}

/* GetRawInputData hook (USER32 import of Fallout4.exe) */
typedef u32 (WINAPI *GetRawInputData_t)(HANDLE, u32, void*, u32*, u32);
static GetRawInputData_t o_GetRawInputData;
#define RID_INPUT 0x10000003
static u32 WINAPI hk_GetRawInputData(HANDLE h, u32 cmd, void* data, u32* size, u32 hdr) {
    u32 r = o_GetRawInputData(h, cmd, data, size, hdr);
    if (cmd == RID_INPUT && data && r != (u32)-1 && r >= 24 + 16 && C.enableMouseKeyboard && g_orbitActive) {
        u8* raw = (u8*)data;
        if (*(u32*)raw == 1) {   /* RIM_TYPEKEYBOARD: Q/E (configurable) move the camera up/down while the rotate button is held */
            u8* k = raw + 24; u16 kflags = *(u16*)(k + 2), vk = *(u16*)(k + 6); int up = kflags & 1;
            if (!g_sawKeyboard) { g_sawKeyboard = 1; logf("keyboard raw input seen (Q/E while rotating can be kept away from LooksMenu)"); }
            if ((vk == C.keyUp || vk == C.keyDown) && vk) {
                volatile int* st = (vk == C.keyUp) ? &g_keyUpHeld : &g_keyDownHeld;
                if (!up && (g_rotateHeld || C.rotateButton == 0)) { *st = 1; if (C.blockGameInput) { *(u16*)(k + 6) = 0xFF; *(u16*)k = 0; } }
                else if (up && *st) { *st = 0; if (C.blockGameInput) { *(u16*)(k + 6) = 0xFF; *(u16*)k = 0; } }
            }
        }
        if (*(u32*)raw == 0) {   /* RIM_TYPEMOUSE */
            u8* m = raw + 24; u16 flags = *(u16*)m, bf = *(u16*)(m + 4); s16 bd = *(s16*)(m + 6);
            s32* lx = (s32*)(m + 12); s32* ly = (s32*)(m + 16);
            u16 downMask = 0, upMask = 0, keep = bf;
            if (!g_sawMouse) { g_sawMouse = 1; logf("mouse raw input seen"); }
            if (C.rotateButton == 1) { downMask = 0x1; upMask = 0x2; }
            else if (C.rotateButton == 2) { downMask = 0x4; upMask = 0x8; }
            else if (C.rotateButton == 3) { downMask = 0x10; upMask = 0x20; }
            if (bf & downMask) { g_rotateHeld = 1; keep &= ~downMask; }
            if (bf & upMask) { g_rotateHeld = 0; keep &= ~upMask; }
            if ((bf & 0x10) && C.enableReset && C.rotateButton != 3) { g_resetRequest = 1; keep &= ~0x30; }
            if (bf & 0x20 && C.rotateButton != 3) keep &= ~0x20;
            if (bf & 0x0400) {   /* RI_MOUSE_WHEEL */
                if (!C.wheelNeedsRotateButton || g_rotateHeld || C.rotateButton == 0) { g_wheel += bd; keep &= ~0x0400; }
            }
            if (!(flags & 1) && (g_rotateHeld || C.rotateButton == 0)) {   /* relative movement */
                g_mouseDX += *lx; g_mouseDY += *ly;
                if (C.blockGameInput && C.rotateButton != 0) { *lx = 0; *ly = 0; }
            }
            if (C.blockGameInput) { *(u16*)(m + 4) = keep; if (!(keep & 0x0400)) *(u16*)(m + 6) = 0; }
        }
    }
    return r;
}

/* XInputGetState hook (XINPUT1_3 ordinal 2) */
typedef u32 (WINAPI *XInputGetState_t)(u32, void*);
static XInputGetState_t o_XInputGetState;
static void try_install_camera(void);
static u32 WINAPI hk_XInputGetState(u32 user, void* st) {
    u32 r = o_XInputGetState(user, st);
    try_install_camera();
    if (r != 0 && user == 0) g_padValid = 0;   /* controller unplugged: no stale stick values */
    if (r == 0 && st && C.enableController && g_orbitActive) {
        u8* g = (u8*)st + 4;   /* XINPUT_STATE: dwPacketNumber, XINPUT_GAMEPAD */
        Pad p; s64 now;
        p.buttons = *(u16*)g; p.lt = g[2]; p.rt = g[3]; p.lx = *(s16*)(g + 4); p.ly = *(s16*)(g + 6); p.rx = *(s16*)(g + 8); p.ry = *(s16*)(g + 10);
        if (user == 0 || !g_padValid) { g_pad = p; g_padValid = 1; QueryPerformanceCounter(&now); g_lastPadTime = now; }
        if (C.blockGameInput) {   /* LooksMenu itself never uses the right stick or R3; the rest only when the camera uses it */
            *(s16*)(g + 8) = 0; *(s16*)(g + 10) = 0;   /* right stick: orbit (with R3 held: zoom) */
            *(u16*)g &= ~0x0080;                        /* R3: tap = reset, hold = modifier */
            if (C.triggerZoom) { g[2] = 0; g[3] = 0; }  /* triggers are LooksMenu buttons unless trigger zoom is switched on */
            if (C.dpadHeight == 2 || (C.dpadHeight == 1 && (p.buttons & 0x80))) *(u16*)g &= ~0x0003;
            if (C.leftStickHeight) { *(s16*)(g + 4) = 0; *(s16*)(g + 6) = 0; }
        }
    }
    return r;
}

/* replace one import of the exe; returns the old function or 0 */
static void* patch_import(const char* dll, const char* func, u32 ordinal, void* hook) {
    u8* b = (u8*)g_base; u32 pe = *(u32*)(b + 0x3C); u32 impRva = *(u32*)(b + pe + 24 + 112 + 8);
    u32* d;
    if (!impRva) return 0;
    for (d = (u32*)(b + impRva); d[3]; d += 5) {
        const char* name = (const char*)(b + d[3]); u64* ilt; u64* iat;
        if (!streq_i(name, dll)) continue;
        ilt = (u64*)(b + (d[0] ? d[0] : d[4])); iat = (u64*)(b + d[4]);
        for (; *ilt; ilt++, iat++) {
            int match = 0;
            if (*ilt >> 63) match = (ordinal && (u32)(*ilt & 0xFFFF) == ordinal);
            else match = (func && streq_i((const char*)(b + (u32)*ilt + 2), func));
            if (match) {
                u32 old; void* prev = (void*)*iat;
                if (!VirtualProtect(iat, 8, 0x04, &old)) return 0;
                *iat = (u64)hook; VirtualProtect(iat, 8, old, &old);
                return prev;
            }
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ camera state Update hook */
typedef void (*StateUpdate_t)(void* state, void* nextState);
typedef struct { void** vtbl; StateUpdate_t orig; } VHook;
static VHook g_vh[16]; static int g_nvh;
static void hk_StateUpdate(void* state, void* next);
static int patch_vtable(void** vtbl) {
    int i; u32 old;
    for (i = 0; i < g_nvh; i++) if (g_vh[i].vtbl == vtbl) return 1;
    if (g_nvh >= 16 || !vtbl) return 0;
    if (vtbl[0x0B] == (void*)hk_StateUpdate) return 1;
    if (!VirtualProtect(&vtbl[0x0B], 8, 0x04, &old)) return 0;
    g_vh[g_nvh].vtbl = vtbl; g_vh[g_nvh].orig = (StateUpdate_t)vtbl[0x0B];
    g_nvh++;
    vtbl[0x0B] = (void*)hk_StateUpdate;
    VirtualProtect(&vtbl[0x0B], 8, old, &old);
    logf("camera state vtable %p hooked (Update was %p, exe+%x)", vtbl, (void*)g_vh[g_nvh - 1].orig, (u32)((u64)g_vh[g_nvh - 1].orig - g_base));
    return 1;
}
static volatile int g_installed, g_validated;
static void try_install_camera(void) {
    u8* pc; int i, n = 0;
    if (g_installed) return;
    pc = (u8*)player_camera();
    if (!pc) return;
    if (!g_validated) {   /* make sure this really is the camera, laid out as expected, before changing anything */
        /* every camera state must be a real game object (vtable inside the exe); the third-person state LooksMenu
           uses (8) must say it is state 8; most states must point back to this camera */
        int good = 0, idOk = 0, backOk = 0;
        if (IsBadReadPtr(pc, 0x150) || !in_module(*(void**)pc)) { g_validated = -1; g_installed = 1; logf("camera check FAILED (player camera not found) - the orbit camera stays off"); return; }
        for (i = 0; i < 13; i++) {
            u8* st = *(u8**)(pc + 0xE0 + 8 * i);
            if (!st || IsBadReadPtr(st, 0x28) || !in_module(*(void**)st)) continue;
            good++;
            if (*(u32*)(st + 0x20) == (u32)i) idOk++;
            if (*(void**)(st + 0x18) == pc) backOk++;
        }
        /* 1.1.1: the camera states are created after the camera itself (1.1.0 checked too early and switched itself
           off) - wait until all 13 exist; only if some stay missing for a long time judge what is there */
        { static int tries; if (good < 13 && ++tries < 20000) return; }
        {
            u8* st8 = *(u8**)(pc + 0xE0 + 8 * 8);
            int ok8 = st8 && !IsBadReadPtr(st8, 0x28) && *(u32*)(st8 + 0x20) == 8;
            logf("camera check: %d of 13 states valid, %d with matching id, %d pointing back to the camera, third-person state %s", good, idOk, backOk, ok8 ? "ok" : "WRONG");
            if (good < 10 || !ok8) { g_validated = -1; g_installed = 1; logf("camera check FAILED - this game build is not laid out as expected; the orbit camera stays off"); return; }
        }
        g_validated = 1;
    }
    for (i = 0; i < 13; i++) {   /* PlayerCamera +0xE0: cameraStates[13] */
        void* st = *(void**)(pc + 0xE0 + 8 * i);
        if (st && patch_vtable(*(void***)st)) n++;
    }
    if (n) { g_installed = 1; logf("camera hooks installed on %d camera states (%d different classes)", n, g_nvh); }
}

/* ------------------------------------------------------------------ orbit camera */
typedef struct { float yaw, pitch, dist, focus; } Orbit;
static Orbit g_cur, g_tgt;
static int g_wasOpen, g_active, g_axisF, g_axisU, g_axisT; static float g_sF, g_sU, g_sT;
static s64 g_lastTime, g_freq; static u32 g_frames;
static int g_skipLogged;

/* 1.2.0: the orbit centre is the actor LooksMenu is editing (player or NPC), chosen when the menu opens */
static void* g_target;            /* TESObjectREFR* of the orbited character */
static float g_targetScale = 1;   /* its reference scale (refScale / 100) */
static int ref_ok(void* r) {
    float* p;
    if (!r || ((u64)r & 7) || IsBadReadPtr(r, 0x110) || !in_module(*(void**)r)) return 0;
    p = &PTR(r, 0xD0, float);
    return p[0] == p[0] && p[1] == p[1] && p[2] == p[2] && fabsf_(p[0]) < 1e7f && fabsf_(p[1]) < 1e7f && fabsf_(p[2]) < 1e7f;
}
/* the actor in the character-creation slot in use, or 0 */
static void* chargen_actor(u32* slotOut) {
    void** arr; u32 idx; void* cc; void* a;
    if (!RVA_CC || !RVA_CC_INDEX) return 0;
    if (IsBadReadPtr((void*)(g_base + RVA_CC_INDEX), 4) || IsBadReadPtr((void*)(g_base + RVA_CC), 8 * 4)) return 0;
    idx = *(u32*)(g_base + RVA_CC_INDEX); if (slotOut) *slotOut = idx;
    if (idx > 3) return 0;
    arr = (void**)(g_base + RVA_CC); cc = arr[idx];
    if (!cc || ((u64)cc & 7) || IsBadReadPtr(cc, 0x70)) return 0;
    a = PTR(cc, 0x58, void*);
    return ref_ok(a) ? a : 0;
}
static void* target_ref(void) { return (g_target && ref_ok(g_target)) ? g_target : player_ref(); }
static int g_fzOnFwd; static V3 g_fzPosFwd; static float g_fzHeadingFwd;   /* frozen spot (set by the freeze code) */
static V3 player_pos(void) { u8* p; if (g_fzOnFwd) return g_fzPosFwd; p = (u8*)target_ref(); return p ? PTR(p, 0xD0, V3) : v3(0, 0, 0); }
static float player_heading(void) { u8* p; if (g_fzOnFwd) return g_fzHeadingFwd; p = (u8*)target_ref(); return p ? PTR(p, 0xC8, float) : 0; }
static float ref_scale(void* r) { u16 s = r ? PTR(r, 0x108, u16) : 100; return (s >= 20 && s <= 500) ? s / 100.0f : 1.0f; }
/* does the camera's view ray pass close to this character's vertical axis (in front of the camera)? */
__attribute__((unused)) static float view_miss(V3 cam, V3 dir, void* r, float* along, float* height) {
    V3 base = PTR(r, 0xD0, V3), rel = vsub(base, cam); float t = (dir.x * rel.x + dir.y * rel.y) / (dir.x * dir.x + dir.y * dir.y + 1e-6f);
    V3 closest = vadd(cam, vmul(dir, t));
    if (along) *along = t; if (height) *height = closest.z - base.z;
    return t <= 0 ? 1e9f : vlen(v3(closest.x - base.x, closest.y - base.y, 0));
}
/* 0.2.0: the game stores node rotations transposed (row j of the stored 3x4 matrix = the node's local axis j in
   world space). 0.1.x read and wrote columns, which mirrored the left/right turn of the view whenever the camera was
   not straight in front of or behind the character (the "all over the place" camera). Confirmed from the 0.1.1 log:
   the NiCamera child's world matrix is the root's with its rows permuted. */
static V3 mcol(const NiTransform* t, int j) { return v3(t->m[j][0], t->m[j][1], t->m[j][2]); }
static V3 orbit_cam_pos(const Orbit* o, V3 base) {
    V3 f = vadd(base, v3(0, 0, o->focus)); float cp = fcos(o->pitch);
    return vadd(f, v3(o->dist * cp * fsin(o->yaw), o->dist * cp * fcos(o->yaw), o->dist * fsin(o->pitch)));
}
static void reset_orbit(Orbit* o);
static void reset_orbit_fwd(void) { reset_orbit(&g_tgt); }
static void* nearby_other(float* outDist);
static void reset_orbit(Orbit* o) {
    void* other; float od = 0;
    o->yaw = wrap_pi(player_heading());   /* camera in front of the character, looking at its face side */
    /* 1.4.2: if another character stands close by (the spouse at the mirror in a new game), put the camera on the far
       side from it, so it can never stand between the camera and the edited character. */
    other = nearby_other(&od);
    if (other) {
        V3 me = player_pos(), ot = PTR(other, 0xD0, V3); V3 d = vsub(me, ot);
        if (d.x * d.x + d.y * d.y > 1) { o->yaw = fatan2(d.x, d.y); logf("  another character %f units away - camera placed on the far side from it", od); }
    }
    /* 1.2.0: distance and focus height follow the character's size (a scaled NPC gets a scaled view) */
    o->pitch = 0.087f; o->dist = clampf(C.distance * g_targetScale, C.minZoom, C.maxZoom); o->focus = clampf(C.focusHeight * g_targetScale, C.minFocus, C.maxFocus);
}

/* first frame: work out how the camera node's axes map to forward/up (calibration) and where the view starts */

/* ------------------------------------------------------------------ 1.3.0: freeze the edited character
   While LooksMenu is open the character's whole 3D (skeleton and everything attached) is held in the pose it had when
   the menu opened: every frame, after the game has animated it, the position and rotation of each node are put back
   and the world transforms are recomputed. Scale is left alone, so LooksMenu's own changes still show. Nothing is
   written to the character's data; on closing the game simply animates it again. Only nodes that are in the live 3D
   this frame are touched (clothes or the whole 3D can be replaced while the menu is open). */
#define FZ_MAX 4096
typedef struct { void* node; float m[3][4]; V3 t; } FzEntry;
static FzEntry g_fz[FZ_MAX]; static int g_fzCount, g_fzOn, g_fzLogs; static u32 g_fzFrames, g_fzBusy; static void* g_fzRoot; static void* g_fzRef;
static u16 g_fzHash[8192];   /* open addressing: index + 1 into g_fz, 0 = empty */
static V3 g_fzPos; static float g_fzHeading;
static u32 fz_h(void* p) { u64 x = (u64)p >> 4; x ^= x >> 17; x *= 0x9E3779B97F4A7C15ull; return (u32)(x >> 51) & 8191; }
static int fz_find(void* n) { u32 h = fz_h(n), k; for (k = 0; k < 64; k++) { u16 v = g_fzHash[(h + k) & 8191]; if (!v) return -1; if (g_fz[v - 1].node == n) return v - 1; } return -1; }
static void fz_add(void* n) {
    u32 h = fz_h(n), k; NiTransform* l = &PTR(n, 0x30, NiTransform);
    if (g_fzCount >= FZ_MAX) return;
    for (k = 0; k < 64; k++) if (!g_fzHash[(h + k) & 8191]) {
        FzEntry* e = &g_fz[g_fzCount]; e->node = n; memcpy(e->m, l->m, sizeof(e->m)); e->t = l->t;
        g_fzHash[(h + k) & 8191] = (u16)(++g_fzCount); return;
    }
}
static void* ref_root3d(void* r) {
    void* ld = r ? PTR(r, 0xF0, void*) : 0; void* root;
    if (!ld || IsBadReadPtr(ld, 0x10)) return 0;
    root = PTR(ld, 0x08, void*);
    return (root && !((u64)root & 7) && !IsBadReadPtr(root, 0x140) && in_module(*(void**)root)) ? root : 0;
}
static void* as_ninode(void* o) { typedef void* (*F)(void*); return (*(F**)o)[3](o); }
/* walks the live 3D; snapshot = 1: remember new nodes, restore = 1: put remembered nodes back. Returns nodes changed. */
static int fz_walk(void* root, int restore) {
    void* stack[512]; int sp = 0, visited = 0, changed = 0, i;
    stack[sp++] = root;
    while (sp && visited < 6000) {
        void* n = stack[--sp]; void* nn; void** ch; u16 cnt; int idx;
        visited++;
        idx = fz_find(n);
        if (idx < 0) fz_add(n);
        else if (restore) {
            NiTransform* l = &PTR(n, 0x30, NiTransform); FzEntry* e = &g_fz[idx];
            if (l->t.x != e->t.x || l->t.y != e->t.y || l->t.z != e->t.z || l->m[0][0] != e->m[0][0] || l->m[1][1] != e->m[1][1] || l->m[2][2] != e->m[2][2] || l->m[0][1] != e->m[0][1]) {
                memcpy(l->m, e->m, sizeof(e->m)); l->t = e->t; changed++;
            }
        }
        nn = as_ninode(n);
        if (!nn) continue;
        ch = PTR(n, 0x128, void**); cnt = PTR(n, 0x132, u16);
        if (!ch || cnt > 1000 || IsBadReadPtr(ch, cnt * 8ull)) continue;
        for (i = 0; i < cnt && sp < 512; i++) { void* c = ch[i]; if (c && !((u64)c & 7) && !IsBadReadPtr(c, 0x140) && in_module(*(void**)c)) stack[sp++] = c; }
    }
    return changed;
}
static const char* node_name(void* o);
static void fz_dump(void* n, int depth) {   /* 1.3.2: diagnostics when the character's 3D looks too small */
    void** ch; u16 cnt; int i; void* nn;
    if (!n || depth > 4) return;
    nn = as_ninode(n); ch = nn ? PTR(n, 0x128, void**) : 0; cnt = nn ? PTR(n, 0x132, u16) : 0;
    logf("  freeze 3D: depth %d %s (vtable exe+%x, node %d, children %u, scale %f)", depth, node_name(n), (u32)((u64)*(void**)n - g_base), nn != 0, cnt, PTR(n, 0x6C, float));
    if (!ch || cnt > 64 || IsBadReadPtr(ch, cnt * 8ull)) return;
    for (i = 0; i < cnt; i++) {
        void* c = ch[i];
        if (!c) continue;
        if (((u64)c & 7) || IsBadReadPtr(c, 0x140) || !in_module(*(void**)c)) { logf("  freeze 3D: depth %d child %d %p skipped (not readable or not a game object)", depth + 1, i, c); continue; }
        fz_dump(c, depth + 1);
    }
}
static void freeze_begin(void* ref) {
    void* root;
    g_fzOn = 0; g_fzCount = 0; g_fzRoot = 0; g_fzRef = 0; memset(g_fzHash, 0, sizeof(g_fzHash)); g_fzLogs = 0; g_fzFrames = g_fzBusy = 0;
    if (!C.freeze || !ref_ok(ref)) return;
    root = ref_root3d(ref);
    if (!root) { logf("  freeze: the character has no 3D - not frozen"); return; }
    g_fzRef = ref; g_fzRoot = root; g_fzPos = PTR(ref, 0xD0, V3); g_fzHeading = PTR(ref, 0xC8, float);
    fz_walk(root, 0);
    if (g_fzCount < 30) fz_dump(root, 0);
    g_fzOn = 1; g_fzOnFwd = 1; g_fzPosFwd = g_fzPos; g_fzHeadingFwd = g_fzHeading;
    logf("  character frozen: %d nodes held in their pose", g_fzCount);
}

/* 1.4.1: the freeze waits until the character has stopped moving. At the start of a new game the game walks the two
   characters into place when you switch between them at the mirror; 1.4.0 froze the newly chosen one at the moment of
   the switch, before it had walked to the mirror, so it stayed behind (invisible spot) while the other character stood
   in the camera's way. The orbit follows the character's live position until the freeze starts. */
static void* g_fzPending; static V3 g_fzLastPos; static int g_fzStill, g_fzWaited, g_fzResetOrbit;
static void freeze_request(void* ref, int resetOrbit) {
    g_fzPending = ref; g_fzStill = 0; g_fzWaited = 0; g_fzResetOrbit = resetOrbit;
    if (ref_ok(ref)) g_fzLastPos = PTR(ref, 0xD0, V3);
}
static void reset_orbit_fwd(void);
static void freeze_pending_frame(void) {
    V3 p; float d;
    if (!g_fzPending) return;
    if (!ref_ok(g_fzPending)) { g_fzPending = 0; return; }
    p = PTR(g_fzPending, 0xD0, V3); d = vlen(vsub(p, g_fzLastPos)); g_fzLastPos = p;
    g_fzWaited++;
    if (d > 0.05f) g_fzStill = 0; else g_fzStill++;
    if (g_fzStill >= 20 || g_fzWaited >= 600) {   /* 20 frames without moving (or 10 seconds at most) */
        void* r = g_fzPending; g_fzPending = 0;
        logf("  character stood still after %d frames - freezing it now", g_fzWaited);
        freeze_begin(r);
        if (g_fzResetOrbit) reset_orbit_fwd();
    }
}
static void freeze_frame(void) {
    void* root; int changed; typedef void (*U)(void*, void*, u32); u8 ud[0x40];
    if (!g_fzOn) return;
    if (!ref_ok(g_fzRef)) { g_fzOn = 0; g_fzOnFwd = 0; logf("  freeze: the character went away - stopped"); return; }
    root = ref_root3d(g_fzRef);
    if (!root) return;
    if (root != g_fzRoot) {   /* the whole 3D was rebuilt (e.g. LooksMenu reloaded the character): hold the new one */
        g_fzCount = 0; memset(g_fzHash, 0, sizeof(g_fzHash)); g_fzRoot = root; fz_walk(root, 0);
        logf("  freeze: the character's 3D was rebuilt - holding the new pose (%d nodes)", g_fzCount);
        return;
    }
    changed = fz_walk(root, 1);
    if (changed) { memset(ud, 0, sizeof(ud)); (*(U**)root)[0x30](root, ud, 0); }
    g_fzFrames++; if (changed) g_fzBusy++;
    if (g_fzLogs < (C.debugLog ? 20 : 3)) { g_fzLogs++; logf("  freeze: %d nodes put back this frame", changed); }
}
static void freeze_end(void) {
    if (g_fzOn) logf("  character unfrozen (held for %u frames; the game had moved it in %u of them)", g_fzFrames, g_fzBusy);
    g_fzOn = 0; g_fzOnFwd = 0; g_fzRoot = 0; g_fzRef = 0; g_fzCount = 0;
}
/* 1.3.1: the console's selected reference (slm edits it). F4SE GameAPI.cpp g_consoleHandle + GameHandle.cpp
   LookupREFRByHandle, which hands back a counted pointer: the count is given back at once (the reference stays loaded
   because it is in the world; only its pointer is kept while LooksMenu is open). */
static void* console_ref(u32* handleOut) {
    typedef u8 (*Lookup)(const u32*, void**); u32 h; void* r = 0;
    if (!RVA_CONSOLE || !RVA_LOOKUP || IsBadReadPtr((void*)(g_base + RVA_CONSOLE), 4)) return 0;
    h = *(u32*)(g_base + RVA_CONSOLE); if (handleOut) *handleOut = h;
    if (!h) return 0;
    if (!((Lookup)(g_base + RVA_LOOKUP))(&h, &r) || !r) return 0;
    { volatile u32* rc = &PTR(r, 0x28, u32); if ((*rc & 0x3FF) > 1) __atomic_fetch_sub(rc, 1, __ATOMIC_SEQ_CST); }
    return ref_ok(r) && ref_root3d(r) ? r : 0;
}
/* number of nodes in a reference's 3D (up to 'limit'): a real character has hundreds, a marker or a corpse
   placeholder a handful (the 1.3.3 log: the stale console selection 21E19E had 3) */
static int body_nodes(void* r, int limit) {
    void* stack[256]; int sp = 0, n = 0, i; void* root = ref_ok(r) ? ref_root3d(r) : 0;
    if (!root) return 0;
    stack[sp++] = root;
    while (sp && n < limit) {
        void* x = stack[--sp]; void** ch; u16 cnt; n++;
        if (!as_ninode(x)) continue;
        ch = PTR(x, 0x128, void**); cnt = PTR(x, 0x132, u16);
        if (!ch || cnt > 1000 || IsBadReadPtr(ch, cnt * 8ull)) continue;
        for (i = 0; i < cnt && sp < 256; i++) { void* c = ch[i]; if (c && !((u64)c & 7) && !IsBadReadPtr(c, 0x140) && in_module(*(void**)c)) stack[sp++] = c; }
    }
    return n;
}
static void choose_target(V3 cam, V3 cols[3], int stateId) {
    /* 1.3.5 rules (from the in-game logs):
       - the player's own LooksMenu (barber, surgeon, start of the game) keeps the third-person camera;
       - an NPC (slm) keeps the camera in first person (state 0);
       - the game's character-creation slot holds the NPC once LooksMenu has loaded it, sometimes only later;
       - the console selection is what slm was used on, if it is a real character.
       So: an NPC in the slot wins; in first person, the console selection with a real body comes next; otherwise
       the player (third person) or nothing yet (first person - the slot is watched and the camera starts as soon as
       the NPC appears). */
    void* pl = player_ref(); u32 slot = 99, ch = 0; void* cc = chargen_actor(&slot); void* con = console_ref(&ch);
    int ccNodes = cc ? body_nodes(cc, 60) : 0, conNodes = con ? body_nodes(con, 60) : 0; const char* why;
    (void)cam; (void)cols;
    logf("  candidates: slot %u actor %p (%d+ nodes), console selection %p handle %x (%d+ nodes), player %p, camera state %d",
         slot, cc, ccNodes, con, ch, conNodes, pl, stateId);
    if (cc && cc != pl && ccNodes >= 30) { g_target = cc; why = "the NPC LooksMenu is editing"; }
    else if (stateId == 0 && con && con != pl && conNodes >= 30) { g_target = con; why = "the character selected in the console"; }
    else if (stateId != 0) { g_target = pl; why = "the player"; }
    else { g_target = 0; why = "no NPC found yet - waiting for LooksMenu to load it"; }
    g_targetScale = g_target ? ref_scale(g_target) : 1;
    {
        void* base = g_target ? PTR(g_target, 0xE0, void*) : 0;
        u32 refId = g_target ? PTR(g_target, 0x14, u32) : 0, baseId = (base && !IsBadReadPtr(base, 0x18)) ? PTR(base, 0x14, u32) : 0;
        logf("  orbit target: %s (%s); reference %x, base %x, scale %f", !g_target ? "NONE" : g_target == pl ? "PLAYER" : "NPC", why, refId, baseId, g_targetScale);
    }
}
/* 1.3.3: the game may fill in the edited character a little after LooksMenu has opened (the 1.3.2 log: at the opening
   frame the slot in use still held the player, while an NPC was being edited). Watch the slots while the menu is open
   and switch to a new character the moment one appears. */
static int g_fpN; static void fp_hide(void);
static void* g_lastSlotActor[4]; static u32 g_lastSlotIdx = 99, g_watchFrames;

/* 1.4.2: the closest other character (player or a character-creation slot actor) within 250 units of the target */
static void* nearby_other(float* outDist) {
    void* cands[5]; int n = 0, i; void* best = 0; float bestD = 250; V3 me; void* pl = player_ref();
    if (!g_target) return 0;
    me = player_pos();
    if (pl && pl != g_target) cands[n++] = pl;
    if (RVA_CC && !IsBadReadPtr((void*)(g_base + RVA_CC), 32)) for (i = 0; i < 4; i++) {
        void* c = ((void**)(g_base + RVA_CC))[i]; void* a = (c && !((u64)c & 7) && !IsBadReadPtr(c, 0x70)) ? PTR(c, 0x58, void*) : 0;
        if (a && a != g_target && a != pl && ref_ok(a)) cands[n++] = a;
    }
    for (i = 0; i < n; i++) {
        V3 p; float d;
        if (!ref_ok(cands[i])) continue;
        p = PTR(cands[i], 0xD0, V3); d = vlen(v3(p.x - me.x, p.y - me.y, 0));
        if (d < bestD) { bestD = d; best = cands[i]; }
    }
    if (outDist) *outDist = bestD;
    return best;
}
static void target_watch(void) {
    u32 idx = 99; void* a = chargen_actor(&idx); int i; void* pl = player_ref();
    g_watchFrames++;
    if (RVA_CC && !IsBadReadPtr((void*)(g_base + RVA_CC), 32)) for (i = 0; i < 4; i++) {
        void* c = ((void**)(g_base + RVA_CC))[i]; void* ca = (c && !((u64)c & 7) && !IsBadReadPtr(c, 0x70)) ? PTR(c, 0x58, void*) : 0;
        if (ca != g_lastSlotActor[i]) { logf("  frame %u: character-creation slot %d now holds actor %p", g_watchFrames, i, ca); g_lastSlotActor[i] = ca; }
    }
    if (idx != g_lastSlotIdx) {
        int first = g_lastSlotIdx == 99;
        logf("  frame %u: character-creation slot in use is now %u", g_watchFrames, idx); g_lastSlotIdx = idx;
        /* 1.4.2: back to the player (new game: switching from Nora back to Nate, or the other way round) */
        if (!first && a == pl && pl && g_target && g_target != pl && g_active) {
            u8* pc = (u8*)player_camera(); void* st = pc ? PTR(pc, 0x28, void*) : 0;
            if (st && PTR(st, 0x20, int) != 0) {
                g_target = pl; g_targetScale = ref_scale(pl);
                logf("  frame %u: LooksMenu is editing the player again - the camera and the freeze switch back", g_watchFrames);
                freeze_end(); freeze_request(pl, 1); reset_orbit(&g_tgt);
            }
        }
    }
    if (!g_target && g_watchFrames % 15 == 0) {   /* nothing found at the opening: the console selection may load late too */
        u32 ch = 0; void* con = console_ref(&ch);
        if (con && con != pl && body_nodes(con, 60) >= 30 && (!a || a == pl)) a = con;
    }
    if (a && a != pl && a != g_target && body_nodes(a, 60) >= 30) {
        u32 refId = PTR(a, 0x14, u32);
        g_target = a; g_targetScale = ref_scale(a);
        logf("  frame %u: LooksMenu is editing NPC %x - the camera and the freeze switch to it", g_watchFrames, refId);
        { V3 p = PTR(a, 0xD0, V3), q = pl ? PTR(pl, 0xD0, V3) : v3(0, 0, 0); logf("    NPC at %f %f %f heading %f, player at %f %f %f heading %f", p.x, p.y, p.z, PTR(a, 0xC8, float), q.x, q.y, q.z, pl ? PTR(pl, 0xC8, float) : 0.0f); }
        freeze_end(); freeze_request(a, 1);
        reset_orbit(&g_tgt);
        if (!g_active) {   /* the orbit had not started (no target at the opening): start it now with the standard axes */
            g_axisF = 1; g_sF = 1; g_axisU = 2; g_sU = 1; g_axisT = 0; g_sT = 1;
            g_cur = g_tgt; g_active = 1; g_orbitActive = 1;
            logf("  orbit started on the NPC: distance %f focus height %f", g_tgt.dist, g_tgt.focus);
        }
        if (!g_fpN && g_active) { u8* pc = (u8*)player_camera(); void* st = pc ? PTR(pc, 0x28, void*) : 0; if (st && PTR(st, 0x20, int) == 0) fp_hide(); }
    }
}

/* 1.3.4: while an NPC is edited the game keeps the camera in first person, so the player's first-person arms and
   weapon hang in front of the orbiting camera. They are hidden (NiAVObject flags +0x108, bit 0 = app-culled) for as
   long as LooksMenu is open, and the flags are put back exactly as they were on closing. */
static void* g_fpHide[3]; static u64 g_fpFlags[3];
static void fp_hide(void) {
    u8* pc = (u8*)player_ref(); void* n; int i;
    g_fpN = 0;
    if (!pc || IsBadReadPtr(pc + 0xB78, 8)) return;
    n = PTR(pc, 0xB78, void*);   /* PlayerCharacter::firstPersonSkeleton (F4SE 0.6.23 - 0.7.9) */
    for (i = 0; i < 2 && n && !((u64)n & 7) && !IsBadReadPtr(n, 0x120) && in_module(*(void**)n); i++) {
        void* par = PTR(n, 0x28, void*); u16 cnt = as_ninode(n) ? PTR(n, 0x132, u16) : 0;
        if (i > 0 && (cnt > 6 || !par)) break;   /* only the first-person root right above the skeleton, never a scene node */
        g_fpHide[g_fpN] = n; g_fpFlags[g_fpN] = PTR(n, 0x108, u64); g_fpN++;
        logf("  first-person model hidden: %s (children %u, scale %f, parent %s)", node_name(n), cnt, PTR(n, 0x6C, float), par ? node_name(par) : "none");
        n = par;
    }
}
/* 1.3.6: the hidden flag alone did not remove the arms in game (1.3.5). The first-person model is also shrunk to
   nothing: the local scale of its root (skeleton.nif) is set to 0.0001 every frame - skinned arms, hands and the
   weapon all hang from it - and put back on closing. */
static float g_fpScale; static int g_fpScaled;
static void fp_update(void* n) { typedef void (*U)(void*, void*, u32); u8 ud[0x40]; memset(ud, 0, sizeof(ud)); (*(U**)n)[0x30](n, ud, 0); }
static void fp_frame(void) {
    int i;
    for (i = 0; i < g_fpN; i++) if (!IsBadReadPtr(g_fpHide[i], 0x110)) PTR(g_fpHide[i], 0x108, u64) |= 1;
    if (g_fpN && !IsBadReadPtr(g_fpHide[0], 0x110)) {
        float* sc = &PTR(g_fpHide[0], 0x6C, float);
        if (!g_fpScaled) { g_fpScale = *sc; g_fpScaled = 1; }
        if (*sc != 0.0001f) { *sc = 0.0001f; fp_update(g_fpHide[0]); }
    }
}
static void fp_restore(void) {
    int i;
    if (g_fpN && g_fpScaled && !IsBadReadPtr(g_fpHide[0], 0x110)) {
        PTR(g_fpHide[0], 0x6C, float) = (g_fpScale > 0.01f && g_fpScale < 100) ? g_fpScale : 1.0f; fp_update(g_fpHide[0]);
    }
    g_fpScaled = 0;
    for (i = 0; i < g_fpN; i++) if (!IsBadReadPtr(g_fpHide[i], 0x110)) PTR(g_fpHide[i], 0x108, u64) = (PTR(g_fpHide[i], 0x108, u64) & ~1ull) | (g_fpFlags[i] & 1);
    if (g_fpN) logf("  first-person model shown again");
    g_fpN = 0;
}

/* 1.4.3: new game, at the mirror. When the spouse is edited the game puts the spouse exactly where the player stands
   (1.4.2 log: same position and heading), so the player's body overlaps the spouse and shows around her once the
   camera moves away from her face. While LooksMenu edits another character that stands within 60 units of the
   player, the player's third-person body is shrunk to nothing (local scale of its 3D root) and put back as soon as
   that is no longer the case or the menu closes. */
static void* g_tpNode; static void* g_tpRef; static float g_tpScale; static int g_tpOn;
static void tp_restore(const char* why) {
    if (g_tpOn && g_tpNode && !IsBadReadPtr(g_tpNode, 0x110)) {
        PTR(g_tpNode, 0x6C, float) = (g_tpScale > 0.01f && g_tpScale < 100) ? g_tpScale : 1.0f; fp_update(g_tpNode);
        logf("  hidden character's body shown again (%s)", why);
    }
    g_tpOn = 0; g_tpNode = 0; g_tpRef = 0;
}
/* 1.4.4: the other way round too - when the player is edited again at the mirror, the spouse stands in the player's
   spot. Whoever overlaps the edited character (the player, or a character in another character-creation slot) is
   hidden. */
static void* overlapping_other(void) {
    void* cands[5]; int n = 0, i; void* pl = player_ref(); V3 b;
    if (!g_target || !ref_ok(g_target)) return 0;
    b = PTR(g_target, 0xD0, V3);
    if (pl && pl != g_target) cands[n++] = pl;
    if (RVA_CC && !IsBadReadPtr((void*)(g_base + RVA_CC), 32)) for (i = 0; i < 4; i++) {
        void* c = ((void**)(g_base + RVA_CC))[i]; void* a = (c && !((u64)c & 7) && !IsBadReadPtr(c, 0x70)) ? PTR(c, 0x58, void*) : 0;
        if (a && a != g_target && a != pl) cands[n++] = a;
    }
    for (i = 0; i < n; i++) {
        V3 a;
        if (!ref_ok(cands[i])) continue;
        a = PTR(cands[i], 0xD0, V3);
        if (vlen(v3(a.x - b.x, a.y - b.y, a.z - b.z)) < 60) return cands[i];
    }
    return 0;
}
static void tp_frame(void) {
    void* other = overlapping_other(); void* root = other ? ref_root3d(other) : 0;
    if (!root) { if (g_tpOn) tp_restore("it no longer overlaps the edited character"); return; }
    if (g_tpOn && (root != g_tpNode || other != g_tpRef)) tp_restore("another character overlaps now");
    if (!g_tpOn) {
        g_tpNode = root; g_tpRef = other; g_tpScale = PTR(root, 0x6C, float); g_tpOn = 1;
        logf("  %s stands where the edited character stands - its body is hidden while LooksMenu edits the other one", other == player_ref() ? "the player" : "another character");
    }
    if (PTR(root, 0x6C, float) != 0.0001f) { PTR(root, 0x6C, float) = 0.0001f; fp_update(root); }
}
static int begin_orbit(void* root, int stateId) {
    NiTransform* w = &PTR(root, 0x70, NiTransform); V3 cam = w->t, base, d, toAxis, cols[3];
    float best = -1, bestU = -1, along, horiz; int j;
    for (j = 0; j < 3; j++) cols[j] = mcol(w, j);
    choose_target(cam, cols, stateId);
    if (!g_target) return 0;
    base = player_pos();
    /* which node axis points from the camera towards the character's vertical axis */
    toAxis = vsub(v3(base.x, base.y, cam.z), cam);
    horiz = vlen(toAxis);
    if (horiz < 1) toAxis = v3(0, 1, 0);
    toAxis = vnorm(toAxis);
    g_axisF = 0;
    for (j = 0; j < 3; j++) { float k = vdot(cols[j], toAxis); if (fabsf_(k) > best) { best = fabsf_(k); g_axisF = j; g_sF = k < 0 ? -1.f : 1.f; } }
    g_axisU = (g_axisF + 1) % 3;
    for (j = 0; j < 3; j++) { float k; if (j == g_axisF) continue; k = vdot(cols[j], v3(0, 0, 1)); if (fabsf_(k) > bestU) { bestU = fabsf_(k); g_axisU = j; g_sU = k < 0 ? -1.f : 1.f; } }
    g_axisT = 3 - g_axisF - g_axisU;
    g_sT = vdot(cols[g_axisT], vcross(cols[g_axisF], cols[g_axisU])) < 0 ? -1.f : 1.f;
    logf("LooksMenu opened: camera state %d, camera at %f %f %f, orbit target at %f %f %f heading %f", stateId, cam.x, cam.y, cam.z, base.x, base.y, base.z, player_heading());
    logf("  camera node axes: forward = axis %d (%f, match %f), up = axis %d (%f, match %f), third = axis %d (%f); parent %p",
         g_axisF, g_sF, best, g_axisU, g_sU, bestU, g_axisT, g_sT, PTR(root, 0x28, void*));
    if (best < 0.9f || bestU < 0.9f) {
        /* 1.3.2: only a camera that is clearly aimed at the character can be calibrated from it. Otherwise (NPC with slm:
           the camera stays in first person, pointed anywhere - the 1.3.1 log picked axis 0, which turned the view
           sideways) use the layout every session showed: forward = axis 1, up = axis 2, third = axis 0, all +1. */
        logf("  camera not aimed at the character (forward match %f, up match %f) - using the standard camera axes", best, bestU);
        g_axisF = 1; g_sF = 1; g_axisU = 2; g_sU = 1; g_axisT = 0; g_sT = 1; best = 1; bestU = 1;
    }
    if (best < 0.5f || bestU < 0.3f) { logf("  calibration unclear (forward match %f, up match %f) - orbit camera not used this time", best, bestU); return 0; }
    /* where does the camera's view ray pass the character? */
    d = vmul(cols[g_axisF], g_sF);
    {
        V3 rel = vsub(base, cam); float t = (d.x * rel.x + d.y * rel.y) / (d.x * d.x + d.y * d.y + 1e-6f);
        V3 closest = vadd(cam, vmul(d, t)); float miss = vlen(v3(closest.x - base.x, closest.y - base.y, 0));
        along = t;
        logf("  view ray passes the character's axis %f units away at height %f (distance %f)", miss, closest.z - base.z, t);
        if (miss > 120 || t <= 0) {
            if (g_target == player_ref()) { logf("  the camera is not looking at the player character - orbit camera not used this time"); return 0; }
            logf("  the camera is not looking at the edited character yet - moving the camera to it");
            reset_orbit(&g_tgt); g_cur = g_tgt;
            logf("  orbit start: yaw %f pitch %f distance %f focus height %f", g_tgt.yaw * 180 / PI, g_tgt.pitch * 180 / PI, g_tgt.dist, g_tgt.focus);
            return 1;
        }
        if (C.startFromLooksMenuView) {
            V3 rel2;
            g_tgt.focus = clampf(closest.z - base.z, C.minFocus, C.maxFocus);
            rel2 = vsub(cam, vadd(base, v3(0, 0, g_tgt.focus)));
            g_tgt.dist = clampf(vlen(rel2), C.minZoom, C.maxZoom);
            g_tgt.pitch = clampf(fasin(rel2.z / (vlen(rel2) + 1e-6f)), C.minPitch, C.maxPitch);
            g_tgt.yaw = fatan2(rel2.x, rel2.y);
        } else reset_orbit(&g_tgt);
        (void)along;
    }
    g_cur = g_tgt;
    if (C.startFromLooksMenuView == 0) {   /* glide in from where LooksMenu put the camera */
        V3 rel3 = vsub(cam, vadd(base, v3(0, 0, g_tgt.focus)));
        g_cur.dist = clampf(vlen(rel3), C.minZoom, C.maxZoom); g_cur.yaw = fatan2(rel3.x, rel3.y);
        g_cur.pitch = clampf(fasin(rel3.z / (vlen(rel3) + 1e-6f)), C.minPitch, C.maxPitch);
    }
    logf("  orbit start: yaw %f pitch %f distance %f focus height %f", g_tgt.yaw * 180 / PI, g_tgt.pitch * 180 / PI, g_tgt.dist, g_tgt.focus);
    return 1;
}

static float stick(s16 v, float dz) {
    float f = v / 32767.0f; if (f < -1) f = -1;
    if (fabsf_(f) < dz) return 0;
    return (f > 0 ? f - dz : f + dz) / (1 - dz);
}

static void read_inputs(float dt) {
    s32 dx = g_mouseDX, dy = g_mouseDY, wh = g_wheel; float sx = C.invertX ? -1.f : 1.f, sy = C.invertY ? -1.f : 1.f;
    g_mouseDX -= dx; g_mouseDY -= dy; g_wheel -= wh;
    if (C.enableMouseKeyboard) {
        g_tgt.yaw = wrap_pi(g_tgt.yaw + sx * dx * 0.006f * C.sensitivity);
        g_tgt.pitch = clampf(g_tgt.pitch + sy * dy * 0.006f * C.sensitivity * C.vertSensitivity, C.minPitch, C.maxPitch);
        if (wh) {
            float steps = wh / 120.0f, k = 1 - steps * C.mouseZoomStep;
            if (k < 0.2f) k = 0.2f;
            g_tgt.dist = clampf(g_tgt.dist * k, C.minZoom, C.maxZoom);
        }
        if (C.enableVertical) {
            int focus = game_has_focus(), upk = g_keyUpHeld, dnk = g_keyDownHeld;
            if (upk && !(GetAsyncKeyState(C.keyUp) & 0x8000)) g_keyUpHeld = upk = 0;       /* released while not seen */
            if (dnk && !(GetAsyncKeyState(C.keyDown) & 0x8000)) g_keyDownHeld = dnk = 0;
            if (!g_sawKeyboard && (g_rotateHeld || C.rotateButton == 0) && focus) {   /* no raw keyboard: read the keys directly */
                upk = C.keyUp && (GetAsyncKeyState(C.keyUp) & 0x8000); dnk = C.keyDown && (GetAsyncKeyState(C.keyDown) & 0x8000);
            }
            if (focus && C.keyUpAlways && (GetAsyncKeyState(C.keyUpAlways) & 0x8000)) upk = 1;
            if (focus && C.keyDownAlways && (GetAsyncKeyState(C.keyDownAlways) & 0x8000)) dnk = 1;
            if (upk) g_tgt.focus = clampf(g_tgt.focus + C.heightSpeed * dt, C.minFocus, C.maxFocus);
            if (dnk) g_tgt.focus = clampf(g_tgt.focus - C.heightSpeed * dt, C.minFocus, C.maxFocus);
        }
        if (C.enableReset && C.keyReset && game_has_focus() && (GetAsyncKeyState(C.keyReset) & 1)) g_resetRequest = 1;
    }
    if (C.enableController && g_padValid) {
        Pad p = g_pad; float rx = stick(p.rx, C.deadZone), ry = stick(p.ry, C.deadZone), lt = p.lt / 255.f, rt = p.rt / 255.f;
        static u16 prevButtons; static float r3Time; static int r3Used;
        int r3 = (p.buttons & 0x80) != 0;
        if (r3 && !(prevButtons & 0x80)) { r3Time = 0; r3Used = 0; }
        if (r3) r3Time += dt;
        g_tgt.yaw = wrap_pi(g_tgt.yaw + sx * rx * C.rStickSens * 1.6f * dt);
        if (r3) {   /* R3 held: right stick up/down zooms */
            if (ry != 0) { r3Used = 1; g_tgt.dist = clampf(g_tgt.dist - ry * C.zoomSpeed * 40 * dt, C.minZoom, C.maxZoom); }
            if (rx != 0) r3Used = 1;
        } else g_tgt.pitch = clampf(g_tgt.pitch - sy * ry * C.rStickSens * 1.2f * C.vertSensitivity * dt, C.minPitch, C.maxPitch);
        if (C.triggerZoom) {
            if (lt < 0.08f) lt = 0;
            if (rt < 0.08f) rt = 0;
            g_tgt.dist = clampf(g_tgt.dist + (lt - rt) * C.zoomSpeed * 40 * dt, C.minZoom, C.maxZoom);
        }
        if (C.enableVertical) {
            float h = 0;
            if (C.dpadHeight == 2 || (C.dpadHeight == 1 && r3)) { if (p.buttons & 1) h += 1; if (p.buttons & 2) h -= 1; if (r3 && (p.buttons & 3)) r3Used = 1; }
            if (C.leftStickHeight) h += stick(p.ly, C.deadZone) * C.lStickSens;
            g_tgt.focus = clampf(g_tgt.focus + h * C.heightSpeed * dt, C.minFocus, C.maxFocus);
        }
        /* R3 tapped (released quickly without being used as a modifier): reset */
        if (C.enableReset && !r3 && (prevButtons & 0x80) && !r3Used && r3Time < 0.35f) g_resetRequest = 1;
        prevButtons = p.buttons;
    }
    if (g_resetRequest) { g_resetRequest = 0; reset_orbit(&g_tgt); if (C.debugLog) logf("camera reset"); }
}

static void apply_orbit(void* root, float dt) {
    V3 base = player_pos(), focus, cam, F, U, R, axes[3]; float a = smooth_alpha(C.smoothing, dt), dyaw;
    NiTransform* loc = &PTR(root, 0x30, NiTransform); void* parent = PTR(root, 0x28, void*); int i;
    typedef void (*UpdateDown_t)(void*, void*, u32);
    u8 ud[0x40];
    /* smoothing (yaw along the short way round) */
    dyaw = wrap_pi(g_tgt.yaw - g_cur.yaw);
    g_cur.yaw = wrap_pi(g_cur.yaw + dyaw * a);
    g_cur.pitch += (g_tgt.pitch - g_cur.pitch) * a;
    g_cur.dist += (g_tgt.dist - g_cur.dist) * a;
    g_cur.focus += (g_tgt.focus - g_cur.focus) * a;
    focus = vadd(base, v3(0, 0, g_cur.focus));
    cam = orbit_cam_pos(&g_cur, base);
    F = vnorm(vsub(focus, cam));
    R = vcross(F, v3(0, 0, 1)); if (vlen(R) < 1e-4f) R = v3(1, 0, 0); R = vnorm(R);
    U = vnorm(vcross(R, F));
    axes[g_axisF] = vmul(F, g_sF); axes[g_axisU] = vmul(U, g_sU);
    axes[g_axisT] = vmul(vcross(axes[g_axisF], axes[g_axisU]), g_sT);
    if (parent) {   /* the root has a parent: convert the wanted world transform into a local one */
        NiTransform* pw = &PTR(parent, 0x70, NiTransform); V3 rel = vsub(cam, pw->t); float inv = pw->s != 0 ? 1 / pw->s : 1; int r, c;
        for (r = 0; r < 3; r++) for (c = 0; c < 3; c++) {   /* local = Rp^T * Rw */
            /* R_local = R_parent^T * R_world (axes as columns); stored transposed */
            loc->m[c][r] = vdot(mcol(pw, r), axes[c]);
        }
        loc->t = v3(vdot(mcol(pw, 0), rel) * inv, vdot(mcol(pw, 1), rel) * inv, vdot(mcol(pw, 2), rel) * inv);
    } else {
        for (i = 0; i < 3; i++) { loc->m[i][0] = axes[i].x; loc->m[i][1] = axes[i].y; loc->m[i][2] = axes[i].z; }
        loc->t = cam;
    }
    memset(ud, 0, sizeof(ud));
    (*(UpdateDown_t**)root)[0x30](root, ud, 0);   /* NiAVObject::UpdateDownwardPass, as the game does after placing the camera */
    if (C.debugLog && (g_frames % 240) == 1)
        logf("  orbit: yaw %f pitch %f distance %f focus %f -> camera %f %f %f", g_cur.yaw * 180 / PI, g_cur.pitch * 180 / PI, g_cur.dist, g_cur.focus, cam.x, cam.y, cam.z);
}

/* ---- diagnostics (0.1.1): what is under the camera root, and does anything move the camera after us? */
static V3 g_lastCam, g_lastF; static int g_haveLast, g_driftLogs;
static const char* node_name(void* o) { void* e = o ? PTR(o, 0x10, void*) : 0; return e ? (const char*)e + 0x18 : "(no name)"; }
static void dump_tree(void* node, int depth) {
    typedef void* (*AsNode_t)(void*);
    NiTransform* l; NiTransform* w; void* asNode; int i;
    if (!node || depth > 3) return;
    l = &PTR(node, 0x30, NiTransform); w = &PTR(node, 0x70, NiTransform);
    logf("  %s%s: local pos %f %f %f, world pos %f %f %f, world axis1 %f %f %f", depth == 0 ? "" : depth == 1 ? "  " : "    ", node_name(node),
         l->t.x, l->t.y, l->t.z, w->t.x, w->t.y, w->t.z, w->m[1][0], w->m[1][1], w->m[1][2]);
    asNode = (*(AsNode_t**)node)[3](node);
    if (!asNode) return;
    {
        void** data = PTR(node, 0x128, void**); u16 n = PTR(node, 0x132, u16);
        if (!data || n > 32) return;
        for (i = 0; i < n; i++) if (data[i]) dump_tree(data[i], depth + 1);
    }
}
static void check_drift(void* root) {
    NiTransform* w = &PTR(root, 0x70, NiTransform); V3 f = vmul(mcol(w, g_axisF), g_sF);
    float dp = vlen(vsub(w->t, g_lastCam)), df = vlen(vsub(f, g_lastF));
    if (C.debugLog && (dp > 0.5f || df > 0.01f) && g_driftLogs < 40) {
        g_driftLogs++;
        logf("  !! camera was changed after the orbit camera placed it (moved %f units, turned %f): now at %f %f %f", dp, df, w->t.x, w->t.y, w->t.z);
    }
}

static void hk_StateUpdate(void* state, void* next) {
    int i; StateUpdate_t orig = 0; u8* pc; void* root; int open; s64 now; float dt;
    for (i = 0; i < g_nvh; i++) if (g_vh[i].vtbl == *(void***)state) { orig = g_vh[i].orig; break; }
    if (g_orbitActive && g_haveLast) {   /* before the game's update: is the camera still where we put it last frame? */
        u8* pc0 = (u8*)player_camera(); void* r0 = pc0 ? PTR(pc0, 0x20, void*) : 0;
        if (r0 && PTR(pc0, 0x28, void*) == state) check_drift(r0);
    }
    if (orig) orig(state, next);
    if (!C.enable) return;
    pc = (u8*)player_camera();
    if (!pc || PTR(pc, 0x28, void*) != state) return;   /* only the camera's current state */
    root = PTR(pc, 0x20, void*);
    open = menu_open("LooksMenu", C.debugLog);
    QueryPerformanceCounter(&now);
    dt = g_lastTime ? (float)(now - g_lastTime) / (float)g_freq : 0.016f;
    g_lastTime = now;
    if (dt <= 0 || dt > 0.25f) dt = 0.016f;
    if (open && !g_wasOpen) {
        int id = PTR(state, 0x20, int);
        g_mouseDX = g_mouseDY = g_wheel = 0; g_resetRequest = 0; g_rotateHeld = 0; g_skipLogged = 0; g_padValid = 0; g_keyUpHeld = g_keyDownHeld = 0;
        g_fzOnFwd = 0; g_watchFrames = 0; g_lastSlotIdx = 99; memset(g_lastSlotActor, 0, sizeof(g_lastSlotActor));
        g_active = root ? begin_orbit(root, id) : 0;
        g_fzPending = 0; if (g_target) freeze_request(g_target, 0);
        g_fpN = 0; g_fpScaled = 0; if (g_active && id == 0 && g_target && g_target != player_ref()) fp_hide();
        g_frames = 0; g_haveLast = 0; g_driftLogs = 0;
        if (root && C.debugLog) { logf("  camera node tree (before the orbit camera):"); dump_tree(root, 0); if (PTR(root, 0x28, void*)) logf("  root parent: %s", node_name(PTR(root, 0x28, void*))); }
    }
    if (!open && g_wasOpen) { fp_restore(); tp_restore("LooksMenu closed"); freeze_end(); g_fzPending = 0; logf("LooksMenu closed - game camera restored (%u orbit frames)", g_frames); g_active = 0; g_target = 0; g_targetScale = 1; }
    g_wasOpen = open;
    if (open) { target_watch(); freeze_pending_frame(); freeze_frame(); fp_frame(); tp_frame(); }
    g_orbitActive = open && g_active;
    if (!g_orbitActive || !root) return;
    if (C.rotateButton != 0 && C.enableMouseKeyboard) {   /* button released while another window had focus etc. */
        int vk = C.rotateButton == 1 ? 0x01 : C.rotateButton == 2 ? 0x02 : 0x04;
        if (!(GetAsyncKeyState(vk) & 0x8000)) g_rotateHeld = 0;
    }
    g_frames++;
    read_inputs(dt);
    apply_orbit(root, dt);
    { NiTransform* w = &PTR(root, 0x70, NiTransform); g_lastCam = w->t; g_lastF = vmul(mcol(w, g_axisF), g_sF); g_haveLast = 1; }
    if (C.debugLog && (g_frames == 2 || g_frames % 600 == 0)) { logf("  camera node tree (orbit camera, frame %u):", g_frames); dump_tree(root, 0); }
}

/* ------------------------------------------------------------------ F4SE plugin entry points */
typedef u32 PluginHandle;
typedef struct { u32 f4seVersion, runtimeVersion, editorVersion, isEditor; void* (*QueryInterface)(u32); PluginHandle (*GetPluginHandle)(void); u32 (*GetReleaseIndex)(void); } F4SEInterface;
typedef struct { u32 f4seVersion, runtimeVersion, editorVersion, isEditor; void* (*QueryInterface)(u32); PluginHandle (*GetPluginHandle)(void); u32 (*GetReleaseIndex)(void); const void* (*GetPluginInfo)(const char*); const char* (*GetSaveFolderName)(void); } F4SEInterface_NG;
typedef struct { u32 infoVersion; const char* name; u32 version; } PluginInfo;
typedef struct { const char* sender; u32 type; u32 dataLen; void* data; } F4SEMessage;
typedef struct { u32 interfaceVersion; BOOL (*RegisterListener)(PluginHandle, const char*, void (*)(F4SEMessage*)); } F4SEMessagingInterface;
static HANDLE g_self;
static PluginHandle g_handle;

static void on_message(F4SEMessage* m) {
    if (!m) return;
    if (m->type == 7 /* InputLoaded */ || m->type == 9 /* GameLoaded */ || m->type == 10 /* GameDataReady */ || m->type == 3 /* PostLoadGame */) {
        if (!g_installed) logf("F4SE message %u: trying to hook the camera", m->type);
        try_install_camera();
    }
}

/* F4SE 0.6.x (1.10.163) */
__declspec(dllexport) BOOL F4SEPlugin_Query(const F4SEInterface* f4se, PluginInfo* info) {
    info->infoVersion = 1; info->name = "LooksMenuOrbitCamera"; info->version = LMOC_VERSION_NUM;
    if (f4se->isEditor) return 0;
    return find_version(f4se->runtimeVersion) != 0;
}

__declspec(dllexport) BOOL F4SEPlugin_Load(const F4SEInterface* f4se) {
    char dir[300]; u32 n, i; HANDLE h; F4SEMessagingInterface* msg; const VersionEntry* ver;
    g_base = (u64)GetModuleHandleA(0);
    { u8* b = (u8*)g_base; u32 pe = *(u32*)(b + 0x3C); g_modEnd = g_base + *(u32*)(b + pe + 24 + 56); }   /* SizeOfImage */
    if (SHGetFolderPathA(0, 5, 0, 0, dir) == 0) {
        /* F4SE 0.7.1+ tells which My Games folder this game build uses */
        const char* folder = "Fallout4"; u32 k;
        if (f4se->f4seVersion >= 0x00070010 && ((const F4SEInterface_NG*)f4se)->GetSaveFolderName) {
            const char* f = ((const F4SEInterface_NG*)f4se)->GetSaveFolderName();
            if (f && f[0] && slen(f) < 60) folder = f;
        }
        n = slen(dir);
        memcpy(g_logPath, dir, n); memcpy(g_logPath + n, "\\My Games\\", 10); n += 10;
        k = slen(folder); memcpy(g_logPath + n, folder, k); n += k;
        memcpy(g_logPath + n, "\\F4SE\\LooksMenuOrbitCamera.log", 31);
        h = CreateFileA(g_logPath, 0x40000000, 3, 0, 2, 0x80, 0); if (h != (HANDLE)(long long)-1) CloseHandle(h);
    }
    n = GetModuleFileNameA(g_self, g_iniPath, sizeof(g_iniPath) - 40);
    for (i = n; i > 0 && g_iniPath[i - 1] != '\\' && g_iniPath[i - 1] != '/'; i--) {}
    memcpy(g_iniPath + i, "LooksMenuOrbitCamera.ini", 25);
    load_config();
    logf("LooksMenu Orbit Camera %s - game runtime %x, F4SE %x", LMOC_VERSION_STR, f4se->runtimeVersion, f4se->f4seVersion);
    logf("settings: %s", g_iniPath);
    logf("  orbit %d, controller %d, mouse/keyboard %d, rotate mouse button %d, wheel only while rotating %d, block used input %d",
         C.enable, C.enableController, C.enableMouseKeyboard, C.rotateButton, C.wheelNeedsRotateButton, C.blockGameInput);
    logf("  distance %f (%f-%f), focus height %f, smoothing %f, sensitivity %f/%f, stick %f deadzone %f zoom speed %f",
         C.distance, C.minZoom, C.maxZoom, C.focusHeight, C.smoothing, C.sensitivity, C.vertSensitivity, C.rStickSens, C.deadZone, C.zoomSpeed);
    ver = find_version(f4se->runtimeVersion);
    if (!ver) { logf("Unknown game version %x - the plugin does nothing (known: 1.10.163, 1.10.980, 1.10.984, 1.11.137, 1.11.159, 1.11.169, 1.11.191, 1.11.221, 1.11.240).", f4se->runtimeVersion); return 0; }
    RVA_PLAYER_CAMERA = ver->camera; RVA_UI = ver->ui; RVA_PLAYER = ver->player; RVA_CC = ver->cc; RVA_CC_INDEX = ver->ccIndex; RVA_CONSOLE = ver->consoleHandle; RVA_LOOKUP = ver->lookupHandle;
    logf("game version %s%s", ver->name, (f4se->runtimeVersion & 0xF) ? " (non-Steam build: addresses are checked before use)" : "");
    QueryPerformanceFrequency(&g_freq); if (g_freq <= 0) g_freq = 1;
    o_GetRawInputData = (GetRawInputData_t)patch_import("USER32.dll", "GetRawInputData", 0, (void*)hk_GetRawInputData);
    logf("mouse input hook: %s", o_GetRawInputData ? "ok" : "FAILED (mouse control disabled)");
    if (!o_GetRawInputData) C.enableMouseKeyboard = 0;
    o_XInputGetState = (XInputGetState_t)patch_import("XINPUT1_3.dll", 0, 2, (void*)hk_XInputGetState);
    logf("controller input hook: %s", o_XInputGetState ? "ok" : "FAILED (controller control disabled)");
    if (!o_XInputGetState) C.enableController = 0;
    g_handle = f4se->GetPluginHandle();
    msg = (F4SEMessagingInterface*)f4se->QueryInterface(1);
    if (msg) msg->RegisterListener(g_handle, "F4SE", on_message);
    try_install_camera();
    return 1;
}

BOOL WINAPI DllMain(HANDLE h, u32 reason, void* r) { if (reason == 1) g_self = h; (void)r; return 1; }
