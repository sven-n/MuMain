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
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "UI/Widgets/UIControls.h"      // g_pSingleTextInputBox, InputBoxConfig, SaveIMEStatus

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#include <algorithm>
#include <numeric>
#include <random>

// Same ad-hoc extern convention every other native window/dialog in this codebase uses for
// g_iChatInputType. Default is 1 (Winmain.cpp) -- the modern g_pSingleTextInputBox portable
// widget path, the only one this class implements (see GetInputText()'s own comment).
extern int g_iChatInputType;

namespace mu::ui::window
{

CGenericConfirmDialog* g_pGenericConfirmDialog = nullptr;

namespace
{
    // Reference-space size of the 3D-item preview slot -- matches C3DItemCommonMsgBox's own
    // MSGBOX_3DITEM_WIDTH/HEIGHT (CommonMessageBox.h) exactly.
    constexpr float kItem3DSize = 40.0f;

    // Real screen-pixel size of the Mode::Text native input widget -- matches the field's own
    // .gcd-input-anchor box in generic_confirm_dialog.rcss (both themes). InputBoxConfig::size
    // needs real pixels, not a value read back from RmlUi (the anchor only supplies position).
    constexpr int kInputFieldWidth = 150;
    constexpr int kInputFieldHeight = 18;
}

void CGenericConfirmDialog::Create(CManager* pMng)
{
    Release();

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    pMng->AddUIObj(mu::ui::window::INTERFACE_GENERIC_CONFIRM_DIALOG, this);

    // Registered for this object's whole lifetime -- Render3D() below no-ops whenever the active
    // config has no `item3D`, and the camera's own IsVisible() gate skips calling it at all while
    // no dialog is active, so there's no cost to staying registered between dialogs.
    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Add3DRenderObj(this);
}

void CGenericConfirmDialog::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "generic_confirm_dialog",
        [this](Rml::DataModelConstructor& c, GenericDialogRmlModel& model)
        {
            auto line = c.RegisterStruct<LineEntry>();
            line.RegisterMember("text", &LineEntry::text);
            line.RegisterMember("bold", &LineEntry::bold);
            c.RegisterArray<std::vector<LineEntry>>();
            c.Bind("lines", &model.lines);

            c.Bind("primary_label", &model.primaryLabel);
            c.Bind("has_secondary", &model.hasSecondary);
            c.Bind("secondary_label", &model.secondaryLabel);
            c.Bind("show_cancel", &model.showCancel);
            c.Bind("cancel_label", &model.cancelLabel);

            c.Bind("has_title", &model.hasTitle);
            c.Bind("title", &model.title);
            c.Bind("severity_warning", &model.severityWarning);
            c.Bind("severity_error", &model.severityError);

            c.Bind("has_input", &model.hasInput);
            c.Bind("input_is_keypad", &model.inputIsKeypad);
            c.Bind("input_text", &model.inputText);
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
        });

    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
            "Data/Interface/RmlUi/generic_confirm_dialog.rml");

    // Background-context companion -- see the class comment for the mechanism. No RmlModelBinder
    // needed (100% static markup, see generic_confirm_dialog_bg.rml). Starts hidden
    // (CreateBackgroundDocument()); Show()/ShowNext()/Resolve()/Release()/ReloadRmlTheme() keep it
    // in lockstep with m_pRmlDoc from here on.
    m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/generic_confirm_dialog_bg.rml",
        RmlUiRuntime::Instance().GetDialogBackgroundContext());
}

void CGenericConfirmDialog::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    const bool wasVisible = m_pRmlDoc->IsVisible();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    if (m_pRmlBgDoc)
    {
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetDialogBackgroundContext())
            bgContext->UnloadDocument(m_pRmlBgDoc);
        m_pRmlBgDoc = nullptr;
    }

    BuildRmlUi(); // leaves m_pRmlBgDoc Hidden -- re-Show() below if this dialog was actually open
    if (wasVisible)
    {
        SyncRmlModel();
        if (m_pRmlDoc)
            m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
        if (m_pRmlBgDoc)
            m_pRmlBgDoc->Show();
    }
}

void CGenericConfirmDialog::Release()
{
    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->Remove3DRenderObj(this);

    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();
    m_bActive = false;
    m_Queue.clear();
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

    void ResetInputWidgetState()
    {
        if (g_iChatInputType == 1 && g_pSingleTextInputBox)
        {
            g_pSingleTextInputBox->SetText(nullptr);
            SaveIMEStatus();
            g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        }
    }
}

void CGenericConfirmDialog::Show(GenericDialogConfig cfg)
{
    // Protects a pending dialog from being silently overwritten -- e.g. a network-pushed guild
    // invite arriving while a quest-giveup confirm is already open (see class comment).
    if (m_bActive)
    {
        m_Queue.push_back(std::move(cfg));
        return;
    }

    m_Active = std::move(cfg);
    m_bActive = true;
    m_bPrimaryClicked = false;
    m_bSecondaryClicked = false;
    m_bCancelClicked = false;
    m_KeypadBuffer.clear();
    m_KeypadMapping.clear();
    m_bItem3DDebugLogged = false;

    if (m_Active.input)
    {
        if (m_Active.input->mode == GenericDialogConfig::InputField::Mode::NumericKeypad)
            m_KeypadMapping = ShuffledDigits();
        else
            m_KeypadBuffer = m_Active.input->initialText; // Mode::Text seeds the native widget in
                                                           // UpdateTextInputWidget(), not here
    }

    if (m_Active.progress)
    {
        m_dwProgressStartTime = timeGetTime();
        m_dwProgressEndTime = m_dwProgressStartTime + m_Active.progress->elapseMs;
    }

    if (m_pRmlDoc)
    {
        SyncRmlModel();
        // Modal: blocks the game world/other UI from stealing focus or clicks while this is open.
        m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
    if (m_pRmlBgDoc)
    {
        m_pRmlBgDoc->Show();
        // The bg document has no data model of its own -- tallPanel's "tall" class is mirrored
        // onto its #panel imperatively instead.
        if (Rml::Element* bgPanel = m_pRmlBgDoc->GetElementById("panel"))
            bgPanel->SetClass("tall", m_Active.tallPanel);
    }
}

void CGenericConfirmDialog::ShowNext()
{
    if (m_Queue.empty())
    {
        m_bActive = false;
        return;
    }

    m_Active = std::move(m_Queue.front());
    m_Queue.pop_front();
    m_bPrimaryClicked = false;
    m_bSecondaryClicked = false;
    m_bCancelClicked = false;
    m_KeypadBuffer.clear();
    m_KeypadMapping.clear();
    m_bItem3DDebugLogged = false;

    if (m_Active.input)
    {
        if (m_Active.input->mode == GenericDialogConfig::InputField::Mode::NumericKeypad)
            m_KeypadMapping = ShuffledDigits();
        else
            m_KeypadBuffer = m_Active.input->initialText;
    }

    if (m_Active.progress)
    {
        m_dwProgressStartTime = timeGetTime();
        m_dwProgressEndTime = m_dwProgressStartTime + m_Active.progress->elapseMs;
    }

    if (m_pRmlDoc)
    {
        SyncRmlModel();
        m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
    if (m_pRmlBgDoc)
    {
        m_pRmlBgDoc->Show();
        if (Rml::Element* bgPanel = m_pRmlBgDoc->GetElementById("panel"))
            bgPanel->SetClass("tall", m_Active.tallPanel);
    }
}

void CGenericConfirmDialog::Resolve(ClickResult which)
{
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

    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();

    // Release the shared widget/IME state so the next window to use g_pSingleTextInputBox
    // doesn't inherit it.
    if (cfg.input && cfg.input->mode == GenericDialogConfig::InputField::Mode::Text)
        ResetInputWidgetState();

    ShowNext();
}

bool CGenericConfirmDialog::Render()
{
    // RmlUi's #panel owns this dialog's entire visual, except the Mode::Text input widget --
    // drawn from RenderTextOnTop() instead: this Render() runs before RmlUi's own main-context
    // composite, so anything drawn here would get painted over by #panel's opaque background.
    // item3D still renders via Render3D() below (I3DRenderObj).
    SyncRmlModel();
    return true;
}

void CGenericConfirmDialog::UpdateTextInputWidget()
{
    if (!m_pRmlDoc || g_iChatInputType != 1 || !g_pSingleTextInputBox)
        return;

    Rml::Element* pAnchor = m_pRmlDoc->GetElementById("gcd_input_anchor");
    if (!pAnchor)
        return;

    // Read every frame, not once on Show() -- a same-frame read right after opening the document
    // can catch RmlUi's layout mid-resolve, self-correcting one frame later.
    const Rml::Vector2f correction = PanelTranslateCorrection();
    const Rml::Vector2f rawPos = pAnchor->GetAbsoluteOffset();
    const Rml::Vector2f pos = { rawPos.x + correction.x, rawPos.y + correction.y };

    const auto& field = *m_Active.input;
    InputBoxConfig config;
    config.pos = { static_cast<int>(pos.x), static_cast<int>(pos.y) };
    config.size = { kInputFieldWidth, kInputFieldHeight };
    config.textLimit = field.maxLength;
    config.password = field.masked;
    config.options = field.numericOnly ? UIOPTION_NUMBERONLY : UIOPTION_NULL;
    // InputBoxConfig's default text color is opaque BLACK -- invisible against this dialog's own
    // dark panel fill. Also gives the field a visible recessed background so it reads as a
    // clickable box even before the user types anything.
    config.textAlpha = 255;
    config.textR = 255;
    config.textG = 230;
    config.textB = 210;
    config.backAlpha = 255;
    config.backR = 0x10;
    config.backG = 0x0c;
    config.backB = 0x06;
    g_pSingleTextInputBox->Configure(config);
    g_pSingleTextInputBox->GiveFocus();
    g_pSingleTextInputBox->DoAction();
}

Rml::Vector2f CGenericConfirmDialog::PanelTranslateCorrection() const
{
    Rml::Element* pPanel = m_pRmlDoc ? m_pRmlDoc->GetElementById("panel") : nullptr;
    if (!pPanel)
        return { 0.f, 0.f };
    const Rml::Vector2f size = pPanel->GetBox().GetSize();
    return { -size.x * 0.5f, -size.y * 0.5f };
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

    auto& model = m_RmlBinder.GetModel();
    if (model.progressFraction != fraction)
    {
        model.progressFraction = fraction;
        m_RmlBinder.MarkDirty("progress_fraction");
    }
}

bool CGenericConfirmDialog::Update()
{
    if (!m_bActive)
        return true;

    if (m_Active.progress)
    {
        UpdateProgress();
        return true; // progress dialogs have no buttons to poll
    }

    if (m_Active.input && m_Active.input->mode == GenericDialogConfig::InputField::Mode::Text)
        UpdateTextInputWidget();

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

void CGenericConfirmDialog::RenderTextOnTop()
{
    if (!m_bActive || !m_Active.input || m_Active.input->mode != GenericDialogConfig::InputField::Mode::Text)
        return;
    if (g_iChatInputType != 1 || !g_pSingleTextInputBox)
        return;

    // Forces an identity-like transform to match UpdateTextInputWidget()'s real-pixel
    // GetAbsoluteOffset() position -- this runs from Winmain.cpp's post-RmlUi callback, entirely
    // outside CManager::Render()'s per-object loop, so whatever transform was last active would
    // otherwise leak in unpredictably.
    const auto transform = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Legacy, WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform identity(transform);
    g_pSingleTextInputBox->Render();
}

void CGenericConfirmDialog::Render3D()
{
    // Guarded here, not by staying unregistered -- see Create()'s own comment.
    //
    // KNOWN GAP: this draws behind the dialog's own opaque panel background, since RmlUi's main
    // context always composites LAST in the frame -- see the class's header comment.
    if (!m_bActive || !m_Active.item3D || !m_pRmlDoc)
        return;

    Rml::Element* pAnchor = m_pRmlDoc->GetElementById("gcd_item3d_anchor");
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

    // Mode::Text -- only the g_iChatInputType == 1 path (the actual runtime default and the one
    // every modern text field uses). The older g_iChatInputType == 0 raw-global-buffer path is a
    // deliberate gap -- not wired here.
    if (g_iChatInputType == 1 && g_pSingleTextInputBox)
    {
        wchar_t buffer[1024] = {};
        g_pSingleTextInputBox->GetText(buffer, 1024);
        return buffer;
    }
    return L"";
}

void CGenericConfirmDialog::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();

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

    const std::string primaryLabel = StringUtils::WideToNarrow(m_Active.primaryLabel.c_str());
    if (model.primaryLabel != primaryLabel)
    {
        model.primaryLabel = primaryLabel;
        m_RmlBinder.MarkDirty("primary_label");
    }

    const bool hasSecondary = m_Active.secondaryLabel.has_value();
    if (model.hasSecondary != hasSecondary)
    {
        model.hasSecondary = hasSecondary;
        m_RmlBinder.MarkDirty("has_secondary");
    }
    const std::string secondaryLabel = hasSecondary
        ? StringUtils::WideToNarrow(m_Active.secondaryLabel->c_str()) : std::string();
    if (model.secondaryLabel != secondaryLabel)
    {
        model.secondaryLabel = secondaryLabel;
        m_RmlBinder.MarkDirty("secondary_label");
    }

    if (model.showCancel != m_Active.showCancel)
    {
        model.showCancel = m_Active.showCancel;
        m_RmlBinder.MarkDirty("show_cancel");
    }
    const std::string cancelLabel = StringUtils::WideToNarrow(m_Active.cancelLabel.c_str());
    if (model.cancelLabel != cancelLabel)
    {
        model.cancelLabel = cancelLabel;
        m_RmlBinder.MarkDirty("cancel_label");
    }

    const bool hasTitle = !m_Active.title.empty();
    if (model.hasTitle != hasTitle)
    {
        model.hasTitle = hasTitle;
        m_RmlBinder.MarkDirty("has_title");
    }
    const std::string title = StringUtils::WideToNarrow(m_Active.title.c_str());
    if (model.title != title)
    {
        model.title = title;
        m_RmlBinder.MarkDirty("title");
    }

    const bool severityWarning = m_Active.severity == GenericDialogConfig::Severity::Warning;
    if (model.severityWarning != severityWarning)
    {
        model.severityWarning = severityWarning;
        m_RmlBinder.MarkDirty("severity_warning");
    }
    const bool severityError = m_Active.severity == GenericDialogConfig::Severity::Error;
    if (model.severityError != severityError)
    {
        model.severityError = severityError;
        m_RmlBinder.MarkDirty("severity_error");
    }

    const bool hasInput = m_Active.input.has_value();
    if (model.hasInput != hasInput)
    {
        model.hasInput = hasInput;
        m_RmlBinder.MarkDirty("has_input");
    }
    const bool inputIsKeypad = hasInput
        && m_Active.input->mode == GenericDialogConfig::InputField::Mode::NumericKeypad;
    if (model.inputIsKeypad != inputIsKeypad)
    {
        model.inputIsKeypad = inputIsKeypad;
        m_RmlBinder.MarkDirty("input_is_keypad");
    }
    // Always masked, matching the native keypad's own unconditional asterisk mask.
    const std::string inputText = inputIsKeypad ? std::string(m_KeypadBuffer.size(), '*') : std::string();
    if (model.inputText != inputText)
    {
        model.inputText = inputText;
        m_RmlBinder.MarkDirty("input_text");
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
                m_RmlBinder.MarkDirty(kFieldNames[i]);
            }
        }
    }

    const bool hasProgress = m_Active.progress.has_value();
    if (model.hasProgress != hasProgress)
    {
        model.hasProgress = hasProgress;
        m_RmlBinder.MarkDirty("has_progress");
    }

    const bool hasItem3D = m_Active.item3D.has_value();
    if (model.hasItem3D != hasItem3D)
    {
        model.hasItem3D = hasItem3D;
        m_RmlBinder.MarkDirty("has_item3d");
    }

    const bool hasPortrait2DOverlay = m_Active.portrait2D.has_value()
        && m_Active.portrait2D->layout == GenericDialogConfig::Portrait2D::Layout::Overlay;
    if (model.hasPortrait2DOverlay != hasPortrait2DOverlay)
    {
        model.hasPortrait2DOverlay = hasPortrait2DOverlay;
        m_RmlBinder.MarkDirty("has_portrait2d_overlay");
    }
    const bool hasPortrait2DBeside = m_Active.portrait2D.has_value()
        && m_Active.portrait2D->layout == GenericDialogConfig::Portrait2D::Layout::Beside;
    if (model.hasPortrait2DBeside != hasPortrait2DBeside)
    {
        model.hasPortrait2DBeside = hasPortrait2DBeside;
        m_RmlBinder.MarkDirty("has_portrait2d_beside");
    }
    const std::string portrait2DText = hasPortrait2DOverlay
        ? StringUtils::WideToNarrow(m_Active.portrait2D->text.c_str()) : std::string();
    if (model.portrait2DText != portrait2DText)
    {
        model.portrait2DText = portrait2DText;
        m_RmlBinder.MarkDirty("portrait2d_text");
    }

    const bool hasTallPanel = m_Active.tallPanel;
    if (model.hasTallPanel != hasTallPanel)
    {
        model.hasTallPanel = hasTallPanel;
        m_RmlBinder.MarkDirty("has_tall_panel");
    }
}

} // namespace mu::ui::window
