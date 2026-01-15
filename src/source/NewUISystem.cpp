#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUIMessageBox.h"

#include "PersonalShopTitleImp.h"
#include "MapManager.h"


using namespace SEASON3B;

CNewUISystem::CNewUISystem()
{
    m_pNewUIMng = nullptr;
    m_pNewUIHotKey = nullptr;
    m_pNewChatLogWindow = nullptr;
    m_pNewSystemLogWindow = nullptr;
    m_pNewSlideWindow = nullptr;
    m_pNewGuildMakeWindow = nullptr;
    m_pNewFriendWindow = nullptr;
    m_pNewMainFrameWindow = nullptr;
    m_pNewSkillList = nullptr;
    m_pNewChatInputBox = nullptr;
    m_pNewItemMng = nullptr;
    m_pNewMyInventory = nullptr;
    m_pNewMyInventoryExt = nullptr;
    m_pNewNPCShop = nullptr;
    m_pNewPetInfoWindow = nullptr;
    m_pNewMixInventory = nullptr;
    m_pNewCastleWindow = nullptr;
    m_pNewGuardWindow = nullptr;
    m_pNewGatemanWindow = nullptr;
    m_pNewGateSwitchWindow = nullptr;
    m_pNewStorageInventory = nullptr;
    m_pNewStorageInventoryExt = nullptr;
    m_pNewGuildInfoWindow = nullptr;
    m_pNewMyShopInventory = nullptr;
    m_pNewCharacterInfoWindow = nullptr;
    m_pNewPartyInfoWindow = nullptr;
    m_pNewPartyListWindow = nullptr;
    m_pNewNPCQuest = nullptr;
    m_pNewEnterBloodCastle = nullptr;
    m_pNewEnterDevilSquare = nullptr;
    m_pNewBloodCastle = nullptr;
    m_pNewTrade = nullptr;
    m_pNewKanturu2ndEnterNpc = nullptr;
    m_pNewCatapultWindow = nullptr;
    m_pNewChaosCastleTime = nullptr;
    m_pNewBattleSoccerScore = nullptr;
    m_pNewCommandWindow = nullptr;
    m_pNewWindowMenu = nullptr;
    m_pNewOptionWindow = nullptr;
    m_pNewHeroPositionInfo = nullptr;
    m_pNewHelpWindow = nullptr;
    m_pNewItemExplanationWindow = nullptr;
    m_pNewSetItemExplanation = nullptr;
    m_pNewQuickCommandWindow = nullptr;
    m_pNewMoveCommandWindow = nullptr;
    m_pNewSiegeWarfare = nullptr;
    m_pNewItemEnduranceInfo = nullptr;
    m_pNewBuffWindow = nullptr;
    m_pNewCryWolfInterface = nullptr;
    m_pNewMaster_Level_Interface = nullptr;
    m_pNewCursedTempleResultWindow = nullptr;
    m_pNewCursedTempleWindow = nullptr;
    m_pNewCursedTempleEnterWindow = nullptr;
    m_pNewGoldBowman = nullptr;
    m_pNewGoldBowmanLena = nullptr;
    m_pNewLuckyCoinRegistration = nullptr;
    m_pNewExchangeLuckyCoinWindow = nullptr;
    m_pNewDuelWatchWindow = nullptr;
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    m_pNewInGameShop = nullptr;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
    m_pNewDoppelGangerWindow = nullptr;
    m_pNewDoppelGangerFrame = nullptr;
    m_pNewNPCDialogue = nullptr;
    m_pNewQuestProgress = nullptr;
    m_pNewQuestProgressByEtc = nullptr;
    m_pNewEmpireGuardianNPC = nullptr;
    m_pNewEmpireGuardianTimer = nullptr;
    m_pNewMiniMap = nullptr;
#ifdef PBG_MOD_STAMINA_UI
    m_pNewUIStamina = NULL;
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
    m_pNewGensRanking = nullptr;
#endif //PBG_ADD_GENSRANKING
    m_pNewUnitedMarketPlaceWindow = nullptr;
}

CNewUISystem::~CNewUISystem()
{
    Release();
}

bool CNewUISystem::Create()
{
    m_pNewUIMng = new CNewUIManager;

    m_pNewUI3DRenderMng = new CNewUI3DRenderMng;
    if (false == m_pNewUI3DRenderMng->Create(m_pNewUIMng))
        return false;

    m_pNewChatLogWindow = new CNewUIChatLogWindow;
    if (false == m_pNewChatLogWindow->Create(m_pNewUIMng, 0, 480 - 50 - 47, 6))
        return false;

    m_pNewSystemLogWindow = new CNewUISystemLogWindow;
    if (false == m_pNewSystemLogWindow->Create(m_pNewUIMng, 0, 80))
        return false;

    m_pNewOptionWindow = new CNewUIOptionWindow;
    if (m_pNewOptionWindow->Create(m_pNewUIMng, (640 / 2) - (190 / 2), 70) == false)
    {
        return false;
    }

    m_pNewSlideWindow = new CNewUISlideWindow;
    if (m_pNewSlideWindow->Create(m_pNewUIMng) == false)
    {
        return false;
    }

    if (g_MessageBox->Create(m_pNewUIMng) == false)
        return false;

    return true;
}

void CNewUISystem::Release()
{
    UnloadMainSceneInterface();

    SAFE_DELETE(m_pNewSlideWindow);
    SAFE_DELETE(m_pNewOptionWindow);
    SAFE_DELETE(m_pNewChatLogWindow);
    SAFE_DELETE(m_pNewSystemLogWindow);
    SAFE_DELETE(m_pNewUI3DRenderMng);

    m_pNewUIMng->RemoveAllUIObjs();

    SAFE_DELETE(m_pNewUIMng);
}

bool CNewUISystem::LoadMainSceneInterface()
{
    g_MessageBox->Show(true);
    m_pNewChatLogWindow->Show(true);
    m_pNewSystemLogWindow->Show(true);
    m_pNewSlideWindow->Show(true);

    m_pNewItemMng = new CNewUIItemMng;

    m_pNewChatInputBox = new CNewUIChatInputBox;

    if (false == m_pNewChatInputBox->Create(m_pNewUIMng, m_pNewChatLogWindow, m_pNewSystemLogWindow, 0, 480 - 51 - 47))
    {
        return false;
    }

    SetFocus(g_hWnd);

    m_pNewUIHotKey = new CNewUIHotKey;
    if (false == m_pNewUIHotKey->Create(m_pNewUIMng))
        return false;

    m_pNewMainFrameWindow = new CNewUIMainFrameWindow;
    if (m_pNewMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
        return false;

    m_pNewSkillList = new CNewUISkillList;
    if (m_pNewSkillList->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
        return false;

    m_pNewFriendWindow = new CNewUIFriendWindow;
    if (m_pNewFriendWindow->Create(m_pNewUIMng) == false)
        return false;

    m_pNewMyInventory = new CNewUIMyInventory;
    if (false == m_pNewMyInventory->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640 - 190, 0))
        return false;

    m_pNewMyInventoryExt = new CNewUIInventoryExtension;
    if (false == m_pNewMyInventoryExt->Create(m_pNewUIMng, 640 - 190 * 2, 0))
        return false;

    m_pNewNPCShop = new CNewUINPCShop;
    if (false == m_pNewNPCShop->Create(m_pNewUIMng, CNewUINPCShop::NPCSHOP_POS_X, CNewUINPCShop::NPCSHOP_POS_Y))
        return false;

    m_pNewPetInfoWindow = new CNewUIPetInfoWindow;
    if (false == m_pNewPetInfoWindow->Create(m_pNewUIMng, 640 - (190 * 2), 0))
        return false;

    m_pNewMixInventory = new CNewUIMixInventory;
    if (m_pNewMixInventory->Create(m_pNewUIMng, 260, 0) == false)
        return false;

    m_pNewCastleWindow = new CNewUICastleWindow;
    if (m_pNewCastleWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewGuardWindow = new CNewUIGuardWindow;
    if (m_pNewGuardWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewGatemanWindow = new CNewUIGatemanWindow;
    if (m_pNewGatemanWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewGateSwitchWindow = new CNewUIGateSwitchWindow;
    if (m_pNewGateSwitchWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewStorageInventory = new CNewUIStorageInventory;
    if (m_pNewStorageInventory->Create(m_pNewUIMng, 260, 0) == false)
        return false;

    m_pNewStorageInventoryExt = new CNewUIStorageInventoryExt;
    if (m_pNewStorageInventoryExt->Create(m_pNewUIMng, 260 - 190, 0) == false)
        return false;

    m_pNewGuildInfoWindow = new CNewUIGuildInfoWindow;
    if (m_pNewGuildInfoWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewGuildMakeWindow = new CNewUIGuildMakeWindow;
    if (m_pNewGuildMakeWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    CreatePersonalItemTable();

    m_pNewMyShopInventory = new CNewUIMyShopInventory;
    if (m_pNewMyShopInventory->Create(m_pNewUIMng, 640 - 190 * 2, 0) == false)
        return false;

    m_pNewPurchaseShopInventory = new CNewUIPurchaseShopInventory;
    if (m_pNewPurchaseShopInventory->Create(m_pNewUIMng, 640 - 190 * 2, 0) == false)
        return false;

    m_pNewCharacterInfoWindow = new CNewUICharacterInfoWindow;
    if (m_pNewCharacterInfoWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewMyQuestInfoWindow = new CNewUIMyQuestInfoWindow;
    if (m_pNewMyQuestInfoWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewPartyInfoWindow = new CNewUIPartyInfoWindow;
    if (m_pNewPartyInfoWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewPartyListWindow = new CNewUIPartyListWindow;
    if (m_pNewPartyListWindow->Create(m_pNewUIMng, 640 - 79, 14) == false)
        return false;

    m_pNewNPCQuest = new CNewUINPCQuest;
    if (m_pNewNPCQuest->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640 - 190, 0) == false)
        return false;

    m_pNewEnterBloodCastle = new CNewUIEnterBloodCastle;
    if (m_pNewEnterBloodCastle->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewEnterDevilSquare = new CNewUIEnterDevilSquare;
    if (m_pNewEnterDevilSquare->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewBloodCastle = new CNewUIBloodCastle;
    if (m_pNewBloodCastle->Create(m_pNewUIMng, 640 - 127, 480 - 132) == false)
        return false;

    m_pNewTrade = new CNewUITrade;
    if (m_pNewTrade->Create(m_pNewUIMng, 260, 0) == false)
        return false;

    m_pNewKanturu2ndEnterNpc = new CNewUIKanturu2ndEnterNpc;
    if (m_pNewKanturu2ndEnterNpc->Create(m_pNewUIMng, (640 / 2) - (230 / 2), 20) == false)
    {
        return false;
    }

    m_pNewKanturuInfoWindow = new CNewUIKanturuInfoWindow;
    if (m_pNewKanturuInfoWindow->Create(m_pNewUIMng, 541, 351) == false)
    {
        return false;
    }

    m_pNewChaosCastleTime = new CNewUIChaosCastleTime;
    if (m_pNewChaosCastleTime->Create(m_pNewUIMng, 640 - 127, 480 - 132) == false)
        return false;

    m_pNewBattleSoccerScore = new CNewUIBattleSoccerScore;
    if (m_pNewBattleSoccerScore->Create(m_pNewUIMng, 509, 359) == false)
        return false;

    m_pNewCommandWindow = new CNewUICommandWindow;
    if (m_pNewCommandWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewCatapultWindow = new CNewUICatapultWindow;
    if (m_pNewCatapultWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
    {
        return false;
    }

    m_pNewWindowMenu = new CNewUIWindowMenu;
    if (m_pNewWindowMenu->Create(m_pNewUIMng, 640 - 112, 480 - 171) == false)
    {
        return false;
    }

    m_pNewHeroPositionInfo = new CNewUIHeroPositionInfo;
    if (m_pNewHeroPositionInfo->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewHelpWindow = new CNewUIHelpWindow;
    if (m_pNewHelpWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewItemExplanationWindow = new CNewUIItemExplanationWindow;
    if (m_pNewItemExplanationWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewSetItemExplanation = new CNewUISetItemExplanation;
    if (m_pNewSetItemExplanation->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewQuickCommandWindow = new CNewUIQuickCommandWindow;
    if (m_pNewQuickCommandWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewMoveCommandWindow = new CNewUIMoveCommandWindow;

    if (m_pNewMoveCommandWindow->Create(m_pNewUIMng, 1, 1) == false)
        return false;

    m_pNewDuelWindow = new CNewUIDuelWindow;
    if (m_pNewDuelWindow->Create(m_pNewUIMng, 509, 359) == false)
    {
        return false;
    }

    m_pNewNameWindow = new CNewUINameWindow;
    if (m_pNewNameWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewSiegeWarfare = new CNewUISiegeWarfare;
    if (m_pNewSiegeWarfare->Create(m_pNewUIMng, 486, 234) == false)
        return false;

    m_pNewItemEnduranceInfo = new CNewUIItemEnduranceInfo;
    if (m_pNewItemEnduranceInfo->Create(m_pNewUIMng, 2, 26) == false)
    {
        return false;
    }

    m_pNewBuffWindow = new CNewUIBuffWindow;
    if (m_pNewBuffWindow->Create(m_pNewUIMng, 220, 15) == false)
    {
        return false;
    }

    m_pNewCursedTempleEnterWindow = new CNewUICursedTempleEnter;
    if (m_pNewCursedTempleEnterWindow->Create(m_pNewUIMng, 640 / 2 - 230 / 2, 80) == false)
    {
        return false;
    }
    m_pNewCursedTempleWindow = new CNewUICursedTempleSystem;
    if (m_pNewCursedTempleWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }
    m_pNewCursedTempleResultWindow = new CNewUICursedTempleResult;
    if (m_pNewCursedTempleResultWindow->Create(m_pNewUIMng, 640 / 2 - 230 / 2, 120) == false)
    {
        return false;
    }

    m_pNewCryWolfInterface = new CNewUICryWolf;
    if (m_pNewCryWolfInterface->Create(m_pNewUIMng, 0, 0) == false)
        return false;

    m_pNewMaster_Level_Interface = new CNewUIMasterLevel;
    if (m_pNewMaster_Level_Interface->Create(m_pNewUIMng) == false)
        return false;

    m_pNewMiniMap = new CNewUIMiniMap;
    if (m_pNewMiniMap->Create(m_pNewUIMng, 0, 0) == false)
        return false;

    m_pNewGoldBowman = new CNewUIGoldBowmanWindow;
    if (m_pNewGoldBowman->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewGoldBowmanLena = new CNewUIGoldBowmanLena;
    if (m_pNewGoldBowmanLena->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewLuckyCoinRegistration = new CNewUIRegistrationLuckyCoin;
    if (m_pNewLuckyCoinRegistration->Create(m_pNewUIMng, 640 - (190 * 2), 0) == false)
        return false;

    m_pNewExchangeLuckyCoinWindow = new CNewUIExchangeLuckyCoin;
    if (m_pNewExchangeLuckyCoinWindow->Create(m_pNewUIMng, 640 - (190 * 2), 0) == false)
        return false;

    m_pNewDuelWatchWindow = new CNewUIDuelWatchWindow;
    if (m_pNewDuelWatchWindow->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewDuelWatchMainFrameWindow = new CNewUIDuelWatchMainFrameWindow;
    if (m_pNewDuelWatchMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
        return false;

    m_pNewDuelWatchUserListWindow = new CNewUIDuelWatchUserListWindow;
    if (m_pNewDuelWatchUserListWindow->Create(m_pNewUIMng, 640 - 57, 480 - 51) == false)
        return false;

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    m_pNewInGameShop = new CNewUIInGameShop;
    if (m_pNewInGameShop->Create(m_pNewUIMng, 0, 0) == false)
        return false;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

    m_pNewDoppelGangerWindow = new CNewUIDoppelGangerWindow;
    if (m_pNewDoppelGangerWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640 - 190, 0) == false)
        return false;

    m_pNewDoppelGangerFrame = new CNewUIDoppelGangerFrame;
    if (m_pNewDoppelGangerFrame->Create(m_pNewUIMng, 640 - 227, 480 - 51 - 87) == false)
        return false;

    m_pNewNPCDialogue = new CNewUINPCDialogue;
    if (m_pNewNPCDialogue->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewQuestProgress = new CNewUIQuestProgress;
    if (m_pNewQuestProgress->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewQuestProgressByEtc = new CNewUIQuestProgressByEtc;
    if (m_pNewQuestProgressByEtc->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewEmpireGuardianNPC = new CNewUIEmpireGuardianNPC;
    if (m_pNewEmpireGuardianNPC->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 450, 0) == false)
        return false;

    m_pNewEmpireGuardianTimer = new CNewUIEmpireGuardianTimer;
    if (m_pNewEmpireGuardianTimer->Create(m_pNewUIMng, 507, 342) == false)
        return false;

#ifdef PBG_MOD_STAMINA_UI
    m_pNewUIStamina = new CNewUIStamina;
    if (m_pNewUIStamina->Create(m_pNewUIMng, 640, 480) == false)
        return false;
#endif //PBG_MOD_STAMINA_UI

    m_pNewGensRanking = new CNewUIGensRanking;
    if (m_pNewGensRanking->Create(m_pNewUIMng, 640, 480) == false)
        return false;

    m_pNewUnitedMarketPlaceWindow = new CNewUIUnitedMarketPlaceWindow;
    if (m_pNewUnitedMarketPlaceWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 640 - 190, 0) == false)
        return false;

    m_pNewUILuckyItemWnd = new CNewUILuckyItemWnd;
    if (m_pNewUILuckyItemWnd->Create(m_pNewUIMng, 260, 0) == false)
        return false;

    m_pNewUIMuHelper = new CNewUIMuHelper;
    if (m_pNewUIMuHelper->Create(m_pNewUIMng, 640 - 190, 0) == false)
        return false;

    m_pNewUIMuHelperExt = new CNewUIMuHelperExt;
    if (m_pNewUIMuHelperExt->Create(m_pNewUIMng, 640 - 380, 0) == false)
        return false;

    m_pNewUIMuHelperSkillList = new CNewUIMuHelperSkillList;
    if (m_pNewUIMuHelperSkillList->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
        return false;

    return true;
}

void CNewUISystem::UnloadMainSceneInterface()
{
    if (g_pNewUIMng)
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
    SAFE_DELETE(m_pNewPartyInfoWindow);
    SAFE_DELETE(m_pNewPartyListWindow);
    SAFE_DELETE(m_pNewEnterBloodCastle);
    SAFE_DELETE(m_pNewEnterDevilSquare);
    SAFE_DELETE(m_pNewBloodCastle);
    SAFE_DELETE(m_pNewChaosCastleTime);
    SAFE_DELETE(m_pNewCommandWindow);
    SAFE_DELETE(m_pNewHeroPositionInfo);
    SAFE_DELETE(m_pNewMoveCommandWindow);
    SAFE_DELETE(m_pNewUIHotKey);
    SAFE_DELETE(m_pNewSiegeWarfare);
    SAFE_DELETE(m_pNewItemEnduranceInfo);
    SAFE_DELETE(m_pNewBuffWindow);
    SAFE_DELETE(m_pNewCursedTempleResultWindow);
    SAFE_DELETE(m_pNewCursedTempleWindow);
    SAFE_DELETE(m_pNewCursedTempleEnterWindow);
    SAFE_DELETE(m_pNewCryWolfInterface);
    SAFE_DELETE(m_pNewMaster_Level_Interface);
    SAFE_DELETE(m_pNewGoldBowman);
    SAFE_DELETE(m_pNewGoldBowmanLena);
    SAFE_DELETE(m_pNewLuckyCoinRegistration);
    SAFE_DELETE(m_pNewExchangeLuckyCoinWindow);
    SAFE_DELETE(m_pNewDuelWatchWindow);
    SAFE_DELETE(m_pNewDuelWindow);
    SAFE_DELETE(m_pNewDuelWatchMainFrameWindow);
    SAFE_DELETE(m_pNewDuelWatchUserListWindow);
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    SAFE_DELETE(m_pNewInGameShop);
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
    SAFE_DELETE(m_pNewDoppelGangerWindow);
    SAFE_DELETE(m_pNewDoppelGangerFrame);
    SAFE_DELETE(m_pNewNPCDialogue);
    SAFE_DELETE(m_pNewQuestProgress);
    SAFE_DELETE(m_pNewQuestProgressByEtc);
    SAFE_DELETE(m_pNewEmpireGuardianNPC);
    SAFE_DELETE(m_pNewEmpireGuardianTimer);
    SAFE_DELETE(m_pNewMiniMap);
    SAFE_DELETE(m_pNewItemMng);
#ifdef PBG_MOD_STAMINA_UI
    SAFE_DELETE(m_pNewUIStamina);
#endif //PBG_MOD_STAMINA_UI
    SAFE_DELETE(m_pNewGensRanking);
    SAFE_DELETE(m_pNewUnitedMarketPlaceWindow);
#ifdef LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
    SAFE_DELETE(m_pNewUILuckyItemWnd);
#endif // LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE

    ReleasePersonalItemTable();
}

bool CNewUISystem::IsVisible(DWORD dwKey)
{
    if (m_pNewUIMng)
    {
        return m_pNewUIMng->IsInterfaceVisible(dwKey);
    }

    return false;
}

//bool SortUiObj(const INewUIBase& lhs, const INewUIBase& rhs)
//{
//	return lhs.GetDisplayOrder() > rhs.GetDisplayOrder();
//}

void CNewUISystem::Show(DWORD dwKey)
{
#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP
    if (g_pInGameShop->IsInGameShop())
        return;
#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP
    if (!m_pNewUIMng)
    {
        return;
    }

    /*
    std::list<INewUIBase*> visiblePages = {};

    for (int i = INTERFACE_LIST::INTERFACE_BEGIN; i < INTERFACE_LIST::INTERFACE_END; i++)
    {
        auto const uiObj = m_pNewUIMng->FindUIObj(i);
        if (uiObj->IsVisible() && uiObj->IsRightSideMenu())
        {
            visiblePages.push_back(uiObj);
        }
    }

    visiblePages.sort(SortUiObj);
    // TODO: Close all above the margin.
    */
    // TODO: Refactor this whole method. How would be a fixed priority order
    // for each window. And a maximum of open windows, depending on resolution

    if (dwKey == INTERFACE_FRIEND)
    {
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_FRIEND, true);

        m_pNewFriendWindow->OpenMainWnd(640 - 250, 432 - 173);
    }
    else if (dwKey == INTERFACE_INVENTORY)
    {
        HideGroupBeforeOpenInterface();

        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);

        if (IsVisible(INTERFACE_CHARACTER))
        {
            g_pMyInventory->SetPos(640 - 190 * 2, 0);
        }
        if (IsVisible(INTERFACE_MYQUEST))
        {
            Hide(INTERFACE_MYQUEST);
        }
        g_pMyInventory->OpenningProcess();
    }
    else if (dwKey == INTERFACE_INVENTORY_EXT)
    {
        if (IsVisible(INTERFACE_STORAGE_EXT))
        {
            Hide(INTERFACE_STORAGE_EXT);
        }

        if (IsVisible(INTERFACE_STORAGE))
        {
            g_pStorageInventory->SetPos(640 - 190 * 3, 0);
            Hide(INTERFACE_HERO_POSITION_INFO);
        }

        if (IsVisible(INTERFACE_MYSHOP_INVENTORY))
        {
            g_pMyShopInventory->SetPos(640 - 190 * 3, 0);
            Hide(INTERFACE_HERO_POSITION_INFO);
            if (IsVisible(INTERFACE_MYQUEST))
            {
                Hide(INTERFACE_MYQUEST);
            }

            if (IsVisible(INTERFACE_CHARACTER))
            {
                Hide(INTERFACE_CHARACTER);
            }
        }
        if (IsVisible(INTERFACE_MYQUEST))
        {
            Hide(INTERFACE_MYQUEST);
        }
        if (IsVisible(INTERFACE_CHARACTER))
        {
            Hide(INTERFACE_CHARACTER);
        }
        if (IsVisible(INTERFACE_NPCSHOP))
        {
            g_pNPCShop->SetPos(640 - 190 * 3, 0);
            Hide(INTERFACE_HERO_POSITION_INFO);
        }
        if (IsVisible(INTERFACE_MIXINVENTORY))
        {
            g_pMixInventory->SetPos(640 - 190 * 3, 0);
            Hide(INTERFACE_HERO_POSITION_INFO);
        }
        if (IsVisible(INTERFACE_TRADE))
        {
            g_pTrade->SetPos(640 - 190 * 3, 0);
            Hide(INTERFACE_HERO_POSITION_INFO);
        }
    }
    else if (dwKey == INTERFACE_CHARACTER)
    {
        HideGroupBeforeOpenInterface();

        g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);

        if (IsVisible(INTERFACE_INVENTORY))
        {
            g_pMyInventory->SetPos(640 - 190 * 2, 0);
            if (IsVisible(INTERFACE_INVENTORY_EXT))
            {
                g_pMyInventory->SetPos(640 - 190 * 3, 0);
                Hide(INTERFACE_HERO_POSITION_INFO);
            }
        }
        else if (IsVisible(INTERFACE_MYQUEST))
        {
            g_pMyQuestInfoWindow->SetPos(640 - 190 * 2, 0);
        }
        g_pCharacterInfoWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_PET)
    {
        if (IsVisible(INTERFACE_INVENTORY))
        {
            Hide(INTERFACE_INVENTORY);
        }
        if (IsVisible(INTERFACE_MYQUEST))
        {
            Hide(INTERFACE_MYQUEST);
        }

        HideGroupBeforeOpenInterface();

        m_pNewUIMng->ShowInterface(INTERFACE_CHARACTER);
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);
        m_pNewPetInfoWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_MYQUEST)
    {
        HideGroupBeforeOpenInterface();

        if (IsVisible(INTERFACE_CHARACTER))
        {
            g_pMyQuestInfoWindow->SetPos(640 - 190 * 2, 0);
        }
        if (IsVisible(INTERFACE_INVENTORY))
        {
            Hide(INTERFACE_INVENTORY);
        }
        if (IsVisible(INTERFACE_PET))
        {
            Hide(INTERFACE_PET);
        }
        g_pMyQuestInfoWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_MIXINVENTORY)
    {
        HideAllGroupA();
        g_pMixInventory->OpeningProcess();
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY);
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
    }
    else if (dwKey == INTERFACE_NPCSHOP)
    {
        HideAllGroupA();
        g_pNPCShop->OpenningProcess();
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY);
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
    }
    else if (dwKey == INTERFACE_STORAGE)
    {
        const bool isExtendedInventoryOpen = IsVisible(INTERFACE_INVENTORY_EXT);
        HideAllGroupA();
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY);
        if (isExtendedInventoryOpen)
        {
            Show(INTERFACE_INVENTORY_EXT);
            g_pStorageInventory->SetPos(640 - 190 * 3, 0);
            Hide(INTERFACE_HERO_POSITION_INFO);
        }
        else
        {
            g_pStorageInventory->SetPos(640 - 190 * 2, 0);
            Show(INTERFACE_HERO_POSITION_INFO);
        }

        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
    }
    else if (dwKey == INTERFACE_STORAGE_EXT)
    {
        if (IsVisible(INTERFACE_INVENTORY_EXT))
        {
            m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY_EXT, false);
        }

        Hide(INTERFACE_HERO_POSITION_INFO);
        g_pStorageInventory->SetPos(640 - 190 * 2, 0);
        g_pStorageInventoryExt->SetPos(640 - 190 * 3, 0);

        m_pNewUIMng->ShowInterface(INTERFACE_STORAGE_EXT);
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
    }
    else if (dwKey == INTERFACE_MYSHOP_INVENTORY)
    {
        const bool isExtendedInventoryOpen = IsVisible(INTERFACE_INVENTORY_EXT);
        HideAllGroupA();
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY);
        if (isExtendedInventoryOpen)
        {
            Show(INTERFACE_INVENTORY_EXT);
            g_pMyShopInventory->SetPos(640 - 190 * 3, 0);
            Hide(INTERFACE_HERO_POSITION_INFO);
        }
        else
        {
            g_pMyShopInventory->SetPos(640 - 190 * 2, 0);
            Show(INTERFACE_HERO_POSITION_INFO);
        }

        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
        if (gMapManager.IsCursedTemple() == true)
        {
            g_pMyShopInventory->OpenButtonLock();
        }
        else if (!g_pMyShopInventory->IsEnablePersonalShop())
        {
            g_pMyShopInventory->OpenButtonUnLock();
        }
    }
    else if (dwKey == INTERFACE_PURCHASESHOP_INVENTORY)
    {
        HideAllGroupA();
    }
    else if (dwKey == INTERFACE_PARTY)
    {
        HideAllGroupA();
        m_pNewPartyInfoWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_NPCQUEST)
    {
        HideAllGroupA();
        g_pNPCQuest->ProcessOpening();
    }
    else if (dwKey == INTERFACE_TRADE)
    {
        HideAllGroupA();
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY);
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
    }
    else if (dwKey == INTERFACE_BLOODCASTLE)
    {
        HideAllGroupA();
        g_pEnterBloodCastle->OpenningProcess();
    }
    else if (dwKey == INTERFACE_DEVILSQUARE)
    {
        HideAllGroupA();
        g_pEnterDevilSquare->OpenningProcess();
    }
    else if (dwKey == INTERFACE_CATAPULT)
    {
        HideAllGroupA();
        g_pCatapultWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_COMMAND)
    {
        HideAllGroupA();
        m_pNewCommandWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_GUILDINFO)
    {
        HideAllGroupA();
        g_pGuildInfoWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_WINDOW_MENU)
    {
        g_pWindowMenu->OpenningProcess();
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, true);
    }
    else if (dwKey == INTERFACE_SENATUS)
    {
        HideAllGroupA();
        g_pCastleWindow->OpeningProcess();
    }
    else if (dwKey == INTERFACE_GUARDSMAN)
    {
        HideAllGroupA();
        g_pGuardWindow->OpeningProcess();
    }
    else if (dwKey == INTERFACE_GATEKEEPER)
    {
        HideAllGroupA();
        g_pGatemanWindow->OpeningProcess();
    }
    else if (dwKey == INTERFACE_GATESWITCH)
    {
        HideAllGroupA();
        g_pGateSwitchWindow->OpeningProcess();
    }
    else if (dwKey == INTERFACE_NPCGUILDMASTER)
    {
        HideAllGroupA();
    }
    else if (dwKey == INTERFACE_MASTER_LEVEL)
    {
        HideAllGroupA();
    }
    else if (dwKey == INTERFACE_KANTURU2ND_ENTERNPC)
    {
        HideAllGroupB();
    }
    else if (dwKey == INTERFACE_HERO_POSITION_INFO)
    {
        m_pNewHeroPositionInfo->OpenningProcess();
    }
    else if (dwKey == INTERFACE_CHAOSCASTLE_TIME)
    {
        if (IsVisible(INTERFACE_CHATINPUTBOX))
        {
            Hide(INTERFACE_CHATINPUTBOX);
        }
        m_pNewChaosCastleTime->OpenningProcess();
    }
    else if (dwKey == INTERFACE_BLOODCASTLE_TIME)
    {
        g_pBloodCastle->OpenningProcess();
    }
    else if (dwKey == INTERFACE_OPTION)
    {
        g_pOption->OpenningProcess();
    }
    else if (dwKey == INTERFACE_HELP)
    {
        Hide(INTERFACE_MOVEMAP);
        Hide(INTERFACE_ITEM_EXPLANATION);
        Hide(INTERFACE_SETITEM_EXPLANATION);
        g_pHelp->OpenningProcess();
    }
    else if (dwKey == INTERFACE_ITEM_EXPLANATION)
    {
        Hide(INTERFACE_MOVEMAP);
        Hide(INTERFACE_HELP);
        Hide(INTERFACE_SETITEM_EXPLANATION);
        g_pItemExplanation->OpenningProcess();
    }
    else if (dwKey == INTERFACE_SETITEM_EXPLANATION)
    {
        Hide(INTERFACE_MOVEMAP);
        Hide(INTERFACE_HELP);
        Hide(INTERFACE_ITEM_EXPLANATION);
        g_pSetItemExplanation->OpenningProcess();
    }
    else if (dwKey == INTERFACE_QUICK_COMMAND)
    {
        g_pQuickCommand->OpenningProcess();
    }
    else if (dwKey == INTERFACE_MOVEMAP)
    {
        Hide(INTERFACE_HELP);
        Hide(INTERFACE_ITEM_EXPLANATION);
        Hide(INTERFACE_SETITEM_EXPLANATION);
        m_pNewMoveCommandWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_CHATINPUTBOX)
    {
        m_pNewChatInputBox->OpenningProcess();
    }
    else if (dwKey == INTERFACE_SIEGEWARFARE)
    {
        m_pNewSiegeWarfare->OpenningProcess();
    }
    else if (dwKey == INTERFACE_ITEM_ENDURANCE_INFO)
    {
        m_pNewItemEnduranceInfo->OpenningProcess();
    }
    else if (dwKey == INTERFACE_BUFF_WINDOW)
    {
        m_pNewBuffWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_CRYWOLF)
    {
        m_pNewCryWolfInterface->OpenningProcess();
    }
    else if (dwKey == INTERFACE_GOLD_BOWMAN)
    {
        m_pNewGoldBowman->OpeningProcess();
    }
    else if (dwKey == INTERFACE_GOLD_BOWMAN_LENA)
    {
        m_pNewGoldBowmanLena->OpeningProcess();
    }
    else if (dwKey == INTERFACE_LUCKYCOIN_REGISTRATION)
    {
        HideAllGroupA();
        g_pLuckyCoinRegistration->OpeningProcess();
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY);
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
    }
    else if (dwKey == INTERFACE_EXCHANGE_LUCKYCOIN)
    {
        HideAllGroupA();
        g_pExchangeLuckyCoinWindow->OpenningProcess();
    }
    else if (dwKey == INTERFACE_DUELWATCH)
    {
        m_pNewDuelWatchWindow->OpeningProcess();
    }
    else if (dwKey == INTERFACE_DUELWATCH_MAINFRAME)
    {
        m_pNewDuelWatchMainFrameWindow->OpeningProcess();
    }
    else if (dwKey == INTERFACE_DUELWATCH_USERLIST)
    {
        m_pNewDuelWatchUserListWindow->OpeningProcess();
    }
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    else if (dwKey == INTERFACE_INGAMESHOP)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt CallStack - CNewUISystem.Show()\r\n");
        HideAll();
        g_pInGameShop->OpeningProcess();
#ifndef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    }
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
    else if (dwKey == INTERFACE_DOPPELGANGER_NPC)
    {
        m_pNewDoppelGangerWindow->OpeningProcess();
    }
    else if (dwKey == INTERFACE_DOPPELGANGER_FRAME)
    {
        m_pNewDoppelGangerFrame->OpenningProcess();
    }
    else if (dwKey == INTERFACE_NPC_DIALOGUE)
    {
        HideAllGroupA();
        g_pNPCDialogue->ProcessOpening();
    }
    else if (dwKey == INTERFACE_QUEST_PROGRESS)
    {
        HideAllGroupA();
        g_pQuestProgress->ProcessOpening();
    }
    else if (dwKey == INTERFACE_QUEST_PROGRESS_ETC)
    {
        if (IsVisible(INTERFACE_INVENTORY))
            Hide(INTERFACE_INVENTORY);
        if (IsVisible(INTERFACE_MYQUEST))
        {
            Hide(INTERFACE_MYQUEST);
            g_pQuestProgressByEtc->SetPos(640 - 190, 0);
        }
        if (IsVisible(INTERFACE_CHARACTER))
            g_pQuestProgressByEtc->SetPos(640 - 190 * 2, 0);
        g_pQuestProgressByEtc->ProcessOpening();
    }
    else if (dwKey == INTERFACE_EMPIREGUARDIAN_NPC)
    {
        m_pNewEmpireGuardianNPC->OpenningProcess();
    }
    else if (dwKey == INTERFACE_EMPIREGUARDIAN_TIMER)
    {
        m_pNewEmpireGuardianTimer->OpenningProcess();
    }
    else if (dwKey == INTERFACE_MINI_MAP)
    {
        m_pNewMiniMap->OpenningProcess();
    }
    else if (dwKey == INTERFACE_GENSRANKING)
    {
        HideAllGroupA();
        g_pNewUIGensRanking->OpenningProcess();
        g_pNewUIGensRanking->SetPos(640 - 190, 0);
    }
    else if (dwKey == INTERFACE_UNITEDMARKETPLACE_NPC_JULIA)
    {
        m_pNewUnitedMarketPlaceWindow->OpeningProcess();
    }
    else if (dwKey == SEASON3B::INTERFACE_LUCKYITEMWND)
    {
        HideAllGroupA();
        g_pLuckyItemWnd->OpeningProcess();
        m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY);
    }
    else if (dwKey == INTERFACE_MUHELPER)
    {
        HideAllGroupA();
    }

    m_pNewUIMng->ShowInterface(dwKey);

    int iScreenWidth = GetScreenWidth();
    m_pNewItemEnduranceInfo->SetPos(iScreenWidth);
    m_pNewBuffWindow->SetPos(iScreenWidth);
    m_pNewPartyListWindow->SetPos(iScreenWidth);
}

void CNewUISystem::Hide(DWORD dwKey)
{
    if (!m_pNewUIMng)
    {
        return;
    }

    if (dwKey == INTERFACE_FRIEND)
    {
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_FRIEND, false);
        m_pNewFriendWindow->HideAllWindow(TRUE, TRUE);
    }
    else if (dwKey == INTERFACE_CHARACTER)
    {
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, false);
        if (IsVisible(INTERFACE_MYQUEST))
        {
            g_pMyQuestInfoWindow->SetPos(640 - 190, 0);
        }
        if (IsVisible((INTERFACE_INVENTORY)))
        {
            g_pMyInventory->SetPos(640 - 190, 0);
        }
        if (IsVisible((INTERFACE_PET)))
        {
            Hide(INTERFACE_PET);
        }
        if (IsVisible((INTERFACE_QUEST_PROGRESS_ETC)))
        {
            g_pQuestProgressByEtc->SetPos(640 - 190, 0);
        }
    }
    else if (dwKey == INTERFACE_INVENTORY_EXT)
    {
        constexpr auto secondColumnX = 640 - 190 * 2;
        if (IsVisible(INTERFACE_MYSHOP_INVENTORY))
        {
            g_pMyShopInventory->SetPos(secondColumnX, 0);
        }

        if (IsVisible(INTERFACE_TRADE))
        {
            g_pTrade->SetPos(secondColumnX, 0);
        }

        if (IsVisible(INTERFACE_STORAGE))
        {
            g_pStorageInventory->SetPos(secondColumnX, 0);
        }

        if (IsVisible(INTERFACE_NPCSHOP))
        {
            g_pNPCShop->SetPos(secondColumnX, 0);
        }

        if (IsVisible(INTERFACE_MIXINVENTORY))
        {
            g_pMixInventory->SetPos(secondColumnX, 0);
        }

        Show(INTERFACE_HERO_POSITION_INFO);
    }
    else if (dwKey == INTERFACE_INVENTORY)
    {
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);

        if (IsVisible(INTERFACE_INVENTORY_EXT))
        {
            m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY_EXT, false);
        }

        if (IsVisible(INTERFACE_MIXINVENTORY))
        {
            if (g_pMixInventory->ClosingProcess() == false)
                return;
            m_pNewUIMng->ShowInterface(INTERFACE_MIXINVENTORY, false);
        }
        if (IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND))
        {
            if (g_pLuckyItemWnd->ClosingProcess() == false)
                return;
            m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_LUCKYITEMWND, false);
        }
        if (IsVisible(INTERFACE_NPCSHOP))
        {
            g_pNPCShop->ClosingProcess();
            m_pNewUIMng->ShowInterface(INTERFACE_NPCSHOP, false);
        }
        if (IsVisible(INTERFACE_MYSHOP_INVENTORY))
        {
            m_pNewUIMng->ShowInterface(INTERFACE_MYSHOP_INVENTORY, false);
        }
        if (IsVisible(INTERFACE_PURCHASESHOP_INVENTORY))
        {
            g_pPurchaseShopInventory->ClosingProcess();
            m_pNewUIMng->ShowInterface(INTERFACE_PURCHASESHOP_INVENTORY, false);
        }
        if (IsVisible(INTERFACE_STORAGE))
        {
            g_pStorageInventoryExt->ProcessClosing();
            if (!g_pStorageInventory->ProcessClosing())
                return;

            m_pNewUIMng->ShowInterface(INTERFACE_STORAGE_EXT, false);
            m_pNewUIMng->ShowInterface(INTERFACE_STORAGE, false);
        }
        if (IsVisible(INTERFACE_TRADE))
        {
            g_pTrade->ProcessCloseBtn();
            m_pNewUIMng->ShowInterface(INTERFACE_TRADE, false);
        }

        if (IsVisible(INTERFACE_LUCKYCOIN_REGISTRATION))
        {
            m_pNewLuckyCoinRegistration->ClosingProcess();
            m_pNewUIMng->ShowInterface(INTERFACE_LUCKYCOIN_REGISTRATION, false);
        }
        if (IsVisible(INTERFACE_EXCHANGE_LUCKYCOIN))
        {
            m_pNewExchangeLuckyCoinWindow->ClosingProcess();
            m_pNewUIMng->ShowInterface(INTERFACE_EXCHANGE_LUCKYCOIN, false);
        }

        if (IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND))
        {
            m_pNewUILuckyItemWnd->ClosingProcess();
            m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_LUCKYITEMWND, false);
        }

        g_pMyInventory->SetPos(640 - 190, 0);
        g_pMyInventory->ClosingProcess();
    }
    else if (dwKey == INTERFACE_MIXINVENTORY)
    {
        if (g_pMixInventory->ClosingProcess() == false)
        {
            return;
        }
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY, false);
        Show(INTERFACE_HERO_POSITION_INFO);
    }
    else if (dwKey == INTERFACE_NPCSHOP)
    {
        g_pNPCShop->ClosingProcess();
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY, false);
    }
    else if (dwKey == INTERFACE_MYSHOP_INVENTORY
        || dwKey == INTERFACE_PURCHASESHOP_INVENTORY)
    {
        if (dwKey == INTERFACE_MYSHOP_INVENTORY)
        {
            g_pMyShopInventory->ClosingProcess();
        }
        else if (dwKey == INTERFACE_PURCHASESHOP_INVENTORY)
        {
            g_pPurchaseShopInventory->ClosingProcess();
        }
        g_pMyInventory->SetPos(640 - 190, 0);
        Show(INTERFACE_HERO_POSITION_INFO);
    }
    else if (dwKey == INTERFACE_STORAGE)
    {
        g_pStorageInventoryExt->ProcessClosing();
        if (!g_pStorageInventory->ProcessClosing())
            return;
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY_EXT, false);
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY, false);
        Show(INTERFACE_HERO_POSITION_INFO);
    }
    else if (dwKey == INTERFACE_STORAGE_EXT)
    {
        Show(INTERFACE_HERO_POSITION_INFO);
    }
    else if (dwKey == INTERFACE_PET)
    {
        m_pNewPetInfoWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_PARTY)
    {
        m_pNewPartyInfoWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_MYQUEST)
    {
        m_pNewMyQuestInfoWindow->ClosingProcess();

        m_pNewMyQuestInfoWindow->SetPos(640 - 190, 0);
    }
    else if (dwKey == INTERFACE_SENATUS)
    {
        m_pNewCastleWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_GUARDSMAN)
    {
        m_pNewGuardWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_GATEKEEPER)
    {
        m_pNewGatemanWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_GATESWITCH)
    {
        m_pNewGateSwitchWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_NPCQUEST)
    {
        m_pNewNPCQuest->ProcessClosing();
    }
    else if (dwKey == INTERFACE_BLOODCASTLE)
    {
        g_pEnterBloodCastle->ClosingProcess();
    }
    else if (dwKey == INTERFACE_DEVILSQUARE)
    {
        g_pEnterDevilSquare->ClosingProcess();
    }
    else if (dwKey == INTERFACE_BLOODCASTLE_TIME)
    {
        g_pBloodCastle->ClosingProcess();
    }
    else if (dwKey == INTERFACE_TRADE)
    {
        g_pTrade->ProcessClosing();
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY, false);
        Show(INTERFACE_HERO_POSITION_INFO);
    }
    else if (dwKey == INTERFACE_CATAPULT)
    {
        g_pCatapultWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_CHAOSCASTLE_TIME)
    {
        m_pNewChaosCastleTime->ClosingProcess();
    }
    else if (dwKey == INTERFACE_COMMAND)
    {
        m_pNewCommandWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_WINDOW_MENU)
    {
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, false);
    }
    else if (dwKey == INTERFACE_OPTION)
    {
    }
    else if (dwKey == INTERFACE_HERO_POSITION_INFO)
    {
        m_pNewHeroPositionInfo->ClosingProcess();
    }
    else if (dwKey == INTERFACE_HELP)
    {
        g_pHelp->ClosingProcess();
    }
    else if (dwKey == INTERFACE_ITEM_EXPLANATION)
    {
        g_pItemExplanation->ClosingProcess();
    }
    else if (dwKey == INTERFACE_SETITEM_EXPLANATION)
    {
        g_pSetItemExplanation->ClosingProcess();
    }
    else if (dwKey == INTERFACE_QUICK_COMMAND)
    {
        g_pQuickCommand->ClosingProcess();
    }
    else if (dwKey == INTERFACE_MOVEMAP)
    {
        m_pNewCommandWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_CHATINPUTBOX)
    {
        m_pNewChatInputBox->ClosingProcess();
    }
    else if (dwKey == INTERFACE_GUILDINFO)
    {
        m_pNewGuildInfoWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_NPCGUILDMASTER)
    {
        m_pNewGuildMakeWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_SIEGEWARFARE)
    {
        m_pNewSiegeWarfare->ClosingProcess();
    }
    else if (dwKey == INTERFACE_ITEM_ENDURANCE_INFO)
    {
        m_pNewItemEnduranceInfo->ClosingProcess();
    }
    else if (dwKey == INTERFACE_BUFF_WINDOW)
    {
        m_pNewBuffWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_CURSEDTEMPLE_RESULT)
    {
        m_pNewCursedTempleResultWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_CRYWOLF)
    {
        m_pNewCryWolfInterface->ClosingProcess();
    }
    else if (dwKey == INTERFACE_GOLD_BOWMAN)
    {
        m_pNewGoldBowman->ClosingProcess();
    }
    else if (dwKey == INTERFACE_GOLD_BOWMAN_LENA)
    {
        m_pNewGoldBowmanLena->ClosingProcess();
    }
    else if (dwKey == INTERFACE_LUCKYCOIN_REGISTRATION)
    {
        m_pNewLuckyCoinRegistration->ClosingProcess();

        if (IsVisible(INTERFACE_INVENTORY))
        {
            m_pNewMyInventory->ClosingProcess();
            m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY, false);
        }
    }
    else if (dwKey == INTERFACE_EXCHANGE_LUCKYCOIN)
    {
        m_pNewExchangeLuckyCoinWindow->ClosingProcess();

        if (IsVisible(INTERFACE_INVENTORY))
        {
            m_pNewMyInventory->ClosingProcess();
            m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY, false);
        }
    }
    else if (dwKey == INTERFACE_DUELWATCH)
    {
        m_pNewDuelWatchWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_DUELWATCH_MAINFRAME)
    {
        m_pNewDuelWatchMainFrameWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_DUELWATCH_USERLIST)
    {
        m_pNewDuelWatchUserListWindow->ClosingProcess();
    }
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    else if (dwKey == INTERFACE_INGAMESHOP)
    {
        g_pInGameShop->ClosingProcess();
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, false);
    }
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
    else if (dwKey == INTERFACE_DOPPELGANGER_NPC)
    {
        m_pNewDoppelGangerWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_DOPPELGANGER_FRAME)
    {
        m_pNewDoppelGangerFrame->ClosingProcess();
    }
    else if (dwKey == INTERFACE_NPC_DIALOGUE)
    {
        m_pNewNPCDialogue->ProcessClosing();
    }
    else if (dwKey == INTERFACE_QUEST_PROGRESS)
    {
        m_pNewQuestProgress->ProcessClosing();
    }
    else if (dwKey == INTERFACE_QUEST_PROGRESS_ETC)
    {
        m_pNewQuestProgressByEtc->ProcessClosing();
    }
    else if (dwKey == INTERFACE_EMPIREGUARDIAN_NPC)
    {
        m_pNewEmpireGuardianNPC->ClosingProcess();
    }
    else if (dwKey == INTERFACE_EMPIREGUARDIAN_TIMER)
    {
        m_pNewEmpireGuardianTimer->ClosingProcess();
    }
    else if (dwKey == INTERFACE_MINI_MAP)
    {
        m_pNewMiniMap->ClosingProcess();
    }
    else if (dwKey == INTERFACE_GENSRANKING)
    {
        g_pNewUIGensRanking->ClosingProcess();
    }
    else if (dwKey == INTERFACE_UNITEDMARKETPLACE_NPC_JULIA)
    {
        m_pNewUnitedMarketPlaceWindow->ClosingProcess();
    }
    else if (dwKey == SEASON3B::INTERFACE_LUCKYITEMWND)
    {
        if (g_pLuckyItemWnd->ClosingProcess() == false)
            return;
        if (IsVisible(SEASON3B::INTERFACE_INVENTORY))
        {
            m_pNewMyInventory->ClosingProcess();
            m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_INVENTORY, false);
        }
    }
    else if (dwKey == INTERFACE_MUHELPER)
    {
        m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_MUHELPER_SKILL_LIST, false);
        m_pNewUIMng->ShowInterface(SEASON3B::INTERFACE_MUHELPER_EXT, false);
    }

    m_pNewUIMng->ShowInterface(dwKey, false);

    int iScreenWidth = GetScreenWidth();
    m_pNewItemEnduranceInfo->SetPos(iScreenWidth);
    m_pNewBuffWindow->SetPos(iScreenWidth);
    m_pNewPartyListWindow->SetPos(iScreenWidth);
}

void CNewUISystem::Toggle(DWORD dwKey)
{
    IsVisible(dwKey) ? Hide(dwKey) : Show(dwKey);
}

void CNewUISystem::HideAll()
{
    if (m_pNewUIMng)
    {
        for (int i = INTERFACE_BEGIN + 1; i < INTERFACE_END; i++)
        {
            if (IsImpossibleHideInterface(i) == false)
            {
                if (IsVisible(i) == true)
                {
                    Hide(i);
                }
            }
        }
    }
}

void CNewUISystem::HideAllGroupA()
{
    Hide(INTERFACE_INVENTORY);
    Hide(INTERFACE_CHARACTER);

    DWORD dwGroupA[] =
    {
        //SEASON3B::INTERFACE_INVENTORY,
        //SEASON3B::INTERFACE_CHARACTER,
        //SEASON3B::INTERFACE_WINDOW_MENU,
        INTERFACE_MUHELPER,
        INTERFACE_MUHELPER_EXT,
        INTERFACE_MUHELPER_SKILL_LIST,
        INTERFACE_MIXINVENTORY,
        INTERFACE_STORAGE,
        INTERFACE_NPCSHOP,
        INTERFACE_MYSHOP_INVENTORY,
        INTERFACE_PURCHASESHOP_INVENTORY,
        INTERFACE_PET,
        INTERFACE_MYQUEST,
        INTERFACE_NPCQUEST,
        INTERFACE_PARTY,
        INTERFACE_SENATUS,
        INTERFACE_GUARDSMAN,
        INTERFACE_COMMAND,
        INTERFACE_GUILDINFO,
        INTERFACE_KANTURU2ND_ENTERNPC,
        INTERFACE_DUELWATCH,
        INTERFACE_DOPPELGANGER_NPC,
        //SEASON3B::INTERFACE_HELP,
        //SEASON3B::INTERFACE_ITEM_EXPLANATION,
        //SEASON3B::INTERFACE_SETITEM_EXPLANATION,
        INTERFACE_GOLD_BOWMAN,
        INTERFACE_GOLD_BOWMAN_LENA,
        INTERFACE_NPC_DIALOGUE,
        INTERFACE_QUEST_PROGRESS,
        INTERFACE_QUEST_PROGRESS_ETC,
        INTERFACE_EMPIREGUARDIAN_NPC,
#ifdef PBG_MOD_STAMINA_UI
        SEASON3B::INTERFACE_STAMINA_GAUGE,
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
        INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING
        INTERFACE_UNITEDMARKETPLACE_NPC_JULIA,

        SEASON3B::INTERFACE_LUCKYITEMWND,

        0,
    };

    if (m_pNewUIMng)
    {
        for (int i = 0; dwGroupA[i] != 0; i++)
        {
            m_pNewUIMng->ShowInterface(dwGroupA[i], false);
        }
    }
}

void CNewUISystem::HideAllGroupB()
{
    Hide(INTERFACE_FRIEND);
    Hide(INTERFACE_INVENTORY);
    Hide(INTERFACE_CHARACTER);

    DWORD dwGroupB[] =
    {
        //SEASON3B::INTERFACE_FRIEND,
        //SEASON3B::INTERFACE_INVENTORY,
        //SEASON3B::INTERFACE_CHARACTER,
        //SEASON3B::INTERFACE_WINDOW_MENU,

        INTERFACE_MIXINVENTORY,
        INTERFACE_STORAGE,
        INTERFACE_NPCSHOP,
        INTERFACE_MYSHOP_INVENTORY,
        INTERFACE_PURCHASESHOP_INVENTORY,
        INTERFACE_PET,
        INTERFACE_MYQUEST,
        INTERFACE_NPCQUEST,
        INTERFACE_PARTY,
        INTERFACE_SENATUS,
        INTERFACE_GUARDSMAN,
        INTERFACE_COMMAND,
        INTERFACE_GUILDINFO,
        INTERFACE_KANTURU2ND_ENTERNPC,
        INTERFACE_CURSEDTEMPLE_NPC,
        INTERFACE_DUELWATCH,
        INTERFACE_DOPPELGANGER_NPC,
        //SEASON3B::INTERFACE_HELP,
        //SEASON3B::INTERFACE_ITEM_EXPLANATION,
        //SEASON3B::INTERFACE_SETITEM_EXPLANATION,
        INTERFACE_GOLD_BOWMAN,
        INTERFACE_GOLD_BOWMAN_LENA,
        INTERFACE_NPC_DIALOGUE,
        INTERFACE_QUEST_PROGRESS,
        INTERFACE_QUEST_PROGRESS_ETC,
        INTERFACE_EMPIREGUARDIAN_NPC,
#ifdef PBG_MOD_STAMINA_UI
        SEASON3B::INTERFACE_STAMINA_GAUGE,
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
        INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING
        INTERFACE_UNITEDMARKETPLACE_NPC_JULIA,
        SEASON3B::INTERFACE_LUCKYITEMWND,

        0,
    };

    if (m_pNewUIMng)
    {
        for (int i = 0; dwGroupB[i] != 0; i++)
        {
            m_pNewUIMng->ShowInterface(dwGroupB[i], false);
        }
    }
}
void CNewUISystem::HideGroupBeforeOpenInterface()
{
    DWORD dwGroupC[] =
    {
        INTERFACE_PARTY,
        INTERFACE_COMMAND,
        INTERFACE_GUILDINFO,
        INTERFACE_GOLD_BOWMAN,
        INTERFACE_GOLD_BOWMAN_LENA,
        INTERFACE_GENSRANKING,
        INTERFACE_MUHELPER,
        INTERFACE_MUHELPER_EXT,
        INTERFACE_MUHELPER_SKILL_LIST,
        0,
    };

    if (m_pNewUIMng)
    {
        for (int i = 0; dwGroupC[i] != 0; i++)
        {
            m_pNewUIMng->ShowInterface(dwGroupC[i], false);
        }
    }
}

void CNewUISystem::Enable(DWORD dwKey)
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->EnableInterface(dwKey);
    }
}

void CNewUISystem::Disable(DWORD dwKey)
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->EnableInterface(dwKey, false);
    }
}

bool CNewUISystem::CheckMouseUse()
{
    if (m_pNewUIMng)
    {
        if (m_pNewUIMng->GetActiveMouseUIObj())
            return true;
    }
    return false;
}

bool CNewUISystem::CheckKeyUse()
{
    if (m_pNewUIMng)
    {
        if (m_pNewUIMng->GetActiveKeyUIObj())
            return true;
    }
    return false;
}

bool CNewUISystem::Update()
{
    if (m_pNewItemMng)
    {
        m_pNewItemMng->Update();
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->UpdateMouseEvent();
        m_pNewUIMng->UpdateKeyEvent();
        return m_pNewUIMng->Update();
    }
    return false;
}

bool CNewUISystem::Render()
{
    bool bResult = false;

    BeginBitmap();
    if (m_pNewUIMng)
        bResult = m_pNewUIMng->Render();
    EndBitmap();

    return bResult;
}

CNewUIManager* CNewUISystem::GetNewUIManager() const
{
    return m_pNewUIMng;
}

CNewUI3DRenderMng* CNewUISystem::GetNewUI3DRenderMng() const
{
    return m_pNewUI3DRenderMng;
}

CNewUIHotKey* CNewUISystem::GetNewUIHotKey() const
{
    return m_pNewUIHotKey;
}

bool CNewUISystem::IsImpossibleSendMoveInterface()
{
    if (IsVisible(INTERFACE_MIXINVENTORY)
        || IsVisible(INTERFACE_KANTURU2ND_ENTERNPC)
        || IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
        )
    {
        return true;
    }

    return false;
}

bool CNewUISystem::IsImpossibleTradeInterface()
{
    if (IsVisible(INTERFACE_MIXINVENTORY)
        || IsVisible(INTERFACE_KANTURU2ND_ENTERNPC)
        || IsVisible(INTERFACE_STORAGE)
        || IsVisible(INTERFACE_INGAMESHOP)
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        || IsVisible(INTERFACE_INGAMESHOP)
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
        || IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
        )
    {
        return true;
    }

    return false;
}

bool CNewUISystem::IsImpossibleDuelInterface()
{
    if (IsVisible(INTERFACE_MIXINVENTORY)
        || IsVisible(INTERFACE_KANTURU2ND_ENTERNPC)
        || IsVisible(INTERFACE_STORAGE)
        || IsVisible(INTERFACE_INGAMESHOP)
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        || IsVisible(INTERFACE_INGAMESHOP)
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
        || IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND)
        )
    {
        return true;
    }

    return false;
}

bool CNewUISystem::IsImpossibleHideInterface(DWORD dwKey)
{
    if (dwKey == INTERFACE_MAINFRAME
        || dwKey == INTERFACE_SKILL_LIST
        || dwKey == INTERFACE_SLIDEWINDOW
        || dwKey == INTERFACE_MESSAGEBOX
        || dwKey == INTERFACE_CHATLOGWINDOW
        || dwKey == INTERFACE_SYSTEMLOGWINDOW
        || dwKey == INTERFACE_PARTY_INFO_WINDOW
        || dwKey == INTERFACE_KANTURU_INFO
        || dwKey == INTERFACE_BLOODCASTLE_TIME
        || dwKey == INTERFACE_CHAOSCASTLE_TIME
        || dwKey == INTERFACE_BATTLE_SOCCER_SCORE
        || dwKey == INTERFACE_DUEL_WINDOW
        || dwKey == INTERFACE_CRYWOLF
        || dwKey == INTERFACE_HERO_POSITION_INFO
        || dwKey == INTERFACE_NAME_WINDOW
        || dwKey == INTERFACE_SIEGEWARFARE
        || dwKey == INTERFACE_ITEM_TOOLTIP
        || dwKey == INTERFACE_HOTKEY
        || dwKey == INTERFACE_CURSEDTEMPLE_GAMESYSTEM
        || dwKey == INTERFACE_ITEM_ENDURANCE_INFO
        || dwKey == INTERFACE_BUFF_WINDOW
        || (dwKey >= INTERFACE_3DRENDERING_CAMERA_BEGIN && dwKey <= INTERFACE_3DRENDERING_CAMERA_END)
        || dwKey == INTERFACE_DUELWATCH_MAINFRAME
        || dwKey == INTERFACE_DUELWATCH_USERLIST
        || dwKey == INTERFACE_DOPPELGANGER_FRAME
        || dwKey == INTERFACE_GOLD_BOWMAN
        || dwKey == INTERFACE_GOLD_BOWMAN_LENA
        || dwKey == INTERFACE_EMPIREGUARDIAN_TIMER
        )
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------

void CNewUISystem::UpdateSendMoveInterface()
{
    if (IsVisible(INTERFACE_TRADE))
    {
        SocketClient->ToGameServer()->SendTradeCancel();
        Hide(INTERFACE_TRADE);
    }
    if (IsVisible(INTERFACE_STORAGE_EXT))
    {
        Hide(INTERFACE_STORAGE_EXT);
    }
    if (IsVisible(INTERFACE_STORAGE))
    {
        Hide(INTERFACE_STORAGE);
    }
    if (IsVisible(INTERFACE_NPCGUILDMASTER))
    {
        Hide(INTERFACE_NPCGUILDMASTER);
    }
    if (IsVisible(INTERFACE_MYQUEST))
    {
        Hide(INTERFACE_MYQUEST);
    }
    if (IsVisible(INTERFACE_NPCQUEST))
    {
        Hide(INTERFACE_NPCQUEST);
    }
    if (IsVisible(INTERFACE_NPCSHOP))
    {
        Hide(INTERFACE_NPCSHOP);
    }
    if (IsVisible(INTERFACE_GUARDSMAN))
    {
        Hide(INTERFACE_GUARDSMAN);
    }
    if (IsVisible(INTERFACE_GUARDSMAN))
    {
        Hide(INTERFACE_GUARDSMAN);
    }
    if (IsVisible(INTERFACE_DEVILSQUARE))
    {
        Hide(INTERFACE_DEVILSQUARE);
    }
    if (IsVisible(INTERFACE_BLOODCASTLE))
    {
        Hide(INTERFACE_BLOODCASTLE);
    }
    if (IsVisible(INTERFACE_CURSEDTEMPLE_NPC))
    {
        Hide(INTERFACE_CURSEDTEMPLE_NPC);
    }
    if (IsVisible(INTERFACE_MYSHOP_INVENTORY))
    {
        Hide(INTERFACE_MYSHOP_INVENTORY);
    }
    if (IsVisible(INTERFACE_PURCHASESHOP_INVENTORY))
    {
        Hide(INTERFACE_PURCHASESHOP_INVENTORY);
    }
    if (IsVisible(INTERFACE_DUELWATCH))
    {
        Hide(INTERFACE_DUELWATCH);
    }
    if (IsVisible(INTERFACE_DOPPELGANGER_NPC))
    {
        Hide(INTERFACE_DOPPELGANGER_NPC);
    }
    if (IsVisible(INTERFACE_NPC_DIALOGUE))
    {
        Hide(INTERFACE_NPC_DIALOGUE);
    }
    if (IsVisible(INTERFACE_QUEST_PROGRESS))
    {
        Hide(INTERFACE_QUEST_PROGRESS);
    }
    if (IsVisible(INTERFACE_QUEST_PROGRESS_ETC))
    {
        Hide(INTERFACE_QUEST_PROGRESS_ETC);
    }
    if (IsVisible(INTERFACE_EMPIREGUARDIAN_NPC))
    {
        Hide(INTERFACE_EMPIREGUARDIAN_NPC);
    }
    if (IsVisible(INTERFACE_LUCKYCOIN_REGISTRATION))
    {
        Hide(INTERFACE_LUCKYCOIN_REGISTRATION);
    }
    if (IsVisible(INTERFACE_EXCHANGE_LUCKYCOIN))
    {
        Hide(INTERFACE_EXCHANGE_LUCKYCOIN);
    }
    if (IsVisible(INTERFACE_UNITEDMARKETPLACE_NPC_JULIA))
    {
        Hide(INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
    }
}

// GetInstance()
CNewUISystem* CNewUISystem::GetInstance()
{
    static CNewUISystem s_NewUISystem;
    return &s_NewUISystem;
}

CNewUIChatLogWindow* CNewUISystem::GetUI_NewChatLogWindow() const
{
    return m_pNewChatLogWindow;
}

CNewUISystemLogWindow* CNewUISystem::GetUI_NewSystemLogWindow() const
{
    return m_pNewSystemLogWindow;
}

CNewUISlideWindow* CNewUISystem::GetUI_NewSlideWindow() const
{
    return m_pNewSlideWindow;
}

CNewUIFriendWindow* CNewUISystem::GetUI_NewFriendWindow() const
{
    return m_pNewFriendWindow;
}

CNewUIMainFrameWindow* CNewUISystem::GetUI_NewMainFrameWindow() const
{
    return m_pNewMainFrameWindow;
}

CNewUISkillList* CNewUISystem::GetUI_NewSkillList() const
{
    return m_pNewSkillList;
}

CNewUIChatInputBox* CNewUISystem::GetUI_NewChatInputBox() const
{
    return m_pNewChatInputBox;
}

CNewUIItemMng* CNewUISystem::GetUI_NewItemMng() const
{
    return m_pNewItemMng;
}

CNewUIMyInventory* CNewUISystem::GetUI_NewMyInventory() const
{
    return m_pNewMyInventory;
}

CNewUIInventoryExtension* CNewUISystem::GetUI_NewMyInventoryExt() const
{
    return m_pNewMyInventoryExt;
}

CNewUINPCShop* CNewUISystem::GetUI_NewNpcShop() const
{
    return m_pNewNPCShop;
}

CNewUIPetInfoWindow* CNewUISystem::GetUI_NewPetInfoWindow() const
{
    return m_pNewPetInfoWindow;
}

CNewUIMixInventory* CNewUISystem::GetUI_NewMixInventory() const
{
    return m_pNewMixInventory;
}

CNewUICastleWindow* CNewUISystem::GetUI_NewCastleWindow() const
{
    return m_pNewCastleWindow;
}

CNewUIGuardWindow* CNewUISystem::GetUI_NewGuardWindow() const
{
    return m_pNewGuardWindow;
}

CNewUIGatemanWindow* CNewUISystem::GetUI_NewGatemanWindow() const
{
    return m_pNewGatemanWindow;
}

CNewUIGateSwitchWindow* CNewUISystem::GetUI_NewGateSwitchWindow() const
{
    return m_pNewGateSwitchWindow;
}

CNewUIStorageInventory* CNewUISystem::GetUI_NewStorageInventory() const
{
    return m_pNewStorageInventory;
}

CNewUIStorageInventoryExt* CNewUISystem::GetUI_NewStorageInventoryExt() const
{
    return m_pNewStorageInventoryExt;
}

CNewUIGuildMakeWindow* CNewUISystem::GetUI_NewGuildMakeWindow() const
{
    return m_pNewGuildMakeWindow;
}

CNewUIGuildInfoWindow* CNewUISystem::GetUI_NewGuildInfoWindow() const
{
    return m_pNewGuildInfoWindow;
}

CNewUICryWolf* CNewUISystem::GetUI_NewCryWolfInterface() const
{
    return m_pNewCryWolfInterface;
}

CNewUIMasterLevel* CNewUISystem::GetUI_NewMasterLevelInterface() const
{
    return m_pNewMaster_Level_Interface;
}

CNewUIMyShopInventory* CNewUISystem::GetUI_NewMyShopInventory() const
{
    return m_pNewMyShopInventory;
}

CNewUIPurchaseShopInventory* CNewUISystem::GetUI_NewPurchaseShopInventory() const
{
    return m_pNewPurchaseShopInventory;
}

CNewUICharacterInfoWindow* CNewUISystem::GetUI_NewCharacterInfoWindow() const
{
    return m_pNewCharacterInfoWindow;
}

CNewUIMyQuestInfoWindow* CNewUISystem::GetUI_NewMyQuestInfoWindow() const
{
    return m_pNewMyQuestInfoWindow;
}

CNewUIPartyInfoWindow* CNewUISystem::GetUI_NewPartyInfoWindow() const
{
    return m_pNewPartyInfoWindow;
}

CNewUIPartyListWindow* CNewUISystem::GetUI_NewPartyListWindow() const
{
    return m_pNewPartyListWindow;
}

CNewUINPCQuest* CNewUISystem::GetUI_NewNPCQuest() const
{
    return m_pNewNPCQuest;
}

CNewUIEnterBloodCastle* CNewUISystem::GetUI_NewEnterBloodCastle() const
{
    return m_pNewEnterBloodCastle;
}

CNewUIEnterDevilSquare* CNewUISystem::GetUI_NewEnterDevilSquare() const
{
    return m_pNewEnterDevilSquare;
}

CNewUIBloodCastle* CNewUISystem::GetUI_NewBloodCastle() const
{
    return m_pNewBloodCastle;
}

CNewUITrade* CNewUISystem::GetUI_NewTrade() const
{
    return m_pNewTrade;
}

CNewUIKanturu2ndEnterNpc* CNewUISystem::GetUI_NewKanturu2ndEnterNpc() const
{
    return m_pNewKanturu2ndEnterNpc;
}

CNewUIKanturuInfoWindow* CNewUISystem::GetUI_NewKanturuInfoWindow() const
{
    return m_pNewKanturuInfoWindow;
}

CNewUICatapultWindow* CNewUISystem::GetUI_NewCatapultWindow() const
{
    return m_pNewCatapultWindow;
}

CNewUIChaosCastleTime* CNewUISystem::GetUI_NewChaosCastleTime() const
{
    return m_pNewChaosCastleTime;
}

CNewUICommandWindow* CNewUISystem::GetUI_NewCommandWindow() const
{
    return m_pNewCommandWindow;
}

CNewUIWindowMenu* CNewUISystem::GetUI_NewWindowMenu() const
{
    return m_pNewWindowMenu;
}

CNewUIOptionWindow* CNewUISystem::GetUI_NewOptionWindow() const
{
    return m_pNewOptionWindow;
}

CNewUIHeroPositionInfo* CNewUISystem::GetUI_NewHeroPositionInfo() const
{
    return m_pNewHeroPositionInfo;
}

CNewUIHelpWindow* CNewUISystem::GetUI_NewHelpWindow() const
{
    return m_pNewHelpWindow;
}

CNewUIItemExplanationWindow* CNewUISystem::GetUI_NewItemExplanationWindow() const
{
    return m_pNewItemExplanationWindow;
}

CNewUISetItemExplanation* CNewUISystem::GetUI_NewSetItemExplanation() const
{
    return m_pNewSetItemExplanation;
}

CNewUIQuickCommandWindow* CNewUISystem::GetUI_NewQuickCommandWindow() const
{
    return m_pNewQuickCommandWindow;
}

CNewUIMoveCommandWindow* CNewUISystem::GetUI_NewMoveCommandWindow() const
{
    return m_pNewMoveCommandWindow;
}

CNewUIBattleSoccerScore* CNewUISystem::GetUI_NewBattleSoccerScore() const
{
    return m_pNewBattleSoccerScore;
}

CNewUIDuelWindow* CNewUISystem::GetUI_NewDuelWindow() const
{
    return m_pNewDuelWindow;
}

CNewUISiegeWarfare* CNewUISystem::GetUI_NewSiegeWarfare() const
{
    return m_pNewSiegeWarfare;
}

CNewUIItemEnduranceInfo* CNewUISystem::GetUI_NewItemEnduranceInfo() const
{
    return m_pNewItemEnduranceInfo;
}

CNewUIBuffWindow* CNewUISystem::GetUI_NewBuffWindow() const
{
    return m_pNewBuffWindow;
}

CNewUICursedTempleEnter* CNewUISystem::GetUI_NewCursedTempleEnterWindow() const
{
    return m_pNewCursedTempleEnterWindow;
}

CNewUICursedTempleSystem* CNewUISystem::GetUI_NewCursedTempleWindow() const
{
    return m_pNewCursedTempleWindow;
}

CNewUICursedTempleResult* CNewUISystem::GetUI_NewCursedTempleResultWindow() const
{
    return m_pNewCursedTempleResultWindow;
}

CNewUIGoldBowmanWindow* CNewUISystem::GetUI_pNewGoldBowman() const
{
    return m_pNewGoldBowman;
}

CNewUIGoldBowmanLena* CNewUISystem::GetUI_pNewGoldBowmanLena() const
{
    return m_pNewGoldBowmanLena;
}

CNewUIRegistrationLuckyCoin* CNewUISystem::GetUI_pNewLuckyCoinRegistration() const
{
    return m_pNewLuckyCoinRegistration;
}

CNewUIExchangeLuckyCoin* CNewUISystem::GetUI_pNewExchangeLuckyCoin() const
{
    return m_pNewExchangeLuckyCoinWindow;
}

CNewUIMiniMap* CNewUISystem::GetUI_pNewUIMiniMap() const
{
    return m_pNewMiniMap;
}

CNewUIDuelWatchWindow* CNewUISystem::GetUI_pNewDuelWatch() const
{
    return m_pNewDuelWatchWindow;
}

CNewUIDuelWatchMainFrameWindow* CNewUISystem::GetUI_pNewDuelWatchMainFrame() const
{
    return m_pNewDuelWatchMainFrameWindow;
}

CNewUIDuelWatchUserListWindow* CNewUISystem::GetUI_pNewDuelWatchUserList() const
{
    return m_pNewDuelWatchUserListWindow;
}

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
CNewUIInGameShop* CNewUISystem::GetUI_pNewInGameShop() const
{
    return m_pNewInGameShop;
}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

CNewUIDoppelGangerWindow* CNewUISystem::GetUI_pNewDoppelGangerWindow() const
{
    return m_pNewDoppelGangerWindow;
}

CNewUIDoppelGangerFrame* CNewUISystem::GetUI_pNewDoppelGangerFrame() const
{
    return m_pNewDoppelGangerFrame;
}

CNewUINPCDialogue* CNewUISystem::GetUI_NewNPCDialogue() const
{
    return m_pNewNPCDialogue;
}

CNewUIQuestProgress* CNewUISystem::GetUI_NewQuestProgress() const
{
    return m_pNewQuestProgress;
}

CNewUIQuestProgressByEtc* CNewUISystem::GetUI_NewQuestProgressByEtc() const
{
    return m_pNewQuestProgressByEtc;
}

CNewUIEmpireGuardianNPC* CNewUISystem::GetUI_pNewEmpireGuardianNPC() const
{
    return m_pNewEmpireGuardianNPC;
}

CNewUIEmpireGuardianTimer* CNewUISystem::GetUI_pNewEmpireGuardianTimer() const
{
    return m_pNewEmpireGuardianTimer;
}

#ifdef PBG_MOD_STAMINA_UI
CNewUIStamina* SEASON3B::CNewUISystem::GetUI_pNewUIStamina() const
{
    return m_pNewUIStamina;
}
#endif //PBG_MOD_STAMINA_UI

#ifdef PBG_ADD_GENSRANKING
CNewUIGensRanking* CNewUISystem::GetUI_NewGensRanking() const
{
    return m_pNewGensRanking;
}
#endif //PBG_ADD_GENSRANKING

CNewUIUnitedMarketPlaceWindow* CNewUISystem::GetUI_pNewUnitedMarketPlaceWindow() const
{
    return m_pNewUnitedMarketPlaceWindow;
}

CNewUILuckyItemWnd* SEASON3B::CNewUISystem::Get_pNewUILuckyItemWnd() const
{
    return m_pNewUILuckyItemWnd;
}

CNewUIMuHelper* CNewUISystem::Get_pNewUIMuHelper() const
{
    return m_pNewUIMuHelper;
}

CNewUIMuHelperExt* CNewUISystem::Get_pNewUIMuHelperExt() const
{
    return m_pNewUIMuHelperExt;
}

CNewUIMuHelperSkillList* CNewUISystem::Get_pNewUIMuHelperSkillList() const
{
    return m_pNewUIMuHelperSkillList;
}
