
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
#include "UI/Combat/UISenatus.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cmath>

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

    // The tabs stay a native radio group for their hit tests; castle_window.rml draws them.
    m_TabBtn.CreateRadioGroup(4, BITMAP_GUILDINFO_BEGIN);
    m_TabBtn.ChangeRadioButtonInfo(true, m_Pos.x + 12.f, m_Pos.y + 32.f, 40, 22);
    m_TabBtn.ChangeFrame(m_iNumCurOpenTab);

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

bool CCastleWindow::UpdateMouseEvent()
{
    // The gate and statue picks keep their native hit tests; the buttons are RmlUi's (see
    // Update()).
    UpdateIconPick();

    if (true == BtnProcess())
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY))
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

    if (IsVisible())
    {
        const int iNumCurOpenTab = m_TabBtn.UpdateMouseEvent();
        if (iNumCurOpenTab != RADIOGROUPEVENT_NONE)
        {
            m_iNumCurOpenTab = iNumCurOpenTab;

            if (iNumCurOpenTab == TAB_CASTLE_MIX)
            {
                g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_CASTLE_SENIOR);
                //	 		g_pNewUISystem->Hide(mu::ui::window::INTERFACE_SENATUS);
                g_pNewUISystem->Show(mu::ui::window::INTERFACE_MIXINVENTORY);
            }
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
    m_iNumCurOpenTab = TAB_GATE_MANAGING;
    m_TabBtn.ChangeFrame(TAB_GATE_MANAGING);

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
    const std::vector<CastleButtonEntry>& buttons = m_RmlBinder.GetModel().buttons;
    const auto it = std::find_if(buttons.begin(), buttons.end(),
                                 [button](const CastleButtonEntry& entry) { return entry.id == button; });
    return it != buttons.end() && it->locked;
}

void CCastleWindow::UpdateIconPick()
{
    if (!MouseLButtonPush)
        return;

    const POINT ptOrigin = {m_Pos.x, m_Pos.y + 55 + 6 + 12};
    if (!CheckMouseIn(ptOrigin.x + 15, ptOrigin.y, 160.f, 165.f))
        return;

    if (m_iNumCurOpenTab == TAB_GATE_MANAGING)
    {
        if (CheckMouseIn(ptOrigin.x + 82, ptOrigin.y + 35, 24, 24))
            g_SenatusInfo.SetCurrGate(0);
        else if (CheckMouseIn(ptOrigin.x + 64, ptOrigin.y + 83, 24, 24))
            g_SenatusInfo.SetCurrGate(1);
        else if (CheckMouseIn(ptOrigin.x + 100, ptOrigin.y + 83, 24, 24))
            g_SenatusInfo.SetCurrGate(2);
        else if (CheckMouseIn(ptOrigin.x + 48, ptOrigin.y + 135, 24, 24))
            g_SenatusInfo.SetCurrGate(3);
        else if (CheckMouseIn(ptOrigin.x + 82, ptOrigin.y + 135, 24, 24))
            g_SenatusInfo.SetCurrGate(4);
        else if (CheckMouseIn(ptOrigin.x + 116, ptOrigin.y + 135, 24, 24))
            g_SenatusInfo.SetCurrGate(5);
    }
    else if (m_iNumCurOpenTab == TAB_STATUE_MANAGING)
    {
        if (CheckMouseIn(ptOrigin.x + 82, ptOrigin.y + 20, 24, 24))
            g_SenatusInfo.SetCurrStatue(0);
        else if (CheckMouseIn(ptOrigin.x + 82, ptOrigin.y + 65, 24, 24))
            g_SenatusInfo.SetCurrStatue(1);
        else if (CheckMouseIn(ptOrigin.x + 64, ptOrigin.y + 110, 24, 24))
            g_SenatusInfo.SetCurrStatue(2);
        else if (CheckMouseIn(ptOrigin.x + 100, ptOrigin.y + 110, 24, 24))
            g_SenatusInfo.SetCurrStatue(3);
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
            tab.RegisterMember("label_left", &CastleTabEntry::labelLeft);
            tab.RegisterMember("text_px", &CastleTabEntry::textPx);
            tab.RegisterMember("selected", &CastleTabEntry::selected);
            c.RegisterArray<std::vector<CastleTabEntry>>();
            c.Bind("tabs", &model.tabs);
            auto piece = c.RegisterStruct<CastlePieceEntry>();
            piece.RegisterMember("kind", &CastlePieceEntry::kind);
            piece.RegisterMember("sprite", &CastlePieceEntry::sprite);
            piece.RegisterMember("left", &CastlePieceEntry::left);
            piece.RegisterMember("top", &CastlePieceEntry::top);
            piece.RegisterMember("width", &CastlePieceEntry::width);
            piece.RegisterMember("height", &CastlePieceEntry::height);
            piece.RegisterMember("color", &CastlePieceEntry::color);
            piece.RegisterMember("text", &CastlePieceEntry::text);
            piece.RegisterMember("text_px", &CastlePieceEntry::textPx);
            piece.RegisterMember("align", &CastlePieceEntry::align);
            piece.RegisterMember("bold", &CastlePieceEntry::bold);
            c.RegisterArray<std::vector<CastlePieceEntry>>();
            c.Bind("pieces", &model.pieces);
            auto button = c.RegisterStruct<CastleButtonEntry>();
            button.RegisterMember("label", &CastleButtonEntry::label);
            button.RegisterMember("id", &CastleButtonEntry::id);
            button.RegisterMember("left", &CastleButtonEntry::left);
            button.RegisterMember("top", &CastleButtonEntry::top);
            button.RegisterMember("locked", &CastleButtonEntry::locked);
            c.RegisterArray<std::vector<CastleButtonEntry>>();
            c.Bind("buttons", &model.buttons);
            c.Bind("tax_arrows", &model.taxArrows);
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
    const float x0 = static_cast<float>(m_Pos.x);
    const float y0 = static_cast<float>(m_Pos.y);
    std::vector<CastlePieceEntry> pieces;
    std::vector<CastleButtonEntry> buttons;

    // RenderText(x, y, text, width, 0, sort) in window coordinates, in the font and colour the
    // original had set at that point (its draws leak them from one call to the next).
    bool bold = true;
    DWORD color = RGBA(220, 220, 220, 255);
    auto addText = [&](const wchar_t* text, float x, float y, float width, int align)
    {
        if (text == nullptr || text[0] == L'\0')
            return;
        g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const float px =
            width > 0.f ? UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured), width)
                        : UI::Scaling::NativeTextPixelSize(role, transform);
        CastlePieceEntry entry;
        entry.kind = CastlePieceEntry::KIND_TEXT;
        entry.left = x - x0;
        entry.top = y - y0;
        entry.width = width;
        entry.color = UI::RmlBridge::RgbaToCss(color);
        entry.text = StringUtils::WideToNarrow(text);
        entry.textPx = px;
        entry.align = align;
        entry.bold = bold;
        pieces.push_back(std::move(entry));
    };
    auto addImage = [&](const char* sprite, float x, float y, float width, float height)
    {
        CastlePieceEntry entry;
        entry.kind = CastlePieceEntry::KIND_IMAGE;
        entry.sprite = sprite;
        entry.left = x - x0;
        entry.top = y - y0;
        entry.width = width;
        entry.height = height;
        pieces.push_back(std::move(entry));
    };
    // RenderColorQuadARGB(); a zero or negative size (an empty level bar) draws nothing.
    auto addBox = [&](float x, float y, float width, float height, unsigned int argb)
    {
        if (!(width > 0.f) || !(height > 0.f))
            return;
        CastlePieceEntry entry;
        entry.kind = CastlePieceEntry::KIND_BOX;
        entry.left = x - x0;
        entry.top = y - y0;
        entry.width = width;
        entry.height = height;
        entry.color = ArgbToCss(argb);
        pieces.push_back(std::move(entry));
    };
    auto addButton = [&](SENATUS_BUTTON id, const wchar_t* label, float x, float y, bool locked)
    { buttons.push_back({StringUtils::WideToNarrow(label), id, x - x0, y - y0, locked}); };
    // RenderOutlineUpper() / RenderOutlineLower(): a table's title bar and its frame.
    auto outlineUpper = [&](float x, float y, float width)
    {
        addImage("t-tl", x + 12, y - 4, 14, 14);
        addImage("t-tr", x + width + 4, y - 4, 14, 14);
        addImage("t-top", x + 25, y - 4, width - 21, 14);
        addBox(x + 15, y - 3, width - 2, 15, 0x4D000000u);
    };
    auto outlineLower = [&](float x, float y, float width, float height)
    {
        addImage("t-left", x + 12, y + 9, 14, height);
        addImage("t-right", x + width + 4, y + 9, 14, height);
        addImage("t-bottom", x + 15, y + 3, width - 2, 14);
        addImage("t-bl", x + 12, y + height + 3, 14, 14);
        addImage("t-br", x + width + 4, y + height + 3, 14, 14);
        addImage("t-bottom", x + 25, y + height + 3, width - 21, 14);
    };
    // RenderCastleItem(): a gate or statue icon with its level bars when it stands.
    auto castleItem = [&](float x, float y, LPPMSG_NPCDBLIST pInfo)
    {
        const int nHPBlockSize = 24 / (g_SenatusInfo.GetMaxHPLevel() + 1);
        const int nDefenseBlockSize = 24 / (g_SenatusInfo.GetMaxDefenseLevel() + 1);
        const int nRecoverBlockSize = 24 / (g_SenatusInfo.GetMaxRecoverLevel() + 1);
        const float fHPRate = pInfo->iNpcMaxHp > 0 ? pInfo->iNpcHp / static_cast<float>(pInfo->iNpcMaxHp) : 0.f;

        if (g_SenatusInfo.IsGate(pInfo))
        {
            if (pInfo->btNpcLive)
            {
                const int nHP = g_SenatusInfo.GetHPLevel(pInfo);
                const int nDefense = g_SenatusInfo.GetDefenseLevel(pInfo);
                addBox(x, y - 10, static_cast<float>(nHPBlockSize * (nHP + 1)), 3, 0xFFFFFFFFu);
                addBox(x, y - 5, 24, 3, 0xFFFFFFFFu);
                addBox(x, y - 10, (nHPBlockSize * (nHP + 1)) * fHPRate, 3, 0xFFFF0000u);
                addBox(x, y - 10, 24, 1, 0xFF000000u);
                addBox(x, y - 7, 24, 1, 0xFF000000u);
                addBox(x, y - 10, 1, 3, 0xFF000000u);
                addBox(x + 24, y - 10, 1, 3, 0xFF000000u);
                addBox(x, y - 5, static_cast<float>(nDefenseBlockSize * (nDefense + 1)), 3, 0xFF00FF00u);
            }
            addImage(pInfo->iNpcIndex == g_SenatusInfo.GetCurrGate() + 1 ? "gate-on" : "gate-off", x, y, 24, 24);
        }
        if (g_SenatusInfo.IsStatue(pInfo))
        {
            if (pInfo->btNpcLive)
            {
                const int nHP = g_SenatusInfo.GetHPLevel(pInfo);
                const int nDefense = g_SenatusInfo.GetDefenseLevel(pInfo);
                const int nRecover = g_SenatusInfo.GetRecoverLevel(pInfo);
                addBox(x, y - 15, static_cast<float>(nHPBlockSize * (nHP + 1)), 3, 0xFFFFFFFFu);
                addBox(x, y - 10, 24, 3, 0xFFFFFFFFu);
                addBox(x, y - 5, 24, 3, 0xFFFFFFFFu);
                addBox(x, y - 15, (nHPBlockSize * (nHP + 1)) * fHPRate, 3, 0xFFFF0000u);
                addBox(x, y - 10, static_cast<float>(nDefenseBlockSize * (nDefense + 1)), 3, 0xFF00FF00u);
                addBox(x, y - 5, static_cast<float>(nRecoverBlockSize * (nRecover + 1)), 3, 0xFFFFFF00u);
            }
            addImage(pInfo->iNpcIndex == g_SenatusInfo.GetCurrStatue() + 1 ? "statue-on" : "statue-off", x, y, 24, 24);
        }
    };
    wchar_t szTemp[256] = {};

    // RenderFrame(): the title, bold (220, 220, 220).
    addText(I18N::Game::SeniorNPC, x0 + 15, y0 + 13, 160, 1);

    // The tabs (labels in the normal font): a label wider than its tab starts left of it, from
    // the whole-unit centre of its full width, shrunk towards the tab's 40 px down to the minimum
    // font size, and runs over its neighbours ("Castle Gate", "Guardian Statue"), as in the
    // original client.
    const wchar_t* tabLabels[] = {I18N::Game::CastleGate, I18N::Game::GuardianStatue, I18N::Game::Tax,
                                  I18N::Game::Store1640};
    std::vector<CastleTabEntry> tabs;
    g_pRenderText->SetFont(g_hFont);
    for (int i = 0; i < 4; ++i)
    {
        const SIZE size = g_pRenderText->MeasureText(tabLabels[i], static_cast<int>(wcslen(tabLabels[i])));
        tabs.push_back({StringUtils::WideToNarrow(tabLabels[i]), static_cast<float>(40 / 2 - size.cx / 2),
                        UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Normal, transform,
                                                              static_cast<float>(size.cx), 40.f),
                        i == m_TabBtn.GetCurButtonIndex()});
    }

    // The gate and statue pages share their layout (RenderGateManagingTab(),
    // RenderStatueManagingTab()).
    auto gateOrStatuePage = [&](bool statue)
    {
        LPPMSG_NPCDBLIST pNPCInfo = statue ? &g_SenatusInfo.GetCurrStatueInfo() : &g_SenatusInfo.GetCurrGateInfo();
        const float x = x0;
        float y = y0 + 55 + 6;

        outlineUpper(x, y, 160);
        addText(I18N::Game::PurchaseAndRepair, x, y, 190, 1);
        addImage("map", x + 15, y + 12, 160, 165);
        outlineLower(x, y, 160, 165);

        y += 12;
        if (statue)
        {
            castleItem(x + 82, y + 20, &g_SenatusInfo.GetStatueInfo(0));
            castleItem(x + 82, y + 65, &g_SenatusInfo.GetStatueInfo(1));
            castleItem(x + 64, y + 110, &g_SenatusInfo.GetStatueInfo(2));
            castleItem(x + 100, y + 110, &g_SenatusInfo.GetStatueInfo(3));
        }
        else
        {
            castleItem(x + 82, y + 35, &g_SenatusInfo.GetGateInfo(0));
            castleItem(x + 64, y + 83, &g_SenatusInfo.GetGateInfo(1));
            castleItem(x + 100, y + 83, &g_SenatusInfo.GetGateInfo(2));
            castleItem(x + 48, y + 135, &g_SenatusInfo.GetGateInfo(3));
            castleItem(x + 82, y + 135, &g_SenatusInfo.GetGateInfo(4));
            castleItem(x + 116, y + 135, &g_SenatusInfo.GetGateInfo(5));
        }

        y += 173;
        if (pNPCInfo->btNpcLive == 0)
        {
            addButton(SENATUS_BUTTON_BUY, I18N::Game::Buy1124, x0 + INVENTORY_WIDTH / 2 - 27, y0 + 250, false);
            return;
        }

        const bool repairable = statue ? g_SenatusInfo.IsStatueRepairable() : g_SenatusInfo.IsGateRepairable();
        const bool hpUpgradable = statue ? g_SenatusInfo.IsStatueHPUpgradable() : g_SenatusInfo.IsGateHPUpgradable();
        const bool defenseUpgradable =
            statue ? g_SenatusInfo.IsStatueDefeseUpgradable() : g_SenatusInfo.IsGateDefeseUpgradable();
        addButton(SENATUS_BUTTON_REPAIR, I18N::Game::Repair, x0 + 110, y0 + 260, !repairable);
        addButton(SENATUS_BUTTON_UPGRADE_HP, I18N::Game::Improve, x0 + 110, y0 + 310, !hpUpgradable);
        addButton(SENATUS_BUTTON_UPGRADE_DEFENSE, I18N::Game::Improve, x0 + 110, y0 + 334, !defenseUpgradable);
        if (statue)
            addButton(SENATUS_BUTTON_UPGRADE_RECOVER, I18N::Game::Improve, x0 + 110, y0 + 358,
                      !g_SenatusInfo.IsStatueRecoverUpgradable());

        bold = false;
        color = RGBA(255, 255, 255, 255);
        mu_swprintf(szTemp, I18N::Game::DURDD, pNPCInfo->iNpcHp, pNPCInfo->iNpcMaxHp);
        InsertComma(szTemp, pNPCInfo->iNpcHp);
        InsertComma(szTemp, pNPCInfo->iNpcMaxHp);
        addText(szTemp, x + 20, y, 0, 0);
        y += 13;
        mu_swprintf(szTemp, I18N::Game::DPD, g_SenatusInfo.GetDefense(pNPCInfo->iNpcNumber, pNPCInfo->iNpcDfLevel));
        addText(szTemp, x + 20, y, 0, 0);
        if (statue)
        {
            y += 13;
            mu_swprintf(szTemp, I18N::Game::RRD, g_SenatusInfo.GetRecover(pNPCInfo->iNpcNumber, pNPCInfo->iNpcRgLevel));
            addText(szTemp, x + 20, y, 0, 0);
            y += 22;
        }
        else
        {
            y += 35;
        }

        outlineUpper(x, y, 160);
        outlineLower(x, y, 160, 78);
        bold = true;
        addText(I18N::Game::Improve, x, y, 190, 1);
        bold = false;

        y += 24;
        mu_swprintf(szTemp, I18N::Game::DURD, g_SenatusInfo.GetNextAddHP(pNPCInfo));
        InsertComma(szTemp, g_SenatusInfo.GetNextAddHP(pNPCInfo));
        addText(szTemp, x + 30, y, 0, 0);
        y += 23;
        mu_swprintf(szTemp, I18N::Game::DPD1564, g_SenatusInfo.GetNextAddDefense(pNPCInfo));
        InsertComma(szTemp, g_SenatusInfo.GetNextAddDefense(pNPCInfo));
        addText(szTemp, x + 30, y, 0, 0);
        if (statue)
        {
            y += 23;
            mu_swprintf(szTemp, I18N::Game::RRD1565, g_SenatusInfo.GetNextAddRecover(pNPCInfo));
            InsertComma(szTemp, g_SenatusInfo.GetNextAddRecover(pNPCInfo));
            addText(szTemp, x + 30, y, 0, 0);
        }
    };

    switch (m_iNumCurOpenTab)
    {
    case TAB_GATE_MANAGING:
        gateOrStatuePage(false);
        break;
    case TAB_STATUE_MANAGING:
        gateOrStatuePage(true);
        break;
    case TAB_TAX_MANAGING:
    {
        // RenderTaxManagingTab().
        const float x = x0;
        float y = y0 + 55 + 6;
        color = RGBA(255, 255, 255, 255);

        addBox(x + 15, y + 14, 150, 24, 0x80666666u);
        addBox(x + 15, y + 42, 150, 24, 0x80666666u);
        outlineUpper(x, y, 160);
        bold = true;
        addText(I18N::Game::AdjustTaxRate, x, y, 190, 1);
        outlineLower(x, y, 160, 55);
        addImage("t-bottom", x + 15, y + 30, 160 - 2, 14);

        y += 23;
        bold = false;
        mu_swprintf(szTemp, I18N::Game::ChaosCombinationGoblinDD, g_SenatusInfo.GetRealTaxRateChaos(),
                    g_SenatusInfo.GetChaosTaxRate());
        addText(szTemp, x, y, 175, 1);
        y += 25;
        mu_swprintf(szTemp, I18N::Game::NPCDD, g_SenatusInfo.GetRealTaxRateStore(), g_SenatusInfo.GetNormalTaxRate());
        addText(szTemp, x, y, 175, 1);

        addButton(SENATUS_BUTTON_APPLY_TAX, I18N::Game::Apply, x0 + 120, y0 + 133, false);

        y += 53;
        addText(I18N::Game::OnlyTheLordOfTheCastle, x + 15, y, 160, 1);
        y += 13;
        addText(I18N::Game::CanAdjustTheTaxRate, x + 15, y, 160, 1);
        y += 13;
        addText(I18N::Game::TaxAdjustmentAvailable, x + 15, y, 160, 1);

        bold = true;
        color = 0xFF947BBB;
        y += 20;
        // Formats: "3%%" in some languages.
        mu_swprintf(szTemp, I18N::Game::DuringTrucePeriod);
        addText(szTemp, x + 15, y, 160, 1);
        y += 12;
        mu_swprintf(szTemp, I18N::Game::MaximumTaxRates3);
        addText(szTemp, x + 15, y, 160, 1);
        y += 12;
        addText(I18N::Game::NPCsInclude, x + 15, y, 160, 1);
        y += 12;
        addText(I18N::Game::ElfLalaPotionGirl, x + 15, y, 160, 1);
        y += 12;
        addText(I18N::Game::WizardArenaGuard, x + 15, y, 160, 1);
        y += 12;
        addText(I18N::Game::AndEtc, x + 15, y, 160, 1);

        y += 10;
        addImage("line", x + 1, y, 188, 21);

        // Still bold: the colour went back to white, the font did not.
        color = RGBA(255, 255, 255, 255);
        y += 18;
        addImage("money", x + 10, y, 170, 24);
        mu_swprintf(szTemp, I18N::Game::Zen);
        addText(szTemp, x + 14, y + 7, 0, 0);
        // The original formatted "%I64d", which this platform reads as a 64-wide padded field
        // and drew the amount twice, the second copy past the strip; fixed.
        ConvertGold64(g_SenatusInfo.GetCastleMoney(), szTemp);
        addText(szTemp, x + 90, y + 7, 80, 2);

        addButton(SENATUS_BUTTON_WITHDRAW, I18N::Game::Withdraw, x0 + 120, y0 + 322, false);

        color = 0xFF947BBB;
        y += 54;
        addText(I18N::Game::TaxBelongsToTheCastle, x + 15, y, 160, 1);
        y += 12;
        addText(I18N::Game::AndCanBeUsed, x + 15, y, 160, 1);
        y += 12;
        addText(I18N::Game::ToOperateTheCastle, x + 15, y, 160, 1);
        break;
    }
    default:
        break;
    }

    CastleWindowRmlModel& model = m_RmlBinder.GetModel();
    auto sync = [&](auto field, const char* name, auto value)
    {
        if (!(model.*field == value))
        {
            model.*field = std::move(value);
            m_RmlBinder.MarkDirty(name);
        }
    };
    sync(&CastleWindowRmlModel::tabs, "tabs", std::move(tabs));
    sync(&CastleWindowRmlModel::pieces, "pieces", std::move(pieces));
    sync(&CastleWindowRmlModel::buttons, "buttons", std::move(buttons));
    sync(&CastleWindowRmlModel::taxArrows, "tax_arrows", m_iNumCurOpenTab == TAB_TAX_MANAGING);
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    sync(&CastleWindowRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineHeight) * transform.scaleY);
    sync(&CastleWindowRmlModel::buttonLabelTop, "button_label_top", static_cast<float>(23 / 2 - lineHeight / 2));
    sync(&CastleWindowRmlModel::tabLabelTop, "tab_label_top", static_cast<float>(22 / 2 - lineHeight / 2));
    sync(&CastleWindowRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
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
