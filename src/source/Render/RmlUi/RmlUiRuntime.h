#pragma once

#include "stdafx.h"
#include "Core/Input/UiInputRouter.h"
#include <RmlUi/Core/Context.h>
#include <memory>

class RmlUiRenderInterface;
class RmlUiSystemInterface;
class TextInputMethodEditor_SDL; // ThirdParty/RmlUi/Backends/RmlUi_Platform_SDL.h
union SDL_Event;
struct SDL_Window;

// Owns the Rml::Context lifecycle and the one per-frame Update()/Render() entry point. Create()
// registers RenderFrame() as the SDL_GPU renderer's pre-submit callback (see
// IMuRenderer::SetPreSubmitCallback, MuRenderer.h) -- a true single choke point every frame
// funnels through regardless of active scene.
//
// Implements Core::Input::IUiInputConsumer so input-routing call sites (Winmain.cpp's event pump,
// gameplay's mouse-gating checks) depend on that interface instead of this concrete type -- see
// UiInputRouter.h. Create()/Destroy() register/clear this instance as the router's active
// consumer.
class RmlUiRuntime : public Core::Input::IUiInputConsumer
{
public:
    static RmlUiRuntime& Instance();

    void Create(int windowWidth, int windowHeight);
    void Destroy();
    bool IsCreated() const { return m_Context != nullptr; }

    void OnResize(int windowWidth, int windowHeight);

    void Update();
    void Render();

    // Update() then Render(), in that order -- what Create() registers as the renderer's
    // pre-submit callback. Public (not private+friend) only because that registration is a
    // free function (a lambda) rather than a member, and doesn't need any further access.
    void RenderFrame();

    // The RmlUi-behind-3D-icons seam: a second, background-only Rml::Context (m_BackgroundContext)
    // that CManager::Render() (WindowManager.cpp) drives explicitly, right before the first
    // visible window/C3DCamera in its z-sorted loop each frame, instead of waiting for
    // RenderFrame()'s single fixed pre-submit slot. Flushes whatever legacy content has been
    // recorded so far (mu::GetRenderer().
    // FlushRenderCommands(), MuRenderer.h) so it's actually on screen, then renders this context
    // into the resulting gap -- content recorded after this call returns lands on top of it, in
    // front of everything recorded before this call. No-op if nothing was ever loaded into the
    // background context (every theme but the ones that opt in, e.g. `legacy`). Never receives
    // input (see m_BackgroundContext's own comment) -- Update() still needs calling per frame for
    // data-model/animation purposes, so this does that too, not just Render().
    void RenderBackgroundLayer();

    // CManager::Render() (WindowManager.cpp) calls this once per frame, right before the first
    // visible object whose GetLayerDepth() reaches the shared 3D camera's own z-order
    // (INFORMATION_CAMERA_Z_ORDER, Window3DRenderMng.h) -- i.e. strictly after every ordinary
    // window's own Render() this frame (all of which sit below that z-order) and strictly before
    // that camera's own Render3D() pass (where item3D actually draws, via
    // CGenericConfirmDialog::Render3D()). This precise, still-top-level position (WindowManager.
    // cpp's own per-object loop, before any (*vi)->Render() call) matters: C3DCamera::Render()
    // pushes a legacy GL_PROJECTION/GL_MODELVIEW matrix stack and enables depth test/mask before
    // calling Render3D() in a loop -- recording an RmlUi render pass from INSIDE that block (e.g.
    // from within Render3D() itself) is exactly the kind of mid-frame GPU-state interleaving that
    // twice crashed the SetPostRmlUiCallback-based attempt to solve this same underlying problem
    // (see GenericConfirmDialog.h's own class comment) -- this call site avoids that entirely by
    // staying at the same safe, pre-matrix-stack point RenderBackgroundLayer() itself already
    // uses. No-op past the first call each frame (m_dialogBackgroundLayerRenderedThisFrame), and a
    // no-op entirely if nothing is currently Show()n in GetDialogBackgroundContext() (today, only
    // ever CGenericConfirmDialog's own bg doc).
    void RenderDialogBackgroundLayer();

    // Every RmlUi document meant to render via RenderBackgroundLayer() loads into this context
    // instead of GetContext()'s "main" one (UI::RmlBridge::LoadThemedDocument already takes a
    // Rml::Context* parameter, so no change needed there) -- e.g.
    // LoadThemedDocument(RmlUiRuntime::Instance().GetBackgroundContext(), "...").
    Rml::Context* GetBackgroundContext() const { return m_BackgroundContext; }

    // A THIRD, separate background-only context -- exclusively for CGenericConfirmDialog's own
    // panel chrome, not shared with GetBackgroundContext() (used by every ordinary window's own bg
    // doc, e.g. NPCShop/inventory-family windows). GetBackgroundContext()'s single "renders once,
    // globally, before the very first visible window this frame" timing works fine as long as
    // nothing needs to render BETWEEN "some other window's own foreground content" and "this
    // window's own 3D icon content" -- but a modal dialog does: its own panel needs to land ABOVE
    // any other window it happens to visually overlap (e.g. opened over NPC Shop, whose own
    // inventory-slot icons would otherwise bleed through the dialog's now-paint-less-in-the-shared-
    // context panel), while still landing BELOW its own item3D preview (drawn via
    // I3DRenderObj/Render3D(), the same shared 3D camera every other item-icon renderer uses).
    // RenderDialogBackgroundLayer()'s own comment has the full call-site reasoning.
    Rml::Context* GetDialogBackgroundContext() const { return m_DialogBackgroundContext; }

    // Forwards one SDL event to RmlUi. Motion and button down/up are handled directly (see the
    // .cpp) to avoid two real bugs in RmlUi's official RmlSDL::InputEventHandler (vendored at
    // ThirdParty/RmlUi/Backends/RmlUi_Platform_SDL.cpp) that don't fit this engine's own
    // conventions: it calls SDL_CaptureMouse() on every button press/release, and it scales
    // motion coordinates by SDL_GetWindowPixelDensity(). Every other event type (wheel/key/text)
    // still goes through it, reusing RmlUi's own tested SDL-keycode/modifier mapping.
    // @return false if RmlUi consumed the event (an element under the cursor / holding focus
    // claimed it) -- the caller should then skip its own legacy handling for this event. true
    // means RmlUi did not consume it (matches Rml::Context::Process*'s own "still propagating"
    // convention) and legacy handling should proceed as before.
    //
    // Ordering caveat: this always offers the event to RmlUi first, regardless of whether a
    // legacy CObject/CWin window is actually on top of the RmlUi content at that screen position
    // -- correct as long as migrated-window content and still-legacy windows occupy disjoint
    // z-order bands, wrong if a legacy window ever needs to sit visually on top of migrated RmlUi
    // content. Revisit if that situation arises.
    bool ProcessSdlEvent(SDL_Event& event, SDL_Window* window) override;

    // Rml::Context::IsMouseInteracting() -- true while the mouse is hovering/pressed over any
    // RmlUi element. Intended as a third click-to-move gate alongside MouseOnWindow (CWin tier)
    // and g_pNewUISystem->CheckMouseUse() (mu::ui::window tier) at Input/Selection.cpp's
    // world-pick check.
    bool IsMouseOverUI() const override;
    void CancelSyntheticMousePress(unsigned char button, SDL_Window* window) override;

    Rml::Context* GetContext() const { return m_Context; }

    // True from the moment any RmlUi <input> across any of the three contexts is focused until
    // it's blurred (RmlUiSystemInterface::ActivateKeyboard/DeactivateKeyboard, which
    // WidgetTextInput::SetKeyboardActive() already calls on every real Focus/Blur -- no polling,
    // no heuristics). This is the single ownership signal Winmain.cpp's SDL_EVENT_TEXT_EDITING
    // routing and its per-frame native SDL_StartTextInput/StopTextInput/SetTextInputArea block
    // both key off: RmlUi wins whenever this is true, CUITextInputBox::GetFocusedPortable() is the
    // fallback otherwise. Until an RmlUi <input> exists on a migrated screen, this is always
    // false and every existing native-only screen's behavior is unchanged.
    bool IsTextInputActive() const;

    // Forwards one SDL_EVENT_TEXT_EDITING event to the installed Rml::TextInputHandler (the
    // vendored TextInputMethodEditor_SDL -- see m_TextInputMethodEditor's own comment). Caller
    // (Winmain.cpp) is expected to only call this when IsTextInputActive() is true; harmless
    // no-op otherwise since TextInputMethodEditor_SDL::HandleEdit() itself no-ops with no active
    // TextInputContext.
    void ProcessTextEditing(const SDL_Event& event);

private:
    RmlUiRuntime() = default;
    ~RmlUiRuntime();

    RmlUiRuntime(const RmlUiRuntime&) = delete;
    RmlUiRuntime& operator=(const RmlUiRuntime&) = delete;

    std::unique_ptr<RmlUiRenderInterface> m_RenderInterface;
    std::unique_ptr<RmlUiSystemInterface> m_SystemInterface;

    // Installed globally via Rml::SetTextInputHandler() in Create() -- global, not per-Context,
    // per TextInputHandler.h's own doc comment, so one instance covers all three contexts below.
    // Same class the vendored SDL_GPU/GL/DX/VK sample backends all install (grep
    // ThirdParty/RmlUi/Backends for "text_input_method_editor"); this engine's own RmlUiRuntime
    // never had an equivalent until now. Outlives every Rml::Context (reset only after
    // Rml::Shutdown() in Destroy(), same contract as m_RenderInterface/m_SystemInterface above) --
    // a live WidgetTextInputContext can call back into it during document/element teardown.
    std::unique_ptr<TextInputMethodEditor_SDL> m_TextInputMethodEditor;

    Rml::Context* m_Context = nullptr; // owned by Rml::Core, released via Rml::Shutdown()

    // Background-only companion to m_Context -- see RenderBackgroundLayer()'s own comment. Never
    // registered as (or participating in) the active IUiInputConsumer: every document loaded into
    // it is expected to be entirely `pointer-events: none` (same convention as char_sel_main.rml's
    // #panel), so it needs no ProcessSdlEvent/IsMouseOverUI involvement at all -- Update()+Render()
    // only, driven directly by RenderBackgroundLayer(), never through the single-slot UiInputRouter
    // or the single-slot SetPreSubmitCallback m_Context itself uses. Also owned by Rml::Core,
    // released by the same Rml::Shutdown() call in Destroy().
    Rml::Context* m_BackgroundContext = nullptr;

    // RenderBackgroundLayer() replays the ENTIRE shared background context -- every document
    // currently Show()n in it, not just one particular window's. CManager::Render()
    // (WindowManager.cpp) is the sole caller, once per visible window/C3DCamera in its z-sorted
    // loop -- this guard is what makes only the first (lowest-GetLayerDepth) visible one actually
    // render; every later one this same frame is a cheap no-op. Reset in RenderFrame() (the one
    // guaranteed-once-per-frame choke point), not BeginFrame(), since RenderFrame() fires late
    // (SetPreSubmitCallback) -- after every RenderBackgroundLayer() call this frame already
    // happened, so resetting there arms the guard correctly for next frame.
    bool m_backgroundLayerRenderedThisFrame = false;

    // See GetDialogBackgroundContext()/RenderDialogBackgroundLayer()'s own comments. Same
    // ownership/lifetime contract as m_BackgroundContext (created alongside it in Create(),
    // resized alongside it in OnResize(), released by the same Rml::Shutdown() call in Destroy()).
    Rml::Context* m_DialogBackgroundContext = nullptr;

    // Same once-per-frame-guard idea as m_backgroundLayerRenderedThisFrame, reset alongside it in
    // RenderFrame() -- CManager::Render() calls RenderDialogBackgroundLayer() once per visible
    // object at/past the 3D camera's own z-order, every frame; this is what makes only the first
    // such call actually render.
    bool m_dialogBackgroundLayerRenderedThisFrame = false;
};
