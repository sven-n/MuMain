///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

#include <dpapi.h>
#include <locale.h>
#include <zmouse.h>
#include <cmath>
#include "UIWindows.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "zzzLodTerrain.h"
#include "DSPlaySound.h"

#include "Resource.h"
#include <imm.h>
#include "zzzpath.h"
#include "Nprotect.h"
#include "Local.h"
#include "PersonalShopTitleImp.h"
#include "./Utilities/Log/ErrorReport.h"
#include "UIMapName.h"		// rozy
#include "./ExternalObject/leaf/ExceptionHandler.h"
#include "./Utilities/Dump/CrashReporter.h"
#include "./Utilities/Log/muConsoleDebug.h"
#include "./Utilities/CpuUsage.h"
#include "ProtocolSend.h"
#include "ProtectSysKey.h"
#include "MUHelper/MuHelper.h"

#include "CBTMessageBox.h"
#include "./ExternalObject/leaf/regkey.h"

#include "CSChaosCastle.h"
#include "GMHellas.h"
#include <io.h>
#include "Input.h"
#include "./Time/Timer.h"
#include "UIMng.h"

#ifdef MOVIE_DIRECTSHOW
#include <dshow.h>
#include "MovieScene.h"
#endif // MOVIE_DIRECTSHOW
#include "GameCensorship.h"

#include "w_MapHeaders.h"

#include "w_PetProcess.h"

#include <ThemidaInclude.h>

#include "MultiLanguage.h"
#include "NewUISystem.h"

CUIMercenaryInputBox* g_pMercenaryInputBox = NULL;
CUITextInputBox* g_pSingleTextInputBox = NULL;
CUITextInputBox* g_pSinglePasswdInputBox = NULL;
int g_iChatInputType = 1;
extern BOOL g_bIMEBlock;

CMultiLanguage* pMultiLanguage = NULL;

extern DWORD g_dwTopWindow;

#ifdef MOVIE_DIRECTSHOW
CMovieScene* g_pMovieScene = NULL;
#endif // MOVIE_DIRECTSHOW

CUIManager* g_pUIManager = NULL;
CUIMapName* g_pUIMapName = NULL;		// rozy

float Time_Effect = 0;
bool ashies = false;
int weather = rand() % 3;

HWND      g_hWnd = NULL;
HINSTANCE g_hInst = NULL;
HDC       g_hDC = NULL;
HGLRC     g_hRC = NULL;
HFONT     g_hFont = NULL;
HFONT     g_hFontBold = NULL;
HFONT     g_hFontBig = NULL;
HFONT     g_hFixFont = NULL;

CTimer* g_pTimer = new CTimer();    // performance counter.
bool      Destroy = false;
bool      ActiveIME = false;

BYTE* RendomMemoryDump;
ITEM_ATTRIBUTE* ItemAttRibuteMemoryDump;
CHARACTER* CharacterMemoryDump;

int       RandomTable[100];

char TextMu[] = "mu.exe";

CErrorReport g_ErrorReport;

BOOL g_bMinimizedEnabled = FALSE;
int g_iScreenSaverOldValue = 60 * 15;

extern float g_fScreenRate_x;	// ※
extern float g_fScreenRate_y;

#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
BOOL g_bUseWindowMode = TRUE;
#endif

char Mp3FileName[256];

#pragma comment(lib, "wzAudio.lib")
#include <wzAudio.h>

void StopMp3(char* Name, BOOL bEnforce)
{
    if (!m_MusicOnOff && !bEnforce) return;

    if (Mp3FileName[0] != NULL)
    {
        if (strcmp(Name, Mp3FileName) == 0) {
            wzAudioStop();
            Mp3FileName[0] = NULL;
        }
    }
}

void PlayMp3(char* Name, BOOL bEnforce)
{
    if (Destroy) return;
    if (!m_MusicOnOff && !bEnforce) return;

    if (strcmp(Name, Mp3FileName) == 0)
    {
        return;
    }
    else
    {
        wzAudioPlay(Name, 1);
        strcpy(Mp3FileName, Name);
    }
}

bool IsEndMp3()
{
    if (100 == wzAudioGetStreamOffsetRange())
        return true;
    return false;
}

int GetMp3PlayPosition()
{
    return wzAudioGetStreamOffsetRange();
}

extern int  LogIn;
extern wchar_t LogInID[];
extern bool First;
extern int FirstTime;
extern BOOL g_bGameServerConnected;

void CheckHack(void)
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
    if (g_hRC)
    {
        wglMakeCurrent(nullptr, nullptr);
        if (!wglDeleteContext(g_hRC))
        {
            g_ErrorReport.Write(L"GL - Release Rendering Context Failed\r\n");
            MessageBox(NULL, L"Release Rendering Context Failed.", L"Error", MB_OK | MB_ICONINFORMATION);
        }

        g_hRC = NULL;
    }

    if (g_hDC && !ReleaseDC(g_hWnd, g_hDC))
    {
        g_ErrorReport.Write(L"GL - OpenGL Release Error\r\n");
        MessageBox(NULL, L"OpenGL Release Error.", L"Error", MB_OK | MB_ICONINFORMATION);
        g_hDC = NULL;
    }

#if (defined WINDOWMODE)
    if (g_bUseWindowMode == FALSE)
    {
        ChangeDisplaySettings(NULL, 0);
        ShowCursor(TRUE);
    }
#else
#ifdef ENABLE_FULLSCREEN
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
    if (g_bUseWindowMode == FALSE)
#endif	// USER_WINDOW_MODE
    {
        ChangeDisplaySettings(NULL, 0);
        ShowCursor(TRUE);
    }
#endif //ENABLE_FULLSCREEN
#endif	//WINDOWMODE(#else)
}

BOOL GetFileNameOfFilePath(wchar_t* lpszFile, wchar_t* lpszPath)
{
    int iFind = (int)'\\';
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

    HANDLE hFile = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return (0);
    }

    DWORD dwSize = GetFileSize(hFile, NULL);
    BYTE* pbyBuffer = new BYTE[dwSize];
    DWORD dwNumber;
    ReadFile(hFile, pbyBuffer, dwSize, &dwNumber, 0);
    CloseHandle(hFile);

    DWORD dwCheckSum = GenerateCheckSum(pbyBuffer, dwSize, wKey);
    delete[] pbyBuffer;

    return (dwCheckSum);
}

BOOL GetFileVersion(wchar_t* lpszFileName, WORD* pwVersion)
{
    DWORD dwHandle;
    DWORD dwLen = GetFileVersionInfoSize(lpszFileName, &dwHandle);
    if (dwLen <= 0)
    {
        return (FALSE);
    }

    BYTE* pbyData = new BYTE[dwLen];
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
}

extern PATH* path;

void DestroyWindow()
{
    //. save volume level
    leaf::CRegKey regkey;
    regkey.SetKey(leaf::CRegKey::_HKEY_CURRENT_USER, L"SOFTWARE\\Webzen\\Mu\\Config");
    regkey.WriteDword(L"VolumeLevel", g_pOption->GetVolumeLevel());

    CUIMng::Instance().Release();

#ifdef MOVIE_DIRECTSHOW
    if (g_pMovieScene)
    {
        g_pMovieScene->Destroy();
    }
#endif // MOVIE_DIRECTSHOW

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

#ifdef MOVIE_DIRECTSHOW
    if (SceneFlag != MOVIE_SCENE)
#endif // MOVIE_DIRECTSHOW
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

#ifdef MOVIE_DIRECTSHOW
    SAFE_DELETE(g_pMovieScene);
#endif // MOVIE_DIRECTSHOW

    SAFE_DELETE(pMultiLanguage);
    BoostRest(g_BuffSystem);
    BoostRest(g_MapProcess);
    BoostRest(g_petProcess);

    g_ErrorReport.Write(L"Destroy");

    HWND shWnd = FindWindow(NULL, L"MuPlayer");
    if (shWnd)
        SendMessage(shWnd, WM_DESTROY, 0, 0);
}
void DestroySound()
{
    for (int i = 0; i < MAX_BUFFER; i++)
        ReleaseBuffer(i);

    FreeDirectSound();
    wzAudioDestroy();
}

int g_iInactiveTime = 0;
int g_iNoMouseTime = 0;
int g_iInactiveWarning = 0;
bool g_bWndActive = false;
bool HangulDelete = false;
int Hangul = 0;
bool g_bEnterPressed = false;

int g_iMousePopPosition_x = 0;
int g_iMousePopPosition_y = 0;

extern int TimeRemain;
extern bool EnableFastInput;

LONG FAR PASCAL WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SYSKEYDOWN:
    {
        return 0;
    }
    break;
#if defined PROTECT_SYSTEMKEY && defined NDEBUG
#ifndef FOR_WORK
    case WM_SYSCOMMAND:
    {
        if (wParam == SC_KEYMENU || wParam == SC_SCREENSAVE)
        {
            return 0;
        }
    }
    break;
#endif // !FOR_WORK
#endif // PROTECT_SYSTEMKEY && NDEBUG
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
#ifdef ACTIVE_FOCUS_OUT
            if (g_bUseWindowMode == FALSE)
#endif	// ACTIVE_FOCUS_OUT
                g_bWndActive = false;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
            if (g_bUseWindowMode == TRUE)
            {
                MouseLButton = false;
                MouseLButtonPop = false;
                //MouseLButtonPush = false;
                MouseRButton = false;
                MouseRButtonPop = false;
                MouseRButtonPush = false;
                MouseLButtonDBClick = false;
                MouseMButton = false;
                MouseMButtonPop = false;
                MouseMButtonPush = false;
                MouseWheel = 0;
            }
#endif
        }
        else
        {
            g_bWndActive = true;
        }
        break;
    case WM_TIMER:
        //MessageBox(NULL,GlobalText[16],"Error",MB_OK);
        switch (wParam)
        {
        case HACK_TIMER:
            // PKD_ADD_BINARY_PROTECTION
            VM_START
                CheckHack();
            VM_END
                break;
        case WINDOWMINIMIZED_TIMER:
            PostMessage(g_hWnd, WM_CLOSE, 0, 0);
            break;
        case CHATCONNECT_TIMER:
            g_pFriendMenu->SendChatRoomConnectCheck();
            break;
        case SLIDEHELP_TIMER:
            if (g_bWndActive)
            {
                if (g_pSlideHelpMgr)
                    g_pSlideHelpMgr->CreateSlideText();
            }
            break;
        }
        break;
    case WM_RECEIVE_BUFFER: {
        auto Packet = std::unique_ptr<PacketInfo>(reinterpret_cast<PacketInfo*>(wParam));
        ProcessPacketCallback(Packet.release());
        break;
    }
    case WM_USER_MEMORYHACK:
        //SetTimer( g_hWnd, WINDOWMINIMIZED_TIMER, 1*1000, NULL);
        KillGLWindow();
        break;
    case WM_NPROTECT_EXIT_TWO:
        SocketClient->ToGameServer()->SendLogOutByCheatDetection(0);
        SetTimer(g_hWnd, WINDOWMINIMIZED_TIMER, 1 * 1000, NULL);
        MessageBox(NULL, GlobalText[16], L"Error", MB_OK);
        break;
    case WM_ASYNCSELECTMSG:
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_CONNECT:
            break;
        case FD_READ:
            break;
        case FD_CLOSE:
            g_pSystemLogBox->AddText(GlobalText[3], SEASON3B::TYPE_SYSTEM_MESSAGE);
#ifdef CONSOLE_DEBUG
            switch (WSAGETSELECTERROR(lParam))
            {
            case WSAECONNRESET:
                g_ConsoleDebug->Write(MCD_ERROR, L"The connection was reset by the remote side.");
                g_ErrorReport.Write(L"The connection was reset by the remote side.\r\n");
                g_ErrorReport.WriteCurrentTime();
                break;
            case WSAECONNABORTED:
                g_ConsoleDebug->Write(MCD_ERROR, L"The connection was terminated due to a time-out or other failure.");
                g_ErrorReport.Write(L"The connection was terminated due to a time-out or other failure.\r\n");
                g_ErrorReport.WriteCurrentTime();
                break;
            }
#endif // CONSOLE_DEBUG

            if (SocketClient != nullptr)
            {
                SocketClient->Close();
                g_bGameServerConnected = false;
            }

            CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
            break;
        }
        break;
    case WM_CTLCOLOREDIT:
        SetBkColor((HDC)wParam, RGB(0, 0, 0));
        SetTextColor((HDC)wParam, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
        break;
    case WM_ERASEBKGND:
        return TRUE;
        break;
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
        Destroy = true;
        if (SocketClient != nullptr)
        {
            SocketClient->Close();
            g_bGameServerConnected = false;
        }

        DestroySound();
        //DestroyWindow();
        KillGLWindow();
        PostQuitMessage(0);
    }
    break;
    case WM_SETCURSOR:
        ShowCursor(false);
        break;
        //-----------------------------
    default:
        //if (msg >= WM_CHATROOMMSG_BEGIN && msg < WM_CHATROOMMSG_END)
        //	g_pChatRoomSocketList->ProcessSocketMessage(msg - WM_CHATROOMMSG_BEGIN, WSAGETSELECTEVENT(lParam));
        break;
    }

    MouseLButtonDBClick = false;
    if (MouseLButtonPop == true && (g_iMousePopPosition_x != MouseX || g_iMousePopPosition_y != MouseY))
        MouseLButtonPop = false;
    switch (msg)
    {
    case WM_MOUSEMOVE:
    {
        MouseX = (float)LOWORD(lParam) / g_fScreenRate_x;
        MouseY = (float)HIWORD(lParam) / g_fScreenRate_y;
        if (MouseX < 0)
            MouseX = 0;
        if (MouseX > 640)
            MouseX = 640;
        if (MouseY < 0)
            MouseY = 0;
        if (MouseY > 480)
            MouseY = 480;
    }
    break;
    case WM_LBUTTONDOWN:
        g_iNoMouseTime = 0;
        MouseLButtonPop = false;
        if (!MouseLButton)
            MouseLButtonPush = true;
        MouseLButton = true;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
        SetCapture(g_hWnd);
#endif
        break;
    case WM_LBUTTONUP:
        g_iNoMouseTime = 0;
        MouseLButtonPush = false;
        //if(MouseLButton) MouseLButtonPop = true;
        MouseLButtonPop = true;
        MouseLButton = false;
        g_iMousePopPosition_x = MouseX;
        g_iMousePopPosition_y = MouseY;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
        ReleaseCapture();
#endif
        break;
    case WM_RBUTTONDOWN:
        g_iNoMouseTime = 0;
        MouseRButtonPop = false;
        if (!MouseRButton) MouseRButtonPush = true;
        MouseRButton = true;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
        SetCapture(g_hWnd);
#endif
        break;
    case WM_RBUTTONUP:
        g_iNoMouseTime = 0;
        MouseRButtonPush = false;
        if (MouseRButton) MouseRButtonPop = true;
        MouseRButton = false;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
        ReleaseCapture();
#endif
        break;
    case WM_LBUTTONDBLCLK:
        g_iNoMouseTime = 0;
        MouseLButtonDBClick = true;
        break;
    case WM_MBUTTONDOWN:
        g_iNoMouseTime = 0;
        MouseMButtonPop = false;
        if (!MouseMButton) MouseMButtonPush = true;
        MouseMButton = true;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
        SetCapture(g_hWnd);
#endif
        break;
    case WM_MBUTTONUP:
        g_iNoMouseTime = 0;
        MouseMButtonPush = false;
        if (MouseMButton) MouseMButtonPop = true;
        MouseRButton = false;
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
        ReleaseCapture();
#endif
        break;
    case WM_MOUSEWHEEL:
    {
        MouseWheel = (short)HIWORD(wParam) / WHEEL_DELTA;
    }
    break;
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

    if (g_BuffSystem) {
        LRESULT result;
        TheBuffStateSystem().HandleWindowMessage(msg, wParam, lParam, result);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool CreateOpenglWindow()
{
    PIXELFORMATDESCRIPTOR pfd;

    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;

    if (!(g_hDC = GetDC(g_hWnd)))
    {
        g_ErrorReport.Write(L"OpenGL Get DC Error - ErrorCode : %d\r\n", GetLastError());
        KillGLWindow();
        MessageBox(NULL, GlobalText[4], L"OpenGL Get DC Error.", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    GLuint PixelFormat;

    if (!(PixelFormat = ChoosePixelFormat(g_hDC, &pfd)))
    {
        g_ErrorReport.Write(L"OpenGL Choose Pixel Format Error - ErrorCode : %d\r\n", GetLastError());
        KillGLWindow();
        MessageBox(NULL, GlobalText[4], L"OpenGL Choose Pixel Format Error.", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (!SetPixelFormat(g_hDC, PixelFormat, &pfd))
    {
        g_ErrorReport.Write(L"OpenGL Set Pixel Format Error - ErrorCode : %d\r\n", GetLastError());
        KillGLWindow();
        MessageBox(NULL, GlobalText[4], L"OpenGL Set Pixel Format Error.", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (!(g_hRC = wglCreateContext(g_hDC)))
    {
        g_ErrorReport.Write(L"OpenGL Create Context Error - ErrorCode : %d\r\n", GetLastError());
        KillGLWindow();
        MessageBox(NULL, GlobalText[4], L"OpenGL Create Context Error.", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (!wglMakeCurrent(g_hDC, g_hRC))
    {
        g_ErrorReport.Write(L"OpenGL Make Current Error - ErrorCode : %d\r\n", GetLastError());
        KillGLWindow();
        MessageBox(NULL, GlobalText[4], L"OpenGL Make Current Error.", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    ShowWindow(g_hWnd, SW_SHOW);
    SetForegroundWindow(g_hWnd);
    SetFocus(g_hWnd);
    return true;
}

HWND StartWindow(HINSTANCE hInstance, int nCmdShow)
{
    wchar_t* windowName = L"MU Online";
    WNDCLASS wndClass;
    HWND hWnd;

    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_ICON1);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = windowName;

    if (!RegisterClass(&wndClass))
    {
        MessageBox(NULL, L"Windows aplication error!", L"Aplication Error", MB_ICONERROR);
        return 0;
    }

    if (g_bUseWindowMode == TRUE)
    {
        RECT rc = { 0, 0, WindowWidth, WindowHeight };
        AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, NULL);
        hWnd = CreateWindow(
            windowName, windowName,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN,
            (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2,
            (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2,
            rc.right - rc.left,
            rc.bottom - rc.top,
            NULL, NULL, hInstance, NULL);
    }
    else
    {
        hWnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_APPWINDOW,
            windowName, windowName,
            WS_POPUP,
            0, 0,
            WindowWidth,
            WindowHeight,
            NULL, NULL, hInstance, NULL);
    }
    return hWnd;
}

wchar_t m_ID[11];
wchar_t m_Password[21];
char m_EncryptedPassword[262];
wchar_t m_Version[11];
wchar_t m_ExeVersion[11];
int  m_SoundOnOff;
int  m_MusicOnOff;
int  m_Resolution;
int	m_nColorDepth;
int m_RememberMe;
int	g_iRenderTextType = 0;

wchar_t g_aszMLSelection[MAX_LANGUAGE_NAME_LENGTH] = { '\0' };
std::wstring g_strSelectedML = L"";

BOOL OpenInitFile()
{
    //char szTemp[50];
    wchar_t szIniFilePath[256 + 20] = L"";
    wchar_t szCurrentDir[256];

    GetCurrentDirectory(256, szCurrentDir);

    wcscpy(szIniFilePath, szCurrentDir);
    if (szCurrentDir[wcslen(szCurrentDir) - 1] == '\\')
        wcscat(szIniFilePath, L"config.ini");
    else wcscat(szIniFilePath, L"\\config.ini");

    GetPrivateProfileString(L"LOGIN", L"Version", L"", m_Version, 11, szIniFilePath);

    wchar_t* lpszCommandLine = GetCommandLine();
    wchar_t lpszFile[MAX_PATH];
    if (GetFileNameOfFilePath(lpszFile, lpszCommandLine))
    {
        WORD wVersion[4];
        if (GetFileVersion(lpszFile, wVersion))
        {
            swprintf(m_ExeVersion, L"%d.%02d", wVersion[0], wVersion[1]);
            if (wVersion[2] > 0)
            {
                wchar_t lpszMinorVersion[3] = L"a";
                if (wVersion[2] > 26)
                {
                    lpszMinorVersion[0] = 'A';
                    lpszMinorVersion[0] += (wVersion[2] - 27);
                    lpszMinorVersion[1] = '+';
                }
                else
                {
                    lpszMinorVersion[0] += (wVersion[2] - 1);
                }
                wcscat(m_ExeVersion, lpszMinorVersion);
            }
        }
        else
        {
            wcscpy(m_ExeVersion, m_Version);
        }
    }
    else
    {
        wcscpy(m_ExeVersion, m_Version);
    }

    //#ifdef _DEBUG

    m_ID[0] = '\0';
    m_Password[0] = '\0';
    m_SoundOnOff = 1;
    m_MusicOnOff = 1;
    m_Resolution = 0;
    m_nColorDepth = 0;
    m_RememberMe = 0;

    HKEY hKey;
    DWORD dwDisp;
    DWORD dwSize;
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Webzen\\Mu\\Config", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp))
    {
        dwSize = sizeof(m_ID);
        if (RegQueryValueEx(hKey, L"ID", 0, NULL, (LPBYTE)m_ID, &dwSize) != ERROR_SUCCESS)
        {
        }
        dwSize = sizeof(m_EncryptedPassword);
        if (RegQueryValueEx(hKey, L"Password", 0, NULL, (LPBYTE)m_EncryptedPassword, &dwSize) != ERROR_SUCCESS)
        {
        }
        else
        {
            DATA_BLOB dataIn;
            DATA_BLOB dataOut;
            dataIn.pbData = (BYTE*)m_EncryptedPassword;
            dataIn.cbData = dwSize;

            if (CryptUnprotectData(&dataIn, NULL, NULL, NULL, NULL, 0, &dataOut))
            {
                wcscpy(m_Password, (wchar_t*)dataOut.pbData);
                LocalFree(dataOut.pbData);
            }
        }

        dwSize = sizeof(int);
        if (RegQueryValueEx(hKey, L"SoundOnOff", 0, NULL, (LPBYTE)&m_SoundOnOff, &dwSize) != ERROR_SUCCESS)
        {
            m_SoundOnOff = true;
        }
        dwSize = sizeof(int);
        if (RegQueryValueEx(hKey, L"MusicOnOff", 0, NULL, (LPBYTE)&m_MusicOnOff, &dwSize) != ERROR_SUCCESS)
        {
            m_MusicOnOff = false;
        }
        dwSize = sizeof(int);
        if (RegQueryValueEx(hKey, L"Resolution", 0, NULL, (LPBYTE)&m_Resolution, &dwSize) != ERROR_SUCCESS)
            m_Resolution = 1;

        if (0 == m_Resolution)
            m_Resolution = 1;

        dwSize = sizeof(int);
        if (RegQueryValueEx(hKey, L"ColorDepth", 0, NULL, (LPBYTE)&m_nColorDepth, &dwSize) != ERROR_SUCCESS)
        {
            m_nColorDepth = 0;
        }
        dwSize = sizeof(int);
        if (RegQueryValueEx(hKey, L"RememberMe", 0, NULL, (LPBYTE)&m_RememberMe, &dwSize) != ERROR_SUCCESS)
        {
            m_RememberMe = 0;
        }
        dwSize = sizeof(int);
        if (RegQueryValueEx(hKey, L"TextOut", 0, NULL, (LPBYTE)&g_iRenderTextType, &dwSize) != ERROR_SUCCESS)
        {
            g_iRenderTextType = 0;
        }

        g_iChatInputType = 1;

        dwSize = sizeof(int);
        if (RegQueryValueEx(hKey, L"WindowMode", 0, NULL, (LPBYTE)&g_bUseWindowMode, &dwSize) != ERROR_SUCCESS)
        {
            g_bUseWindowMode = FALSE;
        }

        dwSize = MAX_LANGUAGE_NAME_LENGTH;
        if (RegQueryValueEx(hKey, L"LangSelection", 0, NULL, (LPBYTE)g_aszMLSelection, &dwSize) != ERROR_SUCCESS)
        {
            wcscpy(g_aszMLSelection, L"Eng");
        }
        g_strSelectedML = g_aszMLSelection;
    }
    RegCloseKey(hKey);

    switch (m_Resolution)
    {
    case 0:
        WindowWidth = 640;
        WindowHeight = 480;
        break;
    case 1:
        WindowWidth = 800;
        WindowHeight = 600;
        break;
    case 2:
        WindowWidth = 1024;
        WindowHeight = 768;
        break;
    case 3:
        WindowWidth = 1280;
        WindowHeight = 1024;
        break;
    case 4:
        WindowWidth = 1600;
        WindowHeight = 1200;
        break;
    case 5:
        WindowWidth = 1864;
        WindowHeight = 1400;
        break;
    case 6:
        WindowWidth = 1600;
        WindowHeight = 900;
        break;
    case 7:
        WindowWidth = 1600;
        WindowHeight = 1280;
        break;
    case 8:
        WindowWidth = 1680;
        WindowHeight = 1050;
        break;
    case 9:
        WindowWidth = 1920;
        WindowHeight = 1080;
        break;
    case 10:
        WindowWidth = 2560;
        WindowHeight = 1440;
        break;
    default:
        WindowWidth = 640;
        WindowHeight = 480;
        break;
    }

    g_fScreenRate_x = (float)WindowWidth / 640;
    g_fScreenRate_y = (float)WindowHeight / 480;

    return TRUE;
}

BOOL Util_CheckOption(std::wstring lpszCommandLine, wchar_t cOption, std::wstring& lpszString)
{
    if (lpszCommandLine.empty()) {
        return FALSE;
    }

    // Create both lowercase and uppercase variants of the option character
    std::wstring cOptionLower = L"/";
    cOptionLower += towlower(static_cast<wint_t>(cOption));
    auto foundIndex = lpszCommandLine.find(cOptionLower);
    if (foundIndex == std::wstring::npos)
    {
        std::wstring cOptionUpper = L"/";
        cOptionUpper += towupper(static_cast<wint_t>(cOption));
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

BOOL UpdateFile(wchar_t* lpszOld, wchar_t* lpszNew)
{
    SetFileAttributes(lpszOld, FILE_ATTRIBUTE_NORMAL);
    SetFileAttributes(lpszNew, FILE_ATTRIBUTE_NORMAL);

    DWORD dwStartTickCount = ::GetTickCount();
    while (::GetTickCount() - dwStartTickCount < 5000) {
        if (CopyFile(lpszOld, lpszNew, FALSE))
        {	// 성공
            DeleteFile(lpszOld);
            return (TRUE);
        }
    }
    g_ErrorReport.Write(L"%s to %s CopyFile Error : %d\r\n", lpszNew, lpszOld, GetLastError());
    return (FALSE);
}

#include <tlhelp32.h>

BOOL KillExeProcess(wchar_t* lpszExe)
{
    HANDLE hProcessSnap = NULL;
    BOOL bRet = FALSE;
    PROCESSENTRY32 pe32 = { 0 };

    //  Take a snapshot of all processes in the system.

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
        return (FALSE);

    //  Fill in the size of the structure before using it.

    pe32.dwSize = sizeof(PROCESSENTRY32);

    //  Walk the snapshot of the processes, and for each process,
    //  display information.

    if (Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (wcsicmp(pe32.szExeFile, lpszExe) == 0)
            {
                HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);

                if (process)
                {
                    TerminateProcess(process, 0);
                }
            }
        } while (Process32Next(hProcessSnap, &pe32));
        bRet = TRUE;
    }
    else
        bRet = FALSE;    // could not walk the list of processes

    // Do not forget to clean up the snapshot object.

    CloseHandle(hProcessSnap);

    return bRet;
}

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
#ifdef ENABLE_FULLSCREEN
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
    if (g_bUseWindowMode == FALSE)
#endif	// USER_WINDOW_MODE
    {
        ChangeDisplaySettings(NULL, 0);
    }
#endif //ENABLE_FULLSCREEN
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

        currentUsage = max(0.0, min(100.0, currentUsage));

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
    g_MaxMessagePerCycle = (messages > 0) ? max(messages, custom_min) : messages;
}

MSG MainLoop()
{
    MSG msg;

    constexpr auto target_resolution = 1;
    auto precise = timeBeginPeriod(target_resolution);

    while (1)
    {
        int messageProcessed = 0;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return msg;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
            ++messageProcessed;

            if (g_MaxMessagePerCycle > 0 && messageProcessed >= g_MaxMessagePerCycle)
            {
                break;
            }
        }

        if (CheckRenderNextFrame())
        {
#if (defined WINDOWMODE)
            if (g_bUseWindowMode || g_bWndActive)
            {
                RenderScene(g_hDC);
            }
#ifndef FOR_WORK
            else if (g_bUseWindowMode == FALSE)
            {
                SetForegroundWindow(g_hWnd);
                SetFocus(g_hWnd);

                if (g_iInactiveWarning > 1)
                {
                    SetTimer(g_hWnd, WINDOWMINIMIZED_TIMER, 1 * 1000, NULL);
                    PostMessage(g_hWnd, WM_CLOSE, 0, 0);
                }
                else
                {
                    g_iInactiveWarning++;
                    g_bMinimizedEnabled = TRUE;
                    ShowWindow(g_hWnd, SW_MINIMIZE);
                    g_bMinimizedEnabled = FALSE;
                    ShowWindow(g_hWnd, SW_MAXIMIZE);
                }
            }
#endif//FOR_WORK
#else//WINDOWMODE
            if (g_bWndActive)
                RenderScene(g_hDC);
#endif	//WINDOWMODE(#else)
        }
        else
        {
            if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
            {
                WaitForNextActivity(precise == TIMERR_NOERROR);
            }
        }

    } // while( 1 )

    if (precise == TIMERR_NOERROR)
    {
        timeEndPeriod(target_resolution);
    }

    return msg;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int nCmdShow)
{
    leaf::AttachExceptionHandler(ExceptionCallback);

    wchar_t lpszExeVersion[256] = L"unknown";

    wchar_t* lpszCommandLine = GetCommandLine();
    wchar_t lpszFile[MAX_PATH];
    WORD wVersion[4] = { 0, };
    if (GetFileNameOfFilePath(lpszFile, lpszCommandLine))
    {
        if (GetFileVersion(lpszFile, wVersion))
        {
            swprintf(lpszExeVersion, L"%d.%02d", wVersion[0], wVersion[1]);
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
    g_ErrorReport.Write(L"Mu online %s (%s) executed. (%d.%d.%d.%d)\r\n", lpszExeVersion, L"Eng", wVersion[0], wVersion[1], wVersion[2], wVersion[3]);

    g_ConsoleDebug->Write(MCD_NORMAL, L"Mu Online (Version: %d.%d.%d.%d)", wVersion[0], wVersion[1], wVersion[2], wVersion[3]);

    g_ErrorReport.WriteCurrentTime();
    ER_SystemInfo si;
    ZeroMemory(&si, sizeof(ER_SystemInfo));
    GetSystemInfo(&si);
    g_ErrorReport.AddSeparator();
    g_ErrorReport.WriteSystemInfo(&si);
    g_ErrorReport.AddSeparator();

    // PKD_ADD_BINARY_PROTECTION
    VM_START
        WORD wPortNumber;
    if (GetConnectServerInfo(GetCommandLine(), g_lpszCmdURL, &wPortNumber))
    {
        szServerIpAddress = g_lpszCmdURL;
        g_ServerPort = wPortNumber;
    }
    VM_END

    g_ErrorReport.Write(L"> To read config.ini.\r\n");
    if (OpenInitFile() == FALSE)
    {
        g_ErrorReport.Write(L"config.ini read error\r\n");
        return false;
    }

    pMultiLanguage = new CMultiLanguage(g_strSelectedML);

    if (g_iChatInputType == 1)
        ShowCursor(FALSE);

    g_ErrorReport.Write(L"> Enum display settings.\r\n");
    DEVMODE DevMode;
    DEVMODE* pDevmodes;
    int nModes = 0;
    while (EnumDisplaySettings(NULL, nModes, &DevMode)) nModes++;
    pDevmodes = new DEVMODE[nModes + 1];
    nModes = 0;
    while (EnumDisplaySettings(NULL, nModes, &pDevmodes[nModes])) nModes++;

    DWORD dwBitsPerPel = 16;
    for (int n1 = 0; n1 < nModes; n1++)
    {
        if (pDevmodes[n1].dmBitsPerPel == 16 && m_nColorDepth == 0) {
            dwBitsPerPel = 16; break;
        }
        if (pDevmodes[n1].dmBitsPerPel == 24 && m_nColorDepth == 1) {
            dwBitsPerPel = 24; break;
        }
        if (pDevmodes[n1].dmBitsPerPel == 32 && m_nColorDepth == 1) {
            dwBitsPerPel = 32; break;
        }
    }

#ifdef ENABLE_FULLSCREEN
#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
    if (g_bUseWindowMode == FALSE)
#endif	// USER_WINDOW_MODE
    {
        for (int n2 = 0; n2 < nModes; n2++)
        {
            if (pDevmodes[n2].dmPelsWidth == WindowWidth && pDevmodes[n2].dmPelsHeight == WindowHeight && pDevmodes[n2].dmBitsPerPel == dwBitsPerPel)
            {
                g_ErrorReport.Write(L"> Change display setting %dx%d.\r\n", pDevmodes[n2].dmPelsWidth, pDevmodes[n2].dmPelsHeight);
                ChangeDisplaySettings(&pDevmodes[n2], 0);
                break;
            }
        }
    }
#endif //ENABLE_FULLSCREEN

    delete[] pDevmodes;

    g_ErrorReport.Write(L"> Screen size = %d x %d.\r\n", WindowWidth, WindowHeight);

    g_hInst = hInstance;
    g_hWnd = StartWindow(hInstance, nCmdShow);
    g_ErrorReport.Write(L"> Start window success.\r\n");

    if (!CreateOpenglWindow())
    {
        return FALSE;
    }

    g_ErrorReport.Write(L"> OpenGL init success.\r\n");
    g_ErrorReport.AddSeparator();
    //g_ErrorReport.WriteOpenGLInfo();
    g_ErrorReport.AddSeparator();
    g_ErrorReport.WriteSoundCardInfo();

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    //g_ErrorReport.WriteImeInfo( g_hWnd);
    g_ErrorReport.AddSeparator();

    InitVSync();
    if (IsVSyncAvailable())
    {
        EnableVSync();
        SetTargetFps(-1); // unlimited
    }

    FontHeight = static_cast<int>(std::ceil(12 + ((WindowHeight - 480) / 200.f)));

    int nFixFontHeight = WindowHeight <= 600 ? 14 : 15;
    int nFixFontSize;
    int iFontSize;

    iFontSize = FontHeight - 1;
    nFixFontSize = nFixFontHeight - 1;

    g_hFont = CreateFont(iFontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Tahoma");
    g_hFontBold = CreateFont(iFontSize, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Tahoma");
    g_hFontBig = CreateFont(iFontSize * 2, 0, 0, 0, FW_SEMIBOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Tahoma");
    g_hFixFont = CreateFont(nFixFontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_NATURAL_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Tahoma");

    setlocale(LC_ALL, "english");

    CInput::Instance().Create(g_hWnd, WindowWidth, WindowHeight);

    g_pNewUISystem->Create();

    if (m_MusicOnOff)
    {
        wzAudioCreate(g_hWnd);
        wzAudioOption(WZAOPT_STOPBEFOREPLAY, 1);
    }

    if (m_SoundOnOff)
    {
        InitDirectSound(g_hWnd);
        leaf::CRegKey regkey;
        regkey.SetKey(leaf::CRegKey::_HKEY_CURRENT_USER, L"SOFTWARE\\Webzen\\Mu\\Config");
        DWORD value;
        if (!regkey.ReadDword(L"VolumeLevel", value))
        {
            value = 5;	//. default setting
            regkey.WriteDword(L"VolumeLevel", value);
        }
        if (value < 0 || value >= 10)
            value = 5;

        g_pOption->SetVolumeLevel(int(value));
        SetEffectVolumeLevel(g_pOption->GetVolumeLevel());
    }

    SetTimer(g_hWnd, HACK_TIMER, 20 * 1000, NULL);
    SetTimer(g_hWnd, MUHELPER_TIMER, 250 /* ms */, MUHelper::CMuHelper::TimerProc);

    srand((unsigned)time(NULL));
    for (int i = 0; i < 100; i++)
        RandomTable[i] = rand() % 360;

    //memorydump[0]
    RendomMemoryDump = new BYTE[rand() % 100 + 1];

    GateAttribute = new GATE_ATTRIBUTE[MAX_GATES] { };
    SkillAttribute = new SKILL_ATTRIBUTE[MAX_SKILLS] { };

    //memorydump[1]
    ItemAttRibuteMemoryDump = new ITEM_ATTRIBUTE[MAX_ITEM + 1024] { };
    ItemAttribute = ((ITEM_ATTRIBUTE*)ItemAttRibuteMemoryDump) + rand() % 1024;

    //memorydump[2]
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

#ifdef MOVIE_DIRECTSHOW
    g_pMovieScene = new CMovieScene;
#endif // MOVIE_DIRECTSHOW

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
#if (defined WINDOWMODE)
    if (g_bUseWindowMode == FALSE)
    {
        int nOldVal;
        SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &nOldVal, 0);
        SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &g_iScreenSaverOldValue, 0);
        SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 300 * 60, NULL, 0);
    }
#else
#ifdef NDEBUG
#ifndef FOR_WORK
#ifdef ACTIVE_FOCUS_OUT
    if (g_bUseWindowMode == FALSE)
    {
#endif	// ACTIVE_FOCUS_OUT
        int nOldVal; // 값이 들어갈 필요가 없음
        SystemParametersInfo(SPI_SCREENSAVERRUNNING, 1, &nOldVal, 0);  // 단축키를 못쓰게 함
        SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, &g_iScreenSaverOldValue, 0);  // 스크린세이버 차단
        SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, 300 * 60, NULL, 0);  // 스크린세이버 차단
#ifdef ACTIVE_FOCUS_OUT
    }
#endif	// ACTIVE_FOCUS_OUT
#endif
#endif
#endif	//WINDOWMODE(#else)

#if defined PROTECT_SYSTEMKEY && defined NDEBUG
#ifndef FOR_WORK
    ProtectSysKey::AttachProtectSysKey(g_hInst, g_hWnd);
#endif // !FOR_WORK
#endif // PROTECT_SYSTEMKEY && NDEBUG

    std::thread cpuUsageRecorder(RecordCpuUsage);
    const MSG msg = MainLoop();

    DestroyWindow();

    return msg.wParam;
}