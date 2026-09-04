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
// funnels through regardless of active scene, unlike the RHI-based version's per-Scene hooks.
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
    // still goes through it -- still reusing RmlUi's own tested SDL-keycode/modifier mapping
    // rather than hand-rolling one (RmlUi migration plan Phase 0.8).
    // @return false if RmlUi consumed the event (an element under the cursor / holding focus
    // claimed it) -- the caller should then skip its own legacy handling for this event. true
    // means RmlUi did not consume it (matches Rml::Context::Process*'s own "still propagating"
    // convention) and legacy handling should proceed as before.
    //
    // Ordering caveat (RmlUi migration plan Open Decision #4, not solved here): this always
    // offers the event to RmlUi first, regardless of whether a legacy CNewUIObj/CWin window is
    // actually on top of the RmlUi content at that screen position -- correct as long as
    // migrated-window content and still-legacy windows occupy disjoint z-order bands (Phase
    // 0.8's "one contiguous band" approach), wrong if a legacy window ever needs to sit visually
    // on top of migrated RmlUi content. Revisit if that situation arises.
    bool ProcessSdlEvent(SDL_Event& event, SDL_Window* window) override;

    // Rml::Context::IsMouseInteracting() -- true while the mouse is hovering/pressed over any
    // RmlUi element. Intended as a third click-to-move gate alongside MouseOnWindow (Legacy tier)
    // and g_pNewUISystem->CheckMouseUse() (NewUI tier) at Input/Selection.cpp's world-pick check.
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
};
