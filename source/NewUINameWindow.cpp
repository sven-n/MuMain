// NewUINameWindow.cpp: implementation of the CNewUINameWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUINameWindow.h"
#include "ZzzBmd.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "UIControls.h"
#include "CSChaosCastle.h"
#include "PersonalShopTitleImp.h"
#include "MatchEvent.h"
#include "MapManager.h"


using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUINameWindow::CNewUINameWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;

	m_bShowItemName = false;
}

SEASON3B::CNewUINameWindow::~CNewUINameWindow()
{
	Release();
}

bool SEASON3B::CNewUINameWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NAME_WINDOW, this);

	SetPos(x, y);

	Show(true);

	return true;
}

void SEASON3B::CNewUINameWindow::Release()
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}
		
void SEASON3B::CNewUINameWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUINameWindow::UpdateMouseEvent()
{
	return true;
}

bool SEASON3B::CNewUINameWindow::UpdateKeyEvent()
{
	if(SEASON3B::IsPress(VK_MENU) == true)
	{
		m_bShowItemName = !m_bShowItemName;
	}

	return true;
}

bool SEASON3B::CNewUINameWindow::Update()
{
	return true;
}

bool SEASON3B::CNewUINameWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// 아이템 이름, 몬스터 이름, 캐릭터 이름
	RenderName();

	//  매크로 시간, 카운트 다운.
    RenderTimes();

	// 성 이벤트에 대한 UI Render
    matchEvent::RenderMatchTimes ();
    
	// 말풍선 자동 정렬함수.
    RenderBooleans();

	// 개인상점 제목 렌더링
	DrawPersonalShopTitleImp();
	
	DisableAlphaBlend();
	
	return true;
}

void SEASON3B::CNewUINameWindow::RenderName()
{
#ifdef CSK_ADD_GM_ABILITY
	// GM 감시 기능이 켜져 있다면
	if(g_bGMObservation == true)
	{
		for(int i=0; i<MAX_CHARACTERS_CLIENT; i++)
		{
			CHARACTER *c = &CharactersClient[i];
			OBJECT *o = &c->Object;
			if(o->Live && o->Kind == KIND_PLAYER)
			{
				if(IsShopTitleVisible(c) == false)
				{
					CreateChat(c->ID,"",c);
				}
			}
		}
	}
#endif // CSK_ADD_GM_ABILITY
	
#ifndef GUILD_WAR_EVENT
    if(gMapManager.InChaosCastle() == true)		// 카오스캐슬이고
    {
        if(FindText(Hero->ID, "webzen") == false)	// 아이디가 webzen이 아니면
        {
            if(SelectedNpc != -1 || SelectedCharacter != -1 ) // npc와 캐릭터 이름 안그리게 리턴
			{
				return;
			}
        }
    }
#endif//GUILD_WAR_EVENT

	if(SelectedItem != -1 || SelectedNpc != -1 || SelectedCharacter != -1)
	{
		if(SelectedNpc != -1)
		{
			CHARACTER *c = &CharactersClient[SelectedNpc];
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			OBJECT *o = &c->Object;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			CreateChat(c->ID,"",c);
		}
		else if(SelectedCharacter != -1)
		{
			CHARACTER *c = &CharactersClient[SelectedCharacter];

				OBJECT *o = &c->Object;
				if(o->Kind == KIND_MONSTER)
				{
					g_pRenderText->SetTextColor(255, 230, 200, 255);
					g_pRenderText->SetBgColor(100, 0, 0, 255);
					g_pRenderText->RenderText(320, 2, c->ID, 0 ,0, RT3_WRITE_CENTER);
				}
				else
#ifdef ASG_ADD_GENS_SYSTEM
#ifndef PBG_MOD_STRIFE_GENSMARKRENDER
				if (!::IsStrifeMap(World) || Hero->m_byGensInfluence == c->m_byGensInfluence)	// 분쟁지역에서는 타 세력 플래이어의 이름이 보이면 안됨.
#endif //PBG_MOD_STRIFE_GENSMARKRENDER
#endif	// ASG_ADD_GENS_SYSTEM
				{
					if(IsShopTitleVisible(c) == false)
					{
						CreateChat(c->ID,"",c);
					}
				}
		}
		else if(SelectedItem != -1)
		{
			RenderItemName(SelectedItem,&Items[SelectedItem].Object,Items[SelectedItem].Item.Level,Items[SelectedItem].Item.Option1,Items[SelectedItem].Item.ExtOption,false);
		}
	}

	if(m_bShowItemName || SEASON3B::IsRepeat(VK_MENU))
	{
		for(int i=0;i<MAX_ITEMS;i++)
		{
			OBJECT *o = &Items[i].Object;
			if(o->Live)
			{
				if(o->Visible && i!=SelectedItem)
				{
					RenderItemName(i,o,Items[i].Item.Level,Items[i].Item.Option1,Items[i].Item.ExtOption,true);
				}
			}
		}
	}	
}

float SEASON3B::CNewUINameWindow::GetLayerDepth()
{
	return 1.0f;
}

