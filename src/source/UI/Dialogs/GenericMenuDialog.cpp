//*****************************************************************************
// File: GenericMenuDialog.cpp
//*****************************************************************************
#include "stdafx.h"
#include "UI/Dialogs/GenericMenuDialog.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowManager.h" // CManager::AddUIObj
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

namespace mu::ui::window
{

CGenericMenuDialog* g_pGenericMenuDialog = nullptr;

void CGenericMenuDialog::Create(CManager* pMng)
{
    Release();

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    pMng->AddUIObj(mu::ui::window::INTERFACE_GENERIC_MENU_DIALOG, this);
}

void CGenericMenuDialog::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "generic_menu_dialog",
        [this](Rml::DataModelConstructor& c, GenericMenuRmlModel& model)
        {
            auto line = c.RegisterStruct<LineEntry>();
            line.RegisterMember("text", &LineEntry::text);
            line.RegisterMember("bold", &LineEntry::bold);
            c.RegisterArray<std::vector<LineEntry>>();
            c.Bind("lines", &model.lines);

            auto button = c.RegisterStruct<MenuButtonEntry>();
            button.RegisterMember("label", &MenuButtonEntry::label);
            button.RegisterMember("tooltip", &MenuButtonEntry::tooltip);
            // Reuses the LineEntry/std::vector<LineEntry> array type already registered above for
            // the top-level `lines` -- per-button description text, rendered directly above that
            // button rather than lumped into the shared summary (see MenuButton::lines).
            button.RegisterMember("lines", &MenuButtonEntry::lines);
            button.RegisterMember("has_tooltip", &MenuButtonEntry::hasTooltip);
            button.RegisterMember("has_lines", &MenuButtonEntry::hasLines);
            button.RegisterMember("enabled", &MenuButtonEntry::enabled);
            button.RegisterMember("compact", &MenuButtonEntry::compact);
            button.RegisterMember("cols2", &MenuButtonEntry::cols2);
            c.RegisterArray<std::vector<MenuButtonEntry>>();
            c.Bind("buttons", &model.buttons);

            c.Bind("has_title", &model.hasTitle);
            c.Bind("is_system_menu", &model.isSystemMenu);
            c.Bind("title", &model.title);

            // window_shell's positioning/dragging extension -- unused here, this dialog stays
            // screen-centered (see GenericMenuRmlModel's own comment).
            c.Bind("positioned", &model.positioned);
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);

            // Position argument is the clicked button's own data-for index (it_index) -- a
            // proven RmlUi pattern already used by char_make.rml/server_select.rml/
            // my_quest_info.rml/main_frame.rml, not a fixed-slot workaround.
            c.BindEventCallback("gmd_button_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (!m_bActive) return;
                    const int index = args.empty() ? -1 : args[0].Get<int>(-1);
                    if (index < 0 || index >= static_cast<int>(m_Active.buttons.size())) return;
                    if (!m_Active.buttons[index].enabled) return;
                    m_bButtonClicked = true;
                    m_iClickedButtonIndex = index;
                });
        });

    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
            "Data/Interface/RmlUi/generic_menu_dialog.rml");
}

void CGenericMenuDialog::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    const bool wasVisible = m_pRmlDoc->IsVisible();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    if (wasVisible)
    {
        SyncRmlModel();
        if (m_pRmlDoc)
            m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
}

void CGenericMenuDialog::Release()
{
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    m_bActive = false;
    m_Queue.clear();
}

void CGenericMenuDialog::Show(GenericMenuConfig cfg)
{
    // Protects a pending menu from being silently overwritten -- same reasoning as
    // CGenericConfirmDialog::Show()'s own comment.
    if (m_bActive)
    {
        m_Queue.push_back(std::move(cfg));
        return;
    }

    m_Active = std::move(cfg);
    m_bActive = true;
    m_bButtonClicked = false;
    m_iClickedButtonIndex = -1;

    if (m_pRmlDoc)
    {
        SyncRmlModel();
        m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
}

void CGenericMenuDialog::ShowNext()
{
    if (m_Queue.empty())
    {
        m_bActive = false;
        return;
    }

    m_Active = std::move(m_Queue.front());
    m_Queue.pop_front();
    m_bButtonClicked = false;
    m_iClickedButtonIndex = -1;

    if (m_pRmlDoc)
    {
        SyncRmlModel();
        m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
}

void CGenericMenuDialog::Resolve(int buttonIndex)
{
    GenericMenuConfig cfg = std::move(m_Active);

    if (buttonIndex >= 0 && buttonIndex < static_cast<int>(cfg.buttons.size()))
    {
        if (cfg.buttons[buttonIndex].onClick) cfg.buttons[buttonIndex].onClick();
    }
    else if (cfg.onCancel)
    {
        cfg.onCancel();
    }

    if (m_pRmlDoc)
        m_pRmlDoc->Hide();

    ShowNext();
}

bool CGenericMenuDialog::Render()
{
    SyncRmlModel();
    return true;
}

bool CGenericMenuDialog::Update()
{
    if (!m_bActive)
        return true;

    if (m_bButtonClicked)
    {
        m_bButtonClicked = false;
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(m_iClickedButtonIndex);
    }

    return true;
}

bool CGenericMenuDialog::UpdateKeyEvent()
{
    if (!m_bActive)
        return true;

    if (mu::ui::window::IsPress(VK_ESCAPE))
    {
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(-1);
        // Fully consumed -- Resolve() may have already flipped IsVisible() to false this same
        // frame (queue empty), and this object now runs before CHotKey (GetKeyEventOrder()), so
        // returning !IsVisible() here would let that same Escape press also reach CHotKey and
        // immediately reopen the system menu right after closing it.
        return false;
    }

    return !IsVisible();
}

void CGenericMenuDialog::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();

    const bool hasTitle = !m_Active.title.empty();
    if (model.hasTitle != hasTitle)
    {
        model.hasTitle = hasTitle;
        m_RmlBinder.MarkDirty("has_title");
    }
    const bool isSystemMenu = m_Active.purpose == GenericMenuConfig::Purpose::SystemMenu;
    if (model.isSystemMenu != isSystemMenu)
    {
        model.isSystemMenu = isSystemMenu;
        m_RmlBinder.MarkDirty("is_system_menu");
    }
    const std::string title = StringUtils::WideToNarrow(m_Active.title.c_str());
    if (model.title != title)
    {
        model.title = title;
        m_RmlBinder.MarkDirty("title");
    }

    std::vector<LineEntry> newLines;
    newLines.reserve(m_Active.lines.size());
    for (const auto& line : m_Active.lines)
        newLines.push_back({ StringUtils::WideToNarrow(line.text.c_str()), line.bold });
    bool linesChanged = newLines.size() != model.lines.size();
    for (size_t i = 0; i < newLines.size() && !linesChanged; ++i)
        linesChanged = newLines[i].text != model.lines[i].text || newLines[i].bold != model.lines[i].bold;
    if (linesChanged)
    {
        model.lines = std::move(newLines);
        m_RmlBinder.MarkDirty("lines");
    }

    std::vector<MenuButtonEntry> newButtons;
    newButtons.reserve(m_Active.buttons.size());
    for (const auto& button : m_Active.buttons)
    {
        MenuButtonEntry entry;
        entry.label = StringUtils::WideToNarrow(button.label.c_str());
        entry.tooltip = StringUtils::WideToNarrow(button.tooltip.c_str());
        entry.lines.reserve(button.lines.size());
        for (const auto& line : button.lines)
            entry.lines.push_back({ StringUtils::WideToNarrow(line.text.c_str()), line.bold });
        entry.hasTooltip = !button.tooltip.empty();
        entry.hasLines = !button.lines.empty();
        entry.enabled = button.enabled;
        entry.compact = button.compact;
        entry.cols2 = (m_Active.columns == 2) && !button.compact;
        newButtons.push_back(std::move(entry));
    }
    bool buttonsChanged = newButtons.size() != model.buttons.size();
    for (size_t i = 0; i < newButtons.size() && !buttonsChanged; ++i)
    {
        const auto& a = newButtons[i];
        const auto& b = model.buttons[i];
        buttonsChanged = a.label != b.label || a.tooltip != b.tooltip
            || a.hasTooltip != b.hasTooltip || a.enabled != b.enabled || a.compact != b.compact
            || a.cols2 != b.cols2 || a.lines.size() != b.lines.size();
        for (size_t j = 0; j < a.lines.size() && !buttonsChanged; ++j)
            buttonsChanged = a.lines[j].text != b.lines[j].text || a.lines[j].bold != b.lines[j].bold;
    }
    if (buttonsChanged)
    {
        model.buttons = std::move(newButtons);
        m_RmlBinder.MarkDirty("buttons");
    }

}

} // namespace mu::ui::window
