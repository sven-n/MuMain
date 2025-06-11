///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <imm.h>
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzLodTerrain.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzTexture.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "ZzzScene.h"
#include "ZzzPath.h"
#include "DSPlaySound.h"


#include "MatchEvent.h"
#include "CSQuest.h"
#include "PersonalShopTitleImp.h"
#include "zzzopenglUtil.h"
#include "CSItemOption.h"
#include "npcBreeder.h"
#include "GIPetManager.h"
#include "CSParts.h"
#include "UIMapName.h"	// rozy
#include "CDirection.h"
#include "MapManager.h"
#include "Event.h"

#include "NewUISystem.h"
#include "w_CursedTemple.h"
#include "UIControls.h"
#include "PartyManager.h"
#include "NewUICommonMessageBox.h"
#include "SummonSystem.h"
#include "w_MapHeaders.h"
#include "DuelMgr.h"
#include "ChangeRingManager.h"
#include "NewUIGensRanking.h"
#include "MonkSystem.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "MUHelper/MuHelper.h"

#include "ZzzInterface.h"

extern CUITextInputBox* g_pSingleTextInputBox;
extern CUITextInputBox* g_pSinglePasswdInputBox;
extern int g_iChatInputType;
extern BOOL g_bUseChatListBox;
extern DWORD g_dwMouseUseUIID;
extern CUIMapName* g_pUIMapName;	// rozy
extern bool bCheckNPC;
extern BOOL g_bWhileMovingZone;
extern DWORD g_dwLatestZoneMoving;
extern bool LogOut;
extern int DirTable[16];

extern BOOL g_bUseWindowMode;
extern void SetPlayerBow(CHARACTER* c);

#ifdef _PVP_ADD_MOVE_SCROLL
extern CMurdererMove g_MurdererMove;
#endif	// _PVP_ADD_MOVE_SCROLL






extern vec3_t MousePosition, MouseTarget;

extern void RegisterBuff(eBuffState buff, OBJECT* o, const int bufftime = 0);
extern void UnRegisterBuff(eBuffState buff, OBJECT* o);

MovementSkill g_MovementSkill;

DWORD g_dwLatestMagicTick;

const   float   AutoMouseLimitTime = (1.f * 60.f * 60.f);
int   LoadingWorld = 0;
int   ItemHelp = 0;
int   MouseUpdateTime = 0;
int   MouseUpdateTimeMax = 6;
bool  WhisperEnable = true;
bool  ChatWindowEnable = true;
int   InputFrame = 0;
int   EditFlag = EDIT_NONE;
wchar_t  ColorTable[][10] = { L"White", L"Black", L"Red", L"Yellow", L"Green", L"Cyan", L"Blue", L"Magenta" };

int   SelectMonster = 0;
int   SelectModel = 0;
int   SelectMapping = 0;
int   SelectColor = 0;
int   SelectWall = 0;
float SelectMappingAngle = 0.f;
bool  DebugEnable = true;
int   SelectedItem = -1;
int   SelectedNpc = -1;
int   SelectedCharacter = -1;
int   SelectedOperate = -1;
int   Attacking = -1;

int   g_iFollowCharacter = -1;

bool g_bAutoGetItem = false;

float LButtonPopTime = 0.f;
float LButtonPressTime = 0.f;
float RButtonPopTime = 0.f;
float RButtonPressTime = 0.f;

//
int   BrushSize = 0;
int   HeroTile;
int   TargetNpc = -1;
int   TargetType;
int TargetX;
int TargetY;
float TargetAngle;
int   FontHeight;

OBJECT* TradeNpc = NULL;

bool  DontMove = false;
bool  ServerHide = true;
bool  SkillEnable;
bool  MouseOnWindow;
int   KeyState[256];

int    TerrainWallType[TERRAIN_SIZE * TERRAIN_SIZE];
float  TerrainWallAngle[TERRAIN_SIZE * TERRAIN_SIZE];
OBJECT* PickObject = NULL;
vec3_t PickObjectAngle;
float  PickObjectHeight;
bool   PickObjectLockHeight = false;
bool   EnableRandomObject = false;
float  WallAngle;

bool LockInputStatus = false;

bool GuildInputEnable = false;
bool TabInputEnable = false;
bool GoldInputEnable = false;
bool InputEnable = true;
bool g_bScratchTicket = false;

int  InputGold = 0;
int  InputNumber = 2;
int  InputTextWidth = 110;
int  InputIndex = 0;
int  InputResidentNumber = 6;
int  InputTextMax[12] = { MAX_ID_SIZE,MAX_ID_SIZE,MAX_ID_SIZE,30,30,10,14,20,40 };
wchar_t InputText[12][256];
wchar_t InputTextIME[12][4];
char InputTextHide[12];
int  InputLength[12];
wchar_t MacroText[10][256];

constexpr uint64_t MacroCooldownMs = 4000; // 4 Seconds
uint64_t  LastMacroTime = 0;

int  WhisperIDCurrent = 2;
char WhisperID[MAX_WHISPER_ID][256];
DWORD g_dwOneToOneTick = 0;

bool g_bGMObservation = false;

#ifdef LEM_FIX_USER_LOGOUT
bool g_bExit = false;
#endif // LEM_FIX_USER_LOGOUT [lem_2010.8.18]

#ifdef PK_ATTACK_TESTSERVER_LOG
void PrintPKLog(CHARACTER* pCha)
{
    static DWORD dwStartTime = 0;
    static SHORT sKey = 0;

    if (pCha->Key == sKey && (timeGetTime() - dwStartTime < 1000))
    {
        return;
    }

    if (pCha)
    {
        if (pCha->PK >= PVP_MURDERER2 && pCha->Object.Type == KIND_PLAYER)
        {
            g_ErrorReport.Write(L"!!!!!!!!!!!!!!!!! PK !!!!!!!!!!!!!!!\n");
            g_ErrorReport.WriteCurrentTime();
            g_ErrorReport.Write(L" ID(%s) PK(%d) GuildName(%s)\n", pCha->ID, pCha->PK, GuildMark[pCha->GuildMarkIndex].GuildName);
#ifdef CONSOLE_DEBUG
            g_ConsoleDebug->Write(MCD_ERROR, L"[!!! PK !!! : ID(%s) PK(%d) GuildName(%s)\n]", pCha->ID, pCha->PK, GuildMark[pCha->GuildMarkIndex].GuildName);
#endif // CONSOLE_DEBUG

            sKey = pCha->Key;
            dwStartTime = timeGetTime();
        }
    }
}
#endif // PK_ATTACK_TESTSERVER_LOG

bool PressKey(int Key)
{
    if (HIBYTE(GetAsyncKeyState(Key)) == 128)
    {
        if (KeyState[Key] == false)
        {
            KeyState[Key] = true;
            return true;
        }
    }
    else KeyState[Key] = false;
    return false;
}

void ClearInput(BOOL bClearWhisperTarget)
{
    InputIndex = 0;
    InputResidentNumber = -1;
    for (int i = 0; i < 10; i++)
    {
        if (i == 1 && bClearWhisperTarget == FALSE) continue;
        for (int j = 0; j < 256; j++)
            InputText[i][j] = NULL;
        InputLength[i] = 0;
        InputTextHide[i] = 0;
    }

    if (g_iChatInputType == 1)
    {
        g_pSingleTextInputBox->SetText(NULL);
        g_pSinglePasswdInputBox->SetText(NULL);
    }
}

void SetPositionIME_Wnd(float x, float y)
{
    float xRatio_Input = x / 640.f;
    float yRatio_Input = y / 480.f;

    COMPOSITIONFORM comForm;
    comForm.dwStyle = CFS_POINT;
    comForm.ptCurrentPos.x = (long)(WindowWidth * xRatio_Input);
    comForm.ptCurrentPos.y = (long)(WindowHeight * yRatio_Input);
    SetRect(&comForm.rcArea, 0, 0, WindowWidth, WindowHeight);

    HWND hWnd = ImmGetDefaultIMEWnd(g_hWnd);
    ::SendMessage(hWnd, WM_IME_CONTROL, IMC_SETCOMPOSITIONWINDOW, (LPARAM)&comForm);
}

DWORD   g_dwOldConv = IME_CMODE_ALPHANUMERIC;
DWORD   g_dwOldSent = IME_SMODE_AUTOMATIC;
DWORD   g_dwCurrConv = 0;

void SaveIME_Status()
{
    HIMC  data;

    data = ::ImmGetContext(g_hWnd);

    ::ImmGetConversionStatus(data, &g_dwOldConv, &g_dwOldSent);
    ::ImmReleaseContext(g_hWnd, data);
}

//
void SetIME_Status(bool halfShape)
{
    if (g_iChatInputType == 1) return;

    HIMC  data;
    DWORD dwConv, dwSent;

    data = ::ImmGetContext(g_hWnd);

    //  반각.
    dwConv = g_dwOldConv;
    dwSent = g_dwOldSent;
    if (halfShape)
    {
        dwConv = IME_CMODE_ALPHANUMERIC;
        dwSent = IME_SMODE_NONE;
    }
    ::ImmSetConversionStatus(data, dwConv, dwSent);
    ::ImmReleaseContext(g_hWnd, data);
}

bool CheckIME_Status(bool change, int mode)
{
    if (g_iChatInputType == 1) return false;

    HIMC  data;
    DWORD dwConv, dwSent;
    bool  bIme = false;

    data = ::ImmGetContext(g_hWnd);

    ::ImmGetConversionStatus(data, &dwConv, &dwSent);

    if (dwConv != IME_CMODE_ALPHANUMERIC || dwSent != IME_SMODE_NONE)
    {
        bIme = true;

        if ((mode & IME_CONVERSIONMODE) == IME_CONVERSIONMODE)
        {
            g_dwOldConv = dwConv;
        }
        if ((mode & IME_SENTENCEMODE) == IME_SENTENCEMODE)
        {
            g_dwOldSent = dwSent;
        }

        if (change)
        {
            dwConv = IME_CMODE_ALPHANUMERIC;
            dwSent = IME_SMODE_NONE;
            ::ImmSetConversionStatus(data, dwConv, dwSent);
        }
    }
    ::ImmReleaseContext(g_hWnd, data);

    g_dwCurrConv = dwConv;

    return   bIme;
}

void RenderIME_Status()
{
    wchar_t    Text[100];
    if ((g_dwOldConv & IME_CMODE_NATIVE) == IME_CMODE_NATIVE)
    {
        swprintf(Text, L"ENGLISH");
    }
    else
    {
        swprintf(Text, L"ENGLISH");
    }

    g_pRenderText->SetTextColor(255, 230, 210, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(100, 100, Text);

    HIMC  data;
    DWORD dwConv, dwSent;

    data = ::ImmGetContext(g_hWnd);

    ::ImmGetConversionStatus(data, &dwConv, &dwSent);
    ::ImmReleaseContext(g_hWnd, data);

    swprintf(Text, L"Sentence Mode = %d", dwSent);
    g_pRenderText->RenderText(100, 110, Text);

    swprintf(Text, L"Old Sentence Mode = %d", g_dwOldSent);
    g_pRenderText->RenderText(100, 120, Text);

    swprintf(Text, L"LockInputStatus=%d", LockInputStatus);
    g_pRenderText->RenderText(100, 130, Text);
}

void RenderInputText(int x, int y, int Index, int Gold)
{
    if (g_iChatInputType == 1)
    {
        return;
    }
    else if (g_iChatInputType == 0)
    {
        g_pRenderText->SetTextColor(255, 230, 210, 255);
        g_pRenderText->SetBgColor(0);

        SIZE* Size;
        wchar_t Text[256];
        if (InputTextHide[Index] == 1)
        {
            int iTextSize = 0;
            for (unsigned int i = 0; i < wcslen(InputText[Index]); i++)
            {
                Text[i] = '*';
                iTextSize = i;
            }
            Text[iTextSize] = NULL;
        }
        else if (InputTextHide[Index] == 2)
        {
            int iTextSize = 0;
            for (unsigned int i = 0; i < 7; i++)
            {
                Text[i] = InputText[Index][i];
                iTextSize = i;
            }
            for (unsigned int i = 7; i < wcslen(InputText[Index]); i++)
            {
                Text[i] = '*';
                iTextSize = i;
            }
            Text[iTextSize] = NULL;
        }
        else
        {
            wcscpy(Text, InputText[Index]);
        }
        SIZE TextSize;
        g_pRenderText->RenderText(x, y, Text, InputTextWidth, 0, RT3_SORT_LEFT, &TextSize);
        Size = &TextSize;

        if (Index == InputIndex)
            SetPositionIME_Wnd(x + Size->cx, y);

        if (Index == InputIndex && (InputFrame++) % 2 == 0)
        {
            EnableAlphaTest();
            if (wcslen(InputTextIME[Index]) > 0)
            {
                if (InputTextHide[Index] == 1)
                    g_pRenderText->RenderText(x + Size->cx, y, L"**");
                else
                    g_pRenderText->RenderText(x + Size->cx, y, InputTextIME[Index]);
            }
            else
                g_pRenderText->RenderText(x + Size->cx, y, L"_");
        }
    }
}

extern int  AlphaBlendType;

void RenderTipText(int sx, int sy, const wchar_t* Text)
{
    SIZE TextSize = { 0, 0 };
    g_pRenderText->SetFont(g_hFont);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &TextSize);

    int BackupAlphaBlendType = AlphaBlendType;
    EnableAlphaTest();
    glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    RenderColor((float)sx - 2, (float)sy - 3, (float)TextSize.cx / g_fScreenRate_x + 4, (float)1);	// 위
    RenderColor((float)sx - 2, (float)sy - 3, (float)1, (float)TextSize.cy / g_fScreenRate_y + 4);	// 좌
    RenderColor((float)sx - 2 + TextSize.cx / g_fScreenRate_x + 3, (float)sy - 3, (float)1, (float)TextSize.cy / g_fScreenRate_y + 4);
    RenderColor((float)sx - 2, (float)sy - 3 + TextSize.cy / g_fScreenRate_y + 3, (float)TextSize.cx / g_fScreenRate_x + 4, (float)1);

    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    RenderColor((float)sx - 1, (float)sy - 2, (float)TextSize.cx / g_fScreenRate_x + 2, (float)TextSize.cy / g_fScreenRate_y + 2);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(sx, sy, Text);
    switch (BackupAlphaBlendType)
    {
    case 0:
        DisableAlphaBlend();
        break;
    case 1:
        EnableLightMap();
        break;
    case 2:
        EnableAlphaTest();
        break;
    case 3:
        EnableAlphaBlend();
        break;
    case 4:
        EnableAlphaBlendMinus();
        break;
    case 5:
        EnableAlphaBlend2();
        break;
    default:
        DisableAlphaBlend();
        break;
    }
}

typedef struct
{
    wchar_t      Text[256];
    int       LifeTime;
    BYTE      Color;
} NOTICE;

#define MAX_NOTICE 6

int    NoticeCount = 0;
int    NoticeTime = 300;
NOTICE Notice[MAX_NOTICE];

void ScrollNotice()
{
    if (NoticeCount > MAX_NOTICE - 1)
    {
        NoticeCount = MAX_NOTICE - 1;
        for (int i = 1; i < MAX_NOTICE; i++)
        {
            Notice[i - 1].Color = Notice[i].Color;
            wcscpy(Notice[i - 1].Text, Notice[i].Text);
        }
    }
}

void ClearNotice(void)
{
    memset(Notice, 0, sizeof(NOTICE) * MAX_NOTICE);
}

void CreateNotice(wchar_t* Text, int Color)
{
    SIZE Size;
    g_pRenderText->SetFont(g_hFontBold);
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), Text, lstrlen(Text), &Size);

    ScrollNotice();
    Notice[NoticeCount].Color = Color;
    if (Size.cx < 256)
    {
        wcscpy(Notice[NoticeCount++].Text, Text);
    }
    else
    {
        wchar_t TopText[256] = { 0 };
        wchar_t BottomText[256] = { 0 };
        CutText(Text, TopText, BottomText, 256);
        wcscpy(Notice[NoticeCount++].Text, TopText);
        ScrollNotice();
        Notice[NoticeCount].Color = Color;
        wcscpy(Notice[NoticeCount++].Text, BottomText);
    }
    NoticeTime = 300;
}

void MoveNotices()
{
    NoticeTime -= FPS_ANIMATION_FACTOR;
    if (NoticeTime <= 0)
    {
        NoticeTime = 300;
        CreateNotice(L"", 0);
    }
}

float NoticeInverse = 0;

void RenderNotices()
{
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == true)
        return;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

    EnableAlphaTest();

    g_pRenderText->SetFont(g_hFontBold);

    glColor3f(1.f, 1.f, 1.f);
    for (int i = 0; i < MAX_NOTICE; i++)
    {
        NOTICE* n = &Notice[i];
        if (n->Color == 0)
        {
            g_pRenderText->SetBgColor(0, 0, 0, 128);
            if ((int)NoticeInverse % 10 < 5)
            {
                g_pRenderText->SetTextColor(255, 200, 80, 128);
            }
            else
            {
                g_pRenderText->SetTextColor(255, 200, 80, 255);
            }
        }
        else
        {
            g_pRenderText->SetTextColor(100, 255, 200, 255);
            g_pRenderText->SetBgColor(0, 0, 0, 128);
        }

        if (g_bUseChatListBox == TRUE)
        {
            g_pRenderText->RenderText(320, 300 + i * 13, n->Text, 0, 0, RT3_WRITE_CENTER);
        }
        else
        {
            g_pRenderText->RenderText(320, 300 + i * 13, n->Text, 0, 0, RT3_WRITE_CENTER);
        }
    }

    NoticeInverse += FPS_ANIMATION_FACTOR;
}

void CutText(const wchar_t* Text, wchar_t* Text1, wchar_t* Text2, size_t maxLength)
{
    auto sourceText = std::wstring(Text);
    auto halfLength = sourceText.length() / 2;
    size_t splitOffset = sourceText.find_last_of(L' ', halfLength);

    if (splitOffset == std::wstring::npos)
    {
        splitOffset = sourceText.find_first_of(L' ', halfLength);
    }

    if (splitOffset != std::wstring::npos)
    {
        wcsncpy_s(Text1, maxLength, sourceText.substr(0, splitOffset).c_str(), _TRUNCATE);
        wcsncpy_s(Text2, maxLength, sourceText.substr(splitOffset + 1).c_str(), _TRUNCATE);
    }
    else
    {
        // No spaces found, assign everything to Text1
        wcsncpy_s(Text1, maxLength, sourceText.c_str(), _TRUNCATE);
        Text2[0] = L'\0';  // Empty Text2
    }
}

int     WhisperID_Num = 0;
wchar_t WhisperRegistID[MAX_ID_SIZE + 1][10];

bool CheckWhisperLevel(int lvl, wchar_t* text)
{
    int level = CharacterAttribute->Level;

    if (level >= lvl)
        return  true;

    //
    for (int i = 0; i < 10; ++i)
    {
        if (wcscmp(text, WhisperRegistID[i]) == 0)
        {
            return  true;
        }
    }

    g_pSystemLogBox->AddText(GlobalText[479], SEASON3B::TYPE_SYSTEM_MESSAGE);

    return  false;
}

void RegistWhisperID(int lvl, wchar_t* text)
{
    int level = CharacterAttribute->Level;

    if (level < lvl)
    {
        bool noMatch = true;
        for (int i = 0; i < 10; ++i)
        {
            if (wcscmp(text, WhisperRegistID[i]) == 0)
            {
                noMatch = false;
                break;
            }
        }

        if (noMatch)
        {
            wcscpy(WhisperRegistID[WhisperID_Num], text);
            WhisperID_Num++;

            if (WhisperID_Num >= 10)
            {
                WhisperID_Num = 0;
            }
        }
    }
}

void ClearWhisperID(void)
{
    ZeroMemory(WhisperRegistID, sizeof(char) * (MAX_ID_SIZE + 1) * 10);
}

void RenderWhisperID_List(void)
{
    for (int i = 0; i < 10; ++i)
    {
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->SetBgColor(0, 0, 0, 255);
        g_pRenderText->RenderText(100, 100 + (i * 10), WhisperRegistID[i]);
    }
}

typedef struct
{
    wchar_t   ID[32];
    wchar_t      Union[30];
    wchar_t      Guild[30];
    wchar_t      szShopTitle[16];
    char      Color;
    char      GuildColor;
    float       IDLifeTime;
    wchar_t   Text[2][256];
    float       LifeTime[2];
    CHARACTER* Owner;
    int       x, y;
    int       Width;
    int       Height;
    vec3_t    Position;
} CHAT;

#define MAX_CHAT 120

CHAT Chat[MAX_CHAT];

void SetBooleanPosition(CHAT* c)
{
    BOOL bResult[5];
    SIZE Size[5];
    memset(&Size[0], 0, sizeof(SIZE) * 5);

    if (g_isCharacterBuff((&c->Owner->Object), eBuff_GMEffect) || (c->Owner->CtlCode == CTLCODE_20OPERATOR) || (c->Owner->CtlCode == CTLCODE_08OPERATOR))
    {
        g_pRenderText->SetFont(g_hFontBold);
        bResult[0] = GetTextExtentPoint32(g_pRenderText->GetFontDC(), c->ID, lstrlen(c->ID), &Size[0]);
        g_pRenderText->SetFont(g_hFont);
    }
    else
    {
        bResult[0] = GetTextExtentPoint32(g_pRenderText->GetFontDC(), c->ID, lstrlen(c->ID), &Size[0]);
    }

    bResult[1] = GetTextExtentPoint32(g_pRenderText->GetFontDC(), c->Text[0], lstrlen(c->Text[0]), &Size[1]);
    bResult[2] = GetTextExtentPoint32(g_pRenderText->GetFontDC(), c->Text[1], lstrlen(c->Text[1]), &Size[2]);
    bResult[3] = GetTextExtentPoint32(g_pRenderText->GetFontDC(), c->Union, lstrlen(c->Union), &Size[3]);
    bResult[4] = GetTextExtentPoint32(g_pRenderText->GetFontDC(), c->Guild, lstrlen(c->Guild), &Size[4]);

    Size[0].cx += 3;

    if (c->LifeTime[1] > 0)
        c->Width = max(max(max(Size[0].cx, Size[1].cx), max(Size[2].cx, Size[3].cx)), Size[4].cx);

    else if (c->LifeTime[0] > 0)
        c->Width = max(max(Size[0].cx, Size[1].cx), max(Size[3].cx, Size[4].cx));
    else
        c->Width = max(max(Size[0].cx, Size[3].cx), Size[4].cx);
    c->Height = FontHeight * (bResult[0] + bResult[1] + bResult[2] + bResult[3] + bResult[4]);

    if (lstrlen(c->szShopTitle) > 0)
    {
        SIZE sizeT[2];
        g_pRenderText->SetFont(g_hFontBold);

        if (GetTextExtentPoint32(g_pRenderText->GetFontDC(), c->szShopTitle, lstrlen(c->szShopTitle), &sizeT[0]) && GetTextExtentPoint32(g_pRenderText->GetFontDC(), GlobalText[1104], GlobalText.GetStringSize(1104), &sizeT[1]))
        {
            if (c->Width < sizeT[0].cx + sizeT[1].cx)
                c->Width = sizeT[0].cx + sizeT[1].cx;
            c->Height += max(sizeT[0].cy, sizeT[1].cy);
        }
        g_pRenderText->SetFont(g_hFont);
    }
    c->Width /= g_fScreenRate_x;
    c->Height /= g_fScreenRate_y;
}

void SetPlayerColor(BYTE PK)
{
    switch (PK)
    {
    case 0:g_pRenderText->SetTextColor(150, 255, 240, 255); break;//npc
    case 1:g_pRenderText->SetTextColor(100, 120, 255, 255); break;
    case 2:g_pRenderText->SetTextColor(140, 180, 255, 255); break;
    case 3:g_pRenderText->SetTextColor(200, 220, 255, 255); break;//normal
    case 4:g_pRenderText->SetTextColor(255, 150, 60, 255); break; //pk1
    case 5:g_pRenderText->SetTextColor(255, 80, 30, 255); break;  //pk2
    default:g_pRenderText->SetTextColor(255, 0, 0, 255); break;    //pk3
    }
}

	// ※

const int ciSystemColor = 240;

void RenderBoolean(int x, int y, CHAT* c)
{
    if (g_isCharacterBuff((&c->Owner->Object), eBuff_CrywolfNPCHide))
    {
        return;
    }

    if (c->Owner != Hero && battleCastle::IsBattleCastleStart() == true && g_isCharacterBuff((&c->Owner->Object), eBuff_Cloaking))
    {
        if ((Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK))
        {
            if (!(c->Owner->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || c->Owner->EtcPart == PARTS_ATTACK_TEAM_MARK))
            {
                return;
            }
        }
        else if ((Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2))
        {
            if (!(c->Owner->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || c->Owner->EtcPart == PARTS_ATTACK_TEAM_MARK2))
            {
                return;
            }
        }
        else if ((Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3 || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3))
        {
            if (!(c->Owner->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3 || c->Owner->EtcPart == PARTS_ATTACK_TEAM_MARK3))
            {
                return;
            }
        }
        else if ((Hero->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK))
        {
            if (!(c->Owner->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || c->Owner->EtcPart == PARTS_DEFENSE_TEAM_MARK))
            {
                return;
            }
        }
    }

    EnableAlphaTest();
    glColor3f(1.f, 1.f, 1.f);

    if (FontHeight > 32) FontHeight = 32;

    POINT RenderPos = { x, y };
    SIZE RenderBoxSize = { c->Width, c->Height };
    int iLineHeight = FontHeight / g_fScreenRate_y;

    if (IsShopInViewport(c->Owner))
    {
        SIZE TextSize;
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetBgColor(GetShopBGColor(c->Owner));

        g_pRenderText->SetTextColor(GetShopTextColor(c->Owner));
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, GlobalText[1104], 0, iLineHeight, RT3_SORT_LEFT, &TextSize);
        RenderPos.x += TextSize.cx;

        g_pRenderText->SetTextColor(GetShopText2Color(c->Owner));
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, c->szShopTitle, RenderBoxSize.cx - TextSize.cx, iLineHeight, RT3_SORT_LEFT, &TextSize);
        g_pRenderText->SetFont(g_hFont);

        RenderPos.x = x;
        RenderPos.y += iLineHeight;
    }
    else if (::IsStrifeMap(gMapManager.WorldActive) && Hero->m_byGensInfluence != c->Owner->m_byGensInfluence && !::IsGMCharacter())
    {
        if (!c->Owner->GensContributionPoints)
            return;

        if (KIND_PLAYER == c->Owner->Object.Kind && MODEL_PLAYER == c->Owner->Object.Type)
        {
            int tempX = (int)(c->x + c->Width * 0.5f + 20.0f);
            switch (c->Owner->m_byGensInfluence)
            {
            case 1:
                g_pNewUIGensRanking->RanderMark(tempX, y, (SEASON3B::CNewUIGensRanking::GENS_TYPE)c->Owner->m_byGensInfluence, c->Owner->GensRanking, SEASON3B::CNewUIGensRanking::MARK_BOOLEAN, (float)RenderPos.y);
                return;
            case 2:
                g_pNewUIGensRanking->RanderMark(tempX, y, (SEASON3B::CNewUIGensRanking::GENS_TYPE)c->Owner->m_byGensInfluence, c->Owner->GensRanking, SEASON3B::CNewUIGensRanking::MARK_BOOLEAN, (float)RenderPos.y);
                return;
            default:
                break;
            }
        }
    }

    bool bGmMode = false;

    if (g_isCharacterBuff((&c->Owner->Object), eBuff_GMEffect) || (c->Owner->CtlCode == CTLCODE_20OPERATOR) || (c->Owner->CtlCode == CTLCODE_08OPERATOR))
    {
        bGmMode = true;
        g_pRenderText->SetBgColor(30, 30, 30, 200);
        g_pRenderText->SetTextColor(200, 255, 255, 255);
    }

    if (c->Owner == Hero)
    {
        g_pRenderText->SetBgColor(60, 100, 0, 150);
        g_pRenderText->SetTextColor(200, 255, 0, 255);
    }
    else if (c->Owner->GuildMarkIndex == Hero->GuildMarkIndex)
    {
        g_pRenderText->SetBgColor(GetGuildRelationShipBGColor(GR_UNION));
        g_pRenderText->SetTextColor(GetGuildRelationShipTextColor(GR_UNION));
    }
    else
    {
        g_pRenderText->SetBgColor(GetGuildRelationShipBGColor(c->Owner->GuildRelationShip));
        g_pRenderText->SetTextColor(GetGuildRelationShipTextColor(c->Owner->GuildRelationShip));
    }

    if (c->Union && c->Union[0])
    {
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, c->Union, RenderBoxSize.cx, iLineHeight, RT3_SORT_LEFT);
        RenderPos.y += iLineHeight;
    }
    if (c->Guild && c->Guild[0])
    {
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, c->Guild, RenderBoxSize.cx, iLineHeight, RT3_SORT_LEFT);
        RenderPos.y += iLineHeight;
    }

    if (bGmMode)
    {
        g_pRenderText->SetTextColor(100, 250, 250, 255);
    }
    else
    {
        SetPlayerColor(c->Color);
    }

    if (c->x <= MouseX && MouseX < (int)(c->x + c->Width * 640 / WindowWidth) &&
        c->y <= MouseY && MouseY < (int)(c->y + c->Height * 480 / WindowHeight) &&
        InputEnable && Hero->SafeZone && wcscmp(c->ID, Hero->ID) != NULL &&
        (DWORD)WorldTime % 24 < 12)
    {
        unsigned int Temp = g_pRenderText->GetBgColor();
        g_pRenderText->SetBgColor(g_pRenderText->GetTextColor());
        g_pRenderText->SetTextColor(Temp);
    }

    if (bGmMode)
    {
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, c->ID, RenderBoxSize.cx, iLineHeight, RT3_SORT_LEFT);
        RenderPos.y += iLineHeight;
        g_pRenderText->SetFont(g_hFont);
    }
    else
    {
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, c->ID, RenderBoxSize.cx, iLineHeight, RT3_SORT_LEFT);
        RenderPos.y += iLineHeight;
    }

    if (c->GuildColor == 0)
        g_pRenderText->SetBgColor(10, 30, 50, 150);
    else if (c->GuildColor == 1)
        g_pRenderText->SetBgColor(30, 50, 0, 150);
    else if (bGmMode)
        g_pRenderText->SetBgColor(30, 30, 30, 200);
    else
        g_pRenderText->SetBgColor(50, 0, 0, 150);

    DWORD dwTextColor[2];
    BYTE byAlpha[2] = { 255, 255 };
    if ((c->LifeTime[0] > 0 && c->LifeTime[0] < 10))
        byAlpha[0] = 128;
    if ((c->LifeTime[1] > 0 && c->LifeTime[1] < 10))
        byAlpha[1] = 128;

    if (bGmMode)
    {
        dwTextColor[0] = RGBA(250, 200, 50, byAlpha[0]);
        dwTextColor[1] = RGBA(250, 200, 50, byAlpha[1]);
    }
    else
    {
        dwTextColor[0] = RGBA(230, 220, 200, byAlpha[0]);
        dwTextColor[1] = RGBA(230, 220, 200, byAlpha[1]);
    }

    if (c->LifeTime[1] > 0)
    {
        g_pRenderText->SetTextColor(dwTextColor[1]);
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, c->Text[1], RenderBoxSize.cx, iLineHeight, RT3_SORT_LEFT);
        RenderPos.y += iLineHeight;

        g_pRenderText->SetTextColor(dwTextColor[0]);
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, c->Text[0], RenderBoxSize.cx, iLineHeight);
    }
    else if (c->LifeTime[0] > 0)
    {
        g_pRenderText->SetTextColor(dwTextColor[0]);
        g_pRenderText->RenderText(RenderPos.x, RenderPos.y, c->Text[0], RenderBoxSize.cx, iLineHeight);
    }

    if (KIND_PLAYER == c->Owner->Object.Kind && MODEL_PLAYER == c->Owner->Object.Type)
    {
        const int nGensMarkHeight = 18;
        int nGensMarkPosY = (RenderPos.y - y - nGensMarkHeight) / 2 + y;

        if (c->LifeTime[1] > 0)
            RenderPos.y -= iLineHeight;

        if (1 == c->Owner->m_byGensInfluence)
            g_pNewUIGensRanking->RanderMark(x, y, (SEASON3B::CNewUIGensRanking::GENS_TYPE)c->Owner->m_byGensInfluence, c->Owner->GensRanking, SEASON3B::CNewUIGensRanking::MARK_BOOLEAN, (float)RenderPos.y);
        else if (2 == c->Owner->m_byGensInfluence)
            g_pNewUIGensRanking->RanderMark(x, y, (SEASON3B::CNewUIGensRanking::GENS_TYPE)c->Owner->m_byGensInfluence, c->Owner->GensRanking, SEASON3B::CNewUIGensRanking::MARK_BOOLEAN, (float)RenderPos.y);
    }
}
void AddChat(CHAT* c, const wchar_t* chat_text, int flag)
{
    float Time = 0;
    int Length = (int)wcslen(chat_text);
    switch (flag)
    {
    case 0:
        Time = Length * 2 + 160;
        break;
    case 1:
        Time = 1000;
        g_pChatListBox->AddText(c->ID, chat_text, SEASON3B::TYPE_CHAT_MESSAGE);
        break;
    }

    if (Length >= 20)
    {
        CutText(chat_text, c->Text[1], c->Text[0], 256);
        c->LifeTime[0] = Time;
        c->LifeTime[1] = Time;
    }
    else
    {
        memset(c->Text[0], 0, 256);
        wcscpy(c->Text[0], chat_text);
        c->LifeTime[0] = Time;
    }
}

void AddGuildName(CHAT* c, CHARACTER* Owner)
{
    if (IsShopInViewport(Owner))
    {
        std::wstring summary;
        GetShopTitleSummary(Owner, summary);
        wcscpy(c->szShopTitle, summary.c_str());
    }
    else {
        c->szShopTitle[0] = '\0';
    }

    if (Owner->GuildMarkIndex >= 0 && GuildMark[Owner->GuildMarkIndex].UnionName[0])
    {
        if (Owner->GuildRelationShip == GR_UNION)
            swprintf(c->Union, L"<%s> %s", GuildMark[Owner->GuildMarkIndex].UnionName, GlobalText[1295]);
        if (Owner->GuildRelationShip == GR_UNIONMASTER)
        {
            if (Owner->GuildStatus == G_MASTER)
                swprintf(c->Union, L"<%s> %s", GuildMark[Owner->GuildMarkIndex].UnionName, GlobalText[1296]);
            else
                swprintf(c->Union, L"<%s> %s", GuildMark[Owner->GuildMarkIndex].UnionName, GlobalText[1295]);
        }
        else if (Owner->GuildRelationShip == GR_RIVAL)
        {
            if (Owner->GuildStatus == G_MASTER)
                swprintf(c->Union, L"<%s> %s", GuildMark[Owner->GuildMarkIndex].UnionName, GlobalText[1298]);
            else
                swprintf(c->Union, L"<%s> %s", GuildMark[Owner->GuildMarkIndex].UnionName, GlobalText[1297]);
        }
        else if (Owner->GuildRelationShip == GR_RIVALUNION)
            swprintf(c->Union, L"<%s> %s", GuildMark[Owner->GuildMarkIndex].UnionName, GlobalText[1299]);
        else
            swprintf(c->Union, L"<%s>", GuildMark[Owner->GuildMarkIndex].UnionName);
    }
    else
        c->Union[0] = NULL;

    if (Owner->GuildMarkIndex >= 0)
    {
        c->GuildColor = Owner->GuildTeam;

        if (Owner->GuildStatus == G_PERSON)
            swprintf(c->Guild, L"[%s] %s", GuildMark[Owner->GuildMarkIndex].GuildName, GlobalText[1330]);
        else if (Owner->GuildStatus == G_MASTER)
            swprintf(c->Guild, L"[%s] %s", GuildMark[Owner->GuildMarkIndex].GuildName, GlobalText[1300]);
        else if (Owner->GuildStatus == G_SUB_MASTER)
            swprintf(c->Guild, L"[%s] %s", GuildMark[Owner->GuildMarkIndex].GuildName, GlobalText[1301]);
        else if (Owner->GuildStatus == G_BATTLE_MASTER)
            swprintf(c->Guild, L"[%s] %s", GuildMark[Owner->GuildMarkIndex].GuildName, GlobalText[1302]);
        else
            swprintf(c->Guild, L"[%s]", GuildMark[Owner->GuildMarkIndex].GuildName);
    }
    else
    {
        c->GuildColor = 0;
        c->Guild[0] = NULL;
    }
}

void CreateChat(wchar_t* character_name, const wchar_t* chat_text, CHARACTER* Owner, int Flag, int SetColor)
{
    OBJECT* o = &Owner->Object;
    if (!o->Live || !o->Visible) return;

    int Color;
    if (SetColor != -1)
    {
        Color = SetColor;
    }
    else
    {
        Color = Owner->PK;
        if (o->Kind == KIND_NPC)
            Color = 0;
    }

    for (int i = 0; i < MAX_CHAT; i++)
    {
        CHAT* c = &Chat[i];
        if (c->Owner == Owner)
        {
            wcscpy(c->ID, character_name);
            c->Color = Color;
            AddGuildName(c, Owner);
            if (wcslen(chat_text) == 0)
            {
                c->IDLifeTime = 10;
            }
            else
            {
                if (c->LifeTime[0] > 0)
                {
                    wcscpy(c->Text[1], c->Text[0]);
                    c->LifeTime[1] = c->LifeTime[0];
                }
                c->Owner = Owner;
                AddChat(c, chat_text, Flag);
            }
            return;
        }
    }

    for (int i = 0; i < MAX_CHAT; i++)
    {
        CHAT* c = &Chat[i];
        if (c->IDLifeTime <= 0 && c->LifeTime[0] <= 0)
        {
            c->Owner = Owner;
            wcscpy(c->ID, character_name);
            c->Color = Color;
            AddGuildName(c, Owner);
            if (wcslen(chat_text) == 0)
            {
                c->IDLifeTime = 100;
            }
            else
            {
                AddChat(c, chat_text, Flag);
            }
            return;
        }
    }
}

int CreateChat(wchar_t* character_name, const wchar_t* chat_text, OBJECT* Owner, int Flag, int SetColor)
{
    OBJECT* o = Owner;
    if (!o->Live || !o->Visible) return 0;

    int Color = 0;

    if (SetColor != -1)
    {
        Color = SetColor;
    }

    for (int i = 0; i < MAX_CHAT; i++)
    {
        CHAT* c = &Chat[i];
        if (c->IDLifeTime <= 0 && c->LifeTime[0] <= 0)
        {
            c->Owner = NULL;
            wcscpy(c->ID, character_name);
            c->Color = Color;
            c->GuildColor = 0;
            c->Guild[0] = NULL;
            AddChat(c, chat_text, 0);
            c->LifeTime[0] = Flag;

            Vector(o->Position[0], o->Position[1], o->Position[2] + o->BoundingBoxMax[2] + 60.f, c->Position);
            return c->LifeTime[0];
        }
    }

    return 0;
}

void AssignChat(wchar_t* character_name, const wchar_t* chat_text, int flag)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if (o->Live && o->Kind == KIND_PLAYER)
        {
            if (wcscmp(c->ID, character_name) == NULL)
            {
                CreateChat(character_name, chat_text, c, flag);
                return;
            }
        }
    }

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if (o->Live && o->Kind == KIND_MONSTER)
        {
            if (wcscmp(c->ID, character_name) == NULL)
            {
                CreateChat(character_name, chat_text, c, flag);
                return;
            }
        }
    }
}

void MoveChat()
{
    for (int i = 0; i < MAX_CHAT; i++)
    {
        CHAT* c = &Chat[i];
        if (c->IDLifeTime > 0)
            c->IDLifeTime -= FPS_ANIMATION_FACTOR;
        if (c->LifeTime[0] > 0)
            c->LifeTime[0]-=FPS_ANIMATION_FACTOR;
        if (c->LifeTime[1] > 0)
            c->LifeTime[1]-=FPS_ANIMATION_FACTOR;
        if (c->Owner != NULL && (!c->Owner->Object.Live || !c->Owner->Object.Visible))
        {
            c->IDLifeTime = 0;
            c->LifeTime[0] = 0;
            c->LifeTime[1] = 0;
        }
        if (c->LifeTime[0] > 0) {
            DisableShopTitleDraw(c->Owner);
        }
        if (c->Owner != NULL && c->LifeTime[0] <= 0) {
            EnableShopTitleDraw(c->Owner);
        }
    }
}

BYTE DebugText[10][256];
int  DebugTextLength[10];
char DebugTextCount = 0;

int RenderDebugText(int y)
{
    wchar_t Text[100];
    int Width = 16;
    for (int i = 0; i < min(DebugTextCount, 10); i++)
    {
        int  Type = 0;
        int  Count = 0;
        int  x = 0;
        bool Hex = true;
        int  SizeByte = 1;
        for (int j = 0; j < DebugTextLength[i]; j++)
        {
            glColor3f(0.6f, 0.6f, 0.6f);
            if (j == 0)
            {
                if (DebugText[i][j] == 0xc2) SizeByte = 2;
            }
            if (j == 2)
            {
                if (SizeByte == 1)
                {
                    Type = DebugText[i][j];
                    glColor3f(1.f, 1.f, 1.f);
                    if (DebugText[i][j] == 0x00)
                    {
                        x = Width * 4;
                    }
                }
            }
            if (j == 3)
            {
                if (SizeByte == 2)
                {
                    Type = DebugText[i][j];
                    glColor3f(1.f, 1.f, 1.f);
                }
            }

            SIZE TextSize;
            if (Hex)
            {
                swprintf(Text, L"%0.2x", DebugText[i][j]);
                g_pRenderText->RenderText(x, y, Text, 0, 0, RT3_SORT_CENTER, &TextSize);
            }
            else
            {
                swprintf(Text, L"%c", DebugText[i][j]);
                g_pRenderText->RenderText(x, y, Text, 0, 0, RT3_SORT_CENTER, &TextSize);
            }
            if (Hex)
            {
                x += Width;
            }
            else
            {
                x += TextSize.cx;
            }
            Count++;
        }
        y += 12;
    }
    return y;
}

int SearchArrow()
{
    if (gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_ELF)
    {
        int Arrow = 0;

        if (gCharacterManager.GetEquipedBowType() == BOWTYPE_BOW)
        {
            Arrow = ITEM_ARROWS;
        }
        else if (gCharacterManager.GetEquipedBowType() == BOWTYPE_CROSSBOW)
        {
            Arrow = ITEM_BOLT;
        }

        int iIndex = g_pMyInventory->FindItemReverseIndex(Arrow);
        return iIndex;
    }
    return -1;
}

int SearchArrowCount()
{
    int Count = 0;
    if (gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_ELF)
    {
        int Arrow = 0;

        if (gCharacterManager.GetEquipedBowType() == BOWTYPE_BOW)
        {
            Arrow = ITEM_ARROWS;
        }
        else if (gCharacterManager.GetEquipedBowType() == BOWTYPE_CROSSBOW)
        {
            Arrow = ITEM_BOLT;
        }
        Count = g_pMyInventory->GetNumItemByType(Arrow);
    }
    return Count;
}

bool CheckTile(CHARACTER* c, OBJECT* o, float Range)
{
    if (!c || !o) return false;

    float dx = o->Position[0] - (TargetX * TERRAIN_SCALE + TERRAIN_SCALE * 0.5f);
    float dy = o->Position[1] - (TargetY * TERRAIN_SCALE + TERRAIN_SCALE * 0.5f);

    // Compare squared distance with squared range to avoid sqrt calculation
    float squaredDistance = dx * dx + dy * dy;
    float squaredRange = (TERRAIN_SCALE * Range) * (TERRAIN_SCALE * Range);

    return squaredDistance <= squaredRange;
}

bool CheckWall(int sx1, int sy1, int sx2, int sy2)
{
    int Index = TERRAIN_INDEX_REPEAT(sx1, sy1);

    int nx1, ny1, d1, d2, len1, len2;
    int px1 = sx2 - sx1;
    int py1 = sy2 - sy1;
    if (px1 < 0) { px1 = -px1; nx1 = -1; }
    else nx1 = 1;
    if (py1 < 0) { py1 = -py1; ny1 = -TERRAIN_SIZE; }
    else ny1 = TERRAIN_SIZE;
    if (px1 > py1) { len1 = px1; len2 = py1; d1 = ny1; d2 = nx1; }
    else { len1 = py1; len2 = px1; d1 = nx1; d2 = ny1; }

    int error = 0, count = 0;
    do {
        int _type = (SelectedCharacter >= 0 ? CharactersClient[SelectedCharacter].Object.Type : 0);
        if ((_type != MODEL_EVIL_GATE && _type != MODEL_LION_GATE && _type != MODEL_STAR_GATE && _type != MODEL_RUSH_GATE)
            && (TerrainWall[Index] >= TW_NOMOVE && (TerrainWall[Index] & TW_ACTION) != TW_ACTION && (TerrainWall[Index] & TW_HEIGHT) != TW_HEIGHT && (TerrainWall[Index] & TW_CAMERA_UP) != TW_CAMERA_UP))
        {
            return false;
        }
        error += len2;
        if (error > len1 / 2)
        {
            Index += d1;
            error -= len1;
        }
        Index += d2;
    } while (++count <= len1);
    return true;
}

bool CheckAttack_Fenrir(CHARACTER* c)
{
    if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
    {
        return false;
    }

    if (gMapManager.InChaosCastle() == true && c != Hero)
    {
        return true;
    }
    else if (::IsStrifeMap(gMapManager.WorldActive) && c != Hero && c->m_byGensInfluence != Hero->m_byGensInfluence)
    {
        if (((wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == NULL) || (g_pPartyManager->IsPartyMember(SelectedCharacter))) && (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128))
        {
            return true;
        }
        else if ((wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) != NULL) && !g_pPartyManager->IsPartyMember(SelectedCharacter))
        {
            return true;
        }
    }
    if (c->Object.Kind == KIND_MONSTER)
    {
        if (EnableGuildWar && EnableSoccer)
        {
            return true;
        }
        else if (EnableGuildWar)
        {
            return false;
        }

        return true;
    }
    else if (c->Object.Kind == KIND_PLAYER)
    {
        if (battleCastle::IsBattleCastleStart())
        {
            if ((Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK
                || Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2
                || Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3
                || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2
                || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3
                ) &&
                (c->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || c->EtcPart == PARTS_DEFENSE_TEAM_MARK))
            {
                if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if ((Hero->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK)
                && (c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || c->EtcPart == PARTS_ATTACK_TEAM_MARK
                    || c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3
                    || c->EtcPart == PARTS_ATTACK_TEAM_MARK2 || c->EtcPart == PARTS_ATTACK_TEAM_MARK3))
            {
                if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if (g_isCharacterBuff((&Hero->Object), eBuff_CastleRegimentAttack1)
                || g_isCharacterBuff((&Hero->Object), eBuff_CastleRegimentAttack2)
                || g_isCharacterBuff((&Hero->Object), eBuff_CastleRegimentAttack3))
            {
                OBJECT* o = &c->Object;
                if (!g_isCharacterBuff(o, eBuff_CastleRegimentAttack1)
                    && !g_isCharacterBuff(o, eBuff_CastleRegimentAttack2)
                    && !g_isCharacterBuff(o, eBuff_CastleRegimentAttack3))
                {
                    if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else if (g_isCharacterBuff((&Hero->Object), eBuff_CastleRegimentDefense))
            {
                OBJECT* o = &c->Object;

                if (!g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                {
                    if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }

        if (c->GuildRelationShip == GR_RIVAL || c->GuildRelationShip == GR_RIVALUNION)		//박종훈 표시
        {
            return true;
        }

        if (EnableGuildWar && c->PK >= PVP_MURDERER2 && c->GuildMarkIndex != -1
            && wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == NULL)
        {
            return  false;
        }

        else if (g_DuelMgr.IsDuelEnabled())
        {
            if (g_DuelMgr.IsDuelPlayer(c, DUEL_ENEMY))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (EnableGuildWar)
        {
            if (c->GuildTeam == 2 && c != Hero)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (c->PK >= PVP_MURDERER2 || (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128 && c != Hero))
        {
            return true;
        }
        else if (gMapManager.IsCursedTemple() && !g_CursedTemple->IsPartyMember(SelectedCharacter))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return false;
}

bool CheckAttack()
{
    if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
    {
        return false;
    }

    if (SelectedCharacter < 0 || SelectedCharacter >= MAX_CHARACTERS_CLIENT)
    {
        return false;
    }

    if (IsGMCharacter() && IsNonAttackGM() == true)
    {
        return false;
    }

    CHARACTER* c = &CharactersClient[SelectedCharacter];

    if (c->Dead > 0)
    {
        return false;
    }

    if (gMapManager.InChaosCastle() == true && c != Hero)
    {
        return true;
    }
    else if (::IsStrifeMap(gMapManager.WorldActive) && c != Hero && c->m_byGensInfluence != Hero->m_byGensInfluence)
    {
        if (g_pCommandWindow->GetMouseCursor() == CURSOR_IDSELECT)
        {
            return false;
        }
        if (HIBYTE(GetAsyncKeyState(VK_MENU)) == 128)
        {
            return false;
        }

        if (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128)
        {
            if (EnableGuildWar)
            {
                if (c->GuildTeam == 2 && c != Hero)
                    return true;
                else
                    return false;
            }
            else
                return true;
        }
        else if (wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == NULL)
        {
            if (g_pPartyManager->IsPartyMember(SelectedCharacter))
            {
                return false;
            }
            if (EnableGuildWar)
            {
                if (c->GuildTeam == 2 && c != Hero)
                    return true;
                else
                    return false;
            }
            if (c->GuildRelationShip == GR_NONE)
                return true;
            else
                return false;
        }
        else if ((c->GuildRelationShip == GR_UNION) || (c->GuildRelationShip == GR_UNIONMASTER))
        {
            return false;
        }
        else if (EnableGuildWar)
        {
            if (c->GuildTeam == 2 && c != Hero)
                return true;
            else/* if(c->GuildRelationShip == GR_NONE)*/
                return false;
        }
        else if (g_pPartyManager->IsPartyMember(SelectedCharacter))
        {
            if ((c->GuildRelationShip == GR_RIVAL) || (c->GuildRelationShip == GR_RIVALUNION))
            {
                return true;
            }
            else
                return false;
        }
        else
        {
            return true;
        }
    }

    if (c->Object.Kind == KIND_MONSTER)
    {
        if (EnableGuildWar && EnableSoccer)
        {
            return true;
        }
        else if (EnableGuildWar)
        {
            return false;
        }
        else if (g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
        {
            return false;
        }

        return true;
    }
    else if (c->Object.Kind == KIND_PLAYER)
    {
        if (battleCastle::IsBattleCastleStart())
        {
            if ((Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK
                || Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3
                || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2 || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3)
                && (c->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || c->EtcPart == PARTS_DEFENSE_TEAM_MARK))
            {
                if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if ((Hero->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK)
                && (c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || c->EtcPart == PARTS_ATTACK_TEAM_MARK
                    || c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3
                    || c->EtcPart == PARTS_ATTACK_TEAM_MARK2 || c->EtcPart == PARTS_ATTACK_TEAM_MARK3))
            {
                if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else if (g_isCharacterBuff((&Hero->Object), eBuff_CastleRegimentAttack1)
                || g_isCharacterBuff((&Hero->Object), eBuff_CastleRegimentAttack2)
                || g_isCharacterBuff((&Hero->Object), eBuff_CastleRegimentAttack3))
            {
                OBJECT* o = &c->Object;

                if (!g_isCharacterBuff(o, eBuff_CastleRegimentAttack1)
                    && !g_isCharacterBuff(o, eBuff_CastleRegimentAttack2)
                    && !g_isCharacterBuff(o, eBuff_CastleRegimentAttack3))
                {
                    if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else if (g_isCharacterBuff((&Hero->Object), eBuff_CastleRegimentDefense))
            {
                OBJECT* o = &c->Object;

                if (!g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                {
                    if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }

        if (c->GuildRelationShip == GR_RIVAL || c->GuildRelationShip == GR_RIVALUNION)
        {
            return true;
        }

        if (EnableGuildWar && c->PK >= PVP_MURDERER2 && c->GuildMarkIndex != -1
            && wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == NULL)
        {
            return  false;
        }
        else if (g_DuelMgr.IsDuelEnabled())
        {
            if (g_DuelMgr.IsDuelPlayer(c, DUEL_ENEMY))
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
        {
            return false;
        }
        else if (EnableGuildWar)
        {
            if (c->GuildTeam == 2 && c != Hero)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (c->PK >= PVP_MURDERER2 || (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128 && c != Hero))
        {
            return true;
        }
        else if (gMapManager.IsCursedTemple() && !g_CursedTemple->IsPartyMember(SelectedCharacter))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return false;
}

int	getTargetCharacterKey(CHARACTER* c, int selected)
{
    if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
    {
        return -1;
    }

    if (c != Hero)
    {
        return -1;
    }

    if (selected < 0 || selected >= MAX_CHARACTERS_CLIENT)
    {
        return -1;
    }

    CHARACTER* sc = &CharactersClient[selected];

    if (gMapManager.InChaosCastle() == true)
    {
        return sc->Key;
    }

    if (EnableGuildWar && sc->PK >= PVP_MURDERER2 && sc->GuildMarkIndex != -1 && wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[sc->GuildMarkIndex].GuildName) == NULL)
    {
        return  -1;
    }

    if (g_DuelMgr.IsDuelEnabled())
    {
        if (g_DuelMgr.IsDuelPlayer(sc, DUEL_ENEMY))
        {
            return sc->Key;
        }

        return -1;
    }

    if (sc->GuildRelationShip == GR_RIVAL || sc->GuildRelationShip == GR_RIVALUNION)
    {
        return sc->Key;
    }

    if (EnableGuildWar)
    {
        if (sc->GuildTeam == 2 && sc != Hero)
        {
            return sc->Key;
        }

        return -1;
    }

    if (::IsStrifeMap(gMapManager.WorldActive) && sc != Hero && sc->m_byGensInfluence != Hero->m_byGensInfluence && HIBYTE(GetAsyncKeyState(VK_MENU)) != 128)
    {
        if (sc->GuildRelationShip == GR_NONE && !g_pPartyManager->IsPartyMember(SelectedCharacter))
        {
            return sc->Key;
        }

        if ((wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == NULL) ||
            g_pPartyManager->IsPartyMember(SelectedCharacter))
        {
            if (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128)
            {
                return sc->Key;
            }

            return -1;
        }
    }

    if ((sc->PK >= PVP_MURDERER2 && sc->Object.Type == KIND_PLAYER) || (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128 && sc != Hero))
    {
        return sc->Key;
    }

    if (gMapManager.IsCursedTemple())
    {
        if (g_CursedTemple->IsPartyMember(selected))
        {
            return -1;
        }

        return sc->Key;
    }

    return sc->Key;
}

void SendCharacterMove(unsigned short Key, float Angle, unsigned char PathNum, unsigned char* PathX, unsigned char* PathY, unsigned char TargetX, unsigned char TargetY)
{
    if (PathNum < 1)
        return;

    if (PathNum >= MAX_PATH_FIND)
    {
        PathNum = MAX_PATH_FIND - 1;
    }

    BYTE PathNew[8]{};
    BYTE Dir = 0;
    for (int i = 1; i < PathNum; i++)
    {
        Dir = 0;
        for (int j = 0; j < 8; j++) // loop to find the direction of this step
        {
            if (DirTable[j * 2] == (PathX[i] - PathX[i - 1]) && DirTable[j * 2 + 1] == (PathY[i] - PathY[i - 1]))
            {
                Dir = j;
                break;
            }
        }

        const auto path_index = ((i + 1) / 2) - 1;
        if (i % 2 == 1)
        {
            PathNew[path_index] |= Dir << 4;
        }
        else
        {
            PathNew[path_index] |= Dir;
        }
    }

    if (PathNum == 1)
    {
        // For example, it's 1 when the character stops walking by starting a skill.
        // Then we just send the direction of the character and no steps.
        Dir = ((BYTE)((Angle + 22.5f) / 360.f * 8.f + 1.f) % 8);
    }

    SocketClient->ToGameServer()->SendWalkRequest(PathX[0], PathY[0], PathNum - 1, Dir, PathNew, PathNum / 2);
}

void LetHeroStop(CHARACTER* c, BOOL bSetMovementFalse)
{
    BYTE PathX[1];
    BYTE PathY[1];
    PathX[0] = (Hero->PositionX);
    PathY[0] = (Hero->PositionY);

    SendCharacterMove(Hero->Key, Hero->Object.Angle[2], 1, PathX, PathY, TargetX, TargetY);

    if (c != NULL && bSetMovementFalse == TRUE)
    {
        c->Movement = false;
    }
}

void SetCharacterPos(CHARACTER* c, BYTE posX, BYTE posY, vec3_t position)
{
    BYTE PathX[1];
    BYTE PathY[1];
    PathX[0] = posX;
    PathY[0] = posY;

    c->PositionX = PathX[0];

    c->PositionY = PathY[0];

    VectorCopy(position, c->Object.Position);

    SendCharacterMove(c->Key, c->Object.Angle[2], 1, PathX, PathY, PathX[0], PathY[0]);
}

bool CastWarriorSkill(CHARACTER* c, OBJECT* o, ITEM* p, ActionSkillType iSkill)
{
    if (c == NULL)	return false;
    if (o == NULL)	return false;
    //if (p == NULL)	return false;
    bool Success = false;

    if (SelectedCharacter < 0 || SelectedCharacter >= MAX_CHARACTERS_CLIENT)
    {
        return false;
    }

    TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
    TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);

    g_MovementSkill.m_bMagic = FALSE;
    g_MovementSkill.m_iSkill = iSkill;
    g_MovementSkill.m_iTarget = SelectedCharacter;
    float Distance = gSkillManager.GetSkillDistance(iSkill, c) * 1.2f;

    if ((gMapManager.InBloodCastle() == true)
        && ((iSkill >= AT_SKILL_FALLING_SLASH && iSkill <= AT_SKILL_SLASH)
            || iSkill == AT_SKILL_FALLING_SLASH_STR
            || iSkill == AT_SKILL_LUNGE_STR
            || iSkill == AT_SKILL_CYCLONE_STR
            || iSkill == AT_SKILL_CYCLONE_STR_MG
            || iSkill == AT_SKILL_SLASH_STR
            ))
    {
        Distance = 1.8f;
    }

    if (CheckTile(c, o, Distance))
    {
        UseSkillWarrior(c, o);
        Success = true;
    }
    else if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
    {
        c->Movement = true;
        c->MovementType = MOVEMENT_SKILL;
        SendMove(c, o);
    }

    return (Success);
}

void SendRequestMagic(int Type, int Key)
{
    if (!IsCanBCSkill(Type))
        return;

    if (Type == 40 || Type == 263 || Type == 261 || abs((int)(GetTickCount() - g_dwLatestMagicTick)) > 300)
    {
        g_dwLatestMagicTick = GetTickCount();
        SocketClient->ToGameServer()->SendTargetedSkill(Type, Key);
        g_ConsoleDebug->Write(MCD_SEND, L"0x19 [SendRequestMagic(%d %d)]", Type, Key);
    }
}

extern int CurrentSkill;

inline WORD g_byLastSkillSerialNumber = 0;
inline BYTE MakeSkillSerialNumber(BYTE* pSerialNumber)
{
    if (pSerialNumber == NULL) return 0;

    ++g_byLastSkillSerialNumber;
    if (g_byLastSkillSerialNumber > 50) g_byLastSkillSerialNumber = 1;

    *pSerialNumber = g_byLastSkillSerialNumber;
    return g_byLastSkillSerialNumber;
}

void SendRequestMagicContinue(int Type, int x, int y, int Angle, BYTE Dest, BYTE Tpos, WORD TKey, BYTE* pSkillSerial)
{
    CurrentSkill = Type;

    SocketClient->ToGameServer()->SendAreaSkill(Type, x, y, Angle, TKey, MakeSkillSerialNumber(pSkillSerial));

    g_ConsoleDebug->Write(MCD_SEND, L"0x1E [SendRequestMagicContinue]");
}

bool SkillWarrior(CHARACTER* c, ITEM* p)
{
    OBJECT* o = &c->Object;
    if (o->Type == MODEL_PLAYER)
    {
        if (o->CurrentAction == PLAYER_DEFENSE1) return false;
        if (o->CurrentAction >= PLAYER_ATTACK_SKILL_SWORD1 && o->CurrentAction <= PLAYER_ATTACK_SKILL_SWORD4 || o->CurrentAction == PLAYER_ATTACK_TWO_HAND_SWORD_TWO) return false;
    }
    else
    {
        if (o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2) return false;
    }
    auto Skill = CharacterAttribute->Skill[g_MovementSkill.m_iSkill];
    if (Skill == AT_SKILL_RIDER
        || Skill == AT_SKILL_FIRE_SLASH
        || Skill == AT_SKILL_FIRE_SLASH_STR
        || Skill == AT_SKILL_TWISTING_SLASH
        || Skill == AT_SKILL_TWISTING_SLASH_STR
        || Skill == AT_SKILL_TWISTING_SLASH_STR_MG
        || Skill == AT_SKILL_TWISTING_SLASH_MASTERY
        || Skill == AT_SKILL_DEATHSTAB
        || Skill == AT_SKILL_DEATHSTAB_STR
        || (Skill == AT_SKILL_IMPALE && (Hero->Helper.Type == MODEL_HORN_OF_UNIRIA || Hero->Helper.Type == MODEL_HORN_OF_DINORANT || Hero->Helper.Type == MODEL_DARK_HORSE_ITEM || Hero->Helper.Type == MODEL_HORN_OF_FENRIR))
        || Skill == AT_SKILL_FORCE
        || Skill == AT_SKILL_FORCE_WAVE
        || Skill == AT_SKILL_FORCE_WAVE_STR
        || Skill == AT_SKILL_FIREBURST
        || Skill == AT_SKILL_FIREBURST_STR
        || Skill == AT_SKILL_FIREBURST_MASTERY
        || Skill == AT_SKILL_RUSH
        || Skill == AT_SKILL_SPIRAL_SLASH
        || Skill == AT_SKILL_SPACE_SPLIT
        )
    {
        switch (Skill)
        {
        case AT_SKILL_IMPALE:
            if (!(o->Type == MODEL_PLAYER && Hero->Weapon[0].Type != -1 &&
                (Hero->Weapon[0].Type >= MODEL_SPEAR && Hero->Weapon[0].Type < MODEL_SPEAR + MAX_ITEM_INDEX)))
            {
                return false;
            }
            break;
        case AT_SKILL_DEATHSTAB:
        case AT_SKILL_DEATHSTAB_STR:
            if (!(Hero->Weapon[0].Type != -1 && (Hero->Weapon[0].Type < MODEL_STAFF || Hero->Weapon[0].Type >= MODEL_STAFF + MAX_ITEM_INDEX)))
            {
                return false;
            }
            break;
        case AT_SKILL_SPIRAL_SLASH:
            if (Hero->Weapon[0].Type < MODEL_SWORD || Hero->Weapon[0].Type >= MODEL_SWORD + MAX_ITEM_INDEX)
            {
                return false;
            }
            break;
        }
        int iMana, iSkillMana;
        gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);
        if (CharacterAttribute->Mana < iMana)
        {
            int Index = g_pMyInventory->FindManaItemIndex();

            if (Index != -1)
            {
                SendRequestUse(Index, 0);
            }
            return false;
        }
        if (iSkillMana > CharacterAttribute->SkillMana)
        {
            return false;
        }

        if (!gSkillManager.CheckSkillDelay(g_MovementSkill.m_iSkill))
        {
            return false;
        }
        if (CheckAttack())
        {
            return (CastWarriorSkill(c, o, p, Skill));
        }
    }


    auto baseSkill = gSkillManager.MasterSkillToBaseSkillIndex(Skill);
    bool Success = false;
    for (int i = 0; i < p->SpecialNum; i++)
    {
        if (baseSkill == p->Special[i]) // current skill is available as weapon skill?
        {
            int iMana;
            gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL);
            if (CharacterAttribute->Mana < iMana)
            {
                int Index = g_pMyInventory->FindManaItemIndex();

                if (Index != -1)
                {
                    SendRequestUse(Index, 0);
                }
                continue;
            }

            if (!gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
            {
                continue;
            }

            switch (Skill)
            {
            case AT_SKILL_BLOCKING:
                c->Movement = false;
                if (o->Type == MODEL_PLAYER)
                    SetAction(o, PLAYER_DEFENSE1);
                else
                    SetPlayerAttack(c);
                SendRequestMagic(Skill, Hero->Key);
                Success = true;
                break;
            case AT_SKILL_FALLING_SLASH:
            case AT_SKILL_FALLING_SLASH_STR:
            case AT_SKILL_LUNGE:
            case AT_SKILL_LUNGE_STR:
            case AT_SKILL_UPPERCUT:
            case AT_SKILL_CYCLONE:
            case AT_SKILL_CYCLONE_STR:
            case AT_SKILL_CYCLONE_STR_MG:
            case AT_SKILL_SLASH:
            case AT_SKILL_SLASH_STR:
            case AT_SKILL_RIDER:
                if (CheckAttack())
                    Success = CastWarriorSkill(c, o, p, Skill);
                break;
            }
        }
    }

    if (Skill == AT_SKILL_FIRE_SCREAM
        || Skill == AT_SKILL_FIRE_SCREAM_STR
        || Skill == AT_SKILL_GAOTIC
        )
    {
        int iMana;
        gSkillManager.GetSkillInformation(Skill, 1, nullptr, &iMana, nullptr);
        if (CharacterAttribute->Mana < iMana)
        {
            int Index = g_pMyInventory->FindManaItemIndex();

            if (Index != -1)
            {
                SendRequestUse(Index, 0);
            }
            return Success;
        }

        float distance = gSkillManager.GetSkillDistance(Skill, c);
        if (CheckTile(c, o, distance))
        {
            o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
            WORD TKey = 0xffff;

            if (g_MovementSkill.m_iTarget != -1)
            {
                TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
            }
        }
    }

    return Success;
}

void UseSkillWarrior(CHARACTER* c, OBJECT* o)
{
    auto Skill = g_MovementSkill.m_bMagic ? CharacterAttribute->Skill[g_MovementSkill.m_iSkill] : static_cast<ActionSkillType>(g_MovementSkill.m_iSkill);
    LetHeroStop();
    c->Movement = false;
    if (o->Type == MODEL_PLAYER)
    {
        SetAttackSpeed();

        switch (Skill)
        {
        case AT_SKILL_IMPALE:
            if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
                SetAction(o, PLAYER_FENRIR_ATTACK_SPEAR);
            else
                SetAction(o, PLAYER_ATTACK_SKILL_SPEAR);
            break;
        case AT_SKILL_DEATHSTAB:
        case AT_SKILL_DEATHSTAB_STR:
            SetAction(o, PLAYER_ATTACK_DEATHSTAB);
            break;
        case AT_SKILL_TWISTING_SLASH:
        case AT_SKILL_TWISTING_SLASH_STR:
        case AT_SKILL_TWISTING_SLASH_STR_MG:
        case AT_SKILL_TWISTING_SLASH_MASTERY:
        case AT_SKILL_FIRE_SLASH:
        case AT_SKILL_FIRE_SLASH_STR:
            SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);
            break;
        case AT_SKILL_RIDER:
            if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            else
                SetAction(o, PLAYER_SKILL_RIDER);
            break;
        case AT_SKILL_FIRE_SCREAM:
        case AT_SKILL_FIRE_SCREAM_STR:
            break;
        case AT_SKILL_GAOTIC:
            break;
        case AT_SKILL_FORCE:
        case AT_SKILL_FORCE_WAVE:
        case AT_SKILL_FORCE_WAVE_STR:
        case AT_SKILL_FIREBURST:
        case AT_SKILL_FIREBURST_STR:
        case AT_SKILL_FIREBURST_MASTERY:
        case AT_SKILL_SPACE_SPLIT:
            break;
        case AT_SKILL_RUSH:
            SetAction(o, PLAYER_ATTACK_RUSH);
            break;
        default:
            if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
            {
                auto baseSkill = gSkillManager.MasterSkillToBaseSkillIndex(Skill);
                SetAction(o, PLAYER_ATTACK_SKILL_SWORD1 + baseSkill - AT_SKILL_FALLING_SLASH);
            }
            break;
        }
    }
    else
    {
        SetPlayerAttack(c);
    }

    vec3_t Light;
    Vector(1.f, 1.f, 1.f, Light);

    if (Skill != AT_SKILL_FORCE
        && Skill != AT_SKILL_FORCE_WAVE
        && Skill != AT_SKILL_FORCE_WAVE_STR
        && Skill != AT_SKILL_FIREBURST
        && Skill != AT_SKILL_FIREBURST_STR
        && Skill != AT_SKILL_FIREBURST_MASTERY
        && Skill != AT_SKILL_SPACE_SPLIT
        && Skill != AT_SKILL_FIRE_SCREAM
        && Skill != AT_SKILL_FIRE_SCREAM_STR
        && Skill != AT_SKILL_KILLING_BLOW
        && Skill != AT_SKILL_KILLING_BLOW_STR
        && Skill != AT_SKILL_KILLING_BLOW_MASTERY
        && Skill != AT_SKILL_BEAST_UPPERCUT
        && Skill != AT_SKILL_BEAST_UPPERCUT_STR
        && Skill != AT_SKILL_BEAST_UPPERCUT_MASTERY
        )
    {
        CreateParticle(BITMAP_SHINY + 2, o->Position, o->Angle, Light, 0, 0.f, o);
        PlayBuffer(static_cast<ESound>(SOUND_BRANDISH_SWORD01 + rand() % 2));
    }

    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, c->TargetPosition);
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
    }

    if (Skill != AT_SKILL_GAOTIC)
    {
        WORD TKey = 0xffff;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
		{
			TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
		}

        if (Skill == AT_SKILL_TWISTING_SLASH
            || Skill == AT_SKILL_TWISTING_SLASH_STR
            || Skill == AT_SKILL_TWISTING_SLASH_STR_MG
            || Skill == AT_SKILL_TWISTING_SLASH_MASTERY
            || Skill == AT_SKILL_FIRE_SLASH
            || Skill == AT_SKILL_FIRE_SLASH_STR
            )
        {
            SendRequestMagicContinue(Skill, (c->PositionX), (c->PositionY), 
                (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
        }
        else
        {
            SendRequestMagic(Skill, TKey);
        }
    }

    if (((!g_isCharacterBuff(o, eDeBuff_Harden)) && c->Helper.Type != MODEL_DARK_HORSE_ITEM)
        && Skill != AT_SKILL_FIRE_SCREAM
        && Skill != AT_SKILL_FIRE_SCREAM_STR
        )
    {
        BYTE positionX = (BYTE)(c->TargetPosition[0] / TERRAIN_SCALE);
        BYTE positionY = (BYTE)(c->TargetPosition[1] / TERRAIN_SCALE);

        if ((gMapManager.InBloodCastle() == true)
            || Skill == AT_SKILL_FORCE
            || Skill == AT_SKILL_FORCE_WAVE
            || Skill == AT_SKILL_FORCE_WAVE_STR
            || Skill == AT_SKILL_SPIRAL_SLASH
            || Skill == AT_SKILL_RUSH
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_CASTLE_GATE1
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_GUARDIAN_STATUE
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_LIFE_STONE
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_CANON_TOWER
            )
        {
            int angle = abs((int)(o->Angle[2] / 45.f));
            switch (angle)
            {
            case 0: positionY++; break;
            case 1: positionX--; positionY++; break;
            case 2: positionX--; break;
            case 3: positionX--; positionY--; break;
            case 4: positionY--; break;
            case 5: positionX++; positionY--; break;
            case 6: positionX++; break;
            case 7: positionX++; positionY++; break;
            }
        }

#ifdef SEND_POSITION_TO_SERVER
        int TargetIndex = TERRAIN_INDEX(positionX, positionY);

        if ((TerrainWall[TargetIndex] & TW_NOMOVE) != TW_NOMOVE && (TerrainWall[TargetIndex] & TW_NOGROUND) != TW_NOGROUND)
        {
            if (Skill != AT_SKILL_IMPALE
                && Skill != AT_SKILL_DEATHSTAB
                && Skill != AT_SKILL_DEATHSTAB_STR
                && Skill != AT_SKILL_RIDER
                && Skill != AT_SKILL_FORCE
                && Skill != AT_SKILL_FORCE_WAVE
                && Skill != AT_SKILL_FORCE_WAVE_STR
                && Skill != AT_SKILL_FIREBURST
                && Skill != AT_SKILL_FIREBURST_STR
                && Skill != AT_SKILL_FIREBURST_MASTERY
                && Skill != AT_SKILL_SPACE_SPLIT
                )
            {
                SocketClient->ToGameServer()->SendInstantMoveRequest(positionX, positionY);
            }
        }
#endif
    }

    c->SkillSuccess = true;
}

void UseSkillWizard(CHARACTER* c, OBJECT* o)
{
    auto Skill = CharacterAttribute->Skill[g_MovementSkill.m_iSkill];

    switch (Skill)
    {
    case AT_SKILL_IMPALE:
    case AT_SKILL_DEATHSTAB:
    case AT_SKILL_DEATHSTAB_STR:
        return;
    }

    if (Skill == AT_SKILL_DEATH_CANNON)
    {
        if (Hero->Weapon[0].Type < MODEL_STAFF || Hero->Weapon[0].Type >= MODEL_STAFF + MAX_ITEM_INDEX)
            return;
    }

    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, c->TargetPosition);
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
    }

    WORD TKey = 0xffff;
    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
    }

    switch (Skill)
    {
    case AT_SKILL_POISON:
    case AT_SKILL_POISON_STR:
    case AT_SKILL_METEO:
    case AT_SKILL_LIGHTNING:
    case AT_SKILL_LIGHTNING_STR:
    case AT_SKILL_LIGHTNING_STR_MG:
    case AT_SKILL_ENERGYBALL:
    case AT_SKILL_POWERWAVE:
    case AT_SKILL_ICE:
    case AT_SKILL_ICE_STR:
    case AT_SKILL_ICE_STR_MG:
    case AT_SKILL_FIREBALL:
    case AT_SKILL_JAVELIN:
        SendRequestMagic(Skill, TKey);
        SetPlayerMagic(c);
        LetHeroStop();
        break;
    case AT_SKILL_DEATH_CANNON:
        SendRequestMagic(Skill, TKey);
        SetAction(o, PLAYER_ATTACK_DEATH_CANNON);
        SetAttackSpeed();
        LetHeroStop();
        break;
    case AT_SKILL_BLAST:
    case AT_SKILL_BLAST_STR:
    case AT_SKILL_BLAST_STR_MG:
    {
        SendRequestMagicContinue(Skill, (int)(c->TargetPosition[0] / 100.f), (int)(c->TargetPosition[1] / 100.f), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
    }
    SetPlayerMagic(c);
    LetHeroStop();
    break;
    }

    c->SkillSuccess = true;
}

void UseSkillElf(CHARACTER* c, OBJECT* o)
{
    LetHeroStop();
    int Skill = CharacterAttribute->Skill[g_MovementSkill.m_iSkill];

    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, c->TargetPosition);
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
    }

    WORD TKey = 0xffff;
    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
    {
        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
    }

    switch (Skill)
    {
    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_RECOVER:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
        SendRequestMagic(Skill, TKey);
        SetPlayerMagic(c);
        break;

    case AT_SKILL_ICE_ARROW:
    case AT_SKILL_ICE_ARROW_STR:
    {
        WORD Dexterity;
        const WORD notDexterity = 646;
        Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        if (Dexterity < notDexterity)
        {
            break;
        }
        if (!CheckArrow())
            break;
        SendRequestMagic(Skill, TKey);
        SetPlayerAttack(c);
    }
    break;

    case AT_SKILL_DEEPIMPACT:
        if (!CheckArrow())
            break;
        SendRequestMagic(Skill, TKey);
        SetPlayerHighBowAttack(c);
        break;
    }

    c->SkillSuccess = true;
}

void UseSkillSummon(CHARACTER* pCha, OBJECT* pObj)
{
    int iSkill = CharacterAttribute->Skill[g_MovementSkill.m_iSkill];

    switch (iSkill)
    {
    case AT_SKILL_ALICE_DRAINLIFE:
    case AT_SKILL_ALICE_DRAINLIFE_STR:
    case AT_SKILL_ALICE_LIGHTNINGORB:
    {
        LetHeroStop();
        if (iSkill == AT_SKILL_ALICE_DRAINLIFE || iSkill == AT_SKILL_ALICE_DRAINLIFE_STR)
        {
            switch (pCha->Helper.Type)
            {
            case MODEL_HORN_OF_UNIRIA:
                SetAction(pObj, PLAYER_SKILL_DRAIN_LIFE_UNI);
                break;
            case MODEL_HORN_OF_DINORANT:
                SetAction(pObj, PLAYER_SKILL_DRAIN_LIFE_DINO);
                break;
            case MODEL_HORN_OF_FENRIR:
                SetAction(pObj, PLAYER_SKILL_DRAIN_LIFE_FENRIR);
                break;
            default:
                SetAction(pObj, PLAYER_SKILL_DRAIN_LIFE);
                break;
            }
        }
        else if (iSkill == AT_SKILL_ALICE_LIGHTNINGORB)
        {
            switch (pCha->Helper.Type)
            {
            case MODEL_HORN_OF_UNIRIA:
                SetAction(pObj, PLAYER_SKILL_LIGHTNING_ORB_UNI);
                break;
            case MODEL_HORN_OF_DINORANT:
                SetAction(pObj, PLAYER_SKILL_LIGHTNING_ORB_DINO);
                break;
            case MODEL_HORN_OF_FENRIR:
                SetAction(pObj, PLAYER_SKILL_LIGHTNING_ORB_FENRIR);
                break;
            default:
                SetAction(pObj, PLAYER_SKILL_LIGHTNING_ORB);
                break;
            }
        }

        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            WORD wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
            SendRequestMagicContinue(iSkill, (int)(pCha->TargetPosition[0] / 100.f), (int)(pCha->TargetPosition[1] / 100.f), (BYTE)(pObj->Angle[2] / 360.f * 256.f), 0, 0, wTargetKey, 0);
        }
    }
    break;
    case AT_SKILL_ALICE_CHAINLIGHTNING:
    case AT_SKILL_ALICE_CHAINLIGHTNING_STR:
    {
        LetHeroStop();

        switch (pCha->Helper.Type)
        {
        case MODEL_HORN_OF_UNIRIA:
            SetAction(pObj, PLAYER_SKILL_CHAIN_LIGHTNING_UNI);
            break;
        case MODEL_HORN_OF_DINORANT:
            SetAction(pObj, PLAYER_SKILL_CHAIN_LIGHTNING_DINO);
            break;
        case MODEL_HORN_OF_FENRIR:
            SetAction(pObj, PLAYER_SKILL_CHAIN_LIGHTNING_FENRIR);
            break;
        default:
            SetAction(pObj, PLAYER_SKILL_CHAIN_LIGHTNING);
            break;
        }

        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            WORD wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;

            SendRequestMagicContinue(iSkill, (int)(pCha->TargetPosition[0] / 100.f),
                (int)(pCha->TargetPosition[1] / 100.f),
                (BYTE)(pObj->Angle[2] / 360.f * 256.f),
                0, 0, wTargetKey, 0);
        }
    }
    break;
    case AT_SKILL_ALICE_SLEEP:
    case AT_SKILL_ALICE_SLEEP_STR:
    case AT_SKILL_ALICE_BLIND:
    case AT_SKILL_ALICE_THORNS:
    {
        LetHeroStop();

        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, pCha->TargetPosition);
            pObj->Angle[2] = CreateAngle2D(pObj->Position, pCha->TargetPosition);

            WORD wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
            SendRequestMagic(iSkill, wTargetKey);
        }
    }
    break;
    case AT_SKILL_ALICE_BERSERKER:
    case AT_SKILL_ALICE_BERSERKER_STR:
        LetHeroStop();
        switch (pCha->Helper.Type)
        {
        case MODEL_HORN_OF_UNIRIA:
            SetAction(pObj, PLAYER_SKILL_SLEEP_UNI);
            break;
        case MODEL_HORN_OF_DINORANT:
            SetAction(pObj, PLAYER_SKILL_SLEEP_DINO);
            break;
        case MODEL_HORN_OF_FENRIR:
            SetAction(pObj, PLAYER_SKILL_SLEEP_FENRIR);
            break;
        default:
            SetAction(pObj, PLAYER_SKILL_SLEEP);
            break;
        }
        SendRequestMagic(iSkill, HeroKey);
        break;
    case AT_SKILL_ALICE_WEAKNESS:
    case AT_SKILL_ALICE_ENERVATION:
        LetHeroStop();
        SendRequestMagicContinue(iSkill, pCha->PositionX, pCha->PositionY, (BYTE)(pObj->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
        switch (pCha->Helper.Type)
        {
        case MODEL_HORN_OF_UNIRIA:
            SetAction(pObj, PLAYER_SKILL_SLEEP_UNI);
            break;
        case MODEL_HORN_OF_DINORANT:
            SetAction(pObj, PLAYER_SKILL_SLEEP_DINO);
            break;
        case MODEL_HORN_OF_FENRIR:
            SetAction(pObj, PLAYER_SKILL_SLEEP_FENRIR);
            break;
        default:
            SetAction(pObj, PLAYER_SKILL_SLEEP);
            break;
        }
        break;
    }

    pCha->SkillSuccess = true;
}

BYTE GetDestValue(int xPos, int yPos, int xDst, int yDst)
{
    int DestX = xDst - xPos;
    int DestY = yDst - yPos;
    if (DestX < -8) DestX = -8;
    if (DestX > 7) DestX = 7;
    if (DestY < -8) DestY = -8;
    if (DestY > 7) DestY = 7;
    assert(-8 <= DestX && DestX <= 7);
    assert(-8 <= DestY && DestY <= 7);
    BYTE byValue1 = ((BYTE)(DestX + 8)) << 4;
    BYTE byValue2 = ((BYTE)(DestY + 8)) & 0xf;
    return (byValue1 | byValue2);
}

void UseSkillRagefighter(CHARACTER* pCha, OBJECT* pObj)
{
    int iSkill = g_MovementSkill.m_bMagic ? CharacterAttribute->Skill[g_MovementSkill.m_iSkill] : g_MovementSkill.m_iSkill;

    ITEM* pLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
    ITEM* pRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

    if (g_CMonkSystem.IsChangeringNotUseSkill(pLeftRing->Type, pRightRing->Type, pLeftRing->Level, pRightRing->Level))
        return;

    if (g_CMonkSystem.IsRideNotUseSkill(iSkill, pCha->Helper.Type))
        return;

    if (!g_CMonkSystem.IsSwordformGlovesUseSkill(iSkill))
        return;

    LetHeroStop();
    pCha->Movement = false;

    if (pObj->Type == MODEL_PLAYER)
    {
        g_CMonkSystem.SetRageSkillAni(iSkill, pObj);
        SetAttackSpeed();
    }

    switch (iSkill)
    {
    case AT_SKILL_KILLING_BLOW:
    case AT_SKILL_KILLING_BLOW_STR:
    case AT_SKILL_KILLING_BLOW_MASTERY:
    case AT_SKILL_OCCUPY:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
			VectorCopy(CharactersClient[g_MovementSkill.m_iTarget].Object.Position, pCha->TargetPosition);
            pObj->Angle[2] = CreateAngle2D(pObj->Position, pCha->TargetPosition);
        }
        SendRequestMagic(iSkill, wTargetKey);

        BYTE TargetPosX = (BYTE)(pCha->TargetPosition[0] / TERRAIN_SCALE);
        BYTE TargetPosY = (BYTE)(pCha->TargetPosition[1] / TERRAIN_SCALE);

        if ((gMapManager.InBloodCastle()) || iSkill == AT_SKILL_OCCUPY
            || (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT && (
               CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_CASTLE_GATE1
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_GUARDIAN_STATUE
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_LIFE_STONE
            || CharactersClient[g_MovementSkill.m_iTarget].MonsterIndex == MONSTER_CANON_TOWER))
            )
        {
            int angle = abs((int)(pObj->Angle[2] / 45.f));
            switch (angle)
            {
            case 0: TargetPosY++; break;
            case 1: TargetPosX--; TargetPosY++; break;
            case 2: TargetPosX--; break;
            case 3: TargetPosX--; TargetPosY--; break;
            case 4: TargetPosY--; break;
            case 5: TargetPosX++; TargetPosY--; break;
            case 6: TargetPosX++; break;
            case 7: TargetPosX++; TargetPosY++; break;
            }
        }

        int TargetIndex = TERRAIN_INDEX(TargetPosX, TargetPosY);

        vec3_t vDis;
        Vector(0.0f, 0.0f, 0.0f, vDis);
        VectorSubtract(pCha->TargetPosition, pCha->Object.Position, vDis);
        VectorNormalize(vDis);
        VectorScale(vDis, TERRAIN_SCALE, vDis);
        VectorSubtract(pCha->TargetPosition, vDis, vDis);
        BYTE CharPosX = (BYTE)(vDis[0] / TERRAIN_SCALE);
        BYTE CharPosY = (BYTE)(vDis[1] / TERRAIN_SCALE);

#ifdef SEND_POSITION_TO_SERVER
        if ((TerrainWall[TargetIndex] & TW_NOMOVE) != TW_NOMOVE && (TerrainWall[TargetIndex] & TW_NOGROUND) != TW_NOGROUND)
        {
            SocketClient->ToGameServer()->SendInstantMoveRequest(CharPosX, CharPosY);
        }
#endif
        
        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_BEAST_UPPERCUT:
    case AT_SKILL_BEAST_UPPERCUT_STR:
    case AT_SKILL_BEAST_UPPERCUT_MASTERY:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }

        SocketClient->ToGameServer()->SendRageAttackRequest(iSkill, wTargetKey);

        g_CMonkSystem.InitConsecutiveState(3.0f, 7.0f);
    }
    break;
    case AT_SKILL_CHAIN_DRIVE:
    case AT_SKILL_CHAIN_DRIVE_STR:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }
        SocketClient->ToGameServer()->SendRageAttackRequest(iSkill, wTargetKey);

        g_CMonkSystem.InitConsecutiveState(3.0f, 12.0f);

        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_DRAGON_KICK:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }
        SocketClient->ToGameServer()->SendRageAttackRequest(iSkill, wTargetKey);

        g_CMonkSystem.InitConsecutiveState(3.0f);

        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_DARKSIDE:
    case AT_SKILL_DARKSIDE_STR:
    {
        WORD wTargetKey = 0;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            wTargetKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }

        SocketClient->ToGameServer()->SendRageAttackRangeRequest(iSkill, wTargetKey);
        SocketClient->ToGameServer()->SendRageAttackRequest(iSkill, wTargetKey);

        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_DRAGON_ROAR:
    case AT_SKILL_DRAGON_ROAR_STR:
    {
        BYTE angle = (BYTE)((((pObj->Angle[2] + 180.f) / 360.f) * 255.f));
        WORD TKey = 0xffff;
        if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
        {
            TKey = CharactersClient[g_MovementSkill.m_iTarget].Key;
        }
        BYTE byValue = GetDestValue((pCha->PositionX), (pCha->PositionY), TargetX, TargetY);
        SendRequestMagicContinue(iSkill, pCha->PositionX, pCha->PositionY, ((pObj->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);

        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_ATT_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES:
    case AT_SKILL_DEF_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
    {
        SendRequestMagic(iSkill, HeroKey);
        if (rand_fps_check(2))
        {
            SetAction(pObj, PLAYER_SKILL_ATT_UP_OURFORCES);
            PlayBuffer(SOUND_RAGESKILL_BUFF_1);
        }
        else
        {
            SetAction(pObj, PLAYER_SKILL_HP_UP_OURFORCES);
            PlayBuffer(SOUND_RAGESKILL_BUFF_2);
        }
        g_CMonkSystem.RageCreateEffect(pObj, iSkill);
    }
    break;
    case AT_SKILL_PLASMA_STORM_FENRIR:
    {
        pObj->Angle[2] = CreateAngle2D(pObj->Position, pCha->TargetPosition);

        gSkillManager.CheckSkillDelay(Hero->CurrentSkill);

        BYTE pos = CalcTargetPos(pObj->Position[0], pObj->Position[1], pCha->TargetPosition[0], pCha->TargetPosition[1]);
        WORD TKey = 0xffff;
        TKey = getTargetCharacterKey(pCha, g_MovementSkill.m_iTarget);
        pCha->m_iFenrirSkillTarget = g_MovementSkill.m_iTarget;
        SendRequestMagicContinue(iSkill, (pCha->PositionX), (pCha->PositionY), (BYTE)(pObj->Angle[2] / 360.f * 256.f), 0, pos, TKey, &pObj->m_bySkillSerialNum);
        pCha->Movement = 0;

        if (pObj->Type == MODEL_PLAYER)
        {
            SetAction_Fenrir_Skill(pCha, pObj);
        }
    }
    break;
    default:
        break;
    }

    pCha->SkillSuccess = true;
}

void AttackRagefighter(CHARACTER* pCha, int nSkill, float fDistance)
{
    OBJECT* pObj = &pCha->Object;

    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(nSkill, 1, NULL, &iMana, NULL, &iSkillMana);

    g_ConsoleDebug->Write(MCD_RECEIVE, L"AttackRagefighter ID : %d, Dis : %.2f | %d %d / %d | %d", nSkill, fDistance, iMana, iSkillMana, CharacterAttribute->Mana, gSkillManager.CheckSkillDelay(Hero->CurrentSkill));

    if (CharacterAttribute->Mana < iMana)
    {
        int Index = g_pMyInventory->FindManaItemIndex();

        if (Index != -1)
            SendRequestUse(Index, 0);

        return;
    }

    if (iSkillMana > CharacterAttribute->SkillMana)
        return;

    /*if (!gSkillManager.CheckSkillDelay(Hero->CurrentSkill)) {
        g_ConsoleDebug->Write(MCD_RECEIVE, L"CheckSkillDelay %d", Hero->CurrentSkill);
        return;
    }*/

    bool bSuccess = CheckTarget(pCha);
    bool bCheckAttack = CheckAttack();
    g_MovementSkill.m_bMagic = TRUE;
    g_MovementSkill.m_iSkill = Hero->CurrentSkill;

    if (bCheckAttack)
        g_MovementSkill.m_iTarget = SelectedCharacter;
    else
        g_MovementSkill.m_iTarget = -1;

    g_ConsoleDebug->Write(MCD_SEND, L"AttackRagefighter ID : %d, Success : %d, SelectedCharacter: %d %d | 5d", nSkill, bSuccess, SelectedCharacter, CharactersClient[SelectedCharacter].Dead, bCheckAttack);

    if (bSuccess)
    {
        switch (nSkill)
        {
        case AT_SKILL_KILLING_BLOW:
        case AT_SKILL_KILLING_BLOW_STR:
        case AT_SKILL_KILLING_BLOW_MASTERY:
        case AT_SKILL_BEAST_UPPERCUT:
        case AT_SKILL_BEAST_UPPERCUT_STR:
        case AT_SKILL_BEAST_UPPERCUT_MASTERY:
        case AT_SKILL_CHAIN_DRIVE:
        case AT_SKILL_CHAIN_DRIVE_STR:
        case AT_SKILL_DRAGON_KICK:
        case AT_SKILL_DRAGON_ROAR:
        case AT_SKILL_DRAGON_ROAR_STR:
        case AT_SKILL_OCCUPY:
        case AT_SKILL_PHOENIX_SHOT:
        {
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Dead == 0)
            {
                pCha->TargetCharacter = SelectedCharacter;
                
                TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);

                if (bCheckAttack)
                {
                    fDistance = gSkillManager.GetSkillDistance(nSkill, pCha) * 1.2f;
                    if (CheckTile(pCha, pObj, fDistance) && pCha->SafeZone == false)
                    {
                        bool bNoneWall = CheckWall((pCha->PositionX), (pCha->PositionY), TargetX, TargetY);
                        g_ConsoleDebug->Write(MCD_SEND, L"check wall %d", bNoneWall);
                        if (bNoneWall)
                            UseSkillRagefighter(pCha, pObj);
                    }
                    else
                    {
                        if (PathFinding2(pCha->PositionX, pCha->PositionY, TargetX, TargetY, &pCha->Path, fDistance))
                        {
                            pCha->Movement = true;
                            pCha->MovementType = MOVEMENT_SKILL;
                            SendMove(pCha, pObj);
                        }
                    }
                }
            }
        }
        break;
        case AT_SKILL_DARKSIDE:
        case AT_SKILL_DARKSIDE_STR:
            UseSkillRagefighter(pCha, pObj);
            break;
        case AT_SKILL_ATT_UP_OURFORCES:
        case AT_SKILL_HP_UP_OURFORCES:
        case AT_SKILL_HP_UP_OURFORCES_STR:
        case AT_SKILL_DEF_UP_OURFORCES:
        case AT_SKILL_DEF_UP_OURFORCES_STR:
        case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
            UseSkillRagefighter(pCha, pObj);
            break;
        case AT_SKILL_PLASMA_STORM_FENRIR:
        {
            if (gMapManager.InChaosCastle())
                break;

            int nTargetX = (int)(pCha->TargetPosition[0] / TERRAIN_SCALE);
            int nTargetY = (int)(pCha->TargetPosition[1] / TERRAIN_SCALE);
            if (CheckTile(pCha, pObj, fDistance))
            {
                if (g_MovementSkill.m_iTarget != -1)
                {
                    UseSkillRagefighter(pCha, pObj);
                }
                else
                {
                    pCha->m_iFenrirSkillTarget = -1;
                }
            }
            else
            {
                if (g_MovementSkill.m_iTarget != -1)
                {
                    if (PathFinding2(pCha->PositionX, pCha->PositionY, nTargetX, nTargetY, &pCha->Path, fDistance * 1.2f))
                    {
                        pCha->Movement = true;
                    }
                }
                else
                {
                    Attacking = -1;
                }
            }
        }
        break;
        }
    }

    pCha->SkillSuccess = true;
}

bool UseSkillRagePosition(CHARACTER* pCha)
{
    OBJECT* pObj = &pCha->Object;

    if (pObj->CurrentAction == PLAYER_SKILL_GIANTSWING || pObj->CurrentAction == PLAYER_SKILL_STAMP || pObj->CurrentAction == PLAYER_SKILL_DRAGONKICK)
    {
        if (g_CMonkSystem.IsConsecutiveAtt(pObj->AnimationFrame))
        {
            int iSkill = g_MovementSkill.m_bMagic ? CharacterAttribute->Skill[g_MovementSkill.m_iSkill] : g_MovementSkill.m_iSkill;
            return g_CMonkSystem.SendAttackPacket(pCha, g_MovementSkill.m_iTarget, iSkill);
        }

        if (pObj->CurrentAction == PLAYER_SKILL_STAMP && pObj->AnimationFrame >= 2.0f)
        {
            pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
            g_CMonkSystem.RageCreateEffect(pObj, AT_SKILL_BEAST_UPPERCUT);
        }
    }
    else if (pObj->CurrentAction == PLAYER_SKILL_DARKSIDE_READY)
    {
        int AttTime = (int)(2.5f / Models[pObj->Type].Actions[PLAYER_SKILL_DARKSIDE_READY].PlaySpeed);
        if (pCha->AttackTime >= AttTime)
        {
            if (g_CMonkSystem.SendDarksideAtt(pObj))
            {
                pCha->AttackTime = 1;
                return true;
            }
        }
        return false;
    }
    else if (pObj->CurrentAction == PLAYER_SKILL_THRUST)
    {
        pObj->Angle[2] = CreateAngle2D(pObj->Position, pCha->TargetPosition);
        pObj->m_sTargetIndex = g_MovementSkill.m_iTarget;
    }
    else
    {
        g_CMonkSystem.InitConsecutiveState();
        g_CMonkSystem.InitEffectOnce();
    }
    return false;
}

void ReloadArrow()
{
    int Index = SearchArrow();

    if (Index != -1)
    {
        ITEM* rp = NULL;
        ITEM* lp = NULL;

        bool Success = false;

        if (gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_ELF && SEASON3B::CNewUIInventoryCtrl::GetPickedItem() == NULL)
        {
            rp = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
            lp = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
            Success = true;
        }

        if (Success)
        {
            if ((gCharacterManager.GetEquipedBowType(lp) == BOWTYPE_BOW) && (rp->Type == -1))
            {
                ITEM* pItem = g_pMyInventory->FindItem(Index);
                SEASON3B::CNewUIInventoryCtrl::CreatePickedItem(g_pMyInventory->GetInventoryCtrl(), pItem);
                if (pItem)
                {
                    SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, Index, pItem, STORAGE_TYPE::INVENTORY, EQUIPMENT_WEAPON_RIGHT);
                }
                g_pMyInventory->DeleteItem(Index);
                g_pSystemLogBox->AddText(GlobalText[250], SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
            else
                if ((gCharacterManager.GetEquipedBowType(rp) == BOWTYPE_CROSSBOW) && (lp->Type == -1))
                {
                    ITEM* pItem = g_pMyInventory->FindItem(Index);
                    SEASON3B::CNewUIInventoryCtrl::CreatePickedItem(g_pMyInventory->GetInventoryCtrl(), pItem);
                    if (pItem)
                    {
                        SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, Index, pItem, STORAGE_TYPE::INVENTORY, EQUIPMENT_WEAPON_LEFT);
                    }
                    g_pMyInventory->DeleteItem(Index);
                    g_pSystemLogBox->AddText(GlobalText[250], SEASON3B::TYPE_SYSTEM_MESSAGE);
                }
        }
    }
    else
    {
        if (g_pSystemLogBox->CheckChatRedundancy(GlobalText[251]) == FALSE)
        {
            g_pSystemLogBox->AddText(GlobalText[251], SEASON3B::TYPE_ERROR_MESSAGE);
        }
    }
}

bool CheckArrow()
{
    int Right = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
    int Left = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;

    if (gCharacterManager.GetEquipedBowType() == BOWTYPE_CROSSBOW)
    {
        if ((Left != ITEM_BOLT) || (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability <= 0))
        {
            ReloadArrow();
            return false;
        }
    }
    else if (gCharacterManager.GetEquipedBowType() == BOWTYPE_BOW)
    {
        if ((Right != ITEM_ARROWS) || (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Durability <= 0))
        {
            ReloadArrow();
            return false;
        }
    }
    return true;
}

bool SkillElf(CHARACTER* c, ITEM* p)
{
    OBJECT* o = &c->Object;
    bool Success = false;
    auto currentSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    for (int i = 0; i < p->SpecialNum; i++)
    {
        int Spe_Num = p->Special[i];
        if (Spe_Num == AT_SKILL_TRIPLE_SHOT && (AT_SKILL_TRIPLE_SHOT_STR == currentSkill || AT_SKILL_TRIPLE_SHOT_MASTERY == currentSkill))
        {
            Spe_Num = currentSkill;
        }

        if (currentSkill == Spe_Num)
        {
            int iMana, iSkillMana;
            gSkillManager.GetSkillInformation(Spe_Num, 1, NULL, &iMana, NULL, &iSkillMana);

            if (g_isCharacterBuff(o, eBuff_InfinityArrow))
                iMana += CharacterMachine->InfinityArrowAdditionalMana;

            if (CharacterAttribute->Mana <= iMana)
            {
                int Index = g_pMyInventory->FindManaItemIndex();

                if (Index != -1)
                {
                    SendRequestUse(Index, 0);
                }
                continue;
            }

            if (iSkillMana > CharacterAttribute->SkillMana)
            {
                return (FALSE);
            }
            if (!gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
            {
                return false;
            }
            float Distance = gSkillManager.GetSkillDistance(Spe_Num, c);
            switch (Spe_Num)
            {
            case AT_SKILL_TRIPLE_SHOT:
            case AT_SKILL_TRIPLE_SHOT_STR:
            case AT_SKILL_TRIPLE_SHOT_MASTERY:
                if (!CheckArrow())
                {
                    i = p->SpecialNum;
                    continue;
                }
                if (CheckTile(c, o, Distance))
                {
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Spe_Num, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
                    SetPlayerAttack(c);
                    if (o->Type != MODEL_PLAYER && o->Kind != KIND_PLAYER)
                        CreateArrows(c, o, NULL, FindHotKey((c->Skill)), 1);
                    Success = true;
                }
                break;

            case AT_SKILL_BLAST_CROSSBOW4:
                if (!CheckArrow())
                {
                    i = p->SpecialNum;
                    continue;
                }
                if (CheckTile(c, o, Distance))
                {
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Spe_Num, (c->PositionX),
                        (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                    SetPlayerAttack(c);
                    if (o->Type != MODEL_PLAYER)
                        CreateArrows(c, o, NULL, FindHotKey((c->Skill)), Spe_Num - AT_SKILL_BLAST_CROSSBOW4 + 2);

                    Success = true;
                }
                break;
            }
        }
    }
    return Success;
}

void SendRequestAction(OBJECT& obj, BYTE action)
{
    BYTE rotation = (BYTE)((obj.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
    SocketClient->ToGameServer()->SendAnimationRequest(rotation, action);
}

int ItemKey = 0;
int ActionTarget = -1;

void Action(CHARACTER* c, OBJECT* o, bool Now)
{
    float Range = 1.8f;
    switch (c->MovementType)
    {
    case MOVEMENT_ATTACK:
    {
        int Right = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
        int Left = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;

        if (Right >= ITEM_SPEAR && Right < ITEM_SPEAR + MAX_ITEM_INDEX)
        {
            Range = 2.2f;
        }

        if (gCharacterManager.GetEquipedBowType() != BOWTYPE_NONE)
        {
            Range = 6.f;
        }

        if (ActionTarget == -1)
            return;

        if (o->Type == MODEL_PLAYER)
        {
            if (o->CurrentAction >= PLAYER_ATTACK_FIST && o->CurrentAction <= PLAYER_RIDE_SKILL
                && o->CurrentAction != PLAYER_STOP_RIDE_HORSE
                && o->CurrentAction != PLAYER_STOP_TWO_HAND_SWORD_TWO
                && o->CurrentAction == PLAYER_FENRIR_SKILL_ONE_RIGHT
                && o->CurrentAction == PLAYER_RAGE_FENRIR_ONE_RIGHT)
                break;
        }
        else
        {
            if (o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)
                break;
        }

        if (ActionTarget <= -1)
            break;

        TargetX = (int)(CharactersClient[ActionTarget].Object.Position[0] / TERRAIN_SCALE);
        TargetY = (int)(CharactersClient[ActionTarget].Object.Position[1] / TERRAIN_SCALE);

        if (c->SafeZone)
        {
            break;
        }
        else if (!CheckTile(c, o, Range))
        {
            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_ELF)
            {
                if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Range))
                {
                    c->Movement = true;
                }
            }
            break;
        }

        MouseUpdateTime = MouseUpdateTimeMax;
        SetPlayerAttack(c);
        c->AttackTime = 1;
        VectorCopy(CharactersClient[ActionTarget].Object.Position, c->TargetPosition);
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
        LetHeroStop();
        c->Movement = false;
        BYTE PathX[1];
        BYTE PathY[1];
        PathX[0] = (c->PositionX);
        PathY[0] = (c->PositionY);
        c->TargetCharacter = ActionTarget;
        int Dir = ((BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8);
        c->Skill = 0;
        SocketClient->ToGameServer()->SendHitRequest(CharactersClient[ActionTarget].Key, AT_ATTACK1, Dir);
    }
    break;
    case MOVEMENT_SKILL:
    {
        auto iSkill = static_cast<ActionSkillType>((g_MovementSkill.m_bMagic) ? (CharacterAttribute->Skill[g_MovementSkill.m_iSkill]) : g_MovementSkill.m_iSkill);

        float Distance = gSkillManager.GetSkillDistance(iSkill, c);
        switch (iSkill)
        {
        case AT_SKILL_IMPALE:
            if (Hero->Helper.Type != MODEL_HORN_OF_UNIRIA)
            {
                break;
            }
        case AT_SKILL_DEATHSTAB:
        case AT_SKILL_DEATHSTAB_STR:
        case AT_SKILL_RIDER:
        case AT_SKILL_FALLING_SLASH:
        case AT_SKILL_FALLING_SLASH_STR:
        case AT_SKILL_LUNGE:
        case AT_SKILL_LUNGE_STR:
        case AT_SKILL_UPPERCUT:
        case AT_SKILL_CYCLONE:
        case AT_SKILL_CYCLONE_STR:
        case AT_SKILL_CYCLONE_STR_MG:
        case AT_SKILL_SLASH:
        case AT_SKILL_SLASH_STR:
        case AT_SKILL_FORCE:
        case AT_SKILL_FORCE_WAVE:
        case AT_SKILL_FORCE_WAVE_STR:
        case AT_SKILL_FIREBURST:
        case AT_SKILL_FIREBURST_STR:
        case AT_SKILL_FIREBURST_MASTERY:
        case AT_SKILL_RUSH:
        case AT_SKILL_SPACE_SPLIT:
        case AT_SKILL_SPIRAL_SLASH:
            if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT && CharactersClient[g_MovementSkill.m_iTarget].Dead == 0)
            {
                TargetX = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance * 1.2f) && !c->SafeZone)
                {
                    UseSkillWarrior(c, o);
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance * 1.2f))
                    {
                        c->Movement = true;
                    }
                }
            }
            break;
        case AT_SKILL_POISON:
        case AT_SKILL_POISON_STR:
        case AT_SKILL_METEO:
        case AT_SKILL_LIGHTNING:
        case AT_SKILL_LIGHTNING_STR:
        case AT_SKILL_LIGHTNING_STR_MG:
        case AT_SKILL_ENERGYBALL:
        case AT_SKILL_POWERWAVE:
        case AT_SKILL_ICE:
        case AT_SKILL_ICE_STR:
        case AT_SKILL_ICE_STR_MG:
        case AT_SKILL_FIREBALL:
        case AT_SKILL_JAVELIN:
        case AT_SKILL_DEATH_CANNON:
            if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT && CharactersClient[g_MovementSkill.m_iTarget].Dead == 0)
            {
                TargetX = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance) && !c->SafeZone)
                {
                    bool Wall = CheckWall((c->PositionX),
                        (c->PositionY), TargetX, TargetY);
                    if (Wall)
                    {
                        UseSkillWizard(c, o);
                    }
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                    {
                        c->Movement = true;
                    }
                }
            }
            break;
        case AT_SKILL_DEEPIMPACT:
        case AT_SKILL_ICE_ARROW:
        case AT_SKILL_ICE_ARROW_STR:
            if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT && CharactersClient[g_MovementSkill.m_iTarget].Dead == 0)
            {
                TargetX = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance) && !c->SafeZone)
                {
                    bool Wall = CheckWall((c->PositionX),
                        (c->PositionY), TargetX, TargetY);
                    if (Wall)
                    {
                        UseSkillElf(c, o);
                    }
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                    {
                        c->Movement = true;
                    }
                }
            }
            break;
        case AT_SKILL_HEALING:
        case AT_SKILL_HEALING_STR:
        case AT_SKILL_ATTACK:
        case AT_SKILL_ATTACK_STR:
        case AT_SKILL_ATTACK_MASTERY:
        case AT_SKILL_DEFENSE:
        case AT_SKILL_DEFENSE_STR:
        case AT_SKILL_DEFENSE_MASTERY:
            if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT && CharactersClient[g_MovementSkill.m_iTarget].Dead == 0)
            {
                TargetX = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance) && !c->SafeZone)
                {
                    bool Wall = CheckWall((c->PositionX),
                        (c->PositionY), TargetX, TargetY);
                    if (Wall)
                    {
                        UseSkillElf(c, o);
                    }
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                    {
                        c->Movement = true;
                    }
                }
            }
            break;
        case AT_SKILL_TWISTING_SLASH:
        case AT_SKILL_TWISTING_SLASH_STR:
        case AT_SKILL_TWISTING_SLASH_MASTERY:
        case AT_SKILL_FIRE_SLASH:
        case AT_SKILL_FIRE_SLASH_STR:
        {
            AttackKnight(c, iSkill, Distance);
        }
        break;
        case AT_SKILL_KILLING_BLOW:
        case AT_SKILL_KILLING_BLOW_STR:
        case AT_SKILL_KILLING_BLOW_MASTERY:
        case AT_SKILL_BEAST_UPPERCUT:
        case AT_SKILL_BEAST_UPPERCUT_STR:
        case AT_SKILL_BEAST_UPPERCUT_MASTERY:
        case AT_SKILL_CHAIN_DRIVE:
        case AT_SKILL_CHAIN_DRIVE_STR:
        case AT_SKILL_DRAGON_KICK:
        case AT_SKILL_DRAGON_ROAR:
        case AT_SKILL_DRAGON_ROAR_STR:
        case AT_SKILL_OCCUPY:
        case AT_SKILL_PHOENIX_SHOT:
        {
            g_ConsoleDebug->Write(MCD_RECEIVE, L"Action ID : %d, %d | %d %d | %d %d", iSkill, Distance, CharactersClient[g_MovementSkill.m_iTarget].Dead, g_MovementSkill.m_iTarget,
                CheckTile(c, o, Distance * 1.2f), !c->SafeZone);
            if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT && CharactersClient[g_MovementSkill.m_iTarget].Dead == 0)
            {
                TargetX = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance * 1.2f) && !c->SafeZone) {
                    UseSkillRagefighter(c, o);
                    g_ConsoleDebug->Write(MCD_RECEIVE, L"Success attack ID : %d, %d", iSkill, Distance);
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance * 1.2f))
                        c->Movement = true;
                }
            }
        }
        break;
        }
    }
    break;
    case MOVEMENT_GET:
        if (!g_bAutoGetItem)
        {
            if (CheckTile(c, o, 1.5f) == false)
            {
                break;
            }
            MouseUpdateTimeMax = 6;
        }

        if (Items[ItemKey].Item.Type == ITEM_ZEN && SendGetItem == -1)
        {
            SendGetItem = ItemKey;
            SocketClient->ToGameServer()->SendPickupItemRequest(ItemKey);
        }
        else if (g_pMyInventory->FindEmptySlot(&Items[ItemKey].Item) == -1)
        {
            wchar_t Text[256];
            swprintf(Text, GlobalText[375]);

            g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);

            OBJECT* pItem = &(Items[ItemKey].Object);
            pItem->Position[2] = RequestTerrainHeight(pItem->Position[0], pItem->Position[1]) + 3.f;
            pItem->Gravity = 50.f;
        }
        else if (SendGetItem == -1)
        {
            SendGetItem = ItemKey;
            SocketClient->ToGameServer()->SendPickupItemRequest(ItemKey);
        }
        break;
	case MOVEMENT_TALK :
		{
			MouseUpdateTimeMax = 12;

			const bool isCryWolfElf = M34CryWolf1st::Get_State_Only_Elf() && M34CryWolf1st::IsCyrWolf1st();
			if (!isCryWolfElf)
			{
				SetPlayerStop(c);
				c->Movement = false;
			}

			if (TargetNpc == -1)
				break;

			// === Rozpoznanie napotkanego NPC ===
			const int npcIndex = 205;
			const int monsterIndex = CharactersClient[TargetNpc].MonsterIndex;

			if (monsterIndex >= npcIndex)
			{
				const int level = CharacterAttribute->Level;

				if (monsterIndex == MONSTER_CHAOS_GOBLIN && level < 10)
				{
					wchar_t text[100];
					swprintf(text, GlobalText[663], CHAOS_MIX_LEVEL);
					g_pSystemLogBox->AddText(text, SEASON3B::TYPE_SYSTEM_MESSAGE);
					break;
				}

				// Sklepy
				bool isRepairNpc =
					monsterIndex == MONSTER_EO_THE_CRAFTSMAN ||
					monsterIndex == MONSTER_ZIENNA_THE_WEAPONS_MERCHANT ||
					monsterIndex == MONSTER_HANZO_THE_BLACKSMITH ||
					monsterIndex == MONSTER_RHEA ||
					monsterIndex == MONSTER_WEAPONS_MERCHANT_BOLO;

				g_pNPCShop->SetRepairShop(isRepairNpc);

				if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST))
					g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYQUEST);

				if (g_csQuest.IsInit())
					SocketClient->ToGameServer()->SendLegacyQuestStateRequest();

				// === Specjalne rozmowy ===
				const int objectType = CharactersClient[TargetNpc].Object.Type;
				if (isCryWolfElf)
				{
					if (objectType >= MODEL_CRYWOLF_ALTAR1 && objectType <= MODEL_CRYWOLF_ALTAR5)
					{
						const int altarNum = objectType - MODEL_CRYWOLF_ALTAR1;
						const bool isElf = gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF;
						const bool altarActive = M34CryWolf1st::Get_AltarState_State(altarNum);
						const BYTE state = (m_AltarState[altarNum] & 0x0f);

						if (isElf && !altarActive)
						{
							if (state > 0)
								SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Get_Temple));
							else
								SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Destroy_Set_Temple));
						}
						else if (isElf && altarActive)
						{
							SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Ing_Set_Temple));
						}
						else
						{
							SocketClient->ToGameServer()->SendTalkToNpcRequest(CharactersClient[TargetNpc].Key);
						}
					}
					else
					{
						SocketClient->ToGameServer()->SendTalkToNpcRequest(CharactersClient[TargetNpc].Key);
					}
				}
				else if (M34CryWolf1st::IsCyrWolf1st())
				{
					if (!(objectType >= MODEL_CRYWOLF_ALTAR1 && objectType <= MODEL_CRYWOLF_ALTAR5))
					{
						if (objectType == MODEL_NPC_QUARREL)
							SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMapEnterWerwolfMsgBoxLayout));

						SocketClient->ToGameServer()->SendTalkToNpcRequest(CharactersClient[TargetNpc].Key);
					}
				}
				else if (SEASON3A::CGM3rdChangeUp::Instance().IsBalgasBarrackMap())
				{
					SocketClient->ToGameServer()->SendTalkToNpcRequest(CharactersClient[TargetNpc].Key);
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMapEnterGateKeeperMsgBoxLayout));
				}
				else if (monsterIndex >= MONSTER_LITTLE_SANTA_YELLOW && monsterIndex <= MONSTER_LITTLE_SANTA_PINK)
				{
					SocketClient->ToGameServer()->SendTalkToNpcRequest(CharactersClient[TargetNpc].Key);

					wchar_t temp[32] = { 0 };
					if (monsterIndex == MONSTER_LITTLE_SANTA_RED)
						swprintf(temp, GlobalText[2596], 100);
					else if (monsterIndex == MONSTER_LITTLE_SANTA_BLUE)
						swprintf(temp, GlobalText[2597], 100);

					g_pSystemLogBox->AddText(temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
				}
				else if (monsterIndex == MONSTER_DELGADO || monsterIndex == MONSTER_LUGARD ||
					monsterIndex == MONSTER_MARKET_UNION_MEMBER_JULIA || monsterIndex == MONSTER_DAVID)
				{
					SocketClient->ToGameServer()->SendTalkToNpcRequest(CharactersClient[TargetNpc].Key);
				}
				else
				{
					if (M38Kanturu2nd::Is_Kanturu2nd())
					{
						if (!g_pKanturu2ndEnterNpc->IsNpcAnimation())
							SocketClient->ToGameServer()->SendTalkToNpcRequest(CharactersClient[TargetNpc].Key);
					}
					else if (gMapManager.IsCursedTemple())
					{
						if (!g_CursedTemple->IsGaugebarEnabled())
						{
							if (
								CharactersClient[TargetNpc].MonsterIndex == MONSTER_STONE_STATUE ||
								(g_pCursedTempleWindow->CheckInventoryHolyItem(Hero) &&
									((g_pCursedTempleWindow->GetMyTeam() == SEASON3A::eTeam_Allied && monsterIndex == MONSTER_ALLIANCE_ITEM_STORAGE) ||
										(g_pCursedTempleWindow->GetMyTeam() == SEASON3A::eTeam_Illusion && monsterIndex == MONSTER_ILLUSION_ITEM_STORAGE)))
								)
							{
								g_CursedTemple->SetGaugebarEnabled(true);
							}
							g_pCursedTempleWindow->CheckTalkProgressNpc(monsterIndex, CharactersClient[TargetNpc].Key);
						}
					}
					else
					{
						SocketClient->ToGameServer()->SendTalkToNpcRequest(CharactersClient[TargetNpc].Key);
					}
				}

				bCheckNPC = (monsterIndex == MONSTER_MARLON);

				if (monsterIndex == MONSTER_PET_TRAINER)
				{
					ITEM* pItem = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
					if (pItem->Type == ITEM_DARK_HORSE_ITEM)
						SocketClient->ToGameServer()->SendPetInfoRequest(PET_TYPE_DARK_HORSE, 0, EQUIPMENT_HELPER);

					pItem = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
					if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
						SocketClient->ToGameServer()->SendPetInfoRequest(PET_TYPE_DARK_SPIRIT, 0, EQUIPMENT_WEAPON_LEFT);
				}
			}

			TargetNpc = -1;
			break;
		}
    case MOVEMENT_OPERATE:
        if (max(abs((Hero->PositionX) - TargetX), abs((Hero->PositionY) - TargetY)) <= 1)
        {
            bool Healing = false;
            bool Pose = false;
            bool Sit = false;
            if (gMapManager.WorldActive == WD_0LORENCIA)
            {
                switch (TargetType)
                {
                case MODEL_POSE_BOX:Pose = true; Hero->Object.Angle[2] = TargetAngle; break;
                case MODEL_TREE01 + 6:Sit = true; break;
                case MODEL_FURNITURE01 + 5:Sit = true; Hero->Object.Angle[2] = TargetAngle; break;
                case MODEL_FURNITURE01 + 6:Sit = true; break;
                    break;
                }
            }
            else if (gMapManager.WorldActive == WD_1DUNGEON)
            {
                switch (TargetType)
                {
                case 60:Pose = true; Hero->Object.Angle[2] = TargetAngle; break;
                case 59:Sit = true; break;
                }
            }
            else if (gMapManager.WorldActive == WD_2DEVIAS)
            {
                switch (TargetType)
                {
                case 91:Pose = true; Hero->Object.Angle[2] = TargetAngle; break;
                case 22:Sit = true; Hero->Object.Angle[2] = TargetAngle; break;
                case 25:Sit = true; Hero->Object.Angle[2] = TargetAngle; break;
                case 40:Sit = true; Hero->Object.Angle[2] = TargetAngle; break;
                case 45:Sit = true; break;
                case 55:Sit = true; Hero->Object.Angle[2] = TargetAngle; break;
                case 73:Sit = true; break;
                }
            }
            else if (gMapManager.WorldActive == WD_3NORIA)
            {
                switch (TargetType)
                {
                case 8:Sit = true; break;
                case 38:Healing = true; Hero->Object.Angle[2] = TargetAngle; break;
                }
            }
            else if (gMapManager.WorldActive == WD_7ATLANSE)
            {
                switch (TargetType)
                {
                case 39:Pose = true; Hero->Object.Angle[2] = TargetAngle; break;
                }
            }
            else if (gMapManager.WorldActive == WD_8TARKAN)
            {
                switch (TargetType)
                {
                case 78:Sit = true; break;
                }
            }
            else if (gMapManager.InBattleCastle())
            {
                if (TargetType == 77)
                {
                    if (battleCastle::GetGuildMaster(Hero))
                    {
                        Sit = true;
                        Hero->Object.Angle[2] = TargetAngle;
                    }
                }
                else if (TargetType == 84)
                {
                    Sit = true;
                    Hero->Object.Angle[2] = TargetAngle;
                }
            }
            else if (M38Kanturu2nd::Is_Kanturu2nd())
            {
                if (TargetType == 3)
                {
                    Sit = true;
                }
            }
            else if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
            {
                switch (TargetType)
                {
                case 103:
                {
                    Sit = true;
                    Hero->Object.Angle[2] = TargetAngle;
                }
                break;
                }
            }
            else if (gMapManager.WorldActive == WD_79UNITEDMARKETPLACE)
            {
                switch (TargetType)
                {
                case 67:
                {
                    Pose = true;
                    Hero->Object.Angle[2] = TargetAngle;
                }
                break;
                }
            }
            if (Healing)
            {
                if (!gCharacterManager.IsFemale(c->Class))
                    SetAction(o, PLAYER_HEALING1);
                else
                    SetAction(o, PLAYER_HEALING_FEMALE1);
                SendRequestAction(Hero->Object, AT_HEALING1);
            }
            else
            {
                BYTE PathX[1];
                BYTE PathY[1];
                PathX[0] = TargetX;
                PathY[0] = TargetY;

                SendCharacterMove(Hero->Key, Hero->Object.Angle[2], 1, PathX, PathY, TargetX, TargetY);

                c->Path.PathNum = 0;
                if (Pose)
                {
                    if (!gCharacterManager.IsFemale(c->Class))
                        SetAction(o, PLAYER_POSE1);
                    else
                        SetAction(o, PLAYER_POSE_FEMALE1);
                    SendRequestAction(Hero->Object, AT_POSE1);
                }
                if (Sit)
                {
                    if ((!c->SafeZone) && (c->Helper.Type == MODEL_HORN_OF_FENRIR || c->Helper.Type == MODEL_HORN_OF_UNIRIA || c->Helper.Type == MODEL_HORN_OF_DINORANT || c->Helper.Type == MODEL_DARK_HORSE_ITEM))
                        return;

                    if (!gCharacterManager.IsFemale(c->Class))
                        SetAction(o, PLAYER_SIT1);
                    else
                        SetAction(o, PLAYER_SIT_FEMALE1);
                    SendRequestAction(Hero->Object, AT_SIT1);
                }
                PlayBuffer(SOUND_DROP_ITEM01, &Hero->Object);
            }
        }
        break;
    }
}

void CloseNPCGMWindow()
{
    if (!g_pUIManager->IsOpen(INTERFACE_NPCGUILDMASTER))
        return;
    g_pUIManager->Close(INTERFACE_NPCGUILDMASTER);
}

void SendMove(CHARACTER* c, OBJECT* o)
{
    if (g_pNewUISystem->IsImpossibleSendMoveInterface() == true)
    {
        return;
    }

    if (g_isCharacterBuff(o, eDeBuff_Harden))
    {
        return;
    }
    if (g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint))
    {
        return;
    }

    if (c->Path.PathNum <= 2)
    {
        MouseUpdateTimeMax = 0;
    }
    else if (c->Path.PathNum == 3)
    {
        MouseUpdateTimeMax = 5;
    }
    else
    {
        MouseUpdateTimeMax = 10 + (c->Path.PathNum - 2) * 3;
    }

    SendCharacterMove(Hero->Key, o->Angle[2], c->Path.PathNum, &c->Path.PathX[0], &c->Path.PathY[0], TargetX, TargetY);

    c->Movement = true;

    g_pNewUISystem->UpdateSendMoveInterface();

    if (g_bEventChipDialogEnable)
    {
        SocketClient->ToGameServer()->SendEventChipExitDialog();

        if (g_bEventChipDialogEnable == EVENT_SCRATCH_TICKET)
        {
            ClearInput(FALSE);
            InputEnable = false;
            GoldInputEnable = false;
            InputGold = 0;
            StorageGoldFlag = 0;
            g_bScratchTicket = false;
        }
        g_bEventChipDialogEnable = EVENT_NONE;

#ifndef FOR_WORK
#ifdef WINDOWMODE
        if (g_bUseWindowMode == FALSE)
        {
#endif	// WINDOWMODE
            int x = 640 * MouseX / 260;
            SetCursorPos((x)*WindowWidth / 640, (MouseY)*WindowHeight / 480);
#ifdef WINDOWMODE
        }
#endif	// WINDOWMODE
#endif	// FOR_WORK
        MouseUpdateTimeMax = 6;
        MouseLButton = false;
    }
}

int StandTime = 0;
int HeroAngle = 0;

bool CheckMacroLimit(wchar_t* Text)
{
    wchar_t string[256];
    int  length;

    memcpy(string, Text + 3, sizeof(char) * (256 - 2));
    length = GlobalText.GetStringSize(258);
    if (wcscmp(string, GlobalText[258]) == 0 || wcscmp(string, GlobalText[259]) == 0 || wcsicmp(string, L"/trade") == 0)
    {
        return  true;
    }
    if (wcscmp(string, GlobalText[256]) == 0 || wcsicmp(string, L"/party") == 0 || wcsicmp(string, L"/pt") == 0)
    {
        return  true;
    }
    if (wcscmp(string, GlobalText[254]) == 0 || wcsicmp(string, L"/guild") == 0)
    {
        return  true;
    }
    if (wcscmp(string, GlobalText[248]) == 0 || wcsicmp(string, L"/GuildWar") == 0)
    {
        return  true;
    }
    if (wcscmp(string, GlobalText[249]) == 0 || wcsicmp(string, L"/BattleSoccer") == 0)
    {
        return  true;
    }

    return  false;
}

bool CheckCommand(wchar_t* Text, bool bMacroText)
{
    if (g_ConsoleDebug->CheckCommand(Text) == true)
    {
        return true;
    }

    if (bMacroText == false && LogOut == false)
    {
        wchar_t Name[256];
        int iTextSize = 0;
        for (int i = 0; i < 256 && Text[i] != ' ' && Text[i] != '\0'; i++)
        {
            Name[i] = Text[i];
            iTextSize = i;
        }
        Name[iTextSize] = NULL;

        if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
        {
            if (wcscmp(Name, GlobalText[258]) == NULL || wcscmp(Name, GlobalText[259]) == NULL || wcsicmp(Text, L"/trade") == NULL)
            {
                if (gMapManager.InChaosCastle() == true)
                {
                    g_pSystemLogBox->AddText(GlobalText[1150], SEASON3B::TYPE_SYSTEM_MESSAGE);

                    return false;
                }

                if (::IsStrifeMap(gMapManager.WorldActive))
                {
                    g_pSystemLogBox->AddText(GlobalText[3147], SEASON3B::TYPE_SYSTEM_MESSAGE);
                    return false;
                }

                int level = CharacterAttribute->Level;

                if (level < TRADELIMITLEVEL)
                {
                    g_pSystemLogBox->AddText(GlobalText[478], SEASON3B::TYPE_SYSTEM_MESSAGE);
                    return true;
                }

                if (!IsCanTrade() || !EnableMainRender)
                {
                    return true;
                }

                if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                {
                    CHARACTER* c = &CharactersClient[SelectedCharacter];
                    OBJECT* o = &c->Object;
                    if (o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) &&
                        abs((c->PositionX) - (Hero->PositionX)) <= 1 &&
                        abs((c->PositionY) - (Hero->PositionY)) <= 1)
                    {
                        if (IsShopInViewport(c))
                        {
                            g_pSystemLogBox->AddText(GlobalText[493], SEASON3B::TYPE_ERROR_MESSAGE);
                            return true;
                        }

                        SocketClient->ToGameServer()->SendTradeRequest(c->Key);
                        wchar_t message[100]{};
                        swprintf(message, GlobalText[475], c->ID);
                        g_pSystemLogBox->AddText(message, SEASON3B::TYPE_SYSTEM_MESSAGE);
                    }
                }
                else for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
                {
                    CHARACTER* c = &CharactersClient[i];
                    OBJECT* o = &c->Object;

                    if (o->Live && o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) &&
                        abs((c->PositionX) - (Hero->PositionX)) <= 1 &&
                        abs((c->PositionY) - (Hero->PositionY)) <= 1)
                    {
                        if (IsShopInViewport(c))
                        {
                            g_pSystemLogBox->AddText(GlobalText[493], SEASON3B::TYPE_SYSTEM_MESSAGE);
                            return true;
                        }

                        BYTE Dir1 = (BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                        BYTE Dir2 = (BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                        if (abs(Dir1 - Dir2) == 4) {
                            SocketClient->ToGameServer()->SendTradeRequest(c->Key);
                            wchar_t message[100]{};
                            swprintf(message, GlobalText[475], c->ID);
                            g_pSystemLogBox->AddText(message, SEASON3B::TYPE_SYSTEM_MESSAGE);
                            break;
                        }
                    }
                }
                return true;
            }
        }

        if (wcscmp(Text, GlobalText[688]) == 0)
        {
            return false;
        }

        if (wcscmp(Text, GlobalText[1117]) == 0 || wcsicmp(Text, L"/personalshop") == 0)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(GlobalText[1150], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }

            int level = CharacterAttribute->Level;
            if (level >= 6)
            {
                g_pNewUISystem->Show(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
            }
            else
            {
                wchar_t szError[48] = L"";
                swprintf(szError, GlobalText[1123], 6);
                g_pSystemLogBox->AddText(szError, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
            return true;
        }
        if (wcsstr(Text, GlobalText[1118]) > 0 || wcsstr(Text, L"/purchase") > 0)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(GlobalText[1150], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }

            if (::IsStrifeMap(gMapManager.WorldActive))
            {
                g_pSystemLogBox->AddText(GlobalText[3147], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }

            if (
                g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)
                || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)
                || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)
                || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
                || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
                )
            {
                g_pSystemLogBox->AddText(GlobalText[1121], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
            wchar_t szCmd[24];
            wchar_t szId[MAX_ID_SIZE];
            swscanf(Text, L"%s %s", szCmd, szId);

            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
            {
                CHARACTER* c = &CharactersClient[SelectedCharacter];
                OBJECT* o = &c->Object;
                if (o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change))
                {
                    SocketClient->ToGameServer()->SendPlayerShopItemListRequest(c->Key, c->ID);
                }
            }
            else
            {
                for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
                {
                    CHARACTER* c = &CharactersClient[i];
                    if (wcslen(szId) > 0 && c->Object.Live)
                    {
                        if (wcscmp(c->ID, szId) == 0)
                        {
                            SocketClient->ToGameServer()->SendPlayerShopItemListRequest(c->Key, c->ID);
                        }
                    }
                    else
                    {
                        OBJECT* o = &c->Object;
                        if (o->Live && o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change))
                        {
                            BYTE Dir1 = (BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                            BYTE Dir2 = (BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                            if (abs(Dir1 - Dir2) == 4)
                            {
                                SocketClient->ToGameServer()->SendPlayerShopItemListRequest(c->Key, c->ID);
                                break;
                            }
                        }
                    }
                }
            }

            return true;
        }

        if (wcscmp(Text, GlobalText[1136]) == 0)
        {
            ShowShopTitles();
            g_pSystemLogBox->AddText(GlobalText[1138], SEASON3B::TYPE_SYSTEM_MESSAGE);
        }

        if (wcscmp(Text, GlobalText[1137]) == 0)
        {
            HideShopTitles();
            g_pSystemLogBox->AddText(GlobalText[1139], SEASON3B::TYPE_ERROR_MESSAGE);
        }
        if (wcscmp(Text, GlobalText[908]) == 0 || wcsicmp(Text, L"/duelstart") == 0)
        {
#ifndef GUILD_WAR_EVENT
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(GlobalText[1150], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
#endif// UILD_WAR_EVENT
            if (!g_DuelMgr.IsDuelEnabled())
            {
                int iLevel = CharacterAttribute->Level;
                if (iLevel < 30)
                {
                    wchar_t szError[48] = L"";
                    swprintf(szError, GlobalText[2704], 30);
                    g_pSystemLogBox->AddText(szError, SEASON3B::TYPE_ERROR_MESSAGE);
                    return 3;
                }
                else
                    if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                    {
                        CHARACTER* c = &CharactersClient[SelectedCharacter];
                        OBJECT* o = &c->Object;
                        if (o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) &&
                            abs((c->PositionX) - (Hero->PositionX)) <= 1 &&
                            abs((c->PositionY) - (Hero->PositionY)) <= 1) {
                            SocketClient->ToGameServer()->SendDuelStartRequest(c->Key, c->ID);
                        }
                    }
                    else for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
                    {
                        CHARACTER* c = &CharactersClient[i];
                        OBJECT* o = &c->Object;

                        if (o->Live && o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) &&
                            abs((c->PositionX) - (Hero->PositionX)) <= 1 &&
                            abs((c->PositionY) - (Hero->PositionY)) <= 1)
                        {
                            BYTE Dir1 = (BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                            BYTE Dir2 = (BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                            if (abs(Dir1 - Dir2) == 4)
                            {
                                SocketClient->ToGameServer()->SendDuelStartRequest(c->Key, c->ID);
                                break;
                            }
                        }
                    }
            }
            else
            {
                g_pSystemLogBox->AddText(GlobalText[915], SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
        }
        if (wcscmp(Text, GlobalText[909]) == 0 || wcsicmp(Text, L"/duelend") == 0)
        {
#ifndef GUILD_WAR_EVENT
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(GlobalText[1150], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
#endif// GUILD_WAR_EVENT
            if (g_DuelMgr.IsDuelEnabled())
            {
                SocketClient->ToGameServer()->SendDuelStopRequest();
            }
        }
        if (wcscmp(Text, GlobalText[254]) == NULL || wcsicmp(Text, L"/guild") == NULL)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(GlobalText[1150], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
            if (Hero->GuildStatus != G_NONE)
            {
                g_pSystemLogBox->AddText(GlobalText[255], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return true;
            }

            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
            {
                CHARACTER* c = &CharactersClient[SelectedCharacter];
                OBJECT* o = &c->Object;
                if (o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) &&
                    abs((c->PositionX) - (Hero->PositionX)) <= 1 &&
                    abs((c->PositionY) - (Hero->PositionY)) <= 1)
                {
                    GuildPlayerKey = c->Key;
                    SocketClient->ToGameServer()->SendGuildJoinRequest(c->Key);
                    wchar_t Text[100];
                    swprintf(Text, GlobalText[477], c->ID);
                    g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
                }
            }
            else for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            {
                CHARACTER* c = &CharactersClient[i];
                OBJECT* o = &c->Object;

                if (o->Live && o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) &&
                    abs((c->PositionX) - (Hero->PositionX)) <= 1 &&
                    abs((c->PositionY) - (Hero->PositionY)) <= 1)
                {
                    BYTE Dir1 = (BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                    BYTE Dir2 = (BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                    if (abs(Dir1 - Dir2) == 4)
                    {
                        GuildPlayerKey = c->Key;
                        SocketClient->ToGameServer()->SendGuildJoinRequest(c->Key);
                        wchar_t Text[100];
                        swprintf(Text, GlobalText[477], c->ID);
                        g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
                        break;
                    }
                }
            }
            return true;
        }
        if (!wcscmp(Text, GlobalText[1354]) || !wcsicmp(Text, L"/union") ||
            !wcscmp(Text, GlobalText[1356]) || !wcsicmp(Text, L"/rival") ||
            !wcscmp(Text, GlobalText[1357]) || !wcsicmp(Text, L"/rivaloff"))
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(GlobalText[1150], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
            if (Hero->GuildStatus == G_NONE)
            {
                g_pSystemLogBox->AddText(GlobalText[1355], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return true;
            }

            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
            {
                CHARACTER* c = &CharactersClient[SelectedCharacter];
                OBJECT* o = &c->Object;
                if (o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) &&
                    abs((c->PositionX) - (Hero->PositionX)) <= 1 &&
                    abs((c->PositionY) - (Hero->PositionY)) <= 1)
                {
                    if (!wcscmp(Text, GlobalText[1354]) || !wcsicmp(Text, L"/union"))
                    {
                        //SendRequestGuildRelationShip(0x01, 0x01, HIBYTE(CharactersClient[SelectedCharacter].Key), LOBYTE(CharactersClient[SelectedCharacter].Key));
                        SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x01, 0x01, CharactersClient[SelectedCharacter].Key);
                    }
                    else if (!wcscmp(Text, GlobalText[1356]) || !wcsicmp(Text, L"/rival"))
                    {
                        //SendRequestGuildRelationShip(0x02, 0x01, HIBYTE(CharactersClient[SelectedCharacter].Key), LOBYTE(CharactersClient[SelectedCharacter].Key));
                        SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x02, 0x01, CharactersClient[SelectedCharacter].Key);
                    }
                    else
                    {
                        SetAction(&Hero->Object, PLAYER_RESPECT1);
                        SendRequestAction(Hero->Object, AT_RESPECT1);
                        //SendRequestGuildRelationShip(0x02, 0x02, HIBYTE(CharactersClient[SelectedCharacter].Key), LOBYTE(CharactersClient[SelectedCharacter].Key));
                        SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x02, 0x02, CharactersClient[SelectedCharacter].Key);
                    }
                }
            }
            else for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            {
                CHARACTER* c = &CharactersClient[i];
                OBJECT* o = &c->Object;

                if (o->Live && o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) &&
                    abs((c->PositionX) - (Hero->PositionX)) <= 1 &&
                    abs((c->PositionY) - (Hero->PositionY)) <= 1)
                {
                    BYTE Dir1 = (BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                    BYTE Dir2 = (BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                    if (abs(Dir1 - Dir2) == 4) {
                        if (!wcscmp(Text, GlobalText[1354]) || !wcsicmp(Text, L"/union"))
                        {
                            //SendRequestGuildRelationShip(0x01, 0x01, HIBYTE(c->Key), LOBYTE(c->Key));
                            SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x01, 0x01, c->Key);
                        }
                        else if (!wcscmp(Text, GlobalText[1356]) || !wcsicmp(Text, L"/rival"))
                        {
                            //SendRequestGuildRelationShip(0x02, 0x01, HIBYTE(c->Key), LOBYTE(c->Key));
                            SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x02, 0x01, c->Key);
                        }
                        else
                        {
                            SetAction(&Hero->Object, PLAYER_RESPECT1);
                            SendRequestAction(Hero->Object, AT_RESPECT1);
                            //SendRequestGuildRelationShip(0x02, 0x02, HIBYTE(c->Key), LOBYTE(c->Key));
                            SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(0x02, 0x02, c->Key);
                        }
                        break;
                    }
                }
            }
            return true;
        }
        if (wcscmp(Text, GlobalText[256]) == NULL || wcsicmp(Text, L"/party") == NULL || wcsicmp(Text, L"/pt") == NULL)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(GlobalText[1150], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
            if (PartyNumber > 0 && wcscmp(Party[0].Name, Hero->ID) != NULL)
            {
                g_pSystemLogBox->AddText(GlobalText[257], SEASON3B::TYPE_SYSTEM_MESSAGE);
                return true;
            }

            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
            {
                CHARACTER* c = &CharactersClient[SelectedCharacter];
                OBJECT* o = &c->Object;
                if (o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) && abs((c->PositionX) - (Hero->PositionX)) <= 1 && abs((c->PositionY) - (Hero->PositionY)) <= 1)
                {
                    PartyKey = c->Key;
                    SocketClient->ToGameServer()->SendPartyInviteRequest(c->Key);
                    wchar_t Text[100];
                    swprintf(Text, GlobalText[476], c->ID);
                    g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
                }
            }
            else for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            {
                CHARACTER* c = &CharactersClient[i];
                OBJECT* o = &c->Object;
                if (o->Live && o->Kind == KIND_PLAYER && c != Hero && (o->Type == MODEL_PLAYER || c->Change) && abs((c->PositionX) - (Hero->PositionX)) <= 1 && abs((c->PositionY) - (Hero->PositionY)) <= 1)
                {
                    BYTE Dir1 = (BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                    BYTE Dir2 = (BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                    if (abs(Dir1 - Dir2) == 4)
                    {
                        PartyKey = c->Key;
                        SocketClient->ToGameServer()->SendPartyInviteRequest(c->Key);
                        wchar_t Text[100];
                        swprintf(Text, GlobalText[476], c->ID);
                        g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
                        break;
                    }
                }
            }
            return true;
        }
        if (wcsicmp(Text, L"/charactername") == NULL)
        {
            if (IsGMCharacter() == true)
            {
                g_bGMObservation = !g_bGMObservation;
                return true;
            }
        }

        for (int i = 0; i < 10; i++)
        {
            wchar_t Name[256];
            if (i != 9)
                swprintf(Name, L"/%d", i + 1);
            else
                swprintf(Name, L"/%d", 0);
            if (Text[0] == Name[0] && Text[1] == Name[1])
            {
                if (CheckMacroLimit(Text) == true)
                {
                    return  false;
                }

                int iTextSize = 0;
                for (int j = 3; j <= (int)wcslen(Text); j++)
                {
                    MacroText[i][j - 3] = Text[j];
                    iTextSize = j;
                }
                MacroText[i][iTextSize - 3] = NULL;
                PlayBuffer(SOUND_CLICK01);
                return true;
            }
        }

        wchar_t lpszFilter[] = L"/filter";
        if ((GlobalText.GetStringSize(753) > 0 && wcsncmp(Text, GlobalText[753], GlobalText.GetStringSize(753)) == NULL)
            || (wcsncmp(Text, lpszFilter, wcslen(lpszFilter)) == NULL))
        {
            g_pChatListBox->SetFilterText(Text);
        }
    }
#ifdef CSK_FIX_MACRO_MOVEMAP
    else if (bMacroText == true)
    {
        wchar_t Name[256];

        int iTextSize = 0;
        for (int i = 0; i < 256 && Text[i] != ' ' && Text[i] != '\0'; i++)
        {
            Name[i] = Text[i];
            iTextSize = i;
        }
        Name[iTextSize] = NULL;

        if (wcscmp(Name, GlobalText[260]) == 0 || wcsicmp(Name, L"/move") == 0)
        {
            if (IsGMCharacter() == true || FindText2(Hero->ID, L"webzen") == true)
            {
                return false;
            }
            return true;
        }
    }
#endif // CSK_FIX_MACRO_MOVEMAP

    if (IsIllegalMovementByUsingMsg(Text))
        return TRUE;

    for (int i = 0; i < 16 * MAX_ITEM_INDEX; ++i)
    {
        ITEM_ATTRIBUTE* p = &ItemAttribute[i];

        if (p->Width != 0)
        {
            wchar_t Name[256];
            swprintf(Name, L"/%s", p->Name);

            if (wcsicmp(Text, Name) == NULL)
            {
                g_csItemOption.ClearOptionHelper();

                g_pNewUISystem->Show(SEASON3B::INTERFACE_ITEM_EXPLANATION);

                ItemHelp = i;
                PlayBuffer(SOUND_CLICK01);
                return true;
            }
        }
    }

    g_csItemOption.CheckRenderOptionHelper(Text);

    return (IsIllegalMovementByUsingMsg(Text)) ? TRUE : FALSE;
    return false;
}

//bool IsWebzenCharacter()
//{
//    const std::wstring character_name = std::wstring(Hero->ID);
//
//    return character_name.find(L"webzen") >= 0;
//}

bool FindText(const wchar_t* Text, const wchar_t* Token, bool First)
{
    int LengthToken = (int)wcslen(Token);
    int Length = (int)wcslen(Text) - LengthToken;
    if (First)
        Length = 0;
    if (Length < 0)
        return false;

    auto* lpszCheck = (unsigned char*)Text;
    for (int i = 0; i <= Length; i += _mbclen(lpszCheck + i))
    {
        bool Success = true;
        for (int j = 0; j < LengthToken; j++)
        {
            if (Text[i + j] != Token[j])
            {
                Success = false;
                break;
            }
        }
        if (Success)
            return true;
    }
    return false;
}

bool FindTextABS(const wchar_t* Text, const wchar_t* Token, bool First)
{
    int LengthToken = (int)wcslen(Token);
    int Length = (int)wcslen(Text) - LengthToken;
    if (First)
        Length = 0;
    if (Length < 0)
        return false;

    auto* lpszCheck = (unsigned char*)Text;
    for (int i = 0; i <= Length; i += _mbclen(lpszCheck + i))
    {
        bool Success = true;
        for (int j = 0; j < LengthToken; j++)
        {
            if (Text[i + j] != Token[j])
            {
                Success = false;
                break;
            }
        }
        if (Success)
            return true;
    }
    return false;
}

void SetActionClass(CHARACTER* c, OBJECT* o, int Action, int ActionType)
{
    if ((o->CurrentAction >= PLAYER_STOP_MALE && o->CurrentAction <= PLAYER_STOP_RIDE_WEAPON)
        || o->CurrentAction == PLAYER_STOP_TWO_HAND_SWORD_TWO
        || o->CurrentAction == PLAYER_RAGE_UNI_STOP_ONE_RIGHT
        || o->CurrentAction == PLAYER_STOP_RAGEFIGHTER)
    {
        if (!gCharacterManager.IsFemale(c->Class) || (Action >= PLAYER_RESPECT1 && Action <= PLAYER_RUSH1))
            SetAction(o, Action);
        else
            SetAction(o, Action + 1);
        SendRequestAction(Hero->Object, ActionType);
    }
}

void CheckChatText(wchar_t* Text)
{
    CHARACTER* c = Hero;
    OBJECT* o = &c->Object;
    if (FindText(Text, GlobalText[270]) || FindText(Text, GlobalText[271]) || FindText(Text, GlobalText[272]) || FindText(Text, GlobalText[273]) || FindText(Text, GlobalText[274]) || FindText(Text, GlobalText[275]) || FindText(Text, GlobalText[276]) || FindText(Text, GlobalText[277]))
    {
        SetActionClass(c, o, PLAYER_GREETING1, AT_GREETING1);
        SendRequestAction(Hero->Object, AT_GREETING1);
    }
    else if (FindText(Text, GlobalText[278]) || FindText(Text, GlobalText[279]) || FindText(Text, GlobalText[280]))
    {
        SetActionClass(c, o, PLAYER_GOODBYE1, AT_GOODBYE1);
        SendRequestAction(Hero->Object, AT_GOODBYE1);
    }
    else if (FindText(Text, GlobalText[281]) || FindText(Text, GlobalText[282]) || FindText(Text, GlobalText[283]) || FindText(Text, GlobalText[284]) || FindText(Text, GlobalText[285]) || FindText(Text, GlobalText[286]))
    {
        SetActionClass(c, o, PLAYER_CLAP1, AT_CLAP1);
        SendRequestAction(Hero->Object, AT_CLAP1);
    }
    else if (FindText(Text, GlobalText[287]) || FindText(Text, GlobalText[288]) || FindText(Text, GlobalText[289]) || FindText(Text, GlobalText[290]))
    {
        SetActionClass(c, o, PLAYER_GESTURE1, AT_GESTURE1);
        SendRequestAction(Hero->Object, AT_GESTURE1);
    }
    else if (FindText(Text, GlobalText[292]) || FindText(Text, GlobalText[293]) || FindText(Text, GlobalText[294]) || FindText(Text, GlobalText[295]))
    {
        SetActionClass(c, o, PLAYER_DIRECTION1, AT_DIRECTION1);
        SendRequestAction(Hero->Object, AT_DIRECTION1);
    }
    else if (FindText(Text, GlobalText[296]) || FindText(Text, GlobalText[297]) || FindText(Text, GlobalText[298]) || FindText(Text, GlobalText[299]) || FindText(Text, GlobalText[300]) || FindText(Text, GlobalText[301]) || FindText(Text, GlobalText[302]))
    {
        SetActionClass(c, o, PLAYER_UNKNOWN1, AT_UNKNOWN1);
        SendRequestAction(Hero->Object, AT_UNKNOWN1);
    }
    else if (FindText(Text, L";") || FindText(Text, GlobalText[303]) || FindText(Text, GlobalText[304]) || FindText(Text, GlobalText[305]))
    {
        SetActionClass(c, o, PLAYER_AWKWARD1, AT_AWKWARD1);
        SendRequestAction(Hero->Object, AT_AWKWARD1);
    }
    else if (FindText(Text, L"ㅠ.ㅠ") || FindText(Text, L"ㅜ.ㅜ") || FindText(Text, L"T_T") || FindText(Text, GlobalText[306]) || FindText(Text, GlobalText[307]) || FindText(Text, GlobalText[308]) || FindText(Text, GlobalText[309]))
    {
        SetActionClass(c, o, PLAYER_CRY1, AT_CRY1);
        SendRequestAction(Hero->Object, AT_CRY1);
    }
    else if (FindText(Text, L"ㅡ.ㅡ") || FindText(Text, L"ㅡ.,ㅡ") || FindText(Text, L"ㅡ,.ㅡ") || FindText(Text, L"-.-") || FindText(Text, L"-_-") || FindText(Text, GlobalText[310]) || FindText(Text, GlobalText[311]))
    {
        SetActionClass(c, o, PLAYER_SEE1, AT_SEE1);
        SendRequestAction(Hero->Object, AT_SEE1);
    }
    else if (FindText(Text, L"^^") || FindText(Text, L"^.^") || FindText(Text, L"^_^") || FindText(Text, GlobalText[312]) || FindText(Text, GlobalText[313]) || FindText(Text, GlobalText[314]) || FindText(Text, GlobalText[315]) || FindText(Text, GlobalText[316]))
    {
        SetActionClass(c, o, PLAYER_SMILE1, AT_SMILE1);
        SendRequestAction(Hero->Object, AT_SMILE1);
    }
    else if (FindText(Text, GlobalText[318]) || FindText(Text, GlobalText[319]) || FindText(Text, GlobalText[320]) || FindText(Text, GlobalText[321]))
    {
        SetActionClass(c, o, PLAYER_CHEER1, AT_CHEER1);
        SendRequestAction(Hero->Object, AT_CHEER1);
    }
    else if (FindText(Text, GlobalText[322]) || FindText(Text, GlobalText[323]) || FindText(Text, GlobalText[324]) || FindText(Text, GlobalText[325]))
    {
        SetActionClass(c, o, PLAYER_WIN1, AT_WIN1);
        SendRequestAction(Hero->Object, AT_WIN1);
    }
    else if (FindText(Text, GlobalText[326]) || FindText(Text, GlobalText[327]) || FindText(Text, GlobalText[328]) || FindText(Text, GlobalText[329]))
    {
        SetActionClass(c, o, PLAYER_SLEEP1, AT_SLEEP1);
        SendRequestAction(Hero->Object, AT_SLEEP1);
    }
    else if (FindText(Text, GlobalText[330]) || FindText(Text, GlobalText[331]) || FindText(Text, GlobalText[332]) || FindText(Text, GlobalText[333]) || FindText(Text, GlobalText[334]))
    {
        SetActionClass(c, o, PLAYER_COLD1, AT_COLD1);
        SendRequestAction(Hero->Object, AT_COLD1);
    }
    else if (FindText(Text, GlobalText[335]) || FindText(Text, GlobalText[336]) || FindText(Text, GlobalText[337]) || FindText(Text, GlobalText[338]))
    {
        SetActionClass(c, o, PLAYER_AGAIN1, AT_AGAIN1);
        SendRequestAction(Hero->Object, AT_AGAIN1);
    }
    else if (FindText(Text, GlobalText[339]) || FindText(Text, GlobalText[340]) || FindText(Text, GlobalText[341]))
    {
        SetActionClass(c, o, PLAYER_RESPECT1, AT_RESPECT1);
        SendRequestAction(Hero->Object, AT_RESPECT1);
    }
    else if (FindText(Text, GlobalText[342]) || FindText(Text, GlobalText[343]) || FindText(Text, L"/ㅡ") || FindText(Text, L"ㅡ^"))
    {
        SetActionClass(c, o, PLAYER_SALUTE1, AT_SALUTE1);
        SendRequestAction(Hero->Object, AT_SALUTE1);
    }
    else if (FindText(Text, GlobalText[344]) || FindText(Text, GlobalText[345]) || FindText(Text, GlobalText[346]) || FindText(Text, GlobalText[347]))
    {
        SetActionClass(c, o, PLAYER_RUSH1, AT_RUSH1);
        SendRequestAction(Hero->Object, AT_RUSH1);
    }
    else if (FindText(Text, GlobalText[783]) || FindText(Text, L"hustle"))
    {
        SetActionClass(c, o, PLAYER_HUSTLE, AT_HUSTLE);
        SendRequestAction(Hero->Object, AT_HUSTLE);
    }
    else if (FindText(Text, GlobalText[291]))
    {
        SetActionClass(c, o, PLAYER_PROVOCATION, AT_PROVOCATION);
        SendRequestAction(Hero->Object, AT_PROVOCATION);
    }
    else if (FindText(Text, GlobalText[317]))
    {
        SetActionClass(c, o, PLAYER_CHEERS, AT_CHEERS);
        SendRequestAction(Hero->Object, AT_CHEERS);
    }
    else if (FindText(Text, GlobalText[348]))
    {
        SetActionClass(c, o, PLAYER_LOOK_AROUND, AT_LOOK_AROUND);
        SendRequestAction(Hero->Object, AT_LOOK_AROUND);
    }
    else if (FindText(Text, GlobalText[2228]))
    {
        ITEM* pItem_rr = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
        ITEM* pItem_rl = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];

        if (pItem_rr->Type == ITEM_JACK_OLANTERN_TRANSFORMATION_RING || pItem_rl->Type == ITEM_JACK_OLANTERN_TRANSFORMATION_RING)
        {
            if (rand_fps_check(2))
            {
                SetAction(o, PLAYER_JACK_1);
                SendRequestAction(Hero->Object, AT_JACK1);
            }
            else
            {
                SetAction(o, PLAYER_JACK_2);
                SendRequestAction(Hero->Object, AT_JACK2);
            }

            o->m_iAnimation = 0;
        }
    }
    else if (FindText(Text, GlobalText[2243]))
    {
        ITEM* pItem_rr = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
        ITEM* pItem_rl = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];

        if (pItem_rr->Type == ITEM_CHRISTMAS_TRANSFORMATION_RING || pItem_rl->Type == ITEM_CHRISTMAS_TRANSFORMATION_RING)
        {
            if (o->CurrentAction != PLAYER_SANTA_1 && o->CurrentAction != PLAYER_SANTA_2)
            {
                int i = rand() % 3;
                if (rand() % 2)
                {
                    SetAction(o, PLAYER_SANTA_1);
                    SendRequestAction(Hero->Object, AT_SANTA1_1 + i);
                    PlayBuffer(static_cast<ESound>(SOUND_XMAS_JUMP_SANTA + i));
                }
                else
                {
                    SetAction(o, PLAYER_SANTA_2);
                    SendRequestAction(Hero->Object, AT_SANTA2_1 + i);
                    PlayBuffer(SOUND_XMAS_TURN);
                }

                g_XmasEvent->CreateXmasEventEffect(c, o, i);
            }

            o->m_iAnimation = 0;
        }
    }
}

bool CheckTarget(CHARACTER* c)
{
    if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
    {
        TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
        TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);
        VectorCopy(CharactersClient[SelectedCharacter].Object.Position, c->TargetPosition);
        return true;
    }
    else
    {
        RenderTerrain(true);
        if (RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true))
        {
            VectorCopy(CollisionPosition, c->TargetPosition);
            TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
            TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
            return true;
        }
    }
    return false;
}

bool EnableFastInput = false;

void AttackElf(CHARACTER* c, int Skill, float Distance)
{
    OBJECT* o = &c->Object;
    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);
    if (g_isCharacterBuff(o, eBuff_InfinityArrow))
        iMana += CharacterMachine->InfinityArrowAdditionalMana;
    if (iMana > CharacterAttribute->Mana)
    {
        int Index = g_pMyInventory->FindManaItemIndex();

        if (Index != -1)
        {
            SendRequestUse(Index, 0);
        }
        return;
    }

    if (iSkillMana > CharacterAttribute->SkillMana)
    {
        return;
    }
    if (!gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
    {
        return;
    }

    int iEnergy;
    gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
    if (iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
    {
        return;
    }

    bool Success = CheckTarget(c);

    switch (Skill)
    {
    case AT_SKILL_SUMMON:
    case AT_SKILL_SUMMON + 1:
    case AT_SKILL_SUMMON + 2:
    case AT_SKILL_SUMMON + 3:
    case AT_SKILL_SUMMON + 4:
    case AT_SKILL_SUMMON + 5:
    case AT_SKILL_SUMMON + 6:
#ifdef ADD_ELF_SUMMON
    case AT_SKILL_SUMMON + 7:
#endif // ADD_ELF_SUMMON
        if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.InChaosCastle() == false)
            if (!g_Direction.m_CKanturu.IsMayaScene())
            {
                SendRequestMagic(Skill, HeroKey);
                SetPlayerMagic(c);
            }
        return;

    case AT_SKILL_PENETRATION:
    case AT_SKILL_PENETRATION_STR:
        if ((o->Type == MODEL_PLAYER) && (gCharacterManager.GetEquipedBowType(Hero) != BOWTYPE_NONE))
        {
            if (!CheckArrow())
                break;
            if (CheckTile(c, o, Distance))
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                WORD TKey = 0xffff;
                if (g_MovementSkill.m_iTarget != -1)
                {
                    TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                }
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
                SetPlayerAttack(c);
                if (o->Type != MODEL_PLAYER)
                {
                    CreateArrows(c, o, NULL, FindHotKey((c->Skill)), 0, (c->Skill));
                }
            }
        }
        break;
    }
    if (SelectedCharacter != -1)
    {
        ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
        g_MovementSkill.m_bMagic = TRUE;
        g_MovementSkill.m_iSkill = Hero->CurrentSkill;
        g_MovementSkill.m_iTarget = SelectedCharacter;
    }
    if (!CheckTile(c, o, Distance))
    {
        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && (
            Skill == AT_SKILL_HEALING
            || Skill == AT_SKILL_HEALING_STR
            || Skill == AT_SKILL_ATTACK
            || Skill == AT_SKILL_ATTACK_STR
            || Skill == AT_SKILL_ATTACK_MASTERY
            || Skill == AT_SKILL_DEFENSE
            || Skill == AT_SKILL_DEFENSE_STR
            || Skill == AT_SKILL_DEFENSE_MASTERY
            || CheckAttack()
            || Skill == AT_SKILL_FIRE_SCREAM
            || Skill == AT_SKILL_FIRE_SCREAM_STR
            )
            )
        {
            if (CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
            {
                if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                {
                    c->Movement = true;
                    c->MovementType = MOVEMENT_SKILL;
                    SendMove(c, o);
                }
            }
        }

        if (Skill != AT_SKILL_STUN && Skill != AT_SKILL_REMOVAL_STUN && Skill != AT_SKILL_MANA && Skill != AT_SKILL_INVISIBLE && Skill != AT_SKILL_REMOVAL_INVISIBLE
            && Skill != AT_SKILL_PLASMA_STORM_FENRIR
            )
            return;
    }
    bool Wall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
    if (Wall)
    {
        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
        {
            if (CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
            {
                switch (Skill)
                {
                case AT_SKILL_HEALING:
                case AT_SKILL_HEALING_STR:
                case AT_SKILL_ATTACK:
                case AT_SKILL_ATTACK_STR:
                case AT_SKILL_ATTACK_MASTERY:
                case AT_SKILL_DEFENSE:
                case AT_SKILL_DEFENSE_STR:
                case AT_SKILL_DEFENSE_MASTERY:
                    UseSkillElf(c, o);
                    return;
                }
            }
            if (CheckAttack())
            {
                if (((Skill == AT_SKILL_ICE_ARROW) || (Skill == AT_SKILL_ICE_ARROW_STR) || (Skill == AT_SKILL_DEEPIMPACT)) && ((o->Type == MODEL_PLAYER) && (gCharacterManager.GetEquipedBowType(Hero) != BOWTYPE_NONE)))
                {
                    UseSkillElf(c, o);
                }
            }
        }
    }

    switch (Skill)
    {
    case AT_SKILL_INFINITY_ARROW:
    case AT_SKILL_INFINITY_ARROW_STR:
    {
        if (g_isCharacterBuff((&Hero->Object), eBuff_InfinityArrow) == false)
        {
            SendRequestMagic(Skill, HeroKey);
            if ((c->Helper.Type == MODEL_HORN_OF_FENRIR)
                || (c->Helper.Type == MODEL_HORN_OF_UNIRIA)
                || (c->Helper.Type == MODEL_HORN_OF_DINORANT)
                || (c->Helper.Type == MODEL_DARK_HORSE_ITEM))
                SetPlayerMagic(c);
            else
                SetAction(o, PLAYER_RUSH1);

            c->Movement = 0;
        }
    }
    break;

    case AT_SKILL_HELLOWIN_EVENT_1:
    case AT_SKILL_HELLOWIN_EVENT_2:
    case AT_SKILL_HELLOWIN_EVENT_3:
    case AT_SKILL_HELLOWIN_EVENT_4:
    case AT_SKILL_HELLOWIN_EVENT_5:
        SendRequestMagic(Skill, HeroKey);
        //					SetPlayerMagic(c);
        c->Movement = 0;
        break;
    case AT_SKILL_RECOVER:
    {
        vec3_t Light, Position, P, dp;

        float Matrix[3][4];

        Vector(0.f, -220.f, 130.f, P);
        AngleMatrix(o->Angle, Matrix);
        VectorRotate(P, Matrix, dp);
        VectorAdd(dp, o->Position, Position);
        Vector(0.7f, 0.6f, 0.f, Light);
        CreateEffect(BITMAP_IMPACT, Position, o->Angle, Light, 0, o);
        SetAction(o, PLAYER_RECOVER_SKILL);

        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
        {
            SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
        }
        else
        {
            SendRequestMagic(Skill, HeroKey);
        }
        //			UseSkillElf( c, o);
    }
    break;
    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
        SendRequestMagic(Skill, HeroKey);
        SetPlayerMagic(c);
        return;
    case AT_SKILL_MULTI_SHOT:
    {
        if (!CheckArrow())
            break;

        if (gCharacterManager.GetEquipedBowType_Skill() == BOWTYPE_NONE)
            return;
        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

        if (CheckTile(c, o, Distance))
        {
            BYTE PathX[1];
            BYTE PathY[1];
            PathX[0] = (c->PositionX);
            PathY[0] = (c->PositionY);

            SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

            BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

            BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
            WORD TKey = 0xffff;
            if (g_MovementSkill.m_iTarget != -1)
            {
                TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
            }

            SendRequestMagicContinue(Skill, (c->PositionX),
                (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
            SetAttackSpeed();
            //									SetAction(o, PLAYER_SKILL_FLAMESTRIKE);
            c->Movement = 0;
            //c->AttackTime = 15;

            SetPlayerBow(c);
            vec3_t Light, Position, P, dp;

            float Matrix[3][4];
            Vector(0.f, 20.f, 0.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);
            Vector(0.8f, 0.9f, 1.6f, Light);
            CreateEffect(MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);
            CreateEffect(MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);

            Vector(0.f, -20.f, 0.f, P);
            Vector(0.f, 0.f, 0.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);

            CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
            CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
            CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);

            Vector(0.f, 20.f, 0.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);
            CreateEffect(MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);
            CreateEffect(MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);

            Vector(0.f, -120.f, 145.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);

            short Key = -1;
            for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            {
                CHARACTER* tc = &CharactersClient[i];
                if (tc == c)
                {
                    Key = i;
                    break;
                }
            }

            CreateEffect(MODEL_BLADE_SKILL, Position, o->Angle, Light, 1, o, Key);
            PlayBuffer(SOUND_SKILL_MULTI_SHOT);
        }
    }
    break;
    case AT_SKILL_IMPROVE_AG:
        SendRequestMagic(Skill, HeroKey);
        SetPlayerMagic(c);
        c->Movement = 0;
        break;
    case AT_SKILL_PLASMA_STORM_FENRIR:
    {
        if (CheckAttack())
        {
            g_MovementSkill.m_iTarget = SelectedCharacter;
        }
        else
        {
            g_MovementSkill.m_iTarget = -1;
        }

        int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
        int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
        if (CheckTile(c, o, Distance))
        {
            BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
            WORD TKey = 0xffff;
            if (g_MovementSkill.m_iTarget != -1)
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                gSkillManager.CheckSkillDelay(Hero->CurrentSkill);

                TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                c->m_iFenrirSkillTarget = g_MovementSkill.m_iTarget;
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, &o->m_bySkillSerialNum);
                c->Movement = 0;

                if (o->Type == MODEL_PLAYER)
                {
                    SetAction_Fenrir_Skill(c, o);
                }
            }
            else
            {
                c->m_iFenrirSkillTarget = -1;
            }
        }
        else
        {
            if (g_MovementSkill.m_iTarget != -1)
            {
                if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance * 1.2f))
                {
                    c->Movement = true;
                }
            }
            else
            {
                Attacking = -1;
            }
        }
    }
    break;
    }

    c->SkillSuccess = true;
}

void AttackKnight(CHARACTER* c, ActionSkillType Skill, float Distance)
{
    OBJECT* o = &c->Object;
    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana,
        NULL, &iSkillMana);

    int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
    int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

    if (((iTypeR != -1 &&
        (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
        (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX))
        || Skill == AT_SKILL_SWELL_LIFE
        || Skill == AT_SKILL_SWELL_LIFE_STR
        || Skill == AT_SKILL_SWELL_LIFE_PROFICIENCY
        || Skill == AT_SKILL_ADD_CRITICAL
        || Skill == AT_SKILL_ADD_CRITICAL_STR1
        || Skill == AT_SKILL_ADD_CRITICAL_STR2
        || Skill == AT_SKILL_ADD_CRITICAL_STR3
        || Skill == AT_SKILL_PARTY_TELEPORT
        || Skill == AT_SKILL_THUNDER_STRIKE
        || Skill == AT_SKILL_EARTHSHAKE
        || Skill == AT_SKILL_EARTHSHAKE_STR
        || Skill == AT_SKILL_EARTHSHAKE_MASTERY
        || Skill == AT_SKILL_BRAND_OF_SKILL
        || Skill == AT_SKILL_PLASMA_STORM_FENRIR
        || Skill == AT_SKILL_FIRE_SCREAM
        || Skill == AT_SKILL_FIRE_SCREAM_STR
        || Skill == AT_SKILL_GIGANTIC_STORM
        || Skill == AT_SKILL_GAOTIC
        || Skill == AT_SKILL_TWISTING_SLASH
        || Skill == AT_SKILL_TWISTING_SLASH_STR
        || Skill == AT_SKILL_TWISTING_SLASH_STR_MG
        || Skill == AT_SKILL_TWISTING_SLASH_MASTERY
        ))
    {
        bool Success = true;

        if (Skill == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
        {
            Success = false;
        }

        if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(Skill))
        {
            Success = false;
        }

        if (Skill == AT_SKILL_PARTY_TELEPORT && g_DuelMgr.IsDuelEnabled())
        {
            Success = false;
        }

        if (Skill == AT_SKILL_PARTY_TELEPORT && (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4()))
        {
            Success = false;
        }

        if (Skill == AT_SKILL_EARTHSHAKE || Skill == AT_SKILL_EARTHSHAKE_STR || Skill == AT_SKILL_EARTHSHAKE_MASTERY)
        {
            BYTE t_DarkLife = 0;
            t_DarkLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
            if (t_DarkLife == 0) Success = false;
        }

        if (gMapManager.InChaosCastle())
        {
            if (Skill == AT_SKILL_EARTHSHAKE
                || Skill == AT_SKILL_EARTHSHAKE_STR
                || Skill == AT_SKILL_EARTHSHAKE_MASTERY
                || Skill == AT_SKILL_RIDER
                || (Skill >= AT_PET_COMMAND_DEFAULT && Skill <= AT_PET_COMMAND_TARGET)
                || Skill == AT_SKILL_PLASMA_STORM_FENRIR
                )
            {
                Success = false;
            }
        }
        else
        {
            if (Skill == AT_SKILL_EARTHSHAKE || Skill == AT_SKILL_EARTHSHAKE_STR || Skill == AT_SKILL_EARTHSHAKE_MASTERY)
            {
                BYTE t_DarkLife = 0;
                t_DarkLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
                if (t_DarkLife == 0) Success = false;
            }
        }

        if (iMana > CharacterAttribute->Mana)
        {
            int Index = g_pMyInventory->FindManaItemIndex();
            if (Index != -1)
            {
                SendRequestUse(Index, 0);
            }
            Success = false;
        }
        if (Success && iSkillMana > CharacterAttribute->SkillMana)
        {
            Success = false;
        }
        if (Success && !gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
        {
            Success = false;
        }

        int iEnergy;
        gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
        if (iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
        {
            Success = false;
        }
        if (ClassIndex == CLASS_DARK_LORD)
        {
            int iCharisma;
            gSkillManager.GetSkillInformation_Charisma(Skill, &iCharisma);
            if (iCharisma > (CharacterAttribute->Charisma + CharacterAttribute->AddCharisma))
            {
                Success = false;
            }
        }

        if (Success)
        {
            switch (Skill)
            {
            case AT_SKILL_TWISTING_SLASH_STR:
            case AT_SKILL_TWISTING_SLASH_STR_MG:
            case AT_SKILL_TWISTING_SLASH_MASTERY:
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                {
                    BYTE PathX[1];
                    BYTE PathY[1];
                    PathX[0] = (c->PositionX);
                    PathY[0] = (c->PositionY);

                    SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
                    SetAttackSpeed();
                    SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);

                    c->Movement = 0;
                }
                break;
            case AT_SKILL_TWISTING_SLASH:
            case AT_SKILL_FIRE_SLASH:
            case AT_SKILL_FIRE_SLASH_STR:
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                {
                    WORD Strength;
                    const WORD notStrength = 596;
                    Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
                    if (Strength < notStrength)
                    {
                        break;
                    }

                    if (CheckTile(c, o, Distance))
                    {
                        BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                        WORD TKey = 0xffff;
                        if (g_MovementSkill.m_iTarget != -1)
                        {
                            TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                        }
                        SendRequestMagicContinue(Skill, (c->PositionX),
                            (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), byValue, 0, TKey, 0);
                        SetAttackSpeed();

                        SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);
                        c->Movement = 0;
                    }
                }
                break;
            case AT_SKILL_POWER_SLASH:
            case AT_SKILL_POWER_SLASH_STR:
                if (c->Helper.Type<MODEL_HORN_OF_UNIRIA || c->Helper.Type>MODEL_DARK_HORSE_ITEM && c->Helper.Type != MODEL_HORN_OF_FENRIR)
                {
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                    if (CheckTile(c, o, Distance))
                    {
                        BYTE PathX[1];
                        BYTE PathY[1];
                        PathX[0] = (c->PositionX);
                        PathY[0] = (c->PositionY);

                        SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                        BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                        BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                        WORD TKey = 0xffff;
                        if (g_MovementSkill.m_iTarget != -1)
                        {
                            TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                        }
                        SendRequestMagicContinue(Skill, (c->PositionX),
                            (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                        SetAttackSpeed();
                        if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
                        }
                        else
                        {
                            SetAction(o, PLAYER_ATTACK_TWO_HAND_SWORD_TWO);
                        }
                        c->Movement = 0;
                    }
                }
                break;
            case AT_SKILL_GAOTIC:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                if (CheckTile(c, o, Distance))
                {
                    BYTE PathX[1];
                    BYTE PathY[1];
                    PathX[0] = (c->PositionX);
                    PathY[0] = (c->PositionY);

                    SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget >= 0 && g_MovementSkill.m_iTarget < MAX_CHARACTERS_CLIENT)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                        if (TKey == 0xffff)
                        {
                            CHARACTER* st = &CharactersClient[g_MovementSkill.m_iTarget];
                            TKey = st->Key;
                        }
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                    SetAttackSpeed();
                    //									SetAction(o, PLAYER_SKILL_FLAMESTRIKE);
                    c->Movement = 0;

                    //									SetPlayerMagic(c);

                    if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
                    {
                        SetAction(o, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
                    }
                    else if ((c->Helper.Type >= MODEL_HORN_OF_UNIRIA) && (c->Helper.Type <= MODEL_DARK_HORSE_ITEM))
                    {
                        SetAction(o, PLAYER_ATTACK_RIDE_STRIKE);
                    }
                    else
                    {
                        SetAction(o, PLAYER_ATTACK_STRIKE);
                    }

                    vec3_t Light, Position, P, dp;

                    float Matrix[3][4];
                    Vector(0.f, -20.f, 0.f, P);
                    Vector(0.f, 0.f, 0.f, P);
                    AngleMatrix(o->Angle, Matrix);
                    VectorRotate(P, Matrix, dp);
                    VectorAdd(dp, o->Position, Position);

                    Vector(0.5f, 0.5f, 0.5f, Light);
                    for (int i = 0; i < 5; ++i)
                    {
                        CreateEffect(BITMAP_SHINY + 6, Position, o->Angle, Light, 3, o, -1, 0, 0, 0, 0.5f);
                    }

                    VectorCopy(o->Position, Position);

                    for (int i = 0; i < 8; i++)
                    {
                        Position[0] = (o->Position[0] - 119.f) + (float)(rand() % 240);
                        Position[2] = (o->Position[2] + 49.f) + (float)(rand() % 60);
                        CreateJoint(BITMAP_2LINE_GHOST, Position, o->Position, o->Angle, 0, o, 20.f, o->PKKey, 0, o->m_bySkillSerialNum);
                    }
                    if (c == Hero && SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                    {
                        vec3_t Pos;
                        CHARACTER* sc = &CharactersClient[SelectedCharacter];
                        VectorCopy(sc->Object.Position, Pos);
                        CreateBomb(Pos, true, 6);
                    }
                    PlayBuffer(SOUND_SKILL_CAOTIC);
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                    {
                        c->Movement = true;
                    }
                }
            }
            break;
            case AT_SKILL_FLAME_STRIKE:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                if (CheckTile(c, o, Distance))
                {
                    BYTE PathX[1];
                    BYTE PathY[1];
                    PathX[0] = (c->PositionX);
                    PathY[0] = (c->PositionY);

                    SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                    SetAttackSpeed();
                    SetAction(o, PLAYER_SKILL_FLAMESTRIKE);
                    c->Movement = 0;
                    //c->AttackTime = 15;
                }
            }
            break;
            case AT_SKILL_GIGANTIC_STORM:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                if (CheckTile(c, o, Distance))
                {
                    BYTE PathX[1];
                    BYTE PathY[1];
                    PathX[0] = (c->PositionX);
                    PathY[0] = (c->PositionY);

                    SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                    SetAttackSpeed();
                    SetAction(o, PLAYER_SKILL_GIGANTICSTORM);
                    c->Movement = 0;
                }
            }
            break;
            case AT_SKILL_PARTY_TELEPORT:
                if (gMapManager.IsCursedTemple()
                    && !g_pMyInventory->IsItem(ITEM_POTION + 64, true))
                {
                    return;
                }
                SendRequestMagic(Skill, HeroKey);
                c->Movement = 0;
                break;

            case AT_SKILL_ADD_CRITICAL:
            case AT_SKILL_ADD_CRITICAL_STR1:
            case AT_SKILL_ADD_CRITICAL_STR2:
            case AT_SKILL_ADD_CRITICAL_STR3:
                SendRequestMagic(Skill, HeroKey);
                c->Movement = 0;
                break;
            case AT_SKILL_BRAND_OF_SKILL:
                SendRequestMagic(Skill, HeroKey);
                c->Movement = 0;
                break;
            case AT_SKILL_FIRE_SCREAM:
            case AT_SKILL_FIRE_SCREAM_STR:
                if (CheckTile(c, o, Distance))
                {
                    int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                    int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    CheckClientArrow(o);
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), byValue, pos, TKey, 0);

                    SetAttackSpeed();
                    {
                        if ((c->Helper.Type >= MODEL_HORN_OF_UNIRIA && c->Helper.Type <= MODEL_DARK_HORSE_ITEM) && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_ATTACK_RIDE_STRIKE);
                        }
                        else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
                        }
                        else
                        {
                            SetAction(o, PLAYER_ATTACK_STRIKE);
                        }
                    }
                    c->Movement = 0;
                }
                else
                {
                    Attacking = -1;
                }
                break;
            case AT_SKILL_EARTHSHAKE:
            case AT_SKILL_EARTHSHAKE_STR:
            case AT_SKILL_EARTHSHAKE_MASTERY:
                if (c->Helper.Type != MODEL_DARK_HORSE_ITEM || c->SafeZone)
                {
                    break;
                }

            case AT_SKILL_THUNDER_STRIKE:
                if (CheckTile(c, o, Distance))
                {
                    int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                    int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                    BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), byValue, pos, TKey, 0);
                    SetAttackSpeed();

                    if (Skill == AT_SKILL_THUNDER_STRIKE)
                    {
                        if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_FLASH);
                        }
                        else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                        {
                            SetAction(o, PLAYER_FENRIR_ATTACK_DARKLORD_FLASH);
                        }
                        else
                        {
                            SetAction(o, PLAYER_SKILL_FLASH);
                        }
                    }
                    else if (Skill == AT_SKILL_EARTHSHAKE || Skill == AT_SKILL_EARTHSHAKE_STR || Skill == AT_SKILL_EARTHSHAKE_MASTERY)
                    {
                        SetAction(o, PLAYER_ATTACK_DARKHORSE);
                        PlayBuffer(SOUND_EARTH_QUAKE);
                    }
                    c->Movement = 0;
                }
                else
                {
                    Attacking = -1;
                }
                break;
            case AT_SKILL_SWELL_LIFE:
            case AT_SKILL_SWELL_LIFE_STR:
            case AT_SKILL_SWELL_LIFE_PROFICIENCY:
                SendRequestMagic(Skill, HeroKey);
                SetAction(o, PLAYER_SKILL_VITALITY);
                c->Movement = 0;
                break;
            case AT_SKILL_RAGEFUL_BLOW:
            case AT_SKILL_RAGEFUL_BLOW_STR:
            case AT_SKILL_RAGEFUL_BLOW_MASTERY:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);

                if (CheckTile(c, o, Distance))
                {
                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, (c->PositionX),
                        (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, 0);
                    SetAction(o, PLAYER_ATTACK_SKILL_FURY_STRIKE);
                    c->Movement = 0;
                }
                else
                {
                    Attacking = -1;
                }
            }
            break;
            case AT_SKILL_STRIKE_OF_DESTRUCTION:
            case AT_SKILL_STRIKE_OF_DESTRUCTION_STR:
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance))
                {
                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                    }
                    SendRequestMagicContinue(Skill, TargetX, TargetY, (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, 0);
                    SetAction(o, PLAYER_SKILL_BLOW_OF_DESTRUCTION);
                    c->Movement = 0;
                }
                else
                {
                    Attacking = -1;
                }
            }
            break;
            case AT_SKILL_PLASMA_STORM_FENRIR:
            {
                int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                if (CheckTile(c, o, Distance))
                {
                    BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                    WORD TKey = 0xffff;
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                        gSkillManager.CheckSkillDelay(Hero->CurrentSkill);

                        TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                        c->m_iFenrirSkillTarget = g_MovementSkill.m_iTarget;
                        SendRequestMagicContinue(Skill, (c->PositionX),
                            (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, &o->m_bySkillSerialNum);
                        c->Movement = 0;

                        if (o->Type == MODEL_PLAYER)
                        {
                            SetAction_Fenrir_Skill(c, o);
                        }
                    }
                    else
                    {
                        c->m_iFenrirSkillTarget = -1;
                    }
                }
                else
                {
                    if (g_MovementSkill.m_iTarget != -1)
                    {
                        if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance * 1.2f))
                        {
                            c->Movement = true;
                        }
                    }
                    else
                    {
                        Attacking = -1;
                    }
                }
            }
            break;
            }
        }
    }

    c->SkillSuccess = true;
}

void AttackWizard(CHARACTER* c, int Skill, float Distance)
{
    OBJECT* o = &c->Object;
    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    int iMana, iSkillMana;
    if (Skill == AT_SKILL_NOVA_BEGIN || Skill == AT_SKILL_NOVA)
    {
        gSkillManager.GetSkillInformation(AT_SKILL_NOVA, 1, NULL, &iMana, NULL, &iSkillMana);

        if (Skill == AT_SKILL_NOVA)
        {
            iSkillMana = 0;
        }
    }
    else
    {
        gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);
    }

    int iEnergy;
    gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
    if (iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
    {
        return;
    }

    if (iMana > CharacterAttribute->Mana)
    {
        int Index = g_pMyInventory->FindManaItemIndex();
        if (Index != -1)
        {
            SendRequestUse(Index, 0);
        }
        return;
    }

    if (iSkillMana > CharacterAttribute->SkillMana)
    {
        if (Skill == AT_SKILL_NOVA_BEGIN || Skill == AT_SKILL_NOVA)
        {
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            MouseRButton = false;

            MouseRButtonPress = 0;
        }
        return;
    }

    if (gSkillManager.CheckSkillDelay(Hero->CurrentSkill) == false)
    {
        return;
    }
    bool Success = CheckTarget(c);

    switch (Skill)
    {
    case AT_SKILL_NOVA_BEGIN:
    {
        SendRequestMagic(Skill, HeroKey);

        SetAttackSpeed();
        SetAction(o, PLAYER_SKILL_HELL_BEGIN);
        c->Movement = 0;
    }
    return;
    case AT_SKILL_NOVA:
    {
        int iTargetKey = getTargetCharacterKey(c, SelectedCharacter);
        if (iTargetKey == -1) {
            iTargetKey = HeroKey;
        }
        SendRequestMagic(Skill, iTargetKey);

        SetAttackSpeed();
        SetAction(o, PLAYER_SKILL_HELL_START);
        c->Movement = 0;
    }
    return;
    case AT_SKILL_SOUL_BARRIER:
    case AT_SKILL_SOUL_BARRIER_STR:
    case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
        if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
        {
            if (CharactersClient[SelectedCharacter].Object.Kind != KIND_PLAYER)
            {
                Attacking = -1;
                return;
            }
            else
            {
                if (c != Hero && !g_pPartyManager->IsPartyMember(SelectedCharacter))
                    return;

                c->TargetCharacter = SelectedCharacter;

                if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                {
                    ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
                    g_MovementSkill.m_bMagic = TRUE;
                    g_MovementSkill.m_iSkill = Hero->CurrentSkill;
                    g_MovementSkill.m_iTarget = SelectedCharacter;
                }

                if (!CheckTile(c, o, Distance))
                {
                    if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                    {
                        if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                        {
                            c->Movement = true;
                            c->MovementType = MOVEMENT_SKILL;
                            SendMove(c, o);
                        }
                    }

                    return;
                }

                SendRequestMagic(Skill, CharactersClient[g_MovementSkill.m_iTarget].Key);
            }
        }
        else
        {
            SendRequestMagic(Skill, HeroKey);
        }
        SetPlayerMagic(c);
        break;
    case AT_SKILL_HELL_FIRE:
    case AT_SKILL_HELL_FIRE_STR:
    {
        SendRequestMagicContinue(Skill, (c->PositionX),
            (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
        SetAttackSpeed();
        SetAction(o, PLAYER_SKILL_HELL);
        c->Movement = 0;
    }
    return;
    case AT_SKILL_INFERNO:
    case AT_SKILL_INFERNO_STR:
    case AT_SKILL_INFERNO_STR_MG:
    {
        SendRequestMagicContinue(Skill, (c->PositionX),
            (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
        SetAttackSpeed();
        SetAction(o, PLAYER_SKILL_INFERNO);
        c->Movement = 0;
    }
    return;
    case AT_SKILL_PLASMA_STORM_FENRIR:
    {
        if (CheckAttack())
        {
            g_MovementSkill.m_iTarget = SelectedCharacter;
        }
        else
        {
            g_MovementSkill.m_iTarget = -1;
        }

        int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
        int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
        if (CheckTile(c, o, Distance))//&& CheckAttack())
        {
            BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
            WORD TKey = 0xffff;
            if (g_MovementSkill.m_iTarget != -1)
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                gSkillManager.CheckSkillDelay(Hero->CurrentSkill);

                TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                c->m_iFenrirSkillTarget = g_MovementSkill.m_iTarget;
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, pos, TKey, &o->m_bySkillSerialNum);
                c->Movement = 0;

                if (o->Type == MODEL_PLAYER)
                {
                    SetAction_Fenrir_Skill(c, o);
                }
            }
            else
            {
                c->m_iFenrirSkillTarget = -1;
            }
        }
        else
        {
            if (g_MovementSkill.m_iTarget != -1)
            {
                if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance * 1.2f))
                {
                    c->Movement = true;
                }
            }
            else
            {
                Attacking = -1;
            }
        }
    }
    return;
    case AT_SKILL_BLOCKING:
    case AT_SKILL_FALLING_SLASH:
    case AT_SKILL_FALLING_SLASH_STR:
    case AT_SKILL_LUNGE:
    case AT_SKILL_LUNGE_STR:
    case AT_SKILL_UPPERCUT:
    case AT_SKILL_CYCLONE:
    case AT_SKILL_CYCLONE_STR:
    case AT_SKILL_CYCLONE_STR_MG:
    case AT_SKILL_SLASH:
    case AT_SKILL_SLASH_STR:
    case AT_SKILL_IMPALE:
        return;
    case AT_SKILL_EXPANSION_OF_WIZARDRY:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_STR:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY:
    {
        if (g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower) == false)
        {
            SendRequestMagic(Skill, HeroKey);

            c->Movement = 0;
        }
    }break;
    }

    if (SelectedCharacter != -1)
    {
        ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
        g_MovementSkill.m_bMagic = TRUE;
        g_MovementSkill.m_iSkill = Hero->CurrentSkill;
        if (CheckAttack())
        {
            g_MovementSkill.m_iTarget = SelectedCharacter;
        }
        else
        {
            g_MovementSkill.m_iTarget = -1;
        }
    }

    if (!CheckTile(c, o, Distance))
    {
        if (SelectedCharacter != -1 && CheckAttack())
        {
            if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
            {
                c->Movement = true;
                c->MovementType = MOVEMENT_SKILL;
                SendMove(c, o);
            }
        }

        if (Skill != AT_SKILL_STUN && Skill != AT_SKILL_REMOVAL_STUN
            && Skill != AT_SKILL_MANA && Skill != AT_SKILL_INVISIBLE
            && Skill != AT_SKILL_REMOVAL_INVISIBLE && Skill != AT_SKILL_PLASMA_STORM_FENRIR
            && Skill != AT_SKILL_ALICE_BERSERKER
            && Skill != AT_SKILL_ALICE_BERSERKER_STR
            && Skill != AT_SKILL_ALICE_WEAKNESS && Skill != AT_SKILL_ALICE_ENERVATION
            )
            return;
    }

    bool Wall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
    if (Wall)
    {
        if (SelectedCharacter != -1)
        {
            if (CheckAttack())
            {
                UseSkillWizard(c, o);
            }
        }
        if (Success)
        {
            o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
            switch (Skill)
            {
            case AT_SKILL_STORM:
            case AT_SKILL_EVIL_SPIRIT:
            case AT_SKILL_EVIL_SPIRIT_STR:
            case AT_SKILL_EVIL_SPIRIT_STR_MG:
            {
                SendRequestMagicContinue(Skill, (c->PositionX), (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, &o->m_bySkillSerialNum);
                SetPlayerMagic(c);
            }
            return;
            case AT_SKILL_FLASH:
            {
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
                SetAttackSpeed();

                if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                    SetAction(o, PLAYER_SKILL_FLASH);
                else
                    SetAction(o, PLAYER_SKILL_FLASH);

                c->Movement = 0;
                StandTime = 0;
            }
            return;
            case AT_SKILL_FLAME:
            case AT_SKILL_FLAME_STR:
            case AT_SKILL_FLAME_STR_MG:
                SendRequestMagicContinue(Skill, (BYTE)(c->TargetPosition[0] / TERRAIN_SCALE), (BYTE)(c->TargetPosition[1] / TERRAIN_SCALE), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, 0xffff, 0);
                SetPlayerMagic(c);
                return;
            case AT_SKILL_DECAY:
            case AT_SKILL_DECAY_STR:
            case AT_SKILL_ICE_STORM:
            {
                WORD TKey = 0xffff;
                if (g_MovementSkill.m_iTarget != -1)
                {
                    TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                }
                SendRequestMagicContinue(Skill, (BYTE)(c->TargetPosition[0] / TERRAIN_SCALE), (BYTE)(c->TargetPosition[1] / TERRAIN_SCALE), (BYTE)(o->Angle[2] / 360.f * 256.f), 0, 0, TKey, 0);
                SetPlayerMagic(c);
                c->Movement = 0;
            }
            return;
            }
        }
    }
    if (ClassIndex == CLASS_WIZARD && Success)
    {
        switch (Skill)
        {
        case AT_SKILL_TELEPORT_ALLY:
            if (gMapManager.IsCursedTemple()
                && !g_pMyInventory->IsItem(ITEM_POTION + 64, true))
            {
                return;
            }
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
            {
                if (!g_pPartyManager->IsPartyMember(SelectedCharacter))
                    return;

                if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
                {
                    return;
                }

                CHARACTER* tc = &CharactersClient[SelectedCharacter];
                OBJECT* to = &tc->Object;
                bool Success = false;
                if (to->Type == MODEL_PLAYER)
                {
                    if (to->CurrentAction != PLAYER_SKILL_TELEPORT)
                        Success = true;
                }
                else
                {
                    if (to->CurrentAction != MONSTER01_SHOCK)
                        Success = true;
                }
                if (Success && to->Teleport != TELEPORT_BEGIN && to->Teleport != TELEPORT && to->Alpha >= 0.7f)
                {
                    int Wall, indexX, indexY, TargetIndex, count = 0;
                    int PositionX = (int)(c->Object.Position[0] / TERRAIN_SCALE);
                    int PositionY = (int)(c->Object.Position[1] / TERRAIN_SCALE);

                    while (1)
                    {
                        indexX = rand() % 3;
                        indexY = rand() % 3;

                        if (indexX != 1 || indexY != 1)
                        {
                            TargetX = (PositionX - 1) + indexX;
                            TargetY = (PositionY - 1) + indexY;

                            TargetIndex = TERRAIN_INDEX(TargetX, TargetY);

                            Wall = TerrainWall[TargetIndex];

                            if ((Wall & TW_ACTION) == TW_ACTION)
                            {
                                Wall -= TW_ACTION;
                            }
                            if (gMapManager.WorldActive == WD_30BATTLECASTLE)
                            {
                                int ax = (Hero->PositionX);
                                int ay = (Hero->PositionY);
                                if ((ax >= 150 && ax <= 200) && (ay >= 180 && ay <= 230))
                                    Wall = 0;
                            }
                            if (Wall == 0) break;

                            count++;
                        }

                        if (count > 10) return;
                    }
                    to->Angle[2] = CreateAngle2D(to->Position, tc->TargetPosition);

                    SocketClient->ToGameServer()->SendTeleportTarget(tc->Key, TargetX, TargetY);
                    SetPlayerTeleport(tc);
                }
            }
            return;

        case AT_SKILL_TELEPORT:
        {
            if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem()
                || g_isCharacterBuff(o, eDeBuff_Stun)
                || g_isCharacterBuff(o, eDeBuff_Sleep)
                )
            {
                return;
            }

            WORD byHeroPriorSkill = g_pSkillList->GetHeroPriorSkill();
            if (c == Hero && byHeroPriorSkill == AT_SKILL_NOVA)
            {
                g_pSkillList->SetHeroPriorSkill(AT_SKILL_TELEPORT);
                SendRequestMagic(byHeroPriorSkill, HeroKey);
                SetAttackSpeed();
                SetAction(&Hero->Object, PLAYER_SKILL_HELL_BEGIN);
                Hero->Movement = 0;
                return;
            }

            bool Success = false;
            if (o->Type == MODEL_PLAYER)
            {
                if (o->CurrentAction != PLAYER_SKILL_TELEPORT)
                    Success = true;
            }
            else
            {
                if (o->CurrentAction != MONSTER01_SHOCK)
                    Success = true;
            }
            if (Success && o->Teleport != TELEPORT_BEGIN && o->Teleport != TELEPORT && o->Alpha >= 0.7f)
            {
                int TargetIndex = TERRAIN_INDEX_REPEAT(TargetX, TargetY);
                int Wall = TerrainWall[TargetIndex];
                if ((Wall & TW_ACTION) == TW_ACTION) Wall -= TW_ACTION;
                if ((Wall & TW_HEIGHT) == TW_HEIGHT) Wall -= TW_HEIGHT;
                if (Wall == 0)
                {
                    o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
                    bool bResult;
                    //SendRequestMagicTeleport(&bResult, 0, TargetX, TargetY);
                    if (g_bWhileMovingZone || (GetTickCount() - g_dwLatestZoneMoving < 3000))\
                    {
                        bResult = false;
                    }
                    else
                    {
                        SocketClient->ToGameServer()->SendEnterGateRequest(0, TargetX, TargetY);
                        bResult = true;
                    }

                    if (bResult)
                    {
                        if (g_isCharacterBuff(o, eDeBuff_Stun))
                        {
                            UnRegisterBuff(eDeBuff_Stun, o);
                        }

                        if (g_isCharacterBuff(o, eDeBuff_Sleep))
                        {
                            UnRegisterBuff(eDeBuff_Sleep, o);
                        }

                        SetPlayerTeleport(c);
                    }
                }
            }
        }
        return;
        }
    }
    else if (ClassIndex == CLASS_SUMMONER && Success)
    {
        if (g_SummonSystem.SendRequestSummonSkill(Skill, c, o) == TRUE)
        {
            return;
        }

        int iEnergy;
        gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
        if (iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
        {
            return;
        }

        g_MovementSkill.m_bMagic = TRUE;
        g_MovementSkill.m_iSkill = Hero->CurrentSkill;
        g_MovementSkill.m_iTarget = SelectedCharacter;

        switch (Skill)
        {
        case AT_SKILL_ALICE_THORNS:
        {
            if (SelectedCharacter == -1 || CharactersClient[SelectedCharacter].Object.Kind != KIND_PLAYER)
            {
                LetHeroStop();

                switch (c->Helper.Type)
                {
                case MODEL_HORN_OF_UNIRIA:
                    SetAction(o, PLAYER_SKILL_SLEEP_UNI);
                    break;
                case MODEL_HORN_OF_DINORANT:
                    SetAction(o, PLAYER_SKILL_SLEEP_DINO);
                    break;
                case MODEL_HORN_OF_FENRIR:
                    SetAction(o, PLAYER_SKILL_SLEEP_FENRIR);
                    break;
                default:
                    SetAction(o, PLAYER_SKILL_SLEEP);
                    break;
                }
                SendRequestMagic(Skill, HeroKey);
            }

            else if (0 == CharactersClient[SelectedCharacter].Dead && CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
            {
                TargetX = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[g_MovementSkill.m_iTarget].Object.Position[1] / TERRAIN_SCALE);

                if (CheckTile(c, o, Distance) && c->SafeZone == false)
                {
                    bool bNoneWall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
                    if (bNoneWall)
                    {
                        UseSkillSummon(c, o);
                    }
                }
                else
                {
                    if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                    {
                        c->Movement = true;
                    }
                }
            }
        }
        break;
        case AT_SKILL_ALICE_BERSERKER:
        case AT_SKILL_ALICE_BERSERKER_STR:
        case AT_SKILL_ALICE_WEAKNESS:
        case AT_SKILL_ALICE_ENERVATION:
            UseSkillSummon(c, o);
            break;
        case AT_SKILL_LIGHTNING_SHOCK:
        case AT_SKILL_LIGHTNING_SHOCK_STR:
        {
            o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

            if (CheckTile(c, o, Distance))
            {
                BYTE PathX[1];
                BYTE PathY[1];
                PathX[0] = (c->PositionX);
                PathY[0] = (c->PositionY);
                SendCharacterMove(c->Key, o->Angle[2], 1, &PathX[0], &PathY[0], TargetX, TargetY);

                BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                BYTE angle = (BYTE)((((o->Angle[2] + 180.f) / 360.f) * 255.f));
                WORD TKey = 0xffff;
                if (g_MovementSkill.m_iTarget != -1)
                {
                    TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                }
                SendRequestMagicContinue(Skill, (c->PositionX),
                    (c->PositionY), ((o->Angle[2] / 360.f) * 255), byValue, angle, TKey, 0);
                SetAttackSpeed();
                SetAction(o, PLAYER_SKILL_LIGHTNING_SHOCK);
                c->Movement = 0;
            }
        }
        break;
        }

        switch (Skill)
        {
        case AT_SKILL_ALICE_DRAINLIFE:
        case AT_SKILL_ALICE_DRAINLIFE_STR:
        case AT_SKILL_ALICE_LIGHTNINGORB:
        case AT_SKILL_ALICE_CHAINLIGHTNING:
        case AT_SKILL_ALICE_CHAINLIGHTNING_STR:
        {
            c->TargetCharacter = SelectedCharacter;
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Dead == 0)
            {
                TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);

                if (CheckAttack() == true)
                {
                    if (CheckTile(c, o, Distance) && c->SafeZone == false)
                    {
                        bool bNoneWall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
                        if (bNoneWall)
                        {
                            UseSkillSummon(c, o);
                        }
                    }
                    else
                    {
                        if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                        {
                            c->Movement = true;
                        }
                    }
                }
            }
        }
        break;
        case AT_SKILL_ALICE_SLEEP:
        case AT_SKILL_ALICE_SLEEP_STR:
        case AT_SKILL_ALICE_BLIND:
        {
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
            {
                if (gMapManager.InChaosCastle() == true
                    || gMapManager.IsCursedTemple() == true
                    || (gMapManager.InBattleCastle() == true && battleCastle::IsBattleCastleStart() == true)
                    || g_DuelMgr.IsDuelEnabled()
                    )
                {
                }
                else
                {
                    break;
                }
            }
            if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Dead == 0)
            {
                TargetX = (int)(CharactersClient[SelectedCharacter].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[SelectedCharacter].Object.Position[1] / TERRAIN_SCALE);

                if (CheckAttack() == true)
                {
                    if (CheckTile(c, o, Distance) && c->SafeZone == false)
                    {
                        bool bNoneWall = CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY);
                        if (bNoneWall)
                        {
                            UseSkillSummon(c, o);
                        }
                    }
                    else
                    {
                        if (PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path, Distance))
                        {
                            c->Movement = true;
                        }
                    }
                }
            }
        }
        break;
        }
    }

    c->SkillSuccess = true;
}

void AttackCommon(CHARACTER* c, int Skill, float Distance)
{
    OBJECT* o = &c->Object;

    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);

    if (o->Type == MODEL_PLAYER)
    {
        bool Success = true;

        if (iMana > CharacterAttribute->Mana)
        {
            int Index = g_pMyInventory->FindManaItemIndex();
            if (Index != -1)
            {
                SendRequestUse(Index, 0);
            }
            Success = false;
        }
        if (Success && iSkillMana > CharacterAttribute->SkillMana)
        {
            Success = false;
        }
        if (Success && !gSkillManager.CheckSkillDelay(Hero->CurrentSkill))
        {
            Success = false;
        }

        int iEnergy;
        gSkillManager.GetSkillInformation_Energy(Skill, &iEnergy);
        if (Success && iEnergy > (CharacterAttribute->Energy + CharacterAttribute->AddEnergy))
        {
            Success = false;
        }

        switch (Skill)
        {
        case    AT_SKILL_STUN:
        {
            //	                        if ( CheckTile( c, o, Distance ) )
            {
                o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);

                int TargetX = (int)(c->TargetPosition[0] / TERRAIN_SCALE);
                int TargetY = (int)(c->TargetPosition[1] / TERRAIN_SCALE);
                BYTE byValue = GetDestValue((c->PositionX), (c->PositionY), TargetX, TargetY);

                BYTE pos = CalcTargetPos(o->Position[0], o->Position[1], c->TargetPosition[0], c->TargetPosition[1]);
                WORD TKey = 0xffff;
                if (g_MovementSkill.m_iTarget != -1)
                {
                    TKey = getTargetCharacterKey(c, g_MovementSkill.m_iTarget);
                }
                SendRequestMagicContinue(Skill, (c->PositionX), (c->PositionY), (BYTE)(o->Angle[2] / 360.f * 256.f), byValue, pos, TKey, 0);
                SetAttackSpeed();

                if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
                {
                    SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
                }
                else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
                {
                    SetAction(o, PLAYER_SKILL_RIDER);
                }
                else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
                {
                    SetAction(o, PLAYER_SKILL_RIDER_FLY);
                }
                else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                {
                    SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
                }
                else
                {
                    SetAction(o, PLAYER_SKILL_VITALITY);
                }
                c->Movement = 0;
            }
        }
        break;

        case    AT_SKILL_REMOVAL_STUN:
        {
            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else
                if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
                {
                    SetAction(o, PLAYER_SKILL_RIDER);
                }
                else
                    if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
                    {
                        SetAction(o, PLAYER_SKILL_RIDER_FLY);
                    }
                    else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
                    {
                        SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
                    }
                    else
                    {
                        SetAction(o, PLAYER_ATTACK_REMOVAL);
                    }
            c->Movement = 0;
        }
        break;

        case    AT_SKILL_MANA:
            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
                SetAction(o, PLAYER_SKILL_VITALITY);
            c->Movement = 0;
            break;

        case    AT_SKILL_INVISIBLE:

            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                if (CharactersClient[SelectedCharacter].Object.Kind == KIND_PLAYER)
                    SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
                SetAction(o, PLAYER_SKILL_VITALITY);
            c->Movement = 0;
            break;

        case AT_SKILL_REMOVAL_INVISIBLE:
            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
                SetAction(o, PLAYER_ATTACK_REMOVAL);
            c->Movement = 0;
            break;

        case AT_SKILL_REMOVAL_BUFF:

            if (SelectedCharacter == -1)
            {
                SendRequestMagic(Skill, HeroKey);
            }
            else
            {
                SendRequestMagic(Skill, CharactersClient[SelectedCharacter].Key);
            }

            if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)
            {
                SetAction(o, PLAYER_SKILL_RIDER_FLY);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
            }
            else
                SetAction(o, PLAYER_SKILL_VITALITY);
            c->Movement = 0;
            break;
        }
    }
    c->SkillSuccess = true;
}

bool SkillKeyPush(int Skill)
{
    if (Skill == AT_SKILL_NOVA && MouseLButtonPush)
    {
        return true;
    }
    return false;
}

void Attack(CHARACTER* c)
{
    if ((MouseOnWindow || !SEASON3B::CheckMouseIn(0, 0, GetScreenWidth(), 429)) && MouseLButtonPush)
    {
        MouseRButtonPop = false;
        MouseRButtonPush = false;
        MouseRButton = false;
        MouseRButtonPress = 0;

        return;
    }

    if (g_isCharacterBuff((&c->Object), eDeBuff_Stun) || g_isCharacterBuff((&c->Object), eDeBuff_Sleep) || g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
    {
        return;
    }

    OBJECT* o = &c->Object;

    if (o->Teleport == TELEPORT_BEGIN)
    {
        o->Alpha -= 0.1f;
        if (o->Alpha < 0.1f)
        {
            o->Teleport = TELEPORT;
        }
    }

    bool Success = false;

    auto Skill = CharacterAttribute->Skill[Hero->CurrentSkill];
    float Distance = gSkillManager.GetSkillDistance(Skill, c);

    if (!EnableFastInput)
    {
        if (MouseRButtonPress != 0)
        {
            if (MouseRButtonPop || SkillKeyPush(Skill))
            {
                MouseRButtonPop = false;
                MouseRButtonPush = false;
                MouseRButton = false;

                MouseRButtonPress = 0;
                Success = true;
            }
            else
            {
                MouseRButtonPress++;
            }
        }
        else if ((MouseRButtonPush || MouseRButton) && !(MouseLButtonPush || MouseLButton) && !c->Movement)
        {
            if (Skill == AT_SKILL_NOVA)
            {
                if (o->Teleport != TELEPORT_END && o->Teleport != TELEPORT_NONE) return;
                int iReqEng = 0;
                gSkillManager.GetSkillInformation_Energy(Skill, &iReqEng);
                if (CharacterAttribute->Energy + CharacterAttribute->AddEnergy < iReqEng) return;

                MouseRButtonPress = 1;
                Hero->Object.m_bySkillCount = 0;
                Skill = AT_SKILL_NOVA_BEGIN;
            }
            SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
            MouseRButtonPush = false;
            Success = true;
        }
        if (
            g_pOption->IsAutoAttack()
            && gMapManager.WorldActive != WD_6STADIUM
            && gMapManager.InChaosCastle() == false
            && Attacking == 2
            && SelectedCharacter != -1)
        {
            Success = true;
        }

        if (Success)
        {
            RButtonPressTime = ((WorldTime - RButtonPopTime) / CLOCKS_PER_SEC);

            if (RButtonPressTime >= AutoMouseLimitTime)
            {
                MouseRButtonPush = false;
                MouseRButton = false;
                Success = FALSE;
            }
        }
        else
        {
            RButtonPopTime = WorldTime;
            RButtonPressTime = 0.f;
        }
    }
    else
    {
        if (MouseLButtonPush || MouseLButton)
        {
            MouseLButtonPush = false;
            Success = true;
        }
    }

    if (!Success)
    {
        return;
    }

    g_iFollowCharacter = -1;

    if (!g_isCharacterBuff((&Hero->Object), eBuff_Cloaking))
    {
        if (giPetManager::SendPetCommand(c, Hero->CurrentSkill) == true)
        {
            return;
        }
    }

    ExecuteSkill(c, Skill, Distance);
}

bool CanExecuteSkill(CHARACTER* c, ActionSkillType Skill, float Distance)
{
    OBJECT* o = &c->Object;

    if (c->Dead > 0)
    {
        return false;
    }

    if (c->SafeZone)
    {
        if ((gMapManager.InBloodCastle() == true) || gMapManager.InChaosCastle() == true)
        {
            if (Skill != AT_SKILL_HEALING
                && Skill != AT_SKILL_HEALING_STR
                && Skill != AT_SKILL_DEFENSE
                && Skill != AT_SKILL_DEFENSE_STR
                && Skill != AT_SKILL_DEFENSE_MASTERY
                && Skill != AT_SKILL_ATTACK
                && Skill != AT_SKILL_ATTACK_STR
                && Skill != AT_SKILL_ATTACK_MASTERY
                && Skill != AT_SKILL_SOUL_BARRIER
                && Skill != AT_SKILL_SOUL_BARRIER_STR
                && Skill != AT_SKILL_SOUL_BARRIER_PROFICIENCY
                && Skill != AT_SKILL_SWELL_LIFE
                && Skill != AT_SKILL_SWELL_LIFE_STR
                && Skill != AT_SKILL_SWELL_LIFE_PROFICIENCY
                && Skill != AT_SKILL_INFINITY_ARROW
                && Skill != AT_SKILL_INFINITY_ARROW_STR
                && Skill != AT_SKILL_EXPANSION_OF_WIZARDRY
                && Skill != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
                && Skill != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY
                && Skill != AT_SKILL_RECOVER
                && Skill != AT_SKILL_ALICE_BERSERKER
                && Skill != AT_SKILL_ALICE_BERSERKER_STR
                && Skill != AT_SKILL_IMPROVE_AG
                && Skill != AT_SKILL_ADD_CRITICAL
                && Skill != AT_SKILL_ADD_CRITICAL_STR1
                && Skill != AT_SKILL_ADD_CRITICAL_STR2
                && Skill != AT_SKILL_ADD_CRITICAL_STR3
                && Skill != AT_SKILL_PARTY_TELEPORT
                && (Skill<AT_SKILL_STUN || Skill>AT_SKILL_REMOVAL_BUFF)
                && Skill != AT_SKILL_BRAND_OF_SKILL
                )
            {
                return false;
            }
        }
        return false;
    }

    if (gMapManager.IsCursedTemple())
    {
        if (!g_pCursedTempleWindow->IsCursedTempleSkillKey(SelectedCharacter))
        {
            if (g_CursedTemple->IsPartyMember(SelectedCharacter) == true)
            {
                if (!IsCorrectSkillType_FrendlySkill(Skill) && !IsCorrectSkillType_Buff(Skill))
                {
                    return false;
                }
            }
            else
            {
                if (IsCorrectSkillType_FrendlySkill(Skill) || IsCorrectSkillType_Buff(Skill))
                {
                    if (-1 != SelectedCharacter)
                    {
                        return false;
                    }
                }
            }
        }
    }

    if (Skill == AT_SKILL_SUMMON_EXPLOSION || Skill == AT_SKILL_SUMMON_REQUIEM || Skill == AT_SKILL_SUMMON_POLLUTION)
    {
        CheckTarget(c);
        if (!CheckTile(c, o, Distance))
        {
            return false;
        }
    }

    if (!gSkillManager.AreSkillRequirementsFulfilled(Skill))
    {
        return false;
    }

    if (CheckSkillUseCondition(o, Skill) == false)
    {
        return false;
    }

    if (CheckMana(c, Skill) == false)
    {
        return false;
    }

    return true;
}

bool CheckMana(CHARACTER* c, int Skill)
{
    int iMana, iSkillMana;
    gSkillManager.GetSkillInformation(Skill, 1, NULL, &iMana, NULL, &iSkillMana);
    if (CharacterAttribute->Mana < iMana)
    {
        int Index = g_pMyInventory->FindManaItemIndex();

        if (Index != -1)
        {
            SendRequestUse(Index, 0);
        }
        return false;
    }
    if (iSkillMana > CharacterAttribute->SkillMana)
    {
        return false;
    }
    return true;
}

int ExecuteSkill(CHARACTER* c, ActionSkillType Skill, float Distance)
{
    OBJECT* o = &c->Object;

    int ClassIndex = gCharacterManager.GetBaseClass(c->Class);

    if (!CanExecuteSkill(c, Skill, Distance))
    {
        return 0;
    }

    int iSkillIndex = g_pSkillList->GetSkillIndex(Skill);
    if (iSkillIndex == -1)
    {
        return 0;
    }

    Hero->CurrentSkill = iSkillIndex;

    if (gMapManager.IsCursedTemple() && g_pCursedTempleWindow->IsCursedTempleSkillKey(SelectedCharacter))
    {
        g_pCursedTempleWindow->SetCursedTempleSkill(c, o, SelectedCharacter);
        return 0;
    }

    if (c->Movement)
    {
        c->SkillSuccess = false;
    }

    if (
        g_pOption->IsAutoAttack()
        && gMapManager.WorldActive != WD_6STADIUM
        && gMapManager.InChaosCastle() == false)
    {
        if (ClassIndex == CLASS_ELF
            && (Skill != AT_SKILL_TRIPLE_SHOT
                && Skill != AT_SKILL_TRIPLE_SHOT_STR
                && Skill != AT_SKILL_TRIPLE_SHOT_MASTERY
                && Skill != AT_SKILL_MULTI_SHOT
                && Skill != AT_SKILL_BOW
                && Skill != AT_SKILL_PENETRATION
                && Skill != AT_SKILL_PENETRATION_STR
                && Skill != AT_SKILL_BLAST_CROSSBOW4
                && Skill != AT_SKILL_PLASMA_STORM_FENRIR
                ))
        {
            Attacking = -1;
        }
        else if (ClassIndex == CLASS_KNIGHT && (Skill == AT_SKILL_SWELL_LIFE || Skill == AT_SKILL_SWELL_LIFE_STR || Skill == AT_SKILL_SWELL_LIFE_PROFICIENCY))
        {
            Attacking = -1;
        }
        else if (ClassIndex == CLASS_DARK_LORD
            && (Skill == AT_SKILL_ADD_CRITICAL || Skill == AT_SKILL_ADD_CRITICAL_STR1 || Skill == AT_SKILL_ADD_CRITICAL_STR2 || Skill == AT_SKILL_ADD_CRITICAL_STR3
                || Skill == AT_SKILL_PARTY_TELEPORT))
        {
            Attacking = -1;
        }
        else if (ClassIndex == CLASS_WIZARD
            && (Skill == AT_SKILL_NOVA_BEGIN || Skill == AT_SKILL_NOVA))
        {
            Attacking = -1;
        }
        else if (Skill >= AT_SKILL_STUN && Skill <= AT_SKILL_REMOVAL_BUFF)
        {
            Attacking = -1;
        }
        else if (Skill == AT_SKILL_BRAND_OF_SKILL)
        {
            Attacking = -1;
        }
        else if (Skill == AT_SKILL_ALICE_THORNS
            || Skill == AT_SKILL_ALICE_BERSERKER
            || Skill == AT_SKILL_ALICE_BERSERKER_STR
            || Skill == AT_SKILL_ALICE_SLEEP
            || Skill == AT_SKILL_ALICE_SLEEP_STR
            || Skill == AT_SKILL_ALICE_BLIND || Skill == AT_SKILL_ALICE_WEAKNESS || Skill == AT_SKILL_ALICE_ENERVATION)
        {
            Attacking = -1;
        }
        else if (AT_SKILL_ATT_UP_OURFORCES == Skill
            || AT_SKILL_HP_UP_OURFORCES == Skill
            || AT_SKILL_HP_UP_OURFORCES_STR == Skill
            || AT_SKILL_DEF_UP_OURFORCES == Skill
            || AT_SKILL_DEF_UP_OURFORCES_STR == Skill
            || AT_SKILL_DEF_UP_OURFORCES_MASTERY == Skill
            )
        {
            Attacking = -1;
        }
        else
        {
            Attacking = 2;
        }
    }

    if (o->Type == MODEL_PLAYER)
    {
        if (o->CurrentAction < PLAYER_STOP_MALE
            || o->CurrentAction > PLAYER_STOP_RIDE_WEAPON
            && o->CurrentAction != PLAYER_STOP_TWO_HAND_SWORD_TWO
            && o->CurrentAction != PLAYER_SKILL_HELL_BEGIN
            && o->CurrentAction != PLAYER_DARKLORD_STAND
            && o->CurrentAction != PLAYER_STOP_RIDE_HORSE
            && o->CurrentAction != PLAYER_FENRIR_STAND
            && o->CurrentAction != PLAYER_FENRIR_STAND_TWO_SWORD
            && o->CurrentAction != PLAYER_FENRIR_STAND_ONE_RIGHT
            && o->CurrentAction != PLAYER_FENRIR_STAND_ONE_LEFT
            && !(o->CurrentAction >= PLAYER_RAGE_FENRIR_STAND && o->CurrentAction <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT)
            && o->CurrentAction != PLAYER_RAGE_UNI_STOP_ONE_RIGHT
            && o->CurrentAction != PLAYER_STOP_RAGEFIGHTER)
        {
            MouseRButtonPress = 0;
            return 0;
        }
    }
    else
    {
        if (o->CurrentAction < MONSTER01_STOP1 || o->CurrentAction > MONSTER01_STOP2)
        {
            return 0;
        }
    }

    if (ClassIndex != CLASS_WIZARD)
    {
        CheckTarget(c);

        if (CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY))
        {
            for (int i = EQUIPMENT_WEAPON_RIGHT; i <= EQUIPMENT_WEAPON_LEFT; i++)
            {
                if (ClassIndex == CLASS_KNIGHT || ClassIndex == CLASS_DARK || ClassIndex == CLASS_DARK_LORD
                    || ClassIndex == CLASS_RAGEFIGHTER)
                {
                    bool bOk = false;
                    if (c->Helper.Type != MODEL_HORN_OF_UNIRIA
                        && c->Helper.Type != MODEL_HORN_OF_DINORANT
                        && c->Helper.Type != MODEL_DARK_HORSE_ITEM
                        && c->Helper.Type != MODEL_HORN_OF_FENRIR
                        )
                    {
                        bOk = true;
                    }
                    else
                    {
                        switch (Skill)
                        {
                        case AT_SKILL_GAOTIC:
                        case AT_SKILL_IMPALE:
                        case AT_SKILL_RIDER:
                        case AT_SKILL_DEATHSTAB:
                        case AT_SKILL_DEATHSTAB_STR:
                        case AT_SKILL_FORCE:
                        case AT_SKILL_FORCE_WAVE:
                        case AT_SKILL_FORCE_WAVE_STR:
                        case AT_SKILL_FIREBURST:
                        case AT_SKILL_FIREBURST_STR:
                        case AT_SKILL_FIREBURST_MASTERY:
                        case AT_SKILL_EARTHSHAKE:
                        case AT_SKILL_EARTHSHAKE_STR:
                        case AT_SKILL_EARTHSHAKE_MASTERY:
                        case AT_SKILL_THUNDER_STRIKE:
                        case AT_SKILL_SPACE_SPLIT:
                        case AT_SKILL_PLASMA_STORM_FENRIR:
                        case AT_SKILL_FIRE_SCREAM:
                        case AT_SKILL_FIRE_SCREAM_STR:
                            bOk = true;
                            break;
                        }
                    }

                    if (bOk)
                    {
                        g_MovementSkill.m_bMagic = TRUE;
                        g_MovementSkill.m_iSkill = Hero->CurrentSkill;

                        if (CheckAttack())
                        {
                            g_MovementSkill.m_iTarget = SelectedCharacter;
                        }
                        else
                        {
                            g_MovementSkill.m_iTarget = -1;
                        }
                        if (SkillWarrior(c, &CharacterMachine->Equipment[i]))
                        {
                            return (int) ExecuteSkillComplete(c);
                        }
                    }
                }
                if (ClassIndex == CLASS_ELF)
                {
                    if (SkillElf(c, &CharacterMachine->Equipment[i]))
                    {
                        return (int) ExecuteSkillComplete(c);
                    }
                }
            }
        }
        else
        {
            if (PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
            {
                SendMove(c, o);
                return 0;
            }
            else
            {
                ZeroMemory(&g_MovementSkill, sizeof(g_MovementSkill));
                g_MovementSkill.m_iTarget = -1;
                return -1;
            }
        }
    }
    if (ClassIndex == CLASS_ELF)
    {
        AttackElf(c, Skill, Distance);
    }
    if (ClassIndex == CLASS_KNIGHT || ClassIndex == CLASS_DARK || ClassIndex == CLASS_DARK_LORD)
    {
        AttackKnight(c, Skill, Distance);
    }
    if (ClassIndex == CLASS_RAGEFIGHTER)
    {
        AttackRagefighter(c, Skill, Distance);
    }
    if (ClassIndex == CLASS_WIZARD || ClassIndex == CLASS_DARK || ClassIndex == CLASS_SUMMONER)
    {
        AttackWizard(c, Skill, Distance);
    }
    if ((Skill >= AT_SKILL_STUN && Skill <= AT_SKILL_REMOVAL_BUFF))
    {
        AttackCommon(c, Skill, Distance);
    }

    return (int) ExecuteSkillComplete(c);
}

bool ExecuteSkillComplete(CHARACTER* c)
{
    return c->SkillSuccess && !c->Movement;
}

BOOL g_bWhileMovingZone = FALSE;
DWORD g_dwLatestZoneMoving = 0;

void CheckGate()
{
    if ((g_pMyInventory->IsItem(ITEM_POTION + 64, true)) || (gMapManager.IsCursedTemple() && g_pMyInventory->IsItem(ITEM_POTION + 64, false)))
    {
        return;
    }

    for (int i = 0; i < MAX_GATES; i++)
    {
        GATE_ATTRIBUTE* gs = &GateAttribute[i];
        if (gs->Flag == 1 && gs->Map == gMapManager.WorldActive)
        {
            if ((Hero->PositionX) >= gs->x1 && (Hero->PositionY) >= gs->y1 &&
                (Hero->PositionX) <= gs->x2 && (Hero->PositionY) <= gs->y2)
            {
                GATE_ATTRIBUTE* gt = &GateAttribute[gs->Target];
                if ((LoadingWorld) == 0 && Hero->JumpTime == 0)
                {
                    bool Success = false;
                    int Level;

                    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD
                        || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
                        Level = gs->Level * 2 / 3;
                    else
                        Level = gs->Level;

                    if (i == 28)
                    {
                        if (CharacterAttribute->Level >= Level)
                            Success = true;
                    }
                    else
                        Success = true;
                    if (Success)
                    {
                        if (((i >= 45 && i <= 49) || (i >= 55 && i <= 56)) &&
                            ((CharacterMachine->Equipment[EQUIPMENT_HELPER].Type >= ITEM_HORN_OF_UNIRIA && CharacterMachine->Equipment[EQUIPMENT_HELPER].Type <= ITEM_HORN_OF_DINORANT)))
                        {
                            g_pSystemLogBox->AddText(GlobalText[261], SEASON3B::TYPE_ERROR_MESSAGE);
                        }
                        else if ((62 <= i && i <= 65) &&
                            !((CharacterMachine->Equipment[EQUIPMENT_WING].Type >= ITEM_WING && CharacterMachine->Equipment[EQUIPMENT_WING].Type <= ITEM_WINGS_OF_DARKNESS
                                || CharacterMachine->Equipment[EQUIPMENT_HELPER].Type == ITEM_DARK_HORSE_ITEM
                                || CharacterMachine->Equipment[EQUIPMENT_WING].Type == ITEM_CAPE_OF_LORD
                                ) || CharacterMachine->Equipment[EQUIPMENT_HELPER].Type == ITEM_HORN_OF_DINORANT
                                || CharacterMachine->Equipment[EQUIPMENT_HELPER].Type == ITEM_HORN_OF_FENRIR
                                || (CharacterMachine->Equipment[EQUIPMENT_WING].Type >= ITEM_WING_OF_STORM && CharacterMachine->Equipment[EQUIPMENT_WING].Type <= ITEM_WING_OF_DIMENSION)
                                || (ITEM_WING + 130 <= CharacterMachine->Equipment[EQUIPMENT_WING].Type && CharacterMachine->Equipment[EQUIPMENT_WING].Type <= ITEM_WING + 134)
                                || (CharacterMachine->Equipment[EQUIPMENT_WING].Type >= ITEM_CAPE_OF_FIGHTER && CharacterMachine->Equipment[EQUIPMENT_WING].Type <= ITEM_CAPE_OF_OVERRULE)
                                || (CharacterMachine->Equipment[EQUIPMENT_WING].Type == ITEM_WING + 135)))
                        {
                            g_pSystemLogBox->AddText(GlobalText[263], SEASON3B::TYPE_ERROR_MESSAGE);

                            if (CharacterAttribute->Level < Level)
                            {
                                wchar_t Text[100];
                                swprintf(Text, GlobalText[350], Level);
                                g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_ERROR_MESSAGE);
                            }
                        }

                        else if ((62 <= i && i <= 65) && (CharacterMachine->Equipment[EQUIPMENT_HELPER].Type == ITEM_HORN_OF_UNIRIA))
                        {
                            g_pSystemLogBox->AddText(GlobalText[569], SEASON3B::TYPE_ERROR_MESSAGE);
                        }
                        else if (CharacterAttribute->Level < Level)
                        {
                            LoadingWorld = 50;
                            wchar_t Text[100];
                            swprintf(Text, GlobalText[350], Level);
                            g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_ERROR_MESSAGE);
                            //							return;
                        }
                        else
                        {
                            bool bResult = false;
                            if ((LoadingWorld) <= 30)
                            {
                                if (g_bWhileMovingZone || (GetTickCount() - g_dwLatestZoneMoving < 3000))
                                {
                                    bResult = false;
                                }
                                else
                                {
                                    SocketClient->ToGameServer()->SendEnterGateRequest(i, 0, 0);
                                    bResult = true;
                                }
                            }

                            if (!bResult)
                            {
                                g_bWhileMovingZone = FALSE;
                            }
                            else
                            {
                                LoadingWorld = 9999999;

                                if (gt->Map == WD_30BATTLECASTLE || gt->Map == WD_31HUNTING_GROUND)
                                {
                                    SaveOptions();
                                    SaveMacro(L"Data\\Macro.txt");
                                }

                                SelectedItem = -1;
                                SelectedNpc = -1;
                                SelectedCharacter = -1;
                                SelectedOperate = -1;
                                Attacking = -1;

                                if ((gs->Map == WD_0LORENCIA && gt->Map == WD_30BATTLECASTLE) || (gs->Map == WD_30BATTLECASTLE && gt->Map == WD_0LORENCIA))
                                {
                                    g_dwLatestZoneMoving = GetTickCount();
                                    g_bWhileMovingZone = FALSE;
                                }
                                else
                                {
                                    g_bWhileMovingZone = TRUE;
                                }

                                StandTime = 0;
                            }
                        }
                    }
                }
            }
        }
    }
}

LONG FAR PASCAL WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MoveEffect(OBJECT* o, int iIndex);

void MoveHero()
{
    CHARACTER* c = Hero;
    OBJECT* o = &c->Object;

    if (o->CurrentAction == PLAYER_CHANGE_UP)
    {
        return;
    }

    if (g_Direction.IsDirection())
    {
        SetPlayerStop(c);
        return;
    }

    if (SelectedCharacter != -1 && g_isCharacterBuff((&Hero->Object), eBuff_CrywolfHeroContracted))
    {
        return;
    }

    if (c->Dead > 0
        || g_isCharacterBuff(o, eDeBuff_Stun)
        || g_isCharacterBuff(o, eDeBuff_Sleep))
    {
        return;
    }

    if (c->Object.Live == 0)
        return;

    if (LoadingWorld > 0)
    {
        LoadingWorld--;
        if (LoadingWorld == 9999998 || LoadingWorld == 29)

            SetPlayerStop(c);
    }
    if (LoadingWorld > 30)
    {
        return;
    }

    if (g_pWindowMgr->GetAddFriendWindow() > 0)
    {
        if (MouseRButtonPush)
        {
            if (!::IsStrifeMap(gMapManager.WorldActive))
            {
                auto* pWindow = (CUITextInputWindow*)g_pWindowMgr->GetWindow(g_pWindowMgr->GetAddFriendWindow());
                if (pWindow != NULL)
                {
                    pWindow->SetText(CharactersClient[SelectedCharacter].ID);
                }
            }
        }
    }

    int HeroX = GetScreenWidth() / 2;
    int HeroY = 180;

    int Angle;
    bool bLookAtMouse = true;
    bool NoAutoAttacking = false;

    if (g_isCharacterBuff(o, eDeBuff_Stun)
        || g_isCharacterBuff(o, eDeBuff_Sleep)
        || o->CurrentAction == PLAYER_SKILL_GIANTSWING)
    {
        Angle = (int)Hero->Object.Angle[2];
        bLookAtMouse = false;
    }
    else
    {
        Angle = (int)(Hero->Object.Angle[2] + CreateAngle((float)HeroX, (float)HeroY, (float)MouseX, (float)MouseY)) + 360 - 45;
        Angle %= 360;
        if (Angle < 120) Angle = 120;
        if (Angle > 240) Angle = 240;
        Angle += 180;
        Angle %= 360;
    }

    Hero->Object.HeadTargetAngle[2] = 0.f;

    if ((
        g_pOption->IsAutoAttack()
        && Attacking != -1
        && gMapManager.WorldActive != WD_6STADIUM
        && gMapManager.InChaosCastle() == false)
        || o->CurrentAction == PLAYER_ATTACK_SKILL_FURY_STRIKE
        || o->CurrentAction == PLAYER_SKILL_GIANTSWING)
    {
        bLookAtMouse = false;
    }
    if (bLookAtMouse)
    {
        int mousePosY = MouseY;

        if (mousePosY > 480)
        {
            mousePosY = 480;
        }
        Hero->Object.HeadTargetAngle[0] = (float)Angle;
        Hero->Object.HeadTargetAngle[1] = (HeroY - mousePosY) * 0.05f;

        NoAutoAttacking = true;
    }
    else
    {
        Hero->Object.HeadTargetAngle[0] = 0;
        Hero->Object.HeadTargetAngle[1] = 0;
    }

    if (c->Movement)
    {
        if (g_isCharacterBuff(o, eDeBuff_Harden) || g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint))
        {
            SetPlayerStop(c);
        }
        else
        {
            SetPlayerWalk(c);
            if (MovePath(c))
            {
                c->Movement = false;
                SetPlayerStop(c);
                HeroAngle = (int)c->Object.Angle[2];
                StandTime = 0;

                if (c->MovementType == MOVEMENT_OPERATE)
                    Action(c, o, false);
                else
                    if (!CheckArrow() && gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
                        SetPlayerStop(Hero);
                    else
                        Action(c, o, false);
            }
            else
            {
                g_CharacterUnRegisterBuff((&Hero->Object), eBuff_CrywolfHeroContracted);
                MoveCharacterPosition(c);
            }
        }
    }
    else
    {
        StandTime++;

        if (StandTime >= 40 && !MouseOnWindow && Hero->Dead == 0 &&
            o->CurrentAction != PLAYER_POSE1 && o->CurrentAction != PLAYER_POSE_FEMALE1 &&
            o->CurrentAction != PLAYER_SIT1 && o->CurrentAction != PLAYER_SIT_FEMALE1 && NoAutoAttacking &&
            o->CurrentAction != PLAYER_ATTACK_TELEPORT &&
            o->CurrentAction != PLAYER_ATTACK_RIDE_TELEPORT &&
            o->CurrentAction != PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT &&
            o->CurrentAction != PLAYER_SKILL_ATT_UP_OURFORCES &&
            o->CurrentAction != PLAYER_SKILL_HP_UP_OURFORCES &&
            Hero->AttackTime == 0)
        {
            StandTime = 0;
            HeroAngle = -(int)(CreateAngle((float)MouseX, (float)MouseY, (float)HeroX, (float)HeroY)) + 360 + 45;
            HeroAngle %= 360;
            BYTE Angle1 = ((BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8);
            BYTE Angle2 = ((BYTE)(((float)HeroAngle + 22.5f) / 360.f * 8.f + 1.f) % 8);
            if (Angle1 != Angle2)
            {
                if (o->CurrentAction != PLAYER_ATTACK_SKILL_SWORD2)
                {
                    Hero->Object.Angle[2] = (float)HeroAngle;
                }
                SendRequestAction(Hero->Object, AT_STAND1);
            }
        }

        UseSkillRagePosition(c);
    }

    CheckGate();

    if (!MouseOnWindow && false == g_pNewUISystem->CheckMouseUse())
    {
        bool Success = false;
        if (MouseUpdateTime >= MouseUpdateTimeMax)
        {
            if (!EnableFastInput)
            {
                if (MouseLButtonPush)
                {
                    MouseLButtonPush = false;
                    Success = true;
                }
                if (MouseLButton)
                {
                    Success = true;
                }

                if ((
                    g_pOption->IsAutoAttack()
                    && gMapManager.WorldActive != WD_6STADIUM
                    && gMapManager.InChaosCastle() == false)
                    && (Attacking == 1 && SelectedCharacter != -1))
                {
                    Success = true;
                }

                if (Success && !g_isCharacterBuff(o, eDeBuff_Stun) && !g_isCharacterBuff(o, eDeBuff_Sleep))
                {
                    g_iFollowCharacter = -1;

                    LButtonPressTime = ((WorldTime - LButtonPopTime) / CLOCKS_PER_SEC);

                    if (LButtonPressTime >= AutoMouseLimitTime)
                    {
                        MouseLButtonPush = false;
                        MouseLButton = false;
                        Success = FALSE;
                    }
                }
                else
                {
                    LButtonPopTime = WorldTime;
                    LButtonPressTime = 0.f;
                }
            }
        }
        if (g_iFollowCharacter >= 0 && g_iFollowCharacter < MAX_CHARACTERS_CLIENT)
        {
            CHARACTER* followCharacter = &CharactersClient[g_iFollowCharacter];
            if (followCharacter->Object.Live == 0)
            {
                g_iFollowCharacter = -1;
            }
            else
            {
                //
                c->MovementType = MOVEMENT_MOVE;
                ActionTarget = g_iFollowCharacter;
                TargetX = (int)(followCharacter->Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(followCharacter->Object.Position[1] / TERRAIN_SCALE);
                if (PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
                    SendMove(c, o);
            }
        }
        else if (Success &&
            ((o->CurrentAction != PLAYER_SHOCK && (o->Teleport != TELEPORT_BEGIN && o->Teleport != TELEPORT && o->Alpha >= 0.7f) &&
                (o->CurrentAction<PLAYER_ATTACK_FIST || o->CurrentAction>PLAYER_RIDE_SKILL)
                && (o->CurrentAction<PLAYER_SKILL_SLEEP || o->CurrentAction>PLAYER_SKILL_LIGHTNING_SHOCK)
                && o->CurrentAction != PLAYER_RECOVER_SKILL
                && (o->CurrentAction<PLAYER_SKILL_THRUST || o->CurrentAction>PLAYER_SKILL_HP_UP_OURFORCES))
                || (o->CurrentAction >= PLAYER_STOP_TWO_HAND_SWORD_TWO && o->CurrentAction <= PLAYER_RUN_TWO_HAND_SWORD_TWO)
                || (o->CurrentAction >= PLAYER_DARKLORD_STAND && o->CurrentAction <= PLAYER_RUN_RIDE_HORSE)
                || (o->CurrentAction >= PLAYER_FENRIR_RUN && o->CurrentAction <= PLAYER_FENRIR_WALK_ONE_LEFT)
                || (o->CurrentAction >= PLAYER_RAGE_FENRIR_WALK && o->CurrentAction <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT)
                ))
        {
            int RightType = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
            int LeftType = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;

            SEASON3B::CNewUIPickedItem* pPickedItem = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();

            if (!pPickedItem && RightType == -1 &&
                ((LeftType >= ITEM_SWORD && LeftType < ITEM_MACE + MAX_ITEM_INDEX)
                    || (LeftType >= ITEM_STAFF && LeftType < ITEM_STAFF + MAX_ITEM_INDEX
                        && !(LeftType >= ITEM_BOOK_OF_SAHAMUTT && LeftType <= ITEM_STAFF + 29)
                        )))
            {
                if (g_pMyInventory->IsEquipable(EQUIPMENT_WEAPON_LEFT, &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT]))
                {
                    memcpy(&PickItem, &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT], sizeof(ITEM));
                    CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type = -1;
                    CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Level = 0;
                    SetCharacterClass(Hero);
                    SrcInventory = Inventory;
                    SrcInventoryIndex = EQUIPMENT_WEAPON_LEFT;
                    DstInventoryIndex = EQUIPMENT_WEAPON_RIGHT;
                    SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, SrcInventoryIndex, &PickItem, STORAGE_TYPE::INVENTORY, DstInventoryIndex);
                }
            }
            MouseUpdateTime = 0;
            Success = false;

            if (!c->SafeZone)
            {
                Success = CheckAttack();
            }

            if (Success)
            {
#ifdef SEND_POSITION_TO_SERVER
                if (c->Movement && c->MovementType == MOVEMENT_MOVE && gCharacterManager.GetBaseClass(c->Class) == CLASS_ELF)
                {
                    if (gCharacterManager.GetEquipedBowType(CharacterMachine->Equipment) != BOWTYPE_NONE)
                    {
                        SocketClient->ToGameServer()->SendInstantMoveRequest(c->PositionX, c->PositionY);
                    }
                }
#endif

                if (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT)
                {
                    Attacking = 1;

                    c->MovementType = MOVEMENT_ATTACK;
                    ActionTarget = SelectedCharacter;
                    TargetX = (int)(CharactersClient[ActionTarget].Object.Position[0] / TERRAIN_SCALE);
                    TargetY = (int)(CharactersClient[ActionTarget].Object.Position[1] / TERRAIN_SCALE);

                    if (CheckWall((c->PositionX), (c->PositionY), TargetX, TargetY))
                    {
                        if (!PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
                        {
                            if (CheckArrow() == false)
                            {
                                return;
                            }
                            Action(c, o, true);
                        }
                        else
                        {
                            if ((gCharacterManager.GetEquipedBowType() != BOWTYPE_NONE) || (c->MonsterIndex == MONSTER_THUNDER_LICH))
                            {
                                if (CheckArrow() == false)
                                {
                                    return;
                                }
                                Action(c, o, true);
                            }
                            else
                            {
                                SendMove(c, o);
                            }
                        }
                    }
                }
            }
            else if (SelectedOperate != -1 && (c->SafeZone || (c->Helper.Type<MODEL_HORN_OF_UNIRIA || c->Helper.Type>MODEL_DARK_HORSE_ITEM || c->Helper.Type != MODEL_HORN_OF_FENRIR)))
            {
                TargetX = (int)(Operates[SelectedOperate].Owner->Position[0] / TERRAIN_SCALE);
                TargetY = (int)(Operates[SelectedOperate].Owner->Position[1] / TERRAIN_SCALE);
                int wall = TerrainWall[TERRAIN_INDEX(TargetX, TargetY)];

                if (wall == TW_HEIGHT || wall < TW_CHARACTER)
                    if (!c->Movement)
                    {
                        c->MovementType = MOVEMENT_OPERATE;
                        TargetType = Operates[SelectedOperate].Owner->Type;
                        TargetAngle = Operates[SelectedOperate].Owner->Angle[2];
                        if (PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
                            SendMove(c, o);
                        else
                            Action(c, o, true);
                    }
            }
            else if (SelectedNpc != -1
                && !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)
                && !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)
                )
            {
                if (g_isCharacterBuff(o, eBuff_CrywolfNPCHide) == false)
                    c->MovementType = MOVEMENT_TALK;

                ActionTarget = SelectedNpc;
                TargetX = (int)(CharactersClient[ActionTarget].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(CharactersClient[ActionTarget].Object.Position[1] / TERRAIN_SCALE);
                TargetNpc = ActionTarget;
                TargetType = CharactersClient[ActionTarget].Object.Type;
                TargetAngle = CharactersClient[ActionTarget].Object.Angle[2];

                if (TargetType == MODEL_KANTURU2ND_ENTER_NPC)
                {
                    vec3_t vHero, vTarget, vSubstract;
                    VectorCopy(o->Position, vHero);
                    vHero[2] = 0.f;
                    VectorCopy(CharactersClient[ActionTarget].Object.Position, vTarget);
                    vTarget[2] = 0.f;
                    VectorSubtract(vHero, vTarget, vSubstract);
                    float fLength = VectorLength(vSubstract);
                    if (fLength <= 550.f)
                    {
                        Action(c, o, true);
                    }
                    else
                    {
                        if (PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
                            SendMove(c, o);
                        else
                            Action(c, o, true);
                    }
                }
                else
                {
                    if (PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
                        SendMove(c, o);
                    else
                        Action(c, o, true);
                }
            }
            else if (SelectedItem != -1)
            {
                c->MovementType = MOVEMENT_GET;
                ItemKey = SelectedItem;
                TargetX = (int)(Items[SelectedItem].Object.Position[0] / TERRAIN_SCALE);
                TargetY = (int)(Items[SelectedItem].Object.Position[1] / TERRAIN_SCALE);
                if (PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
                    SendMove(c, o);
                else
                {
                    Action(c, o, true);
                    c->MovementType = MOVEMENT_MOVE;
                }
            }
            else if (HIBYTE(GetAsyncKeyState(VK_SHIFT)) != 128)
            {
                RenderTerrain(true);
                bool Success = RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true);

                if (Success && c->Object.Live)
                {
                    TargetX = (BYTE)(CollisionPosition[0] / TERRAIN_SCALE);
                    TargetY = (BYTE)(CollisionPosition[1] / TERRAIN_SCALE);
                    int Wall;
                    //if(CharacterMachine->Equipment[EQUIPMENT_WING].Type!=-1)
                    //	Wall = TW_NOMOVE;
                    //else
                    Wall = TW_NOGROUND;
                    WORD CurrAtt = TerrainWall[TargetY * 256 + TargetX];
                    if (CurrAtt >= Wall && (CurrAtt & TW_ACTION) != TW_ACTION
                        && (CurrAtt & TW_HEIGHT) != TW_HEIGHT
                        )
                        DontMove = true;
                    else
                        DontMove = false;
                    int xPos = (int)(0.01f * o->Position[0]);
                    int yPos = (int)(0.01f * o->Position[1]);

                    if (!c->Movement || (abs((c->PositionX) - xPos) < 2 && abs((c->PositionY) - yPos) < 2))
                    {
                        if (((c->PositionX) != TargetX || (c->PositionY) != TargetY || !c->Movement) &&
                            PathFinding2((c->PositionX), (c->PositionY), TargetX, TargetY, &c->Path))
                        {
                            c->MovementType = MOVEMENT_MOVE;
                            SendMove(c, o);
                            OBJECT* pHeroObj = &Hero->Object;
                            vec3_t vLight, vPos;
                            Vector(1.f, 1.f, 0.f, vLight);
                            VectorCopy(CollisionPosition, vPos);
                            DeleteEffect(MODEL_MOVE_TARGETPOSITION_EFFECT);

                            int iTerrainIndex = TERRAIN_INDEX((int)SelectXF, (int)SelectYF);
                            if ((TerrainWall[iTerrainIndex] & TW_NOMOVE) != TW_NOMOVE)
                            {
                                CreateEffect(MODEL_MOVE_TARGETPOSITION_EFFECT, vPos, pHeroObj->Angle, vLight, 0, pHeroObj, -1, 0, 0, 0, 0.6f);
                            }
                        }
                        else
                        {
                            MouseUpdateTime = MouseUpdateTimeMax;
                            MouseUpdateTime = 0;
                        }
                    }
                }
            }
        }
        MouseUpdateTime++;
    }

    Attack(Hero);

    int Index = ((int)Hero->Object.Position[1] / (int)TERRAIN_SCALE) * 256 + ((int)Hero->Object.Position[0] / (int)TERRAIN_SCALE);
    if (Index < 0) Index = 0;
    else if (Index > 65535) Index = 65535;
    HeroTile = TerrainMappingLayer1[Index];
}

int SelectItem()
{
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        OBJECT* o = &Items[i].Object;
        if (o->Live && o->Visible)
        {
            o->LightEnable = true;
            Vector(0.2f, 0.2f, 0.2f, o->Light);
        }
    }
    float Luminosity = 1.5f;

    for (int i = 0; i < MAX_ITEMS; i++)
    {
        OBJECT* o = &Items[i].Object;
        if (o->Live && o->Visible)
        {
            if (CollisionDetectLineToOBB(MousePosition, MouseTarget, o->OBB))
            {
                {
                    o->LightEnable = false;
                    Vector(Luminosity, Luminosity, Luminosity, o->Light);
                    return i;
                }
            }
        }
    }
    return -1;
}

int SelectCharacter(BYTE Kind)
{
    bool Main = true;
    if (SceneFlag == CHARACTER_SCENE)
        Main = false;

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if ((Kind & o->Kind) == o->Kind && o->Live && o->Visible && o->Alpha > 0.f)
        {
            o->LightEnable = true;
            switch (c->Level)
            {
            case 0:Vector(0.2f, 0.2f, 0.2f, o->Light); break;
            case 1:Vector(-0.4f, -0.4f, -0.4f, o->Light); break;
            case 2:Vector(0.2f, -0.6f, -0.6f, o->Light); break;
            case 3:Vector(1.5f, 1.5f, 1.5f, o->Light); break;
            case 4:Vector(0.3f, 0.2f, -0.5f, o->Light); break;
            }
            if (c->PK >= PVP_MURDERER2)
            {
                Vector(-0.4f, -0.4f, -0.4f, o->Light);
            }
        }
    }
    vec3_t Light;
    Vector(0.8f, 0.8f, 0.8f, Light);
    int iSelected = -1;
    float fNearestDist = 1000000000000.0f;

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;

        if (o->Live && o->Visible && o->Alpha > 0.f && c->Dead == 0 && !g_isCharacterBuff(o, eBuff_CrywolfNPCHide))
        {
            if (o->Kind == KIND_PLAYER)		//. bug fixed by soyaviper
            {
                for (int j = 0; j < PartyNumber; ++j)
                {
                    PARTY_t* p = &Party[j];

                    if (p->index != -2) continue;
                    if (p->index > -1) continue;

                    int length = max(wcslen(p->Name), max(1, wcslen(c->ID)));

                    if (!wcsncmp(p->Name, c->ID, length))
                    {
                        p->index = i;
                        break;
                    }
                }
            }

            if (Main && c == Hero)
            {
                continue;
            }

            if (c->m_bIsSelected == false)
            {
                continue;
            }

            if ((Kind & o->Kind) == o->Kind)
            {
                if (o->Type == MODEL_SMELTING_NPC)
                {
                    o->OBB.StartPos[2] += 300.0f;
                }
                else if (o->Type == MODEL_MAYA_HAND_LEFT || o->Type == MODEL_MAYA_HAND_RIGHT)
                {
                    o->OBB.StartPos[2] += 200.0f;
                }
                else if (o->Type == MODEL_KANTURU2ND_ENTER_NPC)
                {
                    o->OBB.StartPos[0] -= 100.f;
                    o->OBB.StartPos[2] += 100.0f;
                    o->OBB.XAxis[0] += 100.f;
                    o->OBB.ZAxis[2] += 100.f;
                }

                if (CollisionDetectLineToOBB(MousePosition, MouseTarget, o->OBB))
                {
                    vec3_t vSub;
                    VectorSubtract(o->Position, CameraPosition, vSub);

                    float fNewDist = DotProduct(vSub, vSub);

                    if (fNewDist < fNearestDist)
                    {
                        BOOL bCanTalk = TRUE;
                        if (gMapManager.WorldActive == WD_0LORENCIA || gMapManager.WorldActive == WD_2DEVIAS)
                        {
                            int Index = ((int)o->Position[1] / (int)TERRAIN_SCALE) * 256 + ((int)o->Position[0] / (int)TERRAIN_SCALE);
                            if ((gMapManager.WorldActive == WD_0LORENCIA && TerrainMappingLayer1[Index] == 4) ||
                                (gMapManager.WorldActive == WD_2DEVIAS && TerrainMappingLayer1[Index] == 3))
                            {
                                if (TerrainMappingLayer1[Index] != HeroTile && (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 11))
                                    bCanTalk = FALSE;
                            }
                        }
                        if (bCanTalk == TRUE)
                        {
                            iSelected = i;
                            fNearestDist = fNewDist;
                        }
                    }
                }
            }
        }
    }

    for (int j = 0; j < PartyNumber; ++j)
    {
        PARTY_t* p = &Party[j];

        if (p->index >= 0) continue;

        int length = max(wcslen(p->Name), max(1, wcslen(Hero->ID)));

        if (!wcsncmp(p->Name, Hero->ID, length))
        {
            p->index = -3;
        }
        else
        {
            p->index = -1;
        }
    }

    return iSelected;
}

int SelectOperate()
{
    for (int i = 0; i < MAX_OPERATES; i++)
    {
        OPERATE* n = &Operates[i];
        OBJECT* o = n->Owner;
        if (n->Live && o->Visible && o->HiddenMesh == -1)
        {
            float* Light = &o->Light[0];
            Vector(0.2f, 0.2f, 0.2f, Light);
        }
    }
    if (IsBattleCastleStart() && gMapManager.WorldActive == WD_30BATTLECASTLE)
        return -1;

    for (int i = 0; i < MAX_OPERATES; i++)
    {
        OPERATE* n = &Operates[i];
        OBJECT* o = n->Owner;
        if (n->Live && o->Visible)
        {
            float* Light = &o->Light[0];
            if (CollisionDetectLineToOBB(MousePosition, MouseTarget, o->OBB))
            {
                Vector(1.5f, 1.5f, 1.5f, Light);
                return i;
            }
        }
    }
    return -1;
}

void SelectObjects()
{
    BYTE CKind_1, CKind_2;

    if (g_pOption->IsAutoAttack() && gMapManager.WorldActive != WD_6STADIUM && gMapManager.InChaosCastle() == false)
    {
        if (SelectedCharacter < 0 || SelectedCharacter >= MAX_CHARACTERS_CLIENT + 1)
        {
            SelectedCharacter = -1;
            Attacking = -1;
        }
        else
        {
            CHARACTER* sc = &CharactersClient[SelectedCharacter];

            if (sc->Dead > 0 || sc->Object.Kind != KIND_MONSTER)
            {
                SelectedCharacter = -1;
                Attacking = -1;
            }

            if (Attacking != -1)
            {
                if (MouseLButton || MouseLButtonPush || MouseRButton || MouseRButtonPush || Hero->Dead > 0)
                {
                    SelectedCharacter = -1;
                }
            }
            else
            {
                SelectedCharacter = -1;
            }
        }
    }
    else
    {
        SelectedCharacter = -1;
        Attacking = -1;
    }

    SelectedItem = -1;
    SelectedNpc = -1;
    SelectedOperate = -1;

    if (!MouseOnWindow && false == g_pNewUISystem->CheckMouseUse() && SEASON3B::CheckMouseIn(0, 0, GetScreenWidth(), 429))
    {
        if (HIBYTE(GetAsyncKeyState(VK_MENU)) == 128)
        {
            if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem() == NULL)
                SelectedItem = SelectItem();

            if (SelectedItem == -1)
            {
                SelectedNpc = SelectCharacter(KIND_NPC);
                if (SelectedNpc == -1)
                {
                    SelectedCharacter = SelectCharacter(KIND_MONSTER | KIND_EDIT);
                    if (SelectedCharacter == -1)
                    {
                        SelectedCharacter = SelectCharacter(KIND_PLAYER);
                        if (SelectedCharacter == -1)
                        {
                            SelectedOperate = SelectOperate();
                        }
                    }
                }
            }
            else
            {
                g_pPartyManager->SearchPartyMember();
            }
        }
        else
        {
            CKind_1 = KIND_MONSTER | KIND_EDIT;
            CKind_2 = KIND_PLAYER;

            if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_WIZARD)
            {
                auto Skill = CharacterAttribute->Skill[Hero->CurrentSkill];

                if (Skill == AT_SKILL_HEALING
                    || Skill == AT_SKILL_HEALING_STR
                    || Skill == AT_SKILL_DEFENSE
                    || Skill == AT_SKILL_DEFENSE_STR
                    || Skill == AT_SKILL_DEFENSE_MASTERY
                    || Skill == AT_SKILL_ATTACK
                    || Skill == AT_SKILL_ATTACK_STR
                    || Skill == AT_SKILL_ATTACK_MASTERY
                    || Skill == AT_SKILL_TELEPORT_ALLY
                    || Skill == AT_SKILL_SOUL_BARRIER
                    || Skill == AT_SKILL_SOUL_BARRIER_STR
                    || Skill == AT_SKILL_SOUL_BARRIER_PROFICIENCY
                    )
                {
                    CKind_1 = KIND_PLAYER;
                    CKind_2 = KIND_MONSTER | KIND_EDIT;
                }
            }

            if (g_pPartyListWindow && g_pPartyListWindow->GetSelectedCharacter() != -1)
            {
                g_pPartyManager->SearchPartyMember();
            }
            else
            {
                if (SelectedCharacter == -1)
                {
                    SelectedCharacter = SelectCharacter(CKind_1);
                }
                if (SelectedCharacter == -1)
                {
                    SelectedCharacter = SelectCharacter(CKind_2);
                    if (SelectedCharacter == -1)
                    {
                        SelectedNpc = SelectCharacter(KIND_NPC);
                        if (SelectedNpc == -1)
                        {
                            if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem() == NULL)
                            {
                                SelectedItem = SelectItem();
                            }
                            if (SelectedItem == -1)
                            {
                                SelectedOperate = SelectOperate();
                            }
                        }
                    }
                }
                else if (Attacking != -1)
                {
                    g_pPartyManager->SearchPartyMember();
                }
            }
        }
    }
    else
    {
        g_pPartyManager->SearchPartyMember();
    }

    if (SelectedCharacter == -1)
    {
        Attacking = -1;
    }

    if (g_pPartyListWindow)
    {
        g_pPartyListWindow->SetListBGColor();
    }
}

int FindHotKey(int Skill)
{
    int SkillIndex = 0;

    for (int i = 0; i < MAX_MAGIC; i++)
    {
        if (CharacterAttribute->Skill[i] == Skill)
        {
            SkillIndex = i;
            break;
        }
    }

    return SkillIndex;
}

extern int   CheckX;
extern int   CheckY;
extern int   CheckSkill;

void SendMacroChat(wchar_t* Text)
{
    if (!CheckCommand(Text, true))
    {
        if ((Hero->Helper.Type<MODEL_HORN_OF_UNIRIA || Hero->Helper.Type>MODEL_DARK_HORSE_ITEM) || Hero->SafeZone)
        {
            // Send animation for specific texts
            CheckChatText(Text);
        }

        //if (CheckAbuseFilter(Text))
        //{
        //    SendChat(GlobalText[570]);
        //}
        //else
        //{
        //    SendChat(Text);
        //}

        SocketClient->ToGameServer()->SendPublicChatMessage(Hero->ID, Text);

        LastMacroTime = GetTickCount64();
    }
}

void MoveInterface()
{
    if (g_Direction.IsDirection())
    {
        return;
    }

    if (battleCastle::IsBattleCastleStart() == true)
    {
        if (CharacterAttribute->SkillNumber > 0)
        {
            UseBattleMasterSkill();
        }
    }

    if (LastMacroTime < GetTickCount64() - MacroCooldownMs)
    {
        if (gMapManager.InChaosCastle() == false)
        {
            if (HIBYTE(GetAsyncKeyState(VK_MENU)))
            {
                for (int i = 0; i < 9; i++)
                {
                    if (HIBYTE(GetAsyncKeyState('1' + i)))
                    {
                        SendMacroChat(MacroText[i]);
                    }
                }
                if (HIBYTE(GetAsyncKeyState('0')))
                {
                    SendMacroChat(MacroText[9]);
                }
            }
        }
    }

    if (Hero->Dead == 0)
    {
        g_pMainFrame->UseHotKeyItemRButton();
    }

    if (g_pUIManager->IsInputEnable())
    {
        int x, y, Width, Height;
        if (g_iChatInputType == 0)
        {
            Width = 190; Height = 29; x = 186; y = 415;
            if (MouseX >= x && MouseX < x + Width && MouseY >=
                y && MouseY < y + Height)
            {
                if (MouseLButtonPush)
                {
                    MouseLButtonPush = false;
                    InputIndex = 0;
                    MouseUpdateTime = 0;
                    MouseUpdateTimeMax = 6;
                    PlayBuffer(SOUND_CLICK01);
                }
            }
            Width = 58; Height = 29; x = 186 + 190; y = 415;
            if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height)
            {
                if (MouseLButtonPush)
                {
                    MouseLButtonPush = false;
                    InputIndex = 1;
                    MouseUpdateTime = 0;
                    MouseUpdateTimeMax = 6;
                    PlayBuffer(SOUND_CLICK01);
                }
            }
        }
        Width = 20; Height = 29; x = 186 + 190 + 58; y = 415;
        if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height)
        {
            if (MouseLButtonPush)
            {
                MouseLButtonPush = false;
                if (WhisperEnable)
                    WhisperEnable = false;
                else
                    WhisperEnable = true;
                MouseUpdateTime = 0;
                MouseUpdateTimeMax = 6;
                PlayBuffer(SOUND_CLICK01);
            }
        }
    }
}

bool IsCanBCSkill(int Type)
{
    if (Type == 44 || Type == 45 || Type == 46 || Type == 57 || Type == 73 || Type == 74
        || Type == AT_SKILL_OCCUPY)
    {
        if (!gMapManager.InBattleCastle() || !battleCastle::IsBattleCastleStart())
        {
            return false;
        }
    }

    return true;
}

bool CheckSkillUseCondition(OBJECT* o, int Type)
{
#ifdef LEM_FIX_USER_LOGOUT_SKILL_ENABLE
    if (g_bExit)		return false;
#endif	// LEM_FIX_USER_LOGOUT_SKILL_ENABLE [lem_2010.8.18]
    if (IsCanBCSkill(Type) == false)
    {
        return false;
    }

    BYTE SkillUseType = SkillAttribute[Type].SkillUseType;

    if (SkillUseType == SKILL_USE_TYPE_BRAND && g_isCharacterBuff(o, eBuff_AddSkill) == false)
    {
        return false;
    }

    return true;
}

extern wchar_t TextList[50][100];
extern int  TextListColor[50];
extern int  TextBold[50];

void GetTime(DWORD time, std::wstring& timeText, bool isSecond)
{
    wchar_t buff[100];

    if (isSecond)
    {
        DWORD day = time / (1440 * 60);
        DWORD oClock = (time - (day * (1440 * 60))) / 3600;
        DWORD minutes = (time - ((oClock * 3600) + (day * (1440 * 60)))) / 60;
        DWORD second = time % 60;

        if (day != 0)
        {
            swprintf(buff, L"%d %s %d %s %d %s %d %s", day, GlobalText[2298], oClock, GlobalText[2299], minutes, GlobalText[2300], second, GlobalText[2301]);
            timeText = buff;
        }
        else if (day == 0 && oClock != 0)
        {
            swprintf(buff, L"%d %s %d %s %d %s", oClock, GlobalText[2299], minutes, GlobalText[2300], second, GlobalText[2301]);
            timeText = buff;
        }
        else if (day == 0 && oClock == 0 && minutes != 0)
        {
            swprintf(buff, L"%d %s %d %s", minutes, GlobalText[2300], second, GlobalText[2301]);
            timeText = buff;
        }
        else if (day == 0 && oClock == 0 && minutes == 0)
        {
            timeText = GlobalText[2308];
        }
    }
    else
    {
        DWORD day = time / 1440;
        DWORD oClock = (time - (day * 1440)) / 60;
        DWORD minutes = time % 60;

        if (day != 0)
        {
            swprintf(buff, L"%d %s %d %s %d %s", day, GlobalText[2298], oClock, GlobalText[2299], minutes, GlobalText[2300]);
            timeText = buff;
        }
        else if (day == 0 && oClock != 0)
        {
            swprintf(buff, L"%d %s %d %s", oClock, GlobalText[2299], minutes, GlobalText[2300]);
            timeText = buff;
        }
        else if (day == 0 && oClock == 0 && minutes != 0)
        {
            swprintf(buff, L"%d %s", minutes, GlobalText[2300]);
            timeText = buff;
        }
    }
}

void RenderBar(float x, float y, float Width, float Height, float Bar, bool Disabled, bool clipping)
{
    if (clipping)
    {
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x + Width + 4 > 640) x = 640 - (Width + 1 + 4);
        if (y + Height + 4 > 480 - 47) y = 480 - 47 - (Height + 1 + 4);
    }

    EnableAlphaTest();
    glColor4f(0.f, 0.f, 0.f, 0.5f);
    RenderColor(x + 1, y + 1, Width + 4, Height + 4);

    EnableAlphaBlend();
    if (Disabled)
        glColor3f(0.2f, 0.0f, 0.0f);
    else
        glColor3f(0.f, 0.2f, 0.2f);
    RenderColor(x, y, Width + 4, Height + 4);
    if (Disabled)
        glColor3f(50.f / 255.f, 10 / 255.f, 0.f);
    else
        glColor3f(0.f / 255.f, 50 / 255.f, 50.f / 255.f);
    RenderColor(x + 2, y + 2, Width, Height);
    if (Disabled)
        glColor3f(200.f / 255.f, 50 / 255.f, 0.f);
    else
        glColor3f(0.f / 255.f, 200 / 255.f, 50.f / 255.f);
    RenderColor(x + 2, y + 2, Bar, Height);

    DisableAlphaBlend();
}

void RenderSwichState()
{
    wchar_t Buff[300];

    if (Switch_Info == NULL)
        return;

    int ax = (Hero->PositionX);
    int ay = (Hero->PositionY);

    if (!(ax >= 150 && ax <= 200 && ay >= 180 && ay <= 230))
    {
        Delete_Switch();
        return;
    }

    for (int i = 0; i < 2; i++)
    {
        if (Switch_Info[i].m_bySwitchState > 0)
        {
            swprintf(Buff, L"%s%d / %s / %s", GlobalText[1981], i + 1, Switch_Info[i].m_szGuildName, Switch_Info[i].m_szUserName);
            g_pRenderText->SetFont(g_hFont);
            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->SetBgColor(0);
            g_pRenderText->RenderText(0, 480 - 85 + (i * 15), Buff, 640, 0, RT3_SORT_CENTER);
        }
    }
}

void RenderInterface(bool Render)
{
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    RenderOutSides();
    RenderPartyHP();

    RenderSwichState();
    battleCastle::RenderBuildTimes();

    g_pUIMapName->Render();		// rozy

    //	M34CryWolf1st::Render_Mvp_Interface();
    M39Kanturu3rd::RenderKanturu3rdinterface();
    //	M34CryWolf1st::Sub_Interface();
}

void RenderOutSides()
{
    if (gMapManager.WorldActive == WD_8TARKAN)
    {
        EnableAlphaTest();
        glColor4f(1.f, 1.f, 1.f, 0.5f);
        EnableAlphaBlend();
        glColor3f(0.3f, 0.3f, 0.25f);
        float WindX = (float)((int)WorldTime % 100000) * 0.0002f;
        RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, 640.f, 480.f - 45.f, WindX, 0.f, 0.3f, 0.3f);
        float WindX2 = (float)((int)WorldTime % 100000) * 0.001f;
        RenderBitmapUV(BITMAP_CHROME + 3, 0.f, 0.f, 640.f, 480.f - 45.f, WindX2, 0.f, 3.f, 2.f);
    }
#ifdef ASG_ADD_MAP_KARUTAN
    else if (IsKarutanMap())
    {
        EnableAlphaTest();
        EnableAlphaBlend();
        glColor3f(0.3f, 0.3f, 0.25f);
        float fWindX = (float)((int)WorldTime % 100000) * 0.004f;
        RenderBitmapUV(BITMAP_CHROME + 3, 0.f, 0.f, 640.f, 480.f - 45.f, fWindX, 0.f, 3.f, 2.f);
    }
#endif	// ASG_ADD_MAP_KARUTAN
    else if (WD_34CRYWOLF_1ST == gMapManager.WorldActive)
    {
        if (ashies == true)
        {
            M34CryWolf1st::RenderBaseSmoke();
            ashies = false;
        }
    }
    else if (gMapManager.WorldActive == WD_56MAP_SWAMP_OF_QUIET)
    {
        SEASON3C::GMSwampOfQuiet::RenderBaseSmoke();
    }
    else if (IsIceCity())
    {
        g_Raklion.RenderBaseSmoke();
    }
    else
    {
        battleCastle::RenderBaseSmoke();
    }
    TheMapProcess().RenderFrontSideVisual();

    glColor3f(1.f, 1.f, 1.f);
}

void MoveTournamentInterface()
{
    static unsigned int s_effectCount = 0;
    int Width = 70, Height = 20;
    int WindowX = (640 - Width) / 2;
    int WindowY = (480 - Height) / 2 + 50;
    if (MouseLButtonPush)
    {
        float wRight = WindowX + Width;
        float wBottom = WindowY + Height;

        if (WindowY <= MouseY && MouseY <= WindowY + Height &&
            WindowX <= MouseX && MouseX <= WindowX + Width)
        {
            g_wtMatchResult.Clear();
            g_wtMatchTimeLeft.m_Time = 0;
        }
    }

    if (g_iGoalEffect)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            MoveBattleSoccerEffect(&CharactersClient[i]);
        g_iGoalEffect = 0;
    }
}

void MoveBattleSoccerEffect(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    if (o->Live)
    {
        if (g_iGoalEffect == 1)
        {
            if (c->GuildTeam == 1)
            {
                vec3_t Position, Angle;
                for (int i = 0; i < 36; ++i)
                {
                    Angle[0] = -10.f;
                    Angle[1] = 0.f;
                    Angle[2] = 45.f;
                    float x_bias = cosf(Q_PI / 180.f * 10.0f * i);
                    float y_bias = sinf(Q_PI / 180.f * 10.0f * i);
                    Position[0] = c->Object.Position[0] + x_bias * TERRAIN_SCALE;
                    Position[1] = c->Object.Position[1] + y_bias * TERRAIN_SCALE;
                    Position[2] = c->Object.Position[2];
                    CreateJoint(BITMAP_FLARE, Position, Position, Angle, 22, &c->Object, 35);
                }
            }
        }
        else if (g_iGoalEffect == 2)
        {
            if (c->GuildTeam == 2)
            {
                vec3_t Position, Angle;
                for (int i = 0; i < 36; ++i)
                {
                    Angle[0] = -10.f;
                    Angle[1] = 0.f;
                    Angle[2] = 45.f;
                    float x_bias = cosf(Q_PI / 180.f * 10.0f * i);
                    float y_bias = sinf(Q_PI / 180.f * 10.0f * i);
                    Position[0] = c->Object.Position[0] + x_bias * TERRAIN_SCALE;
                    Position[1] = c->Object.Position[1] + y_bias * TERRAIN_SCALE;
                    Position[2] = c->Object.Position[2];
                    CreateJoint(BITMAP_FLARE, Position, Position, Angle, 22, &c->Object, 35);
                }
            }
        }
    }
}

void RenderTournamentInterface()
{
    int Width = 300, Height = 2 * 5 + 6 * 30;
    int WindowX = (640 - Width) / 2;
    int WindowY = 120 + 0;
    float x = 0.0f, y = 0.0f;
    wchar_t t_Str[20];
    wcscpy(t_Str, L"");

    if (g_wtMatchTimeLeft.m_Time)
    {
        int t_valueSec = g_wtMatchTimeLeft.m_Time % 60;
        int t_valueMin = g_wtMatchTimeLeft.m_Time / 60;
        if (t_valueMin <= 10)
        {
            g_pRenderText->SetFont(g_hFontBig);
            g_pRenderText->SetTextColor(255, 10, 10, 255);
            g_pRenderText->SetBgColor(0);

            if (g_wtMatchTimeLeft.m_Type == 3)
            {
                g_pRenderText->SetTextColor(255, 255, 10, 255);
                swprintf(t_Str, GlobalText[1392], t_valueSec);
            }
            else
            {
                if (g_wtMatchTimeLeft.m_Time < 60)
                {
                    g_pRenderText->SetTextColor(255, 255, 10, 255);
                }
                if (t_valueSec < 10)
                {
                    swprintf(t_Str, GlobalText[1390], t_valueMin, t_valueSec);
                }
                else
                {
                    swprintf(t_Str, GlobalText[1391], t_valueMin, t_valueSec);
                }
            }
            x += (float)GetScreenWidth() / 2; y += 350;
            g_pRenderText->RenderText((int)x, (int)y, t_Str, 0, 0, RT3_WRITE_CENTER); x++; y++;

            g_pRenderText->SetTextColor(0xffffffff);
            g_pRenderText->RenderText((int)x, (int)y, t_Str, 0, 0, RT3_WRITE_CENTER);

            g_pRenderText->SetFont(g_hFont);
            g_pRenderText->SetTextColor(255, 255, 255, 255);
        }
    }

    if (!wcscmp(g_wtMatchResult.m_MatchTeamName1, L""))
    {
        return;
    }

    Width = 300; Height = 2 * 5 + 5 * 40; WindowX = (640 - Width) / 2; WindowY = 120 + 0;
    int yPos = WindowY;
    RenderBitmap(BITMAP_INTERFACE + 22, (float)WindowX, (float)yPos, (float)Width, (float)5, 0.f, 0.f, Width / 512.f, 5.f / 8.f);
    yPos += 5;

    for (int i = 0; i < 5; ++i)
    {
        RenderBitmap(BITMAP_INTERFACE + 21, WindowX, (float)yPos,
            Width, 40.f, 0.f, 0.0f, 213.f / 256.f, 40.f / 64.f);
        yPos += 40.f;
    }
    RenderBitmap(BITMAP_INTERFACE + 22, (float)WindowX, (float)yPos, (float)Width, (float)5, 0.f, 0.f, Width / 512.f, 5.f / 8.f);

    EnableAlphaBlend();
    glColor4f(1.f, 1.f, 1.f, 1.f);
    g_pRenderText->SetFont(g_hFontBig);
    g_pRenderText->SetTextColor(200, 240, 255, 255);
    swprintf(t_Str, GlobalText[1393]);
    g_pRenderText->RenderText(WindowX + Width / 2 - 50, WindowY + 20, t_Str);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    swprintf(t_Str, GlobalText[1394]);
    g_pRenderText->SetTextColor(255, 255, 10, 255);
    g_pRenderText->RenderText(WindowX + Width / 2 - 13, WindowY + 50, t_Str);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    float t_temp = 0.0f;
    swprintf(t_Str, L"%s", g_wtMatchResult.m_MatchTeamName1);
    t_temp = (MAX_ID_SIZE - wcslen(t_Str)) * 5;
    g_pRenderText->RenderText(WindowX + 10 + t_temp, WindowY + 50, t_Str);
    swprintf(t_Str, L"%s", g_wtMatchResult.m_MatchTeamName2);
    t_temp = (MAX_ID_SIZE - wcslen(t_Str)) * 5;
    g_pRenderText->RenderText(WindowX + Width - 120 + t_temp, WindowY + 50, t_Str);

    swprintf(t_Str, L"(%d)", g_wtMatchResult.m_Score1);
    g_pRenderText->RenderText(WindowX + 45, WindowY + 75, t_Str);
    swprintf(t_Str, L"(%d)", g_wtMatchResult.m_Score2);
    g_pRenderText->RenderText(WindowX + Width - 85, WindowY + 75, t_Str);

    if (g_wtMatchResult.m_Score1 == g_wtMatchResult.m_Score2)
    {
        g_pRenderText->SetFont(g_hFontBig);
        g_pRenderText->SetTextColor(255, 255, 10, 255);
        swprintf(t_Str, GlobalText[1395]);
        g_pRenderText->RenderText(WindowX + Width / 2 - 35, WindowY + 115, t_Str);
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
    }
    else if (g_wtMatchResult.m_Score1 > g_wtMatchResult.m_Score2)
    {
        g_pRenderText->SetFont(g_hFontBig);
        g_pRenderText->SetTextColor(255, 255, 10, 10);
        swprintf(t_Str, GlobalText[1396]);
        g_pRenderText->RenderText(WindowX + 47, WindowY + 115, t_Str);
        g_pRenderText->SetTextColor(255, 10, 10, 255);
        swprintf(t_Str, GlobalText[1397]);
        g_pRenderText->RenderText(WindowX + Width - 82, WindowY + 115, t_Str);
        g_pRenderText->SetFont(g_hFont);
    }
    else
    {
        g_pRenderText->SetFont(g_hFontBig);
        g_pRenderText->SetTextColor(255, 255, 10, 10);
        swprintf(t_Str, GlobalText[1397]);
        g_pRenderText->RenderText(WindowX + 47, WindowY + 115, t_Str);
        g_pRenderText->SetTextColor(255, 10, 10, 255);
        swprintf(t_Str, GlobalText[1396]);
        g_pRenderText->RenderText(WindowX + Width - 82, WindowY + 115, t_Str);
        g_pRenderText->SetFont(g_hFont);
    }
    g_pRenderText->SetFont(g_hFont);

    Width = 70; Height = 20; x = (640 - Width) / 2; y = (480 - Height) / 2 + 50;
    if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height)
    {
        RenderBitmap(BITMAP_INTERFACE + 12, (float)x, (float)y, (float)Width, (float)Height, 0.f, 0.f, Width / 128.f, Height / 32.f);
    }
    else
    {
        RenderBitmap(BITMAP_INTERFACE + 11, (float)x, (float)y, (float)Width, (float)Height, 0.f, 0.f, Width / 128.f, Height / 32.f);
    }

    glColor3f(1.f, 1.f, 1.f);
    DisableAlphaBlend();
}

void RenderPartyHP()
{
    if (PartyNumber <= 0) return;

    float   Width = 38.f;
    wchar_t    Text[100];

    for (int j = 0; j < PartyNumber; ++j)
    {
        PARTY_t* p = &Party[j];

        if (p->index <= -1) continue;

        CHARACTER* c = &CharactersClient[p->index];
        OBJECT* o = &c->Object;
        vec3_t      Position;
        int         ScreenX, ScreenY;

        Vector(o->Position[0], o->Position[1], o->Position[2] + o->BoundingBoxMax[2] + 100.f, Position);
        Projection(Position, &ScreenX, &ScreenY);
        ScreenX -= (int)(Width / 2);

        if ((MouseX >= ScreenX && MouseX < ScreenX + Width && MouseY >= ScreenY - 2 && MouseY < ScreenY + 6))
        {
            swprintf(Text, L"HP : %d0%%", p->stepHP);
            g_pRenderText->SetTextColor(255, 230, 210, 255);
            g_pRenderText->RenderText(ScreenX, ScreenY - 6, Text);
        }

        EnableAlphaTest();
        glColor4f(0.f, 0.f, 0.f, 0.5f);
        RenderColor((float)(ScreenX + 1), (float)(ScreenY + 1), Width + 4.f, 5.f);

        EnableAlphaBlend();
        glColor3f(0.2f, 0.0f, 0.0f);
        RenderColor((float)ScreenX, (float)ScreenY, Width + 4.f, 5.f);

        glColor3f(50.f / 255.f, 10 / 255.f, 0.f);
        RenderColor((float)(ScreenX + 2), (float)(ScreenY + 2), Width, 1.f);

        int stepHP = min(10, p->stepHP);

        glColor3f(250.f / 255.f, 10 / 255.f, 0.f);
        for (int k = 0; k < stepHP; ++k)
        {
            RenderColor((float)(ScreenX + 2 + (k * 4)), (float)(ScreenY + 2), 3.f, 2.f);
        }
        DisableAlphaBlend();
    }
    DisableAlphaBlend();
    glColor3f(1.f, 1.f, 1.f);
}

void RenderBooleans()
{
    g_pRenderText->SetFont(g_hFont);

    for (int i = 0; i < MAX_CHAT; i++)
    {
        CHAT* ci = &Chat[i];
        if (ci->IDLifeTime > 0 || ci->LifeTime[0] > 0)
        {
            vec3_t Position;
            int ScreenX, ScreenY;
            if (ci->Owner != NULL)
            {
                OBJECT* o = &ci->Owner->Object;
                Vector(o->Position[0], o->Position[1], o->Position[2] + o->BoundingBoxMax[2] + 60.f, Position);

                if (o->Type >= MODEL_LITTLESANTA && o->Type <= MODEL_LITTLESANTA_END)
                {
                    Vector(o->Position[0], o->Position[1], o->Position[2] + o->BoundingBoxMax[2] + 20.f, Position);
                }
                if (o->Type == MODEL_MERCHANT_MAN)
                {
                    Vector(o->Position[0], o->Position[1], o->Position[2] + o->BoundingBoxMax[2] + 20.f, Position);
                }
                Projection(Position, &ScreenX, &ScreenY);
            }
            else
            {
                Projection(ci->Position, &ScreenX, &ScreenY);
            }
            SetBooleanPosition(ci);
            ci->x = ScreenX - (ci->Width / 2);
            ci->y = ScreenY - ci->Height;
        }
    }

    for (int i = 0; i < MAX_CHAT; i++)		//. Bubble sorting
    {
        CHAT* ci = &Chat[i];
        if (ci->IDLifeTime > 0 || ci->LifeTime[0] > 0)
        {
            for (int j = 0; j < MAX_CHAT; j++)
            {
                CHAT* cj = &Chat[j];
                if (i != j && (cj->IDLifeTime > 0 || cj->LifeTime[0] > 0))
                {
                    if (ci->x + ci->Width > cj->x && ci->x<cj->x + cj->Width &&
                        ci->y + ci->Height>cj->y && ci->y < cj->y + cj->Height)
                    {
                        if (ci->y < cj->y + cj->Height / 2)
                            ci->y = cj->y - ci->Height;
                        else
                            ci->y = cj->y + cj->Height;
                    }
                }
            }
        }
    }

    for (int i = 0; i < MAX_CHAT; i++)
    {
        CHAT* ci = &Chat[i];
        if (ci->IDLifeTime > 0 || ci->LifeTime[0] > 0)
        {
            //. Fit to screen
            if (ci->x < 0) ci->x = 0;
            if (ci->x >= (int)WindowWidth - ci->Width) ci->x = WindowWidth - ci->Width;
            if (ci->y < 0) ci->y = 0;
            if (ci->y >= (int)WindowHeight - ci->Height) ci->y = WindowHeight - ci->Height;
            RenderBoolean(ci->x, ci->y, ci);
        }
    }
}

void RenderTimes()
{
    const uint64_t currentTickCount = GetTickCount64();
    if (LastMacroTime > currentTickCount - MacroCooldownMs)
    {
        constexpr float width = 50;
        constexpr float height = 2;
        constexpr int y = 480 - 48 - 40;
        const float x = (static_cast<float>(GetScreenWidth()) - width) / 2.0f;

        const uint64_t remainingMacroCooldownTime = MacroCooldownMs - (currentTickCount - LastMacroTime);
        const float progressValue = static_cast<float>(remainingMacroCooldownTime) / MacroCooldownMs * width;

        EnableAlphaTest();
        g_pRenderText->RenderText(static_cast<int>(x), y, L"Macro Time");
        RenderBar(x, y + 12, width, height, (float)progressValue);
    }

    glColor3f(1.f, 1.f, 1.f);

    matchEvent::RenderTime();
}

extern int g_iKeyPadEnable;

void RenderCursor()
{
    EnableAlphaTest();
    glColor3f(1.f, 1.f, 1.f);

    float u = 0.f;
    float v = 0.f;
    int Frame = (int)(WorldTime * 0.01f) % 6;
    if (Frame == 1 || Frame == 3 || Frame == 5) u = 0.5f;
    if (Frame == 2 || Frame == 3 || Frame == 4) v = 0.5f;
    if (g_iKeyPadEnable || ErrorMessage)
    {
        RenderBitmap(BITMAP_CURSOR, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
    }
    else if (SelectedItem != -1)
    {
        RenderBitmap(BITMAP_CURSOR + 3, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
    }
    else if (SelectedNpc != -1)
    {
        if (M38Kanturu2nd::Is_Kanturu2nd())
        {
            RenderBitmap(BITMAP_CURSOR2, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
        }
        else
        {
            RenderBitmap(BITMAP_CURSOR + 4, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f, u, v, 0.5f, 0.5f);
        }
    }
    else if (SelectedOperate != -1)
    {
        if ((gMapManager.WorldActive == WD_0LORENCIA && Operates[SelectedOperate].Owner->Type == MODEL_POSE_BOX) ||
            (gMapManager.WorldActive == WD_1DUNGEON && Operates[SelectedOperate].Owner->Type == 60) ||
            (gMapManager.WorldActive == WD_2DEVIAS && Operates[SelectedOperate].Owner->Type == 91) ||
            (gMapManager.WorldActive == WD_3NORIA && Operates[SelectedOperate].Owner->Type == 38)
            )
            RenderBitmap(BITMAP_CURSOR + 6, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
        else
            RenderBitmap(BITMAP_CURSOR + 7, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
    }
    else if ((!Hero->SafeZone/*||EnableEdit*/) && SelectedCharacter != -1)
    {
        if (CheckAttack() && !MouseOnWindow)
        {
            if (gMapManager.InBattleCastle())
            {
                RenderBitmap(BITMAP_CURSOR2, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
            }
            else
            {
                RenderBitmap(BITMAP_CURSOR + 2, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
            }
        }
        else
            RenderBitmap(BITMAP_CURSOR, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
    }
    else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND))
    {
        if (g_pCommandWindow->GetMouseCursor() == CURSOR_IDSELECT)
        {
            RenderBitmap(BITMAP_INTERFACE_EX + 29, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
        }
        else if (g_pCommandWindow->GetMouseCursor() == CURSOR_NORMAL)
        {
            RenderBitmap(BITMAP_CURSOR, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
        }
        else if (g_pCommandWindow->GetMouseCursor() == CURSOR_PUSH)
        {
            RenderBitmap(BITMAP_CURSOR + 1, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
        }
    }
    else if (((g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY) || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY_EXT))
        && g_pMyInventory->GetRepairMode() == SEASON3B::CNewUIMyInventory::REPAIR_MODE_ON)
        || (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)
            && g_pNPCShop->GetShopState() == SEASON3B::CNewUINPCShop::SHOP_STATE_REPAIR)
        )
    {
        if (MouseLButton == false)
        {
            RenderBitmap(BITMAP_CURSOR + 5, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
        }
        else
        {
            RenderBitmapRotate(BITMAP_CURSOR + 5, (float)MouseX + 5.f, (float)MouseY + 18.f, 24.f, 24.f, 45.f);
        }
    }
    else if (RepairEnable == 2)
    {
        if (sin(WorldTime * 0.02f) > 0)
        {
            RenderBitmapRotate(BITMAP_CURSOR + 5, (float)MouseX + 10.f, (float)MouseY + 10.f, 24.f, 24.f, 0.f);
        }
        else
        {
            RenderBitmapRotate(BITMAP_CURSOR + 5, (float)MouseX + 5.f, (float)MouseY + 18.f, 24.f, 24.f, 45.f);
        }
    }
    else
    {
        if (!MouseLButton)
            RenderBitmap(BITMAP_CURSOR, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
        else
        {
            if (DontMove)
                RenderBitmap(BITMAP_CURSOR + 8, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
            else
                RenderBitmap(BITMAP_CURSOR + 1, (float)MouseX - 2.f, (float)MouseY - 2.f, 24.f, 24.f);
        }
    }
}

void BackSelectModel()
{
    for (int i = 1; i < 20; i++)
    {
        if (SelectModel - i < 0)
            break;
        if (Models[SelectModel - i].NumMeshs > 0)
        {
            SelectModel -= i;
            break;
        }
    }
}

void ForwardSelectModel()
{
    for (int i = 1; i < 20; i++)
    {
        if (Models[SelectModel + i].NumMeshs > 0)
        {
            SelectModel += i;
            break;
        }
    }
}

extern int  EditMonsterNumber;

extern int MonsterKey;

void EditObjects()
{
    if (EditFlag == EDIT_MONSTER)
    {
        if (MouseLButtonPush)
        {
            MouseLButtonPush = false;
            bool Success = RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true);
            if (Success)
            {
                CHARACTER* c = CreateMonster((EMonsterType)MonsterScript[SelectMonster].Type, (BYTE)(CollisionPosition[0] / TERRAIN_SCALE), (BYTE)(CollisionPosition[1] / TERRAIN_SCALE), MonsterKey++);
                c->Object.Kind = KIND_EDIT;
            }
        }
        if (MouseRButtonPush)
        {
            MouseRButtonPush = false;
            if (SelectedCharacter != -1)
            {
                CharactersClient[SelectedCharacter].Object.Live = false;
            }
        }
    }
    if (EditFlag == EDIT_OBJECT)
    {
        if (MouseRButtonPush)
        {
            MouseRButtonPush = false;
            bool Success = RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true);
            if (Success)
            {
                OBJECT* o = CreateObject(SelectModel, CollisionPosition, PickObjectAngle);
                int Scale = (int)TERRAIN_SCALE;
                o->Position[0] = (float)((int)o->Position[0] / Scale + 0.5f) * Scale;
                o->Position[1] = (float)((int)o->Position[1] / Scale + 0.5f) * Scale;
            }
        }
        if (MouseLButtonPush)
        {
            MouseLButtonPush = false;
            if (!PickObject)
            {
                if (MouseX < 100 && MouseY < 100)
                {
                    PickObject = CreateObject(SelectModel, MouseTarget, PickObjectAngle);
                }
                else
                {
                    PickObject = CollisionDetectObjects(PickObject);
                    if (PickObject)
                        PickObjectHeight = PickObject->Position[2] - RequestTerrainHeight(PickObject->Position[0], PickObject->Position[1]);
                }
            }
        }
        if (PickObject)
        {
            if (MouseLButton)
            {
                bool Success = RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, true);
                if (Success)
                {
                    VectorCopy(CollisionPosition, PickObject->Position);
                    if (PickObjectLockHeight)
                    {
                        int Scale = (int)TERRAIN_SCALE / 2;
                        PickObject->Position[0] = (float)((int)PickObject->Position[0] / Scale * Scale);
                        PickObject->Position[1] = (float)((int)PickObject->Position[1] / Scale * Scale);
                    }
                    else
                        PickObject->Position[2] += PickObjectHeight;
                }
                if (HIBYTE(GetAsyncKeyState('Q')) == 128)
                    PickObject->Angle[0] -= 5.f;
                if (HIBYTE(GetAsyncKeyState('E')) == 128)
                    PickObject->Angle[0] += 5.f;
                if (HIBYTE(GetAsyncKeyState('A')) == 128)
                    PickObject->Angle[2] += 30.f;
                if (HIBYTE(GetAsyncKeyState('D')) == 128)
                    PickObject->Angle[2] -= 30.f;
                if (HIBYTE(GetAsyncKeyState('W')) == 128)
                    PickObjectHeight += 5.f;
                if (HIBYTE(GetAsyncKeyState('S')) == 128)
                    PickObjectHeight -= 5.f;
                if (HIBYTE(GetAsyncKeyState('R')) == 128)
                    PickObject->Scale += 0.02f;
                if (HIBYTE(GetAsyncKeyState('F')) == 128)
                    PickObject->Scale -= 0.02f;
                if (MouseX >= 640 - 100 && MouseY < 100)
                {
                    DeleteObject(PickObject, &ObjectBlock[PickObject->Block]);
                    PickObject = NULL;
                }
            }
            else
            {
                VectorCopy(PickObject->Angle, PickObjectAngle);
                CreateObject(PickObject->Type, PickObject->Position, PickObject->Angle, PickObject->Scale);
                if (EnableRandomObject)
                {
                    vec3_t Position, Angle;
                    for (int i = 0; i < 9; i++)
                    {
                        VectorCopy(PickObject->Position, Position);
                        VectorCopy(PickObject->Angle, Angle);
                        Position[0] += (float)(rand() % 2000 - 1000);
                        Position[1] += (float)(rand() % 2000 - 1000);
                        Position[2] = RequestTerrainHeight(Position[0], Position[1]);
                        Angle[2] = (float)(rand() % 360);
                        CreateObject(PickObject->Type, Position, Angle, PickObject->Scale + (float)(rand() % 16 - 8) * 0.01f);
                    }
                }
                DeleteObject(PickObject, &ObjectBlock[PickObject->Block]);
                PickObject = NULL;
            }
        }
    }

    if (EditFlag == EDIT_HEIGHT)
    {
        if (MouseLButton)
        {
            AddTerrainHeight(CollisionPosition[0], CollisionPosition[1], -10.f, BrushSize + 1, BackTerrainHeight);
            CreateTerrainNormal();
            CreateTerrainLight();
        }
        if (MouseRButton)
        {
            AddTerrainHeight(CollisionPosition[0], CollisionPosition[1], 10.f, BrushSize + 1, BackTerrainHeight);
            CreateTerrainNormal();
            CreateTerrainLight();
        }
    }
    if (EditFlag == EDIT_LIGHT)
    {
        vec3_t Light;
        if (MouseLButton)
        {
            switch (SelectColor)
            {
            case 0:
                Vector(0.1f, 0.1f, 0.1f, Light);
                break;
            case 1:
                Vector(-0.1f, -0.1f, -0.1f, Light);
                break;
            case 2:
                Vector(0.05f, -0.05f, -0.05f, Light);
                break;
            case 3:
                Vector(0.05f, 0.05f, -0.05f, Light);
                break;
            case 4:
                Vector(-0.05f, 0.05f, -0.05f, Light);
                break;
            case 5:
                Vector(-0.05f, 0.05f, 0.05f, Light);
                break;
            case 6:
                Vector(-0.05f, -0.05f, 0.05f, Light);
                break;
            case 7:
                Vector(0.05f, -0.05f, 0.05f, Light);
                break;
            }
            AddTerrainLightClip(CollisionPosition[0], CollisionPosition[1], Light, BrushSize + 1, TerrainLight);
            CreateTerrainLight();
        }
        if (MouseRButton)
        {
            int mx = (int)(CollisionPosition[0] / TERRAIN_SCALE);
            int my = (int)(CollisionPosition[1] / TERRAIN_SCALE);
            for (int y = my - 2; y <= my + 2; y++)
            {
                for (int x = mx - 2; x <= mx + 2; x++)
                {
                    int Index1 = TERRAIN_INDEX_REPEAT(x, y);
                    int Index2 = TERRAIN_INDEX_REPEAT(x - 1, y);
                    int Index3 = TERRAIN_INDEX_REPEAT(x + 1, y);
                    int Index4 = TERRAIN_INDEX_REPEAT(x, y - 1);
                    int Index5 = TERRAIN_INDEX_REPEAT(x, y + 1);
                    for (int i = 0; i < 3; i++)
                    {
                        TerrainLight[Index1][i] = (TerrainLight[Index1][i] + TerrainLight[Index2][i] + TerrainLight[Index3][i] + TerrainLight[Index4][i] + TerrainLight[Index5][i]) * 0.2f;
                    }
                }
            }
            CreateTerrainLight();
        }
    }
    if (EditFlag == EDIT_MAPPING)
    {
        int sx = 640 - 30;
        int sy = 0;
        for (int i = 0; i < 14; i++)
        {
            if (MouseX >= sx && MouseY >= sy + i * 30 && MouseX < sx + 30 && MouseY < sy + i * 30 + 30)
            {
                if (MouseLButton)
                {
                    SelectMapping = i;
                    return;
                }
            }
        }
        int x = (int)SelectXF;
        int y = (int)SelectYF;
        int Index1 = TERRAIN_INDEX_REPEAT(x, y);
        int Index2 = TERRAIN_INDEX_REPEAT(x + 1, y);
        int Index3 = TERRAIN_INDEX_REPEAT(x + 1, y + 1);
        int Index4 = TERRAIN_INDEX_REPEAT(x, y + 1);
        if (Bitmaps[BITMAP_MAPTILE + SelectMapping].Components != 4)
        {
            if (MouseLButton)
            {
                if (CurrentLayer == 0)
                {
                    for (int i = y - BrushSize; i <= y + BrushSize; i++)
                    {
                        for (int j = x - BrushSize; j <= x + BrushSize; j++)
                        {
                            TerrainMappingLayer1[TERRAIN_INDEX_REPEAT(j, i)] = SelectMapping;
                        }
                    }
                }
                if (CurrentLayer == 1)
                {
                    for (int i = y - 1; i <= y + 1; i++)
                    {
                        for (int j = x - 1; j <= x + 1; j++)
                        {
                            TerrainMappingLayer2[TERRAIN_INDEX_REPEAT(j, i)] = SelectMapping;
                        }
                    }
                    TerrainMappingAlpha[Index1] += 0.1f;
                    TerrainMappingAlpha[Index2] += 0.1f;
                    TerrainMappingAlpha[Index3] += 0.1f;
                    TerrainMappingAlpha[Index4] += 0.1f;
                    if (TerrainMappingAlpha[Index1] > 1.f) TerrainMappingAlpha[Index1] = 1.f;
                    if (TerrainMappingAlpha[Index2] > 1.f) TerrainMappingAlpha[Index2] = 1.f;
                    if (TerrainMappingAlpha[Index3] > 1.f) TerrainMappingAlpha[Index3] = 1.f;
                    if (TerrainMappingAlpha[Index4] > 1.f) TerrainMappingAlpha[Index4] = 1.f;
                }
            }
            if (MouseRButton)
            {
                if (CurrentLayer == 1)
                {
                    for (int i = y - 1; i <= y + 1; i++)
                    {
                        for (int j = x - 1; j <= x + 1; j++)
                        {
                            TerrainMappingLayer2[TERRAIN_INDEX_REPEAT(j, i)] = 255;
                        }
                    }
                    TerrainMappingAlpha[Index1] = 0.f;
                    TerrainMappingAlpha[Index2] = 0.f;
                    TerrainMappingAlpha[Index3] = 0.f;
                    TerrainMappingAlpha[Index4] = 0.f;
                }
            }
        }
    }
}

int TotalPacketSize = 0;
int OldTime = timeGetTime();
extern int MixItemValue;
extern int MixItemValueOld;

void RenderDebugWindow()
{
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 255);

    if (timeGetTime() - OldTime >= 1000)
    {
        OldTime = timeGetTime();
        TotalPacketSize = 0;
    }

#ifdef ENABLE_EDIT
    wchar_t Text[256] {};
    if (EditFlag == EDIT_MAPPING)
    {
        int sx = 640 - 30;
        int sy = 0;
        for (int i = 0; i < 14; i++)
        {
            if (i == SelectMapping)
                glColor3f(1.f, 1.f, 1.f);
            else
                glColor3f(0.8f, 0.8f, 0.8f);
            RenderBitmap(BITMAP_MAPTILE + i, (float)(sx), (float)(sy + i * 30), 30.f, 30.f);
        }
        if (CurrentLayer == 0)
            g_pRenderText->RenderText(640 - 100, sy, L"Background");
        else
            g_pRenderText->RenderText(640 - 100, sy, L"Layer1");
        swprintf(Text, L"Brush Size: %d", BrushSize * 2 + 1);
        g_pRenderText->RenderText(640 - 100, sy + 11, Text);
    }
    glColor3f(1.f, 1.f, 1.f);
    if (EditFlag == EDIT_OBJECT)
    {
        g_pRenderText->RenderText(640 - 100, 0, L"Garbage");
        CMultiLanguage::ConvertFromUtf8(Text, Models[SelectModel].Name, sizeof Models[SelectModel].Name);
        g_pRenderText->RenderText(0, 0, Text);
    }
    if (EditFlag == EDIT_MONSTER)
    {
        for (int i = 0; i < EditMonsterNumber; i++)
        {
            if (i == SelectMonster)
                glColor3f(1.f, 0.8f, 0.f);
            else
                glColor3f(1.f, 1.f, 1.f);

            swprintf(Text, L"%2d: %s", MonsterScript[i].Type, MonsterScript[i].Name);
            g_pRenderText->RenderText(640 - 100, i * 10, Text);
        }
    }
    if (EditFlag == EDIT_LIGHT)
    {
        for (int i = 0; i < 8; i++)
        {
            if (i == SelectColor)
                glColor3f(1.f, 0.8f, 0.f);
            else
                glColor3f(1.f, 1.f, 1.f);

            g_pRenderText->RenderText(640 - 64, i * 10, ColorTable[i]);
        }
    }
#endif //ENABLE_EDIT
}

void MouseRButtonReset()
{
    MouseRButtonPop = false;
    MouseRButtonPush = false;
    MouseRButton = false;
    MouseRButtonPress = 0;
}

bool IsGMCharacter()
{
    if ((Hero->Object.Kind == KIND_PLAYER && Hero->Object.Type == MODEL_PLAYER && Hero->Object.SubType == MODEL_GM_CHARACTER)
        || (g_isCharacterBuff((&Hero->Object), eBuff_GMEffect))
        || (Hero->CtlCode & CTLCODE_08OPERATOR)
        || (Hero->CtlCode & CTLCODE_20OPERATOR))
    {
        return true;
    }
    return false;
}

bool IsNonAttackGM()
{
    if (Hero->CtlCode & CTLCODE_04FORTV || Hero->CtlCode & CTLCODE_08OPERATOR)
    {
        return true;
    }

    return false;
}

bool IsIllegalMovementByUsingMsg(const wchar_t* szChatText)
{
    bool bCantFly = false;
    bool bCantSwim = false;
    bool bEquipChangeRing = false;

    bool bMoveAtlans = false;
    bool bMoveIcarus = false;

    wchar_t szChatTextUpperChars[256];
    wcscpy(szChatTextUpperChars, szChatText);
    _wcsupr(szChatTextUpperChars);

    short pEquipedRightRingType = (&CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT])->Type;
    short pEquipedLeftRingType = (&CharacterMachine->Equipment[EQUIPMENT_RING_LEFT])->Type;
    short pEquipedHelperType = (&CharacterMachine->Equipment[EQUIPMENT_HELPER])->Type;
    short pEquipedWingType = (&CharacterMachine->Equipment[EQUIPMENT_WING])->Type;

    if ((pEquipedWingType == -1 && pEquipedHelperType != ITEM_HORN_OF_DINORANT &&
        pEquipedHelperType != ITEM_HORN_OF_FENRIR && pEquipedHelperType != ITEM_DARK_HORSE_ITEM) ||
        pEquipedHelperType == ITEM_HORN_OF_UNIRIA)
    {
        bCantFly = true;
    }
    if (pEquipedHelperType == ITEM_HORN_OF_UNIRIA || pEquipedHelperType == ITEM_HORN_OF_DINORANT)
    {
        bCantSwim = true;
    }
    if (g_ChangeRingMgr->CheckMoveMap(pEquipedLeftRingType, pEquipedRightRingType))
    {
        bEquipChangeRing = true;
    }

    if ((wcsstr(szChatTextUpperChars, L"/MOVE") != NULL) ||
        (GlobalText.GetStringSize(260) > 0 && wcsstr(szChatTextUpperChars, GlobalText[260]) != NULL))
    {
        std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*> m_listMoveInfoData;
        m_listMoveInfoData = SEASON3B::CMoveCommandData::GetInstance()->GetMoveCommandDatalist();

        auto li = m_listMoveInfoData.begin();

        while (li != m_listMoveInfoData.end())
        {
            wchar_t cMapNameUpperChars[256];
            wcscpy(cMapNameUpperChars, (*li)->_ReqInfo.szSubMapName);
            _wcsupr(cMapNameUpperChars);

            if (wcsstr(szChatText, ((*li)->_ReqInfo.szMainMapName)) != NULL ||
                wcsstr(szChatTextUpperChars, cMapNameUpperChars) != NULL)
                break;

            li++;
        }

        if (li != m_listMoveInfoData.end())
        {
            if (wcsicmp((*li)->_ReqInfo.szMainMapName, GlobalText[37]) == 0)
            {
                bMoveAtlans = true;
            }
            else if (wcsicmp((*li)->_ReqInfo.szMainMapName, GlobalText[55]) == 0)
            {
                bMoveIcarus = true;
            }
        }
    }

    if (bCantSwim && bMoveAtlans)
    {
        g_pSystemLogBox->AddText(GlobalText[261], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return true;
    }

    if ((bCantFly || bEquipChangeRing) && bMoveIcarus)
    {
        g_pSystemLogBox->AddText(GlobalText[263], SEASON3B::TYPE_SYSTEM_MESSAGE);
        return true;
    }

    return false;
}