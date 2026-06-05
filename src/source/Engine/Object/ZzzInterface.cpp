///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <imm.h>
#include "UI/Legacy/UIManager.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/PlayerActionState.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Combat/ClassAttack.h"
#include "GameLogic/Combat/SkillCast.h"
#include "GameLogic/Combat/SkillExecution.h"
#include "Core/Input/ImeInput.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Effects/ZzzEffect.h"
#include "Scenes/SceneCore.h"
#include "Engine/Pathing/ZzzPath.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"


#include "GameLogic/Events/MatchEvent.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/NPCs/npcBreeder.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "Character/CSParts.h"
#include "UI/Legacy/UIMapName.h"	// rozy
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "World/MapInfra/MapManager.h"
#include "GameLogic/Events/Event.h"

#include "UI/NewUI/NewUISystem.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "UI/Legacy/UIControls.h"
#include "GameLogic/Social/PartyManager.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"
#include "GameLogic/Skills/SummonSystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "World/MapInfra/w_MapHeaders.h"
#include "GameLogic/Combat/DuelMgr.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "UI/NewUI/HUD/NewUIGensRanking.h"
#include "GameLogic/Social/MonkSystem.h"
#include "Character/CharacterManager.h"
#include "MUHelper/MuHelper.h"


#include "Camera/CameraProjection.h"
#include "Scenes/SceneCommon.h"

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
extern bool SelectFlag;

extern void RegisterBuff(eBuffState buff, OBJECT* o, const int bufftime = 0);
extern void UnRegisterBuff(eBuffState buff, OBJECT* o);

MovementSkill g_MovementSkill;

DWORD g_dwLatestMagicTick;

const   float   AutoMouseLimitTime = (1.f * 60.f * 60.f);
int   LoadingWorld = 0;
int   ItemHelp = 0;
int   MouseUpdateTime = 0;
int   MouseUpdateTimeMax = 6;
// Latched when a click opens an NPC conversation while the button is still held.
// The world click handler ignores the held button until it is physically released, so the
// same click can't fall through to ground movement and instantly close the NPC window.
// A fresh press (e.g. deliberately clicking the ground to walk away) still works normally.
static bool s_bIgnoreHeldClickAfterNpcTalk = false;
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

bool g_bRenderGameCursor = true;

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
int  InputTextMax[12] = { MAX_USERNAME_SIZE,MAX_USERNAME_SIZE,MAX_USERNAME_SIZE,30,30,10,14,20,40 };
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
        if (pCha->PK >= PVP_MURDERER2 && pCha->Object.Kind == KIND_PLAYER)
        {
            g_ErrorReport.Write(L"!!!!!!!!!!!!!!!!! PK !!!!!!!!!!!!!!!\n");
            g_ErrorReport.WriteCurrentTime();
            g_ErrorReport.Write(L" ID(%ls) PK(%d) GuildName(%ls)\n", pCha->ID, pCha->PK, GuildMark[pCha->GuildMarkIndex].GuildName);
#ifdef CONSOLE_DEBUG
            g_ConsoleDebug->Write(MCD_ERROR, L"[!!! PK !!! : ID(%ls) PK(%d) GuildName(%ls)\n]", pCha->ID, pCha->PK, GuildMark[pCha->GuildMarkIndex].GuildName);
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
            InputText[i][j] = 0;
        InputLength[i] = 0;
        InputTextHide[i] = 0;
    }

    if (g_iChatInputType == 1)
    {
        if (g_pSingleTextInputBox != nullptr)
        {
            g_pSingleTextInputBox->SetText(nullptr);
        }
        if (g_pSinglePasswdInputBox != nullptr)
        {
            g_pSinglePasswdInputBox->SetText(nullptr);
        }
    }
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
            Text[iTextSize] = 0;
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
            Text[iTextSize] = 0;
        }
        else
        {
            wcscpy(Text, InputText[Index]);
        }
        SIZE TextSize;
        g_pRenderText->RenderText(x, y, Text, InputTextWidth, 0, RT3_SORT_LEFT, &TextSize);
        Size = &TextSize;

        if (Index == InputIndex)
            Input::IME::SetCompositionWindow(x + Size->cx, y);

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

BYTE DebugText[10][256];
int  DebugTextLength[10];
char DebugTextCount = 0;

int RenderDebugText(int y)
{
    wchar_t Text[100];
    int Width = 16;
    for (int i = 0; i < std::min<int>(DebugTextCount, 10); i++)
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
                mu_swprintf(Text, L"%0.2x", DebugText[i][j]);
                g_pRenderText->RenderText(x, y, Text, 0, 0, RT3_SORT_CENTER, &TextSize);
            }
            else
            {
                mu_swprintf(Text, L"%c", DebugText[i][j]);
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
        if (((wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == 0) || (g_pPartyManager->IsPartyMember(SelectedCharacter))) && (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128))
        {
            return true;
        }
        else if ((wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) != 0) && !g_pPartyManager->IsPartyMember(SelectedCharacter))
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
            && wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == 0)
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
        else if (wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == 0)
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
            && wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == 0)
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

    if (EnableGuildWar && sc->PK >= PVP_MURDERER2 && sc->GuildMarkIndex != -1 && wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[sc->GuildMarkIndex].GuildName) == 0)
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

        if ((wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == 0) ||
            g_pPartyManager->IsPartyMember(SelectedCharacter))
        {
            if (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128)
            {
                return sc->Key;
            }

            return -1;
        }
    }

    if ((sc->PK >= PVP_MURDERER2 && sc->Object.Kind == KIND_PLAYER) || (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128 && sc != Hero))
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
                g_pSystemLogBox->AddText(I18N::Game::ArrowsReloaded, SEASON3B::TYPE_SYSTEM_MESSAGE);
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
                    g_pSystemLogBox->AddText(I18N::Game::ArrowsReloaded, SEASON3B::TYPE_SYSTEM_MESSAGE);
                }
        }
    }
    else
    {
        if (g_pSystemLogBox->CheckChatRedundancy(I18N::Game::NoMoreArrows) == FALSE)
        {
            g_pSystemLogBox->AddText(I18N::Game::NoMoreArrows, SEASON3B::TYPE_ERROR_MESSAGE);
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

        // To debounce repeat left-clicks while a swing animation is still
        // playing, gate on a small *include* list of the actual swing
        // animations -- never a `>= … <=` range, which would also sweep in
        // idle/locomotion frames and lock out attacks while mounted. e.g.:
        //
        //   if (o->CurrentAction == PLAYER_ATTACK_FIST
        //    || o->CurrentAction == PLAYER_ATTACK_SWORD_RIGHT1
        //    || ... )
        //       break;

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
                    GameLogic::Combat::UseSkillWarrior(c, o);
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
                        GameLogic::Combat::UseSkillWizard(c, o);
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
                        GameLogic::Combat::UseSkillElf(c, o);
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
                        GameLogic::Combat::UseSkillElf(c, o);
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
            GameLogic::Combat::AttackKnight(c, iSkill, Distance);
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
                    GameLogic::Combat::UseSkillRagefighter(c, o);
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
        else if (g_pMyInventory->FindEmptySlotIncludingExtensions(&Items[ItemKey].Item) == -1)
        {
            wchar_t Text[256];
            mu_swprintf(Text, I18N::Game::InventoryIsFull);

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
					mu_swprintf(text, I18N::Game::OnlyLevelAboveDCanDoTheChaosCombination, CHAOS_MIX_LEVEL);
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
						mu_swprintf(temp, I18N::Game::HealthHasBeenRecoveredOf100, 100);
					else if (monsterIndex == MONSTER_LITTLE_SANTA_BLUE)
						mu_swprintf(temp, I18N::Game::ManaHasBeenRecoveredOf100, 100);

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
						SocketClient->ToGameServer()->SendPetInfoRequest(PetType::DarkHorse, StorageType::Inventory, EQUIPMENT_HELPER);

					pItem = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
					if (pItem->Type == ITEM_DARK_RAVEN_ITEM)
						SocketClient->ToGameServer()->SendPetInfoRequest(PetType::DarkRaven, StorageType::Inventory, EQUIPMENT_WEAPON_LEFT);
				}
			}

			TargetNpc = -1;
			break;
		}
    case MOVEMENT_OPERATE:
        if (std::max<int>(abs((Hero->PositionX) - TargetX), abs((Hero->PositionY) - TargetY)) <= 1)
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
            int x = REFERENCE_WIDTH * MouseX / 260;
            SetCursorPos((x)*WindowWidth / REFERENCE_WIDTH, (MouseY)*WindowHeight / REFERENCE_HEIGHT);
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
    length = wcslen(I18N::Game::Exchange);
    if (wcscmp(string, I18N::Game::Exchange) == 0 || wcscmp(string, I18N::Game::Trade259) == 0 || wcsicmp(string, L"/trade") == 0)
    {
        return  true;
    }
    if (wcscmp(string, I18N::Game::Party256) == 0 || wcsicmp(string, L"/party") == 0 || wcsicmp(string, L"/pt") == 0)
    {
        return  true;
    }
    if (wcscmp(string, I18N::Game::Guild254) == 0 || wcsicmp(string, L"/guild") == 0)
    {
        return  true;
    }
    if (wcscmp(string, I18N::Game::Battle) == 0 || wcsicmp(string, L"/GuildWar") == 0)
    {
        return  true;
    }
    if (wcscmp(string, I18N::Game::BattleSoccer) == 0 || wcsicmp(string, L"/BattleSoccer") == 0)
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
        Name[iTextSize] = 0;

        if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
        {
            if (wcscmp(Name, I18N::Game::Exchange) == 0 || wcscmp(Name, I18N::Game::Trade259) == 0 || wcsicmp(Text, L"/trade") == 0)
            {
                if (gMapManager.InChaosCastle() == true)
                {
                    g_pSystemLogBox->AddText(I18N::Game::CanTBeInChaosCastle, SEASON3B::TYPE_SYSTEM_MESSAGE);

                    return false;
                }

                if (::IsStrifeMap(gMapManager.WorldActive))
                {
                    g_pSystemLogBox->AddText(I18N::Game::CannotApplyInBattleZone, SEASON3B::TYPE_SYSTEM_MESSAGE);
                    return false;
                }

                int level = CharacterAttribute->Level;

                if (level < TRADELIMITLEVEL)
                {
                    g_pSystemLogBox->AddText(I18N::Game::YouCanUseTheTradeCommandAtCharacterLevel6, SEASON3B::TYPE_SYSTEM_MESSAGE);
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
                            g_pSystemLogBox->AddText(I18N::Game::YouCannotTradeRightNow, SEASON3B::TYPE_ERROR_MESSAGE);
                            return true;
                        }

                        SocketClient->ToGameServer()->SendTradeRequest(c->Key);
                        wchar_t message[100]{};
                        mu_swprintf(message, I18N::Game::YouHaveRequestedSToTrade, c->ID);
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
                            g_pSystemLogBox->AddText(I18N::Game::YouCannotTradeRightNow, SEASON3B::TYPE_SYSTEM_MESSAGE);
                            return true;
                        }

                        BYTE Dir1 = (BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                        BYTE Dir2 = (BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8;
                        if (abs(Dir1 - Dir2) == 4) {
                            SocketClient->ToGameServer()->SendTradeRequest(c->Key);
                            wchar_t message[100]{};
                            mu_swprintf(message, I18N::Game::YouHaveRequestedSToTrade, c->ID);
                            g_pSystemLogBox->AddText(message, SEASON3B::TYPE_SYSTEM_MESSAGE);
                            break;
                        }
                    }
                }
                return true;
            }
        }

        if (wcscmp(Text, I18N::Game::Firecracker688) == 0)
        {
            return false;
        }

        if (wcscmp(Text, I18N::Game::PersonalStore1117) == 0 || wcsicmp(Text, L"/personalshop") == 0)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::CanTBeInChaosCastle, SEASON3B::TYPE_SYSTEM_MESSAGE);
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
                mu_swprintf(szError, I18N::Game::OnlyAboveLevelDCanUse, 6);
                g_pSystemLogBox->AddText(szError, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
            return true;
        }
        if (wcsstr(Text, I18N::Game::Buy) != nullptr || wcsstr(Text, L"/purchase") != nullptr)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::CanTBeInChaosCastle, SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }

            if (::IsStrifeMap(gMapManager.WorldActive))
            {
                g_pSystemLogBox->AddText(I18N::Game::CannotApplyInBattleZone, SEASON3B::TYPE_SYSTEM_MESSAGE);
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
                g_pSystemLogBox->AddText(I18N::Game::StoreCanTBeOpened, SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
            wchar_t szCmd[24];
            wchar_t szId[MAX_USERNAME_SIZE];
            swscanf(Text, L"%ls %ls", szCmd, szId);

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

        if (wcscmp(Text, I18N::Game::ViewStoreOn) == 0)
        {
            ShowShopTitles();
            g_pSystemLogBox->AddText(I18N::Game::CanViewPersonalStoreWindow, SEASON3B::TYPE_SYSTEM_MESSAGE);
        }

        if (wcscmp(Text, I18N::Game::ViewStoreOff) == 0)
        {
            HideShopTitles();
            g_pSystemLogBox->AddText(I18N::Game::CannotViewPersonalStoreWindow, SEASON3B::TYPE_ERROR_MESSAGE);
        }
        if (wcscmp(Text, I18N::Game::DuelChallenge) == 0 || wcsicmp(Text, L"/duelstart") == 0)
        {
#ifndef GUILD_WAR_EVENT
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::CanTBeInChaosCastle, SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
#endif// UILD_WAR_EVENT
            if (!g_DuelMgr.IsDuelEnabled())
            {
                int iLevel = CharacterAttribute->Level;
                if (iLevel < 30)
                {
                    wchar_t szError[48] = L"";
                    mu_swprintf(szError, I18N::Game::OpenOnlyForLevelDOrHigher, 30);
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
                g_pSystemLogBox->AddText(I18N::Game::YouCannotChallengePlayerIsAlreadyInADuel, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
        }
        if (wcscmp(Text, I18N::Game::DuelCancel) == 0 || wcsicmp(Text, L"/duelend") == 0)
        {
#ifndef GUILD_WAR_EVENT
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::CanTBeInChaosCastle, SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
#endif// GUILD_WAR_EVENT
            if (g_DuelMgr.IsDuelEnabled())
            {
                SocketClient->ToGameServer()->SendDuelStopRequest();
            }
        }
        if (wcscmp(Text, I18N::Game::Guild254) == 0 || wcsicmp(Text, L"/guild") == 0)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::CanTBeInChaosCastle, SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
            if (Hero->GuildStatus != G_NONE)
            {
                g_pSystemLogBox->AddText(I18N::Game::YouAreAlreadyInAGuild, SEASON3B::TYPE_SYSTEM_MESSAGE);
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
                    mu_swprintf(Text, I18N::Game::YouHaveRequestedSToJoinYourGuild, c->ID);
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
                        mu_swprintf(Text, I18N::Game::YouHaveRequestedSToJoinYourGuild, c->ID);
                        g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
                        break;
                    }
                }
            }
            return true;
        }
        if (!wcscmp(Text, I18N::Game::Alliance1354) || !wcsicmp(Text, L"/union") ||
            !wcscmp(Text, I18N::Game::Hostilities) || !wcsicmp(Text, L"/rival") ||
            !wcscmp(Text, I18N::Game::SuspendHostilities1357) || !wcsicmp(Text, L"/rivaloff"))
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::CanTBeInChaosCastle, SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
            if (Hero->GuildStatus == G_NONE)
            {
                g_pSystemLogBox->AddText(I18N::Game::DoNotBelongToTheGuild, SEASON3B::TYPE_SYSTEM_MESSAGE);
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
                    if (!wcscmp(Text, I18N::Game::Alliance1354) || !wcsicmp(Text, L"/union"))
                    {
                        //SendRequestGuildRelationShip(0x01, 0x01, HIBYTE(CharactersClient[SelectedCharacter].Key), LOBYTE(CharactersClient[SelectedCharacter].Key));
                        SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Alliance, GuildRequestType::Join, CharactersClient[SelectedCharacter].Key);
                    }
                    else if (!wcscmp(Text, I18N::Game::Hostilities) || !wcsicmp(Text, L"/rival"))
                    {
                        //SendRequestGuildRelationShip(0x02, 0x01, HIBYTE(CharactersClient[SelectedCharacter].Key), LOBYTE(CharactersClient[SelectedCharacter].Key));
                        SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Hostility, GuildRequestType::Join, CharactersClient[SelectedCharacter].Key);
                    }
                    else
                    {
                        SetAction(&Hero->Object, PLAYER_RESPECT1);
                        SendRequestAction(Hero->Object, AT_RESPECT1);
                        //SendRequestGuildRelationShip(0x02, 0x02, HIBYTE(CharactersClient[SelectedCharacter].Key), LOBYTE(CharactersClient[SelectedCharacter].Key));
                        SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Hostility, GuildRequestType::Leave, CharactersClient[SelectedCharacter].Key);
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
                        if (!wcscmp(Text, I18N::Game::Alliance1354) || !wcsicmp(Text, L"/union"))
                        {
                            //SendRequestGuildRelationShip(0x01, 0x01, HIBYTE(c->Key), LOBYTE(c->Key));
                            SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Alliance, GuildRequestType::Join, c->Key);
                        }
                        else if (!wcscmp(Text, I18N::Game::Hostilities) || !wcsicmp(Text, L"/rival"))
                        {
                            //SendRequestGuildRelationShip(0x02, 0x01, HIBYTE(c->Key), LOBYTE(c->Key));
                            SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Hostility, GuildRequestType::Join, c->Key);
                        }
                        else
                        {
                            SetAction(&Hero->Object, PLAYER_RESPECT1);
                            SendRequestAction(Hero->Object, AT_RESPECT1);
                            //SendRequestGuildRelationShip(0x02, 0x02, HIBYTE(c->Key), LOBYTE(c->Key));
                            SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Hostility, GuildRequestType::Leave, c->Key);
                        }
                        break;
                    }
                }
            }
            return true;
        }
        if (wcscmp(Text, I18N::Game::Party256) == 0 || wcsicmp(Text, L"/party") == 0 || wcsicmp(Text, L"/pt") == 0)
        {
            if (gMapManager.InChaosCastle() == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::CanTBeInChaosCastle, SEASON3B::TYPE_SYSTEM_MESSAGE);
                return false;
            }
            if (PartyNumber > 0 && wcscmp(Party[0].Name, Hero->ID) != 0)
            {
                g_pSystemLogBox->AddText(I18N::Game::YouAreAlreadyInAParty, SEASON3B::TYPE_SYSTEM_MESSAGE);
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
                    mu_swprintf(Text, I18N::Game::YouHaveRequestedSToJoinYourParty, c->ID);
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
                        mu_swprintf(Text, I18N::Game::YouHaveRequestedSToJoinYourParty, c->ID);
                        g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
                        break;
                    }
                }
            }
            return true;
        }
        if (wcsicmp(Text, L"/charactername") == 0)
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
                mu_swprintf(Name, L"/%d", i + 1);
            else
                mu_swprintf(Name, L"/%d", 0);
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
                MacroText[i][iTextSize - 3] = 0;
                PlayBuffer(SOUND_CLICK01);
                return true;
            }
        }

        wchar_t lpszFilter[] = L"/filter";
        if ((wcslen(I18N::Game::Filter) > 0 && wcsncmp(Text, I18N::Game::Filter, wcslen(I18N::Game::Filter)) == 0)
            || (wcsncmp(Text, lpszFilter, wcslen(lpszFilter)) == 0))
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

        if (wcscmp(Name, I18N::Game::Warp) == 0 || wcsicmp(Name, L"/move") == 0)
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
            mu_swprintf(Name, L"/%ls", p->Name);

            if (wcsicmp(Text, Name) == 0)
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
    if (FindText(Text, I18N::Game::Hello) || FindText(Text, I18N::Game::Hi) || FindText(Text, I18N::Game::Welcome) || FindText(Text, I18N::Game::Welcome) || FindText(Text, I18N::Game::Thanks) || FindText(Text, I18N::Game::Thanks) || FindText(Text, I18N::Game::Thanks) || FindText(Text, I18N::Game::Thanks))
    {
        SetActionClass(c, o, PLAYER_GREETING1, AT_GREETING1);
        SendRequestAction(Hero->Object, AT_GREETING1);
    }
    else if (FindText(Text, I18N::Game::EnjoyTheGame) || FindText(Text, I18N::Game::Bye) || FindText(Text, I18N::Game::Bye280))
    {
        SetActionClass(c, o, PLAYER_GOODBYE1, AT_GOODBYE1);
        SendRequestAction(Hero->Object, AT_GOODBYE1);
    }
    else if (FindText(Text, I18N::Game::Good) || FindText(Text, I18N::Game::Good) || FindText(Text, I18N::Game::Wow) || FindText(Text, I18N::Game::Wow) || FindText(Text, I18N::Game::Nice) || FindText(Text, I18N::Game::Nice))
    {
        SetActionClass(c, o, PLAYER_CLAP1, AT_CLAP1);
        SendRequestAction(Hero->Object, AT_CLAP1);
    }
    else if (FindText(Text, I18N::Game::Here) || FindText(Text, I18N::Game::Here) || FindText(Text, I18N::Game::Come) || FindText(Text, I18N::Game::Come))
    {
        SetActionClass(c, o, PLAYER_GESTURE1, AT_GESTURE1);
        SendRequestAction(Hero->Object, AT_GESTURE1);
    }
    else if (FindText(Text, I18N::Game::There) || FindText(Text, I18N::Game::There) || FindText(Text, I18N::Game::That) || FindText(Text, I18N::Game::That))
    {
        SetActionClass(c, o, PLAYER_DIRECTION1, AT_DIRECTION1);
        SendRequestAction(Hero->Object, AT_DIRECTION1);
    }
    else if (FindText(Text, I18N::Game::Not) || FindText(Text, I18N::Game::Not) || FindText(Text, I18N::Game::Never) || FindText(Text, I18N::Game::Never) || FindText(Text, I18N::Game::DoNot) || FindText(Text, I18N::Game::DoNot) || FindText(Text, I18N::Game::DoNot302))
    {
        SetActionClass(c, o, PLAYER_UNKNOWN1, AT_UNKNOWN1);
        SendRequestAction(Hero->Object, AT_UNKNOWN1);
    }
    else if (FindText(Text, L";") || FindText(Text, I18N::Game::Sorry) || FindText(Text, I18N::Game::Sorry) || FindText(Text, I18N::Game::Sorry))
    {
        SetActionClass(c, o, PLAYER_AWKWARD1, AT_AWKWARD1);
        SendRequestAction(Hero->Object, AT_AWKWARD1);
    }
    else if (FindText(Text, L"ㅠ.ㅠ") || FindText(Text, L"ㅜ.ㅜ") || FindText(Text, L"T_T") || FindText(Text, I18N::Game::Sad) || FindText(Text, I18N::Game::Sad) || FindText(Text, I18N::Game::Cry) || FindText(Text, I18N::Game::Cry))
    {
        SetActionClass(c, o, PLAYER_CRY1, AT_CRY1);
        SendRequestAction(Hero->Object, AT_CRY1);
    }
    else if (FindText(Text, L"ㅡ.ㅡ") || FindText(Text, L"ㅡ.,ㅡ") || FindText(Text, L"ㅡ,.ㅡ") || FindText(Text, L"-.-") || FindText(Text, L"-_-") || FindText(Text, I18N::Game::Huh) || FindText(Text, I18N::Game::Pooh))
    {
        SetActionClass(c, o, PLAYER_SEE1, AT_SEE1);
        SendRequestAction(Hero->Object, AT_SEE1);
    }
    else if (FindText(Text, L"^^") || FindText(Text, L"^.^") || FindText(Text, L"^_^") || FindText(Text, I18N::Game::Haha) || FindText(Text, I18N::Game::Hehe) || FindText(Text, I18N::Game::Hoho) || FindText(Text, I18N::Game::Hoho) || FindText(Text, I18N::Game::Hihi))
    {
        SetActionClass(c, o, PLAYER_SMILE1, AT_SMILE1);
        SendRequestAction(Hero->Object, AT_SMILE1);
    }
    else if (FindText(Text, I18N::Game::Great) || FindText(Text, I18N::Game::OhYeah) || FindText(Text, I18N::Game::OhYeah320) || FindText(Text, I18N::Game::BeatIt))
    {
        SetActionClass(c, o, PLAYER_CHEER1, AT_CHEER1);
        SendRequestAction(Hero->Object, AT_CHEER1);
    }
    else if (FindText(Text, I18N::Game::Win) || FindText(Text, I18N::Game::Win) || FindText(Text, I18N::Game::Victory) || FindText(Text, I18N::Game::Victory))
    {
        SetActionClass(c, o, PLAYER_WIN1, AT_WIN1);
        SendRequestAction(Hero->Object, AT_WIN1);
    }
    else if (FindText(Text, I18N::Game::Sleep) || FindText(Text, I18N::Game::Sleep) || FindText(Text, I18N::Game::Tired) || FindText(Text, I18N::Game::Tired))
    {
        SetActionClass(c, o, PLAYER_SLEEP1, AT_SLEEP1);
        SendRequestAction(Hero->Object, AT_SLEEP1);
    }
    else if (FindText(Text, I18N::Game::Cold) || FindText(Text, I18N::Game::Cold) || FindText(Text, I18N::Game::Hurt) || FindText(Text, I18N::Game::Hurt) || FindText(Text, I18N::Game::Hurt))
    {
        SetActionClass(c, o, PLAYER_COLD1, AT_COLD1);
        SendRequestAction(Hero->Object, AT_COLD1);
    }
    else if (FindText(Text, I18N::Game::Again) || FindText(Text, I18N::Game::Again) || FindText(Text, I18N::Game::OK) || FindText(Text, I18N::Game::Great))
    {
        SetActionClass(c, o, PLAYER_AGAIN1, AT_AGAIN1);
        SendRequestAction(Hero->Object, AT_AGAIN1);
    }
    else if (FindText(Text, I18N::Game::Respect) || FindText(Text, I18N::Game::Respect) || FindText(Text, I18N::Game::Defeated))
    {
        SetActionClass(c, o, PLAYER_RESPECT1, AT_RESPECT1);
        SendRequestAction(Hero->Object, AT_RESPECT1);
    }
    else if (FindText(Text, I18N::Game::Sir) || FindText(Text, I18N::Game::Sir) || FindText(Text, L"/ㅡ") || FindText(Text, L"ㅡ^"))
    {
        SetActionClass(c, o, PLAYER_SALUTE1, AT_SALUTE1);
        SendRequestAction(Hero->Object, AT_SALUTE1);
    }
    else if (FindText(Text, I18N::Game::Rush) || FindText(Text, I18N::Game::Rush) || FindText(Text, I18N::Game::GoGo) || FindText(Text, I18N::Game::GoGo))
    {
        SetActionClass(c, o, PLAYER_RUSH1, AT_RUSH1);
        SendRequestAction(Hero->Object, AT_RUSH1);
    }
    else if (FindText(Text, I18N::Game::Hustle) || FindText(Text, L"hustle"))
    {
        SetActionClass(c, o, PLAYER_HUSTLE, AT_HUSTLE);
        SendRequestAction(Hero->Object, AT_HUSTLE);
    }
    else if (FindText(Text, I18N::Game::ComeOn))
    {
        SetActionClass(c, o, PLAYER_PROVOCATION, AT_PROVOCATION);
        SendRequestAction(Hero->Object, AT_PROVOCATION);
    }
    else if (FindText(Text, I18N::Game::Great))
    {
        SetActionClass(c, o, PLAYER_CHEERS, AT_CHEERS);
        SendRequestAction(Hero->Object, AT_CHEERS);
    }
    else if (FindText(Text, I18N::Game::LookAround))
    {
        SetActionClass(c, o, PLAYER_LOOK_AROUND, AT_LOOK_AROUND);
        SendRequestAction(Hero->Object, AT_LOOK_AROUND);
    }
    else if (FindText(Text, I18N::Game::TheForehead))
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
    else if (FindText(Text, I18N::Game::Christmas))
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
        if (SelectFlag)
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

    GameLogic::Combat::ExecuteSkill(c, Skill, Distance);
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
                            g_pSystemLogBox->AddText(I18N::Game::YouCannotGoToAtlansWhileRidingAUnicorn, SEASON3B::TYPE_ERROR_MESSAGE);
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
                            g_pSystemLogBox->AddText(I18N::Game::YouCanEnterIcarusOnlyWithWingsDinorantFenrirr, SEASON3B::TYPE_ERROR_MESSAGE);

                            if (CharacterAttribute->Level < Level)
                            {
                                wchar_t Text[100];
                                mu_swprintf(Text, I18N::Game::OnlyCharactersOverLevelDCanEnter, Level);
                                g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_ERROR_MESSAGE);
                            }
                        }

                        else if ((62 <= i && i <= 65) && (CharacterMachine->Equipment[EQUIPMENT_HELPER].Type == ITEM_HORN_OF_UNIRIA))
                        {
                            g_pSystemLogBox->AddText(I18N::Game::YouCannotWarpWhileRidingOnAUnicorn, SEASON3B::TYPE_ERROR_MESSAGE);
                        }
                        else if (CharacterAttribute->Level < Level)
                        {
                            LoadingWorld = 50;
                            wchar_t Text[100];
                            mu_swprintf(Text, I18N::Game::OnlyCharactersOverLevelDCanEnter, Level);
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MoveEffect(OBJECT* o, int iIndex);

namespace
{
    // While the hero slides to a server-set position (the basic weapon skills reposition the
    // hero on every cast), MoveHero would freeze all input until the slide ended, which capped
    // those skills' auto-attack cadence below the player's attack speed (issue #350). Keep the
    // auto-attack re-cast running through the slide; the slide still animates smoothly and only
    // manual move/click input stays suppressed. Returns true when the hero is mid-slide, in
    // which case MoveHero should stop after this.
    bool HandleHeroPositionSlide(CHARACTER* c)
    {
        if (c->JumpTime <= 0)
        {
            return false;
        }

        if (g_pOption->IsAutoAttack() && Attacking != -1 && SelectedCharacter != -1)
        {
            Attack(Hero);
        }
        return true;
    }
}

void MoveHero()
{
    CHARACTER* c = Hero;
    OBJECT* o = &c->Object;

    // Re-arm world clicks once the button is released; the latch only suppresses the held click.
    // Kept above the early returns below (stun/sleep/dead/loading) so a release during any of
    // those states still clears the latch and the next click is honoured.
    if (!MouseLButton)
        s_bIgnoreHeldClickAfterNpcTalk = false;

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

    if (HandleHeroPositionSlide(c))
    {
        return;
    }

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
        // The screen-space angle from hero to cursor needs to be rotated into world space
        // by the active camera's yaw. The legacy hardcoded -45 was DefaultCamera's default
        // yaw value (DefaultCamera.cpp:260), which silently broke for OrbitalCamera and
        // for any rotated DefaultCamera state.
        Angle = (int)(Hero->Object.Angle[2] + CreateAngle((float)HeroX, (float)HeroY, (float)MouseX, (float)MouseY) + g_Camera.Angle[2]) + 360;
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

        if (mousePosY > REFERENCE_HEIGHT)
        {
            mousePosY = REFERENCE_HEIGHT;
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
            // CreateAngle args are swapped vs the head-aim case above and the result is
            // negated, so the camera-yaw correction enters with the same magnitude but
            // appears with opposite sign in the sum (i.e. -45 became +45 for DefaultCamera).
            HeroAngle = -(int)(CreateAngle((float)MouseX, (float)MouseY, (float)HeroX, (float)HeroY) + g_Camera.Angle[2]) + 360;
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

        GameLogic::Combat::UseSkillRagePosition(c);
    }

    CheckGate();

    if (!MouseOnWindow && false == g_pNewUISystem->CheckMouseUse())
    {
        bool Success = false;
        if (MouseUpdateTime >= MouseUpdateTimeMax && !s_bIgnoreHeldClickAfterNpcTalk)
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
                !Engine::Object::IsAttackAction(o->CurrentAction)
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
                // Talking to an NPC opens a window (dialogue/quest/shop). The physical button is
                // usually still held at this point, and the held button keeps re-entering this
                // handler every frame. The moment the cursor isn't on the NPC's pick-box it would
                // fall through to the ground-move branch below and walk the hero away, instantly
                // closing the window the same click just opened. Most visible on adjacent NPCs such
                // as the Elf Soldier, whose narrow dialogue doesn't cover the cursor the way a wide
                // shop window does. Latch the held click so it's ignored until released; a fresh
                // press still moves the hero normally. Also restart the debounce like the sibling
                // branches (attack/ground-move) do.
                s_bIgnoreHeldClickAfterNpcTalk = true;
                MouseUpdateTime = 0;

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

                if (SelectFlag && c->Object.Live)
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
        //    SendChat(I18N::Game::PwnedByTheFilter);
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
            mu_swprintf(buff, L"%d %ls %d %ls %d %ls %d %ls", day, I18N::Game::Day, oClock, I18N::Game::Hour, minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buff;
        }
        else if (day == 0 && oClock != 0)
        {
            mu_swprintf(buff, L"%d %ls %d %ls %d %ls", oClock, I18N::Game::Hour, minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buff;
        }
        else if (day == 0 && oClock == 0 && minutes != 0)
        {
            mu_swprintf(buff, L"%d %ls %d %ls", minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buff;
        }
        else if (day == 0 && oClock == 0 && minutes == 0)
        {
            timeText = I18N::Game::LessThan1Minutes;
        }
    }
    else
    {
        DWORD day = time / 1440;
        DWORD oClock = (time - (day * 1440)) / 60;
        DWORD minutes = time % 60;

        if (day != 0)
        {
            mu_swprintf(buff, L"%d %ls %d %ls %d %ls", day, I18N::Game::Day, oClock, I18N::Game::Hour, minutes, I18N::Game::Minute);
            timeText = buff;
        }
        else if (day == 0 && oClock != 0)
        {
            mu_swprintf(buff, L"%d %ls %d %ls", oClock, I18N::Game::Hour, minutes, I18N::Game::Minute);
            timeText = buff;
        }
        else if (day == 0 && oClock == 0 && minutes != 0)
        {
            mu_swprintf(buff, L"%d %ls", minutes, I18N::Game::Minute);
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
        if (x + Width + 4 > REFERENCE_WIDTH) x = REFERENCE_WIDTH - (Width + 1 + 4);
        if (y + Height + 4 > REFERENCE_HEIGHT - 47) y = REFERENCE_HEIGHT - 47 - (Height + 1 + 4);
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
            mu_swprintf(Buff, L"%ls%d / %ls / %ls", I18N::Game::CrownSwitch, i + 1, Switch_Info[i].m_szGuildName, Switch_Info[i].m_szUserName);
            g_pRenderText->SetFont(g_hFont);
            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->SetBgColor(0);
            g_pRenderText->RenderText(0, REFERENCE_HEIGHT - 85 + (i * 15), Buff, REFERENCE_WIDTH, 0, RT3_SORT_CENTER);
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
        RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, (float)REFERENCE_WIDTH, (float)REFERENCE_HEIGHT - 45.f, WindX, 0.f, 0.3f, 0.3f);
        float WindX2 = (float)((int)WorldTime % 100000) * 0.001f;
        RenderBitmapUV(BITMAP_CHROME + 3, 0.f, 0.f, (float)REFERENCE_WIDTH, (float)REFERENCE_HEIGHT - 45.f, WindX2, 0.f, 3.f, 2.f);
    }
#ifdef ASG_ADD_MAP_KARUTAN
    else if (IsKarutanMap())
    {
        EnableAlphaTest();
        EnableAlphaBlend();
        glColor3f(0.3f, 0.3f, 0.25f);
        float fWindX = (float)((int)WorldTime % 100000) * 0.004f;
        RenderBitmapUV(BITMAP_CHROME + 3, 0.f, 0.f, (float)REFERENCE_WIDTH, (float)REFERENCE_HEIGHT - 45.f, fWindX, 0.f, 3.f, 2.f);
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
    int WindowX = (REFERENCE_WIDTH - Width) / 2;
    int WindowY = (REFERENCE_HEIGHT - Height) / 2 + 50;
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
    int WindowX = (REFERENCE_WIDTH - Width) / 2;
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
                mu_swprintf(t_Str, I18N::Game::ItWillStartAfterDSeconds, t_valueSec);
            }
            else
            {
                if (g_wtMatchTimeLeft.m_Time < 60)
                {
                    g_pRenderText->SetTextColor(255, 255, 10, 255);
                }
                if (t_valueSec < 10)
                {
                    mu_swprintf(t_Str, I18N::Game::RemainingHoursD0D, t_valueMin, t_valueSec);
                }
                else
                {
                    mu_swprintf(t_Str, I18N::Game::RemainingSecondsDD, t_valueMin, t_valueSec);
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

    Width = 300; Height = 2 * 5 + 5 * 40; WindowX = (REFERENCE_WIDTH - Width) / 2; WindowY = 120 + 0;
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
    mu_swprintf(t_Str, I18N::Game::TournamentResult);
    g_pRenderText->RenderText(WindowX + Width / 2 - 50, WindowY + 20, t_Str);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    mu_swprintf(t_Str, I18N::Game::VS);
    g_pRenderText->SetTextColor(255, 255, 10, 255);
    g_pRenderText->RenderText(WindowX + Width / 2 - 13, WindowY + 50, t_Str);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    float t_temp = 0.0f;
    mu_swprintf(t_Str, L"%ls", g_wtMatchResult.m_MatchTeamName1);
    t_temp = (MAX_USERNAME_SIZE - wcslen(t_Str)) * 5;
    g_pRenderText->RenderText(WindowX + 10 + t_temp, WindowY + 50, t_Str);
    mu_swprintf(t_Str, L"%ls", g_wtMatchResult.m_MatchTeamName2);
    t_temp = (MAX_USERNAME_SIZE - wcslen(t_Str)) * 5;
    g_pRenderText->RenderText(WindowX + Width - 120 + t_temp, WindowY + 50, t_Str);

    mu_swprintf(t_Str, L"(%d)", g_wtMatchResult.m_Score1);
    g_pRenderText->RenderText(WindowX + 45, WindowY + 75, t_Str);
    mu_swprintf(t_Str, L"(%d)", g_wtMatchResult.m_Score2);
    g_pRenderText->RenderText(WindowX + Width - 85, WindowY + 75, t_Str);

    if (g_wtMatchResult.m_Score1 == g_wtMatchResult.m_Score2)
    {
        g_pRenderText->SetFont(g_hFontBig);
        g_pRenderText->SetTextColor(255, 255, 10, 255);
        mu_swprintf(t_Str, I18N::Game::Tie);
        g_pRenderText->RenderText(WindowX + Width / 2 - 35, WindowY + 115, t_Str);
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
    }
    else if (g_wtMatchResult.m_Score1 > g_wtMatchResult.m_Score2)
    {
        g_pRenderText->SetFont(g_hFontBig);
        g_pRenderText->SetTextColor(255, 255, 10, 10);
        mu_swprintf(t_Str, I18N::Game::Win);
        g_pRenderText->RenderText(WindowX + 47, WindowY + 115, t_Str);
        g_pRenderText->SetTextColor(255, 10, 10, 255);
        mu_swprintf(t_Str, I18N::Game::Lose);
        g_pRenderText->RenderText(WindowX + Width - 82, WindowY + 115, t_Str);
        g_pRenderText->SetFont(g_hFont);
    }
    else
    {
        g_pRenderText->SetFont(g_hFontBig);
        g_pRenderText->SetTextColor(255, 255, 10, 10);
        mu_swprintf(t_Str, I18N::Game::Lose);
        g_pRenderText->RenderText(WindowX + 47, WindowY + 115, t_Str);
        g_pRenderText->SetTextColor(255, 10, 10, 255);
        mu_swprintf(t_Str, I18N::Game::Win);
        g_pRenderText->RenderText(WindowX + Width - 82, WindowY + 115, t_Str);
        g_pRenderText->SetFont(g_hFont);
    }
    g_pRenderText->SetFont(g_hFont);

    Width = 70; Height = 20; x = (REFERENCE_WIDTH - Width) / 2; y = (REFERENCE_HEIGHT - Height) / 2 + 50;
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
        CameraProjection::WorldToScreen(g_Camera, Position, &ScreenX, &ScreenY);
        ScreenX -= (int)(Width / 2);

        if ((MouseX >= ScreenX && MouseX < ScreenX + Width && MouseY >= ScreenY - 2 && MouseY < ScreenY + 6))
        {
            mu_swprintf(Text, L"HP : %d0%%", p->stepHP);
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

        int stepHP = std::min<int>(10, p->stepHP);

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


void RenderTimes()
{
    const uint64_t currentTickCount = GetTickCount64();
    if (LastMacroTime > currentTickCount - MacroCooldownMs)
    {
        constexpr float width = 50;
        constexpr float height = 2;
        constexpr int y = REFERENCE_HEIGHT - 48 - 40;
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
    if (!g_bRenderGameCursor)
        return;

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
        && g_pMyInventory->GetRepairMode() == SEASON3B::REPAIR_MODE_ON)
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
        int sx = REFERENCE_WIDTH - 30;
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
            g_pRenderText->RenderText(REFERENCE_WIDTH - 100, sy, L"Background");
        else
            g_pRenderText->RenderText(REFERENCE_WIDTH - 100, sy, L"Layer1");
        mu_swprintf(Text, L"Brush Size: %d", BrushSize * 2 + 1);
        g_pRenderText->RenderText(REFERENCE_WIDTH - 100, sy + 11, Text);
    }
    glColor3f(1.f, 1.f, 1.f);
    if (EditFlag == EDIT_OBJECT)
    {
        g_pRenderText->RenderText(REFERENCE_WIDTH - 100, 0, L"Garbage");
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

            mu_swprintf(Text, L"%2d: %ls", MonsterScript[i].Type, MonsterScript[i].Name);
            g_pRenderText->RenderText(REFERENCE_WIDTH - 100, i * 10, Text);
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

            g_pRenderText->RenderText(REFERENCE_WIDTH - 64, i * 10, ColorTable[i]);
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
        (wcslen(I18N::Game::Warp) > 0 && wcsstr(szChatTextUpperChars, I18N::Game::Warp) != NULL))
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
            if (wcsicmp((*li)->_ReqInfo.szMainMapName, I18N::Game::Atlans) == 0)
            {
                bMoveAtlans = true;
            }
            else if (wcsicmp((*li)->_ReqInfo.szMainMapName, I18N::Game::Icarus) == 0)
            {
                bMoveIcarus = true;
            }
        }
    }

    if (bCantSwim && bMoveAtlans)
    {
        g_pSystemLogBox->AddText(I18N::Game::YouCannotGoToAtlansWhileRidingAUnicorn, SEASON3B::TYPE_SYSTEM_MESSAGE);
        return true;
    }

    if ((bCantFly || bEquipChangeRing) && bMoveIcarus)
    {
        g_pSystemLogBox->AddText(I18N::Game::YouCanEnterIcarusOnlyWithWingsDinorantFenrirr, SEASON3B::TYPE_SYSTEM_MESSAGE);
        return true;
    }

    return false;
}
