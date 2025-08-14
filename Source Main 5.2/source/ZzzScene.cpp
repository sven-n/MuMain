///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIManager.h"
#include "GuildCache.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "ZzzEffect.h"
#include "ZzzAI.h"
#include "DSPlaySound.h"

#include "SMD.h"
#include "Local.h"
#include "MatchEvent.h"
#include "PhysicsManager.h"

#include "CSQuest.h"
#include "PersonalShopTitleImp.h"
#include "uicontrols.h"
#include "GOBoid.h"
#include "CSItemOption.h"
#include "npcBreeder.h"
#include "CSPetSystem.h"
#include "GIPetManager.h"
#include "CComGem.h"
#include "UIMapName.h"	// rozy
#include "./Time/Timer.h"
#include "Input.h"
#include "UIMng.h"
#include "LoadingScene.h"
#include "CDirection.h"
#include "Event.h"
#include "MixMgr.h"

#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "PartyManager.h"
#include "w_CursedTemple.h"
#include "CameraMove.h"
#include "w_MapHeaders.h"
#include "w_PetProcess.h"
#include "PortalMgr.h"
#include "ServerListManager.h"
#include "MapManager.h"
#include <chrono>
#include <thread>

#include "CharacterManager.h"

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


bool CheckAbuseFilter(wchar_t* Text, bool bCheckSlash)
{
    if (bCheckSlash == true)
    {
        if (Text[0] == '/')
        {
            return false;
        }
    }

    int icntText = 0;
    wchar_t TmpText[2048];
    for (int i = 0; i < wcslen(Text); ++i)
    {
        if (Text[i] != 32)
        {
            TmpText[icntText] = Text[i];
            icntText++;
        }
    }
    TmpText[icntText] = 0;

    for (int i = 0; i < AbuseFilterNumber; i++)
    {
        if (FindText(TmpText, AbuseFilter[i]))
        {
            return true;
        }
    }
    return false;
}

bool CheckAbuseNameFilter(wchar_t* Text)
{
    int icntText = 0;
    wchar_t TmpText[256];
    for (int i = 0; i < wcslen(Text); ++i)
    {
        if (Text[i] != 32)
        {
            TmpText[icntText] = Text[i];
            icntText++;
        }
    }
    TmpText[icntText] = 0;

    for (int i = 0; i < AbuseNameFilterNumber; i++)
    {
        if (FindText(TmpText, AbuseNameFilter[i]))
        {
            return true;
        }
    }
    return false;
}

bool CheckName()
{
    if (CheckAbuseNameFilter(InputText[0]) || CheckAbuseFilter(InputText[0]) ||
        FindText(InputText[0], L" ") || FindText(InputText[0], L"　") ||
        FindText(InputText[0], L".") || FindText(InputText[0], L"·") || FindText(InputText[0], L"∼") ||
        FindText(InputText[0], L"Webzen") || FindText(InputText[0], L"WebZen") || FindText(InputText[0], L"webzen") || FindText(InputText[0], L"WEBZEN") ||
        FindText(InputText[0], GlobalText[457]) || FindText(InputText[0], GlobalText[458]))
        return true;
    return false;
}

bool EnableMainRender = false;
extern int HeroKey;

void WebzenScene(HDC hDC)
{
    CUIMng& rUIMng = CUIMng::Instance();

    OpenFont();
    ClearInput();

    LoadBitmap(L"Interface\\New_lo_back_01.jpg", BITMAP_TITLE, GL_LINEAR);
    LoadBitmap(L"Interface\\New_lo_back_02.jpg", BITMAP_TITLE + 1, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_121518.tga", BITMAP_TITLE + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_lo.jpg", BITMAP_TITLE + 5, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Interface\\lo_back_s5_03.jpg", BITMAP_TITLE + 6, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_back_s5_04.jpg", BITMAP_TITLE + 7, GL_LINEAR);
    if (rand() % 100 <= 70)
    {
        LoadBitmap(L"Interface\\lo_back_im01.jpg", BITMAP_TITLE + 8, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im02.jpg", BITMAP_TITLE + 9, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im03.jpg", BITMAP_TITLE + 10, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im04.jpg", BITMAP_TITLE + 11, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im05.jpg", BITMAP_TITLE + 12, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im06.jpg", BITMAP_TITLE + 13, GL_LINEAR);
    }
    else
    {
        LoadBitmap(L"Interface\\lo_back_s5_im01.jpg", BITMAP_TITLE + 8, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im02.jpg", BITMAP_TITLE + 9, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im03.jpg", BITMAP_TITLE + 10, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im04.jpg", BITMAP_TITLE + 11, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im05.jpg", BITMAP_TITLE + 12, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im06.jpg", BITMAP_TITLE + 13, GL_LINEAR);
    }

    rUIMng.CreateTitleSceneUI();

    FogEnable = false;

    ::EnableAlphaTest();
    OpenBasicData(hDC);

    g_pNewUISystem->LoadMainSceneInterface();

    CUIMng::Instance().RenderTitleSceneUI(hDC, 11, 11);

    rUIMng.ReleaseTitleSceneUI();
    DeleteBitmap(BITMAP_TITLE);
    DeleteBitmap(BITMAP_TITLE + 1);
    DeleteBitmap(BITMAP_TITLE + 3);
    DeleteBitmap(BITMAP_TITLE + 5);

    for (int i = 6; i < 14; ++i)
        DeleteBitmap(BITMAP_TITLE + i);

    g_ErrorReport.Write(L"> Loading ok.\r\n");

    SceneFlag = LOG_IN_SCENE;	//
}

int DeleteGuildIndex = -1;

void DeleteCharacter()
{
    SelectedHero = -1;
    if (g_iChatInputType == 1)
    {
        g_pSinglePasswdInputBox->GetText(InputText[0]);
        g_pSinglePasswdInputBox->SetText(NULL);
        g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
    }

    CurrentProtocolState = REQUEST_DELETE_CHARACTER;
    SocketClient->ToGameServer()->SendDeleteCharacter(CharactersClient[SelectedHero].ID, InputText[0]);

    PlayBuffer(SOUND_MENU01);

    ClearInput();
    InputEnable = false;
}
int  ErrorMessage = NULL;
int	 ErrorMessageNext = NULL;
extern bool g_bEnterPressed;

bool IsEnterPressed() {
    return g_bEnterPressed;
}

void SetEnterPressed(bool enterpressed) {
    g_bEnterPressed = enterpressed;
}

BOOL CheckOptionMouseClick(int iOptionPos_y, BOOL bPlayClickSound = TRUE)
{
    if (CheckMouseIn((640 - 120) / 2, 30 + iOptionPos_y, 120, 22) && MouseLButtonPush)
    {
        MouseLButtonPush = false;
        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;
        if (bPlayClickSound == TRUE) PlayBuffer(SOUND_CLICK01);
        return TRUE;
    }
    return FALSE;
}

int SeparateTextIntoLines(const wchar_t* lpszText, wchar_t* lpszSeparated, int iMaxLine, int iLineSize)
{
    int iLine = 0;
    const wchar_t* lpLineStart = lpszText;
    wchar_t* lpDst = lpszSeparated;
    const wchar_t* lpSpace = NULL;
    int iMbclen = 0;
    for (const wchar_t* lpSeek = lpszText; *lpSeek; lpSeek += iMbclen, lpDst += iMbclen)
    {
        iMbclen = _tclen(lpSeek);
        if (iMbclen + (lpSeek - lpLineStart) >= iLineSize)
        {
            if (lpSpace && (lpSeek - lpSpace) < min(10, iLineSize / 2))
            {
                lpDst -= lpSeek - lpSpace - 1;
                lpSeek = lpSpace + 1;
            }

            lpLineStart = lpSeek;
            *lpDst = L'\0';
            if (iLine >= iMaxLine - 1)
            {
                break;
            }
            ++iLine;
            lpDst = lpszSeparated + iLine * iLineSize;
            lpSpace = NULL;
        }

        memcpy(lpDst, lpSeek, iMbclen);
        if (*lpSeek == L' ')
        {
            lpSpace = lpSeek;
        }
    }
    *lpDst = L'\0';

    return (iLine + 1);
}

void SetEffectVolumeLevel(int level)
{
    if (level > 9)
        level = 9;
    if (level < 0)
        level = 0;

    if (level == 0)
    {
        SetMasterVolume(-10000);
    }
    else
    {
        long vol = -2000 * log10(10.f / float(level));
        SetMasterVolume(vol);
    }
}

void SetViewPortLevel(int Wheel)
{
    if ((HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128))
    {
        if (Wheel > 0)
            g_shCameraLevel--;
        else if (Wheel < 0)
            g_shCameraLevel++;

        MouseWheel = 0;

        if (g_shCameraLevel > 4)
            g_shCameraLevel = 4;
        if (g_shCameraLevel < 0)
            g_shCameraLevel = 0;
    }
}

void RenderInfomation3D()
{
    bool Success = false;

    if (((ErrorMessage == MESSAGE_TRADE_CHECK || ErrorMessage == MESSAGE_CHECK) && AskYesOrNo == 1)
        || ErrorMessage == MESSAGE_USE_STATE
        || ErrorMessage == MESSAGE_USE_STATE2)
    {
        Success = true;
    }

    if (ErrorMessage == MESSAGE_TRADE_CHECK && AskYesOrNo == 5)
    {
        Success = true;
    }
    if (ErrorMessage == MESSAGE_PERSONALSHOP_WARNING)
    {
        Success = true;
    }

    if (Success)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glViewport2(0, 0, WindowWidth, WindowHeight);
        gluPerspective2(1.f, (float)(WindowWidth) / (float)(WindowHeight), CameraViewNear, CameraViewFar);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        GetOpenGLMatrix(CameraMatrix);
        EnableDepthTest();
        EnableDepthMask();

        float Width, Height;
        float x = (640 - 150) / 2;
        float y;
        if (ErrorMessage == MESSAGE_TRADE_CHECK)
        {
            y = 60 + 55;
        }
        else
        {
            y = 60 + 55;
        }

        Width = 40.f; Height = 60.f;
        int iRenderType = ErrorMessage;
        if (AskYesOrNo == 5)
            iRenderType = MESSAGE_USE_STATE;
        switch (iRenderType)
        {
        case MESSAGE_USE_STATE:
        case MESSAGE_USE_STATE2:
        case MESSAGE_PERSONALSHOP_WARNING:
            RenderItem3D(x, y, Width, Height, TargetItem.Type, TargetItem.Level, TargetItem.ExcellentFlags, TargetItem.AncientDiscriminator, true);
            break;

        default:
            RenderItem3D(x, y, Width, Height, PickItem.Type, PickItem.Level, PickItem.ExcellentFlags, PickItem.AncientDiscriminator, true);
            break;
        }

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        UpdateMousePositionn();
    }
}

void RenderInfomation()
{
    RenderNotices();

    CUIMng::Instance().Render();

    if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
    {
        RenderCursor();
    }

    RenderInfomation3D();
}

BOOL ShowCheckBox(int num, int index, int message)
{
    if (message == MESSAGE_USE_STATE || message == MESSAGE_USE_STATE2)
    {
        wchar_t Name[50] = { 0, };
        if (TargetItem.Type == ITEM_FRUITS)
        {
            switch (TargetItem.Level)
            {
            case 0:wprintf(Name, L"%s", GlobalText[168]); break;
            case 1:wprintf(Name, L"%s", GlobalText[169]); break;
            case 2:wprintf(Name, L"%s", GlobalText[167]); break;
            case 3:wprintf(Name, L"%s", GlobalText[166]); break;
            case 4:wprintf(Name, L"%s", GlobalText[1900]); break;
            }
        }

        if (message == MESSAGE_USE_STATE2)
            swprintf(g_lpszMessageBoxCustom[0], L"( %s%s )", Name, GlobalText[1901]);
        else
            swprintf(g_lpszMessageBoxCustom[0], L"( %s )", Name);

        num++;
        for (int i = 1; i < num; ++i)
        {
            swprintf(g_lpszMessageBoxCustom[i], GlobalText[index]);
        }
        g_iNumLineMessageBoxCustom = num;
    }
    else if (message == MESSAGE_PERSONALSHOP_WARNING)
    {
        wchar_t szGold[256];
        ConvertGold(InputGold, szGold);
        swprintf(g_lpszMessageBoxCustom[0], GlobalText[index], szGold);

        for (int i = 1; i < num; ++i)
        {
            swprintf(g_lpszMessageBoxCustom[i], GlobalText[index + i]);
        }
        g_iNumLineMessageBoxCustom = num;
    }
    else if (message == MESSAGE_CHAOS_CASTLE_CHECK)
    {
        g_iNumLineMessageBoxCustom = 0;
        for (int i = 0; i < num; ++i)
        {
            g_iNumLineMessageBoxCustom += SeparateTextIntoLines(GlobalText[index + i], g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], NUM_LINE_CMB, MAX_LENGTH_CMB);
        }
    }
    else if (message == MESSAGE_GEM_INTEGRATION3)
    {
        wchar_t tBuf[MAX_GLOBAL_TEXT_STRING];
        wchar_t tLines[2][30];
        for (int t = 0; t < 2; ++t) memset(tLines[t], 0, 20);
        g_iNumLineMessageBoxCustom = 0;
        if (COMGEM::isComMode())
        {
            if (COMGEM::m_cGemType == 0) swprintf(tBuf, GlobalText[1809], GlobalText[1806], COMGEM::m_cCount);
            else swprintf(tBuf, GlobalText[1809], GlobalText[1807], COMGEM::m_cCount);

            g_iNumLineMessageBoxCustom += SeparateTextIntoLines(tBuf,
                tLines[g_iNumLineMessageBoxCustom], 2, 30);

            for (int t = 0; t < 2; ++t)
                wcscpy(g_lpszMessageBoxCustom[t], tLines[t]);

            swprintf(g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], GlobalText[1810], COMGEM::m_iValue);
            ++g_iNumLineMessageBoxCustom;
        }
        else
        {
            int t_GemLevel = COMGEM::GetUnMixGemLevel() + 1;
            if (COMGEM::m_cGemType == 0) swprintf(tBuf, GlobalText[1813], GlobalText[1806], t_GemLevel);
            else swprintf(tBuf, GlobalText[1813], GlobalText[1807], t_GemLevel);

            g_iNumLineMessageBoxCustom += SeparateTextIntoLines(tBuf,
                tLines[g_iNumLineMessageBoxCustom], 2, 30);

            for (int t = 0; t < 2; ++t)
                wcscpy(g_lpszMessageBoxCustom[t], tLines[t]);

            swprintf(g_lpszMessageBoxCustom[g_iNumLineMessageBoxCustom], GlobalText[1814], COMGEM::m_iValue);
            ++g_iNumLineMessageBoxCustom;
        }
    }
    else if (message == MESSAGE_CANCEL_SKILL)
    {
        wchar_t tBuf[MAX_GLOBAL_TEXT_STRING];
        swprintf(tBuf, L"%s%s", SkillAttribute[index].Name, GlobalText[2046]);
        g_iNumLineMessageBoxCustom = SeparateTextIntoLines(tBuf, g_lpszMessageBoxCustom[0], 2, MAX_LENGTH_CMB);
        g_iCancelSkillTarget = index;
    }
    else
    {
        for (int i = 0; i < num; ++i)
        {
            wcscpy(g_lpszMessageBoxCustom[i], GlobalText[index + i]);
        }

        g_iNumLineMessageBoxCustom = num;
    }

    ZeroMemory(g_iCustomMessageBoxButton, NUM_BUTTON_CMB * NUM_PAR_BUTTON_CMB * sizeof(int));

    int iOkButton[5] = { 1,  21, 90, 70, 21 };
    int iCancelButton[5] = { 3, 120, 90, 70, 21 };

    if (message == MESSAGE_USE_STATE2)
    {
        iOkButton[1] = 22;
        iOkButton[2] = 92;	// y
        iOkButton[3] = 49;
        iOkButton[4] = 16;

        iCancelButton[1] = 82;
        iCancelButton[2] = 92;	// y
        iCancelButton[3] = 49;
        iCancelButton[4] = 16;

        g_iCustomMessageBoxButton_Cancel[0] = 5;
        g_iCustomMessageBoxButton_Cancel[1] = 142;	// x
        g_iCustomMessageBoxButton_Cancel[2] = 92;	// y
        g_iCustomMessageBoxButton_Cancel[3] = 49;	// width
        g_iCustomMessageBoxButton_Cancel[4] = 16;	// height
    }

    if (message == MESSAGE_CHAOS_CASTLE_CHECK)
    {
        iOkButton[2] = 120;
        iCancelButton[2] = 120;
    }

    memcpy(g_iCustomMessageBoxButton[0], iOkButton, 5 * sizeof(int));
    memcpy(g_iCustomMessageBoxButton[1], iCancelButton, 5 * sizeof(int));

    return true;
}

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

    if (CurrentCameraCount == -1)
    {
        for (int i = 0; i < 3; i++)
        {
            CurrentCameraPosition[i] = CameraWalk[i];
            CurrentCameraAngle[i] = CameraWalk[i + 3];
        }
        CurrentCameraNumber = 1;
        CurrentCameraWalkType = 1;

        for (int i = 0; i < 3; i++)
        {
            CurrentCameraWalkDelta[i] = (CameraWalk[CurrentCameraNumber * 6 + i] - CurrentCameraPosition[i]) / 128;
            CurrentCameraWalkDelta[i + 3] = (CameraWalk[CurrentCameraNumber * 6 + i + 3] - CurrentCameraAngle[i]) / 128;
        }
    }
    CurrentCameraCount++;
    if ((CameraWalkCut == 0 && CurrentCameraCount >= 40) || (CameraWalkCut > 0 && CurrentCameraCount >= 128))
    {
        CurrentCameraCount = 0;
        if (CameraWalkCut == 0)
        {
            CameraWalkCut = 1;
        }
        else
        {
            if (SceneFlag == LOG_IN_SCENE)
            {
                CurrentCameraNumber = rand() % 4 + 1;
                CurrentCameraWalkType = rand() % 2;
            }
            else
            {
                CurrentCameraNumber = 5;
                CurrentCameraWalkType = 0;
            }
        }
        for (int i = 0; i < 3; i++)
        {
            CurrentCameraWalkDelta[i] = (CameraWalk[CurrentCameraNumber * 6 + i] - CurrentCameraPosition[i]) / 128;
            CurrentCameraWalkDelta[i + 3] = (CameraWalk[CurrentCameraNumber * 6 + i + 3] - CurrentCameraAngle[i]) / 128;
        }
    }
    if (CurrentCameraWalkType == 0)
    {
        for (int i = 0; i < 3; i++)
        {
            CurrentCameraPosition[i] += (CameraWalk[CurrentCameraNumber * 6 + i] - CurrentCameraPosition[i]) / 6;
            CurrentCameraAngle[i] += (CameraWalk[CurrentCameraNumber * 6 + i + 3] - CurrentCameraAngle[i]) / 6;
        }
    }
    else
    {
        for (int i = 0; i < 2; i++)
        {
            CurrentCameraPosition[i] += CurrentCameraWalkDelta[i];
        }
    }
    CameraFOV = 45.f;
    vec3_t Position;
    Vector(0.f, 0.f, 0.f, Position);
    MoveCharacterCamera(Position, CurrentCameraPosition, CurrentCameraAngle);
}

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

void StartGame()
{
    {
        if (CTLCODE_01BLOCKCHAR & CharactersClient[SelectedHero].CtlCode)
            CUIMng::Instance().PopUpMsgWin(MESSAGE_BLOCKED_CHARACTER);
        else
        {
            CharacterAttribute->Level = CharactersClient[SelectedHero].Level;
            CharacterAttribute->Class = CharactersClient[SelectedHero].Class;
            CharacterAttribute->Skin = CharactersClient[SelectedHero].Skin;
            ::wcscpy(CharacterAttribute->Name, CharactersClient[SelectedHero].ID);

            ::ReleaseCharacterSceneData();
            InitLoading = false;
            SceneFlag = LOADING_SCENE;
        }
    }
}

void CreateCharacterScene()
{
    g_pNewUIMng->ResetActiveUIObj();

    EnableMainRender = true;
    MouseOnWindow = false;
    ErrorMessage = NULL;

    gMapManager.WorldActive = WD_74NEW_CHARACTER_SCENE;

    gMapManager.LoadWorld(gMapManager.WorldActive);
    OpenCharacterSceneData();

    CreateCharacterPointer(&CharacterView, MODEL_FACE + 1, 0, 0);
    CharacterView.Class = CLASS_KNIGHT;
    CharacterView.SkinIndex = gCharacterManager.GetSkinModelIndex(CLASS_KNIGHT);
    CharacterView.Object.Kind = 0;

    SelectedHero = -1;
    CUIMng::Instance().CreateCharacterScene();

    ClearInventory();
    CharacterAttribute->SkillNumber = 0;

    for (int i = 0; i < MAX_MAGIC; i++)
        CharacterAttribute->Skill[i] = AT_SKILL_UNDEFINED;

    for (int i = EQUIPMENT_WEAPON_RIGHT; i < EQUIPMENT_HELPER; i++)
        CharacterMachine->Equipment[i].Level = 0;

    g_pNewUISystem->HideAll();

    g_iKeyPadEnable = 0;
    GuildInputEnable = false;
    TabInputEnable = false;
    GoldInputEnable = false;
    InputEnable = true;
    ClearInput();
    InputIndex = 0;
    InputTextWidth = 90;
    InputNumber = 1;

    for (int i = 0; i < MAX_WHISPER; i++)
    {
        g_pChatListBox->AddText(L"", L"", SEASON3B::TYPE_WHISPER_MESSAGE);
    }

    HIMC hIMC = ImmGetContext(g_hWnd);
    DWORD Conversion, Sentence;

    Conversion = IME_CMODE_NATIVE;
    Sentence = IME_SMODE_NONE;

    g_bIMEBlock = FALSE;
    RestoreIMEStatus();
    ImmSetConversionStatus(hIMC, Conversion, Sentence);
    ImmGetConversionStatus(hIMC, &g_dwBKConv, &g_dwBKSent);
    SaveIMEStatus();
    ImmReleaseContext(g_hWnd, hIMC);
    g_bIMEBlock = TRUE;

    g_ErrorReport.Write(L"> Character scene init success.\r\n");
}

void NewMoveCharacterScene()
{
    if (CurrentProtocolState < RECEIVE_CHARACTERS_LIST)
    {
        return;
    }

    if (!InitCharacterScene)
    {
        InitCharacterScene = true;
        CreateCharacterScene();
    }
    InitTerrainLight();
    MoveObjects();
    MoveMounts();
    MoveCharactersClient();
    MoveCharacterClient(&CharacterView);

    MoveEffects();
    MoveJoints();
    MoveParticles();
    MoveBoids();

    ThePetProcess().UpdatePets();

    MoveCamera();

#if defined _DEBUG || defined FOR_WORK
    std::wstring lpszTemp = { 0 };
    if (::Util_CheckOption(::GetCommandLine(), L'c', lpszTemp))
    {
        SelectedHero = ::_wtoi(lpszTemp.c_str());
        ::StartGame();
    }
#endif

    CInput& rInput = CInput::Instance();
    CUIMng& rUIMng = CUIMng::Instance();

    if (rInput.IsKeyDown(VK_RETURN))
    {
        if (!(rUIMng.m_MsgWin.IsShow() || rUIMng.m_CharMakeWin.IsShow()
            || rUIMng.m_SysMenuWin.IsShow() || rUIMng.m_OptionWin.IsShow())
            && SelectedHero > -1 && SelectedHero < 5)
        {
            ::PlayBuffer(SOUND_CLICK01);

            if (SelectedCharacter >= 0)
                SelectedHero = SelectedCharacter;

            ::StartGame();
        }
    }
    else if (rInput.IsKeyDown(VK_ESCAPE))
    {
        if (!(rUIMng.m_MsgWin.IsShow() || rUIMng.m_CharMakeWin.IsShow()
            || rUIMng.m_SysMenuWin.IsShow() || rUIMng.m_OptionWin.IsShow()
            )
            && rUIMng.IsSysMenuWinShow())
        {
            ::PlayBuffer(SOUND_CLICK01);
            rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
        }
    }

    if (rUIMng.IsCursorOnUI())
    {
        return;
    }

    if (rInput.IsLBtnDbl() && rUIMng.m_CharSelMainWin.IsShow())
    {
        if (SelectedCharacter < 0 || SelectedCharacter > 4)
        {
            return;
        }

        SelectedHero = SelectedCharacter;
        ::StartGame();
    }
    else if (rInput.IsLBtnDn())
    {
        if (SelectedCharacter < 0 || SelectedCharacter > 4)
            SelectedHero = -1;
        else
            SelectedHero = SelectedCharacter;
        rUIMng.m_CharSelMainWin.UpdateDisplay();
    }

    g_ConsoleDebug->UpdateMainScene();
}

bool NewRenderCharacterScene(HDC hDC)
{
    if (!InitCharacterScene)
    {
        return false;
    }
    if (CurrentProtocolState < RECEIVE_CHARACTERS_LIST)
    {
        return false;
    }

    FogEnable = false;
    vec3_t pos;
    Vector(9758.0f, 18913.0f, 675.0f, pos);

    MoveMainCamera();

    int Width, Height;

    glColor3f(1.f, 1.f, 1.f);
    Height = 480;
    Width = GetScreenWidth();

    glClearColor(0.f, 0.f, 0.f, 1.f);
    BeginOpengl(0, 25, 640, 430);

    CreateFrustrum((float)Width / (float)640, (float)Height / 480.f, pos);

    OBJECT* o = &CharactersClient[SelectedHero].Object;

    CreateScreenVector(MouseX, MouseY, MouseTarget);
    for (int i = 0; i < 5; i++)
    {
        CharactersClient[i].Object.Position[2] = 163.0f;
        Vector(0.0f, 0.0f, 0.0f, CharactersClient[i].Object.Light);
    }

    if (SelectedHero != -1 && o->Live)
    {
        EnableAlphaBlend();
        vec3_t Light;
        Vector(1.0f, 1.0f, 1.0f, Light);
        Vector(1.0f, 1.0f, 1.0f, o->Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 1, PrimaryTerrainLight);
        DisableAlphaBlend();
    }

    CHARACTER* pCha = NULL;
    OBJECT* pObj = NULL;

    for (int i = 0; i < 5; ++i)
    {
        pCha = &CharactersClient[i];
        pObj = &pCha->Object;
        if (pCha->Helper.Type == MODEL_HORN_OF_DINORANT)
        {
            pObj->Position[2] = 194.5f;
        }
        else
        {
            pObj->Position[2] = 169.5f;
        }
    }

    RenderTerrain(false);
    RenderObjects();
    RenderCharactersClient();

    if (!CUIMng::Instance().IsCursorOnUI())
        SelectObjects();

    RenderMount();
    RenderBlurs();
    RenderJoints();
    RenderEffects();
    ThePetProcess().RenderPets();
    RenderBoids();
    RenderObjects_AfterCharacter();
    CheckSprites();

    if (SelectedHero != -1 && o->Live)
    {
        vec3_t vLight;

        Vector(1.0f, 1.0f, 1.f, vLight);
        float fLumi = sinf(WorldTime * 0.0015f) * 0.3f + 0.5f;
        Vector(fLumi * vLight[0], fLumi * vLight[1], fLumi * vLight[2], vLight);

        EnableAlphaBlend();
        RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.8f, 1.8f, vLight, WorldTime * 0.01f);
        RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.2f, 1.2f, vLight, -WorldTime * 0.01f);
        DisableAlphaBlend();

        float Rotation = (int)WorldTime % 3600 / (float)10.f;
        Vector(0.15f, 0.15f, 0.15f, o->Light);
        CreateParticleFpsChecked(BITMAP_EFFECT, o->Position, o->Angle, o->Light, 4);
        CreateParticleFpsChecked(BITMAP_EFFECT, o->Position, o->Angle, o->Light, 5);

        g_csMapServer.SetHeroID((wchar_t*)CharactersClient[SelectedHero].ID);
    }

    BeginSprite();
    RenderSprites();
    RenderParticles();
    RenderPoints();
    EndSprite();
    BeginBitmap();
    RenderInfomation();

#ifdef ENABLE_EDIT
    RenderDebugWindow();
#endif

    EndBitmap();

    EndOpengl();

    return true;
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
        wcscpy(InputText[0], m_ID);
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

    wcscpy(Text, GlobalText[454]);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);
    g_pRenderText->RenderText(335 - Size.cx * 640 / WindowWidth, 480 - Size.cy * 640 / WindowWidth - 1, Text);

    wcscpy(Text, GlobalText[455]);

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);
    g_pRenderText->RenderText(335, 480 - Size.cy * 640 / WindowWidth - 1, Text);

    swprintf(Text, GlobalText[456], m_ExeVersion);

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

bool MoveMainCamera()
{
    bool bLockCamera = false;

    if (
        gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE
        && CCameraMove::GetInstancePtr()->IsTourMode())
        CameraFOV = 65.0f;
    else
        CameraFOV = 30.f;

#ifdef ENABLE_EDIT2
    {
        bool EditMove = false;
        if (!g_pUIManager->IsInputEnable())
        {
            if (HIBYTE(GetAsyncKeyState(VK_INSERT)) == 128)
                CameraAngle[2] += 15;
            if (HIBYTE(GetAsyncKeyState(VK_DELETE)) == 128)
                CameraAngle[2] -= 15;
            if (HIBYTE(GetAsyncKeyState(VK_HOME)) == 128)
                CameraAngle[2] = -45;

            if (CameraAngle[2] < -360)
                CameraAngle[2] += 360;
            else if (CameraAngle[2] > 0)
                CameraAngle[2] -= 360;


            vec3_t p1, p2;
            Vector(0.f, 0.f, 0.f, p1);
            FLOAT Velocity = sqrtf(TERRAIN_SCALE * TERRAIN_SCALE) * 1.25f * FPS_ANIMATION_FACTOR;

            if (HIBYTE(GetAsyncKeyState(VK_LEFT)) == 128)
            {
                Vector(-Velocity, -Velocity, 0.f, p1);
                EditMove = true;
            }
            if (HIBYTE(GetAsyncKeyState(VK_RIGHT)) == 128)
            {
                Vector(Velocity, Velocity, 0.f, p1);
                EditMove = true;
            }
            if (HIBYTE(GetAsyncKeyState(VK_UP)) == 128)
            {
                Vector(-Velocity, Velocity, 0.f, p1);
                EditMove = true;
            }
            if (HIBYTE(GetAsyncKeyState(VK_DOWN)) == 128)
            {
                Vector(Velocity, -Velocity, 0.f, p1);
                EditMove = true;
            }

            glPushMatrix();
            glLoadIdentity();
            glRotatef(-CameraAngle[2], 0.f, 0.f, 1.f);
            float Matrix[3][4];
            GetOpenGLMatrix(Matrix);
            glPopMatrix();
            VectorRotate(p1, Matrix, p2);
            VectorAdd(Hero->Object.Position, p2, Hero->Object.Position);
        }

        if (gMapManager.InChaosCastle() == false || !Hero->Object.m_bActionStart)
        {
            if (gMapManager.WorldActive == WD_39KANTURU_3RD && Hero->Object.m_bActionStart)
            {
            }
            else
                if (gMapManager.WorldActive == -1 || Hero->Helper.Type != MODEL_HORN_OF_DINORANT || Hero->SafeZone)
                {
                    Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]);
                }
                else
                {
                    if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
                        Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]) + 90.f;
                    else
                        Hero->Object.Position[2] = RequestTerrainHeight(Hero->Object.Position[0], Hero->Object.Position[1]) + 30.f;
                }
        }

        if (EditMove)
        {
            BYTE PathX[1];
            BYTE PathY[1];
            PathX[0] = (BYTE)(Hero->Object.Position[0] / TERRAIN_SCALE);
            PathY[0] = (BYTE)(Hero->Object.Position[1] / TERRAIN_SCALE);

            SendCharacterMove(Hero->Key, Hero->Object.Angle[2], 1, PathX, PathY, PathX[0], PathY[0]);

            Hero->Path.PathNum = 0;
        }
    }
#endif //ENABLE_EDIT2

    CameraAngle[0] = 0.f;
    CameraAngle[1] = 0.f;

    if (CameraTopViewEnable)
    {
        CameraViewFar = 3200.f;
        //CameraViewFar = 60000.f;
        CameraPosition[0] = Hero->Object.Position[0];
        CameraPosition[1] = Hero->Object.Position[1];
        CameraPosition[2] = CameraViewFar;
    }
    else
    {
        int iIndex = TERRAIN_INDEX((Hero->PositionX), (Hero->PositionY));
        vec3_t Position, TransformPosition;
        float Matrix[3][4];

        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            CameraViewFar = 3000.f;
        }
        else if (gMapManager.InBattleCastle() && SceneFlag == MAIN_SCENE)
        {
            CameraViewFar = 2500.f;
        }
        else if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
        {
            CameraViewFar = 2800.f * 1.15f;
        }
        else if (gMapManager.IsPKField() || IsDoppelGanger2())
        {
            CameraViewFar = 3700.0f;
        }
        else
        {
            switch (g_shCameraLevel)
            {
            case 0:
                if (SceneFlag == LOG_IN_SCENE)
                {
                }
                else if (SceneFlag == CHARACTER_SCENE)
                {
                    CameraViewFar = 3500.f;
                }
                else if (g_Direction.m_CKanturu.IsMayaScene())
                {
                    CameraViewFar = 2000.f * 10.0f * 0.115f;
                }
                else
                {
                    CameraViewFar = 2000.f;
                }
                break;
            case 1: CameraViewFar = 2500.f; break;
            case 2: CameraViewFar = 2600.f; break;
            case 3: CameraViewFar = 2950.f; break;
            case 5:
            case 4: CameraViewFar = 3200.f; break;
            }
        }

        Vector(0.f, -CameraDistance, 0.f, Position);//-750
        AngleMatrix(CameraAngle, Matrix);
        VectorIRotate(Position, Matrix, TransformPosition);

        if (SceneFlag == MAIN_SCENE)
        {
            g_pCatapultWindow->GetCameraPos(Position);
        }
        else if (CCameraMove::GetInstancePtr()->IsTourMode())
        {
            CCameraMove::GetInstancePtr()->UpdateTourWayPoint();
            CCameraMove::GetInstancePtr()->GetCurrentCameraPos(Position);
            CameraViewFar = 390.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel();
        }

        if (g_Direction.IsDirection() && !g_Direction.m_bDownHero)
        {
            Hero->Object.Position[2] = 300.0f;
            g_shCameraLevel = g_Direction.GetCameraPosition(Position);
        }
        else if (gMapManager.IsPKField() || IsDoppelGanger2())
        {
            g_shCameraLevel = 5;
        }
        else if (IsDoppelGanger1())
        {
            g_shCameraLevel = 5;
        }
        else g_shCameraLevel = 0;

        if (CCameraMove::GetInstancePtr()->IsTourMode())
        {
            vec3_t temp = { 0.0f,0.0f,-100.0f };
            VectorAdd(TransformPosition, temp, TransformPosition);
        }

        VectorAdd(Position, TransformPosition, CameraPosition);

        if (gMapManager.InBattleCastle() == true)
        {
            CameraPosition[2] = 255.f;//700
        }
        else if (CCameraMove::GetInstancePtr()->IsTourMode());
        else
        {
            CameraPosition[2] = Hero->Object.Position[2];//700
        }

        if ((TerrainWall[iIndex] & TW_HEIGHT) == TW_HEIGHT)
        {
            CameraPosition[2] = g_fSpecialHeight = 1200.f + 1;
        }
        CameraPosition[2] += CameraDistance - 150.f;//700

        if (CCameraMove::GetInstancePtr()->IsTourMode())
        {
            CCameraMove::GetInstancePtr()->SetAngleFrustum(-112.5f);
            CameraAngle[0] = CCameraMove::GetInstancePtr()->GetAngleFrustum();
            CameraAngle[1] = 0.0f;
            CameraAngle[2] = CCameraMove::GetInstancePtr()->GetCameraAngle();
        }
        else if (SceneFlag == CHARACTER_SCENE)
        {
            CameraAngle[0] = -84.5f;
            CameraAngle[1] = 0.0f;
            CameraAngle[2] = -75.0f;
            CameraPosition[0] = 9758.93f;
            CameraPosition[1] = 18913.11f;
            CameraPosition[2] = 675.5f;
        }
        else
        {
            CameraAngle[0] = -48.5f;
        }

        CameraAngle[0] += EarthQuake;

        if ((TerrainWall[iIndex] & TW_CAMERA_UP) == TW_CAMERA_UP)
        {
            if (g_fCameraCustomDistance <= CUSTOM_CAMERA_DISTANCE1)
            {
                g_fCameraCustomDistance += 10;
            }
        }
        else
        {
            if (g_fCameraCustomDistance > 0)
            {
                g_fCameraCustomDistance -= 10;
            }
        }

        if (g_fCameraCustomDistance > 0)
        {
            vec3_t angle = { 0.f, 0.f, -45.f };
            Vector(0.f, g_fCameraCustomDistance, 0.f, Position);
            AngleMatrix(angle, Matrix);
            VectorIRotate(Position, Matrix, TransformPosition);
            VectorAdd(CameraPosition, TransformPosition, CameraPosition);
        }
        else if (g_fCameraCustomDistance < 0)
        {
            vec3_t angle = { 0.f, 0.f, -45.f };
            Vector(0.f, g_fCameraCustomDistance, 0.f, Position);
            AngleMatrix(angle, Matrix);
            VectorIRotate(Position, Matrix, TransformPosition);
            VectorAdd(CameraPosition, TransformPosition, CameraPosition);
        }
    }
    if (gMapManager.WorldActive == 5)
    {
        CameraAngle[0] += sinf(WorldTime * 0.0005f) * 2.f;
        CameraAngle[1] += sinf(WorldTime * 0.0008f) * 2.5f;
    }
    else if (CCameraMove::GetInstancePtr()->IsTourMode())
    {
        CameraDistanceTarget = 1100.f * CCameraMove::GetInstancePtr()->GetCurrentCameraDistanceLevel() * 0.1f;
        CameraDistance = CameraDistanceTarget;
    }
    else
    {
        if (gMapManager.InBattleCastle())
        {
            CameraDistanceTarget = 1100.f;
            CameraDistance = CameraDistanceTarget;
        }
        else
        {
            switch (g_shCameraLevel)
            {
            case 0: CameraDistanceTarget = 1000.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 1: CameraDistanceTarget = 1100.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 2: CameraDistanceTarget = 1200.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 3: CameraDistanceTarget = 1300.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 4: CameraDistanceTarget = 1400.f; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            case 5: CameraDistanceTarget = g_Direction.m_fCameraViewFar; CameraDistance += (CameraDistanceTarget - CameraDistance) / 3; break;
            }
        }
    }

    return bLockCamera;
}

void MoveMainScene()
{
    if (!InitMainScene)
    {
        g_pMainFrame->ResetSkillHotKey();

        g_ConsoleDebug->Write(MCD_NORMAL, L"Join the game with the following character: %s", CharactersClient[SelectedHero].ID);

        g_ErrorReport.Write(L"> Character selected <%d> \"%s\"\r\n", SelectedHero + 1, CharactersClient[SelectedHero].ID);

        InitMainScene = true;

        g_ConsoleDebug->Write(MCD_SEND, L"SendRequestJoinMapServer");

        CurrentProtocolState = REQUEST_JOIN_MAP_SERVER;
        SocketClient->ToGameServer()->SendSelectCharacter(CharactersClient[SelectedHero].ID);
        // SendRequestJoinMapServer(CharactersClient[SelectedHero].ID);

        CUIMng::Instance().CreateMainScene();

        CameraAngle[2] = -45.f;

        ClearInput();
        InputEnable = false;
        TabInputEnable = false;
        InputTextWidth = 256;
        InputTextMax[0] = 42;
        InputTextMax[1] = 10;
        InputNumber = 2;
        for (int i = 0; i < MAX_WHISPER; i++)
        {
            g_pChatListBox->AddText(L"", L"", SEASON3B::TYPE_WHISPER_MESSAGE);
        }

        g_GuildNotice[0][0] = '\0';
        g_GuildNotice[1][0] = '\0';

        g_pPartyManager->Create();

        g_pChatListBox->ClearAll();
        g_pSystemLogBox->ClearAll();

        g_pSlideHelpMgr->Init();
        g_pUIMapName->Init();
        g_pNewUIMuHelper->Reset();

        g_GuildCache.Reset();

        g_PortalMgr.Reset();

        ClearAllObjectBlurs();

        SetFocus(g_hWnd);

        g_ErrorReport.Write(L"> Main Scene init success. ");
        g_ErrorReport.WriteCurrentTime();

        g_ConsoleDebug->Write(MCD_NORMAL, L"MainScene Init Success");
    }

    if (CurrentProtocolState == RECEIVE_JOIN_MAP_SERVER)
    {
        EnableMainRender = true;
    }
    if (EnableMainRender == false)
    {
        return;
    }
    //init
    EarthQuake *= 0.2f;

    InitTerrainLight();

    CheckInventory = NULL;
    CheckSkill = -1;
    MouseOnWindow = false;

    if (!CameraTopViewEnable && LoadingWorld < 30)
    {
        if (MouseY >= (int)(480 - 48))
            MouseOnWindow = true;

        g_pPartyManager->Update();
        g_pNewUISystem->Update();

        if (MouseLButton == true && false == g_pNewUISystem->CheckMouseUse() && g_dwMouseUseUIID == 0 && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == false)
        {
            g_pWindowMgr->SetWindowsEnable(FALSE);
            g_pFriendMenu->HideMenu();
            g_dwKeyFocusUIID = 0;
            if (GetFocus() != g_hWnd)
            {
                SaveIMEStatus();
                SetFocus(g_hWnd);
            }
        }
        MoveInterface();
        MoveTournamentInterface();
        if (ErrorMessage != MESSAGE_LOG_OUT)
            g_pUIManager->UpdateInput();
    }

    if (ErrorMessage != NULL)
        MouseOnWindow = true;

    MoveObjects();
    if (!CameraTopViewEnable)
        MoveItems();
    if ((gMapManager.WorldActive == WD_0LORENCIA && HeroTile != 4) ||
        (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
        || gMapManager.WorldActive == WD_3NORIA
        || gMapManager.WorldActive == WD_7ATLANSE
        || gMapManager.InDevilSquare() == true
        || gMapManager.WorldActive == WD_10HEAVEN
        || gMapManager.InChaosCastle() == true
        || gMapManager.InBattleCastle() == true
        || M31HuntingGround::IsInHuntingGround() == true
        || M33Aida::IsInAida() == true
        || M34CryWolf1st::IsCyrWolf1st() == true
        || gMapManager.WorldActive == WD_42CHANGEUP3RD_2ND
        || IsIceCity()
        || IsSantaTown()
        || gMapManager.IsPKField()
        || IsDoppelGanger2()
        || gMapManager.IsEmpireGuardian1()
        || gMapManager.IsEmpireGuardian2()
        || gMapManager.IsEmpireGuardian3()
        || gMapManager.IsEmpireGuardian4()
        || IsUnitedMarketPlace()
        )
    {
        MoveLeaves();
    }

    MoveBoids();
    MoveFishs();
    MoveChat();
    UpdatePersonalShopTitleImp();
    MoveHero();
    MoveCharactersClient();
    MoveMounts();
    ThePetProcess().UpdatePets();
    MovePoints();
    MoveEffects();
    MoveJoints();
    MoveParticles();
    MovePointers();

    g_Direction.CheckDirection();

#ifdef ENABLE_EDIT
    EditObjects();
#endif //ENABLE_EDIT

    g_ConsoleDebug->UpdateMainScene();
}

bool RenderMainScene()
{
    if (EnableMainRender == false)
    {
        return false;
    }

    if ((LoadingWorld) > 30)
    {
        return false;
    }

    FogEnable = false;

    vec3_t pos;

    if (MoveMainCamera() == true)
    {
        VectorCopy(Hero->Object.StartPosition, pos);
    }
    else
    {
        g_pCatapultWindow->GetCameraPos(pos);

        if (g_Direction.IsDirection() && g_Direction.m_bDownHero == false)
        {
            g_Direction.GetCameraPosition(pos);
        }
    }

    int Width, Height;

    BYTE byWaterMap = 0;

    if (CameraTopViewEnable == false)
    {
        Height = 480 - 48;
    }
    else
    {
        Height = 480;
    }

    Width = GetScreenWidth();
    if (gMapManager.WorldActive == WD_0LORENCIA)
    {
        glClearColor(10 / 256.f, 20 / 256.f, 14 / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_2DEVIAS)
    {
        glClearColor(0.f / 256.f, 0.f / 256.f, 10.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        glClearColor(3.f / 256.f, 25.f / 256.f, 44.f / 256.f, 1.f);
    }
    else if (gMapManager.InChaosCastle() == true)
    {
        glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive >= WD_45CURSEDTEMPLE_LV1 && gMapManager.WorldActive <= WD_45CURSEDTEMPLE_LV6)
    {
        glClearColor(9.f / 256.f, 8.f / 256.f, 33.f / 256.f, 1.f);
    }
    else if (gMapManager.InHellas() == true)
    {
        byWaterMap = 1;
        glClearColor(0.f / 256.f, 0.f / 256.f, 0.f / 256.f, 1.f);
    }
    else
    {
        glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
    }

    BeginOpengl(0, 0, Width, Height);

    CreateFrustrum((float)Width / (float)640, (float)Height / 480.f, pos);

    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            vec3_t Color = { 0.f, 0.f, 0.f };
            battleCastle::StartFog(Color);
        }
        else
        {
            glDisable(GL_FOG);
        }
    }

    CreateScreenVector(MouseX, MouseY, MouseTarget);

    if (IsWaterTerrain() == false)
    {
        if (gMapManager.WorldActive == WD_39KANTURU_3RD)
        {
            if (!g_Direction.m_CKanturu.IsMayaScene())
                RenderTerrain(false);
        }
        else
            if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.WorldActive != -1)
            {
                if (gMapManager.IsPKField() || IsDoppelGanger2())
                {
                    RenderObjects();
                }
                RenderTerrain(false);
            }
    }

    if (!gMapManager.IsPKField() && !IsDoppelGanger2())
        RenderObjects();

    RenderEffectShadows();
    RenderBoids();

    RenderCharactersClient();

    if (EditFlag != EDIT_NONE)
    {
        RenderTerrain(true);
    }
    if (!CameraTopViewEnable)
        RenderItems();

    RenderFishs();
    RenderMount();
    RenderLeaves();

    if (!gMapManager.InChaosCastle())
        ThePetProcess().RenderPets();

    RenderBoids(true);
    RenderObjects_AfterCharacter();

    RenderJoints(byWaterMap);
    RenderEffects();
    RenderBlurs();
    CheckSprites();
    BeginSprite();

    if ((gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
        || IsIceCity()
        || IsSantaTown()
        || gMapManager.IsPKField()
        || IsDoppelGanger2()
        || gMapManager.IsEmpireGuardian1()
        || gMapManager.IsEmpireGuardian2()
        || gMapManager.IsEmpireGuardian3()
        || gMapManager.IsEmpireGuardian4()
        || IsUnitedMarketPlace()
        )
    {
        RenderLeaves();
    }

    RenderSprites();
    RenderParticles();

    if (IsWaterTerrain() == false)
    {
        RenderPoints(byWaterMap);
    }

    EndSprite();

    RenderAfterEffects();

    if (IsWaterTerrain() == true)
    {
        byWaterMap = 2;

        EndOpengl();
        BeginOpengl(0, 0, Width, Height);
        RenderWaterTerrain();
        RenderJoints(byWaterMap);
        RenderEffects(true);
        RenderBlurs();
        CheckSprites();
        BeginSprite();

        if (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
            RenderLeaves();

        RenderSprites(byWaterMap);
        RenderParticles(byWaterMap);
        RenderPoints(byWaterMap);

        EndSprite();
        EndOpengl();

        BeginOpengl(0, 0, Width, Height);
    }

    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            battleCastle::EndFog();
        }
    }

    SelectObjects();
    BeginBitmap();
    RenderObjectDescription();

    if (CameraTopViewEnable == false)
    {
        RenderInterface(true);
    }
    RenderTournamentInterface();
    EndBitmap();

    g_pPartyManager->Render();
    g_pNewUISystem->Render();

    BeginBitmap();

    RenderInfomation();

#ifdef ENABLE_EDIT
    RenderDebugWindow();
#endif //ENABLE_EDIT

    EndBitmap();
    BeginBitmap();

    RenderCursor();

    EndBitmap();
    EndOpengl();

    return true;
}

extern int WaterTextureNumber;

int TestTime = 0;
extern int  GrabScreen;

void MoveCharacter(CHARACTER* c, OBJECT* o);

double target_fps = 60;
double ms_per_frame = 1000.0 / target_fps;

void SetTargetFps(double targetFps)
{
    if (IsVSyncEnabled() && targetFps >= GetFPSLimit())
    {
        targetFps = -1;
    }

    target_fps = targetFps;
    ms_per_frame = 1000.0 / target_fps;
}

double last_render_tick_count = 0;
double current_tick_count = 0;
double last_water_change = 0;

void UpdateSceneState()
{
    g_pNewKeyInput->ScanAsyncKeyState();

    g_dwMouseUseUIID = 0;

    switch (SceneFlag)
    {
    case LOG_IN_SCENE:
        NewMoveLogInScene();
        break;

    case CHARACTER_SCENE:
        NewMoveCharacterScene();
        break;

    case MAIN_SCENE:
        MoveMainScene();
        break;
    }

    MoveNotices();

    if (PressKey(VK_SNAPSHOT))
    {
        if (GrabEnable)
            GrabEnable = false;
        else
            GrabEnable = true;
    }

    const bool addTimeStampToCapture = !HIBYTE(GetAsyncKeyState(VK_SHIFT));
    wchar_t screenshotText[256];
    if (GrabEnable)
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        swprintf(GrabFileName, L"Screen(%02d_%02d-%02d_%02d)-%04d.jpg", st.wMonth, st.wDay, st.wHour, st.wMinute, GrabScreen);
        swprintf(screenshotText, GlobalText[459], GrabFileName);
        wchar_t lpszTemp[64];
        swprintf(lpszTemp, L" [%s / %s]", g_ServerListManager->GetSelectServerName(), Hero->ID);
        wcscat(screenshotText, lpszTemp);
        if (addTimeStampToCapture)
        {
            g_pSystemLogBox->AddText(screenshotText, SEASON3B::TYPE_SYSTEM_MESSAGE);
        }

        auto Buffer = new unsigned char[WindowWidth * WindowHeight * 3];

        glReadPixels(0, 0, WindowWidth, WindowHeight, GL_RGB, GL_UNSIGNED_BYTE, &Buffer[0]);

        WriteJpeg(GrabFileName, WindowWidth, WindowHeight, &Buffer[0], 100);

        SAFE_DELETE_ARRAY(Buffer);

        GrabScreen++;
        GrabScreen %= 10000;
    }

    if (GrabEnable && !addTimeStampToCapture)
    {
        g_pSystemLogBox->AddText(screenshotText, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    GrabEnable = false;
}

void MainScene(HDC hDC)
{
    if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
    {
        double dDeltaTick = g_pTimer->GetTimeElapsed();
        dDeltaTick = MIN(dDeltaTick, 200.0 * FPS_ANIMATION_FACTOR);
        
        CInput::Instance().Update();
        CUIMng::Instance().Update(dDeltaTick);
    }

    constexpr int NumberOfWaterTextures = 32;
    constexpr double timePerFrame = 1000 / REFERENCE_FPS;
    auto time_since_last_render = current_tick_count - last_water_change;
    while (time_since_last_render > timePerFrame)
    {
        WaterTextureNumber++;
        WaterTextureNumber %= NumberOfWaterTextures;
        time_since_last_render -= timePerFrame;
        last_water_change = current_tick_count;
    }

    if (Destroy) {
        return;
    }

    g_PhysicsManager.Move(0.025f * FPS_ANIMATION_FACTOR);

    Bitmaps.Manage();

    Set3DSoundPosition();

    if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        glClearColor(3.f / 256.f, 25.f / 256.f, 44.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE || gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (gMapManager.InHellas(gMapManager.WorldActive))
    {
        glClearColor(30.f / 256.f, 40.f / 256.f, 40.f / 256.f, 1.f);
    }
    else if (gMapManager.InChaosCastle() == true)
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
    }
    else if (gMapManager.InBattleCastle() && battleCastle::InBattleCastle2(Hero->Object.Position))
    {
        glClearColor(0.f, 0.f, 0.f, 1.f);
    }
    else if (gMapManager.WorldActive >= WD_45CURSEDTEMPLE_LV1 && gMapManager.WorldActive <= WD_45CURSEDTEMPLE_LV6)
    {
        glClearColor(9.f / 256.f, 8.f / 256.f, 33.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR
        )
    {
        glClearColor(178.f / 256.f, 178.f / 256.f, 178.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_65DOPPLEGANGER1)
    {
        glClearColor(148.f / 256.f, 179.f / 256.f, 223.f / 256.f, 1.f);
    }
    else
    {
        glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool Success = false;

    try
    {
        if (SceneFlag == LOG_IN_SCENE)
        {
            Success = NewRenderLogInScene(hDC);
        }
        else if (SceneFlag == CHARACTER_SCENE)
        {
            Success = NewRenderCharacterScene(hDC);
        }
        else if (SceneFlag == MAIN_SCENE)
        {
            Success = RenderMainScene();
        }

        g_PhysicsManager.Render();

#if defined(_DEBUG) || defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
        BeginBitmap();
        wchar_t szDebugText[128];
        swprintf(szDebugText, L"FPS: %.1f Vsync: %d CPU: %.1f%%", FPS_AVG, IsVSyncEnabled(), CPU_AVG);
        wchar_t szMousePos[128];
        swprintf(szMousePos, L"MousePos : %d %d %d", MouseX, MouseY, MouseLButtonPush);
        wchar_t szCamera3D[128];
        swprintf(szCamera3D, L"Camera3D : %.1f %.1f:%.1f:%.1f", CameraFOV, CameraAngle[0], CameraAngle[1], CameraAngle[2]);
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetBgColor(0, 0, 0, 100);
        g_pRenderText->SetTextColor(255, 255, 255, 200);
        g_pRenderText->RenderText(10, 26, szDebugText);
        g_pRenderText->RenderText(10, 36, szMousePos);
        g_pRenderText->RenderText(10, 46, szCamera3D);
        g_pRenderText->SetFont(g_hFont);
        EndBitmap();
#endif // defined(_DEBUG) || defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)

        if (Success)
        {
            SwapBuffers(hDC);
        }

        if (SocketClient == nullptr || !SocketClient->IsConnected())
        {
            static BOOL s_bClosed = FALSE;
            if (!s_bClosed)
            {
                s_bClosed = TRUE;
                g_ErrorReport.Write(L"> Connection closed. ");
                g_ErrorReport.WriteCurrentTime();
                g_ConsoleDebug->Write(MCD_NORMAL, L"Connection closed");
                CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
            }
        }

        if (SceneFlag == MAIN_SCENE)
        {
            switch (gMapManager.WorldActive)
            {
            case WD_0LORENCIA:
                if (HeroTile == 4)
                {
                    StopBuffer(SOUND_WIND01, true);
                    StopBuffer(SOUND_RAIN01, true);
                }
                else
                {
                    PlayBuffer(SOUND_WIND01, NULL, true);
                    if (RainCurrent > 0)
                        PlayBuffer(SOUND_RAIN01, NULL, true);
                }
                break;
            case WD_1DUNGEON:
                PlayBuffer(SOUND_DUNGEON01, NULL, true);
                break;
            case WD_2DEVIAS:
                if (HeroTile == 3 || HeroTile >= 10)
                    StopBuffer(SOUND_WIND01, true);
                else
                    PlayBuffer(SOUND_WIND01, NULL, true);
                break;
            case WD_3NORIA:
                PlayBuffer(SOUND_WIND01, NULL, true);
                if (rand_fps_check(512))
                    PlayBuffer(SOUND_FOREST01);
                break;
            case WD_4LOSTTOWER:
                PlayBuffer(SOUND_TOWER01, NULL, true);
                break;
            case WD_5UNKNOWN:
                //PlayBuffer(SOUND_BOSS01,NULL,true);
                break;
            case WD_7ATLANSE:
                PlayBuffer(SOUND_WATER01, NULL, true);
                break;
            case WD_8TARKAN:
                PlayBuffer(SOUND_DESERT01, NULL, true);
                break;
            case WD_10HEAVEN:
                PlayBuffer(SOUND_HEAVEN01, NULL, true);
                if (rand_fps_check(100))
                {
                    //                PlayBuffer(SOUND_HEAVEN01);
                }
                else if (rand_fps_check(10))
                {
                    //                PlayBuffer(SOUND_THUNDERS02);
                }
                break;
            case WD_58ICECITY_BOSS:
                PlayBuffer(SOUND_WIND01, NULL, true);
                break;
            case WD_79UNITEDMARKETPLACE:
            {
                PlayBuffer(SOUND_WIND01, NULL, true);
                PlayBuffer(SOUND_RAIN01, NULL, true);
            }
            break;
#ifdef ASG_ADD_MAP_KARUTAN
            case WD_80KARUTAN1:
                PlayBuffer(SOUND_KARUTAN_DESERT_ENV, NULL, true);
                break;
            case WD_81KARUTAN2:
                if (HeroTile == 12)
                {
                    StopBuffer(SOUND_KARUTAN_DESERT_ENV, true);
                    PlayBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, NULL, true);
                }
                else
                {
                    StopBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, true);
                    PlayBuffer(SOUND_KARUTAN_DESERT_ENV, NULL, true);
                }
                break;
#endif	// ASG_ADD_MAP_KARUTAN
            }
            if (gMapManager.WorldActive != WD_0LORENCIA && gMapManager.WorldActive != WD_2DEVIAS && gMapManager.WorldActive != WD_3NORIA && gMapManager.WorldActive != WD_58ICECITY_BOSS && gMapManager.WorldActive != WD_79UNITEDMARKETPLACE)
            {
                StopBuffer(SOUND_WIND01, true);
            }
            if (gMapManager.WorldActive != WD_0LORENCIA && gMapManager.InDevilSquare() == false && gMapManager.WorldActive != WD_79UNITEDMARKETPLACE)
            {
                StopBuffer(SOUND_RAIN01, true);
            }
            if (gMapManager.WorldActive != WD_1DUNGEON)
            {
                StopBuffer(SOUND_DUNGEON01, true);
            }
            if (gMapManager.WorldActive != WD_3NORIA)
            {
                StopBuffer(SOUND_FOREST01, true);
            }
            if (gMapManager.WorldActive != WD_4LOSTTOWER)
            {
                StopBuffer(SOUND_TOWER01, true);
            }
            if (gMapManager.WorldActive != WD_7ATLANSE)
            {
                StopBuffer(SOUND_WATER01, true);
            }
            if (gMapManager.WorldActive != WD_8TARKAN)
            {
                StopBuffer(SOUND_DESERT01, true);
            }
            if (gMapManager.WorldActive != WD_10HEAVEN)
            {
                StopBuffer(SOUND_HEAVEN01, true);
            }
            if (gMapManager.WorldActive != WD_51HOME_6TH_CHAR)
            {
                StopBuffer(SOUND_ELBELAND_VILLAGEPROTECTION01, true);
                StopBuffer(SOUND_ELBELAND_WATERFALLSMALL01, true);
                StopBuffer(SOUND_ELBELAND_WATERWAY01, true);
                StopBuffer(SOUND_ELBELAND_ENTERDEVIAS01, true);
                StopBuffer(SOUND_ELBELAND_WATERSMALL01, true);
                StopBuffer(SOUND_ELBELAND_RAVINE01, true);
                StopBuffer(SOUND_ELBELAND_ENTERATLANCE01, true);
            }
#ifdef ASG_ADD_MAP_KARUTAN
            if (!IsKarutanMap())
                StopBuffer(SOUND_KARUTAN_DESERT_ENV, true);
            if (gMapManager.WorldActive != WD_80KARUTAN1)
                StopBuffer(SOUND_KARUTAN_INSECT_ENV, true);
            if (gMapManager.WorldActive != WD_81KARUTAN2)
                StopBuffer(SOUND_KARUTAN_KARDAMAHAL_ENV, true);
#endif	// ASG_ADD_MAP_KARUTAN

            if (gMapManager.WorldActive == WD_0LORENCIA)
            {
                if (Hero->SafeZone)
                {
                    if (HeroTile == 4)
                        PlayMp3(MUSIC_PUB);
                    else
                        PlayMp3(MUSIC_MAIN_THEME);
                }
            }
            else
            {
                StopMp3(MUSIC_PUB);
                StopMp3(MUSIC_MAIN_THEME);
            }
            if (gMapManager.WorldActive == WD_2DEVIAS)
            {
                if (Hero->SafeZone)
                {
                    if ((Hero->PositionX) >= 205 && (Hero->PositionX) <= 214 &&
                        (Hero->PositionY) >= 13 && (Hero->PositionY) <= 31)
                    {
                        PlayMp3(MUSIC_CHURCH);
                    }
                    else
                    {
                        PlayMp3(MUSIC_DEVIAS);
                    }
                }
            }
            else
            {
                StopMp3(MUSIC_CHURCH);
                StopMp3(MUSIC_DEVIAS);
            }
            if (gMapManager.WorldActive == WD_3NORIA)
            {
                if (Hero->SafeZone)
                    PlayMp3(MUSIC_NORIA);
            }
            else
            {
                StopMp3(MUSIC_NORIA);
            }
            if (gMapManager.WorldActive == WD_1DUNGEON || gMapManager.WorldActive == WD_5UNKNOWN)
            {
                PlayMp3(MUSIC_DUNGEON);
            }
            else
            {
                StopMp3(MUSIC_DUNGEON);
            }

            if (gMapManager.WorldActive == WD_7ATLANSE) {
                PlayMp3(MUSIC_ATLANS);
            }
            else {
                StopMp3(MUSIC_ATLANS);
            }
            if (gMapManager.WorldActive == WD_10HEAVEN) {
                PlayMp3(MUSIC_ICARUS);
            }
            else {
                StopMp3(MUSIC_ICARUS);
            }
            if (gMapManager.WorldActive == WD_8TARKAN) {
                PlayMp3(MUSIC_TARKAN);
            }
            else {
                StopMp3(MUSIC_TARKAN);
            }
            if (gMapManager.WorldActive == WD_4LOSTTOWER) {
                PlayMp3(MUSIC_LOSTTOWER_A);
            }
            else {
                StopMp3(MUSIC_LOSTTOWER_A);
            }

            if (gMapManager.InHellas(gMapManager.WorldActive)) {
                PlayMp3(MUSIC_KALIMA);
            }
            else {
                StopMp3(MUSIC_KALIMA);
            }

            if (gMapManager.WorldActive == WD_31HUNTING_GROUND) {
                PlayMp3(MUSIC_BC_HUNTINGGROUND);
            }
            else {
                StopMp3(MUSIC_BC_HUNTINGGROUND);
            }

            if (gMapManager.WorldActive == WD_33AIDA) {
                PlayMp3(MUSIC_BC_ADIA);
            }
            else {
                StopMp3(MUSIC_BC_ADIA);
            }

            M34CryWolf1st::ChangeBackGroundMusic(gMapManager.WorldActive);
            M39Kanturu3rd::ChangeBackGroundMusic(gMapManager.WorldActive);

            if (gMapManager.WorldActive == WD_37KANTURU_1ST)
                PlayMp3(MUSIC_KANTURU_1ST);
            else
                StopMp3(MUSIC_KANTURU_1ST);
            M38Kanturu2nd::PlayBGM();
            SEASON3A::CGM3rdChangeUp::Instance().PlayBGM();
            if (gMapManager.IsCursedTemple())
            {
                g_CursedTemple->PlayBGM();
            }
            if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR) {
                PlayMp3(MUSIC_ELBELAND);
            }
            else {
                StopMp3(MUSIC_ELBELAND);
            }

            if (gMapManager.WorldActive == WD_56MAP_SWAMP_OF_QUIET) {
                PlayMp3(MUSIC_SWAMP_OF_QUIET);
            }
            else {
                StopMp3(MUSIC_SWAMP_OF_QUIET);
            }

            g_Raklion.PlayBGM();
            g_SantaTown.PlayBGM();
            g_PKField.PlayBGM();
            g_DoppelGanger1.PlayBGM();
            g_EmpireGuardian1.PlayBGM();
            g_EmpireGuardian2.PlayBGM();
            g_EmpireGuardian3.PlayBGM();
            g_EmpireGuardian4.PlayBGM();
            g_UnitedMarketPlace.PlayBGM();
#ifdef ASG_ADD_MAP_KARUTAN
            g_Karutan1.PlayBGM();
#endif	// ASG_ADD_MAP_KARUTAN
        }
    }
    catch (const std::exception&)
    {
    }
}

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
            const auto sleep_ms = min(rest_ms - sleep_duration_offset_ms, max_sleep_ms);
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

void RenderScene(HDC hDC)
{
    CalcFPS();
    UpdateSceneState();

    last_render_tick_count = current_tick_count;

    try
    {
        g_Luminosity = sinf(WorldTime * 0.004f) * 0.15f + 0.6f;
        switch (SceneFlag)
        {
        case WEBZEN_SCENE:
            WebzenScene(hDC);
            break;
        case LOADING_SCENE:
            LoadingScene(hDC);
            break;
        case LOG_IN_SCENE:
        case CHARACTER_SCENE:
        case MAIN_SCENE:
            MainScene(hDC);
            break;
        }

        if (g_iNoMouseTime > 31)
        {
            KillGLWindow();
        }
    }
    catch (const std::exception&)
    {
    }
}

bool GetTimeCheck(int DelayTime)
{
    int PresentTime = timeGetTime();

    if (g_bTimeCheck)
    {
        g_iBackupTime = PresentTime;
        g_bTimeCheck = false;
    }

    if (g_iBackupTime + DelayTime <= PresentTime)
    {
        g_bTimeCheck = true;
        return true;
    }
    return false;
}