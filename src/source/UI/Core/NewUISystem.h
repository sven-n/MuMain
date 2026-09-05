#ifndef _NEWUISYSTEM_H_
#define _NEWUISYSTEM_H_

#pragma once

#include "UI/Core/NewUIManager.h"
#include "UI/Core/NewUI3DRenderMng.h"
#include "UI/HUD/NewUIHotKey.h"
#include "UI/HUD/NewUIChatLogWindow.h"
#include "UI/Widgets/NewUISlideWindow.h"
#include "Guild/NewUIGuildMakeWindow.h"
#include "UI/Party/NewUIFriendWindow.h"
#include "UI/HUD/NewUIMainFrameWindow.h"
#include "UI/Widgets/NewUIChatInputBox.h"
#include "UI/Inventory/NewUIItemMng.h"
#include "UI/Inventory/NewUIMyInventory.h"
#include "UI/NPCs/NewUINPCShop.h"
#include "UI/Character/NewUIPetInfoWindow.h"
#include "UI/Inventory/NewUIMixInventory.h"
#include "UI/Combat/NewUICastleWindow.h"
#include "UI/Combat/NewUIGuardWindow.h"
#include "UI/NPCs/NewUIGatemanWindow.h"
#include "UI/Events/NewUIGateSwitchWindow.h"
#include "UI/Inventory/NewUIStorageInventory.h"
#include "UI/Inventory/NewUIStorageInventoryExt.h"
#include "Guild/NewUIGuildInfoWindow.h"
#include "UI/Inventory/NewUIMyShopInventory.h"
#include "UI/Inventory/NewUIPurchaseShopInventory.h"
#include "UI/Character/NewUICharacterInfoWindow.h"
#include "UI/Quests/NewUIMyQuestInfoWindow.h"
#include "UI/Party/NewUIPartyInfoWindow.h"
#include "UI/Party/NewUIPartyListWindow.h"
#include "UI/Quests/NewUINPCQuest.h"
#include "UI/Events/NewUIEnterDevilSquare.h"
#include "UI/Events/NewUIBloodCastleEnter.h"
#include "UI/Events/NewUIBloodCastleTime.h"
#include "UI/Inventory/NewUITrade.h"
#include "UI/Events/NewUIKanturuEvent.h"
#include "UI/Events/NewUICatapultWindow.h"
#include "UI/Events/NewUIChaosCastleTime.h"
#include "UI/Events/NewUIBattleSoccerScore.h"
#include "UI/HUD/NewUICommandWindow.h"
#include "UI/Dialogs/NewUIWindowMenu.h"
#include "UI/Options/NewUIOptionWindow.h"
#include "UI/HUD/MuHelperBar.h"
#include "UI/Dialogs/NewUIHelpWindow.h"
#include "UI/Dialogs/NewUIChatCommandWindow.h"
#include "UI/Inventory/NewUIItemExplanationWindow.h"
#include "UI/Inventory/NewUISetItemExplanation.h"
#include "UI/HUD/NewUIQuickCommandWindow.h"
#include "UI/HUD/NewUIMoveCommandWindow.h"
#include "UI/Combat/NewUIDuelWindow.h"
#include "UI/Combat/NewUISeigeWarfare.h"
#include "UI/Character/NewUINameWindow.h"
#include "UI/Inventory/NewUIItemEnduranceInfo.h"
#include "UI/HUD/BuffStrip.h"
#include "UI/Events/NewUICursedTempleEnter.h"
#include "UI/Events/NewUICursedTempleSystem.h"
#include "UI/Events/NewUICursedTempleResult.h"
#include "UI/Events/NewUICryWolf.h"
#include "UI/HUD/NewUIMasterLevel.h"
#include "UI/Events/NewUIGoldBowmanWindow.h"
#include "UI/Events/NewUIGoldBowmanLena.h"
#include "UI/Events/NewUIRegistrationLuckyCoin.h"
#include "UI/Events/NewUIExchangeLuckyCoin.h"
#include "UI/Combat/NewUIDuelWatchWindow.h"
#include "UI/Combat/NewUIDuelWatchMainFrameWindow.h"
#include "UI/Combat/NewUIDuelWatchUserListWindow.h"
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "GameShop/NewUIInGameShop.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "UI/Events/NewUIDoppelGangerWindow.h"
#include "UI/Events/NewUIDoppelGangerFrame.h"
#include "UI/Quests/NewUIQuestProgress.h"
#include "UI/Quests/NewUIQuestProgressByEtc.h"
#include "UI/NPCs/NewUIEmpireGuardianNPC.h"
#include "UI/NPCs/NewUIEmpireGuardianTimer.h"
#include "UI/NPCs/NewUINPCDialogue.h"
#include "UI/Inventory/NewUIInventoryExtension.h"
#include "UI/HUD/NewUIMiniMap.h"
#include "UI/HUD/NewUIGensRanking.h"
#include "UI/Inventory/NewUIUnitedMarketPlaceWindow.h"

#include "UI/Inventory/NewUILuckyItemWnd.h"
#include "UI/Core/NewUIMuHelper.h"

namespace mu::ui::window
{
    class CNewUISystem
    {
        CNewUIManager* m_pNewUIMng;
        CNewUI3DRenderMng* m_pNewUI3DRenderMng;
        CNewUIHotKey* m_pNewUIHotKey;
        bool m_mouseInputCaptured = false;

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

        // Shared handler for the top-right "X" close glyph baked into the common
        // item-frame (newui_item_back04.tga). Hides `dwKey` when the corner is
        // left-clicked and swallows the mouse so the click can't fall through to
        // the world (which would walk the character). Returns true if handled.
        // Replaces the ptExitBtn1 block that was copy-pasted across the windows.
        bool HandleFrameCornerClose(const POINT& winPos, DWORD dwKey);

        void Enable(DWORD dwKey);
        void Disable(DWORD dwKey);

        bool CheckMouseUse();
        bool CheckKeyUse();

        bool Update();
        bool Render();

        // Re-syncs the MU Helper bar's and buff strip's RmlUi documents against the current
        // scene, independent of Update()/Render() above (which -- like everything else in this
        // class -- only ever run while SceneFlag == MAIN_SCENE, MainScene.cpp). That was already
        // a complete visibility gate before these two became RmlUi-backed (nothing drew
        // otherwise); now that a persistent RmlUi document owns their visuals, leaving MAIN_SCENE
        // needs an explicit hide, from somewhere that keeps ticking after Update() stops -- called
        // every frame, regardless of scene, from Winmain.cpp's SetPostRmlUiCallback. See
        // CMuHelperBar::SyncDocVisibility()'s header comment for the full rationale.
        void SyncMainSceneHudVisibility();

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
        void UpdateMuHelperBarVisibilityForLayoutChange(DWORD dwKey);
        void SyncMuHelperBarVisibility();
        bool ShouldHideMuHelperBar();

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
        CMuHelperBar* m_pMuHelperBar;
        CNewUIWindowMenu* m_pNewWindowMenu;
        CNewUIOptionWindow* m_pNewOptionWindow;
        CNewUIHelpWindow* m_pNewHelpWindow;
        CNewUIChatCommandWindow* m_pNewChatCommandWindow;
        CNewUIItemExplanationWindow* m_pNewItemExplanationWindow;
        CNewUISetItemExplanation* m_pNewSetItemExplanation;
        CNewUIQuickCommandWindow* m_pNewQuickCommandWindow;
        CNewUIMoveCommandWindow* m_pNewMoveCommandWindow;
        CNewUIDuelWindow* m_pNewDuelWindow;
        CNewUINameWindow* m_pNewNameWindow;
        CNewUISiegeWarfare* m_pNewSiegeWarfare;
        CNewUIItemEnduranceInfo* m_pNewItemEnduranceInfo;
        CBuffStrip* m_pBuffStrip;
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

        bool CreateChatCommandWindow();

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
        CMuHelperBar* GetUI_MuHelperBar() const;
        CNewUIWindowMenu* GetUI_NewWindowMenu() const;
        CNewUIOptionWindow* GetUI_NewOptionWindow() const;
        CNewUIHelpWindow* GetUI_NewHelpWindow() const;
        CNewUIChatCommandWindow* GetUI_NewChatCommandWindow() const;
        CNewUIItemExplanationWindow* GetUI_NewItemExplanationWindow() const;
        CNewUISetItemExplanation* GetUI_NewSetItemExplanation() const;
        CNewUIQuickCommandWindow* GetUI_NewQuickCommandWindow() const;
        CNewUIMoveCommandWindow* GetUI_NewMoveCommandWindow() const;
        CNewUIDuelWindow* GetUI_NewDuelWindow() const;
        CNewUISiegeWarfare* GetUI_NewSiegeWarfare() const;
        CNewUIItemEnduranceInfo* GetUI_NewItemEnduranceInfo() const;
        CBuffStrip* GetUI_BuffStrip() const;
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

#define g_pNewUISystem mu::ui::window::CNewUISystem::GetInstance()
#define g_pNewUIMng mu::ui::window::CNewUISystem::GetInstance()->GetNewUIManager()
#define g_pNewUI3DRenderMng mu::ui::window::CNewUISystem::GetInstance()->GetNewUI3DRenderMng()
#define g_pNewUIHotKey mu::ui::window::CNewUISystem::GetInstance()->GetNewUIHotKey()
#define g_pNewItemMng mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewItemMng()
#define g_pChatInputBox mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewChatInputBox()
#define g_pChatListBox mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewChatLogWindow()
#define g_pSystemLogBox mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewSystemLogWindow()
#define g_pSlideHelpMgr mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewSlideWindow()
#define g_pWindowMgr mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewFriendWindow()
#define g_pMainFrame mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewMainFrameWindow()
#define g_pSkillList mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewSkillList()
#define g_pMyInventory mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewMyInventory()
#define g_pMyInventoryExt mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewMyInventoryExt()
#define g_pNPCShop mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewNpcShop()
#define g_pPetInfoWindow	mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewPetInfoWindow()
#define g_pMixInventory mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewMixInventory()
#define g_pCastleWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewCastleWindow()
#define g_pGuardWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewGuardWindow()
#define g_pGatemanWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewGatemanWindow()
#define g_pGateSwitchWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewGateSwitchWindow()
#define g_pStorageInventory mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewStorageInventory()
#define g_pStorageInventoryExt mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewStorageInventoryExt()
#define g_pGuildMakeWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewGuildMakeWindow()
#define g_pGuildInfoWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewGuildInfoWindow()
#define g_pMyShopInventory mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewMyShopInventory()
#define g_pPurchaseShopInventory mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewPurchaseShopInventory()
#define g_pCharacterInfoWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewCharacterInfoWindow()
#define g_pMyQuestInfoWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewMyQuestInfoWindow()
#define g_pPartyInfoWindow		mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewPartyInfoWindow()
#define g_pPartyListWindow		mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewPartyListWindow()
#define g_pNPCQuest mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewNPCQuest()
#define g_pEnterBloodCastle mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewEnterBloodCastle()
#define g_pEnterDevilSquare mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewEnterDevilSquare()
#define g_pBloodCastle mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewBloodCastle()
#define g_pTrade mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewTrade()
#define g_pKanturu2ndEnterNpc mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewKanturu2ndEnterNpc()
#define g_pKanturuInfoWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewKanturuInfoWindow()
#define g_pCatapultWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewCatapultWindow()
#define g_pChaosCastleTime mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewChaosCastleTime()
#define g_pBattleSoccerScore mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewBattleSoccerScore()
#define g_pChatCommandWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewChatCommandWindow()
#define g_pCommandWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewCommandWindow()
#define g_pWindowMenu mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewWindowMenu()
#define g_pOption mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewOptionWindow()
#define g_pMuHelperBar mu::ui::window::CNewUISystem::GetInstance()->GetUI_MuHelperBar()
#define g_pHelp mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewHelpWindow()
#define g_pItemExplanation mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewItemExplanationWindow()
#define g_pSetItemExplanation mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewSetItemExplanation()
#define g_pQuickCommand mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewQuickCommandWindow()
#define g_pMoveCommandWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewMoveCommandWindow()
#define g_pDuelWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewDeulWindow()
#define g_pSiegeWarfare mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewSiegeWarfare()
#define g_pItemEnduranceInfo mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewItemEnduranceInfo()
#define g_pBuffStrip mu::ui::window::CNewUISystem::GetInstance()->GetUI_BuffStrip()
#define g_pCursedTempleEnterWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewCursedTempleEnterWindow()
#define g_pCursedTempleWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewCursedTempleWindow()
#define g_pCursedTempleResultWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewCursedTempleResultWindow()
#define g_pCryWolfInterface mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewCryWolfInterface()
#define g_pMasterLevelInterface mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewMasterLevelInterface()
#define g_pGoldBowmanInterface mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewGoldBowman()
#define g_pGoldBowmanLenaInterface mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewGoldBowmanLena()
#define g_pLuckyCoinRegistration mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewLuckyCoinRegistration()
#define g_pExchangeLuckyCoinWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewExchangeLuckyCoin()
#define g_pDuelWatchWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewDuelWatch()
#define g_pDuelWatchMainFrameWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewDuelWatchMainFrame()
#define g_pDuelWatchUserList mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewDuelWatchUserList()
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#define g_pInGameShop mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewInGameShop()
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#define g_pDoppelGangerWindow mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewDoppelGangerWindow()
#define g_pDoppelGangerFrame mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewDoppelGangerFrame()
#define g_pNPCDialogue mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewNPCDialogue()
#define g_pQuestProgress mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewQuestProgress()
#define g_pQuestProgressByEtc mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewQuestProgressByEtc()
#define g_pEmpireGuardianNPC mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewEmpireGuardianNPC()
#define g_pEmpireGuardianTimer mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewEmpireGuardianTimer()
#define g_pNewUIMiniMap mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewUIMiniMap()
#ifdef PBG_MOD_STAMINA_UI
#define g_pNewUIStamina mu::ui::window::CNewUISystem::GetInstance()->GetUI_pNewUIStamina()
#endif //PBG_MOD_STAMINA_UI
#define g_pNewUIGensRanking mu::ui::window::CNewUISystem::GetInstance()->GetUI_NewGensRanking()
#define g_pLuckyItemWnd	mu::ui::window::CNewUISystem::GetInstance()->Get_pNewUILuckyItemWnd()
#define g_pNewUIMuHelper mu::ui::window::CNewUISystem::GetInstance()->Get_pNewUIMuHelper()
#define g_pNewUIMuHelperExt mu::ui::window::CNewUISystem::GetInstance()->Get_pNewUIMuHelperExt()
#define g_pNewUIMuHelperSkillList mu::ui::window::CNewUISystem::GetInstance()->Get_pNewUIMuHelperSkillList()
#endif // _NEWUISYSTEM_H_
