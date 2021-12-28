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
#include "wsclientinline.h"
#include "ZzzScene.h"
#include "./Utilities/Log/ErrorReport.h"
#include "CSQuest.h"
#include "Local.h"
#include "zzzMixInventory.h"
#include "PersonalShopTitleImp.h"
#include "GOBoid.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "GMHellas.h"
#include "npcBreeder.h"
#include "GIPetManager.h"
#include "CSParts.h"
#include "npcGateSwitch.h"
#include "GMBattleCastle.h"
#include "CComGem.h"
#include "GMCrywolf1st.h"
#include "CDirection.h"
#include "ChangeRingManager.h"
#include "MixMgr.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "NewUIInventoryCtrl.h"
#include "GM3rdChangeUp.h"
#include "w_CursedTemple.h"
#include "SocketSystem.h"
#include "PortalMgr.h"
#include "NewUISystem.h"
#include "ServerListManager.h"
#include <time.h>
#include "MapManager.h"
#include "CharacterManager.h"
#include "SkillManager.h"


extern CUITextInputBox * g_pSingleTextInputBox;
extern int g_iChatInputType;
extern CUIGuildListBox * g_pGuildListBox;

int			g_nTaxRate			= 0;
int			g_nChaosTaxRate		= 0;

char         g_GuildNotice[3][128];
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

ITEM Inventory         [MAX_INVENTORY];
ITEM ShopInventory     [MAX_SHOP_INVENTORY];
ITEM g_PersonalShopInven	[MAX_PERSONALSHOP_INVEN];
ITEM g_PersonalShopBackup	[MAX_PERSONALSHOP_INVEN];
bool g_bEnablePersonalShop = false;
int g_iPShopWndType = PSHOPWNDTYPE_NONE;
POINT g_ptPersonalShop = { 0, 0 };
int g_iPersonalShopMsgType = 0;
char g_szPersonalShopTitle[64] = { 0, };

CHARACTER g_PersonalShopSeller;
bool g_bIsTooltipOn = false;

int   CheckSkill = -1;
ITEM  *CheckInventory = NULL;

bool  EquipmentSuccess = false;
bool  CheckShop;
int   CheckX;
int   CheckY;
ITEM  *SrcInventory;
int   SrcInventoryIndex;
int   DstInventoryIndex;

int  AllRepairGold              = 0;
int  StorageGoldFlag            = 0;

int ListCount                = 0;
int GuildListPage            = 0;

int  g_bEventChipDialogEnable  = EVENT_NONE;
int  g_shEventChipCount        = 0;
short g_shMutoNumber[3]        = { -1, -1, -1 };

bool g_bServerDivisionAccept   = false;
char g_strGiftName[64];

bool RepairShop                = false;
int  RepairEnable              = 0;
int AskYesOrNo                  = 0; 

char OkYesOrNo                  = -1;

int g_iKeyPadEnable = 0;
WORD g_wStoragePassword = 0;
short	g_nKeyPadMapping[10];
char	g_lpszKeyPadInput[2][MAX_KEYPADINPUT + 1];

BYTE g_byItemUseType = 0;

static  const int DEFAULT_DEVILSQUARELEVEL[6][2] = { {15, 130}, { 131, 180}, { 181, 230}, {231, 280}, { 281, 330}, { 331, 99999} };
static  const int DARKCLASS_DEVILSQUARELEVEL[6][2] = { {15, 110}, { 111, 160}, { 161, 210}, {211, 260}, { 261, 310}, { 311, 99999} };
int g_iDevilSquareLevel[6][2];

static  const int g_iChaosCastleLevel[12][2] = {  
						{ 15, 49 }, { 50, 119 }, { 120, 179 }, { 180, 239 }, { 240, 299 }, { 300, 999 }, 
                        { 15, 29 }, { 30,  99 }, { 100, 159 }, { 160, 219 }, { 220, 279 }, { 280, 999 }  
												};
static  const int g_iChaosCastleZen[6] = {  25, 80, 150, 250, 400, 650 };

BYTE BuyItem[4];

static  int iStateNum = 4;

extern bool Teleport;

extern float g_fScreenRate_x;	// ※
extern float g_fScreenRate_y;

#ifdef _PVP_ADD_MOVE_SCROLL
extern CMurdererMove g_MurdererMove;
#endif	// _PVP_ADD_MOVE_SCROLL

int getLevelGeneration ( int level, unsigned int* color )
{
    int lvl;
    if(level>=300)
    {
        lvl = 300;
	    *color = (255<<24)+(255<<16)+(153<<8)+(255);
    }
    else if(level>=200)
    {
        lvl = 200;
	    *color = (255<<24)+(255<<16)+(230<<8)+(210);
    }
    else if(level>=100)
    {
        lvl = 100;
        *color = (255<<24)+(24<<16)+(201<<8)+(0);
    }
    else if(level>=50)
    {
        lvl = 50;
        *color = (255<<24)+(0<<16)+(150<<8)+(255);
    }
    else
    {
        lvl = 10;
        *color = (255<<24)+(0<<16)+(0<<8)+(255);
    }
    return lvl;
}

char TextList[30][100];
int  TextListColor[30];
int  TextBold[30];
SIZE Size[30];

int RenderTextList(int sx,int sy,int TextNum,int Tab, int iSort = RT3_SORT_CENTER)
{
	int TextWidth = 0;
	float fsy = sy;
	for(int i=0;i<TextNum;i++)
	{
     	if(TextBold[i])
		{
			g_pRenderText->SetFont(g_hFontBold);
		}
		else
		{
			g_pRenderText->SetFont(g_hFont);
		}

		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i],lstrlen(TextList[i]),&Size[i]);
      	
		if(TextWidth < Size[i].cx)
		{
			TextWidth = Size[i].cx;
		}
	}

	if(Tab == 0)
	{
		sx -= (TextWidth+Tab)*640/WindowWidth/2;
	}

	for(int i=0;i<TextNum;i++)
	{
		g_pRenderText->SetTextColor(0xffffffff);
		
		switch(TextListColor[i])
		{
		case TEXT_COLOR_WHITE:
		case TEXT_COLOR_DARKRED:
        case TEXT_COLOR_DARKBLUE:
        case TEXT_COLOR_DARKYELLOW:
         	glColor3f(1.f,1.f,1.f);
			break;
		case TEXT_COLOR_BLUE:
            glColor3f(0.5f,0.7f,1.f);
			break;
        case TEXT_COLOR_GRAY:
			glColor3f(0.4f,0.4f,0.4f);
            break;
		case TEXT_COLOR_GREEN_BLUE:
			glColor3f(1.f,1.f,1.f);
			break;
		case TEXT_COLOR_RED:
            glColor3f(1.f,0.2f,0.1f);
			break;
		case TEXT_COLOR_YELLOW:
            glColor3f(1.f,0.8f,0.1f);
			break;
		case TEXT_COLOR_GREEN:
            glColor3f(0.1f,1.f,0.5f);
			break;
        case TEXT_COLOR_PURPLE:
            glColor3f(1.f,0.1f,1.f);
            break;
		}
		if ( TEXT_COLOR_DARKRED == TextListColor[i])
		{
			g_pRenderText->SetBgColor(160, 0, 0, 255);
		}
		else if ( TEXT_COLOR_DARKBLUE==TextListColor[i])
		{
			g_pRenderText->SetBgColor(0, 0, 160, 255);
		}
		else if ( TEXT_COLOR_DARKYELLOW==TextListColor[i])
		{
			g_pRenderText->SetBgColor(160, 102, 0, 255);
		}
        else if ( TEXT_COLOR_GREEN_BLUE==TextListColor[i])
        {
			g_pRenderText->SetTextColor(0, 255, 0, 255);
			g_pRenderText->SetBgColor(60, 60, 200, 255);
        }
		else
		{
			g_pRenderText->SetBgColor(0, 0, 0, 0);
		}

     	if(TextBold[i])
		{
			g_pRenderText->SetFont(g_hFontBold);
		}
		else
		{
			g_pRenderText->SetFont(g_hFont);
		}
		SIZE TextSize;
		g_pRenderText->RenderText(sx, fsy, TextList[i], TextWidth+Tab, 0, iSort, &TextSize);
		fsy += TextSize.cy;
	}
	return TextWidth+Tab;
}

void RenderTipTextList(const int sx, const int sy, int TextNum, int Tab, int iSort, int iRenderPoint, BOOL bUseBG)
{
	SIZE TextSize = {0, 0};
	int TextLine = 0; int EmptyLine = 0;
	float fWidth = 0; float fHeight = 0;
	for(int i = 0; i < TextNum; ++i)
	{
		if (TextList[i][0] == '\0')
		{
			TextNum = i;
			break;
		}

		if(TextBold[i])
		{
			g_pRenderText->SetFont(g_hFontBold);
		}
		else
		{
			g_pRenderText->SetFont(g_hFont);
		}

		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);
		
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
	if(iPos_x < 0) iPos_x = 0;
	if(iPos_x + fWidth > (int)WindowWidth / g_fScreenRate_x)
	{
		iPos_x = ((int)WindowWidth) / g_fScreenRate_x - fWidth - 1;
	}
	
	float fsx = iPos_x + 1;
	float fsy = 0.f;
	
	switch( iRenderPoint )
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
		RenderColor ((float)iPos_x-1, fsy - 1, (float)fWidth + 1, (float)1);
		RenderColor ((float)iPos_x-1, fsy - 1, (float)1, (float)fHeight + 1);
		RenderColor ((float)iPos_x-1 + fWidth + 1, (float)fsy - 1, (float)1, (float)fHeight + 1);	
		RenderColor ((float)iPos_x-1, fsy - 1 + fHeight + 1, (float)fWidth + 2, (float)1);

		glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
		RenderColor ((float)iPos_x, fsy, (float)fWidth, (float)fHeight);
		glEnable(GL_TEXTURE_2D);
	}

	for(int i=0;i<TextNum;i++)
	{
     	if(TextBold[i])
		{
			g_pRenderText->SetFont(g_hFontBold);
		}
		else
		{
			g_pRenderText->SetFont(g_hFont);
		}
		
		float fHeight = 0;
		if(TextList[i][0] == 0x0a || (TextList[i][0] == ' ' && TextList[i][1] == 0x00))
		{
			g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);
			fHeight = (float)TextSize.cy / g_fScreenRate_y / (TextList[i][0] == 0x0a ? 2.0f : 1.0f);
		}
		else
		{
		g_pRenderText->SetTextColor(0xffffffff);
			switch(TextListColor[i])
			{
			case TEXT_COLOR_WHITE:
			case TEXT_COLOR_DARKRED:
            case TEXT_COLOR_DARKBLUE:
            case TEXT_COLOR_DARKYELLOW:
         		glColor3f(1.f,1.f,1.f);
				break;
			case TEXT_COLOR_BLUE:
				glColor3f(0.5f,0.7f,1.f);
				break;
            case TEXT_COLOR_GRAY:
				glColor3f(0.4f,0.4f,0.4f);
                break;
            case TEXT_COLOR_GREEN_BLUE:
				glColor3f(1.f,1.f,1.f);
				break;
            case TEXT_COLOR_RED:
				glColor3f(1.f,0.2f,0.1f);
				break;
			case TEXT_COLOR_YELLOW:
				glColor3f(1.f,0.8f,0.1f);
				break;
			case TEXT_COLOR_GREEN:
				glColor3f(0.1f,1.f,0.5f);
				break;
            case TEXT_COLOR_PURPLE:
                glColor3f(1.f,0.1f,1.f);
                break;
			case TEXT_COLOR_REDPURPLE:
				glColor3f(0.8f,0.5f,0.8f);
				break;
			case TEXT_COLOR_VIOLET:
				glColor3f(0.7f,0.4f,1.0f);
				break;
			case TEXT_COLOR_ORANGE:
				glColor3f(0.9f, 0.42f, 0.04f );
				break;
			}
			if ( TEXT_COLOR_DARKRED == TextListColor[i])
			{
				g_pRenderText->SetBgColor(160, 0, 0, 255);
			}
		    else if ( TEXT_COLOR_DARKBLUE==TextListColor[i])
		    {
				g_pRenderText->SetBgColor(0, 0, 160, 255);
		    }
			else if ( TEXT_COLOR_DARKYELLOW==TextListColor[i])
			{
				g_pRenderText->SetBgColor(160, 102, 0, 255);
			}
            else if ( TEXT_COLOR_GREEN_BLUE==TextListColor[i])
            {
				g_pRenderText->SetBgColor(60, 60, 200, 255);
				g_pRenderText->SetTextColor(0, 255, 0, 255);
            }
			else
			{
				g_pRenderText->SetBgColor(0);
			}
			SIZE TextSize;
			g_pRenderText->RenderText(fsx,fsy,TextList[i],(fWidth-2),0,iSort, &TextSize);
			fHeight = TextSize.cy;
		}
		fsy += fHeight * 1.1f;
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	DisableAlphaBlend();
}

bool IsCanUseItem()
{
	if(	g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE) || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE))
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
	if( g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPSALE) || g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPPURCHASE) )
	{
		return false;
	}
	return true;
}

int TextNum = 0;
int SkipNum = 0;

bool IsRequireClassRenderItem(const short sType)
{
	if(
		sType == ITEM_HELPER+19
		|| sType == ITEM_HELPER+29
		|| sType == ITEM_WING+26
		|| (sType >= ITEM_WING+30 && sType <= ITEM_WING+31)
		|| (sType >= ITEM_HELPER+43 && sType <= ITEM_HELPER+45)
		|| sType == ITEM_HELPER+10
		|| (sType >= ITEM_HELPER+39 && sType <= ITEM_HELPER+42)
		|| sType == ITEM_HELPER+37
		|| sType == ITEM_WING+15
		|| sType == ITEM_WING+32
		|| sType == ITEM_WING+33
		|| sType == ITEM_WING+34
		)
	{
		return false;
	}

	if((sType >= ITEM_HELPER+43 && sType <= ITEM_HELPER+45)
		|| (sType >= ITEM_HELPER+46 && sType <= ITEM_HELPER+48)
		|| (sType >= ITEM_HELPER+125 && sType <= ITEM_HELPER+127)
		|| (sType == ITEM_POTION+54)
		|| (sType >= ITEM_POTION+58 && sType <= ITEM_POTION+62)
		|| (sType == ITEM_POTION+53)
		|| (sType >= ITEM_POTION+70 && sType <= ITEM_POTION+71)
		|| (sType >= ITEM_POTION+72 && sType <= ITEM_POTION+77)
		|| (sType >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN 
			&& sType <= ITEM_TYPE_CHARM_MIXWING+EWS_END)
		|| (sType == ITEM_HELPER+59)
		|| ( sType >= ITEM_HELPER+54 && sType <= ITEM_HELPER+58)
		||(sType >= ITEM_POTION+78 && sType <= ITEM_POTION+82)
		|| (sType == ITEM_HELPER+60)
		|| (sType == ITEM_HELPER+61)
		|| (sType == ITEM_POTION+91)
		|| (sType == ITEM_POTION+94)
		|| (sType >= ITEM_POTION+92 && sType <= ITEM_POTION+93)
		|| (sType == ITEM_POTION+95)
		|| ( sType >= ITEM_HELPER+62 && sType <= ITEM_HELPER+63 )
		|| ( sType >= ITEM_POTION+97 && sType <= ITEM_POTION+98 )
		|| (sType == ITEM_POTION+96)
		|| ( sType == ITEM_HELPER+64 || sType == ITEM_HELPER+65 )
		|| ( sType == ITEM_HELPER+67 )
		|| ( sType == ITEM_HELPER+68 )
		|| ( sType == ITEM_HELPER+76 )
		|| ( sType == ITEM_HELPER+122 )
		|| ( sType == ITEM_HELPER+69 )
		|| ( sType == ITEM_HELPER+70 )
		|| ( sType == ITEM_HELPER+71 || sType == ITEM_HELPER+72 || sType == ITEM_HELPER+73 || sType == ITEM_HELPER+74 ||sType == ITEM_HELPER+75 )
		|| ( sType == ITEM_HELPER+80 )
		|| ( sType == ITEM_HELPER+106 )
		|| sType == ITEM_HELPER+81
		|| sType == ITEM_HELPER+82
		|| sType == ITEM_HELPER+93
		|| sType == ITEM_HELPER+94
		|| sType == ITEM_HELPER+121
		|| (sType >= ITEM_POTION+145 && sType <= ITEM_POTION+150)
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(sType)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (sType==ITEM_POTION+133)
		)
	{
		return false;
	}	

	return true;
}

void RequireClass(ITEM_ATTRIBUTE* pItem)
{
	if(pItem == NULL)
		return;

	BYTE byFirstClass = gCharacterManager.GetBaseClass(Hero->Class);
	BYTE byStepClass = gCharacterManager.GetStepClass(Hero->Class);

	int iTextColor = 0;

	TextListColor[TextNum + 2] = TextListColor[TextNum + 3] = TEXT_COLOR_WHITE;
	sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

	int iCount = 0;
	for(int i=0; i<MAX_CLASS; ++i)
	{
		if(pItem->RequireClass[i] == 1)
		{
			iCount++;
		}
	}
	if(iCount == MAX_CLASS)
		return;

	for(int i=0; i<MAX_CLASS; ++i)
	{
		BYTE byRequireClass = pItem->RequireClass[i];

		if(byRequireClass == 0)
			continue;

		if(i == byFirstClass && byRequireClass <= byStepClass)
		{
			iTextColor = TEXT_COLOR_WHITE;
		}
		else
		{
			iTextColor = TEXT_COLOR_DARKRED;
		}

		switch(i)
		{
		case CLASS_WIZARD:
			{
				if(byRequireClass == 1)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[20]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 2)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[25]);
					TextListColor[TextNum] = iTextColor;	
				}
				else if(byRequireClass == 3)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1669]);
					TextListColor[TextNum] = iTextColor;	
				}

				TextBold[TextNum] = false;	TextNum++;
			}
			break;
		case CLASS_KNIGHT:
			{
				if(byRequireClass == 1)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[21]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 2)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[26]);
					TextListColor[TextNum] = iTextColor;	
				}
				else if(byRequireClass == 3)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1668]);
					TextListColor[TextNum] = iTextColor;	
				}

				TextBold[TextNum] = false;	TextNum++;
			}
			break;
		case CLASS_ELF:
			{	
				if(byRequireClass == 1)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[22]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 2)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[27]);
					TextListColor[TextNum] = iTextColor;	
				}
				else if(byRequireClass == 3)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1670]);
					TextListColor[TextNum] = iTextColor;	
				}

				TextBold[TextNum] = false;	TextNum++;
			}
			break;
		case CLASS_DARK:
			{	
				if(byRequireClass == 1)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[23]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 3)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1671]);
					TextListColor[TextNum] = iTextColor;	
				}

				TextBold[TextNum] = false;	TextNum++;
			}
			break;	
		case CLASS_DARK_LORD:
			{	
				if(byRequireClass == 1)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[24]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 3)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1672]);
					TextListColor[TextNum] = iTextColor;	
				}

				TextBold[TextNum] = false;	TextNum++;
			}
			break;
		case CLASS_SUMMONER:
			{	
				if(byRequireClass == 1)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1687]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 2)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1688]);
					TextListColor[TextNum] = iTextColor;	
				}
				else if(byRequireClass == 3)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1689]);
					TextListColor[TextNum] = iTextColor;	
				}

				TextBold[TextNum] = false;	TextNum++;
			}
			break;
#ifdef PBG_ADD_NEWCHAR_MONK
		case CLASS_RAGEFIGHTER:
			{
				if(byRequireClass==1)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[3150]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass==3)
				{
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[3151]);
					TextListColor[TextNum] = iTextColor;	
				}
				TextBold[TextNum] = false;	TextNum++;
			}
			break;
#endif //PBG_ADD_NEWCHAR_MONK
		}
	}
}

const int iMaxLevel = 15;
const int iMaxColumn = 17;
int g_iCurrentItem = -1;
int g_iItemInfo[iMaxLevel + 1][iMaxColumn];

void RenderHelpLine(int iColumnType, const char * pPrintStyle, int & TabSpace, const char * pGapText, int Pos_y, int iType)
{
    int iCurrMaxLevel = iMaxLevel;

    if ( iType==5 )
        iCurrMaxLevel = 0;

	for (int Level = 0; Level <= iCurrMaxLevel; ++Level)
	{
		sprintf(TextList[TextNum], pPrintStyle, g_iItemInfo[Level][iColumnType]);
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
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[TextNum - 1], lstrlen(TextList[TextNum - 1]), &TextSize);
	}
	else 
	{
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), pGapText, lstrlen(pGapText), &TextSize);
	}
	TabSpace += int(TextSize.cx / g_fScreenRate_x);
	if(iType == 6)
	{
		TabSpace += 5;	
	}
	TextNum -= iCurrMaxLevel + 1;
}

void RenderHelpCategory(int iColumnType, int Pos_x, int Pos_y)
{
	const char* pText = NULL;

	switch(iColumnType)
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
	sprintf(TextList[TextNum], pText);
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

	ITEM_ATTRIBUTE *p = &ItemAttribute[ItemHelp];

	for(int Level = 0; Level <= iMaxLevel; Level++)
	{
		int RequireStrength = 0;
		int RequireDexterity = 0;
		int RequireEnergy = 0;
		int RequireCharisma = 0;
		int RequireVitality = 0;
		int RequireLevel = 0;
		int DamageMin = p->DamageMin;
		int DamageMax = p->DamageMax;
		int Defense   = p->Defense;
		int Magic	  = p->MagicPower;
		int Blocking  = p->SuccessfulBlocking;

		if(DamageMin > 0)
        {
            DamageMin += (min(9,Level)*3);
			switch(Level - 9)
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
		if(DamageMax > 0)
        {
            DamageMax += (min(9,Level)*3);
			switch(Level - 9)
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

		if(Magic > 0)
		{
			Magic += (min(9,Level)*3);	// ~ +9
			switch(Level - 9)
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

			if(IsCepterItem(ItemHelp) == false)
            {
    			Magic += Level*2;
			}
		}

        if(Defense > 0)
		{
			if(ItemHelp>=ITEM_SHIELD && ItemHelp<ITEM_SHIELD+MAX_ITEM_INDEX)
			{
				Defense += Level;
			}
			else
            {
                Defense     += (min(9,Level)*3);	// ~ +9
				switch(Level - 9)
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
		if(Blocking  > 0)
		{
            Blocking     += (min(9,Level)*3);	// ~ +9
			switch(Level - 9)
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

		if(p->RequireLevel)
		{
			RequireLevel     = p->RequireLevel;
		}
		else
		{
			RequireLevel  = 0;
		}

		if(p->RequireStrength)
		{
			RequireStrength  = 20+p->RequireStrength *(p->Level+Level*3)*3/100;
		}
		else
		{
			RequireStrength  = 0;
		}

		if(p->RequireDexterity)	RequireDexterity = 20+p->RequireDexterity*(p->Level+Level*3)*3/100;
		else RequireDexterity = 0;

		if(p->RequireVitality)	RequireVitality  = 20+p->RequireVitality *(p->Level+Level*3)*3/100;
		else RequireVitality  = 0;

		if(p->RequireEnergy)
		{
			if (ItemHelp >= ITEM_STAFF+21 && ItemHelp <= ITEM_STAFF+29)
			{
				RequireEnergy    = 20+p->RequireEnergy   *(p->Level+Level*1)*3/100;
			}
			else
			{
				if((p->RequireLevel > 0) && (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC + MAX_ITEM_INDEX))
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
			RequireEnergy    = 0;
		}

		if(p->RequireCharisma)	RequireCharisma  = 20+p->RequireCharisma *(p->Level+Level*3)*3/100;
		else RequireCharisma  = 0;

		g_iItemInfo[Level][_COLUMN_TYPE_LEVEL] = Level;
		g_iItemInfo[Level][_COLUMN_TYPE_ATTMIN] = DamageMin;
		g_iItemInfo[Level][_COLUMN_TYPE_ATTMAX] = DamageMax;
		
		if (ItemHelp >= ITEM_STAFF+21 && ItemHelp <= ITEM_STAFF+29)
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

		if ( IsCepterItem(ItemHelp) == true )

        {
    		g_iItemInfo[Level][_COLUMN_TYPE_MAGIC] = 0;
        }
        else
        {
    		g_iItemInfo[Level][_COLUMN_TYPE_PET_ATTACK] = 0;
        }

		WORD Strength, Dexterity, Energy, Vitality, Charisma;

		Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
		Dexterity= CharacterAttribute->Dexterity+ CharacterAttribute->AddDexterity;
		Energy	 = CharacterAttribute->Energy   + CharacterAttribute->AddEnergy;
		Vitality	 = CharacterAttribute->Vitality   + CharacterAttribute->AddVitality;
		Charisma	 = CharacterAttribute->Charisma   + CharacterAttribute->AddCharisma;

		if(RequireStrength  <=Strength
			&&	RequireDexterity<=Dexterity
			&&	RequireEnergy   <=Energy
			&&	RequireVitality <= Vitality
			&&	RequireCharisma <= Charisma
			&&	RequireLevel <= CharacterAttribute->Level )
			g_iItemInfo[Level][_COLUMN_TYPE_CAN_EQUIP] = TRUE;
		else
			g_iItemInfo[Level][_COLUMN_TYPE_CAN_EQUIP] = FALSE;
	}
}

unsigned int getGoldColor ( DWORD Gold )
{
    if ( Gold>=10000000 )
    {
        return  (255<<24)+(0<<16)+(0<<8)+(255);
    }
    else if ( Gold>=1000000 )
    {
        return  (255<<24)+(0<<16)+(150<<8)+(255);
    }
    else if( Gold>=100000 )
    {
        return  (255<<24)+(24<<16)+(201<<8)+(0);
    }

    return  (255<<24)+(150<<16)+(220<<8)+(255);
}

void ConvertGold(double dGold, unicode::t_char* szText, int iDecimals /*= 0*/)
{
	unicode::t_char szTemp[256];
	int iCipherCnt=0;
	DWORD dwValueTemp = (DWORD)dGold;
	
	while( dwValueTemp/1000 > 0 )
	{
		iCipherCnt = iCipherCnt + 3;
		dwValueTemp = dwValueTemp/1000;
	}

	unicode::_sprintf(szText, "%d", dwValueTemp);

	while( iCipherCnt > 0 )
	{
		dwValueTemp = (DWORD)dGold;
		dwValueTemp = (dwValueTemp%(int)pow(10.f,(float)iCipherCnt))/(int)pow(10.f,(float)(iCipherCnt-3));
		unicode::_sprintf(szTemp, ",%03d", dwValueTemp);
		strcat(szText, szTemp);
		iCipherCnt = iCipherCnt - 3;
	}

	if( iDecimals > 0 )
	{
		dwValueTemp = (int)(dGold*pow(10.f,(float)iDecimals))%(int)pow(10.f, (float)iDecimals);
		unicode::_sprintf(szTemp, ".%d", dwValueTemp);
		strcat(szText, szTemp);
	}
}

void ConvertGold64(__int64 Gold,unicode::t_char* Text)
{
	int Gold1 = Gold%1000;
	int Gold2 = Gold%1000000/1000;
	int Gold3 = Gold%1000000000/1000000;
	int Gold4 = Gold%1000000000000/1000000000;
	int Gold5 = Gold%1000000000000000/1000000000000;
	int Gold6 = Gold/1000000000000000;
	if(Gold >= 1000000000000000)
		unicode::_sprintf(Text,"%d,%03d,%03d,%03d,%03d,%03d",Gold6,Gold5,Gold4,Gold3,Gold2,Gold1);
	else if(Gold >= 1000000000000)
		unicode::_sprintf(Text,"%d,%03d,%03d,%03d,%03d",Gold5,Gold4,Gold3,Gold2,Gold1);
	else if(Gold >= 1000000000)
		unicode::_sprintf(Text,"%d,%03d,%03d,%03d",Gold4,Gold3,Gold2,Gold1);
	else if(Gold >= 1000000)
		unicode::_sprintf(Text,"%d,%03d,%03d",Gold3,Gold2,Gold1);
	else if(Gold >= 1000)
		unicode::_sprintf(Text,"%d,%03d",Gold2,Gold1);
	else 
		unicode::_sprintf(Text,"%d",Gold1);
}

void ConvertTaxGold(DWORD Gold,char *Text)
{
	Gold += ((LONGLONG)Gold * g_pNPCShop->GetTaxRate()) / 100;

	int Gold1 = Gold%1000;
	int Gold2 = Gold%1000000/1000;
	int Gold3 = Gold%1000000000/1000000;
	int Gold4 = Gold/1000000000;
	if(Gold >= 1000000000)
       	sprintf(Text,"%d,%03d,%03d,%03d",Gold4,Gold3,Gold2,Gold1);
	else if(Gold >= 1000000)
       	sprintf(Text,"%d,%03d,%03d",Gold3,Gold2,Gold1);
	else if(Gold >= 1000)
       	sprintf(Text,"%d,%03d",Gold2,Gold1);
	else 
       	sprintf(Text,"%d",Gold1);
}

void ConvertChaosTaxGold(DWORD Gold,char *Text)
{
	Gold += ((LONGLONG)Gold*g_nChaosTaxRate)/100;

	int Gold1 = Gold%1000;
	int Gold2 = Gold%1000000/1000;
	int Gold3 = Gold%1000000000/1000000;
	int Gold4 = Gold/1000000000;
	if(Gold >= 1000000000)
       	sprintf(Text,"%d,%03d,%03d,%03d",Gold4,Gold3,Gold2,Gold1);
	else if(Gold >= 1000000)
       	sprintf(Text,"%d,%03d,%03d",Gold3,Gold2,Gold1);
	else if(Gold >= 1000)
       	sprintf(Text,"%d,%03d",Gold2,Gold1);
	else 
       	sprintf(Text,"%d",Gold1);
}

int ConvertRepairGold(int Gold,int Durability, int MaxDurability, short Type, char *Text)
{
	Gold = min(Gold,400000000);

	float   repairGold = (float)Gold;
	float   persent = 1.f - (float)( Durability / (float)MaxDurability );
	bool    doubleP = false;
	
	if ( persent > 0 )
	{
		float fRoot = ( float)sqrt( ( double)repairGold);
		float fRootRoot = ( float)sqrt( sqrt( ( double)repairGold));
		repairGold = 3.f * fRoot * fRootRoot;

		if(doubleP)
		{
			repairGold *= 2;
		}
		
		repairGold *= persent;
		repairGold++;

		if ( Durability <= 0 )
		{
			if(Type == ITEM_HELPER+4 ||  Type == ITEM_HELPER+5)
			{
				repairGold *= 2;
			}
			else
			{
				repairGold += repairGold*0.4f;
			}
		}
	}
	else
	{
		repairGold = 0;
	}
	
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true && g_pNPCShop->IsRepairShop())
	{
		Gold = (int)(repairGold);
	}
	else if((g_pMyInventory->IsVisible()) && (!g_pNPCShop->IsVisible()))
	{
		Gold = (int)(repairGold + (repairGold * 1.5f));
	}	
	else
	{
		Gold = (int)(repairGold);
	}
	
	if(Gold >= 1000)
	{
		Gold = (Gold / 100) * 100;
	}
	else if(Gold >= 100)
	{
		Gold = (Gold / 10) * 10;
	}
	
	ConvertGold(Gold, Text);
	
	return  Gold;
}

void RepairAllGold ( void )
{
    char    text[100];

    AllRepairGold = 0;

	for( int i=0; i<MAX_EQUIPMENT; ++i )
	{
		if( CharacterMachine->Equipment[i].Type != -1)
        {
            ITEM* ip = &CharacterMachine->Equipment[i];
	        ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];

			int Level = (ip->Level>>3)&15;
            int maxDurability = calcMaxDurability(ip, p, Level);

			if(IsRepairBan(ip) == true)
			{
				continue;
			}

			//. check durability
            if( ip->Durability<maxDurability )
            {
                int gold = ConvertRepairGold(ItemValue(ip,2),ip->Durability, maxDurability, ip->Type, text);

#ifdef LEM_ADD_LUCKYITEM
				if( Check_LuckyItem( ip->Type ) )	gold = 0;
#endif // LEM_ADD_LUCKYITEM
                AllRepairGold += gold;
            }
        }
	}

	ITEM * pItem = NULL;
	for(int i = 0; i < (int)(g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems()); ++i)
	{
		pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(i);

		if(pItem)
		{
			ITEM_ATTRIBUTE* p = &ItemAttribute[pItem->Type];
			
			int Level = (pItem->Level>>3)&15;
			int maxDurability = calcMaxDurability( pItem, p, Level );
			
			if(pItem->Type >= ITEM_POTION+55 && pItem->Type <= ITEM_POTION+57)
			{
				continue;
			}
			//. item filtering
			if( (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HELPER+5) || pItem->Type == ITEM_HELPER+10  || pItem->Type == ITEM_HELPER+31)
				continue;
			if( pItem->Type == ITEM_BOW+7 || pItem->Type == ITEM_BOW+15 || pItem->Type >= ITEM_POTION )
				continue;
			if( pItem->Type >= ITEM_WING+7 && pItem->Type <= ITEM_WING+19 )
				continue;
			if( (pItem->Type >= ITEM_HELPER+14 && pItem->Type <= ITEM_HELPER+19) || pItem->Type==ITEM_POTION+21 )
				continue;
			if( pItem->Type == ITEM_HELPER+20)
				continue;
			if( pItem->Type == ITEM_HELPER+38 )
				continue;

			if(pItem->Type >= ITEM_HELPER+46 && pItem->Type <= ITEM_HELPER+48)
			{
				continue;
			}
			if(pItem->Type >= ITEM_HELPER+125 && pItem->Type <= ITEM_HELPER+127)
			{
				continue;
			}
			if(pItem->Type >= ITEM_POTION+145 && pItem->Type <= ITEM_POTION+150)
			{
				continue;
			}
			if(pItem->Type >= ITEM_POTION+58 && pItem->Type <= ITEM_POTION+62)
			{
				continue;
			}
			if( pItem->Type == ITEM_POTION+53 )
			{
				continue;
			}
			if( pItem->Type == ITEM_HELPER+43 || pItem->Type == ITEM_HELPER+44 || pItem->Type == ITEM_HELPER+45 )
			{
				continue;
			}
			if( pItem->Type >= ITEM_POTION+70 && pItem->Type <= ITEM_POTION+71 )
			{
				continue;
			}
			if( pItem->Type >= ITEM_POTION+72 && pItem->Type <= ITEM_POTION+77 )
			{
				continue;
			}
			if( pItem->Type == ITEM_HELPER+59 )
			{
				continue;
			}
			if( pItem->Type >= ITEM_HELPER+54 && pItem->Type <= ITEM_HELPER+58 )
			{
				continue;
			}
			if( pItem->Type == ITEM_HELPER+60 )
			{
				continue;
			}
			if( pItem->Type == ITEM_HELPER+61 )
			{
				continue;
			}
			if( pItem->Type == ITEM_POTION+91 )
			{
				continue;
			}
			if( pItem->Type >= ITEM_POTION+92 && pItem->Type <= ITEM_POTION+93 )
			{
				continue;
			}
			if( pItem->Type == ITEM_POTION+95 )
			{
				continue;
			}
			if( pItem->Type == ITEM_POTION+95 )
			{
				continue;
			}
			if( pItem->Type >= ITEM_HELPER+62 && pItem->Type <= ITEM_HELPER+63 )
			{
				continue;
			}
			if( pItem->Type >= ITEM_POTION+97 && pItem->Type <= ITEM_POTION+98 )
			{
				continue;
			}
			if (pItem->Type == ITEM_POTION+140)
			{
				continue;
			}
			if( pItem->Type == ITEM_POTION+96 )
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+64 || pItem->Type == ITEM_HELPER+65 ) 
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+67 ) 
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+80 ) 
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+106 ) 
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+123 )
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+68 ) 
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+76 ) 
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+122 )
			{
				continue;
			}
			if ( pItem->Type == ITEM_HELPER+69 )
				continue;
			if ( pItem->Type == ITEM_HELPER+70 )
				continue;

			if(pItem->Type == ITEM_HELPER+37)
				continue;

			if(pItem->Type == ITEM_HELPER+66)
				continue;

			if(	pItem->Type == ITEM_HELPER+71 
				|| pItem->Type == ITEM_HELPER+72 
				|| pItem->Type == ITEM_HELPER+73
				|| pItem->Type == ITEM_HELPER+74
				|| pItem->Type == ITEM_HELPER+75 )
				continue;

			if (pItem->Type == ITEM_HELPER+81)
				continue;
			if (pItem->Type == ITEM_HELPER+82)
				continue;
			if (pItem->Type == ITEM_HELPER+93)
				continue;
			if (pItem->Type == ITEM_HELPER+94)
				continue;

			if( pItem->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN && pItem->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END )
			{
				continue;
			}
			if(pItem->Type == ITEM_HELPER+97 || pItem->Type == ITEM_HELPER+98 || pItem->Type == ITEM_POTION+91)
				continue;

			if(pItem->Type == ITEM_HELPER+121)
				continue;

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			if(g_pMyInventory->IsInvenItem(pItem->Type))
				continue;

#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

			if( pItem->Type >= ITEM_WING+130 && pItem->Type <= ITEM_WING+134 )
				continue;
			if( pItem->Type == ITEM_HELPER+109 )
				continue;
			if( pItem->Type == ITEM_HELPER+110 )
				continue;
			if( pItem->Type == ITEM_HELPER+111 )
				continue;
			if( pItem->Type == ITEM_HELPER+112 )
				continue;
			if( pItem->Type == ITEM_HELPER+113 )
				continue;
			if( pItem->Type == ITEM_HELPER+114 )
				continue;
			if( pItem->Type == ITEM_HELPER+115 )
				continue;
			if( pItem->Type == ITEM_HELPER+107 )
				continue;

#ifdef LEM_ADD_LUCKYITEM
		if( Check_ItemAction( pItem, eITEM_REPAIR ) )	continue;
#endif // LEM_ADD_LUCKYITEM

			//. check durability
			if( pItem->Durability<maxDurability )
			{
				int gold = ConvertRepairGold(ItemValue(pItem,2),pItem->Durability, maxDurability, pItem->Type, text);
#ifdef LEM_ADD_LUCKYITEM
				if( Check_LuckyItem( pItem->Type ) )	gold = 0;
#endif // LEM_ADD_LUCKYITEM
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
int SommonTable[]={2,7,14,8,9,41};

char ChaosEventName[][100] = {
	"히돼� 고향 여행권",
	"펜티엄4 컴퓨터",
	"디지탈카메라",
	"로지텍 무선 마우스+키보드 세트",
	"256M 램",
    "6개� 잡지 구독권",
	"문화상품권(만원)",
	"뮤 머그컵",
	"뮤 T셔츠",
	"뮤 10시간 무료이용권"
};

WORD calcMaxDurability ( const ITEM* ip, ITEM_ATTRIBUTE *p, int Level )
{
    WORD maxDurability = p->Durability;
	
    if( ip->Type>=ITEM_STAFF && ip->Type<ITEM_STAFF+MAX_ITEM_INDEX )
    {
        maxDurability = p->MagicDur;
    }
    for( int i=0; i<Level; i++ )
    {
		if (ip->Type>=ITEM_HELPER+51)
		{
			break;
		}
		else
		if(i>=14)
		{
			maxDurability = (maxDurability+8 >= 255 ? 255 : maxDurability+8);
		}
		else if(i>=13)	// 14
		{
			maxDurability = (maxDurability+7 >= 255 ? 255 : maxDurability+7);
		}
		else
		if(i>=12)	// 13
		{
			maxDurability+=6;
		}
		else if(i>=11)	// 12
		{
			maxDurability+=5;
		}
		else 
		if(i>=10)	// 11
		{
			maxDurability+=4;
		}
        else if(i>=9)	// 10
		{
			maxDurability+=3;
		}
        else if(i>=4)	// 5~9
        {
            maxDurability+=2;
        }
        else	// 1~4
        {
            maxDurability++;
        }
    }

	if( ip->Type==ITEM_HELPER+4 || ip->Type== ITEM_HELPER+5)
	{
		maxDurability = 255;
	}

    if ( (ip->ExtOption%0x04)==EXT_A_SET_OPTION || (ip->ExtOption%0x04)==EXT_B_SET_OPTION )
    {
        maxDurability+=20;
    }
    else if( ( ip->Option1&63)>0 && 
		( ip->Type<ITEM_WING+3 || ip->Type>ITEM_WING+6 ) &&
		( ip->Type!=ITEM_SWORD+19 && ip->Type!=ITEM_BOW+18 && ip->Type!=ITEM_STAFF+10 )
        && ip->Type!=ITEM_HELPER+30
		&& ( ip->Type<ITEM_WING+36 || ip->Type>ITEM_WING+40 )
		&& (ip->Type<ITEM_WING+42 || ip->Type>ITEM_WING+43)
		&& ip->Type!=ITEM_MACE+13
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		&& !(ip->Type >= ITEM_WING+49 && ip->Type <= ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
	  )
    {
        maxDurability += 15;
    }
#ifdef LEM_ADD_LUCKYITEM
	if( Check_LuckyItem(ip->Type) )
	{
		maxDurability	= 255;
	}
#endif // LEM_ADD_LUCKYITEM

    return  maxDurability;
}

void GetItemName ( int iType, int iLevel, char* Text )
{
	ITEM_ATTRIBUTE *p = &ItemAttribute[iType];

    if (iType>=ITEM_POTION+23 && iType<=ITEM_POTION+26 )
    {
		if(iType == ITEM_POTION+23)
		{
			switch( iLevel )
			{
			case 0: sprintf ( Text, "%s", p->Name ); break;
			case 1: sprintf ( Text, "%s", GlobalText[906] ); break;
			}
		}
		else if(iType == ITEM_POTION+24)
		{
			switch( iLevel )
			{
			case 0: sprintf ( Text, "%s", p->Name ); break;
			case 1: sprintf ( Text, "%s", GlobalText[907] ); break;
			}
		}
		else
		{
			sprintf ( Text, "%s", p->Name );
		}
    }
	else if(iType == ITEM_POTION+12)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", GlobalText[100] ); break;
		case 1:	sprintf ( Text, "%s", GlobalText[101] ); break;
		case 2:	sprintf ( Text, "%s", ChaosEventName[p->Durability] ); break;
		}
	}
	else if(iType == ITEM_POTION+11)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
		case 1: sprintf ( Text, "%s", GlobalText[105] ); break;
		case 2: sprintf ( Text, "%s", GlobalText[106] ); break;
		case 3: sprintf ( Text, "%s", GlobalText[107] ); break;
		case 5: sprintf ( Text, "%s", GlobalText[109] ); break;
		case 6: sprintf ( Text, "%s", GlobalText[110] ); break;
        case 7: sprintf ( Text, "%s", GlobalText[111] ); break;
			break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			sprintf(Text,"%s +%d",GlobalText[115], iLevel - 7);
			break;
		case 13:
			sprintf(Text, GlobalText[117]); break;
        case 14:
			sprintf(Text, GlobalText[1650]); break;
            break;

        case 15:
			sprintf(Text, GlobalText[1651]); break;
            break;
		}
	}
    else if ( iType==ITEM_HELPER+15 )
    {
        switch ( iLevel )
        {
        case 0:sprintf(Text,"%s %s", GlobalText[168], p->Name ); break;
        case 1:sprintf(Text,"%s %s", GlobalText[169], p->Name ); break;
        case 2:sprintf(Text,"%s %s", GlobalText[167], p->Name ); break;
        case 3:sprintf(Text,"%s %s", GlobalText[166], p->Name ); break;
		case 4:sprintf(Text,"%s %s", GlobalText[1900], p->Name ); break;
        }
    }
    else if ( iType==ITEM_HELPER+14 )
    {
        switch ( iLevel )
        {
        case 0: sprintf ( Text, "%s", p->Name ); break;
        case 1: sprintf ( Text, "%s", GlobalText[1235] ); break;
        }
    }
    else if ( iType==ITEM_HELPER+31 )
    {
        switch ( iLevel )
        {
        case 0: sprintf ( Text, "%s %s", GlobalText[1187], p->Name ); break;
        case 1: sprintf ( Text, "%s %s", GlobalText[1214], p->Name ); break;
        }
    }
    else if ( iType==ITEM_POTION+21 )
    {
        switch ( iLevel )
        {
        case 0: sprintf ( Text, "%s", p->Name ); break;
        case 1: sprintf ( Text, "%s", GlobalText[810] ); break;
        case 2: sprintf ( Text, "%s", GlobalText[1098] ); break;
        case 3: sprintf ( Text, "%s", GlobalText[1290] ); break;
        }
    }
    else if ( iType==ITEM_HELPER+19 )
    {
        sprintf ( Text, "%s", GlobalText[809] );
    }
	else if ( iType == ITEM_HELPER+20 )
	{
		switch(iLevel)
		{
		case 0: sprintf( Text, "%s", p->Name ); break;
		case 1: sprintf( Text, "%s", GlobalText[922] ); break;
        case 2: sprintf( Text, "%s", GlobalText[928] ); break;
        case 3: sprintf( Text, "%s", GlobalText[929] ); break;
		}
	}
	else if(iType == ITEM_POTION+9)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
		case 1:	sprintf ( Text, "%s", GlobalText[108] ); break;
		}
	}
    else if(iType == ITEM_WING+11)
	{
		sprintf(Text,"%s %s",SkillAttribute[30+iLevel].Name,GlobalText[102]);
	}
	else if(iType == ITEM_WING+32)
	{
		sprintf(Text, "%s", p->Name);	
	}
	else if(iType == ITEM_WING+33)
	{
		sprintf(Text, "%s", p->Name);	
	}
	else if(iType == ITEM_WING+34)
	{
		sprintf(Text, "%s", p->Name);	
	}
	else if(iType == ITEM_WING+35)
	{
		sprintf(Text, "%s", p->Name);	
	}
	else if(iType >= ITEM_POTION+45 && iType <= ITEM_POTION+50)
	{
 		sprintf ( Text, "%s", p->Name ); 
	}
	else if(iType == ITEM_POTION+32)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
 		case 1:	sprintf ( Text, "%s", GlobalText[2012] ); break;
		}
	}
	else if(iType == ITEM_POTION+33)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
		case 1:	sprintf ( Text, "%s", GlobalText[2013] ); break;
		}
	}
	else if(iType == ITEM_POTION+34)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
		case 1:	sprintf ( Text, "%s", GlobalText[2014] ); break;
		}
	}
    else if(iType == ITEM_HELPER+10)
	{
		for(int i=0;i<MAX_MONSTER;i++)
		{
			if(SommonTable[iLevel] == MonsterScript[i].Type)
			{
        		sprintf(Text,"%s %s",MonsterScript[i].Name,GlobalText[103]);
			}
		}
	}
    else if ( iType>=ITEM_WING+3 && iType<=ITEM_WING+6 )
    {
		if(iLevel==0)
			sprintf(Text,"%s",p->Name);
		else
			sprintf(Text,"%s +%d",p->Name,iLevel);
    }
	else if ((iType>=ITEM_WING+36 && iType<=ITEM_WING+40) || (iType>=ITEM_WING+42 && iType<=ITEM_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (iType == ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
    {
		if(iLevel==0)
			sprintf(Text,"%s",p->Name);
		else
			sprintf(Text,"%s +%d",p->Name,iLevel);
    }
	else if ( iType==ITEM_SWORD+19 || iType==ITEM_BOW+18 || iType==ITEM_STAFF+10 || iType==ITEM_MACE+13)
	{
		if(iLevel==0)
			sprintf(Text,"%s",p->Name);
		else
			sprintf(Text,"%s +%d",p->Name,iLevel);
	}
	else if ( COMGEM::NOGEM != COMGEM::Check_Jewel_Com(iType) )
	{
 		sprintf(Text,"%s +%d", p->Name, iLevel+1);
	}
	else if ( iType==INDEX_COMPILED_CELE)
	{
		sprintf(Text,"%s +%d", GlobalText[1806], iLevel+1);
	}
	else if ( iType==INDEX_COMPILED_SOUL)
	{
		sprintf(Text,"%s +%d", GlobalText[1807], iLevel+1);
	}
	else if ((iType >= ITEM_WING+60 && iType <= ITEM_WING+65)
		|| (iType >= ITEM_WING+70 && iType <= ITEM_WING+74)
		|| (iType >= ITEM_WING+100 && iType <= ITEM_WING+129))
	{
		sprintf(Text,"%s",p->Name);
	}
    else if ( iType == ITEM_POTION+7 )
    {
		int iTextIndex = 0;
		iTextIndex = (iLevel == 0) ? 1413 : 1414;
        sprintf ( Text, "%s", GlobalText[iTextIndex] );
    }
	else
	{
		if(iLevel==0)
			sprintf(Text,"%s",p->Name);
		else
			sprintf(Text,"%s +%d",p->Name,iLevel);
	}
}

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
void GetSpecialOptionText ( int Type, char* Text, WORD Option, BYTE Value, int iMana )
#else //PBG_ADD_NEWCHAR_MONK_SKILL
void GetSpecialOptionText ( int Type, char* Text, BYTE Option, BYTE Value, int iMana )
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
{
    switch(Option)
    {
    case AT_SKILL_BLOCKING:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[80], iMana);
        break;
    case AT_SKILL_SWORD1:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[81], iMana);
        break;
    case AT_SKILL_SWORD2:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[82], iMana);
        break;
    case AT_SKILL_SWORD3:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[83], iMana);
        break;
    case AT_SKILL_SWORD4:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[84], iMana);
        break;
    case AT_SKILL_SWORD5:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[85], iMana);
        break;
	case AT_SKILL_MANY_ARROW_UP:
	case AT_SKILL_MANY_ARROW_UP+1:
	case AT_SKILL_MANY_ARROW_UP+2:
	case AT_SKILL_MANY_ARROW_UP+3:
	case AT_SKILL_MANY_ARROW_UP+4:

    case AT_SKILL_CROSSBOW:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[86], iMana);
        break;
    case AT_SKILL_BLAST_CROSSBOW4:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[920], iMana);
        break;
	case AT_SKILL_MULTI_SHOT:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[920], iMana);
		break;
	case AT_SKILL_RECOVER:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[920], iMana);
		break;
	case AT_SKILL_POWER_SLASH_UP:
	case AT_SKILL_POWER_SLASH_UP+1:
	case AT_SKILL_POWER_SLASH_UP+2:
	case AT_SKILL_POWER_SLASH_UP+3:
	case AT_SKILL_POWER_SLASH_UP+4:
    case AT_SKILL_ICE_BLADE:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[98], iMana);
        break;
    case AT_LUCK:
        sprintf(Text,GlobalText[87]);
        break;
    case AT_IMPROVE_DAMAGE:
        sprintf(Text,GlobalText[88],Value);
        break;
    case AT_IMPROVE_MAGIC:
		sprintf(Text,GlobalText[89],Value);
        break;
	case AT_IMPROVE_CURSE:
		sprintf(Text,GlobalText[1697],Value);
		break;
    case AT_IMPROVE_BLOCKING:
        sprintf(Text,GlobalText[90],Value);
        break;
    case AT_IMPROVE_DEFENSE:
        sprintf(Text,GlobalText[91],Value);
        break;
    case AT_LIFE_REGENERATION:
        if ( !( ITEM_HELPER+14 <= Type && Type <= ITEM_HELPER+18))
        {
            sprintf(Text,GlobalText[92],Value);
        }
        break;
    case AT_IMPROVE_LIFE:
        sprintf(Text,GlobalText[622]);
        break;
    case AT_IMPROVE_MANA:
        sprintf(Text,GlobalText[623]);
        break;
    case AT_DECREASE_DAMAGE:
        sprintf(Text,GlobalText[624]);
        break;

    case AT_REFLECTION_DAMAGE:
        sprintf(Text,GlobalText[625]);
        break;
    case AT_IMPROVE_BLOCKING_PERCENT:
        sprintf(Text,GlobalText[626]);
        break;
    case AT_IMPROVE_GAIN_GOLD:
        sprintf(Text,GlobalText[627]);
        break;
    case AT_EXCELLENT_DAMAGE:
        sprintf(Text,GlobalText[628]);
        break;
    case AT_IMPROVE_DAMAGE_LEVEL:
        sprintf(Text,GlobalText[629]);
        break;
    case AT_IMPROVE_DAMAGE_PERCENT:
        sprintf(Text,GlobalText[630], Value);
        break;
    case AT_IMPROVE_MAGIC_LEVEL:
        sprintf(Text,GlobalText[631]);
        break;
    case AT_IMPROVE_MAGIC_PERCENT:
        sprintf(Text,GlobalText[632], Value);
        break;
    case AT_IMPROVE_ATTACK_SPEED:
        sprintf(Text,GlobalText[633], Value);
        break;
    case AT_IMPROVE_GAIN_LIFE:
        sprintf(Text,GlobalText[634]);
        break;
    case AT_IMPROVE_GAIN_MANA:
        sprintf(Text,GlobalText[635]);
        break;
    case AT_IMPROVE_EVADE :
        sprintf(Text, GlobalText[746] );
        break;
    case AT_SKILL_RIDER :
       gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text, GlobalText[745], iMana  );
        break;
    case AT_SKILL_STRONG_PIER:  //  
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf ( Text, GlobalText[1210], iMana );
        break;
    case AT_SKILL_LONG_SPEAR:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf ( Text, GlobalText[1186], iMana );
        break;
	case AT_SKILL_ASHAKE_UP:
	case AT_SKILL_ASHAKE_UP+1:
	case AT_SKILL_ASHAKE_UP+2:
	case AT_SKILL_ASHAKE_UP+3:
	case AT_SKILL_ASHAKE_UP+4:
    case AT_SKILL_DARK_HORSE:
        gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf ( Text, GlobalText[1189], iMana );
        break;
	case AT_SKILL_PLASMA_STORM_FENRIR:
		gSkillManager.GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf ( Text, GlobalText[1928], iMana );
		break;
    case AT_SET_OPTION_IMPROVE_DEFENCE :
        sprintf ( Text, GlobalText[959], Value );
        break;
    case AT_SET_OPTION_IMPROVE_CHARISMA :
        sprintf ( Text, GlobalText[954], Value );
        break;
    case AT_SET_OPTION_IMPROVE_DAMAGE :
        sprintf ( Text, GlobalText[577], Value );
        break;
    case AT_IMPROVE_HP_MAX :
        sprintf(Text, GlobalText[740], Value);
        break;
    case AT_IMPROVE_MP_MAX :
        sprintf(Text, GlobalText[741], Value);
        break;
    case AT_ONE_PERCENT_DAMAGE :
        sprintf(Text, GlobalText[742], Value);
        break;
    case AT_IMPROVE_AG_MAX :
        sprintf(Text, GlobalText[743], Value);
        break;
    case AT_DAMAGE_ABSORB :
        sprintf(Text, GlobalText[744], Value);
        break;
    case AT_SET_OPTION_IMPROVE_STRENGTH :
        sprintf(Text, GlobalText[985], Value );
        break;
    case AT_SET_OPTION_IMPROVE_DEXTERITY :
        sprintf(Text, GlobalText[986], Value );
        break;
    case AT_SET_OPTION_IMPROVE_VITALITY :
        sprintf(Text, GlobalText[987], Value );
        break;
    case AT_SET_OPTION_IMPROVE_ENERGY :
        sprintf(Text, GlobalText[988], Value );
        break;
    case AT_IMPROVE_MAX_MANA :
        sprintf(Text, GlobalText[1087], Value );
        break;
    case AT_IMPROVE_MAX_AG :
        sprintf(Text, GlobalText[1088], Value );
        break;
	case AT_DAMAGE_REFLECTION:
		sprintf(Text, GlobalText[1673], Value );
		break;
	case AT_RECOVER_FULL_LIFE:
		sprintf(Text, GlobalText[1674], Value );
		break;
	case AT_RECOVER_FULL_MANA:
		sprintf(Text, GlobalText[1675], Value );
		break;
	case AT_SKILL_SUMMON_EXPLOSION:
		sprintf(Text, GlobalText[1695], iMana );
		break;
	case AT_SKILL_SUMMON_REQUIEM:
		sprintf(Text, GlobalText[1696], iMana );
		break;
	case AT_SKILL_SUMMON_POLLUTION:
		sprintf(Text, GlobalText[1789], iMana );
		break;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case AT_SKILL_THRUST:
		GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[3153], iMana);
		break;
	case AT_SKILL_STAMP:
		GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[3154], iMana);
		break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
    }
}

void RenderItemInfo(int sx,int sy,ITEM *ip,bool Sell, int Inventype, bool bItemTextListBoxUse)
{
	if (ip->Type == -1)
		return;

	tm* ExpireTime;
	if( ip->bPeriodItem == true && ip->bExpiredPeriod == false)
	{
		_tzset();
		if( ip->lExpireTime == 0 )
			return;

		ExpireTime = localtime((time_t*)&(ip->lExpireTime));
	}
	
	ITEM_ATTRIBUTE *p = &ItemAttribute[ip->Type];
	TextNum = 0;
	SkipNum = 0;

	ZeroMemory( TextListColor, 20 * sizeof ( int));
	for(int i=0;i<30;i++)
	{
		TextList[i][0] = NULL;
	}


	if ( ip->Type==ITEM_HELPER+4 || ip->Type==ITEM_HELPER+5 ) 
	{
		BYTE PetType = PET_TYPE_DARK_SPIRIT;
		if(ip->Type==ITEM_HELPER+4)
		{
			PetType = PET_TYPE_DARK_HORSE;

			if((g_pMyInventory->GetPointedItemIndex()) == EQUIPMENT_HELPER)
				SendRequestPetInfo(PetType, Inventype, EQUIPMENT_HELPER);
		}
		else
			if((g_pMyInventory->GetPointedItemIndex()) == EQUIPMENT_WEAPON_LEFT)
			SendRequestPetInfo(PetType, Inventype, EQUIPMENT_WEAPON_LEFT);

		giPetManager::RenderPetItemInfo( sx, sy, ip, Inventype );
		return;
	}


	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	int Level = (ip->Level>>3)&15;
	int Color;

	if(ip->Type==ITEM_POTION+13 || ip->Type==ITEM_POTION+14 || ip->Type==ITEM_WING+15 || ip->Type==ITEM_POTION+31 ||
		(COMGEM::isCompiledGem(ip)) ||
		ip->Type == ITEM_POTION+65 || ip->Type == ITEM_POTION+66 || ip->Type == ITEM_POTION+67 ||
		ip->Type == ITEM_POTION+68 ||
		ip->Type==ITEM_HELPER+52 || ip->Type==ITEM_HELPER+53 ||
		ip->Type==ITEM_POTION+100 ||
		(ip->Type >= ITEM_POTION+141 && ip->Type <= ITEM_POTION+144) ||
#ifdef LEM_ADD_LUCKYITEM
		(ip->Type >= ITEM_HELPER+135 && ip->Type <= ITEM_HELPER+145) ||
		(ip->Type == ITEM_POTION+160 || ip->Type == ITEM_POTION+161) ||
#endif // LEM_ADD_LUCKYITEM
		ip->Type==ITEM_POTION+16 || ip->Type==ITEM_POTION+22 )
	{
		Color = TEXT_COLOR_YELLOW;
	}
    else if ( ip->Type==ITEM_STAFF+10 || ip->Type==ITEM_SWORD+19 || ip->Type==ITEM_BOW+18 || ip->Type==ITEM_MACE+13)
    {
        Color = TEXT_COLOR_PURPLE;
    }
	else if(ip->Type==ITEM_POTION+17 || ip->Type==ITEM_POTION+18 || ip->Type==ITEM_POTION+19)
	{
		Color = TEXT_COLOR_YELLOW;
	}
    else if( ip->Type==ITEM_HELPER+16 || ip->Type==ITEM_HELPER+17 )
    {
        Color = TEXT_COLOR_YELLOW;
    }
    else if ( (ip->ExtOption%0x04)==EXT_A_SET_OPTION || (ip->ExtOption%0x04)==EXT_B_SET_OPTION )
    {
        Color = TEXT_COLOR_GREEN_BLUE;
    }
    else if( g_SocketItemMgr.IsSocketItem(ip) )
    {
        Color = TEXT_COLOR_VIOLET;
    }
	else if(ip->SpecialNum > 0 && (ip->Option1&63) > 0)
	{
		Color = TEXT_COLOR_GREEN;
	}
	else if(Level >= 7)
	{
		Color = TEXT_COLOR_YELLOW;
	}
	else
	{
		if(ip->SpecialNum > 0)
		{
   			Color = TEXT_COLOR_BLUE;
		}
		else
		{
			Color = TEXT_COLOR_WHITE;
		}
	}

    if ( ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 ) || ip->Type==ITEM_HELPER+30 
		|| ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )
		|| (ip->Type>=ITEM_WING+42 && ip->Type<=ITEM_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
    {
	    if ( Level >= 7 )
	    {
		    Color = TEXT_COLOR_YELLOW;
	    }
	    else
	    {
		    if(ip->SpecialNum > 0)
		    {
   			    Color = TEXT_COLOR_BLUE;
		    }
		    else
		    {
			    Color = TEXT_COLOR_WHITE;
		    }
	    }
    }

	int nGemType = COMGEM::Check_Jewel( ip->Type );
	if( nGemType != COMGEM::NOGEM)
	{
		Color = TEXT_COLOR_YELLOW;
	}

	if ( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) && !IsSellingBan(ip) )
	{
		char Text[100];
		{
			if(Sell)
			{
				DWORD dwValue = ItemValue(ip, 0);
				ConvertGold(dwValue, Text);
				char Text2[100];

				ConvertTaxGold(ItemValue(ip,0),Text2);
				sprintf(TextList[TextNum],GlobalText[1620],Text2,Text);
			}
			else
			{
				ConvertGold(ItemValue(ip,1),Text);
				sprintf(TextList[TextNum],GlobalText[63],Text);
			}

			TextListColor[TextNum] = Color;
//			TextBold[TextNum] = true;
			TextNum++;
			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
		}
	}
		if(	(Inventype == SEASON3B::TOOLTIP_TYPE_MY_SHOP || Inventype == SEASON3B::TOOLTIP_TYPE_PURCHASE_SHOP)
			&& !IsPersonalShopBan(ip) )
	{
		{
			int price = 0;
			int indexInv = (MAX_EQUIPMENT + MAX_INVENTORY)+(ip->y*COL_PERSONALSHOP_INVEN)+ip->x;
			char Text[100];

			if(GetPersonalItemPrice(indexInv, price, g_IsPurchaseShop)) 
			{
				ConvertGold(price, Text);
				sprintf(TextList[TextNum], GlobalText[63],Text);
				
				if ( price>=10000000 )
					TextListColor[TextNum] = TEXT_COLOR_RED;
				else if ( price>=1000000 )
					TextListColor[TextNum] = TEXT_COLOR_YELLOW;
				else if( price>=100000 )
					TextListColor[TextNum] = TEXT_COLOR_GREEN;
				else
					TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = true;
				TextNum++;
				sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
				
				DWORD gold = CharacterMachine->Gold;

				if((int)gold < price && g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE) 
				{
					TextListColor[TextNum] = TEXT_COLOR_RED;
					TextBold[TextNum] = true;
					sprintf(TextList[TextNum], GlobalText[423]);
					TextNum++;
					sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
				}
			}
			else if(g_IsPurchaseShop == PSHOPWNDTYPE_SALE)
			{
				TextListColor[TextNum] = TEXT_COLOR_RED;
				TextBold[TextNum] = true;
				sprintf(TextList[TextNum], GlobalText[1101]);
				TextNum++;
				sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
			}
		}
	}

    if (ip->Type>=ITEM_POTION+23 && ip->Type<=ITEM_POTION+26 )
    {
		if(ip->Type == ITEM_POTION+23)
		{
			Color = TEXT_COLOR_YELLOW;
			switch( Level )
			{
			case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;
			case 1: sprintf ( TextList[TextNum], GlobalText[906] ); break;
			}
		}
		else if(ip->Type == ITEM_POTION+24)
		{
			Color = TEXT_COLOR_YELLOW;
			switch( Level )
			{
			case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;
			case 1: sprintf ( TextList[TextNum], GlobalText[907] ); break;
			}
		}
		else{
			sprintf(TextList[TextNum],"%s", p->Name);
			Color = TEXT_COLOR_YELLOW;
		}
    }
	else if(ip->Type == ITEM_POTION+12)
	{
		switch(Level)
		{
		case 0:sprintf(TextList[TextNum],GlobalText[100]);break;
		case 1:sprintf(TextList[TextNum],GlobalText[101]);break;
		case 2:sprintf(TextList[TextNum],"%s",ChaosEventName[ip->Durability]);break;
		}
	}
	else if(ip->Type == ITEM_POTION+11)
	{
		switch(Level)
		{
		case 0:sprintf(TextList[TextNum],"%s",p->Name);break;
		case 1:sprintf(TextList[TextNum],GlobalText[105]);break;
		case 2:sprintf(TextList[TextNum],GlobalText[106]);break;
		case 3:sprintf(TextList[TextNum],GlobalText[107]);break;
		case 5:sprintf(TextList[TextNum],GlobalText[109]);break;
		case 6:sprintf(TextList[TextNum],GlobalText[110]);break;
        case 7:sprintf(TextList[TextNum],GlobalText[111]);break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
			sprintf(TextList[TextNum],"%s +%d",GlobalText[115], Level - 7);
			break;
		case 13:
			sprintf(TextList[TextNum],GlobalText[117]);
			break;
        case 14:
            sprintf(TextList[TextNum],GlobalText[1650]);
            break;
        case 15:
            sprintf(TextList[TextNum],GlobalText[1651]);
            break;
		}
	}
	else if(ip->Type == ITEM_POTION+12)
	{
		switch(Level)
		{
		case 0:sprintf(TextList[TextNum],GlobalText[100]);break;
		case 1:sprintf(TextList[TextNum],GlobalText[101]);break;
		case 2:sprintf(TextList[TextNum],"%s",ChaosEventName[ip->Durability]);break;
		}
	}
    else if ( ip->Type==ITEM_HELPER+15 )
    {
        Color = TEXT_COLOR_YELLOW;
        switch ( Level )
        {
        case 0:sprintf(TextList[TextNum],"%s %s", GlobalText[168], p->Name );break;
        case 1:sprintf(TextList[TextNum],"%s %s", GlobalText[169], p->Name );break;
        case 2:sprintf(TextList[TextNum],"%s %s", GlobalText[167], p->Name );break;
        case 3:sprintf(TextList[TextNum],"%s %s", GlobalText[166], p->Name );break;
		case 4:sprintf(TextList[TextNum],"%s %s", GlobalText[1900], p->Name );break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+14 )
    {
        Color = TEXT_COLOR_YELLOW;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;
        case 1: sprintf ( TextList[TextNum], "%s", GlobalText[1235] ); break;
        }
    }
    else if ( ip->Type==ITEM_POTION+21 )
    {
        Color = TEXT_COLOR_YELLOW;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;
        case 1: sprintf ( TextList[TextNum], GlobalText[810] ); break;
        case 2: sprintf ( TextList[TextNum], GlobalText[1098] ); break;
        case 3: sprintf ( TextList[TextNum], GlobalText[1290] ); break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+19 )
    {
        Color = TEXT_COLOR_YELLOW;
        switch ( Level )
        {
		case 0: sprintf(TextList[TextNum], GlobalText[811]); break;
		case 1: sprintf(TextList[TextNum], GlobalText[812]); break;
		case 2: sprintf(TextList[TextNum], GlobalText[817]); break;
        }
    }
	else if(ip->Type == ITEM_HELPER+20)
	{
		Color = TEXT_COLOR_YELLOW;
		switch( Level )
		{
		case 0: sprintf( TextList[TextNum], p->Name ); break;
		case 1: sprintf( TextList[TextNum], GlobalText[922] ); break;
        case 2: sprintf( TextList[TextNum], GlobalText[928] ); break;
        case 3: sprintf ( TextList[TextNum], GlobalText[929] ); break;
		}
	}
	else if(ip->Type == ITEM_HELPER+107)
	{
		Color = TEXT_COLOR_YELLOW;
		sprintf( TextList[TextNum], p->Name );
	}
    else if ( ip->Type==ITEM_POTION+7 )
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1413] ); break;
        case 1: sprintf( TextList[TextNum], GlobalText[1414] ); break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+7 )
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1460] ); break; 
        case 1: sprintf( TextList[TextNum], GlobalText[1461] ); break; 
        }
    }
    else if ( ip->Type==ITEM_HELPER+11 )
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1416] ); break;
        case 1: sprintf( TextList[TextNum], GlobalText[1462] ); break;
        }
    }
	else if ( ip->Type==ITEM_POTION+9 )
	{
		switch(Level)
		{
		case 0:sprintf(TextList[TextNum],"%s",p->Name);break;
		case 1:sprintf(TextList[TextNum],GlobalText[108]);break;
		}
	}
    else if(ip->Type == ITEM_WING+11)
	{
		sprintf(TextList[TextNum],"%s %s",SkillAttribute[30+Level].Name,GlobalText[102]);
	}
    else if(ip->Type == ITEM_HELPER+10)
	{
		for(int i=0;i<MAX_MONSTER;i++)
		{
			if(SommonTable[Level] == MonsterScript[i].Type)
			{
        		sprintf(TextList[TextNum],"%s %s",MonsterScript[i].Name,GlobalText[103]);
				break;
			}
		}
	}
    else if ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )
    {
		if(Level==0)
			sprintf(TextList[TextNum],"%s",p->Name);
		else
			sprintf(TextList[TextNum],"%s +%d",p->Name,Level);
    }
	else if ((ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40) || (ip->Type>=ITEM_WING+42 && ip->Type<=ITEM_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			|| (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
    {
		if(Level==0)
			sprintf(TextList[TextNum],"%s",p->Name);
		else
			sprintf(TextList[TextNum],"%s +%d",p->Name,Level);
    }
    else if ( ip->Type==ITEM_HELPER+31 )
    {
        switch ( Level )
        {
		case 0: sprintf ( TextList[TextNum], "%s of %s", p->Name, GlobalText[1187] ); break;
        case 1: sprintf ( TextList[TextNum], "%s of %s", p->Name, GlobalText[1214] ); break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+30 )
    {
		if ( Level==0 )
			sprintf ( TextList[TextNum], "%s", p->Name );
		else
			sprintf ( TextList[TextNum], "%s +%d", p->Name, Level );
    }
    else if ( ip->Type==ITEM_POTION+29 )
    {
        sprintf ( TextList[TextNum], GlobalText[1180], Level );
    }
    else if ( ip->Type==ITEM_POTION+28 )
    {
        Color = TEXT_COLOR_YELLOW;
        sprintf ( TextList[TextNum], "%s +%d", p->Name, Level );
    }
	else if(ip->Type == ITEM_WING+32)
	{
		sprintf(TextList[TextNum], "%s", p->Name);	
	}
	else if(ip->Type == ITEM_WING+33)
	{
		sprintf(TextList[TextNum], "%s", p->Name);	
	}
	else if(ip->Type == ITEM_WING+34)
	{
		sprintf(TextList[TextNum], "%s", p->Name);	
	}
	else if(ip->Type == ITEM_WING+35)
	{
		sprintf(TextList[TextNum], "%s", p->Name);	
	}
	else if(ip->Type >= ITEM_POTION+45 && ip->Type <= ITEM_POTION+50)
	{
		sprintf ( TextList[TextNum], "%s", p->Name );
	}
	else if(ip->Type == ITEM_POTION+32)
	{
		switch(Level)
		{
		case 0:
			sprintf( TextList[TextNum], "%s", p->Name); break;
		case 1:
			sprintf( TextList[TextNum], "%s", GlobalText[2012]); break;
		}
	}
	else if(ip->Type == ITEM_POTION+33)
	{
		switch(Level)
		{
		case 0:
			sprintf( TextList[TextNum], "%s", p->Name); break;
		case 1:
			sprintf( TextList[TextNum], "%s", GlobalText[2013]); break;
		}
	}
	else if(ip->Type == ITEM_POTION+34)
	{
		switch(Level)
		{
		case 0:
			sprintf( TextList[TextNum], "%s", p->Name); break;
		case 1:
			sprintf( TextList[TextNum], "%s", GlobalText[2014]); break;
		}
	}
	else if(ip->Type >= ITEM_HELPER+32 && ip->Type <= ITEM_HELPER+37)	
	{
		if(ip->Type == ITEM_HELPER+37)
		{
			Color = TEXT_COLOR_BLUE;
			if((ip->Option1&63) == 0x01)
				sprintf(TextList[TextNum], "%s %s", p->Name, GlobalText[1863]);
			else if((ip->Option1&63) == 0x02)
				sprintf(TextList[TextNum], "%s %s", p->Name, GlobalText[1864]);
			else if((ip->Option1&63) == 0x04)
				sprintf(TextList[TextNum], "%s %s", p->Name, GlobalText[1866]);
			else
				sprintf(TextList[TextNum], "%s", p->Name);
		}
		else
		{
			Color = TEXT_COLOR_WHITE;
			sprintf(TextList[TextNum], "%s", p->Name);	
		}
	}
	else if ( ip->Type==ITEM_SWORD+19 || ip->Type==ITEM_BOW+18 || ip->Type==ITEM_STAFF+10 || ip->Type==ITEM_MACE+13)
	{
		if(Level==0)
			sprintf(TextList[TextNum],"%s",p->Name);
		else
			sprintf(TextList[TextNum],"%s +%d",p->Name,Level);
	}
	else if( nGemType != COMGEM::NOGEM && nGemType%2 == 1 )
	{
		int nGlobalIndex = COMGEM::GetJewelIndex( nGemType , COMGEM::eGEM_NAME );
		sprintf(TextList[TextNum],"%s +%d", GlobalText[nGlobalIndex], Level+1);
	}
	else if( ip->Type == ITEM_POTION + 41 || ip->Type == ITEM_POTION + 42 ||
			 ip->Type == ITEM_POTION + 43 || ip->Type == ITEM_POTION + 44 ||
			 ip->Type == ITEM_HELPER + 38
		    ) 
	{
		sprintf(TextList[TextNum],"%s",p->Name);
		Color = TEXT_COLOR_YELLOW;
	}
	else if ((ip->Type >= ITEM_WING+60 && ip->Type <= ITEM_WING+65)
		|| (ip->Type >= ITEM_WING+70 && ip->Type <= ITEM_WING+74)
		|| (ip->Type >= ITEM_WING+100 && ip->Type <= ITEM_WING+129))
	{
		sprintf(TextList[TextNum],"%s",p->Name);
		Color = TEXT_COLOR_VIOLET;
	}
    else if ( ip->Type==ITEM_POTION+111 )
    {
        Color = TEXT_COLOR_YELLOW;
        sprintf ( TextList[TextNum], "%s", p->Name );
    }
	else if( ITEM_POTION+101 <= ip->Type && ip->Type <= ITEM_POTION+109 )
	{
        Color = TEXT_COLOR_YELLOW;
        sprintf ( TextList[TextNum], "%s", p->Name );
	}
	else
	{
		char TextName[64];
		if ( g_csItemOption.GetSetItemName( TextName, ip->Type, ip->ExtOption ) )
		{
			strcat ( TextName, p->Name );
		}
		else
		{
			strcpy ( TextName, p->Name );
		}

		if((ip->Option1&63) > 0)
		{
			if(Level==0)
				sprintf(TextList[TextNum],"%s %s", GlobalText[620], TextName);
			else
				sprintf(TextList[TextNum],"%s %s +%d", GlobalText[620], TextName,Level);
		}
		else
		{
			if(Level==0)
				sprintf(TextList[TextNum],"%s",TextName);
			else
				sprintf(TextList[TextNum],"%s +%d",TextName,Level);
		}
	}

	TextListColor[TextNum] = Color;TextBold[TextNum] = true;TextNum++;
	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

    if ( ip->Type==ITEM_HELPER+19)
    {
        int iMana;
        int iWeaponSpeed;
        int iNeedStrength;
        int iNeedDex;

        TextListColor[TextNum] = TEXT_COLOR_WHITE;
		sprintf ( TextList[TextNum], GlobalText[730] ); TextBold[TextNum] = false; TextNum++;

        TextListColor[TextNum] = TEXT_COLOR_DARKRED;
        sprintf ( TextList[TextNum], GlobalText[815] ); TextBold[TextNum] = false; TextNum++;
        sprintf ( TextList[TextNum], "\n" ); TextBold[TextNum] = false; TextNum++; SkipNum++;

        switch ( Level )
        {
        case 0:
    		sprintf ( TextList[TextNum], "%s: %d ~ %d", GlobalText[42], 107, 110 );  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            iWeaponSpeed  = 20;
            iNeedStrength = 132;
            iNeedDex      = 32;
            break;  
        case 1:
    		sprintf ( TextList[TextNum], "%s: %d ~ %d", GlobalText[40], 110, 120 );  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            iWeaponSpeed  = 35;
            iNeedStrength = 381;
            iNeedDex      = 149;
            break;
        case 2:
    		sprintf ( TextList[TextNum], "%s: %d ~ %d", GlobalText[41], 120, 140 );  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            iWeaponSpeed  = 35;
            iNeedStrength = 140;
            iNeedDex      = 350;
            break;  
        }

    	sprintf ( TextList[TextNum], GlobalText[64], iWeaponSpeed ); TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        sprintf ( TextList[TextNum], GlobalText[73], iNeedStrength );TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        sprintf ( TextList[TextNum], GlobalText[75], iNeedDex );     TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        sprintf ( TextList[TextNum], "\n" ); TextBold[TextNum] = false; TextNum++; SkipNum++;
		sprintf ( TextList[TextNum], GlobalText[87] );     TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
		sprintf ( TextList[TextNum], GlobalText[94], 20 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;

        switch ( Level )
        {
        case 0:
            {
		        sprintf ( TextList[TextNum], GlobalText[79], 53 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;
                sprintf ( TextList[TextNum], GlobalText[631] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
		        sprintf ( TextList[TextNum], GlobalText[632], 2 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
            }
            break;  
        case 1:
            {
                gSkillManager.GetSkillInformation ( AT_SKILL_SWORD4, 1, NULL, &iMana, NULL );
			    sprintf ( TextList[TextNum], GlobalText[84], iMana ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
                sprintf ( TextList[TextNum], GlobalText[629] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
                sprintf ( TextList[TextNum], GlobalText[630], 2 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
            }
            break;
        case 2:
            {
			    gSkillManager.GetSkillInformation ( AT_SKILL_CROSSBOW, 1, NULL, &iMana, NULL );
			    sprintf ( TextList[TextNum], GlobalText[86], iMana ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
                sprintf ( TextList[TextNum], GlobalText[629] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
                sprintf ( TextList[TextNum], GlobalText[630], 2 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
           }
            break;  
        }
        sprintf ( TextList[TextNum], GlobalText[628] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
		sprintf ( TextList[TextNum], GlobalText[633], 7 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
		sprintf ( TextList[TextNum], GlobalText[634] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
		sprintf ( TextList[TextNum], GlobalText[635] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;

    }

    if (ip->Type>=ITEM_POTION+23 && ip->Type<=ITEM_POTION+26 )
    {
		sprintf(TextList[TextNum],GlobalText[730]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],GlobalText[731]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],GlobalText[732]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
    }
	else if(ip->Type == ITEM_POTION+12)
	{
		if(Level <= 1)
		{
			sprintf(TextList[TextNum],GlobalText[119]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
		}
	}
	else if(ip->Type >= ITEM_HELPER+46 && ip->Type <= ITEM_HELPER+48)
	{
		int iMap = 0;
		if(ip->Type == ITEM_HELPER+46)
			iMap = 39;
		else if(ip->Type == ITEM_HELPER+47)
			iMap = 56;
		else if(ip->Type == ITEM_HELPER+48)
			iMap = 58;

		sprintf(TextList[TextNum], GlobalText[2259], GlobalText[iMap]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2270]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], "\n");
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}

	else if(ip->Type >= ITEM_HELPER+125 && ip->Type <= ITEM_HELPER+127)
	{
		int iMap = 0;
		if(ip->Type == ITEM_HELPER+125)
			iMap = 3057;
		else if(ip->Type == ITEM_HELPER+126)
			iMap = 2806;
		else if(ip->Type == ITEM_HELPER+127)
			iMap = 3107;
		
		sprintf(TextList[TextNum], GlobalText[2259], GlobalText[iMap]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], "\n");
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+54)
	{
		sprintf(TextList[TextNum], GlobalText[2261]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type >= ITEM_POTION+58 && ip->Type <= ITEM_POTION+62)
	{
		sprintf(TextList[TextNum], GlobalText[2269]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+83 )
	{
		sprintf(TextList[TextNum], GlobalText[2269]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type >= ITEM_POTION+145 && ip->Type <= ITEM_POTION+150)
	{
		sprintf(TextList[TextNum], GlobalText[2269]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+53 )
	{
		sprintf(TextList[TextNum], GlobalText[2250]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+43 )
	{
		sprintf(TextList[TextNum], GlobalText[2256]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2567]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2568]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;

	}
	else if( ip->Type == ITEM_HELPER+44 )
	{
		sprintf(TextList[TextNum], GlobalText[2257]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2567]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2568]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+45 )
	{
		sprintf(TextList[TextNum], GlobalText[2258]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2566]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type >= ITEM_POTION+70 && ip->Type <= ITEM_POTION+71)
	{
		sprintf(TextList[TextNum], GlobalText[69], ip->Durability );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		int index = ip->Type-(ITEM_POTION+70);

		sprintf(TextList[TextNum], GlobalText[2500+index]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type >= ITEM_POTION+72 && ip->Type <= ITEM_POTION+77)
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

		sprintf(TextList[TextNum], GlobalText[2503+(ip->Type-(ITEM_POTION+72))],Item_data.m_byValue1);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2502]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+59)
	{
		sprintf(TextList[TextNum], GlobalText[2509]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type >= ITEM_HELPER+54 && ip->Type <= ITEM_HELPER+58)
	{
		DWORD statpoint = 0;
		statpoint = ip->Durability*10;

		sprintf(TextList[TextNum], GlobalText[2511], statpoint );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2510]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;
		sprintf(TextList[TextNum], GlobalText[1908]);
		TextNum++;
		
		if(ip->Type == ITEM_HELPER+58)
		{
			if(gCharacterManager.GetBaseClass( Hero->Class ) == CLASS_DARK_LORD)
			{
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
			}
			else
			{
				TextListColor[TextNum] = TEXT_COLOR_DARKRED;
			}
			
			sprintf(TextList[TextNum],GlobalText[61], GlobalText[24]);
			TextNum++;
		}
	}
	else if(ip->Type >= ITEM_POTION+78 && ip->Type <= ITEM_POTION+82)
	{
		int index = ip->Type-(ITEM_POTION+78);
		DWORD value = 0;

		sprintf(TextList[TextNum], GlobalText[69], ip->Durability );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
		value = Item_data.m_byValue1;

		sprintf( TextList[TextNum],GlobalText[2512+index], value );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2517]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2518]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		if( ip->Type == ITEM_POTION+82 )
		{
			sprintf(TextList[TextNum], GlobalText[3115]);
			TextListColor[TextNum] = TEXT_COLOR_YELLOW;
			TextBold[TextNum] = false;
			TextNum++;
		}

		std::string timetext;
		g_StringTime( Item_data.m_Time, timetext, true );
		sprintf(TextList[TextNum], timetext.c_str() );
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2302] );
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+60)
	{
		sprintf(TextList[TextNum], GlobalText[2519]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+62 )
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

		sprintf(TextList[TextNum], GlobalText[2253], Item_data.m_byValue1 );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2569], Item_data.m_byValue2 );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2570] );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2566]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;

	}
	else if( ip->Type == ITEM_HELPER+63 )
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

		sprintf(TextList[TextNum], GlobalText[2254], Item_data.m_byValue1 );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2571], Item_data.m_byValue2 );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2572] );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+97 )
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
		
		sprintf(TextList[TextNum], GlobalText[2580]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2502]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+98 )
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
		
		sprintf(TextList[TextNum], GlobalText[2581]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2502]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if (ip->Type == ITEM_POTION+140)
	{
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2188], 100);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+96 )
	{
		sprintf(TextList[TextNum], GlobalText[2573]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2574]);
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2708]);
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+64 )
	{
		sprintf(TextList[TextNum], GlobalText[2575]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2576]);
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+65 )
	{
		sprintf(TextList[TextNum], GlobalText[2577]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2578]);
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+67 )
	{
		sprintf(TextList[TextNum], GlobalText[2600]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+123 )
	{
		sprintf(TextList[TextNum], GlobalText[2600]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[3068]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[3069]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[3070]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+80 )
	{
		sprintf(TextList[TextNum], GlobalText[2746]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2747]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2748]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+106 )
	{
		sprintf(TextList[TextNum], GlobalText[2746]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2744]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2748]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+107)
	{
		sprintf(TextList[TextNum],GlobalText[926]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+104)
	{
		sprintf(TextList[TextNum], GlobalText[2968]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+105)
	{
		sprintf(TextList[TextNum], GlobalText[2969]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+103)
	{
		sprintf(TextList[TextNum], GlobalText[2970], 170);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+69 )
	{
		sprintf(TextList[TextNum], GlobalText[2602]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		if (g_PortalMgr.IsRevivePositionSaved())
		{
			sprintf(TextList[TextNum], GlobalText[2603]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;
			g_PortalMgr.GetRevivePositionText(TextList[TextNum]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;
		}
	}
	else if( ip->Type == ITEM_HELPER+70 )
	{
		sprintf(TextList[TextNum], GlobalText[2604]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if (ip->Type == ITEM_HELPER+81)
	{
		sprintf(TextList[TextNum], GlobalText[2714]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		sprintf(TextList[TextNum], GlobalText[2729]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		sprintf(TextList[TextNum], GlobalText[3084]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
	}
	else if (ip->Type == ITEM_HELPER+82)
	{
		sprintf(TextList[TextNum], GlobalText[2715]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		sprintf(TextList[TextNum], GlobalText[2730]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		sprintf(TextList[TextNum], GlobalText[2716]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
	}
	else if (ip->Type == ITEM_HELPER+93)
	{
		sprintf(TextList[TextNum], GlobalText[2256]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
	}
	else if (ip->Type == ITEM_HELPER+94)
	{
		sprintf(TextList[TextNum], GlobalText[2257]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
	}
	else if(ip->Type == ITEM_HELPER+61)
	{
		sprintf(TextList[TextNum], GlobalText[2259], GlobalText[2369]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2270]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], "\n");
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+91)
	{
		sprintf(TextList[TextNum], GlobalText[2551]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+92)
	{
		sprintf(TextList[TextNum], GlobalText[2261]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2553]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+93)
	{
		sprintf(TextList[TextNum], GlobalText[2261]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2556]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+95)
	{
		sprintf(TextList[TextNum], GlobalText[2261]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2552]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+94 )
	{
		sprintf(TextList[TextNum], GlobalText[69], ip->Durability );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		
		sprintf(TextList[TextNum], GlobalText[2559]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+84 )
	{
		sprintf(TextList[TextNum], GlobalText[2011] );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+85 )
	{
		sprintf(TextList[TextNum], GlobalText[2549] );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		std::string timetext;
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
		g_StringTime( Item_data.m_Time, timetext, true );
		sprintf(TextList[TextNum], timetext.c_str() );
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+86 )
	{
		sprintf(TextList[TextNum], GlobalText[2550]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		std::string timetext;
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
		g_StringTime( Item_data.m_Time, timetext, true );
		sprintf(TextList[TextNum], timetext.c_str() );
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+87)
	{
		sprintf(TextList[TextNum], GlobalText[2532]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		std::string timetext;
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
		g_StringTime( Item_data.m_Time, timetext, true );
		sprintf(TextList[TextNum], timetext.c_str() );
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+88 )
	{
		sprintf(TextList[TextNum], GlobalText[69], ip->Durability );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2534]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2535]);
		TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+89 )
	{
		sprintf(TextList[TextNum], GlobalText[69], ip->Durability );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2534]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2536]);
		TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+90 )
	{
		sprintf(TextList[TextNum], GlobalText[69], ip->Durability );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2534]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2537]);
		TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
		TextBold[TextNum] = false;
		TextNum++;

	}
	else if(ip->Type == ITEM_HELPER+49)
	{
		sprintf(TextList[TextNum], GlobalText[2397]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+50)
	{
		sprintf(TextList[TextNum], GlobalText[2398]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+51)
	{
		sprintf(TextList[TextNum], GlobalText[2399]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+64)
	{
		sprintf(TextList[TextNum], GlobalText[2420]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}	
	else if(ip->Type >= ITEM_POTION+65 && ip->Type <= ITEM_POTION+68)
	{
		sprintf(TextList[TextNum], GlobalText[730]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],GlobalText[731]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],GlobalText[732]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+52)
	{
		sprintf(TextList[TextNum], GlobalText[1665]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+53)
	{
		sprintf(TextList[TextNum], GlobalText[1665]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if ((ip->Type >= ITEM_WING+60 && ip->Type <= ITEM_WING+65)	|| (ip->Type >= ITEM_WING+70 && ip->Type <= ITEM_WING+74) || (ip->Type >= ITEM_WING+100 && ip->Type <= ITEM_WING+129))
	{
		TextNum = g_SocketItemMgr.AttachToolTipForSeedSphereItem(ip, TextNum);
	}
	else if( ip->Type == ITEM_HELPER+71 || ip->Type == ITEM_HELPER+72 || ip->Type == ITEM_HELPER+73 || ip->Type == ITEM_HELPER+74 ||ip->Type == ITEM_HELPER+75 )
	{
		sprintf(TextList[TextNum], GlobalText[2709]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+38)
	{
		sprintf(TextList[TextNum],GlobalText[926]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+20)
	{
        switch ( Level )
        {
        case 0:
            {
                sprintf(TextList[TextNum],GlobalText[926]);
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
            }
            break;

        case 1:
		    {
			    sprintf(TextList[TextNum],GlobalText[924], 40 );
			    TextListColor[TextNum] = TEXT_COLOR_WHITE;
			    TextBold[TextNum] = false;
			    TextNum++;
			    sprintf(TextList[TextNum],GlobalText[731]);
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
			    sprintf(TextList[TextNum],GlobalText[732]);
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
			    sprintf(TextList[TextNum],GlobalText[733]);
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
		    }
            break;
        case 2:
		    {
			    sprintf(TextList[TextNum],GlobalText[924], 80 );
			    TextListColor[TextNum] = TEXT_COLOR_WHITE;
			    TextBold[TextNum] = false;
			    TextNum++;
			    sprintf(TextList[TextNum],GlobalText[731]);
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
			    sprintf(TextList[TextNum],GlobalText[732]);
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
			    sprintf(TextList[TextNum],GlobalText[733]);
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
		    }
            break;
        case 3:
            {
                sprintf(TextList[TextNum],GlobalText[926]);
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
            }
            break;
        }
	}
	else if(ip->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN && ip->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END)
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
		sprintf(TextList[TextNum],GlobalText[2717]);
		TextBold[TextNum] = false;
		TextNum++;
		
		switch (ip->Type)
		{
		case ITEM_TYPE_CHARM_MIXWING+EWS_KNIGHT_1_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2718], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_MAGICIAN_1_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2720], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_ELF_1_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2722], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_SUMMONER_1_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2724], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_DARKLORD_1_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2727], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_KNIGHT_2_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2719], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_MAGICIAN_2_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2721], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_ELF_2_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2723], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_SUMMONER_2_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2725], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_DARKKNIGHT_2_CHARM:
			{
				sprintf(TextList[TextNum], GlobalText[2726], Item_data.m_byValue1);
			}break;
		}

		sprintf(TextList[TextNum], GlobalText[2732+(ip->Type-(ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN))],
			Item_data.m_byValue1);

		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+110 )
	{
		sprintf(TextList[TextNum], GlobalText[2773]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2774]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2775]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum], GlobalText[1181], ip->Durability, 5);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2776], 5 - ip->Durability);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+111 )
	{
		sprintf(TextList[TextNum], GlobalText[2777]);
		TextListColor[TextNum] = TEXT_COLOR_DARKBLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2778]);
		TextListColor[TextNum] = TEXT_COLOR_DARKBLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_POTION+101 <= ip->Type && ip->Type <= ITEM_POTION+109 )
	{
		switch(ip->Type)
		{
		case ITEM_POTION+101:
			{
				sprintf ( TextList[TextNum], GlobalText[1181], ip->Durability, 5 ); 
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
				TextNum++;

				sprintf(TextList[TextNum], GlobalText[2788]);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
				TextNum++;
			}break;
		case ITEM_POTION+102:
			{
				sprintf(TextList[TextNum], GlobalText[2784]);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
				TextNum++;
				sprintf(TextList[TextNum], GlobalText[2785]);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
				TextNum++;
				sprintf(TextList[TextNum], GlobalText[2786]);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
				TextNum++;
			}
			break;
		case ITEM_POTION+103:
		case ITEM_POTION+104: 
		case ITEM_POTION+105: 
		case ITEM_POTION+106: 
		case ITEM_POTION+107: 
		case ITEM_POTION+108: 
			{
				sprintf(TextList[TextNum], GlobalText[2790]);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
				TextNum++;
			}break;
		case ITEM_POTION+109:
			{
				sprintf(TextList[TextNum], GlobalText[2792]);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
				TextNum++;
				sprintf(TextList[TextNum], GlobalText[2793]);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;
				TextBold[TextNum] = false;
				TextNum++;
			}break;
		}
	}
	else if( ITEM_HELPER+109 == ip->Type )
	{
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[3058], 4);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_HELPER+110 == ip->Type )
	{
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[622], 4);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_HELPER+111 == ip->Type )
	{
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[627], 50);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_HELPER+112 == ip->Type )
	{
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[624], 4);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_HELPER+113 == ip->Type )
	{
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[628], 10);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_HELPER+114 == ip->Type )
	{
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2229], 7);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_HELPER+115 == ip->Type )
	{
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[635]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_POTION+112 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2876] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_POTION+113 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2875] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+120)
	{
		sprintf ( TextList[TextNum], GlobalText[2971] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_POTION+121 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2877] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_POTION+122 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2878] );
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_POTION+123 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2879] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_POTION+124 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2880] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_POTION+134 <= ip->Type && ITEM_POTION+139 >= ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2972] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_HELPER+116 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[3018] ); 
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ITEM_HELPER+121 == ip->Type )
	{
		int iMap = 57;
		sprintf ( TextList[TextNum], GlobalText[2259], GlobalText[iMap] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf ( TextList[TextNum], GlobalText[2270] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf ( TextList[TextNum], GlobalText[2260], ip->Durability ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+124)
	{
		sprintf(TextList[TextNum], GlobalText[3116]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[3127]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = true;
		TextNum++;
	}
	else if(ip->Type >= ITEM_POTION+141 && ip->Type <= ITEM_POTION+144)
	{
		sprintf(TextList[TextNum], GlobalText[571]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type==ITEM_POTION+133)
	{
		sprintf(TextList[TextNum], GlobalText[69], ip->Durability );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		
		sprintf(TextList[TextNum], GlobalText[3267]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}

	Color = TEXT_COLOR_YELLOW;
	sprintf ( TextList[TextNum], "%s", p->Name );

	if(ip->Type == ITEM_POTION+19)
	{
		sprintf(TextList[TextNum],GlobalText[638]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],GlobalText[639]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}

	if(ip->DamageMin)
	{

		int minindex = 0, maxindex = 0, magicalindex = 0; 

		if( Level >= ip->Jewel_Of_Harmony_OptionLevel )	
		{
			StrengthenCapability SC;
			g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC, ip, 1 );

			if( SC.SI_isSP )
			{
				minindex     = SC.SI_SP.SI_minattackpower;
				maxindex     = SC.SI_SP.SI_maxattackpower;
				magicalindex = SC.SI_SP.SI_magicalpower;
			}
		}
		int DamageMin = ip->DamageMin;
		int DamageMax = ip->DamageMax;
		if(ip->Type>>4 == 15)
		{
			sprintf(TextList[TextNum],"%s: %d ~ %d",GlobalText[40+2],DamageMin,DamageMax);
		}
		else if ( ip->Type != ITEM_ETC+5 && ip->Type != ITEM_ETC+14 && ip->Type != ITEM_ETC+15)
		{	
			if ( ip->Type>=ITEM_ETC && ip->Type<ITEM_ETC+MAX_ITEM_INDEX )
			{
				int SkillIndex = getSkillIndexByBook( ip->Type );

				SKILL_ATTRIBUTE *skillAtt = &SkillAttribute[SkillIndex];
				
				DamageMin = skillAtt->Damage;
				DamageMax = skillAtt->Damage+skillAtt->Damage/2;

				sprintf(TextList[TextNum],"%s: %d ~ %d",GlobalText[42],DamageMin,DamageMax);
			}
			else
			{
				if( DamageMin + minindex >= DamageMax + maxindex )
					sprintf(TextList[TextNum],"%s: %d ~ %d",GlobalText[40+p->TwoHand],DamageMax + maxindex, DamageMax + maxindex);
				else
					sprintf(TextList[TextNum],"%s: %d ~ %d",GlobalText[40+p->TwoHand],DamageMin + minindex, DamageMax + maxindex);
			}
		}
		else
		{
			TextNum--;
		}

		if ( DamageMin>0 )
		{
			if( minindex != 0 || maxindex != 0 )
			{
				TextListColor[TextNum] = TEXT_COLOR_YELLOW;
				TextBold[TextNum] = false;
				TextNum++;
			}
			else
			{
				if((ip->Option1&63) > 0)
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
	if(ip->Defense)
	{
		int maxdefense = 0; 

		if( Level >= ip->Jewel_Of_Harmony_OptionLevel )	
		{
			StrengthenCapability SC;
			g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC, ip, 2 );

			if( SC.SI_isSD )
			{
				maxdefense     = SC.SI_SD.SI_defense;
			}
		}
		sprintf(TextList[TextNum],GlobalText[65],ip->Defense + maxdefense );

		if( maxdefense != 0 )
			TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		else
		{
			if(ip->Type>=ITEM_HELM && ip->Type<ITEM_BOOTS+MAX_ITEM_INDEX && (ip->Option1&63) > 0)
      			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			else
      			TextListColor[TextNum] = TEXT_COLOR_WHITE;

		}

		TextBold[TextNum] = false;
		TextNum++;
	}
	if(ip->MagicDefense)
	{
    	sprintf(TextList[TextNum],GlobalText[66],ip->MagicDefense);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	if(p->SuccessfulBlocking)
	{
		sprintf(TextList[TextNum],GlobalText[67],ip->SuccessfulBlocking);
		if((ip->Option1&63) > 0)
      		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		else
      		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	if(p->WeaponSpeed)
	{
    	sprintf(TextList[TextNum],GlobalText[64],p->WeaponSpeed);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	if(p->WalkSpeed)
	{
    	sprintf(TextList[TextNum],GlobalText[68],p->WalkSpeed);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	if(ip->Type >= ITEM_WING+32 && ip->Type <= ITEM_WING+34)
	{
		sprintf(TextList[TextNum],GlobalText[571]);
		switch(ip->Type)
		{
		case ITEM_WING+32:
			TextListColor[TextNum] = TEXT_COLOR_RED;
			break;
		case ITEM_WING+33:
			TextListColor[TextNum] = TEXT_COLOR_GREEN;
			break;
		case ITEM_WING+34:
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			break;
		}
		TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type >= ITEM_POTION+45 && ip->Type <= ITEM_POTION+50) //Halloween event
	{
		char Text_data[300];
		ITEM_ADD_OPTION Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);

		switch(ip->Type)
		{
		case ITEM_POTION+45:
			sprintf(TextList[TextNum],GlobalText[2011]);
			TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
			break;
		case ITEM_POTION+46:
			wsprintf(Text_data,GlobalText[2229],Item_data.m_byValue1);
			sprintf(TextList[TextNum],Text_data);
			TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
			break;
		case ITEM_POTION+47:
			wsprintf(Text_data,GlobalText[2230],Item_data.m_byValue1);
			sprintf(TextList[TextNum],Text_data);
			TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
			break;
		case ITEM_POTION+48:
			wsprintf(Text_data,GlobalText[2231],Item_data.m_byValue1);
			sprintf(TextList[TextNum],Text_data);
			TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
			break;
		case ITEM_POTION+49:
			wsprintf(Text_data,GlobalText[960],Item_data.m_byValue1);
			sprintf(TextList[TextNum],Text_data);
			TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
			break;
		case ITEM_POTION+50:
			wsprintf(Text_data,GlobalText[961],Item_data.m_byValue1);
			sprintf(TextList[TextNum],Text_data);
			TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
			break;
		}
		TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type >= ITEM_POTION+32 && ip->Type <= ITEM_POTION+34)
	{
		sprintf(TextList[TextNum],GlobalText[2011]);
		switch(ip->Type)
		{
		case ITEM_POTION+32:
			if(Level == 0)
				TextListColor[TextNum] = TEXT_COLOR_PURPLE;
			else
			if(Level == 1)
				TextListColor[TextNum] = TEXT_COLOR_PURPLE;
			break;
		case ITEM_POTION+33:
			if(Level == 0)
				TextListColor[TextNum] = TEXT_COLOR_RED;
			else
			if(Level == 1)
				TextListColor[TextNum] = TEXT_COLOR_RED;
			break;
		case ITEM_POTION+34:
			if(Level == 0)
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
			else
			if(Level == 1)
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
			break;
		}
		TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+11)
	{
		if( Level==7 )
		{
            sprintf(TextList[TextNum],GlobalText[112]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
            sprintf(TextList[TextNum],GlobalText[113]);  
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
            sprintf(TextList[TextNum],GlobalText[114]);  
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
        else if ( Level==14 )
        {
            sprintf(TextList[TextNum],GlobalText[1652]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
            sprintf(TextList[TextNum],GlobalText[1653]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        }
        else
        {
            sprintf(TextList[TextNum],GlobalText[571]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        }
		sprintf(TextList[TextNum],GlobalText[733]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
		if (Level == 13)
		{
			sprintf(TextList[TextNum],GlobalText[731]);
			TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
		}
	}
	char tCount = COMGEM::CalcCompiledCount(ip);
	if(tCount > 0)
	{
		int	nJewelIndex = COMGEM::Check_Jewel_Com(ip->Type);
		if( nJewelIndex != COMGEM::NOGEM )
		{
			sprintf(TextList[TextNum], GlobalText[1819], tCount, GlobalText[COMGEM::GetJewelIndex(nJewelIndex, COMGEM::eGEM_NAME)]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum], GlobalText[1820]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
	}
	if(ip->Type==ITEM_POTION+13)
	{
		sprintf(TextList[TextNum],GlobalText[572]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+14)
	{
		sprintf(TextList[TextNum],GlobalText[573]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+16)
	{
		sprintf(TextList[TextNum],GlobalText[621]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+17 || ip->Type==ITEM_POTION+18)
	{
		sprintf(TextList[TextNum],GlobalText[637]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+10 && Level >= 1 && Level <= 8)
	{
		sprintf(TextList[TextNum],GlobalText[157], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_WING+15)
	{
		sprintf(TextList[TextNum],GlobalText[574]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+22)
	{
		sprintf(TextList[TextNum],GlobalText[619]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+31)
	{
		sprintf(TextList[TextNum],GlobalText[1289]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_HELPER+0) //  
	{
		sprintf(TextList[TextNum],GlobalText[578], 20);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[739], 50);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_HELPER+1)
	{
		sprintf(TextList[TextNum],GlobalText[576]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if((ip->Type>=ITEM_WING && ip->Type<=ITEM_WING+2) || ip->Type==ITEM_WING+41)
	{
		sprintf(TextList[TextNum],GlobalText[577],12+Level*2);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[578],12+Level*2);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[579]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	else if( ip->Type == ITEM_HELPER + 38 )
	{
		sprintf(TextList[TextNum],GlobalText[2207]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	else if( ip->Type == ITEM_POTION + 41 )
	{
		sprintf(TextList[TextNum],GlobalText[2208]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	else if( ip->Type == ITEM_POTION + 42 )
	{
		sprintf(TextList[TextNum],GlobalText[2209]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	else if( ip->Type == ITEM_POTION + 43 )
	{
		sprintf(TextList[TextNum],GlobalText[2210]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	else if( ip->Type == ITEM_POTION + 44 )
	{
		sprintf(TextList[TextNum],GlobalText[2210]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
#ifdef LEM_ADD_LUCKYITEM
	else if( ip->Type == ITEM_POTION+160 )
	{
		// 연장의 보석
		sprintf(TextList[TextNum],GlobalText[3305]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	else if( ip->Type == ITEM_POTION+161 )
	{
		// 상승의 보석
		sprintf(TextList[TextNum],GlobalText[2209]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
#endif // LEM_ADD_LUCKYITEM
	else if ((ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6) || ip->Type==ITEM_WING+42) //날개
    {
		sprintf(TextList[TextNum],GlobalText[577],32+Level);  //  데미지 몇%증가.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[578],25+Level*2);  //  데미지 몇%흡수.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[579]);             //  이동 속도 향상.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
    }
	else if ((ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40) || ip->Type==ITEM_WING+43
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| ip->Type==ITEM_WING+50
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
    {
		sprintf(TextList[TextNum],GlobalText[577],39+Level*2);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		if ( ip->Type==ITEM_WING+40 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
#ifndef PBG_MOD_NEWCHAR_MONK_WING
			|| ip->Type==ITEM_WING+50
#endif //PBG_MOD_NEWCHAR_MONK_WING
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			)
		{
			sprintf(TextList[TextNum],GlobalText[578],24+Level*2);
		}
		else
		{
			sprintf(TextList[TextNum],GlobalText[578],39+Level*2);
		}
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[579]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
    }
	else if( ITEM_WING+130 <= ip->Type && 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		ip->Type <= ITEM_WING+135
#else //PBG_ADD_NEWCHAR_MONK_ITEM
		ip->Type <= ITEM_WING+134
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
	{
		switch(ip->Type)
		{
		case ITEM_WING+130:
		case ITEM_WING+135:
			{
				sprintf(TextList[TextNum],GlobalText[577],20+Level*2);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
				sprintf(TextList[TextNum],GlobalText[578],20+Level*2);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			}break;
		case ITEM_WING+131:
		case ITEM_WING+132:
		case ITEM_WING+133:
		case ITEM_WING+134:
			{
				sprintf(TextList[TextNum],GlobalText[577],12+Level*2);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
				sprintf(TextList[TextNum],GlobalText[578],12+Level*2);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			}break;
		}
		sprintf(TextList[TextNum],GlobalText[579]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	else if ( ip->Type==ITEM_HELPER+3 )
    {
		sprintf(TextList[TextNum],GlobalText[577],15 );
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[578],10 );
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+31 )
    {
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], GlobalText[1215] ); TextNum++; break;
        case 1: sprintf ( TextList[TextNum], GlobalText[1216] ); TextNum++; break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+14 )
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], "%s", GlobalText[748] ); TextNum++; break;
        case 1: sprintf ( TextList[TextNum], "%s", GlobalText[1236] );TextNum++; break;        }
    }
	
    else if ( ip->Type==ITEM_HELPER+15 )
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch ( Level )
        {
		case 0:sprintf(TextList[TextNum],"%s %s", GlobalText[168], GlobalText[636] );break;
		case 1:sprintf(TextList[TextNum],"%s %s", GlobalText[169], GlobalText[636] );break;
		case 2:sprintf(TextList[TextNum],"%s %s", GlobalText[167], GlobalText[636] );break;
		case 3:sprintf(TextList[TextNum],"%s %s", GlobalText[166], GlobalText[636] );break;
		case 4:sprintf(TextList[TextNum],"%s %s", GlobalText[1900], GlobalText[636]);break;
		}
		TextNum++;
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch ( Level )
        {
		case 0:sprintf(TextList[TextNum],"%s %s", GlobalText[168], GlobalText[1910] );break;
		case 1:sprintf(TextList[TextNum],"%s %s", GlobalText[169], GlobalText[1910] );break;
		case 2:sprintf(TextList[TextNum],"%s %s", GlobalText[167], GlobalText[1910] );break;
		case 3:sprintf(TextList[TextNum],"%s %s", GlobalText[166], GlobalText[1910] );break;
		case 4:sprintf(TextList[TextNum],"%s %s", GlobalText[1900], GlobalText[1910]);break;
		}
		TextNum++;
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;
		sprintf(TextList[TextNum], GlobalText[1908]);
		
		
		if(Level == 4)
		{
			TextNum++;
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			sprintf(TextList[TextNum],GlobalText[61], GlobalText[24]);
		}
        TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+16 )
    {
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
        sprintf ( TextList[TextNum], GlobalText[816] );
        TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+17 )
    {
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
        sprintf ( TextList[TextNum], GlobalText[816] );
        TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+18 )
    {
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
        sprintf ( TextList[TextNum], GlobalText[814] );
        TextNum++;
        sprintf ( TextList[TextNum], "\n" );
        TextNum++; SkipNum++;
		sprintf(TextList[TextNum],GlobalText[638]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],GlobalText[639]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
    }
    else if ( ip->Type==ITEM_POTION+7 )
    {
        switch ( Level )
        {
        case 0:
            {
                sprintf( TextList[TextNum], GlobalText[1417] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
                sprintf( TextList[TextNum], GlobalText[1418] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
                sprintf( TextList[TextNum], GlobalText[1419] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            }
            break;
        case 1:
            {
                sprintf( TextList[TextNum], GlobalText[1638] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
                sprintf( TextList[TextNum], GlobalText[1639] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
                sprintf( TextList[TextNum], GlobalText[1472] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            }
            break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+7 )
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1460] ); break;
        case 1: sprintf( TextList[TextNum], GlobalText[1461] ); break;
        }
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+11 )
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1416] ); break;
        case 1: sprintf( TextList[TextNum], GlobalText[1462] ); break;
        }
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+29 )
    {
        int startIndex = 0;
        if ( gCharacterManager.GetBaseClass( Hero->Class )==CLASS_DARK || gCharacterManager.GetBaseClass( Hero->Class )==CLASS_DARK_LORD 
#ifdef PBG_ADD_NEWCHAR_MONK
			|| GetBaseClass( Hero->Class )==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
			)
		{
            startIndex = 6;
		}

        int HeroLevel = CharacterAttribute->Level;

		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		sprintf(TextList[TextNum],"%s %s    %s      %s    ",GlobalText[1147], GlobalText[368], GlobalText[935], GlobalText[936] ); TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;

        for ( int i=0; i<6; i++ )
        {
            int Zen = g_iChaosCastleZen[i];

    		sprintf(TextList[TextNum],"        %d             %3d~%3d     %3d,000", i+1,  g_iChaosCastleLevel[startIndex+i][0],  min( 400, g_iChaosCastleLevel[startIndex+i][1] ),  Zen ); 
            if ( (HeroLevel>=g_iChaosCastleLevel[startIndex+i][0] && HeroLevel<=g_iChaosCastleLevel[startIndex+i][1]) && gCharacterManager.IsMasterLevel(Hero->Class) == false)
            {
                TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW; 
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE; 
            }
            TextBold[TextNum] = false; TextNum++;
        }
		sprintf(TextList[TextNum], "         %d          %s   %3d,000", 7, GlobalText[737], 1000); 
		if(gCharacterManager.IsMasterLevel(Hero->Class) == true)
		{
			TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW; 
		}
		else
		{
			TextListColor[TextNum] = TEXT_COLOR_WHITE; 
		}
		TextBold[TextNum] = false; 
		TextNum++;

        sprintf ( TextList[TextNum], "\n" ); 
		TextNum++; 
		SkipNum++;
    	sprintf ( TextList[TextNum], GlobalText[1157] );  
		TextListColor[TextNum] = TEXT_COLOR_DARKBLUE; 
		TextNum++;
    }
    else if ( ip->Type==ITEM_POTION+21 )
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch ( Level )
        {
        case 1: sprintf ( TextList[TextNum], GlobalText[813] ); break;
        case 2: sprintf ( TextList[TextNum], GlobalText[1099] ); break;
        case 3: sprintf ( TextList[TextNum], GlobalText[1291] ); break;
		default: break;
        }
        TextNum++;
    }
    else if ( ip->Type==ITEM_POTION+28 || ip->Type==ITEM_POTION+29 )
    {
        TextNum = RenderHellasItemInfo ( ip, TextNum );
    }
#ifdef PBG_MOD_NEWCHAR_MONK_WING_2
	else if(ip->Type==ITEM_WING+49 || ip->Type==ITEM_HELPER+30)
	{
		// 망토 관련 옵션변경
		sprintf(TextList[TextNum],GlobalText[577],20+Level*2);  //  데미지 몇%증가
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		int _iDamage = (ip->Type==ITEM_WING+49) ? 10+Level*2 : 10+Level;
		sprintf(TextList[TextNum],GlobalText[578],_iDamage);  //  데미지 몇%흡수
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
#endif //PBG_MOD_NEWCHAR_MONK_WING_2

	BOOL bDurExist = FALSE;
	if ( ( p->Durability || p->MagicDur ) && 
         ( (ip->Type<ITEM_WING || ip->Type>=ITEM_HELPER ) && ip->Type<ITEM_POTION ) ||
           (ip->Type>=ITEM_WING && ip->Type<=ITEM_WING+6)
		|| (ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
       )
	{
		bDurExist = TRUE;
	}
	if( ( bDurExist || ip->Durability) && 
		( ip->Type<ITEM_HELPER+14 || ip->Type>ITEM_HELPER+19 )
		&& !(ip->Type==ITEM_HELPER+20 && Level == 1)
		&& !(ip->Type==ITEM_HELPER+20 && Level == 2)
		&& !(ip->Type==ITEM_HELPER+29 )
        && ip->Type!=ITEM_POTION+7 && ip->Type!=ITEM_HELPER+7 && ip->Type!=ITEM_HELPER+11
		&& ip->Type != ITEM_HELPER+35
		&& !(ip->Type >= ITEM_POTION+70 && ip->Type <= ITEM_POTION+71)
		&& !(ip->Type >= ITEM_HELPER+54 && ip->Type <= ITEM_HELPER+58) 
		&& !(ip->Type >= ITEM_POTION+78 && ip->Type <= ITEM_POTION+82)
		&& !(ip->Type == ITEM_HELPER+66)
		&& !( ip->Type == ITEM_HELPER+71 || ip->Type == ITEM_HELPER+72 || ip->Type == ITEM_HELPER+73 || ip->Type == ITEM_HELPER+74 || ip->Type == ITEM_HELPER+75 )
		&& !(ip->Type == ITEM_HELPER+97)
		&& !(ip->Type == ITEM_HELPER+98)
		&& !(ip->Type == ITEM_POTION+91)
		&& !(ip->Type == ITEM_HELPER+99)
		&& !(ip->Type==ITEM_POTION+133)
      )
	{
		int Success = false;
        int arrow = false;
		if(ip->Type>=ITEM_POTION && ip->Type<=ITEM_POTION+8)
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type == ITEM_POTION+21 && Level == 3 )
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type==ITEM_BOW+7 || ip->Type==ITEM_BOW+15)
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
            arrow = true;
		}
		else if(ip->Type>=ITEM_POTION+35 && ip->Type<=ITEM_POTION+40)
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type==ITEM_POTION+133)
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type >= ITEM_POTION+46 && ip->Type <= ITEM_POTION+50)
		{
			sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type >= ITEM_POTION+153 && ip->Type <= ITEM_POTION+156)
		{
			sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type >= ITEM_HELPER+32 && ip->Type <= ITEM_HELPER+33)
		{
			sprintf(TextList[TextNum],GlobalText[1181],ip->Durability, 20);
			Success = true;
		}
		else if(ip->Type == ITEM_HELPER+34)
		{
			sprintf(TextList[TextNum],GlobalText[1181],ip->Durability, 10);
			Success = true;
		}
		else if(ip->Type == ITEM_HELPER+37)
		{
			if( ip->bPeriodItem == false )
			{
				sprintf(TextList[TextNum],GlobalText[70],ip->Durability);
				Success = true;
			}
		}
		else if(ip->Type>=ITEM_HELPER && ip->Type<=ITEM_HELPER+7)
		{
			if( ip->bPeriodItem == false )
			{
				sprintf(TextList[TextNum],GlobalText[70],ip->Durability);
				Success = true;
			}
		}
        else if(ip->Type == ITEM_HELPER+10)
		{
          	sprintf(TextList[TextNum],GlobalText[95],ip->Durability);
			Success = true;
		}
		else if(ip->Type == ITEM_HELPER+64 || ip->Type == ITEM_HELPER+65)
		{
			if( ip->bPeriodItem == false )
			{
				sprintf(TextList[TextNum],GlobalText[70],ip->Durability);
				Success = true;
			}
		}
		else if(ip->Type == ITEM_HELPER+67 || ip->Type == ITEM_HELPER+80
			|| ip->Type == ITEM_HELPER+106 ||ip->Type == ITEM_HELPER+123)
		{
			if( ip->bPeriodItem == false )
			{
				sprintf(TextList[TextNum],GlobalText[70],ip->Durability);
				Success = true;
			}
		}
		else if(ip->Type >= ITEM_HELPER+46 && ip->Type <= ITEM_HELPER+48)
		{
			sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
			Success = true;
		}
		else if(ip->Type >= ITEM_HELPER+125 && ip->Type <= ITEM_HELPER+127)
		{
			sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
			
			if (ip->Type == ITEM_HELPER+126)
			{
				TextNum++;
				sprintf(TextList[TextNum], GlobalText[3105]);
			
			}
			else if (ip->Type == ITEM_HELPER+127)
			{
				TextNum++;
				sprintf(TextList[TextNum], GlobalText[3106]);
			}
			Success = true;
		}
		else if( ip->Type == ITEM_POTION+53 )
		{
			sprintf(TextList[TextNum], GlobalText[2296], ip->Durability);
			Success = true;
		}
		else if(ip->Type == ITEM_HELPER+61)
		{
			sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
			Success = true;
		}
		else if(ip->Type == ITEM_POTION+100)
		{
			sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type == ITEM_HELPER+70 )
		{
			if (ip->Durability == 2)
			{
				sprintf(TextList[TextNum], GlobalText[2605]);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
			}
			else if (ip->Durability == 1)
			{
				sprintf(TextList[TextNum], GlobalText[2606]);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				g_PortalMgr.GetPortalPositionText(TextList[TextNum]);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
			}
		}

#ifdef LEM_ADD_LUCKYITEM
		else if( ip->Type >= ITEM_HELPER+135 && ip->Type <= ITEM_HELPER+145 )
		{
			sprintf(TextList[TextNum], GlobalText[2261]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;
			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
			Success = true;
		}
#endif // LEM_ADD_LUCKYITEM

		else if ( (bDurExist) && (ip->bPeriodItem == false) )
        {
            int maxDurability = calcMaxDurability ( ip, p, Level );

            sprintf(TextList[TextNum],GlobalText[71],ip->Durability, maxDurability);
			Success = true;
        }
		else if( ip->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN && ip->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END )
		{
			sprintf(TextList[TextNum], GlobalText[2732+(ip->Type-(MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN))]);
			
			Success = true;
		}
		else if(ip->Type == ITEM_HELPER+121)
		{
			sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
			Success = true;
		}

		if(Success)
		{
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
		}
    }
    else
    {
        if(ip->Type == ITEM_HELPER+10)
		{
          	sprintf(TextList[TextNum],GlobalText[95],ip->Durability);

            TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
		}
    }

    if( ip->Type==ITEM_BOW+7 || ip->Type==ITEM_BOW+15)
	{
        if ( Level>=1 )
        {
            int value = Level*2+1;

          	sprintf(TextList[TextNum],GlobalText[577], value );
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;
            sprintf(TextList[TextNum],GlobalText[88], 1);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;
        }
    }

	for(int i=0;i<MAX_RESISTANCE;i++)
	{
		if(p->Resistance[i])
		{
			sprintf(TextList[TextNum],GlobalText[72],GlobalText[48+i],Level+1);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
		}
	}

	if(ip->RequireLevel && ip->Type!=ITEM_HELPER+14 )
	{
		sprintf(TextList[TextNum],GlobalText[76],ip->RequireLevel);
		if(CharacterAttribute->Level < ip->RequireLevel)
		{
    		TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
      		sprintf(TextList[TextNum],GlobalText[74],ip->RequireLevel - CharacterAttribute->Level);
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

	int si_iNeedStrength= 0, si_iNeedDex = 0;
	bool bRequireStat	= true;

#ifdef LEM_ADD_LUCKYITEM
	if( Check_LuckyItem( ip->Type ) ) bRequireStat = false;
#endif // LEM_ADD_LUCKYITEM

	if( Level >= ip->Jewel_Of_Harmony_OptionLevel )
	{
		StrengthenCapability SC;
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &SC, ip, 0 );

		if( SC.SI_isNB ) 
		{
			si_iNeedStrength = SC.SI_NB.SI_force;
			si_iNeedDex      = SC.SI_NB.SI_activity;
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

	if(ip->RequireStrength && bRequireStat )
	{
		sprintf(TextList[TextNum],GlobalText[73],ip->RequireStrength - si_iNeedStrength);

		WORD Strength;
		Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
     	if( Strength < ip->RequireStrength - si_iNeedStrength )
		{
    		TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
      		sprintf(TextList[TextNum],GlobalText[74],(ip->RequireStrength - Strength) - si_iNeedStrength );
			TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
		}
		else
		{
			if( si_iNeedStrength != 0 )
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
	if(ip->RequireDexterity && bRequireStat )
	{
		sprintf(TextList[TextNum],GlobalText[75],ip->RequireDexterity - si_iNeedDex );
		WORD Dexterity;
		Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
		if( Dexterity < ( ip->RequireDexterity - si_iNeedDex) )
		{
    		TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;

			sprintf(TextList[TextNum],GlobalText[74],(ip->RequireDexterity - Dexterity) - si_iNeedDex);
			TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
		}
		else
		{
    		if( si_iNeedDex != 0 )
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

	if(ip->RequireVitality && bRequireStat ) //  요구체력.
	{
		sprintf(TextList[TextNum],GlobalText[1930],ip->RequireVitality);

		WORD Vitality;
		Vitality = CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
     	if(Vitality < ip->RequireVitality)
		{
    		TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
      		sprintf(TextList[TextNum],GlobalText[74],ip->RequireVitality - Vitality);
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

	if(ip->RequireEnergy && bRequireStat )
	{
   		sprintf(TextList[TextNum],GlobalText[77],ip->RequireEnergy);

        WORD Energy;
		Energy = CharacterAttribute->Energy   + CharacterAttribute->AddEnergy;  

		if( Energy < ip->RequireEnergy)
		{
    		TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
      		sprintf(TextList[TextNum],GlobalText[74],ip->RequireEnergy - Energy);
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

	if(ip->RequireCharisma && bRequireStat )
	{
		sprintf(TextList[TextNum],GlobalText[698],ip->RequireCharisma);

		WORD Charisma;
		Charisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
     	if(Charisma < ip->RequireCharisma)
		{
    		TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
      		sprintf(TextList[TextNum],GlobalText[74],ip->RequireCharisma - Charisma);
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

	if(IsRequireClassRenderItem(ip->Type))
    {
        RequireClass(p);
    }

	if(ip->Type>=MODEL_BOOTS-MODEL_ITEM && ip->Type<MODEL_BOOTS+MAX_ITEM_INDEX-MODEL_ITEM)
	{
		if(Level >= 5)
		{
			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
			sprintf(TextList[TextNum],GlobalText[78]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = true;TextNum++;
		}
	}

	if(ip->Type>=MODEL_GLOVES-MODEL_ITEM && ip->Type<MODEL_GLOVES+MAX_ITEM_INDEX-MODEL_ITEM)
	{
		if(Level >= 5)
		{
			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
			
			sprintf(TextList[TextNum],GlobalText[93]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = true;TextNum++;
		}
	}
	if ( ( ip->Type>=MODEL_STAFF-MODEL_ITEM && ip->Type<MODEL_STAFF+MAX_ITEM_INDEX-MODEL_ITEM ) 
		 || ( ip->Type==(MODEL_SWORD+31-MODEL_ITEM) ) 
		 || ( ip->Type==(MODEL_SWORD+23-MODEL_ITEM) )
		 || ( ip->Type==(MODEL_SWORD+25-MODEL_ITEM) )
		 || ( ip->Type==(MODEL_SWORD+21-MODEL_ITEM) )
		 || ( ip->Type==(MODEL_SWORD+28-MODEL_ITEM) )
	   )
	{
		sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

		int nText = ITEM_STAFF+21 <= ip->Type && ip->Type <= ITEM_STAFF+29 ? 1691 : 79;
		::sprintf(TextList[TextNum], GlobalText[nText], ip->MagicPower);

		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = true;
		TextNum++;
	}

	if(IsCepterItem(ip->Type) == true)
    {
		sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
        sprintf(TextList[TextNum],GlobalText[1234],ip->MagicPower);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = true;TextNum++;
    }

	if(ip->SpecialNum > 0)
	{
		int iModelType = ip->Type;
		int iStartModelType = ITEM_HELPER+109;
		int iEndModelType = ITEM_HELPER+115;

		if(!( ITEM_HELPER+109 <= ip->Type  && ITEM_HELPER+115 >= ip->Type ))
		{
			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
		}
	}

	if( ip->option_380 != 0 )
	{
		std::vector<std::string> Text380;

		if( g_pItemAddOptioninfo )
		{
			g_pItemAddOptioninfo->GetItemAddOtioninfoText( Text380, ip->Type );

			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

			for( int i = 0; i < (int)Text380.size(); ++i )
			{
				strncpy(TextList[TextNum], Text380[i].c_str(), 100);
				TextListColor[TextNum] = TEXT_COLOR_REDPURPLE;TextBold[TextNum] = true;TextNum++;
			}

			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
		}
	}
#ifndef PBG_MOD_NEWCHAR_MONK_WING_2
#ifdef PBG_MOD_NEWCHAR_MONK_WING
	if(ip->Type==ITEM_WING+49)
	{
		sprintf(TextList[TextNum],GlobalText[578],15+Level);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextNum++;
	}
#endif //PBG_MOD_NEWCHAR_MONK_WING
#endif //PBG_MOD_NEWCHAR_MONK_WING_2
	if (g_SocketItemMgr.IsSocketItem(ip));
	else
	if( ip->Jewel_Of_Harmony_Option != 0 )
	{
		StrengthenItem type = g_pUIJewelHarmonyinfo->GetItemType( static_cast<int>(ip->Type) );

		if( type < SI_None )
		{
			if( g_pUIJewelHarmonyinfo->IsHarmonyJewelOption( type, ip->Jewel_Of_Harmony_Option ) )
			{
				sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

				HARMONYJEWELOPTION harmonyjewel = g_pUIJewelHarmonyinfo->GetHarmonyJewelOptionInfo( type, ip->Jewel_Of_Harmony_Option );

				if(type == SI_Defense && ip->Jewel_Of_Harmony_Option == 7)
				{
					sprintf(TextList[TextNum], "%s +%d%%", harmonyjewel.Name, harmonyjewel.HarmonyJewelLevel[ip->Jewel_Of_Harmony_OptionLevel]);
				}
				else
				{
					sprintf(TextList[TextNum], "%s +%d", harmonyjewel.Name, harmonyjewel.HarmonyJewelLevel[ip->Jewel_Of_Harmony_OptionLevel]);
				}

				if( Level >= ip->Jewel_Of_Harmony_OptionLevel ) TextListColor[TextNum] = TEXT_COLOR_YELLOW;
				else TextListColor[TextNum] = TEXT_COLOR_GRAY;
				
				TextBold[TextNum] = true;TextNum++;

				sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
			}
			else
			{
				sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

				sprintf( TextList[TextNum], "%s : %d %d %d"
					   ,GlobalText[2204]
					   , (int)type
					   , (int)ip->Jewel_Of_Harmony_Option
					   , (int)ip->Jewel_Of_Harmony_OptionLevel
					   );

				TextListColor[TextNum] = TEXT_COLOR_DARKRED;
				TextBold[TextNum] = true;TextNum++;

				sprintf( TextList[TextNum], GlobalText[2205] );

				TextListColor[TextNum] = TEXT_COLOR_DARKRED;
				TextBold[TextNum] = true;TextNum++;

				sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
			}
		}
	}

	TextNum = g_csItemOption.RenderDefaultOptionText( ip, TextNum );

	int iMana;
	for(int i=0;i<ip->SpecialNum;i++)
	{
		if( ITEM_HELPER+109 <= ip->Type  && ITEM_HELPER+115 >= ip->Type )
		{
			break;
		}

		gSkillManager.GetSkillInformation( ip->Special[i], 1, NULL, &iMana, NULL);
        GetSpecialOptionText ( ip->Type, TextList[TextNum], ip->Special[i], ip->SpecialValue[i], iMana );

       	TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;TextNum++;

		if(ip->Special[i] == AT_LUCK)
		{
			sprintf(TextList[TextNum],GlobalText[94],ip->SpecialValue[i]);
     		TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = false;TextNum++;
		}
        else if ( ip->Special[i]==AT_SKILL_RIDER )
        {
			sprintf(TextList[TextNum], GlobalText[179] );
     		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
        }
        else if ( ip->Special[i]==AT_SKILL_DARK_HORSE || (AT_SKILL_ASHAKE_UP <= ip->Special[i] && ip->Special[i] <= AT_SKILL_ASHAKE_UP+4))
        {
			sprintf(TextList[TextNum], GlobalText[1201] );
     		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
        }
		else if ( (ip->Special[i]==AT_IMPROVE_DAMAGE) && 
					( (ip->Type==ITEM_SWORD+31)
					|| (ip->Type==ITEM_SWORD+21)
					|| (ip->Type==ITEM_SWORD+23)
					|| (ip->Type==ITEM_SWORD+25)
					|| (ip->Type==ITEM_SWORD+28)
					)
				)
		{
			sprintf(TextList[TextNum],GlobalText[89],ip->SpecialValue[i]);
       		TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;TextNum++;
		}
	}

	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	if(ip->Type == ITEM_HELPER+32 || ip->Type == ITEM_HELPER+33)
	{
		sprintf(TextList[TextNum], GlobalText[1917]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW; 
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+34 || ip->Type == ITEM_HELPER+35)
	{
		sprintf(TextList[TextNum], GlobalText[1918]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW; 
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+36)
	{
		sprintf(TextList[TextNum], GlobalText[1919]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW; 
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+37)
	{
		GetSpecialOptionText ( 0, TextList[TextNum], AT_SKILL_PLASMA_STORM_FENRIR, 0, 0 );
   		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false; TextNum++; SkipNum++;
		if(ip->Option1 == 0x01)
		{
			sprintf(TextList[TextNum], GlobalText[1860], 10);
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;

			sprintf(TextList[TextNum], GlobalText[579]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
		}
		else if(ip->Option1 == 0x02)
		{
			sprintf(TextList[TextNum], GlobalText[1861], 10);
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;

			sprintf(TextList[TextNum], GlobalText[579]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
		}
		else if(ip->Option1 == 0x04)
		{
			WORD wLevel = CharacterAttribute->Level;
	
			sprintf(TextList[TextNum], GlobalText[1867], (wLevel / 2)); 
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
			sprintf(TextList[TextNum], GlobalText[1868], (wLevel / 2));
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
			sprintf(TextList[TextNum], GlobalText[1869], (wLevel / 12));
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
			sprintf(TextList[TextNum], GlobalText[1870], (wLevel / 25));
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
			sprintf(TextList[TextNum], "\n");
			TextNum++;
			sprintf(TextList[TextNum], GlobalText[1871], (Hero->Level / 2));
			TextListColor[TextNum] = TEXT_COLOR_GREEN; 
			TextNum++;
			sprintf(TextList[TextNum], GlobalText[1872], (Hero->Level / 2));
			TextListColor[TextNum] = TEXT_COLOR_GREEN; 
			TextNum++;
		}

		sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

		sprintf(TextList[TextNum], GlobalText[1920]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW; 
		TextNum++;
		
		if(ip->Option1 == 0x00)
		{
			sprintf(TextList[TextNum], GlobalText[1929]);
			TextListColor[TextNum] = TEXT_COLOR_YELLOW; 
			TextNum++;
		}
	}
    else if(ip->Type == ITEM_HELPER+10)
	{
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+39)
	{
		char strText[100];
		sprintf(strText, GlobalText[959], 10);
		sprintf(TextList[TextNum], "%s%%", strText);
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;

		WORD wlevel = CharacterAttribute->Level;
		sprintf(TextList[TextNum], GlobalText[2225], wlevel);
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;

		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if (ip->Type == ITEM_POTION+63)
	{
		TextListColor[TextNum] = TEXT_COLOR_WHITE; 
		sprintf(TextList[TextNum], GlobalText[2244]); TextNum++;
	}
	else if (ip->Type == ITEM_POTION+52)
	{
		TextListColor[TextNum] = TEXT_COLOR_WHITE; 
		sprintf(TextList[TextNum], GlobalText[2323]); TextNum++;
		TextListColor[TextNum] = TEXT_COLOR_WHITE; 
		sprintf(TextList[TextNum], GlobalText[2011]); TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+40)
	{
		sprintf(TextList[TextNum], "%s", GlobalText[2232]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
		TextNum++;	

		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+41)
	{
		sprintf(TextList[TextNum], GlobalText[88], 20);
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[89], 20);
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum], "%s", GlobalText[2248]);
		TextListColor[TextNum] = TEXT_COLOR_RED; 
		TextNum++;
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+76 )
	{
		sprintf(TextList[TextNum], GlobalText[2743]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2744]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[2745]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+122)
	{
		sprintf(TextList[TextNum], GlobalText[3065]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[3066]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum], GlobalText[3067]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum], GlobalText[3072]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+51)
	{
		sprintf(TextList[TextNum], "%s", GlobalText[2244]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+42)
	{
		for(int i=0; i<7; ++i)
		{
			sprintf(TextList[TextNum], GlobalText[976+i], 255);
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
		}

		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+66)
	{
		TextNum--;
		sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
 		TextListColor[TextNum] = TEXT_COLOR_RED;
 		TextNum++;
 		sprintf(TextList[TextNum], "%s", GlobalText[2589]);
 		TextListColor[TextNum] = TEXT_COLOR_BLUE;
 		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+100)
	{
		sprintf(TextList[TextNum], GlobalText[1887], ip->Durability);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
 		TextNum++;
	}
	else if( ip->Type == ITEM_HELPER+123 )
	{
		TextNum--; SkipNum--;

		sprintf(TextList[TextNum], GlobalText[3071]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[3072]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
	else if(g_pMyInventory->IsInvenItem(ip->Type))
	{
		TextNum--; SkipNum--;

		if (ip->Durability == 254)
		{
			sprintf(TextList[TextNum], GlobalText[3143]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;
			sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		}

		switch (ip->Type)
		{
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
			case ITEM_HELPER+128:
			case ITEM_HELPER+129:
				sprintf(TextList[TextNum], GlobalText[3121]);
				break;
			case ITEM_HELPER+134:
				sprintf(TextList[TextNum], GlobalText[3123]);
				break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
			case ITEM_HELPER+130:
			case ITEM_HELPER+131:
			case ITEM_HELPER+132:
			case ITEM_HELPER+133:
				sprintf(TextList[TextNum], GlobalText[3122]);
				break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

			default:
				break;
		}

		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		switch (ip->Type)
		{
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
			case ITEM_HELPER+128:
				sprintf(TextList[TextNum], GlobalText[965], 10);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+129:
				sprintf(TextList[TextNum], GlobalText[967], 10);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+134:
				sprintf(TextList[TextNum], GlobalText[3126], 20);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
			case ITEM_HELPER+130:
				sprintf(TextList[TextNum], GlobalText[3132], 50);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+131:
				sprintf(TextList[TextNum], GlobalText[3134], 50);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+132:
#ifdef LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
				sprintf(TextList[TextNum], GlobalText[3132], 100);
#else //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
				sprintf(TextList[TextNum], GlobalText[3132], 150);
#endif //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
#ifdef LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
				sprintf(TextList[TextNum], GlobalText[3133], 500);
#else //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
				sprintf(TextList[TextNum], GlobalText[3133], 50);
#endif //LJH_MOD_CHANGED_GOLDEN_OAK_CHARM_STAT
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+133:	// 골든메이플참
				sprintf(TextList[TextNum], GlobalText[3134], 150);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				sprintf(TextList[TextNum], GlobalText[3135], 50);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
			default:
				break;
		}

		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3124]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef PBG_ADD_NEWCHAR_MONK
    else if(ip->Type == ITEM_HELPER+68)
	{
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //PBG_ADD_NEWCHAR_MONK

	if( ip->bPeriodItem == true )
	{
		if( ip->bExpiredPeriod == true )
		{
			sprintf(TextList[TextNum], GlobalText[3266]);
			TextListColor[TextNum] = TEXT_COLOR_RED; 	
		}
		else
		{
			sprintf(TextList[TextNum], GlobalText[3265]);
			TextListColor[TextNum] = TEXT_COLOR_ORANGE; 	
			TextNum++;
			SkipNum++;

			sprintf(TextList[TextNum], "%d-%02d-%02d  %02d:%02d", ExpireTime->tm_year+1900, ExpireTime->tm_mon+1, 
					ExpireTime->tm_mday, ExpireTime->tm_hour, ExpireTime->tm_min);
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		}

		TextNum++;
	}

	if (!bItemTextListBoxUse)
	{
		bool bThisisEquippedItem = false;

		SEASON3B::CNewUIInventoryCtrl * pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
		ITEM *pFindItem = pNewInventoryCtrl->FindItemByKey( ip->Key );
		(pFindItem==NULL)?bThisisEquippedItem=true:bThisisEquippedItem=false;

		TextNum = g_csItemOption.RenderSetOptionListInItem ( ip, TextNum, bThisisEquippedItem );

		TextNum = g_SocketItemMgr.AttachToolTipForSocketItem(ip, TextNum);

		SIZE TextSize = {0, 0};
		float fRateY	= g_fScreenRate_y;
		int	Height		= 0;
		int	EmptyLine	= 0;
		int TextLine	= 0;

		for(int i = 0; i < TextNum; ++i)
		{
			if(TextList[i][0] == '\0')		break;
			else if(TextList[i][0] == '\n')	++EmptyLine;
			else							++TextLine;
		}
		fRateY	= fRateY / 1.1f;
		g_pRenderText->SetFont(g_hFont);

		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);

		Height = (TextLine * TextSize.cy + EmptyLine * TextSize.cy / 2.0f) / fRateY;
		
		int iScreenHeight	= 420;

		int nInvenHeight	= p->Height*INVENTORY_SCALE;

		sy += INVENTORY_SCALE;
		if( sy + Height > iScreenHeight)
		{
			sy += iScreenHeight - ( sy + Height );

		}	
		else if(sy + Height > iScreenHeight)
		{
			
		}
	}
	
	bool isrendertooltip = true;

	if( isrendertooltip )
	{
		if (bItemTextListBoxUse)
			RenderTipTextList(sx, sy, TextNum, 0, RT3_SORT_CENTER, STRP_BOTTOMCENTER);
		else
			RenderTipTextList(sx,sy,TextNum,0);
	}
}

void RenderRepairInfo(int sx,int sy,ITEM *ip,bool Sell)
{
	if(IsRepairBan(ip) == true)
	{
		return;
	}
	if( ip->Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN && ip->Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END )
	{
		return;
	}
	if(ip->Type == ITEM_HELPER+107)
	{
		return;
	}
	if(ip->Type == ITEM_HELPER+104)
	{
		return;
	}
	if(ip->Type == ITEM_HELPER+105)
	{
		return;
	}
	if(ip->Type == ITEM_HELPER+103)
	{
		return;
	}
	if(ip->Type == ITEM_POTION+133)
	{
		return;
	}
	if(ip->Type == MODEL_HELPER+109)
	{
		return;
	}
	if(ip->Type == MODEL_HELPER+110)
	{
		return;
	}
	if(ip->Type == MODEL_HELPER+111)
	{
		return;
	}
	if(ip->Type == MODEL_HELPER+112)
	{
		return;
	}
	if(ip->Type == MODEL_HELPER+113)
	{
		return;
	}
	if(ip->Type == MODEL_HELPER+114)
	{
		return;
	}
	if(ip->Type == MODEL_HELPER+115)
	{
		return;
	}
	if(ip->Type == MODEL_POTION+112)
	{
		return;
	}
	if(ip->Type == MODEL_POTION+113)
	{
		return;
	}
	if(ip->Type == ITEM_POTION+120)
	{
		return;
	}
	if(ip->Type == ITEM_POTION+121)
	{
		return;
	}
	if(ip->Type == ITEM_POTION+122)
	{
		return;
	}
	if( ITEM_POTION+123 == ip->Type )
	{
		return;
	}
	if( ITEM_POTION+124 == ip->Type )
	{
		return;
	}
	if( ITEM_POTION+134 <= ip->Type && ip->Type <= ITEM_POTION+139)
	{
		return;
	}

	if( ITEM_WING+130 <= ip->Type && 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		ip->Type <= ITEM_WING+135
#else //PBG_ADD_NEWCHAR_MONK_ITEM
		ip->Type <= ITEM_WING+134
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
	{
		return;
	}

	if( ITEM_POTION+114 <= ip->Type && ip->Type <= ITEM_POTION+119)
	{
		return;
	}
	if( ITEM_POTION+126 <= ip->Type && ip->Type <= ITEM_POTION+129)
	{
		return;
	}
	if( ITEM_POTION+130 <= ip->Type && ip->Type <= ITEM_POTION+132)
	{
		return;
	}
	if( ITEM_HELPER+121 == ip->Type )
	{
		return;
	}

	ITEM_ATTRIBUTE *p = &ItemAttribute[ip->Type];
	TextNum = 0;
	SkipNum = 0;
	for(int i=0;i<30;i++)
	{
		TextList[i][0] = NULL;
	}

	int Level = (ip->Level>>3)&15;
	int Color;

	if(ip->Type==ITEM_POTION+13 || ip->Type==ITEM_POTION+14 || ip->Type==ITEM_WING+15)
	{
		Color = TEXT_COLOR_YELLOW;
	}
	else if(COMGEM::isCompiledGem(ip))
	{
		Color = TEXT_COLOR_YELLOW;
	}
    else if ( ip->Type==ITEM_STAFF+10 || ip->Type==ITEM_SWORD+19 || ip->Type==ITEM_BOW+18 || ip->Type==ITEM_MACE+13)
    {
        Color = TEXT_COLOR_PURPLE;
    }
	else if(ip->Type==ITEM_POTION+17 || ip->Type==ITEM_POTION+18 || ip->Type==ITEM_POTION+19)
	{
		Color = TEXT_COLOR_YELLOW;
	}
	else if(ip->SpecialNum > 0 && (ip->Option1&63) > 0)
	{
		Color = TEXT_COLOR_GREEN;
	}
	else if(Level >= 7)
	{
		Color = TEXT_COLOR_YELLOW;
	}
	else
	{
		if(ip->SpecialNum > 0)
		{
   			Color = TEXT_COLOR_BLUE;
		}
		else
		{
			Color = TEXT_COLOR_WHITE;
		}
	}

    if ( (  ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )
         || ip->Type>=ITEM_HELPER+30
		 || ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )
		 || (ip->Type>=ITEM_WING+42 && ip->Type<=ITEM_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		 || (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
       )  
	{
	    if ( Level >= 7 )
	    {
		    Color = TEXT_COLOR_YELLOW;
	    }
	    else
	    {
		    if(ip->SpecialNum > 0)
		    {
   			    Color = TEXT_COLOR_BLUE;
		    }
		    else
		    {
			    Color = TEXT_COLOR_WHITE;
		    }
	    }
    }

	if ( ip->Type<ITEM_POTION)
    {
        int maxDurability;

	    sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	    char Text[100];

        maxDurability = calcMaxDurability ( ip, p, Level );
        if(ip->Durability<maxDurability)
        {
            RepairEnable = 2;
			
			int iGold = ItemValue(ip,2);
			if( iGold == -1 )
				return;
            ConvertRepairGold(iGold, ip->Durability, maxDurability, ip->Type, Text);
		    sprintf(TextList[TextNum],GlobalText[238],Text);

            TextListColor[TextNum] = Color;
        }
        else
        {
            RepairEnable = 1;
		    sprintf(TextList[TextNum],GlobalText[238], "0" );
            TextListColor[TextNum] = Color;
        }
	    TextBold[TextNum] = true;
	    TextNum++;

//        RepairEnable = 1;
    }
	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

    if(ip->Type == ITEM_WING+11)
	{
		sprintf(TextList[TextNum],"%s %s",SkillAttribute[30+Level].Name,GlobalText[102]);
	}
    else if(ip->Type == ITEM_HELPER+10)
	{
		for(int i=0;i<MAX_MONSTER;i++)
		{
			if(SommonTable[Level] == MonsterScript[i].Type)
			{
        		sprintf(TextList[TextNum],"%s %s",MonsterScript[i].Name,GlobalText[103]);
				break;
			}
		}
	}
	else if( (ip->Type==ITEM_HELPER+4) || (ip->Type==ITEM_HELPER+5) )
	{
		sprintf(TextList[TextNum],"%s",p->Name);
	}
    else if ( ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )
             || ip->Type>=ITEM_HELPER+30
			 || ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )
			 || (ip->Type>=ITEM_WING+42 && ip->Type<=ITEM_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			 || (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
            )
    {
		if(Level==0)
			sprintf(TextList[TextNum],"%s",p->Name);
		else
			sprintf(TextList[TextNum],"%s +%d",p->Name,Level);
    }
	else
	{
		if((ip->Option1&63) > 0)
		{
			if(Level==0)
				sprintf(TextList[TextNum],"%s %s", GlobalText[620], p->Name);
			else
				sprintf(TextList[TextNum],"%s %s +%d", GlobalText[620], p->Name,Level);
		}
		else
		{
			if(Level==0)
				sprintf(TextList[TextNum],"%s",p->Name);
			else
				sprintf(TextList[TextNum],"%s +%d",p->Name,Level);
		}
	}
	TextListColor[TextNum] = Color;TextBold[TextNum] = true;TextNum++;
	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	if ( ip->Type<ITEM_POTION )
    {
		if( ip->bPeriodItem == false )
		{
			int maxDurability = calcMaxDurability ( ip, p, Level );
			
			sprintf(TextList[TextNum],GlobalText[71],ip->Durability, maxDurability);
			
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
		}
    }

	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	SIZE TextSize = {0, 0};

	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
	
	int Height = ((TextNum-SkipNum)*TextSize.cy+SkipNum*TextSize.cy/2)*480/WindowHeight;
	if(sy-Height >= 0)
		sy -= Height;
	else
		sy += p->Height*INVENTORY_SCALE;

	RenderTipTextList(sx,sy,TextNum,0);
}

bool GetAttackDamage ( int* iMinDamage, int* iMaxDamage )
{
	int AttackDamageMin;
	int AttackDamageMax;


	ITEM *r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
	ITEM *l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
	if ( PickItem.Number>0 && SrcInventory==Inventory )
	{	
		switch ( SrcInventoryIndex)
		{
		case EQUIPMENT_WEAPON_RIGHT:
			r = &PickItem;
			break;
		case EQUIPMENT_WEAPON_LEFT:
			l = &PickItem;
			break;
		}
	}
	if( gCharacterManager.GetEquipedBowType( ) == BOWTYPE_CROSSBOW )
	{
		AttackDamageMin = CharacterAttribute->AttackDamageMinRight;
		AttackDamageMax = CharacterAttribute->AttackDamageMaxRight;
	}
	else if( gCharacterManager.GetEquipedBowType( ) == BOWTYPE_BOW )
	{
		AttackDamageMin = CharacterAttribute->AttackDamageMinLeft;
		AttackDamageMax = CharacterAttribute->AttackDamageMaxLeft;
	}
	else if(r->Type == -1)
	{
		AttackDamageMin = CharacterAttribute->AttackDamageMinLeft;
		AttackDamageMax = CharacterAttribute->AttackDamageMaxLeft;
	}
	else if(r->Type >= ITEM_STAFF && r->Type < ITEM_SHIELD)
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
	if ( gCharacterManager.GetBaseClass(Hero->Class)==CLASS_KNIGHT || gCharacterManager.GetBaseClass(Hero->Class)==CLASS_DARK )
	{
		if ( l->Type>=ITEM_SWORD && l->Type<ITEM_STAFF+MAX_ITEM_INDEX && r->Type>=ITEM_SWORD && r->Type<ITEM_STAFF+MAX_ITEM_INDEX )
		{
			Alpha = true;
			AttackDamageMin = ((CharacterAttribute->AttackDamageMinRight*55)/100+(CharacterAttribute->AttackDamageMinLeft*55)/100);
			AttackDamageMax = ((CharacterAttribute->AttackDamageMaxRight*55)/100+(CharacterAttribute->AttackDamageMaxLeft*55)/100);
		}
	}
    else if(gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF )
    {
        if ( ( r->Type>=ITEM_BOW && r->Type<ITEM_BOW+MAX_ITEM_INDEX ) &&
             ( l->Type>=ITEM_BOW && l->Type<ITEM_BOW+MAX_ITEM_INDEX ) )
        {
            if ( ( l->Type==ITEM_BOW+7 && ((l->Level>>3)&15)>=1 ) || ( r->Type==ITEM_BOW+15 && ((r->Level>>3)&15)>=1 ) )
            {
                Alpha = true;
            }
        }
    }
#ifdef PBG_ADD_NEWCHAR_MONK
	else if(GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
	{
		if(l->Type>=ITEM_SWORD && l->Type<ITEM_MACE+MAX_ITEM_INDEX && r->Type>=ITEM_SWORD && r->Type<ITEM_MACE+MAX_ITEM_INDEX)
		{
			Alpha = true;
			AttackDamageMin = ((CharacterAttribute->AttackDamageMinRight+CharacterAttribute->AttackDamageMinLeft)*60/100);
			AttackDamageMax = ((CharacterAttribute->AttackDamageMaxRight+CharacterAttribute->AttackDamageMaxLeft)*65/100);
		}
	}
#endif //PBG_ADD_NEWCHAR_MONK

	if ( CharacterAttribute->Ability&ABILITY_PLUS_DAMAGE )
	{
		AttackDamageMin += 15;
		AttackDamageMax += 15;
	}
	

	*iMinDamage = AttackDamageMin;
	*iMaxDamage = AttackDamageMax;

	return Alpha;
}

void RenderSkillInfo(int sx,int sy,int Type,int SkillNum, int iRenderPoint /*= STRP_NONE*/)
{
	char lpszName[256];
	int  iMinDamage, iMaxDamage;
    int  HeroClass = gCharacterManager.GetBaseClass ( Hero->Class );
	int  iMana, iDistance, iSkillMana;
    int  TextNum = 0;
	int  SkipNum = 0;

    if ( giPetManager::RenderPetCmdInfo( sx, sy, Type ) ) return;

	int  AttackDamageMin, AttackDamageMax;
	int  iSkillMinDamage, iSkillMaxDamage;

    int  SkillType = CharacterAttribute->Skill[Type];
    gCharacterManager.GetMagicSkillDamage( CharacterAttribute->Skill[Type], &iMinDamage, &iMaxDamage);
	gCharacterManager.GetSkillDamage( CharacterAttribute->Skill[Type], &iSkillMinDamage, &iSkillMaxDamage );
	
	GetAttackDamage(&AttackDamageMin, &AttackDamageMax);	

    iSkillMinDamage += AttackDamageMin;
	iSkillMaxDamage += AttackDamageMax;
	gSkillManager.GetSkillInformation( CharacterAttribute->Skill[Type], 1, lpszName, &iMana, &iDistance, &iSkillMana);
	
    if ( CharacterAttribute->Skill[Type]==AT_SKILL_STRONG_PIER && Hero->Weapon[0].Type!=-1 )
    {
        for ( int i=0; i<CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].SpecialNum; i++ )
        {
            if ( CharacterMachine->Equipment[0].Special[i]==AT_SKILL_LONG_SPEAR )    
            {
                wsprintf ( lpszName, "%s", GlobalText[1200] );
                break;
            }
        }
    }

	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
   	sprintf(TextList[TextNum],"%s",lpszName);
	TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = true;TextNum++;
	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	WORD Dexterity;
	WORD Energy;
	WORD Strength;
	WORD Vitality;
    WORD Charisma;

	Dexterity	= CharacterAttribute->Dexterity+ CharacterAttribute->AddDexterity;
	Energy		= CharacterAttribute->Energy   + CharacterAttribute->AddEnergy;  
	Strength	= CharacterAttribute->Strength + CharacterAttribute->AddStrength;
	Vitality	= CharacterAttribute->Vitality + CharacterAttribute->AddVitality;
	Charisma	= CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;

	int skillattackpowerRate = 0;

	StrengthenCapability rightinfo, leftinfo;

	ITEM* rightweapon = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
	ITEM* leftweapon  = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];

	int rightlevel = (rightweapon->Level>>3)&15;

	if( rightlevel >= rightweapon->Jewel_Of_Harmony_OptionLevel )
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &rightinfo, rightweapon, 1 );
	}

	int leftlevel = (leftweapon->Level>>3)&15;

	if( leftlevel >= leftweapon->Jewel_Of_Harmony_OptionLevel )
	{
		g_pUIJewelHarmonyinfo->GetStrengthenCapability( &leftinfo, leftweapon, 1 );
	}

	if( rightinfo.SI_isSP )
	{
		skillattackpowerRate += rightinfo.SI_SP.SI_skillattackpower;
		skillattackpowerRate += rightinfo.SI_SP.SI_magicalpower;
	}
	if( leftinfo.SI_isSP )
	{
		skillattackpowerRate += leftinfo.SI_SP.SI_skillattackpower;
	}

	if (HeroClass==CLASS_WIZARD || HeroClass==CLASS_SUMMONER)
	{
        if ( CharacterAttribute->Skill[Type]==AT_SKILL_WIZARDDEFENSE || (AT_SKILL_SOUL_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_SOUL_UP+4))
        {
			int iDamageShield;
			if( CharacterAttribute->Skill[Type]==AT_SKILL_WIZARDDEFENSE )
			{
				iDamageShield = (int)(10+(Dexterity/50.f)+(Energy/200.f));
			}
			else if((AT_SKILL_SOUL_UP <= CharacterAttribute->Skill[Type]) && (CharacterAttribute->Skill[Type] <= AT_SKILL_SOUL_UP+4))
			{
				iDamageShield = (int)(10+(Dexterity/50.f)+(Energy/200.f)) + ((CharacterAttribute->Skill[Type]-AT_SKILL_SOUL_UP+1)*5);		
			}

            int iDeleteMana   = (int)(CharacterAttribute->ManaMax*0.02f);
            int iLimitTime    = (int)(60+(Energy/40.f));

		    sprintf(TextList[TextNum],GlobalText[578],iDamageShield);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;

		    sprintf(TextList[TextNum],GlobalText[880],iDeleteMana);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;

		    sprintf(TextList[TextNum],GlobalText[881],iLimitTime);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        }
		else if( SkillType != AT_SKILL_SWELL_OF_MAGICPOWER && SkillType != AT_SKILL_ALICE_SLEEP	&& !(AT_SKILL_ALICE_SLEEP_UP <= SkillType && SkillType <= AT_SKILL_ALICE_SLEEP_UP + 4 ))
        {
			WORD bySkill = CharacterAttribute->Skill[Type];
			if (!(AT_SKILL_STUN <= bySkill && bySkill <= AT_SKILL_MANA)	&& !(AT_SKILL_ALICE_THORNS <= bySkill && bySkill <= AT_SKILL_ALICE_ENERVATION) && bySkill != AT_SKILL_TELEPORT && bySkill != AT_SKILL_TELEPORT_B)
			{
				if (AT_SKILL_SUMMON_EXPLOSION <= bySkill && bySkill <= AT_SKILL_SUMMON_POLLUTION)
				{
					gCharacterManager.GetCurseSkillDamage(bySkill, &iMinDamage, &iMaxDamage);
					sprintf(TextList[TextNum], GlobalText[1692], iMinDamage, iMaxDamage);
				}
				else
					sprintf(TextList[TextNum],GlobalText[170],iMinDamage + skillattackpowerRate,iMaxDamage + skillattackpowerRate);
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			}
        }
	}
    if ( HeroClass==CLASS_KNIGHT || HeroClass==CLASS_DARK || HeroClass==CLASS_ELF || HeroClass==CLASS_DARK_LORD 
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		|| HeroClass==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		)
    {
        switch ( CharacterAttribute->Skill[Type] )
        {
        case AT_SKILL_TELEPORT :
        case AT_SKILL_TELEPORT_B :
		case AT_SKILL_SOUL_UP:
		case AT_SKILL_SOUL_UP+1:
		case AT_SKILL_SOUL_UP+2:
		case AT_SKILL_SOUL_UP+3:
		case AT_SKILL_SOUL_UP+4:

		case AT_SKILL_HEAL_UP:
		case AT_SKILL_HEAL_UP+1:
		case AT_SKILL_HEAL_UP+2:
		case AT_SKILL_HEAL_UP+3:
		case AT_SKILL_HEAL_UP+4:

		case AT_SKILL_LIFE_UP:
		case AT_SKILL_LIFE_UP+1:
		case AT_SKILL_LIFE_UP+2:
		case AT_SKILL_LIFE_UP+3:
		case AT_SKILL_LIFE_UP+4:

        case AT_SKILL_WIZARDDEFENSE :
        case AT_SKILL_BLOCKING :
        case AT_SKILL_VITALITY :
        case AT_SKILL_HEALING :
		case AT_SKILL_DEF_POWER_UP:
		case AT_SKILL_DEF_POWER_UP+1:
		case AT_SKILL_DEF_POWER_UP+2:
		case AT_SKILL_DEF_POWER_UP+3:
		case AT_SKILL_DEF_POWER_UP+4:
        case AT_SKILL_DEFENSE :
		case AT_SKILL_ATT_POWER_UP:
		case AT_SKILL_ATT_POWER_UP+1:
		case AT_SKILL_ATT_POWER_UP+2:
		case AT_SKILL_ATT_POWER_UP+3:
		case AT_SKILL_ATT_POWER_UP+4:
        case AT_SKILL_ATTACK :
        case AT_SKILL_SUMMON :
        case AT_SKILL_SUMMON+1 :
        case AT_SKILL_SUMMON+2 :
        case AT_SKILL_SUMMON+3 :
        case AT_SKILL_SUMMON+4 :
        case AT_SKILL_SUMMON+5 :
        case AT_SKILL_SUMMON+6 :
		case AT_SKILL_SUMMON+7:
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
            break;
		case AT_SKILL_ASHAKE_UP:
		case AT_SKILL_ASHAKE_UP+1:
		case AT_SKILL_ASHAKE_UP+2:
		case AT_SKILL_ASHAKE_UP+3:
		case AT_SKILL_ASHAKE_UP+4:
        case AT_SKILL_DARK_HORSE:
		    sprintf ( TextList[TextNum], GlobalText[1237] );
		    TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
            break;
        case AT_SKILL_BRAND_OF_SKILL:
            break;
		case AT_SKILL_PLASMA_STORM_FENRIR:
		case AT_SKILL_RECOVER:
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		case AT_SKILL_ATT_UP_OURFORCES:
		case AT_SKILL_HP_UP_OURFORCES:
		case AT_SKILL_DEF_UP_OURFORCES:
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			break;
        default :
		    sprintf(TextList[TextNum],GlobalText[879],iSkillMinDamage,iSkillMaxDamage + skillattackpowerRate);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
            break;
        }
	}

	if(CharacterAttribute->Skill[Type] == AT_SKILL_PLASMA_STORM_FENRIR)
	{
		int iSkillDamage;
		gSkillManager.GetSkillInformation_Damage(AT_SKILL_PLASMA_STORM_FENRIR, &iSkillDamage);

		if(HeroClass == CLASS_KNIGHT || HeroClass == CLASS_DARK)
		{
			iSkillMinDamage = (Strength/3)+(Dexterity/5)+(Vitality/5)+(Energy/7)+iSkillDamage;
		}
		else if(HeroClass == CLASS_WIZARD || HeroClass == CLASS_SUMMONER)
		{
			iSkillMinDamage = (Strength/5)+(Dexterity/5)+(Vitality/7)+(Energy/3)+iSkillDamage;
		}
		else if(HeroClass == CLASS_ELF)
		{
			iSkillMinDamage = (Strength/5)+(Dexterity/3)+(Vitality/7)+(Energy/5)+iSkillDamage;
		}
		else if(HeroClass == CLASS_DARK_LORD)
		{
			iSkillMinDamage = (Strength/5)+(Dexterity/5)+(Vitality/7)+(Energy/3)+(Charisma/3)+iSkillDamage;
		}
#ifdef PBG_ADD_NEWCHAR_MONK
		else if(HeroClass == CLASS_RAGEFIGHTER)
		{
			iSkillMinDamage = (Strength/5)+(Dexterity/5)+(Vitality/3)+(Energy/7)+iSkillDamage;
		}
#endif //PBG_ADD_NEWCHAR_MONK
		iSkillMaxDamage = iSkillMinDamage + 30;
		
		sprintf(TextList[TextNum],GlobalText[879],iSkillMinDamage,iSkillMaxDamage + skillattackpowerRate);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}

    if(gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
	{
		bool Success = true;
		switch(CharacterAttribute->Skill[Type])
		{
		case AT_SKILL_HEAL_UP:
		case AT_SKILL_HEAL_UP+1:
		case AT_SKILL_HEAL_UP+2:
		case AT_SKILL_HEAL_UP+3:
		case AT_SKILL_HEAL_UP+4:
		{
			int Cal = (Energy/5)+5;
			sprintf(TextList[TextNum],GlobalText[171],(Cal) + (int)((float)Cal*(float)(SkillAttribute[CharacterAttribute->Skill[Type]].Damage/(float)100)));
		}
		break;
	case AT_SKILL_HEALING:
		sprintf(TextList[TextNum],GlobalText[171],Energy/5+5);
		break;
	case AT_SKILL_DEF_POWER_UP:
	case AT_SKILL_DEF_POWER_UP+1:
	case AT_SKILL_DEF_POWER_UP+2:
	case AT_SKILL_DEF_POWER_UP+3:
	case AT_SKILL_DEF_POWER_UP+4:
		{
			int Cal = Energy/8+2;
			sprintf(TextList[TextNum],GlobalText[172],(Cal) + (int)((float)Cal/(float)((float)SkillAttribute[CharacterAttribute->Skill[Type]].Damage/(float)10)));
		}
		break;
	case AT_SKILL_DEFENSE:sprintf(TextList[TextNum],GlobalText[172],Energy/8+2);break;
	case AT_SKILL_ATT_POWER_UP:
	case AT_SKILL_ATT_POWER_UP+1:
	case AT_SKILL_ATT_POWER_UP+2:
	case AT_SKILL_ATT_POWER_UP+3:
	case AT_SKILL_ATT_POWER_UP+4:
		{
			int Cal = Energy/7+3;
			sprintf(TextList[TextNum],GlobalText[173],(Cal) + (int)((float)Cal/(float)((float)SkillAttribute[CharacterAttribute->Skill[Type]].Damage/(float)10)));
		}
	break;
		case AT_SKILL_ATTACK :sprintf(TextList[TextNum],GlobalText[173],Energy/7+3);break;
		case AT_SKILL_RECOVER:
			{
				int Cal = Energy/4;
				sprintf(TextList[TextNum],GlobalText[1782], (int)((float)Cal+(float)CharacterAttribute->Level));
			}
		break;
		default:Success = false;
		}
		if(Success)
		{
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
	}

	if( SkillType != AT_SKILL_SWELL_OF_MAGICPOWER )
	{
		if(iDistance)
		{
			sprintf(TextList[TextNum],GlobalText[174],iDistance);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
	}

   	sprintf(TextList[TextNum],GlobalText[175],iMana);
	TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	if ( iSkillMana > 0)
	{
   		sprintf(TextList[TextNum],GlobalText[360],iSkillMana);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
    if ( gCharacterManager.GetBaseClass(Hero->Class) == CLASS_KNIGHT )
    {
        if ( CharacterAttribute->Skill[Type]==AT_SKILL_SPEAR )
        {
            sprintf(TextList[TextNum],GlobalText[96] );
		    TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
        }

		if ( Hero->byExtensionSkill == 1 && CharacterAttribute->Level >= 220 )
		{
			if ( ( CharacterAttribute->Skill[Type] >= AT_SKILL_SWORD1 && CharacterAttribute->Skill[Type] <= AT_SKILL_SWORD5 ) 
				|| CharacterAttribute->Skill[Type]==AT_SKILL_WHEEL || CharacterAttribute->Skill[Type]==AT_SKILL_FURY_STRIKE 
				|| CharacterAttribute->Skill[Type]==AT_SKILL_ONETOONE 
				|| (AT_SKILL_ANGER_SWORD_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_ANGER_SWORD_UP+4)
				|| (AT_SKILL_BLOW_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_BLOW_UP+4)
				|| (AT_SKILL_TORNADO_SWORDA_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_TORNADO_SWORDA_UP+4)
				|| (AT_SKILL_TORNADO_SWORDB_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_TORNADO_SWORDB_UP+4)
				   )
			{
				sprintf(TextList[TextNum], GlobalText[99]);
				TextListColor[TextNum] = TEXT_COLOR_DARKRED;
				TextBold[TextNum] = false;
				TextNum++;
			}
			else if(CharacterAttribute->Skill[Type] == AT_SKILL_BLOW_OF_DESTRUCTION)
			{
				sprintf(TextList[TextNum], GlobalText[2115] );
				TextListColor[TextNum] = TEXT_COLOR_DARKRED;
				TextBold[TextNum] = false;
				TextNum++;
			}
		}
    }

    BYTE MasteryType = gSkillManager.GetSkillMasteryType( CharacterAttribute->Skill[Type] );
    if ( MasteryType!=255 )
    {
        sprintf ( TextList[TextNum], GlobalText[1080+MasteryType] );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = false;TextNum++;
    }

    int SkillUseType;
    int BrandType = SkillAttribute[SkillType].SkillBrand;
    SkillUseType = SkillAttribute[SkillType].SkillUseType;
    if ( SkillUseType==SKILL_USE_TYPE_BRAND )
    {
		sprintf ( TextList[TextNum], GlobalText[1480], SkillAttribute[BrandType].Name );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
        sprintf ( TextList[TextNum], GlobalText[1481], SkillAttribute[BrandType].Damage );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
    }
    SkillUseType = SkillAttribute[SkillType].SkillUseType;
    if ( SkillUseType==SKILL_USE_TYPE_MASTER )
    {
        sprintf ( TextList[TextNum], GlobalText[1482] );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
        sprintf ( TextList[TextNum], GlobalText[1483], SkillAttribute[SkillType].KillCount );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
    }

    if ( gCharacterManager.GetBaseClass(Hero->Class)==CLASS_DARK_LORD )
    {
        if ( CharacterAttribute->Skill[Type]==AT_SKILL_PARTY_TELEPORT && PartyNumber<=0 )
        {
		    sprintf ( TextList[TextNum], GlobalText[1185] );
		    TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
        }
	}

	if(CharacterAttribute->Skill[Type] == AT_SKILL_PLASMA_STORM_FENRIR)
	{
		sprintf ( TextList[TextNum], GlobalText[1926] );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
		sprintf ( TextList[TextNum], GlobalText[1927] );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;	
	}

	if(CharacterAttribute->Skill[Type] == AT_SKILL_INFINITY_ARROW)
	{
		sprintf(TextList[1],lpszName);
		TextListColor[1] = TEXT_COLOR_BLUE;TextBold[1] = true;
		sprintf(TextList[2],"\n");
		sprintf(TextList[3],GlobalText[2040]);
		TextListColor[3] = TEXT_COLOR_DARKRED;TextBold[3] = false;
		sprintf(TextList[4],GlobalText[175],iMana);
		TextListColor[4] = TEXT_COLOR_WHITE;TextBold[4] = false;
		sprintf(TextList[5],GlobalText[360],iSkillMana);
		TextListColor[5] = TEXT_COLOR_WHITE;TextBold[5] = false;
		TextNum = 6; SkipNum = 2;
	}

	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	if(CharacterAttribute->Skill[Type] == AT_SKILL_RUSH || CharacterAttribute->Skill[Type] == AT_SKILL_SPACE_SPLIT
		|| CharacterAttribute->Skill[Type] == AT_SKILL_DEEPIMPACT || CharacterAttribute->Skill[Type] == AT_SKILL_JAVELIN
		|| CharacterAttribute->Skill[Type] == AT_SKILL_ONEFLASH || CharacterAttribute->Skill[Type] == AT_SKILL_DEATH_CANNON
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		|| CharacterAttribute->Skill[Type] == AT_SKILL_OCCUPY
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		)
	{
		sprintf ( TextList[TextNum], GlobalText[2047] );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
	}
	if(CharacterAttribute->Skill[Type] == AT_SKILL_STUN || CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_STUN
		|| CharacterAttribute->Skill[Type] == AT_SKILL_INVISIBLE || CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_INVISIBLE
		|| CharacterAttribute->Skill[Type] == AT_SKILL_REMOVAL_BUFF)
	{
		sprintf ( TextList[TextNum], GlobalText[2048] );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
	}
	if(CharacterAttribute->Skill[Type] == AT_SKILL_SPEAR)
	{
		sprintf ( TextList[TextNum], GlobalText[2049] );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
	}

	if( SkillType == AT_SKILL_SWELL_OF_MAGICPOWER )
	{
		sprintf ( TextList[TextNum], GlobalText[2054] );
		TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
	}

	SIZE TextSize = {0, 0};	
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
	
	if( iRenderPoint == STRP_NONE )
	{
		int Height = ((TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2) / g_fScreenRate_y;
		sy -= Height;	
	}
	
	RenderTipTextList(sx,sy,TextNum,0, RT3_SORT_CENTER, iRenderPoint);
}

void RenderItemName(int i,OBJECT *o,int ItemLevel,int ItemOption,int ItemExtOption,bool Sort)
{
	char Name[80];

	int Level = (ItemLevel>>3)&15;
	
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 255);

	bool bFirstOK = true;

	if(o->Type==MODEL_POTION+15)
	{
		glColor3f(1.f,0.8f,0.1f);
       	sprintf(Name,"%s %d",ItemAttribute[o->Type-MODEL_ITEM].Name,ItemLevel);
	}
	else if( (o->Type==MODEL_POTION+13) 
		  || (o->Type==MODEL_POTION+14)
		  || (o->Type==MODEL_POTION+16)
		  || (o->Type==MODEL_WING+15)
		  || (o->Type==MODEL_POTION+22)
		  || (o->Type==MODEL_POTION+31)
		  || (o->Type==MODEL_HELPER+14)
		  || (o->Type==MODEL_POTION+41)
		  || (o->Type==MODEL_POTION+42)
		  || (o->Type==MODEL_POTION+43)
		  || (o->Type==MODEL_POTION+44)
		   )
	{
		g_pRenderText->SetFont(g_hFontBold);
		glColor3f(1.f,0.8f,0.1f);
       	sprintf(Name,"%s",ItemAttribute[o->Type-MODEL_ITEM].Name);
	}
	else if( o->Type == MODEL_WING+11)
	{
		glColor3f(0.7f,0.7f,0.7f);
		sprintf( Name,"%s %s", SkillAttribute[ 30 + Level ].Name, GlobalText[102] );
	}
	else if(o->Type == MODEL_HELPER+46)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+46].Name);	
	}
	else if(o->Type == MODEL_HELPER+47)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+47].Name);
	}
	else if(o->Type == MODEL_HELPER+48)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+48].Name);
	}
	else if(o->Type == MODEL_POTION+54)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+54].Name);
	}
	else if(o->Type >= MODEL_POTION+58 && o->Type <= MODEL_POTION+62)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type >= MODEL_POTION+145 && o->Type <= MODEL_POTION+150)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type == MODEL_HELPER+125)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+125].Name);	
	}
	else if(o->Type == MODEL_HELPER+126)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+126].Name);
	}
	else if(o->Type == MODEL_HELPER+127)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+127].Name);
	}
	else if( o->Type == MODEL_POTION+53 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+53].Name);
	}
	else if( o->Type == MODEL_HELPER+43 || o->Type == MODEL_HELPER+44 || o->Type == MODEL_HELPER+45 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_HELPER].Name);
	}
	else if(o->Type >= ITEM_POTION+70 && o->Type <= ITEM_POTION+71)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type >= ITEM_POTION+72 && o->Type <= ITEM_POTION+77)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type == ITEM_HELPER+59)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if( o->Type >= ITEM_HELPER+54 && o->Type <= ITEM_HELPER+58)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type >= ITEM_POTION+78 && o->Type <= ITEM_POTION+82)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type == ITEM_HELPER+60)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type == ITEM_HELPER+61)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type == MODEL_POTION+83)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if( o->Type == MODEL_HELPER+43 || o->Type == MODEL_HELPER+44 || o->Type == MODEL_HELPER+45 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_HELPER].Name);
	}
	else if(o->Type == MODEL_POTION+91)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+91].Name);
	}
	else if(o->Type == MODEL_POTION+92)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+92].Name);
	}
	else if(o->Type == MODEL_POTION+93)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+93].Name);
	}
	else if(o->Type == MODEL_POTION+95)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+95].Name);
	}
	else if( o->Type == MODEL_POTION + 94 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+94].Name);
	}
	else if( o->Type==MODEL_POTION+84 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+84].Name);
	}
	else if( o->Type==MODEL_POTION+85 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+85].Name);
	}
	else if( o->Type==MODEL_POTION+86 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+86].Name);
	}
	else if( o->Type==MODEL_POTION+87 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+87].Name);
	}
	else if( o->Type==MODEL_POTION+88 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+88].Name);
	}
	else if( o->Type==MODEL_POTION+89 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+89].Name);
	}
	else if( o->Type==MODEL_POTION+90 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+90].Name);
	}
	else if( o->Type == MODEL_HELPER+62 ) {
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+62].Name);
	}
	else if( o->Type == MODEL_HELPER+63 ) {
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+63].Name);
	}
	else if( o->Type == MODEL_POTION+97 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+97].Name);
	}
	else if( o->Type == MODEL_POTION+98 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+98].Name);
	}
	else if( o->Type == MODEL_POTION+96 ) 
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+96].Name);
	}
	else if( o->Type == MODEL_HELPER+64 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+64].Name);
	}
	else if( o->Type == MODEL_HELPER+65 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+65].Name);
	}
	else if( o->Type == MODEL_HELPER+67 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+67].Name);
	}
	else if( o->Type == MODEL_HELPER+80 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+80].Name);
	}
	else if( o->Type == MODEL_HELPER+106 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+106].Name);
	}
	else if( o->Type == MODEL_HELPER+123 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+123].Name);
	}
	else if( o->Type == MODEL_HELPER+68 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+68].Name);
	}
	else if( o->Type == MODEL_HELPER+76 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+76].Name);
	}
	else if( o->Type == MODEL_HELPER+122 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+122].Name);
	}
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM

	else if( o->Type == MODEL_HELPER+128 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+128].Name);
	}
	else if( o->Type == MODEL_HELPER+129 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+129].Name);
	}
	else if( o->Type == MODEL_HELPER+134 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+134].Name);
	}
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	else if( o->Type == MODEL_HELPER+130 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+130].Name);
	}
	else if( o->Type == MODEL_HELPER+131 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+131].Name);
	}
	else if( o->Type == MODEL_HELPER+132 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+132].Name);
	}
	else if( o->Type == MODEL_HELPER+133 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+133].Name);
	}
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	else if ( COMGEM::NOGEM != COMGEM::Check_Jewel_Com(o->Type, true) )
	{
		int iJewelItemIndex = COMGEM::GetJewelIndex(COMGEM::Check_Jewel_Com(o->Type, true), COMGEM::eGEM_NAME);
		g_pRenderText->SetFont(g_hFontBold);
		glColor3f(1.f,0.8f,0.1f);
		sprintf(Name,"%s",GlobalText[iJewelItemIndex]);
	}
	else if(o->Type==MODEL_COMPILED_CELE)
	{
		g_pRenderText->SetFont(g_hFontBold);
		glColor3f(1.f,0.8f,0.1f);
		sprintf(Name,"%s",ItemAttribute[MODEL_POTION+13-MODEL_ITEM].Name);
	}
	else if(o->Type==MODEL_COMPILED_SOUL)
	{
		g_pRenderText->SetFont(g_hFontBold);
		glColor3f(1.f,0.8f,0.1f);
		sprintf(Name,"%s",ItemAttribute[MODEL_POTION+14-MODEL_ITEM].Name);		
	}
	else if(o->Type==MODEL_POTION+17 || o->Type==MODEL_POTION+18 || o->Type==MODEL_POTION+19)
	{	
		g_pRenderText->SetFont(g_hFontBold);
		glColor3f(1.f,0.8f,0.1f);
		if ( ((ItemLevel>>3)&15) == 0)
		{
			sprintf(Name,"%s",ItemAttribute[o->Type-MODEL_ITEM].Name);
		}
		else
		{
       		sprintf(Name,"%s +%d",ItemAttribute[o->Type-MODEL_ITEM].Name,((ItemLevel>>3)&15));
		}
	}
    else if(o->Type == MODEL_POTION+11 && Level==7 )
    {
		glColor3f(1.f,0.8f,0.1f);
        sprintf (Name, GlobalText[111]);
    }
	else if(o->Type == MODEL_POTION+12)
	{
		switch(Level)
		{
		case 0:sprintf(Name,GlobalText[100]);break;
		case 1:sprintf(Name,GlobalText[101]);break;
		case 2:sprintf(Name,GlobalText[104]);break;
		}
	}
    else if ( o->Type==MODEL_HELPER+15 )
    {
		glColor3f(1.f,0.8f,0.1f);
        switch ( Level )
        {
        case 0:sprintf(Name,"%s %s", GlobalText[168], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
        case 1:sprintf(Name,"%s %s", GlobalText[169], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
        case 2:sprintf(Name,"%s %s", GlobalText[167], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
        case 3:sprintf(Name,"%s %s", GlobalText[166], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
		case 4:sprintf(Name,"%s %s", GlobalText[1900], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
        }
    }
    else if ( o->Type==MODEL_HELPER+31 )
    {
		glColor3f(1.f,0.8f,0.1f);
        switch ( Level )
        {
		case 0:sprintf ( Name, "%s of %s", ItemAttribute[o->Type-MODEL_ITEM].Name, GlobalText[1187] ); break;
        case 1:sprintf ( Name, "%s of %s", ItemAttribute[o->Type-MODEL_ITEM].Name, GlobalText[1214] ); break;
        }
    }
	else if ( o->Type==MODEL_EVENT+16 )
	{
		glColor3f(1.f,0.8f,0.1f);
		sprintf ( Name, GlobalText[1235] );
	}
	else if(o->Type == MODEL_EVENT+4)
	{
		sprintf(Name,GlobalText[105]);
	}
	else if(o->Type == MODEL_EVENT+5)
	{
    	glColor3f(1.f,0.8f,0.1f);
        switch ( Level )
        {
        case 14:
	    	sprintf(Name,GlobalText[1650]);
            break;

        case 15:
	    	sprintf(Name,GlobalText[1651]);
            break;

        default:
		    sprintf(Name,GlobalText[106]);
            break;
        }
	}
	else if(o->Type == MODEL_EVENT+6)
	{
		if (Level == 13)
		{
			glColor3f ( 1.f, 0.8f, 0.1f );
			sprintf(Name,"%s",GlobalText[117]);
		}
		else
		{
			sprintf(Name,GlobalText[107]);
		}
	}
	else if(o->Type == MODEL_EVENT+7)
	{
		sprintf(Name,GlobalText[108]);
	}
	else if(o->Type == MODEL_EVENT+8)
	{
		sprintf(Name,GlobalText[109]);
	}
	else if(o->Type == MODEL_EVENT+9)
	{
		sprintf(Name,GlobalText[110]);
	}
	else if(o->Type == MODEL_EVENT+10)
	{
		sprintf(Name,"%s +%d",GlobalText[115], Level - 7);
	}
	else if(o->Type == MODEL_WING+32)
	{
		glColor3f ( 1.f, 0.3f, 0.3f );
		sprintf(Name, ItemAttribute[ITEM_WING+32].Name);	
	}
	else if(o->Type == MODEL_WING+33)
	{
		glColor3f ( 0.3f, 1.0f, 0.3f );
		sprintf(Name, ItemAttribute[ITEM_WING+33].Name);	
	}
	else if(o->Type == MODEL_WING+34)
	{
		glColor3f ( 0.3f, 0.3f, 1.f );
		sprintf(Name, ItemAttribute[ITEM_WING+34].Name);	
	}
	else if(o->Type == ITEM_HELPER+49)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+49].Name);
	}
	else if(o->Type == ITEM_HELPER+50)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+50].Name);
	}
	else if(o->Type == ITEM_HELPER+51)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+51].Name);
	}
	else if(o->Type == MODEL_POTION+64)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_POTION+64].Name);
	}
	else if(o->Type == MODEL_WING+48)//
	{
		glColor3f ( 1.f, 1.f, 1.f );
		sprintf(Name, ItemAttribute[ITEM_WING+48].Name);	
	}
	else if(o->Type == MODEL_WING+35)
	{
		glColor3f ( 1.f, 1.f, 1.f );
		sprintf(Name, ItemAttribute[ITEM_WING+35].Name);	
	}
	else if(o->Type == MODEL_POTION+45)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+45].Name);	
	}
	else if(o->Type == MODEL_POTION+46)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+46].Name);	
	}
	else if(o->Type == MODEL_POTION+47)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+47].Name);	
	}
	else if(o->Type == MODEL_POTION+48)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+48].Name);		
	}
	else if(o->Type == MODEL_POTION+49)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+49].Name);	
	}
	else if(o->Type == MODEL_POTION+50)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+50].Name);		
	}
	else if(o->Type == MODEL_POTION+32)
	{
		int i = MODEL_POTION+32;
		int k = ITEM_POTION+32;
		if(Level == 0)
		{
			glColor3f ( 1.f, 0.3f, 1.f );
			sprintf(Name, ItemAttribute[ITEM_POTION+32].Name);	
		}
		else
		if(Level == 1)
		{
			glColor3f ( 1.f, 0.3f, 1.f );
			sprintf(Name, GlobalText[2012]);	
		}
	}
	else if(o->Type == MODEL_POTION+33)
	{
		if(Level == 0)
		{
			glColor3f ( 1.0f, 0.3f, 0.3f );
			sprintf(Name, ItemAttribute[ITEM_POTION+33].Name);	
		}
		else
		if(Level == 1)
		{
			glColor3f ( 1.0f, 0.3f, 0.3f );
			sprintf(Name, GlobalText[2013]);	
		}
	}
	else if(o->Type == MODEL_POTION+34)
	{
		if(Level == 0)
		{
			glColor3f ( 0.3f, 0.3f, 1.f );
			sprintf(Name, ItemAttribute[ITEM_POTION+34].Name);	
		}
		else
		if(Level == 1)
		{
			glColor3f ( 0.3f, 0.3f, 1.f );
			sprintf(Name, GlobalText[2014]);	
		}
	}
	else if(o->Type == MODEL_EVENT+21)
	{
		glColor3f ( 1.f, 0.3f, 1.f );
		sprintf(Name, GlobalText[2012]);	
	}
	else if(o->Type == MODEL_EVENT+22)
	{
		glColor3f ( 1.0f, 0.3f, 0.3f );
		sprintf(Name, GlobalText[2013]);	
	}
	else if(o->Type == MODEL_EVENT+23)
	{
		glColor3f ( 0.3f, 0.3f, 1.f );
		sprintf(Name, GlobalText[2014]);	
	}
    else if ( o->Type==MODEL_EVENT+11 )
    {
        glColor3f ( 1.f, 0.8f, 0.1f );
        {
            sprintf ( Name, GlobalText[810] );
        }
    }
	else if(o->Type==MODEL_EVENT+12)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf ( Name, GlobalText[906] );
	}
	else if(o->Type==MODEL_EVENT+13)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf ( Name, GlobalText[907] );
	}
	else if(o->Type==MODEL_EVENT+14)
	{
		glColor3f( 1.f, 0.8f, 0.1f );
        switch ( Level )
        {
        case 2:
		    sprintf( Name, GlobalText[928] );
            break;
        case 3:
		    sprintf( Name, GlobalText[929] );
            break;
        default :
		    sprintf( Name, GlobalText[922] );
            break;
        }
	}
	else if(o->Type==MODEL_EVENT+15)
	{
		glColor3f( 1.f, 0.8f, 0.1f );
		sprintf( Name, GlobalText[925] );
	}
	else if(o->Type == MODEL_WING+11)
	{
		sprintf(Name,"%s %s",SkillAttribute[30+Level].Name,GlobalText[102]);
	}
    else if(o->Type == MODEL_HELPER+10)
	{
		for(int i=0;i<MAX_MONSTER;i++)
		{
			if(SommonTable[Level] == MonsterScript[i].Type)
			{
        		sprintf(Name,"%s %s",MonsterScript[i].Name,GlobalText[103]);
				break;
			}
		}
	}
    else if ( o->Type==MODEL_POTION+21 && Level == 3 )
    {
	    glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf( Name, GlobalText[1290] );
    }
    else if ( o->Type==MODEL_POTION+7 )
    {
        switch ( Level )
        {
        case 0: sprintf( Name, GlobalText[1413] ); break;
        case 1: sprintf( Name, GlobalText[1414] ); break;
        }
    }
    else if ( o->Type==MODEL_HELPER+7 )
    {
        switch ( Level )
        {
        case 0: sprintf( Name, GlobalText[1460] ); break;
        case 1: sprintf( Name, GlobalText[1461] ); break;
        }
    }
    else if ( o->Type==MODEL_HELPER+11 )
    {
        switch ( Level )
        {
        case 0: sprintf( Name, GlobalText[1416] ); break;
        case 1: sprintf( Name, GlobalText[1462] ); break;
        }
    }
    else if ( o->Type==MODEL_EVENT+18 )
    {
        sprintf( Name, GlobalText[1462] );
    }
	else
	{
		bFirstOK = false;
	}

	if( bFirstOK == false )
	{
		if ((o->Type >= MODEL_WING+60 && o->Type <= MODEL_WING+65)
			|| (o->Type >= MODEL_WING+70 && o->Type <= MODEL_WING+74)
		|| (o->Type >= MODEL_WING+100 && o->Type <= MODEL_WING+129))
	{
		glColor3f(0.7f,0.4f,1.0f);	// TEXT_COLOR_VIOLET
		strcpy ( Name, ItemAttribute[o->Type-MODEL_ITEM].Name );
	}
	else if( o->Type == MODEL_HELPER+66 )
	{
		glColor3f( 0.6f, 0.4f, 1.0f);
		sprintf(Name, ItemAttribute[ITEM_HELPER+66].Name);
	}
	else if( o->Type == MODEL_POTION+100 )
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[ITEM_POTION+100].Name);
	}
	else if(o->Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN 
			&& o->Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
	else if(o->Type == MODEL_HELPER+97 || o->Type == MODEL_HELPER+98 || o->Type == MODEL_POTION+91)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[o->Type].Name);
	}
	else if(o->Type == MODEL_HELPER+99)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[ITEM_HELPER+99].Name);
	}
	else if(o->Type == MODEL_HELPER+109)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+109].Name);
	}
	else if(o->Type == MODEL_HELPER+110)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+110].Name);
	}
	else if(o->Type == MODEL_HELPER+111)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+111].Name);
	}
	else if(o->Type == MODEL_HELPER+112)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+112].Name);
	}
	else if(o->Type == MODEL_HELPER+113)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+113].Name);
	}
	else if(o->Type == MODEL_HELPER+114)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+114].Name);
	}
	else if(o->Type == MODEL_HELPER+115)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+115].Name);
	}
	else if(o->Type == MODEL_POTION+112)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_POTION+112].Name);
	}
	else if(o->Type == MODEL_POTION+113)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_POTION+113].Name);
	}
	else
	{
        if ( o->Type==MODEL_STAFF+10 || o->Type==MODEL_SWORD+19 || o->Type==MODEL_BOW+18 || o->Type==MODEL_MACE+13)
        {
            glColor3f(1.f,0.1f,1.f);
        }
		else if (g_SocketItemMgr.IsSocketItem(o))
		{
			glColor3f(0.7f,0.4f,1.0f);	// TEXT_COLOR_VIOLET
		}
        else if((ItemOption&63) > 0 && ( o->Type<MODEL_WING+3 || o->Type>MODEL_WING+6 ) && o->Type!=MODEL_HELPER+30 
			&& ( o->Type<MODEL_WING+36 || o->Type>MODEL_WING+40 )
			&& (o->Type<MODEL_WING+42 || o->Type>MODEL_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			&& !(o->Type>=MODEL_WING+49 && o->Type<=MODEL_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			) 
		{
			glColor3f(0.1f,1.f,0.5f);
		}
		else if( MODEL_POTION+101 <= o->Type && o->Type <= MODEL_POTION+109 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if (o->Type == MODEL_POTION+111)
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if(o->Type == MODEL_POTION+120)
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if(o->Type == MODEL_POTION+121)
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if(o->Type == MODEL_POTION+122)
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if(o->Type == MODEL_POTION+123 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if(o->Type == MODEL_POTION+124 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if( MODEL_POTION+134 <= o->Type && o->Type <= MODEL_POTION+139 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if( o->Type == MODEL_HELPER+116 )
		{
			glColor3f ( 0.9f, 0.53f, 0.13f );
			sprintf(Name, ItemAttribute[o->Type].Name);
		}

		else if( ITEM_WING+130 <= o->Type && 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			o->Type <= ITEM_WING+135
#else //PBG_ADD_NEWCHAR_MONK_ITEM
			o->Type <= ITEM_WING+134 
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			)
		{
			glColor3f ( 0.9f, 0.53f, 0.13f );
			sprintf(Name, ItemAttribute[o->Type].Name);
		}

		else if( MODEL_POTION+114 <= o->Type && o->Type <= MODEL_POTION+119 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if( MODEL_POTION+126 <= o->Type && o->Type <= MODEL_POTION+129 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if( MODEL_POTION+130 <= o->Type && o->Type <= MODEL_POTION+132 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if( MODEL_HELPER+121==o->Type )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if(o->Type>=MODEL_POTION+141 && o->Type<=MODEL_POTION+144)
		{
		   	glColor3f (0.9f, 0.53f, 0.13f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
		else if ( o->Type==MODEL_WING+25 )
		{
		    glColor3f ( 1.f, 0.8f, 0.1f );
		}
        else if ( o->Type==MODEL_POTION+28 || o->Type==MODEL_POTION+29 )
        {
		    glColor3f ( 1.f, 0.8f, 0.1f );
        }
		else if(Level >= 7)
		{
			glColor3f(1.f,0.8f,0.1f);
		}
		else if(((ItemLevel>>7)&1) || ((ItemLevel>>2)&1) || ((ItemLevel>>1)&1) || (ItemLevel&1))
		{
			glColor3f(0.4f,0.7f,1.f);
		}
		else if(Level == 0)
		{
			glColor3f(0.7f,0.7f,0.7f);
		}
		else if(Level < 3)
		{
			glColor3f(0.9f,0.9f,0.9f);
		}
		else if(Level < 5)
		{
			glColor3f(1.f,0.5f,0.2f);
		}
		else if(Level < 7)
		{
			glColor3f(0.4f,0.7f,1.f);
		}

		char TextName[64];
		if ( g_csItemOption.GetSetItemName( TextName, o->Type-MODEL_ITEM, ItemExtOption ) )
		{
            glColor3f ( 1.f, 1.f, 1.f );
			g_pRenderText->SetFont(g_hFontBold);
			g_pRenderText->SetTextColor(0, 255, 0, 255);
			g_pRenderText->SetBgColor(60, 60, 200, 255);

			strcat ( TextName, ItemAttribute[o->Type-MODEL_ITEM].Name );
		}
		else
		{
			strcpy ( TextName, ItemAttribute[o->Type-MODEL_ITEM].Name );
		}

		if(Level == 0)
			sprintf(Name,"%s",TextName);
		else
			sprintf(Name,"%s +%d",TextName,Level);

		if((ItemLevel>>7)&1)
		{
			if(o->Type!=MODEL_HELPER+3)
    			strcat(Name,GlobalText[176]);
			else
            {
                strcat(Name, " +" );
    			strcat(Name,GlobalText[179]);
            }
		}
		if( (ItemLevel&3) || ((ItemOption>>6)&1) )
			strcat(Name,GlobalText[177]);
		if((ItemLevel>>2)&1)
			strcat(Name,GlobalText[178]);
	}
	}

	GLfloat fCurColor[4] = { 1.f, 1.f, 1.f, 0.0f };
	glGetFloatv(GL_CURRENT_COLOR, fCurColor);
	if(fCurColor[0] < 0.9f || fCurColor[1] < 0.9f || fCurColor[2] < 0.9f)
		g_pRenderText->SetTextColor(fCurColor[0]*255, fCurColor[1]*255, fCurColor[2]*255, 255);
	
	if(!Sort)
	{
    	g_pRenderText->RenderText(MouseX,MouseY-15,Name,0, 0, RT3_WRITE_CENTER);
	}
	else
	{
    	g_pRenderText->RenderText(o->ScreenX,o->ScreenY-15,Name,0, 0, RT3_WRITE_CENTER);
	}

	g_pRenderText->SetTextColor(255, 230, 200, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 255);
}

int GetScreenWidth()
{
	int iWidth = 0;

	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
		&& (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION)
#ifdef LEM_ADD_LUCKYITEM
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
		))
	{
		iWidth = 640 - (190 * 2);
	}
	else if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
		&& (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST)
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
		)
	{
     	iWidth = 640 - (190 * 2);
	}
	else if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER) 
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PET)
		)
	{
        iWidth = 640 - (190 * 2);
	}
	else if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_REFINERY) )
	{
		iWidth = 640 - (190 * 2);
	}
	else if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY) 
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
	for(int i=0;i<MAX_EQUIPMENT;i++)
	{
		CharacterMachine->Equipment[i].Type = -1;
		CharacterMachine->Equipment[i].Number = 0;
	}
	for(int i=0;i<MAX_INVENTORY;i++)
	{
		Inventory[i].Type = -1;
		Inventory[i].Number = 0;
	}
	for(int i=0;i<MAX_SHOP_INVENTORY;i++)
	{
		ShopInventory[i].Type = -1;
		ShopInventory[i].Number = 0;
	}

	COMGEM::Init();
}

void SetItemColor ( int index, ITEM* Inv, int color )
{
	int Width  = ItemAttribute[Inv[index].Type].Width;
	int Height = ItemAttribute[Inv[index].Type].Height;

    for(int k=Inv[index].y;k<Inv[index].y+Height;k++)
	{
		for(int l=Inv[index].x;l<Inv[index].x+Width;l++)
		{
			int Number = k*COLUMN_TRADE_INVENTORY+l;
            Inv[Number].Color = color;
		}
	}
}

int CompareItem ( ITEM item1, ITEM item2 )
{
    int equal = 0;
    if ( item1.Type!=item2.Type )
    {
        return  2;
    }
    else
    if ( item1.Class==item2.Class && item1.Type==item2.Type )
    {
        int level1 = (item1.Level>>3)&15;
        int level2 = (item2.Level>>3)&15;
        int option1= (item1.Option1&63);
        int option2= (item2.Option1&63);
        int skill1 = (item1.Level>>7)&1;
        int skill2 = (item2.Level>>7)&1;

        equal = 1;
        if( level1==level2 )
        {
            equal = 0;
        }
        else
        if( level1<level2 )
        {
            equal = -1;
        }
        if ( equal==0 )
        {
            if ( skill1<skill2 )
            {
                equal = -1;
            }
            else if ( skill1>skill2 )
            {
                equal = 1;
            }
        }
        if ( equal==0 )
        {
            if ( option1<option2 )
            {
                equal = -1;
            }
            else if ( option1>option2 )
            {
                equal = 1;
            }
        }
        if ( equal==0 )
        {
            if ( item1.SpecialNum<item2.SpecialNum )
            {
                equal = -1;
            }
            else if ( item1.SpecialNum>item2.SpecialNum )
            {
                equal = 1;
            }
            else
            {
                int     Num = max ( item1.SpecialNum, item2.SpecialNum );
                int     addOption1 = 0;
                int     addOptionV1= 0;
                int     addOption2 = 0;
                int     addOptionV2= 0;
                for( int i=0; i<Num; ++i )
                {
		            switch(item1.Special[i])
		            {
		            case AT_IMPROVE_DAMAGE:
		            case AT_IMPROVE_MAGIC:
					case AT_IMPROVE_CURSE:
		            case AT_IMPROVE_BLOCKING:
		            case AT_IMPROVE_DEFENSE:
			            addOption1 = 1;
                        addOptionV1= item1.SpecialValue[i];
			            break;
		            }
		            switch(item2.Special[i])
		            {
		            case AT_IMPROVE_DAMAGE:
		            case AT_IMPROVE_MAGIC:
					case AT_IMPROVE_CURSE:
		            case AT_IMPROVE_BLOCKING:
		            case AT_IMPROVE_DEFENSE:
			            addOption2 = 1;
                        addOptionV2= item2.SpecialValue[i];
			            break;
		            }
                }

                if ( addOption1<addOption2 || addOptionV1<addOptionV2 )
                {
                    equal = -1;
                }
                else if ( addOption1!=addOption2 && addOptionV1!=addOptionV2 )
                {
                    equal = 1;
                }
            }
        }
         if ( equal==0 )
        {
            if( item1.Durability<item2.Durability  )
                equal = -1;
            if( item1.Durability>item2.Durability  )
                equal = 1;
        }
    }
    return  equal;
}

void InsertInventory(ITEM *Inv,int Width,int Height,int Index,BYTE *Item,bool First)
{
	int Type = ConvertItemType(Item);

	if(Type == 0x1FFF) return;

	if(Inv==Inventory)
	{
		if(Index < 12)
		{
			ITEM *ip = &CharacterMachine->Equipment[Index];
			ip->Type       = Type;
			ip->Durability = Item[2];
			ip->Option1    = Item[3];
            ip->ExtOption  = Item[4];
			ip->Number     = 1;
			ip->option_380 = false;
			BYTE b = ( ( (Item[5] & 0x08) << 4) >>7);
			ip->option_380 = b;
			ip->Jewel_Of_Harmony_Option = (Item[6] & 0xf0) >> 4;
			ip->Jewel_Of_Harmony_OptionLevel = Item[6] & 0x0f;

			ItemConvert(ip,Item[1],Item[3],Item[4]);			
            SetCharacterClass(Hero);
			OBJECT *o = &Hero->Object;
			if(Index==EQUIPMENT_HELPER)
			{
                if ( gMapManager.InChaosCastle()==false )
                {
                    switch(Type)
                    {
                    case ITEM_HELPER  :CreateBug(MODEL_HELPER  ,o->Position,o);break;
                    case ITEM_HELPER+2:CreateBug(MODEL_UNICON  ,o->Position,o);
                        if(!Hero->SafeZone)
                            CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,1,o);
                        break;
                    case ITEM_HELPER+3:CreateBug(MODEL_PEGASUS, o->Position,o);
                        if(!Hero->SafeZone)
                            CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,1,o);
                        break;
                    case ITEM_HELPER+4:CreateBug(MODEL_DARK_HORSE ,o->Position,o);
                        if(!Hero->SafeZone)
                            CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,1,o);
                        break;
					case ITEM_HELPER+37:
						Hero->Helper.Option1 = ip->Option1;
						if(ip->Option1 == 0x01)
						{
							CreateBug(MODEL_FENRIR_BLACK, o->Position, o);
						}
						else if(ip->Option1 == 0x02)
						{
							CreateBug(MODEL_FENRIR_BLUE, o->Position, o);
						}
						else if(ip->Option1 == 0x04)
						{
							CreateBug(MODEL_FENRIR_GOLD, o->Position, o);
						}
						else
						{
							CreateBug(MODEL_FENRIR_RED, o->Position, o);
						}
						if(!Hero->SafeZone)
                            CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,1,o);
						break;
                    }
                }
			}
            else if ( Index==EQUIPMENT_WEAPON_LEFT )
            {
                switch ( Type )
                {
                case ITEM_HELPER+5:
                    giPetManager::CreatePetDarkSpirit ( Hero );            
                    break;
                }
            }

            if ( Index==EQUIPMENT_RING_LEFT || Index==EQUIPMENT_RING_RIGHT && ( Hero->EtcPart<=0 || Hero->EtcPart>3 ) )
            {
                if ( Type==ITEM_HELPER+20 && (ip->Level>>3)==3 )
                {
                    DeleteParts ( Hero );
                    Hero->EtcPart = PARTS_LION;
                }
            }

			if ( Index==EQUIPMENT_WING)
			{
				if (Type==ITEM_WING+39 || 
					Type==ITEM_HELPER+30 || 
					Type==ITEM_WING+130 ||
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
					Type==ITEM_WING+49 ||
					Type==ITEM_WING+50 ||
					Type==ITEM_WING+135||
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
					Type==ITEM_WING+40 )
				{
					DeleteCloth(Hero, &Hero->Object);
				}
			}

		}
		else if(Index >= 12 && Index < 76)
		{
			int InventoryIndex = Index - 12;
			ITEM_ATTRIBUTE *ap = &ItemAttribute[Type];
			for(int i=0;i<ap->Height;i++)
			{
				for(int j=0;j<ap->Width;j++)
				{
					ITEM *ip = &Inv[InventoryIndex+i*Width+j];
					ip->Type       = Type;
           			ip->Durability = Item[2];
           			ip->Option1    = Item[3];
                    ip->ExtOption  = Item[4];
					ip->x          = InventoryIndex%8;
					ip->y          = InventoryIndex/8;
					ip->option_380 = false;
					BYTE b = ( ( (Item[5] & 0x08) << 4) >>7);
					ip->option_380 = b;
					ip->Jewel_Of_Harmony_Option = (Item[6] & 0xf0) >> 4;
					ip->Jewel_Of_Harmony_OptionLevel = Item[6] & 0x0f;

					if(i==0 && j==0)
						ip->Number = 1;
					else
						ip->Number = 0;
		        	ItemConvert(ip,Item[1],Item[3],Item[4]);


				}
			}

            if( SrcInventoryIndex < 12 )
            {
        		if(CharacterAttribute->AbilityTime[0] != 0)
                {
                    CharacterMachine->CalculateAttackSpeed();
                }
        		if(CharacterAttribute->AbilityTime[2] != 0)
                {
                    CharacterMachine->CalculateAttackSpeed();
                }
            }
		}
		else
		{
			int InventoryIndex = Index - 76;
			ITEM_ATTRIBUTE *ap = &ItemAttribute[Type];
			for(int i=0;i<ap->Height;i++)
			{
				for(int j=0;j<ap->Width;j++)
				{
					ITEM *ip = &g_PersonalShopInven[InventoryIndex+i*Width+j];
					ip->Type       = Type;
					ip->Durability = Item[2];
					ip->Option1    = Item[3];
                    ip->ExtOption  = Item[4];
					ip->x          = InventoryIndex%8;
					ip->y          = InventoryIndex/8;
					ip->option_380 = false;
					BYTE b = ( ( (Item[5] & 0x08) << 4) >>7);
					ip->option_380 = b;
					ip->Jewel_Of_Harmony_Option = (Item[6] & 0xf0) >> 4;
					ip->Jewel_Of_Harmony_OptionLevel = Item[6] & 0x0f;
					if(i==0 && j==0)
						ip->Number = 1;
					else
						ip->Number = 0;
					ItemConvert(ip,Item[1],Item[3],Item[4]);
				}
			}
			
            if( SrcInventoryIndex < 12 )
            {
				if(CharacterAttribute->AbilityTime[0] != 0)
                {
                    CharacterMachine->CalculateAttackSpeed();
                }
				if(CharacterAttribute->AbilityTime[2] != 0)
                {
                    CharacterMachine->CalculateAttackSpeed();
                }
            }
		}
	}
	else
	{
		int InventoryIndex = Index;
		ITEM_ATTRIBUTE *ap = &ItemAttribute[Type];
		for(int i=0;i<ap->Height;i++)
		{
			for(int j=0;j<ap->Width;j++)
			{
				ITEM *ip = &Inv[InventoryIndex+i*Width+j];
				ip->Type       = Type;
       			ip->Durability = Item[2];
      			ip->Option1    = Item[3];
                ip->ExtOption  = Item[4];
				ip->x          = InventoryIndex%8;
				ip->y          = InventoryIndex/8;
				ip->option_380 = false;
				BYTE b = ( ( (Item[5] & 0x08) << 4) >>7);
				ip->option_380 = b;
				ip->Jewel_Of_Harmony_Option = (Item[6] & 0xf0) >> 4;
				ip->Jewel_Of_Harmony_OptionLevel = Item[6] & 0x0f;
				if(i==0 && j==0)
					ip->Number = 1;
				else
					ip->Number = 0;
	    		ItemConvert(ip,Item[1],Item[3],Item[4]);
			}
		}
	}

	if(!First)
	{
		if(CharacterMachine->Equipment[EQUIPMENT_HELPER].Type == -1)
    		DeleteBug(&Hero->Object);
        if ( CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type==-1 )
            giPetManager::DeletePet ( Hero );            

        ITEM& rl = CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
        ITEM& rr = CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
        if ( ( rl.Type!=ITEM_HELPER+20 || (rl.Level>>3)!=3 ) && ( rr.Type!=ITEM_HELPER+20 || (rr.Level>>3)!=3 ) )
        {
            if ( Hero->EtcPart<PARTS_ATTACK_TEAM_MARK )
            {
                DeleteParts ( Hero );
                if ( Hero->EtcPart>3 ) Hero->EtcPart = 0;
            }
        }
	}
}

bool EquipmentItem = false;
extern int BuyCost;

bool IsPartChargeItem(ITEM* pItem)
{
	if(	(pItem->Type >= ITEM_HELPER+46 && pItem->Type <= ITEM_HELPER+48)
		|| (pItem->Type == ITEM_POTION+54)
		|| (pItem->Type >= ITEM_POTION+58 && pItem->Type <= ITEM_POTION+62)
		|| (pItem->Type >= ITEM_POTION+145 && pItem->Type <= ITEM_POTION+150)
		|| (pItem->Type >= ITEM_HELPER+125 && pItem->Type <= ITEM_HELPER+127)
		|| pItem->Type == ITEM_POTION+53
		|| (pItem->Type >= ITEM_HELPER+43 && pItem->Type <= ITEM_HELPER+45)
		|| (pItem->Type >= ITEM_POTION+70 && pItem->Type <= ITEM_POTION+71)
		|| (pItem->Type >= ITEM_POTION+72 && pItem->Type <= ITEM_POTION+77)
		|| (pItem->Type == ITEM_HELPER+59)
		|| ( pItem->Type >= ITEM_HELPER+54 && pItem->Type <= ITEM_HELPER+58)
		|| (pItem->Type >= ITEM_POTION+78 && pItem->Type <= ITEM_POTION+82)
		|| (pItem->Type == ITEM_HELPER+60)
		|| (pItem->Type == ITEM_HELPER+61)
		|| (pItem->Type == ITEM_POTION+91)
		|| (pItem->Type >= ITEM_POTION+92 && pItem->Type <= ITEM_POTION+93)
		|| (pItem->Type == ITEM_POTION+95)
		|| (pItem->Type == ITEM_POTION+94)
		|| (pItem->Type >= ITEM_HELPER+62 && pItem->Type <= ITEM_HELPER+63)
		|| (pItem->Type >= ITEM_POTION+97 && pItem->Type <= ITEM_POTION+98)
		|| (pItem->Type == ITEM_POTION+96)
		|| ( pItem->Type == ITEM_HELPER+64 || pItem->Type == ITEM_HELPER+65 )
		|| ( pItem->Type == ITEM_HELPER+69 )
		|| ( pItem->Type == ITEM_HELPER+70 )
		|| pItem->Type == ITEM_HELPER+81
		|| pItem->Type == ITEM_HELPER+82
		|| pItem->Type == ITEM_HELPER+93
		|| pItem->Type == ITEM_HELPER+94
		|| pItem->Type == ITEM_HELPER+107
		|| pItem->Type == ITEM_HELPER+104
		|| pItem->Type == ITEM_HELPER+105
		|| pItem->Type == ITEM_HELPER+103
		|| pItem->Type == ITEM_POTION+133
		|| pItem->Type == ITEM_HELPER+109
		|| pItem->Type == ITEM_HELPER+110
		|| pItem->Type == ITEM_HELPER+111
		|| pItem->Type == ITEM_HELPER+112
		|| pItem->Type == ITEM_HELPER+113
		|| pItem->Type == ITEM_HELPER+114
		|| pItem->Type == ITEM_HELPER+115
		|| pItem->Type == ITEM_POTION+112
		|| pItem->Type == ITEM_POTION+113
		|| pItem->Type == ITEM_POTION+120
		|| pItem->Type == ITEM_POTION+123
		|| pItem->Type == ITEM_POTION+124
		|| pItem->Type == ITEM_POTION+134			
		|| pItem->Type == ITEM_POTION+135			
		|| pItem->Type == ITEM_POTION+136			
		|| pItem->Type == ITEM_POTION+137			
		|| pItem->Type == ITEM_POTION+138			
		|| pItem->Type == ITEM_POTION+139			
		|| pItem->Type == ITEM_WING+130
		|| pItem->Type == ITEM_WING+131			
		|| pItem->Type == ITEM_WING+132			
		|| pItem->Type == ITEM_WING+133			
		|| pItem->Type == ITEM_WING+134	
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_WING+135
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_HELPER+116
		|| pItem->Type == ITEM_HELPER+106
		|| pItem->Type == ITEM_HELPER+124
		|| pItem->Type == ITEM_POTION+114
		|| pItem->Type == ITEM_POTION+115
		|| pItem->Type == ITEM_POTION+116
		|| pItem->Type == ITEM_POTION+117
		|| pItem->Type == ITEM_POTION+118
		|| pItem->Type == ITEM_POTION+119
		|| pItem->Type == ITEM_POTION+126
		|| pItem->Type == ITEM_POTION+127
		|| pItem->Type == ITEM_POTION+128
		|| pItem->Type == ITEM_POTION+129
		|| pItem->Type == ITEM_POTION+130
		|| pItem->Type == ITEM_POTION+131
		|| pItem->Type == ITEM_POTION+132
		|| pItem->Type == ITEM_HELPER+121
		|| pItem->Type == ITEM_POTION+140
	)
	{
		return true;
	}

	return false;
}

bool IsHighValueItem(ITEM* pItem)
{
	int iLevel = (pItem->Level >> 3) & 15;

	if(
		pItem->Type == ITEM_HELPER+3 ||
		pItem->Type == ITEM_POTION+13 ||
		pItem->Type == ITEM_POTION+14 ||
		pItem->Type == ITEM_POTION+16 ||
		pItem->Type == ITEM_POTION+22 ||
		pItem->Type == ITEM_WING+15 ||
		pItem->Type == ITEM_POTION+31 ||
		pItem->Type == ITEM_WING+30	||
		pItem->Type == ITEM_WING+31	||
        (pItem->Type >= ITEM_WING && pItem->Type <= ITEM_WING+6 ) ||	
        pItem->Type == ITEM_HELPER+4 ||
		pItem->Type == ITEM_HELPER+5 ||
		pItem->Type == ITEM_HELPER+30 || 
		(pItem->Type >= ITEM_WING+36 && pItem->Type <= ITEM_WING+43 ) ||
		((pItem->ExtOption%0x04) == EXT_A_SET_OPTION || (pItem->ExtOption%0x04) == EXT_B_SET_OPTION ) ||
		pItem->Type == ITEM_SWORD+19 ||
		pItem->Type == ITEM_STAFF+10 ||
		pItem->Type == ITEM_BOW+18 ||	
		pItem->Type == ITEM_MACE+13 ||
		pItem->Type == ITEM_HELPER+14 ||	
		pItem->Type == ITEM_HELPER+15 ||
		pItem->Type == ITEM_HELPER+19 ||
		pItem->Type == ITEM_HELPER+31 ||
		(pItem->Type >= ITEM_POTION+41 && pItem->Type <= ITEM_POTION+44) ||
		(iLevel > 6 && pItem->Type < ITEM_WING) ||
		(pItem->Option1 & 63) > 0 ||
		(pItem->Type >= ITEM_HELPER+34 && pItem->Type <= ITEM_HELPER+37)
		|| pItem->Type == ITEM_HELPER+52
		|| pItem->Type == ITEM_HELPER+53
		|| pItem->Type == ITEM_POTION+121
		|| pItem->Type == ITEM_POTION+122
		|| pItem->Type == ITEM_WING+130
		|| pItem->Type == ITEM_WING+131
		|| pItem->Type == ITEM_WING+132
		|| pItem->Type == ITEM_WING+133
		|| pItem->Type == ITEM_WING+134
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_WING+135
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
		|| pItem->Type == ITEM_HELPER+64
		|| pItem->Type == ITEM_HELPER+65
		|| pItem->Type == ITEM_HELPER+109
		|| pItem->Type == ITEM_HELPER+110
		|| pItem->Type == ITEM_HELPER+111
		|| pItem->Type == ITEM_HELPER+112
		|| pItem->Type == ITEM_HELPER+113
		|| pItem->Type == ITEM_HELPER+114
		|| pItem->Type == ITEM_HELPER+115
		|| pItem->Type == ITEM_POTION+112
		|| pItem->Type == ITEM_POTION+113
		|| ( pItem->Type==ITEM_HELPER+20 && iLevel==0)
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 255)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (pItem->Type >= ITEM_WING+49 && pItem->Type <= ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#ifdef KJH_FIX_SELL_LUCKYITEM
		|| ( Check_ItemAction(pItem, eITEM_SELL) && pItem->Durability > 0 )
#endif // KJH_FIX_SELL_LUCKYITEM
		|| ( COMGEM::isCompiledGem( pItem ) )
		)
	{
		if(true == pItem->bPeriodItem && false == pItem->bExpiredPeriod)
		{
			return false;
		}
		else if(pItem->Type == ITEM_HELPER+80
			|| pItem->Type == ITEM_HELPER+76
			|| pItem->Type == ITEM_HELPER+64
			|| pItem->Type == ITEM_HELPER+65
			|| pItem->Type == ITEM_HELPER+122
			|| pItem->Type == ITEM_HELPER+123
			)
		{
			if(true == pItem->bPeriodItem && true == pItem->bExpiredPeriod)
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
	if(pItem == NULL)
	{
		return false;
	}

#ifdef KJH_FIX_PERSONALSHOP_BAN_CASHITEM
	if(pItem->bPeriodItem)
	{
		return true;
	}
#endif // KJH_FIX_PERSONALSHOP_BAN_CASHITEM

	if( (!pItem->bPeriodItem) && 
		pItem->Type == ITEM_HELPER+64
		|| pItem->Type == ITEM_HELPER+65
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 255)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (pItem->Type==ITEM_HELPER+20 && ((pItem->Level >> 3) & 15)==0)
		)
	{
		return false;
	}		

	if(pItem->Type == ITEM_HELPER+38
		|| pItem->Type == ITEM_HELPER+39
		|| (pItem->Type == ITEM_POTION+21 && ((pItem->Level>>3)&15) != 3)
		|| ( pItem->Type >= ITEM_POTION+23 && pItem->Type <= ITEM_POTION+26 )
		|| pItem->Type == ITEM_HELPER+19
		|| (pItem->Type == ITEM_POTION+11 && ((pItem->Level>>3)&0x0F) == 13)
		|| (pItem->Type >= ITEM_HELPER+43 && pItem->Type <= ITEM_HELPER+45)
		|| (pItem->Type == ITEM_HELPER+20 && ((pItem->Level>>3)&15) != 0)
		|| pItem->Type == ITEM_POTION+65
		|| pItem->Type == ITEM_POTION+66
		|| pItem->Type == ITEM_POTION+67
		|| pItem->Type == ITEM_POTION+68
		|| IsPartChargeItem(pItem)
		|| pItem->Type == ITEM_HELPER+97
		|| pItem->Type == ITEM_HELPER+98
		|| pItem->Type == ITEM_POTION+91
		|| pItem->Type == ITEM_HELPER+99
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
		|| (pItem->Type == MODEL_POTION+96)
		|| pItem->Type ==ITEM_HELPER+109
		|| pItem->Type ==ITEM_HELPER+110
		|| pItem->Type ==ITEM_HELPER+111
		|| pItem->Type ==ITEM_HELPER+112
		|| pItem->Type ==ITEM_HELPER+113
		|| pItem->Type ==ITEM_HELPER+114
		|| pItem->Type ==ITEM_HELPER+115
#ifdef LDK_MOD_INGAMESHOP_WIZARD_RING_PERSONALSHOPBAN
		|| ( pItem->Type==ITEM_HELPER+20 && ((pItem->Level >> 3) & 15)==0)
#endif //LDK_MOD_INGAMESHOP_WIZARD_RING_PERSONALSHOPBAN
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| (pItem->Type >= ITEM_POTION+151 && pItem->Type <= ITEM_POTION+156)
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		)
	{		
		return true;
	}
#ifdef LEM_ADD_LUCKYITEM
	if( Check_ItemAction( pItem, eITEM_PERSONALSHOP ) )	return true;
#endif // LEM_ADD_LUCKYITEM
	
	return false;
}

bool IsTradeBan(ITEM* pItem)
{
	if(pItem->Type == ITEM_HELPER+38
		|| pItem->Type == ITEM_HELPER+39
		|| (pItem->Type == ITEM_POTION+21 && ((pItem->Level>>3)&15) != 3)
		|| ( pItem->Type >= ITEM_POTION+23 && pItem->Type <= ITEM_POTION+26 )
		|| pItem->Type == ITEM_HELPER+19 
		|| (pItem->Type == ITEM_POTION+11 && ((pItem->Level>>3)&0x0F) == 13)
		|| (pItem->Type >= ITEM_HELPER+43 && pItem->Type <= ITEM_HELPER+45)
		|| (pItem->Type == ITEM_HELPER+20 && ((pItem->Level>>3)&15) != 0)
		|| pItem->Type == ITEM_POTION+64
		|| pItem->Type == ITEM_POTION+65
		|| pItem->Type == ITEM_POTION+66
		|| pItem->Type == ITEM_POTION+67
		|| pItem->Type == ITEM_POTION+68
		|| IsPartChargeItem(pItem)
		|| pItem->Type == ITEM_HELPER+97
		|| pItem->Type == ITEM_HELPER+98
		|| pItem->Type == ITEM_POTION+91
		|| pItem->Type == ITEM_HELPER+99
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| (pItem->Type >= ITEM_POTION+151 && pItem->Type <= ITEM_POTION+156)
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		)
	{
		return true;
	}

	if( pItem->Type == ITEM_POTION+52)
	{
		if( g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) ||	
			(Hero->CtlCode == CTLCODE_20OPERATOR) || (Hero->CtlCode == CTLCODE_08OPERATOR) )
			return false;
		else
			return true;
	}
#ifdef LEM_ADD_LUCKYITEM
	if( Check_ItemAction( pItem, eITEM_TRADE ) )	return true;
#endif // LEM_ADD_LUCKYITEM

	return false;
}

bool IsStoreBan(ITEM* pItem)
{
	if(( pItem->Type >= ITEM_POTION+23 && pItem->Type <= ITEM_POTION+26 )
		|| (pItem->Type == ITEM_POTION+21 && ((pItem->Level>>3)&15) != 3)
		|| pItem->Type == ITEM_HELPER+19 
		|| (pItem->Type == ITEM_POTION+11 && ((pItem->Level>>3)&0x0F) == 13)
		|| (pItem->Type >= ITEM_HELPER+43 && pItem->Type <= ITEM_HELPER+45)
		|| pItem->Type == ITEM_HELPER+93
		|| pItem->Type == ITEM_HELPER+94
		|| (pItem->Type == ITEM_HELPER+20 && ((pItem->Level>>3)&15) != 0)
		|| pItem->Type == ITEM_POTION+65
		|| pItem->Type == ITEM_POTION+66
		|| pItem->Type == ITEM_POTION+67
		|| pItem->Type == ITEM_POTION+68
		|| (pItem->Type == ITEM_HELPER+70 && pItem->Durability == 1)
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| (pItem->Type >= ITEM_POTION+151 && pItem->Type <= ITEM_POTION+156)
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
#ifdef LEM_ADD_LUCKYITEM
	if( Check_ItemAction( pItem, eITEM_STORE ) )	return true;
#endif // LEM_ADD_LUCKYITEM

	return false;
}

#ifdef LEM_ADD_LUCKYITEM
sItemAct Set_ItemActOption( int _nIndex, int _nOption )
{
	sItemAct	sItem;
	// eITEM_PERSONALSHOP = 개인상점, eITEM_STORE = 창고, eITEM_TRADE = 거래, eITEM_DROP = 버리기, eITEM_SELL = 판매, eITEM_REPAIR = 수리
	int	nItemOption[][eITEM_END]	= { 0, 1, 1, 0, 0, 0,
										0, 0, 0, 0, 1, 0,
		-1 };
	
	
	sItem.s_nItemIndex	= _nIndex;
	
	for( int i=0; i< eITEM_END; i++ )
	{
		sItem.s_bType[i] = nItemOption[_nOption][i];
	}
	return sItem;
}


bool Check_ItemAction( ITEM* _pItem, ITEMSETOPTION _eAction, bool _bType )
{
	std::vector<sItemAct>			sItem;
	std::vector<sItemAct>::iterator li;
	int		i			= 0;
	
	for( i=0; i<12; i++ )	sItem.push_back( Set_ItemActOption( ITEM_HELPER+135 +i, 0 ) );
	for( i=0; i< 2; i++ )	sItem.push_back( Set_ItemActOption( ITEM_POTION+160 +i, 0 ) );
	for( i=0; i<12; i++ )
	{
		sItem.push_back( Set_ItemActOption( ITEM_ARMOR+62 +i, 1 ) );
		sItem.push_back( Set_ItemActOption( ITEM_HELM+62 +i, 1 ) );
		sItem.push_back( Set_ItemActOption( ITEM_BOOTS+62 +i, 1 ) );
		sItem.push_back( Set_ItemActOption( ITEM_GLOVES+62 +i, 1 ) );
		sItem.push_back( Set_ItemActOption( ITEM_PANTS+62 +i, 1 ) );
	}
	
	for( li = sItem.begin(); li != sItem.end(); li++ )
	{
		if( li->s_nItemIndex == _pItem->Type )	
		{
			_bType = (li->s_bType[_eAction])^ (!_bType);
			return _bType;
		}
	}
	
	// 등록되지 않은 아이템은 무시.
	return false;
}

bool Check_LuckyItem( int _nIndex, int _nType )
{
	int		nItemTabIndex	= (_nIndex + _nType)% 512;

	if( _nIndex < ITEM_HELM || _nIndex > ITEM_WING )	return false;
	if( nItemTabIndex >= 62 && nItemTabIndex <= 72 )		return true;

	return false;
}
#endif // LEM_ADD_LUCKYITEM

#ifdef KJH_FIX_SELL_LUCKYITEM
bool IsLuckySetItem( int iType )
{
	int iItemIndex = iType%MAX_ITEM_INDEX;
	
#ifdef LEM_FIX_SELL_LUCKYITEM_BOOTS_POPUP
	if( (iType >= ITEM_HELM && iType <= ITEM_WING)
#else // LEM_FIX_SELL_LUCKYITEM_BOOTS_POPUP
	if( (iType >= ITEM_HELM && iType <= ITEM_BOOTS)
#endif // LEM_FIX_SELL_LUCKYITEM_BOOTS_POPUP
		&& (iItemIndex >= 62 && iItemIndex <= 72 ) )
	{
		return true;
	}
	
	return false;
}
#endif // KJH_FIX_SELL_LUCKYITEM

bool IsDropBan(ITEM* pItem)
{
	if( (!pItem->bPeriodItem) &&
		( pItem->Type == ITEM_POTION+96
		|| pItem->Type == ITEM_POTION+54
		|| pItem->Type == ITEM_HELPER+64
		|| pItem->Type == ITEM_HELPER+65
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
		|| (pItem->Type==ITEM_HELPER+20 && ((pItem->Level >> 3) & 15)==0)
		) )
	{
		return false;
	}		

	if( true == false || pItem->Type == ITEM_POTION+123 || pItem->Type == ITEM_POTION+124)
	{
		return false;
	}

	if (( pItem->Type >= ITEM_POTION+23 && pItem->Type <= ITEM_POTION+26 )
		|| (pItem->Type >= ITEM_POTION+65 && pItem->Type <= ITEM_POTION+68)
		|| IsPartChargeItem(pItem)
		|| ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN)
			&& (pItem->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END))
		|| pItem->Type == ITEM_HELPER+97
		|| pItem->Type == ITEM_HELPER+98
		|| pItem->Type == ITEM_POTION+91
		|| pItem->Type == ITEM_HELPER+99
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
		|| pItem->Type == ITEM_POTION+121
		|| pItem->Type == ITEM_POTION+122
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		)
	{
		return true;
	}
#ifdef LEM_ADD_LUCKYITEM
	if( Check_ItemAction( pItem, eITEM_DROP ) )	return true;
#endif // LEM_ADD_LUCKYITEM
	
	return false;
}

bool IsSellingBan(ITEM* pItem)
{
	int Level = (pItem->Level>>3)&15;

	if( true == false
		|| pItem->Type ==ITEM_POTION+112
		|| pItem->Type ==ITEM_POTION+113
		|| pItem->Type == ITEM_POTION+121
		|| pItem->Type == ITEM_POTION+122
		|| pItem->Type == ITEM_POTION+123
		|| pItem->Type == ITEM_POTION+124
		|| pItem->Type == ITEM_WING+130
		|| pItem->Type == ITEM_WING+131
		|| pItem->Type == ITEM_WING+132
		|| pItem->Type == ITEM_WING+133
		|| pItem->Type == ITEM_WING+134
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_WING+135
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
		|| pItem->Type == ITEM_HELPER+64
		|| pItem->Type == ITEM_HELPER+65
		|| pItem->Type ==ITEM_HELPER+109
		|| pItem->Type ==ITEM_HELPER+110
		|| pItem->Type ==ITEM_HELPER+111
		|| pItem->Type ==ITEM_HELPER+112
		|| pItem->Type ==ITEM_HELPER+113
		|| pItem->Type ==ITEM_HELPER+114
		|| pItem->Type ==ITEM_HELPER+115
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability != 254)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| ((pItem->Type==ITEM_HELPER+20)&&(Level == 0))
		|| (pItem->Type == ITEM_HELPER+106)
		|| (pItem->Type == ITEM_HELPER+107)
		)
	{
		if(true == pItem->bPeriodItem && true == pItem->bExpiredPeriod)
		{
			return false;
		}
	}
	
	if(pItem->Type==ITEM_POTION+11
		|| ( pItem->Type==ITEM_POTION+21 && Level==1 )
		|| ((pItem->bPeriodItem==true)&&(pItem->bExpiredPeriod==false)&&(pItem->Type==ITEM_HELPER+20)&&(Level==0))
		|| (pItem->Type==ITEM_HELPER+20 && (Level==1 || Level==2) )
		|| pItem->Type == ITEM_HELPER+19
		|| (pItem->Type==ITEM_POTION+20 && Level >= 1 && Level <= 5)
		|| IsPartChargeItem(pItem)
		|| ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN)
			&& (pItem->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END))
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type))
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (pItem->Type == ITEM_HELPER+106)
		|| (pItem->Type == ITEM_HELPER+107)
#ifdef KJH_FIX_SELL_LUCKYITEM
		|| ( (IsLuckySetItem(pItem->Type) == true ) && (pItem->Durability > 0) )
#endif // KJH_FIX_SELL_LUCKYITEM
		)
	{
		return true;
	}
#ifdef LEM_ADD_LUCKYITEM
	if( Check_ItemAction( pItem, eITEM_SELL ) )	return true;
#endif // LEM_ADD_LUCKYITEM

	return false;
}



bool IsRepairBan(ITEM* pItem)
{
	if(g_ChangeRingMgr->CheckRepair(pItem->Type) == true)
	{
		return true;
	}
	if( IsPartChargeItem(pItem) == true || ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN) && (pItem->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END)))
	{
		return true;
	}

	if(	(  pItem->Type >= ITEM_POTION+55 && pItem->Type <= ITEM_POTION+57)
		|| pItem->Type == MODEL_HELPER+43
		|| pItem->Type == MODEL_HELPER+44
		|| pItem->Type == MODEL_HELPER+45
		|| (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HELPER+3)
		|| pItem->Type == ITEM_BOW+7
		|| pItem->Type == ITEM_BOW+15
		|| pItem->Type >= ITEM_POTION
		|| (pItem->Type >= ITEM_WING+7 && pItem->Type <= ITEM_WING+19)
		|| (pItem->Type >= ITEM_HELPER+14 && pItem->Type <= ITEM_HELPER+19)
		|| pItem->Type == ITEM_POTION+21
		|| pItem->Type == ITEM_HELPER+4
		|| pItem->Type == ITEM_HELPER+5
		|| pItem->Type == ITEM_HELPER+38
		|| pItem->Type == ITEM_HELPER+67
		|| pItem->Type == ITEM_HELPER+80
		|| pItem->Type == ITEM_HELPER+76
		|| pItem->Type == ITEM_HELPER+122
		|| pItem->Type == ITEM_HELPER+123
		|| pItem->Type == ITEM_HELPER+106
		|| pItem->Type == ITEM_POTION+84
		|| pItem->Type == ITEM_POTION+85
		|| pItem->Type == ITEM_POTION+86
		|| pItem->Type == ITEM_POTION+87
		|| pItem->Type == ITEM_POTION+88
		|| pItem->Type == ITEM_POTION+89
		|| pItem->Type == ITEM_POTION+90
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| pItem->Type == ITEM_HELPER+7
		|| pItem->Type == ITEM_HELPER+10
		|| pItem->Type == ITEM_HELPER+11
		|| pItem->Type == ITEM_HELPER+20
		|| pItem->Type == ITEM_HELPER+29
		|| pItem->Type == ITEM_HELPER+32
		|| pItem->Type == ITEM_HELPER+33
		|| pItem->Type == ITEM_HELPER+34
		|| pItem->Type == ITEM_HELPER+35
		|| pItem->Type == ITEM_HELPER+36
		|| pItem->Type == ITEM_HELPER+37
		|| pItem->Type == ITEM_HELPER+49
		|| pItem->Type == ITEM_HELPER+50
		|| pItem->Type == ITEM_HELPER+51
		|| pItem->Type == ITEM_HELPER+66
		|| pItem->Type == ITEM_HELPER+71
		|| pItem->Type == ITEM_HELPER+72
		|| pItem->Type == ITEM_HELPER+73
		|| pItem->Type == ITEM_HELPER+74
		|| pItem->Type == ITEM_HELPER+75
		)
	{
		return true;
	}
#ifdef LEM_ADD_LUCKYITEM
	if( Check_ItemAction( pItem, eITEM_REPAIR ) )	return true;
#endif // LEM_ADD_LUCKYITEM
	
	return false;
}

bool IsWingItem(ITEM* pItem)
{
	switch(pItem->Type)
	{
	case ITEM_WING:
	case ITEM_WING+1:
	case ITEM_WING+2:
	case ITEM_WING+3:
	case ITEM_WING+4:
	case ITEM_WING+5:
	case ITEM_WING+6:
	case ITEM_HELPER+30:
	case ITEM_WING+36:
	case ITEM_WING+37:
	case ITEM_WING+38:
	case ITEM_WING+39:
	case ITEM_WING+40:
	case ITEM_WING+41:
	case ITEM_WING+42:
	case ITEM_WING+43:
	case ITEM_WING+130:
	case ITEM_WING+131:
	case ITEM_WING+132:
	case ITEM_WING+133:
	case ITEM_WING+134:
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+49:
	case ITEM_WING+50:
	case ITEM_WING+135:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		return true;
	}

	return false;
}

OBJECT ObjectSelect;

void RenderObjectScreen(int Type,int ItemLevel,int Option1,int ExtOption,vec3_t Target,int Select,bool PickUp)
{	
   	int Level = (ItemLevel>>3)&15;
    vec3_t Direction,Position;

	VectorSubtract(Target,MousePosition,Direction);
	if(PickUp)
      	VectorMA(MousePosition,0.07f,Direction,Position);
	else
      	VectorMA(MousePosition,0.1f,Direction,Position);

	if(Type == MODEL_SWORD+0)
	{
		Position[0] -= 0.02f;
		Position[1] += 0.03f;
		Vector(180.f,270.f,15.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_BOW+7 || Type==MODEL_BOW+15 )
	{
    	Vector(0.f,270.f,15.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_SPEAR+0)
	{
		Position[1] += 0.05f;
		Vector(0.f,90.f,20.f,ObjectSelect.Angle);
	}
	else if( Type==MODEL_BOW+17)
	{
    	Vector(0.f,90.f,15.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELM+31)
	{
		Position[1] -= 0.06f;
		Position[0] += 0.03f;
    	Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELM+30)
	{
		Position[1] += 0.07f;
		Position[0] -= 0.03f;
    	Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_ARMOR+30)
	{
		Position[1] += 0.1f;
    	Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_ARMOR+29)
	{
		Position[1] += 0.07f;
    	Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}	
	else if( Type == MODEL_BOW+21)
	{
		Position[1] += 0.12f;
    	Vector(180.f,-90.f,15.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_STAFF+12)
	{
		Position[1] -= 0.1f;
		Position[0] += 0.025f;
    	Vector(180.f,0.f,8.f,ObjectSelect.Angle);
	}
	else if (Type >= MODEL_STAFF+21 && Type <= MODEL_STAFF+29)
	{
    	Vector(0.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_MACE+14)
	{
		Position[1] += 0.1f;
		Position[0] -= 0.01;
    	Vector(180.f,90.f,13.f,ObjectSelect.Angle);
	}	
	else if(Type == MODEL_ARMOR+34)
	{
		Position[1] += 0.03f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELM+35)
	{
		Position[0] -= 0.02f;
		Position[1] += 0.05f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+35)
	{
		Position[1] += 0.05f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+36)
	{
		Position[1] -= 0.05f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+37)
	{
		Position[1] -= 0.05f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if (MODEL_HELM+39 <= Type && MODEL_HELM+44 >= Type)
	{
		Position[1] -= 0.05f;
		Vector(-90.f,25.f,0.f,ObjectSelect.Angle);
	}
	else if(MODEL_ARMOR+38 <= Type && MODEL_ARMOR+44 >= Type)
	{
		Position[1] -= 0.08f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_SWORD+24)
	{
		Position[0] -= 0.02f;
		Position[1] += 0.03f;
    	Vector(180.f,90.f,15.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_MACE+15)
	{
		Position[1] += 0.05f;
    	Vector(180.f,90.f,13.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_BOW+22 || Type == MODEL_BOW+23)
	{
		Position[0] -= 0.10f;
		Position[1] += 0.08f;
    	Vector(180.f,-90.f,15.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_STAFF+13)
	{
		Position[0] += 0.02f;
		Position[1] += 0.02f;
		Vector(180.f,90.f,8.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_BOW+20)
	{
		Vector(180.f,-90.f,15.f,ObjectSelect.Angle);
	}
	else if(Type>=MODEL_BOW+8 && Type<MODEL_BOW+MAX_ITEM_INDEX)
	{
    	Vector(90.f,180.f,20.f,ObjectSelect.Angle);
	}
	else if ( Type==MODEL_SPEAR+10 )
	{
      	Vector(180.f,270.f,20.f,ObjectSelect.Angle);
	}
	else if(Type >= MODEL_SWORD && Type < MODEL_STAFF+MAX_ITEM_INDEX)
	{
		switch (Type)
		{
		case MODEL_STAFF+14:							Position[1] += 0.04f;	break;
		case MODEL_STAFF+17:	Position[0] += 0.02f;	Position[1] += 0.03f;	break;
		case MODEL_STAFF+18:	Position[0] += 0.02f;							break;
		case MODEL_STAFF+19:	Position[0] -= 0.02f;	Position[1] -= 0.02f;	break;
		case MODEL_STAFF+20:	Position[0] += 0.01f;	Position[1] -= 0.01f;	break;
		}

		if(!ItemAttribute[Type-MODEL_ITEM].TwoHand)
		{
      		Vector(180.f,270.f,15.f,ObjectSelect.Angle);
		}
		else
		{
      		Vector(180.f,270.f,25.f,ObjectSelect.Angle);
		}
	}									
	else if(Type>=MODEL_SHIELD && Type<MODEL_SHIELD+MAX_ITEM_INDEX)
	{
		Vector(270.f,270.f,0.f,ObjectSelect.Angle);
	}
    else if(Type==MODEL_HELPER+3)
    {
		Vector(-90.f,-90.f,0.f,ObjectSelect.Angle);
    }
    else if ( Type==MODEL_HELPER+4 ) 
    {
		Vector(-90.f,-90.f,0.f,ObjectSelect.Angle);
    }
    else if ( Type==MODEL_HELPER+5 )
    {
		Vector(-90.f,-35.f,0.f,ObjectSelect.Angle);
    }
    else if ( Type==MODEL_HELPER+31 )
    {
		Vector(-90.f,-90.f,0.f,ObjectSelect.Angle);
    }
    else if ( Type==MODEL_HELPER+30 )    
    {
        Vector ( -90.f, 0.f, 0.f, ObjectSelect.Angle );
    }
	else if ( Type==MODEL_EVENT+16 )
	{
		Vector ( -90.f, 0.f, 0.f, ObjectSelect.Angle );
	}
    else if ( Type==MODEL_HELPER+16 || Type == MODEL_HELPER+17 )
    {
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
    }
	else if ( Type==MODEL_HELPER+18 )
	{
		Vector(290.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if ( Type==MODEL_EVENT+11 )
	{
        Vector(-90.f, -20.f, -20.f, ObjectSelect.Angle);
	}
	else if ( Type==MODEL_EVENT+12)
	{
		Vector(250.f, 140.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type==MODEL_EVENT+14)
	{
		Vector(255.f, 160.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type==MODEL_EVENT+15)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
    else if ( Type>=MODEL_HELPER+21 && Type<=MODEL_HELPER+24 )
    {
		Vector(270.f, 160.f, 20.f, ObjectSelect.Angle);
    }
	else if ( Type==MODEL_HELPER+29 )
	{
		Vector(290.f,0.f,0.f,ObjectSelect.Angle);
	}

	else if(Type == MODEL_HELPER+32)
	{
		Position[0] += 0.01f;
		Position[1] -= 0.03f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+33)
	{
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+34)
	{
		Position[0] += 0.01f;
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+35)
	{
		Position[0] += 0.01f;
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+36)
	{
		Position[0] += 0.01f;
		Position[1] += 0.05f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+37)
	{
		Position[0] += 0.01f;
		Position[1] += 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+49)
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+50)
	{
		Position[1] -= 0.03f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+51)
	{
		Position[1] -= 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+64)
	{
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+52)
	{
		Position[1] += 0.045f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+53)
	{
		Position[1] += 0.04f;
		Vector(270.f, 120.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+37)
	{
		Position[1] += 0.05f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+38)
	{
		Position[1] += 0.05f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+39)
	{
		Position[1] += 0.08f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+40)
	{
		Position[1] += 0.05f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+42)
	{
		Position[1] += 0.05f;
		Vector(270.f,0.f,2.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+46)
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+47)
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+48)
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);	
	}
	else if(Type == MODEL_POTION+54)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+58)
	{
		Position[1] += 0.07f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+59 || Type == MODEL_POTION+60)
	{
		Position[1] += 0.06f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+61 || Type == MODEL_POTION+62)
	{
		Position[1] += 0.06f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_POTION+53 )
	{
		Position[1] += 0.042f;
		Vector(180.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+43 )
	{
		Position[1] -= 0.027f;
		Position[0] += 0.005f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+44 )
	{
		Position[1] -= 0.03f;
		Position[0] += 0.005f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+45 )
	{
		Position[1] -= 0.02f;
		Position[0] += 0.005f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_POTION+70 && Type <= MODEL_POTION+71 )
	{
		Position[0] += 0.01f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_POTION+72 && Type <= MODEL_POTION+77 )
	{
		Position[1] += 0.08f;
		Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+59 )
	{
		Position[0] += 0.01f;
		Position[1] += 0.02f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_HELPER+54 && Type <= MODEL_HELPER+58 )
	{
  		Position[1] -= 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_POTION+78 && Type <= MODEL_POTION+82 )
	{
		Position[1] += 0.01f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+60 )
	{
		Position[1] -= 0.06f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+61 )
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+83)
	{
		Position[1] += 0.06f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+91)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+92)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+93)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+95)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_POTION+94 )
	{
		Position[0] += 0.01f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_POTION+84 && Type <= MODEL_POTION+90 )
	{
		if( Type == MODEL_POTION+84 )
		{
			Position[1] += 0.01f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+85 )
		{
			Position[1] -= 0.01f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+86 )
		{
			Position[1] += 0.01f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+87 )
		{
			Position[1] += 0.01f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+88 )
		{
			Position[1] += 0.015f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+89 )
		{
			Position[1] += 0.015f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+90 )
		{
			Position[1] += 0.015f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
	}
	else if(Type == MODEL_HELPER+62)
	{
		Position[0] += 0.01f;
		Position[1] -= 0.03f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+63)
	{
		Position[0] += 0.01f;
		Position[1] += 0.082f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type >= MODEL_POTION+97 && Type <= MODEL_POTION+98)
	{
		Position[1] += 0.09f;
		Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_POTION+96 ) 
	{
		Position[1] -= 0.013f;
		Position[0] += 0.003f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( MODEL_HELPER+64 <= Type && Type <= MODEL_HELPER+65 )
	{
		switch(Type)
		{
		case MODEL_HELPER+64:
			Position[1] -= 0.05f;
			break;
		case MODEL_HELPER+65: 
			Position[1] -= 0.02f;
			break;
		}
		Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type == MODEL_POTION+65)
	{
		Position[1] += 0.05f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type == MODEL_POTION+66)
	{
		Position[1] += 0.11f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type == MODEL_POTION+67)
	{
		Position[1] += 0.11f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+39)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+43 )
	{
//		Position[1] += 0.082f;
		Position[1] -= 0.03f;
		Vector(90.f, 0.f, 180.f, ObjectSelect.Angle);
	}
 	else if( Type == MODEL_HELPER+44 )
	{
		Position[1] += 0.08f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+45 )
	{
		Position[1] += 0.07f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+40)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+41)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+51)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+42)
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type==MODEL_HELPER+38 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.02f;
		Vector( -48-150.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+41)
	{
		Position[1] += 0.02f;
		Vector(270.f, 90.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type==MODEL_POTION+42 )
	{
		Position[1] += 0.02f;
		Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type==MODEL_POTION+43 || Type==MODEL_POTION+44 )
	{
		Position[0] -= 0.04f;
		Position[1] += 0.02f;
		Position[2] += 0.02f;
		Vector( 270.f, -10.f, -45.f, ObjectSelect.Angle );
	}
	else if(Type>=MODEL_HELPER+12 && Type<MODEL_HELPER+MAX_ITEM_INDEX && Type!=MODEL_HELPER+14  && Type!=MODEL_HELPER+15)
	{
		Vector(270.f+90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_POTION+12)
	{
		switch(Level)
		{
		case 0:Vector(180.f,0.f,0.f,ObjectSelect.Angle);break;
		case 1:Vector(270.f,90.f,0.f,ObjectSelect.Angle);break;
		case 2:Vector(90.f,0.f,0.f,ObjectSelect.Angle);break;
		}
	}
	else if(Type==MODEL_EVENT+5)
	{
		Vector(270.f,180.f,0.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_EVENT+6)
	{
		Vector(270.f,90.f,0.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_EVENT+7)
	{
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_POTION+20)
	{
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
    else if ( Type==MODEL_POTION+27 )
    {
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
    }
	else if ( Type == MODEL_POTION+63 )
	{
		Position[1] += 0.08f;
		Vector(-50.f,-60.f,0.f,ObjectSelect.Angle);
	}
	else if ( Type == MODEL_POTION+52)
	{
		//Position[1] += 0.08f;
		Vector(270.f,-25.f,0.f,ObjectSelect.Angle);
	}
	else if(Type >= MODEL_ETC+19 && Type <= MODEL_ETC+27)
	{
		Position[0] += 0.03f;
		Position[1] += 0.03f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+7)
	{
		Position[0] += 0.005f;
		Position[1] -= 0.015f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+10)
	{
		Position[1] -= 0.1f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_PANTS+10)
	{
		Position[1] -= 0.08f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+11)
	{
		Position[1] -= 0.1f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_PANTS+11)
	{
		Position[1] -= 0.08f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+44)
	{
		Position[0] += 0.005f;
		Position[1] -= 0.015f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+46 || Type==MODEL_WING+45)
	{
		Position[0] += 0.005f;
		Position[1] -= 0.015f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
    else
	{
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}

	if(Type >= MODEL_WING+60 && Type <= MODEL_WING+65)
	{
		Vector(10.f,-10.f,10.f,ObjectSelect.Angle);
	}
	else if(Type >= MODEL_WING+70 && Type <= MODEL_WING+74)
	{
		Vector(0.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type >= MODEL_WING+100 && Type <= MODEL_WING+129)
	{
		Vector(0.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+67 )
	{
		Position[1] -= 0.05f;
		Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+123 )
	{
		Position[1] -= 0.05f;
		Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+140)
	{
		Position[1] += 0.09f;
		Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type >= MODEL_POTION+145 && Type <= MODEL_POTION+150)
	{
		Position[0] += 0.010f;
		Position[1] += 0.040f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type >= MODEL_HELPER+125 && Type <= MODEL_HELPER+127)
	{
		Position[0] += 0.007f;
		Position[1] -= 0.035f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type == MODEL_HELPER+124)
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+80 )
	{
		Position[1] -= 0.05f;
		Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+106 )
	{
		Position[0] += 0.01f;
		Position[1] -= 0.05f;
		Vector(255.f, 45.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+68 )
	{
		Position[0] += 0.02f;
		Position[1] -= 0.02f;
		Vector(300.f, 10.f, 20.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+76 )
	{
//		Position[0] += 0.02f;
		Position[1] -= 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+122 )
	{
		Position[0] += 0.01f;
		Position[1] -= 0.035f;
		Vector(290.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+128 )
	{
		Position[0] += 0.017f;
		Position[1] -= 0.053f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+129 )
	{
		Position[0] += 0.012f;
		Position[1] -= 0.045f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+134 )
	{
		Position[0] += 0.005f;
		Position[1] -= 0.033f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+130 )
	{
		Position[0] += 0.007f;
		Position[1] += 0.005f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+131 )
	{
		Position[0] += 0.017f;
		Position[1] -= 0.053f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+132 )
	{
		Position[0] += 0.007f;
		Position[1] += 0.045f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+133 )
	{
		Position[0] += 0.017f;
		Position[1] -= 0.053f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+69 )
	{
		Position[0] += 0.005f;
		Position[1] -= 0.05f;
		Vector(270.f, -30.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+70 )
	{
		Position[0] += 0.040f;
		Position[1] -= 0.000f;
		Vector(270.f, -0.f, 70.f, ObjectSelect.Angle);
	}

	else if (Type == MODEL_HELPER+81)
	{
		Position[0] += 0.005f;
		Position[1] += 0.035f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type == MODEL_HELPER+82)
	{
		Position[0] += 0.005f;
		Position[1] += 0.035f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type == MODEL_HELPER+93)
	{
		Position[0] += 0.005f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type == MODEL_HELPER+94)
	{
		Position[0] += 0.005f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+66 )
	{
		Position[0] += 0.01f;
		Position[1] -= 0.05f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_POTION+100 )
	{
		Position[0] += 0.01f;
		Position[1] -= 0.05f;
		Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if (Type == MODEL_POTION+99)
	{
		Position[0] += 0.02f;
		Position[1] -= 0.03f;
		//Vector(270.f,0.f,30.f,ObjectSelect.Angle);
		Vector(290.f,-40.f,0.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_STAFF+33 )
	{
		Position[0] += 0.02f;
		Position[1] -= 0.06f;
		Vector(180.f,90.f,10.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_STAFF+34 )
	{
		Position[1] -= 0.05f;
		Vector(180.f,90.f,10.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_SPEAR+11 )
	{
		Position[1] += 0.02f;
		Vector(180.f,90.f,15.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_MACE+18 )
	{
		Position[0] -= 0.03f;
		Position[1] += 0.06f;
		Vector(180.f,90.f,2.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_BOW+24 )
	{
		Position[0] -= 0.07f;
		Position[1] += 0.07f;
    	Vector(180.f,-90.f,15.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+47)
	{
		Position[0] += 0.005f;
		Position[1] -= 0.015f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if ( Type==MODEL_HELPER+71 || Type==MODEL_HELPER+72 || Type==MODEL_HELPER+73 || Type==MODEL_HELPER+74 || Type==MODEL_HELPER+75 )
	{
		Position[1] += 0.07f;
      	Vector(270.f,180.f,0.f,ObjectSelect.Angle);
		if(Select != 1)
		{
			ObjectSelect.Angle[1] = WorldTime*0.2f;
		}
	}
	else if( Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN	&& Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END )
	{
		Position[1] -= 0.03f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+97 || Type == MODEL_HELPER+98 || Type == MODEL_POTION+91)
	{
		Position[1] -= 0.04f;
		Position[0] += 0.002f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+99)
	{
		Position[0] += 0.002f;
		Position[1] += 0.025f;
		Vector(270.0f, 180.0f, 45.0f, ObjectSelect.Angle);
	}
	else if ( Type==MODEL_POTION+110 )
	{
		Position[0] += 0.005f;
		Position[1] -= 0.02f;
	}
	else if ( Type==MODEL_POTION+111 )
	{
		Position[0] += 0.01f;
		Position[1] -= 0.02f;
	}
	else if(Type == MODEL_HELPER+107)
	{
		Position[0] -= 0.0f;
		Position[1] += 0.0f;
		Vector(90.0f, 225.0f, 45.0f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+104)
	{
		Position[0] += 0.01f;
		Position[1] -= 0.03f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+105)
	{
		Position[0] += 0.01f;
		Position[1] -= 0.03f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+103)
	{
		Position[0] += 0.01f;
		Position[1] += 0.01f;
		Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+133)
	{
		Position[0] += 0.01f;
		Position[1] -= 0.0f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if( MODEL_POTION+101 <= Type && Type <= MODEL_POTION+109 )
	{
		switch(Type)
		{
		case MODEL_POTION+101:
			{
				Position[0] += 0.005f;
				//Position[1] -= 0.02f;
			}break;
		case MODEL_POTION+102:
			{
				Position[0] += 0.005f;
				Position[1] += 0.05f;
				Vector(0.0f, 0.0f, 30.0f, ObjectSelect.Angle);
			}break;
		case MODEL_POTION+103:
		case MODEL_POTION+104: 
		case MODEL_POTION+105: 
		case MODEL_POTION+106: 
		case MODEL_POTION+107: 
		case MODEL_POTION+108: 
			{
				Position[0] += 0.005f;
				Position[1] += 0.05f;
				Vector(0.0f, 0.0f, 30.0f, ObjectSelect.Angle);
			}break;
		case MODEL_POTION+109:
			{
				Position[0] += 0.005f;
				Position[1] += 0.05f;
				Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
			}break;
		}
	}
	else if( Type >= MODEL_HELPER+109 && Type <= MODEL_HELPER+112 )
	{
		Position[0] += 0.025f;
		Position[1] -= 0.035f;
		Vector(270.0f, 25.0f, 25.0f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_HELPER+113 && Type <= MODEL_HELPER+115 )
	{
		Position[0] += 0.005f;
		Position[1] -= 0.00f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_POTION+112 && Type <= MODEL_POTION+113 )
	{
 		Position[0] += 0.05f;
 		Position[1] += 0.009f;
		Vector(270.0f, 180.0f, 45.0f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_POTION+120 )
	{
		Position[0] += 0.01f;
		Position[1] += 0.05f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
		
	}
	else if( MODEL_POTION+134 <= Type && Type <= MODEL_POTION+139 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.05f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+116 )
	{

		Position[1] -= 0.03f;
		Position[0] += 0.005f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_POTION+114 && Type <= MODEL_POTION+119 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.06f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_POTION+126 && Type <= MODEL_POTION+129 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.06f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if( Type >= MODEL_POTION+130 && Type <= MODEL_POTION+132 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.06f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+121 )
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	else if(Type == MODEL_HELM+59)
	{
		Position[1] += 0.04f;
		Vector(-90.f,25.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMORINVEN_60
		|| Type ==  MODEL_ARMORINVEN_61
		|| Type == MODEL_ARMORINVEN_62)
	{
		Position[0] += 0.01f;
		Position[1] += 0.08f;
		Vector(0.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_SWORD+32)
	{
		Position[0] += 0.005f;
		Position[1] += 0.015f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type >= MODEL_SWORD+33 && Type <= MODEL_SWORD+34)
	{
		Position[0] += 0.002f;
		Position[1] += 0.02f;
		Vector(0.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_WING+49)
	{
		Position[1] += 0.01f;
		Position[0] += 0.015f;
		Vector ( -90.f, 0.f, 0.f, ObjectSelect.Angle );
	}
	else if(Type==MODEL_WING+50)
	{
		Position[1] += 0.15f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
	else if(Type >= MODEL_ETC+30 && Type <= MODEL_ETC+36)
	{
		Position[0] += 0.03f;
		Position[1] += 0.03f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_WING+135)
	{
		Position[1] += 0.05f;
		Position[0] += 0.005f;
	}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#ifdef LEM_ADD_LUCKYITEM
	else if( Type >= MODEL_HELPER+135 && Type <= MODEL_HELPER+145 )
	{
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_POTION+160 || Type == MODEL_POTION+161 )
	{
		Position[1] += 0.05f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif // LEM_ADD_LUCKYITEM
	else if( COMGEM::Check_Jewel_Com(Type-MODEL_ITEM) != COMGEM::NOGEM)
	{
		Vector(270.f, -10.f, 0.f, ObjectSelect.Angle);
		switch( COMGEM::Check_Jewel_Com(Type-MODEL_ITEM) )
		{
		case COMGEM::eLOW_C:
		case COMGEM::eUPPER_C:
			Vector( 270.f, -10.f, -45.f, ObjectSelect.Angle );
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
	case MODEL_SWORD+26:
		{
			Position[0] -= 0.02f;
			Position[1] += 0.04f;
			Vector(180.f,270.f,10.f,ObjectSelect.Angle);
		}break;
	case MODEL_SWORD+27:
		{
			Vector(180.f,270.f,15.f,ObjectSelect.Angle);
		}break;
	case MODEL_SWORD+28:
		{
			Position[1] += 0.02f;
			Vector(180.f,270.f,10.f,ObjectSelect.Angle);
		}break;
	case MODEL_MACE+16:
		{
			Position[0] -= 0.02f;
			Vector(180.f,270.f,15.f,ObjectSelect.Angle);
		}
		break;
	case MODEL_MACE+17:
		{
			Position[0] -= 0.02f;
			Position[1] += 0.04f;
			Vector(180.f,270.f,15.f,ObjectSelect.Angle);
		}break;
	case MODEL_STAFF+30:
		{
			Vector(180.f,90.f,10.f,ObjectSelect.Angle);
		}break;
	case MODEL_STAFF+31:
		{
			Vector(180.f,90.f,10.f,ObjectSelect.Angle);
		}break;
	case MODEL_STAFF+32:
		{
			Vector(180.f,90.f,10.f,ObjectSelect.Angle);
		}break;
	}

	switch(Type)
	{
	case MODEL_STAFF+7:
		{
			Vector(0.f,0.f,205.f,ObjectSelect.Angle);
		}break;
	}

	switch(Type)
	{
	case MODEL_WING+8:
	case MODEL_WING+9:
	case MODEL_WING+10:
	case MODEL_WING+11:
		{
			Position[0] += 0.005f;
			Position[1] -= 0.02f;
		}
		break;
	case MODEL_POTION+21:
		{
			Position[0] += 0.005f;
			Position[1] -= 0.005f;
		}
		break;
	case MODEL_POTION+13:
	case MODEL_POTION+14:
	case MODEL_POTION+22:
		{			
			Position[0] += 0.005f;
			Position[1] += 0.015f;
		}
		break;
	}

	if(1==Select)
	{
		ObjectSelect.Angle[1] = WorldTime*0.45f;
	}

	ObjectSelect.Type = Type;
	if(ObjectSelect.Type>=MODEL_HELM && ObjectSelect.Type<MODEL_BOOTS+MAX_ITEM_INDEX
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| ObjectSelect.Type == MODEL_ARMORINVEN_60
		|| ObjectSelect.Type == MODEL_ARMORINVEN_61
		|| ObjectSelect.Type == MODEL_ARMORINVEN_62
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
		ObjectSelect.Type = MODEL_PLAYER;
	else if(ObjectSelect.Type==MODEL_POTION+12)
	{
		if(Level==0)
		{
			ObjectSelect.Type = MODEL_EVENT;
			Type = MODEL_EVENT;
		}
		else if(Level==2)
		{
			ObjectSelect.Type = MODEL_EVENT+1;
			Type = MODEL_EVENT+1;
		}
	}
	
	BMD *b = &Models[ObjectSelect.Type];
	b->CurrentAction                 = 0;
	ObjectSelect.AnimationFrame      = 0;
	ObjectSelect.PriorAnimationFrame = 0;
	ObjectSelect.PriorAction         = 0;
	if(Type >= MODEL_HELM && Type<MODEL_HELM+MAX_ITEM_INDEX)
	{
		b->BodyHeight = -160.f;

#ifdef LEM_ADD_LUCKYITEM
		if( Check_LuckyItem( Type - MODEL_ITEM ))				b->BodyHeight-=10.0f;
		if( Type == MODEL_HELM+65 || Type == MODEL_HELM+70 )	Position[0] += 0.04f;
#endif // LEM_ADD_LUCKYITEM
	}
	else if(Type >= MODEL_ARMOR && Type<MODEL_ARMOR+MAX_ITEM_INDEX)
	{
		b->BodyHeight = -100.f;

#ifdef LEM_ADD_LUCKYITEM
		if( Check_LuckyItem( Type - MODEL_ITEM ))	b->BodyHeight-=13.0f;
#endif // LEM_ADD_LUCKYITEM
	}
	else if(Type >= MODEL_GLOVES && Type<MODEL_GLOVES+MAX_ITEM_INDEX)
		b->BodyHeight = -70.f;
	else if(Type >= MODEL_PANTS && Type<MODEL_PANTS+MAX_ITEM_INDEX)
		b->BodyHeight = -50.f;
	else
		b->BodyHeight = 0.f;
	float Scale  = 0.f;

	if(Type>=MODEL_HELM && Type<MODEL_BOOTS+MAX_ITEM_INDEX)
	{
		if(Type>=MODEL_HELM && Type<MODEL_HELM+MAX_ITEM_INDEX)			
		{
			Scale = MODEL_HELM+39 <= Type && MODEL_HELM+44 >= Type ? 0.007f : 0.0039f;
			if( Type == MODEL_HELM+31)
				Scale = 0.007f;

#ifdef LEM_ADD_LUCKYITEM
			if( Type == MODEL_HELM+65 || Type == MODEL_HELM+70 )	Scale = 0.007f;
#endif // LEM_ADD_LUCKYITEM
		}
		else if(Type>=MODEL_ARMOR && Type<MODEL_ARMOR+MAX_ITEM_INDEX)
      		Scale = 0.0039f;
		else if(Type>=MODEL_GLOVES && Type<MODEL_GLOVES+MAX_ITEM_INDEX)
      		Scale = 0.0038f;
		else if(Type>=MODEL_PANTS && Type<MODEL_PANTS+MAX_ITEM_INDEX)
      		Scale = 0.0033f;
		else if(Type>=MODEL_BOOTS && Type<MODEL_BOOTS+MAX_ITEM_INDEX)
      		Scale = 0.0032f;
		else if (Type == MODEL_ARMOR+30)
			Scale = 0.0035f;
		else if (Type == MODEL_ARMOR+32)
			Scale = 0.0035f;
		else if (Type == MODEL_ARMOR+29)
			Scale = 0.0033f;
		if(Type == MODEL_ARMOR+34)
			Scale = 0.0032f;
		else if(Type == MODEL_ARMOR+35)
			Scale = 0.0032f;
		else if(Type == MODEL_GLOVES+38)
			Scale = 0.0032f;
	}
	else
	{
        if(Type==MODEL_WING+6)
            Scale = 0.0015f;
		else if( COMGEM::Check_Jewel_Com(Type-MODEL_ITEM) != COMGEM::NOGEM)
		{
			Scale = 0.004f;
			switch( COMGEM::Check_Jewel_Com(Type-MODEL_ITEM) )
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
		else if ( Type>=MODEL_WING+32 && Type<=MODEL_WING+34)
		{
			Scale = 0.001f;
			Position[1] -= 0.05f;
		}
		else if(Type >= MODEL_WING+60 && Type <= MODEL_WING+65)
			Scale = 0.0022f; 
		else if(Type >= MODEL_WING+70 && Type <= MODEL_WING+74)
			Scale = 0.0017f; 
		else if(Type >= MODEL_WING+100 && Type <= MODEL_WING+129)
			Scale = 0.0017f; 
		else if(Type>=MODEL_WING && Type<MODEL_WING+MAX_ITEM_INDEX)
		{
      			Scale = 0.002f;
		}
		else
		if ( Type==MODEL_POTION+45 || Type==MODEL_POTION+49)
		{
			Scale = 0.003f;
		}
		else
		if(Type>=MODEL_POTION+46 && Type<=MODEL_POTION+48)
		{
			Scale = 0.0025f;
		}
		else
		if(Type == MODEL_POTION+50)
		{
			Scale = 0.001f;
		}
		else
		if ( Type>=MODEL_POTION+32 && Type<=MODEL_POTION+34)
		{
			Scale = 0.002f;
			Position[1] += 0.05f;
   			Vector(0.f,ObjectSelect.Angle[1],0.f,ObjectSelect.Angle);
		}
		else
		if ( Type>=MODEL_EVENT+21 && Type<=MODEL_EVENT+23)
		{
			Scale = 0.002f;
			if(Type==MODEL_EVENT+21)
				Position[1] += 0.08f;
			else
				Position[1] += 0.06f;
   			Vector(0.f,ObjectSelect.Angle[1],0.f,ObjectSelect.Angle);
		}
        else if(Type==MODEL_POTION+21)
			Scale = 0.002f;
		else if(Type == MODEL_BOW+19)
			Scale = 0.002f;
        else if(Type==MODEL_EVENT+11)
			Scale = 0.0015f;
        else if ( Type==MODEL_HELPER+4 )
			Scale = 0.0015f;
        else if ( Type==MODEL_HELPER+5 )
			Scale = 0.005f;
        else if ( Type==MODEL_HELPER+30 )    
			Scale = 0.002f;
        else if ( Type==MODEL_EVENT+16 )
 			Scale = 0.002f;
		else if(Type==MODEL_HELPER+16)
			Scale = 0.002f;
		else if(Type==MODEL_HELPER+17)
			Scale = 0.0018f;
		else if(Type==MODEL_HELPER+18)
			Scale = 0.0018f;
		else if(Type == MODEL_HELPER+46)
		{
			Scale = 0.0018f;
		}
		else if(Type == MODEL_HELPER+47)
		{
			Scale = 0.0018f;
		}
		else if(Type == MODEL_HELPER+48)
		{
			Scale = 0.0018f;
		}
		else if(Type == MODEL_POTION+54)
		{
			Scale = 0.0024f;
		}
		else if(Type == MODEL_POTION+58)
		{
			Scale = 0.0012f;
		}
		else if(Type == MODEL_POTION+59 || Type == MODEL_POTION+60)
		{
			Scale = 0.0010f;
		}
		else if(Type == MODEL_POTION+61 || Type == MODEL_POTION+62)
		{
			Scale = 0.0009f;
		}
		else if( Type == MODEL_POTION+53 )
		{
			Scale = 0.00078f;
		}
		else if( Type == MODEL_HELPER+43 || Type == MODEL_HELPER+44 || Type == MODEL_HELPER+45 )
		{
			Scale = 0.0021f;
		}
		else if( Type >= MODEL_POTION+70 && Type <= MODEL_POTION+71 )
		{
			Scale = 0.0028f;
		}
		else if( Type >= MODEL_POTION+72 && Type <= MODEL_POTION+77 )
		{
			Scale = 0.0025f;
		}
		else if( Type == MODEL_HELPER+59 )
		{
			Scale = 0.0008f;
		}
		else if( Type >= MODEL_HELPER+54 && Type <= MODEL_HELPER+58 )
		{
			Scale = 0.004f;
		}
		else if( Type >= MODEL_POTION+78 && Type <= MODEL_POTION+82 )
		{
			Scale = 0.0025f;
		}
		else if( Type == MODEL_HELPER+60 )
		{
			Scale = 0.005f;
		}
		else if( Type == MODEL_HELPER+61 )
		{
			Scale = 0.0018f;
		}
		else if(Type == MODEL_POTION+83)
		{
			Scale = 0.0009f;
		}
		else if( Type == MODEL_HELPER+43 || Type == MODEL_HELPER+44 || Type == MODEL_HELPER+45 )
		{
			Scale = 0.0021f;
		}
		else if(Type == MODEL_POTION+91)
		{
			Scale = 0.0034f;
		}
		else if(Type == MODEL_POTION+92)
		{
			Scale = 0.0024f;
		}
		else if(Type == MODEL_POTION+93)
		{
			Scale = 0.0024f;
		}
		else if(Type == MODEL_POTION+95)
		{
			Scale = 0.0024f;
		}
		else if( Type == MODEL_POTION+94 )
		{
			Scale = 0.0022f;
		}
		else if( Type == MODEL_POTION+84 )
		{
			Scale = 0.0031f;
		}
		else if( Type == MODEL_POTION+85 )
		{
			Scale = 0.0044f;
		}
		else if( Type == MODEL_POTION+86 )
		{
			Scale = 0.0031f;
		}
		else if( Type == MODEL_POTION+87 )
		{
			Scale = 0.0061f;
		}
		else if( Type == MODEL_POTION+88 )
		{
			Scale = 0.0035f;
		}
		else if( Type == MODEL_POTION+89 )
		{
			Scale = 0.0035f;
		}
		else if( Type == MODEL_POTION+90 )
		{
			Scale = 0.0035f;
		}
		else if(Type >= MODEL_HELPER+62 && Type <= MODEL_HELPER+63)
		{
			Scale = 0.002f;
		}
		else if(Type >= MODEL_POTION+97 && Type <= MODEL_POTION+98)
		{
			Scale = 0.003f;
		}
		else if( Type == MODEL_POTION+96 ) 
		{
			Scale = 0.0028f;
		}
		else if( MODEL_HELPER+64 == Type || Type == MODEL_HELPER+65 )
		{
			switch(Type)
			{
			case MODEL_HELPER+64: Scale = 0.0005f; break;
			case MODEL_HELPER+65: Scale = 0.0016f; break;
			}			
		}
		else if( Type == MODEL_HELPER+67 )
		{
			Scale = 0.0015f;
		}
		else if( Type == MODEL_HELPER+80 )
		{
			Scale = 0.0020f;
		}
		else if( Type == MODEL_HELPER+68 )
		{
			Scale = 0.0026f;
		}
		else if( Type == MODEL_HELPER+76 )
		{
			Scale = 0.0026f;
		}
		else if( Type == MODEL_HELPER+69 )
		{
			Scale = 0.0023f;
		}
		else if( Type == MODEL_HELPER+70 )
		{
			Scale = 0.0018f;
		}
		else if (Type == MODEL_HELPER+81)
			Scale = 0.0012f;
		else if (Type == MODEL_HELPER+82)
			Scale = 0.0012f;
		else if (Type == MODEL_HELPER+93)
			Scale = 0.0021f;
		else if (Type == MODEL_HELPER+94)
			Scale = 0.0021f;
        else if(Type==MODEL_SWORD+19)
        {
            if ( ItemLevel>=0 )
            {
                Scale = 0.0025f;
            }
            else
            {
                Scale = 0.001f;
                ItemLevel = 0;
            }
        }
        else if(Type==MODEL_STAFF+10)
        {
            if ( ItemLevel>=0 )
            {
                Scale = 0.0019f;
            }
            else
            {
                Scale = 0.001f;
                ItemLevel = 0;
            }
        }
        else if(Type==MODEL_BOW+18)
        {
            if ( ItemLevel>=0 )
            {
                Scale = 0.0025f;
            }
            else
            {
                Scale = 0.0015f;
                ItemLevel = 0;
            }
        }
        else if ( Type>=MODEL_MACE+8 && Type<=MODEL_MACE+11 )
        {
            Scale = 0.003f;
        }
		else if(Type == MODEL_MACE+12)
		{
			Scale = 0.0025f;
		}
		else if( Type == MODEL_MACE+18 )
		{
			Scale = 0.0024f;
		}
		else if(Type == MODEL_EVENT+12)
		{
			Scale = 0.0012f;
		}
		else if(Type == MODEL_EVENT+13)
		{
			Scale = 0.0025f;
		}
		else if ( Type == MODEL_EVENT+14)
		{
			Scale = 0.0028f;
		}
		else if ( Type == MODEL_EVENT+15)
		{
			Scale = 0.0023f;
		}
        else if ( Type>=MODEL_POTION+22 && Type<MODEL_POTION+25 )
        {
            Scale = 0.0025f;
        }
        else if ( Type>=MODEL_POTION+25 && Type<MODEL_POTION+27 )
        {
            Scale = 0.0028f;
        }
		else if ( Type == MODEL_POTION+63)
		{
			Scale = 0.007f;
		}
		else if ( Type == MODEL_POTION+99)
		{
			Scale = 0.0025f;
		}
		else if ( Type == MODEL_POTION+52)
		{
			Scale = 0.0014f;
		}
		else if( Type==MODEL_HELPER+38 )
		{
			Scale = 0.0025f;
		}
		else if( Type==MODEL_POTION+41 )
		{
			Scale = 0.0035f;
		}
		else if( Type==MODEL_POTION+42 )
		{
			Scale = 0.005f;
		}
		else if( Type==MODEL_POTION+43 )
		{
			Position[1] += -0.005f;
			Scale = 0.0035f;
		}
		else if( Type==MODEL_POTION+44 )
		{
			Position[1] += -0.005f;
			Scale = 0.004f;
		}
        else if ( Type==MODEL_POTION+7 )
        {
            Scale = 0.0025f;
        }
		else if( Type == MODEL_HELPER+43 || Type == MODEL_HELPER+44 || Type == MODEL_HELPER+45 )
		{
			Scale = 0.0021f;
		}
        else if ( Type==MODEL_HELPER+7 )
        {
            Scale = 0.0025f;
        }
        else if ( Type==MODEL_HELPER+11 )
        {
            Scale = 0.0025f;
        }
		else if(Type == MODEL_HELPER+32)
		{
			Scale = 0.0019f;
		}
		else if(Type == MODEL_HELPER+33)
		{
			Scale = 0.004f;
		}
		else if(Type == MODEL_HELPER+34)
		{
			Scale = 0.004f;
		}
		else if(Type == MODEL_HELPER+35)
		{
			Scale = 0.004f;
		}
		else if(Type == MODEL_HELPER+36)
		{
			Scale = 0.007f;
		}
		else if(Type == MODEL_HELPER+37)
		{
			Scale = 0.005f;
		}
		else if( Type == MODEL_BOW+21)
		{
			Scale = 0.0022f;
		}
		else if(Type == MODEL_HELPER+49)
		{
			Scale = 0.0013f;
		}
		else if(Type == MODEL_HELPER+50)
		{
			Scale = 0.003f;
		}
		else if(Type == MODEL_HELPER+51)
		{
			Scale = 0.003f;
		}
		else if(Type == MODEL_POTION+64)
		{
			Scale = 0.003f;
		}
		else if (Type == MODEL_POTION+65)
			Scale = 0.003f;
		else if (Type == MODEL_POTION+66)
			Scale = 0.0035f;
		else if (Type == MODEL_POTION+67)
			Scale = 0.0035f;
		else if (Type == MODEL_POTION+68)
			Scale = 0.003f;
		else if (Type == MODEL_HELPER+52)
			Scale = 0.005f;
		else if (Type == MODEL_HELPER+53)
			Scale = 0.005f; 
		else if(Type == MODEL_SWORD+24)
		{
			Scale = 0.0028f;
		}
		else if(Type == MODEL_BOW+22)
		{
			Scale = 0.0020f;
		}
		else if( Type == MODEL_BOW+23 )
		{
			Scale = 0.0032f;
		}
        else if( Type==MODEL_HELPER+14 || Type==MODEL_HELPER+15 )
        {
            Scale = 0.003f;
        }
		else if(Type == MODEL_POTION+100)
		{
			Scale = 0.0040f;
		}
		else if(Type>=MODEL_POTION && Type<MODEL_POTION+MAX_ITEM_INDEX)
		{
      		Scale = 0.0035f;
		}
		else if(Type>=MODEL_SPEAR && Type<MODEL_SPEAR+MAX_ITEM_INDEX)
        {
			if(Type == MODEL_SPEAR+10)
				Scale = 0.0018f;
			else if( Type == MODEL_SPEAR+11 )
				Scale = 0.0025f;
			else
				Scale = 0.0021f;
		}
		else if(Type>=MODEL_STAFF && Type<MODEL_STAFF+MAX_ITEM_INDEX)
		{
			if (Type >= MODEL_STAFF+14 && Type <= MODEL_STAFF+20)
				Scale = 0.0028f;
			else if (Type >= MODEL_STAFF+21 && Type <= MODEL_STAFF+29)
				Scale = 0.004f;
			else if( Type == MODEL_STAFF+33 )
				Scale = 0.0028f;
			else if( Type == MODEL_STAFF+34 )
				Scale = 0.0028f;
			else
      			Scale = 0.0022f;
		}
        else if(Type==MODEL_BOW+15)
      		Scale = 0.0011f;
		else if(Type==MODEL_BOW+7)
      		Scale = 0.0012f;
		else if(Type==MODEL_EVENT+6)
      		Scale = 0.0039f;
		else if(Type==MODEL_EVENT+8)
			Scale = 0.0015f;
		else if(Type==MODEL_EVENT+9)
			Scale = 0.0019f;
		else
		{
			Scale = 0.0025f;
		}

		if( Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN
			&& Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END )
		{
			Scale = 0.0020f;
		}

		if(Type==MODEL_EVENT+10)
		{
			Scale = 0.001f;
		}
		else if(Type >= MODEL_ETC+19 && Type <= MODEL_ETC+27)
		{
			Scale = 0.0023f;
		}
		else if(Type == MODEL_HELPER+66)
		{
			Scale = 0.0020f;
		}
		else if(Type == MODEL_POTION+140)
		{
			Scale = 0.0026f;
		}
		else if( Type == MODEL_HELPER+122 )
		{
			Scale = 0.0033f;
		}
		else if( Type == MODEL_HELPER+123 )
		{
			Scale = 0.0009f;
		}
		else if(Type >= MODEL_POTION+145 && Type <= MODEL_POTION+150)
		{
			Scale = 0.0018f;
		}
		else if(Type >= MODEL_HELPER+125 && Type <= MODEL_HELPER+127)
		{
			Scale = 0.0013f;
		}
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM	
		else if( Type == MODEL_HELPER+128 )		// 매조각상
		{
			Scale = 0.0035f;
		}
		else if( Type == MODEL_HELPER+129 )		// 양조각상
		{
			Scale = 0.0035f;
		}
		else if( Type == MODEL_HELPER+134 )		// 편자
		{
			Scale = 0.0033f;
		}
#endif	//LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
		else if( Type == MODEL_HELPER+130 )		// 오크참
		{
			Scale = 0.0032f;
		}
		else if( Type == MODEL_HELPER+131 )		// 메이플참
		{
			Scale = 0.0033f;
		}
		else if( Type == MODEL_HELPER+132 )		// 골든오크참
		{
			Scale = 0.0025f;
		}
		else if( Type == MODEL_HELPER+133 )		// 골든메이플참
		{
			Scale = 0.0033f;
		}
#endif	//LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
		else if ( Type==MODEL_HELPER+71 || Type==MODEL_HELPER+72 || Type==MODEL_HELPER+73 || Type==MODEL_HELPER+74 || Type==MODEL_HELPER+75 )
		{
			Scale = 0.0019f;
		}
		else if( Type == MODEL_BOW+24 )
		{
			Scale = 0.0023f;
		}
		else if(Type == MODEL_HELPER+97 || Type == MODEL_HELPER+98 || Type == MODEL_POTION+91)
		{
			Scale = 0.0028f;
		}
		else if(Type == MODEL_HELPER+99)
		{
			Scale = 0.0025f;
		}
		else if (Type == MODEL_POTION+110)
		{
			Scale = 0.004f;
		}
		else if(Type == MODEL_HELPER+107)
		{
			Scale = 0.0034f;
		}
		else if(Type == MODEL_POTION+133)
		{
			Scale = 0.0030f;
		}
		else if(Type == MODEL_HELPER+105)
		{
			Scale = 0.002f;
		}
		else if( MODEL_POTION+101 <= Type && Type <= MODEL_POTION+109 )
		{
			switch(Type)
			{
			case MODEL_POTION+101:
				{
					Scale = 0.004f;
				}break;
			case MODEL_POTION+102:
				{
					Scale = 0.005f;
				}break;
			case MODEL_POTION+103:
			case MODEL_POTION+104: 
			case MODEL_POTION+105: 
			case MODEL_POTION+106: 
			case MODEL_POTION+107: 
			case MODEL_POTION+108: 
				{
					Scale = 0.004f;
				}break;
			case MODEL_POTION+109:
				{
					Scale = 0.003f;
				}break;
			}
		}
		else if( Type == MODEL_HELPER+106 )
		{
			Scale = 0.0015f;
		}	
		else if( Type == MODEL_WING+130 )
		{
			Scale = 0.0012f;
		}
		else if( Type >= MODEL_POTION+134 && Type <= MODEL_POTION+139 )
		{
			Scale = 0.0050f;
		}
		else if( Type >= MODEL_HELPER+109 && Type <= MODEL_HELPER+112  )
		{
			Scale = 0.0045f;
		}
	
		else if( Type >= MODEL_HELPER+113 && Type <= MODEL_HELPER+115 )
		{
			Scale = 0.0018f;
		}
		else if( Type >= MODEL_POTION+112 && Type <= MODEL_POTION+113 )
		{
			Scale = 0.0032f;
		}
		else if( Type == MODEL_HELPER+116 )
		{
			Scale = 0.0021f;
		}
		else if( Type >= MODEL_POTION+114 && Type <= MODEL_POTION+119 )
		{
			Scale = 0.0038f;
		}
		else if( Type >= MODEL_POTION+126 && Type <= MODEL_POTION+129 )
		{
			Scale = 0.0038f;
		}
		else if( Type >= MODEL_POTION+130 && Type <= MODEL_POTION+132 )
		{
			Scale = 0.0038f;
		}
		else if( Type == MODEL_HELPER+121 )
		{
			Scale = 0.0018f;
			//Scale = 1.f;
		}
		else if(Type == MODEL_HELPER+124)
			Scale = 0.0018f;
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		else if(Type >= MODEL_WING+49 && Type <= MODEL_WING+50)
		{
			Scale = 0.002f;
		}
		else if(Type == MODEL_WING+135)
		{
			Scale = 0.0012f;
		}
		else if(Type >= MODEL_SWORD+32 && Type <= MODEL_SWORD+34)
		{
			Scale = 0.0035f;
		}
		else if(Type >= MODEL_ETC+30 && Type <= MODEL_ETC+36)
		{
			Scale = 0.0023f;
		}
		else if(Type == MODEL_ARMORINVEN_60 || Type == MODEL_ARMORINVEN_62 || Type == MODEL_ARMORINVEN_61)
		{
			b->BodyHeight = -100.f;
			Scale = 0.0039f;
		}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 인벤토리 스케일 설정 [lem_2010.9.7]
		// LEM_TSET  상승의 보석, 연장의 보석 스케일[lem_2010.9.7]
		else if(Type >= MODEL_HELPER+135 && Type <= MODEL_HELPER+145)
		{
			Scale = 0.001f;
		}
		else if(Type == MODEL_POTION+160 || Type == MODEL_POTION+161)
		{
			Scale = 0.001f;
		}
		else if(Type == MODEL_HELM+62 || Type == MODEL_HELM+63 || Type == MODEL_HELM+65 || Type == MODEL_HELM+70)
		{
			Scale = 0.001f;
		}
#endif // LEM_ADD_LUCKYITEM
	}

	b->Animation(BoneTransform,ObjectSelect.AnimationFrame,ObjectSelect.PriorAnimationFrame,ObjectSelect.PriorAction,ObjectSelect.Angle,ObjectSelect.HeadAngle,false,false);

	CHARACTER Armor;
	OBJECT *o      = &Armor.Object;
	o->Type        = Type;
    ItemObjectAttribute(o);
	o->LightEnable = false;
	Armor.Class    = 2;

#ifdef PBG_ADD_ITEMRESIZE
	int ScreenPos_X=0, ScreenPos_Y=0;
	Projection(Position,&ScreenPos_X, &ScreenPos_Y);
#endif //PBG_ADD_ITEMRESIZE

	o->Scale = Scale;
		
	VectorCopy(Position,o->Position);
	
    vec3_t Light;
    float  alpha = o->Alpha;

    Vector(1.f,1.f,1.f,Light);

    RenderPartObject(o,Type,NULL,Light,alpha,ItemLevel,Option1,ExtOption,true,true,true);
}

void RenderItem3D(float sx,float sy,float Width,float Height,int Type,int Level,int Option1,int ExtOption,bool PickUp)
{
	bool Success = false;
	if((g_pPickedItem == NULL || PickUp) 
		&& SEASON3B::CheckMouseIn(sx, sy, Width, Height))
	{
#ifdef PBG_ADD_INGAMESHOPMSGBOX
		if(g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_INGAMESHOP))
		{
			Success = true;
		}
		else
#endif //PBG_ADD_INGAMESHOPMSGBOX
		{
			if( g_pNewUISystem->CheckMouseUse() == false )
				Success = true;
		}
	}

	if(Type>=ITEM_SWORD && Type<ITEM_SWORD+MAX_ITEM_INDEX)
	{
		sx += Width*0.8f;
		sy += Height*0.85f;
	}
	else if(Type>=ITEM_AXE && Type<ITEM_MACE+MAX_ITEM_INDEX)
	{
		if(Type==ITEM_MACE+13)
		{
			sx += Width*0.6f;
			sy += Height*0.5f;
		}
		else
		{
			sx += Width*0.8f;
			sy += Height*0.7f;
		}
	}
	else if(Type>=ITEM_SPEAR && Type<ITEM_SPEAR+MAX_ITEM_INDEX)
	{
		sx += Width*0.6f;
		sy += Height*0.65f;
	}
	else if( Type==ITEM_BOW+17 )
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if( Type==ITEM_BOW+19 )
	{
		sx += Width*0.7f;
		sy += Height*0.75f;
	}
	else if(Type==ITEM_BOW+20) 
	{
		sx += Width*0.5f;
		sy += Height*0.55f;
	}
	else if( Type>=ITEM_BOW+8 && Type<ITEM_BOW+MAX_ITEM_INDEX )
	{
		sx += Width*0.7f;
		sy += Height*0.7f;
	}
	else if(Type>=ITEM_STAFF && Type<ITEM_STAFF+MAX_ITEM_INDEX)
	{
		sx += Width*0.6f;
		sy += Height*0.55f;
	}
	else if(Type>=ITEM_SHIELD && Type<ITEM_SHIELD+MAX_ITEM_INDEX)
	{
		sx += Width*0.5f;
		if(Type == ITEM_SHIELD+15)
			sy += Height*0.7f;
		else if(Type == ITEM_SHIELD+16)
			sy += Height*0.9f;
		else if(Type == ITEM_SHIELD+21)
		{
			sx += Width*0.05f;
			sy += Height*0.5f;
		}
		else
			sy += Height*0.6f;
	}
	else if(Type>=ITEM_HELM && Type<ITEM_HELM+MAX_ITEM_INDEX)
	{
		sx += Width*0.5f;
		sy += Height*0.8f;
	}
	else if(Type>=ITEM_ARMOR && Type<ITEM_ARMOR+MAX_ITEM_INDEX)
	{
		sx += Width*0.5f;
		if(Type==ITEM_ARMOR+2 || Type==ITEM_ARMOR+4 || Type==ITEM_ARMOR+6)
		{
			sy += Height*1.05f;
		}
		else if(Type==ITEM_ARMOR+3 || Type==ITEM_ARMOR+8)
		{
			sy += Height*1.1f;
		}
		else if ( Type==ITEM_ARMOR+17 || Type==ITEM_ARMOR+18 || Type==ITEM_ARMOR+20 )
		{
			sy += Height*0.8f;
		}
		else if(Type == ITEM_ARMOR+15)
		{
			sy += Height * 1.0f;
		}
		else
		{
			sy += Height*0.8f;
		}
	}
	else if(Type>=ITEM_PANTS && Type<ITEM_BOOTS+MAX_ITEM_INDEX)
	{
		sx += Width*0.5f;
		sy += Height*0.9f;
	}
	else if ( Type==ITEM_HELPER+14 && (Level>>3)==1 )
	{
		sx += Width*0.55f;
		sy += Height*0.85f;
	}
	else if( Type==ITEM_HELPER+14 || Type==ITEM_HELPER+15 )
	{
		sx += Width*0.6f;
		sy += Height*1.f;
	}
	else if(Type==ITEM_HELPER+16 || Type==ITEM_HELPER+17)
	{
		sx += Width*0.5f;
		sy += Height*0.9f;
	}
	else if(Type==ITEM_HELPER+18)
	{
		sx += Width*0.5f;
		sy += Height*0.75f;
	}
	else if(Type==ITEM_HELPER+19)
	{
		switch ( Level>>3 )
		{
		case 0: sx += Width*0.5f; sy += Height*0.5f; break;
		case 1: sx += Width*0.7f; sy += Height*0.8f; break;
		case 2: sx += Width*0.7f; sy += Height*0.7f; break;
		}
	}
	else if( Type == ITEM_HELPER+20 )
	{
		switch( Level>>3 )
		{
		case 0: sx += Width*0.5f; sy+= Height*0.65f; break;
		case 1:
		case 2:
		case 3:
			sx += Width*0.5f; sy+= Height*0.8f; break;
		}
	}
	else if ( Type==ITEM_HELPER+29 )
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if ( Type==ITEM_HELPER+4 )
	{
		sx += Width*0.5f;
		sy += Height*0.6f;
	}
	else if ( Type==ITEM_HELPER+30 )
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if ( Type==ITEM_HELPER+31 )
	{
		sx += Width*0.5f;
		sy += Height*0.9f;
	}
	else if ( Type==ITEM_POTION+7 )
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if ( Type==ITEM_HELPER+7 )
	{
		sx += Width*0.5f;
		sy += Height*0.9f;
	}
	else if ( Type==ITEM_HELPER+11 )
	{
		switch ( Level>>3 )
		{
		case 0: sx += Width*0.5f; sy += Height*0.8f; break;
		case 1: sx += Width*0.5f; sy += Height*0.5f; break;
		}
	}
	else if(Type == ITEM_HELPER+32)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+33)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+34)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+35)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+36)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+37)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type>=ITEM_HELPER && Type<ITEM_HELPER+MAX_ITEM_INDEX)
	{
		sx += Width*0.5f;
		sy += Height*0.7f;
	}
	else if(Type==ITEM_POTION+12)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type==ITEM_POTION+11 && ((Level>>3) == 3 || (Level>>3) == 13))
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if ( Type==ITEM_POTION+11 && ( (Level>>3)==14 || (Level>>3)==15 ) )
	{
		sx += Width*0.5f;
		sy += Height*0.8f;
	}
	else if(Type==ITEM_POTION+9 && (Level>>3) == 1)
	{
		sx += Width*0.5f;
		sy += Height*0.8f;
	}
	else if(Type==ITEM_POTION+17 || Type==ITEM_POTION+18 || Type==ITEM_POTION+19)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type==ITEM_POTION+21)
	{
		switch ( Level>>3 )
		{
		case 0: sx += Width*0.5f; sy += Height*0.5f; break;
		case 1: sx += Width*0.4f; sy += Height*0.8f; break;
		case 2: sx += Width*0.4f; sy += Height*0.8f; break;
		case 3: sx += Width*0.5f; sy += Height*0.5f; break;
		}
	}
	else if ( Type>=ITEM_POTION+22 && Type<ITEM_POTION+25 )
	{
		if( Type==ITEM_POTION+24 && (Level>>3)==1 )
		{
			sx += Width*0.5f;
			sy += Height*0.8f;
		}
		else
		{
			sx += Width*0.5f;
			sy += Height*0.95f;
		}
	}
	else if (Type>=ITEM_POTION+46 && Type<=ITEM_POTION+48 )
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if ( Type>=ITEM_POTION+25 && Type<ITEM_POTION+27 )
	{
		sx += Width*0.5f;
		sy += Height*0.9f;
	}
	else if(Type==ITEM_POTION+31)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type==INDEX_COMPILED_CELE || Type == INDEX_COMPILED_SOUL)
	{
		sx += Width*0.55f;
		sy += Height*0.8f;
	}
	else if ( Type==ITEM_WING+3 )
	{
		sx += Width*0.5f;
		sy += Height*0.45f;
	}
	else if ( Type==ITEM_WING+4 )
	{
		sx += Width*0.5f;
		sy += Height*0.4f;
	}
	else if ( Type==ITEM_WING+5 )
	{
		sx += Width*0.5f;
		sy += Height*0.75f;
	}
	else if ( Type==ITEM_WING+6 )
	{
		sx += Width*0.5f;
		sy += Height*0.55f;
	}
	else if(Type == ITEM_POTION+100)
	{
		sx += Width*0.49f;
		//sy += Height*0.28f;
		sy += Height*0.28f;
	}
	else if (COMGEM::Check_Jewel_Com(Type) != COMGEM::NOGEM)
	{
		sx += Width*0.55f;
		sy += Height*0.82f;
	}
	else if(Type>=ITEM_POTION && Type<ITEM_POTION+MAX_ITEM_INDEX)
	{
		sx += Width*0.5f;
		sy += Height*0.95f;
	}
	else if((Type >= ITEM_WING+12 && Type <= ITEM_WING+14) || (Type >= ITEM_WING+16 && Type <= ITEM_WING+19))
	{
		sx += Width*0.5f;
		sy += Height*0.75f;
	}
	else if( Type == ITEM_HELPER+66 )
	{
		sx += Width*1.5f;
		sy += Height*1.5f;
	}
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	else if(Type==ITEM_WING+49)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type==ITEM_WING+50)
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
	else
	{
		sx += Width*0.5f;
		sy += Height*0.6f;
	}

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	if(Type>=ITEM_SWORD+32 && Type<=ITEM_SWORD+34)
	{
		sx -= Width*0.25f;
		sy -= Height*0.25f;
	}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM

	vec3_t Position;
	CreateScreenVector((int)(sx),(int)(sy),Position, false);
	//RenderObjectScreen(Type+MODEL_ITEM,Level,Option1,Position,Success,PickUp);
	if ( Type==ITEM_POTION+11 && ( Level>>3) == 1)	// 성탄의별
	{
		RenderObjectScreen(MODEL_EVENT+4,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 2)
	{
		RenderObjectScreen(MODEL_EVENT+5,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 3)
	{
		RenderObjectScreen(MODEL_EVENT+6,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 5)
	{
		RenderObjectScreen(MODEL_EVENT+8,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 6)
	{
		RenderObjectScreen(MODEL_EVENT+9,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && 8 <= ( Level>>3) && ( Level>>3) <= 12)
	{
		RenderObjectScreen(MODEL_EVENT+10,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 13)
	{
		RenderObjectScreen(MODEL_EVENT+6,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && ( (Level>>3)==14 || (Level>>3)==15 ) )
	{
		RenderObjectScreen(MODEL_EVENT+5,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_HELPER+14 && ( Level>>3) == 1)
	{
		RenderObjectScreen ( MODEL_EVENT+16, Level, Option1, ExtOption, Position, Success, PickUp );
	}
	else if ( Type==ITEM_POTION+9 && ( Level>>3) == 1)
	{
		RenderObjectScreen(MODEL_EVENT+7,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+21 )
	{
		switch ( (Level>>3) )
		{
		case 1:
			RenderObjectScreen(MODEL_EVENT+11,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 2:
			RenderObjectScreen(MODEL_EVENT+11,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 3:
			RenderObjectScreen(Type+MODEL_ITEM,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		default:
			RenderObjectScreen(Type+MODEL_ITEM,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
	else if ( Type ==ITEM_POTION+45)
	{
		RenderObjectScreen(MODEL_POTION+45,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type >=ITEM_POTION+46 && Type <=ITEM_POTION+48)
	{
		RenderObjectScreen(MODEL_POTION+46,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type ==ITEM_POTION+49)
	{
		RenderObjectScreen(MODEL_POTION+49,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type ==ITEM_POTION+50)
	{
		RenderObjectScreen(MODEL_POTION+50,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type ==ITEM_POTION+32)
	{
		switch ( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(MODEL_POTION+32,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 1:
			RenderObjectScreen(MODEL_EVENT+21,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
	else if ( Type ==ITEM_POTION+33)
	{
		switch ( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(MODEL_POTION+33,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 1:
			RenderObjectScreen(MODEL_EVENT+22,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
	else if ( Type ==ITEM_POTION+34)
	{
		switch ( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(MODEL_POTION+34,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 1:
			RenderObjectScreen(MODEL_EVENT+23,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
	else if ( Type==ITEM_HELPER+19)
	{
		switch ( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(MODEL_STAFF+10,-1,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 1:
			RenderObjectScreen(MODEL_SWORD+19,-1,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 2:
			RenderObjectScreen(MODEL_BOW+18,-1,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
	else if ( Type==ITEM_POTION+23)
	{
		switch( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(Type+MODEL_ITEM,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 1:
			RenderObjectScreen(MODEL_EVENT+12,-1,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
	else if ( Type==ITEM_POTION+24)
	{
		switch( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(Type+MODEL_ITEM,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 1:
			RenderObjectScreen(MODEL_EVENT+13,-1,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
	else if ( Type==ITEM_HELPER+20)
	{
		switch( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(MODEL_EVENT+15,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 1:
		case 2:
		case 3:
			RenderObjectScreen(MODEL_EVENT+14,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
	else if ( Type==ITEM_HELPER+11 && (Level>>3)==1 )
	{
		RenderObjectScreen ( MODEL_EVENT+18,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if(Type == ITEM_POTION+100)
	{
		bool _Angle;
		if(g_pLuckyCoinRegistration->GetItemRotation())
		{
			_Angle = true;
		}
		else
		{
			_Angle = Success;
		}

		RenderObjectScreen(MODEL_POTION+100,Level,Option1,ExtOption,Position,_Angle,PickUp);
	}
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	else if(Type == ITEM_ARMOR+59)
	{
		RenderObjectScreen(MODEL_ARMORINVEN_60,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if(Type == ITEM_ARMOR+60)
	{
		RenderObjectScreen(MODEL_ARMORINVEN_61,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if(Type == ITEM_ARMOR+61)
	{
		RenderObjectScreen(MODEL_ARMORINVEN_62,Level,Option1,ExtOption,Position,Success,PickUp);
	}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
	else
	{
		RenderObjectScreen(Type+MODEL_ITEM,Level,Option1,ExtOption,Position,Success,PickUp);
	}
}

void InventoryColor(ITEM *p)
{
	switch(p->Color)
	{
	case 0:
		glColor3f(1.f,1.f,1.f);
		break;
	case 1:
		glColor3f(0.8f,0.8f,0.8f);
		break;
	case 2:
		glColor3f(0.6f,0.7f,1.f);
		break;
	case 3:
		glColor3f(1.f,0.2f,0.1f);
		break;
	case 4:
		glColor3f(0.5f,1.f,0.6f);
		break;
    case 5:
		glColor4f(0.8f,0.7f,0.f,1.f);
        break;
    case 6:
		glColor4f(0.8f,0.5f,0.f,1.f);
        break;
    case 7:
		glColor4f(0.8f,0.3f,0.3f,1.f);
        break;
    case 8:
		glColor4f(1.0f,0.f,0.f,1.f);
        break;
    case 99:
		glColor3f(1.f,0.2f,0.1f);
        break;
	}
}

void RenderEqiupmentBox()
{
	int StartX = InventoryStartX;
	int StartY = InventoryStartY;
	float x,y,Width,Height;

	EnableAlphaTest();

	//helper
	Width=40.f;Height=40.f;x=15.f;y=46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_HELPER]);
    RenderBitmap(BITMAP_INVENTORY+15,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
	//wing
	Width=60.f;Height=40.f;x=115.f;y=46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WING]);
    RenderBitmap(BITMAP_INVENTORY+14,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
	if(gCharacterManager.GetBaseClass(CharacterAttribute->Class)!=CLASS_DARK)
	{
		//helmet
		Width=40.f;Height=40.f;x=75.f;y=46.f;
		InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_HELM]);
		RenderBitmap(BITMAP_INVENTORY+3,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
	}
    //armor upper
	Width=40.f;Height=60.f;x=75.f;y=89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_ARMOR]);
    RenderBitmap(BITMAP_INVENTORY+4,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
    //armor lower
	//if(GetBaseClass(CharacterAttribute->Class) != CLASS_ELF)
	{
		Width=40.f;Height=40.f;x=75.f;y=152.f;
		InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_PANTS]);
		RenderBitmap(BITMAP_INVENTORY+5,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
	}
    //weapon right
	Width=40.f;Height=60.f;x=15.f;y=89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT]);
    RenderBitmap(BITMAP_INVENTORY+6,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
    //weapon left
	Width=40.f;Height=60.f;x=134.f;y=89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT]);
    RenderBitmap(BITMAP_INVENTORY+16,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
    //glove
#ifdef PBG_ADD_NEWCHAR_MONK
	if(GetBaseClass(CharacterAttribute->Class)!=CLASS_RAGEFIGHTER)
	{
#endif //PBG_ADD_NEWCHAR_MONK
	Width=40.f;Height=40.f;x=15.f;y=152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_GLOVES]);
    RenderBitmap(BITMAP_INVENTORY+7,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
#ifdef PBG_ADD_NEWCHAR_MONK
	}
#endif //PBG_ADD_NEWCHAR_MONK
    //boot
	Width=40.f;Height=40.f;x=134.f;y=152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_BOOTS]);
    RenderBitmap(BITMAP_INVENTORY+8,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/64.f,Height/64.f);
    //necklace
	Width=20.f;Height=20.f;x=55.f;y=89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_AMULET]);
    RenderBitmap(BITMAP_INVENTORY+9,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/32.f,Height/32.f);
    //ring
	Width=20.f;Height=20.f;x=55.f;y=152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT]);
    RenderBitmap(BITMAP_INVENTORY+10,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/32.f,Height/32.f);
    //ring
	Width=20.f;Height=20.f;x=115.f;y=152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_RING_LEFT]);
    RenderBitmap(BITMAP_INVENTORY+10,x+StartX,y+StartY,Width,Height,0.f,0.f,Width/32.f,Height/32.f);
}

void RenderEqiupmentPart3D(int Index,float sx,float sy,float Width,float Height)
{
	ITEM *p = &CharacterMachine->Equipment[Index];
	if(p->Type != -1)
	{
		if(p->Number > 0)
            RenderItem3D(sx,sy,Width,Height,p->Type,p->Level,p->Option1,p->ExtOption,false);
	}
}

void RenderEqiupment3D()
{
	int StartX = InventoryStartX;
	int StartY = InventoryStartY;
	float x,y,Width,Height;

	//helper
	Width=40.f;Height=40.f;x=15.f;y=46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_HELPER]);
	RenderEqiupmentPart3D(EQUIPMENT_HELPER,StartX+x,StartY+y,Width,Height);
	//wing
	Width=60.f;Height=40.f;x=115.f;y=46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WING]);
	RenderEqiupmentPart3D(EQUIPMENT_WING,StartX+x,StartY+y,Width,Height);
    //helmet
	Width=40.f;Height=40.f;x=75.f;y=46.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_HELM]);
	RenderEqiupmentPart3D(EQUIPMENT_HELM,StartX+x,StartY+y,Width,Height);
    //armor upper
	Width=40.f;Height=60.f;x=75.f;y=89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_ARMOR]);
	RenderEqiupmentPart3D(EQUIPMENT_ARMOR,StartX+x,StartY+y-10,Width,Height);
    //armor lower
	//if(GetBaseClass(CharacterAttribute->Class) != CLASS_ELF)
	{
		Width=40.f;Height=40.f;x=75.f;y=152.f;
		InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_PANTS]);
		RenderEqiupmentPart3D(EQUIPMENT_PANTS,StartX+x,StartY+y,Width,Height);
	}
    //weapon right
	Width=40.f;Height=60.f;x=15.f;y=89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT]);
	RenderEqiupmentPart3D(EQUIPMENT_WEAPON_RIGHT,StartX+x,StartY+y,Width,Height);
    //weapon left
	Width=40.f;Height=60.f;x=134.f;y=89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT]);
	RenderEqiupmentPart3D(EQUIPMENT_WEAPON_LEFT,StartX+x,StartY+y,Width,Height);
    //glove
	Width=40.f;Height=40.f;x=15.f;y=152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_GLOVES]);
	RenderEqiupmentPart3D(EQUIPMENT_GLOVES,StartX+x,StartY+y,Width,Height);
    //boot
	Width=40.f;Height=40.f;x=134.f;y=152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_BOOTS]);
	RenderEqiupmentPart3D(EQUIPMENT_BOOTS,StartX+x,StartY+y,Width,Height);
    //necklace
	Width=20.f;Height=20.f;x=55.f;y=89.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_AMULET]);
	RenderEqiupmentPart3D(EQUIPMENT_AMULET,StartX+x,StartY+y,Width,Height);
    //ring
	Width=20.f;Height=20.f;x=55.f;y=152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT]);
	RenderEqiupmentPart3D(EQUIPMENT_RING_RIGHT,StartX+x,StartY+y,Width,Height);
    //ring
	Width=20.f;Height=20.f;x=115.f;y=152.f;
    InventoryColor(&CharacterMachine->Equipment[EQUIPMENT_RING_LEFT]);
	RenderEqiupmentPart3D(EQUIPMENT_RING_LEFT,StartX+x,StartY+y,Width,Height);

}

bool CheckEmptyInventory(ITEM *Inv,int InvWidth,int InvHeight)
{
	bool Empty = true;
	for(int y=0;y<InvHeight;y++)
	{
		for(int x=0;x<InvWidth;x++)
		{
			int Index  = y*InvWidth+x;
			ITEM *p = &Inv[Index];
			if(p->Type!=-1 && p->Number>0)
			{
				Empty = false;
			}
		}
	}
	return Empty;
}

void InitPartyList ()
{
    PartyNumber = 0;
    PartyKey = 0;
}

void MoveServerDivisionInventory ()
{
    if ( !g_pUIManager->IsOpen(INTERFACE_SERVERDIVISION) ) return;
    int x = 640-190;
    int y = 0;
    int Width, Height;

	if( MouseX>=(int)(x) && MouseX<(int)(x+190) && 
		MouseY>=(int)(y) && MouseY<(int)(y+256+177) )
    {
		MouseOnWindow = true;
    }

    Width = 16; Height = 16; x = InventoryStartX+25;y = 240;
	if(MouseX>=x && MouseX<x+Width && MouseY>=y && MouseY<y+Height && MouseLButtonPush )
    {
        g_bServerDivisionAccept ^= true;

        MouseLButtonPush = false;
        MouseLButton = false;
    }

    if ( g_bServerDivisionAccept )
    {
        Width = 120; Height = 24; x = (float)InventoryStartX+35;y = 320;
	    if(MouseX>=x && MouseX<x+Width && MouseY>=y && MouseY<y+Height && MouseLButtonPush )
        {
            MouseLButtonPush = false;
            MouseLButton = false;
            AskYesOrNo =  4;
            OkYesOrNo  = -1;

            ShowCheckBox(1, 448, MESSAGE_CHECK );
        }
    }

    Width = 120; Height = 24; x = (float)InventoryStartX+35;y = 350;
	if(MouseX>=x && MouseX<x+Width && MouseY>=y && MouseY<y+Height && MouseLButtonPush )
    {
        MouseLButtonPush = false;
        MouseLButton = false;
        MouseUpdateTime = 0;
		MouseUpdateTimeMax = 6;

        SendExitInventory();
		g_pUIManager->CloseAll();
    }
    
    Width=24;Height=24;x=InventoryStartX+25;y=InventoryStartY+395;
	if(MouseX>=x && MouseX<x+Width && MouseY>=y && MouseY<y+Height)
	{
		if(MouseLButtonPush)
		{
			MouseLButtonPush = false;
			MouseUpdateTime = 0;
			MouseUpdateTimeMax = 6;

            g_bEventChipDialogEnable = EVENT_NONE;

			SendExitInventory();
			g_pUIManager->CloseAll();
		}
	}
}

void HideKeyPad( void)
{
	g_iKeyPadEnable = 0;
}

int CheckMouseOnKeyPad( void)
{
	int Width, Height, WindowX, WindowY;
    Width = 213;Height = 2*5+6*40;WindowX = (640-Width)/2;WindowY = 60+40;//60 220

	int iButtonTop = 50;

	for ( int i = 0; i < 11; ++i)
	{
		int xButton = i % 5;
		int yButton = i / 5;

		int xLeft = WindowX + 10 + xButton * 40;
		int yTop = WindowY + iButtonTop + yButton * 40;
		if ( xLeft <= MouseX && MouseX < xLeft + 32 &&
			yTop <= MouseY && MouseY < yTop + 32)
		{
			return ( i);
		}
	}
	// Ok, Cancel ( 11 - 12)
	int yTop = WindowY + iButtonTop + 2 * 40 + 5;

	for ( int i = 0; i < 2; ++i)
	{
		int xLeft = WindowX + 52 + i * 78;
		if ( xLeft <= MouseX && MouseX < xLeft + 70 &&
			yTop <= MouseY && MouseY < yTop + 21)
		{
			return ( 11 + i);
		}
	}

	return ( -1);
}

bool g_bPadPushed = false;

void MovePersonalShop()
{
	if((g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPSALE) || g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPPURCHASE)) && g_iPShopWndType == PSHOPWNDTYPE_SALE)
	{
		if(g_iPersonalShopMsgType == 1)
		{
			if(OkYesOrNo == 1) 
			{
				g_iPersonalShopMsgType = 0;
				OkYesOrNo = -1;
			}
			else if(OkYesOrNo == 2) 
			{
				g_iPersonalShopMsgType = 0;
				OkYesOrNo = -1;
			}
		}
		g_ptPersonalShop.x = 640-190*2;
		g_ptPersonalShop.y = 0;

		int Width = 56, Height = 24;
		int ButtonX = g_ptPersonalShop.x + 30, ButtonY = g_ptPersonalShop.y + 396;
		if(MouseX>=ButtonX && MouseX<ButtonX+Width && MouseY>=ButtonY && MouseY<ButtonY+Height && MouseLButtonPush)
		{
			MouseLButtonPush = false;
			if(!IsExistUndecidedPrice() && strlen(g_szPersonalShopTitle) > 0) 
			{
				if(g_bEnablePersonalShop) 
				{
					SendRequestCreatePersonalShop(g_szPersonalShopTitle);
					g_pUIManager->Close( INTERFACE_INVENTORY );
				}
				else 
				{
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalshopCreateMsgBoxLayout));
				}
			}
			else 
			{
				g_pChatListBox->AddText("", GlobalText[1119], SEASON3B::TYPE_ERROR_MESSAGE);
			}
		}
		
		ButtonX = g_ptPersonalShop.x + 105;
		if(MouseX>=ButtonX && MouseX<ButtonX+Width && MouseY>=ButtonY && MouseY<ButtonY+Height && MouseLButtonPush)
		{
			MouseLButtonPush = false;
			if(g_bEnablePersonalShop) 
			{
				SendRequestDestoryPersonalShop();
			}
		}

		Width = 150;
		ButtonX = g_ptPersonalShop.x + 20;
		ButtonY = g_ptPersonalShop.y + 65;
		if(MouseX>=ButtonX && MouseX<ButtonX+Width && MouseY>=ButtonY && MouseY<ButtonY+Height && MouseLButtonPush)
		{
			OpenPersonalShopMsgWnd(1);
		}
	}
}

void ClosePersonalShop()
{
	if(g_iPShopWndType == PSHOPWNDTYPE_PURCHASE) 
	{	
		memcpy(g_PersonalShopInven, g_PersonalShopBackup, sizeof(ITEM)*MAX_PERSONALSHOP_INVEN);
		if(IsShopInViewport(Hero)) 
		{
			std::string title;
			GetShopTitle(Hero, title);
			strcpy(g_szPersonalShopTitle, title.c_str());
		}
		else 
		{
			g_szPersonalShopTitle[0] = '\0';
		}
		if(g_PersonalShopSeller.Key)
		{
			SendRequestClosePersonalShop(g_PersonalShopSeller.Key, g_PersonalShopSeller.ID);
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

	for(int i=0; i<MAX_PERSONALSHOP_INVEN; ++i) 
	{
		int iPrice = 0;
		int iIndex;
		ITEM* pItem = g_pMyShopInventory->FindItem(i);
		if(pItem)
		{
			bResult = false;

			iIndex = MAX_MY_INVENTORY_INDEX + (pItem->y*COL_PERSONALSHOP_INVEN)+pItem->x;
			if(GetPersonalItemPrice(iIndex, iPrice, g_IsPurchaseShop) == false)
			{
				return true;
			}	

			if(iPrice <= 0)
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
	if(iMsgType == 1) 
	{
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalShopNameMsgBoxLayout));
	}
	else if(iMsgType == 2) 
	{
		SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalShopItemValueMsgBoxLayout));
	}
}
bool IsCorrectShopTitle(const char* szShopTitle)
{
	int j=0;
	char TmpText[2048];
	for( int i=0; i<(int)strlen(szShopTitle); ++i )
	{
		if ( szShopTitle[i]!=32 )
		{
			TmpText[j] = szShopTitle[i];
			j++;
		}
	}
	TmpText[j] = 0;

	for( int i=0;i<AbuseFilterNumber;i++ )
	{
		if(FindText(TmpText,AbuseFilter[i]))
		{
			return false;
		}
	}
	
	int len = strlen(szShopTitle);
	int count = 0;

	for(int i=0; i<len; i++)
	{
		if(szShopTitle[i] == 0x20) { 
			count++; 
			if(i==1 && count>=2) return false;
		}
		else { 
			count = 0;
		}
	}
	if(count >= 2)
		return false;
	return true;
}

extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;

void RenderInventoryInterface(int StartX,int StartY,int Flag)
{
	float x,y,Width,Height; 
	Width=190.f;Height=256.f;x=(float)StartX;y=(float)StartY;

    RenderBitmap(BITMAP_INVENTORY  ,x,y,Width,Height,0.f,0.f,Width/256.f,Height/256.f);

	Width=190.f;Height=177.f;x=(float)StartX;y=(float)StartY+256;
	RenderBitmap(BITMAP_INVENTORY+1,x,y,Width,Height,0.f,0.f,Width/256.f,Height/256.f);

	if(Flag)
	{
		Width=190.f;Height=10.f;x=(float)StartX;y=(float)StartY+225;
		RenderBitmap(BITMAP_INVENTORY+19,x,y,Width,Height,0.f,0.f,Width/256.f,Height/16.f);
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

void CreateGuildMark( int nMarkIndex, bool blend )
{
	BITMAP_t *b = &Bitmaps[BITMAP_GUILD];
	int Width,Height;
	Width  = (int)b->Width;
	Height = (int)b->Height;
	BYTE *Buffer = b->Buffer;
    int alpha = 128;
    if( blend )
    {
        alpha = 0;
    }

	for(int i=0;i<16;i++)
	{
		switch(i)
		{
		case 0 :MarkColor[i] = (alpha<<24)+(0<<16)+(  0<<8)+(  0);break;
		case 1 :MarkColor[i] = (255<<24)+(  0<<16)+(  0<<8)+(  0);break;
		case 2 :MarkColor[i] = (255<<24)+(128<<16)+(128<<8)+(128);break;
		case 3 :MarkColor[i] = (255<<24)+(255<<16)+(255<<8)+(255);break;
		case 4 :MarkColor[i] = (255<<24)+(  0<<16)+(  0<<8)+(255);break;
		case 5 :MarkColor[i] = (255<<24)+(  0<<16)+(128<<8)+(255);break;
		case 6 :MarkColor[i] = (255<<24)+(  0<<16)+(255<<8)+(255);break;
		case 7 :MarkColor[i] = (255<<24)+(  0<<16)+(255<<8)+(128);break;
		case 8 :MarkColor[i] = (255<<24)+(  0<<16)+(255<<8)+(  0);break;
		case 9 :MarkColor[i] = (255<<24)+(128<<16)+(255<<8)+(  0);break;
		case 10:MarkColor[i] = (255<<24)+(255<<16)+(255<<8)+(  0);break;
		case 11:MarkColor[i] = (255<<24)+(255<<16)+(128<<8)+(  0);break;
		case 12:MarkColor[i] = (255<<24)+(255<<16)+(  0<<8)+(  0);break;
		case 13:MarkColor[i] = (255<<24)+(255<<16)+(  0<<8)+(128);break;
		case 14:MarkColor[i] = (255<<24)+(255<<16)+(  0<<8)+(255);break;
		case 15:MarkColor[i] = (255<<24)+(128<<16)+(  0<<8)+(255);break;
		}
	}
	BYTE *MarkBuffer = GuildMark[nMarkIndex].Mark;

	for(int i=0;i<Height;i++)
	{
		for(int j=0;j<Width;j++)
		{
			*((unsigned int *)(Buffer)) = MarkColor[MarkBuffer[0]];
			Buffer += 4;
			MarkBuffer++;
		}
	}

	glBindTexture(GL_TEXTURE_2D,b->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D,0,b->Components,Width,Height,0,GL_RGBA,GL_UNSIGNED_BYTE,b->Buffer);
}


void CreateCastleMark ( int Type, BYTE* buffer, bool blend )
{
    if ( buffer==NULL ) return;

	BITMAP_t *b = &Bitmaps[Type];

	int Width,Height;

	Width  = (int)b->Width;
	Height = (int)b->Height;
	BYTE* Buffer = b->Buffer;

    int alpha = 128;
    if( blend )
    {
        alpha = 0;
    }

	for(int i=0;i<16;i++)
	{
		switch(i)
		{
		case 0 :MarkColor[i] = (alpha<<24)+(0<<16)+(  0<<8)+(  0);break;
		case 1 :MarkColor[i] = (255<<24)+(  0<<16)+(  0<<8)+(  0);break;
		case 2 :MarkColor[i] = (255<<24)+(128<<16)+(128<<8)+(128);break;
		case 3 :MarkColor[i] = (255<<24)+(255<<16)+(255<<8)+(255);break;
		case 4 :MarkColor[i] = (255<<24)+(  0<<16)+(  0<<8)+(255);break;//빨
		case 5 :MarkColor[i] = (255<<24)+(  0<<16)+(128<<8)+(255);break;//
		case 6 :MarkColor[i] = (255<<24)+(  0<<16)+(255<<8)+(255);break;//노
		case 7 :MarkColor[i] = (255<<24)+(  0<<16)+(255<<8)+(128);break;//
		case 8 :MarkColor[i] = (255<<24)+(  0<<16)+(255<<8)+(  0);break;//초
		case 9 :MarkColor[i] = (255<<24)+(128<<16)+(255<<8)+(  0);break;//
		case 10:MarkColor[i] = (255<<24)+(255<<16)+(255<<8)+(  0);break;//청
		case 11:MarkColor[i] = (255<<24)+(255<<16)+(128<<8)+(  0);break;//
		case 12:MarkColor[i] = (255<<24)+(255<<16)+(  0<<8)+(  0);break;//파
		case 13:MarkColor[i] = (255<<24)+(255<<16)+(  0<<8)+(128);break;//
		case 14:MarkColor[i] = (255<<24)+(255<<16)+(  0<<8)+(255);break;//보
		case 15:MarkColor[i] = (255<<24)+(128<<16)+(  0<<8)+(255);break;//
		}
	}
	BYTE MarkBuffer[32*32];

    int offset = 0;

	for ( int i=0; i<32; ++i )
	{
		for ( int j=0; j<32; ++j )
		{
			offset = (j/4)+((i/4)*8);
			MarkBuffer[j+(i*32)] = buffer[offset];
		}
	}

    offset = 0;
    int offset2 = 0;

	for ( int i=0; i<Height; ++i )
	{
		for ( int j=0; j<Width; ++j )
		{
            if ( j>=(Width/2-16) && j<(Width/2+16) && i>=(Height/2-16) && i<(Height/2+16) )
            {
			    *((unsigned int *)(Buffer+offset)) = MarkColor[MarkBuffer[offset2]];
                offset2++;
            }
            else if ( j<3 || j>(Width-4) || i<10 || i>(Height-10) )
            {
			    *((unsigned int *)(Buffer+offset)) = (255<<24)+(  0<<16)+((int)(50+i/100.f*160)<<8)+(50+i/100.f*255);
            }
            else
            {
			    *((unsigned int *)(Buffer+offset)) = (255<<24)+(  i<<16)+(i<<8)+(i);
            }
            offset += 4;
		}
	}
    glBindTexture(GL_TEXTURE_2D,b->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D,0,3,Width,Height,0,GL_RGBA,GL_UNSIGNED_BYTE,b->Buffer);
}


void RenderGuildColor(float x,float y,int SizeX,int SizeY,int Index)
{
	RenderBitmap(BITMAP_INVENTORY+18,x-1,y-1,(float)SizeX+2,(float)SizeY+2,0.f,0.f,SizeX/32.f,SizeY/30.f);

	BITMAP_t *b = &Bitmaps[BITMAP_GUILD];

	int Width,Height;

	Width  = (int)b->Width;
	Height = (int)b->Height;
	BYTE *Buffer = b->Buffer;
	unsigned int Color = MarkColor[Index];

	if(Index==0)
	{
		for(int i=0;i<Height;i++)
		{
			for(int j=0;j<Width;j++)
			{
				*((unsigned int *)(Buffer)) = 255<<24;
				Buffer += 4;
			}
		}
		Color = (255<<24)+(128<<16)+(128<<8)+(128);
		Buffer = b->Buffer;
		for(int i=0;i<8;i++)
		{
			*((unsigned int *)(Buffer)) = Color;
			Buffer += 8*4+4;
		}
		Buffer = b->Buffer+7*4;
		for(int i=0;i<8;i++)
		{
			*((unsigned int *)(Buffer)) = Color;
			Buffer += 8*4-4;
		}
	}
	else
	{
		for(int i=0;i<Height;i++)
		{
			for(int j=0;j<Width;j++)
			{
				*((unsigned int *)(Buffer)) = Color;
				Buffer += 4;
			}
		}
	}

	glBindTexture(GL_TEXTURE_2D,b->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D,0,b->Components,Width,Height,0,GL_RGBA,GL_UNSIGNED_BYTE,b->Buffer);
    RenderBitmap(BITMAP_GUILD,x,y,(float)SizeX,(float)SizeY);
}

void RenderGuildList(int StartX,int StartY)
{
	GuildListStartX = StartX;
	GuildListStartY = StartY;

	glColor3f(1.f,1.f,1.f);

	DisableAlphaBlend();
	float x,y,Width,Height;
	Width=190.f;Height=256.f;x=(float)StartX;y=(float)StartY;
    RenderBitmap(BITMAP_INVENTORY  ,x,y,Width,Height,0.f,0.f,Width/256.f,Height/256.f);
	Width=190.f;Height=177.f;x=(float)StartX;y=(float)StartY+256;
    RenderBitmap(BITMAP_INVENTORY+1,x,y,Width,Height,0.f,0.f,Width/256.f,Height/256.f);

	EnableAlphaTest();

	g_pRenderText->SetBgColor(20, 20, 20, 255);
	g_pRenderText->SetTextColor(220, 220, 220, 255);
	g_pRenderText->SetFont(g_hFontBold);

	char Text[100];
	if(Hero->GuildMarkIndex == -1)
		sprintf(Text,GlobalText[180]);
	else
		sprintf(Text,"%s (Score:%d)",GuildMark[Hero->GuildMarkIndex].GuildName,GuildTotalScore);

	g_pRenderText->RenderText(StartX+95-60,StartY+12,Text,120*WindowWidth/640,true,3);

	g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(230, 230, 230, 255);
	g_pRenderText->SetFont(g_hFont);

	if(g_nGuildMemberCount == 0)
	{
		g_pRenderText->RenderText(StartX+20,StartY+50 ,GlobalText[185]);
		g_pRenderText->RenderText(StartX+20,StartY+65 ,GlobalText[186]);
		g_pRenderText->RenderText(StartX+20,StartY+80 ,GlobalText[187]);
	}
	g_pRenderText->SetBgColor(0, 0, 0, 128);
	g_pRenderText->SetTextColor(100, 255, 200, 255);
	g_pRenderText->RenderText(StartX+( int)Width/2,StartY+44,g_GuildNotice[0], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(StartX+( int)Width/2,StartY+58,g_GuildNotice[1], 0 ,0, RT3_WRITE_CENTER);

	int yGuildStart = 72;
	int Number = g_nGuildMemberCount;

	if(g_nGuildMemberCount >= MAX_GUILD_LINE)
      	Number = MAX_GUILD_LINE;
}

//#define MAX_LENGTH_CMB	( 26)
#define NUM_LINE_CMB	( 7)

void RenderServerDivision ()
{
    if ( !g_pUIManager->IsOpen(INTERFACE_SERVERDIVISION) ) return;

   	float Width, Height, x, y; 

	glColor3f ( 1.f, 1.f, 1.f );
    EnableAlphaTest ();

    InventoryStartX = 640-190;
    InventoryStartY = 0;
    Width = 213;Height = 40;x = (float)InventoryStartX;y = (float)InventoryStartY;
	RenderInventoryInterface ( (int)x, (int)y, 1 );

	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 230, 210, 255);

	g_pRenderText->SetFont(g_hFontBold);
    x = InventoryStartX+(190/2.f);
    y = 50;
    for ( int i=462; i<470; ++i )
    {
		g_pRenderText->RenderText(x, y, GlobalText[i], 0 ,0, RT3_WRITE_CENTER);
        y += 20;
    }

	g_pRenderText->SetFont(g_hFontBold);
    Width = 16; Height = 16; x = (float)InventoryStartX+25;y = 240;
    if ( g_bServerDivisionAccept )
    {
		g_pRenderText->SetTextColor(212, 150, 0, 255);
        RenderBitmap(BITMAP_INVENTORY_BUTTON+11,x,y,Width,Height,0.f,0.f,24/32.f,24/32.f);
    }
    else
    {
		g_pRenderText->SetTextColor(223, 191, 103, 255);
        RenderBitmap(BITMAP_INVENTORY_BUTTON+10,x,y,Width,Height,0.f,0.f,24/32.f,24/32.f);
    }
	g_pRenderText->RenderText((int)(x+Width+3),(int)(y+5),GlobalText[447] );
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 230, 210, 255);

    Width = 120; Height = 24; x = (float)InventoryStartX+35;y = 350;//(Width/2.f); y = 231;
    RenderBitmap(BITMAP_INTERFACE+10,(float)x,(float)y,(float)Width,(float)Height,0.f,0.f,213.f/256.f);
	g_pRenderText->RenderText((int)(x+(Width/2)),(int)(y+5),GlobalText[229], 0 ,0, RT3_WRITE_CENTER);

    Width = 120; Height = 24; x = (float)InventoryStartX+35;y = 320;//(Width/2.f); y = 231;
    if ( g_bServerDivisionAccept )
        glColor3f ( 1.f, 1.f, 1.f );
    else 
        glColor3f ( 0.5f, 0.5f, 0.5f );
    RenderBitmap(BITMAP_INTERFACE+10,(float)x,(float)y,(float)Width,(float)Height,0.f,0.f,213.f/256.f);
	g_pRenderText->RenderText((int)(x+(Width/2)),(int)(y+5),GlobalText[228], 0 ,0, RT3_WRITE_CENTER);

    glColor3f( 1.f, 1.f, 1.f );
}

BYTE CaculateFreeTicketLevel(int iType)
{
	int iChaLevel = CharacterAttribute->Level;
	int iChaClass = gCharacterManager.GetBaseClass(Hero->Class);
	int iChaExClass = gCharacterManager.IsSecondClass(Hero->Class);

	int iItemLevel = 0;

	if(iType == FREETICKET_TYPE_DEVILSQUARE)
	{
		if(iChaClass == CLASS_DARK)
		{
			if(iChaLevel >= 15 && iChaLevel <= 110)
			{
				iItemLevel = 0;
			}
			else if(iChaLevel >= 111 && iChaLevel <= 160)
			{
				iItemLevel = 8;
			}
			else if(iChaLevel >= 161 && iChaLevel <= 210)
			{
				iItemLevel = 16;
			}
			else if(iChaLevel >= 211 && iChaLevel <= 260)
			{
				iItemLevel = 24;
			}
			else if(iChaLevel >= 261 && iChaLevel <= 310)
			{
				iItemLevel = 32;
			}
			else if(iChaLevel >= 311 && iChaLevel <= 400)
			{
				iItemLevel = 40;
			}
		}
		else
		{
			if(iChaLevel >= 15 && iChaLevel <= 130)
			{
				iItemLevel = 0;
			}
			else if(iChaLevel >= 131 && iChaLevel <= 180)
			{
				iItemLevel = 8;
			}
			else if(iChaLevel >= 181 && iChaLevel <= 230)
			{
				iItemLevel = 16;
			}
			else if(iChaLevel >= 231 && iChaLevel <= 280)
			{
				iItemLevel = 24;
			}
			else if(iChaLevel >= 281 && iChaLevel <= 330)
			{
				iItemLevel = 32;
			}
			else if(iChaLevel >= 331 && iChaLevel <= 400)
			{
				iItemLevel = 40;
			}
		}
		return (iItemLevel>>3)&15;
	}
	else if(iType == FREETICKET_TYPE_BLOODCASTLE)
	{
		if(iChaClass == CLASS_DARK)
		{
			if(iChaLevel >= 15 && iChaLevel <= 60)
			{
				iItemLevel = 0;
			}
			else if(iChaLevel >= 61 && iChaLevel <= 110)
			{
				iItemLevel = 8;
			}
			else if(iChaLevel >= 111 && iChaLevel <= 160)
			{
				iItemLevel = 16;
			}
			else if(iChaLevel >= 161 && iChaLevel <= 210)
			{
				iItemLevel = 24;
			}
			else if(iChaLevel >= 211 && iChaLevel <= 260)
			{
				iItemLevel = 32;
			}
			else if(iChaLevel >= 261 && iChaLevel <= 310)
			{
				iItemLevel = 40;
			}
			else if(iChaLevel >= 311 && iChaLevel <= 400)
			{
				iItemLevel = 48;
			}
		}
		else
		{
			if(iChaLevel >= 15 && iChaLevel <= 80)
			{
				iItemLevel = 0;
			}
			else if(iChaLevel >= 81 && iChaLevel <= 130)
			{
				iItemLevel = 8;
			}
			else if(iChaLevel >= 131 && iChaLevel <= 180)
			{
				iItemLevel = 16;
			}
			else if(iChaLevel >= 181 && iChaLevel <= 230)
			{
				iItemLevel = 24;
			}
			else if(iChaLevel >= 231 && iChaLevel <= 280)
			{
				iItemLevel = 32;
			}
			else if(iChaLevel >= 281 && iChaLevel <= 330)
			{
				iItemLevel = 40;
			}
			else if(iChaLevel >= 331 && iChaLevel <= 400)
			{
				iItemLevel = 48;
			}
		}
		return (iItemLevel>>3)&15;
	}
	else if( iType == FREETICKET_TYPE_CURSEDTEMPLE ) {
		if( g_pCursedTempleEnterWindow->CheckEnterLevel( iItemLevel ) )
		{
			return (iItemLevel>>3)&15;
		}
	}
	else if( iType == FREETICKED_TYPE_CHAOSCASTLE ) 
	{
		if( g_pCursedTempleEnterWindow->CheckEnterLevel( iItemLevel ) )
		{
			return (iItemLevel>>3)&15;
		}
	}
	return 0;
}