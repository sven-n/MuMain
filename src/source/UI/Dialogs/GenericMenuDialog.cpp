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
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDialogCanvas.h"
#include "UI/RmlBridge/RmlDraggable.h"
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
            line.RegisterMember("color", &LineEntry::color);
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
            button.RegisterMember("native_top", &MenuButtonEntry::nativeTop);
            button.RegisterMember("native_button_gap", &MenuButtonEntry::nativeButtonGap);
            button.RegisterMember("narrow", &MenuButtonEntry::narrow);
            button.RegisterMember("lines_below", &MenuButtonEntry::linesBelow);
            c.RegisterArray<std::vector<MenuButtonEntry>>();
            c.Bind("buttons", &model.buttons);

            c.Bind("has_title", &model.hasTitle);
            c.Bind("highlight_title", &model.highlightTitle);
            c.Bind("is_system_menu", &model.isSystemMenu);
            c.Bind("native_top", &model.nativeTop);
            c.Bind("native_height", &model.nativeHeight);
            c.Bind("native_text_top", &model.nativeTextTop);
            c.Bind("native_line_advance", &model.nativeLineAdvance);
            c.Bind("native_text_inset", &model.nativeTextInset);
            c.Bind("native_divider_top", &model.nativeDividerTop);
            c.Bind("canvas_top", &model.canvasTop);
            c.Bind("title", &model.title);

            // window_shell's positioning extension -- unused here: the theme centres this dialog
            // and the player may drag it (see MakeDraggable() below).
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
    // Dragged by any part that is not a button (base.rcss blocks those): most menus, the system
    // menu among them, have no title to grab.
    if (m_pRmlDoc)
        UI::RmlBridge::MakeDraggable(m_pRmlDoc, m_pRmlDoc, nullptr,
                                     [this] { UI::RmlBridge::KeepInsideWindow(m_pRmlDoc); });
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
    // Heap-owned by CSystem, so it can be destroyed before shutdown -- see
    // RegisterForThemeReload()'s contract.
    UI::RmlBridge::UnregisterForThemeReload(this);

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
        UI::RmlBridge::ResetDraggedPosition(m_pRmlDoc);
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
        UI::RmlBridge::ResetDraggedPosition(m_pRmlDoc);
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

    SyncCanvasTop();

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

CGenericMenuDialog::LineEntry CGenericMenuDialog::ToLineEntry(const GenericMenuConfig::Line& line)
{
    return {StringUtils::WideToNarrow(line.text.c_str()), line.bold, UI::RmlBridge::RgbaToCss(line.color)};
}

bool CGenericMenuDialog::SameLine(const LineEntry& a, const LineEntry& b)
{
    return a.text == b.text && a.bold == b.bold && a.color == b.color;
}

void CGenericMenuDialog::SyncNativeFrame()
{
    auto& model = m_RmlBinder.GetModel();
    // Native CNewUIMessageBoxBase frame heights: 67 top cap + n * 15 middle strips + 50 bottom cap.
    constexpr float kTopCapHeight = 67.f;
    constexpr float kMiddleStripHeight = 15.f;
    constexpr float kBottomCapHeight = 50.f;

    const auto& frame = m_Active.nativeFrame;
    const float top = static_cast<float>(frame.top);
    const float height =
        frame.middleCount > 0
            ? kTopCapHeight + static_cast<float>(frame.middleCount) * kMiddleStripHeight + kBottomCapHeight
            : 0.f;
    if (model.nativeTop != top)
    {
        model.nativeTop = top;
        m_RmlBinder.MarkDirty("native_top");
    }
    if (model.nativeHeight != height)
    {
        model.nativeHeight = height;
        m_RmlBinder.MarkDirty("native_height");
    }

    SyncField(m_RmlBinder, &GenericMenuRmlModel::nativeTextTop, "native_text_top", static_cast<float>(frame.textTop));
    SyncField(m_RmlBinder, &GenericMenuRmlModel::nativeLineAdvance, "native_line_advance", static_cast<float>(frame.lineAdvance));
    SyncField(m_RmlBinder, &GenericMenuRmlModel::nativeTextInset, "native_text_inset", static_cast<float>(frame.textInset));
    SyncField(m_RmlBinder, &GenericMenuRmlModel::nativeDividerTop, "native_divider_top", static_cast<float>(frame.dividerTop));
}

void CGenericMenuDialog::SyncCanvasTop()
{
    auto& model = m_RmlBinder.GetModel();
    const float canvasTop = UI::RmlBridge::DialogCanvasTop(RmlUiRuntime::Instance().GetContext());
    if (model.canvasTop == canvasTop)
        return;
    model.canvasTop = canvasTop;
    m_RmlBinder.MarkDirty("canvas_top");
}

void CGenericMenuDialog::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    SyncCanvasTop();
    auto& model = m_RmlBinder.GetModel();

    const bool hasTitle = !m_Active.title.empty();
    if (model.hasTitle != hasTitle)
    {
        model.hasTitle = hasTitle;
        m_RmlBinder.MarkDirty("has_title");
    }
    if (model.highlightTitle != m_Active.highlightTitle)
    {
        model.highlightTitle = m_Active.highlightTitle;
        m_RmlBinder.MarkDirty("highlight_title");
    }
    const bool isSystemMenu = m_Active.systemMenu;
    if (model.isSystemMenu != isSystemMenu)
    {
        model.isSystemMenu = isSystemMenu;
        m_RmlBinder.MarkDirty("is_system_menu");
    }
    SyncNativeFrame();
    const std::string title = StringUtils::WideToNarrow(m_Active.title.c_str());
    if (model.title != title)
    {
        model.title = title;
        m_RmlBinder.MarkDirty("title");
    }

    std::vector<LineEntry> newLines;
    newLines.reserve(m_Active.lines.size());
    for (const auto& line : m_Active.lines)
        newLines.push_back(ToLineEntry(line));
    bool linesChanged = newLines.size() != model.lines.size();
    for (size_t i = 0; i < newLines.size() && !linesChanged; ++i)
        linesChanged = !SameLine(newLines[i], model.lines[i]);
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
            entry.lines.push_back(ToLineEntry(line));
        entry.hasTooltip = !button.tooltip.empty();
        entry.hasLines = !button.lines.empty();
        entry.enabled = button.enabled;
        entry.compact = button.compact;
        entry.cols2 = (m_Active.columns == 2) && !button.compact;
        entry.nativeTop = static_cast<float>(button.nativeTop);
        entry.narrow = button.narrow;
        entry.linesBelow = button.linesBelow;
        const float lineAdvance = static_cast<float>(m_Active.nativeFrame.lineAdvance);
        if (button.nativeTop > 0 && button.nativeLinesTop > 0 && lineAdvance > 0.f && !button.linesBelow)
        {
            // The cell starts at the first line's box, which is centred on the 9-unit glyphs like
            // generic_menu_dialog.rml's .gmd-lines; the button keeps its own native offset.
            entry.nativeTop = static_cast<float>(button.nativeLinesTop) - (lineAdvance - 9.f) / 2.f;
            entry.nativeButtonGap = static_cast<float>(button.nativeTop) - entry.nativeTop -
                                    static_cast<float>(button.lines.size()) * lineAdvance;
        }
        newButtons.push_back(std::move(entry));
    }
    bool buttonsChanged = newButtons.size() != model.buttons.size();
    for (size_t i = 0; i < newButtons.size() && !buttonsChanged; ++i)
    {
        const auto& a = newButtons[i];
        const auto& b = model.buttons[i];
        buttonsChanged = a.label != b.label || a.tooltip != b.tooltip || a.hasTooltip != b.hasTooltip ||
                         a.enabled != b.enabled || a.compact != b.compact || a.cols2 != b.cols2 ||
                         a.nativeTop != b.nativeTop || a.nativeButtonGap != b.nativeButtonGap || a.narrow != b.narrow ||
                         a.linesBelow != b.linesBelow || a.lines.size() != b.lines.size();
        for (size_t j = 0; j < a.lines.size() && !buttonsChanged; ++j)
            buttonsChanged = !SameLine(a.lines[j], b.lines[j]);
    }
    if (buttonsChanged)
    {
        model.buttons = std::move(newButtons);
        m_RmlBinder.MarkDirty("buttons");
    }

}

} // namespace mu::ui::window
