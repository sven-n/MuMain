#include "stdafx.h"
#include "UI/Core/NewUISystem.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Scaling/UITransform.h"

#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "World/MapInfra/MapManager.h"


using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
    constexpr int kLayoutBaseX = 640;
    constexpr int kLayoutPanelWidth = 190;

    constexpr int PanelColumnX(int columns)
    {
        return kLayoutBaseX - (kLayoutPanelWidth * columns);
    }

    bool IsHeroPositionLayoutInterface(DWORD dwKey)
    {
        switch (dwKey)
        {
        case INTERFACE_INVENTORY:
        case INTERFACE_INVENTORY_EXT:
        case INTERFACE_STORAGE:
        case INTERFACE_STORAGE_EXT:
        case INTERFACE_CHARACTER:
        case INTERFACE_NPCSHOP:
        case INTERFACE_MIXINVENTORY:
        case INTERFACE_TRADE:
        case INTERFACE_MYSHOP_INVENTORY:
        case INTERFACE_PURCHASESHOP_INVENTORY:
            return true;
        default:
            return false;
        }
    }
}

CSystem::CSystem()
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
    m_pMuHelperBar = nullptr;
    m_pNewHelpWindow = nullptr;
    m_pNewChatCommandWindow = nullptr;
    m_pNewItemExplanationWindow = nullptr;
    m_pNewSetItemExplanation = nullptr;
    m_pNewQuickCommandWindow = nullptr;
    m_pNewMoveCommandWindow = nullptr;
    m_pNewSiegeWarfare = nullptr;
    m_pNewItemEnduranceInfo = nullptr;
    m_pBuffStrip = nullptr;
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

CSystem::~CSystem()
{
    Release();
}

bool CSystem::Create()
{
    m_pNewUIMng = new CManager;

    m_pNewUI3DRenderMng = new C3DRenderMng;
    if (false == m_pNewUI3DRenderMng->Create(m_pNewUIMng))
        return false;

    m_pNewChatLogWindow = new CChatLogWindow;
    if (false == m_pNewChatLogWindow->Create(m_pNewUIMng, 0, 480 - 50 - 47, 6))
        return false;

    m_pNewSystemLogWindow = new CSystemLogWindow;
    if (false == m_pNewSystemLogWindow->Create(m_pNewUIMng, 0, 80))
        return false;

    m_pNewOptionWindow = new COptionWindow;
    if (m_pNewOptionWindow->Create(m_pNewUIMng, (640 / 2) - (190 / 2), 5) == false)
    {
        return false;
    }

    m_pNewSlideWindow = new CSlideWindow;
    if (m_pNewSlideWindow->Create(m_pNewUIMng) == false)
    {
        return false;
    }

    if (g_MessageBox->Create(m_pNewUIMng) == false)
        return false;

    return true;
}

void CSystem::Release()
{
    if (m_pNewUIMng == nullptr)
    {
        return;
    }

    UnloadMainSceneInterface();
    g_MessageBox->Release();

    SAFE_DELETE(m_pNewSlideWindow);
    SAFE_DELETE(m_pNewOptionWindow);
    SAFE_DELETE(m_pNewChatLogWindow);
    SAFE_DELETE(m_pNewSystemLogWindow);
    SAFE_DELETE(m_pNewUI3DRenderMng);

    m_pNewUIMng->RemoveAllUIObjs();

    SAFE_DELETE(m_pNewUIMng);
}

bool CSystem::CreateChatCommandWindow()
{
    m_pNewChatCommandWindow = new CChatCommandWindow;
    if (m_pNewChatCommandWindow->Create(m_pNewUIMng, PanelColumnX(1), 0))
    {
        return true;
    }

    SAFE_DELETE(m_pNewChatCommandWindow);
    return false;
}

bool CSystem::LoadMainSceneInterface()
{
    g_MessageBox->Show(true);
    m_pNewChatLogWindow->Show(true);
    m_pNewSystemLogWindow->Show(true);
    m_pNewSlideWindow->Show(true);

    m_pNewItemMng = new CItemMng;

    m_pNewChatInputBox = new CChatInputBox;

    if (false == m_pNewChatInputBox->Create(m_pNewUIMng, m_pNewChatLogWindow, m_pNewSystemLogWindow, 0, 480 - 51 - 47))
    {
        return false;
    }

    SetFocus(g_hWnd);

    m_pNewUIHotKey = new CHotKey;
    if (false == m_pNewUIHotKey->Create(m_pNewUIMng))
        return false;

    m_pNewMainFrameWindow = new CMainFrameWindow;
    if (m_pNewMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
        return false;

    m_pNewSkillList = new CSkillList;
    if (m_pNewSkillList->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
        return false;

    m_pNewFriendWindow = new CFriendWindow;
    if (m_pNewFriendWindow->Create(m_pNewUIMng) == false)
        return false;

    m_pNewMyInventory = new CMyInventory;
    if (false == m_pNewMyInventory->Create(m_pNewUIMng, m_pNewUI3DRenderMng, PanelColumnX(1), 0))
        return false;

    m_pNewMyInventoryExt = new CInventoryExtension;
    if (false == m_pNewMyInventoryExt->Create(m_pNewUIMng, PanelColumnX(2), 0))
        return false;

    m_pNewNPCShop = new CNPCShop;
    if (false == m_pNewNPCShop->Create(m_pNewUIMng, CNPCShop::NPCSHOP_POS_X, CNPCShop::NPCSHOP_POS_Y))
        return false;

    m_pNewPetInfoWindow = new CPetInfoWindow;
    if (false == m_pNewPetInfoWindow->Create(m_pNewUIMng, PanelColumnX(2), 0))
        return false;

    m_pNewMixInventory = new CMixInventory;
    if (m_pNewMixInventory->Create(m_pNewUIMng, 260, 0) == false)
        return false;

    m_pNewCastleWindow = new CCastleWindow;
    if (m_pNewCastleWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewGuardWindow = new CGuardWindow;
    if (m_pNewGuardWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewGatemanWindow = new CGatemanWindow;
    if (m_pNewGatemanWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewGateSwitchWindow = new CGateSwitchWindow;
    if (m_pNewGateSwitchWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewStorageInventory = new CStorageInventory;
    if (m_pNewStorageInventory->Create(m_pNewUIMng, 260, 0) == false)
        return false;

    m_pNewStorageInventoryExt = new CStorageInventoryExt;
    if (m_pNewStorageInventoryExt->Create(m_pNewUIMng, 260 - 190, 0) == false)
        return false;

    m_pNewGuildInfoWindow = new CGuildInfoWindow;
    if (m_pNewGuildInfoWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewGuildMakeWindow = new CGuildMakeWindow;
    if (m_pNewGuildMakeWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    CreatePersonalItemTable();

    m_pNewMyShopInventory = new CMyShopInventory;
    if (m_pNewMyShopInventory->Create(m_pNewUIMng, PanelColumnX(2), 0) == false)
        return false;

    m_pNewPurchaseShopInventory = new CPurchaseShopInventory;
    if (m_pNewPurchaseShopInventory->Create(m_pNewUIMng, PanelColumnX(2), 0) == false)
        return false;

    m_pNewCharacterInfoWindow = new CCharacterInfoWindow;
    if (m_pNewCharacterInfoWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewMyQuestInfoWindow = new CMyQuestInfoWindow;
    if (m_pNewMyQuestInfoWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewPartyInfoWindow = new CPartyInfoWindow;
    if (m_pNewPartyInfoWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewPartyListWindow = new CPartyListWindow;
    if (m_pNewPartyListWindow->Create(m_pNewUIMng, 640 - 79, 14) == false)
        return false;

    m_pNewNPCQuest = new CNPCQuest;
    if (m_pNewNPCQuest->Create(m_pNewUIMng, m_pNewUI3DRenderMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewEnterBloodCastle = new CEnterBloodCastle;
    if (m_pNewEnterBloodCastle->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewEnterDevilSquare = new CEnterDevilSquare;
    if (m_pNewEnterDevilSquare->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewBloodCastle = new CBloodCastle;
    if (m_pNewBloodCastle->Create(m_pNewUIMng, 640 - 127, 480 - 132) == false)
        return false;

    m_pNewTrade = new CTrade;
    if (m_pNewTrade->Create(m_pNewUIMng, 260, 0) == false)
        return false;

    m_pNewKanturu2ndEnterNpc = new CKanturu2ndEnterNpc;
    if (m_pNewKanturu2ndEnterNpc->Create(m_pNewUIMng, (640 / 2) - (230 / 2), 20) == false)
    {
        return false;
    }

    m_pNewKanturuInfoWindow = new CKanturuInfoWindow;
    if (m_pNewKanturuInfoWindow->Create(m_pNewUIMng, 541, 351) == false)
    {
        return false;
    }

    m_pNewChaosCastleTime = new CChaosCastleTime;
    if (m_pNewChaosCastleTime->Create(m_pNewUIMng, 640 - 127, 480 - 132) == false)
        return false;

    m_pNewBattleSoccerScore = new CBattleSoccerScore;
    if (m_pNewBattleSoccerScore->Create(m_pNewUIMng, 509, 359) == false)
        return false;

    m_pNewCommandWindow = new CCommandWindow;
    if (m_pNewCommandWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewCatapultWindow = new CCatapultWindow;
    if (m_pNewCatapultWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
    {
        return false;
    }

    m_pNewWindowMenu = new CWindowMenu;
    if (m_pNewWindowMenu->Create(m_pNewUIMng, 640 - 112, 480 - 171) == false)
    {
        return false;
    }

    m_pMuHelperBar = new CMuHelperBar;
    if (m_pMuHelperBar->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    if (!CreateChatCommandWindow())
    {
        return false;
    }

    m_pNewHelpWindow = new CHelpWindow;
    if (m_pNewHelpWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewItemExplanationWindow = new CItemExplanationWindow;
    if (m_pNewItemExplanationWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewSetItemExplanation = new CSetItemExplanation;
    if (m_pNewSetItemExplanation->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewQuickCommandWindow = new CQuickCommandWindow;
    if (m_pNewQuickCommandWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewMoveCommandWindow = new CMoveCommandWindow;

    if (m_pNewMoveCommandWindow->Create(m_pNewUIMng, 1, 1) == false)
        return false;

    m_pNewDuelWindow = new CDuelWindow;
    if (m_pNewDuelWindow->Create(m_pNewUIMng, 509, 359) == false)
    {
        return false;
    }

    m_pNewNameWindow = new CNameWindow;
    if (m_pNewNameWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }

    m_pNewSiegeWarfare = new CSiegeWarfare;
    if (m_pNewSiegeWarfare->Create(m_pNewUIMng, 486, 234) == false)
        return false;

    m_pNewItemEnduranceInfo = new CItemEnduranceInfo;
    if (m_pNewItemEnduranceInfo->Create(m_pNewUIMng, 2, 26) == false)
    {
        return false;
    }

    m_pBuffStrip = new CBuffStrip;
    if (m_pBuffStrip->Create(m_pNewUIMng, 220, 15) == false)
    {
        return false;
    }

    m_pNewCursedTempleEnterWindow = new CCursedTempleEnter;
    if (m_pNewCursedTempleEnterWindow->Create(m_pNewUIMng, 640 / 2 - 230 / 2, 80) == false)
    {
        return false;
    }
    m_pNewCursedTempleWindow = new CCursedTempleSystem;
    if (m_pNewCursedTempleWindow->Create(m_pNewUIMng, 0, 0) == false)
    {
        return false;
    }
    m_pNewCursedTempleResultWindow = new CCursedTempleResult;
    if (m_pNewCursedTempleResultWindow->Create(m_pNewUIMng, 640 / 2 - 230 / 2, 120) == false)
    {
        return false;
    }

    m_pNewCryWolfInterface = new CCryWolf;
    if (m_pNewCryWolfInterface->Create(m_pNewUIMng, 0, 0) == false)
        return false;

    m_pNewMaster_Level_Interface = new CMasterLevel;
    if (m_pNewMaster_Level_Interface->Create(m_pNewUIMng) == false)
        return false;

    m_pNewMiniMap = new CMiniMap;
    if (m_pNewMiniMap->Create(m_pNewUIMng, 0, 0) == false)
        return false;

    m_pNewGoldBowman = new CGoldBowmanWindow;
    if (m_pNewGoldBowman->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewGoldBowmanLena = new CGoldBowmanLena;
    if (m_pNewGoldBowmanLena->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewLuckyCoinRegistration = new CRegistrationLuckyCoin;
    if (m_pNewLuckyCoinRegistration->Create(m_pNewUIMng, PanelColumnX(2), 0) == false)
        return false;

    m_pNewExchangeLuckyCoinWindow = new CExchangeLuckyCoin;
    if (m_pNewExchangeLuckyCoinWindow->Create(m_pNewUIMng, PanelColumnX(2), 0) == false)
        return false;

    m_pNewDuelWatchWindow = new CDuelWatchWindow;
    if (m_pNewDuelWatchWindow->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewDuelWatchMainFrameWindow = new CDuelWatchMainFrameWindow;
    if (m_pNewDuelWatchMainFrameWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
        return false;

    m_pNewDuelWatchUserListWindow = new CDuelWatchUserListWindow;
    if (m_pNewDuelWatchUserListWindow->Create(m_pNewUIMng, 640 - 57, 480 - 51) == false)
        return false;

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    m_pNewInGameShop = new CInGameShop;
    if (m_pNewInGameShop->Create(m_pNewUIMng, 0, 0) == false)
        return false;
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

    m_pNewDoppelGangerWindow = new CDoppelGangerWindow;
    if (m_pNewDoppelGangerWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewDoppelGangerFrame = new CDoppelGangerFrame;
    if (m_pNewDoppelGangerFrame->Create(m_pNewUIMng, 640 - 227, 480 - 51 - 87) == false)
        return false;

    m_pNewNPCDialogue = new CNPCDialogue;
    if (m_pNewNPCDialogue->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewQuestProgress = new CQuestProgress;
    if (m_pNewQuestProgress->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewQuestProgressByEtc = new CQuestProgressByEtc;
    if (m_pNewQuestProgressByEtc->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewEmpireGuardianNPC = new CEmpireGuardianNPC;
    if (m_pNewEmpireGuardianNPC->Create(m_pNewUIMng, m_pNewUI3DRenderMng, 450, 0) == false)
        return false;

    m_pNewEmpireGuardianTimer = new CEmpireGuardianTimer;
    if (m_pNewEmpireGuardianTimer->Create(m_pNewUIMng, 507, 342) == false)
        return false;

#ifdef PBG_MOD_STAMINA_UI
    m_pNewUIStamina = new CNewUIStamina;
    if (m_pNewUIStamina->Create(m_pNewUIMng, 640, 480) == false)
        return false;
#endif //PBG_MOD_STAMINA_UI

    m_pNewGensRanking = new CGensRanking;
    if (m_pNewGensRanking->Create(m_pNewUIMng, 640, 480) == false)
        return false;

    m_pNewUnitedMarketPlaceWindow = new CUnitedMarketPlaceWindow;
    if (m_pNewUnitedMarketPlaceWindow->Create(m_pNewUIMng, m_pNewUI3DRenderMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewUILuckyItemWnd = new CLuckyItemWnd;
    if (m_pNewUILuckyItemWnd->Create(m_pNewUIMng, 260, 0) == false)
        return false;

    m_pNewUIMuHelper = new CUIMuHelper;
    if (m_pNewUIMuHelper->Create(m_pNewUIMng, PanelColumnX(1), 0) == false)
        return false;

    m_pNewUIMuHelperExt = new CMuHelperExt;
    if (m_pNewUIMuHelperExt->Create(m_pNewUIMng, PanelColumnX(2), 0) == false)
        return false;

    m_pNewUIMuHelperSkillList = new CMuHelperSkillList;
    if (m_pNewUIMuHelperSkillList->Create(m_pNewUIMng, m_pNewUI3DRenderMng) == false)
        return false;

    return true;
}

void CSystem::UnloadMainSceneInterface()
{
    if (g_pNewUIMng)
    {
        g_pNewUIMng->ShowAllInterfaces(false);
    }

    SAFE_DELETE(m_pNewHelpWindow);
    SAFE_DELETE(m_pNewChatCommandWindow);
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
    SAFE_DELETE(m_pMuHelperBar);
    SAFE_DELETE(m_pNewMoveCommandWindow);
    SAFE_DELETE(m_pNewUIHotKey);
    SAFE_DELETE(m_pNewSiegeWarfare);
    SAFE_DELETE(m_pNewItemEnduranceInfo);
    SAFE_DELETE(m_pBuffStrip);
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

bool CSystem::IsVisible(DWORD dwKey)
{
    if (m_pNewUIMng)
    {
        return m_pNewUIMng->IsInterfaceVisible(dwKey);
    }

    return false;
}

//bool SortUiObj(const IObject& lhs, const IObject& rhs)
//{
//	return lhs.GetDisplayOrder() > rhs.GetDisplayOrder();
//}

void CSystem::Show(DWORD dwKey)
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
    std::list<IObject*> visiblePages = {};

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

        const auto bounds = UI::Scaling::FloatingWorkspaceBounds(WindowWidth, WindowHeight);
        const int contentHeight =
            static_cast<int>(UI::Scaling::FloatingWorkspaceContentHeight(WindowWidth, WindowHeight));
        m_pNewFriendWindow->OpenMainWnd(bounds.width - 250, contentHeight - 170);
    }
    else if (dwKey == INTERFACE_INVENTORY)
    {
        HideGroupBeforeOpenInterface();

        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);

        if (IsVisible(INTERFACE_CHARACTER))
        {
            g_pMyInventory->SetPos(PanelColumnX(2), 0);
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
            g_pStorageInventory->SetPos(PanelColumnX(3), 0);
            Hide(INTERFACE_MU_HELPER_BAR);
        }

        if (IsVisible(INTERFACE_MYSHOP_INVENTORY))
        {
            g_pMyShopInventory->SetPos(PanelColumnX(3), 0);
            Hide(INTERFACE_MU_HELPER_BAR);
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
            g_pNPCShop->SetPos(PanelColumnX(3), 0);
            Hide(INTERFACE_MU_HELPER_BAR);
        }
        if (IsVisible(INTERFACE_MIXINVENTORY))
        {
            g_pMixInventory->SetPos(PanelColumnX(3), 0);
            Hide(INTERFACE_MU_HELPER_BAR);
        }
        if (IsVisible(INTERFACE_TRADE))
        {
            g_pTrade->SetPos(PanelColumnX(3), 0);
            Hide(INTERFACE_MU_HELPER_BAR);
        }
    }
    else if (dwKey == INTERFACE_CHARACTER)
    {
        HideGroupBeforeOpenInterface();

        g_pMainFrame->SetBtnState(MAINFRAME_BTN_CHAINFO, true);

        if (IsVisible(INTERFACE_INVENTORY))
        {
            g_pMyInventory->SetPos(PanelColumnX(2), 0);
            if (IsVisible(INTERFACE_INVENTORY_EXT))
            {
                g_pMyInventory->SetPos(PanelColumnX(3), 0);
                Hide(INTERFACE_MU_HELPER_BAR);
            }
        }
        else if (IsVisible(INTERFACE_MYQUEST))
        {
            g_pMyQuestInfoWindow->SetPos(PanelColumnX(2), 0);
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
            g_pMyQuestInfoWindow->SetPos(PanelColumnX(2), 0);
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
        g_pNPCShop->SetPos(PanelColumnX(2), 0);
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
            g_pStorageInventory->SetPos(PanelColumnX(3), 0);
            Hide(INTERFACE_MU_HELPER_BAR);
        }
        else
        {
            g_pStorageInventory->SetPos(PanelColumnX(2), 0);
            Show(INTERFACE_MU_HELPER_BAR);
        }

        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, true);
    }
    else if (dwKey == INTERFACE_STORAGE_EXT)
    {
        if (IsVisible(INTERFACE_INVENTORY_EXT))
        {
            m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY_EXT, false);
        }

        Hide(INTERFACE_MU_HELPER_BAR);
        g_pStorageInventory->SetPos(PanelColumnX(2), 0);
        g_pStorageInventoryExt->SetPos(PanelColumnX(3), 0);

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
            g_pMyShopInventory->SetPos(PanelColumnX(3), 0);
            Hide(INTERFACE_MU_HELPER_BAR);
        }
        else
        {
            g_pMyShopInventory->SetPos(PanelColumnX(2), 0);
            Show(INTERFACE_MU_HELPER_BAR);
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
    else if (dwKey == INTERFACE_COMMAND_LIST)
    {
        HideAllGroupA();
        m_pNewChatCommandWindow->OpenningProcess();
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
    else if (dwKey == INTERFACE_MU_HELPER_BAR)
    {
        m_pMuHelperBar->OpenningProcess();
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
        m_pBuffStrip->OpenningProcess();
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
        g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt CallStack - CSystem.Show()\r\n");
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
            g_pQuestProgressByEtc->SetPos(PanelColumnX(1), 0);
        }
        if (IsVisible(INTERFACE_CHARACTER))
            g_pQuestProgressByEtc->SetPos(PanelColumnX(2), 0);
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
        g_pNewUIGensRanking->SetPos(PanelColumnX(1), 0);
    }
    else if (dwKey == INTERFACE_UNITEDMARKETPLACE_NPC_JULIA)
    {
        m_pNewUnitedMarketPlaceWindow->OpeningProcess();
    }
    else if (dwKey == mu::ui::window::INTERFACE_LUCKYITEMWND)
    {
        HideAllGroupA();
        g_pLuckyItemWnd->OpeningProcess();
        m_pNewUIMng->ShowInterface(mu::ui::window::INTERFACE_INVENTORY);
    }
    else if (dwKey == INTERFACE_MUHELPER)
    {
        HideAllGroupA();
    }

    m_pNewUIMng->ShowInterface(dwKey);

    UpdateMuHelperBarVisibilityForLayoutChange(dwKey);

    int iScreenWidth = GetScreenWidth();
    m_pNewItemEnduranceInfo->SetPos(iScreenWidth);
    m_pBuffStrip->SetPos(iScreenWidth);
    m_pNewPartyListWindow->SetPos(iScreenWidth);
}

void CSystem::Hide(DWORD dwKey)
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
            g_pMyQuestInfoWindow->SetPos(PanelColumnX(1), 0);
        }
        if (IsVisible((INTERFACE_INVENTORY)))
        {
            g_pMyInventory->SetPos(PanelColumnX(1), 0);
        }
        if (IsVisible((INTERFACE_PET)))
        {
            Hide(INTERFACE_PET);
        }
        if (IsVisible((INTERFACE_QUEST_PROGRESS_ETC)))
        {
            g_pQuestProgressByEtc->SetPos(PanelColumnX(1), 0);
        }
    }
    else if (dwKey == INTERFACE_INVENTORY_EXT)
    {
        constexpr auto secondColumnX = PanelColumnX(2);
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

        Show(INTERFACE_MU_HELPER_BAR);
    }
    else if (dwKey == INTERFACE_INVENTORY)
    {
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);

        if (IsVisible(INTERFACE_INVENTORY_EXT))
        {
            Hide(INTERFACE_INVENTORY_EXT);
        }

        if (IsVisible(INTERFACE_MIXINVENTORY))
        {
            if (g_pMixInventory->ClosingProcess() == false)
                return;
            m_pNewUIMng->ShowInterface(INTERFACE_MIXINVENTORY, false);
        }
        if (IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND))
        {
            if (g_pLuckyItemWnd->ClosingProcess() == false)
                return;
            m_pNewUIMng->ShowInterface(mu::ui::window::INTERFACE_LUCKYITEMWND, false);
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

        if (IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND))
        {
            m_pNewUILuckyItemWnd->ClosingProcess();
            m_pNewUIMng->ShowInterface(mu::ui::window::INTERFACE_LUCKYITEMWND, false);
        }

        g_pMyInventory->SetPos(PanelColumnX(1), 0);
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
        Show(INTERFACE_MU_HELPER_BAR);
    }
    else if (dwKey == INTERFACE_NPCSHOP)
    {
        if (IsVisible(INTERFACE_INVENTORY_EXT))
        {
            Hide(INTERFACE_INVENTORY_EXT);
        }
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
        g_pMyInventory->SetPos(PanelColumnX(1), 0);
        Show(INTERFACE_MU_HELPER_BAR);
    }
    else if (dwKey == INTERFACE_STORAGE)
    {
        g_pStorageInventoryExt->ProcessClosing();
        if (!g_pStorageInventory->ProcessClosing())
            return;
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
        if (IsVisible(INTERFACE_INVENTORY_EXT))
        {
            Hide(INTERFACE_INVENTORY_EXT);
        }
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY, false);
        Show(INTERFACE_MU_HELPER_BAR);
    }
    else if (dwKey == INTERFACE_STORAGE_EXT)
    {
        Show(INTERFACE_MU_HELPER_BAR);
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

        m_pNewMyQuestInfoWindow->SetPos(PanelColumnX(1), 0);
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
        if (IsVisible(INTERFACE_INVENTORY_EXT))
        {
            Hide(INTERFACE_INVENTORY_EXT);
        }
        g_pTrade->ProcessClosing();
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_MYINVEN, false);
        m_pNewUIMng->ShowInterface(INTERFACE_INVENTORY, false);
        Show(INTERFACE_MU_HELPER_BAR);
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
    else if (dwKey == INTERFACE_COMMAND_LIST)
    {
        m_pNewChatCommandWindow->ClosingProcess();
    }
    else if (dwKey == INTERFACE_WINDOW_MENU)
    {
        g_pMainFrame->SetBtnState(MAINFRAME_BTN_WINDOW, false);
    }
    else if (dwKey == INTERFACE_OPTION)
    {
    }
    else if (dwKey == INTERFACE_MU_HELPER_BAR)
    {
        m_pMuHelperBar->ClosingProcess();
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
        m_pBuffStrip->ClosingProcess();
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
    else if (dwKey == mu::ui::window::INTERFACE_LUCKYITEMWND)
    {
        if (g_pLuckyItemWnd->ClosingProcess() == false)
            return;
        if (IsVisible(mu::ui::window::INTERFACE_INVENTORY))
        {
            m_pNewMyInventory->ClosingProcess();
            m_pNewUIMng->ShowInterface(mu::ui::window::INTERFACE_INVENTORY, false);
        }
    }
    else if (dwKey == INTERFACE_MUHELPER)
    {
        m_pNewUIMng->ShowInterface(mu::ui::window::INTERFACE_MUHELPER_SKILL_LIST, false);
        m_pNewUIMng->ShowInterface(mu::ui::window::INTERFACE_MUHELPER_EXT, false);
    }

    m_pNewUIMng->ShowInterface(dwKey, false);

    UpdateMuHelperBarVisibilityForLayoutChange(dwKey);

    int iScreenWidth = GetScreenWidth();
    m_pNewItemEnduranceInfo->SetPos(iScreenWidth);
    m_pBuffStrip->SetPos(iScreenWidth);
    m_pNewPartyListWindow->SetPos(iScreenWidth);
}

void CSystem::Toggle(DWORD dwKey)
{
    IsVisible(dwKey) ? Hide(dwKey) : Show(dwKey);
}

void CSystem::HideAll()
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

void CSystem::HideAllGroupA()
{
    Hide(INTERFACE_INVENTORY);
    Hide(INTERFACE_CHARACTER);

    DWORD dwGroupA[] = {
        // mu::ui::window::INTERFACE_INVENTORY,
        // mu::ui::window::INTERFACE_CHARACTER,
        // mu::ui::window::INTERFACE_WINDOW_MENU,
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
        INTERFACE_COMMAND_LIST,
        INTERFACE_GUILDINFO,
        INTERFACE_KANTURU2ND_ENTERNPC,
        INTERFACE_DUELWATCH,
        INTERFACE_DOPPELGANGER_NPC,
        // mu::ui::window::INTERFACE_HELP,
        // mu::ui::window::INTERFACE_ITEM_EXPLANATION,
        // mu::ui::window::INTERFACE_SETITEM_EXPLANATION,
        INTERFACE_GOLD_BOWMAN,
        INTERFACE_GOLD_BOWMAN_LENA,
        INTERFACE_NPC_DIALOGUE,
        INTERFACE_QUEST_PROGRESS,
        INTERFACE_QUEST_PROGRESS_ETC,
        INTERFACE_EMPIREGUARDIAN_NPC,
#ifdef PBG_MOD_STAMINA_UI
        mu::ui::window::INTERFACE_STAMINA_GAUGE,
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
        INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING
        INTERFACE_UNITEDMARKETPLACE_NPC_JULIA,

        mu::ui::window::INTERFACE_LUCKYITEMWND,

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

void CSystem::HideAllGroupB()
{
    Hide(INTERFACE_FRIEND);
    Hide(INTERFACE_INVENTORY);
    Hide(INTERFACE_CHARACTER);

    DWORD dwGroupB[] = {
        // mu::ui::window::INTERFACE_FRIEND,
        // mu::ui::window::INTERFACE_INVENTORY,
        // mu::ui::window::INTERFACE_CHARACTER,
        // mu::ui::window::INTERFACE_WINDOW_MENU,

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
        INTERFACE_COMMAND_LIST,
        INTERFACE_GUILDINFO,
        INTERFACE_KANTURU2ND_ENTERNPC,
        INTERFACE_CURSEDTEMPLE_NPC,
        INTERFACE_DUELWATCH,
        INTERFACE_DOPPELGANGER_NPC,
        // mu::ui::window::INTERFACE_HELP,
        // mu::ui::window::INTERFACE_ITEM_EXPLANATION,
        // mu::ui::window::INTERFACE_SETITEM_EXPLANATION,
        INTERFACE_GOLD_BOWMAN,
        INTERFACE_GOLD_BOWMAN_LENA,
        INTERFACE_NPC_DIALOGUE,
        INTERFACE_QUEST_PROGRESS,
        INTERFACE_QUEST_PROGRESS_ETC,
        INTERFACE_EMPIREGUARDIAN_NPC,
#ifdef PBG_MOD_STAMINA_UI
        mu::ui::window::INTERFACE_STAMINA_GAUGE,
#endif //PBG_MOD_STAMINA_UI
#ifdef PBG_ADD_GENSRANKING
        INTERFACE_GENSRANKING,
#endif //PBG_ADD_GENSRANKING
        INTERFACE_UNITEDMARKETPLACE_NPC_JULIA,
        mu::ui::window::INTERFACE_LUCKYITEMWND,

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
void CSystem::HideGroupBeforeOpenInterface()
{
    DWORD dwGroupC[] = {
        INTERFACE_PARTY,
        INTERFACE_COMMAND,
        INTERFACE_COMMAND_LIST,
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

void CSystem::SyncMainSceneHudVisibility()
{
    extern EGameScene SceneFlag;
    // LoadingWorld < 30, not just SceneFlag == MAIN_SCENE: SceneFlag flips to MAIN_SCENE the
    // instant LoadingScene()'s one-frame flash ends (LoadingScene.cpp), well before the world/
    // hero data the server sends back is actually ready -- LoadingWorld stays >= 30 for that
    // whole gap (set to 9999999 on scene entry, dropped to ~30-50 once the server confirms
    // placement, then counts down to 0). MainScene.cpp's own UpdateUIAndInput() already treats
    // LoadingWorld >= 30 as "not really in MAIN_SCENE yet" and skips input/Update() for exactly
    // this reason; this is the same gate applied to the persistent RmlUi HUD documents' own
    // Show()/Hide(), which otherwise render every frame regardless of Update() ever running.
    extern int LoadingWorld;
    const bool sceneAllowsShow = (SceneFlag == MAIN_SCENE) && (LoadingWorld < 30);

    if (m_pMuHelperBar)
        m_pMuHelperBar->SyncDocVisibility(sceneAllowsShow);
    if (m_pBuffStrip)
        m_pBuffStrip->SyncDocVisibility(sceneAllowsShow);
    if (m_pNewMainFrameWindow)
        m_pNewMainFrameWindow->SyncDocVisibility(sceneAllowsShow);
}

void CSystem::UpdateMuHelperBarVisibilityForLayoutChange(DWORD dwKey)
{
    if (IsHeroPositionLayoutInterface(dwKey))
    {
        SyncMuHelperBarVisibility();
    }
}

void CSystem::SyncMuHelperBarVisibility()
{
    if (!m_pNewUIMng)
    {
        return;
    }

    m_pNewUIMng->ShowInterface(INTERFACE_MU_HELPER_BAR, !ShouldHideMuHelperBar());
}

bool CSystem::ShouldHideMuHelperBar()
{
    if (!m_pNewUIMng)
    {
        return false;
    }

    if (IsVisible(INTERFACE_STORAGE_EXT))
    {
        return true;
    }

    if (!IsVisible(INTERFACE_INVENTORY_EXT))
    {
        return false;
    }

    return IsVisible(INTERFACE_CHARACTER)
        || IsVisible(INTERFACE_STORAGE)
        || IsVisible(INTERFACE_MYSHOP_INVENTORY)
        || IsVisible(INTERFACE_NPCSHOP)
        || IsVisible(INTERFACE_MIXINVENTORY)
        || IsVisible(INTERFACE_TRADE);
}

void CSystem::Enable(DWORD dwKey)
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->EnableInterface(dwKey);
    }
}

void CSystem::Disable(DWORD dwKey)
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->EnableInterface(dwKey, false);
    }
}

bool CSystem::CheckMouseUse()
{
    if (m_mouseInputCaptured)
    {
        return true;
    }

    if (m_pNewUIMng)
    {
        if (m_pNewUIMng->GetActiveMouseUIObj())
            return true;
    }
    return false;
}

bool CSystem::CheckKeyUse()
{
    if (m_pNewUIMng)
    {
        if (m_pNewUIMng->GetActiveKeyUIObj())
            return true;
    }
    return false;
}

bool CSystem::HandleFrameCornerClose(const POINT& winPos, DWORD dwKey)
{
    // Box of the corner glyph in the shared 190-wide frame. Matches the MU Helper
    // close "X" exactly (13x12 anchored at +169,+7) — the same hit-box the
    // per-window copies used originally, so the click feel is identical across
    // every window. One place to tune for every window that uses this frame.
    constexpr int X_OFFSET = 169, Y_OFFSET = 7, WIDTH = 13, HEIGHT = 12;

    if (IsPress(VK_LBUTTON)
        && CheckMouseIn(winPos.x + X_OFFSET, winPos.y + Y_OFFSET, WIDTH, HEIGHT))
    {
        Hide(dwKey);
        // Clear the raw button state: world movement reads MouseLButtonPush
        // directly (not the UI consume result), so without this the click falls
        // through and walks the character.
        MouseLButton = false;
        MouseLButtonPop = false;
        MouseLButtonPush = false;
        return true;
    }
    return false;
}

bool CSystem::Update()
{
    if (m_pNewItemMng)
    {
        m_pNewItemMng->Update();
    }

    bool result = false;
    if (m_pNewUIMng)
    {
        if (!MouseLButton)
        {
            m_mouseInputCaptured = false;
        }
        else if (m_pNewUIMng->GetActiveMouseUIObj())
        {
            m_mouseInputCaptured = true;
        }

        m_pNewUIMng->UpdateMouseEvent();

        if (MouseLButton && m_pNewUIMng->GetActiveMouseUIObj())
        {
            m_mouseInputCaptured = true;
        }

        m_pNewUIMng->UpdateKeyEvent();
        result = m_pNewUIMng->Update();
    }

    return result;
}

bool CSystem::Render()
{
    bool bResult = false;

    BeginBitmap();
    if (m_pNewUIMng)
        bResult = m_pNewUIMng->Render();
    EndBitmap();

    return bResult;
}

CManager* CSystem::GetNewUIManager() const
{
    return m_pNewUIMng;
}

C3DRenderMng* CSystem::GetNewUI3DRenderMng() const
{
    return m_pNewUI3DRenderMng;
}

CHotKey* CSystem::GetNewUIHotKey() const
{
    return m_pNewUIHotKey;
}

bool CSystem::IsImpossibleSendMoveInterface()
{
    if (IsVisible(INTERFACE_MIXINVENTORY)
        || IsVisible(INTERFACE_KANTURU2ND_ENTERNPC)
        || IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND)
        )
    {
        return true;
    }

    return false;
}

bool CSystem::IsImpossibleTradeInterface()
{
    if (IsVisible(INTERFACE_MIXINVENTORY)
        || IsVisible(INTERFACE_KANTURU2ND_ENTERNPC)
        || IsVisible(INTERFACE_STORAGE)
        || IsVisible(INTERFACE_INGAMESHOP)
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        || IsVisible(INTERFACE_INGAMESHOP)
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
        || IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND)
        )
    {
        return true;
    }

    return false;
}

bool CSystem::IsImpossibleDuelInterface()
{
    if (IsVisible(INTERFACE_MIXINVENTORY)
        || IsVisible(INTERFACE_KANTURU2ND_ENTERNPC)
        || IsVisible(INTERFACE_STORAGE)
        || IsVisible(INTERFACE_INGAMESHOP)
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        || IsVisible(INTERFACE_INGAMESHOP)
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME
        || IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND)
        )
    {
        return true;
    }

    return false;
}

bool CSystem::IsImpossibleHideInterface(DWORD dwKey)
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
        || dwKey == INTERFACE_MU_HELPER_BAR
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

void CSystem::UpdateSendMoveInterface()
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
CSystem* CSystem::GetInstance()
{
    static CSystem s_NewUISystem;
    return &s_NewUISystem;
}

CChatLogWindow* CSystem::GetUI_NewChatLogWindow() const
{
    return m_pNewChatLogWindow;
}

CSystemLogWindow* CSystem::GetUI_NewSystemLogWindow() const
{
    return m_pNewSystemLogWindow;
}

CSlideWindow* CSystem::GetUI_NewSlideWindow() const
{
    return m_pNewSlideWindow;
}

CFriendWindow* CSystem::GetUI_NewFriendWindow() const
{
    return m_pNewFriendWindow;
}

CMainFrameWindow* CSystem::GetUI_NewMainFrameWindow() const
{
    return m_pNewMainFrameWindow;
}

CSkillList* CSystem::GetUI_NewSkillList() const
{
    return m_pNewSkillList;
}

CChatInputBox* CSystem::GetUI_NewChatInputBox() const
{
    return m_pNewChatInputBox;
}

CItemMng* CSystem::GetUI_NewItemMng() const
{
    return m_pNewItemMng;
}

CMyInventory* CSystem::GetUI_NewMyInventory() const
{
    return m_pNewMyInventory;
}

CInventoryExtension* CSystem::GetUI_NewMyInventoryExt() const
{
    return m_pNewMyInventoryExt;
}

CNPCShop* CSystem::GetUI_NewNpcShop() const
{
    return m_pNewNPCShop;
}

CPetInfoWindow* CSystem::GetUI_NewPetInfoWindow() const
{
    return m_pNewPetInfoWindow;
}

CMixInventory* CSystem::GetUI_NewMixInventory() const
{
    return m_pNewMixInventory;
}

CCastleWindow* CSystem::GetUI_NewCastleWindow() const
{
    return m_pNewCastleWindow;
}

CGuardWindow* CSystem::GetUI_NewGuardWindow() const
{
    return m_pNewGuardWindow;
}

CGatemanWindow* CSystem::GetUI_NewGatemanWindow() const
{
    return m_pNewGatemanWindow;
}

CGateSwitchWindow* CSystem::GetUI_NewGateSwitchWindow() const
{
    return m_pNewGateSwitchWindow;
}

CStorageInventory* CSystem::GetUI_NewStorageInventory() const
{
    return m_pNewStorageInventory;
}

CStorageInventoryExt* CSystem::GetUI_NewStorageInventoryExt() const
{
    return m_pNewStorageInventoryExt;
}

CGuildMakeWindow* CSystem::GetUI_NewGuildMakeWindow() const
{
    return m_pNewGuildMakeWindow;
}

CGuildInfoWindow* CSystem::GetUI_NewGuildInfoWindow() const
{
    return m_pNewGuildInfoWindow;
}

CCryWolf* CSystem::GetUI_NewCryWolfInterface() const
{
    return m_pNewCryWolfInterface;
}

CMasterLevel* CSystem::GetUI_NewMasterLevelInterface() const
{
    return m_pNewMaster_Level_Interface;
}

CMyShopInventory* CSystem::GetUI_NewMyShopInventory() const
{
    return m_pNewMyShopInventory;
}

CPurchaseShopInventory* CSystem::GetUI_NewPurchaseShopInventory() const
{
    return m_pNewPurchaseShopInventory;
}

CCharacterInfoWindow* CSystem::GetUI_NewCharacterInfoWindow() const
{
    return m_pNewCharacterInfoWindow;
}

CMyQuestInfoWindow* CSystem::GetUI_NewMyQuestInfoWindow() const
{
    return m_pNewMyQuestInfoWindow;
}

CPartyInfoWindow* CSystem::GetUI_NewPartyInfoWindow() const
{
    return m_pNewPartyInfoWindow;
}

CPartyListWindow* CSystem::GetUI_NewPartyListWindow() const
{
    return m_pNewPartyListWindow;
}

CNPCQuest* CSystem::GetUI_NewNPCQuest() const
{
    return m_pNewNPCQuest;
}

CEnterBloodCastle* CSystem::GetUI_NewEnterBloodCastle() const
{
    return m_pNewEnterBloodCastle;
}

CEnterDevilSquare* CSystem::GetUI_NewEnterDevilSquare() const
{
    return m_pNewEnterDevilSquare;
}

CBloodCastle* CSystem::GetUI_NewBloodCastle() const
{
    return m_pNewBloodCastle;
}

CTrade* CSystem::GetUI_NewTrade() const
{
    return m_pNewTrade;
}

CKanturu2ndEnterNpc* CSystem::GetUI_NewKanturu2ndEnterNpc() const
{
    return m_pNewKanturu2ndEnterNpc;
}

CKanturuInfoWindow* CSystem::GetUI_NewKanturuInfoWindow() const
{
    return m_pNewKanturuInfoWindow;
}

CCatapultWindow* CSystem::GetUI_NewCatapultWindow() const
{
    return m_pNewCatapultWindow;
}

CChaosCastleTime* CSystem::GetUI_NewChaosCastleTime() const
{
    return m_pNewChaosCastleTime;
}

CCommandWindow* CSystem::GetUI_NewCommandWindow() const
{
    return m_pNewCommandWindow;
}

CWindowMenu* CSystem::GetUI_NewWindowMenu() const
{
    return m_pNewWindowMenu;
}

COptionWindow* CSystem::GetUI_NewOptionWindow() const
{
    return m_pNewOptionWindow;
}

CMuHelperBar* CSystem::GetUI_MuHelperBar() const
{
    return m_pMuHelperBar;
}

CHelpWindow* CSystem::GetUI_NewHelpWindow() const
{
    return m_pNewHelpWindow;
}

CChatCommandWindow* CSystem::GetUI_NewChatCommandWindow() const
{
    return m_pNewChatCommandWindow;
}

CItemExplanationWindow* CSystem::GetUI_NewItemExplanationWindow() const
{
    return m_pNewItemExplanationWindow;
}

CSetItemExplanation* CSystem::GetUI_NewSetItemExplanation() const
{
    return m_pNewSetItemExplanation;
}

CQuickCommandWindow* CSystem::GetUI_NewQuickCommandWindow() const
{
    return m_pNewQuickCommandWindow;
}

CMoveCommandWindow* CSystem::GetUI_NewMoveCommandWindow() const
{
    return m_pNewMoveCommandWindow;
}

CBattleSoccerScore* CSystem::GetUI_NewBattleSoccerScore() const
{
    return m_pNewBattleSoccerScore;
}

CDuelWindow* CSystem::GetUI_NewDuelWindow() const
{
    return m_pNewDuelWindow;
}

CSiegeWarfare* CSystem::GetUI_NewSiegeWarfare() const
{
    return m_pNewSiegeWarfare;
}

CItemEnduranceInfo* CSystem::GetUI_NewItemEnduranceInfo() const
{
    return m_pNewItemEnduranceInfo;
}

CBuffStrip* CSystem::GetUI_BuffStrip() const
{
    return m_pBuffStrip;
}

CCursedTempleEnter* CSystem::GetUI_NewCursedTempleEnterWindow() const
{
    return m_pNewCursedTempleEnterWindow;
}

CCursedTempleSystem* CSystem::GetUI_NewCursedTempleWindow() const
{
    return m_pNewCursedTempleWindow;
}

CCursedTempleResult* CSystem::GetUI_NewCursedTempleResultWindow() const
{
    return m_pNewCursedTempleResultWindow;
}

CGoldBowmanWindow* CSystem::GetUI_pNewGoldBowman() const
{
    return m_pNewGoldBowman;
}

CGoldBowmanLena* CSystem::GetUI_pNewGoldBowmanLena() const
{
    return m_pNewGoldBowmanLena;
}

CRegistrationLuckyCoin* CSystem::GetUI_pNewLuckyCoinRegistration() const
{
    return m_pNewLuckyCoinRegistration;
}

CExchangeLuckyCoin* CSystem::GetUI_pNewExchangeLuckyCoin() const
{
    return m_pNewExchangeLuckyCoinWindow;
}

CMiniMap* CSystem::GetUI_pNewUIMiniMap() const
{
    return m_pNewMiniMap;
}

CDuelWatchWindow* CSystem::GetUI_pNewDuelWatch() const
{
    return m_pNewDuelWatchWindow;
}

CDuelWatchMainFrameWindow* CSystem::GetUI_pNewDuelWatchMainFrame() const
{
    return m_pNewDuelWatchMainFrameWindow;
}

CDuelWatchUserListWindow* CSystem::GetUI_pNewDuelWatchUserList() const
{
    return m_pNewDuelWatchUserListWindow;
}

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
CInGameShop* CSystem::GetUI_pNewInGameShop() const
{
    return m_pNewInGameShop;
}
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

CDoppelGangerWindow* CSystem::GetUI_pNewDoppelGangerWindow() const
{
    return m_pNewDoppelGangerWindow;
}

CDoppelGangerFrame* CSystem::GetUI_pNewDoppelGangerFrame() const
{
    return m_pNewDoppelGangerFrame;
}

CNPCDialogue* CSystem::GetUI_NewNPCDialogue() const
{
    return m_pNewNPCDialogue;
}

CQuestProgress* CSystem::GetUI_NewQuestProgress() const
{
    return m_pNewQuestProgress;
}

CQuestProgressByEtc* CSystem::GetUI_NewQuestProgressByEtc() const
{
    return m_pNewQuestProgressByEtc;
}

CEmpireGuardianNPC* CSystem::GetUI_pNewEmpireGuardianNPC() const
{
    return m_pNewEmpireGuardianNPC;
}

CEmpireGuardianTimer* CSystem::GetUI_pNewEmpireGuardianTimer() const
{
    return m_pNewEmpireGuardianTimer;
}

#ifdef PBG_MOD_STAMINA_UI
CNewUIStamina* mu::ui::window::CSystem::GetUI_pNewUIStamina() const
{
    return m_pNewUIStamina;
}
#endif //PBG_MOD_STAMINA_UI

#ifdef PBG_ADD_GENSRANKING
CGensRanking* CSystem::GetUI_NewGensRanking() const
{
    return m_pNewGensRanking;
}
#endif //PBG_ADD_GENSRANKING

CUnitedMarketPlaceWindow* CSystem::GetUI_pNewUnitedMarketPlaceWindow() const
{
    return m_pNewUnitedMarketPlaceWindow;
}

CLuckyItemWnd* mu::ui::window::CSystem::Get_pNewUILuckyItemWnd() const
{
    return m_pNewUILuckyItemWnd;
}

CUIMuHelper* CSystem::Get_pNewUIMuHelper() const
{
    return m_pNewUIMuHelper;
}

CMuHelperExt* CSystem::Get_pNewUIMuHelperExt() const
{
    return m_pNewUIMuHelperExt;
}

CMuHelperSkillList* CSystem::Get_pNewUIMuHelperSkillList() const
{
    return m_pNewUIMuHelperSkillList;
}
