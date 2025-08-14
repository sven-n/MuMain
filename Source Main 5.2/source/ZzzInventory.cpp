///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzLodTerrain.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzTexture.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"

#include "ZzzScene.h"

#include "./Utilities/Debouncer.h"
#include "CSQuest.h"
#include "Local.h"
#include "PersonalShopTitleImp.h"
#include "GOBoid.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "npcBreeder.h"
#include "GIPetManager.h"
#include "CSParts.h"
#include "npcGateSwitch.h"
#include "CComGem.h"
#include "CDirection.h"
#include "ChangeRingManager.h"
#include "MixMgr.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "NewUIInventoryCtrl.h"
#include "w_CursedTemple.h"
#include "SocketSystem.h"
#include "PortalMgr.h"
#include "NewUISystem.h"
#include "ServerListManager.h"
#include <time.h>
#include <unordered_set>

#include "MapManager.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "NewUISystem.h"

extern CUITextInputBox* g_pSingleTextInputBox;
extern int g_iChatInputType;
extern CUIGuildListBox* g_pGuildListBox;

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
POINT g_ptPersonalShop = { 0, 0 };
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

BYTE g_byItemUseType = 0;

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
        *color = (255 << 24) + (255 << 16) + (153 << 8) + (255);
    }
    else if (level >= 200)
    {
        lvl = 200;
        *color = (255 << 24) + (255 << 16) + (230 << 8) + (210);
    }
    else if (level >= 100)
    {
        lvl = 100;
        *color = (255 << 24) + (24 << 16) + (201 << 8) + (0);
    }
    else if (level >= 50)
    {
        lvl = 50;
        *color = (255 << 24) + (0 << 16) + (150 << 8) + (255);
    }
    else
    {
        lvl = 10;
        *color = (255 << 24) + (0 << 16) + (0 << 8) + (255);
    }
    return lvl;
}

wchar_t TextList[50][100];
int  TextListColor[50];
int  TextBold[50];
SIZE Size[50];

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

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &Size[i]);

        if (TextWidth < Size[i].cx)
        {
            TextWidth = Size[i].cx;
        }
    }

    if (Tab == 0)
    {
        sx -= (TextWidth + Tab) * 640 / WindowWidth / 2;
    }

    for (int i = 0; i < TextNum; i++)
    {
        g_pRenderText->SetTextColor(0xffffffff);

        switch (TextListColor[i])
        {
        case TEXT_COLOR_WHITE:
        case TEXT_COLOR_DARKRED:
        case TEXT_COLOR_DARKBLUE:
        case TEXT_COLOR_DARKYELLOW:
            glColor3f(1.f, 1.f, 1.f);
            break;
        case TEXT_COLOR_BLUE:
            glColor3f(0.5f, 0.7f, 1.f);
            break;
        case TEXT_COLOR_GRAY:
            glColor3f(0.4f, 0.4f, 0.4f);
            break;
        case TEXT_COLOR_GREEN_BLUE:
            glColor3f(1.f, 1.f, 1.f);
            break;
        case TEXT_COLOR_RED:
            glColor3f(1.f, 0.2f, 0.1f);
            break;
        case TEXT_COLOR_YELLOW:
            glColor3f(1.f, 0.8f, 0.1f);
            break;
        case TEXT_COLOR_GREEN:
            glColor3f(0.1f, 1.f, 0.5f);
            break;
        case TEXT_COLOR_PURPLE:
            glColor3f(1.f, 0.1f, 1.f);
            break;
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
    int TextLine = 0; int EmptyLine = 0;
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

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);

        if (fWidth < TextSize.cx)
            fWidth = TextSize.cx;

        if (TextList[i][0] == '\n')
        {
            ++EmptyLine;
        }
        else
        {
            ++TextLine;
        }
    }

    fHeight = TextSize.cy * TextLine + TextSize.cy / 2.0f * EmptyLine;
    fHeight /= g_fScreenRate_y / 1.1f;
    EnableAlphaTest();
    fWidth /= g_fScreenRate_x;
    if (Tab > 0)
        fWidth = Tab / g_fScreenRate_x * 2;
    fWidth += 4;
    int iPos_x = sx - fWidth / 2;
    if (iPos_x < 0) iPos_x = 0;
    if (iPos_x + fWidth > (int)WindowWidth / g_fScreenRate_x)
    {
        iPos_x = ((int)WindowWidth) / g_fScreenRate_x - fWidth - 1;
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
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        RenderColor((float)iPos_x - 1, fsy - 1, (float)fWidth + 1, (float)1);
        RenderColor((float)iPos_x - 1, fsy - 1, (float)1, (float)fHeight + 1);
        RenderColor((float)iPos_x - 1 + fWidth + 1, (float)fsy - 1, (float)1, (float)fHeight + 1);
        RenderColor((float)iPos_x - 1, fsy - 1 + fHeight + 1, (float)fWidth + 2, (float)1);

        glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
        RenderColor((float)iPos_x, fsy, (float)fWidth, (float)fHeight);
        glEnable(GL_TEXTURE_2D);
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
            GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);
            fHeight = (float)TextSize.cy / g_fScreenRate_y / (TextList[i][0] == 0x0a ? 2.0f : 1.0f);
        }
        else
        {
            g_pRenderText->SetTextColor(0xffffffff);
            switch (TextListColor[i])
            {
            case TEXT_COLOR_WHITE:
            case TEXT_COLOR_DARKRED:
            case TEXT_COLOR_DARKBLUE:
            case TEXT_COLOR_DARKYELLOW:
                glColor3f(1.f, 1.f, 1.f);
                break;
            case TEXT_COLOR_BLUE:
                glColor3f(0.5f, 0.7f, 1.f);
                break;
            case TEXT_COLOR_GRAY:
                glColor3f(0.4f, 0.4f, 0.4f);
                break;
            case TEXT_COLOR_GREEN_BLUE:
                glColor3f(1.f, 1.f, 1.f);
                break;
            case TEXT_COLOR_RED:
                glColor3f(1.f, 0.2f, 0.1f);
                break;
            case TEXT_COLOR_YELLOW:
                glColor3f(1.f, 0.8f, 0.1f);
                break;
            case TEXT_COLOR_GREEN:
                glColor3f(0.1f, 1.f, 0.5f);
                break;
            case TEXT_COLOR_PURPLE:
                glColor3f(1.f, 0.1f, 1.f);
                break;
            case TEXT_COLOR_REDPURPLE:
                glColor3f(0.8f, 0.5f, 0.8f);
                break;
            case TEXT_COLOR_VIOLET:
                glColor3f(0.7f, 0.4f, 1.0f);
                break;
            case TEXT_COLOR_ORANGE:
                glColor3f(0.9f, 0.42f, 0.04f);
                break;
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

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    DisableAlphaBlend();
}

void SendRequestUse(int Index, int Target)
{
    if (!IsCanUseItem())
    {
        g_pSystemLogBox->AddText(GlobalText[474], SEASON3B::TYPE_ERROR_MESSAGE);
        return;
    }
    if (EnableUse > 0)
    {
        return;
    }

    EnableUse = 10;
    SocketClient->ToGameServer()->SendConsumeItemRequest(Index, Target, g_byItemUseType);
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

    SocketClient->ToGameServer()->SendItemMoveRequestExtended((uint32_t)iSrcType, iSrcIndex, (uint32_t)iDstType, iDstIndex);

    g_ConsoleDebug->Write(MCD_SEND, L"0x24 [SendRequestEquipmentItem(%d %d %d %d %d %d %d)]", iSrcIndex, iDstIndex, iSrcType, iDstType, (pItem->Type & 0x1FFF), (BYTE)(pItem->Level), (BYTE)(pItem->Durability));

    return true;
}

bool IsCanUseItem()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE) || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE))
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
    if (g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPSALE) || g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPPURCHASE))
    {
        return false;
    }
    return true;
}

int TextNum = 0;
int SkipNum = 0;

bool IsRequireClassRenderItem(const short sType)
{
    if (
        sType == ITEM_WEAPON_OF_ARCHANGEL
        || sType == ITEM_ARMOR_OF_GUARDSMAN
        || sType == ITEM_WING + 26
        || (sType >= ITEM_PACKED_JEWEL_OF_BLESS && sType <= ITEM_PACKED_JEWEL_OF_SOUL)
        || (sType >= ITEM_HELPER + 43 && sType <= ITEM_HELPER + 45)
        || sType == ITEM_TRANSFORMATION_RING
        || (sType >= ITEM_ELITE_TRANSFER_SKELETON_RING && sType <= ITEM_GAME_MASTER_TRANSFORMATION_RING)
        || sType == ITEM_HORN_OF_FENRIR
        || sType == ITEM_JEWEL_OF_CHAOS
        || sType == ITEM_RED_RIBBON_BOX
        || sType == ITEM_GREEN_RIBBON_BOX
        || sType == ITEM_BLUE_RIBBON_BOX
        )
    {
        return false;
    }

    if ((sType >= ITEM_HELPER + 43 && sType <= ITEM_HELPER + 45)
        || (sType >= ITEM_HELPER + 46 && sType <= ITEM_HELPER + 48)
        || (sType >= ITEM_HELPER + 125 && sType <= ITEM_HELPER + 127)
        || (sType == ITEM_POTION + 54)
        || (sType >= ITEM_POTION + 58 && sType <= ITEM_POTION + 62)
        || (sType == ITEM_POTION + 53)
        || (sType >= ITEM_POTION + 70 && sType <= ITEM_POTION + 71)
        || (sType >= ITEM_POTION + 72 && sType <= ITEM_POTION + 77)
        || (sType >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN
            && sType <= ITEM_TYPE_CHARM_MIXWING + EWS_END)
        || (sType == ITEM_HELPER + 59)
        || (sType >= ITEM_HELPER + 54 && sType <= ITEM_HELPER + 58)
        || (sType >= ITEM_POTION + 78 && sType <= ITEM_POTION + 82)
        || (sType == ITEM_HELPER + 60)
        || (sType == ITEM_HELPER + 61)
        || (sType == ITEM_POTION + 91)
        || (sType == ITEM_POTION + 94)
        || (sType >= ITEM_POTION + 92 && sType <= ITEM_POTION + 93)
        || (sType == ITEM_POTION + 95)
        || (sType >= ITEM_HELPER + 62 && sType <= ITEM_HELPER + 63)
        || (sType >= ITEM_POTION + 97 && sType <= ITEM_POTION + 98)
        || (sType == ITEM_POTION + 96)
        || (sType == ITEM_DEMON || sType == ITEM_SPIRIT_OF_GUARDIAN)
        || (sType == ITEM_PET_RUDOLF)
        || (sType == ITEM_SNOWMAN_TRANSFORMATION_RING)
        || (sType == ITEM_PANDA_TRANSFORMATION_RING)
        || (sType == ITEM_SKELETON_TRANSFORMATION_RING)
        || (sType == ITEM_HELPER + 69)
        || (sType == ITEM_HELPER + 70)
        || (sType == ITEM_HELPER + 71 || sType == ITEM_HELPER + 72 || sType == ITEM_HELPER + 73 || sType == ITEM_HELPER + 74 || sType == ITEM_HELPER + 75)
        || (sType == ITEM_PET_PANDA)
        || (sType == ITEM_PET_UNICORN)
        || sType == ITEM_HELPER + 81
        || sType == ITEM_HELPER + 82
        || sType == ITEM_HELPER + 93
        || sType == ITEM_HELPER + 94
        || sType == ITEM_HELPER + 121
        || (sType >= ITEM_POTION + 145 && sType <= ITEM_POTION + 150)
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || g_pMyInventory->IsInvenItem(sType)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (sType == ITEM_POTION + 133)
        )
    {
        return false;
    }

    return true;
}

void RequireClass(ITEM_ATTRIBUTE* pItem)
{
    if (pItem == NULL)
        return;

    BYTE byFirstClass = gCharacterManager.GetBaseClass(Hero->Class);
    BYTE byStepClass = gCharacterManager.GetStepClass(Hero->Class);

    int iTextColor = 0;

    TextListColor[TextNum + 2] = TextListColor[TextNum + 3] = TEXT_COLOR_WHITE;
    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

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
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[20]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 2)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[25]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[1669]);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_KNIGHT:
        {
            if (byRequireClass == 1)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[21]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 2)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[26]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[1668]);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_ELF:
        {
            if (byRequireClass == 1)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[22]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 2)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[27]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[1670]);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_DARK:
        {
            if (byRequireClass == 1)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[23]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[1671]);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_DARK_LORD:
        {
            if (byRequireClass == 1)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[24]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[1672]);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_SUMMONER:
        {
            if (byRequireClass == 1)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[1687]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 2)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[1688]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[1689]);
                TextListColor[TextNum] = iTextColor;
            }

            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        case CLASS_RAGEFIGHTER:
        {
            if (byRequireClass == 1)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[3150]);
                TextListColor[TextNum] = iTextColor;
            }
            else if (byRequireClass == 3)
            {
                swprintf(TextList[TextNum], GlobalText[61], GlobalText[3151]);
                TextListColor[TextNum] = iTextColor;
            }
            TextBold[TextNum] = false;	TextNum++;
        }
        break;
        }
    }
}

const int iMaxLevel = 15;
const int iMaxColumn = 17;
int g_iCurrentItem = -1;
int g_iItemInfo[iMaxLevel + 1][iMaxColumn];

void RenderHelpLine(int iColumnType, const wchar_t* pPrintStyle, int& TabSpace, const wchar_t* pGapText, int Pos_y, int iType)
{
    int iCurrMaxLevel = iMaxLevel;

    if (iType == 5)
        iCurrMaxLevel = 0;

    for (int Level = 0; Level <= iCurrMaxLevel; ++Level)
    {
        swprintf(TextList[TextNum], pPrintStyle, g_iItemInfo[Level][iColumnType]);
        if (g_iItemInfo[Level][_COLUMN_TYPE_CAN_EQUIP] == TRUE)
        {
            TextListColor[Level] = TEXT_COLOR_WHITE;
        }
        else
        {
            TextListColor[Level] = TEXT_COLOR_RED;
        }
        TextBold[Level] = false;
        ++TextNum;
    }

    SIZE TextSize;
    RenderTipTextList(TabSpace, Pos_y, TextNum, 0, RT3_SORT_CENTER, FALSE);

    if (pGapText == NULL)
    {
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[TextNum - 1], lstrlen(TextList[TextNum - 1]), &TextSize);
    }
    else
    {
        GetTextExtentPoint32(g_pRenderText->GetFontDC(), pGapText, wcslen(pGapText), &TextSize);
    }
    TabSpace += int(TextSize.cx / g_fScreenRate_x);
    if (iType == 6)
    {
        TabSpace += 5;
    }
    TextNum -= iCurrMaxLevel + 1;
}

void RenderHelpCategory(int iColumnType, int Pos_x, int Pos_y)
{
    const wchar_t* pText = NULL;

    switch (iColumnType)
    {
    case _COLUMN_TYPE_LEVEL:
        pText = GlobalText[161];
        break;
    case _COLUMN_TYPE_ATTMIN: case _COLUMN_TYPE_ATTMAX:
        pText = GlobalText[162];
        break;
    case _COLUMN_TYPE_MAGIC:
        pText = GlobalText[163];
        break;
    case _COLUMN_TYPE_CURSE:
        pText = GlobalText[1144];
        break;
    case _COLUMN_TYPE_PET_ATTACK:
        pText = GlobalText[1239];
        break;
    case _COLUMN_TYPE_DEFENCE:
        pText = GlobalText[164];
        break;
    case _COLUMN_TYPE_DEFRATE:
        pText = GlobalText[165];
        break;
    case _COLUMN_TYPE_REQSTR:
        pText = GlobalText[166];
        break;
    case _COLUMN_TYPE_REQDEX:
        pText = GlobalText[167];
        break;
    case _COLUMN_TYPE_REQENG:
        pText = GlobalText[168];
        break;
    case _COLUMN_TYPE_REQCHA:
        pText = GlobalText[1900];
        break;
    case _COLUMN_TYPE_REQVIT:
        pText = GlobalText[169];
        break;
    case _COLUMN_TYPE_REQNLV:
        pText = GlobalText[1931];
        break;
    default:
        break;
    }
    swprintf(TextList[TextNum], pText);
    TextListColor[TextNum] = TEXT_COLOR_BLUE;
    TextNum++;
    RenderTipTextList(Pos_x, Pos_y, TextNum, 0, RT3_SORT_RIGHT, FALSE);
    TextNum = 0;
}

void ComputeItemInfo(int iHelpItem)
{
    if (g_iCurrentItem == iHelpItem)
        return;
    else
        g_iCurrentItem = iHelpItem;

    ITEM_ATTRIBUTE* p = &ItemAttribute[ItemHelp];

    for (int Level = 0; Level <= iMaxLevel; Level++)
    {
        int RequireStrength = 0;
        int RequireDexterity = 0;
        int RequireEnergy = 0;
        int RequireCharisma = 0;
        int RequireVitality = 0;
        int RequireLevel = 0;
        int DamageMin = p->DamageMin;
        int DamageMax = p->DamageMax;
        int Defense = p->Defense;
        int Magic = p->MagicPower;
        int Blocking = p->SuccessfulBlocking;

        if (DamageMin > 0)
        {
            DamageMin += (min(9, Level) * 3);
            switch (Level - 9)
            {
            case 6: DamageMin += 9; break;	// +15
            case 5: DamageMin += 8; break;	// +14
            case 4: DamageMin += 7; break;	// +13
            case 3: DamageMin += 6; break;	// +12
            case 2: DamageMin += 5; break;	// +11
            case 1: DamageMin += 4; break;	// +10
            default: break;
            };
        }
        if (DamageMax > 0)
        {
            DamageMax += (min(9, Level) * 3);
            switch (Level - 9)
            {
            case 6: DamageMax += 9; break;	// +15
            case 5: DamageMax += 8; break;	// +14
            case 4: DamageMax += 7; break;	// +13
            case 3: DamageMax += 6; break;	// +12
            case 2: DamageMax += 5; break;	// +11
            case 1: DamageMax += 4; break;	// +10
            default: break;
            };
        }

        if (Magic > 0)
        {
            Magic += (min(9, Level) * 3);	// ~ +9
            switch (Level - 9)
            {
            case 6: Magic += 9; break;		// +15
            case 5: Magic += 8; break;		// +14
            case 4: Magic += 7; break;	    // +13
            case 3: Magic += 6; break;		// +12
            case 2: Magic += 5; break;		// +11
            case 1: Magic += 4; break;		// +10
            default: break;
            };
            Magic /= 2;

            if (IsCepterItem(ItemHelp) == false)
            {
                Magic += Level * 2;
            }
        }

        if (Defense > 0)
        {
            if (ItemHelp >= ITEM_SHIELD && ItemHelp < ITEM_SHIELD + MAX_ITEM_INDEX)
            {
                Defense += Level;
            }
            else
            {
                Defense += (min(9, Level) * 3);	// ~ +9
                switch (Level - 9)
                {
                case 6: Defense += 9; break;	// +15
                case 5: Defense += 8; break;	// +14
                case 4: Defense += 7; break;	// +13
                case 3: Defense += 6; break;	// +12
                case 2: Defense += 5; break;	// +11
                case 1: Defense += 4; break;	// +10
                default: break;
                };
            }
        }
        if (Blocking > 0)
        {
            Blocking += (min(9, Level) * 3);	// ~ +9
            switch (Level - 9)
            {
            case 6: Blocking += 9; break;	// +15
            case 5: Blocking += 8; break;	// +14
            case 4: Blocking += 7; break;	// +13
            case 3: Blocking += 6; break;	// +12
            case 2: Blocking += 5; break;	// +11
            case 1: Blocking += 4; break;	// +10
            default: break;
            };
        }

        if (p->RequireLevel)
        {
            RequireLevel = p->RequireLevel;
        }
        else
        {
            RequireLevel = 0;
        }

        if (p->RequireStrength)
        {
            RequireStrength = 20 + p->RequireStrength * (p->Level + Level * 3) * 3 / 100;
        }
        else
        {
            RequireStrength = 0;
        }

        if (p->RequireDexterity)	RequireDexterity = 20 + p->RequireDexterity * (p->Level + Level * 3) * 3 / 100;
        else RequireDexterity = 0;

        if (p->RequireVitality)	RequireVitality = 20 + p->RequireVitality * (p->Level + Level * 3) * 3 / 100;
        else RequireVitality = 0;

        if (p->RequireEnergy)
        {
            if (ItemHelp >= ITEM_BOOK_OF_SAHAMUTT && ItemHelp <= ITEM_STAFF + 29)
            {
                RequireEnergy = 20 + p->RequireEnergy * (p->Level + Level * 1) * 3 / 100;
            }
            else
            {
                if ((p->RequireLevel > 0) && (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX))
                {
                    RequireEnergy = 20 + (p->RequireEnergy) * (p->RequireLevel) * 4 / 100;
                }
                else
                {
                    RequireEnergy = 20 + p->RequireEnergy * (p->Level + Level * 3) * 4 / 100;
                }
            }
        }
        else
        {
            RequireEnergy = 0;
        }

        if (p->RequireCharisma)	RequireCharisma = 20 + p->RequireCharisma * (p->Level + Level * 3) * 3 / 100;
        else RequireCharisma = 0;

        g_iItemInfo[Level][_COLUMN_TYPE_LEVEL] = Level;
        g_iItemInfo[Level][_COLUMN_TYPE_ATTMIN] = DamageMin;
        g_iItemInfo[Level][_COLUMN_TYPE_ATTMAX] = DamageMax;

        if (ItemHelp >= ITEM_BOOK_OF_SAHAMUTT && ItemHelp <= ITEM_STAFF + 29)
        {
            g_iItemInfo[Level][_COLUMN_TYPE_CURSE] = Magic;
        }
        else
        {
            g_iItemInfo[Level][_COLUMN_TYPE_MAGIC] = Magic;
        }

        g_iItemInfo[Level][_COLUMN_TYPE_PET_ATTACK] = Magic;
        g_iItemInfo[Level][_COLUMN_TYPE_DEFENCE] = Defense;
        g_iItemInfo[Level][_COLUMN_TYPE_DEFRATE] = Blocking;
        g_iItemInfo[Level][_COLUMN_TYPE_REQSTR] = RequireStrength;
        g_iItemInfo[Level][_COLUMN_TYPE_REQDEX] = RequireDexterity;
        g_iItemInfo[Level][_COLUMN_TYPE_REQENG] = RequireEnergy;
        g_iItemInfo[Level][_COLUMN_TYPE_REQCHA] = RequireCharisma;
        g_iItemInfo[Level][_COLUMN_TYPE_REQVIT] = RequireVitality;
        g_iItemInfo[Level][_COLUMN_TYPE_REQNLV] = RequireLevel;

        if (IsCepterItem(ItemHelp) == true)

        {
            g_iItemInfo[Level][_COLUMN_TYPE_MAGIC] = 0;
        }
        else
        {
            g_iItemInfo[Level][_COLUMN_TYPE_PET_ATTACK] = 0;
        }

        WORD Strength, Dexterity, Energy, Vitality, Charisma;

        Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        Energy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
        Vitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
        Charisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

        if (RequireStrength <= Strength
            && RequireDexterity <= Dexterity
            && RequireEnergy <= Energy
            && RequireVitality <= Vitality
            && RequireCharisma <= Charisma
            && RequireLevel <= CharacterAttribute->Level)
            g_iItemInfo[Level][_COLUMN_TYPE_CAN_EQUIP] = TRUE;
        else
            g_iItemInfo[Level][_COLUMN_TYPE_CAN_EQUIP] = FALSE;
    }
}

unsigned int getGoldColor(DWORD Gold)
{
    if (Gold >= 10000000)
    {
        return  (255 << 24) + (0 << 16) + (0 << 8) + (255);
    }
    else if (Gold >= 1000000)
    {
        return  (255 << 24) + (0 << 16) + (150 << 8) + (255);
    }
    else if (Gold >= 100000)
    {
        return  (255 << 24) + (24 << 16) + (201 << 8) + (0);
    }

    return  (255 << 24) + (150 << 16) + (220 << 8) + (255);
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

    swprintf(szText, L"%d", dwValueTemp);

    while (iCipherCnt > 0)
    {
        dwValueTemp = (DWORD)dGold;
        dwValueTemp = (dwValueTemp % (int)pow(10.f, (float)iCipherCnt)) / (int)pow(10.f, (float)(iCipherCnt - 3));
        swprintf(szTemp, L",%03d", dwValueTemp);
        wcscat(szText, szTemp);
        iCipherCnt = iCipherCnt - 3;
    }

    if (iDecimals > 0)
    {
        dwValueTemp = (int)(dGold * pow(10.f, (float)iDecimals)) % (int)pow(10.f, (float)iDecimals);
        swprintf(szTemp, L".%d", dwValueTemp);
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
        swprintf(Text, L"%d,%03d,%03d,%03d,%03d,%03d", Gold6, Gold5, Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000000000)
        swprintf(Text, L"%d,%03d,%03d,%03d,%03d", Gold5, Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000000)
        swprintf(Text, L"%d,%03d,%03d,%03d", Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000)
        swprintf(Text, L"%d,%03d,%03d", Gold3, Gold2, Gold1);
    else if (Gold >= 1000)
        swprintf(Text, L"%d,%03d", Gold2, Gold1);
    else
        swprintf(Text, L"%d", Gold1);
}

void ConvertTaxGold(DWORD Gold, wchar_t* Text)
{
    Gold += ((LONGLONG)Gold * g_pNPCShop->GetTaxRate()) / 100;

    int Gold1 = Gold % 1000;
    int Gold2 = Gold % 1000000 / 1000;
    int Gold3 = Gold % 1000000000 / 1000000;
    int Gold4 = Gold / 1000000000;
    if (Gold >= 1000000000)
        swprintf(Text, L"%d,%03d,%03d,%03d", Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000)
        swprintf(Text, L"%d,%03d,%03d", Gold3, Gold2, Gold1);
    else if (Gold >= 1000)
        swprintf(Text, L"%d,%03d", Gold2, Gold1);
    else
        swprintf(Text, L"%d", Gold1);
}

void ConvertChaosTaxGold(DWORD Gold, wchar_t* Text)
{
    Gold += ((LONGLONG)Gold * g_nChaosTaxRate) / 100;

    int Gold1 = Gold % 1000;
    int Gold2 = Gold % 1000000 / 1000;
    int Gold3 = Gold % 1000000000 / 1000000;
    int Gold4 = Gold / 1000000000;
    if (Gold >= 1000000000)
        swprintf(Text, L"%d,%03d,%03d,%03d", Gold4, Gold3, Gold2, Gold1);
    else if (Gold >= 1000000)
        swprintf(Text, L"%d,%03d,%03d", Gold3, Gold2, Gold1);
    else if (Gold >= 1000)
        swprintf(Text, L"%d,%03d", Gold2, Gold1);
    else
        swprintf(Text, L"%d", Gold1);
}

int64_t CalcRepairCost(int64_t ItemValue, int Durability, int MaxDurability, short Type, bool SelfRepair)
{
    // Cap ItemValue at 400M
    int64_t repairGold = min(ItemValue, int64_t(400000000));

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
            if (Type == ITEM_DARK_HORSE_ITEM || Type == ITEM_DARK_RAVEN_ITEM)
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

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) && g_pNPCShop->IsRepairShop()) {
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

            if (IsRepairBan(ip) == true)
            {
                continue;
            }

            //. check durability
            if (ip->Durability < maxDurability)
            {
                int gold = ConvertRepairGold(ItemValue(ip, 2), ip->Durability, maxDurability, ip->Type, text);

                if (Check_LuckyItem(ip->Type))	gold = 0;
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

            if (pItem->Type >= ITEM_POTION + 55 && pItem->Type <= ITEM_POTION + 57)
            {
                continue;
            }
            //. item filtering
            if ((pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_DARK_RAVEN_ITEM) || pItem->Type == ITEM_TRANSFORMATION_RING || pItem->Type == ITEM_SPIRIT)
                continue;
            if (pItem->Type == ITEM_BOLT || pItem->Type == ITEM_ARROWS || pItem->Type >= ITEM_POTION)
                continue;
            if (pItem->Type >= ITEM_ORB_OF_TWISTING_SLASH && pItem->Type <= ITEM_ORB_OF_DEATH_STAB)
                continue;
            if ((pItem->Type >= ITEM_LOCHS_FEATHER && pItem->Type <= ITEM_WEAPON_OF_ARCHANGEL) || pItem->Type == ITEM_POTION + 21)
                continue;
            if (pItem->Type == ITEM_WIZARDS_RING)
                continue;
            if (pItem->Type == ITEM_MOONSTONE_PENDANT)
                continue;

            if (pItem->Type >= ITEM_HELPER + 46 && pItem->Type <= ITEM_HELPER + 48)
            {
                continue;
            }
            if (pItem->Type >= ITEM_HELPER + 125 && pItem->Type <= ITEM_HELPER + 127)
            {
                continue;
            }
            if (pItem->Type >= ITEM_POTION + 145 && pItem->Type <= ITEM_POTION + 150)
            {
                continue;
            }
            if (pItem->Type >= ITEM_POTION + 58 && pItem->Type <= ITEM_POTION + 62)
            {
                continue;
            }
            if (pItem->Type == ITEM_POTION + 53)
            {
                continue;
            }
            if (pItem->Type == ITEM_HELPER + 43 || pItem->Type == ITEM_HELPER + 44 || pItem->Type == ITEM_HELPER + 45)
            {
                continue;
            }
            if (pItem->Type >= ITEM_POTION + 70 && pItem->Type <= ITEM_POTION + 71)
            {
                continue;
            }
            if (pItem->Type >= ITEM_POTION + 72 && pItem->Type <= ITEM_POTION + 77)
            {
                continue;
            }
            if (pItem->Type == ITEM_HELPER + 59)
            {
                continue;
            }
            if (pItem->Type >= ITEM_HELPER + 54 && pItem->Type <= ITEM_HELPER + 58)
            {
                continue;
            }
            if (pItem->Type == ITEM_HELPER + 60)
            {
                continue;
            }
            if (pItem->Type == ITEM_HELPER + 61)
            {
                continue;
            }
            if (pItem->Type == ITEM_POTION + 91)
            {
                continue;
            }
            if (pItem->Type >= ITEM_POTION + 92 && pItem->Type <= ITEM_POTION + 93)
            {
                continue;
            }
            if (pItem->Type == ITEM_POTION + 95)
            {
                continue;
            }
            if (pItem->Type == ITEM_POTION + 95)
            {
                continue;
            }
            if (pItem->Type >= ITEM_HELPER + 62 && pItem->Type <= ITEM_HELPER + 63)
            {
                continue;
            }
            if (pItem->Type >= ITEM_POTION + 97 && pItem->Type <= ITEM_POTION + 98)
            {
                continue;
            }
            if (pItem->Type == ITEM_POTION + 140)
            {
                continue;
            }
            if (pItem->Type == ITEM_POTION + 96)
            {
                continue;
            }
            if (pItem->Type == ITEM_DEMON || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN)
            {
                continue;
            }
            if (pItem->Type == ITEM_PET_RUDOLF)
            {
                continue;
            }
            if (pItem->Type == ITEM_PET_PANDA)
            {
                continue;
            }
            if (pItem->Type == ITEM_PET_UNICORN)
            {
                continue;
            }
            if (pItem->Type == ITEM_PET_SKELETON)
            {
                continue;
            }
            if (pItem->Type == ITEM_SNOWMAN_TRANSFORMATION_RING)
            {
                continue;
            }
            if (pItem->Type == ITEM_PANDA_TRANSFORMATION_RING)
            {
                continue;
            }
            if (pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING)
            {
                continue;
            }
            if (pItem->Type == ITEM_HELPER + 69)
                continue;
            if (pItem->Type == ITEM_HELPER + 70)
                continue;

            if (pItem->Type == ITEM_HORN_OF_FENRIR)
                continue;

            if (pItem->Type == ITEM_HELPER + 66)
                continue;

            if (pItem->Type == ITEM_HELPER + 71
                || pItem->Type == ITEM_HELPER + 72
                || pItem->Type == ITEM_HELPER + 73
                || pItem->Type == ITEM_HELPER + 74
                || pItem->Type == ITEM_HELPER + 75)
                continue;

            if (pItem->Type == ITEM_HELPER + 81)
                continue;
            if (pItem->Type == ITEM_HELPER + 82)
                continue;
            if (pItem->Type == ITEM_HELPER + 93)
                continue;
            if (pItem->Type == ITEM_HELPER + 94)
                continue;

            if (pItem->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN && pItem->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END)
            {
                continue;
            }
            if (pItem->Type == ITEM_HELPER + 97 || pItem->Type == ITEM_HELPER + 98 || pItem->Type == ITEM_POTION + 91)
                continue;

            if (pItem->Type == ITEM_HELPER + 121)
                continue;

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
            if (g_pMyInventory->IsInvenItem(pItem->Type))
                continue;

#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

            if (pItem->Type >= ITEM_WING + 130 && pItem->Type <= ITEM_WING + 134)
                continue;
            if (pItem->Type == ITEM_HELPER + 109)
                continue;
            if (pItem->Type == ITEM_HELPER + 110)
                continue;
            if (pItem->Type == ITEM_HELPER + 111)
                continue;
            if (pItem->Type == ITEM_HELPER + 112)
                continue;
            if (pItem->Type == ITEM_HELPER + 113)
                continue;
            if (pItem->Type == ITEM_HELPER + 114)
                continue;
            if (pItem->Type == ITEM_HELPER + 115)
                continue;
            if (pItem->Type == ITEM_HELPER + 107)
                continue;

            if (Check_ItemAction(pItem, eITEM_REPAIR))	continue;

            //. check durability
            if (pItem->Durability < maxDurability)
            {
                int gold = ConvertRepairGold(ItemValue(pItem, 2), pItem->Durability, maxDurability, pItem->Type, text);
                if (Check_LuckyItem(pItem->Type))	gold = 0;
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
int GuildListStartX;
int GuildListStartY;
int SommonTable[] = { 2,7,14,8,9,41 };

wchar_t ChaosEventName[][100] = {
    L"히돼?고향 여행권",
    L"펜티엄4 컴퓨터",
    L"디지탈카메라",
    L"로지텍 무선 마우스+키보드 세트",
    L"256M 램",
    L"6개?잡지 구독권",
    L"문화상품권(만원)",
    L"뮤 머그컵",
    L"뮤 T셔츠",
    L"뮤 10시간 무료이용권"
};

WORD CalcMaxDurability(const ITEM* ip, ITEM_ATTRIBUTE* p, int Level)
{
    WORD maxDurability = p->Durability;

    if (ip->Type >= ITEM_STAFF && ip->Type < ITEM_STAFF + MAX_ITEM_INDEX)
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

    if (ip->Type == ITEM_DARK_HORSE_ITEM || ip->Type == ITEM_DARK_RAVEN_ITEM)
    {
        maxDurability = 255;
    }

    if (ip->AncientDiscriminator > 0)
    {
        maxDurability += 20;
    }
    else if (ip->ExcellentFlags > 0 &&
        (ip->Type<ITEM_WINGS_OF_SPIRITS || ip->Type>ITEM_WINGS_OF_DARKNESS) &&
        (ip->Type != ITEM_DIVINE_SWORD_OF_ARCHANGEL && ip->Type != ITEM_DIVINE_CB_OF_ARCHANGEL && ip->Type != ITEM_DIVINE_STAFF_OF_ARCHANGEL)
        && ip->Type != ITEM_CAPE_OF_LORD
        && (ip->Type<ITEM_WING_OF_STORM || ip->Type>ITEM_CAPE_OF_EMPEROR)
        && (ip->Type<ITEM_WINGS_OF_DESPAIR || ip->Type>ITEM_WING_OF_DIMENSION)
        && ip->Type != ITEM_DIVINE_SCEPTER_OF_ARCHANGEL
        && !(ip->Type >= ITEM_CAPE_OF_FIGHTER && ip->Type <= ITEM_CAPE_OF_OVERRULE))
    {
        maxDurability += 15;
    }

    if (Check_LuckyItem(ip->Type))
    {
        maxDurability = 255;
    }

    return  maxDurability;
}

void GetItemName(int iType, int iLevel, wchar_t* Text)
{
    ITEM_ATTRIBUTE* p = &ItemAttribute[iType];

    if (iType >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && iType <= ITEM_SOUL_SHARD_OF_WIZARD)
    {
        if (iType == ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR)
        {
            switch (iLevel)
            {
            case 0: swprintf(Text, L"%s", p->Name); break;
            case 1: swprintf(Text, L"%s", GlobalText[906]); break;
            }
        }
        else if (iType == ITEM_BROKEN_SWORD_DARK_STONE)
        {
            switch (iLevel)
            {
            case 0: swprintf(Text, L"%s", p->Name); break;
            case 1: swprintf(Text, L"%s", GlobalText[907]); break;
            }
        }
        else
        {
            swprintf(Text, L"%s", p->Name);
        }
    }
    else if (iType == ITEM_POTION + 12)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", GlobalText[100]); break;
        case 1: swprintf(Text, L"%s", GlobalText[101]); break;
        case 2: swprintf(Text, L"%s", ChaosEventName[p->Durability]); break;
        }
    }
    else if (iType == ITEM_BOX_OF_LUCK)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", p->Name); break;
        case 1: swprintf(Text, L"%s", GlobalText[105]); break;
        case 2: swprintf(Text, L"%s", GlobalText[106]); break;
        case 3: swprintf(Text, L"%s", GlobalText[107]); break;
        case 5: swprintf(Text, L"%s", GlobalText[109]); break;
        case 6: swprintf(Text, L"%s", GlobalText[110]); break;
        case 7: swprintf(Text, L"%s", GlobalText[111]); break;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            swprintf(Text, L"%s +%d", GlobalText[115], iLevel - 7);
            break;
        case 13:
            swprintf(Text, GlobalText[117]); break;
        case 14:
            swprintf(Text, GlobalText[1650]); break;
            break;

        case 15:
            swprintf(Text, GlobalText[1651]); break;
            break;
        }
    }
    else if (iType == ITEM_FRUITS)
    {
        switch (iLevel)
        {
        case 0:swprintf(Text, L"%s %s", GlobalText[168], p->Name); break;
        case 1:swprintf(Text, L"%s %s", GlobalText[169], p->Name); break;
        case 2:swprintf(Text, L"%s %s", GlobalText[167], p->Name); break;
        case 3:swprintf(Text, L"%s %s", GlobalText[166], p->Name); break;
        case 4:swprintf(Text, L"%s %s", GlobalText[1900], p->Name); break;
        }
    }
    else if (iType == ITEM_LOCHS_FEATHER)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", p->Name); break;
        case 1: swprintf(Text, L"%s", GlobalText[1235]); break;
        }
    }
    else if (iType == ITEM_SPIRIT)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s %s", GlobalText[1187], p->Name); break;
        case 1: swprintf(Text, L"%s %s", GlobalText[1214], p->Name); break;
        }
    }
    else if (iType == ITEM_POTION + 21)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", p->Name); break;
        case 1: swprintf(Text, L"%s", GlobalText[810]); break;
        case 2: swprintf(Text, L"%s", GlobalText[1098]); break;
        case 3: swprintf(Text, L"%s", GlobalText[1290]); break;
        }
    }
    else if (iType == ITEM_WEAPON_OF_ARCHANGEL)
    {
        swprintf(Text, L"%s", GlobalText[809]);
    }
    else if (iType == ITEM_WIZARDS_RING)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", p->Name); break;
        case 1: swprintf(Text, L"%s", GlobalText[922]); break;
        case 2: swprintf(Text, L"%s", GlobalText[928]); break;
        case 3: swprintf(Text, L"%s", GlobalText[929]); break;
        }
    }
    else if (iType == ITEM_ALE)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", p->Name); break;
        case 1:	swprintf(Text, L"%s", GlobalText[108]); break;
        }
    }
    else if (iType == ITEM_ORB_OF_SUMMONING)
    {
        swprintf(Text, L"%s %s", SkillAttribute[30 + iLevel].Name, GlobalText[102]);
    }
    else if (iType == ITEM_RED_RIBBON_BOX)
    {
        swprintf(Text, L"%s", p->Name);
    }
    else if (iType == ITEM_GREEN_RIBBON_BOX)
    {
        swprintf(Text, L"%s", p->Name);
    }
    else if (iType == ITEM_BLUE_RIBBON_BOX)
    {
        swprintf(Text, L"%s", p->Name);
    }
    else if (iType == ITEM_SCROLL_OF_FIRE_SCREAM)
    {
        swprintf(Text, L"%s", p->Name);
    }
    else if (iType >= ITEM_PUMPKIN_OF_LUCK && iType <= ITEM_JACK_OLANTERN_DRINK)
    {
        swprintf(Text, L"%s", p->Name);
    }
    else if (iType == ITEM_PINK_CHOCOLATE_BOX)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", p->Name); break;
        case 1:	swprintf(Text, L"%s", GlobalText[2012]); break;
        }
    }
    else if (iType == ITEM_RED_CHOCOLATE_BOX)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", p->Name); break;
        case 1:	swprintf(Text, L"%s", GlobalText[2013]); break;
        }
    }
    else if (iType == ITEM_BLUE_CHOCOLATE_BOX)
    {
        switch (iLevel)
        {
        case 0: swprintf(Text, L"%s", p->Name); break;
        case 1:	swprintf(Text, L"%s", GlobalText[2014]); break;
        }
    }
    else if (iType == ITEM_TRANSFORMATION_RING)
    {
        for (int i = 0; i < MAX_MONSTER; i++)
        {
            if (SommonTable[iLevel] == MonsterScript[i].Type)
            {
                swprintf(Text, L"%s %s", MonsterScript[i].Name, GlobalText[103]);
            }
        }
    }
    else if (iType >= ITEM_WINGS_OF_SPIRITS && iType <= ITEM_WINGS_OF_DARKNESS)
    {
        if (iLevel == 0)
            swprintf(Text, L"%s", p->Name);
        else
            swprintf(Text, L"%s +%d", p->Name, iLevel);
    }
    else if ((iType >= ITEM_WING_OF_STORM && iType <= ITEM_CAPE_OF_EMPEROR) || (iType >= ITEM_WINGS_OF_DESPAIR && iType <= ITEM_WING_OF_DIMENSION)
        || (iType == ITEM_CAPE_OF_OVERRULE))
    {
        if (iLevel == 0)
            swprintf(Text, L"%s", p->Name);
        else
            swprintf(Text, L"%s +%d", p->Name, iLevel);
    }
    else if (iType == ITEM_DIVINE_SWORD_OF_ARCHANGEL || iType == ITEM_DIVINE_CB_OF_ARCHANGEL || iType == ITEM_DIVINE_STAFF_OF_ARCHANGEL || iType == ITEM_DIVINE_SCEPTER_OF_ARCHANGEL)
    {
        if (iLevel == 0)
            swprintf(Text, L"%s", p->Name);
        else
            swprintf(Text, L"%s +%d", p->Name, iLevel);
    }
    else if (COMGEM::NOGEM != COMGEM::Check_Jewel_Com(iType))
    {
        swprintf(Text, L"%s +%d", p->Name, iLevel + 1);
    }
    else if (iType == INDEX_COMPILED_CELE)
    {
        swprintf(Text, L"%s +%d", GlobalText[1806], iLevel + 1);
    }
    else if (iType == INDEX_COMPILED_SOUL)
    {
        swprintf(Text, L"%s +%d", GlobalText[1807], iLevel + 1);
    }
    else if ((iType >= ITEM_SEED_FIRE && iType <= ITEM_SEED_EARTH)
        || (iType >= ITEM_SPHERE_MONO && iType <= ITEM_SPHERE_5)
        || (iType >= ITEM_SEED_SPHERE_FIRE_1 && iType <= ITEM_SEED_SPHERE_EARTH_5))
    {
        swprintf(Text, L"%s", p->Name);
    }
    else if (iType == ITEM_SIEGE_POTION)
    {
        int iTextIndex = 0;
        iTextIndex = (iLevel == 0) ? 1413 : 1414;
        swprintf(Text, L"%s", GlobalText[iTextIndex]);
    }
    else
    {
        if (iLevel == 0)
            swprintf(Text, L"%s", p->Name);
        else
            swprintf(Text, L"%s +%d", p->Name, iLevel);
    }
}

void GetSpecialOptionText(int Type, wchar_t* Text, WORD Option, BYTE Value, int iMana)
{
    switch (Option)
    {
    case AT_SKILL_BLOCKING:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[80], iMana);
        break;
    case AT_SKILL_FALLING_SLASH:
    case AT_SKILL_FALLING_SLASH_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[81], iMana);
        break;
    case AT_SKILL_LUNGE:
    case AT_SKILL_LUNGE_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[82], iMana);
        break;
    case AT_SKILL_UPPERCUT:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[83], iMana);
        break;
    case AT_SKILL_CYCLONE:
    case AT_SKILL_CYCLONE_STR:
    case AT_SKILL_CYCLONE_STR_MG:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[84], iMana);
        break;
    case AT_SKILL_SLASH:
    case AT_SKILL_SLASH_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[85], iMana);
        break;
    case AT_SKILL_TRIPLE_SHOT:
    case AT_SKILL_TRIPLE_SHOT_STR:
    case AT_SKILL_TRIPLE_SHOT_MASTERY:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[86], iMana);
        break;
    case AT_SKILL_BLAST_CROSSBOW4:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[920], iMana);
        break;
    case AT_SKILL_MULTI_SHOT:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[920], iMana);
        break;
    case AT_SKILL_RECOVER:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[920], iMana);
        break;
    case AT_SKILL_POWER_SLASH:
    case AT_SKILL_POWER_SLASH_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[98], iMana);
        break;
    case AT_LUCK:
        swprintf(Text, GlobalText[87]);
        break;
    case AT_IMPROVE_DAMAGE:
        swprintf(Text, GlobalText[88], Value);
        break;
    case AT_IMPROVE_MAGIC:
        swprintf(Text, GlobalText[89], Value);
        break;
    case AT_IMPROVE_CURSE:
        swprintf(Text, GlobalText[1697], Value);
        break;
    case AT_IMPROVE_BLOCKING:
        swprintf(Text, GlobalText[90], Value);
        break;
    case AT_IMPROVE_DEFENSE:
        swprintf(Text, GlobalText[91], Value);
        break;
    case AT_LIFE_REGENERATION:
        if (!(ITEM_LOCHS_FEATHER <= Type && Type <= ITEM_INVISIBILITY_CLOAK))
        {
            swprintf(Text, GlobalText[92], Value);
        }
        break;
    case AT_IMPROVE_LIFE:
        swprintf(Text, GlobalText[622]);
        break;
    case AT_IMPROVE_MANA:
        swprintf(Text, GlobalText[623]);
        break;
    case AT_DECREASE_DAMAGE:
        swprintf(Text, GlobalText[624]);
        break;

    case AT_REFLECTION_DAMAGE:
        swprintf(Text, GlobalText[625]);
        break;
    case AT_IMPROVE_BLOCKING_PERCENT:
        swprintf(Text, GlobalText[626]);
        break;
    case AT_IMPROVE_GAIN_GOLD:
        swprintf(Text, GlobalText[627]);
        break;
    case AT_EXCELLENT_DAMAGE:
        swprintf(Text, GlobalText[628]);
        break;
    case AT_IMPROVE_DAMAGE_LEVEL:
        swprintf(Text, GlobalText[629]);
        break;
    case AT_IMPROVE_DAMAGE_PERCENT:
        swprintf(Text, GlobalText[630], Value);
        break;
    case AT_IMPROVE_MAGIC_LEVEL:
        swprintf(Text, GlobalText[631]);
        break;
    case AT_IMPROVE_MAGIC_PERCENT:
        swprintf(Text, GlobalText[632], Value);
        break;
    case AT_IMPROVE_ATTACK_SPEED:
        swprintf(Text, GlobalText[633], Value);
        break;
    case AT_IMPROVE_GAIN_LIFE:
        swprintf(Text, GlobalText[634]);
        break;
    case AT_IMPROVE_GAIN_MANA:
        swprintf(Text, GlobalText[635]);
        break;
    case AT_IMPROVE_EVADE:
        swprintf(Text, GlobalText[746]);
        break;
    case AT_SKILL_RIDER:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[745], iMana);
        break;
    case AT_SKILL_FORCE:  //
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[1210], iMana);
        break;
    case AT_SKILL_FORCE_WAVE:
    case AT_SKILL_FORCE_WAVE_STR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[1186], iMana);
        break;
    case AT_SKILL_EARTHSHAKE:
    case AT_SKILL_EARTHSHAKE_STR:
    case AT_SKILL_EARTHSHAKE_MASTERY:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[1189], iMana);
        break;
    case AT_SKILL_PLASMA_STORM_FENRIR:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[1928], iMana);
        break;
    case AT_SET_OPTION_IMPROVE_DEFENCE:
        swprintf(Text, GlobalText[959], Value);
        break;
    case AT_SET_OPTION_IMPROVE_CHARISMA:
        swprintf(Text, GlobalText[954], Value);
        break;
    case AT_SET_OPTION_IMPROVE_DAMAGE:
        swprintf(Text, GlobalText[577], Value);
        break;
    case AT_IMPROVE_HP_MAX:
        swprintf(Text, GlobalText[740], Value);
        break;
    case AT_IMPROVE_MP_MAX:
        swprintf(Text, GlobalText[741], Value);
        break;
    case AT_ONE_PERCENT_DAMAGE:
        swprintf(Text, GlobalText[742], Value);
        break;
    case AT_IMPROVE_AG_MAX:
        swprintf(Text, GlobalText[743], Value);
        break;
    case AT_DAMAGE_ABSORB:
        swprintf(Text, GlobalText[744], Value);
        break;
    case AT_SET_OPTION_IMPROVE_STRENGTH:
        swprintf(Text, GlobalText[985], Value);
        break;
    case AT_SET_OPTION_IMPROVE_DEXTERITY:
        swprintf(Text, GlobalText[986], Value);
        break;
    case AT_SET_OPTION_IMPROVE_VITALITY:
        swprintf(Text, GlobalText[987], Value);
        break;
    case AT_SET_OPTION_IMPROVE_ENERGY:
        swprintf(Text, GlobalText[988], Value);
        break;
    case AT_IMPROVE_MAX_MANA:
        swprintf(Text, GlobalText[1087], Value);
        break;
    case AT_IMPROVE_MAX_AG:
        swprintf(Text, GlobalText[1088], Value);
        break;
    case AT_DAMAGE_REFLECTION:
        swprintf(Text, GlobalText[1673], Value);
        break;
    case AT_RECOVER_FULL_LIFE:
        swprintf(Text, GlobalText[1674], Value);
        break;
    case AT_RECOVER_FULL_MANA:
        swprintf(Text, GlobalText[1675], Value);
        break;
    case AT_SKILL_SUMMON_EXPLOSION:
        swprintf(Text, GlobalText[1695], iMana);
        break;
    case AT_SKILL_SUMMON_REQUIEM:
        swprintf(Text, GlobalText[1696], iMana);
        break;
    case AT_SKILL_SUMMON_POLLUTION:
        swprintf(Text, GlobalText[1789], iMana);
        break;
    case AT_SKILL_KILLING_BLOW:
    case AT_SKILL_KILLING_BLOW_STR:
    case AT_SKILL_KILLING_BLOW_MASTERY:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[3153], iMana);
        break;
    case AT_SKILL_BEAST_UPPERCUT:
    case AT_SKILL_BEAST_UPPERCUT_STR:
    case AT_SKILL_BEAST_UPPERCUT_MASTERY:
        gSkillManager.GetSkillInformation(Option, 1, NULL, &iMana, NULL);
        swprintf(Text, GlobalText[3154], iMana);
        break;
    }
}

void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bItemTextListBoxUse)
{
    if (ip->Type == -1)
        return;

    tm* ExpireTime;
    if (ip->bPeriodItem == true && ip->bExpiredPeriod == false)
    {
        _tzset();
        if (ip->lExpireTime == 0)
            return;

        ExpireTime = localtime((time_t*)&(ip->lExpireTime));
    }

    ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];
    TextNum = 0;
    SkipNum = 0;

    ZeroMemory(TextListColor, 20 * sizeof(int));
    for (int i = 0; i < 30; i++)
    {
        TextList[i][0] = NULL;
    }

    if (!Sell && (ip->Type == ITEM_DARK_HORSE_ITEM || ip->Type == ITEM_DARK_RAVEN_ITEM))
    {
        static DebouncedAction debouncedPetInfoRequest([ip, Inventype]()
            {
                BYTE PetType = PET_TYPE_DARK_SPIRIT;
                if (ip->Type == ITEM_DARK_HORSE_ITEM)
                {
                    PetType = PET_TYPE_DARK_HORSE;

                    if ((g_pMyInventory->GetPointedItemIndex()) == EQUIPMENT_HELPER) 
                    {
                        SocketClient->ToGameServer()->SendPetInfoRequest(PetType, Inventype, EQUIPMENT_HELPER);
                    }
                }
                else if ((g_pMyInventory->GetPointedItemIndex()) == EQUIPMENT_WEAPON_LEFT) 
                {
                    SocketClient->ToGameServer()->SendPetInfoRequest(PetType, Inventype, EQUIPMENT_WEAPON_LEFT);
                }
            }, 1000); // 1-second intervals

        debouncedPetInfoRequest.invoke();

        giPetManager::RenderPetItemInfo(sx, sy, ip, Inventype);
        return;
    }

    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    int Level = ip->Level;
    int Color;

    if (ip->Type == ITEM_JEWEL_OF_BLESS || ip->Type == ITEM_JEWEL_OF_SOUL || ip->Type == ITEM_JEWEL_OF_CHAOS || ip->Type == ITEM_JEWEL_OF_GUARDIAN ||
        (COMGEM::isCompiledGem(ip)) ||
        ip->Type == ITEM_FLAME_OF_DEATH_BEAM_KNIGHT || ip->Type == ITEM_HORN_OF_HELL_MAINE || ip->Type == ITEM_FEATHER_OF_DARK_PHOENIX ||
        ip->Type == ITEM_EYE_OF_ABYSSAL ||
        ip->Type == ITEM_FLAME_OF_CONDOR || ip->Type == ITEM_FEATHER_OF_CONDOR ||
        ip->Type == ITEM_POTION + 100 ||
        (ip->Type >= ITEM_POTION + 141 && ip->Type <= ITEM_POTION + 144) ||
        (ip->Type >= ITEM_HELPER + 135 && ip->Type <= ITEM_HELPER + 145) ||
        (ip->Type == ITEM_POTION + 160 || ip->Type == ITEM_POTION + 161) ||
        ip->Type == ITEM_JEWEL_OF_LIFE || ip->Type == ITEM_JEWEL_OF_CREATION)
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else if (ip->Type == ITEM_DIVINE_STAFF_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_SWORD_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_CB_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_SCEPTER_OF_ARCHANGEL)
    {
        Color = TEXT_COLOR_PURPLE;
    }
    else if (ip->Type == ITEM_DEVILS_EYE || ip->Type == ITEM_DEVILS_KEY || ip->Type == ITEM_DEVILS_INVITATION)
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else if (ip->Type == ITEM_SCROLL_OF_ARCHANGEL || ip->Type == ITEM_BLOOD_BONE)
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else if (ip->AncientDiscriminator > 0)
    {
        Color = TEXT_COLOR_GREEN_BLUE;
    }
    else if (g_SocketItemMgr.IsSocketItem(ip))
    {
        Color = TEXT_COLOR_VIOLET;
    }
    else if (ip->SpecialNum > 0 && ip->ExcellentFlags > 0)
    {
        Color = TEXT_COLOR_GREEN;
    }
    else if (Level >= 7)
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else
    {
        if (ip->SpecialNum > 0)
        {
            Color = TEXT_COLOR_BLUE;
        }
        else
        {
            Color = TEXT_COLOR_WHITE;
        }
    }

    if ((ip->Type >= ITEM_WINGS_OF_SPIRITS && ip->Type <= ITEM_WINGS_OF_DARKNESS) || ip->Type == ITEM_CAPE_OF_LORD
        || (ip->Type >= ITEM_WING_OF_STORM && ip->Type <= ITEM_CAPE_OF_EMPEROR)
        || (ip->Type >= ITEM_WINGS_OF_DESPAIR && ip->Type <= ITEM_WING_OF_DIMENSION)
        || (ip->Type >= ITEM_CAPE_OF_FIGHTER && ip->Type <= ITEM_CAPE_OF_OVERRULE))
    {
        if (Level >= 7)
        {
            Color = TEXT_COLOR_YELLOW;
        }
        else
        {
            if (ip->SpecialNum > 0)
            {
                Color = TEXT_COLOR_BLUE;
            }
            else
            {
                Color = TEXT_COLOR_WHITE;
            }
        }
    }

    int nGemType = COMGEM::Check_Jewel(ip->Type);
    if (nGemType != COMGEM::NOGEM)
    {
        Color = TEXT_COLOR_YELLOW;
    }

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) && !IsSellingBan(ip))
    {
        wchar_t Text[100];
        {
            if (Sell)
            {
                DWORD dwValue = ItemValue(ip, 0);
                ConvertGold(dwValue, Text);
                wchar_t Text2[100];

                ConvertTaxGold(ItemValue(ip, 0), Text2);
                swprintf(TextList[TextNum], GlobalText[1620], Text2, Text);
            }
            else
            {
                ConvertGold(ItemValue(ip, 1), Text);
                swprintf(TextList[TextNum], GlobalText[63], Text);
            }

            TextListColor[TextNum] = Color;
            //			TextBold[TextNum] = true;
            TextNum++;
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        }
    }
    if ((Inventype == SEASON3B::TOOLTIP_TYPE_MY_SHOP || Inventype == SEASON3B::TOOLTIP_TYPE_PURCHASE_SHOP)
        && !IsPersonalShopBan(ip))
    {
        {
            int price = 0;
            int indexInv = g_pMyShopInventory->GetInventoryCtrl()->GetIndexByItem(ip);
            wchar_t Text[100];

            if (GetPersonalItemPrice(indexInv, price, g_IsPurchaseShop))
            {
                ConvertGold(price, Text);
                swprintf(TextList[TextNum], GlobalText[63], Text);

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
                swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

                DWORD gold = CharacterMachine->Gold;

                if ((int)gold < price && g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE)
                {
                    TextListColor[TextNum] = TEXT_COLOR_RED;
                    TextBold[TextNum] = true;
                    swprintf(TextList[TextNum], GlobalText[423]);
                    TextNum++;
                    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
                }
            }
            else if (g_IsPurchaseShop == PSHOPWNDTYPE_SALE)
            {
                TextListColor[TextNum] = TEXT_COLOR_RED;
                TextBold[TextNum] = true;
                swprintf(TextList[TextNum], GlobalText[1101]);
                TextNum++;
                swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            }
        }
    }

    if (ip->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && ip->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
    {
        if (ip->Type == ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR)
        {
            Color = TEXT_COLOR_YELLOW;
            switch (Level)
            {
            case 0: swprintf(TextList[TextNum], L"%s", p->Name); break;
            case 1: swprintf(TextList[TextNum], GlobalText[906]); break;
            }
        }
        else if (ip->Type == ITEM_BROKEN_SWORD_DARK_STONE)
        {
            Color = TEXT_COLOR_YELLOW;
            switch (Level)
            {
            case 0: swprintf(TextList[TextNum], L"%s", p->Name); break;
            case 1: swprintf(TextList[TextNum], GlobalText[907]); break;
            }
        }
        else {
            swprintf(TextList[TextNum], L"%s", p->Name);
            Color = TEXT_COLOR_YELLOW;
        }
    }
    else if (ip->Type == ITEM_POTION + 12)
    {
        switch (Level)
        {
        case 0:swprintf(TextList[TextNum], GlobalText[100]); break;
        case 1:swprintf(TextList[TextNum], GlobalText[101]); break;
        case 2:swprintf(TextList[TextNum], L"%s", ChaosEventName[ip->Durability]); break;
        }
    }
    else if (ip->Type == ITEM_BOX_OF_LUCK)
    {
        switch (Level)
        {
        case 0:swprintf(TextList[TextNum], L"%s", p->Name); break;
        case 1:swprintf(TextList[TextNum], GlobalText[105]); break;
        case 2:swprintf(TextList[TextNum], GlobalText[106]); break;
        case 3:swprintf(TextList[TextNum], GlobalText[107]); break;
        case 5:swprintf(TextList[TextNum], GlobalText[109]); break;
        case 6:swprintf(TextList[TextNum], GlobalText[110]); break;
        case 7:swprintf(TextList[TextNum], GlobalText[111]); break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            swprintf(TextList[TextNum], L"%s +%d", GlobalText[115], Level - 7);
            break;
        case 13:
            swprintf(TextList[TextNum], GlobalText[117]);
            break;
        case 14:
            swprintf(TextList[TextNum], GlobalText[1650]);
            break;
        case 15:
            swprintf(TextList[TextNum], GlobalText[1651]);
            break;
        }
    }
    else if (ip->Type == ITEM_POTION + 12)
    {
        switch (Level)
        {
        case 0:swprintf(TextList[TextNum], GlobalText[100]); break;
        case 1:swprintf(TextList[TextNum], GlobalText[101]); break;
        case 2:swprintf(TextList[TextNum], L"%s", ChaosEventName[ip->Durability]); break;
        }
    }
    else if (ip->Type == ITEM_FRUITS)
    {
        Color = TEXT_COLOR_YELLOW;
        switch (Level)
        {
        case 0:swprintf(TextList[TextNum], L"%s %s", GlobalText[168], p->Name); break;
        case 1:swprintf(TextList[TextNum], L"%s %s", GlobalText[169], p->Name); break;
        case 2:swprintf(TextList[TextNum], L"%s %s", GlobalText[167], p->Name); break;
        case 3:swprintf(TextList[TextNum], L"%s %s", GlobalText[166], p->Name); break;
        case 4:swprintf(TextList[TextNum], L"%s %s", GlobalText[1900], p->Name); break;
        }
    }
    else if (ip->Type == ITEM_LOCHS_FEATHER)
    {
        Color = TEXT_COLOR_YELLOW;
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], L"%s", p->Name); break;
        case 1: swprintf(TextList[TextNum], L"%s", GlobalText[1235]); break;
        }
    }
    else if (ip->Type == ITEM_POTION + 21)
    {
        Color = TEXT_COLOR_YELLOW;
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], L"%s", p->Name); break;
        case 1: swprintf(TextList[TextNum], GlobalText[810]); break;
        case 2: swprintf(TextList[TextNum], GlobalText[1098]); break;
        case 3: swprintf(TextList[TextNum], GlobalText[1290]); break;
        }
    }
    else if (ip->Type == ITEM_WEAPON_OF_ARCHANGEL)
    {
        Color = TEXT_COLOR_YELLOW;
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], GlobalText[811]); break;
        case 1: swprintf(TextList[TextNum], GlobalText[812]); break;
        case 2: swprintf(TextList[TextNum], GlobalText[817]); break;
        }
    }
    else if (ip->Type == ITEM_WIZARDS_RING)
    {
        Color = TEXT_COLOR_YELLOW;
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], p->Name); break;
        case 1: swprintf(TextList[TextNum], GlobalText[922]); break;
        case 2: swprintf(TextList[TextNum], GlobalText[928]); break;
        case 3: swprintf(TextList[TextNum], GlobalText[929]); break;
        }
    }
    else if (ip->Type == ITEM_HELPER + 107)
    {
        Color = TEXT_COLOR_YELLOW;
        swprintf(TextList[TextNum], p->Name);
    }
    else if (ip->Type == ITEM_SIEGE_POTION)
    {
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], GlobalText[1413]); break;
        case 1: swprintf(TextList[TextNum], GlobalText[1414]); break;
        }
    }
    else if (ip->Type == ITEM_HELPER + 7)
    {
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], GlobalText[1460]); break;
        case 1: swprintf(TextList[TextNum], GlobalText[1461]); break;
        }
    }
    else if (ip->Type == ITEM_LIFE_STONE_ITEM)
    {
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], GlobalText[1416]); break;
        case 1: swprintf(TextList[TextNum], GlobalText[1462]); break;
        }
    }
    else if (ip->Type == ITEM_ALE)
    {
        switch (Level)
        {
        case 0:swprintf(TextList[TextNum], L"%s", p->Name); break;
        case 1:swprintf(TextList[TextNum], GlobalText[108]); break;
        }
    }
    else if (ip->Type == ITEM_ORB_OF_SUMMONING)
    {
        swprintf(TextList[TextNum], L"%s %s", SkillAttribute[30 + Level].Name, GlobalText[102]);
    }
    else if (ip->Type == ITEM_TRANSFORMATION_RING)
    {
        for (int i = 0; i < MAX_MONSTER; i++)
        {
            if (SommonTable[Level] == MonsterScript[i].Type)
            {
                swprintf(TextList[TextNum], L"%s %s", MonsterScript[i].Name, GlobalText[103]);
                break;
            }
        }
    }
    else if (ip->Type >= ITEM_WINGS_OF_SPIRITS && ip->Type <= ITEM_WINGS_OF_DARKNESS)
    {
        if (Level == 0)
            swprintf(TextList[TextNum], L"%s", p->Name);
        else
            swprintf(TextList[TextNum], L"%s +%d", p->Name, Level);
    }
    else if ((ip->Type >= ITEM_WING_OF_STORM && ip->Type <= ITEM_CAPE_OF_EMPEROR) || (ip->Type >= ITEM_WINGS_OF_DESPAIR && ip->Type <= ITEM_WING_OF_DIMENSION)
        || (ip->Type >= ITEM_CAPE_OF_FIGHTER && ip->Type <= ITEM_CAPE_OF_OVERRULE))
    {
        if (Level == 0)
            swprintf(TextList[TextNum], L"%s", p->Name);
        else
            swprintf(TextList[TextNum], L"%s +%d", p->Name, Level);
    }
    else if (ip->Type == ITEM_SPIRIT)
    {
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], L"%s of %s", p->Name, GlobalText[1187]); break;
        case 1: swprintf(TextList[TextNum], L"%s of %s", p->Name, GlobalText[1214]); break;
        }
    }
    else if (ip->Type == ITEM_CAPE_OF_LORD)
    {
        if (Level == 0)
            swprintf(TextList[TextNum], L"%s", p->Name);
        else
            swprintf(TextList[TextNum], L"%s +%d", p->Name, Level);
    }
    else if (ip->Type == ITEM_SYMBOL_OF_KUNDUN)
    {
        swprintf(TextList[TextNum], GlobalText[1180], Level);
    }
    else if (ip->Type == ITEM_LOST_MAP)
    {
        Color = TEXT_COLOR_YELLOW;
        swprintf(TextList[TextNum], L"%s +%d", p->Name, Level);
    }
    else if (ip->Type == ITEM_RED_RIBBON_BOX)
    {
        swprintf(TextList[TextNum], L"%s", p->Name);
    }
    else if (ip->Type == ITEM_GREEN_RIBBON_BOX)
    {
        swprintf(TextList[TextNum], L"%s", p->Name);
    }
    else if (ip->Type == ITEM_BLUE_RIBBON_BOX)
    {
        swprintf(TextList[TextNum], L"%s", p->Name);
    }
    else if (ip->Type == ITEM_SCROLL_OF_FIRE_SCREAM)
    {
        swprintf(TextList[TextNum], L"%s", p->Name);
    }
    else if (ip->Type >= ITEM_PUMPKIN_OF_LUCK && ip->Type <= ITEM_JACK_OLANTERN_DRINK)
    {
        swprintf(TextList[TextNum], L"%s", p->Name);
    }
    else if (ip->Type == ITEM_PINK_CHOCOLATE_BOX)
    {
        switch (Level)
        {
        case 0:
            swprintf(TextList[TextNum], L"%s", p->Name); break;
        case 1:
            swprintf(TextList[TextNum], L"%s", GlobalText[2012]); break;
        }
    }
    else if (ip->Type == ITEM_RED_CHOCOLATE_BOX)
    {
        switch (Level)
        {
        case 0:
            swprintf(TextList[TextNum], L"%s", p->Name); break;
        case 1:
            swprintf(TextList[TextNum], L"%s", GlobalText[2013]); break;
        }
    }
    else if (ip->Type == ITEM_BLUE_CHOCOLATE_BOX)
    {
        switch (Level)
        {
        case 0:
            swprintf(TextList[TextNum], L"%s", p->Name); break;
        case 1:
            swprintf(TextList[TextNum], L"%s", GlobalText[2014]); break;
        }
    }
    else if (ip->Type >= ITEM_SPLINTER_OF_ARMOR && ip->Type <= ITEM_HORN_OF_FENRIR)
    {
        if (ip->Type == ITEM_HORN_OF_FENRIR)
        {
            Color = TEXT_COLOR_BLUE;
            if ((ip->ExcellentFlags & 63) == 0x01)
                swprintf(TextList[TextNum], L"%s %s", p->Name, GlobalText[1863]);
            else if ((ip->ExcellentFlags & 63) == 0x02)
                swprintf(TextList[TextNum], L"%s %s", p->Name, GlobalText[1864]);
            else if ((ip->ExcellentFlags & 63) == 0x04)
                swprintf(TextList[TextNum], L"%s %s", p->Name, GlobalText[1866]);
            else
                swprintf(TextList[TextNum], L"%s", p->Name);
        }
        else
        {
            Color = TEXT_COLOR_WHITE;
            swprintf(TextList[TextNum], L"%s", p->Name);
        }
    }
    else if (ip->Type == ITEM_DIVINE_SWORD_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_CB_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_STAFF_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_SCEPTER_OF_ARCHANGEL)
    {
        if (Level == 0)
            swprintf(TextList[TextNum], L"%s", p->Name);
        else
            swprintf(TextList[TextNum], L"%s +%d", p->Name, Level);
    }
    else if (nGemType != COMGEM::NOGEM && nGemType % 2 == 1)
    {
        int nGlobalIndex = COMGEM::GetJewelIndex(nGemType, COMGEM::eGEM_NAME);
        swprintf(TextList[TextNum], L"%s +%d", GlobalText[nGlobalIndex], Level + 1);
    }
    else if (ip->Type == ITEM_GEMSTONE || ip->Type == ITEM_JEWEL_OF_HARMONY ||
        ip->Type == ITEM_LOWER_REFINE_STONE || ip->Type == ITEM_HIGHER_REFINE_STONE ||
        ip->Type == ITEM_MOONSTONE_PENDANT
        )
    {
        swprintf(TextList[TextNum], L"%s", p->Name);
        Color = TEXT_COLOR_YELLOW;
    }
    else if ((ip->Type >= ITEM_SEED_FIRE && ip->Type <= ITEM_SEED_EARTH)
        || (ip->Type >= ITEM_SPHERE_MONO && ip->Type <= ITEM_SPHERE_5)
        || (ip->Type >= ITEM_SEED_SPHERE_FIRE_1 && ip->Type <= ITEM_SEED_SPHERE_EARTH_5))
    {
        swprintf(TextList[TextNum], L"%s", p->Name);
        Color = TEXT_COLOR_VIOLET;
    }
    else if (ip->Type == ITEM_POTION + 111)
    {
        Color = TEXT_COLOR_YELLOW;
        swprintf(TextList[TextNum], L"%s", p->Name);
    }
    else if (ITEM_SUSPICIOUS_SCRAP_OF_PAPER <= ip->Type && ip->Type <= ITEM_COMPLETE_SECROMICON)
    {
        Color = TEXT_COLOR_YELLOW;
        swprintf(TextList[TextNum], L"%s", p->Name);
    }
    else
    {
        wchar_t TextName[64];
        if (g_csItemOption.GetSetItemName(TextName, ip->Type, ip->AncientDiscriminator))
        {
            wcscat(TextName, p->Name);
        }
        else
        {
            wcscpy(TextName, p->Name);
        }

        if (ip->ExcellentFlags > 0)
        {
            if (Level == 0)
                swprintf(TextList[TextNum], L"%s %s", GlobalText[620], TextName);
            else
                swprintf(TextList[TextNum], L"%s %s +%d", GlobalText[620], TextName, Level);
        }
        else
        {
            if (Level == 0)
                swprintf(TextList[TextNum], L"%s", TextName);
            else
                swprintf(TextList[TextNum], L"%s +%d", TextName, Level);
        }
    }

    TextListColor[TextNum] = Color; TextBold[TextNum] = true; TextNum++;
    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    if (ip->Type == ITEM_WEAPON_OF_ARCHANGEL)
    {
        int iMana;
        int iWeaponSpeed;
        int iNeedStrength;
        int iNeedDex;

        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        swprintf(TextList[TextNum], GlobalText[730]); TextBold[TextNum] = false; TextNum++;

        TextListColor[TextNum] = TEXT_COLOR_DARKRED;
        swprintf(TextList[TextNum], GlobalText[815]); TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], L"\n"); TextBold[TextNum] = false; TextNum++; SkipNum++;

        switch (Level)
        {
        case 0:
            swprintf(TextList[TextNum], L"%s: %d ~ %d", GlobalText[42], 107, 110);  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            iWeaponSpeed = 20;
            iNeedStrength = 132;
            iNeedDex = 32;
            break;
        case 1:
            swprintf(TextList[TextNum], L"%s: %d ~ %d", GlobalText[40], 110, 120);  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            iWeaponSpeed = 35;
            iNeedStrength = 381;
            iNeedDex = 149;
            break;
        case 2:
            swprintf(TextList[TextNum], L"%s: %d ~ %d", GlobalText[41], 120, 140);  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            iWeaponSpeed = 35;
            iNeedStrength = 140;
            iNeedDex = 350;
            break;
        }

        swprintf(TextList[TextNum], GlobalText[64], iWeaponSpeed); TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[73], iNeedStrength); TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[75], iNeedDex);     TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], L"\n"); TextBold[TextNum] = false; TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[87]);     TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[94], 20); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;

        switch (Level)
        {
        case 0:
        {
            swprintf(TextList[TextNum], GlobalText[79], 53); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
            swprintf(TextList[TextNum], GlobalText[631]); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[632], 2); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        }
        break;
        case 1:
        {
            gSkillManager.GetSkillInformation(AT_SKILL_CYCLONE, 1, NULL, &iMana, NULL);
            swprintf(TextList[TextNum], GlobalText[84], iMana); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[629]); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[630], 2); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        }
        break;
        case 2:
        {
            gSkillManager.GetSkillInformation(AT_SKILL_TRIPLE_SHOT, 1, NULL, &iMana, NULL);
            swprintf(TextList[TextNum], GlobalText[86], iMana); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[629]); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[630], 2); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        }
        break;
        }
        swprintf(TextList[TextNum], GlobalText[628]); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[633], 7); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[634]); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[635]); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
    }

    if (ip->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && ip->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
    {
        swprintf(TextList[TextNum], GlobalText[730]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[731]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[732]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 12)
    {
        if (Level <= 1)
        {
            swprintf(TextList[TextNum], GlobalText[119]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }
    else if (ip->Type >= ITEM_HELPER + 46 && ip->Type <= ITEM_HELPER + 48)
    {
        int iMap = 0;
        if (ip->Type == ITEM_HELPER + 46)
            iMap = 39;
        else if (ip->Type == ITEM_HELPER + 47)
            iMap = 56;
        else if (ip->Type == ITEM_HELPER + 48)
            iMap = 58;

        swprintf(TextList[TextNum], GlobalText[2259], GlobalText[iMap]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2270]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], L"\n");
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }

    else if (ip->Type >= ITEM_HELPER + 125 && ip->Type <= ITEM_HELPER + 127)
    {
        int iMap = 0;
        if (ip->Type == ITEM_HELPER + 125)
            iMap = 3057;
        else if (ip->Type == ITEM_HELPER + 126)
            iMap = 2806;
        else if (ip->Type == ITEM_HELPER + 127)
            iMap = 3107;

        swprintf(TextList[TextNum], GlobalText[2259], GlobalText[iMap]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], L"\n");
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 54)
    {
        swprintf(TextList[TextNum], GlobalText[2261]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type >= ITEM_POTION + 58 && ip->Type <= ITEM_POTION + 62)
    {
        swprintf(TextList[TextNum], GlobalText[2269]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 83)
    {
        swprintf(TextList[TextNum], GlobalText[2269]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type >= ITEM_POTION + 145 && ip->Type <= ITEM_POTION + 150)
    {
        swprintf(TextList[TextNum], GlobalText[2269]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 53)
    {
        swprintf(TextList[TextNum], GlobalText[2250]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 43)
    {
        swprintf(TextList[TextNum], GlobalText[2256]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2297]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2567]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2568]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 44)
    {
        swprintf(TextList[TextNum], GlobalText[2257]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2297]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2567]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2568]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 45)
    {
        swprintf(TextList[TextNum], GlobalText[2258]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2297]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2566]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type >= ITEM_POTION + 70 && ip->Type <= ITEM_POTION + 71)
    {
        swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        int index = ip->Type - (ITEM_POTION + 70);

        swprintf(TextList[TextNum], GlobalText[2500 + index]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type >= ITEM_POTION + 72 && ip->Type <= ITEM_POTION + 77)
    {
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

        swprintf(TextList[TextNum], GlobalText[2503 + (ip->Type - (ITEM_POTION + 72))], Item_data.m_byValue1);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[2502]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 59)
    {
        swprintf(TextList[TextNum], GlobalText[2509]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type >= ITEM_HELPER + 54 && ip->Type <= ITEM_HELPER + 58)
    {
        DWORD statpoint = 0;
        statpoint = ip->Durability * 10;

        swprintf(TextList[TextNum], GlobalText[2511], statpoint);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[2510]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        TextListColor[TextNum] = TEXT_COLOR_DARKRED;
        swprintf(TextList[TextNum], GlobalText[1908]);
        TextNum++;

        if (ip->Type == ITEM_HELPER + 58)
        {
            if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE;
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_DARKRED;
            }

            swprintf(TextList[TextNum], GlobalText[61], GlobalText[24]);
            TextNum++;
        }
    }
    else if (ip->Type >= ITEM_POTION + 78 && ip->Type <= ITEM_POTION + 82)
    {
        int index = ip->Type - (ITEM_POTION + 78);
        DWORD value = 0;

        swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
        value = Item_data.m_byValue1;

        swprintf(TextList[TextNum], GlobalText[2512 + index], value);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2517]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2518]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        if (ip->Type == ITEM_POTION + 82)
        {
            swprintf(TextList[TextNum], GlobalText[3115]);
            TextListColor[TextNum] = TEXT_COLOR_YELLOW;
            TextBold[TextNum] = false;
            TextNum++;
        }

        std::wstring timetext;
        g_StringTime(Item_data.m_Time, timetext, true);
        swprintf(TextList[TextNum], timetext.c_str());
        TextListColor[TextNum] = TEXT_COLOR_PURPLE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[2302]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 60)
    {
        swprintf(TextList[TextNum], GlobalText[2519]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 62)
    {
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

        swprintf(TextList[TextNum], GlobalText[2253], Item_data.m_byValue1);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2569], Item_data.m_byValue2);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2570]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2566]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 63)
    {
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

        swprintf(TextList[TextNum], GlobalText[2254], Item_data.m_byValue1);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2571], Item_data.m_byValue2);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2572]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 97)
    {
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

        swprintf(TextList[TextNum], GlobalText[2580]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[2502]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 98)
    {
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

        swprintf(TextList[TextNum], GlobalText[2581]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2502]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 140)
    {
        swprintf(TextList[TextNum], GlobalText[92], 3);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2188], 100);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 96)
    {
        swprintf(TextList[TextNum], GlobalText[2573]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2574]);
        TextListColor[TextNum] = TEXT_COLOR_PURPLE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2708]);
        TextListColor[TextNum] = TEXT_COLOR_PURPLE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_DEMON)
    {
        swprintf(TextList[TextNum], GlobalText[2575]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2576]);
        TextListColor[TextNum] = TEXT_COLOR_PURPLE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_SPIRIT_OF_GUARDIAN)
    {
        swprintf(TextList[TextNum], GlobalText[2577]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2578]);
        TextListColor[TextNum] = TEXT_COLOR_PURPLE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_PET_RUDOLF)
    {
        swprintf(TextList[TextNum], GlobalText[2600]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_PET_SKELETON)
    {
        swprintf(TextList[TextNum], GlobalText[2600]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[3068]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[3069]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[3070]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_PET_PANDA)
    {
        swprintf(TextList[TextNum], GlobalText[2746]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2747]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2748]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_PET_UNICORN)
    {
        swprintf(TextList[TextNum], GlobalText[2746]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2744]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2748]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 107)
    {
        swprintf(TextList[TextNum], GlobalText[926]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 104)
    {
        swprintf(TextList[TextNum], GlobalText[2968]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 105)
    {
        swprintf(TextList[TextNum], GlobalText[2969]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 103)
    {
        swprintf(TextList[TextNum], GlobalText[2970], 170);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 69)
    {
        swprintf(TextList[TextNum], GlobalText[2602]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        if (g_PortalMgr.IsRevivePositionSaved())
        {
            swprintf(TextList[TextNum], GlobalText[2603]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            g_PortalMgr.GetRevivePositionText(TextList[TextNum]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }
    else if (ip->Type == ITEM_HELPER + 70)
    {
        swprintf(TextList[TextNum], GlobalText[2604]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 81)
    {
        swprintf(TextList[TextNum], GlobalText[2714]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
        swprintf(TextList[TextNum], GlobalText[2729]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
        swprintf(TextList[TextNum], GlobalText[3084]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
    }
    else if (ip->Type == ITEM_HELPER + 82)
    {
        swprintf(TextList[TextNum], GlobalText[2715]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
        swprintf(TextList[TextNum], GlobalText[2730]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
        swprintf(TextList[TextNum], GlobalText[2716]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
    }
    else if (ip->Type == ITEM_HELPER + 93)
    {
        swprintf(TextList[TextNum], GlobalText[2256]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
        swprintf(TextList[TextNum], GlobalText[2297]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
    }
    else if (ip->Type == ITEM_HELPER + 94)
    {
        swprintf(TextList[TextNum], GlobalText[2257]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
        swprintf(TextList[TextNum], GlobalText[2297]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum++] = false;
    }
    else if (ip->Type == ITEM_HELPER + 61)
    {
        swprintf(TextList[TextNum], GlobalText[2259], GlobalText[2369]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2270]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], L"\n");
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 91)
    {
        swprintf(TextList[TextNum], GlobalText[2551]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 92)
    {
        swprintf(TextList[TextNum], GlobalText[2261]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2553]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 93)
    {
        swprintf(TextList[TextNum], GlobalText[2261]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2556]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 95)
    {
        swprintf(TextList[TextNum], GlobalText[2261]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2552]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 94)
    {
        swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[2559]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_CHERRY_BLOSSOM_PLAYBOX)
    {
        swprintf(TextList[TextNum], GlobalText[2011]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_CHERRY_BLOSSOM_WINE)
    {
        swprintf(TextList[TextNum], GlobalText[2549]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        std::wstring timetext;
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
        g_StringTime(Item_data.m_Time, timetext, true);
        swprintf(TextList[TextNum], timetext.c_str());
        TextListColor[TextNum] = TEXT_COLOR_PURPLE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_CHERRY_BLOSSOM_RICE_CAKE)
    {
        swprintf(TextList[TextNum], GlobalText[2550]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        std::wstring timetext;
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
        g_StringTime(Item_data.m_Time, timetext, true);
        swprintf(TextList[TextNum], timetext.c_str());
        TextListColor[TextNum] = TEXT_COLOR_PURPLE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL)
    {
        swprintf(TextList[TextNum], GlobalText[2532]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        std::wstring timetext;
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
        g_StringTime(Item_data.m_Time, timetext, true);
        swprintf(TextList[TextNum], timetext.c_str());
        TextListColor[TextNum] = TEXT_COLOR_PURPLE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 88)
    {
        swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2534]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2535]);
        TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 89)
    {
        swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2534]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2536]);
        TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH)
    {
        swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2534]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2537]);
        TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_OLD_SCROLL)
    {
        swprintf(TextList[TextNum], GlobalText[2397]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_ILLUSION_SORCERER_COVENANT)
    {
        swprintf(TextList[TextNum], GlobalText[2398]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_SCROLL_OF_BLOOD)
    {
        swprintf(TextList[TextNum], GlobalText[2399]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 64)
    {
        swprintf(TextList[TextNum], GlobalText[2420]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type >= ITEM_FLAME_OF_DEATH_BEAM_KNIGHT && ip->Type <= ITEM_EYE_OF_ABYSSAL)
    {
        swprintf(TextList[TextNum], GlobalText[730]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[731]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[732]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_FLAME_OF_CONDOR)
    {
        swprintf(TextList[TextNum], GlobalText[1665]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_FEATHER_OF_CONDOR)
    {
        swprintf(TextList[TextNum], GlobalText[1665]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if ((ip->Type >= ITEM_SEED_FIRE && ip->Type <= ITEM_SEED_EARTH) || (ip->Type >= ITEM_SPHERE_MONO && ip->Type <= ITEM_SPHERE_5) || (ip->Type >= ITEM_SEED_SPHERE_FIRE_1 && ip->Type <= ITEM_SEED_SPHERE_EARTH_5))
    {
        TextNum = g_SocketItemMgr.AttachToolTipForSeedSphereItem(ip, TextNum);
    }
    else if (ip->Type == ITEM_HELPER + 71 || ip->Type == ITEM_HELPER + 72 || ip->Type == ITEM_HELPER + 73 || ip->Type == ITEM_HELPER + 74 || ip->Type == ITEM_HELPER + 75)
    {
        swprintf(TextList[TextNum], GlobalText[2709]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_MOONSTONE_PENDANT)
    {
        swprintf(TextList[TextNum], GlobalText[926]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_WIZARDS_RING)
    {
        switch (Level)
        {
        case 0:
        {
            swprintf(TextList[TextNum], GlobalText[926]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        break;

        case 1:
        {
            swprintf(TextList[TextNum], GlobalText[924], 40);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[731]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[732]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[733]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        break;
        case 2:
        {
            swprintf(TextList[TextNum], GlobalText[924], 80);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[731]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[732]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[733]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        break;
        case 3:
        {
            swprintf(TextList[TextNum], GlobalText[926]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        break;
        }
    }
    else if (ip->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN && ip->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END)
    {
        const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
        swprintf(TextList[TextNum], GlobalText[2717]);
        TextBold[TextNum] = false;
        TextNum++;

        switch (ip->Type)
        {
        case ITEM_TYPE_CHARM_MIXWING + EWS_KNIGHT_1_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2718], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_MAGICIAN_1_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2720], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_ELF_1_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2722], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_SUMMONER_1_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2724], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_DARKLORD_1_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2727], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_KNIGHT_2_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2719], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_MAGICIAN_2_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2721], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_ELF_2_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2723], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_SUMMONER_2_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2725], Item_data.m_byValue1);
        }break;
        case ITEM_TYPE_CHARM_MIXWING + EWS_DARKKNIGHT_2_CHARM:
        {
            swprintf(TextList[TextNum], GlobalText[2726], Item_data.m_byValue1);
        }break;
        }

        swprintf(TextList[TextNum], GlobalText[2732 + (ip->Type - (ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN))],
            Item_data.m_byValue1);

        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 110)
    {
        swprintf(TextList[TextNum], GlobalText[2773]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2774]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2775]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[1181], ip->Durability, 5);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2776], 5 - ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 111)
    {
        swprintf(TextList[TextNum], GlobalText[2777]);
        TextListColor[TextNum] = TEXT_COLOR_DARKBLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2778]);
        TextListColor[TextNum] = TEXT_COLOR_DARKBLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_SUSPICIOUS_SCRAP_OF_PAPER <= ip->Type && ip->Type <= ITEM_COMPLETE_SECROMICON)
    {
        switch (ip->Type)
        {
        case ITEM_SUSPICIOUS_SCRAP_OF_PAPER:
        {
            swprintf(TextList[TextNum], GlobalText[1181], ip->Durability, 5);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;

            swprintf(TextList[TextNum], GlobalText[2788]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }break;
        case ITEM_GAIONS_ORDER:
        {
            swprintf(TextList[TextNum], GlobalText[2784]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[2785]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[2786]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
        break;
        case ITEM_FIRST_SECROMICON_FRAGMENT:
        case ITEM_SECOND_SECROMICON_FRAGMENT:
        case ITEM_THIRD_SECROMICON_FRAGMENT:
        case ITEM_FOURTH_SECROMICON_FRAGMENT:
        case ITEM_FIFTH_SECROMICON_FRAGMENT:
        case ITEM_SIXTH_SECROMICON_FRAGMENT:
        {
            swprintf(TextList[TextNum], GlobalText[2790]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }break;
        case ITEM_COMPLETE_SECROMICON:
        {
            swprintf(TextList[TextNum], GlobalText[2792]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[2793]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }break;
        }
    }
    else if (ITEM_HELPER + 109 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[92], 3);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[3058], 4);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_HELPER + 110 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[92], 3);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[622], 4);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_HELPER + 111 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[92], 3);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[627], 50);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_HELPER + 112 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[92], 3);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[624], 4);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_HELPER + 113 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[92], 3);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[628], 10);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_HELPER + 114 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[92], 3);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2229], 7);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_HELPER + 115 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[92], 3);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[635]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_POTION + 112 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[2876]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_POTION + 113 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[2875]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 120)
    {
        swprintf(TextList[TextNum], GlobalText[2971]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_POTION + 121 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[2877]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_POTION + 122 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[2878]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_POTION + 123 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[2879]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_POTION + 124 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[2880]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_POTION + 134 <= ip->Type && ITEM_POTION + 139 >= ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[2972]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_HELPER + 116 == ip->Type)
    {
        swprintf(TextList[TextNum], GlobalText[3018]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;

        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ITEM_HELPER + 121 == ip->Type)
    {
        int iMap = 57;
        swprintf(TextList[TextNum], GlobalText[2259], GlobalText[iMap]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2270]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 124)
    {
        swprintf(TextList[TextNum], GlobalText[3116]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[3127]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = true;
        TextNum++;
    }
    else if (ip->Type >= ITEM_POTION + 141 && ip->Type <= ITEM_POTION + 144)
    {
        swprintf(TextList[TextNum], GlobalText[571]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 133)
    {
        swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[3267]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }

    Color = TEXT_COLOR_YELLOW;
    swprintf(TextList[TextNum], L"%s", p->Name);

    if (ip->Type == ITEM_DEVILS_INVITATION)
    {
        swprintf(TextList[TextNum], GlobalText[638]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[639]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }

    if (ip->DamageMin)
    {
        int minindex = 0, maxindex = 0, magicalindex = 0;

        if (Level >= ip->Jewel_Of_Harmony_OptionLevel)
        {
            StrengthenCapability SC;
            g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, ip, 1);

            if (SC.SI_isSP)
            {
                minindex = SC.SI_SP.SI_minattackpower;
                maxindex = SC.SI_SP.SI_maxattackpower;
                magicalindex = SC.SI_SP.SI_magicalpower;
            }
        }
        int DamageMin = ip->DamageMin;
        int DamageMax = ip->DamageMax;
        if (ip->Type >> 4 == 15)
        {
            swprintf(TextList[TextNum], L"%s: %d ~ %d", GlobalText[40 + 2], DamageMin, DamageMax);
        }
        else if (ip->Type != ITEM_SCROLL_OF_TELEPORT && ip->Type != ITEM_SCROLL_OF_TELEPORT_ALLY && ip->Type != ITEM_SCROLL_OF_SOUL_BARRIER)
        {
            if (ip->Type >= ITEM_ETC && ip->Type < ITEM_ETC + MAX_ITEM_INDEX)
            {
                auto SkillIndex = GetSkillByBook(ip->Type);

                SKILL_ATTRIBUTE* skillAtt = &SkillAttribute[SkillIndex];

                DamageMin = skillAtt->Damage;
                DamageMax = skillAtt->Damage + skillAtt->Damage / 2;

                swprintf(TextList[TextNum], L"%s: %d ~ %d", GlobalText[42], DamageMin, DamageMax);
            }
            else
            {
                if (DamageMin + minindex >= DamageMax + maxindex)
                    swprintf(TextList[TextNum], L"%s: %d ~ %d", GlobalText[40 + p->TwoHand], DamageMax + maxindex, DamageMax + maxindex);
                else
                    swprintf(TextList[TextNum], L"%s: %d ~ %d", GlobalText[40 + p->TwoHand], DamageMin + minindex, DamageMax + maxindex);
            }
        }
        else
        {
            TextNum--;
        }

        if (DamageMin > 0)
        {
            if (minindex != 0 || maxindex != 0)
            {
                TextListColor[TextNum] = TEXT_COLOR_YELLOW;
                TextBold[TextNum] = false;
                TextNum++;
            }
            else
            {
                if (ip->ExcellentFlags > 0)
                    TextListColor[TextNum] = TEXT_COLOR_BLUE;
                else
                    TextListColor[TextNum] = TEXT_COLOR_WHITE;
                TextBold[TextNum] = false;
                TextNum++;
            }
        }
        else
        {
            TextNum--;
        }
    }
    if (ip->Defense)
    {
        int maxdefense = 0;

        if (Level >= ip->Jewel_Of_Harmony_OptionLevel)
        {
            StrengthenCapability SC;
            g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, ip, 2);

            if (SC.SI_isSD)
            {
                maxdefense = SC.SI_SD.SI_defense;
            }
        }
        swprintf(TextList[TextNum], GlobalText[65], ip->Defense + maxdefense);

        if (maxdefense != 0)
            TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        else
        {
            if (ip->Type >= ITEM_HELM && ip->Type < ITEM_BOOTS + MAX_ITEM_INDEX && ip->ExcellentFlags > 0)
                TextListColor[TextNum] = TEXT_COLOR_BLUE;
            else
                TextListColor[TextNum] = TEXT_COLOR_WHITE;
        }

        TextBold[TextNum] = false;
        TextNum++;
    }
    if (ip->MagicDefense)
    {
        swprintf(TextList[TextNum], GlobalText[66], ip->MagicDefense);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    if (p->SuccessfulBlocking)
    {
        swprintf(TextList[TextNum], GlobalText[67], ip->SuccessfulBlocking);
        if (ip->ExcellentFlags > 0)
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
        else
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    if (p->WeaponSpeed)
    {
        swprintf(TextList[TextNum], GlobalText[64], p->WeaponSpeed);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    if (p->WalkSpeed)
    {
        swprintf(TextList[TextNum], GlobalText[68], p->WalkSpeed);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    if (ip->Type >= ITEM_RED_RIBBON_BOX && ip->Type <= ITEM_BLUE_RIBBON_BOX)
    {
        swprintf(TextList[TextNum], GlobalText[571]);
        switch (ip->Type)
        {
        case ITEM_RED_RIBBON_BOX:
            TextListColor[TextNum] = TEXT_COLOR_RED;
            break;
        case ITEM_GREEN_RIBBON_BOX:
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            break;
        case ITEM_BLUE_RIBBON_BOX:
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            break;
        }
        TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type >= ITEM_PUMPKIN_OF_LUCK && ip->Type <= ITEM_JACK_OLANTERN_DRINK) //Halloween event
    {
        wchar_t Text_data[300];
        ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

        switch (ip->Type)
        {
        case ITEM_PUMPKIN_OF_LUCK:
            swprintf(TextList[TextNum], GlobalText[2011]);
            TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
            break;
        case ITEM_JACK_OLANTERN_BLESSINGS:
            swprintf(Text_data, GlobalText[2229], Item_data.m_byValue1);
            swprintf(TextList[TextNum], Text_data);
            TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
            break;
        case ITEM_JACK_OLANTERN_WRATH:
            swprintf(Text_data, GlobalText[2230], Item_data.m_byValue1);
            swprintf(TextList[TextNum], Text_data);
            TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
            break;
        case ITEM_JACK_OLANTERN_CRY:
            swprintf(Text_data, GlobalText[2231], Item_data.m_byValue1);
            swprintf(TextList[TextNum], Text_data);
            TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
            break;
        case ITEM_JACK_OLANTERN_FOOD:
            swprintf(Text_data, GlobalText[960], Item_data.m_byValue1);
            swprintf(TextList[TextNum], Text_data);
            TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
            break;
        case ITEM_JACK_OLANTERN_DRINK:
            swprintf(Text_data, GlobalText[961], Item_data.m_byValue1);
            swprintf(TextList[TextNum], Text_data);
            TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
            break;
        }
        TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type >= ITEM_PINK_CHOCOLATE_BOX && ip->Type <= ITEM_BLUE_CHOCOLATE_BOX)
    {
        swprintf(TextList[TextNum], GlobalText[2011]);
        switch (ip->Type)
        {
        case ITEM_PINK_CHOCOLATE_BOX:
            if (Level == 0)
                TextListColor[TextNum] = TEXT_COLOR_PURPLE;
            else
                if (Level == 1)
                    TextListColor[TextNum] = TEXT_COLOR_PURPLE;
            break;
        case ITEM_RED_CHOCOLATE_BOX:
            if (Level == 0)
                TextListColor[TextNum] = TEXT_COLOR_RED;
            else
                if (Level == 1)
                    TextListColor[TextNum] = TEXT_COLOR_RED;
            break;
        case ITEM_BLUE_CHOCOLATE_BOX:
            if (Level == 0)
                TextListColor[TextNum] = TEXT_COLOR_BLUE;
            else
                if (Level == 1)
                    TextListColor[TextNum] = TEXT_COLOR_BLUE;
            break;
        }
        TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_BOX_OF_LUCK)
    {
        if (Level == 7)
        {
            swprintf(TextList[TextNum], GlobalText[112]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[113]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[114]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
        else if (Level == 14)
        {
            swprintf(TextList[TextNum], GlobalText[1652]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[1653]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
        else
        {
            swprintf(TextList[TextNum], GlobalText[571]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
        swprintf(TextList[TextNum], GlobalText[733]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
        if (Level == 13)
        {
            swprintf(TextList[TextNum], GlobalText[731]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }
    char tCount = COMGEM::CalcCompiledCount(ip);
    if (tCount > 0)
    {
        int	nJewelIndex = COMGEM::Check_Jewel_Com(ip->Type);
        if (nJewelIndex != COMGEM::NOGEM)
        {
            swprintf(TextList[TextNum], GlobalText[1819], tCount, GlobalText[COMGEM::GetJewelIndex(nJewelIndex, COMGEM::eGEM_NAME)]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[1820]);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
    }
    if (ip->Type == ITEM_JEWEL_OF_BLESS)
    {
        swprintf(TextList[TextNum], GlobalText[572]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_JEWEL_OF_SOUL)
    {
        swprintf(TextList[TextNum], GlobalText[573]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_JEWEL_OF_LIFE)
    {
        swprintf(TextList[TextNum], GlobalText[621]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_DEVILS_EYE || ip->Type == ITEM_DEVILS_KEY)
    {
        swprintf(TextList[TextNum], GlobalText[637]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_TOWN_PORTAL_SCROLL && Level >= 1 && Level <= 8)
    {
        swprintf(TextList[TextNum], GlobalText[157], 3);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_JEWEL_OF_CHAOS)
    {
        swprintf(TextList[TextNum], GlobalText[574]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_JEWEL_OF_CREATION)
    {
        swprintf(TextList[TextNum], GlobalText[619]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_JEWEL_OF_GUARDIAN)
    {
        swprintf(TextList[TextNum], GlobalText[1289]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_GUARDIAN_ANGEL) //
    {
        swprintf(TextList[TextNum], GlobalText[578], 20);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[739], 50);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (ip->Type == ITEM_IMP)
    {
        swprintf(TextList[TextNum], GlobalText[576]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if ((ip->Type >= ITEM_WING && ip->Type <= ITEM_WINGS_OF_SATAN) || ip->Type == ITEM_WING_OF_CURSE)
    {
        swprintf(TextList[TextNum], GlobalText[577], 12 + Level * 2);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[578], 12 + Level * 2);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[579]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_MOONSTONE_PENDANT)
    {
        swprintf(TextList[TextNum], GlobalText[2207]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_GEMSTONE)
    {
        swprintf(TextList[TextNum], GlobalText[2208]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_JEWEL_OF_HARMONY)
    {
        swprintf(TextList[TextNum], GlobalText[2209]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_LOWER_REFINE_STONE)
    {
        swprintf(TextList[TextNum], GlobalText[2210]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_HIGHER_REFINE_STONE)
    {
        swprintf(TextList[TextNum], GlobalText[2210]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 160)
    {
        // 연장의 보석
        swprintf(TextList[TextNum], GlobalText[3305]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 161)
    {
        // 상승의 보석
        swprintf(TextList[TextNum], GlobalText[2209]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if ((ip->Type >= ITEM_WINGS_OF_SPIRITS && ip->Type <= ITEM_WINGS_OF_DARKNESS) || ip->Type == ITEM_WINGS_OF_DESPAIR) //날개
    {
        swprintf(TextList[TextNum], GlobalText[577], 32 + Level);  //  데미지 몇%증가.
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[578], 25 + Level * 2);  //  데미지 몇%흡수.
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[579]);             //  이동 속도 향상.
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if ((ip->Type >= ITEM_WING_OF_STORM && ip->Type <= ITEM_CAPE_OF_EMPEROR) || ip->Type == ITEM_WING_OF_DIMENSION
        || ip->Type == ITEM_CAPE_OF_OVERRULE)
    {
        swprintf(TextList[TextNum], GlobalText[577], 39 + Level * 2);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        if (ip->Type == ITEM_CAPE_OF_EMPEROR || ip->Type == ITEM_CAPE_OF_OVERRULE)
        {
            swprintf(TextList[TextNum], GlobalText[578], 24 + Level * 2);
        }
        else
        {
            swprintf(TextList[TextNum], GlobalText[578], 39 + Level * 2);
        }
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[579]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ITEM_WING + 130 <= ip->Type && ip->Type <= ITEM_WING + 135)
    {
        switch (ip->Type)
        {
        case ITEM_WING + 130:
        case ITEM_WING + 135:
        {
            swprintf(TextList[TextNum], GlobalText[577], 20 + Level * 2);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[578], 20 + Level * 2);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }break;
        case ITEM_WING + 131:
        case ITEM_WING + 132:
        case ITEM_WING + 133:
        case ITEM_WING + 134:
        {
            swprintf(TextList[TextNum], GlobalText[577], 12 + Level * 2);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[578], 12 + Level * 2);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }break;
        }
        swprintf(TextList[TextNum], GlobalText[579]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_HORN_OF_DINORANT)
    {
        swprintf(TextList[TextNum], GlobalText[577], 15);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[578], 10);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    else if (ip->Type == ITEM_SPIRIT)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], GlobalText[1215]); TextNum++; break;
        case 1: swprintf(TextList[TextNum], GlobalText[1216]); TextNum++; break;
        }
    }
    else if (ip->Type == ITEM_LOCHS_FEATHER)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], L"%s", GlobalText[748]); TextNum++; break;
        case 1: swprintf(TextList[TextNum], L"%s", GlobalText[1236]); TextNum++; break;
        }
    }

    else if (ip->Type == ITEM_FRUITS)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch (Level)
        {
        case 0:swprintf(TextList[TextNum], L"%s %s", GlobalText[168], GlobalText[636]); break;
        case 1:swprintf(TextList[TextNum], L"%s %s", GlobalText[169], GlobalText[636]); break;
        case 2:swprintf(TextList[TextNum], L"%s %s", GlobalText[167], GlobalText[636]); break;
        case 3:swprintf(TextList[TextNum], L"%s %s", GlobalText[166], GlobalText[636]); break;
        case 4:swprintf(TextList[TextNum], L"%s %s", GlobalText[1900], GlobalText[636]); break;
        }
        TextNum++;
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch (Level)
        {
        case 0:swprintf(TextList[TextNum], L"%s %s", GlobalText[168], GlobalText[1910]); break;
        case 1:swprintf(TextList[TextNum], L"%s %s", GlobalText[169], GlobalText[1910]); break;
        case 2:swprintf(TextList[TextNum], L"%s %s", GlobalText[167], GlobalText[1910]); break;
        case 3:swprintf(TextList[TextNum], L"%s %s", GlobalText[166], GlobalText[1910]); break;
        case 4:swprintf(TextList[TextNum], L"%s %s", GlobalText[1900], GlobalText[1910]); break;
        }
        TextNum++;
        TextListColor[TextNum] = TEXT_COLOR_DARKRED;
        swprintf(TextList[TextNum], GlobalText[1908]);

        if (Level == 4)
        {
            TextNum++;
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            swprintf(TextList[TextNum], GlobalText[61], GlobalText[24]);
        }
        TextNum++;
    }
    else if (ip->Type == ITEM_SCROLL_OF_ARCHANGEL)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        swprintf(TextList[TextNum], GlobalText[816]);
        TextNum++;
    }
    else if (ip->Type == ITEM_BLOOD_BONE)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        swprintf(TextList[TextNum], GlobalText[816]);
        TextNum++;
    }
    else if (ip->Type == ITEM_INVISIBILITY_CLOAK)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        swprintf(TextList[TextNum], GlobalText[814]);
        TextNum++;
        swprintf(TextList[TextNum], L"\n");
        TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[638]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[639]);
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_SIEGE_POTION)
    {
        switch (Level)
        {
        case 0:
        {
            swprintf(TextList[TextNum], GlobalText[1417]); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[1418]); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[1419]); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }
        break;
        case 1:
        {
            swprintf(TextList[TextNum], GlobalText[1638]); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[1639]); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            swprintf(TextList[TextNum], GlobalText[1472]); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }
        break;
        }
    }
    else if (ip->Type == ITEM_HELPER + 7)
    {
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], GlobalText[1460]); break;
        case 1: swprintf(TextList[TextNum], GlobalText[1461]); break;
        }
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_LIFE_STONE_ITEM)
    {
        switch (Level)
        {
        case 0: swprintf(TextList[TextNum], GlobalText[1416]); break;
        case 1: swprintf(TextList[TextNum], GlobalText[1462]); break;
        }
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_ARMOR_OF_GUARDSMAN)
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
        swprintf(TextList[TextNum], L"%s %s    %s      %s    ", GlobalText[1147], GlobalText[368], GlobalText[935], GlobalText[936]); TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;

        for (int i = 0; i < 6; i++)
        {
            int Zen = g_iChaosCastleZen[i];

            swprintf(TextList[TextNum], L"        %d             %3d~%3d     %3d,000", i + 1, g_iChaosCastleLevel[startIndex + i][0], min(400, g_iChaosCastleLevel[startIndex + i][1]), Zen);
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
        swprintf(TextList[TextNum], L"         %d          %s   %3d,000", 7, GlobalText[737], 1000);
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

        swprintf(TextList[TextNum], L"\n");
        TextNum++;
        SkipNum++;
        swprintf(TextList[TextNum], GlobalText[1157]);
        TextListColor[TextNum] = TEXT_COLOR_DARKBLUE;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 21)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch (Level)
        {
        case 1: swprintf(TextList[TextNum], GlobalText[813]); break;
        case 2: swprintf(TextList[TextNum], GlobalText[1099]); break;
        case 3: swprintf(TextList[TextNum], GlobalText[1291]); break;
        default: break;
        }
        TextNum++;
    }
    else if (ip->Type == ITEM_LOST_MAP || ip->Type == ITEM_SYMBOL_OF_KUNDUN)
    {
        TextNum = RenderHellasItemInfo(ip, TextNum);
    }
    else if (ip->Type == ITEM_CAPE_OF_FIGHTER || ip->Type == ITEM_CAPE_OF_LORD)
    {
        // 망토 관련 옵션변경
        swprintf(TextList[TextNum], GlobalText[577], 20 + Level * 2);  //  데미지 몇%증가
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        int _iDamage = (ip->Type == ITEM_CAPE_OF_FIGHTER) ? 10 + Level * 2 : 10 + Level;
        swprintf(TextList[TextNum], GlobalText[578], _iDamage);  //  데미지 몇%흡수
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }

    BOOL bDurExist = FALSE;
    if ((p->Durability || p->MagicDur) &&
        ((ip->Type < ITEM_WING || ip->Type >= ITEM_HELPER) && ip->Type < ITEM_POTION) ||
        (ip->Type >= ITEM_WING && ip->Type <= ITEM_WINGS_OF_DARKNESS)
        || (ip->Type >= ITEM_WING_OF_STORM && ip->Type <= ITEM_WING_OF_DIMENSION)
        || (ip->Type >= ITEM_CAPE_OF_FIGHTER && ip->Type <= ITEM_CAPE_OF_OVERRULE))
    {
        bDurExist = TRUE;
    }
    if ((bDurExist || ip->Durability) &&
        (ip->Type<ITEM_LOCHS_FEATHER || ip->Type>ITEM_WEAPON_OF_ARCHANGEL)
        && !(ip->Type == ITEM_WIZARDS_RING && Level == 1)
        && !(ip->Type == ITEM_WIZARDS_RING && Level == 2)
        && !(ip->Type == ITEM_ARMOR_OF_GUARDSMAN)
        && ip->Type != ITEM_SIEGE_POTION && ip->Type != ITEM_HELPER + 7 && ip->Type != ITEM_LIFE_STONE_ITEM
        && ip->Type != ITEM_FRAGMENT_OF_HORN
        && !(ip->Type >= ITEM_POTION + 70 && ip->Type <= ITEM_POTION + 71)
        && !(ip->Type >= ITEM_HELPER + 54 && ip->Type <= ITEM_HELPER + 58)
        && !(ip->Type >= ITEM_POTION + 78 && ip->Type <= ITEM_POTION + 82)
        && !(ip->Type == ITEM_HELPER + 66)
        && !(ip->Type == ITEM_HELPER + 71 || ip->Type == ITEM_HELPER + 72 || ip->Type == ITEM_HELPER + 73 || ip->Type == ITEM_HELPER + 74 || ip->Type == ITEM_HELPER + 75)
        && !(ip->Type == ITEM_HELPER + 97)
        && !(ip->Type == ITEM_HELPER + 98)
        && !(ip->Type == ITEM_POTION + 91)
        && !(ip->Type == ITEM_HELPER + 99)
        && !(ip->Type == ITEM_POTION + 133)
        )
    {
        int Success = false;
        int arrow = false;
        if (ip->Type >= ITEM_POTION && ip->Type <= ITEM_ANTIDOTE)
        {
            swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
            Success = true;
        }
        else if (ip->Type == ITEM_POTION + 21 && Level == 3)
        {
            swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
            Success = true;
        }
        else if (ip->Type == ITEM_BOLT || ip->Type == ITEM_ARROWS)
        {
            swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
            Success = true;
            arrow = true;
        }
        else if (ip->Type >= ITEM_SMALL_SHIELD_POTION && ip->Type <= ITEM_LARGE_COMPLEX_POTION)
        {
            swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
            Success = true;
        }
        else if (ip->Type == ITEM_POTION + 133)
        {
            swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
            Success = true;
        }
        else if (ip->Type >= ITEM_JACK_OLANTERN_BLESSINGS && ip->Type <= ITEM_JACK_OLANTERN_DRINK)
        {
            swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
            Success = true;
        }
        else if (ip->Type >= ITEM_POTION + 153 && ip->Type <= ITEM_POTION + 156)
        {
            swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
            Success = true;
        }
        else if (ip->Type >= ITEM_SPLINTER_OF_ARMOR && ip->Type <= ITEM_BLESS_OF_GUARDIAN)
        {
            swprintf(TextList[TextNum], GlobalText[1181], ip->Durability, 20);
            Success = true;
        }
        else if (ip->Type == ITEM_CLAW_OF_BEAST)
        {
            swprintf(TextList[TextNum], GlobalText[1181], ip->Durability, 10);
            Success = true;
        }
        else if (ip->Type == ITEM_HORN_OF_FENRIR)
        {
            if (ip->bPeriodItem == false)
            {
                swprintf(TextList[TextNum], GlobalText[70], ip->Durability);
                Success = true;
            }
        }
        else if (ip->Type >= ITEM_HELPER && ip->Type <= ITEM_HELPER + 7)
        {
            if (ip->bPeriodItem == false)
            {
                swprintf(TextList[TextNum], GlobalText[70], ip->Durability);
                Success = true;
            }
        }
        else if (ip->Type == ITEM_TRANSFORMATION_RING)
        {
            swprintf(TextList[TextNum], GlobalText[95], ip->Durability);
            Success = true;
        }
        else if (ip->Type == ITEM_DEMON || ip->Type == ITEM_SPIRIT_OF_GUARDIAN)
        {
            if (ip->bPeriodItem == false)
            {
                swprintf(TextList[TextNum], GlobalText[70], ip->Durability);
                Success = true;
            }
        }
        else if (ip->Type == ITEM_PET_RUDOLF || ip->Type == ITEM_PET_PANDA
            || ip->Type == ITEM_PET_UNICORN || ip->Type == ITEM_PET_SKELETON)
        {
            if (ip->bPeriodItem == false)
            {
                swprintf(TextList[TextNum], GlobalText[70], ip->Durability);
                Success = true;
            }
        }
        else if (ip->Type >= ITEM_HELPER + 46 && ip->Type <= ITEM_HELPER + 48)
        {
            swprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
            Success = true;
        }
        else if (ip->Type >= ITEM_HELPER + 125 && ip->Type <= ITEM_HELPER + 127)
        {
            swprintf(TextList[TextNum], GlobalText[2260], ip->Durability);

            if (ip->Type == ITEM_HELPER + 126)
            {
                TextNum++;
                swprintf(TextList[TextNum], GlobalText[3105]);
            }
            else if (ip->Type == ITEM_HELPER + 127)
            {
                TextNum++;
                swprintf(TextList[TextNum], GlobalText[3106]);
            }
            Success = true;
        }
        else if (ip->Type == ITEM_POTION + 53)
        {
            swprintf(TextList[TextNum], GlobalText[2296], ip->Durability);
            Success = true;
        }
        else if (ip->Type == ITEM_HELPER + 61)
        {
            swprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
            Success = true;
        }
        else if (ip->Type == ITEM_POTION + 100)
        {
            swprintf(TextList[TextNum], GlobalText[69], ip->Durability);
            Success = true;
        }
        else if (ip->Type == ITEM_HELPER + 70)
        {
            if (ip->Durability == 2)
            {
                swprintf(TextList[TextNum], GlobalText[2605]);
                TextListColor[TextNum] = TEXT_COLOR_BLUE;
                TextBold[TextNum] = false;
                TextNum++;
            }
            else if (ip->Durability == 1)
            {
                swprintf(TextList[TextNum], GlobalText[2606]);
                TextListColor[TextNum] = TEXT_COLOR_BLUE;
                TextBold[TextNum] = false;
                TextNum++;
                g_PortalMgr.GetPortalPositionText(TextList[TextNum]);
                TextListColor[TextNum] = TEXT_COLOR_BLUE;
                TextBold[TextNum] = false;
                TextNum++;
            }
        }
        else if (ip->Type >= ITEM_HELPER + 135 && ip->Type <= ITEM_HELPER + 145)
        {
            swprintf(TextList[TextNum], GlobalText[2261]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            Success = true;
        }
        else if ((bDurExist) && (ip->bPeriodItem == false))
        {
            int maxDurability = CalcMaxDurability(ip, p, Level);

            swprintf(TextList[TextNum], GlobalText[71], ip->Durability, maxDurability);
            Success = true;
        }
        else if (ip->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN && ip->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END)
        {
            swprintf(TextList[TextNum], GlobalText[2732 + (ip->Type - (MODEL_TYPE_CHARM_MIXWING + EWS_BEGIN))]);

            Success = true;
        }
        else if (ip->Type == ITEM_HELPER + 121)
        {
            swprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
            Success = true;
        }

        if (Success)
        {
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }
    else
    {
        if (ip->Type == ITEM_TRANSFORMATION_RING)
        {
            swprintf(TextList[TextNum], GlobalText[95], ip->Durability);

            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    if (ip->Type == ITEM_BOLT || ip->Type == ITEM_ARROWS)
    {
        if (Level >= 1)
        {
            int value = Level * 2 + 1;

            swprintf(TextList[TextNum], GlobalText[577], value);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[88], 1);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    for (int i = 0; i < MAX_RESISTANCE; i++)
    {
        if (p->Resistance[i])
        {
            swprintf(TextList[TextNum], GlobalText[72], GlobalText[48 + i], Level + 1);
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    if (ip->RequireLevel && ip->Type != ITEM_LOCHS_FEATHER)
    {
        swprintf(TextList[TextNum], GlobalText[76], ip->RequireLevel);
        if (CharacterAttribute->Level < ip->RequireLevel)
        {
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[74], ip->RequireLevel - CharacterAttribute->Level);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        else
        {
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    int si_iNeedStrength = 0, si_iNeedDex = 0;
    bool bRequireStat = true;

    if (Check_LuckyItem(ip->Type)) bRequireStat = false;

    if (Level >= ip->Jewel_Of_Harmony_OptionLevel)
    {
        StrengthenCapability SC;
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, ip, 0);

        if (SC.SI_isNB)
        {
            si_iNeedStrength = SC.SI_NB.SI_force;
            si_iNeedDex = SC.SI_NB.SI_activity;
        }
    }

    if (ip->SocketCount > 0)
    {
        for (int i = 0; i < ip->SocketCount; ++i)
        {
            if (ip->SocketSeedID[i] == 38)
            {
                int iReqStrengthDown = g_SocketItemMgr.GetSocketOptionValue(ip, i);
                si_iNeedStrength += iReqStrengthDown;
            }
            else if (ip->SocketSeedID[i] == 39)
            {
                int iReqDexterityDown = g_SocketItemMgr.GetSocketOptionValue(ip, i);
                si_iNeedDex += iReqDexterityDown;
            }
        }
    }

    if (ip->RequireStrength && bRequireStat)
    {
        swprintf(TextList[TextNum], GlobalText[73], ip->RequireStrength - si_iNeedStrength);

        WORD Strength;
        Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        if (Strength < ip->RequireStrength - si_iNeedStrength)
        {
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[74], (ip->RequireStrength - Strength) - si_iNeedStrength);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        else
        {
            if (si_iNeedStrength != 0)
            {
                TextListColor[TextNum] = TEXT_COLOR_YELLOW;
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE;
            }

            TextBold[TextNum] = false;
            TextNum++;
        }
    }
    if (ip->RequireDexterity && bRequireStat)
    {
        swprintf(TextList[TextNum], GlobalText[75], ip->RequireDexterity - si_iNeedDex);
        WORD Dexterity;
        Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        if (Dexterity < (ip->RequireDexterity - si_iNeedDex))
        {
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;

            swprintf(TextList[TextNum], GlobalText[74], (ip->RequireDexterity - Dexterity) - si_iNeedDex);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        else
        {
            if (si_iNeedDex != 0)
            {
                TextListColor[TextNum] = TEXT_COLOR_YELLOW;
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE;
            }
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    if (ip->RequireVitality && bRequireStat) //  요구체력.
    {
        swprintf(TextList[TextNum], GlobalText[1930], ip->RequireVitality);

        WORD Vitality;
        Vitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
        if (Vitality < ip->RequireVitality)
        {
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[74], ip->RequireVitality - Vitality);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        else
        {
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    if (ip->RequireEnergy && bRequireStat)
    {
        swprintf(TextList[TextNum], GlobalText[77], ip->RequireEnergy);

        WORD Energy;
        Energy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;

        if (Energy < ip->RequireEnergy)
        {
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[74], ip->RequireEnergy - Energy);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        else
        {
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    if (ip->RequireCharisma && bRequireStat)
    {
        swprintf(TextList[TextNum], GlobalText[698], ip->RequireCharisma);

        WORD Charisma;
        Charisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
        if (Charisma < ip->RequireCharisma)
        {
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[74], ip->RequireCharisma - Charisma);
            TextListColor[TextNum] = TEXT_COLOR_RED;
            TextBold[TextNum] = false;
            TextNum++;
        }
        else
        {
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    if (IsRequireClassRenderItem(ip->Type))
    {
        RequireClass(p);
    }

    if (ip->Type >= MODEL_BOOTS - MODEL_ITEM && ip->Type < MODEL_BOOTS + MAX_ITEM_INDEX - MODEL_ITEM)
    {
        if (Level >= 5)
        {
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
            swprintf(TextList[TextNum], GlobalText[78]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
        }
    }

    if (ip->Type >= MODEL_GLOVES - MODEL_ITEM && ip->Type < MODEL_GLOVES + MAX_ITEM_INDEX - MODEL_ITEM)
    {
        if (Level >= 5)
        {
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

            swprintf(TextList[TextNum], GlobalText[93]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
        }
    }
    if ((ip->Type >= MODEL_STAFF - MODEL_ITEM && ip->Type < MODEL_STAFF + MAX_ITEM_INDEX - MODEL_ITEM)
        || (ip->Type == (MODEL_RUNE_BLADE - MODEL_ITEM))
        || (ip->Type == (MODEL_EXPLOSION_BLADE - MODEL_ITEM))
        || (ip->Type == (MODEL_SWORD_DANCER - MODEL_ITEM))
        || (ip->Type == (MODEL_DARK_REIGN_BLADE - MODEL_ITEM))
        || (ip->Type == (MODEL_IMPERIAL_SWORD - MODEL_ITEM))
        )
    {
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        int nText = ITEM_BOOK_OF_SAHAMUTT <= ip->Type && ip->Type <= ITEM_STAFF + 29 ? 1691 : 79;
        ::swprintf(TextList[TextNum], GlobalText[nText], ip->MagicPower);

        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = true;
        TextNum++;
    }

    if (IsCepterItem(ip->Type) == true)
    {
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[1234], ip->MagicPower);
        TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
    }

    if (ip->SpecialNum > 0)
    {
        int iModelType = ip->Type;
        int iStartModelType = ITEM_HELPER + 109;
        int iEndModelType = ITEM_HELPER + 115;

        if (!(ITEM_HELPER + 109 <= ip->Type && ITEM_HELPER + 115 >= ip->Type))
        {
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        }
    }

    if (ip->option_380 != 0)
    {
        std::vector<std::wstring> Text380;

        if (g_pItemAddOptioninfo)
        {
            g_pItemAddOptioninfo->GetItemAddOtioninfoText(Text380, ip->Type);

            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

            for (int i = 0; i < (int)Text380.size(); ++i)
            {
                wcsncpy(TextList[TextNum], Text380[i].c_str(), 100);
                TextListColor[TextNum] = TEXT_COLOR_REDPURPLE; TextBold[TextNum] = true; TextNum++;
            }

            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        }
    }
    //#ifndef PBG_MOD_NEWCHAR_MONK_WING_2
    //#ifdef PBG_MOD_NEWCHAR_MONK_WING
    //	if(ip->Type==ITEM_WING+49)
    //	{
    //		swprintf(TextList[TextNum],GlobalText[578],15+Level);
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
                    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

                    HARMONYJEWELOPTION harmonyjewel = g_pUIJewelHarmonyinfo->GetHarmonyJewelOptionInfo(type, ip->Jewel_Of_Harmony_Option);

                    if (type == SI_Defense && ip->Jewel_Of_Harmony_Option == 7)
                    {
                        swprintf(TextList[TextNum], L"%s +%d%%", harmonyjewel.Name, harmonyjewel.HarmonyJewelLevel[ip->Jewel_Of_Harmony_OptionLevel]);
                    }
                    else
                    {
                        swprintf(TextList[TextNum], L"%s +%d", harmonyjewel.Name, harmonyjewel.HarmonyJewelLevel[ip->Jewel_Of_Harmony_OptionLevel]);
                    }

                    if (Level >= ip->Jewel_Of_Harmony_OptionLevel) TextListColor[TextNum] = TEXT_COLOR_YELLOW;
                    else TextListColor[TextNum] = TEXT_COLOR_GRAY;

                    TextBold[TextNum] = true; TextNum++;

                    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
                }
                else
                {
                    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

                    swprintf(TextList[TextNum], L"%s : %d %d %d"
                        , GlobalText[2204]
                        , (int)type
                        , (int)ip->Jewel_Of_Harmony_Option
                        , (int)ip->Jewel_Of_Harmony_OptionLevel
                    );

                    TextListColor[TextNum] = TEXT_COLOR_DARKRED;
                    TextBold[TextNum] = true; TextNum++;

                    swprintf(TextList[TextNum], GlobalText[2205]);

                    TextListColor[TextNum] = TEXT_COLOR_DARKRED;
                    TextBold[TextNum] = true; TextNum++;

                    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
                }
            }
        }

    TextNum = g_csItemOption.RenderDefaultOptionText(ip, TextNum);

    int iMana;
    for (int i = 0; i < ip->SpecialNum; i++)
    {
        if (ITEM_HELPER + 109 <= ip->Type && ITEM_HELPER + 115 >= ip->Type)
        {
            break;
        }

        gSkillManager.GetSkillInformation(ip->Special[i], 1, NULL, &iMana, NULL);
        GetSpecialOptionText(ip->Type, TextList[TextNum], ip->Special[i], ip->SpecialValue[i], iMana);

        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false; TextNum++;

        if (ip->Special[i] == AT_LUCK)
        {
            swprintf(TextList[TextNum], GlobalText[94], ip->SpecialValue[i]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
        }
        else if (ip->Special[i] == AT_SKILL_RIDER)
        {
            swprintf(TextList[TextNum], GlobalText[179]);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }
        else if (ip->Special[i] == AT_SKILL_EARTHSHAKE
            || ip->Special[i] == AT_SKILL_EARTHSHAKE_STR
            || ip->Special[i] == AT_SKILL_EARTHSHAKE_MASTERY
            )
        {
            swprintf(TextList[TextNum], GlobalText[1201]);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }
        else if ((ip->Special[i] == AT_IMPROVE_DAMAGE) &&
            ((ip->Type == ITEM_RUNE_BLADE)
                || (ip->Type == ITEM_DARK_REIGN_BLADE)
                || (ip->Type == ITEM_EXPLOSION_BLADE)
                || (ip->Type == ITEM_SWORD_DANCER)
                || (ip->Type == ITEM_IMPERIAL_SWORD)
                )
            )
        {
            swprintf(TextList[TextNum], GlobalText[89], ip->SpecialValue[i]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false; TextNum++;
        }
    }

    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    if (ip->Type == ITEM_SPLINTER_OF_ARMOR || ip->Type == ITEM_BLESS_OF_GUARDIAN)
    {
        swprintf(TextList[TextNum], GlobalText[1917]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextNum++;
    }
    else if (ip->Type == ITEM_CLAW_OF_BEAST || ip->Type == ITEM_FRAGMENT_OF_HORN)
    {
        swprintf(TextList[TextNum], GlobalText[1918]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextNum++;
    }
    else if (ip->Type == ITEM_BROKEN_HORN)
    {
        swprintf(TextList[TextNum], GlobalText[1919]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextNum++;
    }
    else if (ip->Type == ITEM_HORN_OF_FENRIR)
    {
        GetSpecialOptionText(0, TextList[TextNum], AT_SKILL_PLASMA_STORM_FENRIR, 0, 0);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false; TextNum++; SkipNum++;
        if (ip->ExcellentFlags == 0x01)
        {
            swprintf(TextList[TextNum], GlobalText[1860], 10);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;

            swprintf(TextList[TextNum], GlobalText[579]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
        }
        else if (ip->ExcellentFlags == 0x02)
        {
            swprintf(TextList[TextNum], GlobalText[1861], 10);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;

            swprintf(TextList[TextNum], GlobalText[579]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
        }
        else if (ip->ExcellentFlags == 0x04)
        {
            WORD wLevel = CharacterAttribute->Level;

            swprintf(TextList[TextNum], GlobalText[1867], (wLevel / 2));
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[1868], (wLevel / 2));
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[1869], (wLevel / 12));
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[1870], (wLevel / 25));
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
            swprintf(TextList[TextNum], L"\n");
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[1871], (Hero->Level / 2));
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[1872], (Hero->Level / 2));
            TextListColor[TextNum] = TEXT_COLOR_GREEN;
            TextNum++;
        }

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        swprintf(TextList[TextNum], GlobalText[1920]);
        TextListColor[TextNum] = TEXT_COLOR_YELLOW;
        TextNum++;

        if (ip->ExcellentFlags == 0x00)
        {
            swprintf(TextList[TextNum], GlobalText[1929]);
            TextListColor[TextNum] = TEXT_COLOR_YELLOW;
            TextNum++;
        }
    }
    else if (ip->Type == ITEM_TRANSFORMATION_RING)
    {
        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_ELITE_TRANSFER_SKELETON_RING)
    {
        wchar_t strText[100];
        swprintf(strText, GlobalText[959], 10);
        swprintf(TextList[TextNum], L"%s%%", strText);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextNum++;

        WORD wlevel = CharacterAttribute->Level;
        swprintf(TextList[TextNum], GlobalText[2225], wlevel);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextNum++;

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_FIRECRACKER)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        swprintf(TextList[TextNum], GlobalText[2244]); TextNum++;
    }
    else if (ip->Type == ITEM_GM_GIFT)
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        swprintf(TextList[TextNum], GlobalText[2323]); TextNum++;
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        swprintf(TextList[TextNum], GlobalText[2011]); TextNum++;
    }
    else if (ip->Type == ITEM_JACK_OLANTERN_TRANSFORMATION_RING)
    {
        swprintf(TextList[TextNum], L"%s", GlobalText[2232]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;

        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
        TextNum++;

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_CHRISTMAS_TRANSFORMATION_RING)
    {
        swprintf(TextList[TextNum], GlobalText[88], 20);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[89], 20);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], L"%s", GlobalText[2248]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_PANDA_TRANSFORMATION_RING)
    {
        swprintf(TextList[TextNum], GlobalText[2743]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2744]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[2745]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_SKELETON_TRANSFORMATION_RING)
    {
        swprintf(TextList[TextNum], GlobalText[3065]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[3066]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[3067]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], GlobalText[3072]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_CHRISTMAS_STAR)
    {
        swprintf(TextList[TextNum], L"%s", GlobalText[2244]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextNum++;
    }
    else if (ip->Type == ITEM_GAME_MASTER_TRANSFORMATION_RING)
    {
        for (int i = 0; i < 7; ++i)
        {
            swprintf(TextList[TextNum], GlobalText[976 + i], 255);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextNum++;
        }

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }
    else if (ip->Type == ITEM_HELPER + 66)
    {
        TextNum--;
        swprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextNum++;
        swprintf(TextList[TextNum], L"%s", GlobalText[2589]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextNum++;
    }
    else if (ip->Type == ITEM_POTION + 100)
    {
        swprintf(TextList[TextNum], GlobalText[1887], ip->Durability);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextNum++;
    }
    else if (ip->Type == ITEM_PET_SKELETON)
    {
        TextNum--; SkipNum--;

        swprintf(TextList[TextNum], GlobalText[3071]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], GlobalText[3072]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    else if (g_pMyInventory->IsInvenItem(ip->Type))
    {
        TextNum--; SkipNum--;

        if (ip->Durability == 254)
        {
            swprintf(TextList[TextNum], GlobalText[3143]);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        }

        switch (ip->Type)
        {
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
        case ITEM_HELPER + 128:
        case ITEM_HELPER + 129:
            swprintf(TextList[TextNum], GlobalText[3121]);
            break;
        case ITEM_HELPER + 134:
            swprintf(TextList[TextNum], GlobalText[3123]);
            break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        case ITEM_HELPER + 130:
        case ITEM_HELPER + 131:
        case ITEM_HELPER + 132:
        case ITEM_HELPER + 133:
            swprintf(TextList[TextNum], GlobalText[3122]);
            break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

        default:
            break;
        }

        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        switch (ip->Type)
        {
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
        case ITEM_HELPER + 128:
            swprintf(TextList[TextNum], GlobalText[965], 10);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 129:
            swprintf(TextList[TextNum], GlobalText[967], 10);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 134:
            swprintf(TextList[TextNum], GlobalText[3126], 20);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        case ITEM_HELPER + 130:
            swprintf(TextList[TextNum], GlobalText[3132], 50);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 131:
            swprintf(TextList[TextNum], GlobalText[3134], 50);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 132:
#ifdef LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            swprintf(TextList[TextNum], GlobalText[3132], 100);
#else //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            swprintf(TextList[TextNum], GlobalText[3132], 150);
#endif //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
#ifdef LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            swprintf(TextList[TextNum], GlobalText[3133], 500);
#else //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            swprintf(TextList[TextNum], GlobalText[3133], 50);
#endif //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
        case ITEM_HELPER + 133:	// 골든메이플참
            swprintf(TextList[TextNum], GlobalText[3134], 150);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            swprintf(TextList[TextNum], GlobalText[3135], 50);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
            TextBold[TextNum] = false;
            TextNum++;
            break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        default:
            break;
        }

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        swprintf(TextList[TextNum], GlobalText[3124]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE;
        TextBold[TextNum] = false;
        TextNum++;
    }
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
    else if (ip->Type == ITEM_SNOWMAN_TRANSFORMATION_RING)
    {
        swprintf(TextList[TextNum], GlobalText[3088]);
        TextListColor[TextNum] = TEXT_COLOR_RED;
        TextBold[TextNum] = false;
        TextNum++;
    }

    if (ip->bPeriodItem == true)
    {
        if (ip->bExpiredPeriod == true)
        {
            swprintf(TextList[TextNum], GlobalText[3266]);
            TextListColor[TextNum] = TEXT_COLOR_RED;
        }
        else
        {
            swprintf(TextList[TextNum], GlobalText[3265]);
            TextListColor[TextNum] = TEXT_COLOR_ORANGE;
            TextNum++;
            SkipNum++;

            swprintf(TextList[TextNum], L"%d-%02d-%02d  %02d:%02d", ExpireTime->tm_year + 1900, ExpireTime->tm_mon + 1,
                ExpireTime->tm_mday, ExpireTime->tm_hour, ExpireTime->tm_min);
            TextListColor[TextNum] = TEXT_COLOR_BLUE;
        }

        TextNum++;
    }

    if (!bItemTextListBoxUse)
    {
        bool bThisisEquippedItem = false;

        SEASON3B::CNewUIInventoryCtrl* pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
        ITEM* pFindItem = pNewInventoryCtrl->FindItemByKey(ip->Key);
        (pFindItem == NULL) ? bThisisEquippedItem = true : bThisisEquippedItem = false;

        TextNum = g_csItemOption.RenderSetOptionListInItem(ip, TextNum, bThisisEquippedItem);

        TextNum = g_SocketItemMgr.AttachToolTipForSocketItem(ip, TextNum);

        SIZE TextSize = { 0, 0 };
        float fRateY = g_fScreenRate_y;
        int	Height = 0;
        int	EmptyLine = 0;
        int TextLine = 0;

        for (int i = 0; i < TextNum; ++i)
        {
            if (TextList[i][0] == '\0')		break;
            else if (TextList[i][0] == '\n')	++EmptyLine;
            else							++TextLine;
        }
        fRateY = fRateY / 1.1f;
        g_pRenderText->SetFont(g_hFont);

        GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);

        Height = (TextLine * TextSize.cy + EmptyLine * TextSize.cy / 2.0f) / fRateY;

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
        if (bItemTextListBoxUse)
            RenderTipTextList(sx, sy, TextNum, 0, RT3_SORT_CENTER, STRP_BOTTOMCENTER);
        else
            RenderTipTextList(sx, sy, TextNum, 0);
    }
}

void RenderRepairInfo(int sx, int sy, ITEM* ip, bool Sell)
{
    if (IsRepairBan(ip) == true)
    {
        return;
    }
    if (ip->Type >= MODEL_TYPE_CHARM_MIXWING + EWS_BEGIN && ip->Type <= MODEL_TYPE_CHARM_MIXWING + EWS_END)
    {
        return;
    }
    if (ip->Type == ITEM_HELPER + 107)
    {
        return;
    }
    if (ip->Type == ITEM_HELPER + 104)
    {
        return;
    }
    if (ip->Type == ITEM_HELPER + 105)
    {
        return;
    }
    if (ip->Type == ITEM_HELPER + 103)
    {
        return;
    }
    if (ip->Type == ITEM_POTION + 133)
    {
        return;
    }
    if (ip->Type == MODEL_HELPER + 109)
    {
        return;
    }
    if (ip->Type == MODEL_HELPER + 110)
    {
        return;
    }
    if (ip->Type == MODEL_HELPER + 111)
    {
        return;
    }
    if (ip->Type == MODEL_HELPER + 112)
    {
        return;
    }
    if (ip->Type == MODEL_HELPER + 113)
    {
        return;
    }
    if (ip->Type == MODEL_HELPER + 114)
    {
        return;
    }
    if (ip->Type == MODEL_HELPER + 115)
    {
        return;
    }
    if (ip->Type == MODEL_POTION + 112)
    {
        return;
    }
    if (ip->Type == MODEL_POTION + 113)
    {
        return;
    }
    if (ip->Type == ITEM_POTION + 120)
    {
        return;
    }
    if (ip->Type == ITEM_POTION + 121)
    {
        return;
    }
    if (ip->Type == ITEM_POTION + 122)
    {
        return;
    }
    if (ITEM_POTION + 123 == ip->Type)
    {
        return;
    }
    if (ITEM_POTION + 124 == ip->Type)
    {
        return;
    }
    if (ITEM_POTION + 134 <= ip->Type && ip->Type <= ITEM_POTION + 139)
    {
        return;
    }

    if (ITEM_WING + 130 <= ip->Type && ip->Type <= ITEM_WING + 135)
    {
        return;
    }

    if (ITEM_POTION + 114 <= ip->Type && ip->Type <= ITEM_POTION + 119)
    {
        return;
    }
    if (ITEM_POTION + 126 <= ip->Type && ip->Type <= ITEM_POTION + 129)
    {
        return;
    }
    if (ITEM_POTION + 130 <= ip->Type && ip->Type <= ITEM_POTION + 132)
    {
        return;
    }
    if (ITEM_HELPER + 121 == ip->Type)
    {
        return;
    }

    ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];
    TextNum = 0;
    SkipNum = 0;
    for (int i = 0; i < 30; i++)
    {
        TextList[i][0] = NULL;
    }

    int Level = ip->Level;
    int Color;

    if (ip->Type == ITEM_JEWEL_OF_BLESS || ip->Type == ITEM_JEWEL_OF_SOUL || ip->Type == ITEM_JEWEL_OF_CHAOS)
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else if (COMGEM::isCompiledGem(ip))
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else if (ip->Type == ITEM_DIVINE_STAFF_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_SWORD_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_CB_OF_ARCHANGEL || ip->Type == ITEM_DIVINE_SCEPTER_OF_ARCHANGEL)
    {
        Color = TEXT_COLOR_PURPLE;
    }
    else if (ip->Type == ITEM_DEVILS_EYE || ip->Type == ITEM_DEVILS_KEY || ip->Type == ITEM_DEVILS_INVITATION)
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else if (ip->SpecialNum > 0 && ip->ExcellentFlags > 0)
    {
        Color = TEXT_COLOR_GREEN;
    }
    else if (Level >= 7)
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else
    {
        if (ip->SpecialNum > 0)
        {
            Color = TEXT_COLOR_BLUE;
        }
        else
        {
            Color = TEXT_COLOR_WHITE;
        }
    }

    if ((ip->Type >= ITEM_WINGS_OF_SPIRITS && ip->Type <= ITEM_WINGS_OF_DARKNESS)
        || ip->Type >= ITEM_CAPE_OF_LORD
        || (ip->Type >= ITEM_WING_OF_STORM && ip->Type <= ITEM_CAPE_OF_EMPEROR)
        || (ip->Type >= ITEM_WINGS_OF_DESPAIR && ip->Type <= ITEM_WING_OF_DIMENSION)
        || (ip->Type >= ITEM_CAPE_OF_FIGHTER && ip->Type <= ITEM_CAPE_OF_OVERRULE))
    {
        if (Level >= 7)
        {
            Color = TEXT_COLOR_YELLOW;
        }
        else
        {
            if (ip->SpecialNum > 0)
            {
                Color = TEXT_COLOR_BLUE;
            }
            else
            {
                Color = TEXT_COLOR_WHITE;
            }
        }
    }

    if (ip->Type < ITEM_POTION)
    {
        int maxDurability;

        swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

        wchar_t Text[100];

        maxDurability = CalcMaxDurability(ip, p, Level);
        if (ip->Durability < maxDurability)
        {
            RepairEnable = 2;

            int iGold = ItemValue(ip, 2);
            if (iGold == -1)
                return;
            ConvertRepairGold(iGold, ip->Durability, maxDurability, ip->Type, Text);
            swprintf(TextList[TextNum], GlobalText[238], Text);

            TextListColor[TextNum] = Color;
        }
        else
        {
            RepairEnable = 1;
            swprintf(TextList[TextNum], GlobalText[238], L"0");
            TextListColor[TextNum] = Color;
        }
        TextBold[TextNum] = true;
        TextNum++;

        //        RepairEnable = 1;
    }
    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    if (ip->Type == ITEM_ORB_OF_SUMMONING)
    {
        swprintf(TextList[TextNum], L"%s %s", SkillAttribute[30 + Level].Name, GlobalText[102]);
    }
    else if (ip->Type == ITEM_TRANSFORMATION_RING)
    {
        for (int i = 0; i < MAX_MONSTER; i++)
        {
            if (SommonTable[Level] == MonsterScript[i].Type)
            {
                swprintf(TextList[TextNum], L"%s %s", MonsterScript[i].Name, GlobalText[103]);
                break;
            }
        }
    }
    else if ((ip->Type == ITEM_DARK_HORSE_ITEM) || (ip->Type == ITEM_DARK_RAVEN_ITEM))
    {
        swprintf(TextList[TextNum], L"%s", p->Name);
    }
    else if ((ip->Type >= ITEM_WINGS_OF_SPIRITS && ip->Type <= ITEM_WINGS_OF_DARKNESS)
        || ip->Type >= ITEM_CAPE_OF_LORD
        || (ip->Type >= ITEM_WING_OF_STORM && ip->Type <= ITEM_CAPE_OF_EMPEROR)
        || (ip->Type >= ITEM_WINGS_OF_DESPAIR && ip->Type <= ITEM_WING_OF_DIMENSION)
        || (ip->Type >= ITEM_CAPE_OF_FIGHTER && ip->Type <= ITEM_CAPE_OF_OVERRULE))
    {
        if (Level == 0)
            swprintf(TextList[TextNum], L"%s", p->Name);
        else
            swprintf(TextList[TextNum], L"%s +%d", p->Name, Level);
    }
    else
    {
        if (ip->ExcellentFlags > 0)
        {
            if (Level == 0)
                swprintf(TextList[TextNum], L"%s %s", GlobalText[620], p->Name);
            else
                swprintf(TextList[TextNum], L"%s %s +%d", GlobalText[620], p->Name, Level);
        }
        else
        {
            if (Level == 0)
                swprintf(TextList[TextNum], L"%s", p->Name);
            else
                swprintf(TextList[TextNum], L"%s +%d", p->Name, Level);
        }
    }
    TextListColor[TextNum] = Color; TextBold[TextNum] = true; TextNum++;
    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    if (ip->Type < ITEM_POTION)
    {
        if (ip->bPeriodItem == false)
        {
            int maxDurability = CalcMaxDurability(ip, p, Level);

            swprintf(TextList[TextNum], GlobalText[71], ip->Durability, maxDurability);

            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            TextBold[TextNum] = false;
            TextNum++;
        }
    }

    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    SIZE TextSize = { 0, 0 };

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);

    int Height = ((TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2) * 480 / WindowHeight;
    if (sy - Height >= 0)
        sy -= Height;
    else
        sy += p->Height * INVENTORY_SCALE;

    RenderTipTextList(sx, sy, TextNum, 0);
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
    else if (r->Type >= ITEM_STAFF && r->Type < ITEM_SHIELD)
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
        if (l->Type >= ITEM_SWORD && l->Type < ITEM_STAFF + MAX_ITEM_INDEX && r->Type >= ITEM_SWORD && r->Type < ITEM_STAFF + MAX_ITEM_INDEX)
        {
            Alpha = true;
            AttackDamageMin = ((CharacterAttribute->AttackDamageMinRight * 55) / 100 + (CharacterAttribute->AttackDamageMinLeft * 55) / 100);
            AttackDamageMax = ((CharacterAttribute->AttackDamageMaxRight * 55) / 100 + (CharacterAttribute->AttackDamageMaxLeft * 55) / 100);
        }
    }
    else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        if ((r->Type >= ITEM_BOW && r->Type < ITEM_BOW + MAX_ITEM_INDEX) &&
            (l->Type >= ITEM_BOW && l->Type < ITEM_BOW + MAX_ITEM_INDEX))
        {
            if ((l->Type == ITEM_BOLT && l->Level >= 1) || (r->Type == ITEM_ARROWS && r->Level >= 1))
            {
                Alpha = true;
            }
        }
    }
    else if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
    {
        if (l->Type >= ITEM_SWORD && l->Type < ITEM_MACE + MAX_ITEM_INDEX && r->Type >= ITEM_SWORD && r->Type < ITEM_MACE + MAX_ITEM_INDEX)
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

void RenderSkillInfo(int sx, int sy, int Type, int SkillNum, int iRenderPoint /*= STRP_NONE*/)
{
    wchar_t lpszName[256];
    int  iMinDamage, iMaxDamage;
    int  HeroClass = gCharacterManager.GetBaseClass(Hero->Class);
    int  iMana, iDistance, iSkillMana;
    int  TextNum = 0;
    int  SkipNum = 0;

    if (giPetManager::RenderPetCmdInfo(sx, sy, Type)) return;

    int  AttackDamageMin, AttackDamageMax;
    int  iSkillMinDamage, iSkillMaxDamage;

    int  SkillType = CharacterAttribute->Skill[Type];
    gCharacterManager.GetMagicSkillDamage(CharacterAttribute->Skill[Type], &iMinDamage, &iMaxDamage);
    gCharacterManager.GetSkillDamage(CharacterAttribute->Skill[Type], &iSkillMinDamage, &iSkillMaxDamage);

    GetAttackDamage(&AttackDamageMin, &AttackDamageMax);

    iSkillMinDamage += AttackDamageMin;
    iSkillMaxDamage += AttackDamageMax;
    gSkillManager.GetSkillInformation(CharacterAttribute->Skill[Type], 1, lpszName, &iMana, &iDistance, &iSkillMana);

    if (CharacterAttribute->Skill[Type] == AT_SKILL_FORCE && Hero->Weapon[0].Type != -1)
    {
        for (int i = 0; i < CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].SpecialNum; i++)
        {
            if (CharacterMachine->Equipment[0].Special[i] == AT_SKILL_FORCE_WAVE)
            {
                swprintf(lpszName, L"%s", GlobalText[1200]);
                break;
            }
        }
    }

    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;
    swprintf(TextList[TextNum], L"%s", lpszName);
    TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    WORD Dexterity;
    WORD Energy;
    WORD Strength;
    WORD Vitality;
    WORD Charisma;

    Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
    Energy = CharacterAttribute->Energy + CharacterAttribute->AddEnergy;
    Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
    Vitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
    Charisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

    int skillattackpowerRate = 0;

    StrengthenCapability rightinfo, leftinfo;

    ITEM* rightweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
    ITEM* leftweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];

    int rightlevel = rightweapon->Level;

    if (rightlevel >= rightweapon->Jewel_Of_Harmony_OptionLevel)
    {
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&rightinfo, rightweapon, 1);
    }

    int leftlevel = leftweapon->Level;

    if (leftlevel >= leftweapon->Jewel_Of_Harmony_OptionLevel)
    {
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&leftinfo, leftweapon, 1);
    }

    if (rightinfo.SI_isSP)
    {
        skillattackpowerRate += rightinfo.SI_SP.SI_skillattackpower;
        skillattackpowerRate += rightinfo.SI_SP.SI_magicalpower;
    }
    if (leftinfo.SI_isSP)
    {
        skillattackpowerRate += leftinfo.SI_SP.SI_skillattackpower;
    }

    if (HeroClass == CLASS_WIZARD || HeroClass == CLASS_SUMMONER)
    {
        if (CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER
            || CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER_STR
            || CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER_PROFICIENCY
            )
        {
            int iDamageShield;
            
            if (CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER)
            {
                iDamageShield = (int)(10 + (Dexterity / 50.f) + (Energy / 200.f));
            }
            else if (CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_SOUL_BARRIER_PROFICIENCY)
            {
                auto additionalValue = CharacterAttribute->MasterSkillInfo[CharacterAttribute->Skill[Type]].GetSkillValue();
                iDamageShield = (int)(10 + (Dexterity / 50.f) + (Energy / 200.f));
                iDamageShield += (int)additionalValue;
            }

            int iDeleteMana = (int)(CharacterAttribute->ManaMax * 0.02f);
            int iLimitTime = (int)(60 + (Energy / 40.f));

            swprintf(TextList[TextNum], GlobalText[578], iDamageShield);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;

            swprintf(TextList[TextNum], GlobalText[880], iDeleteMana);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;

            swprintf(TextList[TextNum], GlobalText[881], iLimitTime);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
        else if (SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY
            && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
            && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY
            && SkillType != AT_SKILL_ALICE_SLEEP
            && SkillType != AT_SKILL_ALICE_SLEEP_STR
            )
        {
            WORD bySkill = CharacterAttribute->Skill[Type];
            if (!(AT_SKILL_STUN <= bySkill && bySkill <= AT_SKILL_MANA) && !(AT_SKILL_ALICE_THORNS <= bySkill && bySkill <= AT_SKILL_ALICE_ENERVATION) && bySkill != AT_SKILL_TELEPORT && bySkill != AT_SKILL_TELEPORT_ALLY)
            {
                if (AT_SKILL_SUMMON_EXPLOSION <= bySkill && bySkill <= AT_SKILL_SUMMON_POLLUTION)
                {
                    gCharacterManager.GetCurseSkillDamage(bySkill, &iMinDamage, &iMaxDamage);
                    swprintf(TextList[TextNum], GlobalText[1692], iMinDamage, iMaxDamage);
                }
                else
                    swprintf(TextList[TextNum], GlobalText[170], iMinDamage + skillattackpowerRate, iMaxDamage + skillattackpowerRate);
                TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            }
        }
    }
    if (HeroClass == CLASS_KNIGHT || HeroClass == CLASS_DARK || HeroClass == CLASS_ELF || HeroClass == CLASS_DARK_LORD
        || HeroClass == CLASS_RAGEFIGHTER)
    {
        switch (CharacterAttribute->Skill[Type])
        {
        case AT_SKILL_TELEPORT:
        case AT_SKILL_TELEPORT_ALLY:
        case AT_SKILL_SOUL_BARRIER:
        case AT_SKILL_SOUL_BARRIER_STR:
        case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
        case AT_SKILL_BLOCKING:
        case AT_SKILL_SWELL_LIFE:
        case AT_SKILL_SWELL_LIFE_STR:
        case AT_SKILL_SWELL_LIFE_PROFICIENCY:
        case AT_SKILL_HEALING:
        case AT_SKILL_HEALING_STR:
        case AT_SKILL_DEFENSE:
        case AT_SKILL_DEFENSE_STR:
        case AT_SKILL_DEFENSE_MASTERY:
        case AT_SKILL_ATTACK:
        case AT_SKILL_ATTACK_STR:
        case AT_SKILL_ATTACK_MASTERY:
        case AT_SKILL_SUMMON:
        case AT_SKILL_SUMMON + 1:
        case AT_SKILL_SUMMON + 2:
        case AT_SKILL_SUMMON + 3:
        case AT_SKILL_SUMMON + 4:
        case AT_SKILL_SUMMON + 5:
        case AT_SKILL_SUMMON + 6:
        case AT_SKILL_SUMMON + 7:
        case AT_SKILL_IMPROVE_AG:
        case AT_SKILL_STUN:
        case AT_SKILL_REMOVAL_STUN:
        case AT_SKILL_MANA:
        case AT_SKILL_INVISIBLE:
        case AT_SKILL_REMOVAL_INVISIBLE:
        case AT_SKILL_REMOVAL_BUFF:
            break;
        case AT_SKILL_PARTY_TELEPORT:
        case AT_SKILL_ADD_CRITICAL:
        case AT_SKILL_ADD_CRITICAL_STR1:
        case AT_SKILL_ADD_CRITICAL_STR2:
        case AT_SKILL_ADD_CRITICAL_STR3:
            break;
        case AT_SKILL_EARTHSHAKE:
        case AT_SKILL_EARTHSHAKE_STR:
        case AT_SKILL_EARTHSHAKE_MASTERY:
            swprintf(TextList[TextNum], GlobalText[1237]);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            break;
        case AT_SKILL_BRAND_OF_SKILL:
            break;
        case AT_SKILL_PLASMA_STORM_FENRIR:
        case AT_SKILL_RECOVER:
        case AT_SKILL_ATT_UP_OURFORCES:
        case AT_SKILL_HP_UP_OURFORCES:
        case AT_SKILL_HP_UP_OURFORCES_STR:
        case AT_SKILL_DEF_UP_OURFORCES:
        case AT_SKILL_DEF_UP_OURFORCES_STR:
        case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
            break;
        default:
            swprintf(TextList[TextNum], GlobalText[879], iSkillMinDamage, iSkillMaxDamage + skillattackpowerRate);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            break;
        }
    }

    if (CharacterAttribute->Skill[Type] == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        int iSkillDamage;
        gSkillManager.GetSkillInformation_Damage(AT_SKILL_PLASMA_STORM_FENRIR, &iSkillDamage);

        if (HeroClass == CLASS_KNIGHT || HeroClass == CLASS_DARK)
        {
            iSkillMinDamage = (Strength / 3) + (Dexterity / 5) + (Vitality / 5) + (Energy / 7) + iSkillDamage;
        }
        else if (HeroClass == CLASS_WIZARD || HeroClass == CLASS_SUMMONER)
        {
            iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 7) + (Energy / 3) + iSkillDamage;
        }
        else if (HeroClass == CLASS_ELF)
        {
            iSkillMinDamage = (Strength / 5) + (Dexterity / 3) + (Vitality / 7) + (Energy / 5) + iSkillDamage;
        }
        else if (HeroClass == CLASS_DARK_LORD)
        {
            iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 7) + (Energy / 3) + (Charisma / 3) + iSkillDamage;
        }
        else if (HeroClass == CLASS_RAGEFIGHTER)
        {
            iSkillMinDamage = (Strength / 5) + (Dexterity / 5) + (Vitality / 3) + (Energy / 7) + iSkillDamage;
        }

        iSkillMaxDamage = iSkillMinDamage + 30;

        swprintf(TextList[TextNum], GlobalText[879], iSkillMinDamage, iSkillMaxDamage + skillattackpowerRate);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        bool Success = true;
        switch (CharacterAttribute->Skill[Type])
        {
        case AT_SKILL_HEALING_STR:
        {
            int value = (Energy / 5) + 5;
            auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_HEALING_STR].GetSkillValue();
            value += static_cast<int>((value * boostPercent) / 100.0);
            swprintf(TextList[TextNum], GlobalText[171], value);
        }
        break;
        case AT_SKILL_HEALING:
            swprintf(TextList[TextNum], GlobalText[171], Energy / 5 + 5);
            break;
        case AT_SKILL_DEFENSE_STR:
        case AT_SKILL_DEFENSE_MASTERY:
        {
            int value = Energy / 8 + 2;
            auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_DEFENSE_STR].GetSkillValue();
            auto masteryBoostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_DEFENSE_MASTERY].GetSkillValue();;
            value += static_cast<int>((value * (boostPercent + masteryBoostPercent)) / 100.0);
            swprintf(TextList[TextNum], GlobalText[172], value);
        }
        break;
        case AT_SKILL_DEFENSE:
            swprintf(TextList[TextNum], GlobalText[172], Energy / 8 + 2);
            break;
        case AT_SKILL_ATTACK_STR:
        case AT_SKILL_ATTACK_MASTERY:
        {
            int value = Energy / 7 + 3;
            auto boostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_ATTACK_STR].GetSkillValue();
            auto masteryBoostPercent = CharacterAttribute->MasterSkillInfo[AT_SKILL_ATTACK_MASTERY].GetSkillValue();
            value += static_cast<int>((value * (boostPercent + masteryBoostPercent)) / 100.0);
            swprintf(TextList[TextNum], GlobalText[173], value);
        }
        break;
        case AT_SKILL_ATTACK:
            swprintf(TextList[TextNum], GlobalText[173], Energy / 7 + 3);
            break;
        case AT_SKILL_RECOVER:
        {
            int Cal = Energy / 4;
            swprintf(TextList[TextNum], GlobalText[1782], (int)((float)Cal + (float)CharacterAttribute->Level));
        }
        break;
        default:Success = false;
        }
        if (Success)
        {
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
    }

    if (SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY
        && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_STR
        && SkillType != AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
    {
        if (iDistance)
        {
            swprintf(TextList[TextNum], GlobalText[174], iDistance);
            TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
        }
    }

    swprintf(TextList[TextNum], GlobalText[175], iMana);
    TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    if (iSkillMana > 0)
    {
        swprintf(TextList[TextNum], GlobalText[360], iSkillMana);
        TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
    }
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT)
    {
        if (CharacterAttribute->Skill[Type] == AT_SKILL_IMPALE)
        {
            swprintf(TextList[TextNum], GlobalText[96]);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }

        if (Hero->byExtensionSkill == 1 && CharacterAttribute->Level >= 220)
        {
            // Combo skills:
            if ((CharacterAttribute->Skill[Type] >= AT_SKILL_FALLING_SLASH && CharacterAttribute->Skill[Type] <= AT_SKILL_SLASH)
                || CharacterAttribute->Skill[Type] == AT_SKILL_FALLING_SLASH_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_LUNGE_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_CYCLONE_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_CYCLONE_STR_MG
                || CharacterAttribute->Skill[Type] == AT_SKILL_SLASH_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_TWISTING_SLASH
                || CharacterAttribute->Skill[Type] == AT_SKILL_TWISTING_SLASH_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_TWISTING_SLASH_STR_MG
                || CharacterAttribute->Skill[Type] == AT_SKILL_TWISTING_SLASH_MASTERY
                || CharacterAttribute->Skill[Type] == AT_SKILL_RAGEFUL_BLOW
                || CharacterAttribute->Skill[Type] == AT_SKILL_RAGEFUL_BLOW_STR
                || CharacterAttribute->Skill[Type] == AT_SKILL_RAGEFUL_BLOW_MASTERY
                || CharacterAttribute->Skill[Type] == AT_SKILL_DEATHSTAB
                || CharacterAttribute->Skill[Type] == AT_SKILL_DEATHSTAB_STR
                )
            {
                swprintf(TextList[TextNum], GlobalText[99]);
                TextListColor[TextNum] = TEXT_COLOR_DARKRED;
                TextBold[TextNum] = false;
                TextNum++;
            }
            else if (CharacterAttribute->Skill[Type] == AT_SKILL_STRIKE_OF_DESTRUCTION
                || CharacterAttribute->Skill[Type] == AT_SKILL_STRIKE_OF_DESTRUCTION_STR)
            {
                swprintf(TextList[TextNum], GlobalText[2115]);
                TextListColor[TextNum] = TEXT_COLOR_DARKRED;
                TextBold[TextNum] = false;
                TextNum++;
            }
        }
    }

    BYTE MasteryType = gSkillManager.GetSkillMasteryType(CharacterAttribute->Skill[Type]);
    if (MasteryType != 255)
    {
        swprintf(TextList[TextNum], GlobalText[1080 + MasteryType]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
    }

    int SkillUseType;
    int BrandType = SkillAttribute[SkillType].SkillBrand;
    SkillUseType = SkillAttribute[SkillType].SkillUseType;
    if (SkillUseType == SKILL_USE_TYPE_BRAND)
    {
        swprintf(TextList[TextNum], GlobalText[1480], SkillAttribute[BrandType].Name);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[1481], SkillAttribute[BrandType].Damage);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }
    SkillUseType = SkillAttribute[SkillType].SkillUseType;
    if (SkillUseType == SKILL_USE_TYPE_MASTER)
    {
        swprintf(TextList[TextNum], GlobalText[1482]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[1483], SkillAttribute[SkillType].KillCount);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }

    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
    {
        if (CharacterAttribute->Skill[Type] == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
        {
            swprintf(TextList[TextNum], GlobalText[1185]);
            TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        }
    }

    if (CharacterAttribute->Skill[Type] == AT_SKILL_PLASMA_STORM_FENRIR)
    {
        swprintf(TextList[TextNum], GlobalText[1926]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
        swprintf(TextList[TextNum], GlobalText[1927]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }

    if (CharacterAttribute->Skill[Type] == AT_SKILL_INFINITY_ARROW || CharacterAttribute->Skill[Type] == AT_SKILL_INFINITY_ARROW_STR)
    {
        swprintf(TextList[1], lpszName);
        TextListColor[1] = TEXT_COLOR_BLUE; TextBold[1] = true;
        swprintf(TextList[2], L"\n");
        swprintf(TextList[3], GlobalText[2040]);
        TextListColor[3] = TEXT_COLOR_DARKRED; TextBold[3] = false;
        swprintf(TextList[4], GlobalText[175], iMana);
        TextListColor[4] = TEXT_COLOR_WHITE; TextBold[4] = false;
        swprintf(TextList[5], GlobalText[360], iSkillMana);
        TextListColor[5] = TEXT_COLOR_WHITE; TextBold[5] = false;
        TextNum = 6; SkipNum = 2;
    }

    swprintf(TextList[TextNum], L"\n"); TextNum++; SkipNum++;

    if (CharacterAttribute->Skill[Type] == AT_SKILL_RUSH || CharacterAttribute->Skill[Type] == AT_SKILL_SPACE_SPLIT
        || CharacterAttribute->Skill[Type] == AT_SKILL_DEEPIMPACT || CharacterAttribute->Skill[Type] == AT_SKILL_JAVELIN
        || CharacterAttribute->Skill[Type] == AT_SKILL_SPIRAL_SLASH || CharacterAttribute->Skill[Type] == AT_SKILL_DEATH_CANNON
        || CharacterAttribute->Skill[Type] == AT_SKILL_OCCUPY
        )
    {
        swprintf(TextList[TextNum], GlobalText[2047]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }
    if (CharacterAttribute->Skill[Type] == AT_SKILL_STUN || CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_STUN
        || CharacterAttribute->Skill[Type] == AT_SKILL_INVISIBLE || CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_INVISIBLE
        || CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_BUFF)
    {
        swprintf(TextList[TextNum], GlobalText[2048]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }
    if (CharacterAttribute->Skill[Type] == AT_SKILL_IMPALE)
    {
        swprintf(TextList[TextNum], GlobalText[2049]);
        TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
    }

    if (SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY || SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR || SkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
    {
        swprintf(TextList[TextNum], GlobalText[2054]);
        TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
    }

    SIZE TextSize = { 0, 0 };
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);

    if (iRenderPoint == STRP_NONE)
    {
        int Height = ((TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2) / g_fScreenRate_y;
        sy -= Height;
    }

    RenderTipTextList(sx, sy, TextNum, 0, RT3_SORT_CENTER, iRenderPoint);
}

void SetTextColor(float r, float g, float b)
{
    glColor3f(r, g, b);
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
    MODEL_POTION + 64,
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
    MODEL_POTION + 112,
    MODEL_POTION + 113,
    MODEL_POTION + 120,
    MODEL_POTION + 121,
    MODEL_POTION + 122,
    MODEL_POTION + 123,
    MODEL_POTION + 124,
    MODEL_POTION + 134,
    MODEL_POTION + 135,
    MODEL_POTION + 136,
    MODEL_POTION + 137,
    MODEL_POTION + 138,
    MODEL_POTION + 139,
    MODEL_POTION + 114,
    MODEL_POTION + 115,
    MODEL_POTION + 116,
    MODEL_POTION + 117,
    MODEL_POTION + 118,
    MODEL_POTION + 119,
    MODEL_POTION + 126,
    MODEL_POTION + 127,
    MODEL_POTION + 128,
    MODEL_POTION + 129,
    MODEL_POTION + 130,
    MODEL_POTION + 131,
    MODEL_POTION + 132,
    MODEL_HELPER + 91,
    MODEL_HELPER + 97,
    MODEL_HELPER + 98,
    MODEL_HELPER + 99,
    MODEL_HELPER + 109,
    MODEL_HELPER + 110,
    MODEL_HELPER + 111,
    MODEL_HELPER + 112,
    MODEL_HELPER + 113,
    MODEL_HELPER + 114,
    MODEL_HELPER + 115,
    MODEL_HELPER + 121,
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
    MODEL_HELPER + 43,
    MODEL_HELPER + 44,
    MODEL_HELPER + 45,
    MODEL_HELPER + 46,
    MODEL_HELPER + 47,
    MODEL_HELPER + 48,
    MODEL_HELPER + 54,
    MODEL_HELPER + 55,
    MODEL_HELPER + 56,
    MODEL_HELPER + 57,
    MODEL_HELPER + 58,
    MODEL_HELPER + 59,
    MODEL_HELPER + 60,
    MODEL_HELPER + 61,
    MODEL_HELPER + 62,
    MODEL_HELPER + 63,
    MODEL_HELPER + 116,
    MODEL_HELPER + 125,
    MODEL_HELPER + 126,
    MODEL_HELPER + 127,
    MODEL_HELPER + 128,
    MODEL_HELPER + 129,
    MODEL_HELPER + 130,
    MODEL_HELPER + 131,
    MODEL_HELPER + 132,
    MODEL_HELPER + 133,
    MODEL_HELPER + 134,
    MODEL_POTION + 53,
    MODEL_POTION + 54,
    MODEL_POTION + 58,
    MODEL_POTION + 59,
    MODEL_POTION + 60,
    MODEL_POTION + 61,
    MODEL_POTION + 62,
    MODEL_POTION + 70,
    MODEL_POTION + 71,
    MODEL_POTION + 72,
    MODEL_POTION + 73,
    MODEL_POTION + 74,
    MODEL_POTION + 75,
    MODEL_POTION + 76,
    MODEL_POTION + 77,
    MODEL_POTION + 78,
    MODEL_POTION + 79,
    MODEL_POTION + 80,
    MODEL_POTION + 81,
    MODEL_POTION + 82,
    MODEL_POTION + 83,
    MODEL_POTION + 88,
    MODEL_POTION + 89,
    MODEL_POTION + 91,
    MODEL_POTION + 92,
    MODEL_POTION + 93,
    MODEL_POTION + 94,
    MODEL_POTION + 95,
    MODEL_POTION + 96,
    MODEL_POTION + 97,
    MODEL_POTION + 98,
    MODEL_POTION + 141,
    MODEL_POTION + 142,
    MODEL_POTION + 143,
    MODEL_POTION + 144,
    MODEL_POTION + 145,
    MODEL_POTION + 146,
    MODEL_POTION + 147,
    MODEL_POTION + 148,
    MODEL_POTION + 149,
    MODEL_POTION + 150,
    MODEL_WING + 130,
    MODEL_WING + 131,
    MODEL_WING + 132,
    MODEL_WING + 133,
    MODEL_WING + 134,
    MODEL_WING + 135,
};

void RenderItemName(int i, OBJECT* o, ITEM* ip, bool Sort)
{
    wchar_t Name[80]{};
    auto ItemLevel = ip->Level;
    auto ItemOption = ip->ExcellentFlags;
    GLfloat textColor[] = {1, 1, 1, 1};

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 255);

    // Use the item name by default
    if (o->Type == MODEL_ZEN) // Zen
    {
        swprintf(Name, L"%s %d", ItemAttribute[o->Type - MODEL_ITEM].Name, ItemLevel);
    }
    else if (ItemLevel == 0)
    {
        swprintf(Name, L"%s", ItemAttribute[o->Type - MODEL_ITEM].Name);
    }
    else
    {
        swprintf(Name, L"%s +%d", ItemAttribute[o->Type - MODEL_ITEM].Name, ItemLevel);
    }

    if (boldTextItems.count(o->Type) > 0)
    {
        g_pRenderText->SetFont(g_hFontBold);
    }

    if (whiteTextItems.count(o->Type) > 0)
    {
        SetTextColor(1.f, 1.f, 1.f);
    }
    else if (yellowTextItems.count(o->Type) > 0)
    {
        SetYellowTextColor();
    }
    else if (orangeTextItems.count(o->Type) > 0)
    {
        SetOrangeTextColor();
    }
    else if (o->Type == MODEL_ORB_OF_SUMMONING)
    {
        SetGrayTextColor();
        swprintf(Name, L"%s %s", SkillAttribute[30 + ItemLevel].Name, GlobalText[102]);
    }
    else if (COMGEM::NOGEM != COMGEM::Check_Jewel_Com(o->Type, true))
    {
        int iJewelItemIndex = COMGEM::GetJewelIndex(COMGEM::Check_Jewel_Com(o->Type, true), COMGEM::eGEM_NAME);
        g_pRenderText->SetFont(g_hFontBold);
        SetYellowTextColor();
        swprintf(Name, L"%s", GlobalText[iJewelItemIndex]);
    }
    else if (o->Type == MODEL_COMPILED_CELE)
    {
        swprintf(Name, L"%s", ItemAttribute[MODEL_JEWEL_OF_BLESS - MODEL_ITEM].Name);
    }
    else if (o->Type == MODEL_COMPILED_SOUL)
    {
        swprintf(Name, L"%s", ItemAttribute[MODEL_JEWEL_OF_SOUL - MODEL_ITEM].Name);
    }
    else if (o->Type == MODEL_BOX_OF_LUCK && ItemLevel == 7)
    {
        swprintf(Name, GlobalText[111]);
    }
    else if (o->Type == MODEL_POTION + 12)
    {
        switch (ItemLevel)
        {
        case 0:swprintf(Name, GlobalText[100]); break;
        case 1:swprintf(Name, GlobalText[101]); break;
        case 2:swprintf(Name, GlobalText[104]); break;
        }
    }
    else if (o->Type == MODEL_FRUITS)
    {
        switch (ItemLevel)
        {
        case 0:swprintf(Name, L"%s %s", GlobalText[168], ItemAttribute[o->Type - MODEL_ITEM].Name); break;
        case 1:swprintf(Name, L"%s %s", GlobalText[169], ItemAttribute[o->Type - MODEL_ITEM].Name); break;
        case 2:swprintf(Name, L"%s %s", GlobalText[167], ItemAttribute[o->Type - MODEL_ITEM].Name); break;
        case 3:swprintf(Name, L"%s %s", GlobalText[166], ItemAttribute[o->Type - MODEL_ITEM].Name); break;
        case 4:swprintf(Name, L"%s %s", GlobalText[1900], ItemAttribute[o->Type - MODEL_ITEM].Name); break;
        }
    }
    else if (o->Type == MODEL_SPIRIT)
    {
        switch (ItemLevel)
        {
        case 0:swprintf(Name, L"%s of %s", ItemAttribute[o->Type - MODEL_ITEM].Name, GlobalText[1187]); break;
        case 1:swprintf(Name, L"%s of %s", ItemAttribute[o->Type - MODEL_ITEM].Name, GlobalText[1214]); break;
        }
    }
    else if (o->Type == MODEL_EVENT + 16)
    {
        swprintf(Name, GlobalText[1235]);
    }
    else if (o->Type == MODEL_EVENT + 4)
    {
        swprintf(Name, GlobalText[105]);
    }
    else if (o->Type == MODEL_EVENT + 5)
    {
        switch (ItemLevel)
        {
        case 14:
            swprintf(Name, GlobalText[1650]);
            break;

        case 15:
            swprintf(Name, GlobalText[1651]);
            break;

        default:
            swprintf(Name, GlobalText[106]);
            break;
        }
    }
    else if (o->Type == MODEL_EVENT + 6)
    {
        if (ItemLevel == 13)
        {
            SetYellowTextColor();
            swprintf(Name, L"%s", GlobalText[117]);
        }
        else
        {
            swprintf(Name, GlobalText[107]);
        }
    }
    else if (o->Type == MODEL_EVENT + 7)
    {
        swprintf(Name, GlobalText[108]);
    }
    else if (o->Type == MODEL_EVENT + 8)
    {
        swprintf(Name, GlobalText[109]);
    }
    else if (o->Type == MODEL_EVENT + 9)
    {
        swprintf(Name, GlobalText[110]);
    }
    else if (o->Type == MODEL_EVENT + 10)
    {
        swprintf(Name, L"%s +%d", GlobalText[115], ItemLevel - 7);
    }
    else if (o->Type == MODEL_RED_RIBBON_BOX)
    {
        SetTextColor(1.f, 0.3f, 0.3f); // Color: Red

    }
    else if (o->Type == MODEL_GREEN_RIBBON_BOX)
    {
        SetTextColor(0.3f, 1.0f, 0.3f); // Color: Green
    }
    else if (o->Type == MODEL_BLUE_RIBBON_BOX)
    {
        SetTextColor(0.3f, 0.3f, 1.f); // Color: Blue
    }
    else if (o->Type == MODEL_PINK_CHOCOLATE_BOX)
    {
        int i = MODEL_PINK_CHOCOLATE_BOX;
        int k = ITEM_PINK_CHOCOLATE_BOX;
        if (ItemLevel == 0)
        {
            SetTextColor(1.f, 0.3f, 1.f);
        }
        else
            if (ItemLevel == 1)
            {
                SetTextColor(1.f, 0.3f, 1.f);
                swprintf(Name, GlobalText[2012]);
            }
    }
    else if (o->Type == MODEL_RED_CHOCOLATE_BOX)
    {
        if (ItemLevel == 0)
        {
            SetTextColor(1.0f, 0.3f, 0.3f);
        }
        else
            if (ItemLevel == 1)
            {
                SetTextColor(1.0f, 0.3f, 0.3f);
                swprintf(Name, GlobalText[2013]);
            }
    }
    else if (o->Type == MODEL_BLUE_CHOCOLATE_BOX)
    {
        if (ItemLevel == 0)
        {
            SetTextColor(0.3f, 0.3f, 1.f);
        }
        else
            if (ItemLevel == 1)
            {
                SetTextColor(0.3f, 0.3f, 1.f);
                swprintf(Name, GlobalText[2014]);
            }
    }
    else if (o->Type == MODEL_EVENT + 21)
    {
        SetTextColor(1.f, 0.3f, 1.f); // Color: Pink
        swprintf(Name, GlobalText[2012]);
    }
    else if (o->Type == MODEL_EVENT + 22)
    {
        SetTextColor(1.0f, 0.3f, 0.3f); // Color: Red
        swprintf(Name, GlobalText[2013]);
    }
    else if (o->Type == MODEL_EVENT + 23)
    {
        SetTextColor(0.3f, 0.3f, 1.f); // Color: Blue
        swprintf(Name, GlobalText[2014]);
    }
    else if (o->Type == MODEL_EVENT + 11)
    {
        swprintf(Name, GlobalText[810]);
    }
    else if (o->Type == MODEL_EVENT + 12)
    {
        swprintf(Name, GlobalText[906]);
    }
    else if (o->Type == MODEL_EVENT + 13)
    {
        swprintf(Name, GlobalText[907]);
    }
    else if (o->Type == MODEL_EVENT + 14)
    {
        switch (ItemLevel)
        {
        case 2:
            swprintf(Name, GlobalText[928]);
            break;
        case 3:
            swprintf(Name, GlobalText[929]);
            break;
        default:
            swprintf(Name, GlobalText[922]);
            break;
        }
    }
    else if (o->Type == MODEL_EVENT + 15)
    {
        swprintf(Name, GlobalText[925]);
    }
    else if (o->Type == MODEL_ORB_OF_SUMMONING)
    {
        swprintf(Name, L"%s %s", SkillAttribute[30 + ItemLevel].Name, GlobalText[102]);
    }
    else if (o->Type == MODEL_TRANSFORMATION_RING)
    {
        for (int i = 0; i < MAX_MONSTER; i++)
        {
            if (SommonTable[ItemLevel] == MonsterScript[i].Type)
            {
                swprintf(Name, L"%s %s", MonsterScript[i].Name, GlobalText[103]);
                break;
            }
        }
    }
    else if (o->Type == MODEL_POTION + 21 && ItemLevel == 3)
    {
        SetYellowTextColor();
        swprintf(Name, GlobalText[1290]);
    }
    else if (o->Type == MODEL_SIEGE_POTION)
    {
        switch (ItemLevel)
        {
        case 0: swprintf(Name, GlobalText[1413]); break;
        case 1: swprintf(Name, GlobalText[1414]); break;
        }
    }
    else if (o->Type == MODEL_HELPER + 7)
    {
        switch (ItemLevel)
        {
        case 0: swprintf(Name, GlobalText[1460]); break;
        case 1: swprintf(Name, GlobalText[1461]); break;
        }
    }
    else if (o->Type == MODEL_LIFE_STONE_ITEM)
    {
        switch (ItemLevel)
        {
        case 0: swprintf(Name, GlobalText[1416]); break;
        case 1: swprintf(Name, GlobalText[1462]); break;
        }
    }
    else if (o->Type == MODEL_EVENT + 18)
    {
        swprintf(Name, GlobalText[1462]);
    }
    else if ((o->Type >= MODEL_SEED_FIRE && o->Type <= MODEL_SEED_EARTH)
            || (o->Type >= MODEL_SPHERE_MONO && o->Type <= MODEL_SPHERE_5)
            || (o->Type >= MODEL_SEED_SPHERE_FIRE_1 && o->Type <= MODEL_SEED_SPHERE_EARTH_5))
    {
        SetTextColor(0.7f, 0.4f, 1.0f);	// TEXT_COLOR_VIOLET
        wcscpy(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
    }
    else if (o->Type == MODEL_HELPER + 66)
    {
        SetTextColor(0.6f, 0.4f, 1.0f);
    }
    else if (o->Type >= MODEL_TYPE_CHARM_MIXWING + EWS_BEGIN
        && o->Type <= MODEL_TYPE_CHARM_MIXWING + EWS_END)
    {
        SetOrangeTextColor();
    }
    else
    {
        if (o->Type == MODEL_DIVINE_STAFF_OF_ARCHANGEL || o->Type == MODEL_DIVINE_SWORD_OF_ARCHANGEL || o->Type == MODEL_DIVINE_CB_OF_ARCHANGEL || o->Type == MODEL_DIVINE_SCEPTER_OF_ARCHANGEL)
        {
            SetTextColor(1.f, 0.1f, 1.f);
        }
        else if (g_SocketItemMgr.IsSocketItem(o))
        {
            SetTextColor(0.7f, 0.4f, 1.0f);	// TEXT_COLOR_VIOLET
        }
        else if ((ItemOption & 63) > 0 && (o->Type<MODEL_WINGS_OF_SPIRITS || o->Type>MODEL_WINGS_OF_DARKNESS) && o->Type != MODEL_CAPE_OF_LORD
            && (o->Type<MODEL_WING_OF_STORM || o->Type>MODEL_CAPE_OF_EMPEROR)
            && (o->Type<MODEL_WINGS_OF_DESPAIR || o->Type>MODEL_WING_OF_DIMENSION)
            && !(o->Type >= MODEL_CAPE_OF_FIGHTER && o->Type <= MODEL_CAPE_OF_OVERRULE))
        {
            SetTextColor(0.1f, 1.f, 0.5f);
        }
        else if (ItemLevel >= 7)
        {
            SetYellowTextColor();
        }
        else if (ip->HasSkill || ip->HasLuck || ip->OptionLevel > 0)
        {
            SetTextColor(0.4f, 0.7f, 1.f);
        }
        else if (ItemLevel == 0)
        {
            SetGrayTextColor();
        }
        else if (ItemLevel < 3)
        {
            SetTextColor(0.9f, 0.9f, 0.9f);
        }
        else if (ItemLevel < 5)
        {
            SetTextColor(1.f, 0.5f, 0.2f);
        }
        else if (ItemLevel < 7)
        {
            SetTextColor(0.4f, 0.7f, 1.f);
        }

        wchar_t SetName[64]{};
        if (g_csItemOption.GetSetItemName(SetName, o->Type - MODEL_ITEM, ip->AncientDiscriminator))
        {
            SetTextColor(0.f, 1.f, 0.f);
            g_pRenderText->SetFont(g_hFontBold);
            g_pRenderText->SetTextColor(0, 255, 0, 255);
            g_pRenderText->SetBgColor(60, 60, 200, 255);

            wcscat(SetName, Name);
            wcscpy(Name, SetName);
        }

        if (ip->HasSkill)
        {
            if (o->Type != MODEL_HORN_OF_DINORANT)
            {
                wcscat(Name, GlobalText[176]);
            }
            else
            {
                wcscat(Name, L" +");
                wcscat(Name, GlobalText[179]);
            }
        }
        if (ip->OptionLevel > 0)
            wcscat(Name, GlobalText[177]);
        if (ip->HasLuck)
            wcscat(Name, GlobalText[178]);
    }

    if (!Sort)
    {
        g_pRenderText->RenderText(MouseX, MouseY - 15, Name, 0, 0, RT3_WRITE_CENTER);
    }
    else
    {
        g_pRenderText->RenderText(o->ScreenX, o->ScreenY - 15, Name, 0, 0, RT3_WRITE_CENTER);
    }

    g_pRenderText->SetTextColor(255, 230, 200, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 255);
}

int GetScreenWidth()
{
    int iWidth = 0;

    // TODO: Refactor this. Wouldn't it be easier to just count how many windows are open? ;)

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
        && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY_EXT)
        && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY))
    {
        iWidth = 640 - (190 * 3);
    }
    else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
        && (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE_EXT)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY_EXT)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
            ))
    {
        iWidth = 640 - (190 * 2);
    }
    else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
        && (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST)
            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
        )
    {
        iWidth = 640 - (190 * 2);
    }
    else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
        && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PET)
        )
    {
        iWidth = 640 - (190 * 2);
    }
    else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_REFINERY))
    {
        iWidth = 640 - (190 * 2);
    }
    else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTY)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCGUILDMASTER)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GUILDINFO)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GUARDSMAN)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_SENATUS)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GATEKEEPER)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_SERVERDIVISION)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCQUEST)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GATESWITCH)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CATAPULT)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DUELWATCH)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DOPPELGANGER_NPC)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GENSRANKING)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MUHELPER)
        )
    {
        iWidth = 640 - 190;
    }
    else
    {
        iWidth = 640;
    }

    return iWidth;
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
                    int     Num = max(item1.SpecialNum, item2.SpecialNum);
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

bool IsPartChargeItem(ITEM* pItem)
{
    if ((pItem->Type >= ITEM_HELPER + 46 && pItem->Type <= ITEM_HELPER + 48)
        || (pItem->Type == ITEM_POTION + 54)
        || (pItem->Type >= ITEM_POTION + 58 && pItem->Type <= ITEM_POTION + 62)
        || (pItem->Type >= ITEM_POTION + 145 && pItem->Type <= ITEM_POTION + 150)
        || (pItem->Type >= ITEM_HELPER + 125 && pItem->Type <= ITEM_HELPER + 127)
        || pItem->Type == ITEM_POTION + 53
        || (pItem->Type >= ITEM_HELPER + 43 && pItem->Type <= ITEM_HELPER + 45)
        || (pItem->Type >= ITEM_POTION + 70 && pItem->Type <= ITEM_POTION + 71)
        || (pItem->Type >= ITEM_POTION + 72 && pItem->Type <= ITEM_POTION + 77)
        || (pItem->Type == ITEM_HELPER + 59)
        || (pItem->Type >= ITEM_HELPER + 54 && pItem->Type <= ITEM_HELPER + 58)
        || (pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82)
        || (pItem->Type == ITEM_HELPER + 60)
        || (pItem->Type == ITEM_HELPER + 61)
        || (pItem->Type == ITEM_POTION + 91)
        || (pItem->Type >= ITEM_POTION + 92 && pItem->Type <= ITEM_POTION + 93)
        || (pItem->Type == ITEM_POTION + 95)
        || (pItem->Type == ITEM_POTION + 94)
        || (pItem->Type >= ITEM_HELPER + 62 && pItem->Type <= ITEM_HELPER + 63)
        || (pItem->Type >= ITEM_POTION + 97 && pItem->Type <= ITEM_POTION + 98)
        || (pItem->Type == ITEM_POTION + 96)
        || (pItem->Type == ITEM_DEMON || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN)
        || (pItem->Type == ITEM_HELPER + 69)
        || (pItem->Type == ITEM_HELPER + 70)
        || pItem->Type == ITEM_HELPER + 81
        || pItem->Type == ITEM_HELPER + 82
        || pItem->Type == ITEM_HELPER + 93
        || pItem->Type == ITEM_HELPER + 94
        || pItem->Type == ITEM_HELPER + 107
        || pItem->Type == ITEM_HELPER + 104
        || pItem->Type == ITEM_HELPER + 105
        || pItem->Type == ITEM_HELPER + 103
        || pItem->Type == ITEM_POTION + 133
        || pItem->Type == ITEM_HELPER + 109
        || pItem->Type == ITEM_HELPER + 110
        || pItem->Type == ITEM_HELPER + 111
        || pItem->Type == ITEM_HELPER + 112
        || pItem->Type == ITEM_HELPER + 113
        || pItem->Type == ITEM_HELPER + 114
        || pItem->Type == ITEM_HELPER + 115
        || pItem->Type == ITEM_POTION + 112
        || pItem->Type == ITEM_POTION + 113
        || pItem->Type == ITEM_POTION + 120
        || pItem->Type == ITEM_POTION + 123
        || pItem->Type == ITEM_POTION + 124
        || pItem->Type == ITEM_POTION + 134
        || pItem->Type == ITEM_POTION + 135
        || pItem->Type == ITEM_POTION + 136
        || pItem->Type == ITEM_POTION + 137
        || pItem->Type == ITEM_POTION + 138
        || pItem->Type == ITEM_POTION + 139
        || pItem->Type == ITEM_WING + 130
        || pItem->Type == ITEM_WING + 131
        || pItem->Type == ITEM_WING + 132
        || pItem->Type == ITEM_WING + 133
        || pItem->Type == ITEM_WING + 134
        || pItem->Type == ITEM_WING + 135
        || pItem->Type == ITEM_HELPER + 116
        || pItem->Type == ITEM_PET_UNICORN
        || pItem->Type == ITEM_HELPER + 124
        || pItem->Type == ITEM_POTION + 114
        || pItem->Type == ITEM_POTION + 115
        || pItem->Type == ITEM_POTION + 116
        || pItem->Type == ITEM_POTION + 117
        || pItem->Type == ITEM_POTION + 118
        || pItem->Type == ITEM_POTION + 119
        || pItem->Type == ITEM_POTION + 126
        || pItem->Type == ITEM_POTION + 127
        || pItem->Type == ITEM_POTION + 128
        || pItem->Type == ITEM_POTION + 129
        || pItem->Type == ITEM_POTION + 130
        || pItem->Type == ITEM_POTION + 131
        || pItem->Type == ITEM_POTION + 132
        || pItem->Type == ITEM_HELPER + 121
        || pItem->Type == ITEM_POTION + 140
        )
    {
        return true;
    }

    return false;
}

bool IsHighValueItem(ITEM* pItem)
{
    int iLevel = pItem->Level;

    if (
        pItem->Type == ITEM_HORN_OF_DINORANT ||
        pItem->Type == ITEM_JEWEL_OF_BLESS ||
        pItem->Type == ITEM_JEWEL_OF_SOUL ||
        pItem->Type == ITEM_JEWEL_OF_LIFE ||
        pItem->Type == ITEM_JEWEL_OF_CREATION ||
        pItem->Type == ITEM_JEWEL_OF_CHAOS ||
        pItem->Type == ITEM_JEWEL_OF_GUARDIAN ||
        pItem->Type == ITEM_PACKED_JEWEL_OF_BLESS ||
        pItem->Type == ITEM_PACKED_JEWEL_OF_SOUL ||
        (pItem->Type >= ITEM_WING && pItem->Type <= ITEM_WINGS_OF_DARKNESS) ||
        pItem->Type == ITEM_DARK_HORSE_ITEM ||
        pItem->Type == ITEM_DARK_RAVEN_ITEM ||
        pItem->Type == ITEM_CAPE_OF_LORD ||
        (pItem->Type >= ITEM_WING_OF_STORM && pItem->Type <= ITEM_WING_OF_DIMENSION) ||
        pItem->AncientDiscriminator > 0 ||
        pItem->Type == ITEM_DIVINE_SWORD_OF_ARCHANGEL ||
        pItem->Type == ITEM_DIVINE_STAFF_OF_ARCHANGEL ||
        pItem->Type == ITEM_DIVINE_CB_OF_ARCHANGEL ||
        pItem->Type == ITEM_DIVINE_SCEPTER_OF_ARCHANGEL ||
        pItem->Type == ITEM_LOCHS_FEATHER ||
        pItem->Type == ITEM_FRUITS ||
        pItem->Type == ITEM_WEAPON_OF_ARCHANGEL ||
        pItem->Type == ITEM_SPIRIT ||
        (pItem->Type >= ITEM_GEMSTONE && pItem->Type <= ITEM_HIGHER_REFINE_STONE) ||
        (iLevel > 6 && pItem->Type < ITEM_WING) ||
        pItem->ExcellentFlags > 0 ||
        (pItem->Type >= ITEM_CLAW_OF_BEAST && pItem->Type <= ITEM_HORN_OF_FENRIR)
        || pItem->Type == ITEM_FLAME_OF_CONDOR
        || pItem->Type == ITEM_FEATHER_OF_CONDOR
        || pItem->Type == ITEM_POTION + 121
        || pItem->Type == ITEM_POTION + 122
        || pItem->Type == ITEM_WING + 130
        || pItem->Type == ITEM_WING + 131
        || pItem->Type == ITEM_WING + 132
        || pItem->Type == ITEM_WING + 133
        || pItem->Type == ITEM_WING + 134
        || pItem->Type == ITEM_WING + 135
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_SKELETON
        || pItem->Type == ITEM_DEMON
        || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
        || pItem->Type == ITEM_HELPER + 109
        || pItem->Type == ITEM_HELPER + 110
        || pItem->Type == ITEM_HELPER + 111
        || pItem->Type == ITEM_HELPER + 112
        || pItem->Type == ITEM_HELPER + 113
        || pItem->Type == ITEM_HELPER + 114
        || pItem->Type == ITEM_HELPER + 115
        || pItem->Type == ITEM_POTION + 112
        || pItem->Type == ITEM_POTION + 113
        || (pItem->Type == ITEM_WIZARDS_RING && iLevel == 0)
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 255)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (pItem->Type >= ITEM_CAPE_OF_FIGHTER && pItem->Type <= ITEM_CAPE_OF_OVERRULE)
#ifdef KJH_FIX_SELL_LUCKYITEM
        || (Check_ItemAction(pItem, eITEM_SELL) && pItem->Durability > 0)
#endif // KJH_FIX_SELL_LUCKYITEM
        || (COMGEM::isCompiledGem(pItem))
        )
    {
        if (true == pItem->bPeriodItem && false == pItem->bExpiredPeriod)
        {
            return false;
        }
        else if (pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_DEMON
            || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
            )
        {
            if (true == pItem->bPeriodItem && true == pItem->bExpiredPeriod)
            {
                return true;
            }
            else
                return false;
        }
        return true;
    }

    return false;
}

bool IsPersonalShopBan(ITEM* pItem)
{
    if (pItem == NULL)
    {
        return false;
    }

#ifdef KJH_FIX_PERSONALSHOP_BAN_CASHITEM
    if (pItem->bPeriodItem)
    {
        return true;
    }
#endif // KJH_FIX_PERSONALSHOP_BAN_CASHITEM

    if ((!pItem->bPeriodItem) &&
        pItem->Type == ITEM_DEMON
        || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_SKELETON
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 255)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0)
        )
    {
        return false;
    }

    if (pItem->Type == ITEM_MOONSTONE_PENDANT
        || pItem->Type == ITEM_ELITE_TRANSFER_SKELETON_RING
        || (pItem->Type == ITEM_POTION + 21 && pItem->Level != 3)
        || (pItem->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && pItem->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
        || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
        || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
        || (pItem->Type >= ITEM_HELPER + 43 && pItem->Type <= ITEM_HELPER + 45)
        || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
        || pItem->Type == ITEM_FLAME_OF_DEATH_BEAM_KNIGHT
        || pItem->Type == ITEM_HORN_OF_HELL_MAINE
        || pItem->Type == ITEM_FEATHER_OF_DARK_PHOENIX
        || pItem->Type == ITEM_EYE_OF_ABYSSAL
        || IsPartChargeItem(pItem)
        || pItem->Type == ITEM_HELPER + 97
        || pItem->Type == ITEM_HELPER + 98
        || pItem->Type == ITEM_POTION + 91
        || pItem->Type == ITEM_HELPER + 99
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_SKELETON
        || (pItem->Type == ITEM_POTION + 96)
        || pItem->Type == ITEM_HELPER + 109
        || pItem->Type == ITEM_HELPER + 110
        || pItem->Type == ITEM_HELPER + 111
        || pItem->Type == ITEM_HELPER + 112
        || pItem->Type == ITEM_HELPER + 113
        || pItem->Type == ITEM_HELPER + 114
        || pItem->Type == ITEM_HELPER + 115
#ifdef LDK_MOD_INGAMESHOP_WIZARD_RING_PERSONALSHOPBAN
        || (pItem->Type == ITEM_WIZARDS_RING && (pItem->Level == 0)
#endif //LDK_MOD_INGAMESHOP_WIZARD_RING_PERSONALSHOPBAN
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
        || (pItem->Type >= ITEM_POTION + 151 && pItem->Type <= ITEM_POTION + 156)
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        )
    {
        return true;
    }
    if (Check_ItemAction(pItem, eITEM_PERSONALSHOP))	return true;

    return false;
}

bool IsTradeBan(ITEM* pItem)
{
    if (pItem->Type == ITEM_MOONSTONE_PENDANT
        || pItem->Type == ITEM_ELITE_TRANSFER_SKELETON_RING
        || (pItem->Type == ITEM_POTION + 21 && pItem->Level != 3)
        || (pItem->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && pItem->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
        || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
        || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
        || (pItem->Type >= ITEM_HELPER + 43 && pItem->Type <= ITEM_HELPER + 45)
        || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
        || pItem->Type == ITEM_POTION + 64
        || pItem->Type == ITEM_FLAME_OF_DEATH_BEAM_KNIGHT
        || pItem->Type == ITEM_HORN_OF_HELL_MAINE
        || pItem->Type == ITEM_FEATHER_OF_DARK_PHOENIX
        || pItem->Type == ITEM_EYE_OF_ABYSSAL
        || IsPartChargeItem(pItem)
        || pItem->Type == ITEM_HELPER + 97
        || pItem->Type == ITEM_HELPER + 98
        || pItem->Type == ITEM_POTION + 91
        || pItem->Type == ITEM_HELPER + 99
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_SKELETON
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
        || (pItem->Type >= ITEM_POTION + 151 && pItem->Type <= ITEM_POTION + 156)
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        )
    {
        return true;
    }

    if (pItem->Type == ITEM_GM_GIFT)
    {
        if (g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) ||
            (Hero->CtlCode == CTLCODE_20OPERATOR) || (Hero->CtlCode == CTLCODE_08OPERATOR))
            return false;
        else
            return true;
    }
    if (Check_ItemAction(pItem, eITEM_TRADE))	return true;

    return false;
}

bool IsStoreBan(ITEM* pItem)
{
    if ((pItem->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && pItem->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
        || (pItem->Type == ITEM_POTION + 21 && pItem->Level != 3)
        || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
        || (pItem->Type == ITEM_BOX_OF_LUCK && pItem->Level == 13)
        || (pItem->Type >= ITEM_HELPER + 43 && pItem->Type <= ITEM_HELPER + 45)
        || pItem->Type == ITEM_HELPER + 93
        || pItem->Type == ITEM_HELPER + 94
        || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level != 0)
        || pItem->Type == ITEM_FLAME_OF_DEATH_BEAM_KNIGHT
        || pItem->Type == ITEM_HORN_OF_HELL_MAINE
        || pItem->Type == ITEM_FEATHER_OF_DARK_PHOENIX
        || pItem->Type == ITEM_EYE_OF_ABYSSAL
        || (pItem->Type == ITEM_HELPER + 70 && pItem->Durability == 1)
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
        || (pItem->Type >= ITEM_POTION + 151 && pItem->Type <= ITEM_POTION + 156)
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
        || (pItem->bPeriodItem == true)
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 254)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        )
    {
        return true;
    }

    if (Check_ItemAction(pItem, eITEM_STORE))	return true;

    return false;
}

sItemAct Set_ItemActOption(int _nIndex, int _nOption)
{
    sItemAct	sItem;
    // eITEM_PERSONALSHOP = 개인상점, eITEM_STORE = 창고, eITEM_TRADE = 거래, eITEM_DROP = 버리기, eITEM_SELL = 판매, eITEM_REPAIR = 수리
    int	nItemOption[][eITEM_END] = { 0, 1, 1, 0, 0, 0,
                                        0, 0, 0, 0, 1, 0,
        -1 };

    sItem.s_nItemIndex = _nIndex;

    for (int i = 0; i < eITEM_END; i++)
    {
        sItem.s_bType[i] = nItemOption[_nOption][i];
    }
    return sItem;
}

bool Check_ItemAction(ITEM* _pItem, ITEMSETOPTION _eAction, bool _bType)
{
    std::vector<sItemAct>			sItem;
    std::vector<sItemAct>::iterator li;
    int		i = 0;

    for (i = 0; i < 12; i++)	sItem.push_back(Set_ItemActOption(ITEM_HELPER + 135 + i, 0));
    for (i = 0; i < 2; i++)	sItem.push_back(Set_ItemActOption(ITEM_POTION + 160 + i, 0));
    for (i = 0; i < 12; i++)
    {
        sItem.push_back(Set_ItemActOption(ITEM_ARMOR + 62 + i, 1));
        sItem.push_back(Set_ItemActOption(ITEM_HELM + 62 + i, 1));
        sItem.push_back(Set_ItemActOption(ITEM_BOOTS + 62 + i, 1));
        sItem.push_back(Set_ItemActOption(ITEM_GLOVES + 62 + i, 1));
        sItem.push_back(Set_ItemActOption(ITEM_PANTS + 62 + i, 1));
    }

    for (li = sItem.begin(); li != sItem.end(); li++)
    {
        if (li->s_nItemIndex == _pItem->Type)
        {
            _bType = (li->s_bType[_eAction]) ^ (!_bType);
            return _bType;
        }
    }

    // 등록되지 않은 아이템은 무시.
    return false;
}

bool Check_LuckyItem(int _nIndex, int _nType)
{
    int		nItemTabIndex = (_nIndex + _nType) % 512;

    if (_nIndex < ITEM_HELM || _nIndex > ITEM_WING)	return false;
    if (nItemTabIndex >= 62 && nItemTabIndex <= 72)		return true;

    return false;
}

bool IsLuckySetItem(int iType)
{
    int iItemIndex = iType % MAX_ITEM_INDEX;

#ifdef LEM_FIX_SELL_LUCKYITEM_BOOTS_POPUP
    if ((iType >= ITEM_HELM && iType <= ITEM_WING)
#else // LEM_FIX_SELL_LUCKYITEM_BOOTS_POPUP
    if ((iType >= ITEM_HELM && iType <= ITEM_BOOTS)
#endif // LEM_FIX_SELL_LUCKYITEM_BOOTS_POPUP
        && (iItemIndex >= 62 && iItemIndex <= 72))
    {
        return true;
    }

    return false;
}

bool IsDropBan(ITEM* pItem)
{
    if ((!pItem->bPeriodItem) &&
        (pItem->Type == ITEM_POTION + 96
            || pItem->Type == ITEM_POTION + 54
            || pItem->Type == ITEM_DEMON
            || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
            || pItem->Type == ITEM_PET_PANDA
            || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
            || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
            || pItem->Type == ITEM_PET_SKELETON
            || (pItem->Type == ITEM_WIZARDS_RING && pItem->Level == 0)
            ))
    {
        return false;
    }

    if (true == false || pItem->Type == ITEM_POTION + 123 || pItem->Type == ITEM_POTION + 124)
    {
        return false;
    }

    if ((pItem->Type >= ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR && pItem->Type <= ITEM_SOUL_SHARD_OF_WIZARD)
        || (pItem->Type >= ITEM_FLAME_OF_DEATH_BEAM_KNIGHT && pItem->Type <= ITEM_EYE_OF_ABYSSAL)
        || IsPartChargeItem(pItem)
        || ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN)
            && (pItem->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END))
        || pItem->Type == ITEM_HELPER + 97
        || pItem->Type == ITEM_HELPER + 98
        || pItem->Type == ITEM_POTION + 91
        || pItem->Type == ITEM_HELPER + 99
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_SKELETON
        || pItem->Type == ITEM_POTION + 121
        || pItem->Type == ITEM_POTION + 122
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        )
    {
        return true;
    }

    if (Check_ItemAction(pItem, eITEM_DROP))	return true;

    return false;
}

bool IsSellingBan(ITEM* pItem)
{
    int Level = pItem->Level;

    if (true == false
        || pItem->Type == ITEM_POTION + 112
        || pItem->Type == ITEM_POTION + 113
        || pItem->Type == ITEM_POTION + 121
        || pItem->Type == ITEM_POTION + 122
        || pItem->Type == ITEM_POTION + 123
        || pItem->Type == ITEM_POTION + 124
        || pItem->Type == ITEM_WING + 130
        || pItem->Type == ITEM_WING + 131
        || pItem->Type == ITEM_WING + 132
        || pItem->Type == ITEM_WING + 133
        || pItem->Type == ITEM_WING + 134
        || pItem->Type == ITEM_WING + 135
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_SKELETON
        || pItem->Type == ITEM_DEMON
        || pItem->Type == ITEM_SPIRIT_OF_GUARDIAN
        || pItem->Type == ITEM_HELPER + 109
        || pItem->Type == ITEM_HELPER + 110
        || pItem->Type == ITEM_HELPER + 111
        || pItem->Type == ITEM_HELPER + 112
        || pItem->Type == ITEM_HELPER + 113
        || pItem->Type == ITEM_HELPER + 114
        || pItem->Type == ITEM_HELPER + 115
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability != 254)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || ((pItem->Type == ITEM_WIZARDS_RING) && (Level == 0))
        || (pItem->Type == ITEM_PET_UNICORN)
        || (pItem->Type == ITEM_HELPER + 107)
        )
    {
        if (true == pItem->bPeriodItem && true == pItem->bExpiredPeriod)
        {
            return false;
        }
    }

    if (pItem->Type == ITEM_BOX_OF_LUCK
        || (pItem->Type == ITEM_POTION + 21 && Level == 1)
        || ((pItem->bPeriodItem == true) && (pItem->bExpiredPeriod == false) && (pItem->Type == ITEM_WIZARDS_RING) && (Level == 0))
        || (pItem->Type == ITEM_WIZARDS_RING && (Level == 1 || Level == 2))
        || pItem->Type == ITEM_WEAPON_OF_ARCHANGEL
        || (pItem->Type == ITEM_POTION + 20 && Level >= 1 && Level <= 5)
        || IsPartChargeItem(pItem)
        || ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN)
            && (pItem->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END))
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_SKELETON
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (g_pMyInventory->IsInvenItem(pItem->Type))
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || (pItem->Type == ITEM_PET_UNICORN)
        || (pItem->Type == ITEM_HELPER + 107)
#ifdef KJH_FIX_SELL_LUCKYITEM
        || ((IsLuckySetItem(pItem->Type) == true) && (pItem->Durability > 0))
#endif // KJH_FIX_SELL_LUCKYITEM
        )
    {
        return true;
    }

    if (Check_ItemAction(pItem, eITEM_SELL))	return true;

    return false;
}

bool IsRepairBan(ITEM* pItem)
{
    if (g_ChangeRingMgr->CheckRepair(pItem->Type) == true)
    {
        return true;
    }
    if (IsPartChargeItem(pItem) == true || ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING + EWS_BEGIN) && (pItem->Type <= ITEM_TYPE_CHARM_MIXWING + EWS_END)))
    {
        return true;
    }

    if ((pItem->Type >= ITEM_POTION + 55 && pItem->Type <= ITEM_POTION + 57)
        || pItem->Type == ITEM_HELPER + 43
        || pItem->Type == ITEM_HELPER + 44
        || pItem->Type == ITEM_HELPER + 45
        || (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HORN_OF_DINORANT)
        || pItem->Type == ITEM_BOLT
        || pItem->Type == ITEM_ARROWS
        || pItem->Type >= ITEM_POTION
        || (pItem->Type >= ITEM_ORB_OF_TWISTING_SLASH && pItem->Type <= ITEM_ORB_OF_DEATH_STAB)
        || (pItem->Type >= ITEM_LOCHS_FEATHER && pItem->Type <= ITEM_WEAPON_OF_ARCHANGEL)
        || pItem->Type == ITEM_POTION + 21
        || pItem->Type == ITEM_DARK_HORSE_ITEM
        || pItem->Type == ITEM_DARK_RAVEN_ITEM
        || pItem->Type == ITEM_MOONSTONE_PENDANT
        || pItem->Type == ITEM_PET_RUDOLF
        || pItem->Type == ITEM_PET_PANDA
        || pItem->Type == ITEM_PANDA_TRANSFORMATION_RING
        || pItem->Type == ITEM_SKELETON_TRANSFORMATION_RING
        || pItem->Type == ITEM_PET_SKELETON
        || pItem->Type == ITEM_PET_UNICORN
        || pItem->Type == ITEM_CHERRY_BLOSSOM_PLAYBOX
        || pItem->Type == ITEM_CHERRY_BLOSSOM_WINE
        || pItem->Type == ITEM_CHERRY_BLOSSOM_RICE_CAKE
        || pItem->Type == ITEM_CHERRY_BLOSSOM_FLOWER_PETAL
        || pItem->Type == ITEM_POTION + 88
        || pItem->Type == ITEM_POTION + 89
        || pItem->Type == ITEM_GOLDEN_CHERRY_BLOSSOM_BRANCH
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        || pItem->Type == ITEM_HELPER + 7
        || pItem->Type == ITEM_TRANSFORMATION_RING
        || pItem->Type == ITEM_LIFE_STONE_ITEM
        || pItem->Type == ITEM_WIZARDS_RING
        || pItem->Type == ITEM_ARMOR_OF_GUARDSMAN
        || pItem->Type == ITEM_SPLINTER_OF_ARMOR
        || pItem->Type == ITEM_BLESS_OF_GUARDIAN
        || pItem->Type == ITEM_CLAW_OF_BEAST
        || pItem->Type == ITEM_FRAGMENT_OF_HORN
        || pItem->Type == ITEM_BROKEN_HORN
        || pItem->Type == ITEM_HORN_OF_FENRIR
        || pItem->Type == ITEM_OLD_SCROLL
        || pItem->Type == ITEM_ILLUSION_SORCERER_COVENANT
        || pItem->Type == ITEM_SCROLL_OF_BLOOD
        || pItem->Type == ITEM_HELPER + 66
        || pItem->Type == ITEM_HELPER + 71
        || pItem->Type == ITEM_HELPER + 72
        || pItem->Type == ITEM_HELPER + 73
        || pItem->Type == ITEM_HELPER + 74
        || pItem->Type == ITEM_HELPER + 75
        )
    {
        return true;
    }

    if (Check_ItemAction(pItem, eITEM_REPAIR))	return true;

    return false;
}

bool IsWingItem(ITEM* pItem)
{
    switch (pItem->Type)
    {
    case ITEM_WING:
    case ITEM_WINGS_OF_HEAVEN:
    case ITEM_WINGS_OF_SATAN:
    case ITEM_WINGS_OF_SPIRITS:
    case ITEM_WINGS_OF_SOUL:
    case ITEM_WINGS_OF_DRAGON:
    case ITEM_WINGS_OF_DARKNESS:
    case ITEM_CAPE_OF_LORD:
    case ITEM_WING_OF_STORM:
    case ITEM_WING_OF_ETERNAL:
    case ITEM_WING_OF_ILLUSION:
    case ITEM_WING_OF_RUIN:
    case ITEM_CAPE_OF_EMPEROR:
    case ITEM_WING_OF_CURSE:
    case ITEM_WINGS_OF_DESPAIR:
    case ITEM_WING_OF_DIMENSION:
    case ITEM_WING + 130:
    case ITEM_WING + 131:
    case ITEM_WING + 132:
    case ITEM_WING + 133:
    case ITEM_WING + 134:
    case ITEM_CAPE_OF_FIGHTER:
    case ITEM_CAPE_OF_OVERRULE:
    case ITEM_WING + 135:
        return true;
    }

    return false;
}

bool IsJewelItem(ITEM* pItem)
{
    if (pItem->Type == ITEM_JEWEL_OF_BLESS
        || pItem->Type == ITEM_JEWEL_OF_SOUL
        || pItem->Type == ITEM_JEWEL_OF_LIFE
        || pItem->Type == ITEM_JEWEL_OF_CHAOS
        || pItem->Type == ITEM_JEWEL_OF_CREATION
        || pItem->Type == ITEM_JEWEL_OF_GUARDIAN)
    {
        return true;
    }

    return false;
}

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

std::wstring GetItemDisplayName(ITEM* pItem)
{
    // NOTE: 
    // There may be already another function for this (the one being used for displaying dropped items).
    // This version currently only applies to ascii names of items

    ITEM_ATTRIBUTE* pAttr = &ItemAttribute[pItem->Type];

    auto nNameLen = wcsnlen(pAttr->Name, MAX_ITEM_NAME);
    std::wstring strDisplayName(pAttr->Name, nNameLen);
    std::wstring strOptions;

    if (pItem->Level)
    {
        strOptions += L"+" + std::to_wstring(pItem->Level);
    }
    if (pItem->HasSkill)
    {
        strOptions += L"+Skill"; // TODO: Use GlobalText[176]
    }
    if (pItem->OptionLevel)
    {
        strOptions += L"+Option"; // TODO: Use GlobalText[177]
    }
    if (pItem->HasLuck) {
        strOptions += L"+Luck"; // TODO: Use GlobalText[178]
    }

    return strDisplayName + (strOptions.empty() ? L"" : L" " + strOptions);
}

OBJECT ObjectSelect;

void RenderObjectScreen(int Type, int ItemLevel, int excellentFlags, int ancientDiscriminator, vec3_t Target, int Select, bool PickUp)
{
    int Level = ItemLevel;
    vec3_t Direction, Position;

    VectorSubtract(Target, MousePosition, Direction);
    if (PickUp)
        VectorMA(MousePosition, 0.07f, Direction, Position);
    else
        VectorMA(MousePosition, 0.1f, Direction, Position);

    if (Type == MODEL_KRIS)
    {
        Position[0] -= 0.02f;
        Position[1] += 0.03f;
        Vector(180.f, 270.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_BOLT || Type == MODEL_ARROWS)
    {
        Vector(0.f, 270.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_LIGHT_SPEAR)
    {
        Position[1] += 0.05f;
        Vector(0.f, 90.f, 20.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CELESTIAL_BOW)
    {
        Vector(0.f, 90.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SYLPHID_RAY_HELM)
    {
        Position[1] -= 0.06f;
        Position[0] += 0.03f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_VENOM_MIST_HELM)
    {
        Position[1] += 0.07f;
        Position[0] -= 0.03f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_VENOM_MIST_ARMOR)
    {
        Position[1] += 0.1f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_DRAGON_KNIGHT_ARMOR)
    {
        Position[1] += 0.07f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SYLPH_WIND_BOW)
    {
        Position[1] += 0.12f;
        Vector(180.f, -90.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_GRAND_VIPER_STAFF)
    {
        Position[1] -= 0.1f;
        Position[0] += 0.025f;
        Vector(180.f, 0.f, 8.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_BOOK_OF_SAHAMUTT && Type <= MODEL_STAFF + 29)
    {
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SOLEIL_SCEPTER)
    {
        Position[1] += 0.1f;
        Position[0] -= 0.01;
        Vector(180.f, 90.f, 13.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ASHCROW_ARMOR)
    {
        Position[1] += 0.03f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ECLIPSE_HELM)
    {
        Position[0] -= 0.02f;
        Position[1] += 0.05f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ECLIPSE_ARMOR)
    {
        Position[1] += 0.05f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_IRIS_ARMOR)
    {
        Position[1] -= 0.05f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_VALIANT_ARMOR)
    {
        Position[1] -= 0.05f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (MODEL_MISTERY_HELM <= Type && MODEL_LILIUM_HELM >= Type)
    {
        Position[1] -= 0.05f;
        Vector(-90.f, 25.f, 0.f, ObjectSelect.Angle);
    }
    else if (MODEL_GLORIOUS_ARMOR <= Type && MODEL_LILIUM_ARMOR >= Type)
    {
        Position[1] -= 0.08f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_DAYBREAK)
    {
        Position[0] -= 0.02f;
        Position[1] += 0.03f;
        Vector(180.f, 90.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SHINING_SCEPTER)
    {
        Position[1] += 0.05f;
        Vector(180.f, 90.f, 13.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ALBATROSS_BOW || Type == MODEL_STINGER_BOW)
    {
        Position[0] -= 0.10f;
        Position[1] += 0.08f;
        Vector(180.f, -90.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_PLATINA_STAFF)
    {
        Position[0] += 0.02f;
        Position[1] += 0.02f;
        Vector(180.f, 90.f, 8.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ARROW_VIPER_BOW)
    {
        Vector(180.f, -90.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_CROSSBOW && Type < MODEL_BOW + MAX_ITEM_INDEX)
    {
        Vector(90.f, 180.f, 20.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_DRAGON_SPEAR)
    {
        Vector(180.f, 270.f, 20.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_SWORD && Type < MODEL_STAFF + MAX_ITEM_INDEX)
    {
        switch (Type)
        {
        case MODEL_MISTERY_STICK:							Position[1] += 0.04f;	break;
        case MODEL_ANCIENT_STICK:	Position[0] += 0.02f;	Position[1] += 0.03f;	break;
        case MODEL_DEMONIC_STICK:	Position[0] += 0.02f;							break;
        case MODEL_STORM_BLITZ_STICK:	Position[0] -= 0.02f;	Position[1] -= 0.02f;	break;
        case MODEL_ETERNAL_WING_STICK:	Position[0] += 0.01f;	Position[1] -= 0.01f;	break;
        }

        if (!ItemAttribute[Type - MODEL_ITEM].TwoHand)
        {
            Vector(180.f, 270.f, 15.f, ObjectSelect.Angle);
        }
        else
        {
            Vector(180.f, 270.f, 25.f, ObjectSelect.Angle);
        }
    }
    else if (Type >= MODEL_SHIELD && Type < MODEL_SHIELD + MAX_ITEM_INDEX)
    {
        Vector(270.f, 270.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HORN_OF_DINORANT)
    {
        Vector(-90.f, -90.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_DARK_HORSE_ITEM)
    {
        Vector(-90.f, -90.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_DARK_RAVEN_ITEM)
    {
        Vector(-90.f, -35.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SPIRIT)
    {
        Vector(-90.f, -90.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CAPE_OF_LORD)
    {
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_EVENT + 16)
    {
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SCROLL_OF_ARCHANGEL || Type == MODEL_BLOOD_BONE)
    {
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_INVISIBILITY_CLOAK)
    {
        Vector(290.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_EVENT + 11)
    {
        Vector(-90.f, -20.f, -20.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_EVENT + 12)
    {
        Vector(250.f, 140.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_EVENT + 14)
    {
        Vector(255.f, 160.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_EVENT + 15)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_RING_OF_FIRE && Type <= MODEL_RING_OF_MAGIC)
    {
        Vector(270.f, 160.f, 20.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ARMOR_OF_GUARDSMAN)
    {
        Vector(290.f, 0.f, 0.f, ObjectSelect.Angle);
    }

    else if (Type == MODEL_SPLINTER_OF_ARMOR)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.03f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_BLESS_OF_GUARDIAN)
    {
        Position[1] += 0.02f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CLAW_OF_BEAST)
    {
        Position[0] += 0.01f;
        Position[1] += 0.02f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_FRAGMENT_OF_HORN)
    {
        Position[0] += 0.01f;
        Position[1] += 0.02f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_BROKEN_HORN)
    {
        Position[0] += 0.01f;
        Position[1] += 0.05f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HORN_OF_FENRIR)
    {
        Position[0] += 0.01f;
        Position[1] += 0.04f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_OLD_SCROLL)
    {
        Position[1] -= 0.04f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ILLUSION_SORCERER_COVENANT)
    {
        Position[1] -= 0.03f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SCROLL_OF_BLOOD)
    {
        Position[1] -= 0.02f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 64)
    {
        Position[1] += 0.02f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_FLAME_OF_CONDOR)
    {
        Position[1] += 0.045f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_FEATHER_OF_CONDOR)
    {
        Position[1] += 0.04f;
        Vector(270.f, 120.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_WING_OF_ETERNAL)
    {
        Position[1] += 0.05f;
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_WING_OF_ILLUSION)
    {
        Position[1] += 0.05f;
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_WING_OF_RUIN)
    {
        Position[1] += 0.08f;
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CAPE_OF_EMPEROR)
    {
        Position[1] += 0.05f;
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_WINGS_OF_DESPAIR)
    {
        Position[1] += 0.05f;
        Vector(270.f, 0.f, 2.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 46)
    {
        Position[1] -= 0.04f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 47)
    {
        Position[1] -= 0.04f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 48)
    {
        Position[1] -= 0.04f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 54)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 58)
    {
        Position[1] += 0.07f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 59 || Type == MODEL_POTION + 60)
    {
        Position[1] += 0.06f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 61 || Type == MODEL_POTION + 62)
    {
        Position[1] += 0.06f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 53)
    {
        Position[1] += 0.042f;
        Vector(180.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 43)
    {
        Position[1] -= 0.027f;
        Position[0] += 0.005f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 44)
    {
        Position[1] -= 0.03f;
        Position[0] += 0.005f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 45)
    {
        Position[1] -= 0.02f;
        Position[0] += 0.005f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 70 && Type <= MODEL_POTION + 71)
    {
        Position[0] += 0.01f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 72 && Type <= MODEL_POTION + 77)
    {
        Position[1] += 0.08f;
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 59)
    {
        Position[0] += 0.01f;
        Position[1] += 0.02f;
        Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_HELPER + 54 && Type <= MODEL_HELPER + 58)
    {
        Position[1] -= 0.02f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 78 && Type <= MODEL_POTION + 82)
    {
        Position[1] += 0.01f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 60)
    {
        Position[1] -= 0.06f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 61)
    {
        Position[1] -= 0.04f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 83)
    {
        Position[1] += 0.06f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 91)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 92)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 93)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 95)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 94)
    {
        Position[0] += 0.01f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_CHERRY_BLOSSOM_PLAYBOX && Type <= MODEL_GOLDEN_CHERRY_BLOSSOM_BRANCH)
    {
        if (Type == MODEL_CHERRY_BLOSSOM_PLAYBOX)
        {
            Position[1] += 0.01f;
            Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
        }
        else if (Type == MODEL_CHERRY_BLOSSOM_WINE)
        {
            Position[1] -= 0.01f;
            Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
        }
        else if (Type == MODEL_CHERRY_BLOSSOM_RICE_CAKE)
        {
            Position[1] += 0.01f;
            Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
        }
        else if (Type == MODEL_CHERRY_BLOSSOM_FLOWER_PETAL)
        {
            Position[1] += 0.01f;
            Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
        }
        else if (Type == MODEL_POTION + 88)
        {
            Position[1] += 0.015f;
            Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
        }
        else if (Type == MODEL_POTION + 89)
        {
            Position[1] += 0.015f;
            Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
        }
        else if (Type == MODEL_GOLDEN_CHERRY_BLOSSOM_BRANCH)
        {
            Position[1] += 0.015f;
            Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
        }
    }
    else if (Type == MODEL_HELPER + 62)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.03f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 63)
    {
        Position[0] += 0.01f;
        Position[1] += 0.082f;
        Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 97 && Type <= MODEL_POTION + 98)
    {
        Position[1] += 0.09f;
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 96)
    {
        Position[1] -= 0.013f;
        Position[0] += 0.003f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (MODEL_DEMON <= Type && Type <= MODEL_SPIRIT_OF_GUARDIAN)
    {
        switch (Type)
        {
        case MODEL_DEMON:
            Position[1] -= 0.05f;
            break;
        case MODEL_SPIRIT_OF_GUARDIAN:
            Position[1] -= 0.02f;
            break;
        }
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_FLAME_OF_DEATH_BEAM_KNIGHT)
    {
        Position[1] += 0.05f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HORN_OF_HELL_MAINE)
    {
        Position[1] += 0.11f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_FEATHER_OF_DARK_PHOENIX)
    {
        Position[1] += 0.11f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ELITE_TRANSFER_SKELETON_RING)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 43)
    {
        //		Position[1] += 0.082f;
        Position[1] -= 0.03f;
        Vector(90.f, 0.f, 180.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 44)
    {
        Position[1] += 0.08f;
        Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 45)
    {
        Position[1] += 0.07f;
        Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_JACK_OLANTERN_TRANSFORMATION_RING)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CHRISTMAS_TRANSFORMATION_RING)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SCROLL_OF_BLOOD)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_GAME_MASTER_TRANSFORMATION_RING)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_MOONSTONE_PENDANT)
    {
        Position[0] += 0.00f;
        Position[1] += 0.02f;
        Vector(-48 - 150.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_GEMSTONE)
    {
        Position[1] += 0.02f;
        Vector(270.f, 90.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_JEWEL_OF_HARMONY)
    {
        Position[1] += 0.02f;
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_LOWER_REFINE_STONE || Type == MODEL_HIGHER_REFINE_STONE)
    {
        Position[0] -= 0.04f;
        Position[1] += 0.02f;
        Position[2] += 0.02f;
        Vector(270.f, -10.f, -45.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_PENDANT_OF_LIGHTING && Type < MODEL_HELPER + MAX_ITEM_INDEX && Type != MODEL_LOCHS_FEATHER && Type != MODEL_FRUITS)
    {
        Vector(270.f + 90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 12)
    {
        switch (Level)
        {
        case 0:Vector(180.f, 0.f, 0.f, ObjectSelect.Angle); break;
        case 1:Vector(270.f, 90.f, 0.f, ObjectSelect.Angle); break;
        case 2:Vector(90.f, 0.f, 0.f, ObjectSelect.Angle); break;
        }
    }
    else if (Type == MODEL_EVENT + 5)
    {
        Vector(270.f, 180.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_EVENT + 6)
    {
        Vector(270.f, 90.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_EVENT + 7)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 20)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 27)
    {
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_FIRECRACKER)
    {
        Position[1] += 0.08f;
        Vector(-50.f, -60.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_GM_GIFT)
    {
        //Position[1] += 0.08f;
        Vector(270.f, -25.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_CHAIN_LIGHTNING_PARCHMENT && Type <= MODEL_INNOVATION_PARCHMENT)
    {
        Position[0] += 0.03f;
        Position[1] += 0.03f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ORB_OF_TWISTING_SLASH)
    {
        Position[0] += 0.005f;
        Position[1] -= 0.015f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_VINE_ARMOR)
    {
        Position[1] -= 0.1f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_VINE_PANTS)
    {
        Position[1] -= 0.08f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SILK_ARMOR)
    {
        Position[1] -= 0.1f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SILK_PANTS)
    {
        Position[1] -= 0.08f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CRYSTAL_OF_DESTRUCTION)
    {
        Position[0] += 0.005f;
        Position[1] -= 0.015f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CRYSTAL_OF_RECOVERY || Type == MODEL_CRYSTAL_OF_MULTI_SHOT)
    {
        Position[0] += 0.005f;
        Position[1] -= 0.015f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else
    {
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }

    if (Type >= MODEL_SEED_FIRE && Type <= MODEL_SEED_EARTH)
    {
        Vector(10.f, -10.f, 10.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_SPHERE_MONO && Type <= MODEL_SPHERE_5)
    {
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_SEED_SPHERE_FIRE_1 && Type <= MODEL_SEED_SPHERE_EARTH_5)
    {
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_PET_RUDOLF)
    {
        Position[1] -= 0.05f;
        Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_PET_SKELETON)
    {
        Position[1] -= 0.05f;
        Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 140)
    {
        Position[1] += 0.09f;
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 145 && Type <= MODEL_POTION + 150)
    {
        Position[0] += 0.010f;
        Position[1] += 0.040f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_HELPER + 125 && Type <= MODEL_HELPER + 127)
    {
        Position[0] += 0.007f;
        Position[1] -= 0.035f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 124)
    {
        Position[1] -= 0.04f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_PET_PANDA)
    {
        Position[1] -= 0.05f;
        Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_PET_UNICORN)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.05f;
        Vector(255.f, 45.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SNOWMAN_TRANSFORMATION_RING)
    {
        Position[0] += 0.02f;
        Position[1] -= 0.02f;
        Vector(300.f, 10.f, 20.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_PANDA_TRANSFORMATION_RING)
    {
        //		Position[0] += 0.02f;
        Position[1] -= 0.02f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SKELETON_TRANSFORMATION_RING)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.035f;
        Vector(290.f, -20.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 128)
    {
        Position[0] += 0.017f;
        Position[1] -= 0.053f;
        Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 129)
    {
        Position[0] += 0.012f;
        Position[1] -= 0.045f;
        Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 134)
    {
        Position[0] += 0.005f;
        Position[1] -= 0.033f;
        Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 130)
    {
        Position[0] += 0.007f;
        Position[1] += 0.005f;
        Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 131)
    {
        Position[0] += 0.017f;
        Position[1] -= 0.053f;
        Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 132)
    {
        Position[0] += 0.007f;
        Position[1] += 0.045f;
        Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 133)
    {
        Position[0] += 0.017f;
        Position[1] -= 0.053f;
        Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 69)
    {
        Position[0] += 0.005f;
        Position[1] -= 0.05f;
        Vector(270.f, -30.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 70)
    {
        Position[0] += 0.040f;
        Position[1] -= 0.000f;
        Vector(270.f, -0.f, 70.f, ObjectSelect.Angle);
    }

    else if (Type == MODEL_HELPER + 81)
    {
        Position[0] += 0.005f;
        Position[1] += 0.035f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 82)
    {
        Position[0] += 0.005f;
        Position[1] += 0.035f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 93)
    {
        Position[0] += 0.005f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 94)
    {
        Position[0] += 0.005f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 66)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.05f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 100)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.05f;
        Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CHRISTMAS_FIRECRACKER)
    {
        Position[0] += 0.02f;
        Position[1] -= 0.03f;
        //Vector(270.f,0.f,30.f,ObjectSelect.Angle);
        Vector(290.f, -40.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CHROMATIC_STAFF)
    {
        Position[0] += 0.02f;
        Position[1] -= 0.06f;
        Vector(180.f, 90.f, 10.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_RAVEN_STICK)
    {
        Position[1] -= 0.05f;
        Vector(180.f, 90.f, 10.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_BEUROBA)
    {
        Position[1] += 0.02f;
        Vector(180.f, 90.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_STRYKER_SCEPTER)
    {
        Position[0] -= 0.03f;
        Position[1] += 0.06f;
        Vector(180.f, 90.f, 2.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_AIR_LYN_BOW)
    {
        Position[0] -= 0.07f;
        Position[1] += 0.07f;
        Vector(180.f, -90.f, 15.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CRYSTAL_OF_FLAME_STRIKE)
    {
        Position[0] += 0.005f;
        Position[1] -= 0.015f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 71 || Type == MODEL_HELPER + 72 || Type == MODEL_HELPER + 73 || Type == MODEL_HELPER + 74 || Type == MODEL_HELPER + 75)
    {
        Position[1] += 0.07f;
        Vector(270.f, 180.f, 0.f, ObjectSelect.Angle);
        if (Select != 1)
        {
            ObjectSelect.Angle[1] = WorldTime * 0.2f;
        }
    }
    else if (Type >= MODEL_TYPE_CHARM_MIXWING + EWS_BEGIN && Type <= MODEL_TYPE_CHARM_MIXWING + EWS_END)
    {
        Position[1] -= 0.03f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 97 || Type == MODEL_HELPER + 98 || Type == MODEL_POTION + 91)
    {
        Position[1] -= 0.04f;
        Position[0] += 0.002f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 99)
    {
        Position[0] += 0.002f;
        Position[1] += 0.025f;
        Vector(270.0f, 180.0f, 45.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 110)
    {
        Position[0] += 0.005f;
        Position[1] -= 0.02f;
    }
    else if (Type == MODEL_POTION + 111)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.02f;
    }
    else if (Type == MODEL_HELPER + 107)
    {
        Position[0] -= 0.0f;
        Position[1] += 0.0f;
        Vector(90.0f, 225.0f, 45.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 104)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.03f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 105)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.03f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 103)
    {
        Position[0] += 0.01f;
        Position[1] += 0.01f;
        Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 133)
    {
        Position[0] += 0.01f;
        Position[1] -= 0.0f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (MODEL_SUSPICIOUS_SCRAP_OF_PAPER <= Type && Type <= MODEL_COMPLETE_SECROMICON)
    {
        switch (Type)
        {
        case MODEL_SUSPICIOUS_SCRAP_OF_PAPER:
        {
            Position[0] += 0.005f;
            //Position[1] -= 0.02f;
        }break;
        case MODEL_GAIONS_ORDER:
        {
            Position[0] += 0.005f;
            Position[1] += 0.05f;
            Vector(0.0f, 0.0f, 30.0f, ObjectSelect.Angle);
        }break;
        case MODEL_FIRST_SECROMICON_FRAGMENT:
        case MODEL_SECOND_SECROMICON_FRAGMENT:
        case MODEL_THIRD_SECROMICON_FRAGMENT:
        case MODEL_FOURTH_SECROMICON_FRAGMENT:
        case MODEL_FIFTH_SECROMICON_FRAGMENT:
        case MODEL_SIXTH_SECROMICON_FRAGMENT:
        {
            Position[0] += 0.005f;
            Position[1] += 0.05f;
            Vector(0.0f, 0.0f, 30.0f, ObjectSelect.Angle);
        }break;
        case MODEL_COMPLETE_SECROMICON:
        {
            Position[0] += 0.005f;
            Position[1] += 0.05f;
            Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
        }break;
        }
    }
    else if (Type >= MODEL_HELPER + 109 && Type <= MODEL_HELPER + 112)
    {
        Position[0] += 0.025f;
        Position[1] -= 0.035f;
        Vector(270.0f, 25.0f, 25.0f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_HELPER + 113 && Type <= MODEL_HELPER + 115)
    {
        Position[0] += 0.005f;
        Position[1] -= 0.00f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 112 && Type <= MODEL_POTION + 113)
    {
        Position[0] += 0.05f;
        Position[1] += 0.009f;
        Vector(270.0f, 180.0f, 45.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 120)
    {
        Position[0] += 0.01f;
        Position[1] += 0.05f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (MODEL_POTION + 134 <= Type && Type <= MODEL_POTION + 139)
    {
        Position[0] += 0.00f;
        Position[1] += 0.05f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 116)
    {
        Position[1] -= 0.03f;
        Position[0] += 0.005f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 114 && Type <= MODEL_POTION + 119)
    {
        Position[0] += 0.00f;
        Position[1] += 0.06f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 126 && Type <= MODEL_POTION + 129)
    {
        Position[0] += 0.00f;
        Position[1] += 0.06f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_POTION + 130 && Type <= MODEL_POTION + 132)
    {
        Position[0] += 0.00f;
        Position[1] += 0.06f;
        Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_HELPER + 121)
    {
        Position[1] -= 0.04f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SACRED_HELM)
    {
        Position[1] += 0.04f;
        Vector(-90.f, 25.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ARMORINVEN_60
        || Type == MODEL_ARMORINVEN_61
        || Type == MODEL_ARMORINVEN_62)
    {
        Position[0] += 0.01f;
        Position[1] += 0.08f;
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_ARMORINVEN_74)
    {
        Position[0] += 0.01f;
        Position[1] += 0.05f;
        Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_SACRED_GLOVE)
    {
        Position[0] += 0.005f;
        Position[1] += 0.015f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_STORM_HARD_GLOVE && Type <= MODEL_PIERCING_BLADE_GLOVE)
    {
        Position[0] += 0.002f;
        Position[1] += 0.02f;
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_PHOENIX_SOUL_STAR)
    {
        Position[0] -= 0.005f;
        Position[1] += 0.015f;
        Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CAPE_OF_FIGHTER)
    {
        Position[1] += 0.01f;
        Position[0] += 0.015f;
        Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_CAPE_OF_OVERRULE)
    {
        Position[1] += 0.15f;
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type >= MODEL_CHAIN_DRIVE_PARCHMENT && Type <= MODEL_INCREASE_BLOCK_PARCHMENT)
    {
        Position[0] += 0.03f;
        Position[1] += 0.03f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_WING + 135)
    {
        Position[1] += 0.05f;
        Position[0] += 0.005f;
    }
    else if (Type >= MODEL_HELPER + 135 && Type <= MODEL_HELPER + 145)
    {
        Position[1] += 0.02f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (Type == MODEL_POTION + 160 || Type == MODEL_POTION + 161)
    {
        Position[1] += 0.05f;
        Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
    }
    else if (COMGEM::Check_Jewel_Com(Type - MODEL_ITEM) != COMGEM::NOGEM)
    {
        Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
        switch (COMGEM::Check_Jewel_Com(Type - MODEL_ITEM))
        {
        case COMGEM::eLOW_C:
        case COMGEM::eUPPER_C:
            Vector(270.f, -10.f, -45.f, ObjectSelect.Angle);
            break;
        case COMGEM::eLIFE_C:
        case COMGEM::eCREATE_C:
            Position[1] -= 0.05f;
            break;
        case COMGEM::eGEMSTONE_C:
            Position[1] -= 0.05f;
            Vector(270.f, 90.f, 0.f, ObjectSelect.Angle);
            break;
        }
    }

    switch (Type)
    {
    case MODEL_FLAMBERGE:
    {
        Position[0] -= 0.02f;
        Position[1] += 0.04f;
        Vector(180.f, 270.f, 10.f, ObjectSelect.Angle);
    }break;
    case MODEL_SWORD_BREAKER:
    {
        Vector(180.f, 270.f, 15.f, ObjectSelect.Angle);
    }break;
    case MODEL_IMPERIAL_SWORD:
    {
        Position[1] += 0.02f;
        Vector(180.f, 270.f, 10.f, ObjectSelect.Angle);
    }break;
    case MODEL_FROST_MACE:
    {
        Position[0] -= 0.02f;
        Vector(180.f, 270.f, 15.f, ObjectSelect.Angle);
    }
    break;
    case MODEL_ABSOLUTE_SCEPTER:
    {
        Position[0] -= 0.02f;
        Position[1] += 0.04f;
        Vector(180.f, 270.f, 15.f, ObjectSelect.Angle);
    }break;
    case MODEL_DEADLY_STAFF:
    {
        Vector(180.f, 90.f, 10.f, ObjectSelect.Angle);
    }break;
    case MODEL_IMPERIAL_STAFF:
    {
        Vector(180.f, 90.f, 10.f, ObjectSelect.Angle);
    }break;
    case MODEL_STAFF + 32:
    {
        Vector(180.f, 90.f, 10.f, ObjectSelect.Angle);
    }break;
    }

    switch (Type)
    {
    case MODEL_CHAOS_LIGHTNING_STAFF:
    {
        Vector(0.f, 0.f, 205.f, ObjectSelect.Angle);
    }break;
    }

    switch (Type)
    {
    case MODEL_ORB_OF_HEALING:
    case MODEL_ORB_OF_GREATER_DEFENSE:
    case MODEL_ORB_OF_GREATER_DAMAGE:
    case MODEL_ORB_OF_SUMMONING:
    {
        Position[0] += 0.005f;
        Position[1] -= 0.02f;
    }
    break;
    case MODEL_POTION + 21:
    {
        Position[0] += 0.005f;
        Position[1] -= 0.005f;
    }
    break;
    case MODEL_JEWEL_OF_BLESS:
    case MODEL_JEWEL_OF_SOUL:
    case MODEL_JEWEL_OF_CREATION:
    {
        Position[0] += 0.005f;
        Position[1] += 0.015f;
    }
    break;
    }

    if (1 == Select)
    {
        ObjectSelect.Angle[1] = WorldTime * 0.45f;
    }

    ObjectSelect.Type = Type;
    if (ObjectSelect.Type >= MODEL_HELM && ObjectSelect.Type < MODEL_BOOTS + MAX_ITEM_INDEX
        || ObjectSelect.Type == MODEL_ARMORINVEN_60
        || ObjectSelect.Type == MODEL_ARMORINVEN_61
        || ObjectSelect.Type == MODEL_ARMORINVEN_62
        || ObjectSelect.Type == MODEL_ARMORINVEN_74)
        ObjectSelect.Type = MODEL_PLAYER;
    else if (ObjectSelect.Type == MODEL_POTION + 12)
    {
        if (Level == 0)
        {
            ObjectSelect.Type = MODEL_EVENT;
            Type = MODEL_EVENT;
        }
        else if (Level == 2)
        {
            ObjectSelect.Type = MODEL_EVENT + 1;
            Type = MODEL_EVENT + 1;
        }
    }

    BMD* b = &Models[ObjectSelect.Type];
    b->CurrentAction = 0;
    ObjectSelect.AnimationFrame = 0;
    ObjectSelect.PriorAnimationFrame = 0;
    ObjectSelect.PriorAction = 0;
    if (Type >= MODEL_HELM && Type < MODEL_HELM + MAX_ITEM_INDEX)
    {
        b->BodyHeight = -160.f;

        if (Check_LuckyItem(Type - MODEL_ITEM))				b->BodyHeight -= 10.0f;
        if (Type == MODEL_HELM + 65 || Type == MODEL_HELM + 70)	Position[0] += 0.04f;
    }
    else if (Type >= MODEL_ARMOR && Type < MODEL_ARMOR + MAX_ITEM_INDEX)
    {
        b->BodyHeight = -100.f;

        if (Check_LuckyItem(Type - MODEL_ITEM))	b->BodyHeight -= 13.0f;
    }
    else if (Type >= MODEL_GLOVES && Type < MODEL_GLOVES + MAX_ITEM_INDEX)
        b->BodyHeight = -70.f;
    else if (Type >= MODEL_PANTS && Type < MODEL_PANTS + MAX_ITEM_INDEX)
        b->BodyHeight = -50.f;
    else
        b->BodyHeight = 0.f;
    float Scale = 0.f;

    if (Type >= MODEL_HELM && Type < MODEL_BOOTS + MAX_ITEM_INDEX)
    {
        if (Type >= MODEL_HELM && Type < MODEL_HELM + MAX_ITEM_INDEX)
        {
            Scale = MODEL_MISTERY_HELM <= Type && MODEL_LILIUM_HELM >= Type ? 0.007f : 0.0039f;
            if (Type == MODEL_SYLPHID_RAY_HELM)
                Scale = 0.007f;

            if (Type == MODEL_HELM + 65 || Type == MODEL_HELM + 70)	Scale = 0.007f;
        }
        else if (Type >= MODEL_ARMOR && Type < MODEL_ARMOR + MAX_ITEM_INDEX)
            Scale = 0.0039f;
        else if (Type >= MODEL_GLOVES && Type < MODEL_GLOVES + MAX_ITEM_INDEX)
            Scale = 0.0038f;
        else if (Type >= MODEL_PANTS && Type < MODEL_PANTS + MAX_ITEM_INDEX)
            Scale = 0.0033f;
        else if (Type >= MODEL_BOOTS && Type < MODEL_BOOTS + MAX_ITEM_INDEX)
            Scale = 0.0032f;
        else if (Type == MODEL_VENOM_MIST_ARMOR)
            Scale = 0.0035f;
        else if (Type == MODEL_VOLCANO_ARMOR)
            Scale = 0.0035f;
        else if (Type == MODEL_DRAGON_KNIGHT_ARMOR)
            Scale = 0.0033f;
        if (Type == MODEL_ASHCROW_ARMOR)
            Scale = 0.0032f;
        else if (Type == MODEL_ECLIPSE_ARMOR)
            Scale = 0.0032f;
        else if (Type == MODEL_GLORIOUS_GLOVES)
            Scale = 0.0032f;
    }
    else
    {
        if (Type == MODEL_WINGS_OF_DARKNESS)
            Scale = 0.0015f;
        else if (COMGEM::Check_Jewel_Com(Type - MODEL_ITEM) != COMGEM::NOGEM)
        {
            Scale = 0.004f;
            switch (COMGEM::Check_Jewel_Com(Type - MODEL_ITEM))
            {
            case COMGEM::eLOW_C:
                Position[0] -= 0.05f;
                Scale = 0.003f;
                break;
            case COMGEM::eUPPER_C:
                Position[0] -= 0.05f;
                Scale = 0.004f;
                break;
            case COMGEM::eCREATE_C:
                Position[1] += 0.05f;
                Scale = 0.0025f;
                break;
            case COMGEM::eCHAOS_C:
                Position[1] += 0.025f;
                Scale = 0.002f;
                break;
            case COMGEM::ePROTECT_C:
                Position[1] += 0.05f;
                Scale = 0.0036f;
                break;
            case COMGEM::eLIFE_C:
                Position[1] += 0.025f;
                Scale = 0.0035f;
                break;
            case COMGEM::eGEMSTONE_C:
                Position[1] += 0.05f;
                Scale = 0.0035f;
                break;
            case COMGEM::eHARMONY_C:
                Scale = 0.005f;
                break;
            }
        }
        else if (Type >= MODEL_RED_RIBBON_BOX && Type <= MODEL_BLUE_RIBBON_BOX)
        {
            Scale = 0.001f;
            Position[1] -= 0.05f;
        }
        else if (Type >= MODEL_SEED_FIRE && Type <= MODEL_SEED_EARTH)
            Scale = 0.0022f;
        else if (Type >= MODEL_SPHERE_MONO && Type <= MODEL_SPHERE_5)
            Scale = 0.0017f;
        else if (Type >= MODEL_SEED_SPHERE_FIRE_1 && Type <= MODEL_SEED_SPHERE_EARTH_5)
            Scale = 0.0017f;
        else if (Type >= MODEL_WING && Type < MODEL_WING + MAX_ITEM_INDEX)
        {
            Scale = 0.002f;
        }
        else
            if (Type == MODEL_PUMPKIN_OF_LUCK || Type == MODEL_JACK_OLANTERN_FOOD)
            {
                Scale = 0.003f;
            }
            else
                if (Type >= MODEL_JACK_OLANTERN_BLESSINGS && Type <= MODEL_JACK_OLANTERN_CRY)
                {
                    Scale = 0.0025f;
                }
                else
                    if (Type == MODEL_JACK_OLANTERN_DRINK)
                    {
                        Scale = 0.001f;
                    }
                    else
                        if (Type >= MODEL_PINK_CHOCOLATE_BOX && Type <= MODEL_BLUE_CHOCOLATE_BOX)
                        {
                            Scale = 0.002f;
                            Position[1] += 0.05f;
                            Vector(0.f, ObjectSelect.Angle[1], 0.f, ObjectSelect.Angle);
                        }
                        else
                            if (Type >= MODEL_EVENT + 21 && Type <= MODEL_EVENT + 23)
                            {
                                Scale = 0.002f;
                                if (Type == MODEL_EVENT + 21)
                                    Position[1] += 0.08f;
                                else
                                    Position[1] += 0.06f;
                                Vector(0.f, ObjectSelect.Angle[1], 0.f, ObjectSelect.Angle);
                            }
                            else if (Type == MODEL_POTION + 21)
                                Scale = 0.002f;
                            else if (Type == MODEL_GREAT_REIGN_CROSSBOW)
                                Scale = 0.002f;
                            else if (Type == MODEL_EVENT + 11)
                                Scale = 0.0015f;
                            else if (Type == MODEL_DARK_HORSE_ITEM)
                                Scale = 0.0015f;
                            else if (Type == MODEL_DARK_RAVEN_ITEM)
                                Scale = 0.005f;
                            else if (Type == MODEL_CAPE_OF_LORD)
                                Scale = 0.002f;
                            else if (Type == MODEL_EVENT + 16)
                                Scale = 0.002f;
                            else if (Type == MODEL_SCROLL_OF_ARCHANGEL)
                                Scale = 0.002f;
                            else if (Type == MODEL_BLOOD_BONE)
                                Scale = 0.0018f;
                            else if (Type == MODEL_INVISIBILITY_CLOAK)
                                Scale = 0.0018f;
                            else if (Type == MODEL_HELPER + 46)
                            {
                                Scale = 0.0018f;
                            }
                            else if (Type == MODEL_HELPER + 47)
                            {
                                Scale = 0.0018f;
                            }
                            else if (Type == MODEL_HELPER + 48)
                            {
                                Scale = 0.0018f;
                            }
                            else if (Type == MODEL_POTION + 54)
                            {
                                Scale = 0.0024f;
                            }
                            else if (Type == MODEL_POTION + 58)
                            {
                                Scale = 0.0012f;
                            }
                            else if (Type == MODEL_POTION + 59 || Type == MODEL_POTION + 60)
                            {
                                Scale = 0.0010f;
                            }
                            else if (Type == MODEL_POTION + 61 || Type == MODEL_POTION + 62)
                            {
                                Scale = 0.0009f;
                            }
                            else if (Type == MODEL_POTION + 53)
                            {
                                Scale = 0.00078f;
                            }
                            else if (Type == MODEL_HELPER + 43 || Type == MODEL_HELPER + 44 || Type == MODEL_HELPER + 45)
                            {
                                Scale = 0.0021f;
                            }
                            else if (Type >= MODEL_POTION + 70 && Type <= MODEL_POTION + 71)
                            {
                                Scale = 0.0028f;
                            }
                            else if (Type >= MODEL_POTION + 72 && Type <= MODEL_POTION + 77)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_HELPER + 59)
                            {
                                Scale = 0.0008f;
                            }
                            else if (Type >= MODEL_HELPER + 54 && Type <= MODEL_HELPER + 58)
                            {
                                Scale = 0.004f;
                            }
                            else if (Type >= MODEL_POTION + 78 && Type <= MODEL_POTION + 82)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_HELPER + 60)
                            {
                                Scale = 0.005f;
                            }
                            else if (Type == MODEL_HELPER + 61)
                            {
                                Scale = 0.0018f;
                            }
                            else if (Type == MODEL_POTION + 83)
                            {
                                Scale = 0.0009f;
                            }
                            else if (Type == MODEL_HELPER + 43 || Type == MODEL_HELPER + 44 || Type == MODEL_HELPER + 45)
                            {
                                Scale = 0.0021f;
                            }
                            else if (Type == MODEL_POTION + 91)
                            {
                                Scale = 0.0034f;
                            }
                            else if (Type == MODEL_POTION + 92)
                            {
                                Scale = 0.0024f;
                            }
                            else if (Type == MODEL_POTION + 93)
                            {
                                Scale = 0.0024f;
                            }
                            else if (Type == MODEL_POTION + 95)
                            {
                                Scale = 0.0024f;
                            }
                            else if (Type == MODEL_POTION + 94)
                            {
                                Scale = 0.0022f;
                            }
                            else if (Type == MODEL_CHERRY_BLOSSOM_PLAYBOX)
                            {
                                Scale = 0.0031f;
                            }
                            else if (Type == MODEL_CHERRY_BLOSSOM_WINE)
                            {
                                Scale = 0.0044f;
                            }
                            else if (Type == MODEL_CHERRY_BLOSSOM_RICE_CAKE)
                            {
                                Scale = 0.0031f;
                            }
                            else if (Type == MODEL_CHERRY_BLOSSOM_FLOWER_PETAL)
                            {
                                Scale = 0.0061f;
                            }
                            else if (Type == MODEL_POTION + 88)
                            {
                                Scale = 0.0035f;
                            }
                            else if (Type == MODEL_POTION + 89)
                            {
                                Scale = 0.0035f;
                            }
                            else if (Type == MODEL_GOLDEN_CHERRY_BLOSSOM_BRANCH)
                            {
                                Scale = 0.0035f;
                            }
                            else if (Type >= MODEL_HELPER + 62 && Type <= MODEL_HELPER + 63)
                            {
                                Scale = 0.002f;
                            }
                            else if (Type >= MODEL_POTION + 97 && Type <= MODEL_POTION + 98)
                            {
                                Scale = 0.003f;
                            }
                            else if (Type == MODEL_POTION + 96)
                            {
                                Scale = 0.0028f;
                            }
                            else if (MODEL_DEMON == Type || Type == MODEL_SPIRIT_OF_GUARDIAN)
                            {
                                switch (Type)
                                {
                                case MODEL_DEMON: Scale = 0.0005f; break;
                                case MODEL_SPIRIT_OF_GUARDIAN: Scale = 0.0016f; break;
                                }
                            }
                            else if (Type == MODEL_PET_RUDOLF)
                            {
                                Scale = 0.0015f;
                            }
                            else if (Type == MODEL_PET_PANDA)
                            {
                                Scale = 0.0020f;
                            }
                            else if (Type == MODEL_SNOWMAN_TRANSFORMATION_RING)
                            {
                                Scale = 0.0026f;
                            }
                            else if (Type == MODEL_PANDA_TRANSFORMATION_RING)
                            {
                                Scale = 0.0026f;
                            }
                            else if (Type == MODEL_HELPER + 69)
                            {
                                Scale = 0.0023f;
                            }
                            else if (Type == MODEL_HELPER + 70)
                            {
                                Scale = 0.0018f;
                            }
                            else if (Type == MODEL_HELPER + 81)
                                Scale = 0.0012f;
                            else if (Type == MODEL_HELPER + 82)
                                Scale = 0.0012f;
                            else if (Type == MODEL_HELPER + 93)
                                Scale = 0.0021f;
                            else if (Type == MODEL_HELPER + 94)
                                Scale = 0.0021f;
                            else if (Type == MODEL_DIVINE_SWORD_OF_ARCHANGEL)
                            {
                                if (ItemLevel >= 0)
                                {
                                    Scale = 0.0025f;
                                }
                                else
                                {
                                    Scale = 0.001f;
                                    ItemLevel = 0;
                                }
                            }
                            else if (Type == MODEL_DIVINE_STAFF_OF_ARCHANGEL)
                            {
                                if (ItemLevel >= 0)
                                {
                                    Scale = 0.0019f;
                                }
                                else
                                {
                                    Scale = 0.001f;
                                    ItemLevel = 0;
                                }
                            }
                            else if (Type == MODEL_DIVINE_CB_OF_ARCHANGEL)
                            {
                                if (ItemLevel >= 0)
                                {
                                    Scale = 0.0025f;
                                }
                                else
                                {
                                    Scale = 0.0015f;
                                    ItemLevel = 0;
                                }
                            }
                            else if (Type >= MODEL_BATTLE_SCEPTER && Type <= MODEL_LORD_SCEPTER)
                            {
                                Scale = 0.003f;
                            }
                            else if (Type == MODEL_GREAT_LORD_SCEPTER)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_STRYKER_SCEPTER)
                            {
                                Scale = 0.0024f;
                            }
                            else if (Type == MODEL_EVENT + 12)
                            {
                                Scale = 0.0012f;
                            }
                            else if (Type == MODEL_EVENT + 13)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_EVENT + 14)
                            {
                                Scale = 0.0028f;
                            }
                            else if (Type == MODEL_EVENT + 15)
                            {
                                Scale = 0.0023f;
                            }
                            else if (Type >= MODEL_JEWEL_OF_CREATION && Type < MODEL_TEAR_OF_ELF)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type >= MODEL_TEAR_OF_ELF && Type < MODEL_POTION + 27)
                            {
                                Scale = 0.0028f;
                            }
                            else if (Type == MODEL_FIRECRACKER)
                            {
                                Scale = 0.007f;
                            }
                            else if (Type == MODEL_CHRISTMAS_FIRECRACKER)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_GM_GIFT)
                            {
                                Scale = 0.0014f;
                            }
                            else if (Type == MODEL_MOONSTONE_PENDANT)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_GEMSTONE)
                            {
                                Scale = 0.0035f;
                            }
                            else if (Type == MODEL_JEWEL_OF_HARMONY)
                            {
                                Scale = 0.005f;
                            }
                            else if (Type == MODEL_LOWER_REFINE_STONE)
                            {
                                Position[1] += -0.005f;
                                Scale = 0.0035f;
                            }
                            else if (Type == MODEL_HIGHER_REFINE_STONE)
                            {
                                Position[1] += -0.005f;
                                Scale = 0.004f;
                            }
                            else if (Type == MODEL_SIEGE_POTION)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_HELPER + 43 || Type == MODEL_HELPER + 44 || Type == MODEL_HELPER + 45)
                            {
                                Scale = 0.0021f;
                            }
                            else if (Type == MODEL_HELPER + 7)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_LIFE_STONE_ITEM)
                            {
                                Scale = 0.0025f;
                            }
                            else if (Type == MODEL_SPLINTER_OF_ARMOR)
                            {
                                Scale = 0.0019f;
                            }
                            else if (Type == MODEL_BLESS_OF_GUARDIAN)
                            {
                                Scale = 0.004f;
                            }
                            else if (Type == MODEL_CLAW_OF_BEAST)
                            {
                                Scale = 0.004f;
                            }
                            else if (Type == MODEL_FRAGMENT_OF_HORN)
                            {
                                Scale = 0.004f;
                            }
                            else if (Type == MODEL_BROKEN_HORN)
                            {
                                Scale = 0.007f;
                            }
                            else if (Type == MODEL_HORN_OF_FENRIR)
                            {
                                Scale = 0.005f;
                            }
                            else if (Type == MODEL_SYLPH_WIND_BOW)
                            {
                                Scale = 0.0022f;
                            }
                            else if (Type == MODEL_OLD_SCROLL)
                            {
                                Scale = 0.0013f;
                            }
                            else if (Type == MODEL_ILLUSION_SORCERER_COVENANT)
                            {
                                Scale = 0.003f;
                            }
                            else if (Type == MODEL_SCROLL_OF_BLOOD)
                            {
                                Scale = 0.003f;
                            }
                            else if (Type == MODEL_POTION + 64)
                            {
                                Scale = 0.003f;
                            }
                            else if (Type == MODEL_FLAME_OF_DEATH_BEAM_KNIGHT)
                                Scale = 0.003f;
                            else if (Type == MODEL_HORN_OF_HELL_MAINE)
                                Scale = 0.0035f;
                            else if (Type == MODEL_FEATHER_OF_DARK_PHOENIX)
                                Scale = 0.0035f;
                            else if (Type == MODEL_EYE_OF_ABYSSAL)
                                Scale = 0.003f;
                            else if (Type == MODEL_FLAME_OF_CONDOR)
                                Scale = 0.005f;
                            else if (Type == MODEL_FEATHER_OF_CONDOR)
                                Scale = 0.005f;
                            else if (Type == MODEL_DAYBREAK)
                            {
                                Scale = 0.0028f;
                            }
                            else if (Type == MODEL_ALBATROSS_BOW)
                            {
                                Scale = 0.0020f;
                            }
                            else if (Type == MODEL_STINGER_BOW)
                            {
                                Scale = 0.0032f;
                            }
                            else if (Type == MODEL_LOCHS_FEATHER || Type == MODEL_FRUITS)
                            {
                                Scale = 0.003f;
                            }
                            else if (Type == MODEL_POTION + 100)
                            {
                                Scale = 0.0040f;
                            }
                            else if (Type >= MODEL_POTION && Type < MODEL_POTION + MAX_ITEM_INDEX)
                            {
                                Scale = 0.0035f;
                            }
                            else if (Type >= MODEL_SPEAR && Type < MODEL_SPEAR + MAX_ITEM_INDEX)
                            {
                                if (Type == MODEL_DRAGON_SPEAR)
                                    Scale = 0.0018f;
                                else if (Type == MODEL_BEUROBA)
                                    Scale = 0.0025f;
                                else
                                    Scale = 0.0021f;
                            }
                            else if (Type >= MODEL_STAFF && Type < MODEL_STAFF + MAX_ITEM_INDEX)
                            {
                                if (Type >= MODEL_MISTERY_STICK && Type <= MODEL_ETERNAL_WING_STICK)
                                    Scale = 0.0028f;
                                else if (Type >= MODEL_BOOK_OF_SAHAMUTT && Type <= MODEL_STAFF + 29)
                                    Scale = 0.004f;
                                else if (Type == MODEL_CHROMATIC_STAFF)
                                    Scale = 0.0028f;
                                else if (Type == MODEL_RAVEN_STICK)
                                    Scale = 0.0028f;
                                else
                                    Scale = 0.0022f;
                            }
                            else if (Type == MODEL_ARROWS)
                                Scale = 0.0011f;
                            else if (Type == MODEL_BOLT)
                                Scale = 0.0012f;
                            else if (Type == MODEL_EVENT + 6)
                                Scale = 0.0039f;
                            else if (Type == MODEL_EVENT + 8)
                                Scale = 0.0015f;
                            else if (Type == MODEL_EVENT + 9)
                                Scale = 0.0019f;
                            else
                            {
                                Scale = 0.0025f;
                            }

        if (Type >= MODEL_TYPE_CHARM_MIXWING + EWS_BEGIN
            && Type <= MODEL_TYPE_CHARM_MIXWING + EWS_END)
        {
            Scale = 0.0020f;
        }

        if (Type == MODEL_EVENT + 10)
        {
            Scale = 0.001f;
        }
        else if (Type >= MODEL_CHAIN_LIGHTNING_PARCHMENT && Type <= MODEL_INNOVATION_PARCHMENT)
        {
            Scale = 0.0023f;
        }
        else if (Type == MODEL_HELPER + 66)
        {
            Scale = 0.0020f;
        }
        else if (Type == MODEL_POTION + 140)
        {
            Scale = 0.0026f;
        }
        else if (Type == MODEL_SKELETON_TRANSFORMATION_RING)
        {
            Scale = 0.0033f;
        }
        else if (Type == MODEL_PET_SKELETON)
        {
            Scale = 0.0009f;
        }
        else if (Type >= MODEL_POTION + 145 && Type <= MODEL_POTION + 150)
        {
            Scale = 0.0018f;
        }
        else if (Type >= MODEL_HELPER + 125 && Type <= MODEL_HELPER + 127)
        {
            Scale = 0.0013f;
        }
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
        else if (Type == MODEL_HELPER + 128)		// 매조각상
        {
            Scale = 0.0035f;
        }
        else if (Type == MODEL_HELPER + 129)		// 양조각상
        {
            Scale = 0.0035f;
        }
        else if (Type == MODEL_HELPER + 134)		// 편자
        {
            Scale = 0.0033f;
        }
#endif	//LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        else if (Type == MODEL_HELPER + 130)		// 오크참
        {
            Scale = 0.0032f;
        }
        else if (Type == MODEL_HELPER + 131)		// 메이플참
        {
            Scale = 0.0033f;
        }
        else if (Type == MODEL_HELPER + 132)		// 골든오크참
        {
            Scale = 0.0025f;
        }
        else if (Type == MODEL_HELPER + 133)		// 골든메이플참
        {
            Scale = 0.0033f;
        }
#endif	//LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
        else if (Type == MODEL_HELPER + 71 || Type == MODEL_HELPER + 72 || Type == MODEL_HELPER + 73 || Type == MODEL_HELPER + 74 || Type == MODEL_HELPER + 75)
        {
            Scale = 0.0019f;
        }
        else if (Type == MODEL_AIR_LYN_BOW)
        {
            Scale = 0.0023f;
        }
        else if (Type == MODEL_HELPER + 97 || Type == MODEL_HELPER + 98 || Type == MODEL_POTION + 91)
        {
            Scale = 0.0028f;
        }
        else if (Type == MODEL_HELPER + 99)
        {
            Scale = 0.0025f;
        }
        else if (Type == MODEL_POTION + 110)
        {
            Scale = 0.004f;
        }
        else if (Type == MODEL_HELPER + 107)
        {
            Scale = 0.0034f;
        }
        else if (Type == MODEL_POTION + 133)
        {
            Scale = 0.0030f;
        }
        else if (Type == MODEL_HELPER + 105)
        {
            Scale = 0.002f;
        }
        else if (MODEL_SUSPICIOUS_SCRAP_OF_PAPER <= Type && Type <= MODEL_COMPLETE_SECROMICON)
        {
            switch (Type)
            {
            case MODEL_SUSPICIOUS_SCRAP_OF_PAPER:
            {
                Scale = 0.004f;
            }break;
            case MODEL_GAIONS_ORDER:
            {
                Scale = 0.005f;
            }break;
            case MODEL_FIRST_SECROMICON_FRAGMENT:
            case MODEL_SECOND_SECROMICON_FRAGMENT:
            case MODEL_THIRD_SECROMICON_FRAGMENT:
            case MODEL_FOURTH_SECROMICON_FRAGMENT:
            case MODEL_FIFTH_SECROMICON_FRAGMENT:
            case MODEL_SIXTH_SECROMICON_FRAGMENT:
            {
                Scale = 0.004f;
            }break;
            case MODEL_COMPLETE_SECROMICON:
            {
                Scale = 0.003f;
            }break;
            }
        }
        else if (Type == MODEL_PET_UNICORN)
        {
            Scale = 0.0015f;
        }
        else if (Type == MODEL_WING + 130)
        {
            Scale = 0.0012f;
        }
        else if (Type >= MODEL_POTION + 134 && Type <= MODEL_POTION + 139)
        {
            Scale = 0.0050f;
        }
        else if (Type >= MODEL_HELPER + 109 && Type <= MODEL_HELPER + 112)
        {
            Scale = 0.0045f;
        }

        else if (Type >= MODEL_HELPER + 113 && Type <= MODEL_HELPER + 115)
        {
            Scale = 0.0018f;
        }
        else if (Type >= MODEL_POTION + 112 && Type <= MODEL_POTION + 113)
        {
            Scale = 0.0032f;
        }
        else if (Type == MODEL_HELPER + 116)
        {
            Scale = 0.0021f;
        }
        else if (Type >= MODEL_POTION + 114 && Type <= MODEL_POTION + 119)
        {
            Scale = 0.0038f;
        }
        else if (Type >= MODEL_POTION + 126 && Type <= MODEL_POTION + 129)
        {
            Scale = 0.0038f;
        }
        else if (Type >= MODEL_POTION + 130 && Type <= MODEL_POTION + 132)
        {
            Scale = 0.0038f;
        }
        else if (Type == MODEL_HELPER + 121)
        {
            Scale = 0.0018f;
            //Scale = 1.f;
        }
        else if (Type == MODEL_HELPER + 124)
            Scale = 0.0018f;
        else if (Type >= MODEL_CAPE_OF_FIGHTER && Type <= MODEL_CAPE_OF_OVERRULE)
        {
            Scale = 0.002f;
        }
        else if (Type == MODEL_WING + 135)
        {
            Scale = 0.0012f;
        }
        else if (Type >= MODEL_SACRED_GLOVE && Type <= MODEL_PIERCING_BLADE_GLOVE)
        {
            Scale = 0.0035f;
        }
        else if (Type == MODEL_PHOENIX_SOUL_STAR)
        {
            Scale = 0.003f;
        }
        else if (Type >= MODEL_CHAIN_DRIVE_PARCHMENT && Type <= MODEL_INCREASE_BLOCK_PARCHMENT)
        {
            Scale = 0.0023f;
        }
        else if (Type == MODEL_ARMORINVEN_60 || Type == MODEL_ARMORINVEN_62 || Type == MODEL_ARMORINVEN_61 || Type == MODEL_ARMORINVEN_74)
        {
            b->BodyHeight = -100.f;
            Scale = 0.0039f;
        }
        // LEM_TSET  상승의 보석, 연장의 보석 스케일[lem_2010.9.7]
        else if (Type >= MODEL_HELPER + 135 && Type <= MODEL_HELPER + 145)
        {
            Scale = 0.001f;
        }
        else if (Type == MODEL_POTION + 160 || Type == MODEL_POTION + 161)
        {
            Scale = 0.001f;
        }
        else if (Type == MODEL_HELM + 62 || Type == MODEL_HELM + 63 || Type == MODEL_HELM + 65 || Type == MODEL_HELM + 70)
        {
            Scale = 0.001f;
        }
    }

    b->Animation(BoneTransform, ObjectSelect.AnimationFrame, ObjectSelect.PriorAnimationFrame, ObjectSelect.PriorAction, ObjectSelect.Angle, ObjectSelect.HeadAngle, false, false);

    CHARACTER Armor;
    OBJECT* o = &Armor.Object;
    o->Type = Type;
    ItemObjectAttribute(o);
    o->LightEnable = false;
    Armor.Class = CLASS_ELF; // ??

#ifdef PBG_ADD_ITEMRESIZE
    int ScreenPos_X = 0, ScreenPos_Y = 0;
    Projection(Position, &ScreenPos_X, &ScreenPos_Y);
#endif //PBG_ADD_ITEMRESIZE

    o->Scale = Scale;

    VectorCopy(Position, o->Position);

    vec3_t Light;
    float  alpha = o->Alpha;

    Vector(1.f, 1.f, 1.f, Light);

    RenderPartObject(o, Type, NULL, Light, alpha, ItemLevel, excellentFlags, ancientDiscriminator, true, true, true);
}

void RenderItem3D(float sx, float sy, float Width, float Height, int Type, int Level, int excellentFlags, int ancientDiscriminator, bool PickUp)
{
    bool Success = false;
    if ((g_pPickedItem == NULL || PickUp)
        && SEASON3B::CheckMouseIn(sx, sy, Width, Height))
    {
#ifdef PBG_ADD_INGAMESHOPMSGBOX
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP))
        {
            Success = true;
        }
        else
#endif //PBG_ADD_INGAMESHOPMSGBOX
        {
            if (g_pNewUISystem->CheckMouseUse() == false)
                Success = true;
        }
    }

    if (Type >= ITEM_SWORD && Type < ITEM_SWORD + MAX_ITEM_INDEX)
    {
        sx += Width * 0.8f;
        sy += Height * 0.85f;
    }
    else if (Type >= ITEM_AXE && Type < ITEM_MACE + MAX_ITEM_INDEX)
    {
        if (Type == ITEM_DIVINE_SCEPTER_OF_ARCHANGEL)
        {
            sx += Width * 0.6f;
            sy += Height * 0.5f;
        }
        else
        {
            sx += Width * 0.8f;
            sy += Height * 0.7f;
        }
    }
    else if (Type >= ITEM_SPEAR && Type < ITEM_SPEAR + MAX_ITEM_INDEX)
    {
        sx += Width * 0.6f;
        sy += Height * 0.65f;
    }
    else if (Type == ITEM_CELESTIAL_BOW)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == ITEM_GREAT_REIGN_CROSSBOW)
    {
        sx += Width * 0.7f;
        sy += Height * 0.75f;
    }
    else if (Type == ITEM_ARROW_VIPER_BOW)
    {
        sx += Width * 0.5f;
        sy += Height * 0.55f;
    }
    else if (Type >= ITEM_CROSSBOW && Type < ITEM_BOW + MAX_ITEM_INDEX)
    {
        sx += Width * 0.7f;
        sy += Height * 0.7f;
    }
    else if (Type >= ITEM_STAFF && Type < ITEM_STAFF + MAX_ITEM_INDEX)
    {
        sx += Width * 0.6f;
        sy += Height * 0.55f;
    }
    else if (Type >= ITEM_SHIELD && Type < ITEM_SHIELD + MAX_ITEM_INDEX)
    {
        sx += Width * 0.5f;
        if (Type == ITEM_GRAND_SOUL_SHIELD)
            sy += Height * 0.7f;
        else if (Type == ITEM_ELEMENTAL_SHIELD)
            sy += Height * 0.9f;
        else if (Type == ITEM_CROSS_SHIELD)
        {
            sx += Width * 0.05f;
            sy += Height * 0.5f;
        }
        else
            sy += Height * 0.6f;
    }
    else if (Type >= ITEM_HELM && Type < ITEM_HELM + MAX_ITEM_INDEX)
    {
        sx += Width * 0.5f;
        sy += Height * 0.8f;
    }
    else if (Type >= ITEM_ARMOR && Type < ITEM_ARMOR + MAX_ITEM_INDEX)
    {
        sx += Width * 0.5f;
        if (Type == ITEM_PAD_ARMOR || Type == ITEM_BONE_ARMOR || Type == ITEM_SCALE_ARMOR)
        {
            sy += Height * 1.05f;
        }
        else if (Type == ITEM_LEGENDARY_ARMOR || Type == ITEM_BRASS_ARMOR)
        {
            sy += Height * 1.1f;
        }
        else if (Type == ITEM_DARK_PHOENIX_ARMOR || Type == ITEM_GRAND_SOUL_ARMOR || Type == ITEM_THUNDER_HAWK_ARMOR)
        {
            sy += Height * 0.8f;
        }
        else if (Type == ITEM_STORM_CROW_ARMOR)
        {
            sy += Height * 1.0f;
        }
        else
        {
            sy += Height * 0.8f;
        }
    }
    else if (Type >= ITEM_PANTS && Type < ITEM_BOOTS + MAX_ITEM_INDEX)
    {
        sx += Width * 0.5f;
        sy += Height * 0.9f;
    }
    else if (Type == ITEM_LOCHS_FEATHER && Level == 1)
    {
        sx += Width * 0.55f;
        sy += Height * 0.85f;
    }
    else if (Type == ITEM_LOCHS_FEATHER || Type == ITEM_FRUITS)
    {
        sx += Width * 0.6f;
        sy += Height * 1.f;
    }
    else if (Type == ITEM_SCROLL_OF_ARCHANGEL || Type == ITEM_BLOOD_BONE)
    {
        sx += Width * 0.5f;
        sy += Height * 0.9f;
    }
    else if (Type == ITEM_INVISIBILITY_CLOAK)
    {
        sx += Width * 0.5f;
        sy += Height * 0.75f;
    }
    else if (Type == ITEM_WEAPON_OF_ARCHANGEL)
    {
        switch (Level)
        {
        case 0: sx += Width * 0.5f; sy += Height * 0.5f; break;
        case 1: sx += Width * 0.7f; sy += Height * 0.8f; break;
        case 2: sx += Width * 0.7f; sy += Height * 0.7f; break;
        }
    }
    else if (Type == ITEM_WIZARDS_RING)
    {
        switch (Level)
        {
        case 0: sx += Width * 0.5f; sy += Height * 0.65f; break;
        case 1:
        case 2:
        case 3:
            sx += Width * 0.5f; sy += Height * 0.8f; break;
        }
    }
    else if (Type == ITEM_ARMOR_OF_GUARDSMAN)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == ITEM_DARK_HORSE_ITEM)
    {
        sx += Width * 0.5f;
        sy += Height * 0.6f;
    }
    else if (Type == ITEM_CAPE_OF_LORD)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == ITEM_SPIRIT)
    {
        sx += Width * 0.5f;
        sy += Height * 0.9f;
    }
    else if (Type == ITEM_SIEGE_POTION)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == ITEM_HELPER + 7)
    {
        sx += Width * 0.5f;
        sy += Height * 0.9f;
    }
    else if (Type == ITEM_LIFE_STONE_ITEM)
    {
        switch (Level)
        {
        case 0: sx += Width * 0.5f; sy += Height * 0.8f; break;
        case 1: sx += Width * 0.5f; sy += Height * 0.5f; break;
        }
    }
    else if (Type == ITEM_SPLINTER_OF_ARMOR)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == MODEL_BLESS_OF_GUARDIAN)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == MODEL_CLAW_OF_BEAST)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == MODEL_FRAGMENT_OF_HORN)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == MODEL_BROKEN_HORN)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == MODEL_HORN_OF_FENRIR)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type >= ITEM_HELPER && Type < ITEM_HELPER + MAX_ITEM_INDEX)
    {
        sx += Width * 0.5f;
        sy += Height * 0.7f;
    }
    else if (Type == ITEM_POTION + 12)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == ITEM_BOX_OF_LUCK && (Level == 3 || Level == 13))
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == ITEM_BOX_OF_LUCK && (Level == 14 || Level == 15))
    {
        sx += Width * 0.5f;
        sy += Height * 0.8f;
    }
    else if (Type == ITEM_ALE && Level == 1)
    {
        sx += Width * 0.5f;
        sy += Height * 0.8f;
    }
    else if (Type == ITEM_DEVILS_EYE || Type == ITEM_DEVILS_KEY || Type == ITEM_DEVILS_INVITATION)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == ITEM_POTION + 21)
    {
        switch (Level)
        {
        case 0: sx += Width * 0.5f; sy += Height * 0.5f; break;
        case 1: sx += Width * 0.4f; sy += Height * 0.8f; break;
        case 2: sx += Width * 0.4f; sy += Height * 0.8f; break;
        case 3: sx += Width * 0.5f; sy += Height * 0.5f; break;
        }
    }
    else if (Type >= ITEM_JEWEL_OF_CREATION && Type < ITEM_TEAR_OF_ELF)
    {
        if (Type == ITEM_BROKEN_SWORD_DARK_STONE && Level == 1)
        {
            sx += Width * 0.5f;
            sy += Height * 0.8f;
        }
        else
        {
            sx += Width * 0.5f;
            sy += Height * 0.95f;
        }
    }
    else if (Type >= ITEM_JACK_OLANTERN_BLESSINGS && Type <= ITEM_JACK_OLANTERN_CRY)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type >= ITEM_TEAR_OF_ELF && Type < ITEM_POTION + 27)
    {
        sx += Width * 0.5f;
        sy += Height * 0.9f;
    }
    else if (Type == ITEM_JEWEL_OF_GUARDIAN)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == INDEX_COMPILED_CELE || Type == INDEX_COMPILED_SOUL)
    {
        sx += Width * 0.55f;
        sy += Height * 0.8f;
    }
    else if (Type == ITEM_WINGS_OF_SPIRITS)
    {
        sx += Width * 0.5f;
        sy += Height * 0.45f;
    }
    else if (Type == ITEM_WINGS_OF_SOUL)
    {
        sx += Width * 0.5f;
        sy += Height * 0.4f;
    }
    else if (Type == ITEM_WINGS_OF_DRAGON)
    {
        sx += Width * 0.5f;
        sy += Height * 0.75f;
    }
    else if (Type == ITEM_WINGS_OF_DARKNESS)
    {
        sx += Width * 0.5f;
        sy += Height * 0.55f;
    }
    else if (Type == ITEM_POTION + 100)
    {
        sx += Width * 0.49f;
        //sy += Height*0.28f;
        sy += Height * 0.28f;
    }
    else if (COMGEM::Check_Jewel_Com(Type) != COMGEM::NOGEM)
    {
        sx += Width * 0.55f;
        sy += Height * 0.82f;
    }
    else if (Type >= ITEM_POTION && Type < ITEM_POTION + MAX_ITEM_INDEX)
    {
        sx += Width * 0.5f;
        sy += Height * 0.95f;
    }
    else if ((Type >= ITEM_ORB_OF_RAGEFUL_BLOW && Type <= ITEM_ORB_OF_GREATER_FORTITUDE) || (Type >= ITEM_ORB_OF_FIRE_SLASH && Type <= ITEM_ORB_OF_DEATH_STAB))
    {
        sx += Width * 0.5f;
        sy += Height * 0.75f;
    }
    else if (Type == ITEM_HELPER + 66)
    {
        sx += Width * 1.5f;
        sy += Height * 1.5f;
    }
    else if (Type == ITEM_CAPE_OF_FIGHTER)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else if (Type == ITEM_CAPE_OF_OVERRULE)
    {
        sx += Width * 0.5f;
        sy += Height * 0.5f;
    }
    else
    {
        sx += Width * 0.5f;
        sy += Height * 0.6f;
    }

    if (Type >= ITEM_SACRED_GLOVE && Type <= ITEM_PHOENIX_SOUL_STAR)
    {
        sx -= Width * 0.25f;
        sy -= Height * 0.25f;
    }

    vec3_t Position;
    CreateScreenVector((int)(sx), (int)(sy), Position, false);
    //RenderObjectScreen(Type+MODEL_ITEM,Level,Option1,Position,Success,PickUp);
    if (Type == ITEM_BOX_OF_LUCK && Level == 1)	// 성탄의별
    {
        RenderObjectScreen(MODEL_EVENT + 4, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_BOX_OF_LUCK && Level == 2)
    {
        RenderObjectScreen(MODEL_EVENT + 5, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_BOX_OF_LUCK && Level == 3)
    {
        RenderObjectScreen(MODEL_EVENT + 6, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_BOX_OF_LUCK && Level == 5)
    {
        RenderObjectScreen(MODEL_EVENT + 8, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_BOX_OF_LUCK && Level == 6)
    {
        RenderObjectScreen(MODEL_EVENT + 9, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_BOX_OF_LUCK && 8 <= Level && Level <= 12)
    {
        RenderObjectScreen(MODEL_EVENT + 10, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_BOX_OF_LUCK && Level == 13)
    {
        RenderObjectScreen(MODEL_EVENT + 6, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_BOX_OF_LUCK && (Level == 14 || Level == 15))
    {
        RenderObjectScreen(MODEL_EVENT + 5, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_LOCHS_FEATHER && Level == 1)
    {
        RenderObjectScreen(MODEL_EVENT + 16, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_ALE && Level == 1)
    {
        RenderObjectScreen(MODEL_EVENT + 7, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_POTION + 21)
    {
        switch (Level)
        {
        case 1:
            RenderObjectScreen(MODEL_EVENT + 11, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 2:
            RenderObjectScreen(MODEL_EVENT + 11, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 3:
            RenderObjectScreen(Type + MODEL_ITEM, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        default:
            RenderObjectScreen(Type + MODEL_ITEM, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        }
    }
    else if (Type == ITEM_PUMPKIN_OF_LUCK)
    {
        RenderObjectScreen(MODEL_PUMPKIN_OF_LUCK, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type >= ITEM_JACK_OLANTERN_BLESSINGS && Type <= ITEM_JACK_OLANTERN_CRY)
    {
        RenderObjectScreen(MODEL_JACK_OLANTERN_BLESSINGS, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_JACK_OLANTERN_FOOD)
    {
        RenderObjectScreen(MODEL_JACK_OLANTERN_FOOD, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_JACK_OLANTERN_DRINK)
    {
        RenderObjectScreen(MODEL_JACK_OLANTERN_DRINK, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_PINK_CHOCOLATE_BOX)
    {
        switch (Level)
        {
        case 0:
            RenderObjectScreen(MODEL_PINK_CHOCOLATE_BOX, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 1:
            RenderObjectScreen(MODEL_EVENT + 21, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        }
    }
    else if (Type == ITEM_RED_CHOCOLATE_BOX)
    {
        switch (Level)
        {
        case 0:
            RenderObjectScreen(MODEL_RED_CHOCOLATE_BOX, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 1:
            RenderObjectScreen(MODEL_EVENT + 22, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        }
    }
    else if (Type == ITEM_BLUE_CHOCOLATE_BOX)
    {
        switch (Level)
        {
        case 0:
            RenderObjectScreen(MODEL_BLUE_CHOCOLATE_BOX, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 1:
            RenderObjectScreen(MODEL_EVENT + 23, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        }
    }
    else if (Type == ITEM_WEAPON_OF_ARCHANGEL)
    {
        switch (Level)
        {
        case 0:
            RenderObjectScreen(MODEL_DIVINE_STAFF_OF_ARCHANGEL, -1, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 1:
            RenderObjectScreen(MODEL_DIVINE_SWORD_OF_ARCHANGEL, -1, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 2:
            RenderObjectScreen(MODEL_DIVINE_CB_OF_ARCHANGEL, -1, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        }
    }
    else if (Type == ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR)
    {
        switch (Level)
        {
        case 0:
            RenderObjectScreen(Type + MODEL_ITEM, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 1:
            RenderObjectScreen(MODEL_EVENT + 12, -1, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        }
    }
    else if (Type == ITEM_BROKEN_SWORD_DARK_STONE)
    {
        switch (Level)
        {
        case 0:
            RenderObjectScreen(Type + MODEL_ITEM, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 1:
            RenderObjectScreen(MODEL_EVENT + 13, -1, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        }
    }
    else if (Type == ITEM_WIZARDS_RING)
    {
        switch (Level)
        {
        case 0:
            RenderObjectScreen(MODEL_EVENT + 15, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        case 1:
        case 2:
        case 3:
            RenderObjectScreen(MODEL_EVENT + 14, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
            break;
        }
    }
    else if (Type == ITEM_LIFE_STONE_ITEM && Level == 1)
    {
        RenderObjectScreen(MODEL_EVENT + 18, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_POTION + 100)
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
    else if (Type == ITEM_SACRED_ARMOR)
    {
        RenderObjectScreen(MODEL_ARMORINVEN_60, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_STORM_HARD_ARMOR)
    {
        RenderObjectScreen(MODEL_ARMORINVEN_61, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_PIERCING_ARMOR)
    {
        RenderObjectScreen(MODEL_ARMORINVEN_62, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else if (Type == ITEM_PHOENIX_SOUL_ARMOR)
    {
        RenderObjectScreen(MODEL_ARMORINVEN_74, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
    else
    {
        RenderObjectScreen(Type + MODEL_ITEM, Level, excellentFlags, ancientDiscriminator, Position, Success, PickUp);
    }
}

void InventoryColor(ITEM* p)
{
    switch (p->Color)
    {
    case 0:
        glColor3f(1.f, 1.f, 1.f);
        break;
    case 1:
        glColor3f(0.8f, 0.8f, 0.8f);
        break;
    case 2:
        glColor3f(0.6f, 0.7f, 1.f);
        break;
    case 3:
        glColor3f(1.f, 0.2f, 0.1f);
        break;
    case 4:
        glColor3f(0.5f, 1.f, 0.6f);
        break;
    case 5:
        glColor4f(0.8f, 0.7f, 0.f, 1.f);
        break;
    case 6:
        glColor4f(0.8f, 0.5f, 0.f, 1.f);
        break;
    case 7:
        glColor4f(0.8f, 0.3f, 0.3f, 1.f);
        break;
    case 8:
        glColor4f(1.0f, 0.f, 0.f, 1.f);
        break;
    case 99:
        glColor3f(1.f, 0.2f, 0.1f);
        break;
    }
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

void MoveServerDivisionInventory()
{
    if (!g_pUIManager->IsOpen(INTERFACE_SERVERDIVISION)) return;
    int x = 640 - 190;
    int y = 0;
    int Width, Height;

    if (MouseX >= (int)(x) && MouseX < (int)(x + 190) &&
        MouseY >= (int)(y) && MouseY < (int)(y + 256 + 177))
    {
        MouseOnWindow = true;
    }

    Width = 16; Height = 16; x = InventoryStartX + 25; y = 240;
    if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height && MouseLButtonPush)
    {
        g_bServerDivisionAccept ^= true;

        MouseLButtonPush = false;
        MouseLButton = false;
    }

    if (g_bServerDivisionAccept)
    {
        Width = 120; Height = 24; x = (float)InventoryStartX + 35; y = 320;
        if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height && MouseLButtonPush)
        {
            MouseLButtonPush = false;
            MouseLButton = false;
            AskYesOrNo = 4;
            OkYesOrNo = -1;

            ShowCheckBox(1, 448, MESSAGE_CHECK);
        }
    }

    Width = 120; Height = 24; x = (float)InventoryStartX + 35; y = 350;
    if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height && MouseLButtonPush)
    {
        MouseLButtonPush = false;
        MouseLButton = false;
        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;

        SocketClient->ToGameServer()->SendCloseNpcRequest();
        g_pUIManager->CloseAll();
    }

    Width = 24; Height = 24; x = InventoryStartX + 25; y = InventoryStartY + 395;
    if (MouseX >= x && MouseX < x + Width && MouseY >= y && MouseY < y + Height)
    {
        if (MouseLButtonPush)
        {
            MouseLButtonPush = false;
            MouseUpdateTime = 0;
            MouseUpdateTimeMax = 6;

            g_bEventChipDialogEnable = EVENT_NONE;

            SocketClient->ToGameServer()->SendCloseNpcRequest();
            g_pUIManager->CloseAll();
        }
    }
}

void HideKeyPad(void)
{
    g_iKeyPadEnable = 0;
}

int CheckMouseOnKeyPad(void)
{
    int Width, Height, WindowX, WindowY;
    Width = 213; Height = 2 * 5 + 6 * 40; WindowX = (640 - Width) / 2; WindowY = 60 + 40;//60 220

    int iButtonTop = 50;

    for (int i = 0; i < 11; ++i)
    {
        int xButton = i % 5;
        int yButton = i / 5;

        int xLeft = WindowX + 10 + xButton * 40;
        int yTop = WindowY + iButtonTop + yButton * 40;
        if (xLeft <= MouseX && MouseX < xLeft + 32 &&
            yTop <= MouseY && MouseY < yTop + 32)
        {
            return (i);
        }
    }
    // Ok, Cancel ( 11 - 12)
    int yTop = WindowY + iButtonTop + 2 * 40 + 5;

    for (int i = 0; i < 2; ++i)
    {
        int xLeft = WindowX + 52 + i * 78;
        if (xLeft <= MouseX && MouseX < xLeft + 70 &&
            yTop <= MouseY && MouseY < yTop + 21)
        {
            return (11 + i);
        }
    }

    return (-1);
}

bool g_bPadPushed = false;

void MovePersonalShop()
{
    if ((g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPSALE) || g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPPURCHASE)) && g_iPShopWndType == PSHOPWNDTYPE_SALE)
    {
        if (g_iPersonalShopMsgType == 1)
        {
            if (OkYesOrNo == 1)
            {
                g_iPersonalShopMsgType = 0;
                OkYesOrNo = -1;
            }
            else if (OkYesOrNo == 2)
            {
                g_iPersonalShopMsgType = 0;
                OkYesOrNo = -1;
            }
        }
        g_ptPersonalShop.x = 640 - 190 * 2;
        g_ptPersonalShop.y = 0;

        int Width = 56, Height = 24;
        int ButtonX = g_ptPersonalShop.x + 30, ButtonY = g_ptPersonalShop.y + 396;
        if (MouseX >= ButtonX && MouseX < ButtonX + Width && MouseY >= ButtonY && MouseY < ButtonY + Height && MouseLButtonPush)
        {
            MouseLButtonPush = false;
            if (!IsExistUndecidedPrice() && wcslen(g_szPersonalShopTitle) > 0)
            {
                if (g_bEnablePersonalShop)
                {
                    SocketClient->ToGameServer()->SendPlayerShopOpen(g_szPersonalShopTitle);
                    g_pUIManager->Close(INTERFACE_INVENTORY);
                }
                else
                {
                    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalshopCreateMsgBoxLayout));
                }
            }
            else
            {
                g_pSystemLogBox->AddText(GlobalText[1119], SEASON3B::TYPE_ERROR_MESSAGE);
            }
        }

        ButtonX = g_ptPersonalShop.x + 105;
        if (MouseX >= ButtonX && MouseX < ButtonX + Width && MouseY >= ButtonY && MouseY < ButtonY + Height && MouseLButtonPush)
        {
            MouseLButtonPush = false;
            if (g_bEnablePersonalShop)
            {
                SocketClient->ToGameServer()->SendPlayerShopClose();
            }
        }

        Width = 150;
        ButtonX = g_ptPersonalShop.x + 20;
        ButtonY = g_ptPersonalShop.y + 65;
        if (MouseX >= ButtonX && MouseX < ButtonX + Width && MouseY >= ButtonY && MouseY < ButtonY + Height && MouseLButtonPush)
        {
            OpenPersonalShopMsgWnd(1);
        }
    }
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
            SocketClient->ToGameServer()->SendPlayerShopCloseOther(g_PersonalShopSeller.Key, g_PersonalShopSeller.ID);
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

void OpenPersonalShopMsgWnd(int iMsgType)
{
    if (iMsgType == 1)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalShopNameMsgBoxLayout));
    }
    else if (iMsgType == 2)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalShopItemValueMsgBoxLayout));
    }
}
bool IsCorrectShopTitle(const wchar_t* szShopTitle)
{
    int j = 0;
    wchar_t TmpText[2048];
    for (int i = 0; i < (int)wcslen(szShopTitle); ++i)
    {
        if (szShopTitle[i] != 32)
        {
            TmpText[j] = szShopTitle[i];
            j++;
        }
    }
    TmpText[j] = 0;

    for (int i = 0; i < AbuseFilterNumber; i++)
    {
        if (FindText(TmpText, AbuseFilter[i]))
        {
            return false;
        }
    }

    int len = wcslen(szShopTitle);
    int count = 0;

    for (int i = 0; i < len; i++)
    {
        if (szShopTitle[i] == 0x20) {
            count++;
            if (i == 1 && count >= 2) return false;
        }
        else {
            count = 0;
        }
    }
    if (count >= 2)
        return false;
    return true;
}

extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;

void RenderInventoryInterface(int StartX, int StartY, int Flag)
{
    float x, y, Width, Height;
    Width = 190.f; Height = 256.f; x = (float)StartX; y = (float)StartY;

    RenderBitmap(BITMAP_INVENTORY, x, y, Width, Height, 0.f, 0.f, Width / 256.f, Height / 256.f);

    Width = 190.f; Height = 177.f; x = (float)StartX; y = (float)StartY + 256;
    RenderBitmap(BITMAP_INVENTORY + 1, x, y, Width, Height, 0.f, 0.f, Width / 256.f, Height / 256.f);

    if (Flag)
    {
        Width = 190.f; Height = 10.f; x = (float)StartX; y = (float)StartY + 225;
        RenderBitmap(BITMAP_INVENTORY + 19, x, y, Width, Height, 0.f, 0.f, Width / 256.f, Height / 16.f);
    }
}

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

void CreateGuildMark(int nMarkIndex, bool blend)
{
    BITMAP_t* b = &Bitmaps[BITMAP_GUILD];
    int Width, Height;
    Width = (int)b->Width;
    Height = (int)b->Height;
    BYTE* Buffer = b->Buffer;
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
        case 1:MarkColor[i] = (255 << 24) + (0 << 16) + (0 << 8) + (0); break;
        case 2:MarkColor[i] = (255 << 24) + (128 << 16) + (128 << 8) + (128); break;
        case 3:MarkColor[i] = (255 << 24) + (255 << 16) + (255 << 8) + (255); break;
        case 4:MarkColor[i] = (255 << 24) + (0 << 16) + (0 << 8) + (255); break;
        case 5:MarkColor[i] = (255 << 24) + (0 << 16) + (128 << 8) + (255); break;
        case 6:MarkColor[i] = (255 << 24) + (0 << 16) + (255 << 8) + (255); break;
        case 7:MarkColor[i] = (255 << 24) + (0 << 16) + (255 << 8) + (128); break;
        case 8:MarkColor[i] = (255 << 24) + (0 << 16) + (255 << 8) + (0); break;
        case 9:MarkColor[i] = (255 << 24) + (128 << 16) + (255 << 8) + (0); break;
        case 10:MarkColor[i] = (255 << 24) + (255 << 16) + (255 << 8) + (0); break;
        case 11:MarkColor[i] = (255 << 24) + (255 << 16) + (128 << 8) + (0); break;
        case 12:MarkColor[i] = (255 << 24) + (255 << 16) + (0 << 8) + (0); break;
        case 13:MarkColor[i] = (255 << 24) + (255 << 16) + (0 << 8) + (128); break;
        case 14:MarkColor[i] = (255 << 24) + (255 << 16) + (0 << 8) + (255); break;
        case 15:MarkColor[i] = (255 << 24) + (128 << 16) + (0 << 8) + (255); break;
        }
    }
    BYTE* MarkBuffer = GuildMark[nMarkIndex].Mark;

    for (int i = 0; i < Height; i++)
    {
        for (int j = 0; j < Width; j++)
        {
            *((unsigned int*)(Buffer)) = MarkColor[MarkBuffer[0]];
            Buffer += 4;
            MarkBuffer++;
        }
    }

    glBindTexture(GL_TEXTURE_2D, b->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D, 0, b->Components, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, b->Buffer);
}

void CreateCastleMark(int Type, BYTE* buffer, bool blend)
{
    if (buffer == NULL) return;

    BITMAP_t* b = &Bitmaps[Type];

    int Width, Height;

    Width = (int)b->Width;
    Height = (int)b->Height;
    BYTE* Buffer = b->Buffer;

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
        case 1:MarkColor[i] = (255 << 24) + (0 << 16) + (0 << 8) + (0); break;
        case 2:MarkColor[i] = (255 << 24) + (128 << 16) + (128 << 8) + (128); break;
        case 3:MarkColor[i] = (255 << 24) + (255 << 16) + (255 << 8) + (255); break;
        case 4:MarkColor[i] = (255 << 24) + (0 << 16) + (0 << 8) + (255); break;//빨
        case 5:MarkColor[i] = (255 << 24) + (0 << 16) + (128 << 8) + (255); break;//
        case 6:MarkColor[i] = (255 << 24) + (0 << 16) + (255 << 8) + (255); break;//노
        case 7:MarkColor[i] = (255 << 24) + (0 << 16) + (255 << 8) + (128); break;//
        case 8:MarkColor[i] = (255 << 24) + (0 << 16) + (255 << 8) + (0); break;//초
        case 9:MarkColor[i] = (255 << 24) + (128 << 16) + (255 << 8) + (0); break;//
        case 10:MarkColor[i] = (255 << 24) + (255 << 16) + (255 << 8) + (0); break;//청
        case 11:MarkColor[i] = (255 << 24) + (255 << 16) + (128 << 8) + (0); break;//
        case 12:MarkColor[i] = (255 << 24) + (255 << 16) + (0 << 8) + (0); break;//파
        case 13:MarkColor[i] = (255 << 24) + (255 << 16) + (0 << 8) + (128); break;//
        case 14:MarkColor[i] = (255 << 24) + (255 << 16) + (0 << 8) + (255); break;//보
        case 15:MarkColor[i] = (255 << 24) + (128 << 16) + (0 << 8) + (255); break;//
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
                *((unsigned int*)(Buffer + offset)) = MarkColor[MarkBuffer[offset2]];
                offset2++;
            }
            else if (j<3 || j>(Width - 4) || i<10 || i>(Height - 10))
            {
                *((unsigned int*)(Buffer + offset)) = (255 << 24) + (0 << 16) + ((int)(50 + i / 100.f * 160) << 8) + (50 + i / 100.f * 255);
            }
            else
            {
                *((unsigned int*)(Buffer + offset)) = (255 << 24) + (i << 16) + (i << 8) + (i);
            }
            offset += 4;
        }
    }
    glBindTexture(GL_TEXTURE_2D, b->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, b->Buffer);
}

void RenderGuildColor(float x, float y, int SizeX, int SizeY, int Index)
{
    RenderBitmap(BITMAP_INVENTORY + 18, x - 1, y - 1, (float)SizeX + 2, (float)SizeY + 2, 0.f, 0.f, SizeX / 32.f, SizeY / 30.f);

    BITMAP_t* b = &Bitmaps[BITMAP_GUILD];

    int Width, Height;

    Width = (int)b->Width;
    Height = (int)b->Height;
    BYTE* Buffer = b->Buffer;
    unsigned int Color = MarkColor[Index];

    if (Index == 0)
    {
        for (int i = 0; i < Height; i++)
        {
            for (int j = 0; j < Width; j++)
            {
                *((unsigned int*)(Buffer)) = 255 << 24;
                Buffer += 4;
            }
        }
        Color = (255 << 24) + (128 << 16) + (128 << 8) + (128);
        Buffer = b->Buffer;
        for (int i = 0; i < 8; i++)
        {
            *((unsigned int*)(Buffer)) = Color;
            Buffer += 8 * 4 + 4;
        }
        Buffer = b->Buffer + 7 * 4;
        for (int i = 0; i < 8; i++)
        {
            *((unsigned int*)(Buffer)) = Color;
            Buffer += 8 * 4 - 4;
        }
    }
    else
    {
        for (int i = 0; i < Height; i++)
        {
            for (int j = 0; j < Width; j++)
            {
                *((unsigned int*)(Buffer)) = Color;
                Buffer += 4;
            }
        }
    }

    glBindTexture(GL_TEXTURE_2D, b->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D, 0, b->Components, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, b->Buffer);
    RenderBitmap(BITMAP_GUILD, x, y, (float)SizeX, (float)SizeY);
}

void RenderGuildList(int StartX, int StartY)
{
    GuildListStartX = StartX;
    GuildListStartY = StartY;

    glColor3f(1.f, 1.f, 1.f);

    DisableAlphaBlend();
    float x, y, Width, Height;
    Width = 190.f; Height = 256.f; x = (float)StartX; y = (float)StartY;
    RenderBitmap(BITMAP_INVENTORY, x, y, Width, Height, 0.f, 0.f, Width / 256.f, Height / 256.f);
    Width = 190.f; Height = 177.f; x = (float)StartX; y = (float)StartY + 256;
    RenderBitmap(BITMAP_INVENTORY + 1, x, y, Width, Height, 0.f, 0.f, Width / 256.f, Height / 256.f);

    EnableAlphaTest();

    g_pRenderText->SetBgColor(20, 20, 20, 255);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetFont(g_hFontBold);

    wchar_t Text[100];
    if (Hero->GuildMarkIndex == -1)
        swprintf(Text, GlobalText[180]);
    else
        swprintf(Text, L"%s (Score:%d)", GuildMark[Hero->GuildMarkIndex].GuildName, GuildTotalScore);

    g_pRenderText->RenderText(StartX + 95 - 60, StartY + 12, Text, 120 * WindowWidth / 640, true, 3);

    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
    g_pRenderText->SetFont(g_hFont);

    if (g_nGuildMemberCount == 0)
    {
        g_pRenderText->RenderText(StartX + 20, StartY + 50, GlobalText[185]);
        g_pRenderText->RenderText(StartX + 20, StartY + 65, GlobalText[186]);
        g_pRenderText->RenderText(StartX + 20, StartY + 80, GlobalText[187]);
    }
    g_pRenderText->SetBgColor(0, 0, 0, 128);
    g_pRenderText->SetTextColor(100, 255, 200, 255);
    g_pRenderText->RenderText(StartX + (int)Width / 2, StartY + 44, g_GuildNotice[0], 0, 0, RT3_WRITE_CENTER);
    g_pRenderText->RenderText(StartX + (int)Width / 2, StartY + 58, g_GuildNotice[1], 0, 0, RT3_WRITE_CENTER);

    int yGuildStart = 72;
    int Number = g_nGuildMemberCount;

    if (g_nGuildMemberCount >= MAX_GUILD_LINE)
        Number = MAX_GUILD_LINE;
}

//#define MAX_LENGTH_CMB	( 26)
#define NUM_LINE_CMB	( 7)

void RenderServerDivision()
{
    if (!g_pUIManager->IsOpen(INTERFACE_SERVERDIVISION)) return;

    float Width, Height, x, y;

    glColor3f(1.f, 1.f, 1.f);
    EnableAlphaTest();

    InventoryStartX = 640 - 190;
    InventoryStartY = 0;
    Width = 213; Height = 40; x = (float)InventoryStartX; y = (float)InventoryStartY;
    RenderInventoryInterface((int)x, (int)y, 1);

    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 230, 210, 255);

    g_pRenderText->SetFont(g_hFontBold);
    x = InventoryStartX + (190 / 2.f);
    y = 50;
    for (int i = 462; i < 470; ++i)
    {
        g_pRenderText->RenderText(x, y, GlobalText[i], 0, 0, RT3_WRITE_CENTER);
        y += 20;
    }

    g_pRenderText->SetFont(g_hFontBold);
    Width = 16; Height = 16; x = (float)InventoryStartX + 25; y = 240;
    if (g_bServerDivisionAccept)
    {
        g_pRenderText->SetTextColor(212, 150, 0, 255);
        RenderBitmap(BITMAP_INVENTORY_BUTTON + 11, x, y, Width, Height, 0.f, 0.f, 24 / 32.f, 24 / 32.f);
    }
    else
    {
        g_pRenderText->SetTextColor(223, 191, 103, 255);
        RenderBitmap(BITMAP_INVENTORY_BUTTON + 10, x, y, Width, Height, 0.f, 0.f, 24 / 32.f, 24 / 32.f);
    }
    g_pRenderText->RenderText((int)(x + Width + 3), (int)(y + 5), GlobalText[447]);
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 230, 210, 255);

    Width = 120; Height = 24; x = (float)InventoryStartX + 35; y = 350;//(Width/2.f); y = 231;
    RenderBitmap(BITMAP_INTERFACE + 10, (float)x, (float)y, (float)Width, (float)Height, 0.f, 0.f, 213.f / 256.f);
    g_pRenderText->RenderText((int)(x + (Width / 2)), (int)(y + 5), GlobalText[229], 0, 0, RT3_WRITE_CENTER);

    Width = 120; Height = 24; x = (float)InventoryStartX + 35; y = 320;//(Width/2.f); y = 231;
    if (g_bServerDivisionAccept)
        glColor3f(1.f, 1.f, 1.f);
    else
        glColor3f(0.5f, 0.5f, 0.5f);
    RenderBitmap(BITMAP_INTERFACE + 10, (float)x, (float)y, (float)Width, (float)Height, 0.f, 0.f, 213.f / 256.f);
    g_pRenderText->RenderText((int)(x + (Width / 2)), (int)(y + 5), GlobalText[228], 0, 0, RT3_WRITE_CENTER);

    glColor3f(1.f, 1.f, 1.f);
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