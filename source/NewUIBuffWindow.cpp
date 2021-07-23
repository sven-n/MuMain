// NewUIBuffWindow.cpp: implementation of the CNewUIBuffWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIBuffWindow.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ZzzTexture.h"
#include "ZzzInventory.h"
#include "UIControls.h"
#include "NewUICommonMessageBox.h"

using namespace SEASON3B;

namespace
{
	const float BUFF_IMG_WIDTH = 20.0f;
	const float BUFF_IMG_HEIGHT = 28.0f;
	const int BUFF_MAX_LINE_COUNT = 8;
	const int BUFF_IMG_SPACE = 5;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIBuffWindow::CNewUIBuffWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

SEASON3B::CNewUIBuffWindow::~CNewUIBuffWindow()
{
	Release();
}

bool SEASON3B::CNewUIBuffWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_BUFF_WINDOW, this );		// 인터페이스 오브젝트 등록
	
	SetPos(x, y);

	LoadImages();
	
	Show( true );

	return true;
}

void SEASON3B::CNewUIBuffWindow::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void SEASON3B::CNewUIBuffWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

void SEASON3B::CNewUIBuffWindow::SetPos(int iScreenWidth)
{
	if(iScreenWidth	== 640) {
		SetPos(220, 15);
	}
	else if(iScreenWidth == 450) {
		SetPos(125, 15);
	}
#ifdef ASG_ADD_UI_QUEST_PROGRESS
	else if(iScreenWidth == 373) {
		SetPos(86, 15);
	}
#endif	// ASG_ADD_UI_QUEST_PROGRESS
	else if(iScreenWidth == 260) {
		SetPos(30, 15);
	}
	else {
		SetPos(220, 15);
	}
}

void SEASON3B::CNewUIBuffWindow::BuffSort( list<eBuffState>& buffstate )
{
	OBJECT* pHeroObject = &Hero->Object;

	int iBuffSize = g_CharacterBuffSize( pHeroObject );
	
	for(int i=0; i<iBuffSize; ++i) 
	{
		eBuffState eBuffType = g_CharacterBuff(pHeroObject, i);

#ifdef PBG_ADD_DISABLERENDER_BUFF
		if(SetDisableRenderBuff(eBuffType))	continue;
#endif //PBG_ADD_DISABLERENDER_BUFF

		if(eBuffType != eBuffNone) {
			eBuffClass eBuffClassType = g_IsBuffClass(eBuffType);
			
			if(eBuffClassType == eBuffClass_Buff) {
				buffstate.push_front( eBuffType );
			}
			else if(eBuffClassType == eBuffClass_DeBuff) {
				buffstate.push_back( eBuffType );
			}
			else {
				assert(!"디버깅 필요");
			}
		}
	}	
}
#ifdef PBG_ADD_DISABLERENDER_BUFF
bool SEASON3B::CNewUIBuffWindow::SetDisableRenderBuff(const eBuffState& _BuffState)
{
	//바로 버프를 지운다 해도 연속적일 경우 렌더가 되는 경우가 생긴다
	switch(_BuffState)
	{
	//Rander만 안할 Buff 등록
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
	case eDeBuff_MoveCommandWin:
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
	case eDeBuff_FlameStrikeDamage:
	case eDeBuff_GiganticStormDamage:
	case eDeBuff_LightningShockDamage:
#ifdef YDG_ADD_GM_DISCHARGE_STAMINA_DEBUFF
	case eDeBuff_Discharge_Stamina:
#endif	// YDG_ADD_GM_DISCHARGE_STAMINA_DEBUFF
		return true;
	default:
		return false;
	}
	return false;
}
#endif //PBG_ADD_DISABLERENDER_BUFF

bool SEASON3B::CNewUIBuffWindow::UpdateMouseEvent()
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	OBJECT* pHeroObject = &Hero->Object;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float x = 0.0f, y = 0.0f;
	int buffwidthcount = 0, buffheightcount = 0;

	list<eBuffState> buffstate;
	BuffSort( buffstate );

	list<eBuffState>::iterator iter;
	for( iter = buffstate.begin(); iter != buffstate.end(); )
	{
		list<eBuffState>::iterator tempiter = iter;
		++iter;
		eBuffState buff = (*tempiter);

		x = m_Pos.x + (buffwidthcount * (BUFF_IMG_WIDTH+BUFF_IMG_SPACE));
		y = m_Pos.y + (buffheightcount * (BUFF_IMG_HEIGHT+BUFF_IMG_SPACE)) ;

		if( SEASON3B::CheckMouseIn(x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT) )
		{
			// 취소할수 있는 버프를 이곳에서 추가. (메세지박스 추가 후)
			if(buff == eBuff_InfinityArrow)		// 인피니티애로우
			{
				if(SEASON3B::IsRelease(VK_RBUTTON))
				{
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CInfinityArrowCancelMsgBoxLayout));
				}
			}
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER		// 마력증대
			else if(buff == eBuff_SwellOfMagicPower)
			{
				if(SEASON3B::IsRelease(VK_RBUTTON))
				{
					SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CBuffSwellOfMPCancelMsgBoxLayOut));
				}
			}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
			
			return false;
		}

		if( ++buffwidthcount >= BUFF_MAX_LINE_COUNT ) {
			buffwidthcount = 0;
			++buffheightcount;
		}
	}
/*
	float x = 0.0f, y = 0.0f;
	int iBuffLineIndex = 0, iDebuffLineIndex = 0;
	
	int iBuffSize = g_CharacterBuffSize( pHeroObject );
	
	for(int i=0; i<iBuffSize; ++i)
	{
		eBuffState eBuffType = g_CharacterBuff(pHeroObject, i);
		
		if(eBuffType == eBuffNone)
		{
			eBuffClass eBuffClassType = g_IsBuffClass(eBuffType);
			
			int iLineIndex = 0;

			if(eBuffClassType == eBuffClass_Buff)
			{
				iLineIndex = iBuffLineIndex++;
				y = m_Pos.y + BUFF_IMG_SPACE;
			}
			else if(eBuffClassType == eBuffClass_DeBuff)
			{
				iLineIndex = iDebuffLineIndex++;
				y = m_Pos.y + BUFF_IMG_HEIGHT + BUFF_IMG_SPACE;
			}
			else
			{
				assert(!"디버깅 필요");
			}
			
			x = m_Pos.x + (iLineIndex * (BUFF_IMG_WIDTH+5));
			
			if( SEASON3B::CheckMouseIn(x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT) )
			{
				if(eBuffType == eBuff_InfinityArrow)
				{
					if(SEASON3B::IsRelease(VK_RBUTTON))
					{
						SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CInfinityArrowCancelMsgBoxLayout));
					}
				}

				return false;
			}
		}
		else {
			assert(!"아무버프도 아닌데 들어왔다. 디버깅 해보삼!");
		}
	}
*/

	return true;
}

bool SEASON3B::CNewUIBuffWindow::UpdateKeyEvent()
{
	return true;
}

bool SEASON3B::CNewUIBuffWindow::Update()
{
	return true;
}

bool SEASON3B::CNewUIBuffWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	// icon
	RenderBuffStatus( BUFF_RENDER_ICON );

	// tooltip
	RenderBuffStatus( BUFF_RENDER_TOOLTIP );
		
	DisableAlphaBlend();

	return true;
}

void SEASON3B::CNewUIBuffWindow::RenderBuffStatus( BUFF_RENDER renderstate )
{
	OBJECT* pHeroObject = &Hero->Object;

	float x = 0.0f, y = 0.0f;
	int buffwidthcount = 0, buffheightcount = 0;
	
	list<eBuffState> buffstate;
	BuffSort( buffstate );

	list<eBuffState>::iterator iter;
	for( iter = buffstate.begin(); iter != buffstate.end(); )
	{
		list<eBuffState>::iterator tempiter = iter;
		++iter;
		eBuffState buff = (*tempiter);
		
		x = m_Pos.x + (buffwidthcount * (BUFF_IMG_WIDTH+BUFF_IMG_SPACE));
		y = m_Pos.y + (buffheightcount * (BUFF_IMG_HEIGHT+BUFF_IMG_SPACE)) ;
		
		if( renderstate == BUFF_RENDER_ICON ) 
		{
			// 버프 아이콘 렌더링
			RenderBuffIcon(buff, x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT);
			// 버프 중첩 카운트
#ifdef _DEBUG
			int iBuffReferenceCount = g_CharacterBuffCount( pHeroObject, buff );
			RenderNumber(x+5, y+5, iBuffReferenceCount, 1.f);
#endif //_DEBUG
		}
		else if( renderstate == BUFF_RENDER_TOOLTIP )
		{
			// 버프 툴팁 렌더링
			if(SEASON3B::CheckMouseIn(x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT)) {
				float fTooltip_x = x + (BUFF_IMG_WIDTH / 2);
				float fTooltip_y = y + BUFF_IMG_WIDTH;
				eBuffClass buffclass = g_IsBuffClass(buff);
				RenderBuffTooltip( buffclass, buff, fTooltip_x, fTooltip_y );
			}
		}
		
		if( ++buffwidthcount >= BUFF_MAX_LINE_COUNT ) {
			buffwidthcount = 0;
			++buffheightcount;
		}
	}

/*	
	float x = 0.0f, y = 0.0f;
	
	int iLineIndex = 0;
	int iBuffLineIndex = 0;
	int iDebuffLineIndex = 0;
	
	int iBuffSize = g_CharacterBuffSize( pHeroObject );
	
	for(int i=0; i<iBuffSize; ++i)
	{
		eBuffState eBuffType = g_CharacterBuff(pHeroObject, i);
		
		if(eBuffType > eBuffNone && eBuffType < eBuff_Count)
		{
			eBuffClass eBuffClassType = g_IsBuffClass(eBuffType);
			
			if(eBuffClassType == eBuffClass_Buff)
			{
				iLineIndex = iBuffLineIndex++;
				y = m_Pos.y + BUFF_IMG_SPACE;
			}
			else if(eBuffClassType == eBuffClass_DeBuff)
			{
				iLineIndex = iDebuffLineIndex++;
				y = m_Pos.y + BUFF_IMG_HEIGHT + BUFF_IMG_SPACE;
			}
			else {
				assert(!"디버깅 필요");
			}
			
			x = m_Pos.x + (iLineIndex * (BUFF_IMG_WIDTH+5));

			if( renderstate == BUFF_RENDER_ICON ) 
			{
				// 버프 아이콘 렌더링
				RenderBuffIcon(eBuffType, x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT);
				// 버프 중첩 카운트
#ifdef _DEBUG
				int iBuffReferenceCount = g_CharacterBuffCount( pHeroObject, eBuffType );
				RenderNumber(x+5, y+5, iBuffReferenceCount, 1.f);
#endif //_DEBUG
			}
			else if( renderstate == BUFF_RENDER_TOOLTIP )
			{
				// 버프 툴팁 렌더링
				if(SEASON3B::CheckMouseIn(x, y, BUFF_IMG_WIDTH, BUFF_IMG_HEIGHT)) {
					float fTooltip_x = x + (BUFF_IMG_WIDTH / 2);
					float fTooltip_y = y + BUFF_IMG_WIDTH;
					RenderBuffTooltip( eBuffClassType, eBuffType, fTooltip_x, fTooltip_y );
				}
			}
		}
		else {
			assert(!"아무버프도 아닌데 들어왔다. 디버깅 해보삼!");
		}
	}
*/
}	

void SEASON3B::CNewUIBuffWindow::RenderBuffIcon(eBuffState& eBuffType, float x, float y, float width, float height)
{
	int iWidthIndex, iHeightIndex;
	float u, v;

#if defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4 || defined PBG_ADD_SANTABUFF
	if (eBuffType < 81) // eBuff_Berserker
	{
		iWidthIndex = (eBuffType - 1) % 10;
		iHeightIndex = (eBuffType - 1) / 10;
		
		u = iWidthIndex * width / 256.f;
		v = iHeightIndex * height / 256.f;
		
		RenderBitmap(IMAGE_BUFF_STATUS, x, y, width, height, u, v, width/256.f, height/256.f);
	}
	else	// 버서커 버프부터 IMAGE_BUFF_STATUS2 사용.
	{
		// eBuff_Santa
		iWidthIndex = (eBuffType - 81) % 10; // eBuff_Berserker
		iHeightIndex = (eBuffType - 81) / 10; // eBuff_Berserker
		
		u = iWidthIndex * width / 256.f;
		v = iHeightIndex * height / 256.f;
		
		RenderBitmap(IMAGE_BUFF_STATUS2, x, y, width, height, u, v, width/256.f, height/256.f);
	}
#else	// defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4  || defined PBG_ADD_SANTABUFF
	iWidthIndex = (eBuffType - 1) % 10;
	iHeightIndex = (eBuffType - 1) / 10;
	
	u = iWidthIndex * width / 256.f;
	v = iHeightIndex * height / 256.f;
	
	RenderBitmap(IMAGE_BUFF_STATUS, x, y, width, height, u, v, width/256.f, height/256.f);
#endif	// defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4
}

void SEASON3B::CNewUIBuffWindow::RenderBuffTooltip(eBuffClass& eBuffClassType, eBuffState& eBuffType, float x, float y)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int TextNum = 0;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int TextNum = 0, SkipNum = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	::memset(TextList[0], 0, sizeof(char) * 30 * 100);
	::memset(TextListColor, 0, sizeof(int) * 30);
	::memset(TextBold, 0, sizeof(int) * 30);
	
	list<string> tooltipinfo;
	g_BuffToolTipString( tooltipinfo, eBuffType );
	
	for( list<string>::iterator iter = tooltipinfo.begin(); iter != tooltipinfo.end(); ++iter )
	{
		string& temp = *iter;
		
		unicode::_sprintf(TextList[TextNum], temp.c_str());
		
		if( TextNum == 0 ) //이름
		{
			TextListColor[TextNum] = TEXT_COLOR_BLUE;
			TextBold[TextNum] = true;
		}
		else // 설명
		{
			TextListColor[TextNum] = TEXT_COLOR_WHITE;
			TextBold[TextNum] = false;
		}
		
		TextNum += 1;
	}
	
	string bufftime;
	g_BuffStringTime( eBuffType, bufftime );
	
	if( bufftime.size() != 0 )
	{
#ifdef CSK_EVENT_CHERRYBLOSSOM
		unicode::_sprintf(TextList[TextNum], GlobalText[2533], bufftime.c_str());
#else //CSK_EVENT_CHERRYBLOSSOM
		unicode::_sprintf(TextList[TextNum], bufftime.c_str());
#endif //CSK_EVENT_CHERRYBLOSSOM
		TextListColor[TextNum] = TEXT_COLOR_PURPLE;
		TextBold[TextNum] = false;
		TextNum += 1;
	}
	
	SIZE TextSize = {0, 0};	
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32( g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize );
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint( g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize );
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	RenderTipTextList( x,y,TextNum, 0 );
}

float SEASON3B::CNewUIBuffWindow::GetLayerDepth()	//. 5.3f
{
	return 0.95f;
}

void SEASON3B::CNewUIBuffWindow::OpenningProcess()
{

}

void SEASON3B::CNewUIBuffWindow::ClosingProcess()
{

}

void SEASON3B::CNewUIBuffWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_statusicon.jpg", IMAGE_BUFF_STATUS, GL_LINEAR);
#if defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4 || defined PBG_ADD_SANTABUFF
	LoadBitmap("Interface\\newui_statusicon2.jpg", IMAGE_BUFF_STATUS2, GL_LINEAR);
#endif	// defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4
}

void SEASON3B::CNewUIBuffWindow::UnloadImages()
{
#if defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4 || defined PBG_ADD_SANTABUFF
	DeleteBitmap(IMAGE_BUFF_STATUS2);
#endif	// defined ASG_ADD_SKILL_BERSERKER || defined PSW_PARTCHARGE_ITEM4
	DeleteBitmap(IMAGE_BUFF_STATUS);	
}
