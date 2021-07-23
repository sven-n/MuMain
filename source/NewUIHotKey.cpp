

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
#ifdef PJH_CHARACTER_RENAME
#include "UIMng.h"
#endif //PJH_CHARACTER_RENAME
#ifdef CSK_HACK_TEST
#include "HackTest.h"
#endif // CSK_HACK_TEST
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "InGameShopSystem.h"
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
#ifdef LDK_ADD_SCALEFORM
#include "CGFxProcess.h"
#endif //LDK_ADD_SCALEFORM

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
	
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
	// 관전 버프시 퀵커맨드창이 열리지 않는다
	if(g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
	{
		return true;
	}
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF

	// 퀵커맨드창 단축버튼
	if(SelectedCharacter >= 0)
	{
		if(SEASON3B::IsRepeat(VK_MENU) && SEASON3B::IsRelease(VK_RBUTTON)
			&& InChaosCastle() == false						// 카오스캐슬이 아니고
			&& g_CursedTemple->IsCursedTemple() == false	// 환영사원이 아니면
			)
		{
			CHARACTER* pCha = &CharactersClient[SelectedCharacter];	
			
			if(pCha->Object.Kind != KIND_PLAYER)
			{
				return false;
			}
			
#ifdef KJH_FIX_WOPS_K22844_CHRISTMAS_TRANSFORM_RING_SUMMON_TO_COMMAND
			// 명령어가 적용되지 말아야 할 캐릭터들
			if( (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_DEER)
				|| (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_SNOWMAN)
				|| (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_SSANTA)
				)
			{
				return false;
			}
#endif // KJH_FIX_WOPS_K22844_CHRISTMAS_TRANSFORM_RING_SUMMON_TO_COMMAND

#ifdef ASG_ADD_GENS_SYSTEM
			// 분쟁지역에서는 타 세력 플래이어의 메뉴가 뜨면 안됨.
			if (::IsStrifeMap(World) && Hero->m_byGensInfluence != pCha->m_byGensInfluence)
				return false;
#endif	// ASG_ADD_GENS_SYSTEM
			// 거리 계산
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
				// 1388 "거리가 멀어 사용할 수 없습니다."
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

	// 게임종료 상태일때는 ESC키를 제외한 단축키가 먹지 않는다.
	if( m_bStateGameOver == true )
	{
		return false;
	}

#ifdef PJH_ADD_MINIMAP
	if(SEASON3B::IsPress(VK_TAB) == false && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MINI_MAP) == true)
	{
		return false;
	}
#endif //PJH_ADD_MINIMAP

#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
	// 관전 버프시 ESC키를 제외한 단축키가 먹지 않는다.
	if(g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
	{
		if (SEASON3B::IsPress('M') == true)		// 이동명령만 가능하다 (완전 차단을 위해 아래쪽에서 복사해 왔음: 내용 수정시 아래에 'M'처리도 같이 고쳐야함!)
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
			PlayBuffer(SOUND_CLICK01);
		}
		return false;
	}
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF

	// 스페이스바키 누르면 아이템 자동줍기
	if(AutoGetItem() == true)
	{
		return false;
	}

	// 인벤토리창 단축키만 먹는 창들
	if(CanUpdateKeyEventRelatedMyInventory() == true)
	{
		if(SEASON3B::IsPress('I') || SEASON3B::IsPress('V'))
		{
#ifdef YDG_FIX_NPCSHOP_SELLING_LOCK
			if (g_pNPCShop->IsSellingItem() == false)
#endif	// YDG_FIX_NPCSHOP_SELLING_LOCK
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

	//. 모든 인터페이스 단축키 설정
	if(SEASON3B::IsPress('F'))			// 친구창
	{
		if(InChaosCastle() == true 
#ifndef CSK_FIX_CHAOSFRIENDWINDOW		// 정리할 때 지워야 하는 소스	
			&& g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHAOSCASTLE_TIME) == true
#endif //! CSK_FIX_CHAOSFRIENDWINDOW	// 정리할 때 지워야 하는 소스
			)
		{
			return true;
		}

		int iLevel = CharacterAttribute->Level;

		if(iLevel < 6)
		{
			if(g_pChatListBox->CheckChatRedundancy(GlobalText[1067]) == FALSE)
			{
				g_pChatListBox->AddText("",GlobalText[1067],SEASON3B::TYPE_SYSTEM_MESSAGE);	// "레벨 6부터 내친구 기능 사용이 가능합니다."
			}
		}
		else
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_FRIEND);
		}
		
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('I') || SEASON3B::IsPress('V'))	// 인벤토리창
	{
#ifdef YDG_FIX_NPCSHOP_SELLING_LOCK
		if (g_pNPCShop->IsSellingItem() == false)
#endif	// YDG_FIX_NPCSHOP_SELLING_LOCK
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}
	else if(SEASON3B::IsPress('C'))		// 캐릭터정보창
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('T'))		// 퀘스트창
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MYQUEST);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('P'))		// 파티창
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_PARTY);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('G'))		// 길드정보창
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_GUILDINFO);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}

	else if(SEASON3B::IsPress('A'))
	{


#ifdef DO_PROCESS_DEBUGCAMERA	// 
		if( EDEBUGCAMERA_NONE == g_pDebugCameraManager->GetActiveCameraMode() )
		{
			if(IsMasterLevel( Hero->Class ) == true 
#ifdef KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT
				&& GetCharacterClass(Hero->Class) != CLASS_DIMENSIONMASTER			// 2008.06.13 현재는 소환술사 마스터스킬 불가. 풀릴때 define 주석처리
#endif // KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT
#ifdef PBG_ADD_NEWCHAR_MONK
				&& GetCharacterClass(Hero->Class) != CLASS_TEMPLENIGHT
#endif //PBG_ADD_NEWCHAR_MONK
				)
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MASTER_LEVEL);
			PlayBuffer(SOUND_CLICK01);
		}
#else // DO_PROCESS_DEBUGCAMERA	// 

		if(IsMasterLevel( Hero->Class ) == true 
#ifdef KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT
			&& GetCharacterClass(Hero->Class) != CLASS_DIMENSIONMASTER			// 2008.06.13 현재는 소환술사 마스터스킬 불가. 풀릴때 define 주석처리
#endif // KJH_FIX_WOPS_K22193_SUMMONER_MASTERSKILL_UI_ABNORMAL_TEXT
#ifdef PBG_ADD_NEWCHAR_MONK
			&& GetCharacterClass(Hero->Class) != CLASS_TEMPLENIGHT				// 현재 마스터 스킬트리 없음
#endif //PBG_ADD_NEWCHAR_MONK
			)
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MASTER_LEVEL);
		PlayBuffer(SOUND_CLICK01);

#endif // DO_PROCESS_DEBUGCAMERA

		return false;
	}

	
#if defined NEW_USER_INTERFACE_SHELL && !defined LDK_MOD_GLOBAL_PORTAL_CASHSHOP_BUTTON_DENY
	else if(SEASON3B::IsPress('X')) 
	{
		if( g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_PARTCHARGE_SHOP ) == false ) 
		{
			TheShopServerProxy().SetShopIn();
			return false;
		}
		else 
		{
			SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
			TheShopServerProxy().SetShopOut();
			return false;
		}
	}
#endif //NEW_USER_INTERFACE_SHELL
	else if(SEASON3B::IsPress('U'))		// 윈도우메뉴창
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_WINDOW_MENU);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(InChaosCastle() == false && SEASON3B::IsPress('D'))		// 커맨드 창
	{
#ifdef DO_PROCESS_DEBUGCAMERA
		if( EDEBUGCAMERA_NONE == g_pDebugCameraManager->GetActiveCameraMode() )
		{
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_COMMAND);
			PlayBuffer(SOUND_CLICK01);
		}
#else // DO_PROCESS_DEBUGCAMERA
#ifdef ASG_ADD_GENS_SYSTEM
		if (::IsStrifeMap(World))	// 분쟁지역인가?
		{
			if (g_pChatListBox->CheckChatRedundancy(GlobalText[2989]) == FALSE)	// 메시지 중복을 막음.(하지만 2라인 이상은 막지못하는 버그가 있음)
				g_pChatListBox->AddText("", GlobalText[2989], SEASON3B::TYPE_SYSTEM_MESSAGE);	// "분쟁지역에서는 커맨드창이 활성화 되지 않습니다."
		}
		else
		{
#endif	// ASG_ADD_GENS_SYSTEM
			g_pNewUISystem->Toggle(SEASON3B::INTERFACE_COMMAND);
			PlayBuffer(SOUND_CLICK01);
#ifdef ASG_ADD_GENS_SYSTEM
		}
#endif	// ASG_ADD_GENS_SYSTEM
#endif // DO_PROCESS_DEBUGCAMERA
		
		return false;
	}
	else if(SEASON3B::IsPress(VK_F1) == true)	// 도움말 창
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_HELP);
		PlayBuffer(SOUND_CLICK01);
		return false;
	}
	else if(SEASON3B::IsPress('M') == true)		// 이동 명령 창
	{
		g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
		PlayBuffer(SOUND_CLICK01);

		return false;
	}
	else if(SEASON3B::IsPress(VK_TAB) == true && battleCastle::InBattleCastle() == true )
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

#ifdef _SHOPDEBUGMODE
	else if(SEASON3B::IsPress(VK_F8) == true)
	{
		TheUISystem().SetDebugKey( TheUISystem().IsDebugKey() ? false : true );
		return false;
	}
#endif //_SHOPDEBUGMODE
#ifdef CSK_HACK_TEST
	else if(SEASON3B::IsRepeat(VK_ADD) == true)
	{
		g_pHackTest->PlusAttackSpeed();
	}
	else if(SEASON3B::IsRepeat(VK_SUBTRACT) == true)
	{
		g_pHackTest->MinusAttackSpeed();
	}
	else if(SEASON3B::IsPress('Z') == true)
	{
		g_pHackTest->FindAllMonster();
	}
#endif // CSK_HACK_TEST
	// 인게임샵 단축키 - 국내만적용
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	else if(SEASON3B::IsPress('X') == true)
	{
#ifdef FOR_WORK
		DebugAngel_Write("InGameShopStatue.Txt", "CallStack - CNewUIHotKey.UpdateKeyEvent()\r\n");
#endif // FOR_WORK
		// 인게임샵이 열리면 안돼는 상태
		if(g_pInGameShop->IsInGameShopOpen() == false)
			return false;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
		// 스크립트 다운로드
		if( g_InGameShopSystem->IsScriptDownload() == true )
		{
			if( g_InGameShopSystem->ScriptDownload() == false )
				return false;
		}
		
		// 배너 다운로드
		if( g_InGameShopSystem->IsBannerDownload() == true )
		{
#ifdef KJH_FIX_INGAMESHOP_INIT_BANNER
			if( g_InGameShopSystem->BannerDownload() == true )
			{
				// 배너 초기화
				g_pInGameShop->InitBanner(g_InGameShopSystem->GetBannerFileName(), g_InGameShopSystem->GetBannerURL());
			}
#else // KJH_FIX_INGAMESHOP_INIT_BANNER
			g_InGameShopSystem->BannerDownload();
#endif // KJH_FIX_INGAMESHOP_INIT_BANNER
		}
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

		if( g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == false)
		{
			// 샵 Open 요청중 상태가 아니면 
			if( g_InGameShopSystem->GetIsRequestShopOpenning() == false )		
			{
				SendRequestIGS_CashShopOpen(0);		// 샵 Open요청
				g_InGameShopSystem->SetIsRequestShopOpenning(true);
#ifdef LDK_ADD_SCALEFORM
				if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
				{
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
					g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
				}
#else //LDK_ADD_SCALEFORM
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif //LDK_ADD_SCALEFORM
			}
		}
		else
		{
			SendRequestIGS_CashShopOpen(1);		// 샵 Close요청
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);
		}		

		return false;
	}
#endif // PBG_ADD_INGAMESHOP_UI_MAINFRAME
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
