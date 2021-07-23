// NewUIPartyListWindow.cpp: implementation of the CNewUIPartyInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIPartyListWindow.h"
#include "NewUISystem.h"
#include "wsclientinline.h"
#include "ZzzInventory.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIPartyListWindow::CNewUIPartyListWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_bActive = false;
	m_iVal = 24;
	m_iLimitUserIDHeight[0] = 48;
	m_iLimitUserIDHeight[1] = 58;
	m_iSelectedCharacter = -1;
	
	for( int i=0 ; i<MAX_PARTYS ; i++ )
	{
		m_iPartyListBGColor[ i ]  = PARTY_LIST_BGCOLOR_DEFAULT;
		m_bPartyMemberoutofSight[ i ] = false;
	}
}

CNewUIPartyListWindow::~CNewUIPartyListWindow()
{
	Release();
}

//---------------------------------------------------------------------------------------------
// Create
bool CNewUIPartyListWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_PARTY_INFO_WINDOW, this );		// 인터페이스 오브젝트 등록
	
	SetPos(x, y);
	
	LoadImages();
	
	// Exit Party Button Initialize
	for( int i=0 ; i<MAX_PARTYS ; i++ )
	{	
		int iVal = i*m_iVal;		// 인덱스에 따른 편차(y의 위치)
		m_BtnPartyExit[i].ChangeButtonImgState( true, IMAGE_PARTY_LIST_EXIT );
		m_BtnPartyExit[i].ChangeButtonInfo( m_Pos.x+63, m_Pos.y+3+iVal, 11, 11 );
	}	

	Show( true );
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void CNewUIPartyListWindow::Release()
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
void CNewUIPartyListWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
	
	for( int i=0 ; i<MAX_PARTYS ; i++ )
	{	
		int iVal = i*m_iVal;		// 인덱스에 따른 편차(y의 위치)
		m_BtnPartyExit[i].ChangeButtonInfo( m_Pos.x+63, m_Pos.y+3+iVal, 11, 11 );
	}
}

void CNewUIPartyListWindow::SetPos(int x)
{
	SetPos(x - (PARTY_LIST_WINDOW_WIDTH+2), m_Pos.y);
}

int CNewUIPartyListWindow::GetSelectedCharacter()
{
	if( m_iSelectedCharacter == -1 )
		return -1;
	
	return Party[m_iSelectedCharacter].index;
}

//---------------------------------------------------------------------------------------------
// SetListBGColor
// * zzzInterface() 가 정리될때 까지 임시로 사용
void CNewUIPartyListWindow::SetListBGColor( )
{
	for( int i=0 ; i<PartyNumber ; i++)
	{	
		m_iPartyListBGColor[ i ] = PARTY_LIST_BGCOLOR_DEFAULT;
		
		if( Party[i].index == -1 )			// 주인공 캐릭터 시야 밖에 있는가
		{
			m_iPartyListBGColor[ i ] = PARTY_LIST_BGCOLOR_RED;
		}
		
		if( Party[i].index>-1 )		// 방어력 향상 버프시 
		{
			m_iPartyListBGColor[ i ]  = PARTY_LIST_BGCOLOR_GREEN;
		}
	}
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool CNewUIPartyListWindow::BtnProcess()
{
	m_iSelectedCharacter = -1;
	// 파티미니창 (항상 Render)
	for( int i=0 ; i<PartyNumber ; i++ )
	{	
		int iVal = i*m_iVal;		// 인덱스에 따른 편차(y의 위치)		
		
		// X 버튼을 눌렀을때 처리
		if( !strcmp( Party[0].Name, Hero->ID ) || !strcmp( Party[i].Name, Hero->ID ) )		// 자신이 파티장일때, 자신과 타인 강퇴
		{
			if( m_BtnPartyExit[i].UpdateMouseEvent() )
			{
				g_pPartyInfoWindow->LeaveParty( i );
				return true;
			}
		}
		
		// 케릭터선택
		if( CheckMouseIn( m_Pos.x, m_Pos.y+iVal, PARTY_LIST_WINDOW_WIDTH, PARTY_LIST_WINDOW_HEIGHT) )
		{
			m_iSelectedCharacter = i;

#ifdef PSW_FIX_PARTYLISTSELECTCHARACTER
			if( SelectedCharacter==-1) {
				CHARACTER *c = &CharactersClient[Party[i].index];
				if( c && c != Hero ) {
					CreateChat ( c->ID, "", c );
				}
			}
#endif //PSW_FIX_PARTYLISTSELECTCHARACTER

			if( SelectCharacterInPartyList(	&Party[i] ) )	
			{
				return true;
			}
		}
	}
	
	return false;
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool CNewUIPartyListWindow::UpdateMouseEvent()
{
	if( !m_bActive )
		return true;
	
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불가
	// ( #include "NewUICommon" )
	if( PartyNumber > 0 )
	{
		int iHeight = (PARTY_LIST_WINDOW_HEIGHT*PartyNumber) + (4*(PartyNumber-1));
		if( CheckMouseIn(m_Pos.x, m_Pos.y, PARTY_LIST_WINDOW_WIDTH, iHeight) )
		{
			return false;
		}
	}
	
	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool CNewUIPartyListWindow::UpdateKeyEvent()
{
	return true;
}

//---------------------------------------------------------------------------------------------
// Update
bool CNewUIPartyListWindow::Update()
{
	if( PartyNumber <= 0 )
	{
		m_bActive = false;
		return true;
	}
	
	m_bActive = true;
	
	for( int i=0 ; i<PartyNumber ; i++)
	{	
/*	// 외부에서 함수 호출( zzzInterface()가 정리되면 주석처리 삭제 )
		m_iPartyListBGColor[ i ] = PARTY_LIST_BGCOLOR_DEFAULT;

		if( Party[i].index == -1 )			// 주인공 캐릭터 시야 밖에 있는가
		{
			m_iPartyListBGColor[ i ] = PARTY_LIST_BGCOLOR_RED;
		}
		
		if( Party[i].index>-1 && (Party[i].state&STATE_DEFENSE)==STATE_DEFENSE)		// 방어력 향상 버프시 
		{
			m_iPartyListBGColor[ i ]  = PARTY_LIST_BGCOLOR_GREEN;
		}
*/
		
		// index 초기화
		// zzzInterface의 SelectObjects() 안에서 index를 재입력
		Party[ i ].index = -2;
	}
	
	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool CNewUIPartyListWindow::Render()
{
	if( !m_bActive )		// 파티원이 있을때 만 렌더
		return true;

	// 파티원 머리위 HP를 렌더
	//RenderPartyHPOnHead();
	
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );
	
	for( int i=0 ; i<PartyNumber ; i++)
	{	
		int iVal = i*m_iVal;		// 인덱스에 따른 편차(y의 위치)
		
		// 배경
		glColor4f ( 0.f, 0.f, 0.f, 0.9f );								// 검정색
		RenderColor( float(m_Pos.x+2), float(m_Pos.y+2+iVal), PARTY_LIST_WINDOW_WIDTH-3, PARTY_LIST_WINDOW_HEIGHT-6 );
		EnableAlphaTest();
		
		// 파티원이 화면 밖으로 나갔을때
		if( Party[i].index == -1 )
		{
			glColor4f ( 0.3f, 0.f, 0.f, 0.5f );						// +레드배경
			RenderColor( m_Pos.x+2, m_Pos.y+2+iVal, PARTY_LIST_WINDOW_WIDTH-3, PARTY_LIST_WINDOW_HEIGHT-6 );
			EnableAlphaTest();
		}
		else
		{	
			if( Party[i].index >= 0 && Party[i].index < MAX_CHARACTERS_CLIENT )
			{
				CHARACTER* pChar = &CharactersClient[Party[i].index];
				OBJECT* pObj = &pChar->Object;
				
				// 방어력 향상 버프를 받았을때
				if(g_isCharacterBuff(pObj, eBuff_Defense) == true)
				{
					glColor4f ( 0.2f, 1.f, 0.2f, 0.2f );					// +녹색배경
					RenderColor( m_Pos.x+2, m_Pos.y+2+iVal, PARTY_LIST_WINDOW_WIDTH-3, PARTY_LIST_WINDOW_HEIGHT-6 );
					EnableAlphaTest();
				}
			}
			// 미니 파티창에 마우스 오버시
			if( m_iSelectedCharacter != -1 && m_iSelectedCharacter == i )	// 마우스로 선택시
			{
				glColor4f ( 0.4f, 0.4f, 0.4f, 0.7f );						// +회색배경
				RenderColor( m_Pos.x+2, m_Pos.y+2+iVal, PARTY_LIST_WINDOW_WIDTH-3, PARTY_LIST_WINDOW_HEIGHT-6 );
				EnableAlphaTest();
			}
		}
		
		EndRenderColor();

		// 배경이미지 렌더
		RenderImage( IMAGE_PARTY_LIST_BACK, m_Pos.x, m_Pos.y+iVal, PARTY_LIST_WINDOW_WIDTH, PARTY_LIST_WINDOW_HEIGHT );
			
		if( i==0 ) // 파티장일때 
		{	
			if( Party[i].index == -1 )		// 화면에 존재하지 않음
			{
				g_pRenderText->SetTextColor( RGBA( 128, 75, 11, 255 ) );		
			}
			else
			{
				g_pRenderText->SetTextColor( RGBA( 255, 148, 22, 255 ) );
			}
			
			// 길드장 표시 깃발
			RenderImage( IMAGE_PARTY_LIST_FLAG, m_Pos.x+53, m_Pos.y+3, 9, 10 );
			g_pRenderText->RenderText( m_Pos.x+4, m_Pos.y+4+iVal, Party[i].Name, m_iLimitUserIDHeight[0], 0, RT3_SORT_LEFT );
		}
		else		// 파티장이 아닐때 
		{
			if( Party[i].index == -1 )		// 화면에 존재하지 않음
			{
				g_pRenderText->SetTextColor( RGBA( 128, 128, 128, 255 ) );
			}
			else
			{
				g_pRenderText->SetTextColor( RGBA( 255, 255, 255, 255 ) );		// 힌색 글자
			}
			g_pRenderText->RenderText( m_Pos.x+4, m_Pos.y+4+iVal, Party[i].Name, m_iLimitUserIDHeight[1], 0, RT3_SORT_LEFT );
		}
		
		// ID ( 파티장-주황생, 파티원-흰색 )
			
		// HPUI_Bar
		int iStepHP = min( 10, Party[i].stepHP );
		float fLife = ((float)iStepHP/(float)10)*(float)PARTY_LIST_HP_BAR_WIDTH;
		RenderImage(IMAGE_PARTY_LIST_HPBAR, m_Pos.x+4, m_Pos.y+16+iVal, fLife, 3);
		
		// 파티에서 나가기 버튼( 파티장일때만, 파티장이 아닐때는 자신만 렌더)
		if( !strcmp( Party[0].Name, Hero->ID ) || !strcmp( Party[i].Name, Hero->ID ) )
		{
			m_BtnPartyExit[i].Render();
		}
	}
	
	DisableAlphaBlend();
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  파티원의 HP 를 화면에 보여준다.
///////////////////////////////////////////////////////////////////////////////
void SEASON3B::CNewUIPartyListWindow::RenderPartyHPOnHead( )
{
    if ( PartyNumber<=0 ) 
		return;

    float   Width = 38.f;
    char    Text[100];

    //  현재는 이름으로 찾는다.
    //  다음에는 index로 찾는다.
    for ( int j=0; j<PartyNumber; ++j )
    {
		PARTY_t *p = &Party[j];

        if (p->index<=-1) continue;  //  화면에 보이지 않는 캐릭터.

        CHARACTER*  c = &CharactersClient[p->index];
	    OBJECT*     o = &c->Object;
	    vec3_t      Position;
	    int         ScreenX, ScreenY;

	    Vector ( o->Position[0], o->Position[1], o->Position[2]+o->BoundingBoxMax[2]+100.f, Position );
		
		BeginOpengl();
		Projection ( Position, &ScreenX, &ScreenY );
		EndOpengl();

        ScreenX -= (int)(Width/2);

        //  마우스와 검사를 한다.
	    if((MouseX>=ScreenX && MouseX<ScreenX+Width && MouseY>=ScreenY-2 && MouseY<ScreenY+6))
	    {
		    sprintf ( Text,"HP : %d0%%", p->stepHP );
			g_pRenderText->SetTextColor(255, 230, 210, 255);
		    g_pRenderText->RenderText(ScreenX, ScreenY-6, Text);
	    }
        //  10단계로 표현한다. 
	    EnableAlphaTest ();
   	    glColor4f ( 0.f, 0.f, 0.f, 0.5f );
	    RenderColor ( (float)(ScreenX+1), (float)(ScreenY+1), Width+4.f, 5.f );

	    EnableAlphaBlend ();
	    glColor3f ( 0.2f, 0.0f, 0.0f );
	    RenderColor ( (float)ScreenX, (float)ScreenY, Width+4.f, 5.f );

        glColor3f ( 50.f/255.f, 10/255.f, 0.f );
	    RenderColor ( (float)(ScreenX+2), (float)(ScreenY+2), Width, 1.f );

        int stepHP = min ( 10, p->stepHP );

        glColor3f ( 250.f/255.f, 10/255.f, 0.f );
        for ( int k=0; k<stepHP; ++k )   //  HP를 표시한다.
        {
	        RenderColor ( (float)(ScreenX+2+(k*4)), (float)(ScreenY+2), 3.f, 2.f );
        }
	    DisableAlphaBlend();
    }
	DisableAlphaBlend();
	glColor3f(1.f,1.f,1.f);
}

//---------------------------------------------------------------------------------------------
// GetLayerDepth
float CNewUIPartyListWindow::GetLayerDepth()
{
	return 5.4f;
}

//---------------------------------------------------------------------------------------------
// OpenningProcess
void CNewUIPartyListWindow::OpenningProcess()
{
	
}

//---------------------------------------------------------------------------------------------
// ClosingProcess
void CNewUIPartyListWindow::ClosingProcess()
{
	
}

//---------------------------------------------------------------------------------------------
// SelectCharacterInPartyList
bool CNewUIPartyListWindow::SelectCharacterInPartyList( PARTY_t* pMember )
{
	/* 
		* Index
		* -1 : 화면에 존재하지 않는다.
		* -2 : default	
		* -3 : 자기 자신(?)
		* 0이상 : SelectCharacter값
	*/
	
    int HeroClass = GetBaseClass( Hero->Class );
	
	//  (요정, 법사) 에 한해서 가능해진다.
	if ( HeroClass==CLASS_ELF 
		|| HeroClass==CLASS_WIZARD 
		|| HeroClass == CLASS_SUMMONER		// 소환술사
		)
	{
		int Skill = CharacterAttribute->Skill[Hero->CurrentSkill];
		
		//  방어력, 공격력, 체력 향상, 순간이동, 솔바리어의 스킬일 경우.
		if ( Skill==AT_SKILL_HEALING 
			|| Skill==AT_SKILL_DEFENSE 
#ifdef PJH_SEASON4_MASTER_RANK4
			|| (AT_SKILL_DEF_POWER_UP <= Skill && Skill <= AT_SKILL_DEF_POWER_UP+4)
			|| (AT_SKILL_ATT_POWER_UP <= Skill && Skill <= AT_SKILL_ATT_POWER_UP+4)
#endif //PJH_SEASON4_MASTER_RANK4
			|| Skill==AT_SKILL_ATTACK 
			|| Skill==AT_SKILL_TELEPORT_B 
			|| Skill==AT_SKILL_WIZARDDEFENSE 
			|| (AT_SKILL_HEAL_UP <= Skill && Skill <= AT_SKILL_HEAL_UP+4)
			|| (AT_SKILL_SOUL_UP <= Skill && Skill <= AT_SKILL_SOUL_UP+4)
			|| Skill == AT_SKILL_ALICE_THORNS		// 소환술사 쏜즈 스킬(데미지반사)
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			|| Skill == AT_SKILL_RECOVER
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			)
		{
			//  커서에 해당되는 캐릭터 선택.
			SelectedCharacter = pMember->index;
			return true;	
		}
	}

	return false;
}


//---------------------------------------------------------------------------------------------
// LoadImages
void CNewUIPartyListWindow::LoadImages()
{
	LoadBitmap("Interface\\newui_party_flag.tga", IMAGE_PARTY_LIST_FLAG, GL_LINEAR);
	LoadBitmap("Interface\\newui_party_x.tga", IMAGE_PARTY_LIST_EXIT, GL_LINEAR);
	
	// 파티 미니창배경
	LoadBitmap("Interface\\newui_party_back.tga", IMAGE_PARTY_LIST_BACK, GL_LINEAR);
	
	// HPBar
	LoadBitmap( "Interface\\newui_party_hpbar.jpg", IMAGE_PARTY_LIST_HPBAR, GL_LINEAR );
}

//---------------------------------------------------------------------------------------------
// UnloadImages
void CNewUIPartyListWindow::UnloadImages()
{
	DeleteBitmap(IMAGE_PARTY_LIST_FLAG);
	DeleteBitmap(IMAGE_PARTY_LIST_EXIT);
	
	// 파티미니창 배경
	DeleteBitmap(IMAGE_PARTY_LIST_BACK);
	
	// HPBar
	DeleteBitmap( IMAGE_PARTY_LIST_HPBAR );
}
