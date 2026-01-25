///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzInterface.h"
#include "ZzzTexture.h"
#include "SceneCore.h"

#ifdef _EDITOR
#include "../MuEditor/Core/MuEditorCore.h"
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

#include "Camera/CameraUtility.h"
#include "Scenes/SceneManager.h"

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

const wchar_t* szServerIpAddress = L"127.127.127.127";
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


extern int HeroKey;

int DeleteGuildIndex = -1;

int  ErrorMessage = NULL;
extern bool g_bEnterPressed;

extern wchar_t LogInID[MAX_ID_SIZE + 1];
extern wchar_t m_ExeVersion[11];

BOOL Util_CheckOption(std::wstring lpszCommandLine, wchar_t cOption, std::wstring &lpszString);

extern DWORD g_dwBKConv;
extern DWORD g_dwBKSent;
extern BOOL g_bIMEBlock;

bool MoveMainCamera();

// Legacy global variables (kept for backward compatibility)
float CameraDistanceTarget = 1000.f;
float CameraDistance = 1000.f;
float Camera3DFov = 0.f;
bool Camera3DRoll = false;

// Camera state structure instance (uses legacy variables via references)
CameraState g_CameraState;

extern int WaterTextureNumber;

int TestTime = 0;
extern int  GrabScreen;

void MoveCharacter(CHARACTER* c, OBJECT* o);

float g_Luminosity;

extern int g_iNoMouseTime;
extern GLvoid KillGLWindow(GLvoid);

void WaitForNextActivity(bool usePreciseSleep)
{
    // We only sleep when we have enough time to sleep and have some additional rest time.
    const auto current_frame_time_ms = g_frameTiming.GetCurrentFrameTime();
    const auto current_ms_per_frame = g_frameTiming.GetMsPerFrame();
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
    g_frameTiming.UpdateCurrentTime(g_pTimer->GetTimeElapsed());
    return g_frameTiming.ShouldRenderNextFrame();
}