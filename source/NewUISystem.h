

#ifndef _NEWUISYSTEM_H_
#define _NEWUISYSTEM_H_

#pragma once

#include "NewUIManager.h"
#include "NewUI3DRenderMng.h"
#include "NewUIHotKey.h"
#include "NewUIChatLogWindow.h"
#include "NewUISlideWindow.h"
#include "NewUIGuildMakeWindow.h"
#include "NewUIFriendWindow.h"
#include "NewUIMainFrameWindow.h"
#include "NewUIChatInputBox.h"
#include "NewUIItemMng.h"
#include "NewUIMyInventory.h"
#include "NewUINPCShop.h"
#include "NewUIPetInfoWindow.h"
#include "NewUIMixInventory.h"
#include "NewUICastleWindow.h"
#include "NewUIGuardWindow.h"
#include "NewUIGatemanWindow.h"
#include "NewUIGateSwitchWindow.h"
#include "NewUIStorageInventory.h"
#include "NewUIGuildInfoWindow.h"
#include "NewUIMyShopInventory.h"
#include "NewUIPurchaseShopInventory.h"
#include "NewUICharacterInfoWindow.h"
#include "NewUIMyQuestInfoWindow.h"
#include "NewUIPartyInfoWindow.h"
#include "NewUIPartyListWindow.h"
#include "NewUINPCQuest.h"
#include "NewUIEnterDevilSquare.h"
#include "NewUIBloodCastleEnter.h"
#include "NewUIBloodCastleTime.h"
#include "NewUITrade.h"
#include "NewUIKanturuEvent.h"
#include "NewUICatapultWindow.h"
#include "NewUIChaosCastleTime.h"
#include "NewUIBattleSoccerScore.h"
#include "NewUICommandWindow.h"
#include "NewUIWindowMenu.h"
#include "NewUIOptionWindow.h"
#include "NewUIHeroPositionInfo.h"
#include "NewUIHelpWindow.h"
#include "NewUIItemExplanationWindow.h"
#include "NewUISetItemExplanation.h"
#include "NewUIQuickCommandWindow.h"
#include "NewUIMoveCommandWindow.h"
#include "NewUIDuelWindow.h"
#include "NewUISeigeWarfare.h"
#include "NewUINameWindow.h"
#include "NewUIItemEnduranceInfo.h"
#include "NewUIBuffWindow.h"
#include "NewUICursedTempleEnter.h"
#include "NewUICursedTempleSystem.h"
#include "NewUICursedTempleResult.h"
#include "NewUICryWolf.h"
#include "NewUIMasterLevel.h"
#ifdef NEW_USER_INTERFACE_SHELL
#include "NewUIPartChargeShop.h"
#endif //NEW_USER_INTERFACE_SHELL
#ifdef PSW_GOLDBOWMAN
#include "NewUIGoldBowmanWindow.h"
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
#include "NewUIGoldBowmanLena.h"
#endif //PSW_EVENT_LENA	
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
#include "NewUIRegistrationLuckyCoin.h"
#include "NewUIExchangeLuckyCoin.h"
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_UI
#include "NewUIDuelWatchWindow.h"
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
#include "NewUIDuelWatchMainFrameWindow.h"
#include "NewUIDuelWatchUserListWindow.h"
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "NewUIInGameShop.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef YDG_ADD_DOPPELGANGER_UI
#include "NewUIDoppelGangerWindow.h"
#include "NewUIDoppelGangerFrame.h"
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_QUEST_PROGRESS
#include "NewUIQuestProgress.h"
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
#include "NewUINPCMenu.h"
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
#include "NewUIQuestProgressByEtc.h"
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
#include "NewUIEmpireGuardianNPC.h"
#include "NewUIEmpireGuardianTimer.h"
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
#include "NewUINPCDialogue.h"
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef PJH_ADD_MINIMAP
#include "NewUIMiniMap.h"
#endif //PJH_ADD_MINIMAP
#ifdef PBG_MOD_STAMINA_UI
#include "FatigueTimeSystem.h"
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
#include "NewUIGensRanking.h"
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
#include "NewUIUnitedMarketPlaceWindow.h"
//#include "NewUIUnitedMarketPlaceWindow.h"
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#include "MultiLanguage.h"
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef LEM_ADD_LUCKYITEM
#include "NewUILuckyItemWnd.h"
#endif // LEM_ADD_LUCKYITEM

namespace SEASON3B
{
	class CNewUISystem
	{
		CNewUIManager*		m_pNewUIMng;
		CNewUI3DRenderMng*	m_pNewUI3DRenderMng;
		CNewUIHotKey*		m_pNewUIHotKey;

	public:
		~CNewUISystem();

		bool Create();
		void Release();

		bool LoadMainSceneInterface();
		void UnloadMainSceneInterface();

		bool IsVisible(DWORD dwKey);
		void Show(DWORD dwKey);
		void Hide(DWORD dwKey);
		void Toggle(DWORD dwKey);	//. Show <-> Hide
		void HideAll();
		
#ifdef NEW_USER_INTERFACE_SHELL
		bool IsPartChargeShop( DWORD dwKey, bool isMessagebox = true );
#endif //NEW_USER_INTERFACE_SHELL
	
		void Enable(DWORD dwKey);
		void Disable(DWORD dwKey);

		bool CheckMouseUse();
		bool CheckKeyUse();

		bool Update();
		bool Render();

		CNewUIManager* GetNewUIManager() const;
		CNewUI3DRenderMng* GetNewUI3DRenderMng() const;
		CNewUIHotKey* GetNewUIHotKey() const;

		bool IsImpossibleSendMoveInterface();	// 창이 열렸을 때 캐릭터가 이동 불가능한 창인가?
		void UpdateSendMoveInterface();			// 창이 열렸을 때 캐릭터가 이동하면 닫히는 창들 업데이트
		bool IsImpossibleTradeInterface();		// 창이 열렸을 때 거래가 불가능한 창인가?
		bool IsImpossibleDuelInterface();		// 창이 열렸을 때 결투가 불가능한 창인가?
		bool IsImpossibleHideInterface(DWORD dwKey);		// 창이 절대 닫히면 안되는 창인가? (예:메인프레임)
			
		static CNewUISystem* GetInstance();

#ifndef CSK_MOD_REMOVE_AUTO_V1_FLAG		// 정리할 때 지워야 하는 소스	
#ifdef CSK_MOD_PROTECT_AUTO_V1
		void ToggleMoveCommandWindow();
#endif // CSK_MOD_PROTECT_AUTO_V1
#endif //! CSK_MOD_REMOVE_AUTO_V1_FLAG	// 정리할 때 지워야 하는 소스

	protected:
		CNewUISystem();		//. ban to create instance

		void HideAllGroupA();
		void HideAllGroupB();
#ifdef CSK_FIX_UI_FUNCTIONNAME
		// 어떤 창을 열 때 어떤 창이 열려있고 그 창을 닫아야 할 경우 사용하면 됩니다.
		void HideGroupBeforeOpenInterface();
#else // CSK_FIX_UI_FUNCTIONNAME
		void HideAllGroupC();
#endif // CSK_FIX_UI_FUNCTIONNAME

		/* Interface classes */
	private:
		CNewUIChatInputBox*				m_pNewChatInputBox;
		CNewUIChatLogWindow*			m_pNewChatLogWindow;
		CNewUISlideWindow*				m_pNewSlideWindow;
		CNewUIFriendWindow*				m_pNewFriendWindow;
		CNewUIMainFrameWindow*			m_pNewMainFrameWindow;
		CNewUISkillList*				m_pNewSkillList;
		CNewUIItemMng*					m_pNewItemMng;
		CNewUIMyInventory*				m_pNewMyInventory;
		CNewUINPCShop*					m_pNewNPCShop;
		CNewUIPetInfoWindow*			m_pNewPetInfoWindow;
		CNewUIMixInventory*				m_pNewMixInventory;
		CNewUICastleWindow*				m_pNewCastleWindow;
		CNewUIGuardWindow*				m_pNewGuardWindow;
		CNewUIGatemanWindow*			m_pNewGatemanWindow;
		CNewUIGateSwitchWindow*			m_pNewGateSwitchWindow;
		CNewUIStorageInventory*			m_pNewStorageInventory;
		CNewUIGuildMakeWindow*			m_pNewGuildMakeWindow;
		CNewUIGuildInfoWindow*			m_pNewGuildInfoWindow;
		CNewUIMyShopInventory*			m_pNewMyShopInventory;
		CNewUIPurchaseShopInventory*	m_pNewPurchaseShopInventory;
		CNewUICharacterInfoWindow*		m_pNewCharacterInfoWindow;
		CNewUIMyQuestInfoWindow*		m_pNewMyQuestInfoWindow;
		CNewUIPartyInfoWindow*			m_pNewPartyInfoWindow;
		CNewUIPartyListWindow*			m_pNewPartyListWindow;
		CNewUINPCQuest*					m_pNewNPCQuest;
		CNewUIEnterBloodCastle*			m_pNewEnterBloodCastle;
		CNewUIEnterDevilSquare*			m_pNewEnterDevilSquare;
		CNewUIBloodCastle*				m_pNewBloodCastle;
		CNewUITrade*					m_pNewTrade;
		CNewUIKanturu2ndEnterNpc*		m_pNewKanturu2ndEnterNpc;
		CNewUIKanturuInfoWindow*		m_pNewKanturuInfoWindow;
		CNewUICatapultWindow*			m_pNewCatapultWindow;
		CNewUIChaosCastleTime*			m_pNewChaosCastleTime;
		CNewUIBattleSoccerScore*		m_pNewBattleSoccerScore;
		CNewUICommandWindow*			m_pNewCommandWindow;
		CNewUIHeroPositionInfo*			m_pNewHeroPositionInfo;
		CNewUIWindowMenu*				m_pNewWindowMenu;
		CNewUIOptionWindow*				m_pNewOptionWindow;
		CNewUIHelpWindow*				m_pNewHelpWindow;
		CNewUIItemExplanationWindow*	m_pNewItemExplanationWindow;
		CNewUISetItemExplanation*		m_pNewSetItemExplanation;
		CNewUIQuickCommandWindow*		m_pNewQuickCommandWindow;
		CNewUIMoveCommandWindow*		m_pNewMoveCommandWindow;
		CNewUIDuelWindow*				m_pNewDuelWindow;
		CNewUINameWindow*				m_pNewNameWindow;
		CNewUISiegeWarfare*				m_pNewSiegeWarfare;
		CNewUIItemEnduranceInfo*		m_pNewItemEnduranceInfo;
		CNewUIBuffWindow*				m_pNewBuffWindow;
		CNewUICursedTempleEnter*		m_pNewCursedTempleEnterWindow;
		CNewUICursedTempleSystem*		m_pNewCursedTempleWindow;
		CNewUICursedTempleResult*		m_pNewCursedTempleResultWindow;
		CNewUICryWolf*					m_pNewCryWolfInterface;
		CNewUIMasterLevel*				m_pNewMaster_Level_Interface;
#ifdef NEW_USER_INTERFACE_SHELL
		CNewUIPartChargeShop*			m_pNewPartChargeShop;
#endif //NEW_USER_INTERFACE_SHELL
#ifdef PSW_GOLDBOWMAN
		CNewUIGoldBowmanWindow*			m_pNewGoldBowman;
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
		CNewUIGoldBowmanLena*			m_pNewGoldBowmanLena;
#endif //PSW_EVENT_LENA		
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		CNewUIRegistrationLuckyCoin*	m_pNewLuckyCoinRegistration;
		CNewUIExchangeLuckyCoin*		m_pNewExchangeLuckyCoinWindow;
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_UI
		CNewUIDuelWatchWindow*			m_pNewDuelWatchWindow;
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
		CNewUIDuelWatchMainFrameWindow*	m_pNewDuelWatchMainFrameWindow;
		CNewUIDuelWatchUserListWindow* m_pNewDuelWatchUserListWindow;
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		CNewUIInGameShop* m_pNewInGameShop;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef YDG_ADD_DOPPELGANGER_UI
		CNewUIDoppelGangerWindow*		m_pNewDoppelGangerWindow;
		CNewUIDoppelGangerFrame*		m_pNewDoppelGangerFrame;
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
#ifdef ASG_ADD_UI_NPC_DIALOGUE
		CNewUINPCDialogue*				m_pNewNPCDialogue;
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		CNewUIQuestProgress*			m_pNewQuestProgress;
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
		CNewUINPCMenu*					m_pNewNPCMenu;
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		CNewUIQuestProgressByEtc*		m_pNewQuestProgressByEtc;
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
		CNewUIEmpireGuardianNPC*	m_pNewEmpireGuardianNPC;
		CNewUIEmpireGuardianTimer*  m_pNewEmpireGuardianTimer;
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PJH_ADD_MINIMAP
		CNewUIMiniMap*				m_pNewMiniMap;
#endif //PJH_ADD_MINIMAP		
#ifdef PBG_MOD_STAMINA_UI
		CNewUIStamina*				m_pNewUIStamina;
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
		CNewUIGensRanking*			m_pNewGensRanking;
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
		CNewUIUnitedMarketPlaceWindow* m_pNewUnitedMarketPlaceWindow;
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
#ifdef LEM_ADD_LUCKYITEM
		CNewUILuckyItemWnd*			m_pNewUILuckyItemWnd;
#endif // LEM_ADD_LUCKYITEM
	public:
		CNewUIChatInputBox* GetUI_NewChatInputBox() const;
		CNewUIChatLogWindow* GetUI_NewChatLogWindow() const;
		CNewUISlideWindow* GetUI_NewSlideWindow() const;
		CNewUIGuildMakeWindow* GetUI_NewGuildMakeWindow() const;
		CNewUIFriendWindow* GetUI_NewFriendWindow() const;
		CNewUIMainFrameWindow* GetUI_NewMainFrameWindow() const;
		CNewUISkillList* GetUI_NewSkillList() const;	
		CNewUIItemMng*	GetUI_NewItemMng() const;
		CNewUIMyInventory* GetUI_NewMyInventory() const;
		CNewUINPCShop* GetUI_NewNpcShop() const;
		CNewUIPetInfoWindow* GetUI_NewPetInfoWindow() const;
		CNewUIMixInventory* GetUI_NewMixInventory() const;
		CNewUICastleWindow* GetUI_NewCastleWindow() const;
		CNewUIGuardWindow* GetUI_NewGuardWindow() const;
		CNewUIGatemanWindow* GetUI_NewGatemanWindow() const;
		CNewUIGateSwitchWindow* GetUI_NewGateSwitchWindow() const;
		CNewUIStorageInventory* GetUI_NewStorageInventory() const;
		CNewUIGuildInfoWindow* GetUI_NewGuildInfoWindow() const;
		CNewUIMyShopInventory* GetUI_NewMyShopInventory() const;
		CNewUIPurchaseShopInventory* GetUI_NewPurchaseShopInventory() const;
		CNewUICharacterInfoWindow* GetUI_NewCharacterInfoWindow() const;
		CNewUIMyQuestInfoWindow* GetUI_NewMyQuestInfoWindow() const;
		CNewUIPartyInfoWindow*	GetUI_NewPartyInfoWindow() const;
		CNewUIPartyListWindow*	GetUI_NewPartyListWindow() const;
		CNewUINPCQuest* GetUI_NewNPCQuest() const;
		CNewUIEnterBloodCastle*	GetUI_NewEnterBloodCastle() const;
		CNewUIEnterDevilSquare*	GetUI_NewEnterDevilSquare() const;
		CNewUIBloodCastle* GetUI_NewBloodCastle() const;
		CNewUITrade* GetUI_NewTrade() const;
		CNewUIKanturu2ndEnterNpc* GetUI_NewKanturu2ndEnterNpc() const;
		CNewUIKanturuInfoWindow* GetUI_NewKanturuInfoWindow() const;
		CNewUICatapultWindow* GetUI_NewCatapultWindow() const;
		CNewUIChaosCastleTime* GetUI_NewChaosCastleTime( ) const;
		CNewUIBattleSoccerScore* GetUI_NewBattleSoccerScore() const;
		CNewUICommandWindow*	GetUI_NewCommandWindow() const;
		CNewUIHeroPositionInfo* GetUI_NewHeroPositionInfo() const;
		CNewUIWindowMenu* GetUI_NewWindowMenu() const;
		CNewUIOptionWindow* GetUI_NewOptionWindow() const;
		CNewUIHelpWindow* GetUI_NewHelpWindow() const;
		CNewUIItemExplanationWindow* GetUI_NewItemExplanationWindow() const;
		CNewUISetItemExplanation* GetUI_NewSetItemExplanation() const;
		CNewUIQuickCommandWindow* GetUI_NewQuickCommandWindow() const;
		CNewUIMoveCommandWindow* GetUI_NewMoveCommandWindow() const;
		CNewUIDuelWindow* GetUI_NewDuelWindow() const;
		CNewUISiegeWarfare* GetUI_NewSiegeWarfare() const;
		CNewUIItemEnduranceInfo* GetUI_NewItemEnduranceInfo() const;
		CNewUIBuffWindow* GetUI_NewBuffWindow() const;
		CNewUICursedTempleEnter* GetUI_NewCursedTempleEnterWindow() const;
		CNewUICursedTempleSystem* GetUI_NewCursedTempleWindow() const;
		CNewUICursedTempleResult* GetUI_NewCursedTempleResultWindow() const;
		CNewUICryWolf* GetUI_NewCryWolfInterface() const;
		CNewUIMasterLevel*	GetUI_NewMasterLevelInterface() const;
#ifdef NEW_USER_INTERFACE_SHELL
		CNewUIPartChargeShop* GetUI_pNewPartChargeShopInterface() const;
#endif //NEW_USER_INTERFACE_SHEL
#ifdef PSW_GOLDBOWMAN
		CNewUIGoldBowmanWindow*	GetUI_pNewGoldBowman() const;
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
		CNewUIGoldBowmanLena* GetUI_pNewGoldBowmanLena() const;
#endif //PSW_EVENT_LENA
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		CNewUIRegistrationLuckyCoin* GetUI_pNewLuckyCoinRegistration() const;
		CNewUIExchangeLuckyCoin* GetUI_pNewExchangeLuckyCoin() const;
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_UI
		CNewUIDuelWatchWindow*	GetUI_pNewDuelWatch() const;
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
		CNewUIDuelWatchMainFrameWindow*	GetUI_pNewDuelWatchMainFrame() const;
		CNewUIDuelWatchUserListWindow* GetUI_pNewDuelWatchUserList() const;
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
		CNewUIInGameShop* GetUI_pNewInGameShop() const;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef YDG_ADD_DOPPELGANGER_UI
		CNewUIDoppelGangerWindow*	GetUI_pNewDoppelGangerWindow() const;
		CNewUIDoppelGangerFrame*	GetUI_pNewDoppelGangerFrame() const;
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
		CNewUINPCDialogue* GetUI_NewNPCDialogue() const;
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		CNewUIQuestProgress* GetUI_NewQuestProgress() const;
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
		CNewUINPCMenu* GetUI_NewNPCMenu() const;
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		CNewUIQuestProgressByEtc* GetUI_NewQuestProgressByEtc() const;
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
		CNewUIEmpireGuardianNPC* GetUI_pNewEmpireGuardianNPC() const;
		CNewUIEmpireGuardianTimer* GetUI_pNewEmpireGuardianTimer() const;
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PJH_ADD_MINIMAP
		CNewUIMiniMap* GetUI_pNewUIMiniMap() const;
#endif //PJH_ADD_MINIMAP
#ifdef PBG_MOD_STAMINA_UI
		CNewUIStamina* GetUI_pNewUIStamina() const;
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
		CNewUIGensRanking* GetUI_NewGensRanking() const;
#endif //PBG_ADD_GENSRANKING
#ifdef LDS_ADD_UI_UNITEDMARKETPLACE
		CNewUIUnitedMarketPlaceWindow*	GetUI_pNewUnitedMarketPlaceWindow() const;
		//CNewUIUnitedMarketPlaceWindow*	GetUI_pNewUnitedMarketPlaceFrame() const;
#endif // LDS_ADD_UI_UNITEDMARKETPLACE
#ifdef LEM_ADD_LUCKYITEM
		CNewUILuckyItemWnd*	Get_pNewUILuckyItemWnd() const;
#endif // LEM_ADD_LUCKYITEM
	};
}

#define g_pNewUISystem SEASON3B::CNewUISystem::GetInstance()
#define g_pNewUIMng SEASON3B::CNewUISystem::GetInstance()->GetNewUIManager()
#define g_pNewUI3DRenderMng SEASON3B::CNewUISystem::GetInstance()->GetNewUI3DRenderMng()
#define g_pNewUIHotKey SEASON3B::CNewUISystem::GetInstance()->GetNewUIHotKey()
#define g_pNewItemMng SEASON3B::CNewUISystem::GetInstance()->GetUI_NewItemMng()
#define g_pChatInputBox SEASON3B::CNewUISystem::GetInstance()->GetUI_NewChatInputBox()
#define g_pChatListBox SEASON3B::CNewUISystem::GetInstance()->GetUI_NewChatLogWindow()
#define g_pSlideHelpMgr SEASON3B::CNewUISystem::GetInstance()->GetUI_NewSlideWindow()
#define g_pWindowMgr SEASON3B::CNewUISystem::GetInstance()->GetUI_NewFriendWindow()
#define g_pMainFrame SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMainFrameWindow()
#define g_pSkillList SEASON3B::CNewUISystem::GetInstance()->GetUI_NewSkillList()
#define g_pMyInventory SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMyInventory()
#define g_pNPCShop SEASON3B::CNewUISystem::GetInstance()->GetUI_NewNpcShop()
#define g_pPetInfoWindow	SEASON3B::CNewUISystem::GetInstance()->GetUI_NewPetInfoWindow()
#define g_pMixInventory SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMixInventory()
#define g_pCastleWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCastleWindow()
#define g_pGuardWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGuardWindow()
#define g_pGatemanWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGatemanWindow()
#define g_pGateSwitchWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGateSwitchWindow()
#define g_pStorageInventory SEASON3B::CNewUISystem::GetInstance()->GetUI_NewStorageInventory()
#define g_pGuildMakeWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGuildMakeWindow()
#define g_pGuildInfoWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGuildInfoWindow()
#define g_pMyShopInventory SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMyShopInventory()
#define g_pPurchaseShopInventory SEASON3B::CNewUISystem::GetInstance()->GetUI_NewPurchaseShopInventory()
#define g_pCharacterInfoWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCharacterInfoWindow()
#define g_pMyQuestInfoWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMyQuestInfoWindow()
#define g_pPartyInfoWindow		SEASON3B::CNewUISystem::GetInstance()->GetUI_NewPartyInfoWindow()
#define g_pPartyListWindow		SEASON3B::CNewUISystem::GetInstance()->GetUI_NewPartyListWindow()
#define g_pNPCQuest SEASON3B::CNewUISystem::GetInstance()->GetUI_NewNPCQuest()
#define g_pEnterBloodCastle SEASON3B::CNewUISystem::GetInstance()->GetUI_NewEnterBloodCastle()
#define g_pEnterDevilSquare SEASON3B::CNewUISystem::GetInstance()->GetUI_NewEnterDevilSquare()
#define g_pBloodCastle SEASON3B::CNewUISystem::GetInstance()->GetUI_NewBloodCastle()
#define g_pTrade SEASON3B::CNewUISystem::GetInstance()->GetUI_NewTrade()
#define g_pKanturu2ndEnterNpc SEASON3B::CNewUISystem::GetInstance()->GetUI_NewKanturu2ndEnterNpc()
#define g_pKanturuInfoWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewKanturuInfoWindow()
#define g_pCatapultWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCatapultWindow()
#define g_pChaosCastleTime SEASON3B::CNewUISystem::GetInstance()->GetUI_NewChaosCastleTime()
#define g_pBattleSoccerScore SEASON3B::CNewUISystem::GetInstance()->GetUI_NewBattleSoccerScore()
#define g_pCommandWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCommandWindow()
#define g_pWindowMenu SEASON3B::CNewUISystem::GetInstance()->GetUI_NewWindowMenu()
#define g_pOption SEASON3B::CNewUISystem::GetInstance()->GetUI_NewOptionWindow()
#define g_pHeroPositionInfo SEASON3B::CNewUISystem::GetInstance()->GetUI_NewHeroPositionInfo()
#define g_pHelp SEASON3B::CNewUISystem::GetInstance()->GetUI_NewHelpWindow()
#define g_pItemExplanation SEASON3B::CNewUISystem::GetInstance()->GetUI_NewItemExplanationWindow()
#define g_pSetItemExplanation SEASON3B::CNewUISystem::GetInstance()->GetUI_NewSetItemExplanation()
#define g_pQuickCommand SEASON3B::CNewUISystem::GetInstance()->GetUI_NewQuickCommandWindow()
#define g_pMoveCommandWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMoveCommandWindow()
#define g_pDuelWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewDeulWindow()
#define g_pSiegeWarfare SEASON3B::CNewUISystem::GetInstance()->GetUI_NewSiegeWarfare()
#define g_pItemEnduranceInfo SEASON3B::CNewUISystem::GetInstance()->GetUI_NewItemEnduranceInfo()
#define g_pBuffWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewBuffWindow()
#define g_pCursedTempleEnterWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCursedTempleEnterWindow()
#define g_pCursedTempleWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCursedTempleWindow()
#define g_pCursedTempleResultWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCursedTempleResultWindow()
#define g_pCryWolfInterface SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCryWolfInterface()
#define g_pMasterLevelInterface SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMasterLevelInterface()
#ifdef NEW_USER_INTERFACE_SHELL
#define g_pPartChargeShopInterface SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewPartChargeShopInterface()
#endif //NEW_USER_INTERFACE_SHEL
#ifdef PSW_GOLDBOWMAN
#define g_pGoldBowmanInterface SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewGoldBowman()
#endif //PSW_GOLDBOWMAN
#ifdef PSW_EVENT_LENA
#define g_pGoldBowmanLenaInterface SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewGoldBowmanLena()
#endif //PSW_EVENT_LENA
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
#define g_pLuckyCoinRegistration SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewLuckyCoinRegistration()
#define g_pExchangeLuckyCoinWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewExchangeLuckyCoin()
#endif // KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_UI
#define g_pDuelWatchWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDuelWatch()
#endif	// YDG_ADD_NEW_DUEL_UI	
#ifdef YDG_ADD_NEW_DUEL_WATCH_BUFF
#define g_pDuelWatchMainFrameWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDuelWatchMainFrame()
#define g_pDuelWatchUserList SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDuelWatchUserList()
#endif	// YDG_ADD_NEW_DUEL_WATCH_BUFF
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#define g_pInGameShop SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewInGameShop()
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#ifdef YDG_ADD_DOPPELGANGER_UI
#define g_pDoppelGangerWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDoppelGangerWindow()
#define g_pDoppelGangerFrame SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDoppelGangerFrame()
#endif	// YDG_ADD_DOPPELGANGER_UI
#ifdef ASG_ADD_UI_NPC_DIALOGUE
#define g_pNPCDialogue SEASON3B::CNewUISystem::GetInstance()->GetUI_NewNPCDialogue()
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#ifdef ASG_ADD_UI_QUEST_PROGRESS
#define g_pQuestProgress SEASON3B::CNewUISystem::GetInstance()->GetUI_NewQuestProgress()
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_NPC_MENU
#define g_pNPCMenu SEASON3B::CNewUISystem::GetInstance()->GetUI_NewNPCMenu()
#endif	// ASG_ADD_UI_NPC_MENU
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
#define g_pQuestProgressByEtc SEASON3B::CNewUISystem::GetInstance()->GetUI_NewQuestProgressByEtc()
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
#ifdef LDK_ADD_EMPIREGUARDIAN_UI
#define g_pEmpireGuardianNPC SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewEmpireGuardianNPC()
#define g_pEmpireGuardianTimer SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewEmpireGuardianTimer()
#endif //LDK_ADD_EMPIREGUARDIAN_UI
#ifdef PJH_ADD_MINIMAP
#define g_pNewUIMiniMap SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewUIMiniMap()
#endif //PJH_ADD_MINIMAP
#ifdef PBG_MOD_STAMINA_UI
#define g_pNewUIStamina SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewUIStamina()
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
#define g_pNewUIGensRanking SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGensRanking()
#endif //PBG_ADD_GENSRANKING
#ifdef LEM_ADD_LUCKYITEM
#define g_pLuckyItemWnd	SEASON3B::CNewUISystem::GetInstance()->Get_pNewUILuckyItemWnd()
#endif // LEM_ADD_LUCKYITEM
#endif // _NEWUISYSTEM_H_