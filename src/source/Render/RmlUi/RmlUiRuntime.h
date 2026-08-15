#pragma once

#include "stdafx.h"
#include <RmlUi/Core/Context.h>
#include <memory>

class RmlUiRenderInterface;
class RmlUiSystemInterface;
union SDL_Event;
struct SDL_Window;

// Owns the Rml::Context lifecycle and the one per-frame Update()/Render() entry point. Hooked
// from SceneManager.cpp's RenderScene() -- see the RmlUi migration plan's Phase 0.4 for why that
// call site (not a per-Scene hook) is the single choke point every scene funnels through.
class RmlUiRuntime
{
public:
    static RmlUiRuntime& Instance();

    void Create(int windowWidth, int windowHeight);
    void Destroy();
    bool IsCreated() const { return m_Context != nullptr; }

    void OnResize(int windowWidth, int windowHeight);

    void Update();
    void Render();

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
    bool ProcessSdlEvent(SDL_Event& event, SDL_Window* window);

    // Rml::Context::IsMouseInteracting() -- true while the mouse is hovering/pressed over any
    // RmlUi element. Intended as a third click-to-move gate alongside MouseOnWindow (Legacy tier)
    // and g_pNewUISystem->CheckMouseUse() (NewUI tier) at Input/Selection.cpp's world-pick check.
    bool IsMouseOverUI() const;

    Rml::Context* GetContext() const { return m_Context; }

private:
    RmlUiRuntime() = default;
    ~RmlUiRuntime();

    RmlUiRuntime(const RmlUiRuntime&) = delete;
    RmlUiRuntime& operator=(const RmlUiRuntime&) = delete;

    std::unique_ptr<RmlUiRenderInterface> m_RenderInterface;
    std::unique_ptr<RmlUiSystemInterface> m_SystemInterface;
    Rml::Context* m_Context = nullptr; // owned by Rml::Core, released via Rml::Shutdown()
};
