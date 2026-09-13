//*****************************************************************************
// File: GenericConfirmDialog.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core/Types.h>

#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CManager;

    // Config for one confirm dialog. Passed by value to CGenericConfirmDialog::Show() -- the whole
    // point of this primitive is that a new confirm dialog is a value of this struct, not a new
    // C++ class (contrast with CCommonMessageBox's ~140 TMsgBoxLayout<> subclasses, one per dialog).
    //
    // `title`/`input`/`progress`/`item3D` below are optional additions on top of the original
    // plain-text/OK-Cancel shape (see docs/rmlui-ui-system/dialog-migration-plan.md) -- each one
    // unset/empty reproduces the exact original shape, so every one of the ~65 call sites already
    // using this struct is unaffected. They're deliberately flat optional fields on one struct, not
    // a std::variant of five dialog "kinds" -- native itself never combines more than one of these
    // per dialog either (except title+item3D together, which this shape still allows), and a
    // variant would make the overwhelmingly common plain-text case syntactically heavier for no
    // real benefit.
    struct GenericDialogConfig
    {
        enum class ButtonSet { Ok, OkCancel };
        ButtonSet buttons = ButtonSet::Ok;
        std::wstring primaryLabel = L"OK";
        std::wstring secondaryLabel = L"Cancel"; // ignored unless buttons == OkCancel

        struct Line { std::wstring text; bool bold = false; };
        std::vector<Line> lines;

        // Optional bold title row above `lines`, fixed position/font, never wrapped -- matches
        // CMsgBoxIGSCommon::Initialize(pszTitle, pszText)'s own rendering (GameShop/MsgBoxIGS*.h,
        // the one native family with a genuine title/body split). Empty = no title row.
        std::wstring title;

        // Optional severity, purely a look-and-feel hook (border/accent tint) -- does not gate any
        // behavior. Not modeled on any single native class; added for future dialogs that want a
        // warning/error treatment without a bespoke primitive. Normal is visually identical to
        // today's plain panel.
        enum class Severity { Normal, Warning, Error };
        Severity severity = Severity::Normal;

        // Optional single text-entry field, rendered between `lines` and the button row. Unset =
        // no field (unchanged shape). Collapses two unrelated native classes
        // (CTextInputMsgBox/CKeyPadMsgBox, CustomMessageBox.h) into one config shape plus a mode
        // flag, the same move already made for OK-vs-OkCancel button sets. The typed value is read
        // back via CGenericConfirmDialog::GetInputText() -- called from `onPrimary`, exactly
        // mirroring how e.g. CStorageLockMsgBoxLayout::ProcessOk calls
        // pMsgBox->GetInputBoxText(strText) from inside its own handler today. onPrimary's
        // signature stays the same no-arg std::function<void()> for every dialog shape.
        struct InputField
        {
            // NumericKeypad reproduces CKeyPadMsgBox's shuffled on-screen digit pad (anti-keylogger
            // PIN entry, pure RmlUi buttons -- no native widget involved). Text reproduces
            // CTextInputMsgBox's real keyboard field, hosted via the shared g_pSingleTextInputBox
            // portable widget (same technique CharMakeWin.cpp/LoginWin.cpp already use for their
            // own name/password fields) positioned from an RmlUi anchor element, read back each
            // frame the same way CItemHotKey's item-icon anchors are.
            enum class Mode { Text, NumericKeypad };
            Mode mode = Mode::Text;
            int maxLength = 20;
            bool masked = false;       // password-style masking (Text mode only -- NumericKeypad
                                       // is unconditionally masked, matching native)
            bool numericOnly = false;  // Text mode only; NumericKeypad is always digits-only
            std::wstring initialText;
        };
        std::optional<InputField> input;

        // Optional progress bar / timed auto-close, reproducing CProgressMsgBox/
        // CCursedTempleProgressMsgBox (CustomMessageBox.h). Deliberately minimal: every real native
        // call site is fire-and-forget (grepped -- none register a completion callback or poll
        // afterward), so this is just a duration. When set, the button row is hidden entirely
        // (native's progress boxes have zero buttons); `onPrimary`, if set, fires once when the
        // timer elapses, standing in for CProgressMsgBox's internal ClosingProcess.
        struct Progress { DWORD elapseMs = 3000; };
        std::optional<Progress> progress;

        // Optional 3D item preview slot, reproducing C3DItemCommonMsgBox's Set3DItem (stores a
        // snapshot by value, same as native's own memcpy). Rendered via I3DRenderObj/Render3D(),
        // same as every other legacy 3D icon -- see the class comment for why that now actually
        // renders on top of the panel instead of behind it (the fg/bg document split).
        std::optional<ITEM> item3D;

        std::function<void()> onPrimary;   // OK / Enter / progress-timer-elapsed
        std::function<void()> onSecondary; // Cancel / Esc -- optional, safe to leave empty
    };

    // Reusable RmlUi confirm dialog -- one document/model, one instance, shown with different
    // GenericDialogConfig content per call. Replacement primitive for CCommonMessageBox/
    // CustomMessageBox's whole native TMsgBoxLayout<> family (see STATUS.md); proven on 3 real
    // dialogs first, then batch-ported onto ~65 more (dialog-migration-plan.md).
    //
    // Single active instance, not a stack: if Show() is called while one is already open, the new
    // config is queued and shown once the active one resolves -- matches CMsgWin's own single-
    // instance shape. CMessageBoxMng's vector suggests real stacking but in practice only ever
    // drives the single highest-priority box per frame anyway (confirmed by reading its Render()/
    // Update()), so this isn't a functional regression, just a more explicit version of the same
    // "one at a time" behavior.
    //
    // Also an I3DRenderObj: only actually registers 3D-render behavior while a dialog with
    // `item3D` set is active (see Render3D()'s own guard) -- every other shape never touches that
    // path at all.
    //
    // item3D renders on top of the panel via a foreground/background RmlUi document split, not a
    // post-RmlUi callback. Background: RmlUi's main context always composites LAST in the frame
    // (one fixed pre-submit callback, see RmlUiRuntime::Render()), strictly after C3DRenderMng's
    // own CManager-driven render pass that draws item3D -- so with only ONE document, the item
    // always rendered *behind* the panel's own opaque background, regardless of GetLayerDepth()/
    // z-order. Two attempts (2026-09-13/14) to fix this via a SetPostRmlUiCallback-based
    // RenderItem3DOnTop() (drawing the item manually after RmlUi composites, the same seam
    // CMsgWin/CCharMakeWin/CLoginWin use for native text overlays) both crashed on dialog dismiss;
    // the first crash's cause was found and fixed at the renderer level (MuRendererSDLGpu.cpp's
    // post-RmlUi replay pass re-staged vertex data only, leaving a skinned draw's bone-matrix
    // buffer unstaged/undersized while ReplayDrawCommand's boneDataReady guard stayed stale-true --
    // an out-of-bounds GPU read; fixed by reusing StageDeferredGpuData(), which stays fixed
    // regardless), but a second, unidentified crash persisted and SDL_GPU validation (which would
    // localize it) hit an unrelated pre-existing failure at Debug-build startup. Abandoned that
    // seam in favor of the fg/bg document split (2026-09-14) CMainFrameWindow/the inventory-family
    // windows (CMyInventory, CNPCShop, etc.) already use for their own live 3D icons: the dialog's
    // panel *background* art (frame/gradient or sprite composite, previously part of this same
    // document) moved to a second document (`m_pRmlBgDoc`).
    //
    // First cut loaded `m_pRmlBgDoc` into the SHARED RmlUiRuntime::GetBackgroundContext() (the same
    // one every ordinary window's own bg doc uses, e.g. NPCShop/inventory-family windows), driven by
    // the existing RenderBackgroundLayer() hook. That fixed the standalone case, but broke as soon
    // as this dialog opened over another bg-doc window with native foreground content (NPC Shop):
    // GetBackgroundContext() renders ONCE, globally, before the first visible object each frame --
    // strictly before EVERY window's own 2D Render() this frame, not just this dialog's -- so no
    // matter which document in that shared context painted "on top" of the other, NPC Shop's own
    // inventory-slot icons (drawn later, in its own Render()) would always paint over BOTH of them,
    // bleeding through the dialog's panel wherever they geometrically overlapped. Exactly the shop/
    // gamble dialogs item3D matters most for, since they're almost always shown over an open shop
    // window.
    //
    // Fixed for real by giving this dialog's own bg doc a DEDICATED third context
    // (RmlUiRuntime::GetDialogBackgroundContext()), rendered by a new, separately-guarded
    // RmlUiRuntime::RenderDialogBackgroundLayer() call that CManager::Render() (WindowManager.cpp)
    // fires at a DIFFERENT point than RenderBackgroundLayer(): right before the first visible
    // object whose GetLayerDepth() reaches the shared 3D camera's own z-order
    // (INFORMATION_CAMERA_Z_ORDER, Window3DRenderMng.h), which is when item3D itself draws (this
    // dialog registers its I3DRenderObj at the default z-order, same as every other item-icon
    // renderer, e.g. CInventoryCtrl). Since CManager::Render() sorts every registered object by
    // GetLayerDepth() (CNPCShop = 2.5f, the shared 3D camera = 10.9f, this dialog's own 2D Render()
    // = 60.0f) and calls each one's Render() in that order, "right before the object at/past 10.9f"
    // is guaranteed to be strictly after every ordinary window's own Render() this frame (all below
    // 10.9f) and strictly before item3D draws (inside that same camera's Render3D() pass, called
    // from ITS Render()). Deliberately NOT triggered from inside Render3D() itself, even though that
    // would also come "before item3D" within this one object's own call -- C3DCamera::Render()
    // pushes a legacy GL_PROJECTION/GL_MODELVIEW matrix stack and enables depth test/mask before
    // looping over Render3D() calls, and recording an RmlUi render pass from inside that block is
    // exactly the kind of mid-frame GPU-state interleaving that crashed the SetPostRmlUiCallback
    // attempts above -- CManager::Render()'s own loop, before any (*vi)->Render() call, is the same
    // safe, pre-matrix-stack position RenderBackgroundLayer() itself already uses.
    //
    // This document (the foreground one, still the sole owner of `data-model="generic_confirm_
    // dialog"`, still the one Render3D()/PanelTranslateCorrection() read from) is paint-less where
    // the background used to be, so it no longer covers the item once the dedicated context has
    // already painted it earlier the same frame. No changes were needed to Render3D()/
    // PanelTranslateCorrection()/I3DRenderObj registration themselves -- only to where the panel's
    // background art paints from, and when. See generic_confirm_dialog_bg.rml (both themes) for the
    // actual markup/styling split.
    class CGenericConfirmDialog : public CObject, public I3DRenderObj
    {
    public:
        void Create(CManager* pMng);
        void Release();

        // Shows now if idle, otherwise queues (see class comment).
        void Show(GenericDialogConfig cfg);

        // Only meaningful while an `input` field is configured and active; reads the live typed
        // value out of the RmlUi model (Mode::Text) or the on-screen keypad buffer
        // (Mode::NumericKeypad). Call from `onPrimary`, the same way native code called
        // pMsgBox->GetInputBoxText()/GetInputText() from its own OK handler.
        std::wstring GetInputText() const;

        // Call from `onPrimary`/`onSecondary` to veto this click's Resolve() -- the dialog stays
        // open exactly as it was (nothing hidden, m_bActive/m_Active untouched, the native
        // Mode::Text widget untouched) instead of closing/advancing the queue. Matches every native
        // CTextInputMsgBox-derived OkBtnDown/ReturnDown's own "return CALLBACK_CONTINUE" convention
        // for invalid input (empty field, a zero/unparsed amount) -- native leaves its own MsgBox
        // open for the user to retry instead of closing unconditionally like a plain-text confirm
        // dialog does. Typically called after GetInputText() fails validation, right before
        // returning from `onPrimary`.
        void KeepOpen() { m_bKeepOpenRequested = true; }

        // Called from Winmain.cpp's SetPostRmlUiCallback, after RmlUi's main context composites --
        // NOT from the normal CManager-driven Render() below (that always runs before RmlUi's own
        // composite, so anything drawn there gets painted over by #panel's opaque background).
        // Guarded internally on IsVisible()/the relevant `m_Active` field, same convention as
        // CMsgWin::RenderTextOnTop()/CCharMakeWin::RenderTextOnTop(). item3D has no equivalent
        // here -- see the class comment's "KNOWN GAP" note for why that one stayed on Render3D().
        void RenderTextOnTop(); // Mode::Text's g_pSingleTextInputBox widget

        bool Render() override;
        bool Update() override;
        // Unconditionally claims input while shown -- a true modal, matching CMessageBoxMng's own
        // "return false while any box is open" contract (CMsgWin's RmlUi-tier equivalent doesn't
        // block key dispatch the same way; this class deliberately does, since it stands in for a
        // system that did).
        bool UpdateMouseEvent() override { return !IsVisible(); }
        bool UpdateKeyEvent() override;
        // CObject::IsVisible() reflects Show(bool)/m_bRender, which this class never touches --
        // visibility here is purely "is a dialog currently active," so this is overridden instead.
        bool IsVisible() const override { return m_bActive; }
        // Above CMsgWin's 50.0f -- a confirm dialog should sit on top of an ordinary message window.
        float GetLayerDepth() override { return 60.0f; }
        // Matches CMessageBoxMng's own GetKeyEventOrder() (10.f) -- the system this replaces.
        float GetKeyEventOrder() override { return 10.0f; }
        void ReloadRmlTheme() override;

        // I3DRenderObj -- see the class comment (including its KNOWN GAP note). Window3DRenderMng.cpp's
        // shared render loop (`TransformForOwner()`) dynamic_casts each registered object to CObject
        // first and uses *its own* GetLayoutMode() when that succeeds -- true here, so this object's
        // Dialog layout transform is pushed automatically before Render3D() runs, the same way
        // CManager::Render() already does for the ordinary 2D Render() call.
        void Render3D() override;

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void ShowNext();            // pops m_Queue (if non-empty) and opens the document
        void Resolve(bool primary); // hides the document, invokes the chosen callback, then ShowNext()

        void UpdateProgress();      // advances the progress-bar fraction, auto-resolves on elapse
        void UpdateTextInputWidget();  // per-frame Configure()/GiveFocus()/DoAction() for Mode::Text

        // #panel is centered via `.center-both` (`left:50%; top:50%; transform:translate(-50%,-50%)`,
        // base.rcss) -- GetAbsoluteOffset() walks the ancestor chain summing offsets but does NOT
        // apply CSS `transform` at any level (confirmed 2026-09-14 against this dialog's own logged
        // numbers: #panel's own GetAbsoluteOffset() came back at the exact window center, i.e. its
        // untranslated `(50%,50%)` position, never mind the `translate(-50%,-50%)` that visually
        // shifts it back by half its own size). Every element inside #panel is off by that same
        // missing delta -- add this correction to any of #panel's descendants' own GetAbsoluteOffset()
        // before treating it as a real screen position. Returns {0,0} if #panel can't be found.
        Rml::Vector2f PanelTranslateCorrection() const;

        struct LineEntry { Rml::String text; bool bold = false; };
        struct GenericDialogRmlModel
        {
            std::vector<LineEntry> lines;
            bool showCancel = false;
            Rml::String primaryLabel;
            Rml::String secondaryLabel;

            bool hasTitle = false;
            Rml::String title;
            bool severityWarning = false;
            bool severityError = false;

            bool hasInput = false;
            bool inputIsKeypad = false;
            Rml::String inputText; // display text (already masked, if applicable) for NumericKeypad;
                                   // unused for Mode::Text, which renders via the native widget
            // Individually-bound, not a data-for loop over an array -- this codebase has no
            // proven precedent for a data-for that also needs each row's own index inside its
            // click handler (`gcd_keypad_click(i)`), so 10 fixed slots are bound the same way
            // MainFrameWindow.cpp's #item_slot_0.._3 already are.
            int keypadDigit0 = 0, keypadDigit1 = 0, keypadDigit2 = 0, keypadDigit3 = 0, keypadDigit4 = 0;
            int keypadDigit5 = 0, keypadDigit6 = 0, keypadDigit7 = 0, keypadDigit8 = 0, keypadDigit9 = 0;

            bool hasProgress = false;
            float progressFraction = 0.f;
        };
        RmlModelBinder<GenericDialogRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // Background-context companion to m_pRmlDoc -- see the class comment for the mechanism.
        // No RmlModelBinder of its own: it's 100% static markup (this dialog centers via plain
        // CSS, never a per-frame C++-computed position), unlike CMainFrameWindow's/CNPCShop's own
        // bg docs. Lives in its OWN dedicated context (RmlUiRuntime::GetDialogBackgroundContext()),
        // not the shared one every ordinary window's own bg doc uses -- see the class comment for
        // why. Shown/Hidden in lockstep with m_pRmlDoc at every transition point (Show(), ShowNext(),
        // Resolve(), Release(), ReloadRmlTheme()) -- never left to track m_pRmlDoc's state
        // implicitly, since RenderDialogBackgroundLayer() paints whatever's currently Show()n in
        // this context every frame, regardless of which window is first in z-order.
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

        // Set by RmlUi click bindings; polled and cleared in Update() -- never act synchronously
        // inside the RmlUi callback itself, same convention as CMsgWin/RememberPasswordPrompt.
        bool m_bPrimaryClicked = false;
        bool m_bSecondaryClicked = false;

        // See KeepOpen()'s own comment. Reset at the top of every Resolve() call, read at the
        // bottom to decide whether to actually close.
        bool m_bKeepOpenRequested = false;

        // NumericKeypad's own click-accumulated digit buffer (never the real keyboard focus) --
        // shuffled mapping reproduces CKeyPadMsgBox's own anti-shoulder-surfing behavior.
        std::wstring m_KeypadBuffer;
        std::vector<int> m_KeypadMapping; // 10 entries, shuffled per Show()

        // Progress-bar timing, mirroring CProgressMsgBox's own m_dwStartTime/m_dwEndTime.
        DWORD m_dwProgressStartTime = 0;
        DWORD m_dwProgressEndTime = 0;

        std::deque<GenericDialogConfig> m_Queue;
        GenericDialogConfig m_Active;
        bool m_bActive = false;

        // Diagnostic for the item3D positioning/layering work (2026-09-14) -- logs
        // RenderItem3DOnTop()'s anchor position/transform/final coordinates exactly once per
        // Show(), so a mismatch can be read out of MuError.log instead of guessed at blind. Cheap
        // enough (one log line per dialog open) to leave in permanently.
        bool m_bItem3DDebugLogged = false;
    };

    // Convenience global for the scattered native call sites this primitive is meant to replace
    // (guild/quest UI code, network packet handlers) -- same convention as g_pUIPopup. Set once,
    // in CSystem::Create() (WindowSystem.cpp), alongside every other app-lifetime CObject-tier
    // window; never null after that point during normal play.
    extern CGenericConfirmDialog* g_pGenericConfirmDialog;
}
