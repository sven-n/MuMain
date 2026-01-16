///////////////////////////////////////////////////////////////////////////////
// CharacterScene.cpp - Character selection scene implementation
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CharacterScene.h"
#include "../ZzzScene.h"
#include "../CharacterManager.h"
#include "../MapManager.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzObject.h"
#include "../ZzzCharacter.h"
#include "../ZzzLodTerrain.h"
#include "../ZzzInterface.h"
#include "../ZzzInfomation.h"
#include "../ZzzEffect.h"
#include "../GOBoid.h"
#include "../w_PetProcess.h"
#include "../UIMng.h"
#include "../Input.h"
#include "../WSclient.h"
#include "../CSMapServer.h"
#include "../Utilities/Log/muConsoleDebug.h"
#include "../ZzzInterface.h"
#include "../NewUISystem.h"
#include "../DSPlaySound.h"
#include "../Winmain.h"
#include "SceneCommon.h"
#include "../Camera/CameraUtility.h"
#include "../ZzzOpenData.h"
#include "LoginScene.h"

// External declarations
extern int SelectedHero;
extern bool InitCharacterScene;
extern EGameScene SceneFlag;
extern bool EnableMainRender;
extern int ErrorMessage;
extern CHARACTER CharacterView;
extern int SelectedCharacter;
extern int g_iKeyPadEnable;
extern int g_iChatInputType;
extern CUITextInputBox* g_pSinglePasswdInputBox;
extern BOOL g_bIMEBlock;
extern DWORD g_dwBKConv;
extern DWORD g_dwBKSent;
extern HWND g_hWnd;
extern CErrorReport g_ErrorReport;
extern double WorldTime;
extern int MouseX;
extern int MouseY;
extern vec3_t MouseTarget;
extern float CameraViewFar;

// Forward declaration
BOOL Util_CheckOption(std::wstring lpszCommandLine, wchar_t cOption, std::wstring& lpszString);

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
            ::wcscpy_s(CharacterAttribute->Name, MAX_ID_SIZE + 1, CharactersClient[SelectedHero].ID);

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
            && SelectedHero > -1 && SelectedHero < MAX_CHARACTERS_PER_ACCOUNT)
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
        if (SelectedCharacter < 0 || SelectedCharacter >= MAX_CHARACTERS_PER_ACCOUNT)
        {
            return;
        }

        SelectedHero = SelectedCharacter;
        ::StartGame();
    }
    else if (rInput.IsLBtnDn())
    {
        if (SelectedCharacter < 0 || SelectedCharacter >= MAX_CHARACTERS_PER_ACCOUNT)
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
    for (int i = 0; i < MAX_CHARACTERS_PER_ACCOUNT; i++)
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

    for (int i = 0; i < MAX_CHARACTERS_PER_ACCOUNT; ++i)
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
