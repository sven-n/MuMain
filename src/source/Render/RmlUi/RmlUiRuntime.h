#pragma once

#include "stdafx.h"
#include "Core/Input/UiInputRouter.h"
#include <RmlUi/Core/Context.h>
#include <memory>

class RmlUiRenderInterface;
class RmlUiSystemInterface;
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

    // The RmlUi-behind-3D-icons seam (docs/rmlui-ui-system/STATUS.md's "RmlUi renders last"
    // finding): a second, background-only Rml::Context (m_BackgroundContext) that a caller drives
    // explicitly, mid-frame, instead of waiting for RenderFrame()'s single fixed pre-submit slot.
    // Flushes whatever legacy content has been recorded so far (mu::GetRenderer().
    // FlushRenderCommands(), MuRenderer.h) so it's actually on screen, then renders this context
    // into the resulting gap -- content the caller records right after this call returns lands on
    // top of it, in front of everything recorded before this call. No-op if nothing was ever
    // loaded into the background context (every theme but the ones that opt in, e.g. `legacy`).
    // Never receives input (see m_BackgroundContext's own comment) -- Update() still needs calling
    // per frame for data-model/animation purposes, so this does that too, not just Render().
    void RenderBackgroundLayer();

    // Every RmlUi document meant to render via RenderBackgroundLayer() loads into this context
    // instead of GetContext()'s "main" one (UI::RmlBridge::LoadThemedDocument already takes a
    // Rml::Context* parameter, so no change needed there) -- e.g.
    // LoadThemedDocument(RmlUiRuntime::Instance().GetBackgroundContext(), "...").
    Rml::Context* GetBackgroundContext() const { return m_BackgroundContext; }

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

    Rml::Context* GetContext() const { return m_Context; }

private:
    RmlUiRuntime() = default;
    ~RmlUiRuntime();

    RmlUiRuntime(const RmlUiRuntime&) = delete;
    RmlUiRuntime& operator=(const RmlUiRuntime&) = delete;

    std::unique_ptr<RmlUiRenderInterface> m_RenderInterface;
    std::unique_ptr<RmlUiSystemInterface> m_SystemInterface;
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
    // currently Show()n in it, not just whichever window's own document prompted the call (there's
    // no per-caller scoping, see that method's own comment). Once a second window
    // (CMyInventory, H7 Stage 1) started calling it independently of CMainFrameWindow, a window
    // whose own Render() runs at a *later* GetLayerDepth() than another window's 3D-icon camera
    // (both are ordinary CObjects interleaved by CManager::Render()'s one z-sorted pass, see
    // docs/ui-target-architecture.md Section A) would re-replay -- and thus re-paint over -- that
    // earlier window's already-drawn 3D icons. This guard makes only the FIRST call in a frame
    // actually render; every later call this same frame is a cheap no-op. Safe: the first call in
    // z-order is guaranteed to precede every camera later in the same sorted pass (that's the
    // existing "wire your own call before your own icons" contract every caller already follows),
    // so one replay per frame, positioned at the earliest caller, still correctly precedes all of
    // them. Reset in RenderFrame() (the one guaranteed-once-per-frame choke point), not
    // BeginFrame(), since RenderFrame() fires late (SetPreSubmitCallback) -- after every
    // RenderBackgroundLayer() call this frame already happened, so resetting there arms the guard
    // correctly for the *next* frame's first caller.
    bool m_backgroundLayerRenderedThisFrame = false;
};
