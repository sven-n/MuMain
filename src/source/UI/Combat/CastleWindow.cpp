
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Combat/CastleWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzCharacter.h"

#include "Audio/DSPlaySound.h"
#include "GameLogic/Items/MixMgr.h"
#include "GameLogic/Events/SenatusInfo.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cmath>
#include "Render/Text/CUIRenderText.h"

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// A RenderColorQuadARGB() colour (alpha in the high byte) as a CSS colour.
Rml::String ArgbToCss(unsigned int argb)
{
    return "rgba(" + std::to_string((argb >> 16) & 0xFF) + ", " + std::to_string((argb >> 8) & 0xFF) + ", " +
           std::to_string(argb & 0xFF) + ", " + std::to_string((argb >> 24) & 0xFF) + ")";
}
} // namespace

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCastleWindow::CCastleWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iNumCurOpenTab = TAB_GATE_MANAGING;
    m_iCurrMsgBoxRequest = CASTLE_MSGREQ_NULL;
}
CCastleWindow::~CCastleWindow() { Release(); }

bool CCastleWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SENATUS, this);

    SetPos(x, y);

    SetCurOpenTab(m_iNumCurOpenTab);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CCastleWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CCastleWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

// The page and the tab highlight read the same value.
void CCastleWindow::SetCurOpenTab(int iTab)
{
    m_iNumCurOpenTab = iTab;
}

bool CCastleWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    // #panel's own live RCSS size is the source of truth -- INVENTORY_WIDTH/HEIGHT only cover the
    // first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = INVENTORY_WIDTH;
    float panelHeight = INVENTORY_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth),
                                      static_cast<int>(panelHeight))
            .Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CCastleWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_SENATUS) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_SENATUS);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

bool CCastleWindow::Update()
{
    // A button RmlUi reported (the original's CButton handling in UpdateMouseEvent()).
    const SENATUS_BUTTON button = m_PendingButton;
    m_PendingButton = SENATUS_BUTTON_NONE;
    if (IsVisible() && button == SENATUS_BUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_SENATUS);
    }
    else if (IsVisible() && button != SENATUS_BUTTON_NONE && !ButtonLocked(button))
    {
        switch (m_iNumCurOpenTab)
        {
        case TAB_GATE_MANAGING:
            UpdateGateManagingTab(button);
            break;
        case TAB_STATUE_MANAGING:
            UpdateStatueManagingTab(button);
            break;
        case TAB_TAX_MANAGING:
            UpdateTaxManagingTab(button);
            break;
        }
    }

    const int pick = m_PendingPick;
    m_PendingPick = -1;
    if (IsVisible() && pick >= 0)
    {
        if (m_iNumCurOpenTab == TAB_GATE_MANAGING && pick < 6)
            g_SenatusInfo.SetCurrGate(pick);
        else if (m_iNumCurOpenTab == TAB_STATUE_MANAGING && pick < 4)
            g_SenatusInfo.SetCurrStatue(pick);
    }

    const int tab = m_PendingTab;
    m_PendingTab = -1;
    if (IsVisible() && tab >= TAB_GATE_MANAGING && tab <= TAB_CASTLE_MIX)
    {
        SetCurOpenTab(tab);
        if (tab == TAB_CASTLE_MIX)
        {
            g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_CASTLE_SENIOR);
            g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);
        }
    }

    SyncRmlModel();
    return true;
}

bool CCastleWindow::Render()
{
    // Nothing native left: the frame, the tabs, the pages and the buttons are RmlUi. Kept
    // because CObject requires the override.
    return true;
}

void CCastleWindow::OpeningProcess()
{
    SetCurOpenTab(TAB_GATE_MANAGING);

    g_SenatusInfo.SetCurrGate(0);
    g_SenatusInfo.SetCurrStatue(0);

    SocketClient->ToGameServer()->SendCastleSiegeGateListRequest();
    SocketClient->ToGameServer()->SendCastleSiegeStatueListRequest();
    SocketClient->ToGameServer()->SendCastleSiegeTaxInfoRequest();
}

void CCastleWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
}

float CCastleWindow::GetLayerDepth()
{
    return 5.0f;
}

bool CCastleWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_SENATUS);

    return false;
}

bool CCastleWindow::ButtonLocked(SENATUS_BUTTON button) const
{
    // A locked CButton ignored clicks; the lock is the one the page was last drawn with.
    const CastleWindowRmlModel& model = m_RmlBinder.GetModel();
    switch (button)
    {
    case SENATUS_BUTTON_BUY:
        return model.buyButton.locked;
    case SENATUS_BUTTON_REPAIR:
        return model.repairButton.locked;
    case SENATUS_BUTTON_UPGRADE_HP:
        return model.hpButton.locked;
    case SENATUS_BUTTON_UPGRADE_DEFENSE:
        return model.defenseButton.locked;
    case SENATUS_BUTTON_UPGRADE_RECOVER:
        return model.recoverButton.locked;
    case SENATUS_BUTTON_APPLY_TAX:
        return model.applyButton.locked;
    case SENATUS_BUTTON_WITHDRAW:
        return model.withdrawButton.locked;
    default:
        return false;
    }
}

namespace
{
    void ExecuteCastleMsgBoxRequest()
    {
        switch (g_pCastleWindow->GetCurrMsgBoxRequest())
        {
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_BUY_GATE:
            g_SenatusInfo.DoGateRepairAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_REPAIR_GATE:
            g_SenatusInfo.DoGateRepairAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_UPGRADE_GATE_HP:
            g_SenatusInfo.DoGateUpgradeHPAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_UPGRADE_GATE_DEFENSE:
            g_SenatusInfo.DoGateUpgradeDefenseAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_BUY_STATUE:
            g_SenatusInfo.DoStatueRepairAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_REPAIR_STATUE:
            g_SenatusInfo.DoStatueRepairAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_UPGRADE_STATUE_HP:
            g_SenatusInfo.DoStatueUpgradeHPAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_UPGRADE_STATUE_DEFENSE:
            g_SenatusInfo.DoStatueUpgradeDefenseAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_UPGRADE_STATUE_RECOVER:
            g_SenatusInfo.DoStatueUpgradeRecoverAction();
            break;
        case mu::ui::window::CCastleWindow::CASTLE_MSGREQ_APPLY_TAX:
            g_SenatusInfo.DoApplyTaxAction();
            break;
        default:
            break;
        }
    }
}

void CCastleWindow::UpdateGateManagingTab(SENATUS_BUTTON button)
{
    wchar_t szText[256] = { 0, };
    if (button == SENATUS_BUTTON_BUY)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_BUY_GATE);
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        mu_swprintf(szText, I18N::Game::DZenIsRequired, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrGateInfo()));
        InsertComma(szText, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrGateInfo()));
        cfg.lines = {
            { I18N::Game::ToPurchaseSelectedCastleGate, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToPurchase, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_REPAIR)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_REPAIR_GATE);
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        mu_swprintf(szText, I18N::Game::DZenIsRequired, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrGateInfo()));
        InsertComma(szText, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrGateInfo()));
        cfg.lines = {
            { I18N::Game::ToRepairSelectedCastleGate, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToPurchase, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_UPGRADE_HP)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_GATE_HP);

        if (g_SenatusInfo.GetHPLevel(&g_SenatusInfo.GetCurrGateInfo()) == 0)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 2, 1000000);
        else if (g_SenatusInfo.GetHPLevel(&g_SenatusInfo.GetCurrGateInfo()) == 1)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 3, 1000000);
        else
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 4, 1000000);
        InsertComma(szText, 1000000);
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::UpgradingTheDurabilityOfSelectedCastleGate, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToRepair, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_UPGRADE_DEFENSE)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_GATE_DEFENSE);

        if (g_SenatusInfo.GetDefenseLevel(&g_SenatusInfo.GetCurrGateInfo()) == 0)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 2, 3000000);
        else if (g_SenatusInfo.GetDefenseLevel(&g_SenatusInfo.GetCurrGateInfo()) == 1)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 3, 3000000);
        else
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 4, 3000000);
        InsertComma(szText, 3000000);
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::UpgradingTheDefensivePowerOfSelectedCastleGate, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToRepair, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}

void CCastleWindow::UpdateStatueManagingTab(SENATUS_BUTTON button)
{
    wchar_t szText[256] = { 0, };
    if (button == SENATUS_BUTTON_BUY)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_BUY_STATUE);
        mu_swprintf(szText, I18N::Game::DZenIsRequired, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrStatueInfo()));
        InsertComma(szText, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrStatueInfo()));
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::ToPurchaseSelectedStatue, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToPurchase, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_REPAIR)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_REPAIR_STATUE);
        mu_swprintf(szText, I18N::Game::DZenIsRequired, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrStatueInfo()));
        InsertComma(szText, g_SenatusInfo.GetRepairCost(&g_SenatusInfo.GetCurrStatueInfo()));
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::ToRepairSelectedStatue, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToRepair, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_UPGRADE_HP)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_STATUE_HP);

        if (g_SenatusInfo.GetHPLevel(&g_SenatusInfo.GetCurrStatueInfo()) == 0)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 3, 1000000);
        else if (g_SenatusInfo.GetHPLevel(&g_SenatusInfo.GetCurrStatueInfo()) == 1)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 5, 1000000);
        else
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 7, 1000000);
        InsertComma(szText, 1000000);
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::UpgradingDurabilityOfSelectedCastleGate, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToRepair, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_UPGRADE_DEFENSE)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_STATUE_DEFENSE);

        if (g_SenatusInfo.GetDefenseLevel(&g_SenatusInfo.GetCurrStatueInfo()) == 0)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 3, 3000000);
        else if (g_SenatusInfo.GetDefenseLevel(&g_SenatusInfo.GetCurrStatueInfo()) == 1)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 5, 3000000);
        else
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 7, 3000000);
        InsertComma(szText, 3000000);
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::UpgradingDefensivePowerOfSelectedStatue, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToRepair, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_UPGRADE_RECOVER)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_UPGRADE_STATUE_RECOVER);

        if (g_SenatusInfo.GetRecoverLevel(&g_SenatusInfo.GetCurrStatueInfo()) == 0)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 3, 5000000);
        else if (g_SenatusInfo.GetRecoverLevel(&g_SenatusInfo.GetCurrStatueInfo()) == 1)
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 5, 5000000);
        else
            mu_swprintf(szText, I18N::Game::DGuardianJewelAndDZenAreRequired, 7, 5000000);
        InsertComma(szText, 5000000);
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::UpgradingRecoveryPowerOfSelectedStatue, false },
            { szText, false },
            { I18N::Game::WouldYouLikeToRepair, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}

void CCastleWindow::UpdateTaxManagingTab(SENATUS_BUTTON button)
{
    wchar_t szText[256] = { 0, };
    if (button == SENATUS_BUTTON_APPLY_TAX)
    {
        SetCurrMsgBoxRequest(CASTLE_MSGREQ_APPLY_TAX);
        wchar_t szChaosTaxText[256] = { 0, };
        mu_swprintf(szChaosTaxText, I18N::Game::ChaosCombinationGoblinTaxRateD, g_SenatusInfo.GetChaosTaxRate());
        mu_swprintf(szText, I18N::Game::VariousNPCTaxRateD, g_SenatusInfo.GetNormalTaxRate());
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { szChaosTaxText, false },
            { szText, false },
            { I18N::Game::Apply1568, false },
        };
        cfg.onPrimary = [] { ExecuteCastleMsgBoxRequest(); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_WITHDRAW)
    {
        // Numeric Mode::Text amount entry. Reads its own typed amount directly
        // (GetInputText()), not via ExecuteCastleMsgBoxRequest()'s generic switch -- that helper
        // only covers plain OK/Cancel castle dialogs, none of which need input.
        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::EnterTheWithdrawalAmount, false },
            { I18N::Game::Maximum15000000Zen, false },
        };
        cfg.input = mu::ui::window::GenericDialogConfig::InputField{};
        cfg.input->mode = mu::ui::window::GenericDialogConfig::InputField::Mode::Text;
        cfg.input->maxLength = 8;
        cfg.input->numericOnly = true;
        cfg.onPrimary = []
        {
            const std::wstring strText = mu::ui::window::g_pGenericConfirmDialog->GetInputText();
            const DWORD dwInputZen = strText.empty() ? 0 : static_cast<DWORD>(_wtoi(strText.c_str()));
            if (dwInputZen == 0)
            {
                mu::ui::window::g_pGenericConfirmDialog->KeepOpen();
                return;
            }
            g_SenatusInfo.DoWithdrawAction(dwInputZen);
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    else if (button == SENATUS_BUTTON_CHAOS_TAX_UP)
    {
        g_SenatusInfo.PlusChaosTaxRate(1);
    }
    else if (button == SENATUS_BUTTON_CHAOS_TAX_DOWN)
    {
        g_SenatusInfo.PlusChaosTaxRate(-1);
    }
    else if (button == SENATUS_BUTTON_NPC_TAX_UP)
    {
        g_SenatusInfo.PlusNormalTaxRate(1);
    }
    else if (button == SENATUS_BUTTON_NPC_TAX_DOWN)
    {
        g_SenatusInfo.PlusNormalTaxRate(-1);
    }
}

void CCastleWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "castle_window",
        [this](Rml::DataModelConstructor& c, CastleWindowRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("line_height_px", &model.lineHeightPx);
            c.Bind("button_label_top", &model.buttonLabelTop);
            c.Bind("tab_label_top", &model.tabLabelTop);
            auto tab = c.RegisterStruct<CastleTabEntry>();
            tab.RegisterMember("label", &CastleTabEntry::label);
            tab.RegisterMember("text_px", &CastleTabEntry::textPx);
            tab.RegisterMember("selected", &CastleTabEntry::selected);
            c.RegisterArray<std::vector<CastleTabEntry>>();
            c.Bind("tabs", &model.tabs);
            c.Bind("active_tab", &model.activeTab);
            c.BindEventCallback("senatus_tab",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                                {
                                    if (args.size() == 1)
                                        m_PendingTab = args[0].Get<int>(-1);
                                });
            c.BindEventCallback("senatus_pick",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                                {
                                    if (args.size() == 1)
                                        m_PendingPick = args[0].Get<int>(-1);
                                });
            auto lineType = c.RegisterStruct<CastleLine>();
            lineType.RegisterMember("text", &CastleLine::text);
            lineType.RegisterMember("text_px", &CastleLine::textPx);
            c.Bind("title", &model.title);
            c.Bind("map_title", &model.mapTitle);
            c.Bind("improve_title", &model.improveTitle);
            c.Bind("stat_hp", &model.statHp);
            c.Bind("stat_defense", &model.statDefense);
            c.Bind("stat_recover", &model.statRecover);
            c.Bind("next_hp", &model.nextHp);
            c.Bind("next_defense", &model.nextDefense);
            c.Bind("next_recover", &model.nextRecover);
            c.Bind("tax_title", &model.taxTitle);
            c.Bind("chaos_rate", &model.chaosRate);
            c.Bind("store_rate", &model.storeRate);
            c.Bind("note1", &model.note1);
            c.Bind("note2", &model.note2);
            c.Bind("note3", &model.note3);
            c.Bind("rule1", &model.rule1);
            c.Bind("rule2", &model.rule2);
            c.Bind("rule3", &model.rule3);
            c.Bind("rule4", &model.rule4);
            c.Bind("rule5", &model.rule5);
            c.Bind("rule6", &model.rule6);
            c.Bind("zen_label", &model.zenLabel);
            c.Bind("castle_money", &model.castleMoney);
            c.Bind("footer1", &model.footer1);
            c.Bind("footer2", &model.footer2);
            c.Bind("footer3", &model.footer3);
            auto actionButton = c.RegisterStruct<CastleActionButton>();
            actionButton.RegisterMember("label", &CastleActionButton::label);
            actionButton.RegisterMember("shown", &CastleActionButton::shown);
            actionButton.RegisterMember("locked", &CastleActionButton::locked);
            c.Bind("buy_button", &model.buyButton);
            c.Bind("repair_button", &model.repairButton);
            c.Bind("hp_button", &model.hpButton);
            c.Bind("defense_button", &model.defenseButton);
            c.Bind("recover_button", &model.recoverButton);
            c.Bind("apply_button", &model.applyButton);
            c.Bind("withdraw_button", &model.withdrawButton);
            auto mapItem = c.RegisterStruct<CastleMapItem>();
            mapItem.RegisterMember("statue", &CastleMapItem::statue);
            mapItem.RegisterMember("live", &CastleMapItem::live);
            mapItem.RegisterMember("current", &CastleMapItem::current);
            mapItem.RegisterMember("hp_width", &CastleMapItem::hpWidth);
            mapItem.RegisterMember("hp_fill_width", &CastleMapItem::hpFillWidth);
            mapItem.RegisterMember("defense_width", &CastleMapItem::defenseWidth);
            mapItem.RegisterMember("recover_width", &CastleMapItem::recoverWidth);
            c.RegisterArray<std::vector<CastleMapItem>>();
            c.Bind("map_items", &model.mapItems);
            c.Bind("statue_page", &model.statuePage);
            c.Bind("item_live", &model.itemLive);
            c.Bind("exit_tooltip", &model.exitTooltip);
            c.BindEventCallback("senatus_button",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PendingButton = static_cast<SENATUS_BUTTON>(arguments[0].Get<int>(-1));
                                });
        });
    if (!modelCreated)
        return;

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/castle_window.rml");
}

void CCastleWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void CCastleWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth 5: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
    SyncContent();
}

void CCastleWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();

    // One of the window's own lines: the document places it, so only what it says and the size the
    // native renderer would have shrunk it to for its box travel through the model.
    auto line = [&](const wchar_t* text, bool boldFont, float boxWidth) -> CastleLine
    {
        if (text == nullptr || text[0] == L'\0')
            return {};
        g_pRenderText->SetFont(boldFont ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = boldFont ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const float px =
            boxWidth > 0.f
                ? UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured), boxWidth)
                : UI::Scaling::NativeTextPixelSize(role, transform);
        return {StringUtils::WideToNarrow(text), px};
    };
    wchar_t szTemp[256] = {};

    // RenderFrame(): the title, in the bold font.
    CastleLine title = line(I18N::Game::SeniorNPC, true, 160.f);

    // The tabs: a label wider than its tab is shrunk towards the tab's 40 px down to the minimum
    // font size and overflows both its sides, as in the original client.
    const wchar_t* tabLabels[] = {I18N::Game::CastleGate, I18N::Game::GuardianStatue, I18N::Game::Tax,
                                  I18N::Game::Store1640};
    std::vector<CastleTabEntry> tabs;
    g_pRenderText->SetFont(g_hFont);
    for (int i = 0; i < 4; ++i)
    {
        const SIZE size = g_pRenderText->MeasureText(tabLabels[i], static_cast<int>(wcslen(tabLabels[i])));
        tabs.push_back({StringUtils::WideToNarrow(tabLabels[i]),
                        UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Normal, transform,
                                                              static_cast<float>(size.cx), 40.f),
                        i == m_iNumCurOpenTab});
    }

    std::vector<CastleMapItem> mapItems;
    bool statuePage = false;
    bool itemLive = false;
    CastleLine mapTitle, improveTitle;
    CastleLine statHp, statDefense, statRecover;
    CastleLine nextHp, nextDefense, nextRecover;
    CastleActionButton buyButton, repairButton, hpButton, defenseButton, recoverButton;
    CastleLine taxTitle, chaosRate, storeRate;
    CastleLine note1, note2, note3;
    CastleLine rule1, rule2, rule3, rule4, rule5, rule6;
    CastleLine zenLabel, castleMoney;
    CastleLine footer1, footer2, footer3;
    CastleActionButton applyButton, withdrawButton;

    // The theme places each gate or statue on the map; only bar lengths come from game state.
    auto addItem = [&](LPPMSG_NPCDBLIST pInfo, bool statue)
    {
        CastleMapItem item;
        item.statue = statue;
        item.live = pInfo->btNpcLive != 0;
        item.current = pInfo->iNpcIndex ==
                       (statue ? g_SenatusInfo.GetCurrStatue() : g_SenatusInfo.GetCurrGate()) + 1;
        if (item.live)
        {
            const int nHPBlockSize = 24 / (g_SenatusInfo.GetMaxHPLevel() + 1);
            const int nDefenseBlockSize = 24 / (g_SenatusInfo.GetMaxDefenseLevel() + 1);
            const int nRecoverBlockSize = 24 / (g_SenatusInfo.GetMaxRecoverLevel() + 1);
            const float fHPRate = pInfo->iNpcMaxHp > 0 ? pInfo->iNpcHp / static_cast<float>(pInfo->iNpcMaxHp) : 0.f;
            item.hpWidth = static_cast<float>(nHPBlockSize * (g_SenatusInfo.GetHPLevel(pInfo) + 1));
            item.hpFillWidth = item.hpWidth * fHPRate;
            item.defenseWidth = static_cast<float>(nDefenseBlockSize * (g_SenatusInfo.GetDefenseLevel(pInfo) + 1));
            if (statue)
                item.recoverWidth =
                    static_cast<float>(nRecoverBlockSize * (g_SenatusInfo.GetRecoverLevel(pInfo) + 1));
        }
        mapItems.push_back(item);
    };

    switch (m_iNumCurOpenTab)
    {
    case TAB_GATE_MANAGING:
    case TAB_STATUE_MANAGING:
    {
        // RenderGateManagingTab() / RenderStatueManagingTab(): one layout, with a recovery row
        // the gate has not.
        const bool statue = m_iNumCurOpenTab == TAB_STATUE_MANAGING;
        statuePage = statue;
        LPPMSG_NPCDBLIST pNPCInfo = statue ? &g_SenatusInfo.GetCurrStatueInfo() : &g_SenatusInfo.GetCurrGateInfo();

        mapTitle = line(I18N::Game::PurchaseAndRepair, true, 190.f);
        if (statue)
        {
            addItem(&g_SenatusInfo.GetStatueInfo(0), true);
            addItem(&g_SenatusInfo.GetStatueInfo(1), true);
            addItem(&g_SenatusInfo.GetStatueInfo(2), true);
            addItem(&g_SenatusInfo.GetStatueInfo(3), true);
        }
        else
        {
            addItem(&g_SenatusInfo.GetGateInfo(0), false);
            addItem(&g_SenatusInfo.GetGateInfo(1), false);
            addItem(&g_SenatusInfo.GetGateInfo(2), false);
            addItem(&g_SenatusInfo.GetGateInfo(3), false);
            addItem(&g_SenatusInfo.GetGateInfo(4), false);
            addItem(&g_SenatusInfo.GetGateInfo(5), false);
        }

        itemLive = pNPCInfo->btNpcLive != 0;
        if (!itemLive)
        {
            buyButton = {StringUtils::WideToNarrow(I18N::Game::Buy1124), true, false};
            break;
        }

        const bool repairable = statue ? g_SenatusInfo.IsStatueRepairable() : g_SenatusInfo.IsGateRepairable();
        const bool hpUpgradable = statue ? g_SenatusInfo.IsStatueHPUpgradable() : g_SenatusInfo.IsGateHPUpgradable();
        const bool defenseUpgradable =
            statue ? g_SenatusInfo.IsStatueDefeseUpgradable() : g_SenatusInfo.IsGateDefeseUpgradable();
        repairButton = {StringUtils::WideToNarrow(I18N::Game::Repair), true, !repairable};
        hpButton = {StringUtils::WideToNarrow(I18N::Game::Improve), true, !hpUpgradable};
        defenseButton = {StringUtils::WideToNarrow(I18N::Game::Improve), true, !defenseUpgradable};
        if (statue)
            recoverButton = {StringUtils::WideToNarrow(I18N::Game::Improve), true,
                             !g_SenatusInfo.IsStatueRecoverUpgradable()};

        mu_swprintf(szTemp, I18N::Game::DURDD, pNPCInfo->iNpcHp, pNPCInfo->iNpcMaxHp);
        InsertComma(szTemp, pNPCInfo->iNpcHp);
        InsertComma(szTemp, pNPCInfo->iNpcMaxHp);
        statHp = line(szTemp, false, 0.f);
        mu_swprintf(szTemp, I18N::Game::DPD, g_SenatusInfo.GetDefense(pNPCInfo->iNpcNumber, pNPCInfo->iNpcDfLevel));
        statDefense = line(szTemp, false, 0.f);
        if (statue)
        {
            mu_swprintf(szTemp, I18N::Game::RRD, g_SenatusInfo.GetRecover(pNPCInfo->iNpcNumber, pNPCInfo->iNpcRgLevel));
            statRecover = line(szTemp, false, 0.f);
        }

        improveTitle = line(I18N::Game::Improve, true, 190.f);
        mu_swprintf(szTemp, I18N::Game::DURD, g_SenatusInfo.GetNextAddHP(pNPCInfo));
        InsertComma(szTemp, g_SenatusInfo.GetNextAddHP(pNPCInfo));
        nextHp = line(szTemp, false, 0.f);
        mu_swprintf(szTemp, I18N::Game::DPD1564, g_SenatusInfo.GetNextAddDefense(pNPCInfo));
        InsertComma(szTemp, g_SenatusInfo.GetNextAddDefense(pNPCInfo));
        nextDefense = line(szTemp, false, 0.f);
        if (statue)
        {
            mu_swprintf(szTemp, I18N::Game::RRD1565, g_SenatusInfo.GetNextAddRecover(pNPCInfo));
            InsertComma(szTemp, g_SenatusInfo.GetNextAddRecover(pNPCInfo));
            nextRecover = line(szTemp, false, 0.f);
        }
        break;
    }
    case TAB_TAX_MANAGING:
    {
        // RenderTaxManagingTab().
        taxTitle = line(I18N::Game::AdjustTaxRate, true, 190.f);
        mu_swprintf(szTemp, I18N::Game::ChaosCombinationGoblinDD, g_SenatusInfo.GetRealTaxRateChaos(),
                    g_SenatusInfo.GetChaosTaxRate());
        chaosRate = line(szTemp, false, 175.f);
        mu_swprintf(szTemp, I18N::Game::NPCDD, g_SenatusInfo.GetRealTaxRateStore(), g_SenatusInfo.GetNormalTaxRate());
        storeRate = line(szTemp, false, 175.f);
        applyButton = {StringUtils::WideToNarrow(I18N::Game::Apply), true, false};

        note1 = line(I18N::Game::OnlyTheLordOfTheCastle, false, 160.f);
        note2 = line(I18N::Game::CanAdjustTheTaxRate, false, 160.f);
        note3 = line(I18N::Game::TaxAdjustmentAvailable, false, 160.f);

        // Formats: "3%%" in some languages.
        mu_swprintf(szTemp, I18N::Game::DuringTrucePeriod);
        rule1 = line(szTemp, true, 160.f);
        mu_swprintf(szTemp, I18N::Game::MaximumTaxRates3);
        rule2 = line(szTemp, true, 160.f);
        rule3 = line(I18N::Game::NPCsInclude, true, 160.f);
        rule4 = line(I18N::Game::ElfLalaPotionGirl, true, 160.f);
        rule5 = line(I18N::Game::WizardArenaGuard, true, 160.f);
        rule6 = line(I18N::Game::AndEtc, true, 160.f);

        // Still bold: the colour went back to white, the font did not.
        mu_swprintf(szTemp, I18N::Game::Zen);
        zenLabel = line(szTemp, true, 0.f);
        // The original formatted "%I64d", which this platform reads as a 64-wide padded field
        // and drew the amount twice, the second copy past the strip; fixed.
        ConvertGold64(g_SenatusInfo.GetCastleMoney(), szTemp);
        castleMoney = line(szTemp, true, 80.f);
        withdrawButton = {StringUtils::WideToNarrow(I18N::Game::Withdraw), true, false};

        footer1 = line(I18N::Game::TaxBelongsToTheCastle, true, 160.f);
        footer2 = line(I18N::Game::AndCanBeUsed, true, 160.f);
        footer3 = line(I18N::Game::ToOperateTheCastle, true, 160.f);
        break;
    }
    default:
        break;
    }

    SyncField(m_RmlBinder, &CastleWindowRmlModel::tabs, "tabs", std::move(tabs));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::activeTab, "active_tab", m_iNumCurOpenTab);
    SyncField(m_RmlBinder, &CastleWindowRmlModel::title, "title", std::move(title));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::statuePage, "statue_page", statuePage);
    SyncField(m_RmlBinder, &CastleWindowRmlModel::itemLive, "item_live", itemLive);
    SyncField(m_RmlBinder, &CastleWindowRmlModel::mapItems, "map_items", std::move(mapItems));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::mapTitle, "map_title", std::move(mapTitle));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::improveTitle, "improve_title", std::move(improveTitle));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::statHp, "stat_hp", std::move(statHp));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::statDefense, "stat_defense", std::move(statDefense));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::statRecover, "stat_recover", std::move(statRecover));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::nextHp, "next_hp", std::move(nextHp));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::nextDefense, "next_defense", std::move(nextDefense));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::nextRecover, "next_recover", std::move(nextRecover));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::buyButton, "buy_button", std::move(buyButton));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::repairButton, "repair_button", std::move(repairButton));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::hpButton, "hp_button", std::move(hpButton));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::defenseButton, "defense_button", std::move(defenseButton));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::recoverButton, "recover_button", std::move(recoverButton));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::taxTitle, "tax_title", std::move(taxTitle));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::chaosRate, "chaos_rate", std::move(chaosRate));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::storeRate, "store_rate", std::move(storeRate));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::note1, "note1", std::move(note1));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::note2, "note2", std::move(note2));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::note3, "note3", std::move(note3));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::rule1, "rule1", std::move(rule1));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::rule2, "rule2", std::move(rule2));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::rule3, "rule3", std::move(rule3));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::rule4, "rule4", std::move(rule4));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::rule5, "rule5", std::move(rule5));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::rule6, "rule6", std::move(rule6));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::zenLabel, "zen_label", std::move(zenLabel));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::castleMoney, "castle_money", std::move(castleMoney));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::footer1, "footer1", std::move(footer1));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::footer2, "footer2", std::move(footer2));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::footer3, "footer3", std::move(footer3));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::applyButton, "apply_button", std::move(applyButton));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::withdrawButton, "withdraw_button", std::move(withdrawButton));
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    SyncField(m_RmlBinder, &CastleWindowRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineHeight) * transform.scaleY);
    SyncField(m_RmlBinder, &CastleWindowRmlModel::buttonLabelTop, "button_label_top", static_cast<float>(23 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::tabLabelTop, "tab_label_top", static_cast<float>(22 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &CastleWindowRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}

void CCastleWindow::InsertComma(wchar_t* pszText, DWORD dwNumber)
{
    wchar_t szNumber[32];
    mu_swprintf(szNumber, L"%d", dwNumber);

    wchar_t szTemp[256];
    wcscpy_s(szTemp, 256, pszText);
    wchar_t* pszTextBegin = szTemp;
    wchar_t* pszTextFound = wcsstr(szTemp, szNumber);
    if (pszTextFound == nullptr)
        return; // the number is not in the text (the original dereferenced null here)
    wchar_t* pszTextNext = pszTextFound + wcslen(szNumber);
    *pszTextFound = '\0';
    ConvertGold(dwNumber, szNumber);

    mu_swprintf(pszText, L"%ls%ls%ls", pszTextBegin, szNumber, pszTextNext);
}
