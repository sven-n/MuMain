// NewUIItemEnduranceInfo.cpp: implementation of the CNewUIItemEnduranceInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIItemEnduranceInfo.h"
#include "NewUISystem.h"
#include "wsclientinline.h"

#ifdef PJH_FIX_SPRIT
#include "GIPetManager.h"
#endif //PJH_FIX_SPRIT

extern float g_fScreenRate_x;

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIItemEnduranceInfo::CNewUIItemEnduranceInfo()
{
	memset( &m_UIStartPos, 0, sizeof(POINT) );
	memset( &m_ItemDurUIStartPos, 0, sizeof(POINT) );
	m_iTextEndPosX = 0;
	m_iCurArrowType = ARROWTYPE_NONE;
	m_iTooltipIndex = -1;
}

CNewUIItemEnduranceInfo::~CNewUIItemEnduranceInfo()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool SEASON3B::CNewUIItemEnduranceInfo::Create( CNewUIManager* pNewUIMng, int x, int y )
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_ITEM_ENDURANCE_INFO, this );		// 인터페이스 오브젝트 등록
	
	SetPos( x, y );
	
	LoadImages();

	// 내구도 이미지 인덱스 초기화
	InitImageIndex();
	
	Show( true );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void SEASON3B::CNewUIItemEnduranceInfo::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

//---------------------------------------------------------------------------------------------
// SetPos
void SEASON3B::CNewUIItemEnduranceInfo::SetPos( int x, int y )
{
	m_UIStartPos.x = x;
	m_UIStartPos.y = y;

	m_ItemDurUIStartPos.x = GetScreenWidth() - ITEM_DUR_WIDTH - 2; 
	m_ItemDurUIStartPos.y = 140;
  	
	m_iTextEndPosX = m_UIStartPos.x + PETHP_FRAME_WIDTH;
}

void SEASON3B::CNewUIItemEnduranceInfo::SetPos( int x )
{
	m_ItemDurUIStartPos.x = x - ITEM_DUR_WIDTH - 2; 
	m_ItemDurUIStartPos.y = 140;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool SEASON3B::CNewUIItemEnduranceInfo::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// UI처리 및 이동 불가
	int iNextPosY = m_UIStartPos.y;
	
	// 사탄, 수호천사, 유니리아, 다크호스, 펜릴
	if ( Hero->Helper.Type>=MODEL_HELPER && Hero->Helper.Type<=MODEL_HELPER+4
#ifdef LDK_ADD_PC4_GUARDIAN
		|| Hero->Helper.Type == MODEL_HELPER+64 //데몬
		|| Hero->Helper.Type == MODEL_HELPER+65 //수호정령
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDK_ADD_RUDOLPH_PET
		|| Hero->Helper.Type == MODEL_HELPER+67 //루돌프
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
		|| Hero->Helper.Type == MODEL_HELPER+80 //펜더펫
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
		|| Hero->Helper.Type == MODEL_HELPER+106//유니콘 펫
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
		|| Hero->Helper.Type == MODEL_HELPER+123	// 스켈레톤 펫
#endif	// YDG_ADD_SKELETON_PET
		|| Hero->Helper.Type == MODEL_HELPER+37	)
	{
		if( CheckMouseIn( m_UIStartPos.x, iNextPosY, PETHP_FRAME_WIDTH, PETHP_FRAME_HEIGHT ) )
			return false;
		
		iNextPosY += (UI_INTERVAL_HEIGHT+PETHP_FRAME_HEIGHT);
	}
	
	// 2. 다크로드 펫 (다크스피릿)
	if ( GetBaseClass(Hero->Class) == CLASS_DARK_LORD )
    {
		if( Hero->m_pPet != NULL )
		{
			if( CheckMouseIn( m_UIStartPos.x, iNextPosY, PETHP_FRAME_WIDTH, PETHP_FRAME_HEIGHT ) )
				return false;
			
			iNextPosY += (UI_INTERVAL_HEIGHT+PETHP_FRAME_HEIGHT);
		}
	}
	
	if( GetBaseClass(Hero->Class) == CLASS_ELF )
	{	// 3. 엘프 소환 몬스터
		//소환 몬스터 에너지
		if( SummonLife > 0)
		{
			if( CheckMouseIn( m_UIStartPos.x, iNextPosY, PETHP_FRAME_WIDTH, PETHP_FRAME_HEIGHT ) )
				return false;
			
			iNextPosY += (UI_INTERVAL_HEIGHT+PETHP_FRAME_HEIGHT);
		}
	}

	// 아이템 내구도
	bool bRenderRingWarning = false;
	int	icntItemDurIcon = 0;
	POINT ItemDurPos = POINT(m_ItemDurUIStartPos);

    for( int i = EQUIPMENT_WEAPON_RIGHT; i<MAX_EQUIPMENT; ++i )
    {
		ITEM *pItem = &CharacterMachine->Equipment[i];
		
#ifdef KJH_FIX_RENDER_PERIODITEM_DURABILITY
		// 기간제 아이템이고 기간이 끝나지 않았을 때는 내구도 표시 안함
		if( (pItem->bPeriodItem == true) && (pItem->bExpiredPeriod == false) )
		{
			continue;
		}

#endif // KJH_FIX_RENDER_PERIODITEM_DURABILITY

		if( i == EQUIPMENT_HELPER )
		{
			continue;
		}

		// 아이템이 존재하지 않는다.
		
		if( pItem->Type == -1 )
		{
			continue;
		}

		// 예외
		if( i == EQUIPMENT_WEAPON_RIGHT )
		{
			if( pItem->Type == ITEM_BOW+15 )	// 석궁화살 일때는 내구도 표시안함
			{
				continue;
			}
		}
		else if( i == EQUIPMENT_WEAPON_LEFT)
		{
			if( pItem->Type == ITEM_BOW+7 )		// 화살 일때는 내구도 표시안함.
			{
				continue;
			}
		}		
		
		int iLevel = (pItem->Level>>3)&15;

		// 용사/전사의반지 예외처리
		if( i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
		{
			if( pItem->Type == ITEM_HELPER+20 && iLevel == 1 
				|| iLevel == 2 )
			{
				continue;
			}
		}
			

		ITEM_ATTRIBUTE *pItemAtt = &ItemAttribute[pItem->Type];
		int iMaxDurability = calcMaxDurability( pItem, pItemAtt, iLevel );
	
		if( pItem->Durability <= iMaxDurability*0.5f )		// 50% 남았을때부터 계산
		{
			if( i == EQUIPMENT_RING_RIGHT )
			{
				if( CheckMouseIn( ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH/2, ITEM_DUR_HEIGHT ) )
				{
					m_iTooltipIndex = i;
					return false;
				}
				bRenderRingWarning = true;
			}
			else if( i == EQUIPMENT_RING_LEFT )
			{					
				if( CheckMouseIn( ItemDurPos.x+(ITEM_DUR_WIDTH/2), ItemDurPos.y, ITEM_DUR_WIDTH/2, ITEM_DUR_HEIGHT ) )
				{
					m_iTooltipIndex = i;
					return false;
				}
				bRenderRingWarning = false;
			}
			else
			{
				if( CheckMouseIn( ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH, ITEM_DUR_HEIGHT ) )
				{
					m_iTooltipIndex = i;
					return false;
				}
			}

			if( bRenderRingWarning == false )
			{
				icntItemDurIcon++;
				ItemDurPos.y += ( ITEM_DUR_HEIGHT + UI_INTERVAL_WIDTH );	
				
				if( icntItemDurIcon%2 == 0)
				{
					ItemDurPos.y = m_ItemDurUIStartPos.y;
					ItemDurPos.x -= ( ITEM_DUR_WIDTH + UI_INTERVAL_WIDTH );
				}
			}
			
		}

	
	}


	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool SEASON3B::CNewUIItemEnduranceInfo::UpdateKeyEvent()
{
	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool SEASON3B::CNewUIItemEnduranceInfo::Update()
{
	if( !IsVisible() )
		return true;

	if ( GetBaseClass(Hero->Class)==CLASS_ELF )
    {
#ifdef ADD_SOCKET_ITEM
		//** 활/석궁이 장착되어있지 않으면
		//** 화살/석궁화살이 장착되어도 화살갯수가 표시 안되기 때문에
		//** 활/석궁이 장착되어있는지를 검사하여야 한다.

		// 화살인 경우
		if( GetEquipedBowType( ) == BOWTYPE_BOW )
		{
			m_iCurArrowType = ARROWTYPE_BOW;
		}
		// 석궁 화살인 경우
		else if( GetEquipedBowType( ) == BOWTYPE_CROSSBOW )
		{
		    m_iCurArrowType = ARROWTYPE_CROSSBOW;
	    }
		else
		{
			m_iCurArrowType = ARROWTYPE_NONE;
		}

#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
	    int iRightBowType = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
	    int iLeftBowType = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;

        //  화살일 경우.
	    if( (iLeftBowType>=ITEM_BOW && iLeftBowType<=ITEM_BOW+6) 
			|| iLeftBowType==ITEM_BOW+17 
			|| iLeftBowType==ITEM_BOW+20
			|| iLeftBowType == ITEM_BOW+21 
			|| iLeftBowType == ITEM_BOW+22 )
	    {
		    m_iCurArrowType = ARROWTYPE_BOW;
	    }
        //  석궁 화살일 경우.
	    else if((iRightBowType>=ITEM_BOW+8 && iRightBowType<=ITEM_BOW+14) 
					|| iRightBowType==ITEM_BOW+16 
					|| iRightBowType==ITEM_BOW+18 
					|| iRightBowType>=ITEM_BOW+19)
	    {
		    m_iCurArrowType = ARROWTYPE_CROSSBOW;
	    }
		else
		{
			m_iCurArrowType = ARROWTYPE_NONE;
		}
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
    }
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool SEASON3B::CNewUIItemEnduranceInfo::Render()
{
	EnableAlphaTest();

	glColor3f( 1.f, 1.f, 1.f );
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	
	// 왼쪽에 위치하는 UI정보
	RenderLeft();
	// 오른쪽에 위치하는 UI정보 - 아이템 내구도
	RenderRight();
	
	DisableAlphaBlend();

	return true;
}

//---------------------------------------------------------------------------------------------

void SEASON3B::CNewUIItemEnduranceInfo::RenderLeft()
{
	// Todo
	int iNextPosY = m_UIStartPos.y;
	
#ifndef KJH_DEL_PC_ROOM_SYSTEM			// #ifndef
	// 1. PC방 포인트
	if( RenderPCRoomPoint( m_UIStartPos.x, iNextPosY ) )
	{
		iNextPosY += (UI_INTERVAL_HEIGHT+10);
	}
#endif // KJH_DEL_PC_ROOM_SYSTEM
	if( GetBaseClass(Hero->Class) == CLASS_ELF )
	{
		// 2. 엘프 화살갯수
		if( RenderNumArrow( m_UIStartPos.x, iNextPosY ) )
		{
			iNextPosY += (UI_INTERVAL_HEIGHT+10);
		}
	}

#ifdef PBG_ADD_PCROOM_NEWUI
	// PC방 UI
	if(RenderPCRoomUI(m_UIStartPos.x, iNextPosY))
	{
		iNextPosY += (UI_INTERVAL_HEIGHT+PCROOM_HEIGHT);
	}
#endif //PBG_ADD_PCROOM_NEWUI
	
	// 3.사탄, 수호천사, 유니리아, 다크호스, 펜릴
	if( RenderEquipedHelperLife( m_UIStartPos.x, iNextPosY ) )
	{
		iNextPosY += (UI_INTERVAL_HEIGHT+PETHP_FRAME_HEIGHT);
	}
	
	// 4. 다크로드 펫 (다크스피릿)
	if ( GetBaseClass(Hero->Class) == CLASS_DARK_LORD )
    {
		if( RenderEquipedPetLife( m_UIStartPos.x, iNextPosY ) )
		{
			iNextPosY += (UI_INTERVAL_HEIGHT+PETHP_FRAME_HEIGHT);
		}
	}
	
	if( GetBaseClass(Hero->Class) == CLASS_ELF )
	{	// 5. 엘프 소환 몬스터
		if( RenderSummonMonsterLife( m_UIStartPos.x, iNextPosY ) )
		{
			iNextPosY += (UI_INTERVAL_HEIGHT+PETHP_FRAME_HEIGHT);
		}
	}
}

//---------------------------------------------------------------------------------------------

void SEASON3B::CNewUIItemEnduranceInfo::RenderRight()
{
	// 1. 아이템 내구도 표시
	RenderItemEndurance( m_ItemDurUIStartPos.x, m_ItemDurUIStartPos.y );
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool SEASON3B::CNewUIItemEnduranceInfo::BtnProcess()
{
	
	return false;
}

float SEASON3B::CNewUIItemEnduranceInfo::GetLayerDepth()
{
	return 3.5f;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void SEASON3B::CNewUIItemEnduranceInfo::OpenningProcess()
{

}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void SEASON3B::CNewUIItemEnduranceInfo::ClosingProcess()
{

}

void SEASON3B::CNewUIItemEnduranceInfo::InitImageIndex()
{
	m_iItemDurImageIndex[EQUIPMENT_WEAPON_RIGHT] = IMAGE_ITEM_DUR_WEAPON;
	m_iItemDurImageIndex[EQUIPMENT_WEAPON_LEFT] = IMAGE_ITEM_DUR_SHIELD;
	m_iItemDurImageIndex[EQUIPMENT_HELM] = IMAGE_ITEM_DUR_CAP;
	m_iItemDurImageIndex[EQUIPMENT_ARMOR] = IMAGE_ITEM_DUR_UPPER;
	m_iItemDurImageIndex[EQUIPMENT_PANTS] = IMAGE_ITEM_DUR_LOWER;
	m_iItemDurImageIndex[EQUIPMENT_GLOVES] = IMAGE_ITEM_DUR_GLOVES;
	m_iItemDurImageIndex[EQUIPMENT_BOOTS] = IMAGE_ITEM_DUR_BOOTS;
	m_iItemDurImageIndex[EQUIPMENT_WING] = IMAGE_ITEM_DUR_WING;
	m_iItemDurImageIndex[EQUIPMENT_HELPER] = -1;
	m_iItemDurImageIndex[EQUIPMENT_AMULET] = IMAGE_ITEM_DUR_NECKLACE;
	m_iItemDurImageIndex[EQUIPMENT_RING_RIGHT] = IMAGE_ITEM_DUR_RING;
	m_iItemDurImageIndex[EQUIPMENT_RING_LEFT] = IMAGE_ITEM_DUR_RING;
}

void SEASON3B::CNewUIItemEnduranceInfo::RenderHPUI( int iX, int iY, unicode::t_char* pszName, 
														int iLife, int iMaxLife/*=255*/, bool bWarning/*=false*/ )
{
	EnableAlphaTest();
	
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	
	// HPUI_FRAME	
	if( bWarning == false )
	{	
		glColor4f( 0.f, 0.f, 0.f, 0.7f );
	}
	else
	{
		glColor4f( 0.2f, 0.f, 0.f, 0.7f );
	}

	RenderColor( iX+2, iY+2, PETHP_FRAME_WIDTH-4, PETHP_FRAME_HEIGHT-10 );
	EndRenderColor();

#ifdef PJH_FIX_SPRIT
	if(strcmp(pszName,GlobalText[1214]) == 0)
	{
	int iCharisma = CharacterAttribute->Charisma+CharacterAttribute->AddCharisma;	// 마이너스 열매 작업
	PET_INFO PetInfo;
	giPetManager::GetPetInfo(PetInfo, 421-PET_TYPE_DARK_SPIRIT);
	int RequireCharisma = (185+(PetInfo.m_wLevel*15));
	if( RequireCharisma > iCharisma ) 
		glColor4f ( 1.f, 0.5f, 0.5f, 1.f );
	else
		glColor4f ( 1.f, 1.f, 1.f, 1.f );
	}
	else
#endif //#ifdef PJH_FIX_SPRIT
		glColor4f ( 1.f, 1.f, 1.f, 1.f );
	RenderImage( IMAGE_PETHP_FRAME, iX, iY, PETHP_FRAME_WIDTH, PETHP_FRAME_HEIGHT );					

	// HPUI_Bar
	float fLife = ((float)iLife/(float)iMaxLife)*(float)PETHP_BAR_WIDTH;
	RenderImage( IMAGE_PETHP_BAR, iX+4, iY+PETHP_FRAME_HEIGHT-PETHP_BAR_HEIGHT-4, fLife, PETHP_BAR_HEIGHT );
	// 이름
	g_pRenderText->RenderText( iX+(PETHP_FRAME_WIDTH/2), iY+5, pszName, 0, 0, RT3_WRITE_CENTER );
	
	DisableAlphaBlend();
}

void SEASON3B::CNewUIItemEnduranceInfo::RenderTooltip( int iX, int iY, const ITEM* pItem, const DWORD& dwTextColor )
{
	ITEM_ATTRIBUTE *pItemAtt = &ItemAttribute[pItem->Type];
	SIZE TextSize = {0, 0};	

	int iLevel = (pItem->Level>>3)&15;
	int iMaxDurability = calcMaxDurability( pItem, pItemAtt, iLevel );

	unicode::t_char szText[256] = { NULL, };
	unicode::_sprintf( szText, "%s (%d/%d)", pItemAtt->Name, pItem->Durability, iMaxDurability );
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	g_pMultiLanguage->_GetTextExtentPoint32( g_pRenderText->GetFontDC(), szText, 1, &TextSize );		// Text Size를 계산한다.
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
	unicode::_GetTextExtentPoint( g_pRenderText->GetFontDC(), szText, 1, &TextSize );		// Text Size를 계산한다.
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

	g_pRenderText->SetBgColor( 0, 0, 0, 128 );
	g_pRenderText->SetFont( g_hFontBold );
	g_pRenderText->SetTextColor( dwTextColor );
	int iTextlen = unicode::_strlen( szText );

	int iTooltipWidth = (/*TextSize.cx*/7*iTextlen) / g_fScreenRate_x;

	// 화면 밖으로 넘어가지 않게 처리
	if( iX+(iTooltipWidth/2) > GetScreenWidth() )
	{
		iX = GetScreenWidth() - (iTooltipWidth/2);
	}

	g_pRenderText->RenderText( iX, iY, szText, 0, 0, RT3_WRITE_CENTER );
		
}

bool SEASON3B::CNewUIItemEnduranceInfo::RenderEquipedHelperLife( int iX, int iY )
{
 	// 사탄, 수호천사, 유니리아, 다크호스, 펜릴
 	if ( Hero->Helper.Type>=MODEL_HELPER && Hero->Helper.Type<=MODEL_HELPER+4
#ifdef LDK_ADD_PC4_GUARDIAN
		|| Hero->Helper.Type == MODEL_HELPER+64 //데몬
		|| Hero->Helper.Type == MODEL_HELPER+65 //수호정령
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDK_ADD_RUDOLPH_PET
		|| Hero->Helper.Type == MODEL_HELPER+67 //루돌프
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
		|| Hero->Helper.Type == MODEL_HELPER+80 //펜더
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
		|| Hero->Helper.Type == MODEL_HELPER+106 //유니콘 펫
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
		|| Hero->Helper.Type == MODEL_HELPER+123	// 스켈레톤 펫
#endif	// YDG_ADD_SKELETON_PET
 		|| Hero->Helper.Type == MODEL_HELPER+37	)
 	{
		unicode::t_char szText[256] = {NULL, };

 		switch ( Hero->Helper.Type )
 		{
		case MODEL_HELPER:
			{
				unicode::_sprintf( szText, GlobalText[353] );				// 353 : "수호천사"
			}break;
		case MODEL_HELPER+1:
			{
				ITEM_ATTRIBUTE *p = &ItemAttribute[Hero->Helper.Type-MODEL_SWORD];	
				unicode::_sprintf( szText, p->Name );						// 사탄
			}break;
		case MODEL_HELPER+2:
			{
				unicode::_sprintf( szText,GlobalText[355] );				// 355 : "유니리아"
			}break;
		case MODEL_HELPER+3:
			{
				unicode::_sprintf( szText,GlobalText[354] );				// 354 : "디노란트"
			}break;
        case MODEL_HELPER+4:
			{
				unicode::_sprintf( szText,GlobalText[1187] );				// 1187 : "다크호스"
            }break;
		case MODEL_HELPER+37:	//^ 펜릴 정보 화면 오른쪽 위에 표시
			{
				unicode::_sprintf( szText, GlobalText[1916] );				// 1916 : "펜릴"
			}break;
#ifdef LDK_ADD_PC4_GUARDIAN
		case MODEL_HELPER+64: //데몬
			{
				unicode::_sprintf( szText, ItemAttribute[ITEM_HELPER+64].Name );
			}break;
		case MODEL_HELPER+65: //수호정령
			{
				unicode::_sprintf( szText, ItemAttribute[ITEM_HELPER+65].Name );
			}break;
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef LDK_ADD_RUDOLPH_PET
		case MODEL_HELPER+67: //루돌프
			{
				unicode::_sprintf( szText, ItemAttribute[ITEM_HELPER+67].Name );
			}break;
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
		case MODEL_HELPER+80: //팬더
			{
				unicode::_sprintf( szText, ItemAttribute[ITEM_HELPER+80].Name );
			}break;
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
		case MODEL_HELPER+106: //유니콘 펫
			{
				unicode::_sprintf( szText, ItemAttribute[ITEM_HELPER+106].Name );
			}break;
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
		case MODEL_HELPER+123:	// 스켈레톤 펫
			{
				unicode::_sprintf( szText, ItemAttribute[ITEM_HELPER+123].Name );
			}break;
#endif	// YDG_ADD_SKELETON_PET
		}

		int iLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
		
		RenderHPUI( iX, iY, szText, iLife );

		return true;
	}

	return false;
}

bool SEASON3B::CNewUIItemEnduranceInfo::RenderEquipedPetLife( int iX, int iY )
{
	if( Hero->m_pPet == NULL )
		return false;
		
	unicode::t_char szText[256] = {NULL, };
	unicode::_sprintf( szText, GlobalText[1214] );				// 1214 : 다크스피릿
	
	int iLife = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability;
	
	RenderHPUI( iX, iY, szText, iLife );
	// 
	//     Width=20.f; Height=28.f; x=GetScreenWidth()-Width-PartyWidth-65.f; y=5.f; 
	//     RenderBitmap ( BITMAP_SKILL_INTERFACE+2, (float)x, (float)y, (float)Width-4, (float)Height-8, (((m_byCommand)%8)*32+6.f)/256.f, (((m_byCommand)/8)*Height+3.f)/256.f,Width/256.f,(Height-1.f)/256.f);
	// 
	//     Width -= 8.f; Height -= 8.f;
	//     //  명령 설명을 표시한다.
	//     if ( MouseX>=x && MouseX<=x+Width && MouseY>=y && MouseY<=y+Height )
	//     {
	//         RenderTipText ( (int)x, (int)(y+Height), GlobalText[1219+m_byCommand] );
	//     }
	
	
	return true;
}

bool SEASON3B::CNewUIItemEnduranceInfo::RenderSummonMonsterLife( int iX, int iY )
{
	//소환 몬스터 에너지
    if( SummonLife <= 0)
		return false;
	
	unicode::t_char szText[256] = {NULL, };
	unicode::_sprintf( szText, GlobalText[356] );				// 356 : 소환몬스터
	
	RenderHPUI( iX, iY, szText, SummonLife, 100 );
	
	return true;
}

bool SEASON3B::CNewUIItemEnduranceInfo::RenderNumArrow( int iX, int iY )
{
	// 활이나 석궁이 장착되어있지 않으면 처리하지 않음
	if( m_iCurArrowType == ARROWTYPE_NONE )
		return false;

	unicode::t_char szText[256] = {NULL, };
	int iNumEquipedArrowDurability = 0;			// 장착된 화살의 갯수
	int iNumArrowSetInInven = 0;				// 인벤토리에 있는 화살뭉치 갯수

	if( m_iCurArrowType == ARROWTYPE_BOW )
	{
#ifdef BUGFIX_UI_ARROW_NUM
		// 인벤토리에 있는 화살뭉치 갯수를 구함
		iNumArrowSetInInven = g_pMyInventory->GetNumItemByType( ARROWTYPE_BOW );

		// 화살이 장착되어있으면 화살의 갯수 계산
		if(CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type == ITEM_BOW+15)
		{
			iNumEquipedArrowDurability = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Durability;
		}
				
		// 화살뭉치가 인벤에도 없고, 장착되어있지도 않으면 렌더하지 않음
		if( (iNumArrowSetInInven == 0) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_BOW+15) )
			return false;

		unicode::_sprintf( szText, GlobalText[351], iNumEquipedArrowDurability, iNumArrowSetInInven );
#else // BUGFIX_UI_ARROW_NUM
		iNumEquipedArrowDurability = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Durability;
		iNumArrowSetInInven = g_pMyInventory->GetNumItemByType( ARROWTYPE_BOW );
		unicode::_sprintf( szText, GlobalText[351], iNumEquipedArrowDurability, iNumArrowSetInInven );
#endif // BUGFIX_UI_ARROW_NUM
	}
	else if( m_iCurArrowType == ARROWTYPE_CROSSBOW )
	{
#ifdef BUGFIX_UI_ARROW_NUM
		// 인벤토리에 있는 화살뭉치 갯수를 구함
		iNumArrowSetInInven = g_pMyInventory->GetNumItemByType( ARROWTYPE_CROSSBOW );

		// 화살이 장착되어있으면 화살의 갯수 계산
		if(CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type == ITEM_BOW+7)
		{
			iNumEquipedArrowDurability = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability;
		}
				
		// 화살뭉치가 인벤에도 없고, 장착되어있지도 않으면 렌더하지 않음
		if( (iNumArrowSetInInven == 0) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOW+7) )
			return false;

		unicode::_sprintf( szText, GlobalText[352], iNumEquipedArrowDurability, iNumArrowSetInInven );

#else // BUGFIX_UI_ARROW_NUM
		iNumEquipedArrowDurability = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability;
		iNumArrowSetInInven = g_pMyInventory->GetNumItemByType( ARROWTYPE_CROSSBOW );
		unicode::_sprintf( szText, GlobalText[352], iNumEquipedArrowDurability, iNumArrowSetInInven );
#endif // BUGFIX_UI_ARROW_NUM
	}
	
	g_pRenderText->SetBgColor( 0, 0, 0, 180 );
	g_pRenderText->SetTextColor( 255, 160, 0, 255 );
	g_pRenderText->RenderText( iX, iY, szText, 0, 0, RT3_SORT_LEFT );
	
	return true;
}

#ifndef KJH_DEL_PC_ROOM_SYSTEM		// #ifndef
bool SEASON3B::CNewUIItemEnduranceInfo::RenderPCRoomPoint( int iX, int iY )
{
#ifdef ADD_PCROOM_POINT_SYSTEM
#ifndef ASG_PCROOM_POINT_SYSTEM_MODIFY		// 정리시 삭제.
	// PC방이 아니면 처리하지 않음.
	if ( CPCRoomPtSys::Instance().IsPCRoom() == false )	
		return false;
#endif	// ASG_PCROOM_POINT_SYSTEM_MODIFY	// 정리시 삭제.

	unicode::t_char szText[256] = {NULL, };
	unicode::_sprintf( szText, GlobalText[2319],
		CPCRoomPtSys::Instance().GetNowPoint(), 
		CPCRoomPtSys::Instance().GetMaxPoint() );			// 2319 "PC방 포인트(%d/%d)"
	
	g_pRenderText->SetTextColor(255, 128, 255, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 180);

	g_pRenderText->RenderText( iX, iY, szText, 0, 0, RT3_SORT_LEFT );
#endif //ADD_PCROOM_POINT_SYSTEM
	return true;
}
#endif // KJH_DEL_PC_ROOM_SYSTEM

#ifdef PBG_ADD_PCROOM_NEWUI
bool SEASON3B::CNewUIItemEnduranceInfo::RenderPCRoomUI(int _PosX, int _PosY)
{
	if(!CPCRoomPtSys::Instance().IsPCRoom())
		return false;

	EnableAlphaTest();
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );
	g_pRenderText->SetTextColor(100, 50, 0, 255);
	EndRenderColor();
	glColor4f ( 1.f, 1.f, 1.f, 1.f );
	RenderImage(IMAGE_PCROOM_UI, _PosX, _PosY, PCROOM_WIDTH, PCROOM_HEIGHT);

	unicode::t_char szText[256] = {0,};
	unicode::_sprintf(szText, GlobalText[3062]);
	int _TempPosX=0;
	// 해상도의 따른 위치조정
	switch(WindowWidth)
	{
	case 800:	_TempPosX = 0; break;
	case 1024:	_TempPosX = 4; break;
	case 1280:	_TempPosX = 6; break;
	default:	_TempPosX = 0; break;
	}

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->RenderText(_PosX+_TempPosX, _PosY+PCROOM_HEIGHT*0.4f, szText, 0, 0, RT3_SORT_LEFT);
	
	DisableAlphaBlend();

	RenderPCRoomToolTip(_PosX, _PosY);

#ifdef PBG_FIX_PCROOMFONT
	g_pRenderText->SetFont(g_hFont);
#endif //PBG_FIX_PCROOMFONT

	return true;
}

bool SEASON3B::CNewUIItemEnduranceInfo::RenderPCRoomToolTip(int _PosX, int _PosY)
{
	if(CheckMouseIn(_PosX, _PosY, PCROOM_WIDTH, PCROOM_HEIGHT))
	{
		unicode::t_char _szTempText[256] = {0,};
		
		if(BLUE_MU::IsBlueMuServer())
			unicode::_sprintf(_szTempText, GlobalText[3061]);	//블루 뮤
		else
			unicode::_sprintf(_szTempText, GlobalText[3060]);	//레드 뮤

		unicode::t_char _szText[NUM_LINE_CMB][MAX_TEXT_LENGTH]={0,};
		int _TextLineCnt = ::DivideStringByPixel(&_szText[0][0], NUM_LINE_CMB, MAX_TEXT_LENGTH, _szTempText, 200, true, '/');

		float _fWidth=150, _fHeight=13;
		for (int i=0; i<_TextLineCnt; i++)
		{		
			EnableAlphaTest();	
			glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
			RenderColor ((float)_PosX+PCROOM_WIDTH+5, _PosY+(i*_fHeight)-3, _fWidth, _fHeight);
			glEnable(GL_TEXTURE_2D);

			g_pRenderText->SetBgColor(0, 0, 0, 1.0f);
			g_pRenderText->SetTextColor(255, 255, 255, 255);
			g_pRenderText->SetFont(g_hFont);
			g_pRenderText->RenderText(_PosX+PCROOM_WIDTH+5, _PosY+(i*_fHeight),  _szText[i], _fWidth, 0, RT3_SORT_LEFT);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			DisableAlphaBlend();
		}
	}
	return true;
}
#endif //PBG_ADD_PCROOM_NEWUI

bool SEASON3B::CNewUIItemEnduranceInfo::RenderItemEndurance( int ix, int iY )
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE))
		return false;

    glColor4f( 1.f, 1.f, 1.f, 1.f );
    

	POINT ItemDurPos = POINT(m_ItemDurUIStartPos);
	int icntItemDurIcon = 0;
	int iTempImageIndex;
	bool bRenderRingWarning = false;

	//단계별로 렌더(4단계)
    for ( int i=EQUIPMENT_WEAPON_RIGHT; i<MAX_EQUIPMENT; ++i )
    {
		ITEM *pItem = &CharacterMachine->Equipment[i];

		iTempImageIndex = m_iItemDurImageIndex[i];
		
#ifdef KJH_FIX_RENDER_PERIODITEM_DURABILITY
		// 기간제 아이템이고 기간이 끝나지 않았을 때는 내구도 표시 안함
		if( (pItem->bPeriodItem == true) && (pItem->bExpiredPeriod == false) )
		{
			continue;
		}
#endif // KJH_FIX_RENDER_PERIODITEM_DURABILITY

		if( i == EQUIPMENT_HELPER )
		{
			continue;
		}

		// 아이템이 존재하지 않는다.
		if( pItem->Type == -1 )
		{
			continue;
		}

		// 예외
		if( i == EQUIPMENT_WEAPON_RIGHT )
		{
			if( pItem->Type == ITEM_BOW+15 )			// 석궁화살 일때는 내구도 표시안함
			{
				continue;
			}
		}
		else if( i == EQUIPMENT_WEAPON_LEFT)
		{
			// 활을 사용할때는 왼손을 무기내구도 인덱스로 바꿈	
#ifdef ADD_SOCKET_ITEM
			if( GetEquipedBowType( pItem ) == BOWTYPE_BOW )
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			if( pItem->Type >= ITEM_BOW && pItem->Type <= ITEM_BOW+7 
				|| pItem->Type == ITEM_BOW+17 
				|| pItem->Type >= ITEM_BOW+20 && pItem->Type <= ITEM_BOW+22 )
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			{
				iTempImageIndex = m_iItemDurImageIndex[EQUIPMENT_WEAPON_RIGHT];
			}

			if( pItem->Type == ITEM_BOW+7 )		// 화살 일때는 내구도 표시안함.
			{
				continue;
			}
		}

		int iLevel = (pItem->Level>>3)&15;

		if( i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT)
		{
			if( pItem->Type == ITEM_HELPER+20 && iLevel == 1 
				|| iLevel == 2 )
			{
				continue;
			}
		}

		ITEM_ATTRIBUTE *pItemAtt = &ItemAttribute[pItem->Type];
		int iMaxDurability = calcMaxDurability( pItem, pItemAtt, iLevel );

		// 50% 이상일때 
		if( pItem->Durability > iMaxDurability*0.5f )		
		{
			continue;
		}

		EnableAlphaTest();
		glColor4f( 1.f, 1.f, 1.f, 1.f );

		if( i != EQUIPMENT_RING_LEFT || bRenderRingWarning != true ) 
		{
			RenderImage( m_iItemDurImageIndex[i], ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH, ITEM_DUR_HEIGHT );
		}

		if( i == EQUIPMENT_RING_RIGHT )
		{
			bRenderRingWarning = true;
		}
		
		// 단계별 칼라 바꾸기 (4단계)
		if( pItem->Durability == 0 )							// 0% 남았을때
		{
			glColor4f( 1.f, 0.0f, 0.f, 0.5f );
		}
		else if( pItem->Durability <= iMaxDurability*0.2f )		// 20% 남았을때
		{
			glColor4f( 1.f, 0.2f, 0.0f, 0.5f );
		}
		else if( pItem->Durability <= iMaxDurability*0.3f )		// 30% 남았을때
		{
			glColor4f( 1.0f, 0.5f, 0.f, 0.5f );
		}
		else if( pItem->Durability <= iMaxDurability*0.5f )		// 50% 남았을때
		{
			glColor4f( 1.f, 1.f, 0.f, 0.5f );
		}
		
		if( i == EQUIPMENT_RING_RIGHT )
		{
			RenderColor( ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH/2, ITEM_DUR_HEIGHT );
		}
		else if( i == EQUIPMENT_RING_LEFT )
		{
			RenderColor( ItemDurPos.x+(ITEM_DUR_WIDTH/2), ItemDurPos.y, ITEM_DUR_WIDTH/2, ITEM_DUR_HEIGHT );
			bRenderRingWarning = false;
		}
		else
		{
			RenderColor( ItemDurPos.x, ItemDurPos.y, ITEM_DUR_WIDTH, ITEM_DUR_HEIGHT );
		}
		
		EndRenderColor();
		DisableAlphaBlend();
		
		if( bRenderRingWarning == false )
		{
			icntItemDurIcon++;
			ItemDurPos.y += ( ITEM_DUR_HEIGHT + UI_INTERVAL_WIDTH );	
			
			if( icntItemDurIcon%2 == 0)
			{
				ItemDurPos.y = m_ItemDurUIStartPos.y;
				ItemDurPos.x -= ( ITEM_DUR_WIDTH + UI_INTERVAL_WIDTH );
			}
		}
	}

	// 아이템 내구도 툴팁
	if( m_iTooltipIndex != -1 )
	{
		ITEM *pItem = &CharacterMachine->Equipment[m_iTooltipIndex];	
		DWORD dwColor = 0xFFFFFFFF;

		ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
		int iLevel = (pItem->Level>>3)&15;
		int iMaxDurability = calcMaxDurability(pItem, pItemAttr, iLevel);
		
		if( pItem->Durability <= (iMaxDurability*0.5f))
		{
			if(pItem->Durability <= 0)
			{
				dwColor = 0xFF0000FF;
			}
			else if(pItem->Durability <= (iMaxDurability*0.2f))
			{
				dwColor = 0xFF0053FF;
			}
			else if(pItem->Durability <= (iMaxDurability*0.3f))
			{
				dwColor = 0xFF00A8FF;
			}
			else if(pItem->Durability <= (iMaxDurability*0.5f))
			{
				dwColor = 0xFF00FFFF;
			}
			
			RenderTooltip( MouseX, MouseY-10, pItem, dwColor );
			m_iTooltipIndex = -1;
		}	
	}
	return true;
}

//---------------------------------------------------------------------------------------------
// LoadImages
void SEASON3B::CNewUIItemEnduranceInfo::LoadImages()
{
	LoadBitmap("Interface\\newui_Pet_Back.tga", IMAGE_PETHP_FRAME, GL_LINEAR);
	LoadBitmap("Interface\\newui_Pet_HpBar.jpg", IMAGE_PETHP_BAR, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_boots.tga", IMAGE_ITEM_DUR_BOOTS, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_cap.tga", IMAGE_ITEM_DUR_CAP, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_gloves.tga", IMAGE_ITEM_DUR_GLOVES, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_lower.tga", IMAGE_ITEM_DUR_LOWER, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_necklace.tga", IMAGE_ITEM_DUR_NECKLACE, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_ring.tga",	 IMAGE_ITEM_DUR_RING, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_shield.tga", IMAGE_ITEM_DUR_SHIELD, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_upper.tga", IMAGE_ITEM_DUR_UPPER, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_weapon.tga", IMAGE_ITEM_DUR_WEAPON, GL_LINEAR);
	LoadBitmap("Interface\\newui_durable_wing.tga",	 IMAGE_ITEM_DUR_WING, GL_LINEAR);
#ifdef PBG_ADD_PCROOM_NEWUI
	LoadBitmap("Interface\\newui_PcRoom.tga", IMAGE_PCROOM_UI, GL_LINEAR);
#endif //PBG_ADD_PCROOM_NEWUI
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void SEASON3B::CNewUIItemEnduranceInfo::UnloadImages()
{
	DeleteBitmap( IMAGE_PETHP_FRAME );
	DeleteBitmap( IMAGE_PETHP_BAR );
	DeleteBitmap( IMAGE_ITEM_DUR_BOOTS );
	DeleteBitmap( IMAGE_ITEM_DUR_CAP );
	DeleteBitmap( IMAGE_ITEM_DUR_GLOVES );
	DeleteBitmap( IMAGE_ITEM_DUR_LOWER );
	DeleteBitmap( IMAGE_ITEM_DUR_NECKLACE );
	DeleteBitmap( IMAGE_ITEM_DUR_RING );
	DeleteBitmap( IMAGE_ITEM_DUR_SHIELD );
	DeleteBitmap( IMAGE_ITEM_DUR_UPPER );
	DeleteBitmap( IMAGE_ITEM_DUR_WEAPON );
	DeleteBitmap( IMAGE_ITEM_DUR_WING );
#ifdef PBG_ADD_PCROOM_NEWUI
	DeleteBitmap( IMAGE_PCROOM_UI );
#endif //PBG_ADD_PCROOM_NEWUI
}
