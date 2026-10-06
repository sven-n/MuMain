//*****************************************************************************
// File: GenericConfirmDialog.cpp
//*****************************************************************************
#include "stdafx.h"
#include "UI/Dialogs/GenericConfirmDialog.h"

#include "Audio/DSPlaySound.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/Log/MuLogger.h" // item3D positioning diagnostic, see Render3D()
#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzInventory.h" // RenderItem3D
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/Window3DRenderMng.h"  // g_pNewUI3DRenderMng
#include "UI/Core/WindowCommon.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowSystem.h"       // g_pNewUI3DRenderMng macro resolves through CSystem
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDialogCanvas.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "UI/RmlBridge/RmlNumericInputFilter.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#include <algorithm>
#include <numeric>
#include <random>

namespace mu::ui::window
{

CGenericConfirmDialog* g_pGenericConfirmDialog = nullptr;

namespace
{
    // Reference-space size of the 3D-item preview slot -- matches C3DItemCommonMsgBox's own
    // MSGBOX_3DITEM_WIDTH/HEIGHT (CommonMessageBox.h) exactly.
    constexpr float kItem3DSize = 40.0f;
}

void CGenericConfirmDialog::Create(CManager* pMng)
{
    Release();

    BuildRmlUi();

    pMng->AddUIObj(mu::ui::window::INTERFACE_GENERIC_CONFIRM_DIALOG, this);

    // Registered for this object's whole lifetime -- Render3D() below no-ops whenever the active
    // config has no `item3D`, and the camera's own IsVisible() gate skips calling it at all while
    // no dialog is active, so there's no cost to staying registered between dialogs.
    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Add3DRenderObj(this);
}

void CGenericConfirmDialog::BindRmlModel(Rml::DataModelConstructor& c, GenericDialogRmlModel& model)
{
    auto line = c.RegisterStruct<LineEntry>();
    line.RegisterMember("text", &LineEntry::text);
    line.RegisterMember("bold", &LineEntry::bold);
    line.RegisterMember("color", &LineEntry::color);
    c.RegisterArray<std::vector<LineEntry>>();
    c.Bind("lines", &model.lines);

    c.Bind("primary_label", &model.primaryLabel);
    c.Bind("primary_is_stock_ok", &model.primaryIsStockOk);
    c.Bind("has_secondary", &model.hasSecondary);
    c.Bind("secondary_label", &model.secondaryLabel);
    c.Bind("show_cancel", &model.showCancel);
    c.Bind("cancel_label", &model.cancelLabel);
    c.Bind("cancel_is_stock_cancel", &model.cancelIsStockCancel);

    c.Bind("has_title", &model.hasTitle);
    c.Bind("title", &model.title);
    c.Bind("severity_warning", &model.severityWarning);
    c.Bind("severity_error", &model.severityError);

    c.Bind("has_input", &model.hasInput);
    c.Bind("input_is_keypad", &model.inputIsKeypad);
    c.Bind("input_text", &model.inputText);
    c.Bind("input_value", &model.inputValue);
    c.Bind("keypad_digit_0", &model.keypadDigit0);
    c.Bind("keypad_digit_1", &model.keypadDigit1);
    c.Bind("keypad_digit_2", &model.keypadDigit2);
    c.Bind("keypad_digit_3", &model.keypadDigit3);
    c.Bind("keypad_digit_4", &model.keypadDigit4);
    c.Bind("keypad_digit_5", &model.keypadDigit5);
    c.Bind("keypad_digit_6", &model.keypadDigit6);
    c.Bind("keypad_digit_7", &model.keypadDigit7);
    c.Bind("keypad_digit_8", &model.keypadDigit8);
    c.Bind("keypad_digit_9", &model.keypadDigit9);

    c.Bind("has_progress", &model.hasProgress);
    c.Bind("canvas_top", &model.canvasTop);
    c.Bind("progress_fraction", &model.progressFraction);

    c.Bind("has_item3d", &model.hasItem3D);

    c.Bind("has_portrait2d_overlay", &model.hasPortrait2DOverlay);
    c.Bind("has_portrait2d_beside", &model.hasPortrait2DBeside);
    c.Bind("portrait2d_text", &model.portrait2DText);

    c.Bind("has_tall_panel", &model.hasTallPanel);

    c.BindEventCallback("gcd_primary_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_bPrimaryClicked = true; });
    c.BindEventCallback("gcd_secondary_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_bSecondaryClicked = true; });
    c.BindEventCallback("gcd_cancel_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_bCancelClicked = true; });

    // Position argument is the FIXED grid slot (0-9), not the digit typed -- that's
    // looked up via m_KeypadMapping, the shuffled-position anti-shoulder-surfing mapping.
    c.BindEventCallback("gcd_keypad_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
        {
            if (!m_bActive || !m_Active.input) return;
            const int slot = args.empty() ? -1 : args[0].Get<int>();
            if (slot < 0 || slot >= static_cast<int>(m_KeypadMapping.size())) return;
            if (static_cast<int>(m_KeypadBuffer.size()) >= m_Active.input->maxLength) return;
            m_KeypadBuffer += static_cast<wchar_t>(L'0' + m_KeypadMapping[slot]);
        });
    c.BindEventCallback("gcd_keypad_delete_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            if (!m_KeypadBuffer.empty())
                m_KeypadBuffer.pop_back();
        });
}

void CGenericConfirmDialog::OnRmlBuilt()
{
    Rml::ElementDocument* document = m_RmlView.Document();
    UI::RmlBridge::AttachNumericInputFilter(document);
    // Dragged by any part that is not a control (base.rcss blocks those) for this dialog only.
    if (Rml::Element* panel = document->GetElementById("panel"))
        UI::RmlBridge::MakeDraggable(panel, panel, [this](float, float) { m_bDragged = true; },
                                     [panel] { UI::RmlBridge::KeepInsideWindow(panel); });
}

// The chrome's "tall" class, the dialog above its chrome, and the input field's type, limit and
// focus are set when a dialog shows; a rebuilt document has none of them.
void CGenericConfirmDialog::OnRmlReloaded()
{
    Rml::ElementDocument* document = m_RmlView.Document();
    if (document == nullptr || !document->IsVisible())
        return;
    ShowChrome();
    document->PullToFront();
    ApplyInputFieldConfig(m_RmlView.GetModel().inputValue);
}

// The chrome first: a theme switch rebuilds in the order the views were built, and the dialog has
// to come back above its chrome.
void CGenericConfirmDialog::BuildRmlUi()
{
    m_RmlChromeView.Ensure();
    m_RmlBgView.Ensure();
    m_RmlView.Ensure();
}

Rml::ElementDocument* CGenericConfirmDialog::ActiveChromeDocument() const
{
    return m_Active.item3D ? m_RmlBgView.Document() : m_RmlChromeView.Document();
}

void CGenericConfirmDialog::ShowChrome()
{
    Rml::ElementDocument* chrome = ActiveChromeDocument();
    Rml::ElementDocument* other = chrome == m_RmlBgView.Document() ? m_RmlChromeView.Document() : m_RmlBgView.Document();
    if (other)
        other->Hide();
    if (!chrome)
        return;

    chrome->Show(Rml::ModalFlag::None, Rml::FocusFlag::None);
    chrome->PullToFront();
    // The bg documents have no data model of their own -- tallPanel's "tall" class is mirrored
    // onto their #panel imperatively instead.
    if (Rml::Element* bgPanel = chrome->GetElementById("panel"))
        bgPanel->SetClass("tall", m_Active.tallPanel);
}

void CGenericConfirmDialog::HideChrome()
{
    if (m_RmlBgView.Document())
        m_RmlBgView.Document()->Hide();
    if (m_RmlChromeView.Document())
        m_RmlChromeView.Document()->Hide();
}

void CGenericConfirmDialog::Release()
{
    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Remove3DRenderObj(this);

    DismissActive();
    m_Queue.clear();

    m_RmlView.Release();
    m_RmlBgView.Release();
    m_RmlChromeView.Release();
}

namespace
{
    // 20 random adjacent swaps over 0..9 -- the shuffled keypad-digit mapping.
    std::vector<int> ShuffledDigits()
    {
        std::vector<int> digits(10);
        std::iota(digits.begin(), digits.end(), 0);
        static std::mt19937 rng{ std::random_device{}() };
        std::shuffle(digits.begin(), digits.end(), rng);
        return digits;
    }

}

CGenericConfirmDialog::DialogId CGenericConfirmDialog::Show(GenericDialogConfig cfg)
{
    // onSecondary only fires from the secondary button; Cancel and Esc fire onCancel.
    assert(!cfg.onSecondary || cfg.secondaryLabel);

    if (cfg.isValid && !cfg.isValid())
        return 0;

    const DialogId id = m_NextId++;
    if (m_bActive)
    {
        m_Queue.push_back({id, std::move(cfg)});
        return id;
    }

    Activate(std::move(cfg), id);
    return id;
}

void CGenericConfirmDialog::Activate(GenericDialogConfig cfg, DialogId id)
{
    m_Active = std::move(cfg);
    m_ActiveId = id;
    m_bActive = true;
    m_bPrimaryClicked = false;
    m_bSecondaryClicked = false;
    m_bCancelClicked = false;
    m_KeypadBuffer.clear();
    m_KeypadMapping.clear();
    m_bItem3DDebugLogged = false;

    // Each dialog opens where its theme puts it, wherever the last one was dragged.
    m_bDragged = false;
    if (m_RmlView.Document())
        UI::RmlBridge::ResetDraggedPosition(m_RmlView.Document()->GetElementById("panel"));
    for (Rml::ElementDocument* chrome : {m_RmlBgView.Document(), m_RmlChromeView.Document()})
        UI::RmlBridge::ResetDraggedPosition(chrome ? chrome->GetElementById("panel") : nullptr);

    if (m_Active.input && m_Active.input->mode == GenericDialogConfig::InputField::Mode::NumericKeypad)
        m_KeypadMapping = ShuffledDigits();

    if (m_Active.progress)
    {
        m_dwProgressStartTime = timeGetTime();
        m_dwProgressEndTime = m_dwProgressStartTime + m_Active.progress->elapseMs;
    }

    ShowChrome();
    if (m_RmlView.Document())
    {
        SyncRmlModel();
        // Modal: blocks the game world/other UI from stealing focus or clicks while this is open.
        m_RmlView.Document()->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
        m_RmlView.Document()->PullToFront();
        ApplyInputFieldConfig(InitialInputText());
    }
}

void CGenericConfirmDialog::ShowNext()
{
    while (!m_Queue.empty())
    {
        PendingDialog next = std::move(m_Queue.front());
        m_Queue.pop_front();
        if (next.config.isValid && !next.config.isValid())
            continue;
        Activate(std::move(next.config), next.id);
        return;
    }
    m_bActive = false;
    m_ActiveId = 0;
}

void CGenericConfirmDialog::DismissActive()
{
    if (m_RmlView.Document())
    {
        if (auto* field = m_RmlView.Document()->GetElementById("gcd_input"))
            field->Blur();
        m_RmlView.Document()->Hide();
    }
    HideChrome();
    m_RmlView.GetModel().inputValue.clear();
    m_RmlView.MarkDirty("input_value");
    m_Active = {};
    m_ActiveId = 0;
    m_bActive = false;
}

bool CGenericConfirmDialog::IsPending(DialogId id) const
{
    return IsActive(id) || std::any_of(m_Queue.begin(), m_Queue.end(),
        [id](const PendingDialog& entry) { return entry.id == id; });
}

void CGenericConfirmDialog::Cancel(DialogId id)
{
    if (id == 0)
        return;
    std::erase_if(m_Queue, [id](const PendingDialog& entry) { return entry.id == id; });
    if (IsActive(id))
    {
        DismissActive();
        ShowNext();
    }
}

void CGenericConfirmDialog::SetInputText(DialogId id, const std::wstring& text)
{
    if (!IsActive(id) || !m_Active.input ||
        m_Active.input->mode != GenericDialogConfig::InputField::Mode::Text)
        return;
    const auto value = text.substr(0, static_cast<size_t>(std::max(m_Active.input->maxLength, 0)));
    ApplyInputFieldConfig(StringUtils::WideToNarrow(value.c_str()));
}

void CGenericConfirmDialog::Resolve(ClickResult which)
{
    if (m_Active.isValid && !m_Active.isValid())
    {
        Cancel(m_ActiveId);
        return;
    }
    m_bKeepOpenRequested = false;

    // Move out before invoking -- the callback may itself call Show() (e.g. chaining a follow-up
    // confirm), which must not stomp m_Active while its own onPrimary/onSecondary/onCancel still
    // needs it. Deliberately BEFORE hiding/ShowNext(): a callback that vetoes via KeepOpen() needs
    // nothing touched yet -- not the RmlUi documents, not the native Mode::Text widget, not the queue.
    GenericDialogConfig cfg = std::move(m_Active);
    switch (which)
    {
    case ClickResult::Primary:   if (cfg.onPrimary)   cfg.onPrimary();   break;
    case ClickResult::Secondary: if (cfg.onSecondary) cfg.onSecondary(); break;
    case ClickResult::Cancel:    if (cfg.onCancel)    cfg.onCancel();    break;
    }

    if (m_bKeepOpenRequested)
    {
        // Validation failed -- put everything back exactly as it was.
        m_Active = std::move(cfg);
        return;
    }

    DismissActive();
    ShowNext();
}

bool CGenericConfirmDialog::Render()
{
    // RmlUi's #panel owns this dialog's entire 2D visual, the Mode::Text field included (it's a
    // stock <input> in that same document now). item3D still renders via Render3D() below
    // (I3DRenderObj).
    SyncRmlModel();
    return true;
}

Rml::String CGenericConfirmDialog::InitialInputText() const
{
    return m_Active.input ? StringUtils::WideToNarrow(m_Active.input->initialText.c_str()) : Rml::String();
}

void CGenericConfirmDialog::ApplyInputFieldConfig(const Rml::String& value)
{
    if (!m_RmlView.Document() || !m_Active.input
        || m_Active.input->mode != GenericDialogConfig::InputField::Mode::Text)
        return;

    Rml::Element* field = m_RmlView.Document()->GetElementById("gcd_input");
    if (field == nullptr)
        return;

    // Order matters: changing "type" makes RmlUi tear down and rebuild the element's InputType
    // (ElementFormControlInput::OnAttributeChange), dropping whatever value it held -- so the
    // type/limit go on first and the seeded value last.
    field->SetAttribute("type", m_Active.input->masked ? "password" : "text");
    field->SetAttribute("maxlength", m_Active.input->maxLength);
    field->SetClass(UI::RmlBridge::NumericFieldClass, m_Active.input->numericOnly);

    m_RmlView.GetModel().inputValue = value;
    m_RmlView.MarkDirty("input_value");

    // This dialog opens with FocusFlag::Document, so the field needs an explicit focus rather than
    // an autofocus attribute -- the attribute would also fight the keypad mode, which shares the row.
    field->Focus();
}

Rml::Vector2f CGenericConfirmDialog::PanelTranslateCorrection() const
{
    Rml::Element* pPanel = m_RmlView.Document() ? m_RmlView.Document()->GetElementById("panel") : nullptr;
    if (!pPanel)
        return { 0.f, 0.f };
    // A theme may place #panel without the centering transform (legacy anchors it like native).
    if (!pPanel->GetComputedValues().has_local_transform())
        return {0.f, 0.f};
    const Rml::Vector2f size = pPanel->GetBox().GetSize();
    return { -size.x * 0.5f, -size.y * 0.5f };
}

void CGenericConfirmDialog::SyncBackgroundPanel()
{
    Rml::Element* pPanel = m_RmlView.Document() ? m_RmlView.Document()->GetElementById("panel") : nullptr;
    Rml::ElementDocument* chrome = ActiveChromeDocument();
    Rml::Element* pBgPanel = chrome ? chrome->GetElementById("panel") : nullptr;
    if (!pPanel || !pBgPanel)
        return;

    const float height = pPanel->GetBox().GetSize(Rml::BoxArea::Border).y;
    if (height > 0.f && height != pBgPanel->GetBox().GetSize(Rml::BoxArea::Border).y)
        pBgPanel->SetProperty(Rml::PropertyId::Height, Rml::Property(height, Rml::Unit::PX));

    if (m_bDragged)
    {
        // The chrome takes the dragged panel's drawn top-left, without its own centring.
        const Rml::Vector2f at = pPanel->GetAbsoluteOffset(Rml::BoxArea::Border);
        if (at == pBgPanel->GetAbsoluteOffset(Rml::BoxArea::Border))
            return;
        const Rml::Vector2f origin = pBgPanel->GetAbsoluteOffset(Rml::BoxArea::Border)
            - Rml::Vector2f(pBgPanel->GetOffsetLeft(), pBgPanel->GetOffsetTop());
        pBgPanel->SetClass("dragged", true);
        pBgPanel->SetProperty(Rml::PropertyId::Left, Rml::Property(at.x - origin.x, Rml::Unit::PX));
        pBgPanel->SetProperty(Rml::PropertyId::Top, Rml::Property(at.y - origin.y, Rml::Unit::PX));
        return;
    }

    // Only a panel placed without the centering transform reports its real top edge.
    if (pPanel->GetComputedValues().has_local_transform())
        return;
    const float top = pPanel->GetAbsoluteOffset(Rml::BoxArea::Border).y;
    if (top != pBgPanel->GetAbsoluteOffset(Rml::BoxArea::Border).y)
        pBgPanel->SetProperty(Rml::PropertyId::Top, Rml::Property(top, Rml::Unit::PX));
}

void CGenericConfirmDialog::UpdateProgress()
{
    if (!m_Active.progress)
        return;

    const DWORD now = timeGetTime();
    const DWORD elapse = m_Active.progress->elapseMs > 0 ? m_Active.progress->elapseMs : 1;
    const float fraction = static_cast<float>(now - m_dwProgressStartTime) / static_cast<float>(elapse);

    if (now >= m_dwProgressEndTime)
    {
        // Elapse-and-close; onPrimary (if set) stands in for the auto-close side effect. No
        // onSecondary/onCancel path exists for progress dialogs.
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(ClickResult::Primary);
        return;
    }

    auto& model = m_RmlView.GetModel();
    if (model.progressFraction != fraction)
    {
        model.progressFraction = fraction;
        m_RmlView.MarkDirty("progress_fraction");
    }
}

bool CGenericConfirmDialog::Update()
{
    if (!m_bActive)
        return true;

    if (m_Active.isValid && !m_Active.isValid())
    {
        Cancel(m_ActiveId);
        return true;
    }

    SyncCanvasTop();
    SyncBackgroundPanel();

    if (m_Active.progress)
    {
        UpdateProgress();
        return true; // progress dialogs have no buttons to poll
    }

    if (m_bPrimaryClicked)
    {
        m_bPrimaryClicked = false;
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(ClickResult::Primary);
    }
    else if (m_bSecondaryClicked)
    {
        m_bSecondaryClicked = false;
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(ClickResult::Secondary);
    }
    else if (m_bCancelClicked)
    {
        m_bCancelClicked = false;
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(ClickResult::Cancel);
    }

    return true;
}

bool CGenericConfirmDialog::UpdateKeyEvent()
{
    if (!m_bActive)
        return true;

    // Progress dialogs are non-interactive countdowns natively (no button, no Esc-to-dismiss).
    if (!m_Active.progress)
    {
        if (mu::ui::window::IsPress(VK_RETURN))
        {
            ::PlayBuffer(SOUND_CLICK01);
            Resolve(ClickResult::Primary);
            // Fully consumed -- see CGenericMenuDialog::UpdateKeyEvent()'s own copy of this
            // comment for why (this object now runs before CHotKey; !IsVisible() here could let
            // the same keypress also reach CHotKey the instant this dialog closes).
            return false;
        }
        else if (mu::ui::window::IsPress(VK_ESCAPE))
        {
            ::PlayBuffer(SOUND_CLICK01);
            // Esc never triggers the real `secondary` action -- only dismiss semantics. Mirrors
            // today's exact behavior: no cancel button configured means Esc acts like Primary
            // (matches the old `ButtonSet::Ok` case), otherwise it's a genuine Cancel.
            Resolve(m_Active.showCancel ? ClickResult::Cancel : ClickResult::Primary);
            return false;
        }
    }

    return !IsVisible();
}

void CGenericConfirmDialog::Render3D()
{
    // Guarded here, not by staying unregistered -- see Create()'s own comment.
    //
    // KNOWN GAP: this draws behind the dialog's own opaque panel background, since RmlUi's main
    // context always composites LAST in the frame -- see the class's header comment.
    if (!m_bActive || !m_Active.item3D || !m_RmlView.Document())
        return;

    Rml::Element* pAnchor = m_RmlView.Document()->GetElementById("gcd_item3d_anchor");
    if (!pAnchor)
        return;

    // RenderItem3D() expects REFERENCE-space coordinates and re-applies the active transform
    // internally to reach real screen pixels, so the anchor's real screen position (+
    // PanelTranslateCorrection()) must be converted back to reference space via LogicalX/LogicalY.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    const Rml::Vector2f correction = PanelTranslateCorrection();
    const Rml::Vector2f rawScreenPos = pAnchor->GetAbsoluteOffset();
    const Rml::Vector2f screenPos = { rawScreenPos.x + correction.x, rawScreenPos.y + correction.y };
    const float refX = UI::Scaling::LogicalX(transform, screenPos.x);
    const float refY = UI::Scaling::LogicalY(transform, screenPos.y);

    if (!m_bItem3DDebugLogged)
    {
        m_bItem3DDebugLogged = true;
        mu::log::Get("ui")->info(
            "GenericConfirmDialog item3D debug -- window={}x{} rawAnchorAbsOffset=({:.1f},{:.1f}) "
            "panelTranslateCorrection=({:.1f},{:.1f}) correctedScreenPos=({:.1f},{:.1f}) refXY=({:.1f},{:.1f})",
            WindowWidth, WindowHeight, rawScreenPos.x, rawScreenPos.y,
            correction.x, correction.y, screenPos.x, screenPos.y, refX, refY);
    }

    const ITEM& item = *m_Active.item3D;
    RenderItem3D(refX, refY, kItem3DSize, kItem3DSize, item.Type, item.Level, item.ExcellentFlags,
                 item.AncientDiscriminator, /*PickUp=*/true);
}

std::wstring CGenericConfirmDialog::GetInputText() const
{
    if (!m_Active.input)
        return L"";

    if (m_Active.input->mode == GenericDialogConfig::InputField::Mode::NumericKeypad)
        return m_KeypadBuffer;

    // Mode::Text -- RmlUi owns the edit buffer; the model holds the committed value. Filtered again
    // here because the numeric input filter only covers typed input: a clipboard paste reaches
    // WidgetTextInput without a textinput event, so this is what makes the rule hold either way.
    const Rml::String& value = m_RmlView.GetModel().inputValue;
    return StringUtils::NarrowToWide(m_Active.input->numericOnly ? UI::RmlBridge::KeepDigitsOnly(value) : value);
}

void CGenericConfirmDialog::SyncCanvasTop()
{
    auto& model = m_RmlView.GetModel();
    const float canvasTop = UI::RmlBridge::DialogCanvasTop(RmlUiRuntime::Instance().GetContext());
    if (model.canvasTop == canvasTop)
        return;
    model.canvasTop = canvasTop;
    m_RmlView.MarkDirty("canvas_top");
}

void CGenericConfirmDialog::SyncRmlModel()
{
    if (!m_RmlView.Document()) return;

    SyncCanvasTop();
    auto& model = m_RmlView.GetModel();

    std::vector<LineEntry> newLines;
    newLines.reserve(m_Active.lines.size());
    for (const auto& line : m_Active.lines)
        newLines.push_back(
            {StringUtils::WideToNarrow(line.text.c_str()), line.bold, UI::RmlBridge::RgbaToCss(line.color)});

    bool linesChanged = newLines.size() != model.lines.size();
    for (size_t i = 0; i < newLines.size() && !linesChanged; ++i)
        linesChanged = newLines[i].text != model.lines[i].text || newLines[i].bold != model.lines[i].bold ||
                       newLines[i].color != model.lines[i].color;
    if (linesChanged)
    {
        model.lines = std::move(newLines);
        m_RmlView.MarkDirty("lines");
    }

    const std::string primaryLabel = StringUtils::WideToNarrow(m_Active.primaryLabel.c_str());
    if (model.primaryLabel != primaryLabel)
    {
        model.primaryLabel = primaryLabel;
        m_RmlView.MarkDirty("primary_label");
    }
    const bool primaryIsStockOk = m_Active.primaryLabel == GenericDialogConfig{}.primaryLabel;
    if (model.primaryIsStockOk != primaryIsStockOk)
    {
        model.primaryIsStockOk = primaryIsStockOk;
        m_RmlView.MarkDirty("primary_is_stock_ok");
    }

    const bool hasSecondary = m_Active.secondaryLabel.has_value();
    if (model.hasSecondary != hasSecondary)
    {
        model.hasSecondary = hasSecondary;
        m_RmlView.MarkDirty("has_secondary");
    }
    const std::string secondaryLabel = hasSecondary
        ? StringUtils::WideToNarrow(m_Active.secondaryLabel->c_str()) : std::string();
    if (model.secondaryLabel != secondaryLabel)
    {
        model.secondaryLabel = secondaryLabel;
        m_RmlView.MarkDirty("secondary_label");
    }

    if (model.showCancel != m_Active.showCancel)
    {
        model.showCancel = m_Active.showCancel;
        m_RmlView.MarkDirty("show_cancel");
    }
    const std::string cancelLabel = StringUtils::WideToNarrow(m_Active.cancelLabel.c_str());
    if (model.cancelLabel != cancelLabel)
    {
        model.cancelLabel = cancelLabel;
        m_RmlView.MarkDirty("cancel_label");
    }
    const bool cancelIsStockCancel = m_Active.cancelLabel == GenericDialogConfig{}.cancelLabel;
    if (model.cancelIsStockCancel != cancelIsStockCancel)
    {
        model.cancelIsStockCancel = cancelIsStockCancel;
        m_RmlView.MarkDirty("cancel_is_stock_cancel");
    }

    const bool hasTitle = !m_Active.title.empty();
    if (model.hasTitle != hasTitle)
    {
        model.hasTitle = hasTitle;
        m_RmlView.MarkDirty("has_title");
    }
    const std::string title = StringUtils::WideToNarrow(m_Active.title.c_str());
    if (model.title != title)
    {
        model.title = title;
        m_RmlView.MarkDirty("title");
    }

    const bool severityWarning = m_Active.severity == GenericDialogConfig::Severity::Warning;
    if (model.severityWarning != severityWarning)
    {
        model.severityWarning = severityWarning;
        m_RmlView.MarkDirty("severity_warning");
    }
    const bool severityError = m_Active.severity == GenericDialogConfig::Severity::Error;
    if (model.severityError != severityError)
    {
        model.severityError = severityError;
        m_RmlView.MarkDirty("severity_error");
    }

    const bool hasInput = m_Active.input.has_value();
    if (model.hasInput != hasInput)
    {
        model.hasInput = hasInput;
        m_RmlView.MarkDirty("has_input");
    }
    const bool inputIsKeypad = hasInput
        && m_Active.input->mode == GenericDialogConfig::InputField::Mode::NumericKeypad;
    if (model.inputIsKeypad != inputIsKeypad)
    {
        model.inputIsKeypad = inputIsKeypad;
        m_RmlView.MarkDirty("input_is_keypad");
    }
    // Always masked, matching the native keypad's own unconditional asterisk mask.
    const std::string inputText = inputIsKeypad ? std::string(m_KeypadBuffer.size(), '*') : std::string();
    if (model.inputText != inputText)
    {
        model.inputText = inputText;
        m_RmlView.MarkDirty("input_text");
    }
    if (inputIsKeypad && m_KeypadMapping.size() == 10)
    {
        int* const slots[10] = {
            &model.keypadDigit0, &model.keypadDigit1, &model.keypadDigit2, &model.keypadDigit3,
            &model.keypadDigit4, &model.keypadDigit5, &model.keypadDigit6, &model.keypadDigit7,
            &model.keypadDigit8, &model.keypadDigit9,
        };
        static const char* const kFieldNames[10] = {
            "keypad_digit_0", "keypad_digit_1", "keypad_digit_2", "keypad_digit_3", "keypad_digit_4",
            "keypad_digit_5", "keypad_digit_6", "keypad_digit_7", "keypad_digit_8", "keypad_digit_9",
        };
        for (int i = 0; i < 10; ++i)
        {
            if (*slots[i] != m_KeypadMapping[i])
            {
                *slots[i] = m_KeypadMapping[i];
                m_RmlView.MarkDirty(kFieldNames[i]);
            }
        }
    }

    const bool hasProgress = m_Active.progress.has_value();
    if (model.hasProgress != hasProgress)
    {
        model.hasProgress = hasProgress;
        m_RmlView.MarkDirty("has_progress");
    }

    const bool hasItem3D = m_Active.item3D.has_value();
    if (model.hasItem3D != hasItem3D)
    {
        model.hasItem3D = hasItem3D;
        m_RmlView.MarkDirty("has_item3d");
    }

    const bool hasPortrait2DOverlay = m_Active.portrait2D.has_value()
        && m_Active.portrait2D->layout == GenericDialogConfig::Portrait2D::Layout::Overlay;
    if (model.hasPortrait2DOverlay != hasPortrait2DOverlay)
    {
        model.hasPortrait2DOverlay = hasPortrait2DOverlay;
        m_RmlView.MarkDirty("has_portrait2d_overlay");
    }
    const bool hasPortrait2DBeside = m_Active.portrait2D.has_value()
        && m_Active.portrait2D->layout == GenericDialogConfig::Portrait2D::Layout::Beside;
    if (model.hasPortrait2DBeside != hasPortrait2DBeside)
    {
        model.hasPortrait2DBeside = hasPortrait2DBeside;
        m_RmlView.MarkDirty("has_portrait2d_beside");
    }
    const std::string portrait2DText = hasPortrait2DOverlay
        ? StringUtils::WideToNarrow(m_Active.portrait2D->text.c_str()) : std::string();
    if (model.portrait2DText != portrait2DText)
    {
        model.portrait2DText = portrait2DText;
        m_RmlView.MarkDirty("portrait2d_text");
    }

    const bool hasTallPanel = m_Active.tallPanel;
    if (model.hasTallPanel != hasTallPanel)
    {
        model.hasTallPanel = hasTallPanel;
        m_RmlView.MarkDirty("has_tall_panel");
    }
}

} // namespace mu::ui::window
