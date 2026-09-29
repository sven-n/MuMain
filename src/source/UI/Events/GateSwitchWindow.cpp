
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
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

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
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CGateSwitchWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CGateSwitchWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CGateSwitchWindow::UpdateMouseEvent()
{
    // The Open / Close and exit buttons are RmlUi's (see Update()).
    if (true == BtnProcess())
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
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

bool CGateSwitchWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_GATESWITCH);

    return false;
}

void CGateSwitchWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "gate_switch",
        [this](Rml::DataModelConstructor& c, GateSwitchRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("bold_text_px", &model.boldTextPx);
            c.Bind("title", &model.title);
            c.Bind("title_px", &model.titlePx);
            c.Bind("line1", &model.line1);
            c.Bind("line2", &model.line2);
            c.Bind("warning", &model.warning);
            c.Bind("gate_opened", &model.gateOpened);
            c.Bind("button_text", &model.buttonText);
            c.Bind("label_top", &model.labelTop);
            c.Bind("label_line_px", &model.labelLinePx);
            c.Bind("exit_tooltip", &model.exitTooltip);
            c.BindEventCallback("gate_switch_toggle", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                { m_PendingToggle = true; });
            c.BindEventCallback("gate_switch_exit", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                { m_PendingExit = true; });
        });
    if (!modelCreated)
        return;

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/gate_switch.rml");
}

void CGateSwitchWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void CGateSwitchWindow::SyncRmlModel()
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

    // The original's RenderFrame() and Render(): the title bold (220, 220, 220) in its 160-unit
    // box; the three lines in the font and colour RenderFrame() left set (bold, (220, 220, 220)),
    // each centred on x 95, the warning on a (160, 0, 0) text box; Open or Close by the gate state.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    GateSwitchRmlModel& model = m_RmlBinder.GetModel();
    auto sync = [&](auto field, const char* name, auto value)
    {
        if (!(model.*field == value))
        {
            model.*field = value;
            m_RmlBinder.MarkDirty(name);
        }
    };
    g_pRenderText->SetFont(g_hFontBold);
    const int titleWidth =
        g_pRenderText->MeasureText(I18N::Game::CastleGateSwitch, static_cast<int>(wcslen(I18N::Game::CastleGateSwitch)))
            .cx;
    sync(&GateSwitchRmlModel::boldTextPx, "bold_text_px",
         UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform));
    sync(&GateSwitchRmlModel::titlePx, "title_px",
         UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Bold, transform, static_cast<float>(titleWidth),
                                               160.f));
    sync(&GateSwitchRmlModel::title, "title", StringUtils::WideToNarrow(I18N::Game::CastleGateSwitch));
    sync(&GateSwitchRmlModel::line1, "line1", StringUtils::WideToNarrow(I18N::Game::CanCommandToOpenOrClose));
    sync(&GateSwitchRmlModel::line2, "line2", StringUtils::WideToNarrow(I18N::Game::TheCastleGateInFront));
    sync(&GateSwitchRmlModel::warning, "warning",
         StringUtils::WideToNarrow(I18N::Game::BeCarefulItMightBeBeneficialToTheEnemy));
    const bool opened = npcGateSwitch::IsGateOpened();
    sync(&GateSwitchRmlModel::gateOpened, "gate_opened", opened);
    sync(&GateSwitchRmlModel::buttonText, "button_text",
         StringUtils::WideToNarrow(opened ? I18N::Game::Close388 : I18N::Game::Open1107));
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    const int labelTop = 29 / 2 - lineHeight / 2;
    sync(&GateSwitchRmlModel::labelTop, "label_top", static_cast<float>(labelTop));
    sync(&GateSwitchRmlModel::labelLinePx, "label_line_px", static_cast<float>(lineHeight) * transform.scaleY);
    sync(&GateSwitchRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}
