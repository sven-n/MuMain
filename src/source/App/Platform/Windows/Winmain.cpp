///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Core/Input/KeyState.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#ifdef _WIN32
#include <dpapi.h>
#endif
#include <clocale>
#include "Data/GameConfig/GameConfig.h"
#include "UI/Legacy/UIWindows.h"
#include "UI/Legacy/UIManager.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Scenes/SceneCore.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "Network/IncomingPacketQueue.h"
#include "Core/Time/FrameTimerScheduler.h"
#include <SDL3/SDL.h>
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Audio/DSPlaySound.h"

#include "App/Platform/Windows/resource.h"
#include "Core/Platform/Imm.h"
#include "Core/Platform/BundledFonts.h"
#include "Engine/Pathing/ZzzPath.h"
#include "App/Platform/Windows/Local.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"

#include "UI/Legacy/UIMapName.h"		// rozy
#include "Core/Utilities/CpuUsage.h"

#include "MUHelper/MuHelper.h"
#include "Camera/CameraManager.h"

#include "UI/Windows/CBTMessageBox.h"

#include "GameLogic/Events/CSChaosCastle.h"
#ifdef _WIN32
#include <io.h>
#endif
#include "Core/Input/Input.h"
#include "Core/Time/Timer.h"
#include "UI/Legacy/UIMng.h"


#include "World/MapInfra/w_MapHeaders.h"

#include "GameLogic/Pets/w_PetProcess.h"



#include "UI/NewUI/NewUISystem.h"
#include "Camera/CameraConfig.h"
#include "Camera/CameraProjection.h"
#include "I18N/All.h"

#ifdef _EDITOR
#include "../MuEditor/Core/MuEditorCore.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "../MuEditor/Config/MuEditorConfig.h"
#endif

CUIMercenaryInputBox* g_pMercenaryInputBox = nullptr;
CUITextInputBox* g_pSingleTextInputBox = nullptr;
CUITextInputBox* g_pSinglePasswdInputBox = nullptr;
int g_iChatInputType = 1;
extern BOOL g_bIMEBlock;

CMultiLanguage* pMultiLanguage = nullptr;

extern DWORD g_dwTopWindow;

CUIManager* g_pUIManager = nullptr;
CUIMapName* g_pUIMapName = nullptr;		// rozy

float Time_Effect = 0;
bool ashies = false;
int weather = rand() % 3;

HWND      g_hWnd = nullptr;
HINSTANCE g_hInst = nullptr;
HDC       g_hDC = nullptr;
HGLRC     g_hRC = nullptr;

// SDL owns the window and GL context (issue #442). The native HWND is bridged
// into g_hWnd so the remaining Win32 code (IME, DirectSound, cursor, the legacy
// EDIT-control text boxes) keeps working until those are migrated.
static SDL_Window*   g_sdlWindow = nullptr;
static SDL_GLContext g_sdlGLContext = nullptr;
HFONT     g_hFont = nullptr;
HFONT     g_hFontBold = nullptr;
HFONT     g_hFontBig = nullptr;
HFONT     g_hFixFont = nullptr;

CTimer* g_pTimer = new CTimer();    // performance counter.
bool      Destroy = false;
bool      ActiveIME = false;

BYTE* RendomMemoryDump;
ITEM_ATTRIBUTE* ItemAttRibuteMemoryDump;
CHARACTER* CharacterMemoryDump;

int       RandomTable[100];

CErrorReport g_ErrorReport;

BOOL g_bMinimizedEnabled = FALSE;
int g_iScreenSaverOldValue = 60 * 15;



BOOL g_bUseWindowMode = TRUE;
BOOL g_bUseFullscreenMode = FALSE;

#include "Audio/AudioPlayer.h"

extern int  LogIn;
extern wchar_t LogInID[];
extern bool First;
extern int FirstTime;
extern BOOL g_bGameServerConnected;

void CheckHack()
{
    if (!g_bGameServerConnected)
    {
        return;
    }

    g_ConsoleDebug->Write(MCD_SEND, L"SendCheck");

    auto attackSpeed = CharacterAttribute->AttackSpeed;
    auto magicSpeed = CharacterAttribute->MagicSpeed;
    if (CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED
        || CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED2)
    {
        attackSpeed -= 20;
        magicSpeed -= 20;
    }

    const int dwTick = GetTickCount();
    SocketClient->ToGameServer()->SendPing(dwTick, attackSpeed);

    if (!First)
    {
        First = true;
        FirstTime = dwTick;
    }
}

GLvoid KillGLWindow(GLvoid)
{
    // Release the bridged GDI DC obtained from the SDL window.
    if (g_hDC)
    {
        ReleaseDC(g_hWnd, g_hDC);
        g_hDC = nullptr;
    }

    // SDL owns the GL context and the window (it also restores the display mode
    // when a fullscreen window is destroyed).
    if (g_sdlGLContext)
    {
        SDL_GL_DestroyContext(g_sdlGLContext);
        g_sdlGLContext = nullptr;
        g_hRC = nullptr;
    }

    if (g_sdlWindow)
    {
        SDL_DestroyWindow(g_sdlWindow);
        g_sdlWindow = nullptr;
        g_hWnd = nullptr;
    }
}

#ifndef _WIN32
// Debug-only framebuffer capture: when MU_CAPTURE_FRAME=<N> is set, dump the
// Nth presented frame to MU_CAPTURE_PATH (default /tmp/mu-frame.ppm) as a PPM.
// Used to verify rendering on headless/WSLg setups where X screenshot tools
// cannot read the window (issue #462). No effect unless the env var is set.
static void MaybeCaptureFrame()
{
    const char* want = std::getenv("MU_CAPTURE_FRAME");
    if (!want) return;
    static long s_frame = 0;
    const long target = std::strtol(want, nullptr, 10);
    if (++s_frame != target) return;

    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(g_sdlWindow, &w, &h);
    if (w <= 0 || h <= 0) return;

    std::vector<unsigned char> pixels(static_cast<size_t>(w) * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    const char* path = std::getenv("MU_CAPTURE_PATH");
    if (!path) path = "/tmp/mu-frame.ppm";
    if (FILE* fp = std::fopen(path, "wb"))
    {
        std::fprintf(fp, "P6\n%d %d\n255\n", w, h);
        // glReadPixels origin is bottom-left; write rows top-down for a PPM.
        for (int y = h - 1; y >= 0; --y)
            std::fwrite(pixels.data() + static_cast<size_t>(y) * w * 3, 1, static_cast<size_t>(w) * 3, fp);
        std::fclose(fp);
        std::fprintf(stderr, "[capture] wrote frame %ld (%dx%d) to %s\n", target, w, h, path);
    }
}
#endif

// Present the current GL frame. SDL owns the window/context, so swapping goes
// through SDL_GL_SwapWindow instead of the Win32 ::SwapBuffers (issue #442).
void PlatformSwapBuffers()
{
    if (g_sdlWindow)
    {
#ifndef _WIN32
        MaybeCaptureFrame();
#endif
        SDL_GL_SwapWindow(g_sdlWindow);
    }
}

// Monitor refresh rate (Hz) for the display the window is on, via SDL instead
// of the Win32 GetDeviceCaps(VREFRESH) (issue #442). Falls back to 60.
int GetFPSLimit()
{
    constexpr int DEFAULT_REFRESH_HZ = 60;
    if (g_sdlWindow)
    {
        // Before the window is mapped to a display, SDL_GetDisplayForWindow
        // returns 0; fall back to the primary display so a high-refresh monitor
        // isn't capped at the default 60 Hz.
        SDL_DisplayID displayID = SDL_GetDisplayForWindow(g_sdlWindow);
        if (displayID == 0)
            displayID = SDL_GetPrimaryDisplay();
        if (displayID != 0)
        {
            const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(displayID);
            if (mode && mode->refresh_rate > 0.0f)
                return static_cast<int>(mode->refresh_rate + 0.5f);
        }
    }
    return DEFAULT_REFRESH_HZ;
}

BOOL GetFileNameOfFilePath(wchar_t* lpszFile, wchar_t* lpszPath)
{
    auto iFind = (int)'\\';
    wchar_t* lpFound = lpszPath;
    wchar_t* lpOld = lpFound;
    while (lpFound)
    {
        lpOld = lpFound;
        lpFound = wcschr(lpFound + 1, iFind);
    }

    if (wcschr(lpszPath, iFind))
    {
        wcscpy(lpszFile, lpOld + 1);
    }
    else
    {
        wcscpy(lpszFile, lpOld);
    }

    BOOL bCheck = TRUE;
    for (wchar_t* lpTemp = lpszFile; bCheck; ++lpTemp)
    {
        switch (*lpTemp)
        {
        case '\"':
        case '\\':
        case '/':
        case ' ':
            *lpTemp = '\0';
        case '\0':
            bCheck = FALSE;
            break;
        }
    }

    return (TRUE);
}

WORD DecryptCheckSumKey(WORD wSource)
{
    WORD wAcc = wSource ^ 0xB479;
    return ((wAcc >> 10) << 4) | (wAcc & 0xF);
}

DWORD GenerateCheckSum(BYTE* pbyBuffer, DWORD dwSize, WORD wKey)
{
    auto dwKey = (DWORD)wKey;
    DWORD dwResult = dwKey << 9;
    for (DWORD dwChecked = 0; dwChecked <= dwSize - 4; dwChecked += 4)
    {
        DWORD dwTemp;
        memcpy(&dwTemp, pbyBuffer + dwChecked, sizeof(DWORD));

        switch ((dwChecked / 4 + wKey) % 3)
        {
        case 0:
            dwResult ^= dwTemp;
            break;
        case 1:
            dwResult += dwTemp;
            break;
        case 2:
            dwResult <<= (dwTemp % 11);
            dwResult ^= dwTemp;
            break;
        }

        if (0 == (dwChecked % 4))
        {
            dwResult ^= ((dwKey + dwResult) >> ((dwChecked / 4) % 16 + 3));
        }
    }

    return (dwResult);
}

DWORD GetCheckSum(WORD wKey)
{
    wKey = DecryptCheckSumKey(wKey);

    wchar_t lpszFile[MAX_PATH];

    wcscpy(lpszFile, L"data\\local\\Gameguard.csr");

    HANDLE hFile = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return (0);
    }

    DWORD dwSize = GetFileSize(hFile, nullptr);
    auto* pbyBuffer = new BYTE[dwSize];
    DWORD dwNumber;
    ReadFile(hFile, pbyBuffer, dwSize, &dwNumber, nullptr);
    CloseHandle(hFile);

    DWORD dwCheckSum = GenerateCheckSum(pbyBuffer, dwSize, wKey);
    delete[] pbyBuffer;

    return (dwCheckSum);
}

BOOL GetFileVersion(wchar_t* lpszFileName, WORD* pwVersion)
{
#ifndef _WIN32
    // File version-info is a Win32 crash-report detail; report "unknown".
    (void)lpszFileName; (void)pwVersion;
    return FALSE;
#else
    DWORD dwHandle;
    DWORD dwLen = GetFileVersionInfoSize(lpszFileName, &dwHandle);
    if (dwLen <= 0)
    {
        return (FALSE);
    }

    auto* pbyData = new BYTE[dwLen];
    if (!GetFileVersionInfo(lpszFileName, dwHandle, dwLen, pbyData))
    {
        delete[] pbyData;
        return (FALSE);
    }

    VS_FIXEDFILEINFO* pffi;
    UINT uLen;
    if (!VerQueryValue(pbyData, L"\\", (LPVOID*)&pffi, &uLen))
    {
        delete[] pbyData;
        return (FALSE);
    }

    pwVersion[0] = HIWORD(pffi->dwFileVersionMS);
    pwVersion[1] = LOWORD(pffi->dwFileVersionMS);
    pwVersion[2] = HIWORD(pffi->dwFileVersionLS);
    pwVersion[3] = LOWORD(pffi->dwFileVersionLS);

    delete[] pbyData;
    return (TRUE);
#endif
}

extern PATH* path;

void DestroyWindow()
{
    // Save game configuration to config.ini
    GameConfig::GetInstance().SetSoundVolume(g_pOption->GetVolumeLevel());
    GameConfig::GetInstance().Save();

#ifdef _EDITOR
    // Save editor configuration
    g_MuEditorConfig.Save();
#endif

    CUIMng::Instance().Release();

    //. release font handle
    if (g_hFont)
        DeleteObject((HGDIOBJ)g_hFont);

    if (g_hFontBold)
        DeleteObject((HGDIOBJ)g_hFontBold);

    if (g_hFontBig)
        DeleteObject((HGDIOBJ)g_hFontBig);

    if (g_hFixFont)
        ::DeleteObject((HGDIOBJ)g_hFixFont);

    ReleaseCharacters();

    SAFE_DELETE(GateAttribute);

    SAFE_DELETE(SkillAttribute);

    SAFE_DELETE(CharacterMachine);

    DeleteWaterTerrain();

    {
        gMapManager.DeleteObjects();

        // Object.
        for (int i = MODEL_LOGO; i < MAX_MODELS; i++)
        {
            Models[i].Release();
        }

        // Bitmap
        Bitmaps.UnloadAllImages();
    }

    SAFE_DELETE_ARRAY(CharacterMemoryDump);
    SAFE_DELETE_ARRAY(ItemAttRibuteMemoryDump);
    SAFE_DELETE_ARRAY(RendomMemoryDump);
    SAFE_DELETE_ARRAY(ModelsDump);

#ifdef DYNAMIC_FRUSTRUM
    DeleteAllFrustrum();
#endif //DYNAMIC_FRUSTRUM

    SAFE_DELETE(g_pMercenaryInputBox);
    SAFE_DELETE(g_pSingleTextInputBox);
    SAFE_DELETE(g_pSinglePasswdInputBox);

    SAFE_DELETE(g_pUIMapName);	// rozy
    SAFE_DELETE(g_pTimer);
    SAFE_DELETE(g_pUIManager);

    SAFE_DELETE(pMultiLanguage);
    PtrReset(g_BuffSystem);
    PtrReset(g_MapProcess);
    PtrReset(g_petProcess);

    g_ErrorReport.Write(L"Destroy");

    HWND shWnd = FindWindow(nullptr, L"MuPlayer");
    if (shWnd)
        SendMessage(shWnd, WM_DESTROY, 0, 0);
}
void DestroySound()
{
    for (int i = 0; i < MAX_BUFFER; i++)
        ReleaseBuffer(i);

    FreeDirectSound();
    AudioPlayer::Shutdown();
}

int g_iInactiveTime = 0;
int g_iNoMouseTime = 0;
int g_iInactiveWarning = 0;
bool g_bWndActive = false;
bool HangulDelete = false;
int Hangul = 0;
bool g_bEnterPressed = false;

static double g_TargetFpsBeforeInactive = -1.0;
static bool g_HasInactiveFpsOverride = false;

int g_iMousePopPosition_x = 0;
int g_iMousePopPosition_y = 0;

extern int TimeRemain;
extern bool EnableFastInput;

// The legacy Win32 message handler. SDL owns the event loop on Linux and only
// bridges to this via the Windows-only message hook, so guard it off there.
#ifdef _WIN32
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // F10 zoom-lock toggle. Handled before the ImGui forwarder so editor-open
    // sessions still get the toggle (ImGui captures keyboard messages while a
    // window has focus). Bit 30 of lParam = previous key state — skip
    // auto-repeat ticks so a held key only toggles once.
    constexpr LPARAM PREVIOUS_KEY_STATE_MASK = 1 << 30;
    if (msg == WM_SYSKEYDOWN && wParam == VK_F10 && (lParam & PREVIOUS_KEY_STATE_MASK) == 0)
    {
        CameraManager::Instance().ToggleZoomLock();
        return 0;
    }

    // ImGui (editor) now consumes input from the SDL event loop via
    // ImGui_ImplSDL3_ProcessEvent, not from Win32 messages (issue #442).

    switch (msg)
    {
    case WM_SYSKEYDOWN:
    {
        // F10 is handled above (intercepted before ImGui). Other system keys
        // are silenced here — returning 0 prevents the OS menu activation.
        return 0;
    }
    break;
    // WM_ACTIVATE is handled via SDL window focus events (issue #442).
    case WM_NPROTECT_EXIT_TWO:
        SocketClient->ToGameServer()->SendLogOutByCheatDetection(0);
        // Inform the user, then close. A frame-ticked timer cannot fire while
        // this modal dialog blocks the main loop, so close right after the
        // dialog is dismissed instead of via a timer.
        MessageBox(nullptr, I18N::Game::Error9AHackingToolHasBeen, L"Error", MB_OK);
        PostMessage(g_hWnd, WM_CLOSE, 0, 0);
        break;
    case WM_ERASEBKGND:
        return TRUE;
        break;
    // WM_SIZE is handled via SDL window resize events (issue #442).
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
    }
    return 0;
    break;
    case WM_DESTROY:
    {
        // Just request shutdown; the main loop exits on Destroy and the teardown
        // (sound, GL, window) runs after it. SDL owns the window now, so the GL
        // context and window must not be torn down from inside a message.
        Destroy = true;
        if (SocketClient != nullptr)
        {
            SocketClient->Close();
            g_bGameServerConnected = false;
        }
    }
    break;
    case WM_SETCURSOR:
#ifdef _EDITOR
        // When hovering UI (including Open Editor button), let Windows show cursor
        // Otherwise hide Windows cursor for game cursor
        if (g_MuEditorCore.IsHoveringUI())
        {
            // Let Windows cursor show - don't hide it
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        else
#endif
        {
            ShowCursor(false);
        }
        break;
        //-----------------------------
    default:
        break;
    }

    // Mouse input (move/buttons/wheel) is handled via SDL events (issue #442).
    switch (msg)
    {
    case WM_IME_NOTIFY:
    {
        if (g_iChatInputType == 1)
        {
            switch (wParam)
            {
            case IMN_SETCONVERSIONMODE:
                if (GetFocus() == g_hWnd)
                {
                    CheckTextInputBoxIME(IME_CONVERSIONMODE);
                }
                break;
            case IMN_SETSENTENCEMODE:
                if (GetFocus() == g_hWnd)
                {
                    CheckTextInputBoxIME(IME_SENTENCEMODE);
                }
                break;
            default:
                break;
            }
        }
    }
    break;
    case WM_CHAR:
    {
        switch (wParam)
        {
        case VK_RETURN:
        {
            SetEnterPressed(true);
        }
        break;
        }
    }
    break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif // _WIN32 (WndProc)

wchar_t m_Username[11];
wchar_t m_Password[21];
wchar_t m_Version[11];
wchar_t m_ExeVersion[11];
int  m_SoundOnOff;
int  m_MusicOnOff;
int  m_Resolution;
int m_RememberMe;

wchar_t g_aszMLSelection[MAX_LANGUAGE_NAME_LENGTH] = { '\0' };


BOOL Util_CheckOption(std::wstring lpszCommandLine, wchar_t cOption, std::wstring& lpszString)
{
    if (lpszCommandLine.empty()) {
        return FALSE;
    }

    // Create both lowercase and uppercase variants of the option character
    std::wstring cOptionLower = L"/";
    cOptionLower += static_cast<wchar_t>(towlower(static_cast<wint_t>(cOption)));
    auto foundIndex = lpszCommandLine.find(cOptionLower);
    if (foundIndex == std::wstring::npos)
    {
        std::wstring cOptionUpper = L"/";
        cOptionUpper += static_cast<wchar_t>(towupper(static_cast<wint_t>(cOption)));
        foundIndex = lpszCommandLine.find(cOptionUpper);
    }

    if (foundIndex == std::wstring::npos)
    {
        return FALSE;
    }

    auto endIndex = lpszCommandLine.find(L' ', foundIndex);
    if (endIndex == std::wstring::npos)
    {
        endIndex = lpszCommandLine.length();
    }

    lpszString = lpszCommandLine.substr(foundIndex + 2, endIndex - foundIndex - 2);
    return TRUE;
}

#ifdef _WIN32
#include <tlhelp32.h>
#endif

wchar_t g_lpszCmdURL[50];
BOOL GetConnectServerInfo(wchar_t* szCmdLine, wchar_t* lpszURL, WORD* pwPort)
{
    std::wstring lpszTemp = { 0, };

    if (!Util_CheckOption(szCmdLine, L'u', lpszTemp))
    {
        return FALSE;
    }

    wcscpy(lpszURL, lpszTemp.c_str());
    if (!Util_CheckOption(szCmdLine, L'p', lpszTemp))
    {
        return FALSE;
    }

    *pwPort = static_cast<WORD>(std::stoi(lpszTemp));

    return TRUE;
}

extern int TimeRemain;
BOOL g_bInactiveTimeChecked = FALSE;
void MoveObject(OBJECT* o);

bool ExceptionCallback(_EXCEPTION_POINTERS* pExceptionInfo)
{
    if (g_bUseWindowMode == FALSE && g_bUseFullscreenMode == TRUE)
    {
        ChangeDisplaySettings(nullptr, 0);
    }
    return true;
}

double CPU_AVG = 0.0;
void RecordCpuUsage() 
{
    constexpr int max_recordings = 60;
    double CPU_Recordings[max_recordings] = { 0.0 };
    double currentAvg = 0.0;
    double sum = 0.0;
    int count = 0;
    int numFilled = 0;
    auto lastUpdateTime = std::chrono::steady_clock::now();

    while (!Destroy) 
    {
        double currentUsage = CpuUsage::Instance()->GetUsage();

        currentUsage = std::max<double>(0.0, std::min<double>(100.0, currentUsage));

        // Subtract the old value to maintain the sum
        sum -= CPU_Recordings[count];

        sum += currentUsage;

        CPU_Recordings[count] = currentUsage;

        // Update the count (wrap around when full - FIFO behavior)
        count = (count + 1) % max_recordings;

        if (numFilled < max_recordings)
        {
            numFilled++;
        }

        // Calculate the current average
        currentAvg = sum / numFilled;

        // Update the CPU_AVG every 250 ms
        auto currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count() >= 250)
        {
            CPU_AVG = currentAvg;
            lastUpdateTime = currentTime;
        }

        // Sleep to match a 60Hz frame rate as the basis
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// unlimited as default (same behavior as original)
int g_MaxMessagePerCycle = -1; 

void SetMaxMessagePerCycle(int messages)
{
    constexpr int custom_min = 3;
    g_MaxMessagePerCycle = (messages > 0) ? std::max<int>(messages, custom_min) : messages;
}

#ifdef _WIN32
// Transitional bridge (issue #442): SDL owns the window, but the existing
// WndProc still handles input, IME, the legacy Win32 EDIT-control text boxes,
// the cursor, and shutdown. SDL invokes this for every Win32 message it pumps;
// forward to WndProc and return true so SDL continues its own processing (which
// also dispatches the child EDIT controls). Removed once input/IME are
// SDL-native and the legacy text boxes are replaced (issue #447).
static bool SDLCALL Win32MessageHook(void* /*userdata*/, MSG* msg)
{
    // Let SDL own window close. Forwarding WM_CLOSE to WndProc would reach
    // DefWindowProc, which destroys the window synchronously and out from under
    // SDL. SDL turns the close into SDL_EVENT_QUIT, which the main loop handles.
    if (msg->message == WM_CLOSE)
        return true;

    WndProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
    return true;
}
#endif

// SDL event translation (issue #442). Mouse and window events are handled from
// the SDL event loop instead of WndProc, feeding the same global input state.
namespace
{
    void HandleMouseMotion(float winX, float winY)
    {
        MouseX = static_cast<int>(winX / g_fScreenRate_x);
        MouseY = static_cast<int>(winY / g_fScreenRate_y);
        if (MouseX < 0) MouseX = 0;
        if (MouseX > REFERENCE_WIDTH) MouseX = REFERENCE_WIDTH;
        if (MouseY < 0) MouseY = 0;
        if (MouseY > REFERENCE_HEIGHT) MouseY = REFERENCE_HEIGHT;
    }

    void HandleMouseButton(const SDL_Event& e)
    {
        const bool down = (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
        g_iNoMouseTime = 0;
        switch (e.button.button)
        {
        case SDL_BUTTON_LEFT:
            if (down)
            {
                MouseLButtonPop = false;
                if (!MouseLButton) MouseLButtonPush = true;
                MouseLButton = true;
                if (e.button.clicks >= 2) MouseLButtonDBClick = true;
                SetCapture(g_hWnd);
            }
            else
            {
                MouseLButtonPush = false;
                if (MouseLButton) MouseLButtonPop = true;
                MouseLButton = false;
                g_iMousePopPosition_x = MouseX;
                g_iMousePopPosition_y = MouseY;
                ReleaseCapture();
            }
            break;
        case SDL_BUTTON_RIGHT:
            if (down)
            {
                MouseRButtonPop = false;
                if (!MouseRButton) MouseRButtonPush = true;
                MouseRButton = true;
                SetCapture(g_hWnd);
            }
            else
            {
                MouseRButtonPush = false;
                if (MouseRButton) MouseRButtonPop = true;
                MouseRButton = false;
                ReleaseCapture();
            }
            break;
        case SDL_BUTTON_MIDDLE:
            if (down)
            {
                MouseMButtonPop = false;
                if (!MouseMButton) MouseMButtonPush = true;
                MouseMButton = true;
                SetCapture(g_hWnd);
            }
            else
            {
                MouseMButtonPush = false;
                if (MouseMButton) MouseMButtonPop = true;
                MouseMButton = false;
                ReleaseCapture();
            }
            break;
        }
    }

    void HandleWindowResize(int width, int height)
    {
        if (width <= 0 || height <= 0) return;
        WindowWidth = width;
        WindowHeight = height;
        g_fScreenRate_x = static_cast<float>(WindowWidth) / static_cast<float>(REFERENCE_WIDTH);
        g_fScreenRate_y = static_cast<float>(WindowHeight) / static_cast<float>(REFERENCE_HEIGHT);
        OpenglWindowWidth = WindowWidth;
        OpenglWindowHeight = WindowHeight;
        ReinitializeFonts();
        UpdateResolutionDependentSystems();
        UpdateCursorClip();
    }

    void HandleFocusChange(bool active)
    {
        if (!active)
        {
#ifdef ACTIVE_FOCUS_OUT
            if (g_bUseWindowMode == FALSE)
#endif
                g_bWndActive = false;
            // Release the cursor when losing focus so input can route elsewhere.
            ClipCursor(nullptr);

            if (g_bUseWindowMode == FALSE && !g_HasInactiveFpsOverride)
            {
                g_TargetFpsBeforeInactive = GetTargetFps();
                SetTargetFps(REFERENCE_FPS);
                g_HasInactiveFpsOverride = true;
            }
            if (g_bUseWindowMode == TRUE)
            {
                MouseLButton = false;
                MouseLButtonPop = false;
                MouseRButton = false;
                MouseRButtonPop = false;
                MouseRButtonPush = false;
                MouseLButtonDBClick = false;
                MouseMButton = false;
                MouseMButtonPop = false;
                MouseMButtonPush = false;
                MouseWheel = 0;
            }
        }
        else
        {
            g_bWndActive = true;
            if (g_HasInactiveFpsOverride)
            {
                SetTargetFps(g_TargetFpsBeforeInactive);
                g_HasInactiveFpsOverride = false;
            }
            UpdateCursorClip();
        }
    }

    // --- Portable text field input routing (issue #447) -------------------
    // Map the SDL keys a single-line text field reacts to onto the Win32 VK
    // codes the field already understands. Returns 0 for keys it ignores.
    int MapScancodeToEditVk(SDL_Scancode sc)
    {
        switch (sc)
        {
        case SDL_SCANCODE_LEFT:      return VK_LEFT;
        case SDL_SCANCODE_RIGHT:     return VK_RIGHT;
        case SDL_SCANCODE_HOME:      return VK_HOME;
        case SDL_SCANCODE_END:       return VK_END;
        case SDL_SCANCODE_BACKSPACE: return VK_BACK;
        case SDL_SCANCODE_DELETE:    return VK_DELETE;
        case SDL_SCANCODE_RETURN:
        case SDL_SCANCODE_KP_ENTER:  return VK_RETURN;
        case SDL_SCANCODE_TAB:       return VK_TAB;
        default:                     return 0;
        }
    }

    // UTF-8 <-> UTF-16 conversions sized to the input, so text of any length
    // (typed, copied or pasted) round-trips without truncation (issue #447).
    std::wstring Utf8ToWide(const char* utf8)
    {
        if (utf8 == nullptr) return std::wstring();
        const int needed = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
        if (needed <= 1) return std::wstring();  // <=1 means empty or error
        std::wstring wide(needed - 1, L'\0');  // needed includes the null terminator
        MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wide.data(), needed);
        return wide;
    }

    std::string WideToUtf8(const std::wstring& wide)
    {
        if (wide.empty()) return std::string();
        const int needed = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (needed <= 1) return std::string();  // <=1 means empty or error
        std::string utf8(needed - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, utf8.data(), needed, nullptr, nullptr);
        return utf8;
    }

    void FeedPortableTextInput(const char* utf8)
    {
        auto* box = CUITextInputBox::GetFocusedPortable();
        if (box == nullptr || utf8 == nullptr) return;

        const std::wstring wide = Utf8ToWide(utf8);
        if (!wide.empty())
            box->OnTextInput(wide.c_str());
    }

    // Handle a key for the focused portable field. Returns true if consumed.
    bool FeedPortableKey(const SDL_KeyboardEvent& key)
    {
        auto* box = CUITextInputBox::GetFocusedPortable();
        if (box == nullptr) return false;

        const bool ctrl = (key.mod & SDL_KMOD_CTRL) != 0;
        const bool shift = (key.mod & SDL_KMOD_SHIFT) != 0;

        // Clipboard lives in SDL on this side of the boundary, keeping the text
        // field itself free of SDL; the field only exposes selection helpers.
        if (ctrl)
        {
            switch (key.scancode)
            {
            case SDL_SCANCODE_A:
                box->SelectAll();
                return true;
            case SDL_SCANCODE_C:
            case SDL_SCANCODE_X:
            {
                const std::wstring selection = box->GetSelectedText();
                if (!selection.empty())
                {
                    const std::string utf8 = WideToUtf8(selection);
                    if (!utf8.empty())
                    {
                        SDL_SetClipboardText(utf8.c_str());
                        if (key.scancode == SDL_SCANCODE_X)
                            box->DeleteSelection();
                    }
                }
                return true;
            }
            case SDL_SCANCODE_V:
            {
                char* clip = SDL_GetClipboardText();
                if (clip != nullptr)
                {
                    const std::wstring wide = Utf8ToWide(clip);
                    if (!wide.empty())
                        box->OnTextInput(wide.c_str());
                    SDL_free(clip);
                }
                return true;
            }
            default:
                break;
            }
        }

        const int vk = MapScancodeToEditVk(key.scancode);
        if (vk == 0) return false;

        box->OnEditKey(vk, ctrl, shift);
        return true;
    }
}

// Resolution change through SDL (issue #462). SDL owns the window on every
// platform, so resize it via SDL rather than the OS. The old Windows path in
// ApplyResolution() drove Win32 SetWindowPos/ChangeDisplaySettings on g_hWnd,
// which fought SDL: it pins the min/max tracking size of a non-resizable
// window, so a raw SetWindowPos was clamped back and the resolution never
// changed unless a windowed/fullscreen toggle reset the style first.
// SDL_SetWindowSize resizes regardless of the resizable flag and drives the
// same HandleWindowResize update synchronously, so callers can Save() config
// right after and see the size the window actually ended up with.
void MuApplyWindowResolution(unsigned int width, unsigned int height, bool windowed)
{
    if (!g_sdlWindow || width == 0 || height == 0) return;
    const int w = static_cast<int>(width);
    const int h = static_cast<int>(height);

    if (windowed)
    {
        SDL_SetWindowFullscreen(g_sdlWindow, false);
        SDL_SetWindowSize(g_sdlWindow, w, h);
        SDL_SetWindowPosition(g_sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
    else
    {
        // Pick the closest real fullscreen mode so the monitor switches
        // resolution; fall back to borderless desktop if none matches.
        SDL_DisplayMode mode;
        const SDL_DisplayID display = SDL_GetDisplayForWindow(g_sdlWindow);
        if (SDL_GetClosestFullscreenDisplayMode(display, w, h, 0.0f, false, &mode))
            SDL_SetWindowFullscreenMode(g_sdlWindow, &mode);
        else
            SDL_SetWindowFullscreenMode(g_sdlWindow, nullptr);
        SDL_SetWindowSize(g_sdlWindow, w, h);
        SDL_SetWindowFullscreen(g_sdlWindow, true);
    }

    // The request is not a guarantee: the closest fullscreen mode can differ
    // from what was asked, the borderless fallback is desktop-sized, and mode
    // switches are asynchronous on some window managers. Settle the request,
    // then resize the game to the size the window really got - callers persist
    // WindowWidth/Height, and config must record what happened, not what was
    // asked for. Logical size, matching what SDL_EVENT_WINDOW_RESIZED carries.
    SDL_SyncWindow(g_sdlWindow);
    int actualW = w, actualH = h;
    SDL_GetWindowSize(g_sdlWindow, &actualW, &actualH);
    HandleWindowResize(actualW, actualH);
}

MSG MainLoop()
{
    constexpr auto target_resolution = 1;
    auto precise = timeBeginPeriod(target_resolution);

    while (!Destroy)
    {
        SDL_Event event;
        int messageProcessed = 0;

        // Per-frame mouse-state reset (was done per-message in WndProc): clear a
        // stale double-click and a pop whose position the cursor has moved off.
        MouseLButtonDBClick = false;
        if (MouseLButtonPop && (g_iMousePopPosition_x != MouseX || g_iMousePopPosition_y != MouseY))
            MouseLButtonPop = false;

        // Pumping SDL also drives the Win32 message hook (-> WndProc) for the
        // input still on it (IME, the legacy EDIT text boxes); mouse and window
        // events are handled here, off the hook.
        while (SDL_PollEvent(&event))
        {
#ifdef _EDITOR
            // Feed every event to the editor's ImGui SDL3 backend (issue #442).
            // Guard on an active context: editor init can fail or be shut down.
            if (ImGui::GetCurrentContext() != nullptr)
                ImGui_ImplSDL3_ProcessEvent(&event);
#endif
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                Destroy = true;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                HandleMouseMotion(event.motion.x, event.motion.y);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                HandleMouseButton(event);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                // SDL does not pre-correct flipped (natural) scrolling; invert.
                MouseWheel = (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
                    ? -static_cast<int>(event.wheel.y)
                    : static_cast<int>(event.wheel.y);
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                HandleWindowResize(event.window.data1, event.window.data2);
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                HandleFocusChange(true);
                break;
#ifndef _WIN32
            case SDL_EVENT_WINDOW_MOUSE_ENTER:
                // Wayland can deliver the first pointer-enter after the startup
                // cursor-hide, dropping it and leaving the OS cursor over the
                // game's own; re-apply the hide on every enter (issue #462).
                MuApplyCursorVisibility();
                break;
#endif
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                HandleFocusChange(false);
                break;
            case SDL_EVENT_TEXT_INPUT:
                // Committed characters for the focused portable text field (#447).
                FeedPortableTextInput(event.text.text);
                break;
            case SDL_EVENT_TEXT_EDITING:
                // IME composition preview for the focused portable field (#447).
                if (auto* box = CUITextInputBox::GetFocusedPortable())
                    box->OnTextEditing(Utf8ToWide(event.edit.text).c_str());
                break;
            case SDL_EVENT_KEY_DOWN:
#ifndef _WIN32
                // These mirror what WndProc does from Win32 messages, for the
                // SDL-only input path. On Windows WndProc is still driven (via
                // SDL_SetWindowsMessageHook), so doing them here too would
                // double-fire - guard them off there.
                //
                // Enter is gated through SetEnterPressed: ScanAsyncKeyState
                // suppresses a VK_RETURN press unless this fired that frame
                // (WM_CHAR does it on Windows). Without it Enter never reaches
                // the game (login submit, chat open).
                if (event.key.scancode == SDL_SCANCODE_RETURN ||
                    event.key.scancode == SDL_SCANCODE_KP_ENTER)
                {
                    SetEnterPressed(true);
                }
                // F10 toggles the camera zoom lock (WM_SYSKEYDOWN on Windows,
                // where F10 is a reserved system key). Without it the zoom stays
                // locked and the mouse wheel can never zoom. Edge-triggered.
                if (event.key.scancode == SDL_SCANCODE_F10 && !event.key.repeat)
                {
                    CameraManager::Instance().ToggleZoomLock();
                }
#endif
                // Navigation/erase/clipboard for the focused portable field (#447).
                FeedPortableKey(event.key);
                break;
            default:
                break;
            }

            ++messageProcessed;
            if (g_MaxMessagePerCycle > 0 && messageProcessed >= g_MaxMessagePerCycle)
            {
                break;
            }
        }

        // Start/stop SDL text input as a portable text field gains or loses
        // focus, so SDL only emits SDL_EVENT_TEXT_INPUT while one is active (#447).
        {
            static bool s_textInputActive = false;
            auto* focusedField = CUITextInputBox::GetFocusedPortable();
            const bool wantTextInput = focusedField != nullptr;
            if (wantTextInput != s_textInputActive && g_sdlWindow != nullptr)
            {
                if (wantTextInput)
                    SDL_StartTextInput(g_sdlWindow);
                else
                    SDL_StopTextInput(g_sdlWindow);
                s_textInputActive = wantTextInput;
            }

            // Anchor the IME candidate window at the caret (reference px -> window
            // px) so composition UI appears next to the text being typed (#447).
            int cx, cy, cw, ch;
            if (wantTextInput && g_sdlWindow != nullptr && focusedField->GetCaretArea(cx, cy, cw, ch))
            {
                const SDL_Rect area = {
                    static_cast<int>(cx * g_fScreenRate_x),
                    static_cast<int>(cy * g_fScreenRate_y),
                    static_cast<int>(cw * g_fScreenRate_x),
                    static_cast<int>(ch * g_fScreenRate_y) };
                // Only push when the caret rect actually moves; resending every
                // frame is wasteful and can flicker the candidate window.
                static SDL_Rect s_lastArea = { 0, 0, 0, 0 };
                if (area.x != s_lastArea.x || area.y != s_lastArea.y ||
                    area.w != s_lastArea.w || area.h != s_lastArea.h)
                {
                    SDL_SetTextInputArea(g_sdlWindow, &area, 0);
                    s_lastArea = area;
                }
            }
        }

        // Process server packets handed over from the network thread. Replaces
        // the old WM_RECEIVE_BUFFER message round-trip; runs on the main thread.
        Network::IncomingPacketQueue::Instance().DrainTo(ProcessPacketCallback);

        // Run a pending reconnect teardown between frames (self-guards on its
        // pending flag). Replaces the old WM_START_RECONNECT round-trip.
        ReconnectManager::Instance().Begin();

        // Fire any due timers. Replaces the Win32 SetTimer/WM_TIMER dispatch.
        Core::Time::FrameTimerScheduler::Instance().Tick();

        if (CheckRenderNextFrame())
        {
            if (g_bUseWindowMode || g_bWndActive || g_HasInactiveFpsOverride)
            {
#ifdef _EDITOR
                // F12 key toggle for editor
                static bool wasF12Pressed = false;
                if (Core::Input::IsKeyDown(VK_F12))
                {
                    if (!wasF12Pressed)
                    {
                        g_MuEditorCore.ToggleEditor();
                        fwprintf(stderr, L"[Editor] Toggled: %s\n",
                            g_MuEditorCore.IsEnabled() ? L"ON" : L"OFF");
                        fflush(stderr);
                        wasF12Pressed = true;
                    }
                }
                else
                {
                    wasF12Pressed = false;
                }

                // Update editor UI (must be before RenderScene)
                g_MuEditorCore.Update();
#endif

                // Render game scene (ImGui rendering happens inside before SwapBuffers)
                RenderScene(g_hDC);
            }
        }
        else
        {
            // SDL_PollEvent above already drained pending events, so just pace
            // the frame.
            WaitForNextActivity(precise == TIMERR_NOERROR);
        }

    } // while (!Destroy)

    if (precise == TIMERR_NOERROR)
    {
        timeEndPeriod(target_resolution);
    }

    return MSG{};
}

namespace
{
    // Tahoma font size scales with window height; these are the tuned base values.
    constexpr int BASE_FONT_HEIGHT = 12;
    constexpr float FONT_HEIGHT_GROWTH_PER_PIXEL = 1.f / 200.f;
    constexpr int FIX_FONT_HEIGHT_SMALL = 14;  // used when WindowHeight <= 600
    constexpr int FIX_FONT_HEIGHT_LARGE = 15;
    constexpr int SMALL_WINDOW_HEIGHT_THRESHOLD = 600;

    struct FontSizes { int uiFontSize; int fixFontSize; };

    FontSizes CalculateFontSizes()
    {
        FontHeight = static_cast<int>(std::ceil(
            BASE_FONT_HEIGHT + (WindowHeight - REFERENCE_HEIGHT) * FONT_HEIGHT_GROWTH_PER_PIXEL));
        int fixFontHeight = (WindowHeight <= SMALL_WINDOW_HEIGHT_THRESHOLD)
            ? FIX_FONT_HEIGHT_SMALL : FIX_FONT_HEIGHT_LARGE;
        return { FontHeight - 1, fixFontHeight - 1 };
    }

#ifdef _WIN32
    // Absolute path of a bundled font file (relative to ./fonts) next to the exe.
    // The curated names in kBundledFonts are ASCII, so the byte-wise widen is safe.
    std::wstring BundledFontFullPath(const char* relative)
    {
        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::wstring path(exePath);
        path.resize(path.find_last_of(L"\\/") + 1);   // keep the directory + separator
        while (*relative)
            path.push_back(static_cast<wchar_t>(static_cast<unsigned char>(*relative++)));
        return path;
    }
#endif

    // Privately register the TTFs bundled in ./fonts so GDI resolves their face
    // names even when they are not installed system-wide — parity with the Linux
    // GdiText shim, which reads ./fonts directly. FR_PRIVATE scopes the faces to
    // this process, leaving the system font list untouched. No-op off Windows,
    // where bundled fonts are resolved by GdiText. Shares the kBundledFonts table.
    void RegisterBundledFonts()
    {
#ifdef _WIN32
        for (const auto& font : kBundledFonts)
        {
            AddFontResourceExW(BundledFontFullPath(font.regular).c_str(), FR_PRIVATE, nullptr);
            AddFontResourceExW(BundledFontFullPath(font.bold).c_str(),    FR_PRIVATE, nullptr);
        }
#endif
    }

    // Mirrors RegisterBundledFonts so the process leaves no private faces behind.
    void UnregisterBundledFonts()
    {
#ifdef _WIN32
        for (const auto& font : kBundledFonts)
        {
            RemoveFontResourceExW(BundledFontFullPath(font.regular).c_str(), FR_PRIVATE, nullptr);
            RemoveFontResourceExW(BundledFontFullPath(font.bold).c_str(),    FR_PRIVATE, nullptr);
        }
#endif
    }

    HFONT CreateUIFont(int size, int weight)
    {
        // UI font family from config ([UI] Font); empty keeps the built-in
        // "Tahoma" default. On Windows GDI honors this face name directly; on
        // Linux the GdiText shim resolves it via fontconfig (Core/Platform/GdiText.cpp).
        std::wstring sel = GameConfig::GetInstance().GetFontSelection();
        const wchar_t* face = sel.empty() ? L"Tahoma" : sel.c_str();
        return CreateFont(size, 0, 0, 0, weight, 0, 0, 0, DEFAULT_CHARSET,
                          OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY,
                          DEFAULT_PITCH | FF_DONTCARE, face);
    }

    void CreateNewFonts(FontSizes sizes)
    {
        g_hFont     = CreateUIFont(sizes.uiFontSize, FW_NORMAL);
        g_hFontBold = CreateUIFont(sizes.uiFontSize, FW_SEMIBOLD);
        g_hFontBig  = CreateUIFont(sizes.uiFontSize * 2, FW_SEMIBOLD);
        g_hFixFont  = CreateUIFont(sizes.fixFontSize, FW_NORMAL);
    }

    void ReinitializeTextRenderer()
    {
        // Recreates the font buffer bitmap with new g_fScreenRate values
        g_pRenderText->Release();
        g_pRenderText->Create(g_hDC);
        g_pRenderText->SetFont(g_hFont);
    }

    void RefreshInventoryEquipmentSlots()
    {
        // Inventory slot positions depend on the text buffer size; MUST run after
        // ReinitializeTextRenderer().
        if (!g_pNewUISystem)
            return;
        auto* pInventory = g_pNewUISystem->GetUI_NewMyInventory();
        if (pInventory)
            pInventory->SetEquipmentSlotInfo();
    }

}

// Reinitialize fonts when window resolution changes
void ReinitializeFonts()
{
    // Save old font handles so we can delete them after the renderer has switched over
    HFONT hOldFont     = g_hFont;
    HFONT hOldFontBold = g_hFontBold;
    HFONT hOldFontBig  = g_hFontBig;
    HFONT hOldFixFont  = g_hFixFont;

    FontSizes sizes = CalculateFontSizes();
    CreateNewFonts(sizes);
    ReinitializeTextRenderer();

    if (hOldFont)     DeleteObject(hOldFont);
    if (hOldFontBold) DeleteObject(hOldFontBold);
    if (hOldFontBig)  DeleteObject(hOldFontBig);
    if (hOldFixFont)  DeleteObject(hOldFixFont);

    CInput::Instance().Create(g_hWnd, WindowWidth, WindowHeight);
    RefreshInventoryEquipmentSlots();
    // Text fields render through g_pRenderText and resolve their font by kind
    // each frame, so they need no per-control rebuild after a resolution change.
}

DWORD GetDesktopBitsPerPel()
{
    DEVMODE dm = {};
    dm.dmSize = sizeof(dm);
    if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm))
        return dm.dmBitsPerPel;
    return 32;
}

void UpdateCursorClip()
{
    // Confine cursor in fullscreen + active only. In windowed mode the user
    // must be able to move the cursor to other windows; when deactivated we
    // must also release so Windows can focus other apps.
    if (!g_hWnd || g_bUseWindowMode || !g_bWndActive)
    {
        ClipCursor(nullptr);
        return;
    }
    RECT client;
    if (!GetClientRect(g_hWnd, &client)) return;
    POINT tl = { client.left, client.top };
    POINT br = { client.right, client.bottom };
    ClientToScreen(g_hWnd, &tl);
    ClientToScreen(g_hWnd, &br);
    RECT clip = { tl.x, tl.y, br.x, br.y };
    ClipCursor(&clip);
}

// Update camera state when window resolution changes
void UpdateResolutionDependentSystems()
{
    // Force camera state update with new viewport dimensions
    // This updates ScreenCenterX/Y and PerspectiveX/Y used for 3D item positioning
    extern CameraState g_Camera;
    float aspectRatio = (float)WindowWidth / (float)WindowHeight;
    CameraProjection::SetupPerspective(g_Camera, g_Camera.FOV, aspectRatio,
                                       g_Camera.ViewNear, g_Camera.ViewFar * RENDER_DISTANCE_MULTIPLIER);

    // Update all 3D UI camera dimensions for proper item rendering
    if (g_pNewUI3DRenderMng)
    {
        g_pNewUI3DRenderMng->UpdateAllCameraDimensions(WindowWidth, WindowHeight);
    }

    // Reposition old-style CWin-based UI for the current scene. Without this,
    // login/character-scene info boxes stay anchored to the old screen size
    // until the player re-enters the scene.
    CUIMng::Instance().RepositionSceneUI();
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
#else
// Off Windows the Linux entry point (main.cpp) calls this directly.
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
#endif
{
    wchar_t lpszExeVersion[256] = L"unknown";

    wchar_t* lpszCommandLine = GetCommandLine();
    wchar_t lpszFile[MAX_PATH];
    WORD wVersion[4] = { 0, };
    if (GetFileNameOfFilePath(lpszFile, lpszCommandLine))
    {
        if (GetFileVersion(lpszFile, wVersion))
        {
            mu_swprintf(lpszExeVersion, L"%d.%02d", wVersion[0], wVersion[1]);
            if (wVersion[2] > 0)
            {
                wchar_t lpszMinorVersion[2] = L"a";
                lpszMinorVersion[0] += (wVersion[2] - 1);
                wcscat(lpszExeVersion, lpszMinorVersion);
            }
        }
    }

    g_ErrorReport.Write(L"\r\n");
    g_ErrorReport.WriteLogBegin();
    g_ErrorReport.AddSeparator();
    g_ErrorReport.Write(L"Mu online %ls (%ls) executed. (%d.%d.%d.%d)\r\n", lpszExeVersion, L"Eng", wVersion[0], wVersion[1], wVersion[2], wVersion[3]);

    g_ConsoleDebug->Write(MCD_NORMAL, L"Mu Online (Version: %d.%d.%d.%d)", wVersion[0], wVersion[1], wVersion[2], wVersion[3]);

    g_ErrorReport.WriteCurrentTime();
    ER_SystemInfo si;
    ZeroMemory(&si, sizeof(ER_SystemInfo));
    GetSystemInfo(&si);
    g_ErrorReport.AddSeparator();
    g_ErrorReport.WriteSystemInfo(&si);
    g_ErrorReport.AddSeparator();
    
    g_ErrorReport.Write(L"> To read config.ini.\r\n");

    // Load game settings from INI file first
    GameConfig::GetInstance().Load();

    // Check for command line server override
    WORD wPortNumber;
    if (GetConnectServerInfo(GetCommandLine(), g_lpszCmdURL, &wPortNumber))
    {
        szServerIpAddress = g_lpszCmdURL;
        g_ServerPort = wPortNumber;
    }
    else
    {
        // Use config.ini settings if no command line override
        static std::wstring serverIPFromConfig = GameConfig::GetInstance().GetServerIP();
        szServerIpAddress = serverIPFromConfig.c_str();
        g_ServerPort = GameConfig::GetInstance().GetServerPort();
    }

    //#ifdef _DEBUG

    m_Username[0] = '\0';
    m_Password[0] = '\0';
    m_SoundOnOff = 1;
    m_MusicOnOff = 1;
    m_Resolution = 0;
    m_RememberMe = 0;

    g_iChatInputType = 1;

    // Apply window settings from INI
    WindowWidth = GameConfig::GetInstance().GetWindowWidth();
    WindowHeight = GameConfig::GetInstance().GetWindowHeight();
    g_bUseWindowMode = GameConfig::GetInstance().GetWindowMode() ? TRUE : FALSE;
    g_bUseFullscreenMode = !g_bUseWindowMode;

    // Apply audio settings from INI — volume 0 = off, >0 = on
    m_SoundOnOff = (GameConfig::GetInstance().GetSoundVolume() > 0) ? 1 : 0;
    m_MusicOnOff = (GameConfig::GetInstance().GetMusicVolume() > 0) ? 1 : 0;

    // Apply login settings from INI
    m_RememberMe = GameConfig::GetInstance().GetRememberMe() ? 1 : 0;
    std::wstring langSelection = GameConfig::GetInstance().GetLanguageSelection();
    wcsncpy_s(g_aszMLSelection, langSelection.c_str(), MAX_LANGUAGE_NAME_LENGTH - 1);
    g_strSelectedML = g_aszMLSelection;

    if (m_RememberMe)
    {
        GameConfig::GetInstance().DecryptCredentials(m_Username, m_Password, _countof(m_Username), _countof(m_Password));
    }

    g_fScreenRate_x = (float)WindowWidth / (float)REFERENCE_WIDTH;
    g_fScreenRate_y = (float)WindowHeight / (float)REFERENCE_HEIGHT;


    pMultiLanguage = new CMultiLanguage(g_strSelectedML);

    if (g_iChatInputType == 1)
        ShowCursor(FALSE);

    // Fullscreen is requested via an SDL window flag below; SDL handles the
    // display-mode change and restores it on teardown.
    g_ErrorReport.Write(L"> Screen size = %d x %d.\r\n", WindowWidth, WindowHeight);

    g_hInst = hInstance;

    // SDL owns the window and GL context (issue #442).
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        g_ErrorReport.Write(L"> SDL video init failed.\r\n");
        MessageBox(nullptr, L"Windows aplication error!", L"Aplication Error", MB_ICONERROR);
        return 0;
    }

    // The fixed-function renderer needs a compatibility-profile GL context.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    SDL_WindowFlags windowFlags = SDL_WINDOW_OPENGL;
    if (g_bUseWindowMode != TRUE)
        windowFlags |= SDL_WINDOW_FULLSCREEN;

    g_sdlWindow = SDL_CreateWindow("MU Online", static_cast<int>(WindowWidth), static_cast<int>(WindowHeight), windowFlags);
    if (!g_sdlWindow)
    {
        g_ErrorReport.Write(L"> SDL_CreateWindow failed.\r\n");
        MessageBox(nullptr, L"Windows aplication error!", L"Aplication Error", MB_ICONERROR);
        return 0;
    }

    g_ErrorReport.Write(L"> Start window success.\r\n");

    // Initialize OpenGL viewport dimensions to match window dimensions
    // This ensures they're correct even if WM_SIZE hasn't fired yet or sent wrong values
    OpenglWindowWidth = WindowWidth;
    OpenglWindowHeight = WindowHeight;

    g_sdlGLContext = SDL_GL_CreateContext(g_sdlWindow);
    if (!g_sdlGLContext)
    {
        g_ErrorReport.Write(L"OpenGL Create Context Error.\r\n");
        KillGLWindow();
        MessageBox(nullptr, I18N::Game::InstallTheLatestGraphicsCardDriver, L"OpenGL Create Context Error.", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    SDL_GL_MakeCurrent(g_sdlWindow, g_sdlGLContext);

#ifdef _WIN32
    // Bridge SDL's native handles so the remaining Win32 code (IME, DirectSound,
    // cursor, the legacy EDIT-control text boxes) keeps working.
    g_hWnd = static_cast<HWND>(SDL_GetPointerProperty(
        SDL_GetWindowProperties(g_sdlWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    g_hDC = GetDC(g_hWnd);
    g_hRC = wglGetCurrentContext();

    // Drive the existing WndProc from SDL's Win32 messages (transitional, #442).
    SDL_SetWindowsMessageHook(Win32MessageHook, nullptr);
#endif // _WIN32

    SDL_RaiseWindow(g_sdlWindow);
    SetFocus(g_hWnd);

#ifndef _WIN32
    // The engine hid the OS cursor (it draws its own) before the SDL video
    // subsystem existed, so that call could not reach SDL. Apply the pending
    // state now that the window is up. On Windows WM_SETCURSOR keeps doing this.
    MuApplyCursorVisibility();
#endif

    g_ErrorReport.Write(L"> OpenGL init success.\r\n");
    g_ErrorReport.AddSeparator();
    g_ErrorReport.WriteOpenGLInfo();
    g_ErrorReport.AddSeparator();
    g_ErrorReport.WriteSoundCardInfo();

    // SDL_CreateWindow already shows the window.

    // Initialize translations with the saved UI locale (defaults to "en").
    // The editor still restores its own MuEditorConfig language preference
    // later in its init, which feeds through to I18N::SetLocale as well.
    {
        std::wstring uiLocaleW = GameConfig::GetInstance().GetUILocale();
        std::string uiLocale(uiLocaleW.begin(), uiLocaleW.end());
        I18N::SetLocale(uiLocale.c_str());
    }

#ifdef _EDITOR
    // Initialize MU Editor (ImGui SDL3 backend needs the SDL window + GL context).
    g_MuEditorCore.Initialize(g_sdlWindow, g_sdlGLContext);

    // Check for --editor command line flag
    if (szCmdLine && wcsstr(GetCommandLineW(), L"--editor"))
    {
        g_MuEditorCore.SetEnabled(true);
        fwprintf(stderr, L"[Editor] Starting in editor mode (--editor flag detected)\n");
        std::fflush(stderr);
    }
#endif

    g_ErrorReport.WriteImeInfo( g_hWnd);
    g_ErrorReport.AddSeparator();

    InitVSync();
    if (IsVSyncAvailable())
    {
        EnableVSync();
        SetTargetFps(-1); // unlimited
    }

    // Make the bundled ./fonts faces resolvable by GDI before the first CreateFont,
    // so a chosen curated font works even without a system-wide install.
    RegisterBundledFonts();
    CreateNewFonts(CalculateFontSizes());

    // Log which UI font was resolved now that the fonts have been created (the
    // discovery is lazy on first CreateFont). Helps diagnose "no UI text".
    g_ErrorReport.AddSeparator();
    g_ErrorReport.WriteFontInfo();
    g_ErrorReport.AddSeparator();

    setlocale(LC_ALL, "english");

    CInput::Instance().Create(g_hWnd, WindowWidth, WindowHeight);

    g_pNewUISystem->Create();

    // Always initialize audio system so music can be enabled at runtime
    AudioPlayer::Initialize();

    // Always initialize sound so it can be toggled at runtime
    InitDirectSound(g_hWnd);

    {
        int value = AudioPlayer::ClampVolume(GameConfig::GetInstance().GetSoundVolume());
        g_pOption->SetVolumeLevel(value);
        SetEffectVolumeLevel(value);
    }

    auto& timers = Core::Time::FrameTimerScheduler::Instance();
    timers.SetRepeating(HACK_TIMER, 20 * 1000, [] { CheckHack(); });
    timers.SetRepeating(MUHELPER_TIMER, 250 /* ms */,
        [] { MUHelper::CMuHelper::TimerProc(nullptr, 0, MUHELPER_TIMER, 0); });

    srand((unsigned)time(nullptr));

    for (int & i : RandomTable)
        i = rand() % 360;

    RendomMemoryDump = new BYTE[rand() % 100 + 1];
    GateAttribute = new GATE_ATTRIBUTE[MAX_GATES] { };
    SkillAttribute = new SKILL_ATTRIBUTE[MAX_SKILLS] { };
    ItemAttRibuteMemoryDump = new ITEM_ATTRIBUTE[MAX_ITEM + 1024] { };
    ItemAttribute = ((ITEM_ATTRIBUTE*)ItemAttRibuteMemoryDump) + rand() % 1024;
    CharacterMemoryDump = new CHARACTER[MAX_CHARACTERS_CLIENT + 1 + 128] { };
    CharactersClient = ((CHARACTER*)CharacterMemoryDump) + rand() % 128;
    CharacterMachine = new CHARACTER_MACHINE;

    memset(GateAttribute, 0, sizeof(GATE_ATTRIBUTE) * (MAX_GATES));
    memset(ItemAttribute, 0, sizeof(ITEM_ATTRIBUTE) * (MAX_ITEM));
    memset(SkillAttribute, 0, sizeof(SKILL_ATTRIBUTE) * (MAX_SKILLS));
    memset(CharacterMachine, 0, sizeof(CHARACTER_MACHINE));

    CharacterAttribute = &CharacterMachine->Character;
    CharacterMachine->Init();
    Hero = &CharactersClient[0];

    if (g_iChatInputType == 1)
    {
        g_pMercenaryInputBox = new CUIMercenaryInputBox;
        g_pSingleTextInputBox = new CUITextInputBox;
        g_pSinglePasswdInputBox = new CUITextInputBox;
    }

    g_pUIManager = new CUIManager;
    g_pUIMapName = new CUIMapName;	// rozy

    g_BuffSystem = BuffStateSystem::Make();

    g_MapProcess = MapProcess::Make();

    g_petProcess = PetProcess::Make();

    CUIMng::Instance().Create();

    if (g_iChatInputType == 1)
    {
        g_pMercenaryInputBox->Init(g_hWnd);
        g_pSingleTextInputBox->Init(g_hWnd, 200, 20);
        g_pSinglePasswdInputBox->Init(g_hWnd, 200, 20, 9, TRUE);
        g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);

        g_pMercenaryInputBox->SetFont(g_hFont);
        g_pSingleTextInputBox->SetFont(g_hFont);
        g_pSinglePasswdInputBox->SetFont(g_hFont);

        g_bIMEBlock = FALSE;
        HIMC  hIMC = ImmGetContext(g_hWnd);
        ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE);
        ImmReleaseContext(g_hWnd, hIMC);
        SaveIMEStatus();
        g_bIMEBlock = TRUE;
    }

#ifdef _WIN32
    if (g_bUseWindowMode == FALSE)
    {
        int nOldVal;
        SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &nOldVal, 0);
        SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &g_iScreenSaverOldValue, 0);
        SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 300 * 60, nullptr, 0);
    }
#endif // _WIN32

    std::thread cpuUsageRecorder(RecordCpuUsage);
    const MSG msg = MainLoop();

    // Teardown that used to run in WM_DESTROY, now after the loop exits (SDL owns
    // the window/GL context, so they must not be destroyed from a message).
    DestroySound();
#ifdef _EDITOR
    // Shut the editor's ImGui backends down while the GL context and SDL window
    // are still alive; the static destructor runs too late (after KillGLWindow).
    g_MuEditorCore.Shutdown();
#endif
    UnregisterBundledFonts();   // mirror the startup registration
    KillGLWindow();
    DestroyWindow();

    // RecordCpuUsage loops on !Destroy, so it exits once the loop above ended.
    // Join it before WinMain returns; a joinable std::thread destroyed unjoined
    // calls std::terminate.
    if (cpuUsageRecorder.joinable())
        cpuUsageRecorder.join();

    SDL_Quit();

    return msg.wParam;
}