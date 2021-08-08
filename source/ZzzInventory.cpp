///////////////////////////////////////////////////////////////////////////////
// 창 관련 함수
// 장비창, 거래창, 창고창, 파티창, 길드창 조작 및 랜더링 등등
//
// *** 함수 레벨: 3
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
#include "PvPSystem.h"
#include "GMCrywolf1st.h"
#include "CDirection.h"
#include "ChangeRingManager.h"
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
#include "PCRoomPoint.h"
#endif	// ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
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


///////////////////////////////////////////
extern CUITextInputBox * g_pSingleTextInputBox;
extern int g_iChatInputType;
extern CUIGuildListBox * g_pGuildListBox;
////////////////////////////////////////////


int			g_nTaxRate			= 0;

int			g_nChaosTaxRate		= 0;

char         g_GuildNotice[3][128];
GUILD_LIST_t GuildList[MAX_GUILDS];
int			 g_nGuildMemberCount = 0;
int			 GuildTotalScore = 0;
int			 GuildPlayerKey = 0;
int			 SelectMarkColor = 0;
MARK_t		 GuildMark[MAX_MARKS];

// 파티 관련
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

int  AllRepairGold              = 0;     //  전체 수리 비용.
int  StorageGoldFlag            = 0;

int ListCount                = 0;
int GuildListPage            = 0;

#ifndef YDG_ADD_NEW_DUEL_SYSTEM		// 정리할때 삭제해야 함
#ifdef DUEL_SYSTEM
bool g_bEnableDuel = false;
bool g_PetEnableDuel = true;	// LHJ - 결투중에 다크로드의 다크스피릿이 공격중 인지 판단하는 변수

int g_iDuelPlayerIndex = 0;
char g_szDuelPlayerID[24] = { 0, };
extern int g_iMyPlayerScore = 0;
extern int g_iDuelPlayerScore = 0;
#endif // DUEL_SYSTEM
#endif	// YDG_ADD_NEW_DUEL_SYSTEM	// 정리할때 삭제해야 함

int  g_bEventChipDialogEnable  = EVENT_NONE;       //  레나.     ( 1:레나, 2:스톤, 3:상용화 2주년 복권 이벤트. ).
int  g_shEventChipCount        = 0;       //  레나개수.
short g_shMutoNumber[3]        = { -1, -1, -1 };//  뮤토 번호.

bool g_bServerDivisionAccept   = false; //  서버분할 동의?
#ifdef SCRATCH_TICKET
char g_strGiftName[64];					//	상용화 2주년 경품 이름.
#endif

//  내구력 관련.
bool RepairShop                = false; //  수리를 해주는 상점.
int  RepairEnable              = 0;     //  수리.
int AskYesOrNo                  = 0;    //  확인창(1-팔기, 2-카오스 조합, 3-거래확인, 4-서버분할, 5-구매확인, 6-가격확인)
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
char OkYesOrNo                  = -1;   //  확인창용 확인
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
BYTE OkYesOrNo                  = -1;   //  확인창용 확인
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

// 창고 잠금 관련
int g_iKeyPadEnable = 0;
WORD g_wStoragePassword = 0;
short	g_nKeyPadMapping[10];
char	g_lpszKeyPadInput[2][MAX_KEYPADINPUT + 1];

// 열매사용 관련 변수
BYTE g_byItemUseType = 0;	//: 열매 사용 용도 0x00 스탯생성 0x01 스탯감소

static  const int DEFAULT_DEVILSQUARELEVEL[6][2] = { {15, 130}, { 131, 180}, { 181, 230}, {231, 280}, { 281, 330}, { 331, 99999} };
static  const int DARKCLASS_DEVILSQUARELEVEL[6][2] = { {15, 110}, { 111, 160}, { 161, 210}, {211, 260}, { 261, 310}, { 311, 99999} };
int g_iDevilSquareLevel[6][2];

// (0~5)일반 레벨제한, (6~9)(마검사|다크로드) 레벨제한
static  const int g_iChaosCastleLevel[12][2] = {  
						{ 15, 49 }, { 50, 119 }, { 120, 179 }, { 180, 239 }, { 240, 299 }, { 300, 999 }, 
                        { 15, 29 }, { 30,  99 }, { 100, 159 }, { 160, 219 }, { 220, 279 }, { 280, 999 }  
												};
//  기본 천젠(1000젠)이다.
#ifdef CHINESE_PRICERISE
static  const int g_iChaosCastleZen[6] = {  25, 80, 200, 350, 600, 900 };
#else
static  const int g_iChaosCastleZen[6] = {  25, 80, 150, 250, 400, 650 };
#endif // CHINESE_PRICERISE

BYTE BuyItem[4];

//  스탯 갯수.
static  int iStateNum = 4;

//////////////////////////////////////////////////////////////////////////
//  Extern.
//////////////////////////////////////////////////////////////////////////
extern bool Teleport;

extern float g_fScreenRate_x;	// ※
extern float g_fScreenRate_y;

#ifdef _PVP_ADD_MOVE_SCROLL
extern CMurdererMove g_MurdererMove;
#endif	// _PVP_ADD_MOVE_SCROLL

///////////////////////////////////////////////////////////////////////////////
// 유틸리티.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  캐릭터 레벨에 따른 색을 결정한다.
//////////////////////////////////////////////////////////////////////////
int getLevelGeneration ( int level, unsigned int* color )
{
    int lvl;
    if(level>=300) //
    {
        lvl = 300;
	    *color = (255<<24)+(255<<16)+(153<<8)+(255);
    }
    else if(level>=200) //  흰색.
    {
        lvl = 200;
	    *color = (255<<24)+(255<<16)+(230<<8)+(210);
    }
    else if(level>=100) //  녹색.
    {
        lvl = 100;
        *color = (255<<24)+(24<<16)+(201<<8)+(0);
    }
    else if(level>=50)  //  주황색.
    {
        lvl = 50;
        *color = (255<<24)+(0<<16)+(150<<8)+(255);
    }
    else                //  빨간색.
    {
        lvl = 10;
        *color = (255<<24)+(0<<16)+(0<<8)+(255);
    }
    return lvl;
}
///////////////////////////////////////////////////////////////////////////////
// 정보창 랜더링(도움말, 아이템, 스킬 정보창에 사용)
///////////////////////////////////////////////////////////////////////////////

char TextList[30][100];
int  TextListColor[30];
int  TextBold[30];
SIZE Size[30];

int RenderTextList(int sx,int sy,int TextNum,int Tab, int iSort = RT3_SORT_CENTER)	// ★
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

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i],lstrlen(TextList[i]),&Size[i]);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), TextList[i],lstrlen(TextList[i]),&Size[i]);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
      	
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
	// 창너비와 높이
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

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		
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
	if(iPos_x + fWidth > (int)WindowWidth / g_fScreenRate_x)	// 화면 밖으로 나간 것 처리
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
		RenderColor ((float)iPos_x-1, fsy - 1, (float)fWidth + 1, (float)1);	// 위
		RenderColor ((float)iPos_x-1, fsy - 1, (float)1, (float)fHeight + 1);	// 좌
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
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
			unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
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
#ifdef SOCKET_SYSTEM
			case TEXT_COLOR_VIOLET:
				glColor3f(0.7f,0.4f,1.0f);
				break;
#endif	// SOCKET_SYSTEM
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
			case TEXT_COLOR_ORANGE:
				glColor3f(0.9f, 0.42f, 0.04f );
				break;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

				
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
			g_pRenderText->RenderText(fsx,fsy,TextList[i],(fWidth-2),0,iSort, &TextSize);	// ★
			fHeight = TextSize.cy;
		}
		fsy += fHeight * 1.1f;
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	DisableAlphaBlend();
}

bool IsCanUseItem()
{
	if(	g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)
		)
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

///////////////////////////////////////////////////////////////////////////////
// 아이템 정보창에서 사용할 수 있는 클래스 찾아내는 함수
///////////////////////////////////////////////////////////////////////////////

int TextNum = 0;
int SkipNum = 0;

bool IsRequireClassRenderItem(const short sType)
{
	if(
		sType == ITEM_HELPER+19		// 0: 대천사의 절대지팡이, 1: 대천사의 절대검, 2: 대천사의 절대석궁
		|| sType == ITEM_HELPER+29	// 근위병의 갑옷세트.
		|| sType == ITEM_WING+26	// 신비의 구슬
		|| (sType >= ITEM_WING+30 && sType <= ITEM_WING+31)	// 보석 조합
		|| (sType >= ITEM_HELPER+43 && sType <= ITEM_HELPER+45)	// 인장 종류
		|| sType == ITEM_HELPER+10	// 변신반지
		|| (sType >= ITEM_HELPER+39 && sType <= ITEM_HELPER+42)	// 변신반지류
		|| sType == ITEM_HELPER+37
		|| sType == ITEM_WING+15	// 혼돈의 보석
		|| sType == ITEM_WING+32	// 빨간리본의상자
		|| sType == ITEM_WING+33	// 초록리본의상자
		|| sType == ITEM_WING+34	// 파란리본의상자
		)
	{
		return false;
	}

	if(
#ifdef CSK_LUCKY_SEAL
		(sType >= ITEM_HELPER+43 && sType <= ITEM_HELPER+45)
#endif //CSK_LUCKY_SEAL	
#ifdef CSK_FREE_TICKET
		|| (sType >= ITEM_HELPER+46 && sType <= ITEM_HELPER+48)
#endif // CSK_FREE_TICKET
#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
		|| (sType >= ITEM_HELPER+125 && sType <= ITEM_HELPER+127)
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
#ifdef CSK_CHAOS_CARD
		|| (sType == ITEM_POTION+54)
#endif // CSK_CHAOS_CARD
#ifdef CSK_RARE_ITEM
		|| (sType >= ITEM_POTION+58 && sType <= ITEM_POTION+62)
#endif // CSK_RARE_ITEM
#ifdef CSK_LUCKY_CHARM
		|| (sType == ITEM_POTION+53)
#endif //CSK_LUCKY_CHARM
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
		|| (sType >= ITEM_POTION+70 && sType <= ITEM_POTION+71)
#endif //PSW_ELITE_ITEM
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
		|| (sType >= ITEM_POTION+72 && sType <= ITEM_POTION+77)
#endif //PSW_SCROLL_ITEM
#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING	// 날개 조합 100% 성공 부적
		|| (sType >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN 
			&& sType <= ITEM_TYPE_CHARM_MIXWING+EWS_END)
#endif // LDS_ADD_CS6_CHARM_MIX_ITEM_WING
#ifdef PSW_SEAL_ITEM               // 이동 인장
		|| (sType == ITEM_HELPER+59)
#endif //PSW_SEAL_ITEM
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
		|| ( sType >= ITEM_HELPER+54 && sType <= ITEM_HELPER+58)
#endif //PSW_FRUIT_ITEM
#ifdef PSW_SECRET_ITEM             // 강화의 비약
		||(sType >= ITEM_POTION+78 && sType <= ITEM_POTION+82)
#endif //PSW_SECRET_ITEM
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
		|| (sType == ITEM_HELPER+60)
#endif //PSW_INDULGENCE_ITEM
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET // 환영의 사원 자유 입장권
		|| (sType == ITEM_HELPER+61)
#endif //PSW_CURSEDTEMPLE_FREE_TICKET
#ifdef PSW_CHARACTER_CARD         // 캐릭터 카드
		|| (sType == ITEM_POTION+91)
#endif // PSW_CHARACTER_CARD
#ifdef PSW_NEW_ELITE_ITEM         // 엘리트 중간 치료 물약
		|| (sType == ITEM_POTION+94)
#endif //PSW_NEW_ELITE_ITEM
#ifdef PSW_NEW_CHAOS_CARD
		|| (sType >= ITEM_POTION+92 && sType <= ITEM_POTION+93) // 카오스 카드 골드, 레어
		|| (sType == ITEM_POTION+95)							// 카오스 카드 미니
#endif // PSW_NEW_CHAOS_CARD
#ifdef PSW_ADD_PC4_SEALITEM
		|| ( sType >= ITEM_HELPER+62 && sType <= ITEM_HELPER+63 )
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SCROLLITEM
		|| ( sType >= ITEM_POTION+97 && sType <= ITEM_POTION+98 )
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
		|| (sType == ITEM_POTION+96)
#endif //PSW_ADD_PC4_CHAOSCHARMITEM
#ifdef LDK_ADD_PC4_GUARDIAN
		|| ( sType == ITEM_HELPER+64 || sType == ITEM_HELPER+65 )
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDK_ADD_RUDOLPH_PET
		|| ( sType == ITEM_HELPER+67 )
#endif //LDK_ADD_RUDOLPH_PET
#ifdef LDK_ADD_SNOWMAN_CHANGERING
		|| ( sType == ITEM_HELPER+68 )
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
		|| ( sType == ITEM_HELPER+76 )
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| ( sType == ITEM_HELPER+122 )	// 스켈레톤 변신반지
#endif	// YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
		|| ( sType == ITEM_HELPER+69 )	// 부활의 부적
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
		|| ( sType == ITEM_HELPER+70 )	// 이동의 부적
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef LDK_ADD_GAMBLE_RANDOM_ICON
		|| ( sType == ITEM_HELPER+71 || sType == ITEM_HELPER+72 || sType == ITEM_HELPER+73 || sType == ITEM_HELPER+74 ||sType == ITEM_HELPER+75 )	// 겜블러 아이콘
#endif //LDK_ADD_GAMBLE_RANDOM_ICON
#ifdef PJH_ADD_PANDA_PET
		|| ( sType == ITEM_HELPER+80 )
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
		|| ( sType == ITEM_HELPER+106 )
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef ASG_ADD_CS6_GUARD_CHARM
		|| sType == ITEM_HELPER+81	// 수호의부적
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
		|| sType == ITEM_HELPER+82	// 아이템보호부적
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		|| sType == ITEM_HELPER+93	// 상승의인장마스터
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		|| sType == ITEM_HELPER+94	// 풍요의인장마스터
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
		|| sType == ITEM_HELPER+121	// 카오스케슬 자유입장권
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE
#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12	// 희귀아이템티켓 7-12
		|| (sType >= ITEM_POTION+145 && sType <= ITEM_POTION+150)
#endif //LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(sType)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
		|| (sType==ITEM_POTION+133)		// 엘리트 SD회복 물약
#endif // YDG_ADD_CS7_ELITE_SD_POTION
#endif // KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP
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

	BYTE byFirstClass = GetBaseClass(Hero->Class);
	BYTE byStepClass = GetStepClass(Hero->Class);

	int iTextColor = 0;

	TextListColor[TextNum + 2] = TextListColor[TextNum + 3] = TEXT_COLOR_WHITE;
	sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

	// 모든 클래스가 사용가능하면 지나친다.
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
					// 61 "%s 사용 가능"
					// 20 "흑마법사"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[20]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 2)
				{
					// 61 "%s 사용 가능"
					// 25 "소울마스터"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[25]);
					TextListColor[TextNum] = iTextColor;	
				}
				else if(byRequireClass == 3)
				{
					// 61 "%s 사용 가능"
					// 1669 "그랜드마스터"
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
					// 61 "%s 사용 가능"
					// 21 "흑기사"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[21]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 2)
				{
					// 61 "%s 사용 가능"
					// 26 "블레이드나이트"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[26]);
					TextListColor[TextNum] = iTextColor;	
				}
				else if(byRequireClass == 3)
				{
					// 61 "%s 사용 가능"
					// 1668 "블레이드마스터"
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
					// 61 "%s 사용 가능"
					// 22 "요정"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[22]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 2)
				{
					// 61 "%s 사용 가능"
					// 27 "뮤즈엘프"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[27]);
					TextListColor[TextNum] = iTextColor;	
				}
				else if(byRequireClass == 3)
				{
					// 61 "%s 사용 가능"
					// 1670 "하이엘프"
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
					// 61 "%s 사용 가능"
					// 23 "마검사"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[23]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 3)
				{
					// 61 "%s 사용 가능"
					// 1671 "듀얼마스터"
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
					// 61 "%s 사용 가능"
					// 24 "다크로드"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[24]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 3)
				{
					// 61 "%s 사용 가능"
					// 1672 "로드엠퍼러"
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
					// 61 "%s 사용 가능"
					// 1687 "소환술사"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1687]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass == 2)
				{
					// 61 "%s 사용 가능"
					// 1688 "블러디서머너"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[1688]);
					TextListColor[TextNum] = iTextColor;	
				}
				else if(byRequireClass == 3)
				{
					// 61 "%s 사용 가능"
					// 1689 "디멘션마스터"
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
					// 61 "%s 사용 가능"	3150 "레이지파이터"
					sprintf(TextList[TextNum], GlobalText[61], GlobalText[3150]);
					TextListColor[TextNum] = iTextColor;
				}
				else if(byRequireClass==3)
				{
					// 61 "%s 사용 가능"	3151 "템플나이트"
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

///////////////////////////////////////////////////////////////////////////////
// 도움말 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////

#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
// 아이템 14 15 도움말 추가(09.12.2)
const int iMaxLevel = 15;	// 최대 아이템 레벨수
#else //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
const int iMaxLevel = 13;	// 최대 아이템 레벨수
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO

const int iMaxColumn = 17;	// 최대 정보 항목

int g_iCurrentItem = -1;
int g_iItemInfo[iMaxLevel + 1][iMaxColumn];	// 아이템 정보 미리 계산

void RenderHelpLine(int iColumnType, const char * pPrintStyle, int & TabSpace, const char * pGapText, int Pos_y, int iType)	// ★
{
    int iCurrMaxLevel = iMaxLevel;

    if ( iType==5 ) //  법서.
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
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[TextNum - 1], lstrlen(TextList[TextNum - 1]), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), TextList[TextNum - 1], lstrlen(TextList[TextNum - 1]), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	}
	else 
	{
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), pGapText, lstrlen(pGapText), &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), pGapText, lstrlen(pGapText), &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	}
	TabSpace += int(TextSize.cx / g_fScreenRate_x);
#ifdef CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	if(iType == 6)	// 소환술사 서이면
	{
		TabSpace += 5;	
	}
#endif // CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	TextNum -= iCurrMaxLevel + 1;
}

void RenderHelpCategory(int iColumnType, int Pos_x, int Pos_y)	// ★ 제목 항목 출력
{
	const char* pText = NULL;

	switch(iColumnType)
	{
	case _COLUMN_TYPE_LEVEL:
		pText = GlobalText[161];//" 레벨";
		break;
	case _COLUMN_TYPE_ATTMIN: case _COLUMN_TYPE_ATTMAX:
		pText = GlobalText[162];//" 공격력";
		break;
	case _COLUMN_TYPE_MAGIC:
		pText = GlobalText[163];//"마력 ";
		break;
#ifdef CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	case _COLUMN_TYPE_CURSE:
		pText = GlobalText[1144];	// 1144 "저주력"
		break;
#endif // CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
    case _COLUMN_TYPE_PET_ATTACK:
		pText = GlobalText[1239];//"팻공격력 ";
        break;
	case _COLUMN_TYPE_DEFENCE:
		pText = GlobalText[164];//"방어력 ";
		break;
	case _COLUMN_TYPE_DEFRATE:
		pText = GlobalText[165];//"방어율";
		break;
	case _COLUMN_TYPE_REQSTR:
		pText = GlobalText[166];//"힘 ";
		break;
	case _COLUMN_TYPE_REQDEX:
		pText = GlobalText[167];//"민첩";
		break;
	case _COLUMN_TYPE_REQENG:
		pText = GlobalText[168];//"에너지";
		break;
	case _COLUMN_TYPE_REQCHA:
		pText = GlobalText[1900];//"통솔";
		break;
	case _COLUMN_TYPE_REQVIT:
		pText = GlobalText[169];//"체력";
		break;
	case _COLUMN_TYPE_REQNLV:
		pText = GlobalText[1931];//"필요레벨";
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

void ComputeItemInfo(int iHelpItem)	// ★
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
		int DamageMin = p->DamageMin;	// 최소공격력
		int DamageMax = p->DamageMax;	// 최대공격력
		int Defense   = p->Defense;		// 방어력
		int Magic	  = p->MagicPower;	// 마력
		int Blocking  = p->SuccessfulBlocking;	// 방어율

		if(DamageMin > 0)
        {
            DamageMin += (min(9,Level)*3);	// ~ +9아이템
			switch(Level - 9)
			{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
			case 6: DamageMin += 9;	// +15 아이템
			case 5: DamageMin += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
			case 4: DamageMin += 7;	// +13 아이템
			case 3: DamageMin += 6;	// +12 아이템
			case 2: DamageMin += 5;	// +11 아이템
			case 1: DamageMin += 4;	// +10 아이템
			default: break;
			};
        }
		if(DamageMax > 0)
        {
            DamageMax += (min(9,Level)*3);	// ~ +9아이템
			switch(Level - 9)
			{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
			case 6: DamageMax += 9;	// +15 아이템
			case 5: DamageMax += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
			case 4: DamageMax += 7;	// +13 아이템
			case 3: DamageMax += 6;	// +12 아이템
			case 2: DamageMax += 5;	// +11 아이템
			case 1: DamageMax += 4;	// +10 아이템
			default: break;
			};
        }

		if(Magic > 0)
		{
			Magic += (min(9,Level)*3);	// ~ +9아이템
			switch(Level - 9)
			{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
			case 6: Magic += 9;		// +15 아이템
			case 5: Magic += 8;		// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
			case 4: Magic += 7;	    // +13 아이템
			case 3: Magic += 6;		// +12 아이템
			case 2: Magic += 5;		// +11 아이템
			case 1: Magic += 4;		// +10 아이템
			default: break;
			};
			Magic /= 2;
            //  셉터가 아닐 경우에만.
#ifdef CSK_FIX_EPSOLUTESEPTER
			if(IsCepterItem(ItemHelp) == false)
#else // CSK_FIX_EPSOLUTESEPTER
			if ( ItemHelp < ITEM_MACE+8 || ItemHelp > ITEM_MACE+15 )
#endif // CSK_FIX_EPSOLUTESEPTER
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
                Defense     += (min(9,Level)*3);	// ~ +9아이템
				switch(Level - 9)
				{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
				case 6: Defense += 9;	// +15 아이템
				case 5: Defense += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
				case 4: Defense += 7;	// +13 아이템
				case 3: Defense += 6;	// +12 아이템
				case 2: Defense += 5;	// +11 아이템
				case 1: Defense += 4;	// +10 아이템
				default: break;
				};
            }
		}
		if(Blocking  > 0)
		{
            Blocking     += (min(9,Level)*3);	// ~ +9아이템
			switch(Level - 9)
			{
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
			case 6: Blocking += 9;	// +15 아이템
			case 5: Blocking += 8;	// +14 아이템
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
			case 4: Blocking += 7;	// +13 아이템
			case 3: Blocking += 6;	// +12 아이템
			case 2: Blocking += 5;	// +11 아이템
			case 1: Blocking += 4;	// +10 아이템
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
			if (ItemHelp >= ITEM_STAFF+21 && ItemHelp <= ITEM_STAFF+29)	// 소환술사 서는 요구에너지가 낮다
			{
				RequireEnergy    = 20+p->RequireEnergy   *(p->Level+Level*1)*3/100;
			}
			else
// p->RequireLevel : 아이템 레벨 요구치
// p->RequireEnergy : 아이템 에너지 요구치
// ip->RequireEnergy : 실제 에너지 요구치 (꼐산된값)
#ifdef KJH_FIX_LEARN_SKILL_ITEM_REQUIRE_STAT_CALC
		// 레벨 요구치가 0이면 원래 계산대로 간다.
		// skill.txt 에서 기존 아이템 레벨요구치를 변경할 수가 없어서, 예외처리 함.
		// 요구치 계산이 코드에 왜 있어야 하는지 의문.. 고치자!!!!
		if((p->RequireLevel > 0) && (ItemHelp >= ITEM_ETC && ItemHelp < ITEM_ETC+MAX_ITEM_INDEX) )	// 법서
		{
			RequireEnergy = 20+(p->RequireEnergy)*(p->RequireLevel)*4/100;
		}
		else
#endif // KJH_FIX_LEARN_SKILL_ITEM_REQUIRE_STAT_CALC
			{
				RequireEnergy    = 20+p->RequireEnergy   *(p->Level+Level*3)*4/100;
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
		
#ifdef CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
		// 소환술사 서는 저주력 칼럼에 값을 입력하여야 한다.
		if (ItemHelp >= ITEM_STAFF+21 && ItemHelp <= ITEM_STAFF+29)
		{
			g_iItemInfo[Level][_COLUMN_TYPE_CURSE] = Magic;
		}
		else
		{
			g_iItemInfo[Level][_COLUMN_TYPE_MAGIC] = Magic;
		}
#else // CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
		g_iItemInfo[Level][_COLUMN_TYPE_MAGIC] = Magic;
#endif // CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
		
		g_iItemInfo[Level][_COLUMN_TYPE_PET_ATTACK] = Magic;
		g_iItemInfo[Level][_COLUMN_TYPE_DEFENCE] = Defense;
		g_iItemInfo[Level][_COLUMN_TYPE_DEFRATE] = Blocking;
		g_iItemInfo[Level][_COLUMN_TYPE_REQSTR] = RequireStrength;
		g_iItemInfo[Level][_COLUMN_TYPE_REQDEX] = RequireDexterity;
		g_iItemInfo[Level][_COLUMN_TYPE_REQENG] = RequireEnergy;
		g_iItemInfo[Level][_COLUMN_TYPE_REQCHA] = RequireCharisma;
		g_iItemInfo[Level][_COLUMN_TYPE_REQVIT] = RequireVitality;
		g_iItemInfo[Level][_COLUMN_TYPE_REQNLV] = RequireLevel;

        //  아이템이 셉터일경우.
#ifdef CSK_FIX_EPSOLUTESEPTER
		if ( IsCepterItem(ItemHelp) == true )
#else // CSK_FIX_EPSOLUTESEPTER
		if ( ItemHelp>=ITEM_MACE+8 && ItemHelp<=ITEM_MACE+15 )
#endif // CSK_FIX_EPSOLUTESEPTER	
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

///////////////////////////////////////////////////////////////////////////////
// 아이템 정보창 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////
unsigned int getGoldColor ( DWORD Gold )
{
    if ( Gold>=10000000 )        //  빨강.
    {
        return  (255<<24)+(0<<16)+(0<<8)+(255);
    }
    else if ( Gold>=1000000 )    //  주황.
    {
        return  (255<<24)+(0<<16)+(150<<8)+(255);
    }
    else if( Gold>=100000 )      //  녹색.
    {
        return  (255<<24)+(24<<16)+(201<<8)+(0);
    }

    return  (255<<24)+(150<<16)+(220<<8)+(255);
}

#ifdef KJH_MOD_BTS191_GOLD_FLOATING_NUMBER
void ConvertGold(double dGold, unicode::t_char* szText, int iDecimals /*= 0*/)
{
	unicode::t_char szTemp[256];
	int iCipherCnt=0;
	DWORD dwValueTemp = (DWORD)dGold;
	
	// 정수자리
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

	// 소수점 아랫 자리 
	if( iDecimals > 0 )
	{
		dwValueTemp = (int)(dGold*pow(10.f,(float)iDecimals))%(int)pow(10.f, (float)iDecimals);
		unicode::_sprintf(szTemp, ".%d", dwValueTemp);
		strcat(szText, szTemp);
	}
}
#else // KJH_MOD_BTS191_GOLD_FLOATING_NUMBER
void ConvertGold(DWORD Gold,unicode::t_char* Text)
{
	int Gold1 = Gold%1000;
	int Gold2 = Gold%1000000/1000;
	int Gold3 = Gold%1000000000/1000000;
	int Gold4 = Gold/1000000000;
	if(Gold >= 1000000000)
		unicode::_sprintf(Text,"%d,%03d,%03d,%03d",Gold4,Gold3,Gold2,Gold1);
	else if(Gold >= 1000000)
		unicode::_sprintf(Text,"%d,%03d,%03d",Gold3,Gold2,Gold1);
	else if(Gold >= 1000)
		unicode::_sprintf(Text,"%d,%03d",Gold2,Gold1);
	else 
		unicode::_sprintf(Text,"%d",Gold1);
}
#endif // KJH_MOD_BTS191_GOLD_FLOATING_NUMBER

#ifdef YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH
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
#endif	// YDG_FIX_CATLE_MONEY_INT64_TYPE_CRASH

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
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float   baseP = 1.f;
	float   addP  = 1.f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	bool    doubleP = false;
	
	if ( persent > 0 )
	{
		float fRoot = ( float)sqrt( ( double)repairGold);
		float fRootRoot = ( float)sqrt( sqrt( ( double)repairGold));
		repairGold = 3.f * fRoot * fRootRoot;

		// 두배
		if(doubleP)
		{
			repairGold *= 2;
		}
		
		repairGold *= persent;
		repairGold++;

		// 내구도가 0일때
		if ( Durability <= 0 )
		{
#ifdef KJH_FIX_REPAIR_DARKLOAD_PET_DURABILITY_ZERO
			if(Type == ITEM_HELPER+4 ||  Type == ITEM_HELPER+5)	// 다크호스, 다크스피릿 예외처리
			{
				repairGold *= 2;
			}
			else
			{
#ifdef PBG_FIX_REPAIRGOLD_DURABILITY0
				repairGold = repairGold*0.4f;
#else //PBG_FIX_REPAIRGOLD_DURABILITY0
				repairGold += repairGold*0.4f;
#endif //PBG_FIX_REPAIRGOLD_DURABILITY0
			}
#else // KJH_FIX_REPAIR_DARKLOAD_PET_DURABILITY_ZERO
#ifdef PBG_FIX_REPAIRGOLD_DURABILITY0
				repairGold = repairGold*0.4f;
#else //PBG_FIX_REPAIRGOLD_DURABILITY0
				repairGold += repairGold*0.4f;
#endif //PBG_FIX_REPAIRGOLD_DURABILITY0
#endif // KJH_FIX_REPAIR_DARKLOAD_PET_DURABILITY_ZERO
		}
	}
	else
	{
		repairGold = 0;
	}
	
#ifdef KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true && g_pNPCShop->IsRepairShop())
	{
		Gold = (int)(repairGold);		// npc상점 수리 금액 (일반공식)
	}
	else if((g_pMyInventory->IsVisible()) && (!g_pNPCShop->IsVisible()))
	{
#ifdef PBG_MOD_INVENTORY_REPAIR_COST
		Gold = (int)(repairGold + (repairGold * 1.5f));		// 자동수리 금액 하향 조정
#else //PBG_MOD_INVENTORY_REPAIR_COST
		Gold = (int)(repairGold + (repairGold * 2.0f));		// 인벤토리 자동수리 금액 공식(3배공식)
#endif //PBG_MOD_INVENTORY_REPAIR_COST
	}	
	else
	{
		// 조련사npc 대화창이 떠있는지 검사를 할수 없기 때문에,
		// 위쪽에서 예외처리 하고 나머지를 조련사npc라고 가정하였다.
		Gold = (int)(repairGold);		// npc상점(조련사) 수리 금액 (일반공식)
	}
#else // KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true && g_pNPCShop->IsRepairShop())
	{
		Gold = (int)(repairGold);		// npc상점 전체수리 금액
	}
	else if(Type == ITEM_HELPER+4 ||  Type == ITEM_HELPER+5)	// 다크호스, 다크스피릿은 가격 동일
	{
		Gold = (int)(repairGold);		
	}
	else
	{
#ifdef KJH_FIX_INVENTORY_REPAIR_COST
		Gold = (int)(repairGold + (repairGold * 2.0f));		// 인벤토리 자동수리 금액 공식
#else // KJH_FIX_INVENTORY_REPAIR_COST
		Gold = (int)(repairGold + (repairGold * 0.05f));
#endif // KJH_FIX_INVENTORY_REPAIR_COST
	}
#endif // KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
	
	if(Gold >= 1000)
	{
		Gold = (Gold / 100) * 100;
	}
#ifdef PBG_FIX_REPAIRGOLD_DURABILITY0
	else if(Gold > 10)
#else //PBG_FIX_REPAIRGOLD_DURABILITY0
	else if(Gold >= 100)
#endif //PBG_FIX_REPAIRGOLD_DURABILITY0
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
    //  장비창의 아이템을 검사한다.
	for( int i=0; i<MAX_EQUIPMENT; ++i )
	{
		if( CharacterMachine->Equipment[i].Type != -1)
        {
            ITEM* ip = &CharacterMachine->Equipment[i];
	        ITEM_ATTRIBUTE* p = &ItemAttribute[ip->Type];

			int Level = (ip->Level>>3)&15;
            int maxDurability = calcMaxDurability(ip, p, Level);

#ifdef LDK_FIX_USING_ISREPAIRBAN_FUNCTION
			// 있는 함수를 활용합시다...
			if(IsRepairBan(ip) == true)
			{
				continue;
			}
#else //LDK_FIX_USING_ISREPAIRBAN_FUNCTION
			// CSK수리금지
#ifdef CSK_PCROOM_ITEM
			if(ip->Type >= ITEM_POTION+55 && ip->Type <= ITEM_POTION+57)
			{
				continue;
			}
#endif // CSK_PCROOM_ITEM
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH	
			if(ip->Type == ITEM_HELPER+96)	// 강함의 인장 (PC방 아이템, 일본 6차 컨텐츠)
			{
				continue;
			}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH	

			//. item filtering
			if( (ip->Type >= ITEM_HELPER && ip->Type <= ITEM_HELPER+5) || ip->Type == ITEM_HELPER+10 || ip->Type == ITEM_HELPER+31 )   // 이혁재 - 수정 수리할때 ITEM_HELPER+5 인 다크 스피릿 누락됨
				continue;
			if( ip->Type == ITEM_BOW+7 || ip->Type == ITEM_BOW+15 || ip->Type >= ITEM_POTION )
				continue;
			if( ip->Type >= ITEM_WING+7 && ip->Type <= ITEM_WING+19 )	//. 구슬 필터링
				continue;
			if( (ip->Type >= ITEM_HELPER+14 && ip->Type <= ITEM_HELPER+19) || ip->Type==ITEM_POTION+21 )
				continue;
			if( ip->Type == ITEM_HELPER+20)
				continue;
#ifdef MYSTERY_BEAD
			if( ip->Type == ITEM_WING+26)
				continue;
#endif // MYSTERY_BEAD

			if(ip->Type == ITEM_HELPER+37)
				continue;

			if( ip->Type == ITEM_HELPER+38 )
				continue;
#endif //LDK_FIX_USING_ISREPAIRBAN_FUNCTION

			//. check durability
            if( ip->Durability<maxDurability )
            {
                int gold = ConvertRepairGold(ItemValue(ip,2),ip->Durability, maxDurability, ip->Type, text);

#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 장비창의 수리비 검사 제외
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
			
			// CSK수리금지
#ifdef CSK_PCROOM_ITEM
			if(pItem->Type >= ITEM_POTION+55 && pItem->Type <= ITEM_POTION+57)
			{
				continue;
			}
#endif // CSK_PCROOM_ITEM
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
			if(pItem->Type == ITEM_HELPER+96)	// 강함의 인장 (PC방 아이템, 일본 6차 컨텐츠)
			{
				continue;
			}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH	
			//. item filtering
			if( (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HELPER+5) || pItem->Type == ITEM_HELPER+10  || pItem->Type == ITEM_HELPER+31)  // 이혁재 - 수정 수리할때 ITEM_HELPER+5 인 다크 스피릿 누락됨
				continue;
			if( pItem->Type == ITEM_BOW+7 || pItem->Type == ITEM_BOW+15 || pItem->Type >= ITEM_POTION )
				continue;
			if( pItem->Type >= ITEM_WING+7 && pItem->Type <= ITEM_WING+19 )	//. 구슬 필터링
				continue;
			if( (pItem->Type >= ITEM_HELPER+14 && pItem->Type <= ITEM_HELPER+19) || pItem->Type==ITEM_POTION+21 )
				continue;
			if( pItem->Type == ITEM_HELPER+20)
				continue;
#ifdef MYSTERY_BEAD
			if( pItem->Type == ITEM_WING+26)
				continue;
#endif // MYSTERY_BEAD
			if( pItem->Type == ITEM_HELPER+38 )
				continue;
#ifdef CSK_FREE_TICKET
			// 아이템 수리 안되게 예외 처리
			if(pItem->Type >= ITEM_HELPER+46 && pItem->Type <= ITEM_HELPER+48)
			{
				continue;
			}
#endif // CSK_FREE_TICKET
#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
			if(pItem->Type >= ITEM_HELPER+125 && pItem->Type <= ITEM_HELPER+127)
			{
				continue;
			}
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
			// 아이템 수리 안되게 예외 처리
			if(pItem->Type >= ITEM_POTION+145 && pItem->Type <= ITEM_POTION+150)
			{
				continue;
			}
#endif // LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12

#ifdef CSK_RARE_ITEM
			// 아이템 수리 안되게 예외 처리
			if(pItem->Type >= ITEM_POTION+58 && pItem->Type <= ITEM_POTION+62)
			{
				continue;
			}
#endif // CSK_RARE_ITEM

#ifdef CSK_LUCKY_CHARM
			if( pItem->Type == ITEM_POTION+53 )// 행운의 부적
			{
				continue;
			}
#endif //CSK_LUCKY_CHARM
			
#ifdef CSK_LUCKY_SEAL
			if( pItem->Type == ITEM_HELPER+43 || pItem->Type == ITEM_HELPER+44 || pItem->Type == ITEM_HELPER+45 )
			{
				continue;
			}
#endif //CSK_LUCKY_SEAL
			
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
			if( pItem->Type >= ITEM_POTION+70 && pItem->Type <= ITEM_POTION+71 )
			{
				continue;
			}
#endif //PSW_ELITE_ITEM
			
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
			if( pItem->Type >= ITEM_POTION+72 && pItem->Type <= ITEM_POTION+77 )
			{
				continue;
			}
#endif //PSW_SCROLL_ITEM
			
#ifdef PSW_SEAL_ITEM               // 이동 인장
			if( pItem->Type == ITEM_HELPER+59 )
			{
				continue;
			}
#endif //PSW_SEAL_ITEM
			
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
			if( pItem->Type >= ITEM_HELPER+54 && pItem->Type <= ITEM_HELPER+58 )
			{
				continue;
			}
#endif //PSW_FRUIT_ITEM
			
#ifdef PSW_SECRET_ITEM             // 강화의 비약
			if( pItem->Type >= ITEM_POTION+78 && pItem->Type <= ITEM_POTION+82 )
			{
				continue;
			}
#endif //PSW_SECRET_ITEM
			
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
			if( pItem->Type == ITEM_HELPER+60 )
			{
				continue;
			}
#endif //PSW_INDULGENCE_ITEM
			
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET  // 환영의 사원 자유 입장권
			if( pItem->Type == ITEM_HELPER+61 )
			{
				continue;
			}
#endif //PSW_CURSEDTEMPLE_FREE_TICKET

#ifdef PSW_CHARACTER_CARD						// 소환술사 캐릭터 카드
			if( pItem->Type == ITEM_POTION+91 )
			{
				continue;
			}
#endif //PSW_CHARACTER_CARD						// 소환술사 캐릭터 카드

#ifdef PSW_NEW_CHAOS_CARD						// 신규 카오스 카드
			if( pItem->Type >= ITEM_POTION+92 && pItem->Type <= ITEM_POTION+93 )
			{
				continue;
			}
			if( pItem->Type == ITEM_POTION+95 )
			{
				continue;
			}
#endif //PSW_NEW_CHAOS_CARD						// 신규 카오스 카드

#ifdef PSW_NEW_ELITE_ITEM						// 중간 엘리트 물약
			if( pItem->Type == ITEM_POTION+95 )
			{
				continue;
			}
#endif //PSW_NEW_ELITE_ITEM						// 중간 엘리트 물약	

#ifdef PSW_ADD_PC4_SEALITEM
			if( pItem->Type >= ITEM_HELPER+62 && pItem->Type <= ITEM_HELPER+63 )
			{
				continue;
			}
#endif //PSW_ADD_PC4_SEALITEM

#ifdef PSW_ADD_PC4_SCROLLITEM
			if( pItem->Type >= ITEM_POTION+97 && pItem->Type <= ITEM_POTION+98 )
			{
				continue;
			}
#endif //PSW_ADD_PC4_SCROLLITEM
			
#ifdef YDG_ADD_HEALING_SCROLL
			if (pItem->Type == ITEM_POTION+140)	// 치유의 스크롤
			{
				continue;
			}
#endif	// YDG_ADD_HEALING_SCROLL
			
#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
			if( pItem->Type == ITEM_POTION+96 )
			{
				continue;
			}
#endif //PSW_ADD_PC4_CHAOSCHARMITEM

#ifdef LDK_ADD_PC4_GUARDIAN
			if ( pItem->Type == ITEM_HELPER+64 || pItem->Type == ITEM_HELPER+65 ) 
			{
				continue;
			}
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDK_ADD_RUDOLPH_PET
			if ( pItem->Type == ITEM_HELPER+67 ) 
			{
				continue;
			}
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
			if ( pItem->Type == ITEM_HELPER+80 ) 
			{
				continue;
			}
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
			if ( pItem->Type == ITEM_HELPER+106 ) 
			{
				continue;
			}
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
			if ( pItem->Type == ITEM_HELPER+123 )	// 스켈레톤 펫
			{
				continue;
			}
#endif	// YDG_ADD_SKELETON_PET
#ifdef LDK_ADD_SNOWMAN_CHANGERING
			//수리금지
			if ( pItem->Type == ITEM_HELPER+68 ) 
			{
				continue;
			}
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
			if ( pItem->Type == ITEM_HELPER+76 ) 
			{
				continue;
			}
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
			if ( pItem->Type == ITEM_HELPER+122 )	// 스켈레톤 변신반지
			{
				continue;
			}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
			if ( pItem->Type == ITEM_HELPER+69 ) 	// 부활의 부적
				continue;
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
			if ( pItem->Type == ITEM_HELPER+70 ) 	// 이동의 부적
				continue;
#endif	// YDG_ADD_CS5_PORTAL_CHARM
			if(pItem->Type == ITEM_HELPER+37)	// 펜릴
				continue;
#ifdef PBG_ADD_SANTAINVITATION
			if(pItem->Type == ITEM_HELPER+66)	//산타마을의 초대장
				continue;
#endif //PBG_ADD_SANTAINVITATION
#ifdef LDK_ADD_GAMBLE_RANDOM_ICON //겜블러 아이콘
			if(	pItem->Type == ITEM_HELPER+71 
				|| pItem->Type == ITEM_HELPER+72 
				|| pItem->Type == ITEM_HELPER+73
				|| pItem->Type == ITEM_HELPER+74
				|| pItem->Type == ITEM_HELPER+75 )
				continue;
#endif //LDK_ADD_GAMBLE_RANDOM_ICON
#ifdef ASG_ADD_CS6_GUARD_CHARM
			if (pItem->Type == ITEM_HELPER+81)	// 수호의부적
				continue;
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
			if (pItem->Type == ITEM_HELPER+82)	// 아이템보호부적
				continue;
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
			if (pItem->Type == ITEM_HELPER+93)	// 상승의인장마스터
				continue;
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
			if (pItem->Type == ITEM_HELPER+94)	// 풍요의인장마스터
				continue;
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER


#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING
			if( pItem->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN 
				&& pItem->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END ) // 날개 조합 100% 성공 부적
			{
				continue;
			}
#endif // LDS_ADD_CS6_CHARM_MIX_ITEM_WING
#ifdef PBG_ADD_CHARACTERCARD
			// 마검 다크 소환술사 카드
			if(pItem->Type == ITEM_HELPER+97 || pItem->Type == ITEM_HELPER+98 || pItem->Type == ITEM_POTION+91)
				continue;
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
			if(pItem->Type == ITEM_HELPER+99)		// 슬롯카드
				continue;
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PBG_ADD_SECRETITEM
			if(pItem->Type >= ITEM_HELPER+117 && pItem->Type <= ITEM_HELPER+120)	// 활력의 비약(최하급/하급/중급/상급)
				continue;
#endif //PBG_ADD_SECRETITEM
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
			if(pItem->Type == ITEM_HELPER+121)		// 인게임샾 아이템 // 카오스케슬 자유입장권
				continue;
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
			if(g_pMyInventory->IsInvenItem(pItem->Type))
				continue;

#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef LEM_FIX_SHOPITEM_DURABILITY_REPAIRGOLD	// 인게임샵 5-4에 추가된 일부 누락된 아이템 수리비 제외 [lem.2010.7.28]
		#ifdef LDK_ADD_INGAMESHOP_SMALL_WING	// 기간제 날개 작은(군주/재앙/요정/천공/사탄)의 날개 제외
			if( pItem->Type >= ITEM_WING+130 && pItem->Type <= ITEM_WING+134 )
				continue;
		#endif
		#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE
			if( pItem->Type == ITEM_HELPER+109 )
				continue;
		#endif
		#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY
			if( pItem->Type == ITEM_HELPER+110 )
				continue;
		#endif
		#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ
			if( pItem->Type == ITEM_HELPER+111 )
				continue;
		#endif
		#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST
			if( pItem->Type == ITEM_HELPER+112 )
				continue;
		#endif
		#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY
			if( pItem->Type == ITEM_HELPER+113 )
				continue;
		#endif
		#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD
			if( pItem->Type == ITEM_HELPER+114 )
				continue;
		#endif
		#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE
			if( pItem->Type == ITEM_HELPER+115 )
				continue;
		#endif
		#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
			if( pItem->Type == ITEM_HELPER+107 )
				continue;
		#endif			
#endif	// LEM_FIX_SHOPITEM_DURABILITY_REPAIRGOLD
#ifdef LEM_ADD_LUCKYITEM
		if( Check_ItemAction( pItem, eITEM_REPAIR ) )	continue;
#endif // LEM_ADD_LUCKYITEM

			//. check durability
			if( pItem->Durability<maxDurability )
			{
				int gold = ConvertRepairGold(ItemValue(pItem,2),pItem->Durability, maxDurability, pItem->Type, text);
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 인벤토리의 수리비 검사 제외
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
	"히딩크 고향 여행권",
	"펜티엄4 컴퓨터",
	"디지탈카메라",
	"로지텍 무선 마우스+키보드 세트",
	"256M 램",
    "6개월 잡지 구독권",
	"문화상품권(만원)",
	"뮤 머그컵",
	"뮤 T셔츠",
	"뮤 10시간 무료이용권"
};

WORD calcMaxDurability ( const ITEM* ip, ITEM_ATTRIBUTE *p, int Level )
{
    WORD maxDurability = p->Durability;
	
#ifdef KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
	// 다크로드 펫
#ifndef PBG_FIX_DARKPET_DURABILITY
	if( ip->Type==ITEM_HELPER+4 || ip->Type== ITEM_HELPER+5)
	{
#ifdef PBG_FIX_REPAIRGOLD_DARKPAT
		maxDurability = 255 -1;
#else //PBG_FIX_REPAIRGOLD_DARKPAT
		maxDurability = 255;		// 예외처리
#endif //PBG_FIX_REPAIRGOLD_DARKPAT
	}
#endif //PBG_FIX_DARKPET_DURABILITY
#endif // KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
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
#ifdef LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		if(i>=14)	// 15레벨
		{
			maxDurability = (maxDurability+8 >= 255 ? 255 : maxDurability+8);
		}
		else if(i>=13)	// 14레벨
		{
			maxDurability = (maxDurability+7 >= 255 ? 255 : maxDurability+7);
		}
		else
#endif //LDK_ADD_14_15_GRADE_ITEM_HELP_INFO
		if(i>=12)	// 13레벨
		{
			maxDurability+=6;
		}
		else if(i>=11)	// 12레벨
		{
			maxDurability+=5;
		}
		else 
		if(i>=10)	// 11레벨
		{
			maxDurability+=4;
		}
        else if(i>=9)	// 10레벨
		{
			maxDurability+=3;
		}
        else if(i>=4)	// 5~9레벨
        {
            maxDurability+=2;
        }
        else	// 1~4레벨
        {
            maxDurability++;
        }
    }
#ifdef PBG_FIX_DARKPET_DURABILITY
	if( ip->Type==ITEM_HELPER+4 || ip->Type== ITEM_HELPER+5)
	{
		maxDurability = 255;		// 예외처리
	}
#endif //PBG_FIX_DARKPET_DURABILITY
    if ( (ip->ExtOption%0x04)==EXT_A_SET_OPTION || (ip->ExtOption%0x04)==EXT_B_SET_OPTION )    //  세트 아이템.
    {
        maxDurability+=20;
    }
    else if( ( ip->Option1&63)>0 && 
		( ip->Type<ITEM_WING+3 || ip->Type>ITEM_WING+6 ) &&		// 1차 날개 포함, 2차 날개 제외.
		( ip->Type!=ITEM_SWORD+19 && ip->Type!=ITEM_BOW+18 && ip->Type!=ITEM_STAFF+10 )
        && ip->Type!=ITEM_HELPER+30   //  군주의 망토.
		&& ( ip->Type<ITEM_WING+36 || ip->Type>ITEM_WING+40 )	// 3차날개 제외.
#ifdef ADD_ALICE_WINGS_1
		&& (ip->Type<ITEM_WING+42 || ip->Type>ITEM_WING+43)	// 소환술사 2,3차날개 제외.
#endif	// ADD_ALICE_WINGS_1
		&& ip->Type!=ITEM_MACE+13	// 절대 셉터 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		&& !(ip->Type >= ITEM_WING+49 && ip->Type <= ITEM_WING+50)	//레이지파이터날개
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
	  )
    {
        maxDurability+=15;	// 엑템
    }
#ifdef LEM_ADD_LUCKYITEM		// 럭키아이템 내구도 레벨 Max설정 예외처리
	if( Check_LuckyItem(ip->Type) )
	{
		maxDurability	= 255;
	}
#endif // LEM_ADD_LUCKYITEM

    return  maxDurability;
}


//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
//	해당 아이템의 이름을 알아낸다.
//////////////////////////////////////////////////////////////////////////
void GetItemName ( int iType, int iLevel, char* Text )
{
	ITEM_ATTRIBUTE *p = &ItemAttribute[iType];

    //  아이템의 이름.
    if (iType>=ITEM_POTION+23 && iType<=ITEM_POTION+26 )   //  퀘스트 아이템.
    {
		if(iType == ITEM_POTION+23)	//. 제왕의 서, 영광의반지
		{
			switch( iLevel )
			{
			case 0: sprintf ( Text, "%s", p->Name ); break;			//	제왕의 서
			case 1: sprintf ( Text, "%s", GlobalText[906] ); break;	//  영광의 반지
			}
		}
		else if(iType == ITEM_POTION+24)	//. 부러진검, 다크스톤
		{
			switch( iLevel )
			{
			case 0: sprintf ( Text, "%s", p->Name ); break;			//	부러진검.
			case 1: sprintf ( Text, "%s", GlobalText[907] ); break;	//  다크스톤.
			}
		}
		else
		{
			sprintf ( Text, "%s", p->Name );
		}
    }
	else if(iType == ITEM_POTION+12)//이밴트 아이템
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", GlobalText[100] ); break;
		case 1:	sprintf ( Text, "%s", GlobalText[101] ); break;
		case 2:	sprintf ( Text, "%s", ChaosEventName[p->Durability] ); break;
		}
	}
	else if(iType == ITEM_POTION+11)//행운의 상자/성탄의별/폭죽/마법 주머니./사랑의 하트//훈장
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
		case 1: sprintf ( Text, "%s", GlobalText[105] ); break;
		case 2: sprintf ( Text, "%s", GlobalText[106] ); break;
		case 3: sprintf ( Text, "%s", GlobalText[107] ); break;
		case 5: sprintf ( Text, "%s", GlobalText[109] ); break;
		case 6: sprintf ( Text, "%s", GlobalText[110] ); break;
        case 7: sprintf ( Text, "%s", GlobalText[111] ); break; //  천공의 상자.
			break;
#ifdef USE_EVENT_ELDORADO
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:	// 천공의 상자
			sprintf(Text,"%s +%d",GlobalText[115], iLevel - 7);
			break;
#endif // USE_EVENT_ELDORADO
		case 13:	// 다크로드의 마음
			sprintf(Text, GlobalText[117]); break;
#ifdef NEW_YEAR_BAG_EVENT
        case 14:    //  복주머니.
			sprintf(Text, GlobalText[1650]); break;
            break;

        case 15:
			sprintf(Text, GlobalText[1651]); break;
            break;
#else
    #ifdef CHINA_MOON_CAKE
        case 14:    //  중국 월병.
            sprintf(Text,GlobalText[118]); break;
            break;
    #endif// CHINA_MOON_CAKE
#endif// NEW_YEAR_BAG_EVENT
		}
	}
    else if ( iType==ITEM_HELPER+15 )//  서클. ( 에너지/체력/민첩/힘/통솔 )
    {
        switch ( iLevel )
        {
        case 0:sprintf(Text,"%s %s", GlobalText[168], p->Name ); break;
        case 1:sprintf(Text,"%s %s", GlobalText[169], p->Name ); break;
        case 2:sprintf(Text,"%s %s", GlobalText[167], p->Name ); break;
        case 3:sprintf(Text,"%s %s", GlobalText[166], p->Name ); break;
		case 4:sprintf(Text,"%s %s", GlobalText[1900], p->Name ); break;	// 통솔열매
        }
    }
    else if ( iType==ITEM_HELPER+14 )//  로크의 깃털.
    {
        switch ( iLevel )
        {
        case 0: sprintf ( Text, "%s", p->Name ); break;             //  로크의 깃털.
        case 1: sprintf ( Text, "%s", GlobalText[1235] ); break;    //  군주의 문장.
        }
    }
    else if ( iType==ITEM_HELPER+31 )   //  영혼.
    {
        switch ( iLevel )
        {
        case 0: sprintf ( Text, "%s %s", GlobalText[1187], p->Name ); break;
        case 1: sprintf ( Text, "%s %s", GlobalText[1214], p->Name ); break;
        }
    }
#ifdef ANNIVERSARY_EVENT
    else if ( iType==ITEM_POTION+20 )//  사랑의 묘약
    {
        switch ( iLevel )
        {
        case 0: sprintf ( Text, "%s", p->Name ); break;
        case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			sprintf ( Text, "%s +%d", GlobalText[116], iLevel ); break;     //  애니버서리 박스
        }
    }
#endif	// ANNIVERSARY_EVENT
#ifdef _PVP_ADD_MOVE_SCROLL
    else if ( iType==ITEM_POTION+10 )//  이동문서
    {
        switch ( iLevel )
        {
        case 0: sprintf ( Text, "%s", p->Name ); break;
        case 1: sprintf ( Text, GlobalText[158], GlobalText[30] ); break;     //  로랜시아
		case 2: sprintf ( Text, GlobalText[158], GlobalText[33] ); break;     //  노리아
		case 3: sprintf ( Text, GlobalText[158], GlobalText[32] ); break;     //  데비아스
		case 4: sprintf ( Text, GlobalText[158], GlobalText[31] ); break;     //  던전
		case 5: sprintf ( Text, GlobalText[158], GlobalText[37] ); break;     //  아틀란스
		case 6: sprintf ( Text, GlobalText[158], GlobalText[34] ); break;     //  로스트타워
		case 7: sprintf ( Text, GlobalText[158], GlobalText[38] ); break;     //  타르칸
		case 8: sprintf ( Text, GlobalText[158], GlobalText[55] ); break;     //  이카루스
        }
    }
#endif	// _PVP_ADD_MOVE_SCROLL
#ifdef _PVP_MURDERER_HERO_ITEM
    else if ( iType==ITEM_POTION+30 )//  영웅살인마징표(pick)
    {
		sprintf ( Text, "%s", p->Name );
    }
#endif	// _PVP_MURDERER_HERO_ITEM
    else if ( iType==ITEM_POTION+21 )    //  레나. ( 1:스톤).
    {
        switch ( iLevel )
        {
        case 0: sprintf ( Text, "%s", p->Name ); break;
        case 1: sprintf ( Text, "%s", GlobalText[810] ); break;     //  스톤.
        case 2: sprintf ( Text, "%s", GlobalText[1098] ); break;    //  우정의 돌.
        case 3: sprintf ( Text, "%s", GlobalText[1290] ); break;    //  성주의표식
        }
    }
    else if ( iType==ITEM_HELPER+19 )	//  대천사의 절대 무기
    {
        sprintf ( Text, "%s", GlobalText[809] );
    }
	else if ( iType == ITEM_HELPER+20 )
	{
		switch(iLevel)
		{
		case 0: sprintf( Text, "%s", p->Name ); break;			//. 마법사의 반지	
		case 1: sprintf( Text, "%s", GlobalText[922] ); break;	//. 제왕의 반지
        case 2: sprintf( Text, "%s", GlobalText[928] ); break;	//. 전사의 반지
        case 3: sprintf( Text, "%s", GlobalText[929] ); break;	//. 전사의 반지
		}
	}
	else if(iType == ITEM_POTION+9)//술, 사랑의 올리브
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
		case 1:	sprintf ( Text, "%s", GlobalText[108] ); break;
		}
	}
    else if(iType == ITEM_WING+11)//소환구슬
	{
#ifdef KJW_FIX_ITEMNAME_ORB_OF_SUMMONING
		sprintf(Text,"%s %s",SkillAttribute[30+iLevel].Name,GlobalText[102]);
#else // KJW_FIX_ITEMNAME_ORB_OF_SUMMONING
        sprintf ( Text, "%s %s", GlobalText[738], GlobalText[102] );
#endif // KJW_FIX_ITEMNAME_ORB_OF_SUMMONING
	}
#ifdef MYSTERY_BEAD
	else if(iType == ITEM_WING+26)
	{
		switch(iLevel)
		{
		case 0:		//. 신비의구슬
			strcpy( Text, p->Name); break;
		case 1:		//. 빨강수정
			strcpy( Text, GlobalText[1831]); break;
		case 2:		//. 파랑수정
			strcpy( Text, GlobalText[1832]); break;
		case 3:		//. 검은수정
			strcpy( Text, GlobalText[1833]); break;
		case 4:		//. 보물상자
			strcpy( Text, GlobalText[1834]); break;
		case 5:		//. 깜짝선물
			strcpy( Text, GlobalText[1838]); break;
		}
	}
#endif // MYSTERY_BEAD
	else if(iType == ITEM_WING+32)//빨간리본상자.(x-mas이벤트용)
	{
		sprintf(Text, "%s", p->Name);	
	}
	else if(iType == ITEM_WING+33)//초록리본상자.(x-mas이벤트용)
	{
		sprintf(Text, "%s", p->Name);	
	}
	else if(iType == ITEM_WING+34)//초록리본상자.(x-mas이벤트용)
	{
		sprintf(Text, "%s", p->Name);	
	}
	else if(iType == ITEM_WING+35)//파이어스크림
	{
		sprintf(Text, "%s", p->Name);	
	}
	else if(iType >= ITEM_POTION+45 && iType <= ITEM_POTION+50)
	{
 		sprintf ( Text, "%s", p->Name ); 
	}
#ifdef GIFT_BOX_EVENT
	else if(iType == ITEM_POTION+32)//분홍 초콜릿상자.(발렌타인데이 이벤트용)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
 		case 1:	sprintf ( Text, "%s", GlobalText[2012] ); break;
		}
	}
	else if(iType == ITEM_POTION+33)//빨간 초콜릿상자.(발렌타인데이 이벤트용)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
		case 1:	sprintf ( Text, "%s", GlobalText[2013] ); break;
		}
	}
	else if(iType == ITEM_POTION+34)//파란 초콜릿상자.(발렌타인데이 이벤트용)
	{
		switch(iLevel)
		{
		case 0: sprintf ( Text, "%s", p->Name ); break;
		case 1:	sprintf ( Text, "%s", GlobalText[2014] ); break;
		}
	}
#endif

    else if(iType == ITEM_HELPER+10)//변신반지
	{
		for(int i=0;i<MAX_MONSTER;i++)
		{
			if(SommonTable[iLevel] == MonsterScript[i].Type)
			{
        		sprintf(Text,"%s %s",MonsterScript[i].Name,GlobalText[103]);
			}
		}
	}
    else if ( iType>=ITEM_WING+3 && iType<=ITEM_WING+6 )  //  정령의 날개 ~ 어둠의 날개.
    {
		if(iLevel==0)
			sprintf(Text,"%s",p->Name);
		else
			sprintf(Text,"%s +%d",p->Name,iLevel);
    }
#ifdef ADD_ALICE_WINGS_1
	else if ((iType>=ITEM_WING+36 && iType<=ITEM_WING+40) || (iType>=ITEM_WING+42 && iType<=ITEM_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (iType == ITEM_WING+50) //군림의망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)	// 폭풍의 날개 ~ 제왕의 망토, 소환술사 2,3차 날개.
#else	// ADD_ALICE_WINGS_1
	else if ( iType>=ITEM_WING+36 && iType<=ITEM_WING+40 )  //  폭풍의 날개 ~ 제왕의 망토
#endif	// ADD_ALICE_WINGS_1
    {
		if(iLevel==0)
			sprintf(Text,"%s",p->Name);
		else
			sprintf(Text,"%s +%d",p->Name,iLevel);
    }
	else if ( iType==ITEM_SWORD+19 || iType==ITEM_BOW+18 || iType==ITEM_STAFF+10 || iType==ITEM_MACE+13) //	대천사의 절대 무기.
	{
		if(iLevel==0)
			sprintf(Text,"%s",p->Name);
		else
			sprintf(Text,"%s +%d",p->Name,iLevel);
	}
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	else if ( COMGEM::NOGEM != COMGEM::Check_Jewel_Com(iType) )
	{
		// 보석 묶음
 		sprintf(Text,"%s +%d", p->Name, iLevel+1);
	}
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	else if ( iType==INDEX_COMPILED_CELE)
	{
		sprintf(Text,"%s +%d", GlobalText[1806], iLevel+1);	// 축복의 보석 묶음
	}
	else if ( iType==INDEX_COMPILED_SOUL)
	{
		sprintf(Text,"%s +%d", GlobalText[1807], iLevel+1);	// 영혼의 보석 묶음
	}
#ifdef ADD_SEED_SPHERE_ITEM
	else if ((iType >= ITEM_WING+60 && iType <= ITEM_WING+65)	// 시드
		|| (iType >= ITEM_WING+70 && iType <= ITEM_WING+74)	// 스피어
		|| (iType >= ITEM_WING+100 && iType <= ITEM_WING+129))	// 시드스피어
	{
		sprintf(Text,"%s",p->Name);
	}
#endif	// ADD_SEED_SPHERE_ITEM
#ifdef LJH_FIX_BUG_MISSING_ITEM_NAMES_ITEM_HELPER_7 
    else if ( iType == ITEM_POTION+7 ) //  스패셜 물약.//종훈물약
    {
		int iTextIndex = 0;
		
		iTextIndex = (iLevel == 0) ? 1413 : 1414;
        sprintf ( Text, "%s", GlobalText[iTextIndex] );
    }
#endif //LJH_FIX_BUG_MISSING_ITEM_NAMES_ITEM_HELPER_7
	else
	{
		if(iLevel==0)
			sprintf(Text,"%s",p->Name);
		else
			sprintf(Text,"%s +%d",p->Name,iLevel);
	}
}


//////////////////////////////////////////////////////////////////////////
//  아이템 옵션 설명 얻기.
//////////////////////////////////////////////////////////////////////////
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
void GetSpecialOptionText ( int Type, char* Text, WORD Option, BYTE Value, int iMana )
#else //PBG_ADD_NEWCHAR_MONK_SKILL
void GetSpecialOptionText ( int Type, char* Text, BYTE Option, BYTE Value, int iMana )
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
{
    switch(Option)
    {
    case AT_SKILL_BLOCKING:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[80], iMana);
        break;
    case AT_SKILL_SWORD1:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[81], iMana);
        break;
    case AT_SKILL_SWORD2:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[82], iMana);
        break;
    case AT_SKILL_SWORD3:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[83], iMana);
        break;
    case AT_SKILL_SWORD4:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[84], iMana);
        break;
    case AT_SKILL_SWORD5:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[85], iMana);
        break;
	case AT_SKILL_MANY_ARROW_UP:
	case AT_SKILL_MANY_ARROW_UP+1:
	case AT_SKILL_MANY_ARROW_UP+2:
	case AT_SKILL_MANY_ARROW_UP+3:
	case AT_SKILL_MANY_ARROW_UP+4:

    case AT_SKILL_CROSSBOW:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[86], iMana);
        break;
    case AT_SKILL_BLAST_CROSSBOW4:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[920], iMana);
        break;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
	case AT_SKILL_MULTI_SHOT:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[920], iMana);
		break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	case AT_SKILL_RECOVER:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text,GlobalText[920], iMana);
		break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_POWER_SLASH_UP:
	case AT_SKILL_POWER_SLASH_UP+1:
	case AT_SKILL_POWER_SLASH_UP+2:
	case AT_SKILL_POWER_SLASH_UP+3:
	case AT_SKILL_POWER_SLASH_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
    case AT_SKILL_ICE_BLADE:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
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
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf(Text, GlobalText[745], iMana  );
        break;
    case AT_SKILL_STRONG_PIER:  //  
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf ( Text, GlobalText[1210], iMana );
        break;
    case AT_SKILL_LONG_SPEAR:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf ( Text, GlobalText[1186], iMana );
        break;
	case AT_SKILL_ASHAKE_UP:
	case AT_SKILL_ASHAKE_UP+1:
	case AT_SKILL_ASHAKE_UP+2:
	case AT_SKILL_ASHAKE_UP+3:
	case AT_SKILL_ASHAKE_UP+4:
    case AT_SKILL_DARK_HORSE:
        GetSkillInformation( Option, 1, NULL, &iMana, NULL);
        sprintf ( Text, GlobalText[1189], iMana );
        break;
	case AT_SKILL_PLASMA_STORM_FENRIR:	// 플라즈마 스톰
		GetSkillInformation( Option, 1, NULL, &iMana, NULL);
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
	case AT_DAMAGE_REFLECTION:	// [대상의 공격이 유저에게 명중한 경우] 5% 확률로 적 공격력 50% 돌려줌
		sprintf(Text, GlobalText[1673], Value );
		break;
	case AT_RECOVER_FULL_LIFE:	// [대상의 공격이 유저에게 명중한 경우] 5% 확률로 유저 생명 100% 순간 회복
		sprintf(Text, GlobalText[1674], Value );
		break;
	case AT_RECOVER_FULL_MANA:	// [대상의 공격이 유저에게 명중한 경우] 5% 확률로 유저 마나 100% 순간 회복
		sprintf(Text, GlobalText[1675], Value );
		break;
	case AT_SKILL_SUMMON_EXPLOSION:
		sprintf(Text, GlobalText[1695], iMana );
		break;
	case AT_SKILL_SUMMON_REQUIEM:
		sprintf(Text, GlobalText[1696], iMana );
		break;
#ifdef ASG_ADD_SUMMON_RARGLE
	case AT_SKILL_SUMMON_POLLUTION:
		sprintf(Text, GlobalText[1789], iMana );
		break;
#endif	// ASG_ADD_SUMMON_RARGLE
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



//////////////////////////////////////////////////////////////////////////
//  아이템의 정보 출력.
//////////////////////////////////////////////////////////////////////////
#ifdef ASG_ADD_NEW_QUEST_SYSTEM
void RenderItemInfo(int sx,int sy,ITEM *ip,bool Sell, int Inventype, bool bItemTextListBoxUse)
#else	// ASG_ADD_NEW_QUEST_SYSTEM
void RenderItemInfo(int sx,int sy,ITEM *ip,bool Sell, int Inventype)
#endif	// ASG_ADD_NEW_QUEST_SYSTEM
{
	if (ip->Type == -1)
		return;

#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
	// 기간제 아이템일때 시간계산. (기간제 시간 정보가 오기 전에는 render하지 않는다.)
	tm* ExpireTime;
#ifdef KJH_FIX_EXPIRED_PERIODITEM_TOOLTIP
	if( ip->bPeriodItem == true && ip->bExpiredPeriod == false)
#else // KJH_FIX_EXPIRED_PERIODITEM_TOOLTIP
	if( ip->bPeriodItem == true )
#endif // KJH_FIX_EXPIRED_PERIODITEM_TOOLTIP
	{
		_tzset();
		if( ip->lExpireTime == 0 )
			return;

		ExpireTime = localtime((time_t*)&(ip->lExpireTime));
	}
#endif	// KJH_ADD_PERIOD_ITEM_SYSTEM
	
	ITEM_ATTRIBUTE *p = &ItemAttribute[ip->Type];
	TextNum = 0;
	SkipNum = 0;

	ZeroMemory( TextListColor, 20 * sizeof ( int));
	for(int i=0;i<30;i++)
	{
		TextList[i][0] = NULL;
	}

#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
#ifdef PET_SYSTEM
	if ( ip->Type==ITEM_HELPER+4 || ip->Type==ITEM_HELPER+5 ) 
	{
#ifdef PBG_FIX_DARKPET_TIPUPDATE
		BYTE PetType = PET_TYPE_DARK_SPIRIT;
		if(ip->Type==ITEM_HELPER+4)
		{
			PetType = PET_TYPE_DARK_HORSE;
#ifdef PBG_FIX_PETTIP
			if((g_pMyInventory->GetPointedItemIndex()) == EQUIPMENT_HELPER)
#endif //PBG_FIX_PETTIP
				SendRequestPetInfo(PetType, Inventype, EQUIPMENT_HELPER);
		}
		else
#ifdef PBG_FIX_PETTIP
			if((g_pMyInventory->GetPointedItemIndex()) == EQUIPMENT_WEAPON_LEFT)
#endif //PBG_FIX_PETTIP
			SendRequestPetInfo(PetType, Inventype, EQUIPMENT_WEAPON_LEFT);
#endif //PBG_FIX_DARKPET_TIPUPDATE
		giPetManager::RenderPetItemInfo( sx, sy, ip, Inventype );
		return;
	}
#endif// PET_SYSTEM
#else // KJH_FIX_DARKLOAD_PET_SYSTEM											//## 소스정리 대상임.
#ifdef PET_SYSTEM
    if ( giPetManager::RenderPetItemInfo( sx, sy, ip, Sell ) ) return;
#endif// PET_SYSTEM
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM											//## 소스정리 대상임.

	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	int Level = (ip->Level>>3)&15;
	int Color;

    //  아이템 색.
	if(ip->Type==ITEM_POTION+13 || ip->Type==ITEM_POTION+14 || ip->Type==ITEM_WING+15 || ip->Type==ITEM_POTION+31 ||	// 수호의보석
		(COMGEM::isCompiledGem(ip)) ||
		ip->Type == ITEM_POTION+65 || ip->Type == ITEM_POTION+66 || ip->Type == ITEM_POTION+67 ||	// 데쓰빔나이트의 불꽃, 헬마이네의 뿔, 어둠의불사조의 깃털
		ip->Type == ITEM_POTION+68 ||	// 심연의눈동자
		ip->Type==ITEM_HELPER+52 || ip->Type==ITEM_HELPER+53 ||	// 콘돌의 깃털 불꽃
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		ip->Type==ITEM_POTION+100 ||	//행운의 동전
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef PBG_ADD_GENSRANKING
		(ip->Type >= ITEM_POTION+141 && ip->Type <= ITEM_POTION+144) ||
#endif //PBG_ADD_GENSRANKING

#ifdef LEM_ADD_LUCKYITEM
		(ip->Type >= ITEM_HELPER+135 && ip->Type <= ITEM_HELPER+145) ||
		(ip->Type == ITEM_POTION+160 || ip->Type == ITEM_POTION+161) ||
#endif // LEM_ADD_LUCKYITEM
		ip->Type==ITEM_POTION+16 || ip->Type==ITEM_POTION+22 )  //  축복의 보석 ~ 창조의 보석.
	{
		Color = TEXT_COLOR_YELLOW;
	}
    //  대천사의 절대무기.
    else if ( ip->Type==ITEM_STAFF+10 || ip->Type==ITEM_SWORD+19 || ip->Type==ITEM_BOW+18 || ip->Type==ITEM_MACE+13)
    {
        Color = TEXT_COLOR_PURPLE;
    }
	else if(ip->Type==ITEM_POTION+17 || ip->Type==ITEM_POTION+18 || ip->Type==ITEM_POTION+19)	// 데빌스퀘어 관련 아이템
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
#ifdef _PVP_ADD_MOVE_SCROLL
    else if( ip->Type==ITEM_POTION+10 )
    {
        Color = TEXT_COLOR_WHITE;
    }
#endif	// _PVP_ADD_MOVE_SCROLL
#ifdef _PVP_MURDERER_HERO_ITEM
    else if( ip->Type==ITEM_POTION+30 )
    {
        Color = TEXT_COLOR_WHITE;
    }
#endif	// _PVP_MURDERER_HERO_ITEM
#ifdef SOCKET_SYSTEM
    else if( g_SocketItemMgr.IsSocketItem(ip) )
    {
        Color = TEXT_COLOR_VIOLET;
    }
#endif	// SOCKET_SYSTEM
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
#ifdef ADD_ALICE_WINGS_1
		|| (ip->Type>=ITEM_WING+42 && ip->Type<=ITEM_WING+43)
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)  //  정령의 날개 ~ 암흑의 날개. 망토, 폭풍의 날개 ~ 제왕의 망토, 소환술사 2,3차 날개.
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

#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX	// 인벤토리 아이템 툴팁 명 색깔
	int nGemType = COMGEM::Check_Jewel( ip->Type );
	if( nGemType != COMGEM::NOGEM)
	{
		Color = TEXT_COLOR_YELLOW;
	}
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX

    //  아이템의 가격.

// wani 부분 유료화 아이템도 IsSellingBan 여기에 추가 하였다

	if ( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) && !IsSellingBan(ip) )
	{
		char Text[100];
#ifndef KJH_DEL_PC_ROOM_SYSTEM				// #ifndef
#ifdef ADD_PCROOM_POINT_SYSTEM
		CPCRoomPtSys& rPCRoomPtSys = CPCRoomPtSys::Instance();
		if (rPCRoomPtSys.IsPCRoomPointShopMode())
		{
			if (Sell)    //  구매 가격.
			{
#ifdef ASG_PCROOM_POINT_SYSTEM_MODIFY
				// 2331 "구입 가격 : %d 포인트"
				::sprintf(TextList[TextNum], GlobalText[2331], ItemValue(ip, 0));
#else	// ASG_PCROOM_POINT_SYSTEM_MODIFY
				// 2331 "구입 가격 : %d 포인트 (구입 조건 : %d 이상)"
				::sprintf(TextList[TextNum], GlobalText[2331],
					ItemValue(ip, 0), GetItemBuyingTermsPoint(ip));
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY
				TextListColor[TextNum] = Color;
				++TextNum;

				::sprintf(TextList[TextNum],"\n");
				++TextNum;
				++SkipNum;
			}
		}
		else
#endif	// ADD_PCROOM_POINT_SYSTEM
#endif // KJH_DEL_PC_ROOM_SYSTEM
		{
			if(Sell)    //  구매 가격.
			{
				DWORD dwValue = ItemValue(ip, 0);
				ConvertGold(dwValue, Text);
				char Text2[100];

				// 세율가격 표시
				ConvertTaxGold(ItemValue(ip,0),Text2);
				sprintf(TextList[TextNum],GlobalText[1620],Text2,Text);	// 1620 "구입 가격: %s(%s)"
			}
			else        //  판매 가격.
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
		//. 등록된 물품에 한에서만 출력한다
		{
			int price = 0;
			int indexInv = (MAX_EQUIPMENT + MAX_INVENTORY)+(ip->y*COL_PERSONALSHOP_INVEN)+ip->x;
			char Text[100];

			if(GetPersonalItemPrice(indexInv, price, g_IsPurchaseShop)) 
			{
				ConvertGold(price, Text);
				sprintf(TextList[TextNum], GlobalText[63],Text);
				
				if ( price>=10000000 )        //  빨강.
					TextListColor[TextNum] = TEXT_COLOR_RED;
				else if ( price>=1000000 )    //  주황.
					TextListColor[TextNum] = TEXT_COLOR_YELLOW;
				else if( price>=100000 )      //  녹색.
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

    //  아이템의 이름.
    if (ip->Type>=ITEM_POTION+23 && ip->Type<=ITEM_POTION+26 )   //  퀘스트 아이템.
    {
		if(ip->Type == ITEM_POTION+23)	//. 제왕의 서, 영광의반지
		{
			Color = TEXT_COLOR_YELLOW;
			switch( Level )
			{
			case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;	//. 제왕의 서
			case 1: sprintf ( TextList[TextNum], GlobalText[906] ); break;  //  영광의 반지
			}
		}
		else if(ip->Type == ITEM_POTION+24)	//. 부러진검, 다크스톤
		{
			Color = TEXT_COLOR_YELLOW;
			switch( Level )
			{
			case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;	//. 부러진검
			case 1: sprintf ( TextList[TextNum], GlobalText[907] ); break;  //  다크스톤
			}
		}
		else{
			sprintf(TextList[TextNum],"%s", p->Name);
			Color = TEXT_COLOR_YELLOW;
		}
    }
	else if(ip->Type == ITEM_POTION+12)//이밴트 아이템
	{
		switch(Level)
		{
		case 0:sprintf(TextList[TextNum],GlobalText[100]);break;
		case 1:sprintf(TextList[TextNum],GlobalText[101]);break;
		case 2:sprintf(TextList[TextNum],"%s",ChaosEventName[ip->Durability]);break;
		}
	}
	else if(ip->Type == ITEM_POTION+11)//행운의 상자/성탄의별/폭죽/마법 주머니./사랑의 하트//훈장
	{
		switch(Level)
		{
		case 0:sprintf(TextList[TextNum],"%s",p->Name);break;
		case 1:sprintf(TextList[TextNum],GlobalText[105]);break;
		case 2:sprintf(TextList[TextNum],GlobalText[106]);break;
		case 3:sprintf(TextList[TextNum],GlobalText[107]);break;
		case 5:sprintf(TextList[TextNum],GlobalText[109]);break;
		case 6:sprintf(TextList[TextNum],GlobalText[110]);break;
        case 7:sprintf(TextList[TextNum],GlobalText[111]);break; //  천공의 상자.
#ifdef USE_EVENT_ELDORADO
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:	// 천공의 상자
			sprintf(TextList[TextNum],"%s +%d",GlobalText[115], Level - 7);
			break;
#endif
		case 13:	// 다크로드의 마음
			sprintf(TextList[TextNum],GlobalText[117]);
			break;
#ifdef NEW_YEAR_BAG_EVENT
        case 14:    //  복주머니.
            sprintf(TextList[TextNum],GlobalText[1650]);
            break;

        case 15:
            sprintf(TextList[TextNum],GlobalText[1651]);
            break;
#else 
    #ifdef CHINA_MOON_CAKE
        case 14:    //  중국 월병.
            sprintf(TextList[TextNum],GlobalText[118]);
            break;
    #endif// CHINA_MOON_CAKE
#endif// NEW_YEAR_BAG_EVENT
		}
	}
#ifdef ANNIVERSARY_EVENT
	else if(ip->Type == ITEM_POTION+20)	//사랑의 묘약
	{
		switch(Level)
		{
        case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;
        case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			sprintf ( TextList[TextNum], "%s +%d", GlobalText[116], Level );     //  애니버서리 박스
			break;
        }
    }
#endif	// ANNIVERSARY_EVENT
#ifdef _PVP_ADD_MOVE_SCROLL
    else if (ip->Type == ITEM_POTION+10)//  이동문서
    {
        switch (Level)
        {
        case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;
        case 1: sprintf ( TextList[TextNum], GlobalText[158], GlobalText[30] ); break;     //  로랜시아
		case 2: sprintf ( TextList[TextNum], GlobalText[158], GlobalText[33] ); break;     //  노리아
		case 3: sprintf ( TextList[TextNum], GlobalText[158], GlobalText[32] ); break;     //  데비아스
		case 4: sprintf ( TextList[TextNum], GlobalText[158], GlobalText[31] ); break;     //  던전
		case 5: sprintf ( TextList[TextNum], GlobalText[158], GlobalText[37] ); break;     //  아틀란스
		case 6: sprintf ( TextList[TextNum], GlobalText[158], GlobalText[34] ); break;     //  로스트타워
		case 7: sprintf ( TextList[TextNum], GlobalText[158], GlobalText[38] ); break;     //  타르칸
		case 8: sprintf ( TextList[TextNum], GlobalText[158], GlobalText[55] ); break;     //  이카루스
        }
    }
#endif	// _PVP_ADD_MOVE_SCROLL
#ifdef _PVP_MURDERER_HERO_ITEM
    else if ( ip->Type==ITEM_POTION+30 )//  영웅살인마징표(inv)
    {
		if (ip->Durability % 100 < 3) Color = TEXT_COLOR_WHITE;
		else Color = TEXT_COLOR_WHITE;

		sprintf ( TextList[TextNum], "%s", p->Name );
	}
#endif	// _PVP_MURDERER_HERO_ITEM
	else if(ip->Type == ITEM_POTION+12)//이밴트 아이템
	{
		switch(Level)
		{
		case 0:sprintf(TextList[TextNum],GlobalText[100]);break;
		case 1:sprintf(TextList[TextNum],GlobalText[101]);break;
		case 2:sprintf(TextList[TextNum],"%s",ChaosEventName[ip->Durability]);break;
		}
	}
    else if ( ip->Type==ITEM_HELPER+15 )//  서클. ( 에너지/체력/민첩/힘/통솔 )
    {
        Color = TEXT_COLOR_YELLOW;
        switch ( Level )
        {
        case 0:sprintf(TextList[TextNum],"%s %s", GlobalText[168], p->Name );break;
        case 1:sprintf(TextList[TextNum],"%s %s", GlobalText[169], p->Name );break;
        case 2:sprintf(TextList[TextNum],"%s %s", GlobalText[167], p->Name );break;
        case 3:sprintf(TextList[TextNum],"%s %s", GlobalText[166], p->Name );break;
		case 4:sprintf(TextList[TextNum],"%s %s", GlobalText[1900], p->Name );break;// 통솔열매
        }
    }
    else if ( ip->Type==ITEM_HELPER+14 )//  로크의 깃털.
    {
        Color = TEXT_COLOR_YELLOW;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;            //  로크의 깃털.
        case 1: sprintf ( TextList[TextNum], "%s", GlobalText[1235] ); break;   //  군주의 소매.
        }
    }
    else if ( ip->Type==ITEM_POTION+21 )    //  레나. ( 1:스톤).
    {
        Color = TEXT_COLOR_YELLOW;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], "%s", p->Name ); break;
        case 1: sprintf ( TextList[TextNum], GlobalText[810] ); break;      //  스톤.
#ifdef FRIEND_EVENT
        case 2: sprintf ( TextList[TextNum], GlobalText[1098] ); break;     //  우정의 돌.
#endif// FRIEND_EVENT
        case 3: sprintf ( TextList[TextNum], GlobalText[1290] ); break;     //  성주의표식
        }
    }
    else if ( ip->Type==ITEM_HELPER+19 )    //  0:대천사의 절대지팡이, 1: 대천사의 절대검, 2: 대천사의 절대석궁 ).
    {
        Color = TEXT_COLOR_YELLOW;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], GlobalText[811] ); break;  //  대천사의 절대 지팡이.
        case 1: sprintf ( TextList[TextNum], GlobalText[812] ); break;  //  대천사의 절대 검.
        case 2: sprintf ( TextList[TextNum], GlobalText[817] ); break;  //  대천사의 절대 석궁.
        }
    }
	else if(ip->Type == ITEM_HELPER+20)
	{
		Color = TEXT_COLOR_YELLOW;
		switch( Level )
		{
		case 0: sprintf( TextList[TextNum], p->Name ); break;	//. 마법사의 반지
		case 1: sprintf( TextList[TextNum], GlobalText[922] ); break;	//. 제왕의 반지
        case 2: sprintf( TextList[TextNum], GlobalText[928] ); break;	//. 전사의 반지
        case 3: sprintf ( TextList[TextNum], GlobalText[929] ); break;  //  영예의 반지.
		}
	}
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
	else if(ip->Type == ITEM_HELPER+107)		// 치명마법반지
	{
		Color = TEXT_COLOR_YELLOW;
		sprintf( TextList[TextNum], p->Name );
	}
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
    else if ( ip->Type==ITEM_POTION+7 ) //  스패셜 물약.//종훈물약
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1413] ); break;
        case 1: sprintf( TextList[TextNum], GlobalText[1414] ); break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+7 ) //  계약 문서
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1460] ); break;  //  활 용병.
        case 1: sprintf( TextList[TextNum], GlobalText[1461] ); break;  //  창 용병.
        }
    }
    else if ( ip->Type==ITEM_HELPER+11 )    //  주문서.
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1416] ); break;  //  가디언 주문서.
        case 1: sprintf( TextList[TextNum], GlobalText[1462] ); break;  //  라이프 스톤 설치.
        }
    }
	else if ( ip->Type==ITEM_POTION+9 ) //  술, 사랑의 올리브
	{
		switch(Level)
		{
		case 0:sprintf(TextList[TextNum],"%s",p->Name);break;
		case 1:sprintf(TextList[TextNum],GlobalText[108]);break;
		}
	}
    else if(ip->Type == ITEM_WING+11)//소환구슬
	{
		sprintf(TextList[TextNum],"%s %s",SkillAttribute[30+Level].Name,GlobalText[102]);
	}
    else if(ip->Type == ITEM_HELPER+10)//변신반지
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
    else if ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )  //  정령의 날개 ~ 어둠의 날개.
    {
		if(Level==0)
			sprintf(TextList[TextNum],"%s",p->Name);
		else
			sprintf(TextList[TextNum],"%s +%d",p->Name,Level);
    }
#ifdef ADD_ALICE_WINGS_1
	else if ((ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40) || (ip->Type>=ITEM_WING+42 && ip->Type<=ITEM_WING+43)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			|| (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)  //  폭풍의 날개 ~ 제왕의 망토, 소환술사 2,3차 날개
#else	// ADD_ALICE_WINGS_1
	else if ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )  //  폭풍의 날개 ~ 제왕의 망토
#endif	// ADD_ALICE_WINGS_1
    {
		if(Level==0)
			sprintf(TextList[TextNum],"%s",p->Name);
		else
			sprintf(TextList[TextNum],"%s +%d",p->Name,Level);
    }
    else if ( ip->Type==ITEM_HELPER+31 )    //  영혼.
    {
        switch ( Level )
        {
#if SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH
		case 0: sprintf ( TextList[TextNum], "%s of %s", p->Name, GlobalText[1187] ); break;
        case 1: sprintf ( TextList[TextNum], "%s of %s", p->Name, GlobalText[1214] ); break;
#else // SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH
		// 한국
        case 0: sprintf ( TextList[TextNum], "%s %s", GlobalText[1187], p->Name ); break;
        case 1: sprintf ( TextList[TextNum], "%s %s", GlobalText[1214], p->Name ); break;
#endif // SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES || SELECTED_LANGUAGE == LANGUAGE_ENGLISH
        }
    }
    else if ( ip->Type==ITEM_HELPER+30 )      //  군주의 망토.
    {
		if ( Level==0 )
			sprintf ( TextList[TextNum], "%s", p->Name );
		else
			sprintf ( TextList[TextNum], "%s +%d", p->Name, Level );
    }
    else if ( ip->Type==ITEM_POTION+29 )    //  쿤둔의 표식.
    {
        sprintf ( TextList[TextNum], GlobalText[1180], Level );
    }
    else if ( ip->Type==ITEM_POTION+28 )    //  잃어버린 지도.
    {
        Color = TEXT_COLOR_YELLOW;
        sprintf ( TextList[TextNum], "%s +%d", p->Name, Level );
    }
#ifdef MYSTERY_BEAD
	else if( ip->Type==ITEM_WING+26 )		//. 신비의구슬
	{
		Color = TEXT_COLOR_YELLOW;
		switch(Level)
		{
		case 0:		//. 신비의구슬
			sprintf( TextList[TextNum], "%s", p->Name); break;
		case 1:		//. 빨강수정
			sprintf( TextList[TextNum], "%s", GlobalText[1831]); break;
		case 2:		//. 파랑수정
			sprintf( TextList[TextNum], "%s", GlobalText[1832]); break;
		case 3:		//. 검은수정
			sprintf( TextList[TextNum], "%s", GlobalText[1833]); break;
		case 4:		//. 보물상자
			sprintf( TextList[TextNum], "%s", GlobalText[1834]); break;
		case 5:		//. 깜짝선물
			sprintf( TextList[TextNum], "%s", GlobalText[1838]); break;
		}
	}
#endif // MYSTERY_BEAD
	else if(ip->Type == ITEM_WING+32)//빨간리본상자.(x-mas이벤트용)
	{
		sprintf(TextList[TextNum], "%s", p->Name);	
	}
	else if(ip->Type == ITEM_WING+33)//초록리본상자.(x-mas이벤트용)
	{
		sprintf(TextList[TextNum], "%s", p->Name);	
	}
	else if(ip->Type == ITEM_WING+34)//초록리본상자.(x-mas이벤트용)
	{
		sprintf(TextList[TextNum], "%s", p->Name);	
	}
	else if(ip->Type == ITEM_WING+35)
	{
		sprintf(TextList[TextNum], "%s", p->Name);	
	}
	else if(ip->Type >= ITEM_POTION+45 && ip->Type <= ITEM_POTION+50)//할로윈데이 이벤트
	{
		sprintf ( TextList[TextNum], "%s", p->Name );
	}
#ifdef GIFT_BOX_EVENT
	else if(ip->Type == ITEM_POTION+32)//분홍 초콜릿상자.(발렌타인데이 이벤트용)
	{
		switch(Level)
		{
		case 0:		//. 신비의구슬
			sprintf( TextList[TextNum], "%s", p->Name); break;
		case 1:		//. 빨강수정
			sprintf( TextList[TextNum], "%s", GlobalText[2012]); break;
		}
	}
	else if(ip->Type == ITEM_POTION+33)//빨간 초콜릿상자.(발렌타인데이 이벤트용)
	{
		switch(Level)
		{
		case 0:		//. 신비의구슬
			sprintf( TextList[TextNum], "%s", p->Name); break;
		case 1:		//. 빨강수정
			sprintf( TextList[TextNum], "%s", GlobalText[2013]); break;
		}
	}
	else if(ip->Type == ITEM_POTION+34)//파란 초콜릿상자.(발렌타인데이 이벤트용)
	{
		switch(Level)
		{
		case 0:		//. 신비의구슬
			sprintf( TextList[TextNum], "%s", p->Name); break;
		case 1:		//. 빨강수정
			sprintf( TextList[TextNum], "%s", GlobalText[2014]); break;
		}
	}
#endif
	else if(ip->Type >= ITEM_HELPER+32 && ip->Type <= ITEM_HELPER+37)	
	{
		if(ip->Type == ITEM_HELPER+37)	// 펜릴의 뿔피리
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
	else if ( ip->Type==ITEM_SWORD+19 || ip->Type==ITEM_BOW+18 || ip->Type==ITEM_STAFF+10 || ip->Type==ITEM_MACE+13) //	대천사의 절대 무기.
	{
		if(Level==0)
			sprintf(TextList[TextNum],"%s",p->Name);
		else
			sprintf(TextList[TextNum],"%s +%d",p->Name,Level);
	}
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX	// 인벤토리 보석 묶음 이름 텍스트
	else if( nGemType != COMGEM::NOGEM && nGemType%2 == 1 )
	{
		int nGlobalIndex = COMGEM::GetJewelIndex( nGemType , COMGEM::eGEM_NAME );
		sprintf(TextList[TextNum],"%s +%d", GlobalText[nGlobalIndex], Level+1);
	}
#else // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	else if ( ip->Type==INDEX_COMPILED_CELE)
	{
		sprintf(TextList[TextNum],"%s +%d", GlobalText[1806], Level+1);
	}
	else if ( ip->Type==INDEX_COMPILED_SOUL)
	{
		sprintf(TextList[TextNum],"%s +%d", GlobalText[1807], Level+1);
	}
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	else if( ip->Type == ITEM_POTION + 41 || ip->Type == ITEM_POTION + 42 ||
			 ip->Type == ITEM_POTION + 43 || ip->Type == ITEM_POTION + 44 ||
			 ip->Type == ITEM_HELPER + 38
		    ) 
	{
		sprintf(TextList[TextNum],"%s",p->Name);
		Color = TEXT_COLOR_YELLOW;
	}
#ifdef ADD_SEED_SPHERE_ITEM
	else if ((ip->Type >= ITEM_WING+60 && ip->Type <= ITEM_WING+65)	// 시드
		|| (ip->Type >= ITEM_WING+70 && ip->Type <= ITEM_WING+74)	// 스피어
		|| (ip->Type >= ITEM_WING+100 && ip->Type <= ITEM_WING+129))	// 시드스피어
	{
		sprintf(TextList[TextNum],"%s",p->Name);
		Color = TEXT_COLOR_VIOLET;
	}
#endif	// ADD_SEED_SPHERE_ITEM
#ifdef YDG_ADD_DOPPELGANGER_ITEM
    else if ( ip->Type==ITEM_POTION+111 )    // 차원의마경
    {
        Color = TEXT_COLOR_YELLOW;
        sprintf ( TextList[TextNum], "%s", p->Name );
    }
#endif	// YDG_ADD_DOPPELGANGER_ITEM
#ifdef LDK_ADD_EMPIREGUARDIAN_ITEM
	else if( ITEM_POTION+101 <= ip->Type && ip->Type <= ITEM_POTION+109 )
	{
        Color = TEXT_COLOR_YELLOW;
        sprintf ( TextList[TextNum], "%s", p->Name );
	}
#endif //LDK_ADD_EMPIREGUARDIAN_ITEM
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

    if ( ip->Type==ITEM_HELPER+19)    // 0: 대천사의 절대지팡이, 1: 대천사의 절대검, 2: 대천사의 절대석궁 ).
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

        //  옵션.
        switch ( Level )
        {
        case 0: //  대천사의 절대지팡이.
    		sprintf ( TextList[TextNum], "%s: %d ~ %d", GlobalText[42], 107, 110 );  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;    //  공격력.
            iWeaponSpeed  = 20;
            iNeedStrength = 132;
            iNeedDex      = 32;
            break;  
        case 1: //  대천사의 절대검.
    		sprintf ( TextList[TextNum], "%s: %d ~ %d", GlobalText[40], 110, 120 );  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;    //  공격력.
            iWeaponSpeed  = 35;
            iNeedStrength = 381;
            iNeedDex      = 149;
            break;
        case 2: //  대천사의 절대석궁.
    		sprintf ( TextList[TextNum], "%s: %d ~ %d", GlobalText[41], 120, 140 );  TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;    //  공격력.
            iWeaponSpeed  = 35;
            iNeedStrength = 140;
            iNeedDex      = 350;
            break;  
        }

        //  공격 속도.
    	sprintf ( TextList[TextNum], GlobalText[64], iWeaponSpeed ); TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        //  요구힘.
        sprintf ( TextList[TextNum], GlobalText[73], iNeedStrength );TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        //  요구 민첩.
        sprintf ( TextList[TextNum], GlobalText[75], iNeedDex );     TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        sprintf ( TextList[TextNum], "\n" ); TextBold[TextNum] = false; TextNum++; SkipNum++;

        //  행운.
		sprintf ( TextList[TextNum], GlobalText[87] );     TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
		sprintf ( TextList[TextNum], GlobalText[94], 20 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;

        //  스킬, 액설런트.
        switch ( Level )
        {
        case 0: //  대천사의 절대지팡이.
            {
		        sprintf ( TextList[TextNum], GlobalText[79], 53 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = true; TextNum++;   //  마력 상승.
                sprintf ( TextList[TextNum], GlobalText[631] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //마력 증가(레벨)
		        sprintf ( TextList[TextNum], GlobalText[632], 2 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //마력 증가(퍼센트)
            }
            break;  
        case 1: //  대천사의 절대검.
            {
                GetSkillInformation ( AT_SKILL_SWORD4, 1, NULL, &iMana, NULL );
			    sprintf ( TextList[TextNum], GlobalText[84], iMana ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++; //  돌려치기 스킬.
                sprintf ( TextList[TextNum], GlobalText[629] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //공격력 증가(레벨)
                sprintf ( TextList[TextNum], GlobalText[630], 2 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //공격력 증가(2퍼센트)
            }
            break;
        case 2: //  대천사의 절대석궁.
            {
			    GetSkillInformation ( AT_SKILL_CROSSBOW, 1, NULL, &iMana, NULL );
			    sprintf ( TextList[TextNum], GlobalText[86], iMana ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++; //  다발스킬.
                sprintf ( TextList[TextNum], GlobalText[629] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //공격력 증가(레벨)
                sprintf ( TextList[TextNum], GlobalText[630], 2 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //공격력 증가(2퍼센트)
           }
            break;  
        }
        sprintf ( TextList[TextNum], GlobalText[628] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //액설런트 데미지 증가
		sprintf ( TextList[TextNum], GlobalText[633], 7 ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;//공격 속도
		sprintf ( TextList[TextNum], GlobalText[634] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //획득 생명력
		sprintf ( TextList[TextNum], GlobalText[635] ); TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;   //획득 마나

    }

    if (ip->Type>=ITEM_POTION+23 && ip->Type<=ITEM_POTION+26 )   //  퀘스트 아이템.
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
#ifdef CSK_FREE_TICKET
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
#endif // CSK_FREE_TICKET

#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
	// 자유입장권 아이템 설명
	else if(ip->Type >= ITEM_HELPER+125 && ip->Type <= ITEM_HELPER+127)
	{
		int iMap = 0;
		if(ip->Type == ITEM_HELPER+125)			// 도플갱어 자유입장권
			iMap = 3057;	// 3057 "도플갱어"	
		else if(ip->Type == ITEM_HELPER+126)	// 바르카 자유입장권
			iMap = 2806;	// 2806 "바르카"
		else if(ip->Type == ITEM_HELPER+127)	// 바르카 제7맵  자유입장권
			iMap = 3107;	// 3107 "바르카 제7맵"
		
		// 2259 "%s 입장시 정해진 횟수만큼 사용할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2259], GlobalText[iMap]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// 빈칸 추가
		sprintf(TextList[TextNum], "\n");
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH

#ifdef CSK_CHAOS_CARD
	// 카오스카드 아이템 설명
	else if(ip->Type == ITEM_POTION+54)
	{
		// 2261 "카오스카드 조합을 통해 특별한 아이템을 획득할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2261]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // CSK_CHAOS_CARD
#ifdef CSK_RARE_ITEM
	// 희귀아이템 아이템 설명
	else if(ip->Type >= ITEM_POTION+58 && ip->Type <= ITEM_POTION+62)
	{
		// 2269 "축하합니다. 운영팀에 연락한 후 아이템으로 교환할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2269]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // CSK_RARE_ITEM
#ifdef PSW_RARE_ITEM
	// 희귀아이템 아이템 설명
	else if(ip->Type == ITEM_POTION+83 )
	{
		// 2269 "축하합니다. 운영팀에 연락한 후 아이템으로 교환할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2269]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // PSW_RARE_ITEM
#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
	// 희귀아이템 아이템 설명
	else if(ip->Type >= ITEM_POTION+145 && ip->Type <= ITEM_POTION+150)
	{
		// 2269 "축하합니다. 운영팀에 연락한 후 아이템으로 교환할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2269]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
#ifdef CSK_LUCKY_CHARM
	else if( ip->Type == ITEM_POTION+53 )// 행운의 부적
	{
		sprintf(TextList[TextNum], GlobalText[2250]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //CSK_LUCKY_CHARM
#ifdef CSK_LUCKY_SEAL
	else if( ip->Type == ITEM_HELPER+43 )
	{
		sprintf(TextList[TextNum], GlobalText[2256]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
#ifdef LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA
	#if SELECTED_LANGUAGE != LANGUAGE_CHINESE
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	#endif //SELECTED_LANGUAGE != LANGUAGE_CHINESE
#else
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
#endif //LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA

		sprintf(TextList[TextNum], GlobalText[2567]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2568]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;

#endif // SELECTED_LANGUAGE != LANGUAGE_KOREAN
	}
	else if( ip->Type == ITEM_HELPER+44 )
	{
		sprintf(TextList[TextNum], GlobalText[2257]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
#ifdef LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA
	#if SELECTED_LANGUAGE != LANGUAGE_CHINESE
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	#endif //SELECTED_LANGUAGE != LANGUAGE_CHINESE
#else
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
#endif //LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA

		sprintf(TextList[TextNum], GlobalText[2567]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
		
		sprintf(TextList[TextNum], GlobalText[2568]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
#endif // SELECTED_LANGUAGE != LANGUAGE_KOREAN
	}
	else if( ip->Type == ITEM_HELPER+45 )
	{
		sprintf(TextList[TextNum], GlobalText[2258]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
#ifdef LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA
	#if SELECTED_LANGUAGE != LANGUAGE_CHINESE
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	#endif //SELECTED_LANGUAGE != LANGUAGE_CHINESE
#else
		sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
#endif //LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA

		sprintf(TextList[TextNum], GlobalText[2566]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
#endif // SELECTED_LANGUAGE != LANGUAGE_KOREAN
	}
#endif //CSK_LUCKY_SEAL	
#ifdef CSK_PCROOM_ITEM
	else if(ip->Type >= ITEM_POTION+55 && ip->Type <= ITEM_POTION+57)
	{
		// 571 "바닥에 던지면 돈이나 아이템이 나옴"	// 변경 요망.
		sprintf(TextList[TextNum],GlobalText[571]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // CSK_PCROOM_ITEM
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
	else if(ip->Type == ITEM_HELPER+96)		// 강함의 인장 (PC방 아이템, 일본 6차 컨텐츠)
	{
		sprintf(TextList[TextNum],GlobalText[2742]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
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
#endif //PSW_ELITE_ITEM
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
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
#endif //PSW_SCROLL_ITEM
#ifdef PSW_SEAL_ITEM               // 이동 인장
	else if(ip->Type == ITEM_HELPER+59)
	{
		sprintf(TextList[TextNum], GlobalText[2509]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //PSW_SEAL_ITEM
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
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
		
#ifdef LDS_ADD_NOTICEBOX_STATECOMMAND_ONLYUSEDARKLORD
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;
		sprintf(TextList[TextNum], GlobalText[1908]);	// 1908 "아이템 미착용시 사용가능"
		TextNum++;
		
		if(ip->Type == ITEM_HELPER+58)	// 통솔리셋열매이면?
		{
			if(GetBaseClass( Hero->Class ) == CLASS_DARK_LORD)
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
#endif // LDS_ADD_NOTICEBOX_STATECOMMAND_ONLYUSEDARKLORD
	}
#endif //PSW_FRUIT_ITEM
#ifdef PSW_SECRET_ITEM             // 강화의 비약
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

#ifdef ASG_ADD_LEAP_OF_CONTROL_TOOLTIP_TEXT
		if( ip->Type == ITEM_POTION+82 )
		{
			sprintf(TextList[TextNum], GlobalText[3115]);
			TextListColor[TextNum] = TEXT_COLOR_YELLOW;
			TextBold[TextNum] = false;
			TextNum++;
		}
#endif	// ASG_ADD_LEAP_OF_CONTROL_TOOLTIP_TEXT

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
#endif //PSW_SECRET_ITEM
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
	else if(ip->Type == ITEM_HELPER+60)
	{
		sprintf(TextList[TextNum], GlobalText[2519]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //PSW_INDULGENCE_ITEM

#ifdef PSW_ADD_PC4_SEALITEM
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

#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
	#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH
		sprintf(TextList[TextNum], GlobalText[2566]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	#else
#ifdef LDK_FIX_PC4_SEALITEM_TOOLTIP_BUG
		sprintf(TextList[TextNum], GlobalText[2567]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2568]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
#endif //LDK_FIX_PC4_SEALITEM_TOOLTIP_BUG
	#endif // SELECTED_LANGUAGE == LANGUAGE_ENGLISH
#endif	// SELECTED_LANGUAGE != LANGUAGE_KOREAN
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
		
#if SELECTED_LANGUAGE != LANGUAGE_KOREAN && SELECTED_LANGUAGE != LANGUAGE_ENGLISH
#ifdef LDK_FIX_PC4_SEALITEM_TOOLTIP_BUG
		sprintf(TextList[TextNum], GlobalText[2567]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2568]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
#endif //LDK_FIX_PC4_SEALITEM_TOOLTIP_BUG
#endif	// SELECTED_LANGUAGE != LANGUAGE_KOREAN
	}
#endif //PSW_ADD_PC4_SEALITEM
	
#ifdef PSW_ADD_PC4_SCROLLITEM
	else if( ip->Type == ITEM_POTION+97 )
	{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		
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
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		
		sprintf(TextList[TextNum], GlobalText[2581]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2502]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //PSW_ADD_PC4_SCROLLITEM

#ifdef YDG_ADD_HEALING_SCROLL
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
#endif	// YDG_ADD_HEALING_SCROLL

#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
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

#ifdef LDK_FIX_CHAOSCHARMITEM_TOOLTIP
		sprintf(TextList[TextNum], GlobalText[2708]);
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum++;
#endif //LDK_FIX_CHAOSCHARMITEM_TOOLTIP
	}
#endif //PSW_ADD_PC4_CHAOSCHARMITEM

#ifdef LDK_ADD_PC4_GUARDIAN
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
#endif //LDK_ADD_PC4_GUARDIAN

#ifdef LDK_ADD_RUDOLPH_PET
	else if( ip->Type == ITEM_HELPER+67 )
	{
		sprintf(TextList[TextNum], GlobalText[2600]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDK_ADD_RUDOLPH_PET

#ifdef YDG_ADD_SKELETON_PET
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
#endif	// YDG_ADD_SKELETON_PET

#ifdef PJH_ADD_PANDA_PET
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
#endif //PJH_ADD_PANDA_PET

#ifdef LDK_ADD_CS7_UNICORN_PET
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
#endif //LDK_ADD_CS7_UNICORN_PET
	
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
	else if(ip->Type == ITEM_HELPER+107)		// 치명마법반지
	{
		sprintf(TextList[TextNum],GlobalText[926]);	// 수리 불가능
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING

#ifdef YDG_ADD_CS7_MAX_AG_AURA
	else if(ip->Type == ITEM_HELPER+104)		// AG증가 오라
	{
		sprintf(TextList[TextNum], GlobalText[2968]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif	// YDG_ADD_CS7_MAX_AG_AURA
#ifdef YDG_ADD_CS7_MAX_SD_AURA
	else if(ip->Type == ITEM_HELPER+105)		// SD증가 오라
	{
		sprintf(TextList[TextNum], GlobalText[2969]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
	else if(ip->Type == ITEM_HELPER+103)		// 파티 경험치 증가 아이템
	{
// (일본만)파티 버프 사용시 추가로 얻는 경험치 변경(+190%, 한명 추가시 +10%)(09.11.19)
#ifdef LJH_MOD_JPN_PARTY_EXP_WITH_PARTY_BUFF
		sprintf(TextList[TextNum], GlobalText[2970]);
#else	//LJH_MOD_JPN_PARTY_EXP_WITH_PARTY_BUFF
		sprintf(TextList[TextNum], GlobalText[2970], 170);
#endif	//LJH_MOD_JPN_PARTY_EXP_WITH_PARTY_BUFF
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
	else if( ip->Type == ITEM_HELPER+69 )	// 부활의 부적
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
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
	else if( ip->Type == ITEM_HELPER+70 )	// 이동의 부적
	{
		sprintf(TextList[TextNum], GlobalText[2604]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef ASG_ADD_CS6_GUARD_CHARM
	else if (ip->Type == ITEM_HELPER+81)	// 수호의부적
	{
		::sprintf(TextList[TextNum], GlobalText[2714]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		::sprintf(TextList[TextNum], GlobalText[2729]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
#ifdef PBG_MOD_GUARDCHARMTEXT
		::sprintf(TextList[TextNum], GlobalText[3084]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
#endif //PBG_MOD_GUARDCHARMTEXT
	}
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
	else if (ip->Type == ITEM_HELPER+82)	// 아이템 보호 부적
	{
		::sprintf(TextList[TextNum], GlobalText[2715]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		::sprintf(TextList[TextNum], GlobalText[2730]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
		::sprintf(TextList[TextNum], GlobalText[2716]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
	}
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
	else if (ip->Type == ITEM_HELPER+93)	// 상승의인장마스터
	{
		::sprintf(TextList[TextNum], GlobalText[2256]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
#ifdef LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA
	#if SELECTED_LANGUAGE != LANGUAGE_CHINESE
		::sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
	#endif //SELECTED_LANGUAGE != LANGUAGE_CHINESE
#else
		::sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
#endif //LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA
	}
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
	else if (ip->Type == ITEM_HELPER+94)	// 풍요의인장마스터
	{
		::sprintf(TextList[TextNum], GlobalText[2257]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
#ifdef LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA
	#if SELECTED_LANGUAGE != LANGUAGE_CHINESE
		::sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
	#endif //SELECTED_LANGUAGE != LANGUAGE_CHINESE
#else
		::sprintf(TextList[TextNum], GlobalText[2297]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum++] = false;
#endif //LJW_FIX_CASHSHOPITEM_BUFF_CONTENT_IN_CHANA
	}
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER

#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
	// 자유입장권 아이템 설명
	else if(ip->Type == ITEM_HELPER+61)
	{
		// 2259 "%s 입장시 정해진 횟수만큼 사용할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2259], GlobalText[2369]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		// 2270 "입장 가능한 레벨에 맞추어 자동 입장됩니다."
		sprintf(TextList[TextNum], GlobalText[2270]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		// 빈칸 추가
		sprintf(TextList[TextNum], "\n");
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // PSW_CURSEDTEMPLE_FREE_TICKET
#ifdef PSW_CHARACTER_CARD 
	else if(ip->Type == ITEM_POTION+91) // 캐릭터 카드
	{
		// "소환술사 캐릭터를 생성할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2551]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // PSW_CHARACTER_CARD
#ifdef PSW_NEW_CHAOS_CARD
	else if(ip->Type == ITEM_POTION+92) // 카오스카드 골드
	{
		// 2261 "카오스카드 조합을 통해 특별한 아이템을 획득할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2261]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		// "일반, 고레벨용 아이템 생성"
		sprintf(TextList[TextNum], GlobalText[2553]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+93) // 카오스카드 레어
	{
		// 2261 "카오스카드 조합을 통해 특별한 아이템을 획득할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2261]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		// "이벤트용 아이템 생성"
		sprintf(TextList[TextNum], GlobalText[2556]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+95) // 카오스카드 미니
	{
		// 2261 "카오스카드 조합을 통해 특별한 아이템을 획득할 수 있습니다."
		sprintf(TextList[TextNum], GlobalText[2261]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// "초보용 아이템 생성"
		sprintf(TextList[TextNum], GlobalText[2552]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // PSW_NEW_CHAOS_CARD
#ifdef PSW_NEW_ELITE_ITEM
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
#endif //PSW_NEW_ELITE_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
	else if( ip->Type == ITEM_POTION+84 )  // 벚꽃상자
	{
		sprintf(TextList[TextNum], GlobalText[2011] );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if( ip->Type == ITEM_POTION+85 )  // 벚꽃술
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
	else if( ip->Type == ITEM_POTION+86 )  // 벚꽃경단
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
	else if( ip->Type == ITEM_POTION+87 )  // 벚꽃잎
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
	else if( ip->Type == ITEM_POTION+88 )  // 흰색 벚꽃
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
	else if( ip->Type == ITEM_POTION+89 )  // 붉은색 벚꽃
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
	else if( ip->Type == ITEM_POTION+90 )  // 노란색 벚꽃
	{
		sprintf(TextList[TextNum], GlobalText[69], ip->Durability );
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

		sprintf(TextList[TextNum], GlobalText[2534]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;

#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
		sprintf(TextList[TextNum], GlobalText[2564]);
		TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
		TextBold[TextNum] = false;
		TextNum++;
#else //SELECTED_LANGUAGE == LANGUAGE_KOREAN
		sprintf(TextList[TextNum], GlobalText[2537]);
		TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW;
		TextBold[TextNum] = false;
		TextNum++;
#endif SELECTED_LANGUAGE == LANGUAGE_KOREAN
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
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
	else if(ip->Type >= ITEM_POTION+65 && ip->Type <= ITEM_POTION+68)	// 데쓰빔나이트의 불꽃, 헬마이네의 뿔, 어둠의불사조의 깃털, 심연의눈동자
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
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
	else if(ip->Type >= ITEM_POTION+151 && ip->Type <= ITEM_POTION+156)
	{
		switch(ip->Type)
		{
		case ITEM_POTION+151:	// 1레벨 의뢰서
			sprintf(TextList[TextNum], GlobalText[3268]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
			sprintf(TextList[TextNum],GlobalText[3269]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
			sprintf(TextList[TextNum],GlobalText[733]);
			TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
			break;
		case ITEM_POTION+152:	// 1레벨 의뢰 완료 확인서
			sprintf(TextList[TextNum], GlobalText[3270]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
			sprintf(TextList[TextNum],GlobalText[733]);
			TextListColor[TextNum] = TEXT_COLOR_RED;
			TextBold[TextNum] = false;
			TextNum++;
			break;
		case ITEM_POTION+153:	// 스타더스트
			sprintf(TextList[TextNum], GlobalText[3271]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
			break;
		case ITEM_POTION+154:	// 칼트석
			sprintf(TextList[TextNum], GlobalText[3272]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
			break;
		case ITEM_POTION+155:	// 탄탈로스의 갑옷
			sprintf(TextList[TextNum], GlobalText[3273]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
			break;
		case ITEM_POTION+156:	// 잿더미 도살자의 몽둥이
			sprintf(TextList[TextNum], GlobalText[3274]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
			break;
		}

		sprintf(TextList[TextNum],GlobalText[731]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
		sprintf(TextList[TextNum],GlobalText[732]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
		
	}
	else if(ip->Type == ITEM_POTION+157)	// 초록빛 상자
	{
		sprintf(TextList[TextNum], GlobalText[3275]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+158)	// 붉은빛 상자
	{
		sprintf(TextList[TextNum], GlobalText[3276]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_POTION+159)	// 보라빛 상자
	{
		sprintf(TextList[TextNum], GlobalText[3277]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
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
#ifdef ADD_SEED_SPHERE_ITEM
	else if ((ip->Type >= ITEM_WING+60 && ip->Type <= ITEM_WING+65)	// 시드
		|| (ip->Type >= ITEM_WING+70 && ip->Type <= ITEM_WING+74)	// 스피어
		|| (ip->Type >= ITEM_WING+100 && ip->Type <= ITEM_WING+129))	// 시드스피어
	{
		TextNum = g_SocketItemMgr.AttachToolTipForSeedSphereItem(ip, TextNum);
	}
#endif	// ADD_SEED_SPHERE_ITEM
#ifdef LDK_ADD_GAMBLE_RANDOM_ICON
	else if( ip->Type == ITEM_HELPER+71 || ip->Type == ITEM_HELPER+72 || ip->Type == ITEM_HELPER+73 || ip->Type == ITEM_HELPER+74 ||ip->Type == ITEM_HELPER+75 )	// 겜블러 아이콘
	{
		sprintf(TextList[TextNum], GlobalText[2709]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDK_ADD_GAMBLE_RANDOM_ICON

	else if(ip->Type == ITEM_HELPER+38)
	{
		sprintf(TextList[TextNum],GlobalText[926]);	// 수리 불가능
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+20)		//. 제왕의 반지, 마법사의 반지
	{
        switch ( Level )
        {
        case 0:
            {
                sprintf(TextList[TextNum],GlobalText[926]);	// 수리 불가능
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
        case 3:     //  영예의 반지.
            {
                sprintf(TextList[TextNum],GlobalText[926]);	// 수리 불가능
			    TextListColor[TextNum] = TEXT_COLOR_RED;
			    TextBold[TextNum] = false;
			    TextNum++;
            }
            break;
        }
	}
#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING             // (정보출력)날개 조합 100% 성공 부적
	else if(ip->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN
			&& ip->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END)
	{
		const ITEM_ADD_OPTION& Item_data = g_pItemAddOptioninfo->GetItemAddOtioninfo(ip->Type);
#ifdef LDK_FIX_CHARM_MIX_ITEM_WING_TOOLTIP
		// 아이템이름이 아닌 내용으로 변경 및 빠진 내용 추가

		//"날개 조합 시 원하는 클래스용으로 생성될 수 있도록 해줍니다."
		sprintf(TextList[TextNum],GlobalText[2717]);
		TextBold[TextNum] = false;
		TextNum++;
		
		switch (ip->Type)
		{
		case ITEM_TYPE_CHARM_MIXWING+EWS_KNIGHT_1_CHARM: //사탄날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2718], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_MAGICIAN_1_CHARM: //천공날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2720], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_ELF_1_CHARM: //요정날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2722], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_SUMMONER_1_CHARM: //재앙날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2724], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_DARKLORD_1_CHARM: //망토조합부적
			{
				sprintf(TextList[TextNum], GlobalText[2727], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_KNIGHT_2_CHARM: //드라곤날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2719], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_MAGICIAN_2_CHARM: //영혼날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2721], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_ELF_2_CHARM: //정령날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2723], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_SUMMONER_2_CHARM: //절망날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2725], Item_data.m_byValue1);
			}break;
		case ITEM_TYPE_CHARM_MIXWING+EWS_DARKKNIGHT_2_CHARM: //암흑날개부적
			{
				sprintf(TextList[TextNum], GlobalText[2726], Item_data.m_byValue1);
			}break;
		}

#else //LDK_FIX_CHARM_MIX_ITEM_WING_TOOLTIP

#ifdef PBG_FIX_CHARM_MIX_ITEM_WING_TOOLTIP
		sprintf(TextList[TextNum], GlobalText[2732+(ip->Type-(ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN))],
			Item_data.m_byValue1);
#else //PBG_FIX_CHARM_MIX_ITEM_WING_TOOLTIP
		sprintf(TextList[TextNum], GlobalText[2732+(ip->Type-(MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN))],
			Item_data.m_byValue1);
#endif //PBG_FIX_CHARM_MIX_ITEM_WING_TOOLTIP

#endif //LDK_FIX_CHARM_MIX_ITEM_WING_TOOLTIP

		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDS_ADD_CS6_CHARM_MIX_ITEM_WING
#ifdef YDG_ADD_DOPPELGANGER_ITEM
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
#endif	// YDG_ADD_DOPPELGANGER_ITEM
#ifdef LDK_ADD_EMPIREGUARDIAN_ITEM
	else if( ITEM_POTION+101 <= ip->Type && ip->Type <= ITEM_POTION+109 )
	{
		switch(ip->Type)
		{
		case ITEM_POTION+101: // 의문의쪽지
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
		case ITEM_POTION+102: // 가이온의 명령서
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
			}break;
		case ITEM_POTION+103: // 세크로미콘 조각
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
		case ITEM_POTION+109: // 세크로미콘
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
#endif //LDK_ADD_EMPIREGUARDIAN_ITEM
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
	else if( ITEM_HELPER+109 == ip->Type )
	{
		// 생명 자동 회복 3%
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;

		// 마력 증가 4%
#ifdef LDS_MOD_MODIFYTEXT_TOPAZRING_SAPIRERING
		sprintf(TextList[TextNum], GlobalText[3058], 4);
#else // LDS_MOD_MODIFYTEXT_TOPAZRING_SAPIRERING
		sprintf(TextList[TextNum], GlobalText[632], 4);
#endif // LDS_MOD_MODIFYTEXT_TOPAZRING_SAPIRERING
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링		// MODEL_HELPER+110
	else if( ITEM_HELPER+110 == ip->Type )
	{
		// 생명 자동 회복 3%
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// 최대 생명력 증가 4%
		sprintf(TextList[TextNum], GlobalText[622], 4);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY	// 신규 루비(붉은색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링		// MODEL_HELPER+111
	else if( ITEM_HELPER+111 == ip->Type )
	{
		// 생명 자동 회복 3%
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// 젠 증가 50%
#ifdef LDS_MOD_MODIFYTEXT_TOPAZRING_SAPIRERING
		sprintf(TextList[TextNum], GlobalText[627], 50);
#else // LDS_MOD_MODIFYTEXT_TOPAZRING_SAPIRERING
		sprintf(TextList[TextNum], GlobalText[2744], 50);
#endif // LDS_MOD_MODIFYTEXT_TOPAZRING_SAPIRERING
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링		// MODEL_HELPER+112
	else if( ITEM_HELPER+112 == ip->Type )
	{
		// 생명 자동 회복 3%
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// 데미지 감소 4%
		sprintf(TextList[TextNum], GlobalText[624], 4);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY		// 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
	else if( ITEM_HELPER+113 == ip->Type )
	{
		// 생명 자동 회복 3%
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// 엑설런트 데미지 확률 +10%
		sprintf(TextList[TextNum], GlobalText[628], 10);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY		// 신규 루비(붉은색) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이	// MODEL_HELPER+114
	else if( ITEM_HELPER+114 == ip->Type )
	{
		// 생명 자동 회복 3%
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// 공격속도+7
		sprintf(TextList[TextNum], GlobalText[2229], 7);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE	// 신규 사파이어(녹색) 목걸이	// MODEL_HELPER+115
	else if( ITEM_HELPER+115 == ip->Type )
	{
		// 생명 자동 회복 3%
		sprintf(TextList[TextNum], GlobalText[92], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// 몬스터 죽였을때 획득 마나 증가 +마나/8
		sprintf(TextList[TextNum], GlobalText[635]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE	// 신규 사파이어(녹색) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(실버)	// MODEL_POTION+112
	else if( ITEM_POTION+112 == ip->Type )
	{
		// 봉인된금색상자와의...
		sprintf ( TextList[TextNum], GlobalText[2876] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(실버)	// MODEL_POTION+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD		// 신규 키(골드)	// MODEL_POTION+113
	else if( ITEM_POTION+113 == ip->Type )
	{
		// 봉인된금색상자와의...
		sprintf ( TextList[TextNum], GlobalText[2875] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYGOLD		// 신규 키(골드)	// MODEL_POTION+113
#ifdef LDK_ADD_INGAMESHOP_GOBLIN_GOLD
	// 고블린금화
	else if(ip->Type == ITEM_POTION+120)
	{
		sprintf ( TextList[TextNum], GlobalText[2971] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDK_ADD_INGAMESHOP_GOBLIN_GOLD
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
	else if( ITEM_POTION+121 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2877] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST		// 봉인된 은색상자
	else if( ITEM_POTION+122 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2878] );
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST				// 금색상자
	else if( ITEM_POTION+123 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2879] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST				// 은색상자
	else if( ITEM_POTION+124 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2880] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_PACKAGE_BOX		// 인게임샾 아이템 // 패키지상자6종			// MODEL_POTION+134~139
	else if( ITEM_POTION+134 <= ip->Type && ITEM_POTION+139 >= ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[2972] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDK_ADD_INGAMESHOP_PACKAGE_BOX		// 인게임샾 아이템 // 패키지상자6종			// MODEL_POTION+134~139
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
	else if( ITEM_HELPER+116 == ip->Type )
	{
		sprintf ( TextList[TextNum], GlobalText[3018] ); 
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
	else if( ITEM_HELPER+121 == ip->Type )
	{
		int iMap = 57;

		// 정해진 횟수만큼 사용할 수 있습니다.
		sprintf ( TextList[TextNum], GlobalText[2259], GlobalText[iMap] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
		
		// 레벨에 맞추어 자동 입장 됩니다.
		sprintf ( TextList[TextNum], GlobalText[2270] ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;

		// %d번 사용 가능
		sprintf ( TextList[TextNum], GlobalText[2260], ip->Durability ); 
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
#ifdef ASG_ADD_CHARGED_CHANNEL_TICKET
	// 유료채널 입장권 설명
	else if(ip->Type == ITEM_HELPER+124)
	{
		sprintf(TextList[TextNum], GlobalText[3116]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
#ifdef ASG_MOD_CHARGED_CHANNEL_TICKET_ADD_DESCRIPTION
		sprintf(TextList[TextNum], GlobalText[3127]);
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = true;
		TextNum++;
#endif	// ASG_MOD_CHARGED_CHANNEL_TICKET_ADD_DESCRIPTION
	}
#endif	// ASG_ADD_CHARGED_CHANNEL_TICKET
#ifdef PBG_ADD_GENSRANKING
	else if(ip->Type >= ITEM_POTION+141 && ip->Type <= ITEM_POTION+144)
	{
		sprintf(TextList[TextNum], GlobalText[571]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //PBG_ADD_GENSRANKING
#ifdef KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
	else if(ip->Type==ITEM_POTION+133)		// 엘리트 SD회복 물약
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
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
#endif // KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP


	Color = TEXT_COLOR_YELLOW;
	sprintf ( TextList[TextNum], "%s", p->Name );

	if(ip->Type == ITEM_POTION+19)//초대장
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

    //////////////////////////////////////////////////////////////////////////
    //  옵션.
    //////////////////////////////////////////////////////////////////////////
	if(ip->DamageMin)   //  공격력.
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
			// 일부 법서는 공격력이 없음
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
		{	// 공격력 없는 일부법서
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
	if(ip->Defense)     //  방어력.
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
	if(ip->MagicDefense)//  마법 방어력.
	{
    	sprintf(TextList[TextNum],GlobalText[66],ip->MagicDefense);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	if(p->SuccessfulBlocking)   //  방어 성공율.
	{
		sprintf(TextList[TextNum],GlobalText[67],ip->SuccessfulBlocking);
		if((ip->Option1&63) > 0)
      		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		else
      		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	if(p->WeaponSpeed)  //  공격 속도.
	{
    	sprintf(TextList[TextNum],GlobalText[64],p->WeaponSpeed);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
	if(p->WalkSpeed)    //  이동 속도.
	{
    	sprintf(TextList[TextNum],GlobalText[68],p->WalkSpeed);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#ifdef MYSTERY_BEAD
	if(ip->Type==ITEM_WING+26)
	{
		if(Level == 0)	//. 신비의 구슬
		{
			sprintf(TextList[TextNum],GlobalText[112]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[1835]);  
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
            sprintf(TextList[TextNum],GlobalText[114]);  
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
		else if (Level >= 1 && Level <= 3)	//. 수정들
		{
			sprintf(TextList[TextNum],GlobalText[1836]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[1837]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[732]);
		    TextListColor[TextNum] = TEXT_COLOR_RED;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[733]);
		    TextListColor[TextNum] = TEXT_COLOR_RED;TextBold[TextNum] = false;TextNum++;
		}
		else if (Level == 4)	//. 보물상자
		{
			sprintf(TextList[TextNum],GlobalText[571]);  
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[732]);
		    TextListColor[TextNum] = TEXT_COLOR_RED;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[733]);
		    TextListColor[TextNum] = TEXT_COLOR_RED;TextBold[TextNum] = false;TextNum++;
		}
		else if (Level == 5)	//. 깜짝선물
		{
			sprintf(TextList[TextNum],GlobalText[1839]);  
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[732]);
		    TextListColor[TextNum] = TEXT_COLOR_RED;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[733]);
		    TextListColor[TextNum] = TEXT_COLOR_RED;TextBold[TextNum] = false;TextNum++;
		}
	}
#endif // MYSTERY_BEAD
	if(ip->Type >= ITEM_WING+32 && ip->Type <= ITEM_WING+34)//리본상자.(x-mas이벤트용)
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
#ifdef GIFT_BOX_EVENT
	if(ip->Type >= ITEM_POTION+32 && ip->Type <= ITEM_POTION+34)//리본상자.(x-mas이벤트용)
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
#endif
	if(ip->Type==ITEM_POTION+11)//  행운의 상자.
	{
		if( Level==7 )//  천공의 상자.
		{
            sprintf(TextList[TextNum],GlobalText[112]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
            sprintf(TextList[TextNum],GlobalText[113]);  
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
            sprintf(TextList[TextNum],GlobalText[114]);  
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
#ifdef NEW_YEAR_BAG_EVENT
        else if ( Level==14 )   //  파란 복주머니
        {
            sprintf(TextList[TextNum],GlobalText[1652]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
            sprintf(TextList[TextNum],GlobalText[1653]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        }
#endif// NEW_YEAR_BAG_EVENT
        else
        {
            sprintf(TextList[TextNum],GlobalText[571]);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        }
		sprintf(TextList[TextNum],GlobalText[733]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
		if (Level == 13)	// 다크로드의 마음
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
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX	// 보석 조합 아이템 툴팁 추가
		int	nJewelIndex = COMGEM::Check_Jewel_Com(ip->Type);
		if( nJewelIndex != COMGEM::NOGEM )
		{
			sprintf(TextList[TextNum], GlobalText[1819], tCount, GlobalText[COMGEM::GetJewelIndex(nJewelIndex, COMGEM::eGEM_NAME)]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum], GlobalText[1820]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
#else // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
		switch(COMGEM::CheckOneItem(ip))
		{
			case COMGEM::COMCELE:
				{
					sprintf(TextList[TextNum], GlobalText[1819], tCount, GlobalText[1806]);
					TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
					sprintf(TextList[TextNum], GlobalText[1820]);
					TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
				}
				break;
			case COMGEM::COMSOUL:
				{
					sprintf(TextList[TextNum], GlobalText[1819], tCount, GlobalText[1807]);
					TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
					sprintf(TextList[TextNum], GlobalText[1820]);
					TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
				}
				break;
		}
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	}
	if(ip->Type==ITEM_POTION+13)//  축석
	{
		sprintf(TextList[TextNum],GlobalText[572]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+14)//  영석
	{
		sprintf(TextList[TextNum],GlobalText[573]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+16)//  생명
	{
		sprintf(TextList[TextNum],GlobalText[621]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+17 || ip->Type==ITEM_POTION+18)//악마의 눈, 열쇠
	{
		sprintf(TextList[TextNum],GlobalText[637]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
#ifdef ANNIVERSARY_EVENT
	if(ip->Type==ITEM_POTION+20 && Level >= 1 && Level <= 5)//애니버서리 박스
	{
		sprintf(TextList[TextNum],GlobalText[571]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
#endif	// ANNIVERSARY_EVENT
#ifdef _PVP_ADD_MOVE_SCROLL
	if(ip->Type==ITEM_POTION+10 && Level >= 1 && Level <= 8)//이동문서
	{
		sprintf(TextList[TextNum],GlobalText[157], 3);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
#endif	// _PVP_ADD_MOVE_SCROLL
#ifdef _PVP_MURDERER_HERO_ITEM
	if(ip->Type==ITEM_POTION+30)	//영웅살인마징표
	{
//		ITEM_EX_INFO * pItemExInfo = g_ItemExInfo.GetInfo(wInvenPos);
		if (pItemExInfo != NULL)	// 정보 이미 받았으면
		{
			TextNum--;
			sprintf(TextList[TextNum],"(%d %s %s)", pItemExInfo->m_wLevel_T, GlobalText[368], pItemExInfo->m_szClass_T);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
			sprintf(TextList[TextNum], pItemExInfo->m_szDate);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],"%d %s %s", pItemExInfo->m_wLevel, GlobalText[368], pItemExInfo->m_szClass);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = false;TextNum++;

			if (ip->Durability % 100 < 3)	// 영웅
			{
				sprintf(TextList[TextNum],"%s %s %s", GlobalText[490], pItemExInfo->m_szName, GlobalText[918]);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = false;TextNum++;
			}
			else	// 살인마
			{
				sprintf(TextList[TextNum],"%s %s %s", GlobalText[487], pItemExInfo->m_szName, GlobalText[918]);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = false;TextNum++;
			}
			sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
			sprintf(TextList[TextNum],GlobalText[1193], pItemExInfo->m_wRemainTime);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
		else	// 정보가 없으면
		{
			sprintf(TextList[TextNum],GlobalText[1190]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			sprintf(TextList[TextNum],GlobalText[1191]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		if (ip->Durability / 100 == 2)
		{
			sprintf(TextList[TextNum],GlobalText[1194]);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
		else
		{
			sprintf(TextList[TextNum],GlobalText[1195]);
			TextListColor[TextNum] = TEXT_COLOR_RED;TextBold[TextNum] = false;TextNum++;
		}
	}
#endif	// _PVP_MURDERER_HERO_ITEM
	if(ip->Type==ITEM_WING+15)  //  혼석.
	{
		sprintf(TextList[TextNum],GlobalText[574]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+22)  //  창조의 보석.
	{
		sprintf(TextList[TextNum],GlobalText[619]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_POTION+31)  //  수호의 보석.
	{
//		sprintf(TextList[TextNum],GlobalText[1288]);	// 1288 "장비의 드롭방지"
//		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[1289]);	// 1289 "공성을 위한 개체들의 생성과 강화"
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_HELPER+0) //  
	{
		sprintf(TextList[TextNum],GlobalText[578], 20);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[739], 50);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
	if(ip->Type==ITEM_HELPER+1) //  수호천사/악마.
	{
		sprintf(TextList[TextNum],GlobalText[576]);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
#ifdef ADD_ALICE_WINGS_1
	if((ip->Type>=ITEM_WING && ip->Type<=ITEM_WING+2) || ip->Type==ITEM_WING+41)//날개
#else	// ADD_ALICE_WINGS_1
    if(ip->Type>=ITEM_WING && ip->Type<=ITEM_WING+2)//날개
#endif	// ADD_ALICE_WINGS_1
	{
		sprintf(TextList[TextNum],GlobalText[577],12+Level*2);  //  데미지 몇%증가.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[578],12+Level*2);  //  데미지 몇%흡수.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[579]);             //  이동 속도 향상.
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
#ifdef LEM_ADD_LUCKYITEM		// 럭키아이템 보석 설명 텍스트 문구
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
#ifdef ADD_ALICE_WINGS_1
	else if ((ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6) || ip->Type==ITEM_WING+42) //날개
#else	// ADD_ALICE_WINGS_1
	else if ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 ) //날개
#endif	// ADD_ALICE_WINGS_1
    {
		sprintf(TextList[TextNum],GlobalText[577],32+Level);  //  데미지 몇%증가.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[578],25+Level*2);  //  데미지 몇%흡수.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[579]);             //  이동 속도 향상.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
    }
#ifdef ADD_ALICE_WINGS_1
	else if ((ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40) || ip->Type==ITEM_WING+43
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| ip->Type==ITEM_WING+50
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		) //3차날개
#else	// ADD_ALICE_WINGS_1
	else if ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 ) //3차날개
#endif	// ADD_ALICE_WINGS_1
    {
		sprintf(TextList[TextNum],GlobalText[577],39+Level*2);  //  데미지 몇%증가.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		if ( ip->Type==ITEM_WING+40 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
#ifndef PBG_MOD_NEWCHAR_MONK_WING		//군림의망토 수치 39로 변경
			|| ip->Type==ITEM_WING+50
#endif //PBG_MOD_NEWCHAR_MONK_WING
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			) // 제왕의망토
		{
			sprintf(TextList[TextNum],GlobalText[578],24+Level*2);  //  데미지 몇%흡수.
		}
		else	// 폭풍의날개~파멸의날개
		{
			sprintf(TextList[TextNum],GlobalText[578],39+Level*2);  //  데미지 몇%흡수.
		}
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[579]);             //  이동 속도 향상.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
    }
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
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
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		case ITEM_WING+135:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			{
				sprintf(TextList[TextNum],GlobalText[577],20+Level*2);  //  데미지 몇%증가.
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
				sprintf(TextList[TextNum],GlobalText[578],20+Level*2);  //  데미지 몇%흡수.
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			}break;
		case ITEM_WING+131:
		case ITEM_WING+132:
		case ITEM_WING+133:
		case ITEM_WING+134:
			{
				sprintf(TextList[TextNum],GlobalText[577],12+Level*2);  //  데미지 몇%증가.
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
				sprintf(TextList[TextNum],GlobalText[578],12+Level*2);  //  데미지 몇%흡수.
				TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
			}break;
		}
		sprintf(TextList[TextNum],GlobalText[579]);             //  이동 속도 향상.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
	else if ( ip->Type==ITEM_HELPER+3 )     //  디노란트.
    {
		sprintf(TextList[TextNum],GlobalText[577],15 );  //  데미지 몇%증가.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		sprintf(TextList[TextNum],GlobalText[578],10 );  //  데미지 몇%흡수.
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+31 )    //  영혼.
    {
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], GlobalText[1215] ); TextNum++; break; //  다크호스.
        case 1: sprintf ( TextList[TextNum], GlobalText[1216] ); TextNum++; break; //  다크스피릿.
        }
    }
    else if ( ip->Type==ITEM_HELPER+14 )    //  로크의 깃털.
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch ( Level )
        {
        case 0: sprintf ( TextList[TextNum], "%s", GlobalText[748] ); TextNum++; break;     //  로크의 깃털.
        case 1: sprintf ( TextList[TextNum], "%s", GlobalText[1236] );TextNum++; break;     //  군주의 소매.
        }
    }
	
    else if ( ip->Type==ITEM_HELPER+15 ) //  서클. ( 에너지/체력/민첩/힘/통솔 )
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
		sprintf(TextList[TextNum], GlobalText[1908]);	// 1908 "아이템 미착용시 사용가능"
		
		
		if(Level == 4)	// 통솔열매이면?
		{
			TextNum++;
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			sprintf(TextList[TextNum],GlobalText[61], GlobalText[24]);
		}
        TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+16 )    //  대천사의 서.
    {
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
        sprintf ( TextList[TextNum], GlobalText[816] );
        TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+17 )    //  블러드 본.
    {
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
        sprintf ( TextList[TextNum], GlobalText[816] );
        TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+18 )    //  투명망토.
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
    else if ( ip->Type==ITEM_POTION+7 ) //  스패셜 물약.//종훈물약
    {
        switch ( Level )
        {
        case 0:     //  축복의 물약.
            {
                sprintf( TextList[TextNum], GlobalText[1417] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
                sprintf( TextList[TextNum], GlobalText[1418] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
                sprintf( TextList[TextNum], GlobalText[1419] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            }
            break;
        case 1:     //  영혼의 물약.
            {
                sprintf( TextList[TextNum], GlobalText[1638] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
                sprintf( TextList[TextNum], GlobalText[1639] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
                sprintf( TextList[TextNum], GlobalText[1472] ); TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextBold[TextNum] = false; TextNum++;
            }
            break;
        }
    }
    else if ( ip->Type==ITEM_HELPER+7 ) //  계약 문서
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1460] ); break;  //  활 용병.
        case 1: sprintf( TextList[TextNum], GlobalText[1461] ); break;  //  창 용병.
        }
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+11 )    //  주문서.
    {
        switch ( Level )
        {
        case 0: sprintf( TextList[TextNum], GlobalText[1416] ); break;  //  가디언 주문서.
        case 1: sprintf( TextList[TextNum], GlobalText[1462] ); break;  //  라이프 스톤.
        }
		TextListColor[TextNum] = TEXT_COLOR_YELLOW;
		TextBold[TextNum] = false;
		TextNum++;
    }
    else if ( ip->Type==ITEM_HELPER+29 )    //  근위병의 갑옷세트.
    {
        //  해당 클래스에 맞는 제한 레벨 시작 위치를 찾는다.
        int startIndex = 0;
        if ( GetBaseClass( Hero->Class )==CLASS_DARK || GetBaseClass( Hero->Class )==CLASS_DARK_LORD 
#ifdef PBG_ADD_NEWCHAR_MONK
			|| GetBaseClass( Hero->Class )==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
			)
		{
            startIndex = 6;
		}

        int HeroLevel = CharacterAttribute->Level;

		TextListColor[TextNum] = TEXT_COLOR_WHITE;
		// 1147 "카오스 캐슬"
		// 368 "레벨"
		// 935 "레벨제한"
		// 936 "요구젠"
		sprintf(TextList[TextNum],"%s %s    %s      %s    ",GlobalText[1147], GlobalText[368], GlobalText[935], GlobalText[936] ); TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;

        for ( int i=0; i<6; i++ )
        {
            int Zen = g_iChaosCastleZen[i];

    		sprintf(TextList[TextNum],"        %d             %3d~%3d     %3d,000", i+1,  g_iChaosCastleLevel[startIndex+i][0],  min( 400, g_iChaosCastleLevel[startIndex+i][1] ),  Zen ); 
            if ( (HeroLevel>=g_iChaosCastleLevel[startIndex+i][0] && HeroLevel<=g_iChaosCastleLevel[startIndex+i][1])
				&& IsMasterLevel(Hero->Class) == false)
            {
                TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW; 
            }
            else
            {
                TextListColor[TextNum] = TEXT_COLOR_WHITE; 
            }
            TextBold[TextNum] = false; TextNum++;
        }
		// 737 "마스터레벨"
		sprintf(TextList[TextNum], "         %d          %s   %3d,000", 7, GlobalText[737], 1000); 
		if(IsMasterLevel(Hero->Class) == true)
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
		// 1157 "오른쪽 클릭으로 입장하세요."
    	sprintf ( TextList[TextNum], GlobalText[1157] );  
		TextListColor[TextNum] = TEXT_COLOR_DARKBLUE; 
		TextNum++;
    }
    else if ( ip->Type==ITEM_POTION+21 )    //  레나. ( 1:스톤).
    {
        TextListColor[TextNum] = TEXT_COLOR_WHITE;
        switch ( Level )
        {
#ifndef BLOODCASTLE_2ND_PATCH
        case 1: sprintf ( TextList[TextNum], GlobalText[813] ); break;      //  스톤.
#endif
#ifdef FRIEND_EVENT
        case 2: sprintf ( TextList[TextNum], GlobalText[1099] ); break;     //  우정의 돌.
#endif// FRIEND_EVENT
        case 3: sprintf ( TextList[TextNum], GlobalText[1291] ); break;     //  1291 "공성등록에 사용"
		default: break;
        }
        TextNum++;
    }
    else if ( ip->Type==ITEM_POTION+28 || ip->Type==ITEM_POTION+29 )    //  잃어버린 지도, 쿤둔의 표식
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
           (ip->Type>=ITEM_WING && ip->Type<=ITEM_WING+6)          //  날개 내구력.
#ifdef ADD_ALICE_WINGS_1
		|| (ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+43)          //  날개 내구력.
#else	// ADD_ALICE_WINGS_1
		|| (ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40)          //  날개 내구력.
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
       )
	{
		bDurExist = TRUE;
	}
	if( ( bDurExist || ip->Durability) && 
		( ip->Type<ITEM_HELPER+14 || ip->Type>ITEM_HELPER+19 )  //  블러드 캐슬 퀘스트 아이템은 내구력을 표시하지 않는다.
		&& !(ip->Type==ITEM_HELPER+20 && Level == 1)            //  제왕의 반지도 내구력 표시 않함.
		&& !(ip->Type==ITEM_HELPER+20 && Level == 2)            //  전사의 반지도 내구력 표시 않함.
		&& !(ip->Type==ITEM_HELPER+29 )                         //  카오스 캐슬 마스트 내구력 표시 않함.
        && ip->Type!=ITEM_POTION+7 && ip->Type!=ITEM_HELPER+7 && ip->Type!=ITEM_HELPER+11//종훈물약
		&& ip->Type != ITEM_HELPER+35	//^ 펜릴 아이템 갯수와 생명력 정보 렌더링 관련
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
		&& !(ip->Type >= ITEM_POTION+70 && ip->Type <= ITEM_POTION+71)
#endif //PSW_ELITE_ITEM            // 엘리트 물약
#ifdef PSW_FRUIT_ITEM
		&& !(ip->Type >= ITEM_HELPER+54 && ip->Type <= ITEM_HELPER+58) 
#endif //PSW_FRUIT_ITEM
#ifdef PSW_SECRET_ITEM             // 강화의 비약
		&& !(ip->Type >= ITEM_POTION+78 && ip->Type <= ITEM_POTION+82)
#endif //PSW_SECRET_ITEM             // 강화의 비약
#ifdef PBG_ADD_SANTAINVITATION
		&& !(ip->Type == ITEM_HELPER+66)	//산타마을의 초대장
#endif //PBG_ADD_SANTAINVITATION
#ifdef LDK_ADD_GAMBLE_RANDOM_ICON	// 겜블러 아이콘
		&& !( ip->Type == ITEM_HELPER+71 || ip->Type == ITEM_HELPER+72 || ip->Type == ITEM_HELPER+73 || ip->Type == ITEM_HELPER+74 || ip->Type == ITEM_HELPER+75 )
#endif //LDK_ADD_GAMBLE_RANDOM_ICON
#ifdef PBG_ADD_CHARACTERCARD
		&& !(ip->Type == ITEM_HELPER+97)	//마검사 캐릭터 카드
		&& !(ip->Type == ITEM_HELPER+98)	//다크로드 캐릭터 카드
		&& !(ip->Type == ITEM_POTION+91)	//소환술사 캐릭터 카드
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
		&& !(ip->Type == ITEM_HELPER+99)	//캐릭터 슬롯 열쇠
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PBG_ADD_SECRETITEM
		&& !(ip->Type >= ITEM_HELPER+117 && ip->Type <= ITEM_HELPER+120)	//활력의 비약(최하급/하급/중급/상급)
#endif //PBG_ADD_SECRETITEM
#ifdef KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP
		&& !(ip->Type==ITEM_POTION+133)
#endif // KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP
      )  //  내구력.
	{
		int Success = false;
        int arrow = false;
		if(ip->Type>=ITEM_POTION && ip->Type<=ITEM_POTION+8)    //  물약 갯수.
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type == ITEM_POTION+21 && Level == 3 )    //  성주의표식 갯수.
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
		else if(ip->Type==ITEM_BOW+7 || ip->Type==ITEM_BOW+15)  //  화살 갯수.
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
            arrow = true;
		}
		else if(ip->Type>=ITEM_POTION+35 && ip->Type<=ITEM_POTION+40)    //  물약 갯수.
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
#ifndef KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP			// #ifndef
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
		else if(ip->Type==ITEM_POTION+133)		// 엘리트 SD회복 물약
		{
          	sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
#endif // KJH_FIX_BTS251_ELITE_SD_POTION_TOOLTIP
		else if(ip->Type >= ITEM_POTION+46 && ip->Type <= ITEM_POTION+50)
		{
			sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		else if(ip->Type >= ITEM_POTION+153 && ip->Type <= ITEM_POTION+156)	// 스타더스트 ~ 잿더미 도살자의 몽둥이
		{
			sprintf(TextList[TextNum],GlobalText[69],ip->Durability);		// 69 "개수: %d"
			Success = true;
		}
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
		//^ 펜릴 갯수 표시
		else if(ip->Type >= ITEM_HELPER+32 && ip->Type <= ITEM_HELPER+33)	// 펜릴 아이템 갯수
		{
			sprintf(TextList[TextNum],GlobalText[1181],ip->Durability, 20);	// "%d개 / %d개"(현재갯수 / 최대)
			Success = true;
		}
		else if(ip->Type == ITEM_HELPER+34)	// 맹수의 발톱
		{
			sprintf(TextList[TextNum],GlobalText[1181],ip->Durability, 10);	// "%d개 / %d개"(현재갯수 / 최대)
			Success = true;
		}
		else if(ip->Type == ITEM_HELPER+37)	// 펜릴의 뿔피리
		{
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
			if( ip->bPeriodItem == false )
			{
				sprintf(TextList[TextNum],GlobalText[70],ip->Durability);	// "생명: %d"
				Success = true;
			}
#else // KJH_ADD_PERIOD_ITEM_SYSTEM
			sprintf(TextList[TextNum],GlobalText[70],ip->Durability);	// "생명: %d"
			Success = true;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
		}
		else if(ip->Type>=ITEM_HELPER && ip->Type<=ITEM_HELPER+7)   //  생명력.
		{
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
			if( ip->bPeriodItem == false )
			{
				sprintf(TextList[TextNum],GlobalText[70],ip->Durability);
				Success = true;
			}
#else // KJH_ADD_PERIOD_ITEM_SYSTEM
          	sprintf(TextList[TextNum],GlobalText[70],ip->Durability);
			Success = true;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
		}
        else if(ip->Type == ITEM_HELPER+10)                             //변신반지
		{
          	sprintf(TextList[TextNum],GlobalText[95],ip->Durability);
			Success = true;
		}
#ifdef LDK_FIX_GUARDIAN_CHANGE_LIFEWORD
		else if(ip->Type == ITEM_HELPER+64 || ip->Type == ITEM_HELPER+65)
		{
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
			if( ip->bPeriodItem == false )
			{
				sprintf(TextList[TextNum],GlobalText[70],ip->Durability);	// "생명: %d"
				Success = true;
			}
#else // KJH_ADD_PERIOD_ITEM_SYSTEM
			sprintf(TextList[TextNum],GlobalText[70],ip->Durability);	// "생명: %d"
			Success = true;
#endif // #ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
		}
#endif //LDK_FIX_GUARDIAN_CHANGE_LIFEWORD
#ifdef YDG_ADD_SKELETON_PET
		else if(ip->Type == ITEM_HELPER+67 || ip->Type == ITEM_HELPER+80
			|| ip->Type == ITEM_HELPER+106 ||ip->Type == ITEM_HELPER+123)
		{
#ifdef LJH_FIX_IGNORING_EXPIRATION_PERIOD
			sprintf(TextList[TextNum],GlobalText[70],ip->Durability);	// "생명: %d"
			Success = true;
#else  //LJH_FIX_IGNORING_EXPIRATION_PERIOD
			if( ip->bPeriodItem == false )
			{
				sprintf(TextList[TextNum],GlobalText[70],ip->Durability);	// "생명: %d"
				Success = true;
			}
#endif //LJH_FIX_IGNORING_EXPIRATION_PERIOD
		}
#endif	// YDG_ADD_SKELETON_PET
#ifdef CSK_FREE_TICKET
		// 자유입장권 아이템 사용횟수 설명
		else if(ip->Type >= ITEM_HELPER+46 && ip->Type <= ITEM_HELPER+48)
		{
			// 2260 "%d번 사용 가능"
			sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
			Success = true;
		}
#endif // CSK_FREE_TICKET
#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
		// 자유입장권 아이템 사용횟수 설명
		else if(ip->Type >= ITEM_HELPER+125 && ip->Type <= ITEM_HELPER+127)
		{
			// 2260 "%d번 사용 가능"
			sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
			
			if (ip->Type == ITEM_HELPER+126)
			{
				TextNum++;
				// 3105 "월-토요일맵에 입장이 가능합니다"
				sprintf(TextList[TextNum], GlobalText[3105]);
			
			}
			else if (ip->Type == ITEM_HELPER+127)
			{
				TextNum++;
				// 3106 "일요일맵에 입장이 가능합니다"
				sprintf(TextList[TextNum], GlobalText[3106]);
			}
			Success = true;
		}
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
#ifdef CSK_LUCKY_CHARM	
		else if( ip->Type == ITEM_POTION+53 )// 행운의 부적
		{
			// 2296 "%d% 조합 성공확률 상승"
			sprintf(TextList[TextNum], GlobalText[2296], ip->Durability);
			Success = true;
		}
#endif //CSK_LUCKY_CHARM
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
		// 자유입장권 아이템 사용횟수 설명
		else if(ip->Type == ITEM_HELPER+61)
		{
			// 2260 "%d번 사용 가능"
			sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
			Success = true;
		}
#endif //PSW_CURSEDTEMPLE_FREE_TICKET
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		//행운의 동전
		else if(ip->Type == ITEM_POTION+100)
		{
			//69 "개수: %d"
			sprintf(TextList[TextNum],GlobalText[69],ip->Durability);
			Success = true;
		}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef CSK_LUCKY_SEAL
		/*
		else if( ip->Type >= ITEM_HELPER+43 && ip->Type <= ITEM_HELPER+45 )
		{
		// 2252 "%s %d일 사용 가능"
		sprintf( TextList[TextNum], "%s %d%s %s", p->Name, ip->Durability, GlobalText[2298], GlobalText[2302] );
		Success = true;
		}
		*/
#endif //CSK_LUCKY_SEAL
#ifdef YDG_ADD_CS5_PORTAL_CHARM
		else if(ip->Type == ITEM_HELPER+70 )	// 이동의 부적
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
#endif	// YDG_ADD_CS5_PORTAL_CHARM

#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 내구도 티켓 제외
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

        //  내구력이 존재하는 아이템. (장비에 존재하는 아이템).
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
		else if ( (bDurExist) && (ip->bPeriodItem == false) )		// 기간제 아이템은 내구력이 존재하지 않는다.
#else // KJH_ADD_PERIOD_ITEM_SYSTEM
		else if ( bDurExist)
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
        {
            int maxDurability = calcMaxDurability ( ip, p, Level );

            sprintf(TextList[TextNum],GlobalText[71],ip->Durability, maxDurability);
			Success = true;
        }
#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING	// 정보출력
		else if( ip->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN 
				&& ip->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END ) // 날개 조합 100% 성공 부적
		{
			// 날개 조합 100% 성공 부적
			sprintf(TextList[TextNum], GlobalText[2732+(ip->Type-(MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN))]);
			
			Success = true;
		}
#endif //LDS_ADD_CS6_CHARM_MIX_ITEM_WING
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
		else if(ip->Type == ITEM_HELPER+121)		// 아이템 횟수
		{
			// 2260 "%d번 사용 가능"
			sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
			Success = true;
		}
#endif //LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121

		if(Success)
		{
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
		}
    }
    else
    {
        if(ip->Type == ITEM_HELPER+10)                             //변신반지
		{
          	sprintf(TextList[TextNum],GlobalText[95],ip->Durability);

            TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
		}
    }

    //  화살일 경우.
    if( ip->Type==ITEM_BOW+7 || ip->Type==ITEM_BOW+15)
	{
        if ( Level>=1 )
        {
            int value = Level*2+1;

            //  추가 공격력 2%증가.
          	sprintf(TextList[TextNum],GlobalText[577], value );
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;

            //  추가 공격력 1 증가.
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

	if(ip->RequireLevel && ip->Type!=ITEM_HELPER+14 )    //  요구 레벨. ( 로크의 깃털 )
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

#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 요구 스텟 표시 제거
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
#ifdef ADD_SOCKET_STATUS_BONUS
	if (ip->SocketCount > 0)
	{
		for (int i = 0; i < ip->SocketCount; ++i)
		{
			if (ip->SocketSeedID[i] == 38)	// 필요힘 감소
			{
				int iReqStrengthDown = g_SocketItemMgr.GetSocketOptionValue(ip, i);
				si_iNeedStrength += iReqStrengthDown;
			}
			else if (ip->SocketSeedID[i] == 39)	// 필요민 감소
			{
				int iReqDexterityDown = g_SocketItemMgr.GetSocketOptionValue(ip, i);
				si_iNeedDex += iReqDexterityDown;
			}
		}
	}
#endif	// ADD_SOCKET_STATUS_BONUS

	if(ip->RequireStrength && bRequireStat ) //  요구힘.
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
	if(ip->RequireDexterity && bRequireStat )//  요구 민첩.
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

	if(ip->RequireEnergy && bRequireStat )  //  요구 에너지.
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

	if(ip->RequireCharisma && bRequireStat ) //  요구통솔.
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
        RequireClass(p);    //  요구 클래스.
    }

    //  부츠.
	if(ip->Type>=MODEL_BOOTS-MODEL_ITEM && ip->Type<MODEL_BOOTS+MAX_ITEM_INDEX-MODEL_ITEM)
	{
        //  이동 속도 향상.
		if(Level >= 5)
		{
			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
			
			sprintf(TextList[TextNum],GlobalText[78]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = true;TextNum++;
		}
	}
    //  장갑.
	if(ip->Type>=MODEL_GLOVES-MODEL_ITEM && ip->Type<MODEL_GLOVES+MAX_ITEM_INDEX-MODEL_ITEM)
	{
        //  수영 속도 향상.
		if(Level >= 5)
		{
			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
			
			sprintf(TextList[TextNum],GlobalText[93]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = true;TextNum++;
		}
	}
    //  지팡이.
	if ( ( ip->Type>=MODEL_STAFF-MODEL_ITEM && ip->Type<MODEL_STAFF+MAX_ITEM_INDEX-MODEL_ITEM ) 
		 || ( ip->Type==(MODEL_SWORD+31-MODEL_ITEM) ) 
		 || ( ip->Type==(MODEL_SWORD+23-MODEL_ITEM) )
		 || ( ip->Type==(MODEL_SWORD+25-MODEL_ITEM) )
		 || ( ip->Type==(MODEL_SWORD+21-MODEL_ITEM) )
#ifdef KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
		 || ( ip->Type==(MODEL_SWORD+28-MODEL_ITEM) )
#endif // KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
	   )
	{
		sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

		// 소환술사용 책이면 저주력 아니면 마력.
		int nText = ITEM_STAFF+21 <= ip->Type && ip->Type <= ITEM_STAFF+29 ? 1691 : 79;
		::sprintf(TextList[TextNum], GlobalText[nText], ip->MagicPower);

		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = true;
		TextNum++;
	}
    //  셉터의 팻 공격력 상승 옵션 출력.
#ifdef CSK_FIX_EPSOLUTESEPTER

	if(IsCepterItem(ip->Type) == true)

#else // CSK_FIX_EPSOLUTESEPTER 

	if ( (ip->Type>=ITEM_MACE+8 && ip->Type<=ITEM_MACE+15) 
#ifdef KJH_FIX_ABSOLUTE_CEPTER_TOOLTIP
		|| (ip->Type == ITEM_MACE+17)					// 엡솔루트셉터
#endif // KJH_FIX_ABSOLUTE_CEPTER_TOOLTIP
		)
		
#endif // CSK_FIX_EPSOLUTESEPTER
    {
		sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

        //  팻 공격력 상승.
        sprintf(TextList[TextNum],GlobalText[1234],ip->MagicPower);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;TextBold[TextNum] = true;TextNum++;
    }

	if(ip->SpecialNum > 0)
	{
#ifdef LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR	// 인게임샾의 반지와 목걸이들은 모두 이미 하드코딩정의 되었습니다.
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int iModelType = ip->Type;
		int iStartModelType = ITEM_HELPER+109;
		int iEndModelType = ITEM_HELPER+115;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

		if(!( ITEM_HELPER+109 <= ip->Type  && ITEM_HELPER+115 >= ip->Type ))
		{
			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
		}
#else // LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR
		sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
#endif // LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR
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
#ifdef YDG_FIX_380ITEM_OPTION_TEXT_SD_PERCENT_MISSING
				strncpy(TextList[TextNum], Text380[i].c_str(), 100);
#else	// YDG_FIX_380ITEM_OPTION_TEXT_SD_PERCENT_MISSING
				sprintf(TextList[TextNum], (char*)Text380[i].c_str() );
#endif	// YDG_FIX_380ITEM_OPTION_TEXT_SD_PERCENT_MISSING
				TextListColor[TextNum] = TEXT_COLOR_REDPURPLE;TextBold[TextNum] = true;TextNum++;
			}

			sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;
		}
	}
#ifndef PBG_MOD_NEWCHAR_MONK_WING_2
#ifdef PBG_MOD_NEWCHAR_MONK_WING
	if(ip->Type==ITEM_WING+49)
	{
		sprintf(TextList[TextNum],GlobalText[578],15+Level);	// 데미지 몇%흡수
		TextListColor[TextNum] = TEXT_COLOR_BLUE;				// 옵션위치에 출력요함
		TextNum++;
	}
#endif //PBG_MOD_NEWCHAR_MONK_WING
#endif //PBG_MOD_NEWCHAR_MONK_WING_2
#ifdef ADD_SOCKET_ITEM
	if (g_SocketItemMgr.IsSocketItem(ip));	// 소켓아이템은 조화보석 옵션 출력 안함
	else
#endif	// ADD_SOCKET_ITEM
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

    //  세트 기본 옵션.
	TextNum = g_csItemOption.RenderDefaultOptionText( ip, TextNum );

    //  특수 기능.
	int iMana;
	for(int i=0;i<ip->SpecialNum;i++)
	{
#ifdef LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR
		if( ITEM_HELPER+109 <= ip->Type  && ITEM_HELPER+115 >= ip->Type )
		{
			break;
		}
#endif // LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR

		GetSkillInformation( ip->Special[i], 1, NULL, &iMana, NULL);
        GetSpecialOptionText ( ip->Type, TextList[TextNum], ip->Special[i], ip->SpecialValue[i], iMana );

       	TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;TextNum++;

        //  행운.
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
#ifdef KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
		// 마검사 검
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
#else // KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
		else if ( ip->Type==ITEM_SWORD+31 && ip->Special[i]==AT_IMPROVE_DAMAGE )
		{
			sprintf(TextList[TextNum],GlobalText[89],ip->SpecialValue[i]);
       		TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;TextNum++;
		}
		else if ( ip->Type==ITEM_SWORD+21 && ip->Special[i]==AT_IMPROVE_DAMAGE )
		{
			sprintf(TextList[TextNum],GlobalText[89],ip->SpecialValue[i]);
       		TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;TextNum++;
		}
		else if ( ip->Type==ITEM_SWORD+23 && ip->Special[i]==AT_IMPROVE_DAMAGE )
		{
			sprintf(TextList[TextNum],GlobalText[89],ip->SpecialValue[i]);
       		TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;TextNum++;
		}
		else if(ip->Type == ITEM_SWORD+25 && ip->Special[i]==AT_IMPROVE_DAMAGE)
		{
			sprintf(TextList[TextNum],GlobalText[89],ip->SpecialValue[i]);
       		TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;TextNum++;	
		}
		else if(ip->Type == ITEM_SWORD+28 && ip->Special[i]==AT_IMPROVE_DAMAGE)
		{
			sprintf(TextList[TextNum],GlobalText[89],ip->SpecialValue[i]);
       		TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;TextNum++;	
		}
#endif // KJH_FIX_RUNE_BASTARD_SWORD_TOOLTIP
	}

	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	//^ 펜릴 아이템 설명 관련
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
		if(ip->Option1 == 0x01)	// 최종 데미지 증가 옵션
		{
			sprintf(TextList[TextNum], GlobalText[1860], 10); // 1860 "최종 데미지 %d%% 증가"
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;

			sprintf(TextList[TextNum], GlobalText[579]); // 579 "이동 속도 향상"
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
		}
		else if(ip->Option1 == 0x02) // 최종 데미지 감소 옵션
		{
			sprintf(TextList[TextNum], GlobalText[1861], 10); // 1861 "최종 데미지 %d%% 흡수"
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;

			sprintf(TextList[TextNum], GlobalText[579]); // 579 "이동 속도 향상"
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
		}
		else if(ip->Option1 == 0x04)
		{
			WORD wLevel = CharacterAttribute->Level;
			//1867 "생명 %d 증가"
			//1868 "마나 %d 증가"
			//1869 "공격력 %d 증가"
			//1870 "마력 %d 증가"
			//1871 "환영사원에서 큰 공적을 세운 영웅들을 위해"
			//1872 "특별히 제작되었습니다."
	
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
		
		if(ip->Option1 == 0x00)	// 업그레이드 안한 펜릴의 뿔피리
		{
			sprintf(TextList[TextNum], GlobalText[1929]);
			TextListColor[TextNum] = TEXT_COLOR_YELLOW; 
			TextNum++;
		}
	}
#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
    else if(ip->Type == ITEM_HELPER+10)	// 일반 변신반지에 경고메시지 추가
	{
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
	// 엘리트 해골전사 변신반지 아이템 설명(방어력 증가와 생명력 증가)
	else if(ip->Type == ITEM_HELPER+39)
	{
		// 방어력 증가
		char strText[100];
		sprintf(strText, GlobalText[959], 10);	// 959 "방어력 증가 +%d"
		sprintf(TextList[TextNum], "%s%%", strText);
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;

		// 생명력
		WORD wlevel = CharacterAttribute->Level;
		sprintf(TextList[TextNum], GlobalText[2225], wlevel);	// 2225 "생명력 +%d"
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;

#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
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
	// 할로윈 이벤트 변신반지 아이템 설명
	else if(ip->Type == ITEM_HELPER+40)
	{
		sprintf(TextList[TextNum], "%s", GlobalText[2232]);	// 2232 "할로윈 축제를 즐기세요."
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 

#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
		TextNum++;	

#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
	}
	else if(ip->Type == ITEM_HELPER+41)	// 크리스마스 이벤트 변신반지 설명
	{
		// 추가 공격력 +20
		sprintf(TextList[TextNum], GlobalText[88], 20);	// 88 "추가 공격력 +%d"
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;

		// 추가 마력 +20
		sprintf(TextList[TextNum], GlobalText[89], 20);	// 89 "추가 마력 +%d"
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;

		// 빈칸
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], "%s", GlobalText[2248]);	// 2248 "행복한 크리스마스 보내세요."
		TextListColor[TextNum] = TEXT_COLOR_RED; 
		TextNum++;

#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
	}
#ifdef PJH_ADD_PANDA_CHANGERING
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

#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
	}
#endif //#ifdef PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	else if(ip->Type == ITEM_HELPER+122)	// 스켈레톤 변신반지
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

#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
	else if(ip->Type == ITEM_POTION+51)	// 크리스마스의별
	{
		sprintf(TextList[TextNum], "%s", GlobalText[2244]);	// 2244 "바닥에 던지면 폭죽이 터집니다."
		TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		TextNum++;
	}
	else if(ip->Type == ITEM_HELPER+42)	// GM 변신반지 설명
	{
		// 976 "얼음속성 스킬 공격력 증가 +%d"			 
		// 977 "독속성 스킬 공격력 증가 +%d"			 
		// 978 "번개속성 스킬 공격력 증가 +%d"			 
		// 979 "불속성 스킬 공격력 증가 +%d"			 
		// 980 "땅속성 스킬 공격력 증가 +%d"			 
		// 981 "바람속성 스킬 공격력 증가 +%d"			 
		// 982 "물속성 스킬 공격력 증가 +%d"

		for(int i=0; i<7; ++i)
		{
			sprintf(TextList[TextNum], GlobalText[976+i], 255);
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
			TextNum++;
		}

#ifdef YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
		sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;

		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
#endif	// YDG_MOD_CHANGE_RING_EQUIPMENT_LIMIT
	}
#ifdef PBG_ADD_SANTAINVITATION
	//산타마을의 초대장.
	else if(ip->Type == ITEM_HELPER+66)
	{
		//2260 "%d"번 사용 가능.
		TextNum--;
		sprintf(TextList[TextNum], GlobalText[2260], ip->Durability);
 		TextListColor[TextNum] = TEXT_COLOR_RED;
 		TextNum++;
 		
 		//2589 마우스 오른쪽 클릭 시 산타마을로 이동가능
 		sprintf(TextList[TextNum], "%s", GlobalText[2589]);
 		TextListColor[TextNum] = TEXT_COLOR_BLUE;
 		TextNum++;
	}
#endif //PBG_ADD_SANTAINVITATION
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	//행운의 동전
	else if(ip->Type == ITEM_POTION+100)
	{
		//1887 "NPC에게 등록하면 다양한 선물을 드립니다."
		sprintf(TextList[TextNum], GlobalText[1887], ip->Durability);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
 		TextNum++;
	}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_SKELETON_PET
	else if( ip->Type == ITEM_HELPER+123 )	// 스켈레톤 펫
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
#endif	// YDG_ADD_SKELETON_PET
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
	else if(g_pMyInventory->IsInvenItem(ip->Type))
	{
		TextNum--; SkipNum--;
		
		// 사용중인 경우 "(사용중)" 문구 출력
		if (ip->Durability == 254)
		{
			sprintf(TextList[TextNum], GlobalText[3143]);
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = false;
			TextNum++;
			sprintf(TextList[TextNum],"\n"); TextNum++; SkipNum++;
		}

		// 아이템 종류 출력
		switch (ip->Type)
		{
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
			// 우상 아이템
			case ITEM_HELPER+128:	// 매조각상
			case ITEM_HELPER+129:	// 양조각상
				sprintf(TextList[TextNum], GlobalText[3121]);
				break;

			// 유물 아이템
			case ITEM_HELPER+134:	// 편자
				sprintf(TextList[TextNum], GlobalText[3123]);
				break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
			// 참 아이템
			case ITEM_HELPER+130:	// 오크참
			case ITEM_HELPER+131:	// 메이플참
			case ITEM_HELPER+132:	// 골든오크참
			case ITEM_HELPER+133:	// 골든메이플참
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


		// 각각의 아이템 효과 출력
		switch (ip->Type)
		{
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
			case ITEM_HELPER+128:	// 매조각상
				sprintf(TextList[TextNum], GlobalText[965], 10);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+129:	// 양조각상
				sprintf(TextList[TextNum], GlobalText[967], 10);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+134:	// 편자
				sprintf(TextList[TextNum], GlobalText[3126], 20);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
			case ITEM_HELPER+130:	// 오크참
				sprintf(TextList[TextNum], GlobalText[3132], 50);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+131:	// 메이플참
				sprintf(TextList[TextNum], GlobalText[3134], 50);
				TextListColor[TextNum] = TEXT_COLOR_BLUE;
				TextBold[TextNum] = false;
				TextNum++;
				break;
			case ITEM_HELPER+132:	// 골든오크참
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

		// 인벤장착 아이템용 "인벤에서 오른쪽 마우스로 사용할수 있습니다" 문구 출력
		sprintf(TextList[TextNum], GlobalText[3124]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef PBG_MOD_SECRETITEM
	else if(ip->Type >= ITEM_HELPER+117 && ip->Type <= ITEM_HELPER+120)
	{
		TextNum--;		
		sprintf(TextList[TextNum], GlobalText[3142]);
 		TextListColor[TextNum] = TEXT_COLOR_WHITE;
 		TextNum++;

		char _str_dest[4][32] = {"6", "24", "7", "30"};
		char _str_src[2][32] ={0,};
		int _index =ip->Type - (ITEM_HELPER+117);
		int _index_src = (int)(_index/2);

		strcpy(_str_src[_index_src], GlobalText[2299-_index_src]);
		strcat(_str_dest[_index], _str_src[_index_src]);
		sprintf(TextList[TextNum], GlobalText[2533], _str_dest[_index]);
		TextListColor[TextNum] = TEXT_COLOR_BLUE;
		TextNum++;
	}
#endif //PBG_MOD_SECRETITEM
#ifdef PBG_ADD_NEWCHAR_MONK
    else if(ip->Type == ITEM_HELPER+68)
	{
		sprintf(TextList[TextNum], GlobalText[3088]);
		TextListColor[TextNum] = TEXT_COLOR_RED;
		TextBold[TextNum] = false;
		TextNum++;
	}
#endif //PBG_ADD_NEWCHAR_MONK
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
	if( ip->bPeriodItem == true )		// 기간제 아이템
	{
		if( ip->bExpiredPeriod == true )		// 기간만료
		{
#ifdef KJH_FIX_BTS260_PERIOD_ITEM_INFO_TOOLTIP
			sprintf(TextList[TextNum], GlobalText[3266]);
#else // KJH_FIX_BTS260_PERIOD_ITEM_INFO_TOOLTIP
			sprintf(TextList[TextNum], "만료아이템");
#endif // KJH_FIX_BTS260_PERIOD_ITEM_INFO_TOOLTIP
			TextListColor[TextNum] = TEXT_COLOR_RED; 	
		}
		else
		{
#ifdef KJH_FIX_BTS260_PERIOD_ITEM_INFO_TOOLTIP
			sprintf(TextList[TextNum], GlobalText[3265]);
#else // KJH_FIX_BTS260_PERIOD_ITEM_INFO_TOOLTIP
			sprintf(TextList[TextNum], "만료일");
#endif // KJH_FIX_BTS260_PERIOD_ITEM_INFO_TOOLTIP
			TextListColor[TextNum] = TEXT_COLOR_ORANGE; 	
			TextNum++;
			SkipNum++;

			sprintf(TextList[TextNum], "%d-%02d-%02d  %02d:%02d", ExpireTime->tm_year+1900, ExpireTime->tm_mon+1, 
					ExpireTime->tm_mday, ExpireTime->tm_hour, ExpireTime->tm_min);
			TextListColor[TextNum] = TEXT_COLOR_BLUE; 
		}

		TextNum++;
	}
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

#ifdef ASG_ADD_NEW_QUEST_SYSTEM
	if (!bItemTextListBoxUse)
	{
#endif	// ASG_ADD_NEW_QUEST_SYSTEM

		// 인벤토리에서 현재 아이템을 검색하여, 장착된 아이템인지 인벤토리에 있는아이템인지 여부를 적용.
#ifdef LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
		bool bThisisEquippedItem = false;
#ifdef PBG_FIX_SETITEMTOOLTIP
		// 세트 아이템 인벤외의 슬롯에서 착용한 아이템과 같은 옵션출력막기위함
		int _iEquipIndex = g_pMyInventory->GetPointedItemIndex();
		if((_iEquipIndex != -1) && (_iEquipIndex>=EQUIPMENT_WEAPON_RIGHT && _iEquipIndex<MAX_EQUIPMENT))
		{
			bThisisEquippedItem=false;
		}
		else
		{
			bThisisEquippedItem=true;
		}
#else //PBG_FIX_SETITEMTOOLTIP
		//ITEM *pFindItem = SEASON3B::CNewUIInventoryCtrl::FindItemByKey( ip->Key );	// 인벤토리 검색으로 착용 아이템인지 여부 결정.
		SEASON3B::CNewUIInventoryCtrl * pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
		ITEM *pFindItem = pNewInventoryCtrl->FindItemByKey( ip->Key );	// 인벤토리 검색으로 착용 아이템인지 여부 결정.
		(pFindItem==NULL)?bThisisEquippedItem=true:bThisisEquippedItem=false;
#endif //PBG_FIX_SETITEMTOOLTIP
		TextNum = g_csItemOption.RenderSetOptionListInItem ( ip, TextNum, bThisisEquippedItem );
#else // LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY
		TextNum = g_csItemOption.RenderSetOptionListInItem ( ip, TextNum );
#endif // LDS_FIX_SETITEM_OUTPUTOPTION_WHICH_LOCATED_INVENTORY

#ifdef SOCKET_SYSTEM
		TextNum = g_SocketItemMgr.AttachToolTipForSocketItem(ip, TextNum);
#endif	// SOCKET_SYSTEM

		SIZE TextSize = {0, 0};
		float fRateY	= g_fScreenRate_y;
		int	Height		= 0;
		int	EmptyLine	= 0;
		int TextLine	= 0;

		
#ifdef	LEM_FIX_ITEMTOOLTIP_POS	// 아이템 툴팁 위치 조정 [lem.2010.7.28]
		for(int i = 0; i < TextNum; ++i)
		{
			if(TextList[i][0] == '\0')		break;
			else if(TextList[i][0] == '\n')	++EmptyLine;
			else							++TextLine;
		}
		fRateY	= fRateY / 1.1f;
		g_pRenderText->SetFont(g_hFont);
#endif // LEM_FIX_ITEMTOOLTIP_POS
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
		Height = (TextLine * TextSize.cy + EmptyLine * TextSize.cy / 2.0f) / fRateY;
		
#ifdef CSK_FIX_ITEMTOOLTIP_POS
		int iScreenHeight	= 420;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int nInvenHeight	= p->Height*INVENTORY_SCALE;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

		sy += INVENTORY_SCALE;
		if( sy + Height > iScreenHeight)
		{
			sy += iScreenHeight - ( sy + Height );

		}	
		else if(sy + Height > iScreenHeight)
		{
			
		}

#ifdef ASG_ADD_NEW_QUEST_SYSTEM
	}	// if (!bItemTextListBoxUse) 끝.
#endif	// ASG_ADD_NEW_QUEST_SYSTEM
#else // CSK_FIX_ITEMTOOLTIP_POS
	if(sy-Height >= 0)
		sy -= Height;
	else
		sy += p->Height*INVENTORY_SCALE;
#endif // CSK_FIX_ITEMTOOLTIP_POS
	
	bool isrendertooltip = true;

#ifdef NEW_USER_INTERFACE_SHELL
	isrendertooltip = g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_PARTCHARGE_SHOP ) ? false : true;
#endif //NEW_USER_INTERFACE_SHELL

	if( isrendertooltip )
	{
#ifdef ASG_ADD_NEW_QUEST_SYSTEM
		if (bItemTextListBoxUse)
			RenderTipTextList(sx, sy, TextNum, 0, RT3_SORT_CENTER, STRP_BOTTOMCENTER);
		else
#endif	// ASG_ADD_NEW_QUEST_SYSTEM
			RenderTipTextList(sx,sy,TextNum,0);
	}
}

void RenderRepairInfo(int sx,int sy,ITEM *ip,bool Sell)
{
	// CSK수리금지
    //  장비 창에 넣을수 있는 아이템의 수리 정보만 보여준다.	
	//. item filtering

#ifdef LDK_FIX_USING_ISREPAIRBAN_FUNCTION
	// 있는 함수를 활용합시다...
	if(IsRepairBan(ip) == true)
	{
		return;
	}
#else //LDK_FIX_USING_ISREPAIRBAN_FUNCTION
	if(g_ChangeRingMgr->CheckRepair(ip->Type) == true)
	{
		return;
	}
#ifdef CSK_PCROOM_ITEM
	if(ip->Type >= ITEM_POTION+55 && ip->Type <= ITEM_POTION+57)
	{
		return;
	}
#endif // CSK_PCROOM_ITEM

#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
	if(ip->Type == ITEM_HELPER+96)		// 강함의 인장 (PC방 아이템, 일본 6차 컨텐츠)
	{
		return;
	}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
	
	if( ( ip->Type >= ITEM_HELPER && ip->Type <= ITEM_HELPER+4 ) || ip->Type == ITEM_HELPER+10 )    return;
	if( ip->Type == ITEM_BOW+7 || ip->Type == ITEM_BOW+15 || ip->Type >= ITEM_POTION )              return;
	if( ip->Type >= ITEM_WING+7 && ip->Type <= ITEM_WING+19 )	return;		//. 구슬 필터링
	if( (ip->Type >= ITEM_HELPER+14 && ip->Type <= ITEM_HELPER+19) || ip->Type==ITEM_POTION+21 )    return;
	if( ip->Type == ITEM_HELPER+20)   return;
	if(ip->Type == ITEM_HELPER+29)   return;
    if ( ip->Type==ITEM_HELPER+4 || ip->Type==ITEM_HELPER+5 )  return;
#ifdef DARK_WOLF
    if ( ip->Type==ITEM_HELPER+6 )    return;
#endif// DARK_WOLF
#ifdef MYSTERY_BEAD
	if ( ip->Type==ITEM_WING+26 )	return;
#endif // MYSTERY_BEAD
	if(ip->Type==ITEM_HELPER+7)	return;
	if(ip->Type==ITEM_HELPER+11) return;
	if(ip->Type >= ITEM_HELPER+32 && ip->Type <= ITEM_HELPER+37) return;	//^ 펜릴 아이템 수리 불가능
	if(ip->Type == ITEM_HELPER+38)
		return;
#ifdef CSK_LUCKY_SEAL
	if( ip->Type == ITEM_HELPER+43 || ip->Type == ITEM_HELPER+44 || ip->Type == ITEM_HELPER+45 )
	{
		return;
	}
#endif //CSK_LUCKY_SEAL	
#ifdef CSK_FREE_TICKET
	// 아이템 수리 안되게 예외 처리
	if(ip->Type >= ITEM_HELPER+46 && ip->Type <= ITEM_HELPER+48)
	{
		return;
	}
#endif // CSK_FREE_TICKET
	
#ifdef CSK_RARE_ITEM
	// 아이템 수리 안되게 예외 처리
	if(ip->Type >= ITEM_POTION+58 && ip->Type <= ITEM_POTION+62)
	{
		return;
	}
#endif // CSK_RARE_ITEM

#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
	// 희귀아이템티켓 7-12 수리 안되게 예외 처리
	if(ip->Type >= ITEM_POTION+145 && ip->Type <= ITEM_POTION+150)
	{
		return;
	}
#endif //LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12

#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
	// 아이템 수리 안되게 예외 처리
	if(ip->Type >= ITEM_HELPER+125 && ip->Type <= ITEM_HELPER+127)
	{
		return;
	}
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
	
#ifdef CSK_LUCKY_CHARM
	if( ip->Type == ITEM_POTION+53 )// 행운의 부적
	{
		return;
	}
#endif //CSK_LUCKY_CHARM
	
#ifdef CSK_LUCKY_SEAL
	if( ip->Type == ITEM_HELPER+43 || ip->Type == ITEM_HELPER+44 || ip->Type == ITEM_HELPER+45 )
	{
		return;
	}
#endif //CSK_LUCKY_SEAL
	
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
	if( ip->Type >= ITEM_POTION+70 && ip->Type <= ITEM_POTION+71 )
	{
		return;
	}
#endif //PSW_ELITE_ITEM
	
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
	if( ip->Type >= ITEM_POTION+72 && ip->Type <= ITEM_POTION+77 )
	{
		return;
	}
#endif //PSW_SCROLL_ITEM
	
#ifdef PSW_SEAL_ITEM               // 이동 인장
	if( ip->Type == ITEM_HELPER+59 )
	{
		return;
	}
#endif //PSW_SEAL_ITEM
	
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
	if( ip->Type >= ITEM_HELPER+54 && ip->Type <= ITEM_HELPER+58 )
	{
		return;
	}
#endif //PSW_FRUIT_ITEM
	
#ifdef PSW_SECRET_ITEM             // 강화의 비약
	if( ip->Type >= ITEM_POTION+78 && ip->Type <= ITEM_POTION+82 )
	{
		return;
	}
#endif //PSW_SECRET_ITEM
	
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
	if( ip->Type == ITEM_HELPER+60 )
	{
		return;
	}
#endif //PSW_INDULGENCE_ITEM
	
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
	if( ip->Type == ITEM_HELPER+61 )
	{
		return;
	}
#endif //PSW_CURSEDTEMPLE_FREE_TICKET

#ifdef PSW_ADD_PC4_SEALITEM
	if( ip->Type == ITEM_HELPER+62 ) {
		return;
	}
	if( ip->Type == ITEM_HELPER+63 ) {
		return;
	}
#endif //PSW_ADD_PC4_SEALITEM
	
#ifdef PSW_ADD_PC4_SCROLLITEM
	if( ip->Type == ITEM_POTION+97 ) {
		return;
	}
	if( ip->Type == ITEM_POTION+98 ) {
		return;
	}
#endif //PSW_ADD_PC4_SCROLLITEM

#ifdef YDG_ADD_HEALING_SCROLL
	if (ip->Type == ITEM_POTION+140)	// 치유의 스크롤
	{
		return;
	}
#endif	// YDG_ADD_HEALING_SCROLL

#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
	if( ip->Type == ITEM_POTION+96 ) {
		return;
	}
#endif //PSW_ADD_PC4_CHAOSCHARMITEM

#ifdef LDK_ADD_PC4_GUARDIAN
	if( ip->Type == ITEM_HELPER+64 || ip->Type == ITEM_HELPER+65 ) 
	{
		return;
	}
#endif //LDK_ADD_PC4_GUARDIAN	
#ifdef LDK_ADD_RUDOLPH_PET
	if( ip->Type == ITEM_HELPER+67 )
	{
		return;
	}
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
	if( ip->Type == ITEM_HELPER+80 )
	{
		return;
	}
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
	if( ip->Type == ITEM_HELPER+106 )
	{
		return;
	}
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
	if( ip->Type == ITEM_HELPER+123 )	// 스켈레톤 펫
	{
		return;
	}
#endif	// YDG_ADD_SKELETON_PET
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
	if( ip->Type == ITEM_HELPER+69 )	// 부활의 부적
	{
		return;
	}
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
	if( ip->Type == ITEM_HELPER+70 )	// 이동의 부적
	{
		return;
	}
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef PBG_ADD_SANTAINVITATION
	if( ip->Type == ITEM_HELPER+66 )	return;	//산타마을의 초대장
#endif //PBG_ADD_SANTAINVITATION
	if (ip->Type>=ITEM_HELPER+49 || ip->Type>=ITEM_HELPER+50 || ip->Type>=ITEM_HELPER+51)	// 피의 두루마리 등 수리 불가능
	{
		return;
	}
#endif //LDK_FIX_USING_ISREPAIRBAN_FUNCTION

#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING             // 날개 조합 100% 성공 부적
	if( ip->Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN 
		&& ip->Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END )
	{
		return;
	}
#endif //LDS_ADD_CS6_CHARM_MIX_ITEM_WING
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
	if(ip->Type == ITEM_HELPER+107)		// 치명마법반지
	{
		return;
	}
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
#ifdef YDG_ADD_CS7_MAX_AG_AURA
	if(ip->Type == ITEM_HELPER+104)		// AG증가 오라
	{
		return;
	}
#endif	// YDG_ADD_CS7_MAX_AG_AURA
#ifdef YDG_ADD_CS7_MAX_SD_AURA
	if(ip->Type == ITEM_HELPER+105)		// SD증가 오라
	{
		return;
	}
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
	if(ip->Type == ITEM_HELPER+103)		// 파티 경험치 증가 아이템
	{
		return;
	}
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
	if(ip->Type == ITEM_POTION+133)		// 엘리트 SD회복 물약
	{
		return;
	}
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
	if(ip->Type == MODEL_HELPER+109)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링		// MODEL_HELPER+110
	if(ip->Type == MODEL_HELPER+110)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_RINGRUBY
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링		// MODEL_HELPER+111
	if(ip->Type == MODEL_HELPER+111)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링		// MODEL_HELPER+112
	if(ip->Type == MODEL_HELPER+112)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY		// 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
	if(ip->Type == MODEL_HELPER+113)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이	// MODEL_HELPER+114
	if(ip->Type == MODEL_HELPER+114)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE	// 신규 사파이어(녹색) 목걸이	// MODEL_HELPER+115
	if(ip->Type == MODEL_HELPER+115)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(실버)	// MODEL_POTION+112
	if(ip->Type == MODEL_POTION+112)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(실버)	// MODEL_POTION+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD		// 신규 키(골드)	// MODEL_POTION+113
	if(ip->Type == MODEL_POTION+113)
	{
		return;
	}
#endif	// LDS_ADD_INGAMESHOP_ITEM_KEYGOLD		// 신규 키(골드)	// MODEL_POTION+113

#ifdef LDK_ADD_INGAMESHOP_GOBLIN_GOLD
	// 고블린금화
	if(ip->Type == ITEM_POTION+120)
	{
		return;
	}
#endif //LDK_ADD_INGAMESHOP_GOBLIN_GOLD
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
	if(ip->Type == ITEM_POTION+121)
	{
		return;
	}
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST		// 봉인된 은색상자
	if(ip->Type == ITEM_POTION+122)
	{
		return;
	}
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST				// 금색상자
	if( ITEM_POTION+123 == ip->Type )
	{
		return;
	}
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST				// 은색상자
	if( ITEM_POTION+124 == ip->Type )
	{
		return;
	}
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_PACKAGE_BOX				// 패키지 상자A-F
	if( ITEM_POTION+134 <= ip->Type && ip->Type <= ITEM_POTION+139)
	{
		return;
	}
#endif //LDK_ADD_INGAMESHOP_PACKAGE_BOX
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
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
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
	if( ITEM_POTION+114 <= ip->Type && ip->Type <= ITEM_POTION+119)
	{
		return;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
#ifdef LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
	if( ITEM_POTION+126 <= ip->Type && ip->Type <= ITEM_POTION+129)
	{
		return;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
#ifdef LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
	if( ITEM_POTION+130 <= ip->Type && ip->Type <= ITEM_POTION+132)
	{
		return;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
	if( ITEM_HELPER+121 == ip->Type )
	{
		return;
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121

	ITEM_ATTRIBUTE *p = &ItemAttribute[ip->Type];
	TextNum = 0;
	SkipNum = 0;
	for(int i=0;i<30;i++)
	{
		TextList[i][0] = NULL;
	}

	int Level = (ip->Level>>3)&15;
	int Color;

    //  아이템 색.
	if(ip->Type==ITEM_POTION+13 || ip->Type==ITEM_POTION+14 || ip->Type==ITEM_WING+15)  //  축복의 보석, 영혼의 보석, 생명의 보석.
	{
		Color = TEXT_COLOR_YELLOW;
	}
	else if(COMGEM::isCompiledGem(ip))
	{
		Color = TEXT_COLOR_YELLOW;
	}
    //  대천사의 절대무기.
    else if ( ip->Type==ITEM_STAFF+10 || ip->Type==ITEM_SWORD+19 || ip->Type==ITEM_BOW+18 || ip->Type==ITEM_MACE+13)
    {
        Color = TEXT_COLOR_PURPLE;
    }
	else if(ip->Type==ITEM_POTION+17 || ip->Type==ITEM_POTION+18 || ip->Type==ITEM_POTION+19)	// 데빌스퀘어 관련 아이템
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

    if ( (  ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )    //  정령의 날개 ~ 어둠의 날개.
         || ip->Type>=ITEM_HELPER+30                            //  군주의 망토.
		 || ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )// 폭풍의 날개 ~ 제왕의 망토
#ifdef ADD_ALICE_WINGS_1
		 || (ip->Type>=ITEM_WING+42 && ip->Type<=ITEM_WING+43)	// 소환술사 2,3차 날개.
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		 || (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)	// 레이지파이터날개
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

	//  수리 비용.
	if ( ip->Type<ITEM_POTION)
    {
        int maxDurability;

	    sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

        //  아이템의 수리 비용.
	    char Text[100];

        //  내구력이 떨어졌을때. 
        //  나중에는 그냥 계산으로 처리한다.
        maxDurability = calcMaxDurability ( ip, p, Level );
        if(ip->Durability<maxDurability)
        {
            RepairEnable = 2;
			
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
			int iGold = ItemValue(ip,2);
			if( iGold == -1 )
				return;
            ConvertRepairGold(iGold, ip->Durability, maxDurability, ip->Type, Text);
#else // KJH_FIX_DARKLOAD_PET_SYSTEM
            ConvertRepairGold(ItemValue(ip,2),ip->Durability, maxDurability, ip->Type, Text);
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
		    sprintf(TextList[TextNum],GlobalText[238],Text);

            TextListColor[TextNum] = Color;//TEXT_COLOR_RED;
        }
        else
        {
            RepairEnable = 1;

		    sprintf(TextList[TextNum],GlobalText[238], "0" );

            TextListColor[TextNum] = Color;//TEXT_COLOR_WHITE;
        }
	    TextBold[TextNum] = true;
	    TextNum++;

//        RepairEnable = 1;
    }
	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

    //  아이템의 이름.
    if(ip->Type == ITEM_WING+11)//소환구슬
	{
		sprintf(TextList[TextNum],"%s %s",SkillAttribute[30+Level].Name,GlobalText[102]);
	}
    else if(ip->Type == ITEM_HELPER+10)//변신반지
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
#ifdef KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
	// 다크로트 펫 예외처리
	// 툴팁에 레벨을 표시하지 않는다.
	else if( (ip->Type==ITEM_HELPER+4) || (ip->Type==ITEM_HELPER+5) )
	{
		sprintf(TextList[TextNum],"%s",p->Name);
	}
#endif // KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
    else if ( ( ip->Type>=ITEM_WING+3 && ip->Type<=ITEM_WING+6 )    //  정령의 날개 ~ 어둠의 날개.
             || ip->Type>=ITEM_HELPER+30                            //  군주의 망토.
			 || ( ip->Type>=ITEM_WING+36 && ip->Type<=ITEM_WING+40 )	// 폭풍의 날개 ~ 제왕의 망토
#ifdef ADD_ALICE_WINGS_1
		 || (ip->Type>=ITEM_WING+42 && ip->Type<=ITEM_WING+43)	// 소환술사 2,3차 날개.
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (ip->Type>=ITEM_WING+49 && ip->Type<=ITEM_WING+50)	// 레이지파이터 날개
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
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
		if( ip->bPeriodItem == false )
		{
			int maxDurability = calcMaxDurability ( ip, p, Level );
			
			sprintf(TextList[TextNum],GlobalText[71],ip->Durability, maxDurability);
			
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
			TextNum++;
		}
#else // KJH_ADD_PERIOD_ITEM_SYSTEM
        int maxDurability = calcMaxDurability ( ip, p, Level );

        sprintf(TextList[TextNum],GlobalText[71],ip->Durability, maxDurability);

        TextListColor[TextNum] = TEXT_COLOR_WHITE;
		TextBold[TextNum] = false;
		TextNum++;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
    }

	sprintf(TextList[TextNum],"\n");TextNum++;SkipNum++;

	SIZE TextSize = {0, 0};

#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	
	int Height = ((TextNum-SkipNum)*TextSize.cy+SkipNum*TextSize.cy/2)*480/WindowHeight;
	if(sy-Height >= 0)
		sy -= Height;
	else
		sy += p->Height*INVENTORY_SCALE;

	RenderTipTextList(sx,sy,TextNum,0);
}

//////////////////////////////////////////////////////////////////////////
//	공격력을 계산한다.
//////////////////////////////////////////////////////////////////////////
bool GetAttackDamage ( int* iMinDamage, int* iMaxDamage )
{
	int AttackDamageMin;
	int AttackDamageMax;


	ITEM *r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
	ITEM *l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
	if ( PickItem.Number>0 && SrcInventory==Inventory )
	{	
		// 아이템을 든 경우
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
#ifdef ADD_SOCKET_ITEM
	if( GetEquipedBowType( ) == BOWTYPE_CROSSBOW )
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
	if( (r->Type>=ITEM_BOW+8  && r->Type<ITEM_BOW+15)	||
		(r->Type>=ITEM_BOW+16 && r->Type<ITEM_BOW+17)	||
		(r->Type>=ITEM_BOW+18 && r->Type<ITEM_BOW+MAX_ITEM_INDEX)
	  )
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
	{
		AttackDamageMin = CharacterAttribute->AttackDamageMinRight;
		AttackDamageMax = CharacterAttribute->AttackDamageMaxRight;
	}
#ifdef ADD_SOCKET_ITEM
	else if( GetEquipedBowType( ) == BOWTYPE_BOW )
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
	else if((l->Type>=ITEM_BOW && l->Type<ITEM_BOW+7) 
			|| l->Type==ITEM_BOW+17 
			|| l->Type==ITEM_BOW+20
			|| l->Type == ITEM_BOW+21
			|| l->Type == ITEM_BOW+22
		)
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
	{
		AttackDamageMin = CharacterAttribute->AttackDamageMinLeft;
		AttackDamageMax = CharacterAttribute->AttackDamageMaxLeft;
	}
	else if(r->Type == -1)
	{
		AttackDamageMin = CharacterAttribute->AttackDamageMinLeft;
		AttackDamageMax = CharacterAttribute->AttackDamageMaxLeft;
	}
	else if(r->Type >= ITEM_STAFF && r->Type < ITEM_SHIELD)	//이혁재 - 데미지 계산 적용 빠진거 수정 지팡이류는 왼쪽(AttackDamageMinLeft)에 데미지 적용되어있음
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
	if ( GetBaseClass(Hero->Class)==CLASS_KNIGHT || GetBaseClass(Hero->Class)==CLASS_DARK )
	{
		if ( l->Type>=ITEM_SWORD && l->Type<ITEM_STAFF+MAX_ITEM_INDEX && r->Type>=ITEM_SWORD && r->Type<ITEM_STAFF+MAX_ITEM_INDEX )
		{
			Alpha = true;
			AttackDamageMin = ((CharacterAttribute->AttackDamageMinRight*55)/100+(CharacterAttribute->AttackDamageMinLeft*55)/100);
			AttackDamageMax = ((CharacterAttribute->AttackDamageMaxRight*55)/100+(CharacterAttribute->AttackDamageMaxLeft*55)/100);
		}
	}
    else if(GetBaseClass(Hero->Class) == CLASS_ELF )
    {
        if ( ( r->Type>=ITEM_BOW && r->Type<ITEM_BOW+MAX_ITEM_INDEX ) &&
             ( l->Type>=ITEM_BOW && l->Type<ITEM_BOW+MAX_ITEM_INDEX ) )
        {
            //  ARROW의 LEVEL이 1이상 이면은 공격력 증가. 
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



///////////////////////////////////////////////////////////////////////////////
// 스킬 정보창 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////

void RenderSkillInfo(int sx,int sy,int Type,int SkillNum, int iRenderPoint /*= STRP_NONE*/)
{
	char lpszName[256];
	int  iMinDamage, iMaxDamage;
    int  HeroClass = GetBaseClass ( Hero->Class );
	int  iMana, iDistance, iSkillMana;
    int  TextNum = 0;		// Text줄수
	int  SkipNum = 0;		// 빈Text줄수

#ifdef PET_SYSTEM
    //  팻 명령어 설명을 한다.
    if ( giPetManager::RenderPetCmdInfo( sx, sy, Type ) ) return;
#endif// PET_SYSTEM

	int  AttackDamageMin, AttackDamageMax;
	int  iSkillMinDamage, iSkillMaxDamage;

    int  SkillType = CharacterAttribute->Skill[Type];
    CharacterMachine->GetMagicSkillDamage( CharacterAttribute->Skill[Type], &iMinDamage, &iMaxDamage);
	CharacterMachine->GetSkillDamage( CharacterAttribute->Skill[Type], &iSkillMinDamage, &iSkillMaxDamage );
	
    //	캐릭터의 공격력을 구한다.
	GetAttackDamage ( &AttackDamageMin, &AttackDamageMax );	

    iSkillMinDamage += AttackDamageMin;
	iSkillMaxDamage += AttackDamageMax;
	GetSkillInformation( CharacterAttribute->Skill[Type], 1, lpszName, &iMana, &iDistance, &iSkillMana);
	
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

	WORD Dexterity;		// 민첩
	WORD Energy;		// 에너지

	//^ 펜릴 스킬 공격력
	WORD Strength;		// 힘
	WORD Vitality;		// 체력
    WORD Charisma;		// 통솔

	Dexterity= CharacterAttribute->Dexterity+ CharacterAttribute->AddDexterity;
	Energy	 = CharacterAttribute->Energy   + CharacterAttribute->AddEnergy;  

	//^ 펜릴 스킬 공격력
	Strength	=	CharacterAttribute->Strength+ CharacterAttribute->AddStrength;
	Vitality	=	CharacterAttribute->Vitality+ CharacterAttribute->AddVitality;
	Charisma	=	CharacterAttribute->Charisma+ CharacterAttribute->AddCharisma;

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
		skillattackpowerRate += rightinfo.SI_SP.SI_magicalpower;	// 마력 상승 (오른손에만 지팡이 들 수 있다)
	}
	if( leftinfo.SI_isSP )
	{
		skillattackpowerRate += leftinfo.SI_SP.SI_skillattackpower;
	}

	if (HeroClass==CLASS_WIZARD || HeroClass==CLASS_SUMMONER)
	{
        if ( CharacterAttribute->Skill[Type]==AT_SKILL_WIZARDDEFENSE || (AT_SKILL_SOUL_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_SOUL_UP+4))	// 소울바리어
        {
#ifdef KJH_FIX_WOPS_K29544_SOULBARRIER_UPGRADE_TOOLTIP
			int iDamageShield;
			// 소울바리어의 데미지흡수 수치는 투자한 마스터스킬 레벨에 따라 +5%씩 늘어난다.
			if( CharacterAttribute->Skill[Type]==AT_SKILL_WIZARDDEFENSE )
			{
				iDamageShield = (int)(10+(Dexterity/50.f)+(Energy/200.f));
			}
			else if((AT_SKILL_SOUL_UP <= CharacterAttribute->Skill[Type]) && (CharacterAttribute->Skill[Type] <= AT_SKILL_SOUL_UP+4))
			{
				iDamageShield = (int)(10+(Dexterity/50.f)+(Energy/200.f)) + ((CharacterAttribute->Skill[Type]-AT_SKILL_SOUL_UP+1)*5);		
			}
#else KJH_FIX_WOPS_K29544_SOULBARRIER_UPGRADE_TOOLTIP
            int iDamageShield = (int)(10+(Dexterity/50.f)+(Energy/200.f));
#endif // KJH_FIX_WOPS_K29544_SOULBARRIER_UPGRADE_TOOLTIP
            int iDeleteMana   = (int)(CharacterAttribute->ManaMax*0.02f);
            int iLimitTime    = (int)(60+(Energy/40.f));

		    sprintf(TextList[TextNum],GlobalText[578],iDamageShield);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;

		    sprintf(TextList[TextNum],GlobalText[880],iDeleteMana);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;

		    sprintf(TextList[TextNum],GlobalText[881],iLimitTime);
		    TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
        }
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
		// 예외처리 ( 마력 : %d ~ %d )
		// 마력증가 스킬은 마력을 렌더하지 않는다.
		else if( SkillType != AT_SKILL_SWELL_OF_MAGICPOWER 
#ifdef KJW_FIX_SLEEPUP_SKILL_INFO
			// 슬립 강화는 마력을 렌더하지 않는다.
			&& SkillType != AT_SKILL_ALICE_SLEEP
			&& !(AT_SKILL_ALICE_SLEEP_UP <= SkillType && SkillType <= AT_SKILL_ALICE_SLEEP_UP + 4 )
#endif // KJW_FIX_SLEEPUP_SKILL_INFO
			)
#else // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
        else
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
        {
#ifdef KWAK_FIX_CHARACTER_SKILL_RUNTIME_ERR
			int iSkill = (int)CharacterAttribute->Skill[Type];
			if (!(AT_SKILL_STUN <= iSkill && iSkill <= AT_SKILL_MANA)
				&& !(AT_SKILL_ALICE_THORNS <= iSkill && iSkill <= AT_SKILL_ALICE_ENERVATION)
				&& iSkill != AT_SKILL_TELEPORT
				&& iSkill != AT_SKILL_TELEPORT_B)
			{
#ifdef ASG_ADD_SUMMON_RARGLE
				if (AT_SKILL_SUMMON_EXPLOSION <= iSkill && iSkill <= AT_SKILL_SUMMON_POLLUTION)
#else	// ASG_ADD_SUMMON_RARGLE
				if (AT_SKILL_SUMMON_EXPLOSION <= iSkill && iSkill <= AT_SKILL_SUMMON_REQUIEM)
#endif	// ASG_ADD_SUMMON_RARGLE
				{
					CharacterMachine->GetCurseSkillDamage(iSkill, &iMinDamage, &iMaxDamage);
					sprintf(TextList[TextNum], GlobalText[1692], iMinDamage, iMaxDamage);
				}
#else // KWAK_FIX_CHARACTER_SKILL_RUNTIME_ERR
			BYTE bySkill = CharacterAttribute->Skill[Type];
			if (!(AT_SKILL_STUN <= bySkill && bySkill <= AT_SKILL_MANA)
				&& !(AT_SKILL_ALICE_THORNS <= bySkill && bySkill <= AT_SKILL_ALICE_ENERVATION)
				&& bySkill != AT_SKILL_TELEPORT
				&& bySkill != AT_SKILL_TELEPORT_B)
			{
#ifdef ASG_ADD_SUMMON_RARGLE
				if (AT_SKILL_SUMMON_EXPLOSION <= bySkill && bySkill <= AT_SKILL_SUMMON_POLLUTION)
#else	// ASG_ADD_SUMMON_RARGLE
				if (AT_SKILL_SUMMON_EXPLOSION <= bySkill && bySkill <= AT_SKILL_SUMMON_REQUIEM)
#endif	// ASG_ADD_SUMMON_RARGLE
				{
					CharacterMachine->GetCurseSkillDamage(bySkill, &iMinDamage, &iMaxDamage);
					sprintf(TextList[TextNum], GlobalText[1692], iMinDamage, iMaxDamage);
				}
#endif // KWAK_FIX_CHARACTER_SKILL_RUNTIME_ERR
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
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_DEF_POWER_UP:
	case AT_SKILL_DEF_POWER_UP+1:
	case AT_SKILL_DEF_POWER_UP+2:
	case AT_SKILL_DEF_POWER_UP+3:
	case AT_SKILL_DEF_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
        case AT_SKILL_DEFENSE :
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_ATT_POWER_UP:
	case AT_SKILL_ATT_POWER_UP+1:
	case AT_SKILL_ATT_POWER_UP+2:
	case AT_SKILL_ATT_POWER_UP+3:
	case AT_SKILL_ATT_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
        case AT_SKILL_ATTACK :
        case AT_SKILL_SUMMON :      // 고블린.
        case AT_SKILL_SUMMON+1 :    // 돌괴물.
        case AT_SKILL_SUMMON+2 :    // 암살자.
        case AT_SKILL_SUMMON+3 :    // 설인대장.
        case AT_SKILL_SUMMON+4 :    // 다크나이트.
        case AT_SKILL_SUMMON+5 :    // 발리.
        case AT_SKILL_SUMMON+6 :    // 솔져.
#ifdef ADD_ELF_SUMMON
		case AT_SKILL_SUMMON+7:		// 쉐도우나이트
#endif // ADD_ELF_SUMMON
        case AT_SKILL_IMPROVE_AG:
        case AT_SKILL_STUN:			//  배틀마스터 스킬.
        case AT_SKILL_REMOVAL_STUN:
        case AT_SKILL_MANA:
        case AT_SKILL_INVISIBLE:
		case AT_SKILL_REMOVAL_INVISIBLE:
        case AT_SKILL_REMOVAL_BUFF:
            break;
        case AT_SKILL_PARTY_TELEPORT:   //  파티원 소환.
        case AT_SKILL_ADD_CRITICAL:     //  크리티컬 데미지 확률 증가.
            break;
		case AT_SKILL_ASHAKE_UP:
		case AT_SKILL_ASHAKE_UP+1:
		case AT_SKILL_ASHAKE_UP+2:
		case AT_SKILL_ASHAKE_UP+3:
		case AT_SKILL_ASHAKE_UP+4:
        case AT_SKILL_DARK_HORSE:   //  다크호스.
		    sprintf ( TextList[TextNum], GlobalText[1237] );
		    TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
            break;
        case AT_SKILL_BRAND_OF_SKILL:
            break;
		case AT_SKILL_PLASMA_STORM_FENRIR:	//^ 펜릴 스킬 공격력
#ifdef PBG_FIX_SKILL_RECOVER_TOOLTIP
		case AT_SKILL_RECOVER:				// 회복스킬
#endif //PBG_FIX_SKILL_RECOVER_TOOLTIP
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

	//^ 펜릴 스킬 공격력
	if(CharacterAttribute->Skill[Type] == AT_SKILL_PLASMA_STORM_FENRIR)
	{
		int iSkillDamage;
		GetSkillInformation_Damage(AT_SKILL_PLASMA_STORM_FENRIR, &iSkillDamage);

		if(HeroClass == CLASS_KNIGHT || HeroClass == CLASS_DARK)	// 기사, 마검
		{
			iSkillMinDamage = (Strength/3)+(Dexterity/5)+(Vitality/5)+(Energy/7)+iSkillDamage;
		}
		else if(HeroClass == CLASS_WIZARD || HeroClass == CLASS_SUMMONER)	// 법사, 소환술사
		{
			iSkillMinDamage = (Strength/5)+(Dexterity/5)+(Vitality/7)+(Energy/3)+iSkillDamage;
		}
		else if(HeroClass == CLASS_ELF)	// 요정
		{
			iSkillMinDamage = (Strength/5)+(Dexterity/3)+(Vitality/7)+(Energy/5)+iSkillDamage;
		}
		else if(HeroClass == CLASS_DARK_LORD)	// 다크로드
		{
			iSkillMinDamage = (Strength/5)+(Dexterity/5)+(Vitality/7)+(Energy/3)+(Charisma/3)+iSkillDamage;
		}
#ifdef PBG_ADD_NEWCHAR_MONK
		else if(HeroClass == CLASS_RAGEFIGHTER)	//레이지파이터
		{
			iSkillMinDamage = (Strength/5)+(Dexterity/5)+(Vitality/3)+(Energy/7)+iSkillDamage;
		}
#endif //PBG_ADD_NEWCHAR_MONK
		iSkillMaxDamage = iSkillMinDamage + 30;
		
		sprintf(TextList[TextNum],GlobalText[879],iSkillMinDamage,iSkillMaxDamage + skillattackpowerRate);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}

    if(GetBaseClass(Hero->Class) == CLASS_ELF)
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
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_DEF_POWER_UP:
	case AT_SKILL_DEF_POWER_UP+1:
	case AT_SKILL_DEF_POWER_UP+2:
	case AT_SKILL_DEF_POWER_UP+3:
	case AT_SKILL_DEF_POWER_UP+4:
		{
			int Cal = Energy/8+2;
#ifdef YDG_FIX_MASTERLEVEL_ELF_ATTACK_TOOLTIP
			sprintf(TextList[TextNum],GlobalText[172],(Cal) + (int)((float)Cal/(float)((float)SkillAttribute[CharacterAttribute->Skill[Type]].Damage/(float)10)));
#else	// YDG_FIX_MASTERLEVEL_ELF_ATTACK_TOOLTIP
			sprintf(TextList[TextNum],GlobalText[172],(int)((float)Cal/(float)((float)SkillAttribute[CharacterAttribute->Skill[Type]].Damage/(float)10)));
#endif	// YDG_FIX_MASTERLEVEL_ELF_ATTACK_TOOLTIP
		}
		break;
#endif //PJH_SEASON4_MASTER_RANK4
		case AT_SKILL_DEFENSE:sprintf(TextList[TextNum],GlobalText[172],Energy/8+2);break;
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_ATT_POWER_UP:
	case AT_SKILL_ATT_POWER_UP+1:
	case AT_SKILL_ATT_POWER_UP+2:
	case AT_SKILL_ATT_POWER_UP+3:
	case AT_SKILL_ATT_POWER_UP+4:
		{
			int Cal = Energy/7+3;
#ifdef YDG_FIX_MASTERLEVEL_ELF_ATTACK_TOOLTIP
			sprintf(TextList[TextNum],GlobalText[173],(Cal) + (int)((float)Cal/(float)((float)SkillAttribute[CharacterAttribute->Skill[Type]].Damage/(float)10)));
#else	// YDG_FIX_MASTERLEVEL_ELF_ATTACK_TOOLTIP
			sprintf(TextList[TextNum],GlobalText[173],(int)((float)Cal/(float)((float)SkillAttribute[CharacterAttribute->Skill[Type]].Damage/(float)10)));
#endif	// YDG_FIX_MASTERLEVEL_ELF_ATTACK_TOOLTIP
		}
	break;
#endif //PJH_SEASON4_MASTER_RANK4
		case AT_SKILL_ATTACK :sprintf(TextList[TextNum],GlobalText[173],Energy/7+3);break;
#ifdef PBG_FIX_SKILL_RECOVER_TOOLTIP
		case AT_SKILL_RECOVER:
			{
				int Cal = Energy/4;
				sprintf(TextList[TextNum],GlobalText[1782], (int)((float)Cal+(float)CharacterAttribute->Level));
			}
		break;
#endif //PBG_FIX_SKILL_RECOVER_TOOLTIP
		default:Success = false;
		}
		if(Success)
		{
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
		}
	}

#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	// 예외처리 ( 사용 가능 거리: %d )
	// 마력증가스킬은 사용가능거리를 렌더하지 않는다.
	if( SkillType != AT_SKILL_SWELL_OF_MAGICPOWER )
	{
		if(iDistance)
		{
			sprintf(TextList[TextNum],GlobalText[174],iDistance);
			TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;//SkipNum++;
		}
	}
#else // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	if(iDistance)
	{
		sprintf(TextList[TextNum],GlobalText[174],iDistance);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;//SkipNum++;
	}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER

   	sprintf(TextList[TextNum],GlobalText[175],iMana);
	TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	if ( iSkillMana > 0)
	{
   		sprintf(TextList[TextNum],GlobalText[360],iSkillMana);
		TextListColor[TextNum] = TEXT_COLOR_WHITE;TextBold[TextNum] = false;TextNum++;
	}
    if ( GetBaseClass(Hero->Class) == CLASS_KNIGHT )
    {
        if ( CharacterAttribute->Skill[Type]==AT_SKILL_SPEAR )
        {
            sprintf(TextList[TextNum],GlobalText[96] );
		    TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
        }

		// 콤보스킬을 습득했고 레벨이 220이상이면
		if ( Hero->byExtensionSkill == 1 && CharacterAttribute->Level >= 220 )
		{
			if ( ( CharacterAttribute->Skill[Type] >= AT_SKILL_SWORD1 && CharacterAttribute->Skill[Type] <= AT_SKILL_SWORD5 ) 
				|| CharacterAttribute->Skill[Type]==AT_SKILL_WHEEL || CharacterAttribute->Skill[Type]==AT_SKILL_FURY_STRIKE 
				|| CharacterAttribute->Skill[Type]==AT_SKILL_ONETOONE 
#ifdef PJH_SEASON4_MASTER_RANK4
				|| (AT_SKILL_ANGER_SWORD_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_ANGER_SWORD_UP+4)
				|| (AT_SKILL_BLOW_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_BLOW_UP+4)
#endif	//PJH_SEASON4_MASTER_RANK4
				|| (AT_SKILL_TORNADO_SWORDA_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_TORNADO_SWORDA_UP+4)
				|| (AT_SKILL_TORNADO_SWORDB_UP <= CharacterAttribute->Skill[Type] && CharacterAttribute->Skill[Type] <= AT_SKILL_TORNADO_SWORDB_UP+4)
				   )
			{
				// 99 "콤보 가능"
				sprintf(TextList[TextNum], GlobalText[99] );
				TextListColor[TextNum] = TEXT_COLOR_DARKRED;
				TextBold[TextNum] = false;
				TextNum++;
			}
#ifdef CSK_FIX_SKILL_BLOWOFDESTRUCTION_COMBO
			else if(CharacterAttribute->Skill[Type] == AT_SKILL_BLOW_OF_DESTRUCTION)	// 파괴의 일격
			{
				// 2115 "콤보 가능(2단계만)"
				sprintf(TextList[TextNum], GlobalText[2115] );
				TextListColor[TextNum] = TEXT_COLOR_DARKRED;
				TextBold[TextNum] = false;
				TextNum++;
			}
#endif // CSK_FIX_SKILL_BLOWOFDESTRUCTION_COMBO)
		}
    }

    BYTE MasteryType = CharacterMachine->GetSkillMasteryType( CharacterAttribute->Skill[Type] );
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
		// 1480 "%s의 스킬을 받은후"
		sprintf ( TextList[TextNum], GlobalText[1480], SkillAttribute[BrandType].Name );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
		// 1481 "%d초동안 사용가능합니다"
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

    if ( GetBaseClass(Hero->Class)==CLASS_DARK_LORD )
    {
        if ( CharacterAttribute->Skill[Type]==AT_SKILL_PARTY_TELEPORT && PartyNumber<=0 )
        {
		    sprintf ( TextList[TextNum], GlobalText[1185] );
		    TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
        }
	}

	if(CharacterAttribute->Skill[Type] == AT_SKILL_PLASMA_STORM_FENRIR)	//^ 펜릴 스킬 관련
	{
		sprintf ( TextList[TextNum], GlobalText[1926] );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;
		sprintf ( TextList[TextNum], GlobalText[1927] );
		TextListColor[TextNum] = TEXT_COLOR_DARKRED;TextBold[TextNum] = false;TextNum++;	
	}

	//예외적인 로직은 여기로
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

#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
	if( SkillType == AT_SKILL_SWELL_OF_MAGICPOWER )
	{
		sprintf ( TextList[TextNum], GlobalText[2054] );		// 2054 : "최소 마력 20%상승"
		TextListColor[TextNum] = TEXT_COLOR_BLUE; TextBold[TextNum] = false; TextNum++;
	}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER

	SIZE TextSize = {0, 0};	
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	
	if( iRenderPoint == STRP_NONE )
	{
		int Height = ((TextNum - SkipNum) * TextSize.cy + SkipNum * TextSize.cy / 2) / g_fScreenRate_y;
		sy -= Height;	
	}
	
	RenderTipTextList(sx,sy,TextNum,0, RT3_SORT_CENTER, iRenderPoint);
}

///////////////////////////////////////////////////////////////////////////////
// 바닥에 떨어진 아이템 이름 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////

void RenderItemName(int i,OBJECT *o,int ItemLevel,int ItemOption,int ItemExtOption,bool Sort)
{
	char Name[80];

	int Level = (ItemLevel>>3)&15;
	
	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 255);

#ifdef PBG_FIX_ITEMNAMEINDEX
	////////////////////////////////////////////////////////////////////////
	// 아이템 이름 설정에 있어서 메모리 이외의 위치를 읽어들이는 버그가 있음
	// 방대한? 라인으로 수정하기가 곤란함, 코드 정리위함
	// item의 name중 특정 색에 대하여만 컬러값 설정하세요	nColorType
	// 폰트를 달리하고자 할경우	nFontType 설정하세요
	// 디폴트로 === 컬러 1.0f,1.0f,1.0f === 폰트	g_hFont
	// 특정 값이 아닐경우 설정하지 마세요^^;;
	////////////////////////////////////////////////////////////////////////		[10.09.07]
	int nColorType=0;
	int nFontType=0;
	int nGlobalTextIndex = 0;

	////////////////////////////////////////////////////////////////////////
	// 특정 컬러값사용하는 아이템 추가
	////////////////////////////////////////////////////////////////////////
	if(o->Type==MODEL_POTION+15				//금
		|| (o->Type==MODEL_POTION+13) || (o->Type==MODEL_POTION+14) || (o->Type==MODEL_POTION+16)
		|| (o->Type==MODEL_WING+15) || (o->Type==MODEL_POTION+22) || (o->Type==MODEL_POTION+31)
		|| (o->Type==MODEL_HELPER+14) || (o->Type==MODEL_POTION+41) || (o->Type==MODEL_POTION+42)
		|| (o->Type==MODEL_POTION+43) || (o->Type==MODEL_POTION+44)
		|| (o->Type==MODEL_COMPILED_CELE) || (o->Type==MODEL_COMPILED_SOUL)
		|| (o->Type==MODEL_POTION+17 || o->Type==MODEL_POTION+18 || o->Type==MODEL_POTION+19)
		|| (o->Type == MODEL_POTION+11 && Level==7 )
		|| ( o->Type==MODEL_HELPER+15 ) || ( o->Type==MODEL_HELPER+31 ) || ( o->Type==MODEL_EVENT+16 )
		|| (o->Type == MODEL_EVENT+5) || (o->Type == MODEL_EVENT+6 && Level == 13)
#ifdef MYSTERY_BEAD
		|| ( o->Type == MODEL_EVENT+19 ) || ( o->Type == MODEL_EVENT+20 ) || (o->Type == MODEL_POTION+11 && Level==4)
		|| (o->Type == MODEL_POTION+11 && Level==5)
#endif //MYSTERY_BEAD
#ifdef CSK_PCROOM_ITEM
		|| (o->Type == MODEL_POTION+55) || (o->Type == MODEL_POTION+56) || (o->Type == MODEL_POTION+57)
#endif // CSK_PCROOM_ITEM
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
		|| (o->Type == MODEL_HELPER+96)		// 강함의 인장 (PC방 아이템, 일본 6차 컨텐츠)
#endif //LDS_ADD_PCROOM_ITEM_JPN_6TH
		|| (o->Type == ITEM_HELPER+49) || (o->Type == ITEM_HELPER+50) || (o->Type == ITEM_HELPER+51) || (o->Type == MODEL_POTION+64)
		|| (o->Type>=MODEL_EVENT+11 && o->Type<=MODEL_EVENT+15)
#ifdef CHINA_MOON_CAKE
		|| (o->Type==MODEL_EVENT+17)
#endif// CHINA_MOON_CAKE
		|| (o->Type==MODEL_POTION+21 && Level == 3)
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		|| (o->Type == MODEL_POTION+100)
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef PBG_ADD_CHARACTERCARD
		|| (o->Type == MODEL_HELPER+97 || o->Type == MODEL_HELPER+98 || o->Type == MODEL_POTION+91)
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
		|| (o->Type == MODEL_HELPER+99)
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PBG_ADD_SECRETITEM
		|| (o->Type >= MODEL_HELPER+117 && o->Type <= MODEL_HELPER+120)// 활력의비약(최하급/하급/중급/상급)
#endif //PBG_ADD_SECRETITEM
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
		|| (o->Type == MODEL_HELPER+109)
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY			// 신규 루비(붉은색)링		// MODEL_HELPER+110
		|| (o->Type == MODEL_HELPER+110)
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링		// MODEL_HELPER+111
		|| (o->Type == MODEL_HELPER+111)
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링		// MODEL_HELPER+112
		|| (o->Type == MODEL_HELPER+112)
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST	// 신규 자수정(보라색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY		// 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
		|| (o->Type == MODEL_HELPER+113)
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY	// 신규 루비(붉은색) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이	// MODEL_HELPER+114
		|| (o->Type == MODEL_HELPER+114)
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE	// 신규 사파이어(녹색) 목걸이	// MODEL_HELPER+115
		|| (o->Type == MODEL_HELPER+115)
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE// 신규 사파이어(녹색) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER		// 신규 키(실버)	// MODEL_POTION+112
		|| (o->Type == MODEL_POTION+112)
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER		// 신규 키(실버)	// MODEL_POTION+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD			// 신규 키(골드)	// MODEL_POTION+113
		|| (o->Type == MODEL_POTION+113)
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYGOLD		// 신규 키(골드)	// MODEL_POTION+113
#ifdef LDK_ADD_EMPIREGUARDIAN_ITEM
		|| ( MODEL_POTION+101 <= o->Type && o->Type <= MODEL_POTION+109 )
#endif //LDK_ADD_EMPIREGUARDIAN_ITEM
#ifdef YDG_ADD_DOPPELGANGER_ITEM
		|| (o->Type == MODEL_POTION+111)	// 차원의 마경
#endif	// YDG_ADD_DOPPELGANGER_ITEM
#ifdef LDK_ADD_INGAMESHOP_GOBLIN_GOLD
		|| (o->Type == MODEL_POTION+120)	// 고블린금화
#endif //LDK_ADD_INGAMESHOP_GOBLIN_GOLD
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
		|| (o->Type == MODEL_POTION+121)
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST		// 봉인된 은색상자
		|| (o->Type == MODEL_POTION+122)
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST				// 금색상자
		|| (o->Type == MODEL_POTION+123 )
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST				// 은색상자
		|| (o->Type == MODEL_POTION+124 )
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_PACKAGE_BOX				// 패키지 상자A-F
		|| ( MODEL_POTION+134 <= o->Type && o->Type <= MODEL_POTION+139 )
#endif //LDK_ADD_INGAMESHOP_PACKAGE_BOX
#ifdef LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
		|| ( MODEL_POTION+114 <= o->Type && o->Type <= MODEL_POTION+119 )
#endif // LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
#ifdef LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
		|| ( MODEL_POTION+126 <= o->Type && o->Type <= MODEL_POTION+129 )
#endif // LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
#ifdef LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
		|| ( MODEL_POTION+130 <= o->Type && o->Type <= MODEL_POTION+132 )
#endif // LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
		|| ( MODEL_HELPER+121==o->Type )
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| (o->Type >= MODEL_POTION+157 && o->Type <= MODEL_POTION+159)
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| ( o->Type==MODEL_WING+25 ) || ( o->Type==MODEL_POTION+28 || o->Type==MODEL_POTION+29 )    //  군주의 표식.
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
		|| ( COMGEM::Check_Jewel_Com(o->Type, true) != COMGEM::NOGEM )
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
		)
	{
		nColorType = 1;			//	glColor3f(1.f,0.8f,0.1f);
	}
	else if(o->Type == MODEL_POTION+54
#ifdef CSK_FREE_TICKET
		|| o->Type==MODEL_HELPER+46 || o->Type==MODEL_HELPER+47 || o->Type==MODEL_HELPER+48 
#endif //CSK_FREE_TICKET
#ifdef CSK_RARE_ITEM
		|| (o->Type >= MODEL_POTION+58 && o->Type <= MODEL_POTION+62)	// 땅에 떨어진 아이템 이름(실제로는 땅에 떨어지지 않음, 테스트용)
#endif //CSK_RARE_ITEM
#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12	//희귀아이템티켓 7-12
		|| (o->Type >= MODEL_POTION+145 && o->Type <= MODEL_POTION+150)
#endif //LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
		|| (o->Type == MODEL_HELPER+125 || o->Type == MODEL_HELPER+126 || o->Type == MODEL_HELPER+127)	//도플갱어 자유입장권
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
#ifdef CSK_LUCKY_CHARM
		|| ( o->Type == MODEL_POTION+53 )// 행운의 부적
#endif //CSK_LUCKY_CHARM
#ifdef CSK_LUCKY_SEAL
		|| ( o->Type == MODEL_HELPER+43 || o->Type == MODEL_HELPER+44 || o->Type == MODEL_HELPER+45 )
#endif //CSK_LUCKY_SEAL
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
		|| (o->Type >= ITEM_POTION+70 && o->Type <= ITEM_POTION+71)
#endif //PSW_ELITE_ITEM
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
		|| (o->Type >= ITEM_POTION+72 && o->Type <= ITEM_POTION+77)
#endif //PSW_SCROLL_ITEM
#ifdef PSW_SEAL_ITEM               // 이동 인장
		|| (o->Type == ITEM_HELPER+59)
#endif //PSW_SEAL_ITEM
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
		|| ( o->Type >= ITEM_HELPER+54 && o->Type <= ITEM_HELPER+58)
#endif //PSW_FRUIT_ITEM
#ifdef PSW_SECRET_ITEM             // 강화의 비약
		|| (o->Type >= ITEM_POTION+78 && o->Type <= ITEM_POTION+82)
#endif //PSW_SECRET_ITEM
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
		|| (o->Type == ITEM_HELPER+60)
#endif //PSW_INDULGENCE_ITEM
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
		|| (o->Type == ITEM_HELPER+61)
#endif //PSW_CURSEDTEMPLE_FREE_TICKET
#ifdef PSW_RARE_ITEM
		|| (o->Type == MODEL_POTION+83)
#endif //PSW_RARE_ITEM
#ifdef CSK_LUCKY_SEAL
		|| ( o->Type == MODEL_HELPER+43 || o->Type == MODEL_HELPER+44 || o->Type == MODEL_HELPER+45 )
#endif //CSK_LUCKY_SEAL
#ifdef PSW_CHARACTER_CARD 
		|| (o->Type == MODEL_POTION+91) // 캐릭터 카드
#endif //PSW_CHARACTER_CARD
#ifdef PSW_NEW_CHAOS_CARD
		|| (o->Type == MODEL_POTION+92 || o->Type == MODEL_POTION+93 || o->Type == MODEL_POTION+95) // 카오스카드 골드
#endif //PSW_NEW_CHAOS_CARD
#ifdef PSW_NEW_ELITE_ITEM
		|| (o->Type == ITEM_POTION+94) // 엘리트 중간 치료 물약
#endif //PSW_NEW_ELITE_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
		|| ( o->Type>=MODEL_POTION+84 && o->Type<=MODEL_POTION+90)  // 벚꽃상자
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef PSW_ADD_PC4_SEALITEM
		|| ( o->Type == MODEL_HELPER+62 || o->Type == MODEL_HELPER+63)
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SCROLLITEM
		|| ( o->Type == MODEL_POTION+97 ) || ( o->Type == MODEL_POTION+98 )
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
		|| ( o->Type == MODEL_POTION+96 ) 
#endif //PSW_ADD_PC4_CHAOSCHARMITEM
#ifdef LDK_ADD_PC4_GUARDIAN
		|| ( o->Type == MODEL_HELPER+64 ) || ( o->Type == MODEL_HELPER+65 )
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDK_ADD_RUDOLPH_PET
		|| ( o->Type == MODEL_HELPER+67 )
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
		|| ( o->Type == MODEL_HELPER+80 )
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
		|| ( o->Type == MODEL_HELPER+106 )
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
		|| ( o->Type == MODEL_HELPER+123 )	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifdef LDK_ADD_SNOWMAN_CHANGERING
		|| ( o->Type == MODEL_HELPER+68 )
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
		|| ( o->Type == MODEL_HELPER+76 )
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| ( o->Type == MODEL_HELPER+122 )	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM	// 매조각상, 양조각상, 편자
		|| ( o->Type == MODEL_HELPER+128 ) || ( o->Type == MODEL_HELPER+129 ) || ( o->Type == MODEL_HELPER+134 )
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2	// 오크참, 메이플참, 골든오크참, 골든메이플참
		|| ( o->Type >= MODEL_HELPER+130 && o->Type <= MODEL_HELPER+133)
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
		|| (o->Type >= MODEL_POTION+45 && o->Type <= MODEL_POTION+50)//(발렌타인데이 이벤트용)
#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING // (색감) 날개 조합 100% 성공 부적 
		|| (o->Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN && o->Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END)
#endif //LDS_ADD_CS6_CHARM_MIX_ITEM_WING
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		|| ( o->Type == MODEL_HELPER+116 )
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
		|| ( ITEM_WING+130 <= o->Type && 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		o->Type <= ITEM_WING+135
#else //PBG_ADD_NEWCHAR_MONK_ITEM
		o->Type <= ITEM_WING+134 
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_GENSRANKING
		|| (o->Type>=MODEL_POTION+141 && o->Type<=MODEL_POTION+144)		// 보석함
#endif //PBG_ADD_GENSRANKING
		)
	{
		nColorType = 2;			//	glColor3f(0.9f,0.53f,0.13f);
	}
#ifdef LEM_FIX_RENDER_ITEMTOOLTIP_FIELD
	else if((o->Type == MODEL_WING+7 || o->Type == MODEL_WING+21 || o->Type == MODEL_WING+22 || o->Type == MODEL_WING+23))
	{
		nColorType = 3;			//	glColor3f(0.7f,0.7f,0.7f);
	}
#endif //LEM_FIX_RENDER_ITEMTOOLTIP_FIELD
	else if(o->Type == MODEL_WING+32
#ifdef GIFT_BOX_EVENT
		|| (o->Type == MODEL_POTION+33 && (Level == 0 || Level == 1)) || (o->Type == MODEL_EVENT+22)
#endif //GIFT_BOX_EVENT
		)
	{
		nColorType = 4;			//	glColor3f(1.f, 0.3f, 0.3f);
	}
	else if(o->Type == MODEL_WING+33)
	{
		nColorType = 5;			//	glColor3f(0.3f, 1.0f, 0.3f);
	}
	else if(o->Type == MODEL_WING+34
#ifdef GIFT_BOX_EVENT
		|| (o->Type == MODEL_POTION+34 && (Level == 0 || Level == 1)) || (o->Type == MODEL_EVENT+23)
#endif //GIFT_BOX_EVENT	
		)
	{
		nColorType = 6;			//	glColor3f(0.3f, 0.3f, 1.0f);
	}
	else if((o->Type == MODEL_POTION+32 && (Level == 0 || Level == 1)) || (o->Type == MODEL_EVENT+21))
	{
		nColorType = 7;			//	glColor3f(1.0f, 0.3f, 1.0f);
	}
	else if((g_SocketItemMgr.IsSocketItem(o))
#ifdef ADD_SEED_SPHERE_ITEM
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		|| (o->Type >= MODEL_WING+60 && o->Type <= MODEL_WING+65)	// 시드
		|| (o->Type >= MODEL_WING+70 && o->Type <= MODEL_WING+74)	// 스피어
		|| (o->Type >= MODEL_WING+100 && o->Type <= MODEL_WING+129)	// 시드스피어
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
		|| ((o->Type >= MODEL_WING+60 && o->Type <= MODEL_WING+65)	// 시드
		|| (o->Type >= MODEL_WING+70 && o->Type <= MODEL_WING+74)	// 스피어
		|| (o->Type >= MODEL_WING+100 && o->Type <= MODEL_WING+129)	// 시드스피어
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif	// ADD_SEED_SPHERE_ITEM
		)
	{
		nColorType = 8;			//	glColor3f(0.7f,0.4f,1.0f);
	}
	else if( o->Type == MODEL_HELPER+66)
	{
		nColorType = 9;			//	glColor3f(0.6f, 0.4f, 1.0f);
	}
	else if(o->Type==MODEL_STAFF+10 || o->Type==MODEL_SWORD+19 || o->Type==MODEL_BOW+18 || o->Type==MODEL_MACE+13)
	{
		nColorType = 10;		//	glColor3f(1.f,0.1f,1.f);
	}
	else if((ItemOption&63) > 0 && ( o->Type<MODEL_WING+3 || o->Type>MODEL_WING+6 ) && o->Type!=MODEL_HELPER+30 
		&& ( o->Type<MODEL_WING+36 || o->Type>MODEL_WING+40 )
#ifdef ADD_ALICE_WINGS_1
		&& (o->Type<MODEL_WING+42 || o->Type>MODEL_WING+43)
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		&& !(o->Type>=MODEL_WING+49 && o->Type<=MODEL_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)  //  정령의 날개 ~ 어둠의 날개. 망토. 소환술사 2,3차 날개.
	{
		nColorType = 11;		//	glColor3f(0.1f,1.f,0.5f);
	}
	else
	{
		if(Level >= 7)
		{
			nColorType = 1;
		}
		else if(((ItemLevel>>7)&1) || ((ItemLevel>>2)&1) || ((ItemLevel>>1)&1) || (ItemLevel&1))
		{
			nColorType = 12;
		}
		else if(Level == 0)
		{
			nColorType = 3;
		}
		else if(Level < 3)
		{
			nColorType = 13;
		}
		else if(Level < 5)
		{
			nColorType = 14;
		}
		else if(Level < 7)
		{
			nColorType = 12;
		}
		else
		{
			nColorType = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////
	// 특정 폰트값사용하는 아이템 추가
	////////////////////////////////////////////////////////////////////////
	if((o->Type==MODEL_POTION+13) || (o->Type==MODEL_POTION+14) || (o->Type==MODEL_POTION+16)
		|| (o->Type==MODEL_WING+15) || (o->Type==MODEL_POTION+22) || (o->Type==MODEL_POTION+31)
		|| (o->Type==MODEL_HELPER+14) || (o->Type==MODEL_POTION+41) || (o->Type==MODEL_POTION+42)
		|| (o->Type==MODEL_POTION+43) || (o->Type==MODEL_POTION+44)
		|| (o->Type==MODEL_COMPILED_CELE) || (o->Type==MODEL_COMPILED_SOUL)
		|| (o->Type==MODEL_POTION+17 || o->Type==MODEL_POTION+18 || o->Type==MODEL_POTION+19)
		)
	{
		nFontType=1;				// g_hFontBold
	}

	switch(nColorType)
	{
	case 0:		glColor3f(1.f,1.0f,1.0f);		break;
	case 1:		glColor3f(1.f,0.8f,0.1f);		break;
	case 2:		glColor3f(0.9f,0.53f,0.13f);	break;
	case 3:		glColor3f(0.7f,0.7f,0.7f);		break;
	case 4:		glColor3f(1.0f,0.3f,0.3f);		break;
	case 5:		glColor3f(0.3f,1.0f,0.3f);		break;
	case 6:		glColor3f(0.3f,0.3f,1.0f);		break;
	case 7:		glColor3f(1.0f,0.3f,1.0f);		break;
	case 8:		glColor3f(0.7f,0.4f,1.0f);		break;
	case 9:		glColor3f(0.6f,0.4f,1.0f);		break;
	case 10:	glColor3f(1.0f,0.1f,1.0f);		break;
	case 11:	glColor3f(0.1f,1.0f,0.5f);		break;
	case 12:	glColor3f(0.4f,0.7f,1.0f);		break;
	case 13:	glColor3f(0.9f,0.9f,0.9f);		break;
	case 14:	glColor3f(1.0f,0.5f,0.2f);		break;
	default:	glColor3f(1.0f,1.0f,1.0f);		break;
	}

	if(nFontType==1)
	{
		g_pRenderText->SetFont(g_hFontBold);
	}

	////////////////////////////////////////////////////////////////////////
	// 아이템 이름 설정 아이템의 인덱스를 사용해야 함
	////////////////////////////////////////////////////////////////////////
	if(o->Type==MODEL_POTION+17 || o->Type==MODEL_POTION+18 || o->Type==MODEL_POTION+19)
	{	
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
		sprintf (Name, GlobalText[111]); //  천공의 상자.
	}
#ifdef ANNIVERSARY_EVENT
	else if(o->Type == MODEL_POTION+11 && Level >= 1 && Level <= 5)
	{
		sprintf(Name,"%s +%d",GlobalText[116], Level);
	}
#endif	// ANNIVERSARY_EVENT
	else if(o->Type == MODEL_POTION+12)//이밴트 아이템
	{
		switch(Level)
		{
		case 0:sprintf(Name,GlobalText[100]);break;
		case 1:sprintf(Name,GlobalText[101]);break;
		case 2:sprintf(Name,GlobalText[104]);break;
		}
	}
	else if ( o->Type==MODEL_HELPER+15 ) //     서클.
	{
		switch ( Level )
		{
		case 0:sprintf(Name,"%s %s", GlobalText[168], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
		case 1:sprintf(Name,"%s %s", GlobalText[169], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
		case 2:sprintf(Name,"%s %s", GlobalText[167], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
		case 3:sprintf(Name,"%s %s", GlobalText[166], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
		case 4:sprintf(Name,"%s %s", GlobalText[1900], ItemAttribute[o->Type-MODEL_ITEM].Name );break;
		}
	}
	else if ( o->Type==MODEL_HELPER+31 )    //  영혼.
	{
		switch ( Level )
		{
#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
	case 0:sprintf ( Name, "%s of %s", ItemAttribute[o->Type-MODEL_ITEM].Name, GlobalText[1187] ); break;
	case 1:sprintf ( Name, "%s of %s", ItemAttribute[o->Type-MODEL_ITEM].Name, GlobalText[1214] ); break;
#else // SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
			// 한국
	case 0:sprintf ( Name, "%s %s", GlobalText[1187], ItemAttribute[o->Type-MODEL_ITEM].Name ); break;
	case 1:sprintf ( Name, "%s %s", GlobalText[1214], ItemAttribute[o->Type-MODEL_ITEM].Name ); break;
#endif // SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
		}
	}
	else if ( o->Type==MODEL_EVENT+16 )     //  군주의 소매.
	{
		sprintf ( Name, GlobalText[1235] );
	}
	else if(o->Type == MODEL_EVENT+4)//성탄의별
	{
		sprintf(Name,GlobalText[105]);
	}
	else if(o->Type == MODEL_EVENT+5)//폭죽/마법 주머니.
	{
#ifdef  NEW_YEAR_BAG_EVENT
		switch ( Level )
		{
		case 14: sprintf(Name,GlobalText[1650]); break;
		case 15: sprintf(Name,GlobalText[1651]); break;
		default: sprintf(Name,GlobalText[106]); break;
		}
#else // NEW_YEAR_BAG_EVENT
		sprintf(Name,GlobalText[106]);
#endif// NEW_YEAR_BAG_EVENT
	}
	else if(o->Type == MODEL_EVENT+6)//사랑의 하트
	{
		if (Level == 13)	//다크로드의 마음
		{
			sprintf(Name,"%s",GlobalText[117]);
		}
		else
		{
			sprintf(Name,GlobalText[107]);
		}
	}
	else if(o->Type == MODEL_EVENT+7)//사랑의 올리브
	{
		sprintf(Name,GlobalText[108]);
	}
	else if(o->Type == MODEL_EVENT+8)//은 훈장
	{
		sprintf(Name,GlobalText[109]);
	}
	else if(o->Type == MODEL_EVENT+9)//금 훈장
	{
		sprintf(Name,GlobalText[110]);
	}
#ifdef USE_EVENT_ELDORADO
	else if(o->Type == MODEL_EVENT+10)//엘도라도
	{
		sprintf(Name,"%s +%d",GlobalText[115], Level - 7);
	}
#endif
#ifdef _PVP_ADD_MOVE_SCROLL
	else if (o->Type == MODEL_POTION+10 && Level >= 1 && Level <= 8)//  이동문서
	{
		switch (Level)
		{
		case 1: sprintf ( Name, GlobalText[158], GlobalText[30] ); break;     //  로랜시아
		case 2: sprintf ( Name, GlobalText[158], GlobalText[33] ); break;     //  노리아
		case 3: sprintf ( Name, GlobalText[158], GlobalText[32] ); break;     //  데비아스
		case 4: sprintf ( Name, GlobalText[158], GlobalText[31] ); break;     //  던전
		case 5: sprintf ( Name, GlobalText[158], GlobalText[37] ); break;     //  아틀란스
		case 6: sprintf ( Name, GlobalText[158], GlobalText[34] ); break;     //  로스트타워
		case 7: sprintf ( Name, GlobalText[158], GlobalText[38] ); break;     //  타르칸
		case 8: sprintf ( Name, GlobalText[158], GlobalText[55] ); break;     //  이카루스
		}
	}
#endif	// _PVP_ADD_MOVE_SCROLL
#ifdef MYSTERY_BEAD
	else if( o->Type == MODEL_EVENT+19 )
	{
		sprintf(Name, ItemAttribute[ITEM_WING+26].Name);
	}
	else if( o->Type == MODEL_EVENT+20 )
	{
		switch(Level)
		{
		case 1:		//. 빨강수정
			strcpy( Name, GlobalText[1831]); break;
		case 2:		//. 파랑수정
			strcpy( Name, GlobalText[1832]); break;
		case 3:		//. 검은수정
			strcpy( Name, GlobalText[1833]); break;
		}
	}
	else if(o->Type == MODEL_POTION+11 && Level==4)
	{
		strcpy(Name, GlobalText[1834]);
	}
	else if(o->Type == MODEL_POTION+11 && Level==5)
	{
		strcpy(Name, GlobalText[1838]);
	}
#endif // MYSTERY_BEAD
#ifdef GIFT_BOX_EVENT
	else if(o->Type == MODEL_POTION+32 && Level == 1)
	{
		sprintf(Name, GlobalText[2012]);	
	}
	else if(o->Type == MODEL_POTION+33 && Level == 1)
	{
		sprintf(Name, GlobalText[2013]);	
	}
	else if(o->Type == MODEL_POTION+34 && Level == 1)
	{
		sprintf(Name, GlobalText[2014]);	
	}
	else if(o->Type == MODEL_EVENT+21)
	{
		sprintf(Name, GlobalText[2012]);	
	}
	else if(o->Type == MODEL_EVENT+22)
	{
		sprintf(Name, GlobalText[2013]);	
	}
	else if(o->Type == MODEL_EVENT+23)
	{
		sprintf(Name, GlobalText[2014]);	
	}
#endif //GIFT_BOX_EVENT
	else if ( o->Type==MODEL_EVENT+11 )
	{
#ifdef FRIEND_EVENT
		if ( Level==2 ) //  우정의 돌.
		{
			sprintf ( Name, GlobalText[1098] );  //  우정의 돌/
		}
		else
#endif// FRIEND_EVENT
		{
			sprintf ( Name, GlobalText[810] );  //  스톤.
		}
	}
	else if(o->Type==MODEL_EVENT+12)
	{
		sprintf ( Name, GlobalText[906] );	//. 영광의 반지
	}
	else if(o->Type==MODEL_EVENT+13)
	{
		sprintf ( Name, GlobalText[907] );	//. 다크스톤
	}
	else if(o->Type==MODEL_EVENT+14)
	{
		switch ( Level )
		{
		case 2:
			sprintf( Name, GlobalText[928] );	//. 전사의 반지
			break;
		case 3:
			sprintf( Name, GlobalText[929] );	//. 전사의 반지
			break;
		default :
			sprintf( Name, GlobalText[922] );	//. 제왕의 반지
			break;
		}
	}
	else if(o->Type==MODEL_EVENT+15)
	{
		sprintf( Name, GlobalText[925] );	// 마법사의 반지
	}
#ifdef CHINA_MOON_CAKE
	else if(o->Type==MODEL_EVENT+17)
	{
		sprintf( Name, GlobalText[118]);    //  중국 월병.
	}
#endif// CHINA_MOON_CAKE
	else if(o->Type == MODEL_WING+11)//소환구슬
	{
		sprintf(Name,"%s %s",SkillAttribute[30+Level].Name,GlobalText[102]);
	}
	else if(o->Type == MODEL_HELPER+10)//변신반지
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
	else if ( o->Type==MODEL_POTION+21 && Level == 3 )    //  성주의표식
	{
		sprintf( Name, GlobalText[1290] );
	}
	else if ( o->Type==MODEL_POTION+7 ) //  스패셜 물약.//종훈물약
	{
		switch ( Level )
		{
		case 0: sprintf( Name, GlobalText[1413] ); break;
		case 1: sprintf( Name, GlobalText[1414] ); break;
		}
	}
	else if ( o->Type==MODEL_HELPER+7 ) //  계약 문서
	{
		switch ( Level )
		{
		case 0: sprintf( Name, GlobalText[1460] ); break;  //  활 용병.
		case 1: sprintf( Name, GlobalText[1461] ); break;  //  창 용병.
		}
	}
	else if ( o->Type==MODEL_HELPER+11 )    //  주문서.
	{
		switch ( Level )
		{
		case 0: sprintf( Name, GlobalText[1416] ); break;  //  가디언 주문서.
		case 1: sprintf( Name, GlobalText[1462] ); break;  //  라이프 스톤 설치.
		}
	}
	else if ( o->Type==MODEL_EVENT+18 )
	{
		sprintf( Name, GlobalText[1462] );  //  라이프 스톤 설치.
	}
	else
	{
		strcpy(Name, ItemAttribute[o->Type-MODEL_ITEM].Name);
	}

	char TextName[64]={0,};
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

	if((ItemLevel>>7)&1)
	{
		if(o->Type!=MODEL_HELPER+3)
			strcat(Name,GlobalText[176]);
		else    //  디노란트.
		{
			strcat(Name, " +" );
			strcat(Name,GlobalText[179]);
		}
	}
	//  옵션.
	if( (ItemLevel&3) || ((ItemOption>>6)&1) )
		strcat(Name,GlobalText[177]);
	//  행운.
	if((ItemLevel>>2)&1)
		strcat(Name,GlobalText[178]);
#else //PBG_FIX_ITEMNAMEINDEX

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
	bool bFirstOK = true;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

	if(o->Type==MODEL_POTION+15)//금
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
		   ) //  로크의 깃털.
	{
		g_pRenderText->SetFont(g_hFontBold);
		glColor3f(1.f,0.8f,0.1f);
       	sprintf(Name,"%s",ItemAttribute[o->Type-MODEL_ITEM].Name);
	}
#ifdef KJW_FIX_ITEMNAME_ORB_OF_SUMMONING
	else if( o->Type == MODEL_WING+11)
	{
		glColor3f(0.7f,0.7f,0.7f);
		sprintf( Name,"%s %s", SkillAttribute[ 30 + Level ].Name, GlobalText[102] );
	}
#endif // KJW_FIX_ITEMNAME_ORB_OF_SUMMONING
#ifdef LEM_FIX_RENDER_ITEMTOOLTIP_FIELD
	else if(o->Type == MODEL_WING+7)	
	{
		glColor3f(0.7f,0.7f,0.7f);
		sprintf(Name, ItemAttribute[ITEM_WING+7].Name);	
	}
	else if(o->Type == MODEL_WING+21)	
	{
		glColor3f(0.7f,0.7f,0.7f);
		sprintf(Name, ItemAttribute[ITEM_WING+21].Name);	
	}
	else if(o->Type == MODEL_WING+22)	
	{
		glColor3f(0.7f,0.7f,0.7f);
		sprintf(Name, ItemAttribute[ITEM_WING+22].Name);	
	}
	else if(o->Type == MODEL_WING+23)	
	{
		glColor3f(0.7f,0.7f,0.7f);
		sprintf(Name, ItemAttribute[ITEM_WING+23].Name);	
	}
#endif	// LEM_FIX_RENDER_ITEMTOOLTIP_FIELD [lem_2010.8.18]
#ifdef CSK_FREE_TICKET
	// 땅에 떨어진 아이템 이름(실제로는 땅에 떨어지지 않음, 테스트용)
	else if(o->Type == MODEL_HELPER+46)	// 데빌스퀘어 자유입장권
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+46].Name);	
	}
	else if(o->Type == MODEL_HELPER+47)	// 블러드캐슬 자유입장권
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+47].Name);
	}
	else if(o->Type == MODEL_HELPER+48)	// 칼리마 자유입장권
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+48].Name);
	}
#endif // CSK_FREE_TICKET
#ifdef CSK_CHAOS_CARD
	// 땅에 떨어진 아이템 이름(실제로는 땅에 떨어지지 않음, 테스트용)
	else if(o->Type == MODEL_POTION+54)	// 카오스카드
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+54].Name);
	}
#endif // CSK_CHAOS_CARD
#ifdef CSK_RARE_ITEM
	// 땅에 떨어진 아이템 이름(실제로는 땅에 떨어지지 않음, 테스트용)
	else if(o->Type >= MODEL_POTION+58 && o->Type <= MODEL_POTION+62)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif // CSK_RARE_ITEM
#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12	//희귀아이템티켓 7-12
	else if(o->Type >= MODEL_POTION+145 && o->Type <= MODEL_POTION+150)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
	else if(o->Type == MODEL_HELPER+125)	//도플갱어 자유입장권
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+125].Name);	
	}
	else if(o->Type == MODEL_HELPER+126)	//바르카 자유입장권
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+126].Name);
	}
	else if(o->Type == MODEL_HELPER+127)	//바르카 제7맵 자유입장권
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+127].Name);
	}
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
#ifdef CSK_LUCKY_CHARM
	else if( o->Type == MODEL_POTION+53 )// 행운의 부적
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+53].Name);
	}
#endif //CSK_LUCKY_CHARM
#ifdef CSK_LUCKY_SEAL
	else if( o->Type == MODEL_HELPER+43 || o->Type == MODEL_HELPER+44 || o->Type == MODEL_HELPER+45 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_HELPER].Name);
	}
#endif //CSK_LUCKY_SEAL
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
	else if(o->Type >= ITEM_POTION+70 && o->Type <= ITEM_POTION+71)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //PSW_ELITE_ITEM
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
	else if(o->Type >= ITEM_POTION+72 && o->Type <= ITEM_POTION+77)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //PSW_SCROLL_ITEM
#ifdef PSW_SEAL_ITEM               // 이동 인장
	else if(o->Type == ITEM_HELPER+59)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //PSW_SEAL_ITEM
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
	else if( o->Type >= ITEM_HELPER+54 && o->Type <= ITEM_HELPER+58)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //PSW_FRUIT_ITEM
#ifdef PSW_SECRET_ITEM             // 강화의 비약
	else if(o->Type >= ITEM_POTION+78 && o->Type <= ITEM_POTION+82)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //PSW_SECRET_ITEM
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
	else if(o->Type == ITEM_HELPER+60)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //PSW_INDULGENCE_ITEM
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
	else if(o->Type == ITEM_HELPER+61)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //PSW_CURSEDTEMPLE_FREE_TICKET
#ifdef PSW_RARE_ITEM
	else if(o->Type == MODEL_POTION+83)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //PSW_RARE_ITEM
#ifdef CSK_LUCKY_SEAL
	else if( o->Type == MODEL_HELPER+43 || o->Type == MODEL_HELPER+44 || o->Type == MODEL_HELPER+45 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_HELPER].Name);
	}
#endif //CSK_LUCKY_SEAL
#ifdef PSW_CHARACTER_CARD 
	else if(o->Type == MODEL_POTION+91) // 캐릭터 카드
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+91].Name);
	}
#endif // PSW_CHARACTER_CARD
#ifdef PSW_NEW_CHAOS_CARD
	else if(o->Type == MODEL_POTION+92) // 카오스카드 골드
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+92].Name);
	}
	else if(o->Type == MODEL_POTION+93) // 카오스카드 레어
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+93].Name);
	}
	else if(o->Type == MODEL_POTION+95) // 카오스카드 미니
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+95].Name);
	}
#endif // PSW_NEW_CHAOS_CARD
#ifdef PSW_NEW_ELITE_ITEM
#ifdef KJW_FIX_ITEMNAME_CRITICAL_SCROLL
	else if( o->Type == MODEL_POTION + 94 ) // 엘리트 중간 치료 물약
#else // KJW_FIX_ITEMNAME_CRITICAL_SCROLL
	else if(o->Type == ITEM_POTION+94) // 엘리트 중간 치료 물약
#endif // KJW_FIX_ITEMNAME_CRITICAL_SCROLL
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+94].Name);
	}
#endif //PSW_NEW_ELITE_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
	else if( o->Type==MODEL_POTION+84 )  // 벚꽃상자
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+84].Name);
	}
	else if( o->Type==MODEL_POTION+85 )  // 벚꽃술
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+85].Name);
	}
	else if( o->Type==MODEL_POTION+86 )  // 벚꽃경단
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+86].Name);
	}
	else if( o->Type==MODEL_POTION+87 )  // 벚꽃잎
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+87].Name);
	}
	else if( o->Type==MODEL_POTION+88 )  // 흰색 벚꽃
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+88].Name);
	}
	else if( o->Type==MODEL_POTION+89 )  // 붉은색 벚꽃
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+89].Name);
	}
	else if( o->Type==MODEL_POTION+90 )  // 노란색 벚꽃
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+90].Name);
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef PSW_ADD_PC4_SEALITEM
	else if( o->Type == MODEL_HELPER+62 ) {
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+62].Name);
	}
	else if( o->Type == MODEL_HELPER+63 ) {
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+63].Name);
	}
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SCROLLITEM
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
#endif //PSW_ADD_PC4_SCROLLITEM

#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
	else if( o->Type == MODEL_POTION+96 ) 
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+96].Name);
	}
#endif //PSW_ADD_PC4_CHAOSCHARMITEM

#ifdef LDK_ADD_PC4_GUARDIAN
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
#endif //LDK_ADD_PC4_GUARDIAN

#ifdef LDK_ADD_RUDOLPH_PET
	else if( o->Type == MODEL_HELPER+67 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+67].Name);
	}
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
	else if( o->Type == MODEL_HELPER+80 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+80].Name);
	}
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
	else if( o->Type == MODEL_HELPER+106 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+106].Name);
	}
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
	else if( o->Type == MODEL_HELPER+123 )	// 스켈레톤 펫
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+123].Name);
	}
#endif	// YDG_ADD_SKELETON_PET
#ifdef LDK_ADD_SNOWMAN_CHANGERING
	else if( o->Type == MODEL_HELPER+68 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+68].Name);
	}
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
	else if( o->Type == MODEL_HELPER+76 )
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+76].Name);
	}
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	else if( o->Type == MODEL_HELPER+122 )	// 스켈레톤 변신반지
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_HELPER+122].Name);
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM	// 매조각상, 양조각상, 편자

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
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2	// 오크참, 메이플참, 골든오크참, 골든메이플참
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
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	else if ( COMGEM::NOGEM != COMGEM::Check_Jewel_Com(o->Type, true) )
	{
		int iJewelItemIndex = COMGEM::GetJewelIndex(COMGEM::Check_Jewel_Com(o->Type, true), COMGEM::eGEM_NAME);
		g_pRenderText->SetFont(g_hFontBold);
		glColor3f(1.f,0.8f,0.1f);
		sprintf(Name,"%s",GlobalText[iJewelItemIndex]);
	}
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
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
		// 악마의 눈, 악마의 열쇠, 데빌스퀘어 초대권
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
        sprintf (Name, GlobalText[111]); //  천공의 상자.
    }
#ifdef ANNIVERSARY_EVENT
    else if(o->Type == MODEL_POTION+11 && Level >= 1 && Level <= 5)
    {
		sprintf(Name,"%s +%d",GlobalText[116], Level);
    }
#endif	// ANNIVERSARY_EVENT
	else if(o->Type == MODEL_POTION+12)//이밴트 아이템
	{
		switch(Level)
		{
		case 0:sprintf(Name,GlobalText[100]);break;
		case 1:sprintf(Name,GlobalText[101]);break;
		case 2:sprintf(Name,GlobalText[104]);break;
		}
	}
    else if ( o->Type==MODEL_HELPER+15 ) //     서클.
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
    else if ( o->Type==MODEL_HELPER+31 )    //  영혼.
    {
		glColor3f(1.f,0.8f,0.1f);
        switch ( Level )
        {
#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
		case 0:sprintf ( Name, "%s of %s", ItemAttribute[o->Type-MODEL_ITEM].Name, GlobalText[1187] ); break;
        case 1:sprintf ( Name, "%s of %s", ItemAttribute[o->Type-MODEL_ITEM].Name, GlobalText[1214] ); break;
#else // SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
		// 한국
        case 0:sprintf ( Name, "%s %s", GlobalText[1187], ItemAttribute[o->Type-MODEL_ITEM].Name ); break;
        case 1:sprintf ( Name, "%s %s", GlobalText[1214], ItemAttribute[o->Type-MODEL_ITEM].Name ); break;
#endif // SELECTED_LANGUAGE == LANGUAGE_ENGLISH || SELECTED_LANGUAGE == LANGUAGE_PHILIPPINES
        }
    }
	else if ( o->Type==MODEL_EVENT+16 )     //  군주의 소매.
	{
		glColor3f(1.f,0.8f,0.1f);
		sprintf ( Name, GlobalText[1235] );
	}
	else if(o->Type == MODEL_EVENT+4)//성탄의별
	{
		sprintf(Name,GlobalText[105]);
	}
	else if(o->Type == MODEL_EVENT+5)//폭죽/마법 주머니.
	{
    	glColor3f(1.f,0.8f,0.1f);
#ifdef  NEW_YEAR_BAG_EVENT
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
#else // NEW_YEAR_BAG_EVENT
		sprintf(Name,GlobalText[106]);
#endif// NEW_YEAR_BAG_EVENT
	}
	else if(o->Type == MODEL_EVENT+6)//사랑의 하트
	{
		if (Level == 13)	//다크로드의 마음
		{
			glColor3f ( 1.f, 0.8f, 0.1f );
			sprintf(Name,"%s",GlobalText[117]);
		}
		else
		{
			sprintf(Name,GlobalText[107]);
		}
	}
	else if(o->Type == MODEL_EVENT+7)//사랑의 올리브
	{
		sprintf(Name,GlobalText[108]);
	}
	else if(o->Type == MODEL_EVENT+8)//은 훈장
	{
		sprintf(Name,GlobalText[109]);
	}
	else if(o->Type == MODEL_EVENT+9)//금 훈장
	{
		sprintf(Name,GlobalText[110]);
	}
#ifdef USE_EVENT_ELDORADO
	else if(o->Type == MODEL_EVENT+10)//엘도라도
	{
		sprintf(Name,"%s +%d",GlobalText[115], Level - 7);
	}
#endif
#ifdef _PVP_ADD_MOVE_SCROLL
    else if (o->Type == MODEL_POTION+10 && Level >= 1 && Level <= 8)//  이동문서
    {
		glColor3f ( 1.f, 1.f, 1.f );
        switch (Level)
        {
        case 1: sprintf ( Name, GlobalText[158], GlobalText[30] ); break;     //  로랜시아
		case 2: sprintf ( Name, GlobalText[158], GlobalText[33] ); break;     //  노리아
		case 3: sprintf ( Name, GlobalText[158], GlobalText[32] ); break;     //  데비아스
		case 4: sprintf ( Name, GlobalText[158], GlobalText[31] ); break;     //  던전
		case 5: sprintf ( Name, GlobalText[158], GlobalText[37] ); break;     //  아틀란스
		case 6: sprintf ( Name, GlobalText[158], GlobalText[34] ); break;     //  로스트타워
		case 7: sprintf ( Name, GlobalText[158], GlobalText[38] ); break;     //  타르칸
		case 8: sprintf ( Name, GlobalText[158], GlobalText[55] ); break;     //  이카루스
        }
    }
#endif	// _PVP_ADD_MOVE_SCROLL
#ifdef _PVP_MURDERER_HERO_ITEM
    else if ( o->Type==MODEL_POTION+30 )//  영웅살인마징표(wld)
    {
		glColor3f ( 1.f, 1.f, 1.f );
		sprintf(Name, ItemAttribute[o->Type-MODEL_ITEM].Name);
    }
#endif	// _PVP_MURDERER_HERO_ITEM
#ifdef MYSTERY_BEAD
	else if( o->Type == MODEL_EVENT+19 )
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_WING+26].Name);
	}
	else if( o->Type == MODEL_EVENT+20 )
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		switch(Level)
		{
		case 1:		//. 빨강수정
			strcpy( Name, GlobalText[1831]); break;
		case 2:		//. 파랑수정
			strcpy( Name, GlobalText[1832]); break;
		case 3:		//. 검은수정
			strcpy( Name, GlobalText[1833]); break;
		}
	}
	else if(o->Type == MODEL_POTION+11 && Level==4)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		strcpy(Name, GlobalText[1834]);
	}
	else if(o->Type == MODEL_POTION+11 && Level==5)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		strcpy(Name, GlobalText[1838]);
	}
#endif // MYSTERY_BEAD
	else if(o->Type == MODEL_WING+32)//빨간리본상자.(x-mas이벤트용)
	{
		glColor3f ( 1.f, 0.3f, 0.3f );
		sprintf(Name, ItemAttribute[ITEM_WING+32].Name);	
	}
	else if(o->Type == MODEL_WING+33)//초록리본상자.(x-mas이벤트용)
	{
		glColor3f ( 0.3f, 1.0f, 0.3f );
		sprintf(Name, ItemAttribute[ITEM_WING+33].Name);	
	}
	else if(o->Type == MODEL_WING+34)//초록리본상자.(x-mas이벤트용)
	{
		glColor3f ( 0.3f, 0.3f, 1.f );
		sprintf(Name, ItemAttribute[ITEM_WING+34].Name);	
	}
#ifdef CSK_PCROOM_ITEM
	else if(o->Type == MODEL_POTION+55)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_POTION+55].Name);
	}
	else if(o->Type == MODEL_POTION+56)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_POTION+56].Name);
	}
	else if(o->Type == MODEL_POTION+57)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_POTION+57].Name);
	}
#endif // CSK_PCROOM_ITEM
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
	else if(o->Type == MODEL_HELPER+96)		// 강함의 인장 (PC방 아이템, 일본 6차 컨텐츠)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf(Name, ItemAttribute[ITEM_POTION+57].Name);
	}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
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
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
	else if(o->Type == MODEL_WING+48)//
	{
		//혹시?
		glColor3f ( 1.f, 1.f, 1.f );
		sprintf(Name, ItemAttribute[ITEM_WING+48].Name);	
	}
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
	else if(o->Type == MODEL_WING+35)//
	{
		//혹시?
		glColor3f ( 1.f, 1.f, 1.f );
		sprintf(Name, ItemAttribute[ITEM_WING+35].Name);	
	}
	else if(o->Type == MODEL_POTION+45)//분홍 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+45].Name);	
	}
	else if(o->Type == MODEL_POTION+46)//빨간 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+46].Name);	
	}
	else if(o->Type == MODEL_POTION+47)//파란 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+47].Name);	
	}
	else if(o->Type == MODEL_POTION+48)//분홍 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+48].Name);		
	}
	else if(o->Type == MODEL_POTION+49)//빨간 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+49].Name);	
	}
	else if(o->Type == MODEL_POTION+50)//파란 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[ITEM_POTION+50].Name);		
	}
#ifdef GIFT_BOX_EVENT
	else if(o->Type == MODEL_POTION+32)//분홍 초코릿상자.(발렌타인데이 이벤트용)
	{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		int i = MODEL_POTION+32;
		int k = ITEM_POTION+32;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
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
	else if(o->Type == MODEL_POTION+33)//빨간 초코릿상자.(발렌타인데이 이벤트용)
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
	else if(o->Type == MODEL_POTION+34)//파란 초코릿상자.(발렌타인데이 이벤트용)
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
	else if(o->Type == MODEL_EVENT+21)//분홍 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 1.f, 0.3f, 1.f );
		sprintf(Name, GlobalText[2012]);	
	}
	else if(o->Type == MODEL_EVENT+22)//빨간 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 1.0f, 0.3f, 0.3f );
		sprintf(Name, GlobalText[2013]);	
	}
	else if(o->Type == MODEL_EVENT+23)//파란 초코릿상자.(발렌타인데이 이벤트용)
	{
		glColor3f ( 0.3f, 0.3f, 1.f );
		sprintf(Name, GlobalText[2014]);	
	}
#endif

    else if ( o->Type==MODEL_EVENT+11 )
    {
        glColor3f ( 1.f, 0.8f, 0.1f );
#ifdef FRIEND_EVENT
        if ( Level==2 ) //  우정의 돌.
        {
            sprintf ( Name, GlobalText[1098] );  //  우정의 돌/
        }
        else
#endif// FRIEND_EVENT
        {
            sprintf ( Name, GlobalText[810] );  //  스톤.
        }
    }
	else if(o->Type==MODEL_EVENT+12)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf ( Name, GlobalText[906] );	//. 영광의 반지
	}
	else if(o->Type==MODEL_EVENT+13)
	{
		glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf ( Name, GlobalText[907] );	//. 다크스톤
	}
	else if(o->Type==MODEL_EVENT+14)
	{
		glColor3f( 1.f, 0.8f, 0.1f );
        switch ( Level )
        {
        case 2:
		    sprintf( Name, GlobalText[928] );	//. 전사의 반지
            break;
        case 3:
		    sprintf( Name, GlobalText[929] );	//. 전사의 반지
            break;
        default :
		    sprintf( Name, GlobalText[922] );	//. 제왕의 반지
            break;
        }
	}
	else if(o->Type==MODEL_EVENT+15)
	{
		glColor3f( 1.f, 0.8f, 0.1f );
		sprintf( Name, GlobalText[925] );	// 마법사의 반지
	}
#ifdef CHINA_MOON_CAKE
	else if(o->Type==MODEL_EVENT+17)
    {
		glColor3f( 1.f, 0.8f, 0.1f );
		sprintf( Name, GlobalText[118]);    //  중국 월병.
    }
#endif// CHINA_MOON_CAKE
	else if(o->Type == MODEL_WING+11)//소환구슬
	{
		sprintf(Name,"%s %s",SkillAttribute[30+Level].Name,GlobalText[102]);
	}
    else if(o->Type == MODEL_HELPER+10)//변신반지
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
    else if ( o->Type==MODEL_POTION+21 && Level == 3 )    //  성주의표식
    {
	    glColor3f ( 1.f, 0.8f, 0.1f );
		sprintf( Name, GlobalText[1290] );
    }
    else if ( o->Type==MODEL_POTION+7 ) //  스패셜 물약.//종훈물약
    {
        switch ( Level )
        {
        case 0: sprintf( Name, GlobalText[1413] ); break;
        case 1: sprintf( Name, GlobalText[1414] ); break;
        }
    }
    else if ( o->Type==MODEL_HELPER+7 ) //  계약 문서
    {
        switch ( Level )
        {
        case 0: sprintf( Name, GlobalText[1460] ); break;  //  활 용병.
        case 1: sprintf( Name, GlobalText[1461] ); break;  //  창 용병.
        }
    }
    else if ( o->Type==MODEL_HELPER+11 )    //  주문서.
    {
        switch ( Level )
        {
        case 0: sprintf( Name, GlobalText[1416] ); break;  //  가디언 주문서.
        case 1: sprintf( Name, GlobalText[1462] ); break;  //  라이프 스톤 설치.
        }
    }
    else if ( o->Type==MODEL_EVENT+18 )
    {
        sprintf( Name, GlobalText[1462] );  //  라이프 스톤 설치.
    }
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
	else
	{
		bFirstOK = false;
	}

	if( bFirstOK == false )
	{
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef ADD_SEED_SPHERE_ITEM
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
		if ((o->Type >= MODEL_WING+60 && o->Type <= MODEL_WING+65)	// 시드
			|| (o->Type >= MODEL_WING+70 && o->Type <= MODEL_WING+74)	// 스피어
		|| (o->Type >= MODEL_WING+100 && o->Type <= MODEL_WING+129))	// 시드스피어
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
	else if ((o->Type >= MODEL_WING+60 && o->Type <= MODEL_WING+65)	// 시드
		|| (o->Type >= MODEL_WING+70 && o->Type <= MODEL_WING+74)	// 스피어
		|| (o->Type >= MODEL_WING+100 && o->Type <= MODEL_WING+129))	// 시드스피어
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
	{
		glColor3f(0.7f,0.4f,1.0f);	// TEXT_COLOR_VIOLET
		strcpy ( Name, ItemAttribute[o->Type-MODEL_ITEM].Name );
	}
#endif	// ADD_SEED_SPHERE_ITEM
#ifdef PBG_ADD_SANTAINVITATION
	//산타마을의 초대장.
	else if( o->Type == MODEL_HELPER+66 )
	{
		glColor3f( 0.6f, 0.4f, 1.0f);
		sprintf(Name, ItemAttribute[ITEM_HELPER+66].Name);
	}
#endif //PBG_ADD_SANTAINVITATION
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	//행운의 동전
	else if( o->Type == MODEL_POTION+100 )
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[ITEM_POTION+100].Name);
	}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING // (색감) 날개 조합 100% 성공 부적 
	else if(o->Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN 
			&& o->Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END)
	{
		glColor3f ( 0.9f, 0.53f, 0.13f );
		sprintf(Name, ItemAttribute[o->Type - MODEL_ITEM].Name);
	}
#endif //LDS_ADD_CS6_CHARM_MIX_ITEM_WING
#ifdef PBG_ADD_CHARACTERCARD
	// 마검 다크 소환술사 카드
	else if(o->Type == MODEL_HELPER+97 || o->Type == MODEL_HELPER+98 || o->Type == MODEL_POTION+91)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[o->Type].Name);
	}
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
	else if(o->Type == MODEL_HELPER+99)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[ITEM_HELPER+99].Name);
	}
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PBG_ADD_SECRETITEM
	// 활력의비약(최하급/하급/중급/상급)
	else if(o->Type >= MODEL_HELPER+117 && o->Type <= MODEL_HELPER+120)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[o->Type-MODEL_ITEM].Name);
	}
#endif //PBG_ADD_SECRETITEM
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
	else if(o->Type == MODEL_HELPER+109)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+109].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링		// MODEL_HELPER+110
	else if(o->Type == MODEL_HELPER+110)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+110].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링		// MODEL_HELPER+111
	else if(o->Type == MODEL_HELPER+111)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+111].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링		// MODEL_HELPER+112
	else if(o->Type == MODEL_HELPER+112)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+112].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY		// 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
	else if(o->Type == MODEL_HELPER+113)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+113].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY		// 신규 루비(붉은색) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이	// MODEL_HELPER+114
	else if(o->Type == MODEL_HELPER+114)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+114].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE	// 신규 사파이어(녹색) 목걸이	// MODEL_HELPER+115
	else if(o->Type == MODEL_HELPER+115)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_HELPER+115].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE	// 신규 사파이어(녹색) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(실버)	// MODEL_POTION+112
	else if(o->Type == MODEL_POTION+112)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_POTION+112].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(실버)	// MODEL_POTION+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD		// 신규 키(골드)	// MODEL_POTION+113
	else if(o->Type == MODEL_POTION+113)
	{
		glColor3f(1.0f, 0.8f, 0.1f);
		sprintf(Name, ItemAttribute[MODEL_POTION+113].Name);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYGOLD		// 신규 키(골드)	// MODEL_POTION+113
	else
	{
        //  액설런트 아이템.
		//	절대 시리즈
        if ( o->Type==MODEL_STAFF+10 || o->Type==MODEL_SWORD+19 || o->Type==MODEL_BOW+18 || o->Type==MODEL_MACE+13)
        {
            glColor3f(1.f,0.1f,1.f);
        }
#ifdef SOCKET_SYSTEM
		else if (g_SocketItemMgr.IsSocketItem(o))
		{
			glColor3f(0.7f,0.4f,1.0f);	// TEXT_COLOR_VIOLET
		}
#endif	// SOCKET_SYSTEM
        else if((ItemOption&63) > 0 && ( o->Type<MODEL_WING+3 || o->Type>MODEL_WING+6 ) && o->Type!=MODEL_HELPER+30 
			&& ( o->Type<MODEL_WING+36 || o->Type>MODEL_WING+40 )
#ifdef ADD_ALICE_WINGS_1
			&& (o->Type<MODEL_WING+42 || o->Type>MODEL_WING+43)
#endif	// ADD_ALICE_WINGS_1
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			&& !(o->Type>=MODEL_WING+49 && o->Type<=MODEL_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			)  //  정령의 날개 ~ 어둠의 날개. 망토. 소환술사 2,3차 날개.
		{
			glColor3f(0.1f,1.f,0.5f);
		}
#ifdef LDK_ADD_EMPIREGUARDIAN_ITEM
		else if( MODEL_POTION+101 <= o->Type && o->Type <= MODEL_POTION+109 )
		{
			// 의문의쪽지, 가이온의 명령서, 세크로미콘 조각, 세크로미콘
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //LDK_ADD_EMPIREGUARDIAN_ITEM
#ifdef YDG_ADD_DOPPELGANGER_ITEM
		else if (o->Type == MODEL_POTION+111)	// 차원의 마경
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif	// YDG_ADD_DOPPELGANGER_ITEM
#ifdef LDK_ADD_INGAMESHOP_GOBLIN_GOLD
		// 고블린금화
		else if(o->Type == MODEL_POTION+120)
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //LDK_ADD_INGAMESHOP_GOBLIN_GOLD
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
		else if(o->Type == MODEL_POTION+121)
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST		// 봉인된 은색상자
		else if(o->Type == MODEL_POTION+122)
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST				// 금색상자
		else if(o->Type == MODEL_POTION+123 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST				// 은색상자
		else if(o->Type == MODEL_POTION+124 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_PACKAGE_BOX				// 패키지 상자A-F
		else if( MODEL_POTION+134 <= o->Type && o->Type <= MODEL_POTION+139 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //LDK_ADD_INGAMESHOP_PACKAGE_BOX
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		else if( o->Type == MODEL_HELPER+116 )
		{
			glColor3f ( 0.9f, 0.53f, 0.13f );
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
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
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
		else if( MODEL_POTION+114 <= o->Type && o->Type <= MODEL_POTION+119 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
#ifdef LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
		else if( MODEL_POTION+126 <= o->Type && o->Type <= MODEL_POTION+129 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
#ifdef LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
		else if( MODEL_POTION+130 <= o->Type && o->Type <= MODEL_POTION+132 )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
		else if( MODEL_HELPER+121==o->Type )
		{
			glColor3f(1.f,0.8f,0.1f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
#ifdef PBG_ADD_GENSRANKING
		else if(o->Type>=MODEL_POTION+141 && o->Type<=MODEL_POTION+144)		// 보석함
		{
		   	glColor3f (0.9f, 0.53f, 0.13f);
			sprintf(Name, ItemAttribute[o->Type].Name);
		}
#endif //PBG_ADD_GENSRANKING
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		else if (o->Type >= MODEL_POTION+157 && o->Type <= MODEL_POTION+159)
		{
			glColor3f(1.f, 0.8f, 0.1f);
		}
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
		else if ( o->Type==MODEL_WING+25 )      //  군주의 망토.
		{
		    glColor3f ( 1.f, 0.8f, 0.1f );
		}
        else if ( o->Type==MODEL_POTION+28 || o->Type==MODEL_POTION+29 )    //  군주의 표식.
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
			else    //  디노란트.
            {
                strcat(Name, " +" );
    			strcat(Name,GlobalText[179]);
            }
		}
        //  옵션.
		if( (ItemLevel&3) || ((ItemOption>>6)&1) )
			strcat(Name,GlobalText[177]);
        //  행운.
		if((ItemLevel>>2)&1)
			strcat(Name,GlobalText[178]);
	}
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
	}
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#endif //PBG_FIX_ITEMNAMEINDEX
#ifdef LJH_FIX_RUNTIME_ERROR_WHEN_RENDERING_DROPPED_ITEMNAME
	GLfloat fCurColor[4] = { 1.f, 1.f, 1.f, 0.0f };
#else  //LJH_FIX_RUNTIME_ERROR_WHEN_RENDERING_DROPPED_ITEMNAME
	GLfloat fCurColor[3] = { 1.f, 1.f, 1.f };
#endif //LJH_FIX_RUNTIME_ERROR_WHEN_RENDERING_DROPPED_ITEMNAME
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

#ifdef MOD_INTERFACE_CAMERAWORK
	return 640;
#endif //MOD_INTERFACE_CAMERAWORK

	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
		&& (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY)
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION)
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef LEM_ADD_LUCKYITEM
			|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
		))
	{
		iWidth = 640 - (190 * 2);
	}
	else if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		&& (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST)
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
#else	// ASG_ADD_UI_QUEST_PROGRESS_ETC
		&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYQUEST)
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
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
#ifdef PSW_GOLDBOWMAN
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN)
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA)
#endif //PSW_EVENT_LENA	
#ifdef YDG_ADD_NEW_DUEL_UI
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DUELWATCH)
#endif	// YDG_ADD_NEW_DUEL_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE)
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_NPC_MENU
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_MENU)
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_MOD_3D_CHAR_EXCLUSION_UI
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS)
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC)		
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef YDG_FIX_DOPPELGANGER_NPC_WINDOW_SCREEN_SIZE
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DOPPELGANGER_NPC)
#endif	// YDG_FIX_DOPPELGANGER_NPC_WINDOW_SCREEN_SIZE
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA)
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
#ifdef PBG_ADD_GENSRANKING
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GENSRANKING)
#endif //PBG_ADD_GENSRANKING
		) 
	{
        iWidth = 640 - 190;
	}
#ifndef ASG_MOD_3D_CHAR_EXCLUSION_UI
#ifdef ASG_ADD_UI_QUEST_PROGRESS
	else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
	{
		iWidth = 640 - 267;
	}
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
	else
	{
		iWidth = 640;
	}

	return iWidth;
}

///////////////////////////////////////////////////////////////////////////////
// 인밴토리 전체 초기화
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// 인밴토리에 아이템 추가
///////////////////////////////////////////////////////////////////////////////

//  아이템의 색을 결정한다.
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

//  두 아이템를 비교한다. ( -1: 더 나쁜 아이템, 0: 같은 아이템, 1: 더 좋은 아이템 ).
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
        //  스킬.
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
        //  액설런트.
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
        //  옵션.
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
        //  내구력.
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
		//내장비
		if(Index < 12)
		{
			ITEM *ip = &CharacterMachine->Equipment[Index];
			ip->Type       = Type;
			ip->Durability = Item[2];
			ip->Option1    = Item[3];
            ip->ExtOption  = Item[4];   //  세트 타입.
			ip->Number     = 1;
			ip->option_380 = false;
			BYTE b = ( ( (Item[5] & 0x08) << 4) >>7);
			ip->option_380 = b;
			ip->Jewel_Of_Harmony_Option = (Item[6] & 0xf0) >> 4;//옵션 종류
			ip->Jewel_Of_Harmony_OptionLevel = Item[6] & 0x0f;//옵션 레벨( 값이 아님 )

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
					case ITEM_HELPER+37:	//^ 펜릴 인벤토리에 넣을때 생성및 효과
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
#ifdef PET_SYSTEM
            else if ( Index==EQUIPMENT_WEAPON_LEFT )
            {
                switch ( Type )
                {
                case ITEM_HELPER+5:
                    giPetManager::CreatePetDarkSpirit ( Hero );            
                    break;
    #ifdef DARK_WOLF
                    //  임의로 선택.
                case ITEM_SHIELD :
                    giPetManager::CreatePetDarkWolf ( Hero );            
                    break;
    #endif// DARK_WOLF
                }
            }
#endif// PET_SYSTEM

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
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
				if (Type==ITEM_WING+39 || 
					Type==ITEM_HELPER+30 || 
					Type==ITEM_WING+130 ||	// 추가 날개
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
					Type==ITEM_WING+49 ||
					Type==ITEM_WING+50 ||
					Type==ITEM_WING+135||
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
					Type==ITEM_WING+40 )	// 날개 장착시 망토 모양 변경
#else //LDK_ADD_INGAMESHOP_SMALL_WING
				if (Type==ITEM_WING+39 || Type==ITEM_HELPER+30 || Type==ITEM_WING+40)	// 날개 장착시 망토 모양 변경
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
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
					ip->Jewel_Of_Harmony_Option = (Item[6] & 0xf0) >> 4;//옵션 종류
					ip->Jewel_Of_Harmony_OptionLevel = Item[6] & 0x0f;//옵션 레벨( 값이 아님 )

					if(i==0 && j==0)
						ip->Number = 1;
					else
						ip->Number = 0;
		        	ItemConvert(ip,Item[1],Item[3],Item[4]);


				}
			}

            if( SrcInventoryIndex < 12 )
            {
                //  공격속도 계산.
        		if(CharacterAttribute->AbilityTime[0] != 0)
                {
                    CharacterMachine->CalculateAttackSpeed();
                }
                //  공격속도 계산.
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
					ip->Jewel_Of_Harmony_Option = (Item[6] & 0xf0) >> 4;//옵션 종류
					ip->Jewel_Of_Harmony_OptionLevel = Item[6] & 0x0f;//옵션 레벨( 값이 아님 )
					if(i==0 && j==0)
						ip->Number = 1;
					else
						ip->Number = 0;
					ItemConvert(ip,Item[1],Item[3],Item[4]);
				}
			}
			
            if( SrcInventoryIndex < 12 )
            {
                //  공격속도 계산.
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
		//트레이드장비
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
				ip->Jewel_Of_Harmony_Option = (Item[6] & 0xf0) >> 4;//옵션 종류
				ip->Jewel_Of_Harmony_OptionLevel = Item[6] & 0x0f;//옵션 레벨( 값이 아님 )
				if(i==0 && j==0)
					ip->Number = 1;
				else
					ip->Number = 0;
	    		ItemConvert(ip,Item[1],Item[3],Item[4]);
			}
		}
	}

	hanguo_check12();
	if(!First)
	{
		if(CharacterMachine->Equipment[EQUIPMENT_HELPER].Type == -1)
    		DeleteBug(&Hero->Object);
#ifdef PET_SYSTEM
        if ( CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type==-1 )
            giPetManager::DeletePet ( Hero );            
#endif// PET_SYSTEM

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

#if defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
// 인게임샵이 들어가지 않은 해외전용 캐시샵에서만 쓰는함수 이므로 이 함수에 더이상 추가하지 마세요!!!!!
bool IsPartChargeItem(ITEM* pItem)
{
#if !defined(PSW_PARTCHARGE_ITEM1) && defined(LDK_ADD_CASHSHOP_FUNC)
	//국내 유료 아이템 처리
	switch(pItem->Type)
	{
#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
	case ITEM_POTION+96:
#endif //PSW_ADD_PC4_CHAOSCHARMITEM
#ifdef LDK_ADD_PC4_GUARDIAN
	case ITEM_HELPER+64:
	case ITEM_HELPER+65:
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef PBG_FIX_CHAOSCARD
	case ITEM_POTION+54:
#endif //PBG_FIX_CHAOSCARD
		return true;
	default:
		return false;
	}
#else //!defined(PSW_PARTCHARGE_ITEM1) && defined(LDK_ADD_CASHSHOP_FUNC)
	//해외 유료 아이템 처리 - 차후 한쪽으로 통합하자
	if(
#ifdef CSK_FREE_TICKET
		// 자유 입장권 판매 가격 표시 못하게 예외 처리
		(pItem->Type >= ITEM_HELPER+46 && pItem->Type <= ITEM_HELPER+48)
#endif // CSK_FREE_TICKET
#ifdef CSK_CHAOS_CARD
		|| (pItem->Type == ITEM_POTION+54)
#endif // CSK_CHAOS_CARD
#ifdef CSK_RARE_ITEM
		// 희귀아이템 판매 가격 표시 못하게 예외처리
		|| (pItem->Type >= ITEM_POTION+58 && pItem->Type <= ITEM_POTION+62)
#endif // CSK_RARE_ITEM
#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
		// 희귀아이템 판매 (7-12) 가격 표시 못하게 예외처리
		|| (pItem->Type >= ITEM_POTION+145 && pItem->Type <= ITEM_POTION+150)
#endif //LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
		// 자유 입장권(도플갱어, 바르카, 바르카 제7맵) 판매 가격 표시 못하게 예외 처리
		|| (pItem->Type >= ITEM_HELPER+125 && pItem->Type <= ITEM_HELPER+127)
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
#ifdef CSK_LUCKY_CHARM
		|| pItem->Type == ITEM_POTION+53
#endif //CSK_LUCKY_CHARM
#ifdef CSK_LUCKY_SEAL
		|| (pItem->Type >= ITEM_HELPER+43 && pItem->Type <= ITEM_HELPER+45)
#endif //CSK_LUCKY_SEAL
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
		|| (pItem->Type >= ITEM_POTION+70 && pItem->Type <= ITEM_POTION+71)
#endif //PSW_ELITE_ITEM
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
		|| (pItem->Type >= ITEM_POTION+72 && pItem->Type <= ITEM_POTION+77)
#endif //PSW_SCROLL_ITEM
#ifdef PSW_SEAL_ITEM               // 이동 인장
		|| (pItem->Type == ITEM_HELPER+59)
#endif //PSW_SEAL_ITEM
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
		|| ( pItem->Type >= ITEM_HELPER+54 && pItem->Type <= ITEM_HELPER+58)
#endif //PSW_FRUIT_ITEM
#ifdef PSW_SECRET_ITEM             // 강화의 비약
		|| (pItem->Type >= ITEM_POTION+78 && pItem->Type <= ITEM_POTION+82)
#endif //PSW_SECRET_ITEM
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
		|| (pItem->Type == ITEM_HELPER+60)
#endif //PSW_INDULGENCE_ITEM
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET // 환영의 사원 자유 입장권
		|| (pItem->Type == ITEM_HELPER+61)
#endif //PSW_CURSEDTEMPLE_FREE_TICKET
#ifdef PSW_CHARACTER_CARD
		|| (pItem->Type == ITEM_POTION+91)
#endif // PSW_CHARACTER_CARD
#ifdef PSW_NEW_CHAOS_CARD      
		|| (pItem->Type >= ITEM_POTION+92 && pItem->Type <= ITEM_POTION+93)
		|| (pItem->Type == ITEM_POTION+95)
#endif //PSW_NEW_CHAOS_CARD
#ifdef PSW_NEW_ELITE_ITEM
		|| (pItem->Type == ITEM_POTION+94) // 엘리트 중간 치료 물약
#endif //PSW_NEW_ELITE_ITEM
#ifdef PSW_ADD_PC4_SEALITEM
		|| (pItem->Type >= ITEM_HELPER+62 && pItem->Type <= ITEM_HELPER+63)
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SCROLLITEM
		|| (pItem->Type >= ITEM_POTION+97 && pItem->Type <= ITEM_POTION+98)
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
		|| (pItem->Type == ITEM_POTION+96)
#endif //PSW_ADD_PC4_CHAOSCHARMITEM
#ifdef LDK_ADD_PC4_GUARDIAN
		|| ( pItem->Type == ITEM_HELPER+64 || pItem->Type == ITEM_HELPER+65 )
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
		|| ( pItem->Type == ITEM_HELPER+69 )	// 부활의 부적
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
		|| ( pItem->Type == ITEM_HELPER+70 )	// 이동의 부적
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef ASG_ADD_CS6_GUARD_CHARM
		|| pItem->Type == ITEM_HELPER+81		// 수호의부적
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
		|| pItem->Type == ITEM_HELPER+82		// 아이템보호부적
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM 
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		|| pItem->Type == ITEM_HELPER+93		// 상승의인장마스터
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		|| pItem->Type == ITEM_HELPER+94		// 풍요의인장마스터
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
		|| pItem->Type == ITEM_HELPER+107		// 치명마법반지
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
#ifdef YDG_ADD_CS7_MAX_AG_AURA
		|| pItem->Type == ITEM_HELPER+104		// AG증가 오라
#endif	// YDG_ADD_CS7_MAX_AG_AURA
#ifdef YDG_ADD_CS7_MAX_SD_AURA
		|| pItem->Type == ITEM_HELPER+105		// SD증가 오라
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
		|| pItem->Type == ITEM_HELPER+103		// 파티 경험치 증가 아이템
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
		|| pItem->Type == ITEM_POTION+133		// 엘리트 SD회복 물약
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
		|| pItem->Type == ITEM_HELPER+109
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링		// MODEL_HELPER+110
		|| pItem->Type == ITEM_HELPER+110
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링		// MODEL_HELPER+111
		|| pItem->Type == ITEM_HELPER+111
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ		// 신규 토파즈(주황)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링		// MODEL_HELPER+112
		|| pItem->Type == ITEM_HELPER+112
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY		// 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
		|| pItem->Type == ITEM_HELPER+113
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY		// 신규 루비(붉은색) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이	// MODEL_HELPER+114
		|| pItem->Type == ITEM_HELPER+114
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD	// 신규 에메랄드(푸른) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE	// 신규 사파이어(녹색) 목걸이	// MODEL_HELPER+115
		|| pItem->Type == ITEM_HELPER+115
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE	// 신규 사파이어(녹색) 목걸이
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(실버)	// MODEL_POTION+112
		|| pItem->Type == ITEM_POTION+112
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(실버)	// MODEL_POTION+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(골드)	// MODEL_POTION+113
		|| pItem->Type == ITEM_POTION+113
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER	// 신규 키(골드)	// MODEL_POTION+113
#ifdef LDK_ADD_INGAMESHOP_GOBLIN_GOLD
		|| pItem->Type == ITEM_POTION+120 // 고블린금화
#endif //LDK_ADD_INGAMESHOP_GOBLIN_GOLD
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST				// 금색상자
		|| pItem->Type == ITEM_POTION+123
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST				// 은색상자
		|| pItem->Type == ITEM_POTION+124
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_PACKAGE_BOX				// 패키지 상자A-F
		|| pItem->Type == ITEM_POTION+134			
		|| pItem->Type == ITEM_POTION+135			
		|| pItem->Type == ITEM_POTION+136			
		|| pItem->Type == ITEM_POTION+137			
		|| pItem->Type == ITEM_POTION+138			
		|| pItem->Type == ITEM_POTION+139			
#endif //LDK_ADD_INGAMESHOP_PACKAGE_BOX
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
		|| pItem->Type == ITEM_WING+130
		|| pItem->Type == ITEM_WING+131			
		|| pItem->Type == ITEM_WING+132			
		|| pItem->Type == ITEM_WING+133			
		|| pItem->Type == ITEM_WING+134	
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_WING+135
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		|| pItem->Type == ITEM_HELPER+116			//신규 풍요의 인장
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef LDK_ADD_CS7_UNICORN_PET
		|| pItem->Type == ITEM_HELPER+106			//유니콘 펫
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef ASG_ADD_CHARGED_CHANNEL_TICKET
		|| pItem->Type == ITEM_HELPER+124			// 유료채널 입장권.
#endif	// ASG_ADD_CHARGED_CHANNEL_TICKET
#ifdef LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
		|| pItem->Type == ITEM_POTION+114
		|| pItem->Type == ITEM_POTION+115
		|| pItem->Type == ITEM_POTION+116
		|| pItem->Type == ITEM_POTION+117
		|| pItem->Type == ITEM_POTION+118
		|| pItem->Type == ITEM_POTION+119
#endif // LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
#ifdef LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
		|| pItem->Type == ITEM_POTION+126
		|| pItem->Type == ITEM_POTION+127
		|| pItem->Type == ITEM_POTION+128
		|| pItem->Type == ITEM_POTION+129
#endif // LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
#ifdef LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
		|| pItem->Type == ITEM_POTION+130
		|| pItem->Type == ITEM_POTION+131
		|| pItem->Type == ITEM_POTION+132
#endif // LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
		|| pItem->Type == ITEM_HELPER+121
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
#ifdef YDG_ADD_HEALING_SCROLL
		|| pItem->Type == ITEM_POTION+140	// 치유의 스크롤
#endif	// YDG_ADD_HEALING_SCROLL
	)
	{
		return true;
	}
#endif //!defined(PSW_PARTCHARGE_ITEM1) && defined(LDK_ADD_CASHSHOP_FUNC)
	
	return false;
}
#endif //defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)

// 고가아이템 품목 리스트 정리
bool IsHighValueItem(ITEM* pItem)
{
	// 아이템의 레벨을 먼저 구해놓는다.
	int iLevel = (pItem->Level >> 3) & 15;

	if(
		pItem->Type == ITEM_HELPER+3 ||		// 디노란트
		pItem->Type == ITEM_POTION+13 ||	// 축복의 보석
		pItem->Type == ITEM_POTION+14 ||	// 영혼의 보석
		pItem->Type == ITEM_POTION+16 ||	// 생명의 보석
		pItem->Type == ITEM_POTION+22 ||	// 창조의 보석
		pItem->Type == ITEM_WING+15 ||		// 혼돈의 보석
		pItem->Type == ITEM_POTION+31 ||	// 수호의 보석
		pItem->Type == ITEM_WING+30	||		// 축석 조합
		pItem->Type == ITEM_WING+31	||		// 영석 조합
		// 날개들
        (pItem->Type >= ITEM_WING && pItem->Type <= ITEM_WING+6 ) ||	
        pItem->Type == ITEM_HELPER+4 ||		// 다크호스의 뿔
		pItem->Type == ITEM_HELPER+5 ||		// 다크스피릿의 발톱
		pItem->Type == ITEM_HELPER+30 ||	// 군주의 망토
#ifdef ADD_ALICE_WINGS_1
		(pItem->Type >= ITEM_WING+36 && pItem->Type <= ITEM_WING+43 ) ||	// 3차 날개, 소환술사 날개.
#else	// ADD_ALICE_WINGS_1
		(pItem->Type >= ITEM_WING+36 && pItem->Type <= ITEM_WING+40 ) ||	// 3차 날개들
#endif	// ADD_ALICE_WINGS_1
		// 세트 아이템들
        ((pItem->ExtOption%0x04) == EXT_A_SET_OPTION || (pItem->ExtOption%0x04) == EXT_B_SET_OPTION ) ||
		pItem->Type == ITEM_SWORD+19 ||		// 대천사의 절대검
		pItem->Type == ITEM_STAFF+10 ||		// 대천사의 절대지팡이
		pItem->Type == ITEM_BOW+18 ||		// 대천사의 절대석궁	
		pItem->Type == ITEM_MACE+13 ||		// 대천사의 절대셉터
		pItem->Type == ITEM_HELPER+14 ||	// 로크의 깃털	
		pItem->Type == ITEM_HELPER+15 ||	// 열매
		pItem->Type == ITEM_HELPER+19 ||	// 절대지팡이(퀘스트)
		pItem->Type == ITEM_HELPER+31 ||	// 다크호스의 영혼, 다크스피릿의 영혼
		// 보석원석, 조화의보석, 하급제련석, 상급제련석
		(pItem->Type >= ITEM_POTION+41 && pItem->Type <= ITEM_POTION+44) ||
		// +7레벨 이상의 아이템들
		(iLevel > 6 && pItem->Type < ITEM_WING) ||
		(pItem->Option1 & 63) > 0 ||		// 옵션있는 아이템 
		// 펜릴 관련 아이템
		(pItem->Type >= ITEM_HELPER+34 && pItem->Type <= ITEM_HELPER+37)
		|| pItem->Type == ITEM_HELPER+52	// 콘도르의 불꽃
		|| pItem->Type == ITEM_HELPER+53	// 콘도르의 깃털
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
		|| pItem->Type == ITEM_POTION+121
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST	// 봉인된 은색상자
		|| pItem->Type == ITEM_POTION+122
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifndef LDS_MOD_INGAMESHOPITEM_POSSIBLETRASH_SILVERGOLDBOX	// 주의 ifndef !! 추후 삭제 // 금색상자, 은색상자는 버리기가 가능하도록 기획 수정. 
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST			// 금색상자
		|| pItem->Type == ITEM_POTION+123
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST			// 은색상자
		|| pItem->Type == ITEM_POTION+124
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#endif // LDS_MOD_INGAMESHOPITEM_POSSIBLETRASH_SILVERGOLDBOX	// 주의 ifndef !! 추후 삭제 // 금색상자, 은색상자는 버리기가 가능하도록 기획 수정.
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
		|| pItem->Type == ITEM_WING+130
		|| pItem->Type == ITEM_WING+131
		|| pItem->Type == ITEM_WING+132
		|| pItem->Type == ITEM_WING+133
		|| pItem->Type == ITEM_WING+134
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_WING+135
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PJH_ADD_PANDA_PET
		|| pItem->Type == ITEM_HELPER+80  //펜더펫
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
		|| pItem->Type == ITEM_HELPER+76  //펜더변신반지
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifdef LDK_ADD_PC4_GUARDIAN
		|| pItem->Type == ITEM_HELPER+64  //유료 데몬
		|| pItem->Type == ITEM_HELPER+65  //유료 수호정령
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
		|| pItem->Type == ITEM_HELPER+109
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY			// 인게임샾 아이템 // 신규 루비(붉은색)링		// MODEL_HELPER+110
		|| pItem->Type == ITEM_HELPER+110
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY			// 인게임샾 아이템 // 신규 루비(붉은색)링		// MODEL_HELPER+110
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ			// 인게임샾 아이템 // 신규 토파즈(주황)링		// MODEL_HELPER+111
		|| pItem->Type == ITEM_HELPER+111
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ			// 인게임샾 아이템 // 신규 토파즈(주황)링		// MODEL_HELPER+111
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 인게임샾 아이템 // 신규 자수정(보라색)링		// MODEL_HELPER+112
		|| pItem->Type == ITEM_HELPER+112
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 인게임샾 아이템 // 신규 자수정(보라색)링		// MODEL_HELPER+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY			// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
		|| pItem->Type == ITEM_HELPER+113
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY			// 인게임샾 아이템 // 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD		// 인게임샾 아이템 // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
		|| pItem->Type == ITEM_HELPER+114
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(푸른색)링	// MODEL_HELPER+109
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
		|| pItem->Type == ITEM_HELPER+115
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE		// 인게임샾 아이템 // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER			// 인게임샾 아이템 // 신규 키(실버)				// MODEL_POTION+112
		|| pItem->Type == ITEM_POTION+112
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER			// 인게임샾 아이템 // 신규 키(실버)				// MODEL_POTION+112
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD				// 인게임샾 아이템 // 신규 키(골드)				// MODEL_POTION+113
		|| pItem->Type == ITEM_POTION+113
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYGOLD				// 인게임샾 아이템 // 신규 키(골드)				// MODEL_POTION+113
#ifdef LDK_MOD_PREMIUMITEM_SELL
		|| ( pItem->Type==ITEM_HELPER+20 && iLevel==0) 			// 마법사의 반지
#endif //LDK_MOD_PREMIUMITEM_SELL
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 255)	// 장착해제중인 인벤아이템만 고가아이템 설정 및 판매가능
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (pItem->Type >= ITEM_WING+49 && pItem->Type <= ITEM_WING+50)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#ifdef KJH_FIX_SELL_LUCKYITEM
		|| ( Check_ItemAction(pItem, eITEM_SELL) && pItem->Durability > 0 )		// 럭키 아이템의 내구도가 있을때는 팔수 없다.
#endif // KJH_FIX_SELL_LUCKYITEM
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX	// 보석조합 고가아이템 설정
		|| ( COMGEM::isCompiledGem( pItem ) )
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
		)
	{
#ifdef LDK_FIX_PERIODITEM_SELL_CHECK
		if(true == pItem->bPeriodItem && false == pItem->bExpiredPeriod)
		{
			return false;
		}
#endif //LDK_FIX_PERIODITEM_SELL_CHECK
#ifdef PBG_MOD_PANDAPETRING_NOTSELLING
		else if(pItem->Type == ITEM_HELPER+80
			|| pItem->Type == ITEM_HELPER+76
			|| pItem->Type == ITEM_HELPER+64
			|| pItem->Type == ITEM_HELPER+65
#ifdef YDG_MOD_SKELETON_NOTSELLING
			|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
			|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif // YDG_MOD_SKELETON_NOTSELLING
			)
		{
			// 기간이 만료되면 고가의 아템 판별 이외엔 판매불가
			if(true == pItem->bPeriodItem && true == pItem->bExpiredPeriod)
			{
				return true;
			}
			else	
				return false;
		}
#endif //PBG_MOD_PANDAPETRING_NOTSELLING
		return true;
	}
	
	return false;
}

// 개인상점 거래금지 아이템 품목(나중에 이 리스트를 기본으로 스크립트작업)
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

#ifdef LEM_FIX_ITEMSET_FROMJAPAN	// 개인상점 가능여부 아이템 목록 [lem_2010.8.19]
	int nJapanResult = IsItemSet_FromJapan( pItem, eITEM_PERSONALSHOP );
	if( nJapanResult == 1 )			return true;
	else if( nJapanResult == 0 )	return false;
#endif	// LEM_FIX_ITEMSET_FROMJAPAN [lem_2010.8.19]
#if defined LDK_MOD_ITEM_DROP_TRADE_SHOP_EXCEPTION || defined PBG_MOD_PREMIUMITEM_TRADE_ENDURANCE
	// 기간제 아이템이 아닐경우 일부 아이템 제한 해제( 기획이 계속 바뀌는 중 ㅡ.ㅡ;;;;;; )
	if( (!pItem->bPeriodItem) && 
#ifdef PBG_MOD_PREMIUMITEM_TRADE_ENDURANCE
#ifndef _BLUE_SERVER						// 블루는 불가능
		pItem->Type == ITEM_POTION+96 ||	// 카오스부적
		pItem->Type == ITEM_POTION+54 ||	// 카오스카드
#endif //_BLUE_SERVER
#endif //PBG_MOD_PREMIUMITEM_TRADE_ENDURANCE
		pItem->Type == ITEM_HELPER+64		// 데몬
		|| pItem->Type == ITEM_HELPER+65	// 수호정령
		|| pItem->Type == ITEM_HELPER+80	// 팬더펫
		|| pItem->Type == ITEM_HELPER+76	// 팬더변신반지
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 255)	// 비장착일때 개인거래 가능
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (pItem->Type==ITEM_HELPER+20 && ((pItem->Level >> 3) & 15)==0) 			// 마법사의 반지
		)
	{
		return false;
	}		
#endif //LDK_MOD_ITEM_DROP_TRADE_SHOP_EXCEPTION

// 일본만 엘리트 해골전사 변신반지 기획변경에 의해 설정 변경 (2010.01.21)
// (땅에 버리기, 유저간 거래, 창고사용, 개인상점 불가능->가능)
	if(pItem->Type == ITEM_HELPER+38		// 문스톤 팬던트
#ifndef LJH_MOD_ELITE_SKELETON_WARRIOR_CHANGE_RING_SETTING
		|| pItem->Type == ITEM_HELPER+39	// 엘리트 해골전사 변신반지
#endif	//LJH_MOD_ELITE_SKELETON_WARRIOR_CHANGE_RING_SETTING	
		|| (pItem->Type == ITEM_POTION+21 && ((pItem->Level>>3)&15) != 3)		// 성주의 표식
		|| ( pItem->Type >= ITEM_POTION+23 && pItem->Type <= ITEM_POTION+26 )   //  퀘스트 아이템
		|| pItem->Type == ITEM_HELPER+19	// 절대지팡이
		|| (pItem->Type == ITEM_POTION+11 && ((pItem->Level>>3)&0x0F) == 13)
#ifdef CSK_LUCKY_SEAL
		|| (pItem->Type >= ITEM_HELPER+43 && pItem->Type <= ITEM_HELPER+45)
#endif //CSK_LUCKY_SEAL
		|| (pItem->Type == ITEM_HELPER+20 && ((pItem->Level>>3)&15) != 0)
		|| pItem->Type == ITEM_POTION+65	// 데쓰빔나이트의 불꽃
		|| pItem->Type == ITEM_POTION+66	// 헬마이네의 뿔
		|| pItem->Type == ITEM_POTION+67	// 어둠의불사조의 깃털
		|| pItem->Type == ITEM_POTION+68	// 심연의눈동자
#if defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
		|| IsPartChargeItem(pItem)
#endif //defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
#ifdef PBG_ADD_CHARACTERCARD
		|| pItem->Type == ITEM_HELPER+97	//마검사 캐릭터 카드
		|| pItem->Type == ITEM_HELPER+98	//다크로드 캐릭터 카드
		|| pItem->Type == ITEM_POTION+91	//소환술사 캐릭터 카드
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
		|| pItem->Type == ITEM_HELPER+99	//캐릭터 슬롯 열쇠
#endif //PBG_ADD_CHARACTERSLOT
#ifndef KJH_MOD_CAN_TRADE_PANDA_PET		// 현재 해외용 define
#ifdef PJH_ADD_PANDA_PET
		|| pItem->Type == ITEM_HELPER+80	//팬더펫
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
		|| pItem->Type == ITEM_HELPER+76	//팬더변반
#endif //PJH_ADD_PANDA_CHANGERING
#endif // KJH_MOD_CAN_TRADE_PANDA_PET	// 현재 해외용 define
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifndef PBG_MOD_SECRETITEM
		// 거래 가능하게로 변경
#ifdef PBG_ADD_SECRETITEM
		//활력의 비약(최하급/하급/중급/상급)
		|| (pItem->Type >= ITEM_HELPER+117 && pItem->Type <= ITEM_HELPER+120)
#endif //PBG_ADD_SECRETITEM
#endif //PBG_MOD_SECRETITEM
#ifdef LDS_MOD_CHAOSCHARMITEM_DONOT_TRADE
		|| (pItem->Type == MODEL_POTION+96)		// 카오스 조합 부적
#endif // LDS_MOD_CHAOSCHARMITEM_DONOT_TRADE
#ifdef LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR
		|| pItem->Type ==ITEM_HELPER+109	// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
		|| pItem->Type ==ITEM_HELPER+110	// 신규 루비(붉은색)링		// MODEL_HELPER+110
		|| pItem->Type ==ITEM_HELPER+111	// 신규 토파즈(주황)링		// MODEL_HELPER+111
		|| pItem->Type ==ITEM_HELPER+112	// 신규 자수정(보라색)링	// MODEL_HELPER+112
		|| pItem->Type ==ITEM_HELPER+113	// 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
		|| pItem->Type ==ITEM_HELPER+114	// 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
		|| pItem->Type ==ITEM_HELPER+115	// 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115	
#endif // LDS_MOD_INGAMESHOPITEM_RING_AMULET_CHARACTERATTR
#ifdef LDK_MOD_INGAMESHOP_WIZARD_RING_PERSONALSHOPBAN
		|| ( pItem->Type==ITEM_HELPER+20 && ((pItem->Level >> 3) & 15)==0) 			// 마법사의 반지
#endif //LDK_MOD_INGAMESHOP_WIZARD_RING_PERSONALSHOPBAN
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| (pItem->Type >= ITEM_POTION+151 && pItem->Type <= ITEM_POTION+156)	// 시간제 퀘스트 아이템
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

// 교환 거래금지 아이템 품목
bool IsTradeBan(ITEM* pItem)
{
#ifdef LEM_FIX_ITEMSET_FROMJAPAN	// 아이템 거래 가능여부 아이템 목록 [lem_2010.8.19]
	int nJapanResult = IsItemSet_FromJapan( pItem, eITEM_TRADE );
	if( nJapanResult == 1 )			return true;
	else if( nJapanResult == 0 )	return false;
#endif	// LEM_FIX_ITEMSET_FROMJAPAN [lem_2010.8.19]

#ifdef PBG_MOD_PREMIUMITEM_TRADE
	// 일부 프리미엄 아이템 IsPartChargeItem(pItem)함수내에 포함된 아이템중에
	// 거래만 풀기 위함으로 예외처리.IsPartChargeItem(pItem)안에서 제거하면 모든게 풀림.
	// 먼저 걸러내자,단 해외 아이템관련 사항으로 아이템별 예외처리
#if defined LDK_MOD_ITEM_DROP_TRADE_SHOP_EXCEPTION || defined PBG_MOD_PREMIUMITEM_TRADE_ENDURANCE
	// 밑의 not defined와 반대 되는 내용( 기획이 계속 바뀌는 중 ㅡ.ㅡ;;;;;; )
	if( (!pItem->bPeriodItem) && ( 
#if !defined PBG_MOD_PREMIUMITEM_TRADE_ENDURANCE || defined PBG_MOD_PREMIUMITEM_TRADE_0118
		// 블루 레드 상관없이 개인거래 가능하도록 기획변경..(10.01.18)
//#ifndef _BLUE_SERVER						// 블루는 불가능
		pItem->Type == ITEM_POTION+96 ||	// 카오스부적
		pItem->Type == ITEM_POTION+54 ||	// 카오스카드
#ifdef PBG_MOD_PREMIUMITEM_TRADE_0118
		pItem->Type == ITEM_POTION+53 ||	// 행운의부적
#endif //PBG_MOD_PREMIUMITEM_TRADE_0118
//#endif //_BLUE_SERVER
#endif //!defined PBG_MOD_PREMIUMITEM_TRADE_ENDURANCE || defined PBG_MOD_PREMIUMITEM_TRADE_0118
		pItem->Type == ITEM_HELPER+64		// 데몬
		|| pItem->Type == ITEM_HELPER+65	// 수호정령
		|| pItem->Type == ITEM_HELPER+80	// 팬더펫
		|| pItem->Type == ITEM_HELPER+76	// 팬더변신반지
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 255)	// 비장착일때 판매 가능 
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (pItem->Type==ITEM_HELPER+20 && ((pItem->Level >> 3) & 15)==0) 			// 마법사의 반지
		) )
	{
		return false;
	}		
#endif //LDK_MOD_ITEM_DROP_TRADE_SHOP_EXCEPTION
#ifndef LDK_MOD_TRADEBAN_ITEMLOCK_AGAIN // 일부 유료 아이템중 트레이드 가능 불가능 변경(왜이런식으로 일처리를 해야되나....)(09.10.29)
	if(pItem->Type == ITEM_POTION+96		// 카오스부적
		|| pItem->Type == ITEM_POTION+54	// 카오스카드
		|| pItem->Type == ITEM_HELPER+64	// 데몬
		|| pItem->Type == ITEM_HELPER+65	// 수호정령
		|| pItem->Type == ITEM_HELPER+80	// 팬더펫
		|| pItem->Type == ITEM_HELPER+76	// 팬더변신반지
		)
	{
		return false;
	}		
#endif //LDK_MOD_TRADEBAN_ITEMLOCK_AGAIN
#endif //PBG_MOD_PREMIUMITEM_TRADE

// 일본만 엘리트 해골전사 변신반지 기획변경에 의해 설정 변경 (2010.01.21)
// (땅에 버리기, 유저간 거래, 창고사용, 개인상점 불가능->가능)
	if(
		pItem->Type == ITEM_HELPER+38		// 문스톤 팬던트
#ifndef LJH_MOD_ELITE_SKELETON_WARRIOR_CHANGE_RING_SETTING
		|| pItem->Type == ITEM_HELPER+39	// 엘리트 해골전사 변신반지
#endif //LJH_MOD_ELITE_SKELETON_WARRIOR_CHANGE_RING_SETTING
		|| (pItem->Type == ITEM_POTION+21 && ((pItem->Level>>3)&15) != 3)		// 성주의 표식
		|| ( pItem->Type >= ITEM_POTION+23 && pItem->Type <= ITEM_POTION+26 )   //  퀘스트 아이템
		|| pItem->Type == ITEM_HELPER+19 
		|| (pItem->Type == ITEM_POTION+11 && ((pItem->Level>>3)&0x0F) == 13)
#ifdef CSK_LUCKY_SEAL
		|| (pItem->Type >= ITEM_HELPER+43 && pItem->Type <= ITEM_HELPER+45)
#endif //CSK_LUCKY_SEAL
		|| (pItem->Type == ITEM_HELPER+20 && ((pItem->Level>>3)&15) != 0)
		|| pItem->Type == ITEM_POTION+64	// 성물
		|| pItem->Type == ITEM_POTION+65	// 데쓰빔나이트의 불꽃
		|| pItem->Type == ITEM_POTION+66	// 헬마이네의 뿔
		|| pItem->Type == ITEM_POTION+67	// 어둠의불사조의 깃털
		|| pItem->Type == ITEM_POTION+68	// 심연의눈동자
#if defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
		|| IsPartChargeItem(pItem)
#endif //defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
#ifdef PBG_ADD_CHARACTERCARD
		|| pItem->Type == ITEM_HELPER+97	// 마검사 캐릭터 카드
		|| pItem->Type == ITEM_HELPER+98	// 다크로드 캐릭터 카드
		|| pItem->Type == ITEM_POTION+91	// 소환술사 캐릭터 카드
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
		|| pItem->Type == ITEM_HELPER+99	// 캐릭터 슬롯 열쇠
#endif //PBG_ADD_CHARACTERSLOT
#ifndef KJH_MOD_CAN_TRADE_PANDA_PET		// 현재 해외용 define
#ifdef PJH_ADD_PANDA_PET
		|| pItem->Type == ITEM_HELPER+80	// 팬더펫
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
		|| pItem->Type == ITEM_HELPER+76	//팬더변반
#endif //PJH_ADD_PANDA_CHANGERING
#endif // KJH_MOD_CAN_TRADE_PANDA_PET	// 현재 해외용 define
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifndef PBG_MOD_SECRETITEM
		// 가능하도록 변경
#ifdef PBG_ADD_SECRETITEM
		//활력의 비약(최하급/하급/중급/상급)
		|| (pItem->Type >= ITEM_HELPER+117 && pItem->Type <= ITEM_HELPER+120)
#endif //PBG_ADD_SECRETITEM
#endif //PBG_MOD_SECRETITEM
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| (pItem->Type >= ITEM_POTION+151 && pItem->Type <= ITEM_POTION+156)	// 시간제 퀘스트 아이템
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		)
	{
		return true;
	}

	if( pItem->Type == ITEM_POTION+52)		// GM 선물상자: GM은 거래 가능
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

// 개인창고저장 금지 아이템 품목
bool IsStoreBan(ITEM* pItem)
{
#ifdef LEM_FIX_ITEMSET_FROMJAPAN	// 창고저장 가능여부 아이템 목록 [lem_2010.8.19]
	int nJapanResult = IsItemSet_FromJapan( pItem, eITEM_STORE );
	if( nJapanResult == 1 )			return true;
	else if( nJapanResult == 0 )	return false;
#endif	// LEM_FIX_ITEMSET_FROMJAPAN [lem_2010.8.19]

	if(( pItem->Type >= ITEM_POTION+23 && pItem->Type <= ITEM_POTION+26 )   //  퀘스트 아이템
		|| (pItem->Type == ITEM_POTION+21 && ((pItem->Level>>3)&15) != 3)		// 성주의 표식
		|| pItem->Type == ITEM_HELPER+19 
		|| (pItem->Type == ITEM_POTION+11 && ((pItem->Level>>3)&0x0F) == 13)
#ifdef CSK_LUCKY_SEAL
		|| (pItem->Type >= ITEM_HELPER+43 && pItem->Type <= ITEM_HELPER+45)
#endif //CSK_LUCKY_SEAL
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		|| pItem->Type == ITEM_HELPER+93	// 상승의인장마스터
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		|| pItem->Type == ITEM_HELPER+94	// 풍요의인장마스터
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
		|| (pItem->Type == ITEM_HELPER+20 && ((pItem->Level>>3)&15) != 0)
		|| pItem->Type == ITEM_POTION+65	// 데쓰빔나이트의 불꽃
		|| pItem->Type == ITEM_POTION+66	// 헬마이네의 뿔
		|| pItem->Type == ITEM_POTION+67	// 어둠의불사조의 깃털
		|| pItem->Type == ITEM_POTION+68	// 심연의눈동자
#ifdef YDG_FIX_USED_PORTAL_CHARM_STORE_BAN
#ifdef YDG_ADD_CS5_PORTAL_CHARM
		|| (pItem->Type == ITEM_HELPER+70 && pItem->Durability == 1)	// 장소 저장한 이동의 부적
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#endif	// YDG_FIX_USED_PORTAL_CHARM_STORE_BAN
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| (pItem->Type >= ITEM_POTION+151 && pItem->Type <= ITEM_POTION+156)	// 시간제 퀘스트 아이템
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
		|| (pItem->bPeriodItem == true)
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability == 254)		// 인벤아이템이 일반아이템으로 설정된 경우(레드) 장착시에는 창고보관 불가 
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
//----------------------------------------------------------------------------------------
// Function: 
// Input   :  
// Output  : 
//------------------------------------------------------------------------[lem_2010.9.10]-
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 액션 가능 여부 ( 개인상점/ 창고/ 거래/ 버리기/ 판매 )
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

//----------------------------------------------------------------------------------------
// Function: 
// Input   :  
// Output  : 
//------------------------------------------------------------------------[lem_2010.9.10]-
bool Check_ItemAction( ITEM* _pItem, ITEMSETOPTION _eAction, bool _bType )
{
	std::vector<sItemAct>			sItem;
	std::vector<sItemAct>::iterator li;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int		nMaxClass	= 11;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int		i			= 0;
	
	for( i=0; i<12; i++ )	sItem.push_back( Set_ItemActOption( ITEM_HELPER+135 +i, 0 ) );		// 럭키아이템 교환 티켓 [lem_2010.9.8]
	for( i=0; i< 2; i++ )	sItem.push_back( Set_ItemActOption( ITEM_POTION+160 +i, 0 ) );		// 상승의 보석, 연장의 보석 [lem_2010.9.8]
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

//----------------------------------------------------------------------------------------
// Function: 럭키아이템 체크를 한다.
// Input   : 아이템 포인터
// Output  : 럭키아이템 TRUE, 일반템 FALSE
//------------------------------------------------------------------------[lem_2010.9.7]-
bool Check_LuckyItem( int _nIndex, int _nType )
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int		nItemTab		= int( (_nIndex+_nType)/ 512.0f);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
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

//----------------------------------------------------------------------------------------
// Function: 일본에서 카오스 카드를 통해 원래설정의 캐쉬템을 일반템으로 제공하고 있는 아이템.
// Input   :  ITEM* pItem 
// Output  : 사용할 수 없다면 FALSE, 사용가능 TRUE
//------------------------------------------------------------------------[lem_2010.8.19]-
#ifdef	LEM_FIX_ITEMSET_FROMJAPAN	// IsItemSet_FromJapan 함수[lem_2010.8.19]
int IsItemSet_FromJapan( ITEM* _pItem, int _nType )
{
	int	bEnAble		= 1;
	int	nItem		= -1;
	int nList[9]	= { ITEM_POTION+ 96,	// 0: 카오스 조합부적
						ITEM_HELPER+ 64,	// 1: 데몬 펫
						ITEM_HELPER+ 65,	// 2: 수호정령 펫
						ITEM_HELPER+ 80,	// 3: 팬더 펫
						ITEM_HELPER+ 76,	// 4: 팬더 변신반지
						ITEM_HELPER+122,	// 5: 스켈레톤 변신반지
						ITEM_HELPER+123,	// 6: 스켈레톤 펫
						ITEM_HELPER+106,	// 7: 유니콘 펫
						ITEM_HELPER+107 };	// 8: 치명적인 마법사의 반지


	for( int i=0; i<10; i++ )
	{
		if( nList[i] != _pItem->Type )	continue;
		nItem = i;
		break;
	}
	if( nItem < 0 )		return -1;

	switch( _nType )
	{
		// 개인상점
		case eITEM_PERSONALSHOP:
			if( nItem == 3 || nItem == 4 )	bEnAble	= 0;
		break;
		// 창고이용
		case eITEM_STORE:
			bEnAble	= 0;
		break;
		// 거래
		case eITEM_TRADE:
			if( nItem == 3 || nItem == 4 )	bEnAble	= 0;
		break;
		// 버리기
		case eITEM_DROP:
			if( nItem == 3 || nItem == 4 )	bEnAble	= 0;
		break;
		// NPC판매
		case eITEM_SELL:
			if( nItem == 3 || nItem == 4 )	bEnAble	= 0;
		break;
	}

	return bEnAble;
}
#endif

// 버리기 금지 아이템 품목
bool IsDropBan(ITEM* pItem)
{
#ifdef LEM_FIX_ITEMSET_FROMJAPAN	// 버리기 가능여부 아이템 목록 (2010.8.19)
	int nJapanResult = IsItemSet_FromJapan( pItem, eITEM_DROP );
	if( nJapanResult == 1 )			return true;
	else if( nJapanResult == 0 )	return false;
#endif // LEM_FIX_ITEMSET_FROMJAPAN

#ifdef LDK_MOD_PREMIUMITEM_DROP
	// * 금,은색 상자와 금,은색 잠긴 상자 모두 버리기가 가능한 기획 요청으로 수정 하였습니다.
	// 일부 프리미엄 아이템 IsPartChargeItem(pItem)함수내에 포함된 아이템중에
	// 버리기 풀기 위함으로 예외처리.IsPartChargeItem(pItem)안에서 제거하면 모든게 풀림.
	// 먼저 걸러내자,단 해외 아이템관련 사항으로 아이템별 예외처리

#ifdef LDK_MOD_ITEM_DROP_TRADE_SHOP_EXCEPTION
	// 기간제 아이템이 아닐경우 일부 아이템 제한 해제( 기획이 계속 바뀌는 중 ㅡ.ㅡ;;;;;; )
	if( (!pItem->bPeriodItem) &&
		( pItem->Type == ITEM_POTION+96		// 카오스부적
		|| pItem->Type == ITEM_POTION+54	// 카오스카드
		|| pItem->Type == ITEM_HELPER+64	// 데몬
		|| pItem->Type == ITEM_HELPER+65	// 수호정령
		|| pItem->Type == ITEM_HELPER+80	// 팬더펫
		|| pItem->Type == ITEM_HELPER+76	// 팬더변신반지
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
		|| (pItem->Type==ITEM_HELPER+20 && ((pItem->Level >> 3) & 15)==0) 			// 마법사의 반지
		) )
	{
		return false;
	}		
#endif //LDK_MOD_ITEM_DROP_TRADE_SHOP_EXCEPTION

	if( true == false
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST				// 금색상자
		|| pItem->Type == ITEM_POTION+123
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST				// 은색상자
		|| pItem->Type == ITEM_POTION+124
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
		)
	{
		return false;
	}
#endif //LDK_MOD_PREMIUMITEM_DROP

	if (( pItem->Type >= ITEM_POTION+23 && pItem->Type <= ITEM_POTION+26 )	//  퀘스트 아이템
	//이하 3차 체인지업 퀘스트 관련 아이템들은 서버에서 처리.(파티원은 버릴 수 있으므로)
		|| (pItem->Type >= ITEM_POTION+65 && pItem->Type <= ITEM_POTION+68)	// 데쓰빔나이트의 불꽃, 헬마이네의 뿔, 어둠의불사조의 깃털, 심연의눈동자
#if defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
		|| IsPartChargeItem(pItem)
#ifdef PBG_FIX_CHARM_MIX_ITEM_WING
		|| ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN)		//날개 부적들
			&& (pItem->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END))
#endif //PBG_FIX_CHARM_MIX_ITEM_WING
#endif //defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
#ifdef PBG_ADD_CHARACTERCARD
		|| pItem->Type == ITEM_HELPER+97	//마검사 캐릭터 카드
		|| pItem->Type == ITEM_HELPER+98	//다크로드 캐릭터 카드
		|| pItem->Type == ITEM_POTION+91	//소환술사 캐릭터 카드
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
		|| pItem->Type == ITEM_HELPER+99	//캐릭터 슬롯 열쇠
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PJH_ADD_PANDA_PET
		|| pItem->Type == ITEM_HELPER+80	//캐릭터 슬롯 열쇠
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
		|| pItem->Type == ITEM_HELPER+76	//팬더변반
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifdef PBG_ADD_SECRETITEM
		//활력의 비약(최하급/하급/중급/상급)
		|| (pItem->Type >= ITEM_HELPER+117 && pItem->Type <= ITEM_HELPER+120)
#endif //PBG_ADD_SECRETITEM
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST			// 인게임샾 아이템 - 봉인된 금색상자
		|| pItem->Type == ITEM_POTION+121
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST		// 인게임샾 아이템 - 봉인된 은색상자
		|| pItem->Type == ITEM_POTION+122
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifndef PBG_FIX_DROPBAN_GENS

#ifndef LEM_FIX_JP0711_JEWELBOX_DROPFREE
#ifdef PBG_ADD_GENSRANKING
		|| (pItem->Type>=ITEM_POTION+141 && pItem->Type<=ITEM_POTION+144)		// 보석함
#endif //PBG_ADD_GENSRANKING
#endif // LEM_FIX_JP0711_JEWELBOX_DROPFREE

#endif //PBG_FIX_DROPBAN_GENS
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| (pItem->Type>=ITEM_POTION+151 && pItem->Type<=ITEM_POTION+156)	// 시간제 퀘스트 아이템
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
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

// 상점판매 금지 아이템 품목 (조만간 스크립트로 빼는 작업 예정 - 김재희)
bool IsSellingBan(ITEM* pItem)
{
	int Level = (pItem->Level>>3)&15;

#ifdef LEM_FIX_ITEMSET_FROMJAPAN	// 상점판매 가능여부 아이템 목록 [lem_2010.8.19]
	int nJapanResult = IsItemSet_FromJapan( pItem, eITEM_SELL );
	if( nJapanResult == 1 )			return true;
	else if( nJapanResult == 0 )	return false;
#endif


#ifdef LDK_MOD_PREMIUMITEM_SELL
	// 일부 프리미엄 아이템 IsPartChargeItem(pItem)함수내에 포함된 아이템중에
	// 판매가능 하도록 예외처리.IsPartChargeItem(pItem)안에서 제거하면 모든게 풀림.
	// 먼저 걸러내자,단 해외 아이템관련 사항으로 아이템별 예외처리
	if( true == false
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYSILVER			// 인게임샾 아이템 // 신규 키(실버)				// MODEL_POTION+112
		|| pItem->Type ==ITEM_POTION+112
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYSILVER
#ifdef LDS_ADD_INGAMESHOP_ITEM_KEYGOLD				// 인게임샾 아이템 // 신규 키(골드)				// MODEL_POTION+113
		|| pItem->Type ==ITEM_POTION+113
#endif // LDS_ADD_INGAMESHOP_ITEM_KEYGOLD
#ifdef LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST		// 봉인된 금색상자
		|| pItem->Type == ITEM_POTION+121
#endif //LDK_ADD_INGAMESHOP_LOCKED_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST	// 봉인된 은색상자
		|| pItem->Type == ITEM_POTION+122
#endif //LDK_ADD_INGAMESHOP_LOCKED_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_GOLD_CHEST			// 금색상자
		|| pItem->Type == ITEM_POTION+123
#endif //LDK_ADD_INGAMESHOP_GOLD_CHEST
#ifdef LDK_ADD_INGAMESHOP_SILVER_CHEST			// 은색상자
		|| pItem->Type == ITEM_POTION+124
#endif //LDK_ADD_INGAMESHOP_SILVER_CHEST
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
		|| pItem->Type == ITEM_WING+130
		|| pItem->Type == ITEM_WING+131
		|| pItem->Type == ITEM_WING+132
		|| pItem->Type == ITEM_WING+133
		|| pItem->Type == ITEM_WING+134
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| pItem->Type == ITEM_WING+135
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PJH_ADD_PANDA_PET
		|| pItem->Type == ITEM_HELPER+80  //펜더펫
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
		|| pItem->Type == ITEM_HELPER+76  //펜더변신반지
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifdef LDK_ADD_PC4_GUARDIAN
		|| pItem->Type == ITEM_HELPER+64  //유료 데몬
		|| pItem->Type == ITEM_HELPER+65  //유료 수호정령
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
		|| pItem->Type ==ITEM_HELPER+109
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGRUBY		// 신규 루비(붉은색)링		// MODEL_HELPER+110
		|| pItem->Type ==ITEM_HELPER+110
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGRUBY
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ	// 신규 토파즈(주황)링		// MODEL_HELPER+111
		|| pItem->Type ==ITEM_HELPER+111
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ
#ifdef LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST	// 신규 자수정(보라색)링	// MODEL_HELPER+112
		|| pItem->Type ==ITEM_HELPER+112
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY	// 신규 루비(붉은색) 목걸이	// MODEL_HELPER+113
		|| pItem->Type ==ITEM_HELPER+113
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD // 신규 에메랄드(푸른) 목걸이// MODEL_HELPER+114
		|| pItem->Type ==ITEM_HELPER+114
#endif // LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD
#ifdef LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE // 신규 사파이어(녹색) 목걸이// MODEL_HELPER+115
		|| pItem->Type ==ITEM_HELPER+115
#endif //LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type) && pItem->Durability != 254)	// 비장착중인 인벤아이템만 판매가능
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef KJH_FIX_BTS295_DONT_EXPIRED_WIZARD_RING_RENDER_SELL_PRICE
		|| ((pItem->Type==ITEM_HELPER+20)&&(Level == 0))	// 마법사의 반지
#endif // KJH_FIX_BTS295_DONT_EXPIRED_WIZARD_RING_RENDER_SELL_PRICE
#ifdef KJH_FIX_SELL_EXPIRED_UNICON_PET
		|| (pItem->Type == ITEM_HELPER+106)			// 유니콘 펫
#endif // KJH_FIX_SELL_EXPIRED_UNICON_PET
#ifdef KJH_FIX_SELL_EXPIRED_CRITICAL_WIZARD_RING
		|| (pItem->Type == ITEM_HELPER+107)			// 치명적인 마법사의 반지
#endif // KJH_FIX_SELL_EXPIRED_CRITICAL_WIZARD_RING
		)
	{
#ifdef LDK_FIX_PERIODITEM_SELL_CHECK
		if(true == pItem->bPeriodItem && true == pItem->bExpiredPeriod)
		{
			return false;		// 팔수 있음
		}
#else //LDK_FIX_PERIODITEM_SELL_CHECK
		return false;
#endif //LDK_FIX_PERIODITEM_SELL_CHECK
	}
#endif //LDK_MOD_PREMIUMITEM_SELL
	
	if(
		pItem->Type==ITEM_POTION+11                                  //  행운의 상자는 팔수 없다.
#ifndef BLOODCASTLE_2ND_PATCH
		|| ( pItem->Type==ITEM_POTION+21 && Level==1 )                 //  스톤은 아이템을 팔수 없다.
#endif // BLOODCASTLE_2ND_PATCH
#ifdef FRIEND_EVENT
#ifndef FRIENDLYSTONE_EXCHANGE_ZEN
		|| ( pItem->Type==ITEM_POTION+21 && Level==2 )                 //  우정의 돌은 아이템을 팔수 없다.
#endif // FRIENDLYSTONE_EXCHANGE_ZEN
		|| (pItem->Type==ITEM_HELPER+20 && (Level==1 || Level==2) )    //  마법사의 반지 시리즈.
#endif // FRIEND_EVENT
#ifdef KJH_FIX_BTS295_DONT_EXPIRED_WIZARD_RING_RENDER_SELL_PRICE
		|| ((pItem->bPeriodItem==true)&&(pItem->bExpiredPeriod==false)&&(pItem->Type==ITEM_HELPER+20)&&(Level==0))    //  기간이 지나지 않은 마법사의 반지
		|| (pItem->Type==ITEM_HELPER+20 && (Level==1 || Level==2) )    //  마법사의 반지 시리즈.
#endif // KJH_FIX_BTS295_DONT_EXPIRED_WIZARD_RING_RENDER_SELL_PRICE
		|| pItem->Type == ITEM_HELPER+19			// 대천사의 절대 무기 (가짜)도 팔 수 없다.
		|| (pItem->Type==ITEM_POTION+20 && Level >= 1 && Level <= 5)     //  애니버서리 박스
#if defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
		|| IsPartChargeItem(pItem)
#ifdef PBG_FIX_CHARM_MIX_ITEM_WING
		|| ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN)		//날개 부적들
			&& (pItem->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END))
#endif //PBG_FIX_CHARM_MIX_ITEM_WING
#ifdef PBG_MOD_PANDAPETRING_NOTSELLING
		|| pItem->Type == ITEM_HELPER+80  //펜더펫
		|| pItem->Type == ITEM_HELPER+76  //펜더변신반지
#endif //PBG_MOD_PANDAPETRING_NOTSELLING
#ifdef YDG_MOD_SKELETON_NOTSELLING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif // YDG_MOD_SKELETON_NOTSELLING
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| (g_pMyInventory->IsInvenItem(pItem->Type))	// 비장착중인 인벤아이템만 판매가능
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
#ifdef PBG_MOD_SECRETITEM
		|| (pItem->Type >= ITEM_HELPER+117 && pItem->Type <= ITEM_HELPER+120) // 활력 비약관련
#endif //PBG_MOD_SECRETITEM
#ifdef KJH_FIX_SELL_EXPIRED_UNICON_PET
		|| (pItem->Type == ITEM_HELPER+106)			// 유니콘 펫
#endif // KJH_FIX_SELL_EXPIRED_UNICON_PET
#ifdef KJH_FIX_SELL_EXPIRED_CRITICAL_WIZARD_RING
		|| (pItem->Type == ITEM_HELPER+107)			// 치명적인 마법사의 반지
#endif // KJH_FIX_SELL_EXPIRED_CRITICAL_WIZARD_RING
#endif //#if defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| pItem->Type == ITEM_POTION+151	// 1레벨 의뢰서
		|| pItem->Type == ITEM_POTION+152	// 1레벨 의뢰 완료 확인서
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef KJH_FIX_SELL_LUCKYITEM
		|| ( (IsLuckySetItem(pItem->Type) == true ) && (pItem->Durability > 0) )		// 럭키 아이템의 내구도가 있을때는 팔수 없다.
#endif // KJH_FIX_SELL_LUCKYITEM
		)
	{
		return true;		// 팔수 없음
	}
#ifndef KJH_FIX_SELL_LUCKYITEM			// #ifndef
#ifdef LEM_ADD_LUCKYITEM
	if( Check_ItemAction( pItem, eITEM_SELL ) )	return true;
#endif // LEM_ADD_LUCKYITEM
#endif // KJH_FIX_SELL_LUCKYITEM

	return false;			// 팔수 있음
}



bool IsRepairBan(ITEM* pItem)
{
	if(g_ChangeRingMgr->CheckRepair(pItem->Type) == true)
	{
		return true;
	}
#if defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC) // 유료상점 아이템
	if( IsPartChargeItem(pItem) == true
#ifdef PBG_FIX_CHARM_MIX_ITEM_WING
		|| ((pItem->Type >= ITEM_TYPE_CHARM_MIXWING+EWS_BEGIN)		//날개 부적들
			&& (pItem->Type <= ITEM_TYPE_CHARM_MIXWING+EWS_END))
#endif //PBG_FIX_CHARM_MIX_ITEM_WING
		)
	{
		return true;
	}
#endif //defined(PSW_PARTCHARGE_ITEM1) || defined(LDK_ADD_CASHSHOP_FUNC)

	if(
		(pItem->Type >= ITEM_POTION+55 && pItem->Type <= ITEM_POTION+57)	// 혼돈의 상자
		|| pItem->Type == MODEL_HELPER+43		// 상승의인장 
		|| pItem->Type == MODEL_HELPER+44		// 풍요의인장
		|| pItem->Type == MODEL_HELPER+45		// 유지의인장
#ifdef KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
		|| (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HELPER+3)		// 수호천사, 사탄, 유니리아, 디노란트
#else // KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
		|| (pItem->Type >= ITEM_HELPER && pItem->Type <= ITEM_HELPER+4)		// 수호천사, 사탄, 유니리아, 디노란트, 다크호스의 뿔
#endif // KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET
		|| pItem->Type == ITEM_BOW+7		// 석궁용화살
		|| pItem->Type == ITEM_BOW+15		// 화살
		|| pItem->Type >= ITEM_POTION		// 물약index 전부
		|| (pItem->Type >= ITEM_WING+7 && pItem->Type <= ITEM_WING+19)		// 구슬들
		|| (pItem->Type >= ITEM_HELPER+14 && pItem->Type <= ITEM_HELPER+19)	// 로크의깃털, 열매, 대천사의서, 블러드본, 투명망토, 대천사의절대무기
		|| pItem->Type == ITEM_POTION+21	// 레나
#ifdef MYSTERY_BEAD
		|| pItem->Type == ITEM_WING+26		// 신비의 구슬
#endif // MYSTERY_BEAD	
#ifndef KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET					// #ifndef
		|| pItem->Type == ITEM_HELPER+4		// 다크호스의 뿔
		|| pItem->Type == ITEM_HELPER+5		// 다크스피릿의 뿔
#endif // KJH_ADD_INVENTORY_REPAIR_DARKLOAD_PET					// #ifndef
		|| pItem->Type == ITEM_HELPER+38	// 문스톤팬던트
#ifdef LDK_ADD_RUDOLPH_PET
		|| pItem->Type == ITEM_HELPER+67	//크리스마스 루돌프
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
		|| pItem->Type == ITEM_HELPER+80	//팬더펫
#endif //PJH_ADD_PANDA_PET
#ifdef PJH_ADD_PANDA_CHANGERING
		|| pItem->Type == ITEM_HELPER+76	//팬더변반
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| pItem->Type == ITEM_HELPER+122	// 스켈레톤 변신반지
#endif //YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		|| pItem->Type == ITEM_HELPER+123	// 스켈레톤 펫
#endif //YDG_ADD_SKELETON_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
		|| pItem->Type == ITEM_HELPER+106	//유니콘
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef CSK_EVENT_CHERRYBLOSSOM
		|| pItem->Type == ITEM_POTION+84  // 벚꽃상자
		|| pItem->Type == ITEM_POTION+85  // 벚꽃술
		|| pItem->Type == ITEM_POTION+86  // 벚꽃경단
		|| pItem->Type == ITEM_POTION+87  // 벚꽃잎
		|| pItem->Type == ITEM_POTION+88  // 흰색 벚꽃
		|| pItem->Type == ITEM_POTION+89  // 붉은색 벚꽃
		|| pItem->Type == ITEM_POTION+90  // 노란색 벚꽃
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
		|| g_pMyInventory->IsInvenItem(pItem->Type)
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

//------------------------------------------------------------------------------
#ifdef LDK_FIX_USING_ISREPAIRBAN_FUNCTION
		// 기존에 빠져있던 부분 추가함
		// 함수를 활용합시다...
#ifdef DARK_WOLF
		 || pItem->Type == ITEM_HELPER+6
#endif// DARK_WOLF
		|| pItem->Type == ITEM_HELPER+7
		|| pItem->Type == ITEM_HELPER+10
		|| pItem->Type == ITEM_HELPER+11
		|| pItem->Type == ITEM_HELPER+20
		|| pItem->Type == ITEM_HELPER+29
		
		//^ 펜릴 아이템 수리 불가능
		|| pItem->Type == ITEM_HELPER+32
		|| pItem->Type == ITEM_HELPER+33
		|| pItem->Type == ITEM_HELPER+34
		|| pItem->Type == ITEM_HELPER+35
		|| pItem->Type == ITEM_HELPER+36
		|| pItem->Type == ITEM_HELPER+37

		// 피의 두루마리 등 수리 불가능
		|| pItem->Type == ITEM_HELPER+49
		|| pItem->Type == ITEM_HELPER+50
		|| pItem->Type == ITEM_HELPER+51

#ifdef PBG_ADD_SANTAINVITATION		//산타마을의 초대장
#ifdef YDG_FIX_SANTA_INVITAION_REPAIR
		|| pItem->Type == ITEM_HELPER+66
#else	// YDG_FIX_SANTA_INVITAION_REPAIR
		|| pItem->Type == MODEL_HELPER+66
#endif	// YDG_FIX_SANTA_INVITAION_REPAIR
#endif //PBG_ADD_SANTAINVITATION

#endif //LDK_FIX_USING_ISREPAIRBAN_FUNCTION
//------------------------------------------------------------------------------

#ifdef LDK_ADD_GAMBLE_RANDOM_ICON	// 겜블러 아이콘
		|| pItem->Type == ITEM_HELPER+71
		|| pItem->Type == ITEM_HELPER+72
		|| pItem->Type == ITEM_HELPER+73
		|| pItem->Type == ITEM_HELPER+74
		|| pItem->Type == ITEM_HELPER+75
#endif //LDK_ADD_GAMBLE_RANDOM_ICON
		)
	{
		return true;
	}
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 수리 제외 [lem_2010.9.8]
	if( Check_ItemAction( pItem, eITEM_REPAIR ) )	return true;
#endif // LEM_ADD_LUCKYITEM
	
	return false;
}

bool IsWingItem(ITEM* pItem)
{
	switch(pItem->Type)		// 날개인지 검사
	{
	case ITEM_WING:			// 요정날개
	case ITEM_WING+1:		// 천공날개
	case ITEM_WING+2:		// 사탄날개
	case ITEM_WING+3:		// 정령날개
	case ITEM_WING+4:		// 영혼날개
	case ITEM_WING+5:		// 드라곤날개
	case ITEM_WING+6:		// 암흑날개
	case ITEM_HELPER+30:	// 군주의 망토
	case ITEM_WING+36:		// 폭풍의날개
	case ITEM_WING+37:		// 시공의날개
	case ITEM_WING+38:		// 환영의날개
	case ITEM_WING+39:		// 파멸의날개
	case ITEM_WING+40:		// 제왕의망토
#ifdef ADD_ALICE_WINGS_1
	case ITEM_WING+41:		// 재앙의날개
	case ITEM_WING+42:		// 절망의날개
	case ITEM_WING+43:		// 차원의날개
#endif	// ADD_ALICE_WINGS_1
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
	case ITEM_WING+130:
	case ITEM_WING+131:
	case ITEM_WING+132:
	case ITEM_WING+133:
	case ITEM_WING+134:
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	case ITEM_WING+49:		// 무인의날개
	case ITEM_WING+50:		// 군림의날개
	case ITEM_WING+135:		// 작은무인의망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
// 화면에 오브젝트를 랜더링 하는 함수
// 오브젝트 위치, 각도 설정
///////////////////////////////////////////////////////////////////////////////

#ifdef MR0
AUTOOBJ ObjectSelect;
#else
OBJECT ObjectSelect;
#endif //MR0
void RenderObjectScreen(int Type,int ItemLevel,int Option1,int ExtOption,vec3_t Target,int Select,bool PickUp)
{	
   	int Level = (ItemLevel>>3)&15;
    vec3_t Direction,Position;

	VectorSubtract(Target,MousePosition,Direction);
	if(PickUp)
      	VectorMA(MousePosition,0.07f,Direction,Position);
	else
      	VectorMA(MousePosition,0.1f,Direction,Position);

	// ObjectSelect 처리 부분 1. 일반 아이템
	// =====================================================================================
	// 검류
	if(Type == MODEL_SWORD+0)	// 크리스
	{
		Position[0] -= 0.02f;
		Position[1] += 0.03f;
		Vector(180.f,270.f,15.f,ObjectSelect.Angle);
	}
	// 갑옷류
	else if(Type==MODEL_BOW+7 || Type==MODEL_BOW+15 )
	{
    	Vector(0.f,270.f,15.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_SPEAR+0)	// 광선봉
	{
		Position[1] += 0.05f;
		Vector(0.f,90.f,20.f,ObjectSelect.Angle);
	}
	else if( Type==MODEL_BOW+17)    //  뮤즈활.
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
	else if (Type >= MODEL_STAFF+21 && Type <= MODEL_STAFF+29)	// 사아무트의 서, 닐의 서
	{
    	Vector(0.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_MACE+14)
	{
		Position[1] += 0.1f;
		Position[0] -= 0.01;
    	Vector(180.f,90.f,13.f,ObjectSelect.Angle);
	}	
	//$ 크라이울프 아이템
	else if(Type == MODEL_ARMOR+34)	// 흑기사 갑옷
	{
		Position[1] += 0.03f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELM+35)	// 흑마법사 헬멧
	{
		Position[0] -= 0.02f;
		Position[1] += 0.05f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+35)	// 흑마법사 갑옷
	{
		Position[1] += 0.05f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+36)	// 요정 갑옷
	{
		Position[1] -= 0.05f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+37)	// 다크로드 갑옷
	{
		Position[1] -= 0.05f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	// 바이올렌윈드 ~ 이터널윙 헬멧
	else if (MODEL_HELM+39 <= Type && MODEL_HELM+44 >= Type)
	{
		Position[1] -= 0.05f;
		Vector(-90.f,25.f,0.f,ObjectSelect.Angle);
	}
	// 글로리어스 ~ 이터널윙 갑옷
	else if(MODEL_ARMOR+38 <= Type && MODEL_ARMOR+44 >= Type)
	{
		Position[1] -= 0.08f;
		Vector(-90.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_SWORD+24)	// 흑기사 검
	{
		Position[0] -= 0.02f;
		Position[1] += 0.03f;
    	Vector(180.f,90.f,15.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_MACE+15)	// 다크로드 셉터
	{
		Position[1] += 0.05f;
    	Vector(180.f,90.f,13.f,ObjectSelect.Angle);
	}
#ifdef ADD_SOCKET_ITEM
	else if(Type == MODEL_BOW+22 || Type == MODEL_BOW+23)	// 요정 활
	{
		Position[0] -= 0.10f;
		Position[1] += 0.08f;
    	Vector(180.f,-90.f,15.f,ObjectSelect.Angle);
	}
#else // ADD_SOCKET_ITEM
	else if( Type == MODEL_BOW+22)	// 요정 활
	{
		Position[1] += 0.12f;
    	Vector(180.f,90.f,15.f,ObjectSelect.Angle);
	}
#endif // ADD_SOCKET_ITEM
	else if(Type == MODEL_STAFF+13)	// 흑마법사 지팡이
	{
		Position[0] += 0.02f;
		Position[1] += 0.02f;
		Vector(180.f,90.f,8.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_BOW+20)		//. 요정추가활
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
		// 소켓아이템추가 [시즌4]
	}									
	else if(Type>=MODEL_SHIELD && Type<MODEL_SHIELD+MAX_ITEM_INDEX)
	{
		Vector(270.f,270.f,0.f,ObjectSelect.Angle);
	}
    else if(Type==MODEL_HELPER+3)
    {
		Vector(-90.f,-90.f,0.f,ObjectSelect.Angle);
    }
    else if ( Type==MODEL_HELPER+4 )    //  다크호스.
    {
		Vector(-90.f,-90.f,0.f,ObjectSelect.Angle);
    }
    else if ( Type==MODEL_HELPER+5 )    //  다크스피릿.
    {
		Vector(-90.f,-35.f,0.f,ObjectSelect.Angle);
    }
    else if ( Type==MODEL_HELPER+31 )   //  영혼.
    {
		Vector(-90.f,-90.f,0.f,ObjectSelect.Angle);
    }
    else if ( Type==MODEL_HELPER+30 )   //  망토.    
    {
        Vector ( -90.f, 0.f, 0.f, ObjectSelect.Angle );
    }
	else if ( Type==MODEL_EVENT+16 )    //  군주의 소매
	{
		Vector ( -90.f, 0.f, 0.f, ObjectSelect.Angle );
	}
    else if ( Type==MODEL_HELPER+16 || Type == MODEL_HELPER+17 )
    {	//. 대천사의서, 블러드본
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
    }
	else if ( Type==MODEL_HELPER+18 )	//. 투명망도
	{
		Vector(290.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if ( Type==MODEL_EVENT+11 )	//. 스톤
	{
#ifdef FRIEND_EVENT
        if ( Type==MODEL_EVENT+11 && Level==2 )    //  우정의 돌.
        {
    		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
        }
        else
#endif// FRIEND_EVENT
        {
            Vector(-90.f, -20.f, -20.f, ObjectSelect.Angle);
        }
	}
	else if ( Type==MODEL_EVENT+12)		//. 영광의 반지
	{
		Vector(250.f, 140.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type==MODEL_EVENT+14)		//. 제왕의 반지
	{
		Vector(255.f, 160.f, 0.f, ObjectSelect.Angle);
	}
	else if (Type==MODEL_EVENT+15)		// 마법사의 반지
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
    else if ( Type>=MODEL_HELPER+21 && Type<=MODEL_HELPER+24 )
    {
		Vector(270.f, 160.f, 20.f, ObjectSelect.Angle);
    }
	else if ( Type==MODEL_HELPER+29 )	//. 투명망도
	{
		Vector(290.f,0.f,0.f,ObjectSelect.Angle);
	}
	//^ 펜릴 위치, 각도 조절
	else if(Type == MODEL_HELPER+32)	// 갑옷 파편
	{
		Position[0] += 0.01f;
		Position[1] -= 0.03f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+33)	// 여신의 가호
	{
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+34)	// 맹수의 발톱
	{
		Position[0] += 0.01f;
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+35)	// 뿔피리 조각
	{
		Position[0] += 0.01f;
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+36)	// 부러진 뿔피리
	{
		Position[0] += 0.01f;
		Position[1] += 0.05f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+37)	// 펜릴의 뿔피리
	{
		Position[0] += 0.01f;
		Position[1] += 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#ifdef CSK_PCROOM_ITEM
	else if(Type >= MODEL_POTION+55 && Type <= MODEL_POTION+57
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		|| Type >= MODEL_POTION+157 && Type <= MODEL_POTION+159
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
		)
	{
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif // CSK_PCROOM_ITEM
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
	else if(Type == MODEL_POTION+64
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
			|| Type == MODEL_POTION+153
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
		)
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
// 	else if(Type == MODEL_WING+36)	// 폭풍의날개(흑기사)
// 	{
// 		Position[1] -= 0.35f;
// 		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
// 	}
	else if(Type == MODEL_WING+37)	// 시공의날개(법사)
	{
		Position[1] += 0.05f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+38)	// 환영의날개(요정)
	{
		Position[1] += 0.05f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+39)	// 파멸의날개(마검)
	{
		Position[1] += 0.08f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+40)	// 제왕의망토(다크로드)
	{
		Position[1] += 0.05f;
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
#ifdef ADD_ALICE_WINGS_1
	else if(Type == MODEL_WING+42)	// 절망의날개(소환술사)
	{
		Position[1] += 0.05f;
		Vector(270.f,0.f,2.f,ObjectSelect.Angle);
	}
#endif	// ADD_ALICE_WINGS_1
#ifdef CSK_FREE_TICKET
	// 아이템 위치와 각도 세팅
	else if(Type == MODEL_HELPER+46)	// 데빌스퀘어 자유입장권
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+47)	// 블러드캐슬 자유입장권
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_HELPER+48)	// 칼리마 자유입장권
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);	
	}
#endif // CSK_FREE_TICKET
#ifdef CSK_CHAOS_CARD
	// 아이템 위치와 각도 세팅
	else if(Type == MODEL_POTION+54)	// 카오스카드
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif // CSK_CHAOS_CARD
#ifdef CSK_RARE_ITEM
	// 아이템 위치와 각도세팅
	else if(Type == MODEL_POTION+58)// 희귀 아이템 티켓( 부분 1차 )
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
#endif // CSK_RARE_ITEM
#ifdef CSK_LUCKY_CHARM
	else if( Type == MODEL_POTION+53 )// 행운의 부적
	{
		Position[1] += 0.042f;
		Vector(180.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //CSK_LUCKY_CHARM
#ifdef CSK_LUCKY_SEAL
#ifdef PBG_FIX_ITEMANGLE
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
#else //PBG_FIX_ITEMANGLE
	else if( Type == MODEL_HELPER+43 )// 행운의 인장
	{
		Position[1] += 0.082f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
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
#endif //PBG_FIX_ITEMANGLE
#endif //CSK_LUCKY_SEAL
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
	else if( Type >= MODEL_POTION+70 && Type <= MODEL_POTION+71 )
	{
		Position[0] += 0.01f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_ELITE_ITEM
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
	else if( Type >= MODEL_POTION+72 && Type <= MODEL_POTION+77 )
	{
		Position[1] += 0.08f;
		Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_SCROLL_ITEM
#ifdef PSW_SEAL_ITEM               // 이동 인장
	else if( Type == MODEL_HELPER+59 )
	{
		Position[0] += 0.01f;
		Position[1] += 0.02f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_SEAL_ITEM
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
	else if( Type >= MODEL_HELPER+54 && Type <= MODEL_HELPER+58 )
	{
  		Position[1] -= 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_FRUIT_ITEM
#ifdef PSW_SECRET_ITEM             // 강화의 비약
	else if( Type >= MODEL_POTION+78 && Type <= MODEL_POTION+82 )
	{
		Position[1] += 0.01f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_SECRET_ITEM
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
	else if( Type == MODEL_HELPER+60 )
	{
		Position[1] -= 0.06f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_INDULGENCE_ITEM
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
	else if( Type == MODEL_HELPER+61 )// 환영의 사원 자유 입장권
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_CURSEDTEMPLE_FREE_TICKET
#ifdef PSW_RARE_ITEM
	else if(Type == MODEL_POTION+83)// 희귀 아이템 티켓( 부분 2차 )
	{
		Position[1] += 0.06f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_RARE_ITEM
#ifdef PSW_CHARACTER_CARD 
	else if(Type == MODEL_POTION+91) // 캐릭터 카드
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif // PSW_CHARACTER_CARD
#ifdef PSW_NEW_CHAOS_CARD
	else if(Type == MODEL_POTION+92) // 카오스카드 골드
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+93) // 카오스카드 레어
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if(Type == MODEL_POTION+95) // 카오스카드 미니
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif // PSW_NEW_CHAOS_CARD
#ifdef PSW_NEW_ELITE_ITEM
	else if( Type == MODEL_POTION+94 ) // 엘리트 중간 치료 물약
	{
		Position[0] += 0.01f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_NEW_ELITE_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
	else if( Type >= MODEL_POTION+84 && Type <= MODEL_POTION+90 )
	{
		if( Type == MODEL_POTION+84 )  // 벚꽃상자
		{
			Position[1] += 0.01f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+85 )  // 벚꽃술
		{
			Position[1] -= 0.01f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+86 )  // 벚꽃경단
		{
			Position[1] += 0.01f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+87 )  // 벚꽃잎
		{
			Position[1] += 0.01f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+88 )  // 흰색 벚꽃
		{
			Position[1] += 0.015f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+89 )  // 붉은색 벚꽃
		{
			Position[1] += 0.015f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
		else if( Type == MODEL_POTION+90 )  // 노란색 벚꽃
		{
			Position[1] += 0.015f;
			Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
		}
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef PSW_ADD_PC4_SEALITEM
	else if(Type == MODEL_HELPER+62)
	{
#ifdef PBG_FIX_ITEMANGLE
		Position[0] += 0.01f;
		Position[1] -= 0.03f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
#else //PBG_FIX_ITEMANGLE
		Position[0] += 0.01f;
		Position[1] += 0.08f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
#endif //PBG_FIX_ITEMANGLE
	}
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SEALITEM
	else if(Type == MODEL_HELPER+63)
	{
		Position[0] += 0.01f;
		Position[1] += 0.082f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SCROLLITEM
	else if(Type >= MODEL_POTION+97 && Type <= MODEL_POTION+98)
	{
		Position[1] += 0.09f;
		Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
	else if( Type == MODEL_POTION+96 ) 
	{
#ifdef PBG_FIX_ITEMANGLE
		Position[1] -= 0.013f;
		Position[0] += 0.003f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
#else //PBG_FIX_ITEMANGLE
		Position[1] += 0.13f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
#endif //PBG_FIX_ITEMANGLE
	}
#endif //PSW_ADD_PC4_CHAOSCHARMITEM
#ifdef LDK_ADD_PC4_GUARDIAN
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
#endif //LDK_ADD_PC4_GUARDIAN
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
	// 엘리트 해골전사 변신반지 각도 조절
	else if(Type == MODEL_HELPER+39)	// 엘리트 해골전사 변신반지
	{
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#ifdef CSK_LUCKY_SEAL
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
#endif //CSK_LUCKY_SEAL
	// 할로윈 이벤트 변신반지 각도 조절
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
	// GM 변신반지 각도 조절
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
	else if( Type==MODEL_POTION+43 || Type==MODEL_POTION+44 )	// 제련석 인벤토리 위치 조정.
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
	else if(Type==MODEL_POTION+12)//이밴트 아이템
	{
		switch(Level)
		{
		case 0:Vector(180.f,0.f,0.f,ObjectSelect.Angle);break;
		case 1:Vector(270.f,90.f,0.f,ObjectSelect.Angle);break;
		case 2:Vector(90.f,0.f,0.f,ObjectSelect.Angle);break;
		}
	}
	else if(Type==MODEL_EVENT+5)            //  폭죽. /마법 주머니.
	{
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
#else // SELECTED_LANGUAGE == LANGUAGE_KOREAN
		Vector(270.f,180.f,0.f,ObjectSelect.Angle);
#endif // SELECTED_LANGUAGE == LANGUAGE_KOREAN
	}
	else if(Type==MODEL_EVENT+6)            //  사랑의 하트
	{
		Vector(270.f,90.f,0.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_EVENT+7)            //  사랑의 올리브
	{
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type==MODEL_POTION+20)          //  사랑의 묘약
	{
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
    else if ( Type==MODEL_POTION+27 )    //  고대의 금속.
    {
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
    }
	else if ( Type == MODEL_POTION+63 )	// 폭죽
	{
		Position[1] += 0.08f;
		Vector(-50.f,-60.f,0.f,ObjectSelect.Angle);
	}
	else if ( Type == MODEL_POTION+52)	// GM 선물상자
	{
		//Position[1] += 0.08f;
		Vector(270.f,-25.f,0.f,ObjectSelect.Angle);
	}
#ifdef _PVP_MURDERER_HERO_ITEM
    else if ( Type==MODEL_POTION+30 )    // 징표
    {
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
    }
#endif// _PVP_MURDERER_HERO_ITEM
	else if(Type >= MODEL_ETC+19 && Type <= MODEL_ETC+27)	// 양피지들
	{
		Position[0] += 0.03f;
		Position[1] += 0.03f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_WING+7)	// 회오리베기 구슬
	{
		Position[0] += 0.005f;
		Position[1] -= 0.015f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+10)		// 넝쿨갑옷
	{
		Position[1] -= 0.1f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_PANTS+10)		// 넝쿨바지
	{
		Position[1] -= 0.08f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_ARMOR+11)		// 실크갑옷
	{
		Position[1] -= 0.1f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type == MODEL_PANTS+11)		// 실크바지
	{
		Position[1] -= 0.08f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	else if(Type == MODEL_WING+44)	// 파괴의일격 구슬
	{
		Position[0] += 0.005f;
		Position[1] -= 0.015f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	else if(Type == MODEL_WING+46
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
										|| Type==MODEL_WING+45
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
		)	// 회복 구슬
	{
		Position[0] += 0.005f;
		Position[1] -= 0.015f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
    else
	{
		Vector(270.f,-10.f,0.f,ObjectSelect.Angle);
	}
#ifdef ADD_SEED_SPHERE_ITEM
	
	// if-else if가 128개 넘어가면 컴파일 에러남! 추가할때 이 아래로 추가하던지 아니면 구조를 고쳐야 함 ♤
	if(Type >= MODEL_WING+60 && Type <= MODEL_WING+65)	// 시드
	{
		Vector(10.f,-10.f,10.f,ObjectSelect.Angle);
	}
	else if(Type >= MODEL_WING+70 && Type <= MODEL_WING+74)	// 스피어
	{
		Vector(0.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if(Type >= MODEL_WING+100 && Type <= MODEL_WING+129)	// 시드스피어
	{
		Vector(0.f,0.f,0.f,ObjectSelect.Angle);
	}
#endif	// ADD_SEED_SPHERE_ITEM

#ifdef LDK_ADD_RUDOLPH_PET //루돌프 펫 ... limit 걸림.....
	else if( Type == MODEL_HELPER+67 )
	{
		Position[1] -= 0.05f;
		Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
	}
#endif //LDK_ADD_RUDOLPH_PET
#ifdef YDG_ADD_SKELETON_PET
	else if( Type == MODEL_HELPER+123 )	// 스켈레톤 펫
	{
		Position[1] -= 0.05f;
		Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_SKELETON_PET
#ifdef YDG_ADD_HEALING_SCROLL
	else if(Type == MODEL_POTION+140)	// 치유의 스크롤
	{
		Position[1] += 0.09f;
		Vector(0.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_HEALING_SCROLL
#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
	else if(Type >= MODEL_POTION+145 && Type <= MODEL_POTION+150)
	{
		Position[0] += 0.010f;
		Position[1] += 0.040f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
	else if (Type == MODEL_POTION+151 || Type == MODEL_POTION+152)	// 1레벨 의뢰서, 1레벨 의뢰 완료 확인서
	{
		Position[0] += 0.02f;
		Position[1] += 0.13f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
	else if (Type == MODEL_POTION+155)	// 탄탈로스의 갑옷
	{
		Position[1] += 0.120f;
	}
	else if (Type == MODEL_POTION+156)	// 잿더미 도살자의 몽둥이
	{
		Position[0] += 0.040f;
		Position[1] += 0.110f;
		Vector(180.f, 270.f, 15.f, ObjectSelect.Angle);
	}
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
	// 아이템 위치와 각도 세팅
	else if(Type >= MODEL_HELPER+125 && Type <= MODEL_HELPER+127)	//도플갱어, 바르카, 바르카제7맵 자유입장권
	{
		Position[0] += 0.007f;
		Position[1] -= 0.035f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
#ifdef ASG_ADD_CHARGED_CHANNEL_TICKET
	else if (Type == MODEL_HELPER+124)	// 유료채널 입장권.
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //ASG_ADD_CHARGED_CHANNEL_TICKET
#ifdef PJH_ADD_PANDA_PET 
	else if( Type == MODEL_HELPER+80 )
	{
		Position[1] -= 0.05f;
		Vector(270.f, 40.f, 0.f, ObjectSelect.Angle);
	}
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET	//유니콘
	else if( Type == MODEL_HELPER+106 )
	{
		Position[0] += 0.01f;
		Position[1] -= 0.05f;
		Vector(255.f, 45.f, 0.f, ObjectSelect.Angle);
	}
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef LDK_ADD_SNOWMAN_CHANGERING
	else if( Type == MODEL_HELPER+68 )
	{
		Position[0] += 0.02f;
		Position[1] -= 0.02f;
		Vector(300.f, 10.f, 20.f, ObjectSelect.Angle);
	}
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
	else if( Type == MODEL_HELPER+76 )
	{
//		Position[0] += 0.02f;
		Position[1] -= 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	else if( Type == MODEL_HELPER+122 )	// 스켈레톤 변신반지
	{
		Position[0] += 0.01f;
		Position[1] -= 0.035f;
		Vector(290.f, -20.f, 0.f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM	
	else if( Type == MODEL_HELPER+128 )	// 매조각상
	{
		Position[0] += 0.017f;
		Position[1] -= 0.053f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+129 )	// 양조각상
	{
		Position[0] += 0.012f;
		Position[1] -= 0.045f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+134 )	// 편자
	{
		Position[0] += 0.005f;
		Position[1] -= 0.033f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
#endif	//LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	else if( Type == MODEL_HELPER+130 )	// 오크참
	{
		Position[0] += 0.007f;
		Position[1] += 0.005f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+131 )	// 메이플
	{
		Position[0] += 0.017f;
		Position[1] -= 0.053f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+132 )	// 골든오크참
	{
		Position[0] += 0.007f;
		Position[1] += 0.045f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_HELPER+133 )	// 골든메이플
	{
		Position[0] += 0.017f;
		Position[1] -= 0.053f;
		Vector(270.f, -20.f, 0.f, ObjectSelect.Angle);
	}
#endif	//LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
	else if( Type == MODEL_HELPER+69 )	// 부활의 부적
	{
		Position[0] += 0.005f;
		Position[1] -= 0.05f;
		Vector(270.f, -30.f, 0.f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
	else if( Type == MODEL_HELPER+70 )	// 이동의 부적
	{
		Position[0] += 0.040f;
		Position[1] -= 0.000f;
		Vector(270.f, -0.f, 70.f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef ASG_ADD_CS6_GUARD_CHARM
	else if (Type == MODEL_HELPER+81)	// 수호의부적
	{
		Position[0] += 0.005f;
		Position[1] += 0.035f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
	else if (Type == MODEL_HELPER+82)	// 아이템보호부적
	{
		Position[0] += 0.005f;
		Position[1] += 0.035f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
	else if (Type == MODEL_HELPER+93)	// 상승의인장마스터
	{
		Position[0] += 0.005f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
	else if (Type == MODEL_HELPER+94)	// 풍요의인장마스터
	{
		Position[0] += 0.005f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef PBG_ADD_SANTAINVITATION
	//산타마을의 초대장.
	else if( Type == MODEL_HELPER+66 )
	{
		Position[0] += 0.01f;
		Position[1] -= 0.05f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif //PBG_ADD_SANTAINVITATION
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	//행운의 동전
	else if( Type == MODEL_POTION+100 )
	{
		Position[0] += 0.01f;
		Position[1] -= 0.05f;
		Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_FIRECRACKER_ITEM
	else if (Type == MODEL_POTION+99)	// 크리스마스 폭죽
	{
		Position[0] += 0.02f;
		Position[1] -= 0.03f;
		//Vector(270.f,0.f,30.f,ObjectSelect.Angle);
		Vector(290.f,-40.f,0.f,ObjectSelect.Angle);
	}
#endif	// YDG_ADD_FIRECRACKER_ITEM
#ifdef LDK_ADD_GAMBLERS_WEAPONS
	else if( Type == MODEL_STAFF+33 )	// 겜블 레어 지팡이
	{
		Position[0] += 0.02f;
		Position[1] -= 0.06f;
		Vector(180.f,90.f,10.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_STAFF+34 )	// 겜블 레어 지팡이(소환술사용)
	{
		Position[1] -= 0.05f;
		Vector(180.f,90.f,10.f,ObjectSelect.Angle);
	}
	else if( Type == MODEL_SPEAR+11 )	// 겜블 레어 낫
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
#endif //LDK_ADD_GAMBLERS_WEAPONS
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
	else if(Type == MODEL_WING+47)	// 플레임스트라이크 구슬
	{
		Position[0] += 0.005f;
		Position[1] -= 0.015f;
		Vector(270.f,0.f,0.f,ObjectSelect.Angle);
	}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef LDK_ADD_GAMBLE_RANDOM_ICON
	//겜블 상점 아이콘 모델 번호 수정 해야됨
	else if ( Type==MODEL_HELPER+71 || Type==MODEL_HELPER+72 || Type==MODEL_HELPER+73 || Type==MODEL_HELPER+74 || Type==MODEL_HELPER+75 )
	{
		Position[1] += 0.07f;
      	Vector(270.f,180.f,0.f,ObjectSelect.Angle);
		if(Select != 1)
		{
			ObjectSelect.Angle[1] = WorldTime*0.2f;
		}
	}
#endif //LDK_ADD_GAMBLE_RANDOM_ICON
#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING	// 날개 조합 100% 성공 부적
	else if( Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN
			&& Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END )
	{
		Position[1] -= 0.03f;
		Vector(-90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif //LDS_ADD_CS6_CHARM_MIX_ITEM_WING
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH
	else if(Type == MODEL_HELPER+96)		// 강함의 인장 (PC방 아이템, 일본 6차 컨텐츠)
	{
		Position[0] -= 0.001f;
		Position[1] += 0.028f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH
#ifdef PBG_ADD_CHARACTERCARD
	// 마검 다크 소환술사 카드
	else if(Type == MODEL_HELPER+97 || Type == MODEL_HELPER+98 || Type == MODEL_POTION+91)
	{
		Position[1] -= 0.04f;
		Position[0] += 0.002f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
	else if(Type == MODEL_HELPER+99)
	{
		Position[0] += 0.002f;
		Position[1] += 0.025f;
		Vector(270.0f, 180.0f, 45.0f, ObjectSelect.Angle);
	}
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PBG_ADD_SECRETITEM
	//활력의비약(최하급/하급/중급/상급)
	else if(Type >= MODEL_HELPER+117 && Type <= MODEL_HELPER+120)
	{
		Position[0] += 0.01f;
		Position[1] -= 0.05f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif //PBG_ADD_SECRETITEM
#ifdef YDG_ADD_DOPPELGANGER_ITEM
	else if ( Type==MODEL_POTION+110 )	// 차원의표식
	{
		Position[0] += 0.005f;
		Position[1] -= 0.02f;
	}
	else if ( Type==MODEL_POTION+111 )	// 차원의마경
	{
		Position[0] += 0.01f;
		Position[1] -= 0.02f;
	}
#endif	// YDG_ADD_DOPPELGANGER_ITEM
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
	else if(Type == MODEL_HELPER+107)
	{
		// 치명마법반지
		Position[0] -= 0.0f;
		Position[1] += 0.0f;
		Vector(90.0f, 225.0f, 45.0f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
#ifdef YDG_ADD_CS7_MAX_AG_AURA
	else if(Type == MODEL_HELPER+104)
	{
		// AG증가 오라
		Position[0] += 0.01f;
		Position[1] -= 0.03f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_CS7_MAX_AG_AURA
#ifdef YDG_ADD_CS7_MAX_SD_AURA
	else if(Type == MODEL_HELPER+105)
	{
		// SD증가 오라
		Position[0] += 0.01f;
		Position[1] -= 0.03f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
	else if(Type == MODEL_HELPER+103)
	{
		// 파티 경험치 증가 아이템
		Position[0] += 0.01f;
		Position[1] += 0.01f;
		Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
	else if(Type == MODEL_POTION+133)
	{
		// 엘리트 SD회복 물약
		Position[0] += 0.01f;
		Position[1] -= 0.0f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
#ifdef LDK_ADD_EMPIREGUARDIAN_ITEM
	else if( MODEL_POTION+101 <= Type && Type <= MODEL_POTION+109 )
	{
		switch(Type)
		{
		case MODEL_POTION+101: // 의문의쪽지
			{
				Position[0] += 0.005f;
				//Position[1] -= 0.02f;
			}break;
		case MODEL_POTION+102: // 가이온의 명령서
			{
				Position[0] += 0.005f;
				Position[1] += 0.05f;
				Vector(0.0f, 0.0f, 30.0f, ObjectSelect.Angle);
			}break;
		case MODEL_POTION+103: // 세크로미콘 조각
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
		case MODEL_POTION+109: // 세크로미콘
			{
				Position[0] += 0.005f;
				Position[1] += 0.05f;
				Vector(0.0f, 0.0f, 0.0f, ObjectSelect.Angle);
			}break;
		}
	}
#endif //LDK_ADD_EMPIREGUARDIAN_ITEM
#if defined(LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE) || defined(LDS_ADD_INGAMESHOP_ITEM_RINGRUBY) || defined(LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ) || defined(LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST)	// 신규 사파이어(푸른색)링	// MODEL_HELPER+109
	else if( Type >= MODEL_HELPER+109 && Type <= MODEL_HELPER+112 )	// 사파이어(푸른색)링, 루비(붉은색)링, 토파즈(주황)링, 자수정(보라색)링
	{
		// 신규 사파이어(푸른색)링
		Position[0] += 0.025f;
		Position[1] -= 0.035f;
		Vector(270.0f, 25.0f, 25.0f, ObjectSelect.Angle);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST		// 신규 자수정(보라색)링		// MODEL_HELPER+112
#if defined(LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY) || defined(LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD) || defined(LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE)
	else if( Type >= MODEL_HELPER+113 && Type <= MODEL_HELPER+115 )	// 루비(붉은색), 에메랄드(푸른), 사파이어(녹색) 목걸이
	{
		// 루비(붉은색), 에메랄드(푸른), 사파이어(녹색) 목걸이
		Position[0] += 0.005f;
		Position[1] -= 0.00f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif // defined(LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY) || defined(LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD) || defined(LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE)	// 루비(붉은색), 에메랄드(푸른), 사파이어(녹색) 목걸이
#if defined(LDS_ADD_INGAMESHOP_ITEM_KEYSILVER) || defined(LDS_ADD_INGAMESHOP_ITEM_KEYGOLD)
	else if( Type >= MODEL_POTION+112 && Type <= MODEL_POTION+113 )	// 키(실버), 키(골드)
	{
		// 키(실버), 키(골드)
 		Position[0] += 0.05f;
 		Position[1] += 0.009f;
		Vector(270.0f, 180.0f, 45.0f, ObjectSelect.Angle);
	}
#endif // defined(LDS_ADD_INGAMESHOP_ITEM_KEYSILVER) || defined(LDS_ADD_INGAMESHOP_ITEM_KEYGOLD)
#ifdef LDK_ADD_INGAMESHOP_GOBLIN_GOLD
	// 고블린금화
	else if( Type == MODEL_POTION+120 )
	{
		Position[0] += 0.01f;
		Position[1] += 0.05f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
		
	}
#endif //LDK_ADD_INGAMESHOP_GOBLIN_GOLD
#ifdef LDK_ADD_INGAMESHOP_PACKAGE_BOX				// 패키지 상자A-F
	else if( MODEL_POTION+134 <= Type && Type <= MODEL_POTION+139 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.05f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif //LDK_ADD_INGAMESHOP_PACKAGE_BOX
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
	else if( Type == MODEL_HELPER+116 )
	{
#ifdef PBG_FIX_ITEMANGLE
		Position[1] -= 0.03f;
		Position[0] += 0.005f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
#else //PBG_FIX_ITEMANGLE
		Position[1] += 0.08f;
		Vector(90.f, 0.f, 0.f, ObjectSelect.Angle);
#endif //PBG_FIX_ITEMANGLE
	}
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
	else if( Type >= MODEL_POTION+114 && Type <= MODEL_POTION+119 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.06f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
#ifdef LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
	else if( Type >= MODEL_POTION+126 && Type <= MODEL_POTION+129 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.06f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
#ifdef LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
	else if( Type >= MODEL_POTION+130 && Type <= MODEL_POTION+132 )
	{
		Position[0] += 0.00f;
		Position[1] += 0.06f;
		Vector(270.0f, 0.0f, 0.0f, ObjectSelect.Angle);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
	else if( Type == MODEL_HELPER+121 )
	{
		Position[1] -= 0.04f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
#ifdef LDS_MR0_MOD_PATIALPHYSIQMODEL_PHYSICPROCESS_FIX	// 법사 블랙 소울 바지/그랜드소울 바지 // MODEL_PANTS +22 
	else if( Type == MODEL_PANTS +22 || Type == MODEL_PANTS +18 )
	{
		vec3_t		v3Angle;
		VectorCopy(ObjectSelect.Angle, v3Angle);
		
		v3Angle[1] = v3Angle[1] + 10.0f;
		
		VectorCopy(v3Angle, ObjectSelect.Angle);
	}	
#endif // LDS_MR0_MOD_PATIALPHYSIQMODEL_PHYSICPROCESS_FIX
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
#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 인벤토리 위치 조정
	else if( Type >= MODEL_HELPER+135 && Type <= MODEL_HELPER+145 ) // 럭키 아이템 티켓
	{
		Position[1] += 0.02f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
	else if( Type == MODEL_POTION+160 || Type == MODEL_POTION+161 )	// 상승의 보석, 연장의 보석
	{
		Position[1] += 0.05f;
		Vector(270.f, 0.f, 0.f, ObjectSelect.Angle);
	}
#endif // LEM_ADD_LUCKYITEM
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX	// 보석 조합 인벤토리 위치, 각도 조정
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
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
	// =====================================================================================/
	// ObjectSelect 처리 부분 1. 일반 아이템


	// ObjectSelect 처리 부분 2. 소켓 아이템
	// =====================================================================================
#ifdef ADD_SOCKET_ITEM			
	// 인벤토리 안의 아이템 위치를 수정(장착된 아이템도 수정됨)
	switch (Type)
	{
	case MODEL_SWORD+26:		// 플랑베르주
		{
			Position[0] -= 0.02f;				// 가로
			Position[1] += 0.04f;				// 높이
			Vector(180.f,270.f,10.f,ObjectSelect.Angle);
		}break;
	case MODEL_SWORD+27:		// 소드브레이커
		{
			Vector(180.f,270.f,15.f,ObjectSelect.Angle);
		}break;
	case MODEL_SWORD+28:		// 룬바스타드
		{
			Position[1] += 0.02f;
			Vector(180.f,270.f,10.f,ObjectSelect.Angle);
		}break;
	case MODEL_MACE+16:			// 프로스트메이스
		{
			Position[0] -= 0.02f;
			Vector(180.f,270.f,15.f,ObjectSelect.Angle);
		}
		break;
	case MODEL_MACE+17:			// 앱솔루트셉터
		{
			Position[0] -= 0.02f;
			Position[1] += 0.04f;
			Vector(180.f,270.f,15.f,ObjectSelect.Angle);
		}break;
// 	case MODEL_BOW+23:			// 다크스팅거
// 		{
// 			Position[0] -= 0.04f;
// 			Position[1] += 0.12f;
// 			Vector(180.f, -90.f, 15.f,ObjectSelect.Angle);
// 		}break;
	case MODEL_STAFF+30:			// 데들리스테프
		{
			Vector(180.f,90.f,10.f,ObjectSelect.Angle);
		}break;
	case MODEL_STAFF+31:			// 인베리알스테프
		{
			Vector(180.f,90.f,10.f,ObjectSelect.Angle);
		}break;
	case MODEL_STAFF+32:			// 소울브링거
		{
			Vector(180.f,90.f,10.f,ObjectSelect.Angle);
		}break;
	}
#endif // ADD_SOCKET_ITEM
	// =====================================================================================/
	// ObjectSelect 처리 부분 2. 소켓 아이템


	// ObjectSelect 처리 부분 3. 기타 아이템
	// =====================================================================================
#ifdef LDK_FIX_CAOS_THUNDER_STAFF_ROTATION
	//inventory 카오스 번개 지팡이 회전값 이상(2008.08.12)
	switch(Type)
	{
	case MODEL_STAFF+7:
		{
			Vector(0.f,0.f,205.f,ObjectSelect.Angle);
		}break;
	}
#endif //LDK_FIX_CAOS_THUNDER_STAFF_ROTATION
	// =====================================================================================/
	// ObjectSelect 처리 부분 3. 기타 아이템


#ifdef KJH_FIX_20080904_INVENTORY_ITEM_RENDER
	switch(Type)
	{
	case MODEL_WING+8:			// 치료구슬
	case MODEL_WING+9:			// 방어력향상구슬
	case MODEL_WING+10:			// 공격력향상구슬
	case MODEL_WING+11:			// 소환구슬
		{
			Position[0] += 0.005f;
			Position[1] -= 0.02f;
		}break;
	case MODEL_POTION+21:		// 성주의표식
		{
			Position[0] += 0.005f;
			Position[1] -= 0.005f;
		}break;
	case MODEL_POTION+13:		// 축석
	case MODEL_POTION+14:		// 영석
	case MODEL_POTION+22:		// 창석
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
	case MODEL_POTION+154:		// 칼트석
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
		{			
			Position[0] += 0.005f;
			Position[1] += 0.015f;
		}break;
	}
#endif // KJH_FIX_20080904_INVENTORY_ITEM_RENDER

	//선택 되었을때...--;;
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
	else if(ObjectSelect.Type==MODEL_POTION+12)//이밴트 아이템
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

#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 인벤토리 위치 조정
		if( Check_LuckyItem( Type - MODEL_ITEM ))				b->BodyHeight-=10.0f;
		if( Type == MODEL_HELM+65 || Type == MODEL_HELM+70 )	Position[0] += 0.04f;
#endif // LEM_ADD_LUCKYITEM
	}
	else if(Type >= MODEL_ARMOR && Type<MODEL_ARMOR+MAX_ITEM_INDEX)
	{
		b->BodyHeight = -100.f;

#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 인벤토리 위치 조정
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
#ifdef LDS_FIX_ELFHELM_CILPIDREI_RESIZE			// 실피드레이 헬멧 SIZE 제대로 적용 하기 위함.
			if( Type == MODEL_HELM+31)			// 실피드레이 헬멧인경우 scale 값 조정
				Scale = 0.007f;
#endif // LDS_FIX_ELFHELM_CILPIDREI_RESIZE

#ifdef LEM_ADD_LUCKYITEM	// 럭키아이템 인벤토리 스케일 조정 ( 소환술사HELM )
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
#ifndef LDS_FIX_ELFHELM_CILPIDREI_RESIZE	// 정리할 때 지워야 하는 소스	
		else if( Type == MODEL_HELM+31)				// 실피드레이 헬멧 SIZE 조정 값
			Scale = 0.007f;
#endif // LDS_FIX_ELFHELM_CILPIDREI_RESIZE // 정리할 때 지워야 하는 소스
		else if (Type == MODEL_ARMOR+30)
			Scale = 0.0035f;
		else if (Type == MODEL_ARMOR+32)
			Scale = 0.0035f;
		else if (Type == MODEL_ARMOR+29)
			Scale = 0.0033f;

		//$ 크라이울프 아이템(장비)
		if(Type == MODEL_ARMOR+34)	// 흑기사 갑옷
			Scale = 0.0032f;
		else if(Type == MODEL_ARMOR+35)	// 흑마법사 갑옷
			Scale = 0.0032f;
		else if(Type == MODEL_GLOVES+38)	// 마검사 장갑
			Scale = 0.0032f;
	}
	else
	{
        if(Type==MODEL_WING+6)     //  마검사 날개.
            Scale = 0.0015f;
#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX	// 보석 조합 인벤토리 스케일
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
#else // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
		else if(Type==MODEL_COMPILED_CELE || Type==MODEL_COMPILED_SOUL)
			Scale = 0.004f;
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
		else if ( Type>=MODEL_WING+32 && Type<=MODEL_WING+34)
		{
			Scale = 0.001f;
			Position[1] -= 0.05f;
		}
#ifdef ADD_SEED_SPHERE_ITEM
		else if(Type >= MODEL_WING+60 && Type <= MODEL_WING+65)	// 시드
			Scale = 0.0022f; 
		else if(Type >= MODEL_WING+70 && Type <= MODEL_WING+74)	// 스피어
			Scale = 0.0017f; 
		else if(Type >= MODEL_WING+100 && Type <= MODEL_WING+129)	// 시드스피어
			Scale = 0.0017f; 
#endif	// ADD_SEED_SPHERE_ITEM
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

//			Position[1] += 0.05f;
//   			Vector(0.f,ObjectSelect.Angle[1],0.f,ObjectSelect.Angle);
		}

#ifdef GIFT_BOX_EVENT
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
#endif
        else if(Type==MODEL_POTION+21)	// 레나
			Scale = 0.002f;
		else if(Type == MODEL_BOW+19)
			Scale = 0.002f;
        else if(Type==MODEL_EVENT+11)	// 스톤
			Scale = 0.0015f;
        else if ( Type==MODEL_HELPER+4 )    //  다크호스
			Scale = 0.0015f;
        else if ( Type==MODEL_HELPER+5 )    //  다크스피릿.
			Scale = 0.005f;
        else if ( Type==MODEL_HELPER+30 )   //  망토.    
			Scale = 0.002f;
        else if ( Type==MODEL_EVENT+16 )    //  군주의 문장.
 			Scale = 0.002f;
#ifdef MYSTERY_BEAD
		else if ( Type==MODEL_EVENT+19 )	//. 신비의구슬
			Scale = 0.0025f;
#endif // MYSTERY_BEAD
		else if(Type==MODEL_HELPER+16)	//. 대천사의 서
			Scale = 0.002f;
		else if(Type==MODEL_HELPER+17)	//. 블러드본
			Scale = 0.0018f;
		else if(Type==MODEL_HELPER+18)	//. 투명망토
			Scale = 0.0018f;
#ifdef CSK_FREE_TICKET
		// 아이템 스케일 정하는 곳
		else if(Type == MODEL_HELPER+46)	// 데빌스퀘어 자유입장권
		{
			Scale = 0.0018f;
		}
		else if(Type == MODEL_HELPER+47)	// 블러드캐슬 자유입장권
		{
			Scale = 0.0018f;
		}
		else if(Type == MODEL_HELPER+48)	// 칼리마 자유입장권
		{
			Scale = 0.0018f;
		}
#endif // CSK_FREE_TICKET
#ifdef CSK_CHAOS_CARD
		// 아이템 스케일 정하는 곳
		else if(Type == MODEL_POTION+54)	// 카오스카드
		{
			Scale = 0.0024f;
		}
#endif // CSK_CHAOS_CARD
#ifdef CSK_RARE_ITEM
		// 아이템 스케일 정하는 곳
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
#endif // CSK_RARE_ITEM
#ifdef CSK_LUCKY_CHARM
		else if( Type == MODEL_POTION+53 )// 행운의 부적
		{
			Scale = 0.00078f;
		}
#endif //CSK_LUCKY_CHARM
#ifdef CSK_LUCKY_SEAL
		else if( Type == MODEL_HELPER+43 || Type == MODEL_HELPER+44 || Type == MODEL_HELPER+45 )
		{
			Scale = 0.0021f;
		}
#endif //CSK_LUCKY_SEAL
#ifdef PSW_ELITE_ITEM              // 엘리트 물약
		else if( Type >= MODEL_POTION+70 && Type <= MODEL_POTION+71 )
		{
			Scale = 0.0028f;
		}
#endif //PSW_ELITE_ITEM
#ifdef PSW_SCROLL_ITEM             // 엘리트 스크롤
		else if( Type >= MODEL_POTION+72 && Type <= MODEL_POTION+77 )
		{
			Scale = 0.0025f;
		}
#endif //PSW_SCROLL_ITEM
#ifdef PSW_SEAL_ITEM               // 이동 인장
		else if( Type == MODEL_HELPER+59 )
		{
			Scale = 0.0008f;
		}
#endif //PSW_SEAL_ITEM
#ifdef PSW_FRUIT_ITEM              // 리셋 열매
		else if( Type >= MODEL_HELPER+54 && Type <= MODEL_HELPER+58 )
		{
			Scale = 0.004f;
		}
#endif //PSW_FRUIT_ITEM
#ifdef PSW_SECRET_ITEM             // 강화의 비약
		else if( Type >= MODEL_POTION+78 && Type <= MODEL_POTION+82 )
		{
			Scale = 0.0025f;
		}
#endif //PSW_SECRET_ITEM
#ifdef PSW_INDULGENCE_ITEM         // 면죄부
		else if( Type == MODEL_HELPER+60 )
		{
			Scale = 0.005f;
		}
#endif //PSW_INDULGENCE_ITEM
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
		else if( Type == MODEL_HELPER+61 )
		{
			Scale = 0.0018f;
		}
#endif //PSW_CURSEDTEMPLE_FREE_TICKET
#ifdef PSW_RARE_ITEM
		else if(Type == MODEL_POTION+83)
		{
			Scale = 0.0009f;
		}
#endif //PSW_RARE_ITEM
#ifdef CSK_LUCKY_SEAL
		else if( Type == MODEL_HELPER+43 || Type == MODEL_HELPER+44 || Type == MODEL_HELPER+45 )
		{
			Scale = 0.0021f;
		}
#endif //CSK_LUCKY_SEAL
#ifdef PSW_CHARACTER_CARD 
		else if(Type == MODEL_POTION+91) // 캐릭터 카드
		{
			Scale = 0.0034f;
		}
#endif // PSW_CHARACTER_CARD
#ifdef PSW_NEW_CHAOS_CARD
		else if(Type == MODEL_POTION+92) // 카오스카드 골드
		{
			Scale = 0.0024f;
		}
		else if(Type == MODEL_POTION+93) // 카오스카드 레어
		{
			Scale = 0.0024f;
		}
		else if(Type == MODEL_POTION+95) // 카오스카드 미니
		{
			Scale = 0.0024f;
		}
#endif // PSW_NEW_CHAOS_CARD
#ifdef PSW_NEW_ELITE_ITEM
		else if( Type == MODEL_POTION+94 ) // 엘리트 중간 치료 물약
		{
			Scale = 0.0022f;
		}
#endif //PSW_NEW_ELITE_ITEM
#ifdef CSK_EVENT_CHERRYBLOSSOM
		else if( Type == MODEL_POTION+84 )  // 벚꽃상자
		{
			Scale = 0.0031f;
		}
		else if( Type == MODEL_POTION+85 )  // 벚꽃술
		{
			Scale = 0.0044f;
		}
		else if( Type == MODEL_POTION+86 )  // 벚꽃경단
		{
			Scale = 0.0031f;
		}
		else if( Type == MODEL_POTION+87 )  // 벚꽃잎
		{
			Scale = 0.0061f;
		}
		else if( Type == MODEL_POTION+88 )  // 흰색 벚꽃
		{
			Scale = 0.0035f;
		}
		else if( Type == MODEL_POTION+89 )  // 붉은색 벚꽃
		{
			Scale = 0.0035f;
		}
		else if( Type == MODEL_POTION+90 )  // 노란색 벚꽃
		{
			Scale = 0.0035f;
		}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef PSW_ADD_PC4_SEALITEM
		else if(Type >= MODEL_HELPER+62 && Type <= MODEL_HELPER+63)
		{
			Scale = 0.002f;
		}
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SCROLLITEM
		else if(Type >= MODEL_POTION+97 && Type <= MODEL_POTION+98)
		{
			Scale = 0.003f;
		}
#endif //PSW_ADD_PC4_SCROLLITEM	
#ifdef PSW_ADD_PC4_CHAOSCHARMITEM
		else if( Type == MODEL_POTION+96 ) 
		{
			Scale = 0.0028f;
		}
#endif //PSW_ADD_PC4_CHAOSCHARMITEM
#ifdef LDK_ADD_PC4_GUARDIAN
		else if( MODEL_HELPER+64 == Type || Type == MODEL_HELPER+65 )
		{
			switch(Type)
			{
			case MODEL_HELPER+64: Scale = 0.0005f; break;
			case MODEL_HELPER+65: Scale = 0.0016f; break;
			}			
		}
#endif //LDK_ADD_PC4_GUARDIAN	
#ifdef LDK_ADD_RUDOLPH_PET
		else if( Type == MODEL_HELPER+67 )
		{
			Scale = 0.0015f;
		}
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
		else if( Type == MODEL_HELPER+80 )
		{
			Scale = 0.0020f;
		}
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_SNOWMAN_CHANGERING
		else if( Type == MODEL_HELPER+68 )
		{
			Scale = 0.0026f;
		}
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
		else if( Type == MODEL_HELPER+76 )
		{
			Scale = 0.0026f;
		}
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_CS5_REVIVAL_CHARM
		else if( Type == MODEL_HELPER+69 )	// 부활의 부적
		{
			Scale = 0.0023f;
		}
#endif	// YDG_ADD_CS5_REVIVAL_CHARM
#ifdef YDG_ADD_CS5_PORTAL_CHARM
		else if( Type == MODEL_HELPER+70 )	// 이동의 부적
		{
			Scale = 0.0018f;
		}
#endif	// YDG_ADD_CS5_PORTAL_CHARM
#ifdef ASG_ADD_CS6_GUARD_CHARM
		else if (Type == MODEL_HELPER+81)	// 수호의부적
			Scale = 0.0012f;
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM 
		else if (Type == MODEL_HELPER+82)	// 아이템보호부적
			Scale = 0.0012f;
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM 
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		else if (Type == MODEL_HELPER+93)	// 상승의인장마스터
			Scale = 0.0021f;
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		else if (Type == MODEL_HELPER+94)	// 풍요의인장마스터
			Scale = 0.0021f;
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
        else if(Type==MODEL_SWORD+19)   //  대천사의 절대검.
        {
            if ( ItemLevel>=0 )
            {
                Scale = 0.0025f;
            }
            else    //  퀘스트 아이템.
            {
                Scale = 0.001f;
                ItemLevel = 0;
            }
        }
        else if(Type==MODEL_STAFF+10)   //  대천사의 절대 지팡이.
        {
            if ( ItemLevel>=0 )
            {
                Scale = 0.0019f;
            }
            else    //  퀘스트 아이템.
            {
                Scale = 0.001f;
                ItemLevel = 0;
            }
        }
        else if(Type==MODEL_BOW+18)     //  대천사의 절대석궁.
        {
            if ( ItemLevel>=0 )
            {
                Scale = 0.0025f;
            }
            else    //  퀘스트 아이템.
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
#ifdef LDK_ADD_GAMBLERS_WEAPONS
		else if( Type == MODEL_MACE+18 )
		{
			Scale = 0.0024f;
		}
#endif //LDK_ADD_GAMBLERS_WEAPONS
		else if(Type == MODEL_EVENT+12)		//. 영광의 반지
		{
			Scale = 0.0012f;
		}
		else if(Type == MODEL_EVENT+13)		//. 다크스톤
		{
			Scale = 0.0025f;
		}
		else if ( Type == MODEL_EVENT+14)	//. 제왕의 반지
		{
			Scale = 0.0028f;
		}
		else if ( Type == MODEL_EVENT+15)	// 마법사의 반지
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
		else if ( Type == MODEL_POTION+63)	// 폭죽
		{
			Scale = 0.007f;
		}
#ifdef YDG_ADD_FIRECRACKER_ITEM
		else if ( Type == MODEL_POTION+99)	// 크리스마스 폭죽
		{
			Scale = 0.0025f;
		}
#endif	// YDG_ADD_FIRECRACKER_ITEM

		else if ( Type == MODEL_POTION+52)	// GM 선물상자
		{
			Scale = 0.0014f;
		}
#ifdef _PVP_MURDERER_HERO_ITEM
        else if ( Type==MODEL_POTION+30 )	// 징표
        {
            Scale = 0.002f;
        }
#endif// _PVP_MURDERER_HERO_ITEM
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
#ifdef CSK_LUCKY_SEAL
		else if( Type == MODEL_HELPER+43 || Type == MODEL_HELPER+44 || Type == MODEL_HELPER+45 )
		{
			Scale = 0.0021f;
		}
#endif //CSK_LUCKY_SEAL
        else if ( Type==MODEL_HELPER+7 )
        {
            Scale = 0.0025f;
        }
        else if ( Type==MODEL_HELPER+11 )
        {
            Scale = 0.0025f;
        }
		//^ 펜릴 스케일 조절
		else if(Type == MODEL_HELPER+32)	// 갑옷 파편
		{
			Scale = 0.0019f;
		}
		else if(Type == MODEL_HELPER+33)	// 여신의 가호
		{
			Scale = 0.004f;
		}
		else if(Type == MODEL_HELPER+34)	// 맹수의 발톱
		{
			Scale = 0.004f;
		}
		else if(Type == MODEL_HELPER+35)	// 뿔피리 조각
		{
			Scale = 0.004f;
		}
		else if(Type == MODEL_HELPER+36)	// 부러진 뿔피리
		{
			Scale = 0.007f;
		}
		else if(Type == MODEL_HELPER+37)	// 펜릴의 뿔피리
		{
			Scale = 0.005f;
		}
		else if( Type == MODEL_BOW+21)
		{
			Scale = 0.0022f;
		}
#ifdef CSK_PCROOM_ITEM
		else if(Type >= MODEL_POTION+55 && Type <= MODEL_POTION+57
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
			|| Type >= MODEL_POTION+157 && Type <= MODEL_POTION+159
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
			)
		{
			Scale = 0.0014f;
		}
#endif // CSK_PCROOM_ITEM
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
		else if(Type == MODEL_POTION+64
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
			|| Type == MODEL_POTION+153
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
			)
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
		//$ 크라이울프 아이템(무기)
		else if(Type == MODEL_SWORD+24)	// 흑기사 검
		{
			Scale = 0.0028f;
		}
		else if(Type == MODEL_BOW+22)	// 요정활
		{
			Scale = 0.0020f;
		}
#ifdef ADD_SOCKET_ITEM
		else if( Type == MODEL_BOW+23 )		// 다크스팅거
		{
			Scale = 0.0032f;
		}
#endif // ADD_SOCKET_ITEM
        else if( Type==MODEL_HELPER+14 || Type==MODEL_HELPER+15 )
        {
            Scale = 0.003f;
        }
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		//행운의 동전
		else if(Type == MODEL_POTION+100)
		{
			Scale = 0.0040f;
		}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		else if (Type == MODEL_POTION+156)
			Scale = 0.0039f;
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
		else if(Type>=MODEL_POTION && Type<MODEL_POTION+MAX_ITEM_INDEX)
		{
      		Scale = 0.0035f;
		}
		else if(Type>=MODEL_SPEAR && Type<MODEL_SPEAR+MAX_ITEM_INDEX)
        {
#ifdef LDK_FIX_INVENTORY_SPEAR_SCALE
			// if문 수정
			if(Type == MODEL_SPEAR+10)	//. 기사 창
				Scale = 0.0018f;
#ifdef LDK_ADD_GAMBLERS_WEAPONS
			else if( Type == MODEL_SPEAR+11 )	// 겜블 레어 낫
				Scale = 0.0025f;
#endif //LDK_ADD_GAMBLERS_WEAPONS
			else
				Scale = 0.0021f;
#else //LDK_FIX_INVENTORY_SPEAR_SCALE
			if(MODEL_SPEAR+10)	//. 기사 창
				Scale = 0.0018f;
			else
				Scale = 0.0021f;
#endif //LDK_FIX_INVENTORY_SPEAR_SCALE
		}
		else if(Type>=MODEL_STAFF && Type<MODEL_STAFF+MAX_ITEM_INDEX)
		{
			if (Type >= MODEL_STAFF+14 && Type <= MODEL_STAFF+20)	// 소환술사 스틱.
				Scale = 0.0028f;
			else if (Type >= MODEL_STAFF+21 && Type <= MODEL_STAFF+29)	// 사아무트의 서, 닐의 서
				Scale = 0.004f;
#ifdef LDK_ADD_GAMBLERS_WEAPONS
			else if( Type == MODEL_STAFF+33 )	// 겜블 레어 지팡이
				Scale = 0.0028f;
			else if( Type == MODEL_STAFF+34 )	// 겜블 레어 지팡이(소환술사용)
				Scale = 0.0028f;
#endif //LDK_ADD_GAMBLERS_WEAPONS
			else
      			Scale = 0.0022f;
		}
        else if(Type==MODEL_BOW+15)
      		Scale = 0.0011f;
		else if(Type==MODEL_BOW+7)
      		Scale = 0.0012f;
		else if(Type==MODEL_EVENT+6)
      		Scale = 0.0039f;
		else if(Type==MODEL_EVENT+8)	//  은 훈장
			Scale = 0.0015f;
		else if(Type==MODEL_EVENT+9)	//  금 훈장
			Scale = 0.0019f;
		else
		{
			Scale = 0.0025f;
		}

#ifdef LDS_ADD_CS6_CHARM_MIX_ITEM_WING	// 날개 조합 100% 성공 부적
		if( Type >= MODEL_TYPE_CHARM_MIXWING+EWS_BEGIN
			&& Type <= MODEL_TYPE_CHARM_MIXWING+EWS_END )
		{
			Scale = 0.0020f;
		}
#endif //LDS_ADD_CS6_CHARM_MIX_ITEM_WING

#ifdef USE_EVENT_ELDORADO
		if(Type==MODEL_EVENT+10)	//  엘도라도
		{
			Scale = 0.001f;
		}
#endif // USE_EVENT_ELDORADO
#ifdef LDS_ADD_PCROOM_ITEM_JPN_6TH	// 강함의 인장 (PC방 아이템, 일본 6차 컨텐츠)
		else if(Type == MODEL_HELPER+96)
		{
			Scale = 0.0031f;
		}	
#endif // LDS_ADD_PCROOM_ITEM_JPN_6TH	
		else if(Type >= MODEL_ETC+19 && Type <= MODEL_ETC+27		// 양피지
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
			|| Type == MODEL_POTION+151 || Type == MODEL_POTION+152	// 1레벨 의뢰서, 1레벨 의뢰 완료 확인서
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
			)
		{
			Scale = 0.0023f;
		}
#ifdef PBG_ADD_SANTAINVITATION
		else if(Type == MODEL_HELPER+66)
		{
			Scale = 0.0020f;
		}
#endif //PBG_ADD_SANTAINVITATION
#ifdef YDG_ADD_HEALING_SCROLL
		else if(Type == MODEL_POTION+140)	// 치유의 스크롤
		{
			Scale = 0.0026f;
		}
#endif	// YDG_ADD_HEALING_SCROLL
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		else if( Type == MODEL_HELPER+122 )	// 스켈레톤 변신반지
		{
			Scale = 0.0033f;
		}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
#ifdef YDG_ADD_SKELETON_PET
		else if( Type == MODEL_HELPER+123 )	// 스켈레톤 펫
		{
			Scale = 0.0009f;
		}
#endif	// YDG_ADD_SKELETON_PET
#ifdef LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
		else if(Type >= MODEL_POTION+145 && Type <= MODEL_POTION+150)
		{
			Scale = 0.0018f;
		}
#endif //LJH_ADD_RARE_ITEM_TICKET_FROM_7_TO_12
#ifdef LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
		// 아이템 스케일 정하는 곳
		//도플갱어, 바르카, 바르카제7맵 자유입장권
		else if(Type >= MODEL_HELPER+125 && Type <= MODEL_HELPER+127)	//도플갱어, 바르카, 바르카제7맵 자유입장권
		{
			Scale = 0.0013f;
		}
#endif //LJH_ADD_FREE_TICKET_FOR_DOPPELGANGGER_BARCA_BARCA_7TH
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
#ifdef LDK_ADD_GAMBLE_RANDOM_ICON
		//겜블 상점 아이콘 모델 번호 수정 해야됨
		else if ( Type==MODEL_HELPER+71 || Type==MODEL_HELPER+72 || Type==MODEL_HELPER+73 || Type==MODEL_HELPER+74 || Type==MODEL_HELPER+75 )
		{
			Scale = 0.0019f;
		}
#endif //LDK_ADD_GAMBLE_RANDOM_ICON
#ifdef LDK_ADD_GAMBLERS_WEAPONS
		else if( Type == MODEL_BOW+24 )
		{
			Scale = 0.0023f;
		}
#endif //LDK_ADD_GAMBLERS_WEAPONS
#ifdef PBG_ADD_CHARACTERCARD
		//마검 다크 소환술사 카드
		else if(Type == MODEL_HELPER+97 || Type == MODEL_HELPER+98 || Type == MODEL_POTION+91)
		{
			Scale = 0.0028f;
		}
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
		else if(Type == MODEL_HELPER+99)
		{
			Scale = 0.0025f;
		}
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PBG_ADD_SECRETITEM
		//활력의비약(최하급/하급/중급/상급)
		else if(Type >= MODEL_HELPER+117 && Type <= MODEL_HELPER+120)
		{
			// 인벤에 안들어가서 1x2사이즈로 설정
#ifdef PBG_MOD_SECRETITEM
			Scale = 0.0022f;
#else //PBG_MOD_SECRETITEM
			Scale = 0.0035f;
#endif //PBG_MOD_SECRETITEM
		}
#endif //PBG_ADD_SECRETITEM
#ifdef YDG_ADD_DOPPELGANGER_ITEM
		else if (Type == MODEL_POTION+110)	// 차원의표식
		{
			Scale = 0.004f;
		}
#endif	// YDG_ADD_DOPPELGANGER_ITEM
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
		else if(Type == MODEL_HELPER+107)
		{
			// 치명마법반지
			Scale = 0.0034f;
		}
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
		else if(Type == MODEL_POTION+133)		// 엘리트 SD회복 물약
		{
			Scale = 0.0030f;
		}
#endif	// YDG_ADD_CS7_ELITE_SD_POTION
#ifdef YDG_ADD_CS7_MAX_SD_AURA
		else if(Type == MODEL_HELPER+105)		// SD증가 오라
		{
			Scale = 0.002f;
		}
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef LDK_ADD_EMPIREGUARDIAN_ITEM
		else if( MODEL_POTION+101 <= Type && Type <= MODEL_POTION+109 )
		{
			switch(Type)
			{
			case MODEL_POTION+101: // 의문의쪽지
				{
					Scale = 0.004f;
				}break;
			case MODEL_POTION+102: // 가이온의 명령서
				{
					Scale = 0.005f;
				}break;
			case MODEL_POTION+103: // 세크로미콘 조각
			case MODEL_POTION+104: 
			case MODEL_POTION+105: 
			case MODEL_POTION+106: 
			case MODEL_POTION+107: 
			case MODEL_POTION+108: 
				{
					Scale = 0.004f;
				}break;
			case MODEL_POTION+109: // 세크로미콘
				{
					Scale = 0.003f;
				}break;
			}
		}
#endif //LDK_ADD_EMPIREGUARDIAN_ITEM
#ifdef LDK_ADD_CS7_UNICORN_PET	//유니콘
		else if( Type == MODEL_HELPER+106 )
		{
			Scale = 0.0015f;
		}	
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
		else if( Type == MODEL_WING+130 )
		{
			Scale = 0.0012f;
		}
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef LDK_ADD_INGAMESHOP_PACKAGE_BOX // 인게임샾 아이템 // 패키지상자6종			// MODEL_POTION+134~139
		else if( Type >= MODEL_POTION+134 && Type <= MODEL_POTION+139 )
		{
			Scale = 0.0050f;
		}
#endif // LDK_ADD_INGAMESHOP_PACKAGE_BOX // 인게임샾 아이템 // 패키지상자6종			// MODEL_POTION+134~139
#if defined(LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE)||defined(LDS_ADD_INGAMESHOP_ITEM_RINGRUBY)||defined(LDS_ADD_INGAMESHOP_ITEM_RINGTOPAZ)||defined(LDS_ADD_INGAMESHOP_ITEM_RINGAMETHYST)	
		else if( Type >= MODEL_HELPER+109 && Type <= MODEL_HELPER+112  )	// 사파이어(푸른색)링,루비(붉은색)링,토파즈(주황)링,자수정(보라색)링
		{
			Scale = 0.0045f;
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_RINGSAPPHIRE	// 사파이어(푸른색)링,루비(붉은색)링,토파즈(주황)링,자수정(보라색)링
#if defined(LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY)||defined(LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD)||defined(LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE)		
		else if( Type >= MODEL_HELPER+113 && Type <= MODEL_HELPER+115 )		// 루비(붉은색)목걸이, 에메랄드(푸른), 사파이어(녹색) 목걸이
		{
			Scale = 0.0018f;
		}
#endif // defined(LDS_ADD_INGAMESHOP_ITEM_AMULETRUBY)||defined(LDS_ADD_INGAMESHOP_ITEM_AMULETEMERALD)||defined(LDS_ADD_INGAMESHOP_ITEM_AMULETSAPPHIRE) // 루비(붉은색)목걸이, 에메랄드(푸른), 사파이어(녹색) 목걸이
#if defined(LDS_ADD_INGAMESHOP_ITEM_KEYSILVER) || defined(LDS_ADD_INGAMESHOP_ITEM_KEYGOLD)	// 키(실버), 키(골드)
		else if( Type >= MODEL_POTION+112 && Type <= MODEL_POTION+113 )
		{
			Scale = 0.0032f;
		}
#endif // defined(LDS_ADD_INGAMESHOP_ITEM_KEYSILVER) || defined(LDS_ADD_INGAMESHOP_ITEM_KEYGOLD)	// 키(실버), 키(골드)
#ifdef LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
		else if( Type == MODEL_HELPER+116 )
		{
			Scale = 0.0021f;
		}
#endif //LDK_ADD_INGAMESHOP_NEW_WEALTH_SEAL
#ifdef LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
		else if( Type >= MODEL_POTION+114 && Type <= MODEL_POTION+119 )
		{
			Scale = 0.0038f;
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_PRIMIUMSERVICE6		// 인게임샾 아이템 // 프리미엄서비스6종			// MODEL_POTION+114~119
#ifdef LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
		else if( Type >= MODEL_POTION+126 && Type <= MODEL_POTION+129 )
		{
			Scale = 0.0038f;
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_COMMUTERTICKET4		// 인게임샾 아이템 // 정액권4종					// MODEL_POTION+126~129
#ifdef LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
		else if( Type >= MODEL_POTION+130 && Type <= MODEL_POTION+132 )
		{
			Scale = 0.0038f;
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_SIZECOMMUTERTICKET3	// 인게임샾 아이템 // 정량권3종					// MODEL_POTION+130~132
#ifdef LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
		else if( Type == MODEL_HELPER+121 )
		{
			Scale = 0.0018f;
			//Scale = 1.f;
		}
#endif // LDS_ADD_INGAMESHOP_ITEM_PASSCHAOSCASTLE		// 인게임샾 아이템 // 카오스케슬 자유입장권		// MODEL_HELPER+121
#ifdef ASG_ADD_CHARGED_CHANNEL_TICKET
		else if(Type == MODEL_HELPER+124)
			Scale = 0.0018f;
#endif	// ASG_ADD_CHARGED_CHANNEL_TICKET
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
#ifdef ASG_ADD_TIME_LIMIT_QUEST_ITEM
		else if (Type == MODEL_POTION+156)
			Scale = 0.0025f;
#endif	// ASG_ADD_TIME_LIMIT_QUEST_ITEM
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
#ifdef NEW_USER_INTERFACE
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTCHARGE_SHOP) == true) {
		float ChangeScale = (640.f/static_cast<float>(TheShell().GetWindowSize().x))*3.7f;
		o->Scale = Scale-(Scale/ChangeScale);
	}
	else
#endif // NEW_USER_INTERFACE
	{
		o->Scale = Scale;
	}
		
	VectorCopy(Position,o->Position);
	
    vec3_t Light;
    float  alpha = o->Alpha;

    Vector(1.f,1.f,1.f,Light);

#ifdef MR0
	VPManager::Enable();
#endif //MR0
	
#ifdef MR0
	// Object가 있다면 단지 Lock을 걸고, 없으면 Lock을 걸고 Render 
	ModelManager::SetTargetObject( o );
#endif //MR0

	//인관
#ifdef PJH_NEW_CHROME
	int Set_Count = 0;

	int nItemType = (Type - MODEL_ITEM) / MAX_ITEM_INDEX;
	int nItemSubType = (Type - MODEL_ITEM) % MAX_ITEM_INDEX;
	// 스핑크스 시리즈인가?
	if (nItemType >= 7 && nItemType <= 11 && (nItemSubType >= 62 && nItemSubType <= 72))
	{
		Set_Count = 1;
	}

	if(Set_Count == 1)
		RenderPartObject(o,Type,NULL,Light,alpha,ItemLevel,Option1,MAX_MODELS+1,true,true,true);
	else
#endif //PJH_NEW_CHROME
    RenderPartObject(o,Type,NULL,Light,alpha,ItemLevel,Option1,ExtOption,true,true,true);

#ifdef MR0
	ModelManager::SetTargetObject(NULL);
#endif //MR0
	
#ifdef MR0
	VPManager::Disable();
#endif //MR0
}

///////////////////////////////////////////////////////////////////////////////
// 인밴토리에 아이템을 랜더링 하는 함수
///////////////////////////////////////////////////////////////////////////////

void RenderItem3D(float sx,float sy,float Width,float Height,int Type,int Level,int Option1,int ExtOption,bool PickUp)
{
	bool Success = false;
	if((g_pPickedItem == NULL || PickUp) 
		&& SEASON3B::CheckMouseIn(sx, sy, Width, Height))
	{
#ifdef NEW_USER_INTERFACE
		if( g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_PARTCHARGE_SHOP) ) 
		{
			Success = true;
		}
		else  
#endif // NEW_USER_INTERFACE
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
#ifdef CSK_ADD_ITEM_CROSSSHIELD
		else if(Type == ITEM_SHIELD+21)		// 크로스실드
		{
			sx += Width*0.05f;
			sy += Height*0.5f;
		}
#endif // CSK_ADD_ITEM_CROSSSHIELD
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
		else if ( Type==ITEM_ARMOR+17 || Type==ITEM_ARMOR+18 || Type==ITEM_ARMOR+20 ) //  블레이드나이트.소울마스터. 선더 마검사.
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
	else if ( Type==ITEM_HELPER+14 && (Level>>3)==1 )    //  군주의 문장.
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
		case 0: sx += Width*0.5f; sy += Height*0.5f; break; //  대천사의 절대지팡이.
		case 1: sx += Width*0.7f; sy += Height*0.8f; break; //  대천사의 절대검.
		case 2: sx += Width*0.7f; sy += Height*0.7f; break; //  대천사의 절대석궁.
		}
	}
	else if( Type == ITEM_HELPER+20 )
	{
		switch( Level>>3 )
		{
		case 0: sx += Width*0.5f; sy+= Height*0.65f; break;  // 마법사의 반지
		case 1: // 제왕의 반지
		case 2: // 전사의 반지
		case 3: // 영예의 반지.
			sx += Width*0.5f; sy+= Height*0.8f; break;
		}
	}
	else if ( Type==ITEM_HELPER+29 )	//. 근위병의 갑옷세트
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if ( Type==ITEM_HELPER+4 )
	{
		sx += Width*0.5f;
		sy += Height*0.6f;
	}
	else if ( Type==ITEM_HELPER+30 )    //  망토.
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if ( Type==ITEM_HELPER+31 )    //  영혼.
	{
		sx += Width*0.5f;
		sy += Height*0.9f;
	}
	else if ( Type==ITEM_POTION+7 )//종훈물약
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
	//^ 펜릴 인벤토리 위치 조정
	else if(Type == ITEM_HELPER+32)	// 갑옷 파편
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+33)	// 여신의 가호
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+34)	// 맹수의 발톱
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+35)	// 뿔피리의 조각
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+36)	// 부러진 뿔피리
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type == MODEL_HELPER+37)	// 펜릴의 뿔피리
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
	else if(Type==ITEM_POTION+11 && ((Level>>3) == 3 || (Level>>3) == 13))	// 사랑의 하트, 다크로드의 마음
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
#ifdef NEW_YEAR_BAG_EVENT
	else if ( Type==ITEM_POTION+11 && ( (Level>>3)==14 || (Level>>3)==15 ) )  //  파란, 빨강 복주머니
	{
		sx += Width*0.5f;
		sy += Height*0.8f;
	}
#else
#ifdef CHINA_MOON_CAKE
	else if ( Type==ITEM_POTION+11 && (Level>>3)==14 )  //  중국 월병.
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
#endif// CHINA_MOON_CAKE
#endif// NEW_YEAR_BAG_EVENT
#ifdef MYSTERY_BEAD
	else if(Type==ITEM_WING+26 && (Level>>3) == 0)	//. 신비의구슬
	{
		sx += Width*0.5f;
		sy += Height*0.8f;
	}
#endif // MYSTERY_BEAD
	else if(Type==ITEM_POTION+9 && (Level>>3) == 1)	// 사랑의 올리브
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
		case 0: sx += Width*0.5f; sy += Height*0.5f; break; //  레나.
		case 1: sx += Width*0.4f; sy += Height*0.8f; break; //  스톤.
#ifdef FRIEND_EVENT
		case 2: sx += Width*0.4f; sy += Height*0.8f; break; //  우정의 돌.
#endif	// FRIEND_EVENT
		case 3: sx += Width*0.5f; sy += Height*0.5f; break; //  성주의표식
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
#ifdef _PVP_MURDERER_HERO_ITEM
	else if ( Type==ITEM_POTION+30 )	// 징표
	{
		sx += Width*0.6f;
		sy += Height*1.0f;
	}
#endif	// _PVP_MURDERER_HERO_ITEM
	else if(Type==ITEM_POTION+31)	// 수호의보석
	{
		sx += Width*0.5f;
		sy += Height*0.5f;
	}
	else if(Type==INDEX_COMPILED_CELE || Type == INDEX_COMPILED_SOUL)
	{
		sx += Width*0.55f;
		sy += Height*0.8f;
	}
	//  추가되는 날개.
	else if ( Type==ITEM_WING+3 )   //  정령의 날개.
	{
		sx += Width*0.5f;
		sy += Height*0.45f;
	}
	else if ( Type==ITEM_WING+4 )   //  영혼의 날개.
	{
		sx += Width*0.5f;
		sy += Height*0.4f;
	}
	else if ( Type==ITEM_WING+5 )   //  드라곤 날개.
	{
		sx += Width*0.5f;
		sy += Height*0.75f;
	}
	else if ( Type==ITEM_WING+6 )   //  어둠의 날개.
	{
		sx += Width*0.5f;
		sy += Height*0.55f;
	}
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	//행운의 동전
	else if(Type == ITEM_POTION+100)
	{
		sx += Width*0.49f;
		//sy += Height*0.28f;
		sy += Height*0.28f;
	}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008


#ifdef LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX	// 인벤토리 위치 조정
	else if (COMGEM::Check_Jewel_Com(Type) != COMGEM::NOGEM)
	{
		sx += Width*0.55f;
		sy += Height*0.82f;
	}
#endif // LEM_ADD_SEASON5_PART5_MINIUPDATE_JEWELMIX
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
#ifdef PBG_ADD_SANTAINVITATION
	//산타마을의 초대장.
	else if( Type == ITEM_HELPER+66 )
	{
		sx += Width*1.5f;
		sy += Height*1.5f;
	}
#endif //PBG_ADD_SANTAINVITATION
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
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 2)	// 폭죽/ 마법 주머니.
	{
		RenderObjectScreen(MODEL_EVENT+5,Level,Option1,ExtOption,Position,Success,PickUp);
	}
#ifdef ANNIVERSARY_EVENT
	else if ( Type==ITEM_POTION+20 && ( Level>>3) >= 1 && ( Level>>3) <= 5)	// 애니버서리 박스
	{
		RenderObjectScreen(MODEL_POTION+11,(7 << 3),Option1,ExtOption,Position,Success,PickUp);
	}
#endif	// ANNIVERSARY_EVENT
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 3)	// 사랑의 하트
	{
		RenderObjectScreen(MODEL_EVENT+6,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 5)	// 은훈장
	{
		RenderObjectScreen(MODEL_EVENT+8,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 6)	// 금훈장
	{
		RenderObjectScreen(MODEL_EVENT+9,Level,Option1,ExtOption,Position,Success,PickUp);
	}
#ifdef USE_EVENT_ELDORADO
	else if ( Type==ITEM_POTION+11 && 8 <= ( Level>>3) && ( Level>>3) <= 12)	// 엘도라도
	{
		RenderObjectScreen(MODEL_EVENT+10,Level,Option1,ExtOption,Position,Success,PickUp);
	}
#endif
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 13)	// 다크로드의 마음
	{
		RenderObjectScreen(MODEL_EVENT+6,Level,Option1,ExtOption,Position,Success,PickUp);
	}
#ifdef NEW_YEAR_BAG_EVENT
	else if ( Type==ITEM_POTION+11 && ( (Level>>3)==14 || (Level>>3)==15 ) )    //  복주머니
	{
		RenderObjectScreen(MODEL_EVENT+5,Level,Option1,ExtOption,Position,Success,PickUp);
	}
#else
#ifdef CHINA_MOON_CAKE
	else if ( Type==ITEM_POTION+11 && ( Level>>3) == 14)	//  중국 월병.
	{
		RenderObjectScreen(MODEL_EVENT+17,Level,Option1,ExtOption,Position,Success,PickUp);
	}
#endif	//  CHINA_MOON_CAKE
#endif// NEW_YEAR_BAG_EVENT
	
	else if ( Type==ITEM_HELPER+14 && ( Level>>3) == 1)	// 군주의 소매.
	{
		RenderObjectScreen ( MODEL_EVENT+16, Level, Option1, ExtOption, Position, Success, PickUp );
	}
	else if ( Type==ITEM_POTION+9 && ( Level>>3) == 1)	// 사랑의 올리브
	{
		RenderObjectScreen(MODEL_EVENT+7,Level,Option1,ExtOption,Position,Success,PickUp);
	}
	else if ( Type==ITEM_POTION+21 )
	{
		switch ( (Level>>3) )
		{
		case 1:
			RenderObjectScreen(MODEL_EVENT+11,Level,Option1,ExtOption,Position,Success,PickUp);   //  스톤.
			break;
#ifdef FRIEND_EVENT
		case 2:
			RenderObjectScreen(MODEL_EVENT+11,Level,Option1,ExtOption,Position,Success,PickUp); //  우정의 돌.
			break;
#endif// FRIEND_EVENT
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

#ifdef GIFT_BOX_EVENT
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
	
#endif
	
#ifdef MYSTERY_BEAD
	else if ( Type ==ITEM_WING+26)
	{
		switch ( (Level>>3) )
		{
		case 0:		//. 신비의 구슬
			RenderObjectScreen(MODEL_EVENT+19,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 1:
		case 2:
		case 3:
			RenderObjectScreen(MODEL_EVENT+20,Level,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 4:
			RenderObjectScreen(MODEL_POTION+11,4<<3,Option1,ExtOption,Position,Success,PickUp);
			break;
		case 5:
			RenderObjectScreen(MODEL_POTION+11,7<<3,Option1,ExtOption,Position,Success,PickUp);
			break;
		}
	}
#endif // MYSTERY_BEAD
	else if ( Type==ITEM_HELPER+19)     //  대천사의 절대 무기 시리즈.
	{
		switch ( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(MODEL_STAFF+10,-1,Option1,ExtOption,Position,Success,PickUp);   //  대천사의 절대지팡이.
			break;
		case 1:
			RenderObjectScreen(MODEL_SWORD+19,-1,Option1,ExtOption,Position,Success,PickUp);   //  대천사의 절대검.
			break;
		case 2:
			RenderObjectScreen(MODEL_BOW+18,-1,Option1,ExtOption,Position,Success,PickUp);   //  대천사의 절대석궁.
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
			RenderObjectScreen(MODEL_EVENT+12,-1,Option1,ExtOption,Position,Success,PickUp);   //. 영광의 반지
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
			RenderObjectScreen(MODEL_EVENT+13,-1,Option1,ExtOption,Position,Success,PickUp);   //. 다크스톤
			break;
		}
	}
	else if ( Type==ITEM_HELPER+20)
	{
		switch( (Level>>3) )
		{
		case 0:
			RenderObjectScreen(MODEL_EVENT+15,Level,Option1,ExtOption,Position,Success,PickUp);	// 마법사의 반지
			break;
		case 1:
		case 2: //  전사의 반지.
		case 3: //  영예의 반지.
			RenderObjectScreen(MODEL_EVENT+14,Level,Option1,ExtOption,Position,Success,PickUp);	//. 제왕의 반지
			break;
		}
	}
	else if ( Type==ITEM_HELPER+11 && (Level>>3)==1 )
	{
		RenderObjectScreen ( MODEL_EVENT+18,Level,Option1,ExtOption,Position,Success,PickUp);	//  라이프 스톤.
	}
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	else if(Type == ITEM_POTION+100)
	{
		bool _Angle;
		if(g_pLuckyCoinRegistration->GetItemRotation())
		{
			//등록창 안에 그려질 아이템
			_Angle = true;
		}
		else
		{
			//기타 인벤에 그려질 아이템
			_Angle = Success;
		}

		RenderObjectScreen(MODEL_POTION+100,Level,Option1,ExtOption,Position,_Angle,PickUp);	// 행운의 동전
	}
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
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

///////////////////////////////////////////////////////////////////////////////
// 인밴토리 칼라를 세팅하는 함수
///////////////////////////////////////////////////////////////////////////////

void InventoryColor(ITEM *p)
{
	switch(p->Color)
	{
	case 0:
		glColor3f(1.f,1.f,1.f);     //  보통 상태.
		break;
	case 1:
		glColor3f(0.8f,0.8f,0.8f);  //  
		break;
	case 2:
		glColor3f(0.6f,0.7f,1.f);
		break;
	case 3:
		glColor3f(1.f,0.2f,0.1f);   //  사용 불가능.
		break;
	case 4:
		glColor3f(0.5f,1.f,0.6f);
		break;
    case 5:
		glColor4f(0.8f,0.7f,0.f,1.f);  //  내구력 경고. 50%
        break;
    case 6:
		glColor4f(0.8f,0.5f,0.f,1.f);  //  내구력 경고. 70%
        break;
    case 7:
		glColor4f(0.8f,0.3f,0.3f,1.f); //  내구력 경고. 80%
        break;
    case 8:
		glColor4f(1.0f,0.f,0.f,1.f);    //  내구력 경고. 100%
        break;
    case 99:
		glColor3f(1.f,0.2f,0.1f);       //  거래 창에서 사용.
        break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// 장비창 박스 랜더링 하는 함수
///////////////////////////////////////////////////////////////////////////////

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
	if(GetBaseClass(CharacterAttribute->Class)!=CLASS_DARK)
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

///////////////////////////////////////////////////////////////////////////////
// 장비창의 아이템 한개 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////

void RenderEqiupmentPart3D(int Index,float sx,float sy,float Width,float Height)
{
	ITEM *p = &CharacterMachine->Equipment[Index];
	if(p->Type != -1)
	{
		if(p->Number > 0)
            RenderItem3D(sx,sy,Width,Height,p->Type,p->Level,p->Option1,p->ExtOption,false);
	}
}

///////////////////////////////////////////////////////////////////////////////
// 장비창의 아이템 모두 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////

void RenderEqiupment3D()
{
#ifdef MR0
	VPManager::Enable();
#endif //MR0

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

#ifdef MR0
	VPManager::Disable();
#endif //MR0	
}

///////////////////////////////////////////////////////////////////////////////
// 인밴토리 버튼들 처리하는 함수
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
//  파티창 인터페이스.
///////////////////////////////////////////////////////////////////////////////
void InitPartyList ()
{
    PartyNumber = 0;
    PartyKey = 0;
}

//////////////////////////////////////////////////////////////////////////
//  서버 분할 인터페이스.
//////////////////////////////////////////////////////////////////////////
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

    //  동의.
    Width = 16; Height = 16; x = InventoryStartX+25;y = 240;
	if(MouseX>=x && MouseX<x+Width && MouseY>=y && MouseY<y+Height && MouseLButtonPush )
    {
        g_bServerDivisionAccept ^= true;

        MouseLButtonPush = false;
        MouseLButton = false;
    }

    //  확인 버튼.
    if ( g_bServerDivisionAccept )
    {
        Width = 120; Height = 24; x = (float)InventoryStartX+35;y = 320;
	    if(MouseX>=x && MouseX<x+Width && MouseY>=y && MouseY<y+Height && MouseLButtonPush )
        {
            MouseLButtonPush = false;
            MouseLButton = false;

            AskYesOrNo =  4;
            OkYesOrNo  = -1;

            //  서버분할 확인창.
            ShowCheckBox(1, 448, MESSAGE_CHECK );
        }
    }

    //  취소.
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
    
    //  닫기 버튼.
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


///////////////////////////////////////////////////////////////////////////////
//  창고 잠금/해제를 위한 키패드창
///////////////////////////////////////////////////////////////////////////////

void HideKeyPad( void)
{
	g_iKeyPadEnable = 0;
}

int CheckMouseOnKeyPad( void)
{
	// 1. 틀
	int Width, Height, WindowX, WindowY;
    Width = 213;Height = 2*5+6*40;WindowX = (640-Width)/2;WindowY = 60+40;//60 220
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int yPos = WindowY;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	int iButtonTop = 50;
	// 숫자 ( 0 - 10)
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
	if((g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPSALE) || g_pUIManager->IsOpen(INTERFACE_PERSONALSHOPPURCHASE)) && g_iPShopWndType == PSHOPWNDTYPE_SALE)		//. 판매자 모드
	{
		if(g_iPersonalShopMsgType == 1)		//. 상점 타이틀 변경
		{
			if(OkYesOrNo == 1) {				//. Ok
				g_iPersonalShopMsgType = 0;
				OkYesOrNo = -1;
			}
			else if(OkYesOrNo == 2) {			//. Cancel
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

		//. 상점타이틀 수정
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
	{	//. 구매자 모드일때
		//. 내 개인상점 영역을 복구한다.
		memcpy(g_PersonalShopInven, g_PersonalShopBackup, sizeof(ITEM)*MAX_PERSONALSHOP_INVEN);
		if(IsShopInViewport(Hero)) 
		{	//. 본인이 상점 개설중일때 상점 타이틀도 복구한다.
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
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int Width = 213; int Height = 2*5+6*40;
	int WindowX = (640-Width)/2; int WindowY = 60+40;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	
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

#ifndef YDG_ADD_NEW_DUEL_SYSTEM		// 정리할때 삭제해야 함
void ClearDuelWindow()
{
	g_bEnableDuel = false;
	g_iMyPlayerScore = 0;
	g_iDuelPlayerScore = 0;
}
#endif	// YDG_ADD_NEW_DUEL_SYSTEM	// 정리할때 삭제해야 함

///////////////////////////////////////////////////////////////////////////////
//  인벤토리 처리함수.
///////////////////////////////////////////////////////////////////////////////

extern DWORD g_dwActiveUIID;
extern DWORD g_dwMouseUseUIID;		// 마우스가 윈도우 위에 있는가 (있으면 윈도우 ID)

///////////////////////////////////////////////////////////////////////////////
// 인밴토리 백그라운드 그림 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////

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

	if (BLUE_MU::IsBlueMuServer())
	{
		ENUM_WORLD aeStrife[5] = { WD_7ATLANSE, WD_33AIDA, WD_37KANTURU_1ST, WD_56MAP_SWAMP_OF_QUIET, WD_63PK_FIELD };
		int i;
		for (i = 0; i < 5; ++i)
		{
			if (aeStrife[i] == nMapIndex)
			{
				bStrifeMap = true;
				break;
			}
		}
	}
	else if (!g_ServerListManager->IsNonPvP())
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


///////////////////////////////////////////////////////////////////////////////
// 길드마크 그릴때 칼라 박스 하나 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////
// 길드 리스트창 랜더링하는 함수
///////////////////////////////////////////////////////////////////////////////

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
	// 길드공지
	g_pRenderText->SetBgColor(0, 0, 0, 128);
	g_pRenderText->SetTextColor(100, 255, 200, 255);
	g_pRenderText->RenderText(StartX+( int)Width/2,StartY+44,g_GuildNotice[0], 0 ,0, RT3_WRITE_CENTER);
	g_pRenderText->RenderText(StartX+( int)Width/2,StartY+58,g_GuildNotice[1], 0 ,0, RT3_WRITE_CENTER);

#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int yGuildStart = 72;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int Number = g_nGuildMemberCount;

	if(g_nGuildMemberCount >= MAX_GUILD_LINE)
      	Number = MAX_GUILD_LINE;
}

//#define MAX_LENGTH_CMB	( 26)
#define NUM_LINE_CMB	( 7)

//////////////////////////////////////////////////////////////////////////
//  서버 분할창.
//////////////////////////////////////////////////////////////////////////
void RenderServerDivision ()
{
    if ( !g_pUIManager->IsOpen(INTERFACE_SERVERDIVISION) ) return;

   	float Width, Height, x, y; 

	glColor3f ( 1.f, 1.f, 1.f );
    EnableAlphaTest ();

    //  틀.
    InventoryStartX = 640-190;
    InventoryStartY = 0;
    Width = 213;Height = 40;x = (float)InventoryStartX;y = (float)InventoryStartY;
	RenderInventoryInterface ( (int)x, (int)y, 1 );

	//  버튼.
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(255, 230, 210, 255);

    //  약관.
	g_pRenderText->SetFont(g_hFontBold);
    x = InventoryStartX+(190/2.f);
    y = 50;
    for ( int i=462; i<470; ++i )
    {
		g_pRenderText->RenderText(x, y, GlobalText[i], 0 ,0, RT3_WRITE_CENTER);
        y += 20;
    }

    //  동의.
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

    //  취소.
    Width = 120; Height = 24; x = (float)InventoryStartX+35;y = 350;//(Width/2.f); y = 231;
    RenderBitmap(BITMAP_INTERFACE+10,(float)x,(float)y,(float)Width,(float)Height,0.f,0.f,213.f/256.f);
	g_pRenderText->RenderText((int)(x+(Width/2)),(int)(y+5),GlobalText[229], 0 ,0, RT3_WRITE_CENTER);

    //  확인.
    Width = 120; Height = 24; x = (float)InventoryStartX+35;y = 320;//(Width/2.f); y = 231;
    if ( g_bServerDivisionAccept )
        glColor3f ( 1.f, 1.f, 1.f );
    else 
        glColor3f ( 0.5f, 0.5f, 0.5f );
    RenderBitmap(BITMAP_INTERFACE+10,(float)x,(float)y,(float)Width,(float)Height,0.f,0.f,213.f/256.f);
	g_pRenderText->RenderText((int)(x+(Width/2)),(int)(y+5),GlobalText[228], 0 ,0, RT3_WRITE_CENTER);

    glColor3f( 1.f, 1.f, 1.f );
}

#ifdef CSK_FREE_TICKET
// 자유입장권에서 입장가능한 레벨을 계산해서 아이템의 레벨을 리턴시켜준다.
BYTE CaculateFreeTicketLevel(int iType)
{
	// 캐릭터 레벨
	int iChaLevel = CharacterAttribute->Level;

	// 캐릭터 기본 클래스
	int iChaClass = GetBaseClass(Hero->Class);
	// 캐릭터 전직 클래스
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int iChaExClass = IsSecondClass(Hero->Class);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	int iItemLevel = 0;

	if(iType == FREETICKET_TYPE_DEVILSQUARE)
	{
		// 마검사일 경우
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
		// 다른 클래스일 경우
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

		// 바이트 단위로 계산해서 리턴해 준다.
		return (iItemLevel>>3)&15;
	}
	// 블러드캐슬
	else if(iType == FREETICKET_TYPE_BLOODCASTLE)
	{
		// 마검사 경우
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
		// 다른클래스 경우
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

#ifdef LJH_FIX_INCORRECT_SCHEDULE_FOR_BC_FREE_TICKET
		if(IsMasterLevel(Hero->Class))
			iItemLevel = 56;
#endif //LJH_FIX_INCORRECT_SCHEDULE_FOR_BC_FREE_TICKET

		// 바이트 단위로 계산해서 리턴해 준다.
		return (iItemLevel>>3)&15;
	}
#ifdef PSW_CURSEDTEMPLE_FREE_TICKET
	else if( iType == FREETICKET_TYPE_CURSEDTEMPLE ) {
		if( g_pCursedTempleEnterWindow->CheckEnterLevel( iItemLevel ) )
		{
			return (iItemLevel>>3)&15;
		}
	}
#endif //PSW_CURSEDTEMPLE_FREE_TICKET	
#ifdef LDS_FIX_INGAMESHOPITEM_PASSCHAOSCASTLE_REQUEST	// 카오스캐슬 자유입장권
	else if( iType == FREETICKED_TYPE_CHAOSCASTLE ) 
	{
		if( g_pCursedTempleEnterWindow->CheckEnterLevel( iItemLevel ) )
		{
			return (iItemLevel>>3)&15;
		}
	}
#endif // LDS_FIX_INGAMESHOPITEM_PASSCHAOSCASTLE_REQUEST
	return 0;
}
#endif // CSK_FREE_TICKET

#ifdef NEW_USER_INTERFACE_UISYSTEM
const bool ChangeCodeItem( ITEM* ip, BYTE* itemdata )
{
	int Type = ConvertItemType( itemdata );

	if( Type == 0x1FFF ) 
	{
		return false;
	}
	else
	{
		ip->Type							= Type;
		ip->x								= 0;
		ip->y								= 0;
		ip->Number							= 0;
		ip->Durability						= itemdata[2];
		ip->Option1							= itemdata[3];
		ip->ExtOption						= itemdata[4];
		ip->option_380						= false;
		byte is380option					= ( ( (itemdata[5] & 0x08) << 4) >>7);
		ip->option_380						= is380option;
		ip->Jewel_Of_Harmony_Option			= (itemdata[6] & 0xf0) >> 4;
		ip->Jewel_Of_Harmony_OptionLevel	= itemdata[6] & 0x0f;

#ifdef SOCKET_SYSTEM
		// 소켓 아이템 옵션 (0x00~0xF9: 옵션고유번호, 0xFE: 빈 소켓, 0xFF: 막힌 소켓)
		BYTE bySocketOption[5] = { itemdata[7], itemdata[8], itemdata[9], itemdata[10], itemdata[11] };
		ip->SocketCount = MAX_SOCKETS;

		for (int i = 0; i < MAX_SOCKETS; ++i)
		{
			ip->bySocketOption[i] = bySocketOption[i];	// 서버에서 받은 내용 백업
		}

		for (int i = 0; i < MAX_SOCKETS; ++i)
		{
			if (bySocketOption[i] == 0xFF)		// 소켓이 막힘 (DB상에는 0x00 으로 되어있음)
			{
				ip->SocketCount = i;
				break;
			}
			else if (bySocketOption[i] == 0xFE)	// 소켓이 비어있음 (DB상에는 0xFF 으로 되어있음)
			{
				ip->SocketSeedID[i] = SOCKET_EMPTY;
			}
			else	// 0x00~0xF9 까지 소켓 고유번호로 사용, MAX_SOCKET_OPTION(50)단위로 나누어 고유번호로 표시 (DB상에는 0x01~0xFA로 되어있음)
			{
				ip->SocketSeedID[i] = bySocketOption[i] % SEASON4A::MAX_SOCKET_OPTION;
				ip->SocketSphereLv[i] = int(bySocketOption[i] / SEASON4A::MAX_SOCKET_OPTION) + 1;
			}
		}
		if (g_SocketItemMgr.IsSocketItem(ip))	// 소켓 아이템이면
		{
			ip->SocketSeedSetOption = itemdata[6];	// 조화의보석옵션값으로 시드세트옵션을 표현함
			ip->Jewel_Of_Harmony_Option = 0;
			ip->Jewel_Of_Harmony_OptionLevel = 0;
		}
		else
		{
			ip->SocketSeedSetOption = SOCKET_EMPTY;
		}
#endif	// SOCKET_SYSTEM

		ItemConvert( ip,itemdata[1],itemdata[3],itemdata[4] );
	}

	return true;
}

const int ChangeData( ITEM* tooltipitem, const CASHSHOP_ITEMLIST& indata, vector<string>& outtextdata, vector<DWORD>& outcolordata )
{	
	string  tempstring;
	DWORD   tempstringcolor  = 0;
	int	    tempbackupmaxlen = 0;

	if( ChangeCodeItem( tooltipitem, (BYTE*)indata.s_btItemInfo ) )
	{
		tooltipitem->Durability = indata.s_btItemDuration;

		RenderItemInfo( 0, 0, tooltipitem, false, false );

		for ( int i = 0; i < ITEMINFOCOUNT; ++i )
		{
			tempstring      = TextList[i];
			tempstringcolor = TextListColor[i];

			if( tempstring.size() > 0 && TextList[i][0] != '\0' && TextList[i][0] != '\n' )
			{
				tempbackupmaxlen = Max( tempbackupmaxlen, tempstring.length() );
				outtextdata.push_back( tempstring );
				outcolordata.push_back( tempstringcolor );
			}
		}
	}

	return tempbackupmaxlen;
}

#endif //NEW_USER_INTERFACE_UISYSTEM

