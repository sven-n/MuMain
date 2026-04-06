///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <clocale>
#include "Data/GameConfig.h"
#include "UIWindows.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "Scenes/SceneCore.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzAI.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzLodTerrain.h"
#include "DSPlaySound.h"

#include "ZzzPath.h"
#include "Local.h"
#include "PersonalShopTitleImp.h"

#include "UIMapName.h" // rozy
#include "Core/CpuUsage.h"

#include "MuHelper.h"

#include "CBTMessageBox.h"
#ifdef _WIN32
#include "ThirdParty/regkey.h"
#endif

#include "CSChaosCastle.h"
#include "Input.h"
#include "Core/Timer.h"
#include "Core/MuTimer.h"
#include "UIMng.h"

// Story 5.2.1: miniaudio BGM Implementation [VS1-AUDIO-MINIAUDIO-BGM]
// wzAudio dependency removed — BGM now routed through g_platformAudio (MiniAudioBackend).
// Mp3FileName global removed — same-track guard is handled by MiniAudioBackend::m_currentMusicName.
#include "IPlatformAudio.h"
#include "MiniAudioBackend.h"

#include "w_MapHeaders.h"

#include "w_PetProcess.h"

#include "NewUISystem.h"
#include "Translation/i18n.h"

#ifdef _EDITOR
#include "../MuEditor/Core/MuEditorCore.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "../MuEditor/Config/MuEditorConfig.h"
#include "../MuEditor/Core/MuEditorCore.h"
// Forward declare ImGui WndProc handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

CUIMercenaryInputBox* g_pMercenaryInputBox = nullptr;
CUITextInputBox* g_pSingleTextInputBox = nullptr;
CUITextInputBox* g_pSinglePasswdInputBox = nullptr;
int g_iChatInputType = 1;
extern BOOL g_bIMEBlock;

CMultiLanguage* pMultiLanguage = nullptr;

extern DWORD g_dwTopWindow;

CUIManager* g_pUIManager = nullptr;
CUIMapName* g_pUIMapName = nullptr; // rozy

float Time_Effect = 0;
bool ashies = false;
int weather = rand() % 3;

HWND g_hWnd = nullptr;
HINSTANCE g_hInst = nullptr;
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;
HFONT g_hFont = nullptr;
HFONT g_hFontBold = nullptr;
HFONT g_hFontBig = nullptr;
HFONT g_hFixFont = nullptr;

CTimer* g_pTimer = new CTimer(); // performance counter.
mu::MuTimer g_muFrameTimer;      // frame time instrumentation (Story 7.2.1)
bool Destroy = false;
bool ActiveIME = false;

// Story 7.9.3: Global variables previously in WinMain/Win32 init block.
// Definitions moved here after Win32 entry point deletion; externs remain in
// MuMain.h, SDLEventLoop.cpp, and various game-code files.
wchar_t m_Username[11] = {};
wchar_t m_Password[21] = {};
wchar_t m_ExeVersion[11] = {};
int m_SoundOnOff = 0;
int m_MusicOnOff = 0;
int m_RememberMe = 0;
int g_iRenderTextType = 0;
int g_iNoMouseTime = 0;
int g_iMousePopPosition_x = 0;
int g_iMousePopPosition_y = 0;
bool g_bWndActive = false;
bool g_bEnterPressed = false;
double g_TargetFpsBeforeInactive = -1.0;
bool g_HasInactiveFpsOverride = false;
double CPU_AVG = 0.0;
int g_MaxMessagePerCycle = 1;

// Game memory allocations (Story 7.9.1: C++20 smart pointers)
std::unique_ptr<BYTE[]> g_RendomMemoryDumpPtr;
std::unique_ptr<GATE_ATTRIBUTE[]> g_GateAttributePtr;
std::unique_ptr<SKILL_ATTRIBUTE[]> g_SkillAttributePtr;
std::unique_ptr<ITEM_ATTRIBUTE[]> g_ItemAttRibuteMemoryDumpPtr;
std::unique_ptr<CHARACTER[]> g_CharacterMemoryDumpPtr;
std::unique_ptr<CHARACTER_MACHINE> g_CharacterMachinePtr;

// Raw pointers for compatibility (managed by above unique_ptrs)
BYTE* RendomMemoryDump;
ITEM_ATTRIBUTE* ItemAttRibuteMemoryDump;
CHARACTER* CharacterMemoryDump;

int RandomTable[100];

BOOL g_bMinimizedEnabled = FALSE;
int g_iScreenSaverOldValue = 60 * 15;

BOOL g_bUseWindowMode = TRUE;
BOOL g_bUseFullscreenMode = FALSE;

void StopMusic()
{
    if (g_platformAudio != nullptr)
    {
        // HIGH-2 fix (code-review-finalize 2026-03-19): use hard stop (enforce=TRUE) to
        // release the decoder and file handle immediately — mirrors wzAudioStop() behaviour.
        // Soft stop (FALSE) holds the file handle open until PlayMusic() is next called,
        // which can leak file descriptors across rapid server reconnects (WSclient.cpp:742,762).
        g_platformAudio->StopMusic(nullptr, TRUE);
    }
}

void StopMp3(const char* Name, BOOL bEnforce)
{
    if (!m_MusicOnOff && !bEnforce)
        return;
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->StopMusic(Name, bEnforce);
    }
}

void PlayMp3(const char* Name, BOOL bEnforce)
{
    if (Destroy)
        return;
    if (!m_MusicOnOff && !bEnforce)
        return;
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->PlayMusic(Name, bEnforce);
    }
}

bool IsEndMp3()
{
    if (g_platformAudio == nullptr)
        return true;
    return g_platformAudio->IsEndMusic();
}

int GetMp3PlayPosition()
{
    if (g_platformAudio == nullptr)
        return 0;
    return g_platformAudio->GetMusicPosition();
}

extern int LogIn;
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
    if (CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED ||
        CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED2)
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

// Story 7.9.3: Functions previously defined in the Win32 code block of MuMain.cpp.
// Restored as cross-platform stubs or full implementations after WinMain deletion.

void SetMaxMessagePerCycle(int messages)
{
    constexpr int custom_min = 3;
    g_MaxMessagePerCycle = (messages > 0) ? std::max<int>(messages, custom_min) : messages;
}

// Story 7.9.3: KillGLWindow() deleted — OpenGL context teardown was a no-op on SDL3.
// MuRenderer::Shutdown() handles GPU resource cleanup on the SDL_gpu path.

void DestroySound()
{
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->Shutdown();
        delete g_platformAudio;
        g_platformAudio = nullptr;
    }
}

DWORD GetCheckSum(WORD wKey)
{
    // Checksum validation stub. The original implementation used Win32 CreateFile/ReadFile
    // on a GameGuard file. On cross-platform builds, return 0 (no GameGuard).
    (void)wKey;
    return 0;
}

BOOL Util_CheckOption(std::wstring lpszCommandLine, wchar_t cOption, std::wstring& lpszString)
{
    if (lpszCommandLine.empty())
    {
        return FALSE;
    }

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

#include "Platform/MuPlatform.h"
#include "Platform/IPlatformWindow.h"
#include "RenderFX/MuRenderer.h"
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_main.h> // Story 7.9.3: Windows WinMain → main() remapping

namespace mu
{
[[nodiscard]] bool InitSDLGpuRenderer(void* pNativeWindow);
void ShutdownSDLGpuRenderer();
} // namespace mu

int MuMain(int argc, char* argv[])
{
    // Set working directory to the executable's own directory so that all relative
    // data paths (Data/, Translations/, shaders/) resolve correctly regardless of
    // which directory the process was launched from. No-op on Windows when the
    // launcher already sets CWD correctly; safe to call on all platforms.
    {
        std::error_code ec;
        std::filesystem::current_path(mu_get_app_dir(), ec);
    }

    // Open the error log now that CWD points to the exe directory.
    // CErrorReport constructor defers file creation to avoid writing to the shell's CWD.
    g_ErrorReport.Create(L"MuError.log");

    // Task 1.1 (Story 7.9.3): Error report log header — ported from Win32 init path.
    // Writes version separator and system info to MuError.log on startup.
    g_ErrorReport.Write(L"\r\n");
    g_ErrorReport.WriteLogBegin();
    g_ErrorReport.AddSeparator();
    g_ErrorReport.Write(L"Mu online SDL3 (Eng) executed.\r\n");
    g_ConsoleDebug->Write(MCD_NORMAL, L"Mu Online SDL3");
    g_ErrorReport.WriteCurrentTime();
    {
        ER_SystemInfo si;
        memset(&si, 0, sizeof(ER_SystemInfo));
        MuGetSystemInfo(&si);
        g_ErrorReport.AddSeparator();
        g_ErrorReport.WriteSystemInfo(&si);
        g_ErrorReport.AddSeparator();
    }

    g_ErrorReport.Write(L"> To read config.ini.\r\n");

    // Load game configuration from config.ini now that the CWD is resolved.
    // Must happen before window creation (WindowWidth/Height) and before any
    // locale-specific file loading (g_strSelectedML used by OpenBasicData).
    GameConfig::GetInstance().Load();
    WindowWidth = GameConfig::GetInstance().GetWindowWidth();
    WindowHeight = GameConfig::GetInstance().GetWindowHeight();
    g_bUseWindowMode = GameConfig::GetInstance().GetWindowMode() ? TRUE : FALSE;
    g_bUseFullscreenMode = !g_bUseWindowMode;
    m_SoundOnOff = GameConfig::GetInstance().GetSoundEnabled();
    m_MusicOnOff = GameConfig::GetInstance().GetMusicEnabled();
    m_RememberMe = GameConfig::GetInstance().GetRememberMe() ? 1 : 0;
    g_strSelectedML = GameConfig::GetInstance().GetLanguageSelection();

    // Set locale before any multibyte/wide conversions (e.g., mbstowcs in server override below).
    setlocale(LC_ALL, "");

    // Task 1.2 (Story 7.9.3): Command line server override — ported from Win32 init path.
    // Checks argv for -u <IP> and -p <PORT> to override config.ini server settings.
    {
        const char* cmdUrl = nullptr;
        const char* cmdPort = nullptr;
        for (int i = 1; i < argc - 1; ++i)
        {
            if (argv[i][0] == '-' || argv[i][0] == '/')
            {
                if (argv[i][1] == 'u' || argv[i][1] == 'U')
                    cmdUrl = argv[i + 1];
                else if (argv[i][1] == 'p' || argv[i][1] == 'P')
                    cmdPort = argv[i + 1];
            }
        }
        if (cmdUrl && cmdPort)
        {
            // Convert UTF-8 argv to wchar_t for szServerIpAddress
            static wchar_t s_CmdUrlW[64];
            mbstowcs(s_CmdUrlW, cmdUrl, 63);
            s_CmdUrlW[63] = L'\0';
            szServerIpAddress = s_CmdUrlW;
            long port = std::strtol(cmdPort, nullptr, 10);
            if (port > 0 && port <= 65535)
                g_ServerPort = static_cast<WORD>(port);
            else
                g_ErrorReport.Write(L"> WARNING: Invalid port '%hs', using default %d\r\n", cmdPort, (int)g_ServerPort);
            g_ErrorReport.Write(L"> Command line server override: %ls:%d\r\n", s_CmdUrlW, (int)g_ServerPort);
        }
        else
        {
            static std::wstring serverIPFromConfig = GameConfig::GetInstance().GetServerIP();
            szServerIpAddress = serverIPFromConfig.c_str();
            g_ServerPort = GameConfig::GetInstance().GetServerPort();
        }
    }

    // Task 1.3 (Story 7.9.3): Screen rate calculation — ported from Win32 init path.
    // Used throughout game for UI scaling relative to 640x480 reference resolution.
    g_fScreenRate_x = (float)WindowWidth / 640;
    g_fScreenRate_y = (float)WindowHeight / 480;
    g_ErrorReport.Write(L"> Screen size = %d x %d.\r\n", WindowWidth, WindowHeight);

    if (!mu::MuPlatform::Initialize())
    {
        return 1;
    }

    if (!mu::MuPlatform::CreateWindow("MU Online", WindowWidth, WindowHeight,
                                      g_bUseFullscreenMode ? mu::MU_WINDOW_FULLSCREEN : 0x0u))
    {
        mu::MuPlatform::Shutdown();
        return 1;
    }

#ifdef MU_ENABLE_SDL3
    {
        int nDisplayW = WindowWidth;
        int nDisplayH = WindowHeight;
        if (mu::MuPlatform::GetDisplaySize(nDisplayW, nDisplayH))
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"[VS1-SDL-WINDOW-FOCUS] Display size: %dx%d\r\n", nDisplayW, nDisplayH);
            if (g_bUseWindowMode == FALSE)
            {
                WindowWidth = nDisplayW;
                WindowHeight = nDisplayH;
            }
        }
    }

    // Story 4.3.1: Initialize SDL_gpu backend after window is created.
    mu::IPlatformWindow* pWin = mu::MuPlatform::GetWindow();
    if (!pWin || !mu::InitSDLGpuRenderer(pWin->GetNativeHandle()))
    {
        g_ErrorReport.Write(L"RENDER: SDL_gpu -- backend initialization failed; exiting");
        mu::MuPlatform::Shutdown();
        return 1;
    }
#endif

    // ---- Story 7.9.1: Game state initialisation (cross-platform init sequence) ----
    // Reproduces the cross-platform subset of the Win32 init path that ran
    // before the Win32 game loop. Win32-only init (CreateFont, SetTimer, CInput with
    // HWND, IME, screensaver suppression) is intentionally skipped.

    // Random seed and table (for gameplay RNG)
    srand((unsigned)time(nullptr));
    for (int& i : RandomTable)
        i = rand() % 360;

    // Game data array allocation — using smart pointers per C++20 project standard
    g_RendomMemoryDumpPtr = std::make_unique<BYTE[]>(101); // Fixed size for memory safety
    RendomMemoryDump = g_RendomMemoryDumpPtr.get();

    g_GateAttributePtr = std::make_unique<GATE_ATTRIBUTE[]>(MAX_GATES);
    GateAttribute = g_GateAttributePtr.get();

    g_SkillAttributePtr = std::make_unique<SKILL_ATTRIBUTE[]>(MAX_SKILLS);
    SkillAttribute = g_SkillAttributePtr.get();

    g_ItemAttRibuteMemoryDumpPtr = std::make_unique<ITEM_ATTRIBUTE[]>(MAX_ITEM + 1024);
    ItemAttRibuteMemoryDump = g_ItemAttRibuteMemoryDumpPtr.get();
    // Direct pointer assignment (no random offsets — security fix)
    ItemAttribute = ItemAttRibuteMemoryDump;

    g_CharacterMemoryDumpPtr = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT + 1 + 128);
    CharacterMemoryDump = g_CharacterMemoryDumpPtr.get();
    // Direct pointer assignment (no random offsets)
    CharactersClient = CharacterMemoryDump;

    g_CharacterMachinePtr = std::make_unique<CHARACTER_MACHINE>();
    CharacterMachine = g_CharacterMachinePtr.get();

    memset(GateAttribute, 0, sizeof(GATE_ATTRIBUTE) * (MAX_GATES));
    memset(ItemAttribute, 0, sizeof(ITEM_ATTRIBUTE) * (MAX_ITEM));
    memset(SkillAttribute, 0, sizeof(SKILL_ATTRIBUTE) * (MAX_SKILLS));
    memset(CharacterMachine, 0, sizeof(CHARACTER_MACHINE));

    CharacterAttribute = &CharacterMachine->Character;
    CharacterMachine->Init();
    Hero = &CharactersClient[0];

    // UI managers
    g_pUIManager = new CUIManager;
    g_pUIMapName = new CUIMapName;
    pMultiLanguage = new CMultiLanguage(g_strSelectedML);

    g_BuffSystem = BuffStateSystem::Make();
    g_MapProcess = MapProcess::Make();
    g_petProcess = PetProcess::Make();

    CInput::Instance().SetScreenSize(WindowWidth, WindowHeight);
    CUIMng::Instance().Create();
    if (g_pNewUISystem != nullptr)
    {
        g_pNewUISystem->Create();
    }
    else
    {
        g_ErrorReport.Write(L"WARNING: g_pNewUISystem is null, skipping Create()\r\n");
    }

    // i18n translations
    {
        i18n::Translator& translator = i18n::Translator::GetInstance();
        bool gameLoaded = translator.LoadTranslations(i18n::Domain::Game, L"Translations/en/game.json");
        if (!gameLoaded)
            gameLoaded = translator.LoadTranslations(i18n::Domain::Game, L"bin/Translations/en/game.json");
        translator.SetLocale("en");

        if (gameLoaded)
        {
            g_ErrorReport.Write(L"> Game translations loaded successfully.\r\n");
        }
        else
        {
            g_ErrorReport.Write(L"> WARNING: Game translations not found (game.json missing).\r\n");
        }
    }

    // Audio backend
    if (m_MusicOnOff || m_SoundOnOff)
    {
        g_platformAudio = new mu::MiniAudioBackend();
        if (!g_platformAudio->Initialize())
        {
            g_ErrorReport.Write(L"AUDIO: MiniAudioBackend::Initialize failed — game will run without audio\r\n");
        }
    }

    if (m_SoundOnOff && g_pOption != nullptr)
    {
        int value = GameConfig::GetInstance().GetVolumeLevel();
        if (value < 0 || value >= 10)
            value = 5;
        g_pOption->SetVolumeLevel(value);
    }

    // Restore BGM and SFX volume from config
    if (g_pOption != nullptr)
    {
        int bgmLevel = GameConfig::GetInstance().GetBGMVolumeLevel();
        int sfxLevel = GameConfig::GetInstance().GetSFXVolumeLevel();
        if (bgmLevel < 0 || bgmLevel > 10)
            bgmLevel = 5;
        if (sfxLevel < 0 || sfxLevel > 10)
            sfxLevel = 5;
        g_pOption->SetBGMVolumeLevel(bgmLevel);
        g_pOption->SetSFXVolumeLevel(sfxLevel);
    }

    // Set audio volume if audio backend is available
    if (g_platformAudio != nullptr && g_pOption != nullptr)
    {
        int bgmLevel = GameConfig::GetInstance().GetBGMVolumeLevel();
        int sfxLevel = GameConfig::GetInstance().GetSFXVolumeLevel();
        if (bgmLevel < 0 || bgmLevel > 10)
            bgmLevel = 5;
        if (sfxLevel < 0 || sfxLevel > 10)
            sfxLevel = 5;
        g_platformAudio->SetBGMVolume(static_cast<float>(bgmLevel) / 10.0f);
        g_platformAudio->SetSFXVolume(static_cast<float>(sfxLevel) / 10.0f);
    }

    // ---- End game state initialisation ----
    // Game data loading (OpenBasicData) happens on the first frame via
    // RenderScene → WebzenScene (SceneFlag == WEBZEN_SCENE).
    // WebzenScene handles fonts, title bitmaps, loading screen, and
    // transitions SceneFlag to LOG_IN_SCENE when done.

    while (!Destroy)
    {
        if (!mu::MuPlatform::PollEvents())
        {
            break;
        }

        if (CheckRenderNextFrame())
        {
            g_muFrameTimer.FrameStart();

#ifdef MU_ENABLE_SDL3
            // Drain packets queued by the .NET I/O thread before processing the frame.
            // On Win32 this happens via PostMessage/WM_RECEIVE_BUFFER in the message pump;
            // on SDL3 we poll the thread-safe queue directly.
            DrainPacketQueue();

            // Story 4.3.1: Per-frame SDL_gpu command buffer / render pass lifecycle.
            mu::GetRenderer().BeginFrame();

            // Story 7.9.1: Full render path — RenderScene dispatches to
            // WebzenScene / LoadingScene / MainScene based on SceneFlag.
            // hDC is nullptr on SDL3 — HDC dereferences removed (story 7-9-2).
            RenderScene(nullptr);

            mu::GetRenderer().EndFrame();
#endif

            g_muFrameTimer.FrameEnd();
        }
        else
        {
            SDL_Delay(1); // Yield CPU when not rendering to avoid spinlock
        }
    }

    // UI cleanup (mirrors DestroyWindow() in Win32 path)
    SAFE_DELETE(g_pUIMapName);
    SAFE_DELETE(g_pUIManager);

    // Smart pointer managers
    PtrReset(g_BuffSystem);
    PtrReset(g_MapProcess);
    PtrReset(g_petProcess);

    // Audio cleanup (mirrors DestroySound() in Win32 path)
    if (g_platformAudio != nullptr)
    {
        g_platformAudio->Shutdown();
        delete g_platformAudio;
        g_platformAudio = nullptr;
    }

#ifdef MU_ENABLE_SDL3
    mu::ShutdownSDLGpuRenderer();
#endif

    mu::MuPlatform::Shutdown();
    return 0;
}

int main(int argc, char* argv[])
{
    return MuMain(argc, argv);
}
