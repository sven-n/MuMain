
#include "stdafx.h"

#include "NewUISystem.h"
#include "NewUIMessageBox.h"
#include "wsclientinline.h"
#include "PersonalShopTitleImp.h"
#include "w_CursedTemple.h"
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
	#include "ProtectAuto.h"
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
#ifdef LDK_ADD_SCALEFORM
#include "CGFxProcess.h"
#endif //LDK_ADD_SCALEFORM

#ifdef CONSOLE_DEBUG
#include "./Utilities/Log/muConsoleDebug.h"
#endif // CONSOLE_DEBUG

#ifdef FOR_WORK
	#include "./Utilities/Log/DebugAngel.h"
#endif // FOR_WORK

using namespace SEASON3B;

//---------------------------------------------------------------------------------------------
// Constructor
SEASON3B::CNewUISystem::CNewUISystem()
{
	m_pNewUIMng = NULL;
	m_pNewUIHotKey = NULL;
	m_pNewChatLogWindow = NULL;
	m_pNewSlideWindow = NULL;
	m_pNewGuildMakeWindow = NULL;
	m_pNewFriendWindow = NULL;
	m_pNewMainFrameWindow = NULL;
	m_pNewSkillList = NULL;
	m_pNewChatInputBox = NULL;
	m_pNewItemMng = NULL;
	m_pNewMyInventory = NULL;
	m_pNewNPCShop = NULL;
	m_pNewPetInfoWindow = NULL;
	m_pNewMixInventory = NULL;
	m_pNewCastleWindow = NULL;
	m_pNewGuardWindow = NULL;
	m_pNewGatemanWindow = NULL;
	m_pNewGateSwitchWindow = NULL;
	m_pNewStorageInventory = NULL;
	m_pNewGuildInfoWindow = NULL;
	m_pNewMyShopInventory = NULL;
	m_pNewCharacterInfoWindow = NULL;
	m_pNewPartyInfoWindow = NULL;
	m_pNewPartyListWindow = NULL;
	m_pNewNPCQuest = NULL;
	m_pNewEnterBloodCastle = NULL;
	m_pNewEnterDevilSquare = NULL;
	m_pNewBloodCastle = NULL;
	m_pNewTrade = NULL;
	m_pNewKanturu2ndEnterNpc = NULL;
	m_pNewCatapultWindow = NULL;
	m_pNewChaosCastleTime = NULL;
	m_pNewBattleSoccerScore = NULL;
	m_pNewCommandWindow = NULL;
	m_pNewWindowMenu = NULL;
	m_pNewOptionWindow = NULL;
	m_pNewHeroPositionInfo = NULL;
	m_pNewHelpWindow = NULL;
	m_pNewItemExplanationWindow = NULL;
	m_pNewSetItemExplanation = NULL;
	m_pNewQuickCommandWindow = NULL;
	m_pNewMoveCommandWindow = NULL;
	m_pNewSiegeWarfare = NULL;
	m_pNewItemEnduranceInfo = NULL;
	m_pNewBuffWindow = NULL;
	m_pNewCryWolfInterface = NULL;
	m_pNewMaster_Level_Interface = NULL;
	m_pNewCursedTempleResultWindow = NULL;
	m_pNewCursedTempleWindow = NULL;
	m_pNewCursedTempleEnterWindow = NULL;
#ifdef NEW_USER_INTERFACE_SHELL
	m_pNewPartChargeShop = NULL;
#endif //NEW_USER_INTERFACE_SHELL
#ifdef PSW_GOLDBOWMAN
	m_pNewGoldBowman = NULL;
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
	m_pNewGoldBowmanLena = NULL;
#endif //PSW_EVENT_LENA	
	
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	m_pNewLuckyCoinRegistration = NULL;
	m_pNewExchangeLuckyCoinWindow = NULL;
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_UI
	m_pNewDuelWatchWindow = NULL;
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	m_pNewInGameShop = NULL;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef YDG_ADD_DOPPELGANGER_UI
	m_pNewDoppelGangerWindow = NULL;
	m_pNewDoppelGangerFrame = NULL;
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	m_pNewNPCDialogue = NULL;
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
	m_pNewQuestProgress = NULL;
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
	m_pNewNPCMenu = NULL;
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
	m_pNewQuestProgressByEtc = NULL;
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
	m_pNewEmpireGuardianNPC = NULL;
	m_pNewEmpireGuardianTimer = NULL;
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PJH_ADD_MINIMAP
	m_pNewMiniMap = NULL;
#endif //PJH_ADD_MINIMAP
#ifdef PBG_MOD_STAMINA_UI
	m_pNewUIStamina = NULL;
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
	m_pNewGensRanking = NULL;
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
	m_pNewUnitedMarketPlaceWindow = NULL;
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE
}

//---------------------------------------------------------------------------------------------
// Destructor
SEASON3B::CNewUISystem::~CNewUISystem() 
{ 
	Release(); 
}

//---------------------------------------------------------------------------------------------
// Create()
bool SEASON3B::CNewUISystem::Create()
{
	m_pNewUIMng = new CNewUIManager;
	
	m_pNewUI3DRenderMng = new CNewUI3DRenderMng;
	if(false == m_pNewUI3DRenderMng->Create(m_pNewUIMng))
		return false;

	m_pNewChatLogWindow = new CNewUIChatLogWindow;
	if(false == m_pNewChatLogWindow->Create(m_pNewUIMng, 0, 480-50-47, 6))
		return false;

	m_pNewOptionWindow = new CNewUIOptionWindow;
	if(m_pNewOptionWindow->Create(m_pNewUIMng, (640/2)-(190/2), 70) == false)
	{
		return false;
	}

	m_pNewSlideWindow = new CNewUISlideWindow;
	if(m_pNewSlideWindow->Create(m_pNewUIMng) == false)
	{
		return false;
	}

	if(g_MessageBox->Create(m_pNewUIMng) == false)
		return false;

	return true;
}


//---------------------------------------------------------------------------------------------
// Release()
void SEASON3B::CNewUISystem::Release()
{
	UnloadMainSceneInterface();

#ifdef LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
	SAFE_DELETE(m_pNewSlideWindow);
	SAFE_DELETE(m_pNewOptionWindow);
	SAFE_DELETE(m_pNewChatLogWindow);
	SAFE_DELETE(m_pNewUI3DRenderMng);
	
	m_pNewUIMng->RemoveAllUIObjs();

	SAFE_DELETE(m_pNewUIMng);
#else LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
	m_pNewUIMng->RemoveAllUIObjs();

 	SAFE_DELETE(m_pNewSlideWindow);
 	SAFE_DELETE(m_pNewOptionWindow);
 	SAFE_DELETE(m_pNewChatLogWindow);
 	SAFE_DELETE(m_pNewUI3DRenderMng);
 	SAFE_DELETE(m_pNewUIMng);
#endif // LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
}

//---------------------------------------------------------------------------------------------
// LoadMainSceneInterface()
bool SEASON3B::CNewUISystem::LoadMainSceneInterface()
{
	g_MessageBox->Show(true);
	m_pNewChatLogWindow->Show(true);
	m_pNewSlideWindow->Show(true);

	m_pNewItemMng = new CNewUIItemMng;

	m_pNewChatInputBox = new CNewUIChatInputBox;
#ifdef MOD_CHATINPOTBOX_POSITION
	//신규ui 채팅창 겹침
	if(false == m_pNewChatInputBox->Create(m_pNewUIMng, m_pNewChatLogWindow, 0, (GFxProcess::GetInstancePtr()->GetUISelect() == 0 ? 480-51-47 : 480-51-47-15) ))
#else //MOD_CHATINPOTBOX_POSITION
	if(false == m_pNewChatInputBox->Create(m_pNewUIMng, m_pNewChatLogWindow, 0, 480-51-47))
#endif //MOD_CHATINPOTBOX_POSITION
	{
		return false;
	}
	SetFocus(g_hWnd);		//. 메인윈도우로 포커스 초기화

	m_pNewUIHotKey = new CNewUIHotKey;
	if(false == m_pNewUIHotKey->Create(m_pNewUIMng))
		return false;

#ifdef LDK_ADD_SCALEFORM
	if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
	{
		m_pNewMainFrameWindow = new CNewUIMainFrameWindow;
		if(m_pNewMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
			return false;
	}
#else //LDK_ADD_SCALEFORM
	m_pNewMainFrameWindow = new CNewUIMainFrameWindow;
	if(m_pNewMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
		return false;
#endif //LDK_ADD_SCALEFORM

	m_pNewSkillList = new CNewUISkillList;
	if(m_pNewSkillList->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
		return false;

	m_pNewFriendWindow = new CNewUIFriendWindow;
	if(m_pNewFriendWindow->Create(m_pNewUIMng) == false)
		return false;

	m_pNewMyInventory = new CNewUIMyInventory;
	if(false == m_pNewMyInventory->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640-190, 0))
		return false;

	m_pNewNPCShop = new CNewUINPCShop;
	if(false == m_pNewNPCShop->Create(m_pNewUIMng, CNewUINPCShop::NPCSHOP_POS_X, CNewUINPCShop::NPCSHOP_POS_Y))
		return false;

	m_pNewPetInfoWindow = new CNewUIPetInfoWindow;
	if(false == m_pNewPetInfoWindow->Create(m_pNewUIMng, 640-(190*2), 0))
		return false;
	
	m_pNewMixInventory = new CNewUIMixInventory;
	if(m_pNewMixInventory->Create(m_pNewUIMng, 260, 0) == false)
		return false;

	m_pNewCastleWindow = new CNewUICastleWindow;
	if(m_pNewCastleWindow->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	m_pNewGuardWindow = new CNewUIGuardWindow;
	if(m_pNewGuardWindow->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	m_pNewGatemanWindow = new CNewUIGatemanWindow;
	if(m_pNewGatemanWindow->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	m_pNewGateSwitchWindow = new CNewUIGateSwitchWindow;
	if(m_pNewGateSwitchWindow->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	m_pNewStorageInventory = new CNewUIStorageInventory;
	if(m_pNewStorageInventory->Create(m_pNewUIMng, 260, 0) == false)
		return false;

	m_pNewGuildInfoWindow = new CNewUIGuildInfoWindow;
	if(m_pNewGuildInfoWindow->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	m_pNewGuildMakeWindow = new CNewUIGuildMakeWindow;
	if(m_pNewGuildMakeWindow->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	CreatePersonalItemTable();

	m_pNewMyShopInventory = new CNewUIMyShopInventory;
	if(m_pNewMyShopInventory->Create(m_pNewUIMng, 640-190*2, 0) == false)
		return false;
	
	m_pNewPurchaseShopInventory = new CNewUIPurchaseShopInventory;
	if(m_pNewPurchaseShopInventory->Create(m_pNewUIMng, 640-190*2, 0) == false)
		return false;

	m_pNewCharacterInfoWindow = new CNewUICharacterInfoWindow;
	if(m_pNewCharacterInfoWindow->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	m_pNewMyQuestInfoWindow = new CNewUIMyQuestInfoWindow;
	if(m_pNewMyQuestInfoWindow->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;

	m_pNewPartyInfoWindow = new CNewUIPartyInfoWindow;		//  파티 정보 창
	if( m_pNewPartyInfoWindow->Create( m_pNewUIMng, 640-190, 0 ) == false )
		return false;

	m_pNewPartyListWindow = new CNewUIPartyListWindow;		//	파티 미니리스트 창
	if( m_pNewPartyListWindow->Create( m_pNewUIMng, 640-79, 14 ) == false )
		return false;

	m_pNewNPCQuest = new CNewUINPCQuest;
	if (m_pNewNPCQuest->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640-190, 0) == false)
		return false;

	m_pNewEnterBloodCastle = new CNewUIEnterBloodCastle;
	if( m_pNewEnterBloodCastle->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;

	m_pNewEnterDevilSquare = new CNewUIEnterDevilSquare;
	if( m_pNewEnterDevilSquare->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;
	
	m_pNewBloodCastle = new CNewUIBloodCastle;
	if( m_pNewBloodCastle->Create(m_pNewUIMng, 640-127, 480-132) == false )
		return false;

	m_pNewTrade = new CNewUITrade;
	if (m_pNewTrade->Create(m_pNewUIMng, 260, 0) == false)
		return false;

	m_pNewKanturu2ndEnterNpc = new CNewUIKanturu2ndEnterNpc;
	if(m_pNewKanturu2ndEnterNpc->Create(m_pNewUIMng, (640/2) - (230/2), 20) == false)	
	{
		return false;
	}

	m_pNewKanturuInfoWindow = new CNewUIKanturuInfoWindow;
	if(m_pNewKanturuInfoWindow->Create(m_pNewUIMng, 541, 351) == false)	
	{
		return false;
	}

	m_pNewChaosCastleTime = new CNewUIChaosCastleTime;
	if( m_pNewChaosCastleTime->Create(m_pNewUIMng, 640-127, 480-132) == false )
		return false;

	m_pNewBattleSoccerScore = new CNewUIBattleSoccerScore;
	if (m_pNewBattleSoccerScore->Create(m_pNewUIMng, 509, 359) == false)
		return false;
	
	m_pNewCommandWindow = new CNewUICommandWindow;
	if( m_pNewCommandWindow->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;

	m_pNewCatapultWindow = new CNewUICatapultWindow;
	if(m_pNewCatapultWindow->Create(m_pNewUIMng, 640-190, 0) == false)
	{
		return false;
	}

	m_pNewWindowMenu = new CNewUIWindowMenu;
	if(m_pNewWindowMenu->Create(m_pNewUIMng, 640-112, 480-
#ifdef PJH_ADD_MINIMAP
		171
#else
		156
#endif //PJH_ADD_MINIMAP

		) == false)
	{
		return false;
	}

	m_pNewHeroPositionInfo = new CNewUIHeroPositionInfo;
	if(m_pNewHeroPositionInfo->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}

	m_pNewHelpWindow = new CNewUIHelpWindow;
	if(m_pNewHelpWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}

	m_pNewItemExplanationWindow = new CNewUIItemExplanationWindow;
	if(m_pNewItemExplanationWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}

	m_pNewSetItemExplanation = new CNewUISetItemExplanation;
	if(m_pNewSetItemExplanation->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}

	m_pNewQuickCommandWindow = new CNewUIQuickCommandWindow;
	if(m_pNewQuickCommandWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}

#ifdef YDG_ADD_ENC_MOVE_COMMAND_WINDOW
	m_pNewMoveCommandWindow = new CNewUIMoveCommandWindow;
#else	// YDG_ADD_ENC_MOVE_COMMAND_WINDOW
#ifdef CSK_MOD_REMOVE_AUTO_V1_FLAG
	m_pNewMoveCommandWindow = new CNewUIMoveCommandWindowNew;
#else // CSK_MOD_REMOVE_AUTO_V1_FLAG
	m_pNewMoveCommandWindow = new CNewUIMoveCommandWindow;
#endif // CSK_MOD_REMOVE_AUTO_V1_FLAG
#endif	// YDG_ADD_ENC_MOVE_COMMAND_WINDOW
#ifdef YDG_MOD_PROTECT_AUTO_V4
#ifdef YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
	CMoveCommandWindowEncrypt::Enable(g_pProtectAuto->IsNewVersion());
#else	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
	CMoveCommandWindowEncrypt::Enable(TRUE);
#endif	// YDG_MOD_PROTECT_AUTO_FLAG_CHECK_V4
#endif	// YDG_MOD_PROTECT_AUTO_V4
	
	if(m_pNewMoveCommandWindow->Create(m_pNewUIMng, 1, 1) == false)
		return false;

	m_pNewDuelWindow = new CNewUIDuelWindow;
	if(m_pNewDuelWindow->Create(m_pNewUIMng, 509, 359) == false)
	{
		return false;
	}
	
	m_pNewNameWindow = new CNewUINameWindow;
	if(m_pNewNameWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}

	m_pNewSiegeWarfare = new CNewUISiegeWarfare;
	if(m_pNewSiegeWarfare->Create(m_pNewUIMng, 486, 234) == false)
		return false;

	m_pNewItemEnduranceInfo = new CNewUIItemEnduranceInfo;
	if( m_pNewItemEnduranceInfo->Create( m_pNewUIMng, 2, 26 ) == false )
	{
		return false;
	}

	m_pNewBuffWindow = new CNewUIBuffWindow;
	if(m_pNewBuffWindow->Create(m_pNewUIMng, 220, 15) == false)
	{
		return false;
	}

	m_pNewCursedTempleEnterWindow = new CNewUICursedTempleEnter;
	if(m_pNewCursedTempleEnterWindow->Create(m_pNewUIMng, 640/2-230/2, 80) == false)
	{
		return false;
	}
	m_pNewCursedTempleWindow = new CNewUICursedTempleSystem;
	if(m_pNewCursedTempleWindow->Create(m_pNewUIMng, 0, 0) == false)
	{
		return false;
	}
	m_pNewCursedTempleResultWindow = new CNewUICursedTempleResult;
	if(m_pNewCursedTempleResultWindow->Create(m_pNewUIMng, 640/2-230/2, 120) == false)
	{
		return false;
	}

	m_pNewCryWolfInterface = new CNewUICryWolf;
	if(m_pNewCryWolfInterface->Create(m_pNewUIMng, 0, 0) == false)
		return false;

	m_pNewMaster_Level_Interface = new CNewUIMasterLevel;
	if(m_pNewMaster_Level_Interface->Create(m_pNewUIMng, 0, 0) == false)
		return false;

#ifdef PJH_ADD_MINIMAP
	m_pNewMiniMap = new CNewUIMiniMap;
	if(m_pNewMiniMap->Create(m_pNewUIMng, 0, 0) == false)
		return false;
#endif //PJH_ADD_MINIMAP

#ifdef NEW_USER_INTERFACE_SHELL
	m_pNewPartChargeShop = new CNewUIPartChargeShop;
	if(m_pNewPartChargeShop->Create(m_pNewUIMng, 0, 0) == false)
		return false;
#endif //NEW_USER_INTERFACE_SHELL

#ifdef PSW_GOLDBOWMAN
	m_pNewGoldBowman = new CNewUIGoldBowmanWindow;
	if( m_pNewGoldBowman->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;
#endif //PSW_GOLDBOWMAN

#ifdef PSW_EVENT_LENA
	m_pNewGoldBowmanLena = new CNewUIGoldBowmanLena;
	if( m_pNewGoldBowmanLena->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;
#endif //PSW_EVENT_LENA	

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	m_pNewLuckyCoinRegistration = new CNewUIRegistrationLuckyCoin;
	if( m_pNewLuckyCoinRegistration->Create(m_pNewUIMng, 640 - (190 * 2), 0) == false )
		return false;	

	m_pNewExchangeLuckyCoinWindow = new CNewUIExchangeLuckyCoin;
	if( m_pNewExchangeLuckyCoinWindow->Create(m_pNewUIMng, 640-(190*2), 0) == false )
		return false;
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008

#ifdef YDG_ADD_NEW_DUEL_UI
	m_pNewDuelWatchWindow = new CNewUIDuelWatchWindow;
	if( m_pNewDuelWatchWindow->Create(m_pNewUIMng, 640-190, 0) == false )
		return false;
#endif	// YDG_ADD_NEW_DUEL_UI	

#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
	m_pNewDuelWatchMainFrameWindow = new CNewUIDuelWatchMainFrameWindow;
	if(m_pNewDuelWatchMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
		return false;

	m_pNewDuelWatchUserListWindow = new CNewUIDuelWatchUserListWindow;
	if(m_pNewDuelWatchUserListWindow->Create(m_pNewUIMng, 640-57, 480-51) == false)
		return false;
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	m_pNewInGameShop = new CNewUIInGameShop;
	if(m_pNewInGameShop->Create(m_pNewUIMng, 0, 0) == false)
		return false;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

#ifdef YDG_ADD_DOPPELGANGER_UI
	m_pNewDoppelGangerWindow = new CNewUIDoppelGangerWindow;
	if( m_pNewDoppelGangerWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640-190, 0) == false )
		return false;

	m_pNewDoppelGangerFrame = new CNewUIDoppelGangerFrame;
#ifdef MOD_DOPPELGANGERFRAME_POSITION
	// 도플갱어 진행 UI 위치 수정(2010.09.07)
	float _cx = 640-227;
	float _cy = GFxProcess::GetInstancePtr()->GetUISelect() == 0 ? 480-51-87 : 480-51-87-15;
	if( m_pNewDoppelGangerFrame->Create(m_pNewUIMng, _cx, _cy) == false )
#else //MOD_DOPPELGANGERFRAME_POSITION
	if( m_pNewDoppelGangerFrame->Create(m_pNewUIMng, 640-227, 480-51-87) == false )
#endif //MOD_DOPPELGANGERFRAME_POSITION
		return false;
#endif	// YDG_ADD_DOPPELGANGER_UI

#ifdef ASG_ADD_UI_NPC_DIALOGUE
	m_pNewNPCDialogue = new CNewUINPCDialogue;
	if (m_pNewNPCDialogue->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;
#endif	// ASG_ADD_UI_NPC_DIALOGUE

#ifdef ASG_ADD_UI_QUEST_PROGRESS
	m_pNewQuestProgress = new CNewUIQuestProgress;
#ifdef ASG_MOD_3D_CHAR_EXCLUSION_UI
	if (m_pNewQuestProgress->Create(m_pNewUIMng, 640-190, 0) == false)
#else	// ASG_MOD_3D_CHAR_EXCLUSION_UI
	if (m_pNewQuestProgress->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640-267, 0) == false)
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
		return false;
#endif	// ASG_ADD_UI_QUEST_PROGRESS

#ifdef ASG_ADD_UI_NPC_MENU
	m_pNewNPCMenu = new CNewUINPCMenu;
	if (m_pNewNPCMenu->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640-190, 0) == false)
		return false;
#endif	// ASG_ADD_UI_NPC_MENU

#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
	m_pNewQuestProgressByEtc = new CNewUIQuestProgressByEtc;
	if (m_pNewQuestProgressByEtc->Create(m_pNewUIMng, 640-190, 0) == false)
		return false;
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC

#ifdef LDK_ADD_EMPIREGUARDIAN_UI
	m_pNewEmpireGuardianNPC = new CNewUIEmpireGuardianNPC;
	if( m_pNewEmpireGuardianNPC->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 450, 0) == false )
		return false;

	m_pNewEmpireGuardianTimer = new CNewUIEmpireGuardianTimer;
	if( m_pNewEmpireGuardianTimer->Create(m_pNewUIMng, 507, 342) == false )
		return false;
#endif //LDK_ADD_EMPIREGUARDIAN_UI
	
#ifdef PBG_MOD_STAMINA_UI
	m_pNewUIStamina = new CNewUIStamina;
	if( m_pNewUIStamina->Create(m_pNewUIMng, 640, 480) == false )
		return false;
#endif //PBG_MOD_STAMINA_UI

#ifdef PBG_ADD_GENSRANKING
	m_pNewGensRanking = new CNewUIGensRanking;
	if(m_pNewGensRanking->Create(m_pNewUIMng, 640, 480) == false)
		return false;
#endif //PBG_ADD_GENSRANKING

#ifdef LDS_ADD_UI_UNITEDMARKETPLACE	// 통합 시장 맵 UI 클래스 추가.
	m_pNewUnitedMarketPlaceWindow = new CNewUIUnitedMarketPlaceWindow;
	if( m_pNewUnitedMarketPlaceWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640-190, 0) == false )
		return false;
#endif // LDS_ADD_UI_UNITEDMARKETPLACE

#ifdef LEM_ADD_LUCKYITEM
	m_pNewUILuckyItemWnd = new CNewUILuckyItemWnd;
	if(m_pNewUILuckyItemWnd->Create(m_pNewUIMng, 260, 0) == false)
		return false;
#endif // LEM_ADD_LUCKYITEM

	return true;
}

//---------------------------------------------------------------------------------------------
// UnloadMainSceneInterface()
void SEASON3B::CNewUISystem::UnloadMainSceneInterface()
{
	if(g_pNewUIMng)
	{
		g_pNewUIMng->ShowAllInterfaces(false);
	}

	SAFE_DELETE(m_pNewHelpWindow);
	SAFE_DELETE(m_pNewItemExplanationWindow);
	SAFE_DELETE(m_pNewSetItemExplanation);
	SAFE_DELETE(m_pNewQuickCommandWindow);
	SAFE_DELETE(m_pNewWindowMenu);
	SAFE_DELETE(m_pNewBattleSoccerScore);
	SAFE_DELETE(m_pNewCatapultWindow);
	SAFE_DELETE(m_pNewKanturu2ndEnterNpc);
	SAFE_DELETE(m_pNewKanturuInfoWindow);
	SAFE_DELETE(m_pNewTrade);
	SAFE_DELETE(m_pNewNPCQuest);
	SAFE_DELETE(m_pNewMyQuestInfoWindow);
	SAFE_DELETE(m_pNewCharacterInfoWindow);
	SAFE_DELETE(m_pNewPurchaseShopInventory);
	SAFE_DELETE(m_pNewMyShopInventory);
	SAFE_DELETE(m_pNewGuildMakeWindow);
	SAFE_DELETE(m_pNewGuildInfoWindow);
	SAFE_DELETE(m_pNewStorageInventory);
	SAFE_DELETE(m_pNewMixInventory);
	SAFE_DELETE(m_pNewCastleWindow);
	SAFE_DELETE(m_pNewGuardWindow);
	SAFE_DELETE(m_pNewGatemanWindow);
	SAFE_DELETE(m_pNewGateSwitchWindow);
	SAFE_DELETE(m_pNewNPCShop);
	SAFE_DELETE(m_pNewPetInfoWindow);
	SAFE_DELETE(m_pNewMyInventory);
	SAFE_DELETE(m_pNewFriendWindow);
	SAFE_DELETE(m_pNewChatInputBox);
	SAFE_DELETE(m_pNewNameWindow);
	SAFE_DELETE(m_pNewSkillList);
	SAFE_DELETE(m_pNewMainFrameWindow);
	SAFE_DELETE( m_pNewPartyInfoWindow );
	SAFE_DELETE( m_pNewPartyListWindow );
	SAFE_DELETE( m_pNewEnterBloodCastle );
	SAFE_DELETE( m_pNewEnterDevilSquare );
	SAFE_DELETE( m_pNewBloodCastle );
	SAFE_DELETE( m_pNewChaosCastleTime );
	SAFE_DELETE( m_pNewCommandWindow );
	SAFE_DELETE( m_pNewHeroPositionInfo );
	SAFE_DELETE( m_pNewMoveCommandWindow );
	SAFE_DELETE(m_pNewUIHotKey);
	SAFE_DELETE( m_pNewSiegeWarfare);
	SAFE_DELETE( m_pNewItemEnduranceInfo );
	SAFE_DELETE(m_pNewBuffWindow);
	SAFE_DELETE(m_pNewCursedTempleResultWindow);
	SAFE_DELETE(m_pNewCursedTempleWindow);
	SAFE_DELETE(m_pNewCursedTempleEnterWindow);
	SAFE_DELETE(m_pNewCryWolfInterface);
	SAFE_DELETE(m_pNewMaster_Level_Interface);
#ifdef NEW_USER_INTERFACE_SHELL
	SAFE_DELETE(m_pNewPartChargeShop);
#endif //NEW_USER_INTERFACE_SHELL

#ifdef PSW_GOLDBOWMAN
	SAFE_DELETE(m_pNewGoldBowman);
#endif //PSW_GOLDBOWMAN

#ifdef PSW_EVENT_LENA
	SAFE_DELETE(m_pNewGoldBowmanLena);
#endif //PSW_EVENT_LENA	

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	SAFE_DELETE(m_pNewLuckyCoinRegistration);
	SAFE_DELETE(m_pNewExchangeLuckyCoinWindow);
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008

#ifdef YDG_ADD_NEW_DUEL_UI
	SAFE_DELETE(m_pNewDuelWatchWindow);
#endif	// YDG_ADD_NEW_DUEL_UI	

#ifdef YDG_FIX_MEMORY_LEAK_0905
	SAFE_DELETE(m_pNewDuelWindow);

#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
	SAFE_DELETE(m_pNewDuelWatchMainFrameWindow);
	SAFE_DELETE(m_pNewDuelWatchUserListWindow);
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#endif	// YDG_FIX_MEMORY_LEAK_0905
	
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
	SAFE_DELETE(m_pNewInGameShop);
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

#ifdef YDG_ADD_DOPPELGANGER_UI
	SAFE_DELETE(m_pNewDoppelGangerWindow);
	SAFE_DELETE(m_pNewDoppelGangerFrame);
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	SAFE_DELETE(m_pNewNPCDialogue);
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
	SAFE_DELETE(m_pNewQuestProgress);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
	SAFE_DELETE(m_pNewNPCMenu);
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
	SAFE_DELETE(m_pNewQuestProgressByEtc);
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
	SAFE_DELETE(m_pNewEmpireGuardianNPC);
	SAFE_DELETE(m_pNewEmpireGuardianTimer);
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PJH_ADD_MINIMAP
	SAFE_DELETE(m_pNewMiniMap);
#endif //PJH_ADD_MINIMAP
	SAFE_DELETE(m_pNewItemMng);
#ifdef PBG_MOD_STAMINA_UI
	SAFE_DELETE(m_pNewUIStamina);
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
	SAFE_DELETE(m_pNewGensRanking);
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
	SAFE_DELETE(m_pNewUnitedMarketPlaceWindow);
#endif // LDS_ADD_MAP_UNITEDMARKETPLACE

#ifdef LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
	SAFE_DELETE( m_pNewUILuckyItemWnd );
#endif // LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE

	ReleasePersonalItemTable();
}

//---------------------------------------------------------------------------------------------
// IsVisible()
bool SEASON3B::CNewUISystem::IsVisible(DWORD dwKey)
{
	if(m_pNewUIMng)
	{
		return m_pNewUIMng->IsInterfaceVisible(dwKey);
	}

	return false;
}

#ifdef NEW_USER_INTERFACE_SHELL
bool SEASON3B::CNewUISystem::IsPartChargeShop( DWORD dwKey, bool isMessagebox )
{
	if( IsVisible( SEASON3B::INTERFACE_PARTCHARGE_SHOP ) ) {
		if( isMessagebox )
		{
			MESSAGEBOXDATA data;
			data.s_message.push_back( ( format( GlobalText[2561] ) ).str() );
			data.s_id      = 1;
			data.s_type    = eMB_OK;
			MessageBoxMake( data );
		}
		return true;
	}
	else if( dwKey == SEASON3B::INTERFACE_PARTCHARGE_SHOP ){
		if( IsVisible( SEASON3B::INTERFACE_MIXINVENTORY )
#ifdef LEM_ADD_LUCKYITEM
			IsVisible( SEASON3B::INTERFACE_LUCKYITEMWND )
#endif // LEM_ADD_LUCKYITEM
		  ) {
			SEASON3B::CreateOkMessageBox(GlobalText[2562], RGBA(255, 30, 0, 255) );
			return true;
		}
	}
	return false;
}
#endif //NEW_USER_INTERFACE_SHELL

//---------------------------------------------------------------------------------------------
// Show()
void SEASON3B::CNewUISystem::Show(DWORD dwKey)
{
#ifdef NEW_USER_INTERFACE_SHELL
	if( IsPartChargeShop( dwKey ) ) {
		return;
	}
#endif //NEW_USER_INTERFACE_SHELL
#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP
	// 인게임샵이 열렸으면 모든 창을 막는다
	if(g_pInGameShop->IsInGameShop())
		return;
#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP
	if(m_pNewUIMng) 
	{
		if(dwKey == SEASON3B::INTERFACE_FRIEND)
		{
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_FRIEND, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_FRIEND, true);
#endif //LDK_ADD_SCALEFORM
			m_pNewFriendWindow->OpenMainWnd(640-250, 432-173);
		}
		else if(dwKey == SEASON3B::INTERFACE_INVENTORY)
		{
#ifdef CSK_FIX_UI_FUNCTIONNAME
			HideGroupBeforeOpenInterface();
#else // CSK_FIX_UI_FUNCTIONNAME
			HideAllGroupC();
#endif // CSK_FIX_UI_FUNCTIONNAME
			
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif //LDK_ADD_SCALEFORM
			if(IsVisible(SEASON3B::INTERFACE_CHARACTER))
			{
				g_pMyInventory->SetPos(640-190*2, 0);
			}
			if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				Hide(SEASON3B::INTERFACE_MYQUEST);
			}
			g_pMyInventory->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHARACTER)
		{
#ifdef CSK_FIX_UI_FUNCTIONNAME
			HideGroupBeforeOpenInterface();
#else // CSK_FIX_UI_FUNCTIONNAME
			HideAllGroupC();
#endif // CSK_FIX_UI_FUNCTIONNAME
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);
#endif //LDK_ADD_SCALEFORM
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				g_pMyInventory->SetPos(640-190*2, 0);
			}
			else if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				g_pMyQuestInfoWindow->SetPos(640-190*2, 0);
			}
			g_pCharacterInfoWindow->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_PET )
		{
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				Hide(SEASON3B::INTERFACE_INVENTORY);
			}
			if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				Hide(SEASON3B::INTERFACE_MYQUEST);
			}
#ifdef CSK_FIX_UI_FUNCTIONNAME
			HideGroupBeforeOpenInterface();
#else // CSK_FIX_UI_FUNCTIONNAME
			HideAllGroupC();
#endif // CSK_FIX_UI_FUNCTIONNAME
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_CHARACTER);
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);
#endif //LDK_ADD_SCALEFORM
			m_pNewPetInfoWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MYQUEST)
		{
#ifdef CSK_FIX_UI_FUNCTIONNAME
			HideGroupBeforeOpenInterface();
#else // CSK_FIX_UI_FUNCTIONNAME
			HideAllGroupC();
#endif // CSK_FIX_UI_FUNCTIONNAME
			if(IsVisible(SEASON3B::INTERFACE_CHARACTER))
			{
				g_pMyQuestInfoWindow->SetPos(640-190*2, 0);
			}
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				Hide(SEASON3B::INTERFACE_INVENTORY);
			}
			if(IsVisible(SEASON3B::INTERFACE_PET))
			{
				Hide(SEASON3B::INTERFACE_PET);
			}
			g_pMyQuestInfoWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MIXINVENTORY)
		{
			HideAllGroupA();
			g_pMixInventory->OpeningProcess();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif //LDK_ADD_SCALEFORM
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCSHOP)
		{
			HideAllGroupA();
			// 인벤토리도 함께 열어야 한다.
#ifdef KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
			g_pNPCShop->OpenningProcess();
#endif // KJH_FIX_WOPS_K22181_ITEM_PURCHASED_OPENNING_SHOP
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif //LDK_ADD_SCALEFORM
		}
		else if(dwKey == SEASON3B::INTERFACE_STORAGE)
		{
			HideAllGroupA();
			// 인벤토리도 함께 열어야 한다.
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif //LDK_ADD_SCALEFORM
		}
		else if(dwKey == SEASON3B::INTERFACE_MYSHOP_INVENTORY)
		{			// 인벤토리도 함께 열어야 한다.
			HideAllGroupA();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif //LDK_ADD_SCALEFORM
		
			// 환영사원이면
			if(g_CursedTemple->IsCursedTemple() == true)
			{
				g_pMyShopInventory->OpenButtonLock();
			}
			else
			{
				g_pMyShopInventory->OpenButtonUnLock();
			}
		}
		else if(dwKey == SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)
		{
			HideAllGroupA();
		}
		else if( dwKey == SEASON3B::INTERFACE_PARTY)
		{
			HideAllGroupA();
			m_pNewPartyInfoWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCQUEST)
		{
			HideAllGroupA();
			g_pNPCQuest->ProcessOpening();
		}
		else if(dwKey == SEASON3B::INTERFACE_TRADE)
		{
			HideAllGroupA();
			// 인벤토리도 함께 열어야 한다.
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif //LDK_ADD_SCALEFORM
		}
		else if( dwKey == SEASON3B::INTERFACE_BLOODCASTLE )
		{
			HideAllGroupA();
			g_pEnterBloodCastle->OpenningProcess( );
		} 
		else if( dwKey == SEASON3B::INTERFACE_DEVILSQUARE )
		{
			HideAllGroupA();
			g_pEnterDevilSquare->OpenningProcess();
		} 
		else if(dwKey == SEASON3B::INTERFACE_CATAPULT)
		{
			HideAllGroupA();
			g_pCatapultWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_COMMAND)
		{
			HideAllGroupA();
			m_pNewCommandWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GUILDINFO)
		{
			HideAllGroupA();
			g_pGuildInfoWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_WINDOW_MENU)
		{
			g_pWindowMenu->OpenningProcess();
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, true);
#endif //LDK_ADD_SCALEFORM
		}
		else if(dwKey == SEASON3B::INTERFACE_SENATUS)
		{
			HideAllGroupA();
			g_pCastleWindow->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GUARDSMAN)
		{
			HideAllGroupA();
			g_pGuardWindow->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GATEKEEPER)
		{
			HideAllGroupA();
			g_pGatemanWindow->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GATESWITCH)
		{
			HideAllGroupA();
			g_pGateSwitchWindow->OpeningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCGUILDMASTER)
		{
			HideAllGroupA();
		}
		else if(dwKey == SEASON3B::INTERFACE_MASTER_LEVEL)
		{
			HideAllGroupA();
		}
		else if(dwKey == SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)
		{
			HideAllGroupB();
		}
		else if(dwKey == SEASON3B::INTERFACE_HERO_POSITION_INFO)
		{
			m_pNewHeroPositionInfo->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHAOSCASTLE_TIME)
		{
			if(IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
			{
				Hide(SEASON3B::INTERFACE_CHATINPUTBOX);
			}
			m_pNewChaosCastleTime->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_BLOODCASTLE_TIME )
		{
			g_pBloodCastle->OpenningProcess( );
		} 
		else if(dwKey == SEASON3B::INTERFACE_OPTION)
		{
			g_pOption->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_HELP)
		{
			Hide(SEASON3B::INTERFACE_MOVEMAP);
			Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
			Hide(SEASON3B::INTERFACE_SETITEM_EXPLANATION);
			g_pHelp->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_ITEM_EXPLANATION)
		{
			Hide(SEASON3B::INTERFACE_MOVEMAP);
			Hide(SEASON3B::INTERFACE_HELP);
			Hide(SEASON3B::INTERFACE_SETITEM_EXPLANATION);
			g_pItemExplanation->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_SETITEM_EXPLANATION)
		{
			Hide(SEASON3B::INTERFACE_MOVEMAP);
			Hide(SEASON3B::INTERFACE_HELP);
			Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
			g_pSetItemExplanation->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_QUICK_COMMAND)
		{
			g_pQuickCommand->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MOVEMAP)
		{
			Hide(SEASON3B::INTERFACE_HELP);
			Hide(SEASON3B::INTERFACE_ITEM_EXPLANATION);
			Hide(SEASON3B::INTERFACE_SETITEM_EXPLANATION);
			m_pNewMoveCommandWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHATINPUTBOX)
		{
			m_pNewChatInputBox->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_SIEGEWARFARE)
		{
			m_pNewSiegeWarfare->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_ITEM_ENDURANCE_INFO )
		{
			m_pNewItemEnduranceInfo->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_BUFF_WINDOW)
		{
			m_pNewBuffWindow->OpenningProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CRYWOLF)
		{
			m_pNewCryWolfInterface->OpenningProcess();
		}
#ifdef NEW_USER_INTERFACE_SHELL
		else if(dwKey == SEASON3B::INTERFACE_PARTCHARGE_SHOP)
		{
			m_pNewPartChargeShop->OpeningProcess();
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif //LDK_ADD_SCALEFORM
		}
#endif //NEW_USER_INTERFACE_SHELL
#ifdef PSW_GOLDBOWMAN
		else if( dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN )
		{
			m_pNewGoldBowman->OpeningProcess();
		}
#endif //PSW_GOLDBOWMAN

#ifdef PSW_EVENT_LENA
		else if( dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN_LENA )
		{
			m_pNewGoldBowmanLena->OpeningProcess();
		}
#endif //PSW_EVENT_LENA	

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		else if(dwKey == SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION)
		{
			HideAllGroupA();
			g_pLuckyCoinRegistration->OpeningProcess();
			//인벤토리와 같이 연다.
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
#endif //LDK_ADD_SCALEFORM
		}
		else if(dwKey == SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN)
		{
			HideAllGroupA();
			g_pExchangeLuckyCoinWindow->OpenningProcess();
		}
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_UI
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH )
		{
			m_pNewDuelWatchWindow->OpeningProcess();
		}
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH_MAINFRAME )
		{
			m_pNewDuelWatchMainFrameWindow->OpeningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH_USERLIST )
		{
			m_pNewDuelWatchUserListWindow->OpeningProcess();
		}
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef YDG_ADD_DOPPELGANGER_UI
		else if( dwKey == SEASON3B::INTERFACE_DOPPELGANGER_NPC )
		{
			m_pNewDoppelGangerWindow->OpeningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DOPPELGANGER_FRAME )
		{
			m_pNewDoppelGangerFrame->OpenningProcess();
		}
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
		else if(dwKey == SEASON3B::INTERFACE_NPC_DIALOGUE)
		{
			HideAllGroupA();
			g_pNPCDialogue->ProcessOpening();
		}
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		else if(dwKey == SEASON3B::INTERFACE_QUEST_PROGRESS)
		{
			HideAllGroupA();
			g_pQuestProgress->ProcessOpening();
		}
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
		else if(dwKey == SEASON3B::INTERFACE_NPC_MENU)
		{
			HideAllGroupA();
			g_pNPCMenu->ProcessOpening();
		}
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		else if(dwKey == SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
		{
			if (IsVisible(SEASON3B::INTERFACE_INVENTORY))
				Hide(SEASON3B::INTERFACE_INVENTORY);
			if (IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				Hide(SEASON3B::INTERFACE_MYQUEST);
				g_pQuestProgressByEtc->SetPos(640-190, 0);
			}
			if (IsVisible(SEASON3B::INTERFACE_CHARACTER))
				g_pQuestProgressByEtc->SetPos(640-190*2, 0);
			g_pQuestProgressByEtc->ProcessOpening();
		}
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		else if(dwKey == SEASON3B::INTERFACE_INGAMESHOP)
		{
#ifdef FOR_WORK
			DebugAngel_Write("InGameShopStatue.Txt", "CallStack - CNewUISystem.Show()\r\n");
#endif // FOR_WORK
			HideAll();
			g_pInGameShop->OpeningProcess();
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
#ifndef KJH_MOD_SHOP_SCRIPT_DOWNLOAD				// #ifndef - 정리할 때 지워야 하는 소스
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
			}
#else //LDK_ADD_SCALEFORM
#ifndef KJH_MOD_SHOP_SCRIPT_DOWNLOAD				// #ifndef - 정리할 때 지워야 하는 소스
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
#endif //LDK_ADD_SCALEFORM
		}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
		else if( dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC )
		{
			m_pNewEmpireGuardianNPC->OpenningProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER)
		{
			m_pNewEmpireGuardianTimer->OpenningProcess();
		}
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PJH_ADD_MINIMAP
		else if( dwKey == SEASON3B::INTERFACE_MINI_MAP)
		{
			m_pNewMiniMap->OpenningProcess();
		}
#endif //PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		else if(dwKey == SEASON3B::INTERFACE_GENSRANKING)
		{
			HideAllGroupA();
			g_pNewUIGensRanking->OpenningProcess();
			g_pNewUIGensRanking->SetPos(640-190, 0);
		}
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
		else if( dwKey == SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA )
		{
			m_pNewUnitedMarketPlaceWindow->OpeningProcess();
		}
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
#ifdef LEM_ADD_LUCKYITEM
		else if( dwKey == SEASON3B::INTERFACE_LUCKYITEMWND )
		{
			HideAllGroupA();
			g_pLuckyItemWnd->OpeningProcess();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
		}
#endif // LEM_ADD_LUCKYITEM

		m_pNewUIMng->ShowInterface(dwKey);
		
		int iScreenWidth = GetScreenWidth();
		// 아이템 내구도 위치 수정
		m_pNewItemEnduranceInfo->SetPos( iScreenWidth );
		// 버프 윈도우 위치 수정
		m_pNewBuffWindow->SetPos(iScreenWidth);
		m_pNewPartyListWindow->SetPos( iScreenWidth );

#ifdef PBG_MOD_STAMINA_UI
		if(BLUE_MU::IsBlueMuServer())
		{
			g_pNewUIStamina->SetPos(640-190*m_pNewUIMng->GetShowUICnt(), 480);
			g_pNewUIStamina->OpeningProcess();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_STAMINA_GAUGE);
		}
#endif //PBG_MOD_STAMINA_UI
	}

	
}

//---------------------------------------------------------------------------------------------
// Hide()
void SEASON3B::CNewUISystem::Hide(DWORD dwKey)
{
	if(m_pNewUIMng) 
	{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		if(SEASON3B::INTERFACE_MAINFRAME == dwKey)
			int aaa = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		/* 예외 */
		if(dwKey == SEASON3B::INTERFACE_FRIEND)
		{
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_FRIEND, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_FRIEND, false);
#endif //LDK_ADD_SCALEFORM
			m_pNewFriendWindow->HideAllWindow(TRUE, TRUE);
		}
		else if(dwKey == SEASON3B::INTERFACE_CHARACTER)
		{
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, false);
#endif //LDK_ADD_SCALEFORM
			if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
			{
				g_pMyQuestInfoWindow->SetPos(640-190, 0);
			}
			if(IsVisible((SEASON3B::INTERFACE_INVENTORY)))
			{
				g_pMyInventory->SetPos(640-190, 0);
			}
			if(IsVisible((SEASON3B::INTERFACE_PET)))
			{
				Hide(SEASON3B::INTERFACE_PET);
			}
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
			if(IsVisible((SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)))
			{
				g_pQuestProgressByEtc->SetPos(640-190, 0);
			}
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
		}
		else if(dwKey == SEASON3B::INTERFACE_INVENTORY)
		{
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
#endif //LDK_ADD_SCALEFORM
			if(IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
			{
				if(g_pMixInventory->ClosingProcess() == false)
					return;
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_MIXINVENTORY, false);
			}
#ifdef LEM_ADD_LUCKYITEM
			if(IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND))
			{
 				if(g_pLuckyItemWnd->ClosingProcess() == false)
 					return;
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_LUCKYITEMWND, false);
			}
#endif // LEM_ADD_LUCKYITEM
			if(IsVisible(SEASON3B::INTERFACE_NPCSHOP))
			{
				g_pNPCShop->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_NPCSHOP, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY))
			{
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_MYSHOP_INVENTORY, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY))
			{
#ifdef LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
				g_pPurchaseShopInventory->ClosingProcess();
#endif // LDS_FIX_MEMORYLEAK_WHERE_NEWUI_DEINITIALIZE
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_STORAGE))
			{
				if (!g_pStorageInventory->ProcessClosing())
					return;
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_STORAGE, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_TRADE))
			{
				g_pTrade->ProcessCloseBtn();
#ifdef PBG_FIX_TRADECLOSE
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_TRADE, false);
#endif //PBG_FIX_TRADECLOSE
			}
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
			if(IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION))
			{
				m_pNewLuckyCoinRegistration->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION, false);
			}
			if(IsVisible(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN))
			{	
				m_pNewExchangeLuckyCoinWindow->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN, false);
			}
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008

#ifdef LEM_ADD_LUCKYITEM
			if(IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND))
			{	
				m_pNewUILuckyItemWnd->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_LUCKYITEMWND, false);
			}
#endif // LEM_ADD_LUCKYITEM

			g_pMyInventory->SetPos(640-190, 0);
			g_pMyInventory->ClosingProcess();
		}
		else if (dwKey == SEASON3B::INTERFACE_MIXINVENTORY)
		{
			if (g_pMixInventory->ClosingProcess() == false)
			{
				return;
			}
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
#endif //LDK_ADD_SCALEFORM
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCSHOP)
		{
			g_pNPCShop->ClosingProcess();
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
#endif //LDK_ADD_SCALEFORM
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
		}
		else if(dwKey == SEASON3B::INTERFACE_MYSHOP_INVENTORY 
			 || dwKey == SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY )
		{
			if(dwKey == SEASON3B::INTERFACE_MYSHOP_INVENTORY)
			{
				g_pMyShopInventory->ClosingProcess();
			}
			else if(dwKey == SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY)
			{
				g_pPurchaseShopInventory->ClosingProcess();
			}
			g_pMyInventory->SetPos(640-190, 0);
		}
		else if (dwKey == SEASON3B::INTERFACE_STORAGE)
		{
 			if (!g_pStorageInventory->ProcessClosing())
				return;
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
#endif //LDK_ADD_SCALEFORM
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
		}
		else if( dwKey == SEASON3B::INTERFACE_PET )
		{
			m_pNewPetInfoWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_PARTY )
		{
			m_pNewPartyInfoWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MYQUEST)
		{
			m_pNewMyQuestInfoWindow->ClosingProcess();

			m_pNewMyQuestInfoWindow->SetPos(640-190, 0);
		}
		else if( dwKey == SEASON3B::INTERFACE_SENATUS )
		{
			m_pNewCastleWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GUARDSMAN )
		{
			m_pNewGuardWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GATEKEEPER )
		{
			m_pNewGatemanWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_GATESWITCH )
		{
			m_pNewGateSwitchWindow->ClosingProcess();
		}
		else if (dwKey == SEASON3B::INTERFACE_NPCQUEST)
		{
			m_pNewNPCQuest->ProcessClosing();
		}
		else if( dwKey == SEASON3B::INTERFACE_BLOODCASTLE )
		{
			g_pEnterBloodCastle->ClosingProcess( );
		} 
		else if( dwKey == SEASON3B::INTERFACE_DEVILSQUARE )
		{
			g_pEnterDevilSquare->ClosingProcess( );
		} 
		else if( dwKey == SEASON3B::INTERFACE_BLOODCASTLE_TIME )
		{
			g_pBloodCastle->ClosingProcess( );
		} 
		else if (dwKey == SEASON3B::INTERFACE_TRADE)
		{
 			g_pTrade->ProcessClosing();
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
#endif //LDK_ADD_SCALEFORM
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
		}
		else if(dwKey == SEASON3B::INTERFACE_CATAPULT)
		{
			g_pCatapultWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHAOSCASTLE_TIME)
		{
			m_pNewChaosCastleTime->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_COMMAND )
		{
			m_pNewCommandWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_WINDOW_MENU)
		{
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, false);
#endif //LDK_ADD_SCALEFORM
		}
		else if(dwKey == SEASON3B::INTERFACE_OPTION)
		{

		}
		else if( dwKey == SEASON3B::INTERFACE_HERO_POSITION_INFO )
		{
			m_pNewHeroPositionInfo->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_HELP)
		{
			g_pHelp->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_ITEM_EXPLANATION)
		{
			g_pItemExplanation->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_SETITEM_EXPLANATION)
		{
			g_pSetItemExplanation->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_QUICK_COMMAND)
		{
			g_pQuickCommand->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_MOVEMAP)
		{
			m_pNewCommandWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CHATINPUTBOX)
		{
			m_pNewChatInputBox->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_GUILDINFO)
		{
			m_pNewGuildInfoWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_NPCGUILDMASTER)
		{
			m_pNewGuildMakeWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_SIEGEWARFARE)
		{
			m_pNewSiegeWarfare->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_ITEM_ENDURANCE_INFO )
		{
			m_pNewItemEnduranceInfo->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_BUFF_WINDOW)
		{
			m_pNewBuffWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT)
		{
			m_pNewCursedTempleResultWindow->ClosingProcess();
		}
		else if(dwKey == SEASON3B::INTERFACE_CRYWOLF)
		{
			m_pNewCryWolfInterface->ClosingProcess();
		}
#ifdef NEW_USER_INTERFACE_SHELL
		else if(dwKey == SEASON3B::INTERFACE_PARTCHARGE_SHOP)
		{
			m_pNewPartChargeShop->ClosingProcess();
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, false);
#endif //LDK_ADD_SCALEFORM
			g_pMyInventory->SetPos(640-190, 0);
		}
#endif //NEW_USER_INTERFACE_SHELL
#ifdef PSW_GOLDBOWMAN
		else if( dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN )
		{
			m_pNewGoldBowman->ClosingProcess();
		}
#endif //PSW_GOLDBOWMAN

#ifdef PSW_EVENT_LENA
		else if( dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN_LENA )
		{
			m_pNewGoldBowmanLena->ClosingProcess();
		}
#endif //PSW_EVENT_LENA	

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		else if(dwKey == SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION)
		{
			m_pNewLuckyCoinRegistration->ClosingProcess();
			
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				m_pNewMyInventory->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
			}
		}
		else if(dwKey == SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN)
		{
			m_pNewExchangeLuckyCoinWindow->ClosingProcess();
			
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				m_pNewMyInventory->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
			}
		}
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_UI
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH )
		{
			m_pNewDuelWatchWindow->ClosingProcess();
		}
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH_MAINFRAME )
		{
			m_pNewDuelWatchMainFrameWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DUELWATCH_USERLIST )
		{
			m_pNewDuelWatchUserListWindow->ClosingProcess();
		}
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		else if(dwKey == SEASON3B::INTERFACE_INGAMESHOP)
		{
			g_pInGameShop->ClosingProcess();
#ifdef LDK_ADD_SCALEFORM
			if(GFxProcess::GetInstancePtr()->GetUISelect() == 0)
			{
				g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, false);
			}
#else //LDK_ADD_SCALEFORM
			g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, false);
#endif //LDK_ADD_SCALEFORM
		}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef YDG_ADD_DOPPELGANGER_UI
		else if( dwKey == SEASON3B::INTERFACE_DOPPELGANGER_NPC )
		{
			m_pNewDoppelGangerWindow->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_DOPPELGANGER_FRAME )
		{
			m_pNewDoppelGangerFrame->ClosingProcess();
		}
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
		else if (dwKey == SEASON3B::INTERFACE_NPC_DIALOGUE)
		{
			m_pNewNPCDialogue->ProcessClosing();
		}
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		else if (dwKey == SEASON3B::INTERFACE_QUEST_PROGRESS)
		{
			m_pNewQuestProgress->ProcessClosing();
		}
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
		else if (dwKey == SEASON3B::INTERFACE_NPC_MENU)
		{
			m_pNewNPCMenu->ProcessClosing();
		}
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		else if (dwKey == SEASON3B::INTERFACE_QUEST_PROGRESS_ETC)
		{
			m_pNewQuestProgressByEtc->ProcessClosing();
		}
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
		else if( dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC )
		{
			m_pNewEmpireGuardianNPC->ClosingProcess();
		}
		else if( dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER)
		{
			m_pNewEmpireGuardianTimer->ClosingProcess();
		}
#endif //LDK_ADD_EMPIREGUARDIAN_UI

#ifdef PJH_ADD_MINIMAP
		else if( dwKey == SEASON3B::INTERFACE_MINI_MAP)
		{
			m_pNewMiniMap->ClosingProcess();
		}
#endif //PJH_ADD_MINIMAP
#ifdef PBG_ADD_GENSRANKING
		else if(dwKey == SEASON3B::INTERFACE_GENSRANKING)
		{
			g_pNewUIGensRanking->ClosingProcess();
		}
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
		else if( dwKey == SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA )
		{
			m_pNewUnitedMarketPlaceWindow->ClosingProcess();
		}
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
#ifdef LEM_ADD_LUCKYITEM
		else if( dwKey == SEASON3B::INTERFACE_LUCKYITEMWND )
		{
			if( g_pLuckyItemWnd->ClosingProcess() == false )
				return;
			if(IsVisible(SEASON3B::INTERFACE_INVENTORY))
			{
				m_pNewMyInventory->ClosingProcess();
				m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
			}
		}
#endif // LEM_ADD_LUCKYITEM

		m_pNewUIMng->ShowInterface(dwKey, false);

		int iScreenWidth = GetScreenWidth();
		// 아이템 내구도 위치 수정
		m_pNewItemEnduranceInfo->SetPos( iScreenWidth );
		// 버프 윈도우 위치 수정
		m_pNewBuffWindow->SetPos(iScreenWidth);
		m_pNewPartyListWindow->SetPos( iScreenWidth );
#ifdef PBG_MOD_STAMINA_UI
		if(BLUE_MU::IsBlueMuServer())
		{
			g_pNewUIStamina->SetPos(640-190*m_pNewUIMng->GetShowUICnt(), 480);
			g_pNewUIStamina->ClosingProcess();
			m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_STAMINA_GAUGE);
		}
#endif //PBG_MOD_STAMINA_UI
	}
}

//---------------------------------------------------------------------------------------------
// Toggle()
void SEASON3B::CNewUISystem::Toggle(DWORD dwKey) 
{ 
	IsVisible(dwKey) ? Hide(dwKey) : Show(dwKey); 
}

//---------------------------------------------------------------------------------------------
// HideAll()
void SEASON3B::CNewUISystem::HideAll()
{
	if(m_pNewUIMng)
	{
		for(int i=SEASON3B::INTERFACE_BEGIN+1; i<INTERFACE_END; i++)
		{
			if(IsImpossibleHideInterface(i) == false)
			{
				if(IsVisible(i) == true)
				{
					Hide(i);
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------------------
// HideAllGroupA()
void SEASON3B::CNewUISystem::HideAllGroupA()
{
	// 이 함수는 친구창을 제외한 화면을 줄이면서 열리는 모든 창들을 닫을 때 사용하는 함수입니다.
	Hide(SEASON3B::INTERFACE_INVENTORY);
	Hide(SEASON3B::INTERFACE_CHARACTER);

	//. 등록후 사용하세요
	DWORD dwGroupA[] = 
	{ 
		//SEASON3B::INTERFACE_INVENTORY,
		//SEASON3B::INTERFACE_CHARACTER,			
		//SEASON3B::INTERFACE_WINDOW_MENU,

		SEASON3B::INTERFACE_MIXINVENTORY,
		SEASON3B::INTERFACE_STORAGE,
		SEASON3B::INTERFACE_NPCSHOP,
		SEASON3B::INTERFACE_MYSHOP_INVENTORY,
		SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY,
		SEASON3B::INTERFACE_PET,
		SEASON3B::INTERFACE_MYQUEST,
		SEASON3B::INTERFACE_NPCQUEST,
		SEASON3B::INTERFACE_PARTY,
		SEASON3B::INTERFACE_SENATUS,
		SEASON3B::INTERFACE_GUARDSMAN,
		SEASON3B::INTERFACE_COMMAND,
		SEASON3B::INTERFACE_GUILDINFO,
		SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC,
#ifdef YDG_ADD_NEW_DUEL_UI
		SEASON3B::INTERFACE_DUELWATCH,
#endif	// YDG_ADD_NEW_DUEL_UI
#ifdef YDG_ADD_DOPPELGANGER_UI
		SEASON3B::INTERFACE_DOPPELGANGER_NPC,
#endif	// YDG_ADD_DOPPELGANGER_UI
		//SEASON3B::INTERFACE_HELP,
		//SEASON3B::INTERFACE_ITEM_EXPLANATION,
		//SEASON3B::INTERFACE_SETITEM_EXPLANATION,
#ifdef KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
#ifdef PSW_GOLDBOWMAN
		SEASON3B::INTERFACE_GOLD_BOWMAN,
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
		SEASON3B::INTERFACE_GOLD_BOWMAN_LENA,
#endif //PSW_EVENT_LENA	
#endif // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
		SEASON3B::INTERFACE_NPC_DIALOGUE,
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		SEASON3B::INTERFACE_QUEST_PROGRESS,
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
		SEASON3B::INTERFACE_NPC_MENU,
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		SEASON3B::INTERFACE_QUEST_PROGRESS_ETC,
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
		SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC,
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PBG_MOD_STAMINA_UI
		SEASON3B::INTERFACE_STAMINA_GAUGE,
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
		SEASON3B::INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
		SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA,
#endif // LDS_ADD_UI_UNITEDMARKETPLACE

#ifdef LEM_ADD_LUCKYITEM
		SEASON3B::INTERFACE_LUCKYITEMWND,
#endif // LEM_ADD_LUCKYITEM

		0,
	};

	if(m_pNewUIMng)
	{
		for(int i=0; dwGroupA[i] != 0; i++)
		{
			m_pNewUIMng->ShowInterface(dwGroupA[i], false);
		}
	}
}

//---------------------------------------------------------------------------------------------
// HideAllGroupB()
void SEASON3B::CNewUISystem::HideAllGroupB()
{
	// 이 함수는 친구창을 포함한 화면을 줄이면서 열리는 모든 창들을 닫을 때 사용하는 함수입니다.
	Hide(SEASON3B::INTERFACE_FRIEND);
	Hide(SEASON3B::INTERFACE_INVENTORY);
	Hide(SEASON3B::INTERFACE_CHARACTER);

	//. 등록후 사용하세요
	DWORD dwGroupB[] = 
	{ 
		//SEASON3B::INTERFACE_FRIEND,
		//SEASON3B::INTERFACE_INVENTORY,
		//SEASON3B::INTERFACE_CHARACTER,
		//SEASON3B::INTERFACE_WINDOW_MENU,

		SEASON3B::INTERFACE_MIXINVENTORY,
		SEASON3B::INTERFACE_STORAGE,
		SEASON3B::INTERFACE_NPCSHOP,
		SEASON3B::INTERFACE_MYSHOP_INVENTORY,
		SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY,
		SEASON3B::INTERFACE_PET,
		SEASON3B::INTERFACE_MYQUEST,
		SEASON3B::INTERFACE_NPCQUEST,
		SEASON3B::INTERFACE_PARTY,
		SEASON3B::INTERFACE_SENATUS,
		SEASON3B::INTERFACE_GUARDSMAN,
		SEASON3B::INTERFACE_COMMAND,
		SEASON3B::INTERFACE_GUILDINFO,
		SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC,
		SEASON3B::INTERFACE_CURSEDTEMPLE_NPC,
#ifdef YDG_ADD_NEW_DUEL_UI
		SEASON3B::INTERFACE_DUELWATCH,
#endif	// YDG_ADD_NEW_DUEL_UI
#ifdef YDG_ADD_DOPPELGANGER_UI
		SEASON3B::INTERFACE_DOPPELGANGER_NPC,
#endif	// YDG_ADD_DOPPELGANGER_UI
		//SEASON3B::INTERFACE_HELP,
		//SEASON3B::INTERFACE_ITEM_EXPLANATION,
		//SEASON3B::INTERFACE_SETITEM_EXPLANATION,	
#ifdef KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
#ifdef PSW_GOLDBOWMAN
		SEASON3B::INTERFACE_GOLD_BOWMAN,
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
		SEASON3B::INTERFACE_GOLD_BOWMAN_LENA,
#endif //PSW_EVENT_LENA	
#endif // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
		SEASON3B::INTERFACE_NPC_DIALOGUE,
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		SEASON3B::INTERFACE_QUEST_PROGRESS,
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
		SEASON3B::INTERFACE_NPC_MENU,
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		SEASON3B::INTERFACE_QUEST_PROGRESS_ETC,
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
		SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC,
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PBG_MOD_STAMINA_UI
		SEASON3B::INTERFACE_STAMINA_GAUGE,
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
		SEASON3B::INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
		SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA,
#endif // LDS_ADD_UI_UNITEDMARKETPLACE

#ifdef LEM_ADD_LUCKYITEM
		SEASON3B::INTERFACE_LUCKYITEMWND,
#endif // LEM_ADD_LUCKYITEM

		0, 
	};
	
	if(m_pNewUIMng)
	{
		for(int i=0; dwGroupB[i] != 0; i++)
		{
			m_pNewUIMng->ShowInterface(dwGroupB[i], false);
		}
	}
}

#ifdef CSK_FIX_UI_FUNCTIONNAME
void SEASON3B::CNewUISystem::HideGroupBeforeOpenInterface()
#else // CSK_FIX_UI_FUNCTIONNAME
void SEASON3B::CNewUISystem::HideAllGroupC()
#endif // CSK_FIX_UI_FUNCTIONNAME
{
	// 
	DWORD dwGroupC[] = 
	{ 
		SEASON3B::INTERFACE_PARTY,
		SEASON3B::INTERFACE_COMMAND,
		SEASON3B::INTERFACE_GUILDINFO,
		//SEASON3B::INTERFACE_WINDOW_MENU,
#ifdef KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
#ifdef PSW_GOLDBOWMAN
		SEASON3B::INTERFACE_GOLD_BOWMAN,
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
		SEASON3B::INTERFACE_GOLD_BOWMAN_LENA,
#endif //PSW_EVENT_LENA	
#endif // KJH_FIX_JP0457_OPENNING_PARTCHARGE_UI
#ifdef PBG_ADD_GENSRANKING
		SEASON3B::INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING

		0, 
	};
	
	if(m_pNewUIMng)
	{
		for(int i=0; dwGroupC[i] != 0; i++)
		{
			m_pNewUIMng->ShowInterface(dwGroupC[i], false);
		}
	}
}

//---------------------------------------------------------------------------------------------
// Enable()
void SEASON3B::CNewUISystem::Enable(DWORD dwKey)
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->EnableInterface(dwKey);
	}
}

//---------------------------------------------------------------------------------------------
// Disable()
void SEASON3B::CNewUISystem::Disable(DWORD dwKey)
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->EnableInterface(dwKey, false);
	}
}

//---------------------------------------------------------------------------------------------
// CheckMouseUse()
bool SEASON3B::CNewUISystem::CheckMouseUse()
{
	if(m_pNewUIMng)
	{
		if(m_pNewUIMng->GetActiveMouseUIObj())
			return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------------
// CheckKeyUse() 
bool SEASON3B::CNewUISystem::CheckKeyUse()
{
	if(m_pNewUIMng)
	{
		if(m_pNewUIMng->GetActiveKeyUIObj())
			return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------------
// Update()
bool SEASON3B::CNewUISystem::Update()
{
	if(m_pNewItemMng)
	{
		m_pNewItemMng->Update();
	}

	if(m_pNewUIMng)
	{
		m_pNewUIMng->UpdateMouseEvent();
		m_pNewUIMng->UpdateKeyEvent();
		return m_pNewUIMng->Update();
	}
	return false;
}

//---------------------------------------------------------------------------------------------
// Render()
bool SEASON3B::CNewUISystem::Render()
{
	bool bResult = false;

	/******************** 2D 그리기 루틴 ********************/
	BeginBitmap();
	if(m_pNewUIMng)
		bResult = m_pNewUIMng->Render();	/* 안에서 3D도 그린다. CNewUI3DCamera 참고 */
	EndBitmap();
	
	return bResult;
}

//---------------------------------------------------------------------------------------------
// GetNewUIManager()
CNewUIManager* SEASON3B::CNewUISystem::GetNewUIManager() const
{ 
	return m_pNewUIMng; 
}

//---------------------------------------------------------------------------------------------
// GetNewUI3DRendererMng()
CNewUI3DRenderMng* SEASON3B::CNewUISystem::GetNewUI3DRenderMng() const
{ 
	return m_pNewUI3DRenderMng; 
}


CNewUIHotKey* SEASON3B::CNewUISystem::GetNewUIHotKey() const
{
	return m_pNewUIHotKey;
}

//---------------------------------------------------------------------------------------------

bool SEASON3B::CNewUISystem::IsImpossibleSendMoveInterface()
{
	if(IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)				// 조합 관련 창
		|| IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)	// 칸투르 3차 입구창
#ifdef LEM_ADD_LUCKYITEM
		|| IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)				// 조합 관련 창
#endif // LEM_ADD_LUCKYITEM
		)
	{
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------

bool SEASON3B::CNewUISystem::IsImpossibleTradeInterface()
{
	if(IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
		|| IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)
		|| IsVisible(SEASON3B::INTERFACE_STORAGE)					// 창고
#ifdef NEW_USER_INTERFACE_UISYSTEM
		|| IsVisible(SEASON3B::INTERFACE_PARTCHARGE_SHOP)			// 부분 유료화 상점 창
#endif //NEW_USER_INTERFACE_UISYSTEM
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		|| IsVisible(SEASON3B::INTERFACE_INGAMESHOP)				// 인게임샵
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef LEM_ADD_LUCKYITEM
		|| IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
#endif // LEM_ADD_LUCKYITEM
	  )
	{
		return true;
	}

	return false;
}

bool SEASON3B::CNewUISystem::IsImpossibleDuelInterface()
{
	if(IsVisible(SEASON3B::INTERFACE_MIXINVENTORY)
		|| IsVisible(SEASON3B::INTERFACE_KANTURU2ND_ENTERNPC)
		|| IsVisible(SEASON3B::INTERFACE_STORAGE)					// 창고
#ifdef NEW_USER_INTERFACE_UISYSTEM
		|| IsVisible(SEASON3B::INTERFACE_PARTCHARGE_SHOP)			// 부분 유료화 상점 창
#endif //NEW_USER_INTERFACE_UISYSTEM
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		|| IsVisible(SEASON3B::INTERFACE_INGAMESHOP)				// 인게임샵
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef LEM_ADD_LUCKYITEM
		|| IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)				
#endif // LEM_ADD_LUCKYITEM
	  )
	{
		return true;
	}

	return false;
}

bool SEASON3B::CNewUISystem::IsImpossibleHideInterface(DWORD dwKey)
{
	if(dwKey == SEASON3B::INTERFACE_MAINFRAME
		|| dwKey == SEASON3B::INTERFACE_SKILL_LIST
		|| dwKey == SEASON3B::INTERFACE_SLIDEWINDOW
		|| dwKey == SEASON3B::INTERFACE_MESSAGEBOX
		|| dwKey == SEASON3B::INTERFACE_CHATLOGWINDOW
		|| dwKey == SEASON3B::INTERFACE_PARTY_INFO_WINDOW
		|| dwKey == SEASON3B::INTERFACE_KANTURU_INFO
		|| dwKey == SEASON3B::INTERFACE_BLOODCASTLE_TIME
		|| dwKey == SEASON3B::INTERFACE_CHAOSCASTLE_TIME
		|| dwKey == SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE
		|| dwKey == SEASON3B::INTERFACE_DUEL_WINDOW
		|| dwKey == SEASON3B::INTERFACE_CRYWOLF
		|| dwKey == SEASON3B::INTERFACE_HERO_POSITION_INFO
		|| dwKey == SEASON3B::INTERFACE_NAME_WINDOW
		|| dwKey == SEASON3B::INTERFACE_SIEGEWARFARE
		|| dwKey == SEASON3B::INTERFACE_ITEM_TOOLTIP
		|| dwKey == SEASON3B::INTERFACE_HOTKEY
		|| dwKey == SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM
		|| dwKey == SEASON3B::INTERFACE_ITEM_ENDURANCE_INFO
		|| dwKey == SEASON3B::INTERFACE_BUFF_WINDOW
		|| (dwKey >= SEASON3B::INTERFACE_3DRENDERING_CAMERA_BEGIN && dwKey <= SEASON3B::INTERFACE_3DRENDERING_CAMERA_END)
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
		|| dwKey == SEASON3B::INTERFACE_DUELWATCH_MAINFRAME
		|| dwKey == SEASON3B::INTERFACE_DUELWATCH_USERLIST
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef YDG_ADD_DOPPELGANGER_UI
		|| dwKey == SEASON3B::INTERFACE_DOPPELGANGER_FRAME
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef PSW_GOLDBOWMAN
		|| dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA		
		|| dwKey == SEASON3B::INTERFACE_GOLD_BOWMAN_LENA
#endif //PSW_EVENT_LENA
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
		|| dwKey == SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER
#endif //LDK_ADD_EMPIREGUARDIAN_UI

		)
	{
		return true;
	}
	
	return false;
}

//---------------------------------------------------------------------------------------------

void SEASON3B::CNewUISystem::UpdateSendMoveInterface()
{
	if(IsVisible(SEASON3B::INTERFACE_TRADE))
	{
		SendRequestTradeExit();
		Hide(SEASON3B::INTERFACE_TRADE);
	}
	if(IsVisible(SEASON3B::INTERFACE_STORAGE))				// 창고
	{
		Hide(SEASON3B::INTERFACE_STORAGE);
	}
	if(IsVisible(SEASON3B::INTERFACE_NPCGUILDMASTER))
	{
		Hide(SEASON3B::INTERFACE_NPCGUILDMASTER);
	}
	if(IsVisible(SEASON3B::INTERFACE_MYQUEST))
	{
		Hide(SEASON3B::INTERFACE_MYQUEST);
	}
	if (IsVisible(SEASON3B::INTERFACE_NPCQUEST))
	{
		Hide(SEASON3B::INTERFACE_NPCQUEST);
	}
	if(IsVisible(SEASON3B::INTERFACE_NPCSHOP))
	{
		Hide(SEASON3B::INTERFACE_NPCSHOP);
	}
	if(IsVisible(SEASON3B::INTERFACE_GUARDSMAN))
	{
		Hide(SEASON3B::INTERFACE_GUARDSMAN);
	}
	if(IsVisible(SEASON3B::INTERFACE_GUARDSMAN))
	{
		Hide(SEASON3B::INTERFACE_GUARDSMAN);
	}
	if(IsVisible(SEASON3B::INTERFACE_DEVILSQUARE))
	{
		Hide(SEASON3B::INTERFACE_DEVILSQUARE);
	}
	if(IsVisible(SEASON3B::INTERFACE_BLOODCASTLE))
	{
		Hide(SEASON3B::INTERFACE_BLOODCASTLE);
	}
	if(IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC))
	{
		Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC);
	}
	if(IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY))
	{
		Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
	}
	if(IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY))
	{
		Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
	}
#ifdef YDG_ADD_NEW_DUEL_UI
	if(IsVisible(SEASON3B::INTERFACE_DUELWATCH))
	{
		Hide(SEASON3B::INTERFACE_DUELWATCH);
	}
#endif	// YDG_ADD_NEW_DUEL_UI
#ifdef YDG_ADD_DOPPELGANGER_UI
	if(IsVisible(SEASON3B::INTERFACE_DOPPELGANGER_NPC))
	{
		Hide(SEASON3B::INTERFACE_DOPPELGANGER_NPC);
	}
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	if (IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
		Hide(SEASON3B::INTERFACE_NPC_DIALOGUE);
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
	if (IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
		Hide(SEASON3B::INTERFACE_QUEST_PROGRESS);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
	if (IsVisible(SEASON3B::INTERFACE_NPC_MENU))
		Hide(SEASON3B::INTERFACE_NPC_MENU);
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
	if (IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
		Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
	if(IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC))
	{
		Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
	}
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PBG_MOD_LUCKYCOINEVENT
	if(IsVisible(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION))
	{
		Hide(SEASON3B::INTERFACE_LUCKYCOIN_REGISTRATION);
	}
	if(IsVisible(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN))
	{
		Hide(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
	}
#endif //PBG_MOD_LUCKYCOINEVENT
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
	if(IsVisible(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA))
	{
		Hide(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
	}
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
}


// GetInstance()
CNewUISystem* SEASON3B::CNewUISystem::GetInstance()
{
	static CNewUISystem s_NewUISystem;
	return &s_NewUISystem;
}

#ifndef CSK_MOD_REMOVE_AUTO_V1_FLAG		// 정리할 때 지워야 하는 소스	
#ifdef CSK_MOD_PROTECT_AUTO_V1
void SEASON3B::CNewUISystem::ToggleMoveCommandWindow()
{
	if(m_pNewMoveCommandWindow->IsNewVersion() == false)
	{
		SAFE_DELETE( m_pNewMoveCommandWindow );
		m_pNewMoveCommandWindow = new CNewUIMoveCommandWindowNew;
		m_pNewMoveCommandWindow->SetNewVersion(true);
	}
	else
	{
		SAFE_DELETE( m_pNewMoveCommandWindow );
		m_pNewMoveCommandWindow = new CNewUIMoveCommandWindow;
		m_pNewMoveCommandWindow->SetNewVersion(false);
	}

	m_pNewMoveCommandWindow->Create(m_pNewUIMng, 1, 1);
}
#endif // CSK_MOD_PROTECT_AUTO_V1
#endif //! CSK_MOD_REMOVE_AUTO_V1_FLAG	// 정리할 때 지워야 하는 소스

//---------------------------------------------------------------------------------------------

CNewUIChatLogWindow* SEASON3B::CNewUISystem::GetUI_NewChatLogWindow() const
{ 
	return m_pNewChatLogWindow; 
}

CNewUISlideWindow* SEASON3B::CNewUISystem::GetUI_NewSlideWindow() const
{ 
	return m_pNewSlideWindow; 
}

CNewUIFriendWindow* SEASON3B::CNewUISystem::GetUI_NewFriendWindow() const
{ 
	return m_pNewFriendWindow; 
}

CNewUIMainFrameWindow* SEASON3B::CNewUISystem::GetUI_NewMainFrameWindow() const
{
	return m_pNewMainFrameWindow;
}

CNewUISkillList* SEASON3B::CNewUISystem::GetUI_NewSkillList() const
{
	return m_pNewSkillList;
}

CNewUIChatInputBox* SEASON3B::CNewUISystem::GetUI_NewChatInputBox() const
{
	return m_pNewChatInputBox;
}

CNewUIItemMng* SEASON3B::CNewUISystem::GetUI_NewItemMng() const
{
	return m_pNewItemMng;
}

CNewUIMyInventory* SEASON3B::CNewUISystem::GetUI_NewMyInventory() const
{ 
	return m_pNewMyInventory; 
}

CNewUINPCShop* SEASON3B::CNewUISystem::GetUI_NewNpcShop() const
{
	return m_pNewNPCShop;
}

CNewUIPetInfoWindow* SEASON3B::CNewUISystem::GetUI_NewPetInfoWindow() const
{
	return m_pNewPetInfoWindow;
}

CNewUIMixInventory* SEASON3B::CNewUISystem::GetUI_NewMixInventory() const
{ 
	return m_pNewMixInventory; 
}

CNewUICastleWindow* SEASON3B::CNewUISystem::GetUI_NewCastleWindow() const
{ 
	return m_pNewCastleWindow; 
}

CNewUIGuardWindow* SEASON3B::CNewUISystem::GetUI_NewGuardWindow() const
{ 
	return m_pNewGuardWindow; 
}

CNewUIGatemanWindow* SEASON3B::CNewUISystem::GetUI_NewGatemanWindow() const
{ 
	return m_pNewGatemanWindow; 
}

CNewUIGateSwitchWindow* SEASON3B::CNewUISystem::GetUI_NewGateSwitchWindow() const
{ 
	return m_pNewGateSwitchWindow; 
}

CNewUIStorageInventory* SEASON3B::CNewUISystem::GetUI_NewStorageInventory() const
{ 
	return m_pNewStorageInventory; 
}

CNewUIGuildMakeWindow* SEASON3B::CNewUISystem::GetUI_NewGuildMakeWindow() const
{ 
	return m_pNewGuildMakeWindow; 
}

CNewUIGuildInfoWindow* SEASON3B::CNewUISystem::GetUI_NewGuildInfoWindow() const
{ 
	return m_pNewGuildInfoWindow; 
}

CNewUICryWolf* SEASON3B::CNewUISystem::GetUI_NewCryWolfInterface() const
{
	return m_pNewCryWolfInterface;
}

CNewUIMasterLevel* SEASON3B::CNewUISystem::GetUI_NewMasterLevelInterface() const
{
	return m_pNewMaster_Level_Interface;
}


CNewUIMyShopInventory* SEASON3B::CNewUISystem::GetUI_NewMyShopInventory() const
{ 
	return m_pNewMyShopInventory;
}

CNewUIPurchaseShopInventory* SEASON3B::CNewUISystem::GetUI_NewPurchaseShopInventory() const
{ 
	return m_pNewPurchaseShopInventory;
}

CNewUICharacterInfoWindow* SEASON3B::CNewUISystem::GetUI_NewCharacterInfoWindow() const
{
	return m_pNewCharacterInfoWindow;
}

CNewUIMyQuestInfoWindow* SEASON3B::CNewUISystem::GetUI_NewMyQuestInfoWindow() const
{
	return m_pNewMyQuestInfoWindow;
}

CNewUIPartyInfoWindow* SEASON3B::CNewUISystem::GetUI_NewPartyInfoWindow() const
{
	return m_pNewPartyInfoWindow;
}

CNewUIPartyListWindow* SEASON3B::CNewUISystem::GetUI_NewPartyListWindow() const
{
	return m_pNewPartyListWindow;
}

CNewUINPCQuest* SEASON3B::CNewUISystem::GetUI_NewNPCQuest() const
{
	return m_pNewNPCQuest;
}

CNewUIEnterBloodCastle* SEASON3B::CNewUISystem::GetUI_NewEnterBloodCastle( ) const
{
	return m_pNewEnterBloodCastle;
}

CNewUIEnterDevilSquare* SEASON3B::CNewUISystem::GetUI_NewEnterDevilSquare( ) const
{
	return m_pNewEnterDevilSquare;
}

CNewUIBloodCastle* SEASON3B::CNewUISystem::GetUI_NewBloodCastle( ) const
{
	return m_pNewBloodCastle;
}

CNewUITrade* SEASON3B::CNewUISystem::GetUI_NewTrade() const
{
	return m_pNewTrade;
}

CNewUIKanturu2ndEnterNpc* SEASON3B::CNewUISystem::GetUI_NewKanturu2ndEnterNpc() const
{
	return m_pNewKanturu2ndEnterNpc;
}

CNewUIKanturuInfoWindow* SEASON3B::CNewUISystem::GetUI_NewKanturuInfoWindow() const
{
	return m_pNewKanturuInfoWindow;
}

CNewUICatapultWindow* SEASON3B::CNewUISystem::GetUI_NewCatapultWindow() const
{
	return m_pNewCatapultWindow;
}

CNewUIChaosCastleTime* SEASON3B::CNewUISystem::GetUI_NewChaosCastleTime() const
{
	return m_pNewChaosCastleTime;
}

CNewUICommandWindow* SEASON3B::CNewUISystem::GetUI_NewCommandWindow() const
{
	return m_pNewCommandWindow;
}

CNewUIWindowMenu* SEASON3B::CNewUISystem::GetUI_NewWindowMenu() const
{
	return m_pNewWindowMenu;
}

CNewUIOptionWindow* SEASON3B::CNewUISystem::GetUI_NewOptionWindow() const
{
	return m_pNewOptionWindow;
}

CNewUIHeroPositionInfo* SEASON3B::CNewUISystem::GetUI_NewHeroPositionInfo() const
{
	return m_pNewHeroPositionInfo;
}

CNewUIHelpWindow* SEASON3B::CNewUISystem::GetUI_NewHelpWindow() const
{
	return m_pNewHelpWindow;
}

CNewUIItemExplanationWindow* SEASON3B::CNewUISystem::GetUI_NewItemExplanationWindow() const
{
	return m_pNewItemExplanationWindow;
}

CNewUISetItemExplanation* SEASON3B::CNewUISystem::GetUI_NewSetItemExplanation() const
{
	return m_pNewSetItemExplanation;
}

CNewUIQuickCommandWindow* SEASON3B::CNewUISystem::GetUI_NewQuickCommandWindow() const
{
	return m_pNewQuickCommandWindow;
}

CNewUIMoveCommandWindow* SEASON3B::CNewUISystem::GetUI_NewMoveCommandWindow() const
{
	return m_pNewMoveCommandWindow;
}

CNewUIBattleSoccerScore* SEASON3B::CNewUISystem::GetUI_NewBattleSoccerScore() const
{
	return m_pNewBattleSoccerScore;
}

CNewUIDuelWindow* SEASON3B::CNewUISystem::GetUI_NewDuelWindow() const
{
	return m_pNewDuelWindow;
}

CNewUISiegeWarfare* SEASON3B::CNewUISystem::GetUI_NewSiegeWarfare() const
{
	return m_pNewSiegeWarfare;
}

CNewUIItemEnduranceInfo* SEASON3B::CNewUISystem::GetUI_NewItemEnduranceInfo() const
{
	return m_pNewItemEnduranceInfo;
}

CNewUIBuffWindow* SEASON3B::CNewUISystem::GetUI_NewBuffWindow() const
{
	return m_pNewBuffWindow;
}

CNewUICursedTempleEnter* SEASON3B::CNewUISystem::GetUI_NewCursedTempleEnterWindow() const
{
	return m_pNewCursedTempleEnterWindow;
}

CNewUICursedTempleSystem* SEASON3B::CNewUISystem::GetUI_NewCursedTempleWindow() const
{
	return m_pNewCursedTempleWindow;
}

CNewUICursedTempleResult* SEASON3B::CNewUISystem::GetUI_NewCursedTempleResultWindow() const
{
	return m_pNewCursedTempleResultWindow;
}

#ifdef NEW_USER_INTERFACE_SHELL
CNewUIPartChargeShop* SEASON3B::CNewUISystem::GetUI_pNewPartChargeShopInterface() const
{
	return m_pNewPartChargeShop;
}
#endif //NEW_USER_INTERFACE_SHEL
#ifdef PSW_GOLDBOWMAN
CNewUIGoldBowmanWindow*	SEASON3B::CNewUISystem::GetUI_pNewGoldBowman() const
{
	return m_pNewGoldBowman;
}
#endif //PSW_GOLDBOWMAN

#ifdef PSW_EVENT_LENA
CNewUIGoldBowmanLena*	SEASON3B::CNewUISystem::GetUI_pNewGoldBowmanLena() const
{
	return m_pNewGoldBowmanLena;
}
#endif //PSW_EVENT_LENA	

#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
CNewUIRegistrationLuckyCoin* SEASON3B::CNewUISystem::GetUI_pNewLuckyCoinRegistration() const
{
	return m_pNewLuckyCoinRegistration;
}

CNewUIExchangeLuckyCoin* SEASON3B::CNewUISystem::GetUI_pNewExchangeLuckyCoin() const
{
	return m_pNewExchangeLuckyCoinWindow;
}
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008

#ifdef PJH_ADD_MINIMAP
CNewUIMiniMap* SEASON3B::CNewUISystem::GetUI_pNewUIMiniMap() const
{
	return m_pNewMiniMap;
}
#endif //PJH_ADD_MINIMAP

#ifdef YDG_ADD_NEW_DUEL_UI
CNewUIDuelWatchWindow* SEASON3B::CNewUISystem::GetUI_pNewDuelWatch() const
{
	return m_pNewDuelWatchWindow;
}
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
CNewUIDuelWatchMainFrameWindow* SEASON3B::CNewUISystem::GetUI_pNewDuelWatchMainFrame() const
{
	return m_pNewDuelWatchMainFrameWindow;
}

CNewUIDuelWatchUserListWindow* SEASON3B::CNewUISystem::GetUI_pNewDuelWatchUserList() const
{
	return m_pNewDuelWatchUserListWindow;
}
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
CNewUIInGameShop* SEASON3B::CNewUISystem::GetUI_pNewInGameShop() const
{
	return m_pNewInGameShop;
}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

#ifdef YDG_ADD_DOPPELGANGER_UI
CNewUIDoppelGangerWindow* SEASON3B::CNewUISystem::GetUI_pNewDoppelGangerWindow() const
{
	return m_pNewDoppelGangerWindow;
}

CNewUIDoppelGangerFrame* SEASON3B::CNewUISystem::GetUI_pNewDoppelGangerFrame() const
{
	return m_pNewDoppelGangerFrame;
}
#endif	// YDG_ADD_DOPPELGANGER_UI

#ifdef ASG_ADD_UI_NPC_DIALOGUE
CNewUINPCDialogue* SEASON3B::CNewUISystem::GetUI_NewNPCDialogue() const
{
	return m_pNewNPCDialogue;
}
#endif	// ASG_ADD_UI_NPC_DIALOGUE

#ifdef ASG_ADD_UI_QUEST_PROGRESS
CNewUIQuestProgress* SEASON3B::CNewUISystem::GetUI_NewQuestProgress() const
{
	return m_pNewQuestProgress;
}
#endif	// ASG_ADD_UI_QUEST_PROGRESS

#ifdef ASG_ADD_UI_NPC_MENU
CNewUINPCMenu* SEASON3B::CNewUISystem::GetUI_NewNPCMenu() const
{
	return m_pNewNPCMenu;
}
#endif	// ASG_ADD_UI_NPC_MENU

#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
CNewUIQuestProgressByEtc* SEASON3B::CNewUISystem::GetUI_NewQuestProgressByEtc() const
{
	return m_pNewQuestProgressByEtc;
}
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC

#ifdef LDK_ADD_EMPIREGUARDIAN_UI
CNewUIEmpireGuardianNPC* SEASON3B::CNewUISystem::GetUI_pNewEmpireGuardianNPC() const
{
	return m_pNewEmpireGuardianNPC;
}

CNewUIEmpireGuardianTimer* SEASON3B::CNewUISystem::GetUI_pNewEmpireGuardianTimer() const
{
	return m_pNewEmpireGuardianTimer;
}
#endif //LDK_ADD_EMPIREGUARDIAN_UI

#ifdef PBG_MOD_STAMINA_UI
CNewUIStamina* SEASON3B::CNewUISystem::GetUI_pNewUIStamina() const
{
	return m_pNewUIStamina;
}
#endif //PBG_MOD_STAMINA_UI

#ifdef PBG_ADD_GENSRANKING
CNewUIGensRanking* SEASON3B::CNewUISystem::GetUI_NewGensRanking() const
{
	return m_pNewGensRanking;
}
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
CNewUIUnitedMarketPlaceWindow* SEASON3B::CNewUISystem::GetUI_pNewUnitedMarketPlaceWindow() const
{
	return m_pNewUnitedMarketPlaceWindow;
}
#endif // LDS_ADD_UI_UNITEDMARKETPLACE

#ifdef LEM_ADD_LUCKYITEM
CNewUILuckyItemWnd* SEASON3B::CNewUISystem::Get_pNewUILuckyItemWnd() const
{
	return m_pNewUILuckyItemWnd;
}
#endif // LEM_ADD_LUCKYITEM