
#include "stdafx.h"
#include "UI/Events/GateSwitchWindow.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "Engine/Object/ZzzInfomation.h"
#include "GameLogic/NPCs/npcGateSwitch.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

CGateSwitchWindow::CGateSwitchWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CGateSwitchWindow::~CGateSwitchWindow()
{
    Release();
}

bool CGateSwitchWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GATESWITCH, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}

void CGateSwitchWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void CGateSwitchWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CGateSwitchWindow::UpdateMouseEvent()
{
    // The Open / Close and exit buttons are RmlUi's (see Update()).
    return !UI::RmlBridge::IsPointerOver(m_RmlView.Document());
}

bool CGateSwitchWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GATESWITCH) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATESWITCH);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CGateSwitchWindow::Update()
{
    // Clicks RmlUi reported (the original's handling in UpdateMouseEvent() / BtnProcess()).
    const bool toggle = m_PendingToggle;
    const bool exit = m_PendingExit;
    m_PendingToggle = m_PendingExit = false;
    if (IsVisible() && toggle)
    {
        npcGateSwitch::SendToggleGate();
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATESWITCH);
    }
    else if (IsVisible() && exit)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATESWITCH);
    }

    SyncRmlModel();
    return true;
}

bool CGateSwitchWindow::Render()
{
    // Nothing native left: the frame, the texts, the gate picture and the buttons are RmlUi.
    // Kept because CObject requires the override.
    return true;
}

void CGateSwitchWindow::OpeningProcess()
{
}

void CGateSwitchWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CGateSwitchWindow::GetLayerDepth()
{
    return 5.0f;
}

void CGateSwitchWindow::BindRmlModel(Rml::DataModelConstructor& c, GateSwitchRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("title", &model.title);
    c.Bind("title_px", &model.titlePx);
    c.Bind("line1", &model.line1);
    c.Bind("line2", &model.line2);
    c.Bind("warning", &model.warning);
    c.Bind("gate_opened", &model.gateOpened);
    c.Bind("button_text", &model.buttonText);
    c.Bind("label_line_px", &model.labelLinePx);
    c.Bind("exit_tooltip", &model.exitTooltip);
    c.BindEventCallback("gate_switch_toggle", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingToggle = true; });
    c.BindEventCallback("gate_switch_exit", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingExit = true; });
}

void CGateSwitchWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CGateSwitchWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 5: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    // The original's RenderFrame() and Render(): the title bold (220, 220, 220) in its 160-unit
    // box; the three lines in the font and colour RenderFrame() left set (bold, (220, 220, 220)),
    // each centred on x 95, the warning on a (160, 0, 0) text box; Open or Close by the gate state.
    GateSwitchRmlModel& model = m_RmlView.GetModel();
    g_pRenderText->SetFont(g_hFontBold);
    const int titleWidth =
        g_pRenderText->MeasureText(I18N::Game::CastleGateSwitch, static_cast<int>(wcslen(I18N::Game::CastleGateSwitch)))
            .cx;
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::boldTextPx, "bold_text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold));
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::titlePx, "title_px",
              UI::RmlBridge::NativeTextPxInBox(UI::Scaling::FontRole::Bold, static_cast<float>(titleWidth), 160.f));
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::title, "title", StringUtils::WideToNarrow(I18N::Game::CastleGateSwitch));
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::line1, "line1", StringUtils::WideToNarrow(I18N::Game::CanCommandToOpenOrClose));
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::line2, "line2", StringUtils::WideToNarrow(I18N::Game::TheCastleGateInFront));
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::warning, "warning",
         StringUtils::WideToNarrow(I18N::Game::BeCarefulItMightBeBeneficialToTheEnemy));
    const bool opened = npcGateSwitch::IsGateOpened();
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::gateOpened, "gate_opened", opened);
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::buttonText, "button_text",
         StringUtils::WideToNarrow(opened ? I18N::Game::Close388 : I18N::Game::Open1107));
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::labelLinePx, "label_line_px", CUIRenderTextSDLTtf::LineHeightPx(UI::Scaling::FontRole::Normal));
    SyncField(m_RmlView.Binder(), &GateSwitchRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}
