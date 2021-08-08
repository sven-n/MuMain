

#include "stdafx.h"

#include "NewUIHotKey.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "DSPlaySound.h"
#include "GMBattleCastle.h"
#include "CSChaosCastle.h"
#include "w_CursedTemple.h"
#include "ZzzInterface.h"
#include "ZzzLodTerrain.h"
#include "wsclientinline.h"
#include "ZzzEffect.h"
#include "UIMng.h"
#include "MapManager.h"

#ifdef FOR_WORK
	#include "./Utilities/Log/DebugAngel.h"
#endif // FOR_WORK

using namespace SEASON3B;

SEASON3B::CNewUIHotKey::CNewUIHotKey() : m_pNewUIMng(NULL) , m_bStateGameOver(false)
{

}

SEASON3B::CNewUIHotKey::~CNewUIHotKey() 
{ 
	Release(); 
}

bool SEASON3B::CNewUIHotKey::Create(CNewUIManager* pNewUIMng)
{
	if(NULL == pNewUIMng)
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_HOTKEY, this);

	Show(true);		//. 그릴필요 없다
	
	return true;
}

void SEASON3B::CNewUIHotKey::Release()
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

bool SEASON3B::CNewUIHotKey::UpdateMouseEvent()
{
	extern int SelectedCharacter;
	
	if(g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
	{
		return true;
	}

	if(SelectedCharacter >= 0)
	{
		if(SEASON3B::IsRepeat(VK_MENU) && SEASON3B::IsRelease(VK_RBUTTON)
			&& gMapManager.InChaosCastle() == false
			&& gMapManager.IsCursedTemple() == false
			)
		{
			CHARACTER* pCha = &CharactersClient[SelectedCharacter];	
			
			if(pCha->Object.Kind != KIND_PLAYER)
			{
				return false;
			}
			
			if( (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_DEER)
				|| (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_SNOWMAN)
				|| (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_SSANTA)
				)
			{
				return false;
			}

			if (::IsStrifeMap(gMapManager.WorldActive) && Hero->m_byGensInfluence != pCha->m_byGensInfluence)
				return false;

			float fPos_x = pCha->Object.Position[0] - Hero->Object.Position[0];
			float fPos_y = pCha->Object.Position[1] - Hero->Object.Position[1];
			float fDistance = sqrtf((fPos_x * fPos_x) + (fPos_y * fPos_y));
			
			if(fDistance < 300.f)
			{
				int x, y;
				x = MouseX + 10;
				y = MouseY - 50;
				if(y < 0)
				{
					y = 0;
				}
				g_pQuickCommand->OpenQuickCommand(pCha->ID, SelectedCharacter, x, y);
			}
			else
			{
				g_pChatListBox->AddText("", GlobalText[1388], SEASON3B::TYPE_ERROR_MESSAGE);
				g_pQuickCommand->CloseQuickCommand();
			}
			
			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIHotKey::UpdateKeyEvent()
{
	if(SEASON3B::IsPress(VK_ESCAPE) == true)
	{
		if(g_MessageBox->IsEmpty())
		{
			SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSystemMenuMsgBoxLayout));
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	if( m_bStateGameOver == true )
	{
		return false;
	}

	if(SEASON3B::IsPress(VK_TAB) == false && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MINI_MAP) == true)
	{
		return false;
	}

	if(g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
	{
		if (SEASON3B::IsPress('M') == true)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
			PlayBuffer(SOUND_CLICK01);
		}
		return false;
	}

	if(AutoGetItem() == true)
	{
		return false;
	}

	if(CanUpdateKeyEventRelatedMyInventory() == true)
	{
		if(SEASON3B::IsPress('I') || SEASON3B::IsPress('V'))
		{
			if (g_pNPCShop->IsSellingItem() == false)
			{
				g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
				PlayBuffer(SOUND_CLICK01);
				return false;
			}
		}

		return true;
	}
	else if(CanUpdateKeyEvent() == false)
	{
		return true;
	}

	if(SEASON3B::IsPress('F'))
	{
		if(gMapManager.InChaosCastle() == true)
		{
			return true;
		}

		int iLevel = CharacterAttribute->Level;

		if(iLevel < 6)
		{
			if(g_pChatListBox->CheckChatRedundancy(GlobalText[1067]) == FALSE)
			{
				g_pChatListBox->AddText("",GlobalText[1067],SEASON3B::TYPE_SYSTEM_MESSAGE);
			}
		}
		else
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_FRIEND);
		}
		
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('I') || SEASON3B::IsPress('V'))
	{
		if (g_pNPCShop->IsSellingItem() == false)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	else if(SEASON3B::IsPress('C'))
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('T'))
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MYQUEST);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('P'))
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_PARTY);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('G'))
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_GUILDINFO);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('A'))
	{
		if(IsMasterLevel( Hero->Class ) == true 
#ifdef PBG_ADD_NEWCHAR_MONK
			&& GetCharacterClass(Hero->Class) != CLASS_TEMPLENIGHT				// 현재 마스터 스킬트리 없음
#endif //PBG_ADD_NEWCHAR_MONK
			)
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MASTER_LEVEL);
		PlayBuffer(SOUND_CLICK01);

		return false;
	}
	else if(SEASON3B::IsPress('U'))	
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_WINDOW_MENU);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(gMapManager.InChaosCastle() == false && SEASON3B::IsPress('D'))
	{
		if (::IsStrifeMap(gMapManager.WorldActive))
		{
			if (g_pChatListBox->CheckChatRedundancy(GlobalText[2989]) == FALSE)
				g_pChatListBox->AddText("", GlobalText[2989], SEASON3B::TYPE_SYSTEM_MESSAGE);
		}
		else
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_COMMAND);
			PlayBuffer(SOUND_CLICK01);
		}
		
		return false;
	}
	else if(SEASON3B::IsPress(VK_F1) == true)
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_HELP);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('M') == true)
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
		PlayBuffer(SOUND_CLICK01);

		return false;
	}
	else if(SEASON3B::IsPress(VK_TAB) == true && gMapManager.InBattleCastle() == true )
	{
		g_pNewUISystem->Toggle( SEASON3B::INTERFACE_SIEGEWARFARE );
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
#ifdef PJH_ADD_MINIMAP
	else if(SEASON3B::IsPress(VK_TAB) == true)
	{
		if(g_pNewUIMiniMap->m_bSuccess == false)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_MINI_MAP);
		}
		else
			g_pNewUISystem->Toggle( SEASON3B::INTERFACE_MINI_MAP );
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
#endif //PJH_ADD_MINIMAP

#ifdef PBG_ADD_GENSRANKING
	else if(SEASON3B::IsPress('B'))				// 겐스랭킹
	{
		// 겐스에 가입되어 있지 않다면 창을 열지 않는다
		if(!g_pNewUIGensRanking->SetGensInfo())
			return false;

		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_GENSRANKING);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
#endif //PBG_ADD_GENSRANKING
#ifdef YDG_ADD_DOPPELGANGER_UI
// 	else if(SEASON3B::IsPress('Z') == true)		// 도플갱어 테스트
// 	{
// 		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);
// // 		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_DOPPELGANGER_NPC);
// 		return false;
// 	}
#endif	// YDG_ADD_DOPPELGANGER_UI
#if defined LDK_TEST_MAP_EMPIREGUARDIAN
// 	else if(SEASON3B::IsPress('Z') == true)		// ui
// 	{
// 		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
// 		return false;
// 	}
#endif //LDK_TEST_MAP_EMPIREGUARDIAN
#ifdef LDS_ADD_TEST_UNITEDMARKETPLACE
	else if(SEASON3B::IsPress('Z') == true)		// 통합시장 테스트.
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
// 		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
		return false;
	}
#endif // LDS_ADD_TEST_UNITEDMARKETPLACE

	return true;
}

bool SEASON3B::CNewUIHotKey::Update()
{
	return true;
}

bool SEASON3B::CNewUIHotKey::Render()
{
	return true;
}

bool SEASON3B::CNewUIHotKey::CanUpdateKeyEventRelatedMyInventory()
{
	// 인벤토리 핫키만 먹는 경우(I, V)
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)	// 조합창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE)		// 거래창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE)	// 창고창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP)	// NPC상점
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY)			// 개인상점
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)	// 구매상점
#ifdef LEM_ADD_LUCKYITEM
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM

		)
	{
		return true;	
	}
	
	return false;
}

bool SEASON3B::CNewUIHotKey::CanUpdateKeyEvent()
{
	// 아무 핫키도 안먹어야 하는 창은 이곳에 추가해주세요.
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)	// 칸투르 입장창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CATAPULT)			// 공성전 투석기창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCQUEST)			// NPC퀘스트창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_SENATUS)			// 원로원창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GATEKEEPER)		// 시련의 땅 문지기 창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GUARDSMAN)			// 공성 근위병 창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GATESWITCH)		// 공성 성문 스위치창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCGUILDMASTER)	// 길드 생성 창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE)		// 블러드캐슬 입장창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE)		// 악마의광장 입장창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC)	// 환영의사원 입장창
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MASTER_LEVEL)		// 마스터 레벨 창
#ifdef YDG_ADD_NEW_DUEL_UI
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DUELWATCH)			// 결투 문지기 창
#endif	// YDG_ADD_NEW_DUEL_UI
#ifdef YDG_ADD_DOPPELGANGER_UI
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DOPPELGANGER_NPC)	// 도플갱어 루가드 창
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE)		// NPC 대화 창.
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS)	// 퀘스트 진행 창(NPC로 인한)
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_MENU)	// NPC 메뉴창.
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)// 퀘스트 진행 창(기타 등등으로 인한)
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
#ifdef PSW_GOLDBOWMAN
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN)
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA)
#endif //PSW_EVENT_LENA	
#endif // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
#ifdef PBG_MOD_LUCKYCOINEVENT
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION)
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN)
#endif //PBG_MOD_LUCKYCOINEVENT
#ifdef LDK_FIX_EMPIREGUARDIAN_UI_HOTKEY
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC)
#endif	// LDK_FIX_EMPIREGUARDIAN_UI_HOTKEY
#ifdef LDS_ADD_NPC_UNITEDMARKETPLACE
		|| g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA)
#endif // LDS_ADD_NPC_UNITEDMARKETPLACE
		)
	{
		return false;
	}

	return true;
}

float SEASON3B::CNewUIHotKey::GetLayerDepth() 
{ 
	return 1.0f; 
}

float SEASON3B::CNewUIHotKey::GetKeyEventOrder() 
{ 
	return 1.0f; 
}

void SEASON3B::CNewUIHotKey::SetStateGameOver( bool bGameOver )
{
	m_bStateGameOver = bGameOver;
}

bool SEASON3B::CNewUIHotKey::IsStateGameOver()
{
	return m_bStateGameOver;
}

bool SEASON3B::CNewUIHotKey::AutoGetItem()
{
	//  아이템 space로 자동 먹기.
	if (
		CNewUIInventoryCtrl::GetPickedItem() == NULL 
		&& SEASON3B::IsPress(VK_SPACE) 
		&& g_pChatInputBox->HaveFocus() == false
		&& CheckMouseIn(0, 0, GetScreenWidth(), 429)
		)
	{
		for(int i=0; i<MAX_ITEMS; ++i)
		{
			OBJECT* pObj = &Items[i].Object;
			if(pObj->Live && pObj->Visible)
			{
				vec3_t vDir;
				VectorSubtract(pObj->Position, Hero->Object.Position, vDir);
				if(VectorLength(vDir) < 300)
				{
					Hero->MovementType = MOVEMENT_GET;
					ItemKey = i;
					g_bAutoGetItem = true;
					Action(Hero, pObj, true);
					Hero->MovementType = MOVEMENT_MOVE;
					g_bAutoGetItem = false;
					
					return true;
				}
			}
		}
	}

	return false;
}
