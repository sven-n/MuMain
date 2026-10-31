///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/Core/UIManager.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ItemTooltipText.h"
#include "Engine/Object/ItemTooltipValues.h"
#include "Engine/AI/ZzzAI.h"
#include "Render/Effects/ZzzEffect.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "Scenes/SceneCore.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "GameLogic/Quests/CSQuest.h"
#include "App/Platform/Windows/Local.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "Engine/AI/GOBoid.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "GameLogic/NPCs/npcBreeder.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "Character/CSParts.h"
#include "GameLogic/NPCs/npcGateSwitch.h"
#include "GameLogic/Items/CComGem.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "GameLogic/Items/ItemCategories.h"
#include "GameLogic/Items/ItemStats.h"
#include "GameLogic/Items/TransformationRing.h"
#include "Render/Items/ItemDisplay.h"
#include <chrono>
#include <cstring>
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "GameLogic/Items/ItemNameParts.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Core/Text/TextLineWrap.h"
#include "Core/Text/Utf8.h"
#include "UI/Scaling/UITransform.h"
#include "GameLogic/Items/ShopRestrictions.h"
#include "GameLogic/Items/TradeRestrictions.h"
#include "GameLogic/Items/MixMgr.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Tooltip/LegacyTextListTooltip.h"
#include "UI/Scaling/UITransform.h"
#include "UI/Inventory/MyShopInventory.h" // ShowPersonalShopItemValueDialog
#include "GameLogic/Events/w_CursedTemple.h"
#include "Network/Server/SocketSystem.h"
#include "World/MapInfra/PortalMgr.h"
#include "UI/Core/WindowSystem.h"
#include "Network/Server/ServerListManager.h"
#include <algorithm>
#include <time.h>
#include <unordered_set>
#include <vector>

#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Camera/CameraProjection.h"
#include "Render/Text/CUIRenderText.h"

namespace TooltipText = Engine::Items::TooltipText;

extern int g_iChatInputType;

int			g_nTaxRate = 0;
int			g_nChaosTaxRate = 0;

wchar_t      g_GuildNotice[3][128];
GUILD_LIST_t GuildList[MAX_GUILDS];
int			 g_nGuildMemberCount = 0;
int			 GuildTotalScore = 0;
int			 GuildPlayerKey = 0;
int			 SelectMarkColor = 0;
MARK_t		 GuildMark[MAX_MARKS];

PARTY_t Party[MAX_PARTYS];
int     PartyNumber = 0;
int     PartyKey = 0;

ITEM_t Items[MAX_ITEMS];
ITEM   PickItem;
ITEM   TargetItem;

ITEM Inventory[MAX_INVENTORY];
ITEM InventoryExt[MAX_INVENTORY_EXT];
ITEM ShopInventory[MAX_SHOP_INVENTORY];
ITEM g_PersonalShopInven[MAX_PERSONALSHOP_INVEN];
ITEM g_PersonalShopBackup[MAX_PERSONALSHOP_INVEN];
bool g_bEnablePersonalShop = false;
int g_iPShopWndType = PSHOPWNDTYPE_NONE;
int g_iPersonalShopMsgType = 0;
wchar_t g_szPersonalShopTitle[MAX_SHOPTITLE + 1] = { 0, };

CHARACTER g_PersonalShopSeller;
bool g_bIsTooltipOn = false;

int   CheckSkill = -1;
ITEM* CheckInventory = NULL;

bool  EquipmentSuccess = false;
bool  CheckShop;
int   CheckX;
int   CheckY;
ITEM* SrcInventory;
int   SrcInventoryIndex;
int   DstInventoryIndex;

int  AllRepairGold = 0;
int  StorageGoldFlag = 0;

int ListCount = 0;
int GuildListPage = 0;

int  g_bEventChipDialogEnable = EVENT_NONE;
int  g_shEventChipCount = 0;
short g_shMutoNumber[3] = { -1, -1, -1 };

bool g_bServerDivisionAccept = false;
wchar_t g_strGiftName[64];

bool RepairShop = false;
int  RepairEnable = 0;
int AskYesOrNo = 0;

char OkYesOrNo = -1;

int g_iKeyPadEnable = 0;
WORD g_wStoragePassword = 0;
short	g_nKeyPadMapping[10];
wchar_t	g_lpszKeyPadInput[2][MAX_KEYPADINPUT + 1];

static  const int DEFAULT_DEVILSQUARELEVEL[6][2] = { {15, 130}, { 131, 180}, { 181, 230}, {231, 280}, { 281, 330}, { 331, 99999} };
static  const int DARKCLASS_DEVILSQUARELEVEL[6][2] = { {15, 110}, { 111, 160}, { 161, 210}, {211, 260}, { 261, 310}, { 311, 99999} };
int g_iDevilSquareLevel[6][2];

static  const int g_iChaosCastleLevel[12][2] = {
                        { 15, 49 }, { 50, 119 }, { 120, 179 }, { 180, 239 }, { 240, 299 }, { 300, 999 },
                        { 15, 29 }, { 30,  99 }, { 100, 159 }, { 160, 219 }, { 220, 279 }, { 280, 999 }
};
static  const int g_iChaosCastleZen[6] = { 25, 80, 150, 250, 400, 650 };

BYTE BuyItem[4];

static  int iStateNum = 4;

	// ※

#ifdef _PVP_ADD_MOVE_SCROLL
extern CMurdererMove g_MurdererMove;
#endif	// _PVP_ADD_MOVE_SCROLL

int getLevelGeneration(int level, unsigned int* color)
{
    int lvl;
    if (level >= 300)
    {
        lvl = 300;
        *color = (255u << 24) + (255 << 16) + (153 << 8) + (255);
    }
    else if (level >= 200)
    {
        lvl = 200;
        *color = (255u << 24) + (255 << 16) + (230 << 8) + (210);
    }
    else if (level >= 100)
    {
        lvl = 100;
        *color = (255u << 24) + (24 << 16) + (201 << 8) + (0);
    }
    else if (level >= 50)
    {
        lvl = 50;
        *color = (255u << 24) + (0 << 16) + (150 << 8) + (255);
    }
    else
    {
        lvl = 10;
        *color = (255u << 24) + (0 << 16) + (0 << 8) + (255);
    }
    return lvl;
}

wchar_t TextList[MaxTextListRows][100];
int TextListColor[MaxTextListRows];
int TextBold[MaxTextListRows];
SIZE Size[MaxTextListRows];

int RenderTextList(int sx, int sy, int TextNum, int Tab, int iSort = RT3_SORT_CENTER)
{
    int TextWidth = 0;
    float fsy = sy;
    for (int i = 0; i < TextNum; i++)
    {
        if (TextBold[i])
        {
            g_pRenderText->SetFont(g_hFontBold);
        }
        else
        {
            g_pRenderText->SetFont(g_hFont);
        }

        Size[i] = g_pRenderText->MeasureText(TextList[i], lstrlen(TextList[i]));

        if (TextWidth < Size[i].cx)
        {
            TextWidth = Size[i].cx;
        }
    }

    if (Tab == 0)
    {
        sx -= (TextWidth + Tab) / 2;
    }

    for (int i = 0; i < TextNum; i++)
    {
        switch (TextListColor[i])
        {
        case TEXT_COLOR_BLUE: g_pRenderText->SetTextColor(128, 179, 255, 255); break;
        case TEXT_COLOR_GRAY: g_pRenderText->SetTextColor(102, 102, 102, 255); break;
        case TEXT_COLOR_RED: g_pRenderText->SetTextColor(255, 51, 26, 255); break;
        case TEXT_COLOR_YELLOW: g_pRenderText->SetTextColor(255, 204, 26, 255); break;
        case TEXT_COLOR_GREEN: g_pRenderText->SetTextColor(26, 255, 128, 255); break;
        case TEXT_COLOR_PURPLE: g_pRenderText->SetTextColor(255, 26, 255, 255); break;
        default: g_pRenderText->SetTextColor(255, 255, 255, 255); break;
        }

        if (TEXT_COLOR_DARKRED == TextListColor[i])
        {
            g_pRenderText->SetBgColor(160, 0, 0, 255);
        }
        else if (TEXT_COLOR_DARKBLUE == TextListColor[i])
        {
            g_pRenderText->SetBgColor(0, 0, 160, 255);
        }
        else if (TEXT_COLOR_DARKYELLOW == TextListColor[i])
        {
            g_pRenderText->SetBgColor(160, 102, 0, 255);
        }
        else if (TEXT_COLOR_GREEN_BLUE == TextListColor[i])
        {
            g_pRenderText->SetTextColor(0, 255, 0, 255);
            g_pRenderText->SetBgColor(60, 60, 200, 255);
        }
        else
        {
            g_pRenderText->SetBgColor(0, 0, 0, 0);
        }

        if (TextBold[i])
        {
            g_pRenderText->SetFont(g_hFontBold);
        }
        else
        {
            g_pRenderText->SetFont(g_hFont);
        }
        SIZE TextSize;
        g_pRenderText->RenderText(sx, fsy, TextList[i], TextWidth + Tab, 0, iSort, &TextSize);
        fsy += TextSize.cy;
    }
    return TextWidth + Tab;
}

void RenderTipTextList(const int sx, const int sy, int TextNum, int Tab, int iSort, int iRenderPoint, BOOL bUseBG)
{
    SIZE TextSize = { 0, 0 };
    float fWidth = 0; float fHeight = 0;
    for (int i = 0; i < TextNum; ++i)
    {
        if (TextList[i][0] == '\0')
        {
            TextNum = i;
            break;
        }

        if (TextBold[i])
        {
            g_pRenderText->SetFont(g_hFontBold);
        }
        else
        {
            g_pRenderText->SetFont(g_hFont);
        }

        TextSize = g_pRenderText->MeasureText(TextList[i], lstrlen(TextList[i]));

        if (fWidth < TextSize.cx)
            fWidth = TextSize.cx;

        const bool halfLine = TextList[i][0] == '\n';
        const bool spacer = halfLine || (TextList[i][0] == L' ' && TextList[i][1] == L'\0');
        const SIZE lineSize = spacer ? g_pRenderText->MeasureText(L"Q", 1) : TextSize;
        fHeight += static_cast<float>(lineSize.cy) * (halfLine ? 0.55f : 1.1f);
    }

    EnableAlphaTest();
    if (Tab > 0)
        fWidth = Tab * 2;
    fWidth += 4;
    int iPos_x = sx - fWidth / 2;
    if (iPos_x < 0) iPos_x = 0;
    if (iPos_x + fWidth > REFERENCE_WIDTH)
    {
        iPos_x = REFERENCE_WIDTH - fWidth - 1;
    }

    float fsx = iPos_x + 1;
    float fsy = 0.f;

    switch (iRenderPoint)
    {
    case STRP_BOTTOMCENTER:
    {
        fsy = sy - fHeight;
    }break;
    default:
    {
        fsy = sy;
    }break;
    }

    if (bUseBG == TRUE && TextNum > 0)
    {
        RenderColor((float)iPos_x - 1, fsy - 1, (float)fWidth + 1, (float)1, 1.0f, 1);
        RenderColor((float)iPos_x - 1, fsy - 1, (float)1, (float)fHeight + 1, 1.0f, 1);
        RenderColor((float)iPos_x - 1 + fWidth + 1, (float)fsy - 1, (float)1, (float)fHeight + 1, 1.0f, 1);
        RenderColor((float)iPos_x - 1, fsy - 1 + fHeight + 1, (float)fWidth + 2, (float)1, 1.0f, 1);

        RenderColor((float)iPos_x, fsy, (float)fWidth, (float)fHeight, 0.8f, 1);
        mu::GetRenderer().SetTexture2D(true);
    }

    for (int i = 0; i < TextNum; i++)
    {
        if (TextBold[i])
        {
            g_pRenderText->SetFont(g_hFontBold);
        }
        else
        {
            g_pRenderText->SetFont(g_hFont);
        }

        float fHeight = 0;
        if (TextList[i][0] == 0x0a || (TextList[i][0] == ' ' && TextList[i][1] == 0x00))
        {
            TextSize = g_pRenderText->MeasureText(L"Q", 1);
            fHeight = static_cast<float>(TextSize.cy) / (TextList[i][0] == 0x0a ? 2.0f : 1.0f);
        }
        else
        {
            switch (TextListColor[i])
            {
            case TEXT_COLOR_BLUE: g_pRenderText->SetTextColor(128, 179, 255, 255); break;
            case TEXT_COLOR_GRAY: g_pRenderText->SetTextColor(102, 102, 102, 255); break;
            case TEXT_COLOR_RED: g_pRenderText->SetTextColor(255, 51, 26, 255); break;
            case TEXT_COLOR_YELLOW: g_pRenderText->SetTextColor(255, 204, 26, 255); break;
            case TEXT_COLOR_GREEN: g_pRenderText->SetTextColor(26, 255, 128, 255); break;
            case TEXT_COLOR_PURPLE: g_pRenderText->SetTextColor(255, 26, 255, 255); break;
            case TEXT_COLOR_REDPURPLE: g_pRenderText->SetTextColor(204, 128, 204, 255); break;
            case TEXT_COLOR_VIOLET: g_pRenderText->SetTextColor(179, 102, 255, 255); break;
            case TEXT_COLOR_ORANGE: g_pRenderText->SetTextColor(230, 107, 10, 255); break;
            default: g_pRenderText->SetTextColor(255, 255, 255, 255); break;
            }
            if (TEXT_COLOR_DARKRED == TextListColor[i])
            {
                g_pRenderText->SetBgColor(160, 0, 0, 255);
            }
            else if (TEXT_COLOR_DARKBLUE == TextListColor[i])
            {
                g_pRenderText->SetBgColor(0, 0, 160, 255);
            }
            else if (TEXT_COLOR_DARKYELLOW == TextListColor[i])
            {
                g_pRenderText->SetBgColor(160, 102, 0, 255);
            }
            else if (TEXT_COLOR_GREEN_BLUE == TextListColor[i])
            {
                g_pRenderText->SetBgColor(60, 60, 200, 255);
                g_pRenderText->SetTextColor(0, 255, 0, 255);
            }
            else
            {
                g_pRenderText->SetBgColor(0);
            }
            SIZE TextSize;
            g_pRenderText->RenderText(fsx, fsy, TextList[i], (fWidth - 2), 0, iSort, &TextSize);
            fHeight = TextSize.cy;
        }
        fsy += fHeight * 1.1f;
    }

    DisableAlphaBlend();
}

void SendRequestUse(int Index, int Target, bool addPoints)
{
    if (!IsCanUseItem())
    {
        g_pSystemLogBox->AddText(I18N::Game::YouCannotUseYourItemsWhileUsingTheVaultOrWhileTrading, mu::ui::window::TYPE_ERROR_MESSAGE);
        return;
    }
    if (EnableUse > 0)
    {
        return;
    }

    EnableUse = 10;
    SocketClient->ToGameServer()->SendConsumeItemRequest(Index, Target, addPoints ? FruitUsage::AddPoints : FruitUsage::RemovePoints);
    g_ConsoleDebug->Write(MCD_SEND, L"0x26 [SendRequestUse(%d)]", Index);
}

bool SendRequestEquipmentItem(STORAGE_TYPE iSrcType, int iSrcIndex, ITEM* pItem, STORAGE_TYPE iDstType, int iDstIndex)
{
    if (EquipmentItem || NULL == pItem) return false;

    EquipmentItem = true;

    BYTE splitType;
    if (pItem->option_380)
    {
        splitType = ((BYTE)(pItem->Type >> 5) & 240) | 0x08;
    }
    else
        splitType = ((BYTE)(pItem->Type >> 5) & 240);

    if (pItem->bPeriodItem == true)
    {
        splitType |= 0x02;
    }

    if (pItem->bExpiredPeriod == true)
    {
        splitType |= 0x04;
    }

    BYTE spareBits;
    if (g_SocketItemMgr.IsSocketItem(pItem))
    {
        spareBits = pItem->SocketSeedSetOption;
    }
    else
    {
        spareBits = (((BYTE)pItem->Jewel_Of_Harmony_Option) << 4) + ((BYTE)pItem->Jewel_Of_Harmony_OptionLevel);
    }

    SocketClient->ToGameServer()->SendItemMoveRequestExtended(static_cast<ItemStorageKind>(iSrcType), iSrcIndex, static_cast<ItemStorageKind>(iDstType), iDstIndex);

    g_ConsoleDebug->Write(MCD_SEND, L"0x24 [SendRequestEquipmentItem(%d %d %d %d %d %d %d)]", iSrcIndex, iDstIndex, iSrcType, iDstType, (pItem->Type & 0x1FFF), (BYTE)(pItem->Level), (BYTE)(pItem->Durability));

    return true;
}

bool IsCanUseItem()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_STORAGE) || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE))
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool IsCanTrade()
{
    if (g_pUIManager->IsOpen(MUTEX_PERSONALSHOPSALE) || g_pUIManager->IsOpen(MUTEX_PERSONALSHOPPURCHASE))
    {
        return false;
    }
    return true;
}

int TextNum = 0;
int SkipNum = 0;

void RequireClass(ITEM_ATTRIBUTE* pItem)
{
    if (pItem == NULL)
        return;

    BYTE byFirstClass = gCharacterManager.GetBaseClass(Hero->Class);
    BYTE byStepClass = gCharacterManager.GetStepClass(Hero->Class);

    int iTextColor = 0;

    TextListColor[TextNum + 2] = TextListColor[TextNum + 3] = TEXT_COLOR_WHITE;
    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    int iCount = 0;
    for (int i = 0; i < MAX_CLASS; ++i)
    {
        if (pItem->RequireClass[i] == 1)
        {
            iCount++;
        }
    }
    if (iCount == MAX_CLASS)
        return;

    for (int i = 0; i < MAX_CLASS; ++i)
    {
        BYTE byRequireClass = pItem->RequireClass[i];

        if (byRequireClass == 0)
            continue;

        if (i == byFirstClass && byRequireClass <= byStepClass)
        {
            iTextColor = TEXT_COLOR_WHITE;
        }
        else
        {
            iTextColor = TEXT_COLOR_DARKRED;
        }

        switch (i)
        {
        case CLASS_WIZARD:
        {
            if (byRequireClass == 1)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::DarkWizard);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 2)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::SoulMaster);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::GrandMaster);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_KNIGHT:
        {
            if (byRequireClass == 1)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::DarkKnight);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 2)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::BladeKnight);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::BladeMaster);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_ELF:
        {
            if (byRequireClass == 1)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::Elf);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 2)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::MuseElf);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::HighElf);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_DARK:
        {
            if (byRequireClass == 1)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::MagicGladiator);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::DualMaster);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_DARK_LORD:
        {
            if (byRequireClass == 1)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::DarkLord);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::LordEmperor);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_SUMMONER:
        {
            if (byRequireClass == 1)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::Summoner);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 2)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::BloodySummoner);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::DimensionMaster);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_RAGEFIGHTER:
        {
            if (byRequireClass == 1)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::RageFighter);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::FistMaster);
                TextListColor[TextNum] = iTextColor;
            }
            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        }
    }
}

int g_iItemInfo[iMaxLevel + 1][_COLUMN_TYPE_COUNT];

namespace
{
// The column the magic power goes into: Curse for summoner books, Attack for sceptres, WIZDmg for
// the other items.
int GetItemInfoMagicColumn(int itemType)
{
    if (GameLogic::Items::IsSummonerBookType(itemType))
    {
        return _COLUMN_TYPE_CURSE;
    }
    if (GameLogic::Items::IsSceptreType(itemType))
    {
        return _COLUMN_TYPE_PET_ATTACK;
    }
    return _COLUMN_TYPE_MAGIC;
}

// Whether the hero reaches every requirement of a row: the window shows the row white, else red.
bool CanEquipItemInfoRow(const int (&row)[_COLUMN_TYPE_COUNT], const ItemInfoHeroStats& hero)
{
    return row[_COLUMN_TYPE_REQSTR] <= hero.strength && row[_COLUMN_TYPE_REQDEX] <= hero.dexterity &&
           row[_COLUMN_TYPE_REQENG] <= hero.energy && row[_COLUMN_TYPE_REQVIT] <= hero.vitality &&
           row[_COLUMN_TYPE_REQCHA] <= hero.charisma && row[_COLUMN_TYPE_REQNLV] <= hero.level;
}
} // namespace

ItemInfoHeroStats GetItemInfoHeroStats()
{
    return {CharacterAttribute->Level,
            static_cast<WORD>(CharacterAttribute->Strength + CharacterAttribute->AddStrength),
            static_cast<WORD>(CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity),
            static_cast<WORD>(CharacterAttribute->Energy + CharacterAttribute->AddEnergy),
            static_cast<WORD>(CharacterAttribute->Vitality + CharacterAttribute->AddVitality),
            static_cast<WORD>(CharacterAttribute->Charisma + CharacterAttribute->AddCharisma)};
}

// Not cached per item: the levels the hero can equip follow the hero's level and stats, and the
// values follow the item editor's changes.
void ComputeItemInfo(int iHelpItem)
{
    ITEM_ATTRIBUTE* p = &ItemAttribute[iHelpItem];
    const int magicColumn = GetItemInfoMagicColumn(iHelpItem);
    const ItemInfoHeroStats hero = GetItemInfoHeroStats();

    for (int Level = 0; Level <= iMaxLevel; Level++)
    {
        // The stats and requirements a normal item of the level has.
        const GameLogic::Items::ItemStatValues stats = GameLogic::Items::ComputeItemStats(*p, {iHelpItem, Level});

        // The window shows every column with a value at +0, so the row starts empty: no column of
        // the item shown before stays.
        std::ranges::fill(g_iItemInfo[Level], 0);
        g_iItemInfo[Level][_COLUMN_TYPE_LEVEL] = Level;
        g_iItemInfo[Level][_COLUMN_TYPE_ATTMIN] = stats.damageMin;
        g_iItemInfo[Level][_COLUMN_TYPE_ATTMAX] = stats.damageMax;
        g_iItemInfo[Level][magicColumn] = stats.magicPower;
        g_iItemInfo[Level][_COLUMN_TYPE_DEFENCE] = stats.defense;
        g_iItemInfo[Level][_COLUMN_TYPE_DEFRATE] = stats.blockRate;
        g_iItemInfo[Level][_COLUMN_TYPE_REQSTR] = stats.requireStrength;
        g_iItemInfo[Level][_COLUMN_TYPE_REQDEX] = stats.requireDexterity;
        g_iItemInfo[Level][_COLUMN_TYPE_REQENG] = stats.requireEnergy;
        g_iItemInfo[Level][_COLUMN_TYPE_REQCHA] = stats.requireLeadership;
        g_iItemInfo[Level][_COLUMN_TYPE_REQVIT] = stats.requireVitality;
        g_iItemInfo[Level][_COLUMN_TYPE_REQNLV] = stats.requireLevel;
        g_iItemInfo[Level][_COLUMN_TYPE_CAN_EQUIP] = CanEquipItemInfoRow(g_iItemInfo[Level], hero) ? TRUE : FALSE;
    }
}

unsigned int getGoldColor(DWORD Gold)
{
    if (Gold >= 10000000)
    {
        return (255u << 24) + (0 << 16) + (0 << 8) + (255);
    }
    else if (Gold >= 1000000)
    {
        return (255u << 24) + (0 << 16) + (150 << 8) + (255);
    }
    else if (Gold >= 100000)
    {
        return (255u << 24) + (24 << 16) + (201 << 8) + (0);
    }

    return (255u << 24) + (150 << 16) + (220 << 8) + (255);
}

void ConvertGold(double dGold, wchar_t* szText, int iDecimals /*= 0*/)
{
    wchar_t szTemp[256];
    int iCipherCnt = 0;
    auto dwValueTemp = (DWORD)dGold;

    while (dwValueTemp / 1000 > 0)
    {
        iCipherCnt = iCipherCnt + 3;
        dwValueTemp = dwValueTemp / 1000;
    }

    mu_swprintf(szText, L"%d", dwValueTemp);

    while (iCipherCnt > 0)
    {
        dwValueTemp = (DWORD)dGold;
        dwValueTemp = (dwValueTemp % (int)pow(10.f, (float)iCipherCnt)) / (int)pow(10.f, (float)(iCipherCnt - 3));
        mu_swprintf(szTemp, L",%03d", dwValueTemp);
        wcscat(szText, szTemp);
        iCipherCnt = iCipherCnt - 3;
    }

    if (iDecimals > 0)
    {
        dwValueTemp = (int)(dGold * pow(10.f, (float)iDecimals)) % (int)pow(10.f, (float)iDecimals);
        mu_swprintf(szTemp, L".%d", dwValueTemp);
        wcscat(szText, szTemp);
    }
}

void ConvertGold64(__int64 Gold, wchar_t* Text)
{
    int Gold1 = Gold % 1000;
    int Gold2 = Gold % 1000000 / 1000;
    int Gold3 = Gold % 1000000000 / 1000000;
    int Gold4 = Gold % 1000000000000 / 1000000000;
    int Gold5 = Gold % 1000000000000000 / 1000000000000;
    int Gold6 = Gold / 1000000000000000;
    if (Gold >= 1000000000000000)
        mu_swprintf(Text, L"%d,%03d,%03d,%03d,%03d,%03d", Gold6, Gold5, Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000000000)
        mu_swprintf(Text, L"%d,%03d,%03d,%03d,%03d", Gold5, Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000000)
        mu_swprintf(Text, L"%d,%03d,%03d,%03d", Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000)
        mu_swprintf(Text, L"%d,%03d,%03d", Gold3, Gold2, Gold1);
    else if (Gold >= 1000)
        mu_swprintf(Text, L"%d,%03d", Gold2, Gold1);
    else
        mu_swprintf(Text, L"%d", Gold1);
}

void ConvertTaxGold(DWORD Gold, wchar_t* Text)
{
    Gold += ((LONGLONG)Gold * g_pNPCShop->GetTaxRate()) / 100;

    int Gold1 = Gold % 1000;
    int Gold2 = Gold % 1000000 / 1000;
    int Gold3 = Gold % 1000000000 / 1000000;
    int Gold4 = Gold / 1000000000;
    if (Gold >= 1000000000)
        mu_swprintf(Text, L"%d,%03d,%03d,%03d", Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000)
        mu_swprintf(Text, L"%d,%03d,%03d", Gold3, Gold2, Gold1);
    else if (Gold >= 1000)
        mu_swprintf(Text, L"%d,%03d", Gold2, Gold1);
    else
        mu_swprintf(Text, L"%d", Gold1);
}

void ConvertChaosTaxGold(DWORD Gold, wchar_t* Text)
{
    Gold += ((LONGLONG)Gold * g_nChaosTaxRate) / 100;

    int Gold1 = Gold % 1000;
    int Gold2 = Gold % 1000000 / 1000;
    int Gold3 = Gold % 1000000000 / 1000000;
    int Gold4 = Gold / 1000000000;
    if (Gold >= 1000000000)
        mu_swprintf(Text, L"%d,%03d,%03d,%03d", Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000)
        mu_swprintf(Text, L"%d,%03d,%03d", Gold3, Gold2, Gold1);
    else if (Gold >= 1000)
        mu_swprintf(Text, L"%d,%03d", Gold2, Gold1);
    else
        mu_swprintf(Text, L"%d", Gold1);
}

int64_t CalcRepairCost(int64_t ItemValue, int Durability, int MaxDurability, short Type, bool SelfRepair)
{
    // Cap ItemValue at 400M
    int64_t repairGold = std::min<int64_t>(ItemValue, 400000000LL);

    // Calculate percentage of durability lost
    float percent = 1.f - static_cast<float>(Durability) / MaxDurability;
    if (percent > 0.f)
    {
        // Repair cost calculation
        double fRoot = sqrt(static_cast<double>(repairGold));
        double fRootRoot = sqrt(fRoot);
        repairGold = static_cast<int64_t>(3.5 * fRoot * fRootRoot * percent) + 1;

        // Adjust for 0 durability
        if (Durability <= 0)
        {
            if (GameLogic::Items::IsDarkLordPetType(Type))
            {
                repairGold *= 2;
            }
            else
            {
                repairGold = static_cast<int64_t>(repairGold * 1.4);
            }
        }
    }
    else
    {
        repairGold = 0; // No cost if no durability loss
    }

    if (SelfRepair)
    {
        // +150% adjustment for self-repair (vs doing it in shop)
        repairGold = static_cast<int64_t>(repairGold * 2.5);
    }

    // Round cost to the nearest 100 or 10
    if (repairGold >= 1000)
    {
        repairGold = (repairGold / 100) * 100;
    }
    else if (repairGold >= 100)
    {
        repairGold = (repairGold / 10) * 10;
    }

    return repairGold;
}

int64_t CalcSelfRepairCost(int64_t ItemValue, int Durability, int MaxDurability, short Type) 
{
    return CalcRepairCost(ItemValue, Durability, MaxDurability, Type, true);
}

int64_t ConvertRepairGold(int64_t Gold, int Durability, int MaxDurability, short Type, wchar_t* Text)
{
    int64_t repairGold = 0;

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop()) {
        repairGold = CalcRepairCost(Gold, Durability, MaxDurability, Type, false);
    }
    else if (g_pMyInventory->IsVisible() && !g_pNPCShop->IsVisible()) {
        repairGold = CalcRepairCost(Gold, Durability, MaxDurability, Type, true);
    }

    ConvertGold(repairGold, Text);

    return repairGold;
}

void RepairAllGold(void)
{
    wchar_t text[100];

    AllRepairGold = 0;

    for (int i = 0; i < MAX_EQUIPMENT; ++i)
    {
        if (CharacterMachine->Equipment[i].Type != -1)
        {
            ITEM* ip = &CharacterMachine->Equipment[i];
            ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];

            int Level = ip->Level;
            int maxDurability = CalcMaxDurability(ip, p, Level);

            if (GameLogic::Items::IsRepairBan(ip) == true)
            {
                continue;
            }

            //. check durability
            if (ip->Durability < maxDurability)
            {
                int gold = ConvertRepairGold(ItemValue(ip, 2), ip->Durability, maxDurability, ip->Type, text);

                if (GameLogic::Items::IsLuckyItemType(ip->Type))
                    gold = 0;
                AllRepairGold += gold;
            }
        }
    }

    ITEM* pItem = NULL;
    for (int i = 0; i < (int)(g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems()); ++i)
    {
        pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(i);

        if (pItem)
        {
            ITEM_ATTRIBUTE* p = &ItemAttribute[pItem->Type];

            int Level = pItem->Level;
            int maxDurability = CalcMaxDurability(pItem, p, Level);

            if (GameLogic::Items::IsRepairBan(pItem))
            {
                continue;
            }

            //. check durability
            if (pItem->Durability < maxDurability)
            {
                int gold = ConvertRepairGold(ItemValue(pItem, 2), pItem->Durability, maxDurability, pItem->Type, text);
                if (GameLogic::Items::IsLuckyItemType(pItem->Type))
                    gold = 0;
                AllRepairGold += gold;
            }
        }
    }
}

int InventoryStartX;
int InventoryStartY;
int ShopInventoryStartX;
int ShopInventoryStartY;
int TradeInventoryStartX;
int TradeInventoryStartY;
int CharacterInfoStartX;
int CharacterInfoStartY;
int GuildStartX;
int GuildStartY;

WORD CalcMaxDurability(const ITEM* ip, ITEM_ATTRIBUTE* p, int Level)
{
    WORD maxDurability = p->Durability;

    // Staffs, sticks and books have a durability of their own for magic.
    if (p->MagicDur > 0)
    {
        maxDurability = p->MagicDur;
    }
    for (int i = 0; i < Level; i++)
    {
        if (ip->Type >= ITEM_SCROLL_OF_BLOOD)
        {
            break;
        }
        else
            if (i >= 14)
            {
                maxDurability = (maxDurability + 8 >= 255 ? 255 : maxDurability + 8);
            }
            else if (i >= 13)	// 14
            {
                maxDurability = (maxDurability + 7 >= 255 ? 255 : maxDurability + 7);
            }
            else
                if (i >= 12)	// 13
                {
                    maxDurability += 6;
                }
                else if (i >= 11)	// 12
                {
                    maxDurability += 5;
                }
                else
                    if (i >= 10)	// 11
                    {
                        maxDurability += 4;
                    }
                    else if (i >= 9)	// 10
                    {
                        maxDurability += 3;
                    }
                    else if (i >= 4)	// 5~9
                    {
                        maxDurability += 2;
                    }
                    else	// 1~4
                    {
                        maxDurability++;
                    }
    }

    if (GameLogic::Items::IsDarkLordPet(ip))
    {
        maxDurability = 255;
    }

    if (ip->AncientDiscriminator > 0)
    {
        maxDurability += 20;
    }
    else if (ip->ExcellentFlags > 0 && !GameLogic::Items::IsSecondOrThirdTierWing(ip) &&
             !GameLogic::Items::IsDivineArchangelWeapon(ip))
    {
        maxDurability += 15;
    }

    if (GameLogic::Items::IsLuckyItemType(ip->Type))
    {
        maxDurability = 255;
    }

    return  maxDurability;
}

void GetSpecialOptionText(int Type, wchar_t* Text, WORD Option, BYTE Value, int iMana)
{
    switch (Option)
    {
    case AT_SKILL_BLOCKING:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::DefendSkillManaD, iMana);
        break;
    case AT_SKILL_FALLING_SLASH:
    case AT_SKILL_FALLING_SLASH_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::FallingSlashSkillManaD, iMana);
        break;
    case AT_SKILL_LUNGE:
    case AT_SKILL_LUNGE_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::LungeSkillManaD, iMana);
        break;
    case AT_SKILL_UPPERCUT:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::UppercutSkillManaD, iMana);
        break;
    case AT_SKILL_CYCLONE:
    case AT_SKILL_CYCLONE_STR:
    case AT_SKILL_CYCLONE_STR_MG:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::CycloneCuttingSkillManaD, iMana);
        break;
    case AT_SKILL_SLASH:
    case AT_SKILL_SLASH_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::SlashingSkillManaD, iMana);
        break;
    case AT_SKILL_TRIPLE_SHOT:
    case AT_SKILL_TRIPLE_SHOT_STR:
    case AT_SKILL_TRIPLE_SHOT_MASTERY:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::TripleShotSkillManaD, iMana);
        break;
    case AT_SKILL_BLAST_CROSSBOW4:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::_4ShotSkillManaD, iMana);
        break;
    case AT_SKILL_MULTI_SHOT:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::_4ShotSkillManaD, iMana);
        break;
    case AT_SKILL_RECOVER:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::_4ShotSkillManaD, iMana);
        break;
    case AT_SKILL_POWER_SLASH:
    case AT_SKILL_POWER_SLASH_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::PowerSlashSkillManaD, iMana);
        break;
    case AT_LUCK:
        mu_swprintf(Text, I18N::Game::LuckSuccessRateOfJewelOfSoul25);
        break;
    case AT_IMPROVE_DAMAGE:
        mu_swprintf(Text, I18N::Game::AdditionalDmgD, Value);
        break;
    case AT_IMPROVE_MAGIC:
        mu_swprintf(Text, I18N::Game::AdditionalWizardryDmgD, Value);
        break;
    case AT_IMPROVE_CURSE:
        mu_swprintf(Text, I18N::Game::AdditionalCurseSpellD, Value);
        break;
    case AT_IMPROVE_BLOCKING:
        mu_swprintf(Text, I18N::Game::AdditionalDefenseRateD, Value);
        break;
    case AT_IMPROVE_DEFENSE:
        mu_swprintf(Text, I18N::Game::AdditionalDefenseD, Value);
        break;
    case AT_LIFE_REGENERATION:
        if (!(ITEM_LOCHS_FEATHER <= Type && Type <= ITEM_INVISIBILITY_CLOAK))
        {
            mu_swprintf(Text, I18N::Game::AutomaticHPRecoveryD, Value);
        }
        break;
    case AT_IMPROVE_LIFE:
        mu_swprintf(Text, I18N::Game::IncreaseMaxHP4);
        break;
    case AT_IMPROVE_MANA:
        mu_swprintf(Text, I18N::Game::IncreaseMaxMana4);
        break;
    case AT_DECREASE_DAMAGE:
        mu_swprintf(Text, I18N::Game::DamageDecrease4);
        break;

    case AT_REFLECTION_DAMAGE:
        mu_swprintf(Text, I18N::Game::ReflectDamage5);
        break;
    case AT_IMPROVE_BLOCKING_PERCENT:
        mu_swprintf(Text, I18N::Game::DefenseSuccessRate10);
        break;
    case AT_IMPROVE_GAIN_GOLD:
        mu_swprintf(Text, I18N::Game::IncreasesAcquisitionRateOfZenAfterHuntingMonsters30);
        break;
    case AT_EXCELLENT_DAMAGE:
        mu_swprintf(Text, I18N::Game::ExcellentDamageRate10);
        break;
    case AT_IMPROVE_DAMAGE_LEVEL:
        mu_swprintf(Text, I18N::Game::IncreaseDamageLevel20);
        break;
    case AT_IMPROVE_DAMAGE_PERCENT:
        mu_swprintf(Text, I18N::Game::IncreaseDamageD, Value);
        break;
    case AT_IMPROVE_MAGIC_LEVEL:
        mu_swprintf(Text, I18N::Game::IncreaseWizardryDmgLevel20);
        break;
    case AT_IMPROVE_MAGIC_PERCENT:
        mu_swprintf(Text, I18N::Game::IncreaseWizardryDmgD, Value);
        break;
    case AT_IMPROVE_ATTACK_SPEED:
        mu_swprintf(Text, I18N::Game::IncreaseAttackingWizardrySpeedD, Value);
        break;
    case AT_IMPROVE_GAIN_LIFE:
        mu_swprintf(Text, I18N::Game::IncreasesAcquisitionRateOfLifeAfterHuntingMonstersLife8);
        break;
    case AT_IMPROVE_GAIN_MANA:
        mu_swprintf(Text, I18N::Game::IncreasesAcquisitionRateOfManaAfterHuntingMonstersMana8);
        break;
    case AT_IMPROVE_EVADE:
        mu_swprintf(Text, I18N::Game::Parrying10Increased);
        break;
    case AT_SKILL_RIDER:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::RaidSkillManaD, iMana);
        break;
    case AT_SKILL_FORCE:  //
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::LongSpearSkillManaD, iMana);
        break;
    case AT_SKILL_FORCE_WAVE:
    case AT_SKILL_FORCE_WAVE_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::ForceWaveSkillManaD, iMana);
        break;
    case AT_SKILL_EARTHSHAKE:
    case AT_SKILL_EARTHSHAKE_STR:
    case AT_SKILL_EARTHSHAKE_MASTERY:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::EarthShakeSkillManaD, iMana);
        break;
    case AT_SKILL_PLASMA_STORM_FENRIR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::PlasmaStormSkillManaD, iMana);
        break;
    case AT_SET_OPTION_IMPROVE_DEFENCE:
        mu_swprintf(Text, I18N::Game::IncreaseDefensiveSkillD, Value);
        break;
    case AT_SET_OPTION_IMPROVE_CHARISMA:
        mu_swprintf(Text, I18N::Game::IncreaseCommandD, Value);
        break;
    case AT_SET_OPTION_IMPROVE_DAMAGE:
        mu_swprintf(Text, I18N::Game::IncreaseDOfDamage, Value);
        break;
    case AT_IMPROVE_HP_MAX:
        mu_swprintf(Text, I18N::Game::HPDIncreased, Value);
        break;
    case AT_IMPROVE_MP_MAX:
        mu_swprintf(Text, I18N::Game::ManaDIncreased, Value);
        break;
    case AT_ONE_PERCENT_DAMAGE:
        mu_swprintf(Text, I18N::Game::IgnorOpponentSDefensivePowerByD, Value);
        break;
    case AT_IMPROVE_AG_MAX:
        mu_swprintf(Text, I18N::Game::MaxAGDIncreased, Value);
        break;
    case AT_DAMAGE_ABSORB:
        mu_swprintf(Text, I18N::Game::AbsorbDAdditionalDamage, Value);
        break;
    case AT_SET_OPTION_IMPROVE_STRENGTH:
        mu_swprintf(Text, I18N::Game::IncreaseStrengthD, Value);
        break;
    case AT_SET_OPTION_IMPROVE_DEXTERITY:
        mu_swprintf(Text, I18N::Game::IncreaseAgilityD, Value);
        break;
    case AT_SET_OPTION_IMPROVE_VITALITY:
        mu_swprintf(Text, I18N::Game::IncreaseStaminaD, Value);
        break;
    case AT_SET_OPTION_IMPROVE_ENERGY:
        mu_swprintf(Text, I18N::Game::IncreaseEnergyD, Value);
        break;
    case AT_IMPROVE_MAX_MANA:
        mu_swprintf(Text, I18N::Game::MaxManaIncreasedByD, Value);
        break;
    case AT_IMPROVE_MAX_AG:
        mu_swprintf(Text, I18N::Game::MaxAGIncreasedByD, Value);
        break;
    case AT_DAMAGE_REFLECTION:
        mu_swprintf(Text, I18N::Game::ReturnSTheEnemySAttackPowerInD, Value);
        break;
    case AT_RECOVER_FULL_LIFE:
        mu_swprintf(Text, I18N::Game::CompleteRecoveryOfLifeInDRate, Value);
        break;
    case AT_RECOVER_FULL_MANA:
        mu_swprintf(Text, I18N::Game::CompleteRecoverOfManaInDRate, Value);
        break;
    case AT_SKILL_SUMMON_EXPLOSION:
        mu_swprintf(Text, I18N::Game::ExplosionSkillManaD, iMana);
        break;
    case AT_SKILL_SUMMON_REQUIEM:
        mu_swprintf(Text, I18N::Game::RequiemManaD, iMana);
        break;
    case AT_SKILL_SUMMON_POLLUTION:
        mu_swprintf(Text, I18N::Game::PollutionSkillManaD, iMana);
        break;
    case AT_SKILL_KILLING_BLOW:
    case AT_SKILL_KILLING_BLOW_STR:
    case AT_SKILL_KILLING_BLOW_MASTERY:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::KillingBlowManaD, iMana);
        break;
    case AT_SKILL_BEAST_UPPERCUT:
    case AT_SKILL_BEAST_UPPERCUT_STR:
    case AT_SKILL_BEAST_UPPERCUT_MASTERY:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        mu_swprintf(Text, I18N::Game::BeastUppercutManaD, iMana);
        break;
    }
}

// How wide a description line is drawn before it wraps, in the tooltip's text units: about the widest lines of the
// original tooltips.
static constexpr int ItemDescriptionWrapWidth = 200;

// (sx, sy) in the screen's 640x480 stretch, `screen`.
// A test's measure of description text (SetItemTooltipTextMeasure); nullptr: the game's font.
static ItemTooltipTextMeasure s_itemTooltipTextMeasure = nullptr;

void SetItemTooltipTextMeasure(ItemTooltipTextMeasure measure)
{
    s_itemTooltipTextMeasure = measure;
}

std::optional<PetInfoRequest> GetPetInfoRequest(int itemType, int pointedIndex, int inventype)
{
    const StorageType storage = static_cast<StorageType>(inventype);
    if (itemType == ITEM_DARK_HORSE_ITEM)
    {
        return pointedIndex == EQUIPMENT_HELPER
                   ? std::optional<PetInfoRequest>(PetInfoRequest{PetType::DarkHorse, storage, EQUIPMENT_HELPER})
                   : std::nullopt;
    }
    return pointedIndex == EQUIPMENT_WEAPON_LEFT
               ? std::optional<PetInfoRequest>(PetInfoRequest{PetType::DarkRaven, storage, EQUIPMENT_WEAPON_LEFT})
               : std::nullopt;
}

void WriteItemDescriptionLines(const Data::Items::ItemDefinition& definition, int level,
                               const ItemTooltipValues& values, bool afterOptions, int& rowsLeft,
                               ItemTooltipTextMeasure measure)
{
    const int firstRow = TextNum;
    const int lastRow = std::min(TextNum + rowsLeft, static_cast<int>(std::size(TextList)));
    const int itemType = Data::Items::MakeItemType(definition.group, definition.number);
    for (const Data::Items::ItemDescriptionLine& line : definition.description)
    {
        if (line.afterOptions != afterOptions || !line.IsShownAt(level) ||
            !Engine::Items::TooltipValues::AreTooltipConditionsMet(line.when, itemType, level, values))
        {
            continue;
        }
        // A line may be a shared text of the tooltip file.
        const Data::LocalizedString* lineText =
            line.textName.empty() ? &line.text : g_ItemDatabase.GetTooltipFile().FindText(line.textName);
        // In the UI's language, also when the item names stay English ("Translate item names" off).
        const std::string_view locale = I18N::GetCurrentLocale();
        // A longer text is cut here; reading the item warns about one (Data::Items::MaxDescriptionLineLength).
        wchar_t filled[Data::Items::MaxDescriptionLineLength + 1];
        const size_t length =
            lineText == nullptr
                ? 0
                : TooltipText::Write(TooltipText::Compile(lineText->Get(locale), line.values, &definition, locale),
                                     level, values, filled, std::size(filled));
        const std::wstring text(filled, length);
        // A blank line is one blank piece. So is a blank line inside a text (`\n\n`, or a text that starts with
        // `\n`): a row left empty would end the tooltip there.
        const std::vector<std::wstring> pieces =
            text.empty() ? std::vector<std::wstring>(1) : WrapTextToWidth(text, ItemDescriptionWrapWidth, measure);
        for (const std::wstring& piece : pieces)
        {
            if (TextNum >= lastRow)
            {
                break;
            }
            if (piece.empty())
            {
                // As the tooltip writes its own blank rows.
                mu_swprintf(TextList[TextNum], L"\n");
                SkipNum++;
            }
            else
            {
                mu_swprintf(TextList[TextNum], L"%.*ls", static_cast<int>(std::size(TextList[0]) - 1), piece.c_str());
            }
            TextListColor[TextNum] = static_cast<int>(
                Engine::Items::TooltipValues::GetTooltipColor(line.colors, line.color, itemType, level, values));
            TextBold[TextNum] = line.bold;
            TextNum++;
        }
    }
    rowsLeft -= TextNum - firstRow;
}

// Writes the lines of the item tooltip layout that come after its section `after`, up to its next section, into
// TextList, TextListColor and TextBold: those whose conditions hold, with their placeholders filled in.
static void WriteItemTooltipLayoutLines(const ITEM& item, int level, const ItemTooltipValues& values,
                                        Data::Items::TooltipSection after)
{
    const Data::Items::ItemTooltipFile& file = g_ItemDatabase.GetTooltipFile();
    const Data::Items::TooltipLayout* layout = file.FindLayout(Data::Items::DefaultTooltipLayout);
    if (layout == nullptr)
    {
        return;
    }
    const auto first = std::find_if(layout->lines.begin(), layout->lines.end(),
                                    [&](const Data::Items::TooltipLayoutLine& line) { return line.section == after; });
    for (auto line = first == layout->lines.end() ? first : first + 1;
         line != layout->lines.end() && line->section == Data::Items::TooltipSection::None; ++line)
    {
        if (!Engine::Items::TooltipValues::AreTooltipConditionsMet(line->when, item.Type, level, values))
        {
            continue;
        }
        if (TextNum >= static_cast<int>(std::size(TextList)))
        {
            break;
        }
        const Data::LocalizedString* text = line->textName.empty() ? &line->text : file.FindText(line->textName);
        if (text == nullptr || text->IsEmpty())
        {
            // A blank line, as the tooltip writes its own.
            mu_swprintf(TextList[TextNum], L"\n");
            TextNum++;
            SkipNum++;
            continue;
        }
        // In the UI's language, also when the item names stay English ("Translate item names" off); written straight
        // into the row, as the tooltip's own lines are.
        const std::string_view locale = I18N::GetCurrentLocale();
        if (TooltipText::Write(TooltipText::Compile(text->Get(locale), line->values, nullptr, locale), level, values,
                               TextList[TextNum], std::size(TextList[TextNum])) == 0)
        {
            // No text in the UI's language: a row left empty would end the tooltip there.
            mu_swprintf(TextList[TextNum], L"\n");
            TextNum++;
            SkipNum++;
            continue;
        }
        TextListColor[TextNum] =
            static_cast<int>(Engine::Items::TooltipValues::GetTooltipLineColor(*line, item.Type, level, values));
        TextBold[TextNum] = line->bold;
        TextNum++;
    }
}

// The item data's description of `item`, measured with the game's font (or a test's measure): see
// WriteItemDescriptionLines.
static void WriteItemDescription(const ITEM& item, int level, const ItemTooltipValues& values, bool afterOptions,
                                 int& rowsLeft)
{
    const Data::Items::ItemDefinition* definition = g_ItemDatabase.Find(item.Type);
    if (definition == nullptr)
    {
        return;
    }
    g_pRenderText->SetFont(g_hFont);
    WriteItemDescriptionLines(*definition, level, values, afterOptions, rowsLeft,
                              [](const wchar_t* text, size_t length) -> int
                              {
                                  return s_itemTooltipTextMeasure != nullptr
                                             ? s_itemTooltipTextMeasure(text, length)
                                             : g_pRenderText->MeasureText(text, static_cast<int>(length)).cx;
                              });
}

// The name the tooltip gives an item and its color.
struct ItemTooltipName
{
    std::wstring text;
    int color = TEXT_COLOR_WHITE;
};

// The item's standard name: " +level" above +0 after its name and, for the tooltip's `full` name, its set's name
// before it for an ancient item and "Excellent" for an excellent one.
static std::wstring StandardItemName(const ITEM& item, int level, bool full)
{
    wchar_t setName[64]{};
    std::wstring name;
    if (full && g_csItemOption.GetSetItemName(setName, item.Type, item.AncientDiscriminator))
        name = setName;
    name += ItemAttribute[item.Type].Name;
    if (full && item.ExcellentFlags > 0)
        name = std::wstring(I18N::Game::Excellent) + L" " + name;
    if (level > 0)
        name += L" +" + std::to_wstring(level);
    return name;
}

// The item's standard name color: an ancient, socket, excellent, +7 and up, or optioned item's, else white.
static int StandardItemNameColor(const ITEM& item, int level)
{
    if (item.AncientDiscriminator > 0)
        return TEXT_COLOR_GREEN_BLUE;
    if (g_SocketItemMgr.IsSocketItem(&item))
        return TEXT_COLOR_VIOLET;
    if (item.SpecialNum > 0 && item.ExcellentFlags > 0)
        return TEXT_COLOR_GREEN;
    if (level >= 7)
        return TEXT_COLOR_YELLOW;
    return item.SpecialNum > 0 ? TEXT_COLOR_BLUE : TEXT_COLOR_WHITE;
}

// The first of the name lines of the item of `type` whose levels and conditions hold at `level`, or nullptr.
static const Data::Items::ItemNameLine* FindItemNameLine(const Data::Items::ItemDefinition& definition, int type,
                                                         int level, const ItemTooltipValues& values)
{
    for (const Data::Items::ItemNameLine& line : definition.nameLines)
    {
        if (line.IsShownAt(level) &&
            Engine::Items::TooltipValues::AreTooltipConditionsMet(line.when, type, level, values))
            return &line;
    }
    return nullptr;
}

// A name line's text in the item names' language (the UI's, or English with "Translate item names" off) with its
// placeholders filled in; nothing when it has none, also when it is empty in that language: the standard name stays
// then (an empty name would be an empty first row of the tooltip, and an empty name in the windows and ground labels).
static std::optional<std::wstring> ItemNameLineText(const Data::Items::ItemNameLine& line,
                                                    const Data::Items::ItemDefinition& definition, int level,
                                                    const ItemTooltipValues& values)
{
    if (!line.hasText)
        return std::nullopt;
    const Data::LocalizedString* text =
        line.textName.empty() ? &line.text : g_ItemDatabase.GetTooltipFile().FindText(line.textName);
    if (text == nullptr)
        return std::nullopt;
    // In the item names' language, as the item's name.
    const std::string& locale = g_ItemDatabase.GetDisplayLocale();
    // A longer text is cut here; reading the item warns about one (Data::Items::MaxNameLineLength).
    wchar_t filled[Data::Items::MaxNameLineLength + 1];
    const size_t length = TooltipText::Write(TooltipText::Compile(text->Get(locale), line.values, &definition, locale),
                                             level, values, filled, std::size(filled));
    if (length == 0)
        return std::nullopt;
    return std::wstring(filled, length);
}

// The item's name in the item names' language and its color, as its tooltip and the repair window write it: the first
// of its name lines whose levels and conditions hold gives them (the standard name where the line has no text, the
// standard color where it has no color, then its color rules); without one, the standard name and color.
static ItemTooltipName GetItemTooltipName(const ITEM& item, int level, const ItemTooltipValues& values)
{
    ItemTooltipName name{StandardItemName(item, level, true), StandardItemNameColor(item, level)};
    const Data::Items::ItemDefinition* definition = g_ItemDatabase.Find(item.Type);
    const Data::Items::ItemNameLine* line =
        definition != nullptr ? FindItemNameLine(*definition, item.Type, level, values) : nullptr;
    if (line == nullptr)
        return name;
    if (std::optional<std::wstring> text = ItemNameLineText(*line, *definition, level, values))
        name.text = std::move(*text);
    const auto base = line->hasColor ? line->color : static_cast<Data::Items::ItemTextColor>(name.color);
    name.color =
        static_cast<int>(Engine::Items::TooltipValues::GetTooltipColor(line->colors, base, item.Type, level, values));
    return name;
}

std::wstring GetItemNameText(int type, int level, const ITEM* item)
{
    if (!Data::Items::IsValidItemType(type))
        return {};
    // Without the item, one of the type at the level and nothing else: a line's condition on the item's durability or
    // options reads 0.
    ITEM bare{};
    if (item == nullptr)
    {
        bare.Type = static_cast<short>(type);
        bare.Level = level;
        item = &bare;
    }
    const Data::Items::ItemDefinition* definition = g_ItemDatabase.Find(type);
    if (definition != nullptr && !definition->nameLines.empty())
    {
        const ItemTooltipValues values = Engine::Items::TooltipValues::ComputeItemTooltipValues(*item);
        if (const Data::Items::ItemNameLine* line = FindItemNameLine(*definition, type, level, values))
        {
            if (std::optional<std::wstring> text = ItemNameLineText(*line, *definition, level, values))
                return std::move(*text);
        }
    }
    return StandardItemName(*item, level, false);
}

void GetItemName(int iType, int iLevel, wchar_t* Text, size_t textSize)
{
    if (textSize == 0)
        return;
    const std::wstring name = GetItemNameText(iType, iLevel);
    const size_t length = std::min(name.size(), textSize - 1);
    std::wmemcpy(Text, name.c_str(), length);
    Text[length] = L'\0';
}

std::wstring GetItemName(int iType, int iLevel)
{
    return GetItemNameText(iType, iLevel);
}

// What BuildItemTooltipText made of an item.
enum class ItemTooltipBuild
{
    Nothing,     // no tooltip for it
    DarkLordPet, // the pet manager draws its own
    Lines,       // TextList, TextListColor and TextBold hold its TextNum lines
};

// Writes the lines of an item's tooltip into TextList, TextListColor and TextBold (TextNum, SkipNum), without drawing
// them; RenderItemInfo draws them.
static ItemTooltipBuild BuildItemTooltipText(ITEM* ip, bool Sell, int Inventype, bool bItemTextListBoxUse)
{
    if (ip->Type == -1)
        return ItemTooltipBuild::Nothing;

    tm* ExpireTime;
    if (ip->bPeriodItem == true && ip->bExpiredPeriod == false)
    {
        _tzset();
        if (ip->lExpireTime == 0)
            return ItemTooltipBuild::Nothing;

        const time_t expireTime = ip->lExpireTime;
        ExpireTime = localtime(&expireTime);
    }

    ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];
    TextNum = 0;
    SkipNum = 0;

    ZeroMemory(TextListColor, 20 * sizeof(int));
    for (int i = 0; i < 30; i++)
    {
        TextList[i][0] = 0;
    }

    if (!Sell && (GameLogic::Items::IsDarkLordPet(ip)))
    {
        return ItemTooltipBuild::DarkLordPet;
    }

    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    SkipNum++;

    int Level = ip->Level;

    // The numbers of the stat lines, the built-in placeholders of the descriptions.
    const ItemTooltipValues values = Engine::Items::TooltipValues::ComputeItemTooltipValues(*ip);
    const ItemTooltipName name = GetItemTooltipName(*ip, Level, values);
    const int Color = name.color;

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCSHOP) && !GameLogic::Items::IsSellingBan(ip))
    {
        wchar_t Text[100];
        {
            if (Sell)
            {
                DWORD dwValue = ItemValue(ip, 0);
                ConvertGold(dwValue, Text);
                wchar_t Text2[100];

                ConvertTaxGold(ItemValue(ip, 0), Text2);
                mu_swprintf(TextList[TextNum], I18N::Game::PurchasingPriceSS, Text2, Text);
            }
            else
            {
                ConvertGold(ItemValue(ip, 1), Text);
                mu_swprintf(TextList[TextNum], I18N::Game::SellingPriceS, Text);
            }

            TextListColor[TextNum] = Color;
            //			TextBold[TextNum] = true;
            TextNum++;
            mu_swprintf(TextList[TextNum], L"\n");
            TextNum++;
            SkipNum++;
        }
    }
    if ((Inventype == mu::ui::window::TOOLTIP_TYPE_MY_SHOP ||
         Inventype == mu::ui::window::TOOLTIP_TYPE_PURCHASE_SHOP) &&
        !GameLogic::Items::IsPersonalShopBan(ip))
    {
        {
            GameLogic::PersonalShop::ItemPrice itemPrice;
            int indexInv = g_pMyShopInventory->GetInventoryCtrl()->GetIndexByItem(ip);
            wchar_t Text[100];

            const bool hasPrice = GetPersonalItemPrice(indexInv, itemPrice, g_IsPurchaseShop);
            if (hasPrice && itemPrice.Kind != GameLogic::PersonalShop::PriceKind::Zen)
            {
                // The server tells the player when the jewels or coins are not enough.
                mu_swprintf(Text, L"%d %ls", itemPrice.Amount, GameLogic::PersonalShop::GetCurrencyName(itemPrice));
                mu_swprintf(TextList[TextNum], I18N::Game::SellingPriceS, Text);
                TextListColor[TextNum] = TEXT_COLOR_YELLOW;
                TextBold[TextNum] = true;
                TextNum++;
                mu_swprintf(TextList[TextNum], L"\n");
                TextNum++;
                SkipNum++;
            }
            else if (hasPrice)
            {
                const int price = itemPrice.Amount;
                ConvertGold(price, Text);
                mu_swprintf(TextList[TextNum], I18N::Game::SellingPriceS, Text);

                if (price >= 10000000)
                    TextListColor[TextNum] = TEXT_COLOR_RED;
                else if (price >= 1000000)
                    TextListColor[TextNum] = TEXT_COLOR_YELLOW;
                else if (price >= 100000)
                    TextListColor[TextNum] = TEXT_COLOR_GREEN;
                else
                    TextListColor[TextNum] = TEXT_COLOR_WHITE;
                TextBold[TextNum] = true;
                TextNum++;
                mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

                DWORD gold = CharacterMachine->Gold;

                if ((int)gold < price && g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE)
                {
                    TextListColor[TextNum] = TEXT_COLOR_RED;
                    TextBold[TextNum] = true;
                    mu_swprintf(TextList[TextNum], I18N::Game::YouAreShortOfZen);
                    TextNum++;
                    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
                }
            }
            else if (g_IsPurchaseShop == PSHOPWNDTYPE_SALE)
            {
                TextListColor[TextNum] = TEXT_COLOR_RED;
                TextBold[TextNum] = true;
                mu_swprintf(TextList[TextNum], I18N::Game::RightClickForPriceSetting);
                TextNum++;
                mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            }
        }
    }

    mu_swprintf(TextList[TextNum], L"%.*ls", static_cast<int>(std::size(TextList[0]) - 1), name.text.c_str());
    TextListColor[TextNum] = Color;
    TextBold[TextNum] = true;
    TextNum++;
    mu_swprintf(TextList[TextNum], L"\n");
    TextNum++;
    SkipNum++;

    // The item cases whose lines need the game (the reset points, the saved revive position, the socket system,
    // the signs still needed); the others' lines are their descriptions.
    if (GameLogic::Items::IsResetFruit(ip))
    {
        DWORD statpoint = 0;
        statpoint = ip->Durability * 10;

        mu_swprintf(TextList[TextNum], I18N::Game::ResetPointD, statpoint);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        mu_swprintf(TextList[TextNum], I18N::Game::ResetsTheStatus);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        TextListColor[TextNum] = TEXT_COLOR_DARKRED;
        mu_swprintf(TextList[TextNum], I18N::Game::ItCanBeUsedWithItemRemoved);
        TextNum++;

        if (ip->Type == ITEM_RESET_FRUIT_CONTROL)
        {
            if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE;
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_DARKRED;
            }

            mu_swprintf(TextList[TextNum], I18N::Game::CanBeEquippedByS, I18N::Game::DarkLord);
            TextNum++;
        }
    }
    else if (ip->Type == ITEM_TALISMAN_OF_RESURRECTION)
    {
        mu_swprintf(TextList[TextNum], I18N::Game::RememberTheLocationOfOneSDeath);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        if (g_PortalMgr.IsRevivePositionSaved())
        {
            mu_swprintf(TextList[TextNum], I18N::Game::MoveByARightMouseClick);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            g_PortalMgr.GetRevivePositionText(TextList[TextNum]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }
    else if (GameLogic::Items::IsSocketSeedOrSphere(ip))
    {
        TextNum = g_SocketItemMgr.AttachToolTipForSeedSphereItem(ip, TextNum);
    }
    else if (ip->Type == ITEM_POTION + 110)
    {
        mu_swprintf(TextList[TextNum], I18N::Game::ItSASignInfusedWithTracesOfDimensions);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        mu_swprintf(TextList[TextNum], I18N::Game::CollectFiveAndTheSignsWillAutomatically);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        mu_swprintf(TextList[TextNum], I18N::Game::TransformIntoAMirrorOfDimensions);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        mu_swprintf(TextList[TextNum], L"\n");
        TextNum++;
        SkipNum++;
        mu_swprintf(TextList[TextNum], I18N::Game::DD, ip->Durability, 5);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        mu_swprintf(TextList[TextNum], I18N::Game::YouNeedDMoreToCreateAMirrorOfDimensions, 5 - ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }

    WriteItemTooltipLayoutLines(*ip, Level, values, Data::Items::TooltipSection::Name);
    // The description's rows, for both of its parts (before and after the options) together.
    int descriptionRowsLeft = MaxItemDescriptionRows;
    WriteItemDescription(*ip, Level, values, false, descriptionRowsLeft);
    if (ip->Type == ITEM_ARMOR_OF_GUARDSMAN)
    {
        int startIndex = 0;
        if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD
            || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER
            )
        {
            startIndex = 6;
        }

        int HeroLevel = CharacterAttribute->Level;

        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        mu_swprintf(TextList[TextNum], L"%ls %ls    %ls      %ls    ", I18N::Game::ChaosCastle, I18N::Game::Level, I18N::Game::MinLevel, I18N::Game::Cost); TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;

        for (int i = 0; i < 6; i++)
        {
            int Zen = g_iChaosCastleZen[i];

            mu_swprintf(TextList[TextNum], L"        %d             %3d~%3d     %3d,000", i + 1, g_iChaosCastleLevel[startIndex + i][0], std::min<int>(400, g_iChaosCastleLevel[startIndex + i][1]), Zen);
            if ((HeroLevel >= g_iChaosCastleLevel[startIndex + i][0] && HeroLevel <= g_iChaosCastleLevel[startIndex + i][1]) && gCharacterManager.IsMasterLevel(Hero->Class) == false)
            {
                TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE;
            }
            TextBold[TextNum] = false; TextNum++;
        }
        mu_swprintf(TextList[TextNum], L"         %d          %ls   %3d,000", 7, I18N::Game::MasterLevel, 1000);
        if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
        {
            TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
        }
        else
        {
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
        }
        TextBold[TextNum] = false;
        TextNum++;

        mu_swprintf(TextList[TextNum], L"\n");
        TextNum++;
        SkipNum++;
        mu_swprintf(TextList[TextNum], I18N::Game::RightClickToEnter);
        TextListColor[TextNum] = TEXT_COLOR_DARKBLUE;
        TextNum++;
    }
    else if (ip->Type == ITEM_LOST_MAP || ip->Type == ITEM_SYMBOL_OF_KUNDUN)
    {
        TextNum = RenderHellasItemInfo(ip, TextNum);
    }

    // The Talisman of Mobility's saved location, under its description's "Returns to the saved location by a click."
    if (ip->Type == ITEM_TALISMAN_OF_MOBILITY && ip->Durability == 1 && g_PortalMgr.IsPortalPositionSaved() &&
        TextNum < static_cast<int>(std::size(TextList)))
    {
        g_PortalMgr.GetPortalPositionText(TextList[TextNum]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }

    WriteItemTooltipLayoutLines(*ip, Level, values, Data::Items::TooltipSection::Description);

    if (!g_ItemDatabase.HasTag(ip->Type, Data::Items::ItemTag::NoClassLines))
    {
        RequireClass(p);
    }

    if (ip->Type >= ITEM_BOOTS && ip->Type < ITEM_BOOTS + MAX_ITEM_INDEX)
    {
        if (Level >= 5)
        {
            mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            mu_swprintf(TextList[TextNum], I18N::Game::IncreasesMovingSpeed);
            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
        }
    }

    if (ip->Type >= ITEM_GLOVES && ip->Type < ITEM_GLOVES + MAX_ITEM_INDEX)
    {
        if (Level >= 5)
        {
            mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

            mu_swprintf(TextList[TextNum], I18N::Game::SwimmingSpeedIncrease);
            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
        }
    }
    WriteItemTooltipLayoutLines(*ip, Level, values, Data::Items::TooltipSection::Classes);

    if (ip->SpecialNum > 0)
    {
        int iModelType = ip->Type;
        int iStartModelType = ITEM_SAPPHIRE_RING;
        int iEndModelType = ITEM_SAPPHIRE_NECKLACE;

        if (!(ITEM_SAPPHIRE_RING <= ip->Type && ITEM_SAPPHIRE_NECKLACE >= ip->Type))
        {
            mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        }
    }

    if (ip->option_380 != 0)
    {
        std::vector<std::wstring> Text380;

        if (g_pItemAddOptioninfo)
        {
            g_pItemAddOptioninfo->GetItemAddOtioninfoText(Text380, ip->Type);

            mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

            for (int i = 0; i < (int)Text380.size(); ++i)
            {
                wcsncpy(TextList[TextNum], Text380[i].c_str(), 100);
                TextListColor[TextNum] = TEXT_COLOR_REDPURPLE; TextBold[TextNum] = true; TextNum++;
            }

            mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        }
    }
    //#ifndef PBG_MOD_NEWCHAR_MONK_WING_2
    //#ifdef PBG_MOD_NEWCHAR_MONK_WING
    //	if(ip->Type==ITEM_WING+49)
    //	{
    //		mu_swprintf(TextList[TextNum],I18N::Game::AbsorbDOfDamage,15+Level);
    //		TextListColor[TextNum] = TEXT_COLOR_BLUE;
    //		TextNum++;
    //	}
    //#endif //PBG_MOD_NEWCHAR_MONK_WING
    //#endif //PBG_MOD_NEWCHAR_MONK_WING_2
    if (g_SocketItemMgr.IsSocketItem(ip));
    else
        if (ip->Jewel_Of_Harmony_Option != 0)
        {
            StrengthenItem type = g_pUIJewelHarmonyinfo->GetItemType(static_cast<int>(ip->Type));

            if (type < SI_None)
            {
                if (g_pUIJewelHarmonyinfo->IsHarmonyJewelOption(type, ip->Jewel_Of_Harmony_Option))
                {
                    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

                    HARMONYJEWELOPTION harmonyjewel = g_pUIJewelHarmonyinfo->GetHarmonyJewelOptionInfo(type, ip->Jewel_Of_Harmony_Option);

                    if (type == SI_Defense && ip->Jewel_Of_Harmony_Option == 7)
                    {
                        mu_swprintf(TextList[TextNum], L"%ls +%d%%", harmonyjewel.Name, harmonyjewel.HarmonyJewelLevel[ip->Jewel_Of_Harmony_OptionLevel]);
                    }
                    else
                    {
                        mu_swprintf(TextList[TextNum], L"%ls +%d", harmonyjewel.Name, harmonyjewel.HarmonyJewelLevel[ip->Jewel_Of_Harmony_OptionLevel]);
                    }

                    if (Level >= ip->Jewel_Of_Harmony_OptionLevel) TextListColor[TextNum] = TEXT_COLOR_YELLOW;
                    else TextListColor[TextNum] = TEXT_COLOR_GRAY;

                    TextBold[TextNum] = true; TextNum++;

                    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
                }
                else
                {
                    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

                    mu_swprintf(TextList[TextNum], L"%ls : %d %d %d"
                        , I18N::Game::ReinforcementOptionError
                        , (int)type
                        , (int)ip->Jewel_Of_Harmony_Option
                        , (int)ip->Jewel_Of_Harmony_OptionLevel
                    );

                    TextListColor[TextNum] = TEXT_COLOR_DARKRED;
                    TextBold[TextNum] = true; TextNum++;

                    mu_swprintf(TextList[TextNum], I18N::Game::SendScreenshotsWithTheReport);

                    TextListColor[TextNum] = TEXT_COLOR_DARKRED;
                    TextBold[TextNum] = true; TextNum++;

                    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
                }
            }
        }

    TextNum = g_csItemOption.RenderDefaultOptionText(ip, TextNum);

    int iMana;
    for (int i = 0; i < ip->SpecialNum; i++)
    {
        if (ITEM_SAPPHIRE_RING <= ip->Type && ITEM_SAPPHIRE_NECKLACE >= ip->Type)
        {
            break;
        }

        gSkillManager.GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL);
        GetSpecialOptionText(ip->Type, TextList[TextNum], ip->Special[i], ip->SpecialValue[i], iMana);

        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false; TextNum++;

        if (ip->Special[i] == AT_LUCK)
        {
            mu_swprintf(TextList[TextNum], I18N::Game::LuckCriticalDamageRate5, ip->SpecialValue[i]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        }
        else if (ip->Special[i] == AT_SKILL_RIDER)
        {
            mu_swprintf(TextList[TextNum], I18N::Game::KnightSpecificSkill);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }
        else if (ip->Special[i] == AT_SKILL_EARTHSHAKE
            || ip->Special[i] == AT_SKILL_EARTHSHAKE_STR
            || ip->Special[i] == AT_SKILL_EARTHSHAKE_MASTERY
            )
        {
            mu_swprintf(TextList[TextNum], I18N::Game::DarkLordExclusiveSkill);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }
        else if (ip->Special[i] == AT_IMPROVE_DAMAGE && GameLogic::Items::IsMagicSwordType(ip->Type))
        {
            mu_swprintf(TextList[TextNum], I18N::Game::AdditionalWizardryDmgD, ip->SpecialValue[i]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false; TextNum++;
        }
    }

    if (!g_ItemDatabase.HasTag(ip->Type, Data::Items::ItemTag::NoBlankAfterOptions))
    {
        mu_swprintf(TextList[TextNum], L"\n");
        TextNum++;
        SkipNum++;
    }
    WriteItemDescription(*ip, Level, values, true, descriptionRowsLeft);

    if (ip->Type == ITEM_HORN_OF_FENRIR)
    {
        GetSpecialOptionText(0, TextList[TextNum], AT_SKILL_PLASMA_STORM_FENRIR, 0, 0);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false; TextNum++; SkipNum++;
        if (ip->ExcellentFlags == 0x01)
        {
            mu_swprintf(TextList[TextNum], I18N::Game::IncreaseFinalDamageD, 10);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;

            mu_swprintf(TextList[TextNum], I18N::Game::IncreaseSpeed);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
        }
        else if (ip->ExcellentFlags == 0x02)
        {
            mu_swprintf(TextList[TextNum], I18N::Game::AbsorbFinalDamageD, 10);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;

            mu_swprintf(TextList[TextNum], I18N::Game::IncreaseSpeed);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
        }
        else if (ip->ExcellentFlags == 0x04)
        {
            WORD wLevel = CharacterAttribute->Level;

            mu_swprintf(TextList[TextNum], I18N::Game::AddedDOfLife, (wLevel / 2));
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
            mu_swprintf(TextList[TextNum], I18N::Game::AddedDOfMana, (wLevel / 2));
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
            mu_swprintf(TextList[TextNum], I18N::Game::AddedDAttack, (wLevel / 12));
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
            mu_swprintf(TextList[TextNum], I18N::Game::AddedDWizardry, (wLevel / 25));
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
            mu_swprintf(TextList[TextNum], L"\n");
            TextNum++;
            mu_swprintf(TextList[TextNum], I18N::Game::GoldenFenrir, (Hero->Level / 2));
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            TextNum++;
            mu_swprintf(TextList[TextNum], I18N::Game::ExclusiveEditionOnlyGivenToMUHeroes, (Hero->Level / 2));
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            TextNum++;
        }

        mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        mu_swprintf(TextList[TextNum], I18N::Game::CanSummonTheFenrirWhenEquipped);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextNum++;

        if (ip->ExcellentFlags == 0x00)
        {
            mu_swprintf(TextList[TextNum], I18N::Game::SkillsWillImproveThroughUpgrading);
            TextListColor[TextNum] = TEXT_COLOR_YELLOW;
            TextNum++;
        }
    }
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    else if (g_pMyInventory->IsInvenItem(ip->Type))
    {
        TextNum--; SkipNum--;

        if (ip->Durability == 254)
        {
            mu_swprintf(TextList[TextNum], I18N::Game::InUse);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        }

        switch (ip->Type)
        {
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
        case ITEM_HELPER + 128:
        case ITEM_HELPER + 129:
            mu_swprintf(TextList[TextNum], I18N::Game::FigurineItem);
            break;
        case ITEM_HELPER + 134:
            mu_swprintf(TextList[TextNum], I18N::Game::RelicItem);
            break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        case ITEM_HELPER + 130:
        case ITEM_HELPER + 131:
        case ITEM_HELPER + 132:
        case ITEM_HELPER + 133:
            mu_swprintf(TextList[TextNum], I18N::Game::CharmItem);
            break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

        default:
            break;
        }

        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        switch (ip->Type)
        {
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
        case ITEM_HELPER + 128:
            mu_swprintf(TextList[TextNum], I18N::Game::IncreaseCriticalDamageD, 10);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 129:
            mu_swprintf(TextList[TextNum], I18N::Game::IncreaseExcellentDamageD, 10);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 134:
            mu_swprintf(TextList[TextNum], I18N::Game::ItemDropRateIncreaseD, 20);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        case ITEM_HELPER + 130:
            mu_swprintf(TextList[TextNum], I18N::Game::MaximumHPIncreaseD, 50);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 131:
            mu_swprintf(TextList[TextNum], I18N::Game::MaximumMPIncreaseD, 50);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 132:
#ifdef LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            mu_swprintf(TextList[TextNum], I18N::Game::MaximumHPIncreaseD, 100);
#else //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            mu_swprintf(TextList[TextNum], I18N::Game::MaximumHPIncreaseD, 150);
#endif //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
#ifdef LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            mu_swprintf(TextList[TextNum], I18N::Game::MaximumSPIncreaseD, 500);
#else //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            mu_swprintf(TextList[TextNum], I18N::Game::MaximumSPIncreaseD, 50);
#endif //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 133:	// 골든메이플참
            mu_swprintf(TextList[TextNum], I18N::Game::MaximumMPIncreaseD, 150);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            mu_swprintf(TextList[TextNum], I18N::Game::MaximumAGIncreaseD, 50);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        default:
            break;
        }

        mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        mu_swprintf(TextList[TextNum], I18N::Game::RightClickOnYourInventoryToUse);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

    if (ip->bPeriodItem == true)
    {
        if (ip->bExpiredPeriod == true)
        {
            mu_swprintf(TextList[TextNum], I18N::Game::ExpiredItem);
            TextListColor[TextNum] = TEXT_COLOR_RED;
        }
        else
        {
            mu_swprintf(TextList[TextNum], I18N::Game::ExpirationDay);
            TextListColor[TextNum] = TEXT_COLOR_ORANGE;
            TextNum++;
            SkipNum++;

            mu_swprintf(TextList[TextNum], L"%d-%02d-%02d  %02d:%02d", ExpireTime->tm_year + 1900, ExpireTime->tm_mon + 1,
                ExpireTime->tm_mday, ExpireTime->tm_hour, ExpireTime->tm_min);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
        }

        TextNum++;
    }

    if (!bItemTextListBoxUse)
    {
        // An item that is not in the inventory grid is a worn one: the item sets count its options as equipped.
        // Without the game's windows (the tooltip tests) every item is one.
        bool bThisisEquippedItem = true;
        if (mu::ui::window::CMyInventory* inventory = g_pMyInventory)
        {
            bThisisEquippedItem = inventory->GetInventoryCtrl()->FindItemByKey(ip->Key) == nullptr;
        }

        TextNum = g_csItemOption.RenderSetOptionListInItem(ip, TextNum, bThisisEquippedItem);

        TextNum = g_SocketItemMgr.AttachToolTipForSocketItem(ip, TextNum);
    }

    return ItemTooltipBuild::Lines;
}

std::vector<ItemTooltipLine> BuildItemTooltipLines(ITEM* ip, bool Sell, int Inventype, bool bItemTextListBoxUse)
{
    std::vector<ItemTooltipLine> lines;
    if (BuildItemTooltipText(ip, Sell, Inventype, bItemTextListBoxUse) != ItemTooltipBuild::Lines)
    {
        return lines;
    }
    lines.reserve(TextNum);
    for (int index = 0; index < TextNum; ++index)
    {
        lines.push_back({TextList[index], TextListColor[index], TextBold[index] != 0});
    }
    return lines;
}

// (sx, sy) in the screen's 640x480 stretch, `screen`.
static void RenderItemInfo(const UI::Scaling::Transform& screen, int sx, int sy, ITEM* ip, bool Sell, int Inventype,
                           bool bItemTextListBoxUse)
{
    // Unconditional: the early returns below (and the pet-item delegation further down, which
    // renders its own tooltip via giPetManager::RenderPetItemInfo() instead) used to mean "this
    // frame draws nothing" under the old per-frame native draw -- already equivalent to "hidden"
    // for that item. The shared tooltip document is persistent, so this replicates that; the real
    // Show() call near the end of this function makes it visible again once actually reached.
    UI::Tooltip::HideLegacyTextList();

    const ItemTooltipBuild build = BuildItemTooltipText(ip, Sell, Inventype, bItemTextListBoxUse);
    if (build == ItemTooltipBuild::Nothing)
        return;
    if (build == ItemTooltipBuild::DarkLordPet)
    {
        // The pet's information is requested once a second at most, for the pet whose tooltip is drawn.
        static std::chrono::steady_clock::time_point lastPetInfoRequest;
        const auto now = std::chrono::steady_clock::now();
        if (lastPetInfoRequest == std::chrono::steady_clock::time_point() ||
            now - lastPetInfoRequest >= std::chrono::seconds(1))
        {
            lastPetInfoRequest = now;
            if (const std::optional<PetInfoRequest> request =
                    GetPetInfoRequest(ip->Type, g_pMyInventory->GetPointedItemIndex(), Inventype))
            {
                SocketClient->ToGameServer()->SendPetInfoRequest(request->pet, request->storage, request->slot);
            }
        }

        giPetManager::RenderPetItemInfo(screen, sx, sy, ip, Inventype);
        return;
    }

    ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];
    if (!bItemTextListBoxUse)
    {
        SIZE TextSize = { 0, 0 };
        int	Height = 0;
        int	EmptyLine = 0;
        int TextLine = 0;

        for (int i = 0; i < TextNum; ++i)
        {
            if (TextList[i][0] == '\0')		break;
            else if (TextList[i][0] == '\n')	++EmptyLine;
            else							++TextLine;
        }
        g_pRenderText->SetFont(g_hFont);
        TextSize = g_pRenderText->MeasureText(L"Q", 1);

        Height = static_cast<int>((TextLine * TextSize.cy + EmptyLine * TextSize.cy / 2.0f) * 1.1f);

        int iScreenHeight = 420;

        int nInvenHeight = p->Height * INVENTORY_SCALE;

        sy += INVENTORY_SCALE;
        if (sy + Height > iScreenHeight)
        {
            sy += iScreenHeight - (sy + Height);
        }
        else if (sy + Height > iScreenHeight)
        {
        }
    }

    bool isrendertooltip = true;

    if (isrendertooltip)
    {
        UI::Tooltip::ShowLegacyTextList(
            TextNum,
            UI::Scaling::PositionX(screen, static_cast<float>(sx)),
            UI::Scaling::PositionY(screen, static_cast<float>(sy)),
            bItemTextListBoxUse ? UI::Tooltip::Placement::Above : UI::Tooltip::Placement::Below);
    }
}

RepairInfoText BuildRepairInfoText(ITEM* ip)
{
    RepairInfoText info;
    if (GameLogic::Items::IsRepairBan(ip) == true)
    {
        return info;
    }

    ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];
    TextNum = 0;
    SkipNum = 0;
    for (int i = 0; i < 30; i++)
    {
        TextList[i][0] = 0;
    }

    int Level = ip->Level;
    // The name and its color as the item's tooltip has them.
    const ItemTooltipName name =
        GetItemTooltipName(*ip, Level, Engine::Items::TooltipValues::ComputeItemTooltipValues(*ip));
    const int Color = name.color;

    if (ip->Type < ITEM_POTION)
    {
        int maxDurability;

        mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        wchar_t Text[100];

        maxDurability = CalcMaxDurability(ip, p, Level);
        if (ip->Durability < maxDurability)
        {
            info.repairEnable = 2;

            int iGold = ItemValue(ip, 2);
            if (iGold == -1)
                return info;
            ConvertRepairGold(iGold, ip->Durability, maxDurability, ip->Type, Text);
            mu_swprintf(TextList[TextNum], I18N::Game::RepairingCostS, Text);

            TextListColor[TextNum] = Color;
        }
        else
        {
            info.repairEnable = 1;
            mu_swprintf(TextList[TextNum], I18N::Game::RepairingCostS, L"0");
            TextListColor[TextNum] = Color;
        }
        TextBold[TextNum] = true;
        TextNum++;

        //        RepairEnable = 1;
    }
    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    mu_swprintf(TextList[TextNum], L"%.*ls", static_cast<int>(std::size(TextList[0]) - 1), name.text.c_str());
    TextListColor[TextNum] = Color; TextBold[TextNum] = true; TextNum++;
    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    if (ip->Type < ITEM_POTION)
    {
        if (ip->bPeriodItem == false)
        {
            int maxDurability = CalcMaxDurability(ip, p, Level);

            mu_swprintf(TextList[TextNum], I18N::Game::DurabilityDD, ip->Durability, maxDurability);

            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    mu_swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
    info.lineCount = TextNum;
    return info;
}

// (sx, sy) in the screen's 640x480 stretch, `screen`.
static void RenderRepairInfo(const UI::Scaling::Transform& screen, int sx, int sy, ITEM* ip, bool Sell)
{
    (void)Sell;
    // Unconditional: an item without a repair tooltip used to mean "this frame draws nothing" under
    // the old per-frame native draw, which was already equivalent to "hidden" for that item type.
    // The shared tooltip document is persistent, so an explicit Hide() here replicates that -- Show()
    // at the very end of this function (reached only for an item with a repair tooltip) makes it
    // visible again, same net effect as before, one frame earlier than a stale previous item's
    // tooltip would otherwise have lingered.
    UI::Tooltip::HideLegacyTextList();

    const RepairInfoText info = BuildRepairInfoText(ip);
    if (info.repairEnable)
    {
        RepairEnable = *info.repairEnable;
    }
    if (info.lineCount == 0)
    {
        return;
    }

    g_pRenderText->SetFont(TextBold[0] ? g_hFontBold : g_hFont);
    const SIZE TextSize = g_pRenderText->MeasureText(L"Q", 1);
    int Height = (TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2;
    if (sy - Height >= 0)
        sy -= Height;
    else
        sy += ItemAttribute[ip->Type].Height * INVENTORY_SCALE;

    UI::Tooltip::ShowLegacyTextList(
        TextNum,
        UI::Scaling::PositionX(screen, static_cast<float>(sx)),
        UI::Scaling::PositionY(screen, static_cast<float>(sy)));
}

namespace
{
// Runs `render` with the anchor (x, y) in window pixels converted into the screen's 640x480
// stretch, which it then runs under (its text is measured there).
template <typename Render> void InScreenSpace(float x, float y, Render render)
{
    const UI::Scaling::Transform screen =
        UI::Scaling::ScreenOverlayTransform(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    const UI::Scaling::ScopedActiveTransform scope(screen);
    render(screen, static_cast<int>(std::lround(UI::Scaling::LogicalX(screen, x))),
           static_cast<int>(std::lround(UI::Scaling::LogicalY(screen, y))));
}
} // namespace

void RenderItemInfoAtPx(float x, float y, ITEM* ip, bool Sell, int Inventype, bool bItemTextListBoxUse)
{
    InScreenSpace(x, y, [&](const UI::Scaling::Transform& screen, int sx, int sy)
                  { RenderItemInfo(screen, sx, sy, ip, Sell, Inventype, bItemTextListBoxUse); });
}

void RenderRepairInfoAtPx(float x, float y, ITEM* ip, bool Sell)
{
    InScreenSpace(x, y, [&](const UI::Scaling::Transform& screen, int sx, int sy)
                  { RenderRepairInfo(screen, sx, sy, ip, Sell); });
}

bool GetAttackDamage(int* iMinDamage, int* iMaxDamage)
{
    int AttackDamageMin;
    int AttackDamageMax;

    ITEM* r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
    ITEM* l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
    if (PickItem.Number > 0 && SrcInventory == Inventory)
    {
        switch (SrcInventoryIndex)
        {
        case EQUIPMENT_WEAPON_RIGHT:
            r = &PickItem;
            break;
        case EQUIPMENT_WEAPON_LEFT:
            l = &PickItem;
            break;
        }
    }
    if (gCharacterManager.GetEquipedBowType() == BOWTYPE_CROSSBOW)
    {
        AttackDamageMin = CharacterAttribute->AttackDamageMinRight;
        AttackDamageMax = CharacterAttribute->AttackDamageMaxRight;
    }
    else if (gCharacterManager.GetEquipedBowType() == BOWTYPE_BOW)
    {
        AttackDamageMin = CharacterAttribute->AttackDamageMinLeft;
        AttackDamageMax = CharacterAttribute->AttackDamageMaxLeft;
    }
    else if (r->Type == -1)
    {
        AttackDamageMin = CharacterAttribute->AttackDamageMinLeft;
        AttackDamageMax = CharacterAttribute->AttackDamageMaxLeft;
    }
    else if (GameLogic::Items::IsStaffOrBookType(r->Type))
    {
        AttackDamageMin = CharacterAttribute->AttackDamageMinLeft;
        AttackDamageMax = CharacterAttribute->AttackDamageMaxLeft;
    }
    else
    {
        AttackDamageMin = CharacterAttribute->AttackDamageMinRight;
        AttackDamageMax = CharacterAttribute->AttackDamageMaxRight;
    }

    bool Alpha = false;
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK)
    {
        if (GameLogic::Items::IsWeaponOrAmmunition(l) && GameLogic::Items::IsWeaponOrAmmunition(r))
        {
            Alpha = true;
            AttackDamageMin = ((CharacterAttribute->AttackDamageMinRight * 55) / 100 + (CharacterAttribute->AttackDamageMinLeft * 55) / 100);
            AttackDamageMax = ((CharacterAttribute->AttackDamageMaxRight * 55) / 100 + (CharacterAttribute->AttackDamageMaxLeft * 55) / 100);
        }
    }
    else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        if (GameLogic::Items::IsBowCrossbowOrAmmunition(r) && GameLogic::Items::IsBowCrossbowOrAmmunition(l))
        {
            if ((l->Type == ITEM_BOLT && l->Level >= 1) || (r->Type == ITEM_ARROWS && r->Level >= 1))
            {
                Alpha = true;
            }
        }
    }
    else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
    {
        if (GameLogic::Items::IsSwordAxeMaceOrGloveType(l->Type) &&
            GameLogic::Items::IsSwordAxeMaceOrGloveType(r->Type))
        {
            Alpha = true;
            AttackDamageMin = ((CharacterAttribute->AttackDamageMinRight + CharacterAttribute->AttackDamageMinLeft) * 60 / 100);
            AttackDamageMax = ((CharacterAttribute->AttackDamageMaxRight + CharacterAttribute->AttackDamageMaxLeft) * 65 / 100);
        }
    }

    if (CharacterAttribute->Ability & ABILITY_PLUS_DAMAGE)
    {
        AttackDamageMin += 15;
        AttackDamageMax += 15;
    }

    *iMinDamage = AttackDamageMin;
    *iMaxDamage = AttackDamageMax;

    return Alpha;
}


void SetTextColor(float r, float g, float b)
{
    g_pRenderText->SetTextColor(r * 255, g * 255, b * 255, 255);
}

void SetYellowTextColor()
{
    SetTextColor(1.f, 0.8f, 0.1f);
}

void SetGrayTextColor()
{
    SetTextColor(0.7f, 0.7f, 0.7f);
}

void SetOrangeTextColor()
{
    SetTextColor(0.9f, 0.53f, 0.13f);
}

void SetJewelTextColor()
{
    g_pRenderText->SetFont(g_hFontBold);
    SetYellowTextColor();
}

std::unordered_set<int> boldTextItems = {
    MODEL_JEWEL_OF_BLESS,
    MODEL_JEWEL_OF_SOUL,
    MODEL_JEWEL_OF_LIFE,
    MODEL_JEWEL_OF_CHAOS,
    MODEL_JEWEL_OF_CREATION,
    MODEL_JEWEL_OF_GUARDIAN,
    MODEL_LOCHS_FEATHER,
    MODEL_GEMSTONE,
    MODEL_JEWEL_OF_HARMONY,
    MODEL_LOWER_REFINE_STONE,
    MODEL_HIGHER_REFINE_STONE,
    MODEL_COMPILED_CELE,
    MODEL_COMPILED_SOUL,
    MODEL_DEVILS_EYE,
    MODEL_DEVILS_KEY,
    MODEL_DEVILS_INVITATION,
};

std::unordered_set<int> whiteTextItems = {
    MODEL_SCROLL_OF_CHAOTIC_DISEIER,
    MODEL_SCROLL_OF_FIRE_SCREAM,
};

std::unordered_set<int> yellowTextItems = {
    
    MODEL_ZEN,
    MODEL_JEWEL_OF_BLESS,
    MODEL_JEWEL_OF_SOUL,
    MODEL_JEWEL_OF_LIFE,
    MODEL_JEWEL_OF_CHAOS,
    MODEL_JEWEL_OF_CREATION,
    MODEL_JEWEL_OF_GUARDIAN,
    MODEL_LOCHS_FEATHER,
    MODEL_GEMSTONE,
    MODEL_JEWEL_OF_HARMONY,
    MODEL_LOWER_REFINE_STONE,
    MODEL_HIGHER_REFINE_STONE,
    MODEL_COMPILED_CELE,
    MODEL_COMPILED_SOUL,
    MODEL_DEVILS_EYE,
    MODEL_DEVILS_KEY,
    MODEL_DEVILS_INVITATION,
    MODEL_BOX_OF_LUCK,
    MODEL_FRUITS,
    MODEL_SPIRIT,
    MODEL_EVENT + 16,
    MODEL_EVENT + 5,
    MODEL_OLD_SCROLL,
    MODEL_ILLUSION_SORCERER_COVENANT,
    MODEL_SCROLL_OF_BLOOD,
    MODEL_CURSED_CASTLE_WATER,
    MODEL_EVENT + 11,
    MODEL_EVENT + 12,
    MODEL_EVENT + 13,
    MODEL_EVENT + 14,
    MODEL_EVENT + 15,
    MODEL_SUSPICIOUS_SCRAP_OF_PAPER,
    MODEL_GAIONS_ORDER,
    MODEL_FIRST_SECROMICON_FRAGMENT,
    MODEL_SECOND_SECROMICON_FRAGMENT,
    MODEL_THIRD_SECROMICON_FRAGMENT,
    MODEL_FOURTH_SECROMICON_FRAGMENT,
    MODEL_FIFTH_SECROMICON_FRAGMENT,
    MODEL_SIXTH_SECROMICON_FRAGMENT,
    MODEL_COMPLETE_SECROMICON,
    MODEL_POTION + 100,
    MODEL_POTION + 111,
    MODEL_SILVER_KEY,
    MODEL_GOLD_KEY,
    MODEL_GOBLIN_GOLD_COIN,
    MODEL_SEALED_GOLDEN_BOX,
    MODEL_SEALED_SILVER_BOX,
    MODEL_GOLDEN_BOX,
    MODEL_SILVER_BOX,
    MODEL_PACKAGE_BOX_A,
    MODEL_PACKAGE_BOX_B,
    MODEL_PACKAGE_BOX_C,
    MODEL_PACKAGE_BOX_D,
    MODEL_PACKAGE_BOX_E,
    MODEL_PACKAGE_BOX_F,
    MODEL_MASTER_SKILL_RESET,
    MODEL_STAT_ADJUSTMENT,
    MODEL_CHARACTER_RELOCATION_SERVICE,
    MODEL_CHARACTER_RENAME_SERVICE,
    MODEL_SERVER_RELOCATION_SERVICE,
    MODEL_PREMIUM_PACKAGE,
    MODEL_30_DAY_PASS,
    MODEL_90_DAY_PASS,
    MODEL_30_DAY_PASS_POINTS,
    MODEL_90_DAY_PASS_POINTS,
    MODEL_3_HOUR_PASS,
    MODEL_5_HOUR_PASS,
    MODEL_10_HOUR_PASS,
    MODEL_HELPER + 91,
    MODEL_MAGIC_GLADIATOR_CHARACTER_CARD,
    MODEL_DARK_LORD_CHARACTER_CARD,
    MODEL_HELPER + 99,
    MODEL_SAPPHIRE_RING,
    MODEL_RUBY_RING,
    MODEL_TOPAZ_RING,
    MODEL_AMETHYST_RING,
    MODEL_RUBY_NECKLACE,
    MODEL_EMERALD_NECKLACE,
    MODEL_SAPPHIRE_NECKLACE,
    MODEL_OPEN_ACCESS_TICKET_TO_CHAOS_CASTLE,
    MODEL_WING + 25,
    MODEL_LOST_MAP,
    MODEL_SYMBOL_OF_KUNDUN,
};

std::unordered_set<int> orangeTextItems = {
    MODEL_CHERRY_BLOSSOM_PLAYBOX,
    MODEL_CHERRY_BLOSSOM_WINE,
    MODEL_CHERRY_BLOSSOM_RICE_CAKE,
    MODEL_CHERRY_BLOSSOM_FLOWER_PETAL,
    MODEL_GOLDEN_CHERRY_BLOSSOM_BRANCH,
    MODEL_DEMON,
    MODEL_SPIRIT_OF_GUARDIAN,
    MODEL_PET_RUDOLF,
    MODEL_PET_PANDA,
    MODEL_PET_UNICORN,
    MODEL_PET_SKELETON,
    MODEL_SNOWMAN_TRANSFORMATION_RING,
    MODEL_PANDA_TRANSFORMATION_RING,
    MODEL_SKELETON_TRANSFORMATION_RING,
    MODEL_PUMPKIN_OF_LUCK,
    MODEL_JACK_OLANTERN_BLESSINGS,
    MODEL_JACK_OLANTERN_WRATH,
    MODEL_JACK_OLANTERN_CRY,
    MODEL_JACK_OLANTERN_FOOD,
    MODEL_JACK_OLANTERN_DRINK,
    MODEL_SEAL_OF_ASCENSION,
    MODEL_SEAL_OF_WEALTH,
    MODEL_SEAL_OF_SUSTENANCE,
    MODEL_DEVIL_SQUARE_TICKET,
    MODEL_BLOOD_CASTLE_TICKET,
    MODEL_KALIMA_TICKET,
    MODEL_RESET_FRUIT_STRENGTH,
    MODEL_RESET_FRUIT_QUICKNESS,
    MODEL_RESET_FRUIT_HEALTH,
    MODEL_RESET_FRUIT_ENERGY,
    MODEL_RESET_FRUIT_CONTROL,
    MODEL_SEAL_OF_MOBILITY,
    MODEL_INDULGENCE,
    MODEL_ILLUSION_TEMPLE_TICKET,
    MODEL_SEAL_OF_HEALING,
    MODEL_SEAL_OF_DIVINITY,
    MODEL_HELPER + 116,
    MODEL_OPEN_ACCESS_TICKET_TO_DOPPELGANGER,
    MODEL_OPEN_ACCESS_TICKET_TO_VARKA,
    MODEL_OPEN_ACCESS_TICKET_TO_VARKA_7,
    MODEL_HELPER + 128,
    MODEL_HELPER + 129,
    MODEL_HELPER + 130,
    MODEL_HELPER + 131,
    MODEL_HELPER + 132,
    MODEL_HELPER + 133,
    MODEL_HELPER + 134,
    MODEL_TALISMAN_OF_LUCK,
    MODEL_CHAOS_CARD,
    MODEL_RARE_ITEM_TICKET_1,
    MODEL_RARE_ITEM_TICKET_2,
    MODEL_RARE_ITEM_TICKET_3,
    MODEL_RARE_ITEM_TICKET_4,
    MODEL_RARE_ITEM_TICKET_5,
    MODEL_ELITE_HEALING_POTION,
    MODEL_ELITE_MANA_POTION,
    MODEL_SCROLL_OF_QUICKNESS,
    MODEL_SCROLL_OF_DEFENSE,
    MODEL_SCROLL_OF_WRATH,
    MODEL_SCROLL_OF_WIZARDRY,
    MODEL_SCROLL_OF_HEALTH,
    MODEL_SCROLL_OF_MANA,
    MODEL_ELIXIR_OF_STRENGTH,
    MODEL_ELIXIR_OF_AGILITY,
    MODEL_ELIXIR_OF_HEALTH,
    MODEL_ELIXIR_OF_ENERGY,
    MODEL_ELIXIR_OF_CONTROL,
    MODEL_POTION + 83,
    MODEL_WHITE_CHERRY_BLOSSOM_BRANCH,
    MODEL_RED_CHERRY_BLOSSOM_BRANCH,
    MODEL_SUMMONER_CHARACTER_CARD,
    MODEL_CHAOS_CARD_GOLD,
    MODEL_CHAOS_CARD_RARE,
    MODEL_MEDIUM_ELITE_HEALING_POTION,
    MODEL_CHAOS_CARD_MINI,
    MODEL_TALISMAN_OF_CHAOS_ASSEMBLY,
    MODEL_SCROLL_OF_BATTLE,
    MODEL_SCROLL_OF_STRENGTH,
    MODEL_POTION + 141,
    MODEL_POTION + 142,
    MODEL_POTION + 143,
    MODEL_POTION + 144,
    MODEL_RARE_ITEM_TICKET_7,
    MODEL_RARE_ITEM_TICKET_8,
    MODEL_RARE_ITEM_TICKET_9,
    MODEL_RARE_ITEM_TICKET_10,
    MODEL_RARE_ITEM_TICKET_11,
    MODEL_RARE_ITEM_TICKET_12,
    MODEL_SMALL_CAPE_OF_LORD,
    MODEL_SMALL_WING_OF_CURSE,
    MODEL_SMALL_WINGS_OF_ELF,
    MODEL_SMALL_WINGS_OF_HEAVEN,
    MODEL_SMALL_WINGS_OF_SATAN,
    MODEL_LITTLE_WARRIORS_CLOAK,
};

namespace
{
struct GroundItemLabelDescriptor
{
    wchar_t Name[80]{};
    HFONT Font = g_hFont;
    DWORD TextColor = 0xFFFFFFFF;
    DWORD BgColor = 0xFF000000;
};

DWORD MakeRgba(BYTE red, BYTE green, BYTE blue, BYTE alpha = 255)
{
    return red + (green << 8) + (blue << 16) + (alpha << 24);
}

void SetDescriptorTextColor(GroundItemLabelDescriptor& descriptor, float red, float green, float blue)
{
    descriptor.TextColor = MakeRgba(
        static_cast<BYTE>(red * 255.f),
        static_cast<BYTE>(green * 255.f),
        static_cast<BYTE>(blue * 255.f),
        255);
}

void SetDescriptorYellowTextColor(GroundItemLabelDescriptor& descriptor)
{
    SetDescriptorTextColor(descriptor, 1.f, 0.8f, 0.1f);
}

void SetDescriptorGrayTextColor(GroundItemLabelDescriptor& descriptor)
{
    SetDescriptorTextColor(descriptor, 0.7f, 0.7f, 0.7f);
}

void SetDescriptorOrangeTextColor(GroundItemLabelDescriptor& descriptor)
{
    SetDescriptorTextColor(descriptor, 0.9f, 0.53f, 0.13f);
}

template <size_t BufferSize, typename... Args>
void FormatGroundItemLabelText(wchar_t(&buffer)[BufferSize], const wchar_t* format, Args... args)
{
    _snwprintf_s(buffer, BufferSize, _TRUNCATE, format, args...);
}

template <size_t BufferSize>
void CopyGroundItemLabelText(wchar_t(&buffer)[BufferSize], const wchar_t* value)
{
    FormatGroundItemLabelText(buffer, L"%ls", value != nullptr ? value : L"");
}

template <size_t BufferSize, typename... Args>
void AppendGroundItemLabelText(wchar_t(&buffer)[BufferSize], const wchar_t* format, Args... args)
{
    const size_t currentLength = wcslen(buffer);
    if (currentLength >= BufferSize - 1)
    {
        return;
    }

    _snwprintf_s(buffer + currentLength, BufferSize - currentLength, _TRUNCATE, format, args...);
}

void BuildGroundItemLabelDescriptor(OBJECT* o, ITEM* ip, GroundItemLabelDescriptor& descriptor)
{
    auto ItemLevel = ip->Level;
    auto ItemOption = ip->ExcellentFlags;

    descriptor.Font = g_hFont;
    descriptor.TextColor = MakeRgba(255, 255, 255, 255);
    descriptor.BgColor = MakeRgba(0, 0, 0, 255);

    // The item's name (its name lines), as windows and messages write it; Zen shows its amount. A ground item keeps its
    // item type and level when it is drawn as an event model (MODEL_EVENT + N).
    if (o->Type == MODEL_ZEN)
    {
        FormatGroundItemLabelText(descriptor.Name, L"%ls %d", ItemAttribute[ITEM_ZEN].Name, ItemLevel);
    }
    else
    {
        CopyGroundItemLabelText(descriptor.Name, GetItemNameText(ip->Type, ItemLevel, ip).c_str());
    }

    if (boldTextItems.count(o->Type) > 0)
    {
        descriptor.Font = g_hFontBold;
    }

    if (whiteTextItems.count(o->Type) > 0)
    {
        SetDescriptorTextColor(descriptor, 1.f, 1.f, 1.f);
    }
    else if (yellowTextItems.count(o->Type) > 0)
    {
        SetDescriptorYellowTextColor(descriptor);
    }
    else if (orangeTextItems.count(o->Type) > 0)
    {
        SetDescriptorOrangeTextColor(descriptor);
    }
    if (o->Type == MODEL_ORB_OF_SUMMONING)
    {
        SetDescriptorGrayTextColor(descriptor);
    }
    else if (COMGEM::NOGEM != COMGEM::Check_Jewel_Com(o->Type, true))
    {
        descriptor.Font = g_hFontBold;
        SetDescriptorYellowTextColor(descriptor);
    }
    else if ((o->Type == MODEL_BOX_OF_LUCK && ItemLevel == 7) || o->Type == MODEL_POTION + 12 ||
             o->Type == MODEL_FRUITS || o->Type == MODEL_SPIRIT ||
             (o->Type >= MODEL_EVENT + 4 && o->Type <= MODEL_EVENT + 5) ||
             (o->Type >= MODEL_EVENT + 7 && o->Type <= MODEL_EVENT + 16) || o->Type == MODEL_EVENT + 18 ||
             o->Type == MODEL_TRANSFORMATION_RING || o->Type == MODEL_SIEGE_POTION ||
             o->Type == MODEL_CONTRACT_SUMMON || o->Type == MODEL_LIFE_STONE_ITEM)
    {
        // White, the label's color.
    }
    else if (o->Type == MODEL_EVENT + 6)
    {
        if (ItemLevel == 13)
        {
            SetDescriptorYellowTextColor(descriptor);
        }
    }
    else if (o->Type == MODEL_RED_RIBBON_BOX)
    {
        SetDescriptorTextColor(descriptor, 1.f, 0.3f, 0.3f);
    }
    else if (o->Type == MODEL_GREEN_RIBBON_BOX)
    {
        SetDescriptorTextColor(descriptor, 0.3f, 1.0f, 0.3f);
    }
    else if (o->Type == MODEL_BLUE_RIBBON_BOX)
    {
        SetDescriptorTextColor(descriptor, 0.3f, 0.3f, 1.f);
    }
    else if (o->Type == MODEL_PINK_CHOCOLATE_BOX)
    {
        if (ItemLevel == 0)
        {
            SetDescriptorTextColor(descriptor, 1.f, 0.3f, 1.f);
        }
        else if (ItemLevel == 1)
        {
            SetDescriptorTextColor(descriptor, 1.f, 0.3f, 1.f);
        }
    }
    else if (o->Type == MODEL_RED_CHOCOLATE_BOX)
    {
        if (ItemLevel == 0)
        {
            SetDescriptorTextColor(descriptor, 1.0f, 0.3f, 0.3f);
        }
        else if (ItemLevel == 1)
        {
            SetDescriptorTextColor(descriptor, 1.0f, 0.3f, 0.3f);
        }
    }
    else if (o->Type == MODEL_BLUE_CHOCOLATE_BOX)
    {
        if (ItemLevel == 0)
        {
            SetDescriptorTextColor(descriptor, 0.3f, 0.3f, 1.f);
        }
        else if (ItemLevel == 1)
        {
            SetDescriptorTextColor(descriptor, 0.3f, 0.3f, 1.f);
        }
    }
    else if (o->Type == MODEL_EVENT + 21)
    {
        SetDescriptorTextColor(descriptor, 1.f, 0.3f, 1.f);
    }
    else if (o->Type == MODEL_EVENT + 22)
    {
        SetDescriptorTextColor(descriptor, 1.0f, 0.3f, 0.3f);
    }
    else if (o->Type == MODEL_EVENT + 23)
    {
        SetDescriptorTextColor(descriptor, 0.3f, 0.3f, 1.f);
    }
    else if (o->Type == MODEL_RENA && ItemLevel == 3)
    {
        SetDescriptorYellowTextColor(descriptor);
    }
    else if (GameLogic::Items::IsSocketSeedOrSphereModel(o->Type))
    {
        SetDescriptorTextColor(descriptor, 0.7f, 0.4f, 1.0f);
    }
    else if (o->Type == MODEL_INVITATION_TO_SANTA_VILLAGE)
    {
        SetDescriptorTextColor(descriptor, 0.6f, 0.4f, 1.0f);
    }
    else if (GameLogic::Items::IsWingMixCharmModel(o->Type))
    {
        SetDescriptorOrangeTextColor(descriptor);
    }
    else if (Data::Items::IsItemModelSlot(o->Type) &&
             (whiteTextItems.count(o->Type) > 0 || yellowTextItems.count(o->Type) > 0 ||
              orangeTextItems.count(o->Type) > 0))
    {
        // Color was already set by Block 1 (white/yellow/orange). No override needed.
    }
    else
    {
        if (GameLogic::Items::IsDivineArchangelWeaponModel(o->Type))
        {
            SetDescriptorTextColor(descriptor, 1.f, 0.1f, 1.f);
        }
        else if (g_SocketItemMgr.IsSocketItem(o))
        {
            SetDescriptorTextColor(descriptor, 0.7f, 0.4f, 1.0f);
        }
        else if ((ItemOption & 63) > 0 && (o->Type < MODEL_WINGS_OF_SPIRITS || o->Type > MODEL_WINGS_OF_DARKNESS) && o->Type != MODEL_CAPE_OF_LORD
            && (o->Type < MODEL_WING_OF_STORM || o->Type > MODEL_CAPE_OF_EMPEROR)
            && (o->Type < MODEL_WINGS_OF_DESPAIR || o->Type > MODEL_WING_OF_DIMENSION)
            && !(o->Type >= MODEL_CAPE_OF_FIGHTER && o->Type <= MODEL_CAPE_OF_OVERRULE))
        {
            SetDescriptorTextColor(descriptor, 0.1f, 1.f, 0.5f);
        }
        else if (ItemLevel >= 7)
        {
            SetDescriptorYellowTextColor(descriptor);
        }
        else if (ip->HasSkill || ip->HasLuck || ip->OptionLevel > 0)
        {
            SetDescriptorTextColor(descriptor, 0.4f, 0.7f, 1.f);
        }
        else if (ItemLevel == 0)
        {
            SetDescriptorGrayTextColor(descriptor);
        }
        else if (ItemLevel < 3)
        {
            SetDescriptorTextColor(descriptor, 0.9f, 0.9f, 0.9f);
        }
        else if (ItemLevel < 5)
        {
            SetDescriptorTextColor(descriptor, 1.f, 0.5f, 0.2f);
        }
        else if (ItemLevel < 7)
        {
            SetDescriptorTextColor(descriptor, 0.4f, 0.7f, 1.f);
        }

        wchar_t SetName[64]{};
        if (g_csItemOption.GetSetItemName(SetName, Data::Items::ToItemType(o->Type), ip->AncientDiscriminator))
        {
            SetDescriptorTextColor(descriptor, 0.f, 1.f, 0.f);
            descriptor.Font = g_hFontBold;
            descriptor.TextColor = MakeRgba(0, 255, 0, 255);
            descriptor.BgColor = MakeRgba(60, 60, 200, 255);

            wchar_t formattedName[_countof(descriptor.Name)]{};
            FormatGroundItemLabelText(formattedName, L"%ls%ls", SetName, descriptor.Name);
            CopyGroundItemLabelText(descriptor.Name, formattedName);
        }

        if (ip->HasSkill)
        {
            if (o->Type != MODEL_HORN_OF_DINORANT)
            {
                AppendGroundItemLabelText(descriptor.Name, L"%ls", I18N::Game::Skill);
            }
            else
            {
                AppendGroundItemLabelText(descriptor.Name, L"%ls", L" +");
                AppendGroundItemLabelText(descriptor.Name, L"%ls", I18N::Game::KnightSpecificSkill);
            }
        }
        if (ip->OptionLevel > 0)
        {
            AppendGroundItemLabelText(descriptor.Name, L"%ls", I18N::Game::Option);
        }
        if (ip->HasLuck)
        {
            AppendGroundItemLabelText(descriptor.Name, L"%ls", I18N::Game::Luck);
        }
    }
}

void ApplyGroundItemLabelDescriptor(const GroundItemLabelDescriptor& descriptor)
{
    g_pRenderText->SetFont(descriptor.Font);
    g_pRenderText->SetTextColor(descriptor.TextColor);
    g_pRenderText->SetBgColor(descriptor.BgColor);
}

}

GroundItemLabel DescribeGroundItemLabel(OBJECT* o, ITEM* ip)
{
    GroundItemLabelDescriptor descriptor;
    BuildGroundItemLabelDescriptor(o, ip, descriptor);
    return {descriptor.Name, descriptor.TextColor, descriptor.BgColor, descriptor.Font == g_hFontBold};
}

void RenderItemName(int i, OBJECT* o, ITEM* ip, bool Sort)
{
    (void)i;

    if (!Sort)
    {
        GroundItemLabelDescriptor descriptor;
        BuildGroundItemLabelDescriptor(o, ip, descriptor);
        ApplyGroundItemLabelDescriptor(descriptor);
        g_pRenderText->RenderText(MouseX, MouseY - 15, descriptor.Name, 0, 0, RT3_WRITE_CENTER);
    }
    else
    {
        GroundItemLabelDescriptor descriptor;
        BuildGroundItemLabelDescriptor(o, ip, descriptor);
        ApplyGroundItemLabelDescriptor(descriptor);
        g_pRenderText->RenderText(o->ScreenX, o->ScreenY - 15, descriptor.Name, 0, 0, RT3_WRITE_CENTER);
    }

    g_pRenderText->SetTextColor(255, 230, 200, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 255);
}

int GetScreenWidth()
{
    if (WindowWidth == 0)
        return REFERENCE_WIDTH;

    // Right edge of the world the open docked windows leave uncovered, in the 640-wide HUD space.
    return static_cast<int>(
        std::lround(UI::Placement::UncoveredWorldRight() * REFERENCE_WIDTH / static_cast<float>(WindowWidth)));
}

int GetScreenLeft()
{
    if (WindowWidth == 0)
        return 0;

    // Left edge of the world the open docked windows leave uncovered, in the 640-wide HUD space.
    return static_cast<int>(
        std::lround(UI::Placement::UncoveredWorldLeft() * REFERENCE_WIDTH / static_cast<float>(WindowWidth)));
}

void ClearInventory()
{
    for (int i = 0; i < MAX_EQUIPMENT; i++)
    {
        CharacterMachine->Equipment[i].Type = -1;
        CharacterMachine->Equipment[i].Number = 0;
    }
    for (int i = 0; i < MAX_INVENTORY; i++)
    {
        Inventory[i].Type = -1;
        Inventory[i].Number = 0;
    }
    for (int i = 0; i < MAX_INVENTORY_EXT; i++)
    {
        InventoryExt[i].Type = -1;
        InventoryExt[i].Number = 0;
    }
    for (int i = 0; i < MAX_SHOP_INVENTORY; i++)
    {
        ShopInventory[i].Type = -1;
        ShopInventory[i].Number = 0;
    }

    COMGEM::Init();
}

void SetItemColor(int index, ITEM* Inv, int color)
{
    int Width = ItemAttribute[Inv[index].Type].Width;
    int Height = ItemAttribute[Inv[index].Type].Height;

    for (int k = Inv[index].y; k < Inv[index].y + Height; k++)
    {
        for (int l = Inv[index].x; l < Inv[index].x + Width; l++)
        {
            int Number = k * COLUMN_TRADE_INVENTORY + l;
            Inv[Number].Color = color;
        }
    }
}

int CompareItem(ITEM item1, ITEM item2)
{
    int equal = 0;
    if (item1.Type != item2.Type)
    {
        return  2;
    }
    else
        if (item1.Class == item2.Class && item1.Type == item2.Type)
        {
            int level1 = item1.Level;
            int level2 = item2.Level;
            int option1 = item1.ExcellentFlags;
            int option2 = item2.ExcellentFlags;
            bool skill1 = item1.HasSkill;
            bool skill2 = item2.HasSkill;

            equal = 1;
            if (level1 == level2)
            {
                equal = 0;
            }
            else
                if (level1 < level2)
                {
                    equal = -1;
                }
            if (equal == 0)
            {
                if (skill1 < skill2)
                {
                    equal = -1;
                }
                else if (skill1 > skill2)
                {
                    equal = 1;
                }
            }
            if (equal == 0)
            {
                if (option1 < option2)
                {
                    equal = -1;
                }
                else if (option1 > option2)
                {
                    equal = 1;
                }
            }
            if (equal == 0)
            {
                if (item1.SpecialNum < item2.SpecialNum)
                {
                    equal = -1;
                }
                else if (item1.SpecialNum > item2.SpecialNum)
                {
                    equal = 1;
                }
                else
                {
                    int     Num = std::max<int>(item1.SpecialNum, item2.SpecialNum);
                    int     addOption1 = 0;
                    int     addOptionV1 = 0;
                    int     addOption2 = 0;
                    int     addOptionV2 = 0;
                    for (int i = 0; i < Num; ++i)
                    {
                        switch (item1.Special[i])
                        {
                        case AT_IMPROVE_DAMAGE:
                        case AT_IMPROVE_MAGIC:
                        case AT_IMPROVE_CURSE:
                        case AT_IMPROVE_BLOCKING:
                        case AT_IMPROVE_DEFENSE:
                            addOption1 = 1;
                            addOptionV1 = item1.SpecialValue[i];
                            break;
                        }
                        switch (item2.Special[i])
                        {
                        case AT_IMPROVE_DAMAGE:
                        case AT_IMPROVE_MAGIC:
                        case AT_IMPROVE_CURSE:
                        case AT_IMPROVE_BLOCKING:
                        case AT_IMPROVE_DEFENSE:
                            addOption2 = 1;
                            addOptionV2 = item2.SpecialValue[i];
                            break;
                        }
                    }

                    if (addOption1 < addOption2 || addOptionV1 < addOptionV2)
                    {
                        equal = -1;
                    }
                    else if (addOption1 != addOption2 && addOptionV1 != addOptionV2)
                    {
                        equal = 1;
                    }
                }
            }
            if (equal == 0)
            {
                if (item1.Durability < item2.Durability)
                    equal = -1;
                if (item1.Durability > item2.Durability)
                    equal = 1;
            }
        }
    return  equal;
}

bool EquipmentItem = false;
extern int BuyCost;

bool IsExcellentItem(ITEM* pItem)
{
    return pItem->ExcellentFlags;
}

bool IsAncientItem(ITEM* pItem)
{
    return pItem->AncientDiscriminator;
}

bool IsMoneyItem(ITEM* pItem)
{
    return pItem->Type == ITEM_ZEN;
}

namespace
{
void AppendItemOptionKeywords(const ITEM& item, std::wstring& options)
{
    if (item.HasSkill)
    {
        options += I18N::Game::Skill;
    }
    if (item.OptionLevel)
    {
        options += I18N::Game::Option;
    }
    if (item.HasLuck)
    {
        options += I18N::Game::Luck;
    }
}
} // namespace

std::wstring GetItemDisplayName(ITEM* pItem)
{
    std::wstring strDisplayName = GetItemName(pItem->Type, pItem->Level);
    std::wstring strOptions;

    if (pItem->Level)
    {
        const std::wstring levelSuffix = L" +" + std::to_wstring(pItem->Level);
        // Keep ordinary equipment keywords contiguous without treating variant levels as upgrades.
        if (strDisplayName.ends_with(levelSuffix))
        {
            strDisplayName.resize(strDisplayName.size() - levelSuffix.size());
            strOptions = levelSuffix.substr(1);
        }
    }
    AppendItemOptionKeywords(*pItem, strOptions);
    if (!strOptions.empty())
    {
        strDisplayName += L" ";
        strDisplayName += strOptions;
    }
    return strDisplayName;
}

OBJECT ObjectSelect;

namespace
{
// Degrees per millisecond of WorldTime.
constexpr float GambleItemTurnSpeed = 0.2f;
constexpr float SelectedItemTurnSpeed = 0.45f;
} // namespace

void RenderObjectScreen(int Type, int ItemLevel, int excellentFlags, int ancientDiscriminator, vec3_t Target,
                        int Select, bool PickUp)
{
    int Level = ItemLevel;
    vec3_t Direction, Position;

    VectorSubtract(Target, MousePosition, Direction);
    if (PickUp)
        VectorMA(MousePosition, 0.07f, Direction, Position);
    else
        VectorMA(MousePosition, 0.1f, Direction, Position);

    Type = Render::Items::Display::GetDrawnModel(Type, Level);

    const Render::Items::Display::InventoryDisplay display = Render::Items::Display::GetInventoryDisplay(Type);
    Vector(display.rotation[0], display.rotation[1], display.rotation[2], ObjectSelect.Angle);

    // Gamble items turn slowly, and every item turns while the mouse is on it.
    if (GameLogic::Items::IsGambleItemModel(Type) && Select != 1)
    {
        ObjectSelect.Angle[1] = WorldTime * GambleItemTurnSpeed;
    }
    if (1 == Select)
    {
        ObjectSelect.Angle[1] = WorldTime * SelectedItemTurnSpeed;
    }

    ObjectSelect.Type = Render::Items::Display::IsDrawnOnCharacterSkeleton(Type) ? MODEL_PLAYER : Type;

    BMD* b = &Models[ObjectSelect.Type];
    b->CurrentAction = 0;
    ObjectSelect.AnimationFrame = 0;
    ObjectSelect.PriorAnimationFrame = 0;
    ObjectSelect.PriorAction = 0;
    b->BodyHeight = display.bodyHeight;

    float Scale = display.scale;
    if (const std::optional<float> smallScale = Render::Items::Display::GetSmallArchangelWeaponScale(Type, ItemLevel))
    {
        Scale = *smallScale;
        ItemLevel = 0;
    }

    VectorAdd(Position, display.offset.data(), Position);

    b->Animation(BoneTransform, ObjectSelect.AnimationFrame, ObjectSelect.PriorAnimationFrame, ObjectSelect.PriorAction, ObjectSelect.Angle, ObjectSelect.HeadAngle, false, false);

    CHARACTER Armor;
    OBJECT* o = &Armor.Object;
    o->Type = Type;
    ItemObjectAttribute(o);
    o->LightEnable = false;
    Armor.Class = CLASS_ELF; // ??

#ifdef PBG_ADD_ITEMRESIZE
    int ScreenPos_X = 0, ScreenPos_Y = 0;
    CameraProjection::WorldToScreen(g_Camera, Position, &ScreenPos_X, &ScreenPos_Y);
#endif //PBG_ADD_ITEMRESIZE

    o->Scale = Scale;

    VectorCopy(Position, o->Position);

    vec3_t Light;
    float  alpha = o->Alpha;

    Vector(1.f, 1.f, 1.f, Light);

    RenderPartObject(o, Type, NULL, Light, alpha, ItemLevel, excellentFlags, ancientDiscriminator, true, true, true);
}

bool UI::Items::ShouldAnimatePreview(bool pointerInside, bool pickedItemActive, bool renderingPickedItem)
{
    return pointerInside && (!pickedItemActive || renderingPickedItem);
}

static void RenderItem3DAt(float sx, float sy, float Width, float Height, int Type, int Level, int excellentFlags,
                           int ancientDiscriminator, bool PickUp, bool hovered);

void RenderItem3D(float sx, float sy, float Width, float Height, int Type, int Level, int excellentFlags, int ancientDiscriminator, bool PickUp)
{
    const bool hovered = g_fWindowMouseX >= sx && g_fWindowMouseX < sx + Width && g_fWindowMouseY >= sy &&
                         g_fWindowMouseY < sy + Height;
    RenderItem3DAt(sx, sy, Width, Height, Type, Level, excellentFlags, ancientDiscriminator, PickUp, hovered);
}

void RenderItem3DWithHover(float sx, float sy, float Width, float Height, int Type, int Level, int excellentFlags,
                           int ancientDiscriminator, bool hovered)
{
    RenderItem3DAt(sx, sy, Width, Height, Type, Level, excellentFlags, ancientDiscriminator, false, hovered);
}

static void RenderItem3DAt(float sx, float sy, float Width, float Height, int Type, int Level, int excellentFlags,
                           int ancientDiscriminator, bool PickUp, bool hovered)
{
    // Item previews are screen-space: keep Classic texture sampling.
    const mu::ScopedScreenSpaceSampling screenSpaceSampling;

    const bool Success = UI::Items::ShouldAnimatePreview(hovered, g_pPickedItem != nullptr, PickUp);

    const Render::Items::Display::Anchor anchor = Render::Items::Display::GetInventoryAnchor(Type, Level);
    sx += Width * anchor.x;
    sy += Height * anchor.y;

    vec3_t Position;
    CameraProjection::ScreenToWorldRay(g_Camera, (int)(sx), (int)(sy), Position, false);
    // The Lucky Coin turns as its registration window says.
    if (Type == ITEM_POTION + 100)
    {
        bool _Angle;
        if (g_pLuckyCoinRegistration->GetItemRotation())
        {
            _Angle = true;
        }
        else
        {
            _Angle = Success;
        }

        RenderObjectScreen(MODEL_POTION + 100, Level, excellentFlags, ancientDiscriminator, Position, _Angle, PickUp);
    }
    else if (const std::optional<Render::Items::Display::InventoryModel> model =
                 Render::Items::Display::FindInventoryModel(Type, Level))
    {
        RenderObjectScreen(model->modelType, model->drawLevel, excellentFlags, ancientDiscriminator, Position, Success,
                           PickUp);
    }
}

void InventoryColor(ITEM* p)
{
    (void)p;
}

void RenderEqiupmentBox()
{
    int StartX = InventoryStartX;
    int StartY = InventoryStartY;
    float x, y, Width, Height;

    EnableAlphaTest();

    //helper
    Width = 40.f; Height = 40.f; x = 15.f; y = 46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_HELPER]);
    RenderBitmap(BITMAP_INVENTORY + 15, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    //wing
    Width = 60.f; Height = 40.f; x = 115.f; y = 46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WING]);
    RenderBitmap(BITMAP_INVENTORY + 14, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    if (gCharacterManager.GetBaseClass(CharacterAttribute->Class) != CLASS_DARK)
    {
        //helmet
        Width = 40.f; Height = 40.f; x = 75.f; y = 46.f;
        InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_HELM]);
        RenderBitmap(BITMAP_INVENTORY + 3, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    }
    //armor upper
    Width = 40.f; Height = 60.f; x = 75.f; y = 89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_ARMOR]);
    RenderBitmap(BITMAP_INVENTORY + 4, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    //armor lower
    //if(GetBaseClass(CharacterAttribute->Class) != CLASS_ELF)
    {
        Width = 40.f; Height = 40.f; x = 75.f; y = 152.f;
        InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_PANTS]);
        RenderBitmap(BITMAP_INVENTORY + 5, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    }
    //weapon right
    Width = 40.f; Height = 60.f; x = 15.f; y = 89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT]);
    RenderBitmap(BITMAP_INVENTORY + 6, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    //weapon left
    Width = 40.f; Height = 60.f; x = 134.f; y = 89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT]);
    RenderBitmap(BITMAP_INVENTORY + 16, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    //glove
    if (gCharacterManager.GetBaseClass(CharacterAttribute->Class) != CLASS_RAGEFIGHTER)
    {
        Width = 40.f; Height = 40.f; x = 15.f; y = 152.f;
        InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_GLOVES]);
        RenderBitmap(BITMAP_INVENTORY + 7, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    }

    //boot
    Width = 40.f; Height = 40.f; x = 134.f; y = 152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_BOOTS]);
    RenderBitmap(BITMAP_INVENTORY + 8, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 64.f, Height / 64.f);
    //necklace
    Width = 20.f; Height = 20.f; x = 55.f; y = 89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_AMULET]);
    RenderBitmap(BITMAP_INVENTORY + 9, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 32.f, Height / 32.f);
    //ring
    Width = 20.f; Height = 20.f; x = 55.f; y = 152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT]);
    RenderBitmap(BITMAP_INVENTORY + 10, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 32.f, Height / 32.f);
    //ring
    Width = 20.f; Height = 20.f; x = 115.f; y = 152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_RING_LEFT]);
    RenderBitmap(BITMAP_INVENTORY + 10, x + StartX, y + StartY, Width, Height, 0.f, 0.f, Width / 32.f, Height / 32.f);
}

void RenderEqiupmentPart3D(int Index, float sx, float sy, float Width, float Height)
{
    ITEM* p = &CharacterMachine->Equipment[Index];
    if (p->Type != -1)
    {
        if (p->Number > 0)
            RenderItem3D(sx, sy, Width, Height, p->Type, p->Level, p->ExcellentFlags, p->AncientDiscriminator, false);
    }
}

void RenderEqiupment3D()
{
    int StartX = InventoryStartX;
    int StartY = InventoryStartY;
    float x, y, Width, Height;

    //helper
    Width = 40.f; Height = 40.f; x = 15.f; y = 46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_HELPER]);
    RenderEqiupmentPart3D(EQUIPMENT_HELPER, StartX + x, StartY + y, Width, Height);
    //wing
    Width = 60.f; Height = 40.f; x = 115.f; y = 46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WING]);
    RenderEqiupmentPart3D(EQUIPMENT_WING, StartX + x, StartY + y, Width, Height);
    //helmet
    Width = 40.f; Height = 40.f; x = 75.f; y = 46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_HELM]);
    RenderEqiupmentPart3D(EQUIPMENT_HELM, StartX + x, StartY + y, Width, Height);
    //armor upper
    Width = 40.f; Height = 60.f; x = 75.f; y = 89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_ARMOR]);
    RenderEqiupmentPart3D(EQUIPMENT_ARMOR, StartX + x, StartY + y - 10, Width, Height);
    //armor lower
    //if(GetBaseClass(CharacterAttribute->Class) != CLASS_ELF)
    {
        Width = 40.f; Height = 40.f; x = 75.f; y = 152.f;
        InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_PANTS]);
        RenderEqiupmentPart3D(EQUIPMENT_PANTS, StartX + x, StartY + y, Width, Height);
    }
    //weapon right
    Width = 40.f; Height = 60.f; x = 15.f; y = 89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT]);
    RenderEqiupmentPart3D(EQUIPMENT_WEAPON_RIGHT, StartX + x, StartY + y, Width, Height);
    //weapon left
    Width = 40.f; Height = 60.f; x = 134.f; y = 89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT]);
    RenderEqiupmentPart3D(EQUIPMENT_WEAPON_LEFT, StartX + x, StartY + y, Width, Height);
    //glove
    Width = 40.f; Height = 40.f; x = 15.f; y = 152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_GLOVES]);
    RenderEqiupmentPart3D(EQUIPMENT_GLOVES, StartX + x, StartY + y, Width, Height);
    //boot
    Width = 40.f; Height = 40.f; x = 134.f; y = 152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_BOOTS]);
    RenderEqiupmentPart3D(EQUIPMENT_BOOTS, StartX + x, StartY + y, Width, Height);
    //necklace
    Width = 20.f; Height = 20.f; x = 55.f; y = 89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_AMULET]);
    RenderEqiupmentPart3D(EQUIPMENT_AMULET, StartX + x, StartY + y, Width, Height);
    //ring
    Width = 20.f; Height = 20.f; x = 55.f; y = 152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT]);
    RenderEqiupmentPart3D(EQUIPMENT_RING_RIGHT, StartX + x, StartY + y, Width, Height);
    //ring
    Width = 20.f; Height = 20.f; x = 115.f; y = 152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_RING_LEFT]);
    RenderEqiupmentPart3D(EQUIPMENT_RING_LEFT, StartX + x, StartY + y, Width, Height);
}

bool CheckEmptyInventory(ITEM* Inv, int InvWidth, int InvHeight)
{
    bool Empty = true;
    for (int y = 0; y < InvHeight; y++)
    {
        for (int x = 0; x < InvWidth; x++)
        {
            int Index = y * InvWidth + x;
            ITEM* p = &Inv[Index];
            if (p->Type != -1 && p->Number > 0)
            {
                Empty = false;
            }
        }
    }
    return Empty;
}

void InitPartyList()
{
    PartyNumber = 0;
    PartyKey = 0;
}

void ClosePersonalShop()
{
    if (g_iPShopWndType == PSHOPWNDTYPE_PURCHASE)
    {
        memcpy(g_PersonalShopInven, g_PersonalShopBackup, sizeof(ITEM) * MAX_PERSONALSHOP_INVEN);
        if (IsShopInViewport(Hero))
        {
            std::wstring title{};
            GetShopTitle(Hero, title);
            wcscpy(g_szPersonalShopTitle, title.c_str());
        }
        else
        {
            g_szPersonalShopTitle[0] = '\0';
        }
        if (g_PersonalShopSeller.Key)
        {
            SocketClient->ToGameServer()->SendPlayerShopCloseOther(g_PersonalShopSeller.Key, MU_C16(g_PersonalShopSeller.ID));
        }
    }

    g_PersonalShopSeller.Initialize();

    g_iPShopWndType = PSHOPWNDTYPE_NONE;
}
void ClearPersonalShop()
{
    g_bEnablePersonalShop = false;
    g_iPShopWndType = PSHOPWNDTYPE_NONE;
    g_iPersonalShopMsgType = 0;
    g_szPersonalShopTitle[0] = '\0';

    RemoveAllShopTitle();
}

bool IsExistUndecidedPrice()
{
    bool bResult = true;

    auto inventoryCtrl = g_pMyShopInventory->GetInventoryCtrl();
    for (int i = 0; i < MAX_PERSONALSHOP_INVEN; ++i)
    {
        int iPrice = 0;
        ITEM* pItem = inventoryCtrl->GetItem(i);
        if (pItem)
        {
            bResult = false;
            int iIndex = inventoryCtrl->GetIndexByItem(pItem);
            if (GetPersonalItemPrice(iIndex, iPrice, g_IsPurchaseShop) == false)
            {
                return true;
            }

            if (iPrice <= 0)
            {
                return true;
            }
        }
        else
        {
            continue;
        }
    }

    return bResult;
}

extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;

bool IsStrifeMap(int nMapIndex)
{
    bool bStrifeMap = false;

    if (!g_ServerListManager->IsNonPvP())
    {
        ENUM_WORLD aeStrife[1] = { WD_63PK_FIELD };
        int i;
        for (i = 0; i < 1; ++i)
        {
            if (aeStrife[i] == nMapIndex)
            {
                bStrifeMap = true;
                break;
            }
        }
    }
    return bStrifeMap;
}

unsigned int MarkColor[16];

// Writes one 32-bit pixel into a texture's byte buffer. It copies the bytes: writing through the buffer cast to an
// unsigned int* would treat the bytes as another type (type punning).
static void WriteMarkPixel(BYTE* pixel, unsigned int color)
{
    std::memcpy(pixel, &color, sizeof(color));
}

void CreateGuildMark(int nMarkIndex, bool blend)
{
    // Callers pass Hero->GuildMarkIndex straight through, which is -1 while the
    // hero has no guild (set on dissolve, and before GetGuildMarkIndex resolves
    // on create). Indexing GuildMark[-1] then reads its Mark[] from outside the
    // array and feeds garbage to the texture upload - undefined, and a crash on
    // Linux. Skip rendering when there is no valid mark.
    if (nMarkIndex < 0 || nMarkIndex >= MAX_MARKS)
        return;

    BITMAP_t* b = &Bitmaps[BITMAP_GUILD];
    int Width, Height;
    Width = (int)b->Width;
    Height = (int)b->Height;
    BYTE* Buffer = b->Buffer;
    if (!Buffer || Width == 0 || Height == 0)
    {
        mu::log::Get("gameplay")->error("[CreateGuildMark] BITMAP_GUILD not loaded (BitmapIndex={})", b->BitmapIndex);
        return;
    }
    int alpha = 128;
    if (blend)
    {
        alpha = 0;
    }

    for (int i = 0; i < 16; i++)
    {
        switch (i)
        {
        case 0:MarkColor[i] = (alpha << 24) + (0 << 16) + (0 << 8) + (0); break;
        case 1:
            MarkColor[i] = (255u << 24) + (0 << 16) + (0 << 8) + (0);
            break;
        case 2:
            MarkColor[i] = (255u << 24) + (128 << 16) + (128 << 8) + (128);
            break;
        case 3:
            MarkColor[i] = (255u << 24) + (255 << 16) + (255 << 8) + (255);
            break;
        case 4:
            MarkColor[i] = (255u << 24) + (0 << 16) + (0 << 8) + (255);
            break;
        case 5:
            MarkColor[i] = (255u << 24) + (0 << 16) + (128 << 8) + (255);
            break;
        case 6:
            MarkColor[i] = (255u << 24) + (0 << 16) + (255 << 8) + (255);
            break;
        case 7:
            MarkColor[i] = (255u << 24) + (0 << 16) + (255 << 8) + (128);
            break;
        case 8:
            MarkColor[i] = (255u << 24) + (0 << 16) + (255 << 8) + (0);
            break;
        case 9:
            MarkColor[i] = (255u << 24) + (128 << 16) + (255 << 8) + (0);
            break;
        case 10:
            MarkColor[i] = (255u << 24) + (255 << 16) + (255 << 8) + (0);
            break;
        case 11:
            MarkColor[i] = (255u << 24) + (255 << 16) + (128 << 8) + (0);
            break;
        case 12:
            MarkColor[i] = (255u << 24) + (255 << 16) + (0 << 8) + (0);
            break;
        case 13:
            MarkColor[i] = (255u << 24) + (255 << 16) + (0 << 8) + (128);
            break;
        case 14:
            MarkColor[i] = (255u << 24) + (255 << 16) + (0 << 8) + (255);
            break;
        case 15:
            MarkColor[i] = (255u << 24) + (128 << 16) + (0 << 8) + (255);
            break;
        }
    }
    BYTE* MarkBuffer = GuildMark[nMarkIndex].Mark;

    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            WriteMarkPixel(Buffer, MarkColor[MarkBuffer[0]]);
            Buffer += 4;
            MarkBuffer++;
        }
    }

    mu::GetRenderer().QueueTextureUpdate(b->BitmapIndex, b->Buffer, static_cast<std::uint32_t>(Width),
                                         static_cast<std::uint32_t>(Height));
    mu::GetRenderer().BindTexture(b->BitmapIndex);
}

void CreateCastleMark(int Type, BYTE* buffer, bool blend)
{
    if (buffer == NULL) return;

    BITMAP_t* b = &Bitmaps[Type];

    int Width, Height;

    Width = (int)b->Width;
    Height = (int)b->Height;
    BYTE* Buffer = b->Buffer;
    if (!Buffer || Width == 0 || Height == 0)
    {
        mu::log::Get("gameplay")
            ->error("[CreateCastleMark] bitmap {} not loaded (BitmapIndex={})", Type, b->BitmapIndex);
        return;
    }

    int alpha = 128;
    if (blend)
    {
        alpha = 0;
    }

    for (int i = 0; i < 16; i++)
    {
        switch (i)
        {
        case 0:MarkColor[i] = (alpha << 24) + (0 << 16) + (0 << 8) + (0); break;
        case 1:
            MarkColor[i] = (255u << 24) + (0 << 16) + (0 << 8) + (0);
            break;
        case 2:
            MarkColor[i] = (255u << 24) + (128 << 16) + (128 << 8) + (128);
            break;
        case 3:
            MarkColor[i] = (255u << 24) + (255 << 16) + (255 << 8) + (255);
            break;
        case 4:
            MarkColor[i] = (255u << 24) + (0 << 16) + (0 << 8) + (255);
            break; // 빨
        case 5:
            MarkColor[i] = (255u << 24) + (0 << 16) + (128 << 8) + (255);
            break; //
        case 6:
            MarkColor[i] = (255u << 24) + (0 << 16) + (255 << 8) + (255);
            break; // 노
        case 7:
            MarkColor[i] = (255u << 24) + (0 << 16) + (255 << 8) + (128);
            break; //
        case 8:
            MarkColor[i] = (255u << 24) + (0 << 16) + (255 << 8) + (0);
            break; // 초
        case 9:
            MarkColor[i] = (255u << 24) + (128 << 16) + (255 << 8) + (0);
            break; //
        case 10:
            MarkColor[i] = (255u << 24) + (255 << 16) + (255 << 8) + (0);
            break; // 청
        case 11:
            MarkColor[i] = (255u << 24) + (255 << 16) + (128 << 8) + (0);
            break; //
        case 12:
            MarkColor[i] = (255u << 24) + (255 << 16) + (0 << 8) + (0);
            break; // 파
        case 13:
            MarkColor[i] = (255u << 24) + (255 << 16) + (0 << 8) + (128);
            break; //
        case 14:
            MarkColor[i] = (255u << 24) + (255 << 16) + (0 << 8) + (255);
            break; // 보
        case 15:
            MarkColor[i] = (255u << 24) + (128 << 16) + (0 << 8) + (255);
            break; //
        }
    }
    BYTE MarkBuffer[32 * 32];

    int offset = 0;

    for (int i = 0; i < 32; ++i)
    {
        for (int j = 0; j < 32; ++j)
        {
            offset = (j / 4) + ((i / 4) * 8);
            MarkBuffer[j + (i * 32)] = buffer[offset];
        }
    }

    offset = 0;
    int offset2 = 0;

    for (int i = 0; i < Height; ++i)
    {
        for (int j = 0; j < Width; ++j)
        {
            if (j >= (Width / 2 - 16) && j < (Width / 2 + 16) && i >= (Height / 2 - 16) && i < (Height / 2 + 16))
            {
                WriteMarkPixel(Buffer + offset, MarkColor[MarkBuffer[offset2]]);
                offset2++;
            }
            else if (j<3 || j>(Width - 4) || i<10 || i>(Height - 10))
            {
                WriteMarkPixel(Buffer + offset, (255u << 24) + (0u << 16) +
                                                    (static_cast<unsigned int>(50 + i / 100.f * 160) << 8) +
                                                    static_cast<unsigned int>(50 + i / 100.f * 255));
            }
            else
            {
                WriteMarkPixel(Buffer + offset, (255u << 24) + (i << 16) + (i << 8) + (i));
            }
            offset += 4;
        }
    }
    mu::GetRenderer().QueueTextureUpdate(
        b->BitmapIndex, b->Buffer, static_cast<std::uint32_t>(b->Width), static_cast<std::uint32_t>(b->Height));
    mu::GetRenderer().BindTexture(b->BitmapIndex);
}

BYTE CaculateFreeTicketLevel(int iType)
{
    int iChaLevel = CharacterAttribute->Level;
    int iChaClass = gCharacterManager.GetBaseClass(Hero->Class);
    int iChaExClass = gCharacterManager.IsSecondClass(Hero->Class);

    int iItemLevel = 0;

    if (iType == FREETICKET_TYPE_DEVILSQUARE)
    {
        if (iChaClass == CLASS_DARK)
        {
            if (iChaLevel >= 15 && iChaLevel <= 110)
            {
                iItemLevel = 0;
            }
            else if (iChaLevel >= 111 && iChaLevel <= 160)
            {
                iItemLevel = 1;
            }
            else if (iChaLevel >= 161 && iChaLevel <= 210)
            {
                iItemLevel = 2;
            }
            else if (iChaLevel >= 211 && iChaLevel <= 260)
            {
                iItemLevel = 3;
            }
            else if (iChaLevel >= 261 && iChaLevel <= 310)
            {
                iItemLevel = 4;
            }
            else if (iChaLevel >= 311 && iChaLevel <= 400)
            {
                iItemLevel = 5;
            }
        }
        else
        {
            if (iChaLevel >= 15 && iChaLevel <= 130)
            {
                iItemLevel = 0;
            }
            else if (iChaLevel >= 131 && iChaLevel <= 180)
            {
                iItemLevel = 1;
            }
            else if (iChaLevel >= 181 && iChaLevel <= 230)
            {
                iItemLevel = 2;
            }
            else if (iChaLevel >= 231 && iChaLevel <= 280)
            {
                iItemLevel = 3;
            }
            else if (iChaLevel >= 281 && iChaLevel <= 330)
            {
                iItemLevel = 4;
            }
            else if (iChaLevel >= 331 && iChaLevel <= 400)
            {
                iItemLevel = 5;
            }
        }
        return iItemLevel;
    }

    if (iType == FREETICKET_TYPE_BLOODCASTLE)
    {
        if (iChaClass == CLASS_DARK)
        {
            if (iChaLevel >= 15 && iChaLevel <= 60)
            {
                iItemLevel = 0;
            }
            else if (iChaLevel >= 61 && iChaLevel <= 110)
            {
                iItemLevel = 1;
            }
            else if (iChaLevel >= 111 && iChaLevel <= 160)
            {
                iItemLevel = 2;
            }
            else if (iChaLevel >= 161 && iChaLevel <= 210)
            {
                iItemLevel = 3;
            }
            else if (iChaLevel >= 211 && iChaLevel <= 260)
            {
                iItemLevel = 4;
            }
            else if (iChaLevel >= 261 && iChaLevel <= 310)
            {
                iItemLevel = 5;
            }
            else if (iChaLevel >= 311 && iChaLevel <= 400)
            {
                iItemLevel = 6;
            }
        }
        else
        {
            if (iChaLevel >= 15 && iChaLevel <= 80)
            {
                iItemLevel = 0;
            }
            else if (iChaLevel >= 81 && iChaLevel <= 130)
            {
                iItemLevel = 1;
            }
            else if (iChaLevel >= 131 && iChaLevel <= 180)
            {
                iItemLevel = 2;
            }
            else if (iChaLevel >= 181 && iChaLevel <= 230)
            {
                iItemLevel = 3;
            }
            else if (iChaLevel >= 231 && iChaLevel <= 280)
            {
                iItemLevel = 4;
            }
            else if (iChaLevel >= 281 && iChaLevel <= 330)
            {
                iItemLevel = 5;
            }
            else if (iChaLevel >= 331 && iChaLevel <= 400)
            {
                iItemLevel = 6;
            }
        }
        return iItemLevel;
    }
    
    if (iType == FREETICKET_TYPE_CURSEDTEMPLE) {
        if (g_pCursedTempleEnterWindow->CheckEnterLevel(iItemLevel))
        {
            return iItemLevel;
        }
    }
    else if (iType == FREETICKED_TYPE_CHAOSCASTLE)
    {
        if (g_pCursedTempleEnterWindow->CheckEnterLevel(iItemLevel))
        {
            return iItemLevel;
        }
    }

    return 0;
}
