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
#include "NewUIStorageInventoryExt.h"
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
#include "NewUIGoldBowmanWindow.h"
#include "NewUIGoldBowmanLena.h"
#include "NewUIRegistrationLuckyCoin.h"
#include "NewUIExchangeLuckyCoin.h"
#include "NewUIDuelWatchWindow.h"
#include "NewUIDuelWatchMainFrameWindow.h"
#include "NewUIDuelWatchUserListWindow.h"
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "GameShop\NewUIInGameShop.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "NewUIDoppelGangerWindow.h"
#include "NewUIDoppelGangerFrame.h"
#include "NewUIQuestProgress.h"
#include "NewUIQuestProgressByEtc.h"
#include "NewUIEmpireGuardianNPC.h"
#include "NewUIEmpireGuardianTimer.h"
#include "NewUINPCDialogue.h"
#include "NewUIInventoryExtension.h"
#include "NewUIMiniMap.h"
#include "NewUIGensRanking.h"
#include "NewUIUnitedMarketPlaceWindow.h"

#include "NewUILuckyItemWnd.h"
#include "NewUIMuHelper.h"

namespace SEASON3B
{
    class CNewUISystem
    {
        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;
        CNewUIHotKey* m_pNewUIHotKey;

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

        void Enable(DWORD dwKey);
        void Disable(DWORD dwKey);

        bool CheckMouseUse();
        bool CheckKeyUse();

        bool Update();
        bool Render();

        CNewUIManager* GetNewUIManager() const;
        CNewUI3DRenderMng* GetNewUI3DRenderMng() const;
        CNewUIHotKey* GetNewUIHotKey() const;

        bool IsImpossibleSendMoveInterface();
        void UpdateSendMoveInterface();
        bool IsImpossibleTradeInterface();
        bool IsImpossibleDuelInterface();
        bool IsImpossibleHideInterface(DWORD dwKey);

        static CNewUISystem* GetInstance();

    protected:
        CNewUISystem();		//. ban to create instance

        void HideAllGroupA();
        void HideAllGroupB();
        void HideGroupBeforeOpenInterface();

        /* Interface classes */
    private:
        CNewUIChatInputBox* m_pNewChatInputBox;
        CNewUIChatLogWindow* m_pNewChatLogWindow;
        CNewUISystemLogWindow* m_pNewSystemLogWindow;
        CNewUISlideWindow* m_pNewSlideWindow;
        CNewUIFriendWindow* m_pNewFriendWindow;
        CNewUIMainFrameWindow* m_pNewMainFrameWindow;
        CNewUISkillList* m_pNewSkillList;
        CNewUIItemMng* m_pNewItemMng;
        CNewUIMyInventory* m_pNewMyInventory;
        CNewUIInventoryExtension* m_pNewMyInventoryExt;
        CNewUINPCShop* m_pNewNPCShop;
        CNewUIPetInfoWindow* m_pNewPetInfoWindow;
        CNewUIMixInventory* m_pNewMixInventory;
        CNewUICastleWindow* m_pNewCastleWindow;
        CNewUIGuardWindow* m_pNewGuardWindow;
        CNewUIGatemanWindow* m_pNewGatemanWindow;
        CNewUIGateSwitchWindow* m_pNewGateSwitchWindow;
        CNewUIStorageInventory* m_pNewStorageInventory;
        CNewUIStorageInventoryExt* m_pNewStorageInventoryExt;
        CNewUIGuildMakeWindow* m_pNewGuildMakeWindow;
        CNewUIGuildInfoWindow* m_pNewGuildInfoWindow;
        CNewUIMyShopInventory* m_pNewMyShopInventory;
        CNewUIPurchaseShopInventory* m_pNewPurchaseShopInventory;
        CNewUICharacterInfoWindow* m_pNewCharacterInfoWindow;
        CNewUIMyQuestInfoWindow* m_pNewMyQuestInfoWindow;
        CNewUIPartyInfoWindow* m_pNewPartyInfoWindow;
        CNewUIPartyListWindow* m_pNewPartyListWindow;
        CNewUINPCQuest* m_pNewNPCQuest;
        CNewUIEnterBloodCastle* m_pNewEnterBloodCastle;
        CNewUIEnterDevilSquare* m_pNewEnterDevilSquare;
        CNewUIBloodCastle* m_pNewBloodCastle;
        CNewUITrade* m_pNewTrade;
        CNewUIKanturu2ndEnterNpc* m_pNewKanturu2ndEnterNpc;
        CNewUIKanturuInfoWindow* m_pNewKanturuInfoWindow;
        CNewUICatapultWindow* m_pNewCatapultWindow;
        CNewUIChaosCastleTime* m_pNewChaosCastleTime;
        CNewUIBattleSoccerScore* m_pNewBattleSoccerScore;
        CNewUICommandWindow* m_pNewCommandWindow;
        CNewUIHeroPositionInfo* m_pNewHeroPositionInfo;
        CNewUIWindowMenu* m_pNewWindowMenu;
        CNewUIOptionWindow* m_pNewOptionWindow;
        CNewUIHelpWindow* m_pNewHelpWindow;
        CNewUIItemExplanationWindow* m_pNewItemExplanationWindow;
        CNewUISetItemExplanation* m_pNewSetItemExplanation;
        CNewUIQuickCommandWindow* m_pNewQuickCommandWindow;
        CNewUIMoveCommandWindow* m_pNewMoveCommandWindow;
        CNewUIDuelWindow* m_pNewDuelWindow;
        CNewUINameWindow* m_pNewNameWindow;
        CNewUISiegeWarfare* m_pNewSiegeWarfare;
        CNewUIItemEnduranceInfo* m_pNewItemEnduranceInfo;
        CNewUIBuffWindow* m_pNewBuffWindow;
        CNewUICursedTempleEnter* m_pNewCursedTempleEnterWindow;
        CNewUICursedTempleSystem* m_pNewCursedTempleWindow;
        CNewUICursedTempleResult* m_pNewCursedTempleResultWindow;
        CNewUICryWolf* m_pNewCryWolfInterface;
        CNewUIMasterLevel* m_pNewMaster_Level_Interface;
        CNewUIGoldBowmanWindow* m_pNewGoldBowman;
        CNewUIGoldBowmanLena* m_pNewGoldBowmanLena;
        CNewUIRegistrationLuckyCoin* m_pNewLuckyCoinRegistration;
        CNewUIExchangeLuckyCoin* m_pNewExchangeLuckyCoinWindow;
        CNewUIDuelWatchWindow* m_pNewDuelWatchWindow;
        CNewUIDuelWatchMainFrameWindow* m_pNewDuelWatchMainFrameWindow;
        CNewUIDuelWatchUserListWindow* m_pNewDuelWatchUserListWindow;
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CNewUIInGameShop* m_pNewInGameShop;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CNewUIDoppelGangerWindow* m_pNewDoppelGangerWindow;
        CNewUIDoppelGangerFrame* m_pNewDoppelGangerFrame;
        CNewUINPCDialogue* m_pNewNPCDialogue;
        CNewUIQuestProgress* m_pNewQuestProgress;
        CNewUIQuestProgressByEtc* m_pNewQuestProgressByEtc;
        CNewUIEmpireGuardianNPC* m_pNewEmpireGuardianNPC;
        CNewUIEmpireGuardianTimer* m_pNewEmpireGuardianTimer;
        CNewUIMiniMap* m_pNewMiniMap;
        CNewUIGensRanking* m_pNewGensRanking;
        CNewUIUnitedMarketPlaceWindow* m_pNewUnitedMarketPlaceWindow;
        CNewUILuckyItemWnd* m_pNewUILuckyItemWnd;
        CNewUIMuHelper* m_pNewUIMuHelper;
        CNewUIMuHelperExt* m_pNewUIMuHelperExt;
        CNewUIMuHelperSkillList* m_pNewUIMuHelperSkillList;
    public:
        CNewUIChatInputBox* GetUI_NewChatInputBox() const;
        CNewUIChatLogWindow* GetUI_NewChatLogWindow() const;
        CNewUISystemLogWindow* GetUI_NewSystemLogWindow() const;
        CNewUISlideWindow* GetUI_NewSlideWindow() const;
        CNewUIGuildMakeWindow* GetUI_NewGuildMakeWindow() const;
        CNewUIFriendWindow* GetUI_NewFriendWindow() const;
        CNewUIMainFrameWindow* GetUI_NewMainFrameWindow() const;
        CNewUISkillList* GetUI_NewSkillList() const;
        CNewUIItemMng* GetUI_NewItemMng() const;
        CNewUIMyInventory* GetUI_NewMyInventory() const;
        CNewUIInventoryExtension* GetUI_NewMyInventoryExt() const;
        CNewUINPCShop* GetUI_NewNpcShop() const;
        CNewUIPetInfoWindow* GetUI_NewPetInfoWindow() const;
        CNewUIMixInventory* GetUI_NewMixInventory() const;
        CNewUICastleWindow* GetUI_NewCastleWindow() const;
        CNewUIGuardWindow* GetUI_NewGuardWindow() const;
        CNewUIGatemanWindow* GetUI_NewGatemanWindow() const;
        CNewUIGateSwitchWindow* GetUI_NewGateSwitchWindow() const;
        CNewUIStorageInventory* GetUI_NewStorageInventory() const;
        CNewUIStorageInventoryExt* GetUI_NewStorageInventoryExt() const;
        CNewUIGuildInfoWindow* GetUI_NewGuildInfoWindow() const;
        CNewUIMyShopInventory* GetUI_NewMyShopInventory() const;
        CNewUIPurchaseShopInventory* GetUI_NewPurchaseShopInventory() const;
        CNewUICharacterInfoWindow* GetUI_NewCharacterInfoWindow() const;
        CNewUIMyQuestInfoWindow* GetUI_NewMyQuestInfoWindow() const;
        CNewUIPartyInfoWindow* GetUI_NewPartyInfoWindow() const;
        CNewUIPartyListWindow* GetUI_NewPartyListWindow() const;
        CNewUINPCQuest* GetUI_NewNPCQuest() const;
        CNewUIEnterBloodCastle* GetUI_NewEnterBloodCastle() const;
        CNewUIEnterDevilSquare* GetUI_NewEnterDevilSquare() const;
        CNewUIBloodCastle* GetUI_NewBloodCastle() const;
        CNewUITrade* GetUI_NewTrade() const;
        CNewUIKanturu2ndEnterNpc* GetUI_NewKanturu2ndEnterNpc() const;
        CNewUIKanturuInfoWindow* GetUI_NewKanturuInfoWindow() const;
        CNewUICatapultWindow* GetUI_NewCatapultWindow() const;
        CNewUIChaosCastleTime* GetUI_NewChaosCastleTime() const;
        CNewUIBattleSoccerScore* GetUI_NewBattleSoccerScore() const;
        CNewUICommandWindow* GetUI_NewCommandWindow() const;
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
        CNewUIMasterLevel* GetUI_NewMasterLevelInterface() const;
        CNewUIGoldBowmanWindow* GetUI_pNewGoldBowman() const;
        CNewUIGoldBowmanLena* GetUI_pNewGoldBowmanLena() const;
        CNewUIRegistrationLuckyCoin* GetUI_pNewLuckyCoinRegistration() const;
        CNewUIExchangeLuckyCoin* GetUI_pNewExchangeLuckyCoin() const;
        CNewUIDuelWatchWindow* GetUI_pNewDuelWatch() const;
        CNewUIDuelWatchMainFrameWindow* GetUI_pNewDuelWatchMainFrame() const;
        CNewUIDuelWatchUserListWindow* GetUI_pNewDuelWatchUserList() const;
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CNewUIInGameShop* GetUI_pNewInGameShop() const;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CNewUIDoppelGangerWindow* GetUI_pNewDoppelGangerWindow() const;
        CNewUIDoppelGangerFrame* GetUI_pNewDoppelGangerFrame() const;
        CNewUINPCDialogue* GetUI_NewNPCDialogue() const;
        CNewUIQuestProgress* GetUI_NewQuestProgress() const;
        CNewUIQuestProgressByEtc* GetUI_NewQuestProgressByEtc() const;
        CNewUIEmpireGuardianNPC* GetUI_pNewEmpireGuardianNPC() const;
        CNewUIEmpireGuardianTimer* GetUI_pNewEmpireGuardianTimer() const;
        CNewUIMiniMap* GetUI_pNewUIMiniMap() const;
        CNewUIGensRanking* GetUI_NewGensRanking() const;
        CNewUIUnitedMarketPlaceWindow* GetUI_pNewUnitedMarketPlaceWindow() const;
        //CNewUIUnitedMarketPlaceWindow*	GetUI_pNewUnitedMarketPlaceFrame() const;
        CNewUILuckyItemWnd* Get_pNewUILuckyItemWnd() const;
        CNewUIMuHelper* Get_pNewUIMuHelper() const;
        CNewUIMuHelperExt* Get_pNewUIMuHelperExt() const;
        CNewUIMuHelperSkillList* Get_pNewUIMuHelperSkillList() const;
    };
}

#define g_pNewUISystem SEASON3B::CNewUISystem::GetInstance()
#define g_pNewUIMng SEASON3B::CNewUISystem::GetInstance()->GetNewUIManager()
#define g_pNewUI3DRenderMng SEASON3B::CNewUISystem::GetInstance()->GetNewUI3DRenderMng()
#define g_pNewUIHotKey SEASON3B::CNewUISystem::GetInstance()->GetNewUIHotKey()
#define g_pNewItemMng SEASON3B::CNewUISystem::GetInstance()->GetUI_NewItemMng()
#define g_pChatInputBox SEASON3B::CNewUISystem::GetInstance()->GetUI_NewChatInputBox()
#define g_pChatListBox SEASON3B::CNewUISystem::GetInstance()->GetUI_NewChatLogWindow()
#define g_pSystemLogBox SEASON3B::CNewUISystem::GetInstance()->GetUI_NewSystemLogWindow()
#define g_pSlideHelpMgr SEASON3B::CNewUISystem::GetInstance()->GetUI_NewSlideWindow()
#define g_pWindowMgr SEASON3B::CNewUISystem::GetInstance()->GetUI_NewFriendWindow()
#define g_pMainFrame SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMainFrameWindow()
#define g_pSkillList SEASON3B::CNewUISystem::GetInstance()->GetUI_NewSkillList()
#define g_pMyInventory SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMyInventory()
#define g_pMyInventoryExt SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMyInventoryExt()
#define g_pNPCShop SEASON3B::CNewUISystem::GetInstance()->GetUI_NewNpcShop()
#define g_pPetInfoWindow	SEASON3B::CNewUISystem::GetInstance()->GetUI_NewPetInfoWindow()
#define g_pMixInventory SEASON3B::CNewUISystem::GetInstance()->GetUI_NewMixInventory()
#define g_pCastleWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewCastleWindow()
#define g_pGuardWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGuardWindow()
#define g_pGatemanWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGatemanWindow()
#define g_pGateSwitchWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGateSwitchWindow()
#define g_pStorageInventory SEASON3B::CNewUISystem::GetInstance()->GetUI_NewStorageInventory()
#define g_pStorageInventoryExt SEASON3B::CNewUISystem::GetInstance()->GetUI_NewStorageInventoryExt()
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
#define g_pGoldBowmanInterface SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewGoldBowman()
#define g_pGoldBowmanLenaInterface SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewGoldBowmanLena()
#define g_pLuckyCoinRegistration SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewLuckyCoinRegistration()
#define g_pExchangeLuckyCoinWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewExchangeLuckyCoin()
#define g_pDuelWatchWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDuelWatch()
#define g_pDuelWatchMainFrameWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDuelWatchMainFrame()
#define g_pDuelWatchUserList SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDuelWatchUserList()
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#define g_pInGameShop SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewInGameShop()
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#define g_pDoppelGangerWindow SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDoppelGangerWindow()
#define g_pDoppelGangerFrame SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewDoppelGangerFrame()
#define g_pNPCDialogue SEASON3B::CNewUISystem::GetInstance()->GetUI_NewNPCDialogue()
#define g_pQuestProgress SEASON3B::CNewUISystem::GetInstance()->GetUI_NewQuestProgress()
#define g_pQuestProgressByEtc SEASON3B::CNewUISystem::GetInstance()->GetUI_NewQuestProgressByEtc()
#define g_pEmpireGuardianNPC SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewEmpireGuardianNPC()
#define g_pEmpireGuardianTimer SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewEmpireGuardianTimer()
#define g_pNewUIMiniMap SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewUIMiniMap()
#ifdef PBG_MOD_STAMINA_UI
#define g_pNewUIStamina SEASON3B::CNewUISystem::GetInstance()->GetUI_pNewUIStamina()
#endif //PBG_MOD_STAMINA_UI
#define g_pNewUIGensRanking SEASON3B::CNewUISystem::GetInstance()->GetUI_NewGensRanking()
#define g_pLuckyItemWnd	SEASON3B::CNewUISystem::GetInstance()->Get_pNewUILuckyItemWnd()
#define g_pNewUIMuHelper SEASON3B::CNewUISystem::GetInstance()->Get_pNewUIMuHelper()
#define g_pNewUIMuHelperExt SEASON3B::CNewUISystem::GetInstance()->Get_pNewUIMuHelperExt()
#define g_pNewUIMuHelperSkillList SEASON3B::CNewUISystem::GetInstance()->Get_pNewUIMuHelperSkillList()
#endif // _NEWUISYSTEM_H_