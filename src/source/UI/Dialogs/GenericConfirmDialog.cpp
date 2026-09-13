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

// Same g_iChatInputType convention every other native window/dialog in this codebase already
// uses (ad-hoc extern, not centralized in a header) -- see UIPopup.cpp/CharMakeWin.cpp for the
// identical pattern. Default is 1 (Winmain.cpp) -- the modern g_pSingleTextInputBox portable
// widget path, the only one this class implements (see GetInputText()'s own comment).
extern int g_iChatInputType;

namespace mu::ui::window
{

CGenericConfirmDialog* g_pGenericConfirmDialog = nullptr;

namespace
{
    // Reference-space size of the 3D-item preview slot -- matches C3DItemCommonMsgBox's own
    // MSGBOX_3DITEM_WIDTH/HEIGHT (CommonMessageBox.h) exactly, not a new invented size.
    constexpr float kItem3DSize = 40.0f;

    // Real screen-pixel size of the Mode::Text native input widget -- matches the field's own
    // .gcd-input-anchor box in generic_confirm_dialog.rcss (both themes); kept as one named
    // constant here since InputBoxConfig::size needs real pixels, not a value read back from
    // RmlUi (the anchor only supplies position, same convention as every other anchor in this
    // codebase -- CItemHotKey/CharMakeWin never read a size back either).
    constexpr int kInputFieldWidth = 150;
    constexpr int kInputFieldHeight = 18;
}

void CGenericConfirmDialog::Create(CManager* pMng)
{
    Release();

    if (RmlUiRuntime::Instance().IsCreated())
        BuildRmlUi();

    pMng->AddUIObj(mu::ui::window::INTERFACE_GENERIC_CONFIRM_DIALOG, this);

    // Registered for this object's whole lifetime, same convention C3DItemCommonMsgBox's own
    // Create() uses -- Render3D() below no-ops whenever the active config has no `item3D`, and
    // the camera's own IsVisible() gate (Window3DRenderMng.cpp) already skips calling it at all
    // while no dialog is active, so there's no cost to staying registered between dialogs.
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

            c.Bind("show_cancel", &model.showCancel);
            c.Bind("primary_label", &model.primaryLabel);
            c.Bind("secondary_label", &model.secondaryLabel);

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

            c.BindEventCallback("gcd_primary_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_bPrimaryClicked = true; });
            c.BindEventCallback("gcd_secondary_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_bSecondaryClicked = true; });

            // Position argument is the FIXED grid slot (0-9), not the digit typed -- that's
            // looked up via m_KeypadMapping, reproducing CKeyPadMsgBox's own shuffled-position
            // anti-shoulder-surfing behavior (KeyPadBtnDown, CustomMessageBox.cpp).
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
    // needed (100% static markup, see generic_confirm_dialog_bg.rml). Loaded into this dialog's
    // OWN dedicated context (GetDialogBackgroundContext()), not the shared GetBackgroundContext()
    // every ordinary window's own bg doc uses -- see the class comment for why.
    // CreateBackgroundDocument() auto-Show()s it; Hide() immediately since this dialog starts
    // inactive -- Show()/ShowNext()/Resolve()/Release()/ReloadRmlTheme() keep it in lockstep with
    // m_pRmlDoc from here on.
    m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/generic_confirm_dialog_bg.rml",
        RmlUiRuntime::Instance().GetDialogBackgroundContext());
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();
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
    // CKeyPadMsgBox's own shuffle (CustomMessageBox.cpp) -- 20 random adjacent swaps over 0..9.
    // Reproduced with std::shuffle instead since this is new code, not a ported native method.
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
        // Modal: blocks the game world/other UI from stealing focus or clicks while this is open --
        // matches CMessageBoxMng's own input-blocking behavior for the system this replaces.
        m_pRmlDoc->Show(Rml::ModalFlag::Modal, Rml::FocusFlag::Document);
    }
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Show();
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
        m_pRmlBgDoc->Show();
}

void CGenericConfirmDialog::Resolve(bool primary)
{
    m_bKeepOpenRequested = false;

    // Move out before invoking -- the callback may itself call Show() (e.g. chaining a follow-up
    // confirm), which must not stomp m_Active while its own onPrimary/onSecondary still needs it.
    // Deliberately BEFORE hiding/ShowNext() now (see KeepOpen()'s own comment): a callback that
    // vetoes via KeepOpen() needs nothing touched yet -- not the RmlUi documents, not the native
    // Mode::Text widget, not the queue.
    GenericDialogConfig cfg = std::move(m_Active);
    if (primary)
    {
        if (cfg.onPrimary) cfg.onPrimary();
    }
    else
    {
        if (cfg.onSecondary) cfg.onSecondary();
    }

    if (m_bKeepOpenRequested)
    {
        // Validation failed -- put everything back exactly as it was. m_pRmlDoc/m_pRmlBgDoc were
        // never hidden and m_bActive was never touched, so as far as anything else can tell this
        // Resolve() call never happened.
        m_Active = std::move(cfg);
        return;
    }

    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();

    // Same cleanup CUIPopup::Close() does for its own POPUP_INPUT case -- release the shared
    // widget/IME state so the next window to use g_pSingleTextInputBox doesn't inherit it.
    if (cfg.input && cfg.input->mode == GenericDialogConfig::InputField::Mode::Text)
        ResetInputWidgetState();

    ShowNext();
}

bool CGenericConfirmDialog::Render()
{
    // RmlUi's #panel owns this dialog's entire visual, except the Mode::Text input widget --
    // drawn from RenderTextOnTop() instead (Winmain.cpp's post-RmlUi seam), not here: this
    // Render() runs through CManager's normal per-object loop, which always executes *before*
    // RmlUi's own main-context composite, so anything drawn here would just get painted over by
    // #panel's own (later-composited, opaque) background. item3D still renders via Render3D()
    // below (I3DRenderObj) -- see that method's KNOWN GAP note in the header.
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

    // Read every frame, not once on Show() -- same lesson CharMakeWin.cpp's own #input_text_anchor
    // read learned live: a same-frame read right after opening the document can catch RmlUi's
    // layout mid-resolve, self-correcting one frame later. This dialog's layout is otherwise
    // static, so at worst the widget is misplaced for one imperceptible frame.
    //
    // + PanelTranslateCorrection(): GetAbsoluteOffset() doesn't see #panel's own
    // `transform: translate(-50%,-50%)` (`.center-both`, base.rcss) -- see that method's own
    // comment for how this was found and confirmed.
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
    // dark panel fill. Same gotcha CharMakeWin.cpp's own #input_text_anchor field already hit and
    // documented; matches LoginWin.cpp's/CharMakeWin.cpp's own light-cream convention instead of
    // rediscovering a third color. Also gives the field a visible recessed background (this
    // dialog's anchor has no CSS frame of its own the way CharMakeWin's native input row does --
    // same dark fill `.gcd-progress-track` already uses elsewhere in this same panel) so the field
    // reads as a clickable box even before the user types anything.
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
        // Mirrors CProgressMsgBox::ClosingProcess -- elapse-and-close, onPrimary (if set) stands
        // in for its hardcoded internal side effect. No onSecondary path exists for progress
        // dialogs -- native never offers one either (see dialog-migration-plan.md's own research
        // note: zero button-bearing progress-bar variants exist anywhere in the native family).
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(true);
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
        return true; // progress dialogs have no buttons to poll -- see UpdateProgress()
    }

    if (m_Active.input && m_Active.input->mode == GenericDialogConfig::InputField::Mode::Text)
        UpdateTextInputWidget();

    if (m_bPrimaryClicked)
    {
        m_bPrimaryClicked = false;
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(true);
    }
    else if (m_bSecondaryClicked)
    {
        m_bSecondaryClicked = false;
        ::PlayBuffer(SOUND_CLICK01);
        Resolve(false);
    }

    return true;
}

bool CGenericConfirmDialog::UpdateKeyEvent()
{
    if (!m_bActive)
        return true;

    // Progress dialogs are non-interactive countdowns natively (no button, no Esc-to-dismiss) --
    // preserve that rather than inventing an early-dismiss gesture no native call site expects.
    if (!m_Active.progress)
    {
        if (mu::ui::window::IsPress(VK_RETURN))
        {
            ::PlayBuffer(SOUND_CLICK01);
            Resolve(true);
        }
        else if (mu::ui::window::IsPress(VK_ESCAPE))
        {
            ::PlayBuffer(SOUND_CLICK01);
            Resolve(m_Active.buttons == GenericDialogConfig::ButtonSet::Ok);
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
    // GetAbsoluteOffset() position -- same reasoning as CMsgWin::RenderTextOnTop()'s identical
    // guard: nothing else pushes an active transform here (this runs from Winmain.cpp's
    // post-RmlUi callback, entirely outside CManager::Render()'s per-object loop), so whatever
    // was last active would otherwise leak in unpredictably.
    const auto transform = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Legacy, WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform identity(transform);
    g_pSingleTextInputBox->Render();
}

void CGenericConfirmDialog::Render3D()
{
    // Guarded here, not by staying unregistered -- see Create()'s own comment. Every non-item3D
    // dialog shape (the overwhelming majority) hits this early-out every frame it's open.
    //
    // KNOWN GAP: this draws behind the dialog's own opaque panel background, since RmlUi's main
    // context always composites LAST in the frame, strictly after C3DRenderMng's own CManager-
    // driven pass -- see the class's header comment for the full writeup and what's been tried.
    if (!m_bActive || !m_Active.item3D || !m_pRmlDoc)
        return;

    Rml::Element* pAnchor = m_pRmlDoc->GetElementById("gcd_item3d_anchor");
    if (!pAnchor)
        return;

    // The active transform here is whatever Window3DRenderMng.cpp's TransformForOwner() just
    // pushed for us (this object's own Dialog-layout transform, since we're a CObject -- see this
    // method's declaration comment in the header). RenderItem3D() expects REFERENCE-space
    // coordinates and re-applies that same transform internally to reach real screen pixels, so
    // the anchor's real screen position must be converted back to reference space first via
    // LogicalX/LogicalY -- the documented inverse of PositionX/PositionY. This is the same overall
    // split CItemHotKey/C3DItemCommonMsgBox use (RmlUi/native chrome owns position, a native 3D
    // pass draws the icon), just without their extra per-window delta-correction math: that exists
    // in CItemHotKey specifically to reconcile OLD hardcoded reference-space slot coordinates with
    // newer RmlUi layout, a problem this brand-new anchor doesn't have.
    //
    // + PanelTranslateCorrection(): GetAbsoluteOffset() alone reports the anchor's position as if
    // #panel were still sitting at its untranslated `left:50%; top:50%` spot -- see that method's
    // own comment for how this was found and confirmed (2026-09-14, logged real numbers showed
    // #panel's own GetAbsoluteOffset() sitting exactly at window-center, off by exactly half its
    // own size from where it's actually painted).
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

    // Mode::Text -- mirrors CUIPopup::GetInputText()'s own g_iChatInputType branch, but only the
    // g_iChatInputType == 1 path (the actual runtime default, Winmain.cpp:101/1985, and the only
    // path CharMakeWin.cpp/LoginWin.cpp's own modern text fields use). The older
    // g_iChatInputType == 0 raw-global-buffer path (InputText[0]/ClearInput(), still used by a
    // handful of legacy screens) is a deliberate, documented gap -- not wired here. If a future
    // port genuinely needs it, add it then rather than speculatively now.
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

    const bool showCancel = m_Active.buttons == GenericDialogConfig::ButtonSet::OkCancel;
    if (model.showCancel != showCancel)
    {
        model.showCancel = showCancel;
        m_RmlBinder.MarkDirty("show_cancel");
    }

    const std::string primaryLabel = StringUtils::WideToNarrow(m_Active.primaryLabel.c_str());
    if (model.primaryLabel != primaryLabel)
    {
        model.primaryLabel = primaryLabel;
        m_RmlBinder.MarkDirty("primary_label");
    }

    const std::string secondaryLabel = StringUtils::WideToNarrow(m_Active.secondaryLabel.c_str());
    if (model.secondaryLabel != secondaryLabel)
    {
        model.secondaryLabel = secondaryLabel;
        m_RmlBinder.MarkDirty("secondary_label");
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
    // Always masked, matching CKeyPadMsgBox::RenderKeyPadInput's own unconditional asterisk mask.
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
}

} // namespace mu::ui::window
