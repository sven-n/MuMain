///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzInterface.h"
#include "ZzzTexture.h"
#include "ZzzScene.h"

#ifdef _EDITOR
#include "MuEditor/Core/MuEditorCore.h"
#include "imgui.h"
#endif

#include "PhysicsManager.h"

#include "CSQuest.h"
#include "UIControls.h"
#include "UIMapName.h"	// rozy
#include "./Time/Timer.h"
#include "UIMng.h"
#include "LoadingScene.h"
#include "CDirection.h"

#include "NewUISystem.h"
#include <chrono>
#include <thread>

extern CUITextInputBox* g_pSingleTextInputBox;
extern CUITextInputBox* g_pSinglePasswdInputBox;
extern int g_iChatInputType;
extern BOOL g_bUseChatListBox;
extern DWORD g_dwMouseUseUIID;
extern DWORD g_dwActiveUIID;
extern DWORD g_dwKeyFocusUIID;
extern CUIMapName* g_pUIMapName;
extern bool HighLight;
extern CTimer* g_pTimer;
extern BOOL g_bGameServerConnected;

bool	g_bTimeCheck = false;
int 	g_iBackupTime = 0;

float	g_fMULogoAlpha = 0;

extern float g_fSpecialHeight;

short   g_shCameraLevel = 0;

int g_iLengthAuthorityCode = 20;

wchar_t* szServerIpAddress = L"127.127.127.127";
WORD g_ServerPort = 44406;

EGameScene  SceneFlag = WEBZEN_SCENE;

extern int g_iKeyPadEnable;

CPhysicsManager g_PhysicsManager;

extern wchar_t Mp3FileName[256];

#define MAX_LENGTH_CMB	( 38)

DWORD   g_dwWaitingStartTick;
int     g_iRequestCount;

int     g_iMessageTextStart = 0;
char    g_cMessageTextCurrNum = 0;
char    g_cMessageTextNum = 0;
int     g_iNumLineMessageBoxCustom;
wchar_t    g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
int     g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];

int     g_iCustomMessageBoxButton_Cancel[NUM_PAR_BUTTON_CMB];

int		g_iCancelSkillTarget = 0;

#define NUM_LINE_DA		( 1)
int g_iCurrentDialogScript = -1;
int g_iNumAnswer = 0;
wchar_t g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];

DWORD GenerateCheckSum2(BYTE* pbyBuffer, DWORD dwSize, WORD wKey);


bool EnableMainRender = false;
extern int HeroKey;

int DeleteGuildIndex = -1;

int  ErrorMessage = NULL;
int	 ErrorMessageNext = NULL;
extern bool g_bEnterPressed;

int    CameraWalkCut;
int    CurrentCameraCount = -1;
int    CurrentCameraWalkType = 0;
int    CurrentCameraNumber = 0;
vec3_t CurrentCameraPosition;
vec3_t CurrentCameraAngle;
float  CurrentCameraWalkDelta[6];
float  CameraWalk[] =
{
     0.f,-1000.f,500.f,-80.f,0.f, 0.f,
     0.f,-1100.f,500.f,-80.f,0.f, 0.f,
     0.f,-1100.f,500.f,-80.f,0.f, 0.f,
     0.f,-1100.f,500.f,-80.f,0.f, 0.f,
     0.f,-1100.f,500.f,-80.f,0.f, 0.f,
    200.f,-800.f,250.f,-87.f,0.f, -10.f,
};

bool MenuCancel = true;
bool InitLogIn = false;
bool InitLoading = false;
bool InitCharacterScene = false;
bool InitMainScene = false;
int  MenuY = 480;
int  MenuX = -200;
extern wchar_t LogInID[MAX_ID_SIZE + 1];
extern wchar_t m_ExeVersion[11];

BOOL Util_CheckOption(std::wstring lpszCommandLine, wchar_t cOption, std::wstring &lpszString);

extern DWORD g_dwBKConv;
extern DWORD g_dwBKSent;
extern BOOL g_bIMEBlock;

int SelectedHero = -1;
bool MoveMainCamera();

void LoadingScene(HDC hDC)
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"LoadingScene_Start");

    CUIMng& rUIMng = CUIMng::Instance();
    if (!InitLoading)
    {
        LoadingWorld = 9999999;

        InitLoading = true;

        LoadBitmap(L"Interface\\LSBg01.JPG", BITMAP_TITLE, GL_LINEAR);
        LoadBitmap(L"Interface\\LSBg02.JPG", BITMAP_TITLE + 1, GL_LINEAR);
        LoadBitmap(L"Interface\\LSBg03.JPG", BITMAP_TITLE + 2, GL_LINEAR);
        LoadBitmap(L"Interface\\LSBg04.JPG", BITMAP_TITLE + 3, GL_LINEAR);

        ::StopMp3(MUSIC_LOGIN_THEME);

        rUIMng.m_pLoadingScene = new CLoadingScene;
        rUIMng.m_pLoadingScene->Create();
    }

    FogEnable = false;
    ::BeginOpengl();
    ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ::BeginBitmap();

    rUIMng.m_pLoadingScene->Render();

    ::EndBitmap();
    ::EndOpengl();
    ::glFlush();
#ifdef _EDITOR
    // Always render ImGui (shows "Open Editor" button when closed, or full UI when open)
    g_MuEditorCore.Render();

    // Render game cursor on top of ImGui if not hovering UI
    extern bool g_bRenderGameCursor;
    if (g_bRenderGameCursor)
    {
        BeginBitmap();
        RenderCursor();
        EndBitmap();
    }
#endif
    ::SwapBuffers(hDC);

    SAFE_DELETE(rUIMng.m_pLoadingScene);

    SceneFlag = MAIN_SCENE;
    for (int i = 0; i < 4; ++i)
        ::DeleteBitmap(BITMAP_TITLE + i);

    ::ClearInput();

    g_ConsoleDebug->Write(MCD_NORMAL, L"LoadingScene_End");
}

float CameraDistanceTarget = 1000.f;
float CameraDistance = CameraDistanceTarget;
float	Camera3DFov = 0.f;
bool	Camera3DRoll = false;

extern int WaterTextureNumber;

int TestTime = 0;
extern int  GrabScreen;

void MoveCharacter(CHARACTER* c, OBJECT* o);

double target_fps = 60;
double ms_per_frame = 1000.0 / target_fps;

double last_render_tick_count = 0;
double current_tick_count = 0;
double last_water_change = 0;

float g_Luminosity;

extern int g_iNoMouseTime;
extern GLvoid KillGLWindow(GLvoid);

void WaitForNextActivity(bool usePreciseSleep)
{
    // We only sleep when we have enough time to sleep and have some additional rest time.
    const auto current_frame_time_ms = current_tick_count - last_render_tick_count;
    const auto current_ms_per_frame = ms_per_frame;
    if (current_ms_per_frame > 0 && current_frame_time_ms > 0 && current_frame_time_ms < current_ms_per_frame)
    {
        const auto sleep_threshold_ms = usePreciseSleep? 4.0 : 16.0;
        const auto sleep_duration_offset_ms = usePreciseSleep? 1.0 : 4.0;
        const auto max_sleep_ms = 10.0;
        const auto rest_ms = current_ms_per_frame - current_frame_time_ms;

        if (rest_ms - sleep_duration_offset_ms > sleep_threshold_ms)
        {
            const auto sleep_ms = std::min<double>(rest_ms - sleep_duration_offset_ms, max_sleep_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long>(sleep_ms)));
        }
        else
        {
            std::this_thread::yield();
        }
    }
    else
    {
        std::this_thread::yield();
    }
}

bool CheckRenderNextFrame()
{
    current_tick_count = g_pTimer->GetTimeElapsed();
    const auto current_frame_time_ms = current_tick_count - last_render_tick_count;

    if (current_frame_time_ms >= ms_per_frame)
    {
        return true;
    }

    return false;
}