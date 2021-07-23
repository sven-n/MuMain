// NewUIItemExplanationWindow.cpp: implementation of the CNewUIItemExplanationWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIItemExplanationWindow.h"
#include "NewUISystem.h"
#include "DSPlaySound.h"
#include "ZzzInventory.h"
#include "CSItemOption.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIItemExplanationWindow::CNewUIItemExplanationWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = 0;
	m_Pos.y = 0;
}

SEASON3B::CNewUIItemExplanationWindow::~CNewUIItemExplanationWindow()
{
	Release();
}

bool SEASON3B::CNewUIItemExplanationWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_ITEM_EXPLANATION, this);
	
	SetPos(x, y);

	Show(false);

	return true;
}

void SEASON3B::CNewUIItemExplanationWindow::Release()
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}	
}

void SEASON3B::CNewUIItemExplanationWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUIItemExplanationWindow::UpdateMouseEvent()
{
	return true;
}

bool SEASON3B::CNewUIItemExplanationWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_ITEM_EXPLANATION))
	{
		if(IsPress(VK_ESCAPE) == true || IsPress(VK_F1) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIItemExplanationWindow::Update()
{
	return true;
}

bool SEASON3B::CNewUIItemExplanationWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	extern int ItemHelp;
	extern char TextList[30][100];
	extern int TextListColor[30];
	extern int TextBold[30];
	extern int TextNum;
	extern int g_iItemInfo[12][17];
	
	int iInfoWidth = 0;
	int iLabelHeight = 0;
	int iDataHeight = 0;
	
	switch(WindowWidth)
	{
	case 640:
		iInfoWidth = 90;
		iLabelHeight = 38;
		iDataHeight = 52;
		break;
	case 800:
		iInfoWidth = 90;
		iLabelHeight = 33;
		iDataHeight = 47;
		break;
	case 1024:
		iInfoWidth = 103;
		iLabelHeight = 28;
		iDataHeight = 40;
		break;
	case 1280:
		iInfoWidth = 123;
		iLabelHeight = 22;
		iDataHeight = 32;
		break;
	}
	
	int iType = 0;
	int TabSpace = 0;
	// 항목파악
	if (ItemHelp==ITEM_BOW+7 || ItemHelp==ITEM_BOW+15)
	{
		// 화살과 석궁용화살
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
		return true;
	}
	else if(ItemHelp>=ITEM_SWORD && ItemHelp<ITEM_BOW+MAX_ITEM_INDEX)
	{
		iType = 1;	// 칼
		TabSpace += int(2160 / iInfoWidth);//24
	}
	else if(ItemHelp>=ITEM_STAFF && ItemHelp<ITEM_STAFF+MAX_ITEM_INDEX)
	{
		iType = 2;	// 지팡이
		TabSpace += int(1800 / iInfoWidth);//10
	}
	else if(ItemHelp>=ITEM_SHIELD && ItemHelp<ITEM_SHIELD+MAX_ITEM_INDEX)
	{
		iType = 3;	// 방패
		TabSpace += int(1800 / iInfoWidth);//20
	}
	else if(ItemHelp>=ITEM_HELM && ItemHelp<ITEM_BOOTS+MAX_ITEM_INDEX)
	{
		iType = 4;	// 투구,신발
		TabSpace += int(1800 / iInfoWidth);//36
	}
	else if(ItemHelp>=ITEM_ETC && ItemHelp<ITEM_ETC+MAX_ITEM_INDEX)
	{
		iType = 5;	// 법서

		if (WindowWidth == 640 || WindowWidth == 1280) 
			TabSpace += int(5940 / iInfoWidth);
		else if (WindowWidth == 800 || WindowWidth == 1024) 
			TabSpace += int(5200 / iInfoWidth);
	}
	else // 아이템 설명이 없는 아이템.
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
		return true;
	}

#ifdef CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	if(ItemHelp >= ITEM_STAFF+21 && ItemHelp <= ITEM_STAFF+29)
	{
		iType = 6; // 소환술사 서
		TabSpace += int(800 / iInfoWidth);//20
	}
#endif // CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	
	int iCurrMaxLevel = iMaxLevel;
	
	if( iType == 5 )	// 법서이면
	{
		iCurrMaxLevel = 0;
	}
	
	ITEM_ATTRIBUTE *p = &ItemAttribute[ItemHelp];
	ComputeItemInfo(ItemHelp);	// 아이템 설명 계산
	
	// 출력 상태 초기화
	TextNum = 0;
	sprintf(TextList[TextNum], "\n");
	TextNum++;

	// 160 "아이템 정보"
	strcpy(TextList[TextNum], GlobalText[160]);
	TextListColor[TextNum] = TEXT_COLOR_BLUE; 
	TextBold[TextNum] = true; 
	TextNum++;

	// 아이템 이름
	sprintf(TextList[TextNum], "%s", p->Name);
	TextListColor[TextNum] = TEXT_COLOR_WHITE; 
	TextBold[TextNum] = true; 
	TextNum++;

	sprintf(TextList[TextNum],"\n");
	TextNum++;
	sprintf(TextList[TextNum]," ");
	TextNum++;
	sprintf(TextList[TextNum],"\n");
	TextNum++;
	
	// 배경칠 -_-
	// 정보창 너비 계산
	float fNumAdd = 1.0f;
	if (!(g_iItemInfo[0][_COLUMN_TYPE_ATTMIN] <= 0 || (ItemHelp>=ITEM_ETC && ItemHelp<ITEM_ETC+MAX_ITEM_INDEX)))
		++fNumAdd;
	if (!(g_iItemInfo[0][_COLUMN_TYPE_ATTMAX] <= 0 || (ItemHelp>=ITEM_ETC && ItemHelp<ITEM_ETC+MAX_ITEM_INDEX)))
		++fNumAdd;
	if (g_iItemInfo[0][_COLUMN_TYPE_MAGIC] > 0)
		++fNumAdd;
#ifdef CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	if (g_iItemInfo[0][_COLUMN_TYPE_CURSE] > 0)
		++fNumAdd;
#endif // CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	if (g_iItemInfo[0][_COLUMN_TYPE_PET_ATTACK] > 0)
		++fNumAdd;
	if (g_iItemInfo[0][_COLUMN_TYPE_DEFENCE] > 0)
		fNumAdd += 1.1f;
	if (g_iItemInfo[0][_COLUMN_TYPE_DEFRATE] > 0)
		fNumAdd += 1.1f;
	if (g_iItemInfo[0][_COLUMN_TYPE_REQSTR] > 0)
		++fNumAdd;
	if (g_iItemInfo[0][_COLUMN_TYPE_REQDEX] > 0 || ItemHelp < ITEM_ETC)
		++fNumAdd;
	if (g_iItemInfo[0][_COLUMN_TYPE_REQENG] > 0)
		fNumAdd += 1.1f;
	if (g_iItemInfo[0][_COLUMN_TYPE_REQNLV] > 0)
		fNumAdd += 1.1f;
	if (g_iItemInfo[0][_COLUMN_TYPE_REQVIT] > 0)
		fNumAdd += 1.1f;
	if (g_iItemInfo[0][_COLUMN_TYPE_REQCHA] > 0)
		fNumAdd += 1.1f;
	
	int iAddWidth = float(17 * iInfoWidth / 90) * fNumAdd + 0.5f;
	if (iInfoWidth < iAddWidth)
		iInfoWidth = iAddWidth;

#ifdef CSK_FIX_MAGICALPOWER_INCREASE
	if(iType == 5 && fNumAdd < 3.f)	// 법서이고 카테고리가 하나인 경우 가운데쯤에 표시되기 위해 수정	
	{
		TabSpace += 20;
	}
#endif // CSK_FIX_MAGICALPOWER_INCREASE

	//8 * iFontWidth;	// 화면 해상도에따라 정보창 사이즈 다르다 -_-
	int iInfoNum = (WindowWidth <= 800 ? 46 : 51);
	memset(TextList[TextNum], ' ', iInfoNum); 
	TextList[TextNum][iInfoNum] = '\0';
	TextListColor[TextNum] = TEXT_COLOR_WHITE;
	TextBold[TextNum] = false;
	TextNum++;

	for (int Level = 0; Level <= iCurrMaxLevel - 1; ++Level)
	{
		TextList[TextNum][0] = ' '; TextList[TextNum][1] = '\0';
		TextBold[TextNum] = false;
		TextNum++;
		TextListColor[TextNum] = TEXT_COLOR_WHITE;
	}

	/*
	WORD mixLevel = g_csItemOption.GetMixItemLevel ( ItemHelp );
	if ( HIBYTE( mixLevel)<=3 )
	{
		char Text[100];
		if ( g_csItemOption.GetSetItemName( Text, ItemHelp, 1 ) )
		{
			TextListColor[TextNum] = TEXT_COLOR_GREEN;
			sprintf(TextList[TextNum],"%s %s %s:(%s+%d)", Text, GlobalText[1089], GlobalText[591], GlobalText[1090], HIBYTE( mixLevel ) );TextNum++;
		}
	}
	if ( LOBYTE( mixLevel)<=3 )
	{
		char Text[100];
		if ( g_csItemOption.GetSetItemName( Text, ItemHelp, 2 ) )
		{
			TextListColor[TextNum] = TEXT_COLOR_GREEN;
			sprintf(TextList[TextNum],"%s %s %s:(%s+%d)", Text, GlobalText[1089], GlobalText[591], GlobalText[1090], LOBYTE( mixLevel ) );TextNum++;
		}
	}
	*/
	
	RequireClass(p);
	sprintf(TextList[TextNum],"\n");
	TextNum++;
	RenderTipTextList(1, 1, TextNum, iInfoWidth, RT3_SORT_CENTER);	// 화면 해상도에따라 정보창 사이드 다르다 -_-
	EnableAlphaTest();
	
	TextNum = 0;
	// 레벨
#ifdef CSK_FIX_MAGICALPOWER_INCREASE
	if(iType != 5)	// 법서는 레벨 카테코리가 필요없다.
	{
		RenderHelpCategory(_COLUMN_TYPE_LEVEL, TabSpace, iLabelHeight);
		RenderHelpLine(_COLUMN_TYPE_LEVEL, "+%d", TabSpace, "000000", iDataHeight, iType);
	}
#else // CSK_FIX_MAGICALPOWER_INCREASE
	RenderHelpCategory(_COLUMN_TYPE_LEVEL, TabSpace, iLabelHeight);
	RenderHelpLine(_COLUMN_TYPE_LEVEL, "+%d", TabSpace, "000000", iDataHeight, iType);
#endif // CSK_FIX_MAGICALPOWER_INCREASE
	
	
	if (g_iItemInfo[0][_COLUMN_TYPE_REQNLV] > 0)
	{
		TabSpace += 2;
		RenderHelpCategory(_COLUMN_TYPE_REQNLV, TabSpace, iLabelHeight);
#ifdef CSK_FIX_MAGICALPOWER_INCREASE
		RenderHelpLine(_COLUMN_TYPE_REQNLV, "%3d", TabSpace, "00000", iDataHeight, iType);
		TabSpace += 14;
#else // CSK_FIX_MAGICALPOWER_INCREASE
		RenderHelpLine(_COLUMN_TYPE_REQNLV, "%3d", TabSpace, "00000", iDataHeight);
		TabSpace += 10;
#endif // CSK_FIX_MAGICALPOWER_INCREASE
	}
	
	// 최소공
	if (g_iItemInfo[0][_COLUMN_TYPE_ATTMIN] <= 0 || (ItemHelp>=ITEM_ETC && ItemHelp<ITEM_ETC+MAX_ITEM_INDEX))
	{

	}
	else
	{
		RenderHelpLine(_COLUMN_TYPE_ATTMIN, "%3d", TabSpace, "00 ", iDataHeight);
		RenderHelpCategory(_COLUMN_TYPE_ATTMIN, TabSpace, iLabelHeight);
		TabSpace += 2;
		RenderHelpLine(_COLUMN_TYPE_LEVEL, "~", TabSpace, " 00", iDataHeight);
	}
	
	// 최대공
	if (g_iItemInfo[0][_COLUMN_TYPE_ATTMAX] <= 0 || (ItemHelp>=ITEM_ETC && ItemHelp<ITEM_ETC+MAX_ITEM_INDEX))
	{

	}
	else
	{
		RenderHelpLine(_COLUMN_TYPE_ATTMAX, "%3d", TabSpace, "00000", iDataHeight);
	}
	
	// 마력
	if (g_iItemInfo[0][_COLUMN_TYPE_MAGIC] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_MAGIC, TabSpace, iLabelHeight);
		RenderHelpLine( _COLUMN_TYPE_MAGIC, "%2d%%", TabSpace, "00000", iDataHeight);
	}

#ifdef CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	// 저주력
	if (g_iItemInfo[0][_COLUMN_TYPE_CURSE] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_CURSE, TabSpace, iLabelHeight);
		RenderHelpLine(_COLUMN_TYPE_CURSE, "%2d", TabSpace, "00000", iDataHeight, iType);
	}	
#endif // CSK_FIX_WOPS_K28219_ITEM_EXPLANATION
	
	//  팻 공격력.
	if (g_iItemInfo[0][_COLUMN_TYPE_PET_ATTACK] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_PET_ATTACK, TabSpace, iLabelHeight);
		RenderHelpLine( _COLUMN_TYPE_PET_ATTACK, "%2d%%", TabSpace, "00000", iDataHeight);
	}
	
	// 방어력
	if (g_iItemInfo[0][_COLUMN_TYPE_DEFENCE] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_DEFENCE, TabSpace, iLabelHeight);
		RenderHelpLine(_COLUMN_TYPE_DEFENCE, "%3d", TabSpace, "000000", iDataHeight);
	}
	
	// 방어율
	if (g_iItemInfo[0][_COLUMN_TYPE_DEFRATE] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_DEFRATE, TabSpace, iLabelHeight);
#ifdef CSK_FIX_WOPS_K28204_ITEM_EXPLANATION
		RenderHelpLine(_COLUMN_TYPE_DEFRATE, "%3d", TabSpace, "000000", iDataHeight);
#else // CSK_FIX_WOPS_K28204_ITEM_EXPLANATION
		RenderHelpLine(_COLUMN_TYPE_DEFRATE, "%3d%%", TabSpace, "000000", iDataHeight);
#endif // CSK_FIX_WOPS_K28204_ITEM_EXPLANATION
	}
	
	// 요구힘
	if (g_iItemInfo[0][_COLUMN_TYPE_REQSTR] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_REQSTR, TabSpace, iLabelHeight);
		RenderHelpLine(_COLUMN_TYPE_REQSTR, "%3d", TabSpace, "00000", iDataHeight);
	}
	
	// 요구민
	if (g_iItemInfo[0][_COLUMN_TYPE_REQDEX] > 0 || ItemHelp < ITEM_ETC)
	{
		RenderHelpCategory(_COLUMN_TYPE_REQDEX, TabSpace, iLabelHeight);
#ifdef JAPANESE_ITEM_HELP_FIX
		RenderHelpLine(_COLUMN_TYPE_REQDEX, "%3d", TabSpace, "000000", iDataHeight);
#else	// JAPANESE_ITEM_HELP_FIX
		RenderHelpLine(_COLUMN_TYPE_REQDEX, "%3d", TabSpace, "00000", iDataHeight);
#endif	// JAPANESE_ITEM_HELP_FIX
	}
	
	// 요구 체력
	if (g_iItemInfo[0][_COLUMN_TYPE_REQVIT] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_REQVIT, TabSpace, iLabelHeight);
		RenderHelpLine(_COLUMN_TYPE_REQVIT, "%3d", TabSpace, "00000", iDataHeight);
	}
	
	// 요구에너지
	if (g_iItemInfo[0][_COLUMN_TYPE_REQENG] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_REQENG, TabSpace, iLabelHeight);
		RenderHelpLine(_COLUMN_TYPE_REQENG, "%3d", TabSpace, "00000", iDataHeight, iType);
	}
	
	// 요구 통솔
	if (g_iItemInfo[0][_COLUMN_TYPE_REQCHA] > 0)
	{
		RenderHelpCategory(_COLUMN_TYPE_REQCHA, TabSpace, iLabelHeight);
		RenderHelpLine(_COLUMN_TYPE_REQCHA, "%3d", TabSpace, "00000", iDataHeight);
	}
	
	DisableAlphaBlend();
	return true;
}

float SEASON3B::CNewUIItemExplanationWindow::GetLayerDepth()
{
	return 6.5f;
}

float SEASON3B::CNewUIItemExplanationWindow::GetKeyEventOrder()
{
	return 10.f;
}

void SEASON3B::CNewUIItemExplanationWindow::OpenningProcess()
{

}

void SEASON3B::CNewUIItemExplanationWindow::ClosingProcess()
{

}
