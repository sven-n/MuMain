///////////////////////////////////////////////////////////////////////////////
// LoginScene.cpp - Login scene implementation
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoginScene.h"
#include "SceneCore.h"
#include "../Camera/CameraUtility.h"
#include "../CameraMove.h"
#include "../DSPlaySound.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzObject.h"
#include "../ZzzCharacter.h"
#include "../ZzzLodTerrain.h"
#include "../ZzzInterface.h"
#include "../ZzzEffect.h"
#include "../GOBoid.h"
#include "../w_PetProcess.h"
#include "../MapManager.h"
#include "../UIMng.h"
#include "../Input.h"
#include "../WSclient.h"
#include "../Utilities/Log/muConsoleDebug.h"
#include "../ZzzInterface.h"
#include "../GlobalText.h"
#include "../ZzzCharacter.h"
#include "../UIControls.h"
#include "SceneCommon.h"
#include "../ZzzOpenData.h"

// External declarations
extern int DeleteGuildIndex;
extern float CameraAngle[3];
extern float CameraPosition[3];
extern float CameraFOV;
extern EGameScene SceneFlag;
extern int g_iChatInputType;
extern CUITextInputBox* g_pSinglePasswdInputBox;
extern float g_fMULogoAlpha;
extern wchar_t m_ID[MAX_ID_SIZE + 1];
extern CHARACTER* Hero;
extern double WorldTime;
extern HFONT g_hFont;
extern wchar_t m_ExeVersion[11];
extern HWND g_hWnd;

//=============================================================================
// LoginScene Camera State (local to this file)
//=============================================================================
namespace {

struct LoginCameraState {
    int walkCut = 0;
    int currentCount = -1;
    int currentNumber = 0;
    int currentWalkType = 0;
    vec3_t currentPosition = {0, 0, 0};
    vec3_t currentAngle = {0, 0, 0};
    float currentWalkDelta[6] = {0, 0, 0, 0, 0, 0};

    // Predefined camera animation paths (6 paths × 6 values each)
    static constexpr float WALK_PATHS[6][6] = {
        {0.f, -1000.f, 500.f, -80.f, 0.f, 0.f},
        {0.f, -1100.f, 500.f, -80.f, 0.f, 0.f},
        {0.f, -1100.f, 500.f, -80.f, 0.f, 0.f},
        {0.f, -1100.f, 500.f, -80.f, 0.f, 0.f},
        {0.f, -1100.f, 500.f, -80.f, 0.f, 0.f},
        {200.f, -800.f, 250.f, -87.f, 0.f, -10.f}
    };

    void Reset() {
        currentCount = -1;
        walkCut = 0;
        currentNumber = 0;
        currentWalkType = 0;
    }
};

// File-local camera state instance
LoginCameraState g_loginCamera;

}  // namespace

//=============================================================================
// Accessor functions for external use
//=============================================================================

int GetLoginCameraCount() {
    return g_loginCamera.currentCount;
}

int GetLoginCameraWalkCut() {
    return g_loginCamera.walkCut;
}

//=============================================================================
// LoginScene Implementation
//=============================================================================

void DeleteCharacter()
{
    if (SelectedHero < 0 || SelectedHero >= MAX_CHARACTERS_PER_ACCOUNT)
    {
        return;
    }

    int characterToDelete = SelectedHero;
    SelectedHero = -1;

    if (g_iChatInputType == 1)
    {
        g_pSinglePasswdInputBox->GetText(InputText[0]);
        g_pSinglePasswdInputBox->SetText(NULL);
        g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
    }

    CurrentProtocolState = REQUEST_DELETE_CHARACTER;
    SocketClient->ToGameServer()->SendDeleteCharacter(CharactersClient[characterToDelete].ID, InputText[0]);

    PlayBuffer(SOUND_MENU01);

    ClearInput();
    InputEnable = false;
}

void MoveCharacterCamera(vec3_t Origin, vec3_t Position, vec3_t Angle)
{
    vec3_t TransformPosition;
    CameraAngle[0] = 0.f;
    CameraAngle[1] = 0.f;
    CameraAngle[2] = Angle[2];
    float Matrix[3][4];
    AngleMatrix(CameraAngle, Matrix);
    VectorIRotate(Position, Matrix, TransformPosition);
    VectorAdd(Origin, TransformPosition, CameraPosition);
    CameraAngle[0] = Angle[0];
}

void MoveCamera()
{
    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        return;
    }

    if (g_loginCamera.currentCount == -1)
    {
        for (int i = 0; i < 3; i++)
        {
            g_loginCamera.currentPosition[i] = LoginCameraState::WALK_PATHS[0][i];
            g_loginCamera.currentAngle[i] = LoginCameraState::WALK_PATHS[0][i + 3];
        }
        g_loginCamera.currentNumber = 1;
        g_loginCamera.currentWalkType = 1;

        for (int i = 0; i < 3; i++)
        {
            g_loginCamera.currentWalkDelta[i] = (LoginCameraState::WALK_PATHS[g_loginCamera.currentNumber][i] - g_loginCamera.currentPosition[i]) / 128;
            g_loginCamera.currentWalkDelta[i + 3] = (LoginCameraState::WALK_PATHS[g_loginCamera.currentNumber][i + 3] - g_loginCamera.currentAngle[i]) / 128;
        }
    }
    g_loginCamera.currentCount++;
    if ((g_loginCamera.walkCut == 0 && g_loginCamera.currentCount >= 40) || (g_loginCamera.walkCut > 0 && g_loginCamera.currentCount >= 128))
    {
        g_loginCamera.currentCount = 0;
        if (g_loginCamera.walkCut == 0)
        {
            g_loginCamera.walkCut = 1;
        }
        else
        {
            if (SceneFlag == LOG_IN_SCENE)
            {
                g_loginCamera.currentNumber = rand() % 4 + 1;
                g_loginCamera.currentWalkType = rand() % 2;
            }
            else
            {
                g_loginCamera.currentNumber = 5;
                g_loginCamera.currentWalkType = 0;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            g_loginCamera.currentWalkDelta[i] = (LoginCameraState::WALK_PATHS[g_loginCamera.currentNumber][i] - g_loginCamera.currentPosition[i]) / 128;
            g_loginCamera.currentWalkDelta[i + 3] = (LoginCameraState::WALK_PATHS[g_loginCamera.currentNumber][i + 3] - g_loginCamera.currentAngle[i]) / 128;
        }
    }
    if (g_loginCamera.currentWalkType == 0)
    {
        for (int i = 0; i < 3; i++)
        {
            g_loginCamera.currentPosition[i] += (LoginCameraState::WALK_PATHS[g_loginCamera.currentNumber][i] - g_loginCamera.currentPosition[i]) / 6;
            g_loginCamera.currentAngle[i] += (LoginCameraState::WALK_PATHS[g_loginCamera.currentNumber][i + 3] - g_loginCamera.currentAngle[i]) / 6;
        }
    }
    else
    {
        for (int i = 0; i < 2; i++)
        {
            g_loginCamera.currentPosition[i] += g_loginCamera.currentWalkDelta[i];
        }
    }
    CameraFOV = 45.f;
    vec3_t Position;
    Vector(0.f, 0.f, 0.f, Position);
    MoveCharacterCamera(Position, g_loginCamera.currentPosition, g_loginCamera.currentAngle);
}

void CreateLogInScene()
{
    EnableMainRender = true;
    gMapManager.WorldActive = WD_73NEW_LOGIN_SCENE;

    gMapManager.LoadWorld(gMapManager.WorldActive);

    OpenLogoSceneData();

    CUIMng::Instance().CreateLoginScene();

    CurrentProtocolState = REQUEST_JOIN_SERVER;
    CreateSocket(szServerIpAddress, g_ServerPort);

    GuildInputEnable = false;
    TabInputEnable = false;
    GoldInputEnable = false;
    InputEnable = true;
    ClearInput();

    if (g_iChatInputType == 0)
    {
        wcscpy_s(InputText[0], 256, m_ID);
        InputLength[0] = wcslen(InputText[0]);
        InputTextMax[0] = MAX_ID_SIZE;
        if (InputLength[0] == 0)	InputIndex = 0;
        else InputIndex = 1;
    }
    InputNumber = 2;
    InputTextHide[1] = 1;

    CCameraMove::GetInstancePtr()->PlayCameraWalk(Hero->Object.Position, 1000);
    CCameraMove::GetInstancePtr()->SetTourMode(TRUE, FALSE, 1);

    MoveMainCamera();

    g_fMULogoAlpha = 0;

    ::PlayMp3(MUSIC_LOGIN_THEME);

    g_ErrorReport.Write(L"> Login Scene init success.\r\n");
}

void NewMoveLogInScene()
{
    if (!InitLogIn)
    {
        InitLogIn = true;
        CreateLogInScene();
    }

    if (!CUIMng::Instance().m_CreditWin.IsShow())
    {
        InitTerrainLight();
        MoveObjects();
        MoveMounts();
        MoveLeaves();
        MoveCharactersClient();
        MoveEffects();
        MoveJoints();
        MoveParticles();
        MoveBoids();
        ThePetProcess().UpdatePets();
        MoveCamera();
    }

    if (CInput::Instance().IsKeyDown(VK_ESCAPE))
    {
        CUIMng& rUIMng = CUIMng::Instance();
        if (!(rUIMng.m_MsgWin.IsShow() || rUIMng.m_LoginWin.IsShow()
            || rUIMng.m_SysMenuWin.IsShow() || rUIMng.m_OptionWin.IsShow()
            || rUIMng.m_CreditWin.IsShow()
            )
            && rUIMng.m_LoginMainWin.IsShow() && rUIMng.m_ServerSelWin.IsShow()
            && rUIMng.IsSysMenuWinShow())
        {
            ::PlayBuffer(SOUND_CLICK01);
            rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
        }
    }
    if (RECEIVE_LOG_IN_SUCCESS == CurrentProtocolState)
    {
        g_ErrorReport.Write(L"> Request Character list\r\n");

        CCameraMove::GetInstancePtr()->SetTourMode(FALSE);

        SceneFlag = CHARACTER_SCENE;
        CurrentProtocolState = REQUEST_CHARACTERS_LIST;
        SocketClient->ToGameServer()->SendRequestCharacterList(g_pMultiLanguage->GetLanguage());

        ReleaseLogoSceneData();

        ClearCharacters();
    }

    g_ConsoleDebug->UpdateMainScene();
}

bool NewRenderLogInScene(HDC hDC)
{
    if (!InitLogIn) return false;

    FogEnable = false;

    vec3_t pos;
    VectorCopy(CameraPosition, pos);
    if (CCameraMove::GetInstancePtr()->IsCameraMove())
    {
        VectorCopy(CameraPosition, pos);
    }

    MoveMainCamera();

    int Width, Height;

    glColor3f(1.f, 1.f, 1.f);

    Height = 480;
    Width = GetScreenWidth();
    glClearColor(0.f, 0.f, 0.f, 1.f);

    BeginOpengl(0, 25, 640, 430);
    CreateFrustrum((float)Width / (float)640, (float)Height / 480.f, pos);

    if (!CUIMng::Instance().m_CreditWin.IsShow())
    {
        CameraViewFar = 330.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();

        RenderTerrain(false);
        CameraViewFar = 7000.f;
        RenderCharactersClient();
        RenderMount();
        RenderObjects();
        RenderJoints();
        RenderEffects();
        CheckSprites();
        RenderLeaves();
        RenderBoids();
        RenderObjects_AfterCharacter();
        ThePetProcess().RenderPets();
    }

    BeginSprite();
    RenderSprites();
    RenderParticles();
    EndSprite();
    BeginBitmap();

    if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        g_fMULogoAlpha += 0.02f;
        if (g_fMULogoAlpha > 10.0f) g_fMULogoAlpha = 10.0f;

        EnableAlphaBlend();
        glColor4f(g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f, g_fMULogoAlpha - 0.3f);
        RenderBitmap(BITMAP_LOG_IN + 17, 320.0f - 128.0f * 0.8f, 25.0f, 256.0f * 0.8f, 128.0f * 0.8f);
        EnableAlphaTest();
        glColor4f(g_fMULogoAlpha, g_fMULogoAlpha, g_fMULogoAlpha, g_fMULogoAlpha);
        RenderBitmap(BITMAP_LOG_IN + 16, 320.0f - 128.0f * 0.8f, 25.0f, 256.0f * 0.8f, 128.0f * 0.8f);
    }

    SIZE Size;
    wchar_t Text[100];

    g_pRenderText->SetFont(g_hFont);

    InputTextWidth = 256;
    glColor3f(0.8f, 0.7f, 0.6f);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 128);

    wcscpy_s(Text, 100, GlobalText[454]);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);
    g_pRenderText->RenderText(335 - Size.cx * 640 / WindowWidth, 480 - Size.cy * 640 / WindowWidth - 1, Text);

    wcscpy_s(Text, 100, GlobalText[455]);

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);
    g_pRenderText->RenderText(335, 480 - Size.cy * 640 / WindowWidth - 1, Text);

    swprintf_s(Text, 100, GlobalText[456], m_ExeVersion);

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);
    g_pRenderText->RenderText(0, 480 - Size.cy * 640 / WindowWidth - 1, Text);

    RenderInfomation();

#ifdef ENABLE_EDIT
    RenderDebugWindow();
#endif

    EndBitmap();

    EndOpengl();

    return true;
}
