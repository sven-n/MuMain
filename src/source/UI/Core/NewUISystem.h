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
    class CSystem
    {
        CManager* m_pNewUIMng;
        C3DRenderMng* m_pNewUI3DRenderMng;
        CHotKey* m_pNewUIHotKey;
        bool m_mouseInputCaptured = false;

    public:
        ~CSystem();

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

        CManager* GetNewUIManager() const;
        C3DRenderMng* GetNewUI3DRenderMng() const;
        CHotKey* GetNewUIHotKey() const;

        bool IsImpossibleSendMoveInterface();
        void UpdateSendMoveInterface();
        bool IsImpossibleTradeInterface();
        bool IsImpossibleDuelInterface();
        bool IsImpossibleHideInterface(DWORD dwKey);

        static CSystem* GetInstance();

    protected:
        CSystem();		//. ban to create instance

        void HideAllGroupA();
        void HideAllGroupB();
        void HideGroupBeforeOpenInterface();
        void UpdateMuHelperBarVisibilityForLayoutChange(DWORD dwKey);
        void SyncMuHelperBarVisibility();
        bool ShouldHideMuHelperBar();

        /* Interface classes */
    private:
        CChatInputBox* m_pNewChatInputBox;
        CChatLogWindow* m_pNewChatLogWindow;
        CSystemLogWindow* m_pNewSystemLogWindow;
        CSlideWindow* m_pNewSlideWindow;
        CFriendWindow* m_pNewFriendWindow;
        CMainFrameWindow* m_pNewMainFrameWindow;
        CSkillList* m_pNewSkillList;
        CItemMng* m_pNewItemMng;
        CMyInventory* m_pNewMyInventory;
        CInventoryExtension* m_pNewMyInventoryExt;
        CNPCShop* m_pNewNPCShop;
        CPetInfoWindow* m_pNewPetInfoWindow;
        CMixInventory* m_pNewMixInventory;
        CCastleWindow* m_pNewCastleWindow;
        CGuardWindow* m_pNewGuardWindow;
        CGatemanWindow* m_pNewGatemanWindow;
        CGateSwitchWindow* m_pNewGateSwitchWindow;
        CStorageInventory* m_pNewStorageInventory;
        CStorageInventoryExt* m_pNewStorageInventoryExt;
        CGuildMakeWindow* m_pNewGuildMakeWindow;
        CGuildInfoWindow* m_pNewGuildInfoWindow;
        CMyShopInventory* m_pNewMyShopInventory;
        CPurchaseShopInventory* m_pNewPurchaseShopInventory;
        CCharacterInfoWindow* m_pNewCharacterInfoWindow;
        CMyQuestInfoWindow* m_pNewMyQuestInfoWindow;
        CPartyInfoWindow* m_pNewPartyInfoWindow;
        CPartyListWindow* m_pNewPartyListWindow;
        CNPCQuest* m_pNewNPCQuest;
        CEnterBloodCastle* m_pNewEnterBloodCastle;
        CEnterDevilSquare* m_pNewEnterDevilSquare;
        CBloodCastle* m_pNewBloodCastle;
        CTrade* m_pNewTrade;
        CKanturu2ndEnterNpc* m_pNewKanturu2ndEnterNpc;
        CKanturuInfoWindow* m_pNewKanturuInfoWindow;
        CCatapultWindow* m_pNewCatapultWindow;
        CChaosCastleTime* m_pNewChaosCastleTime;
        CBattleSoccerScore* m_pNewBattleSoccerScore;
        CCommandWindow* m_pNewCommandWindow;
        CMuHelperBar* m_pMuHelperBar;
        CWindowMenu* m_pNewWindowMenu;
        COptionWindow* m_pNewOptionWindow;
        CHelpWindow* m_pNewHelpWindow;
        CChatCommandWindow* m_pNewChatCommandWindow;
        CItemExplanationWindow* m_pNewItemExplanationWindow;
        CSetItemExplanation* m_pNewSetItemExplanation;
        CQuickCommandWindow* m_pNewQuickCommandWindow;
        CMoveCommandWindow* m_pNewMoveCommandWindow;
        CDuelWindow* m_pNewDuelWindow;
        CNameWindow* m_pNewNameWindow;
        CSiegeWarfare* m_pNewSiegeWarfare;
        CItemEnduranceInfo* m_pNewItemEnduranceInfo;
        CBuffStrip* m_pBuffStrip;
        CCursedTempleEnter* m_pNewCursedTempleEnterWindow;
        CCursedTempleSystem* m_pNewCursedTempleWindow;
        CCursedTempleResult* m_pNewCursedTempleResultWindow;
        CCryWolf* m_pNewCryWolfInterface;
        CMasterLevel* m_pNewMaster_Level_Interface;
        CGoldBowmanWindow* m_pNewGoldBowman;
        CGoldBowmanLena* m_pNewGoldBowmanLena;
        CRegistrationLuckyCoin* m_pNewLuckyCoinRegistration;
        CExchangeLuckyCoin* m_pNewExchangeLuckyCoinWindow;
        CDuelWatchWindow* m_pNewDuelWatchWindow;
        CDuelWatchMainFrameWindow* m_pNewDuelWatchMainFrameWindow;
        CDuelWatchUserListWindow* m_pNewDuelWatchUserListWindow;
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CInGameShop* m_pNewInGameShop;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CDoppelGangerWindow* m_pNewDoppelGangerWindow;
        CDoppelGangerFrame* m_pNewDoppelGangerFrame;
        CNPCDialogue* m_pNewNPCDialogue;
        CQuestProgress* m_pNewQuestProgress;
        CQuestProgressByEtc* m_pNewQuestProgressByEtc;
        CEmpireGuardianNPC* m_pNewEmpireGuardianNPC;
        CEmpireGuardianTimer* m_pNewEmpireGuardianTimer;
        CMiniMap* m_pNewMiniMap;
        CGensRanking* m_pNewGensRanking;
        CUnitedMarketPlaceWindow* m_pNewUnitedMarketPlaceWindow;
        CLuckyItemWnd* m_pNewUILuckyItemWnd;
        CUIMuHelper* m_pNewUIMuHelper;
        CMuHelperExt* m_pNewUIMuHelperExt;
        CMuHelperSkillList* m_pNewUIMuHelperSkillList;

        bool CreateChatCommandWindow();

    public:
        CChatInputBox* GetUI_NewChatInputBox() const;
        CChatLogWindow* GetUI_NewChatLogWindow() const;
        CSystemLogWindow* GetUI_NewSystemLogWindow() const;
        CSlideWindow* GetUI_NewSlideWindow() const;
        CGuildMakeWindow* GetUI_NewGuildMakeWindow() const;
        CFriendWindow* GetUI_NewFriendWindow() const;
        CMainFrameWindow* GetUI_NewMainFrameWindow() const;
        CSkillList* GetUI_NewSkillList() const;
        CItemMng* GetUI_NewItemMng() const;
        CMyInventory* GetUI_NewMyInventory() const;
        CInventoryExtension* GetUI_NewMyInventoryExt() const;
        CNPCShop* GetUI_NewNpcShop() const;
        CPetInfoWindow* GetUI_NewPetInfoWindow() const;
        CMixInventory* GetUI_NewMixInventory() const;
        CCastleWindow* GetUI_NewCastleWindow() const;
        CGuardWindow* GetUI_NewGuardWindow() const;
        CGatemanWindow* GetUI_NewGatemanWindow() const;
        CGateSwitchWindow* GetUI_NewGateSwitchWindow() const;
        CStorageInventory* GetUI_NewStorageInventory() const;
        CStorageInventoryExt* GetUI_NewStorageInventoryExt() const;
        CGuildInfoWindow* GetUI_NewGuildInfoWindow() const;
        CMyShopInventory* GetUI_NewMyShopInventory() const;
        CPurchaseShopInventory* GetUI_NewPurchaseShopInventory() const;
        CCharacterInfoWindow* GetUI_NewCharacterInfoWindow() const;
        CMyQuestInfoWindow* GetUI_NewMyQuestInfoWindow() const;
        CPartyInfoWindow* GetUI_NewPartyInfoWindow() const;
        CPartyListWindow* GetUI_NewPartyListWindow() const;
        CNPCQuest* GetUI_NewNPCQuest() const;
        CEnterBloodCastle* GetUI_NewEnterBloodCastle() const;
        CEnterDevilSquare* GetUI_NewEnterDevilSquare() const;
        CBloodCastle* GetUI_NewBloodCastle() const;
        CTrade* GetUI_NewTrade() const;
        CKanturu2ndEnterNpc* GetUI_NewKanturu2ndEnterNpc() const;
        CKanturuInfoWindow* GetUI_NewKanturuInfoWindow() const;
        CCatapultWindow* GetUI_NewCatapultWindow() const;
        CChaosCastleTime* GetUI_NewChaosCastleTime() const;
        CBattleSoccerScore* GetUI_NewBattleSoccerScore() const;
        CCommandWindow* GetUI_NewCommandWindow() const;
        CMuHelperBar* GetUI_MuHelperBar() const;
        CWindowMenu* GetUI_NewWindowMenu() const;
        COptionWindow* GetUI_NewOptionWindow() const;
        CHelpWindow* GetUI_NewHelpWindow() const;
        CChatCommandWindow* GetUI_NewChatCommandWindow() const;
        CItemExplanationWindow* GetUI_NewItemExplanationWindow() const;
        CSetItemExplanation* GetUI_NewSetItemExplanation() const;
        CQuickCommandWindow* GetUI_NewQuickCommandWindow() const;
        CMoveCommandWindow* GetUI_NewMoveCommandWindow() const;
        CDuelWindow* GetUI_NewDuelWindow() const;
        CSiegeWarfare* GetUI_NewSiegeWarfare() const;
        CItemEnduranceInfo* GetUI_NewItemEnduranceInfo() const;
        CBuffStrip* GetUI_BuffStrip() const;
        CCursedTempleEnter* GetUI_NewCursedTempleEnterWindow() const;
        CCursedTempleSystem* GetUI_NewCursedTempleWindow() const;
        CCursedTempleResult* GetUI_NewCursedTempleResultWindow() const;
        CCryWolf* GetUI_NewCryWolfInterface() const;
        CMasterLevel* GetUI_NewMasterLevelInterface() const;
        CGoldBowmanWindow* GetUI_pNewGoldBowman() const;
        CGoldBowmanLena* GetUI_pNewGoldBowmanLena() const;
        CRegistrationLuckyCoin* GetUI_pNewLuckyCoinRegistration() const;
        CExchangeLuckyCoin* GetUI_pNewExchangeLuckyCoin() const;
        CDuelWatchWindow* GetUI_pNewDuelWatch() const;
        CDuelWatchMainFrameWindow* GetUI_pNewDuelWatchMainFrame() const;
        CDuelWatchUserListWindow* GetUI_pNewDuelWatchUserList() const;
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CInGameShop* GetUI_pNewInGameShop() const;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
        CDoppelGangerWindow* GetUI_pNewDoppelGangerWindow() const;
        CDoppelGangerFrame* GetUI_pNewDoppelGangerFrame() const;
        CNPCDialogue* GetUI_NewNPCDialogue() const;
        CQuestProgress* GetUI_NewQuestProgress() const;
        CQuestProgressByEtc* GetUI_NewQuestProgressByEtc() const;
        CEmpireGuardianNPC* GetUI_pNewEmpireGuardianNPC() const;
        CEmpireGuardianTimer* GetUI_pNewEmpireGuardianTimer() const;
        CMiniMap* GetUI_pNewUIMiniMap() const;
        CGensRanking* GetUI_NewGensRanking() const;
        CUnitedMarketPlaceWindow* GetUI_pNewUnitedMarketPlaceWindow() const;
        //CUnitedMarketPlaceWindow*	GetUI_pNewUnitedMarketPlaceFrame() const;
        CLuckyItemWnd* Get_pNewUILuckyItemWnd() const;
        CUIMuHelper* Get_pNewUIMuHelper() const;
        CMuHelperExt* Get_pNewUIMuHelperExt() const;
        CMuHelperSkillList* Get_pNewUIMuHelperSkillList() const;
    };
}

#define g_pNewUISystem mu::ui::window::CSystem::GetInstance()
#define g_pNewUIMng mu::ui::window::CSystem::GetInstance()->GetNewUIManager()
#define g_pNewUI3DRenderMng mu::ui::window::CSystem::GetInstance()->GetNewUI3DRenderMng()
#define g_pNewUIHotKey mu::ui::window::CSystem::GetInstance()->GetNewUIHotKey()
#define g_pNewItemMng mu::ui::window::CSystem::GetInstance()->GetUI_NewItemMng()
#define g_pChatInputBox mu::ui::window::CSystem::GetInstance()->GetUI_NewChatInputBox()
#define g_pChatListBox mu::ui::window::CSystem::GetInstance()->GetUI_NewChatLogWindow()
#define g_pSystemLogBox mu::ui::window::CSystem::GetInstance()->GetUI_NewSystemLogWindow()
#define g_pSlideHelpMgr mu::ui::window::CSystem::GetInstance()->GetUI_NewSlideWindow()
#define g_pWindowMgr mu::ui::window::CSystem::GetInstance()->GetUI_NewFriendWindow()
#define g_pMainFrame mu::ui::window::CSystem::GetInstance()->GetUI_NewMainFrameWindow()
#define g_pSkillList mu::ui::window::CSystem::GetInstance()->GetUI_NewSkillList()
#define g_pMyInventory mu::ui::window::CSystem::GetInstance()->GetUI_NewMyInventory()
#define g_pMyInventoryExt mu::ui::window::CSystem::GetInstance()->GetUI_NewMyInventoryExt()
#define g_pNPCShop mu::ui::window::CSystem::GetInstance()->GetUI_NewNpcShop()
#define g_pPetInfoWindow	mu::ui::window::CSystem::GetInstance()->GetUI_NewPetInfoWindow()
#define g_pMixInventory mu::ui::window::CSystem::GetInstance()->GetUI_NewMixInventory()
#define g_pCastleWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewCastleWindow()
#define g_pGuardWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewGuardWindow()
#define g_pGatemanWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewGatemanWindow()
#define g_pGateSwitchWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewGateSwitchWindow()
#define g_pStorageInventory mu::ui::window::CSystem::GetInstance()->GetUI_NewStorageInventory()
#define g_pStorageInventoryExt mu::ui::window::CSystem::GetInstance()->GetUI_NewStorageInventoryExt()
#define g_pGuildMakeWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewGuildMakeWindow()
#define g_pGuildInfoWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewGuildInfoWindow()
#define g_pMyShopInventory mu::ui::window::CSystem::GetInstance()->GetUI_NewMyShopInventory()
#define g_pPurchaseShopInventory mu::ui::window::CSystem::GetInstance()->GetUI_NewPurchaseShopInventory()
#define g_pCharacterInfoWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewCharacterInfoWindow()
#define g_pMyQuestInfoWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewMyQuestInfoWindow()
#define g_pPartyInfoWindow		mu::ui::window::CSystem::GetInstance()->GetUI_NewPartyInfoWindow()
#define g_pPartyListWindow		mu::ui::window::CSystem::GetInstance()->GetUI_NewPartyListWindow()
#define g_pNPCQuest mu::ui::window::CSystem::GetInstance()->GetUI_NewNPCQuest()
#define g_pEnterBloodCastle mu::ui::window::CSystem::GetInstance()->GetUI_NewEnterBloodCastle()
#define g_pEnterDevilSquare mu::ui::window::CSystem::GetInstance()->GetUI_NewEnterDevilSquare()
#define g_pBloodCastle mu::ui::window::CSystem::GetInstance()->GetUI_NewBloodCastle()
#define g_pTrade mu::ui::window::CSystem::GetInstance()->GetUI_NewTrade()
#define g_pKanturu2ndEnterNpc mu::ui::window::CSystem::GetInstance()->GetUI_NewKanturu2ndEnterNpc()
#define g_pKanturuInfoWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewKanturuInfoWindow()
#define g_pCatapultWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewCatapultWindow()
#define g_pChaosCastleTime mu::ui::window::CSystem::GetInstance()->GetUI_NewChaosCastleTime()
#define g_pBattleSoccerScore mu::ui::window::CSystem::GetInstance()->GetUI_NewBattleSoccerScore()
#define g_pChatCommandWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewChatCommandWindow()
#define g_pCommandWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewCommandWindow()
#define g_pWindowMenu mu::ui::window::CSystem::GetInstance()->GetUI_NewWindowMenu()
#define g_pOption mu::ui::window::CSystem::GetInstance()->GetUI_NewOptionWindow()
#define g_pMuHelperBar mu::ui::window::CSystem::GetInstance()->GetUI_MuHelperBar()
#define g_pHelp mu::ui::window::CSystem::GetInstance()->GetUI_NewHelpWindow()
#define g_pItemExplanation mu::ui::window::CSystem::GetInstance()->GetUI_NewItemExplanationWindow()
#define g_pSetItemExplanation mu::ui::window::CSystem::GetInstance()->GetUI_NewSetItemExplanation()
#define g_pQuickCommand mu::ui::window::CSystem::GetInstance()->GetUI_NewQuickCommandWindow()
#define g_pMoveCommandWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewMoveCommandWindow()
#define g_pDuelWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewDeulWindow()
#define g_pSiegeWarfare mu::ui::window::CSystem::GetInstance()->GetUI_NewSiegeWarfare()
#define g_pItemEnduranceInfo mu::ui::window::CSystem::GetInstance()->GetUI_NewItemEnduranceInfo()
#define g_pBuffStrip mu::ui::window::CSystem::GetInstance()->GetUI_BuffStrip()
#define g_pCursedTempleEnterWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewCursedTempleEnterWindow()
#define g_pCursedTempleWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewCursedTempleWindow()
#define g_pCursedTempleResultWindow mu::ui::window::CSystem::GetInstance()->GetUI_NewCursedTempleResultWindow()
#define g_pCryWolfInterface mu::ui::window::CSystem::GetInstance()->GetUI_NewCryWolfInterface()
#define g_pMasterLevelInterface mu::ui::window::CSystem::GetInstance()->GetUI_NewMasterLevelInterface()
#define g_pGoldBowmanInterface mu::ui::window::CSystem::GetInstance()->GetUI_pNewGoldBowman()
#define g_pGoldBowmanLenaInterface mu::ui::window::CSystem::GetInstance()->GetUI_pNewGoldBowmanLena()
#define g_pLuckyCoinRegistration mu::ui::window::CSystem::GetInstance()->GetUI_pNewLuckyCoinRegistration()
#define g_pExchangeLuckyCoinWindow mu::ui::window::CSystem::GetInstance()->GetUI_pNewExchangeLuckyCoin()
#define g_pDuelWatchWindow mu::ui::window::CSystem::GetInstance()->GetUI_pNewDuelWatch()
#define g_pDuelWatchMainFrameWindow mu::ui::window::CSystem::GetInstance()->GetUI_pNewDuelWatchMainFrame()
#define g_pDuelWatchUserList mu::ui::window::CSystem::GetInstance()->GetUI_pNewDuelWatchUserList()
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#define g_pInGameShop mu::ui::window::CSystem::GetInstance()->GetUI_pNewInGameShop()
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
#define g_pDoppelGangerWindow mu::ui::window::CSystem::GetInstance()->GetUI_pNewDoppelGangerWindow()
#define g_pDoppelGangerFrame mu::ui::window::CSystem::GetInstance()->GetUI_pNewDoppelGangerFrame()
#define g_pNPCDialogue mu::ui::window::CSystem::GetInstance()->GetUI_NewNPCDialogue()
#define g_pQuestProgress mu::ui::window::CSystem::GetInstance()->GetUI_NewQuestProgress()
#define g_pQuestProgressByEtc mu::ui::window::CSystem::GetInstance()->GetUI_NewQuestProgressByEtc()
#define g_pEmpireGuardianNPC mu::ui::window::CSystem::GetInstance()->GetUI_pNewEmpireGuardianNPC()
#define g_pEmpireGuardianTimer mu::ui::window::CSystem::GetInstance()->GetUI_pNewEmpireGuardianTimer()
#define g_pNewUIMiniMap mu::ui::window::CSystem::GetInstance()->GetUI_pNewUIMiniMap()
#ifdef PBG_MOD_STAMINA_UI
#define g_pNewUIStamina mu::ui::window::CSystem::GetInstance()->GetUI_pNewUIStamina()
#endif //PBG_MOD_STAMINA_UI
#define g_pNewUIGensRanking mu::ui::window::CSystem::GetInstance()->GetUI_NewGensRanking()
#define g_pLuckyItemWnd	mu::ui::window::CSystem::GetInstance()->Get_pNewUILuckyItemWnd()
#define g_pNewUIMuHelper mu::ui::window::CSystem::GetInstance()->Get_pNewUIMuHelper()
#define g_pNewUIMuHelperExt mu::ui::window::CSystem::GetInstance()->Get_pNewUIMuHelperExt()
#define g_pNewUIMuHelperSkillList mu::ui::window::CSystem::GetInstance()->Get_pNewUIMuHelperSkillList()
#endif // _NEWUISYSTEM_H_
