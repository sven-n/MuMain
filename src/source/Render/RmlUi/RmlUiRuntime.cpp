#include "stdafx.h"
#include "RmlUiRuntime.h"
#include "RmlUiRenderInterface.h"
#include "RmlUiSystemInterface.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi_Platform_SDL.h> // ThirdParty/RmlUi/Backends -- see the CMakeLists.txt addition
#include "Render/RHI/RHI.h"
#include "Render/Core/GlobalUBO.h"
#include "Render/Core/ImmediateRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h" // WindowWidth/WindowHeight
#include <cstring>

RmlUiRuntime& RmlUiRuntime::Instance()
{
    static RmlUiRuntime instance;
    return instance;
}

RmlUiRuntime::~RmlUiRuntime()
{
    Destroy();
}

void RmlUiRuntime::Create(int windowWidth, int windowHeight)
{
    if (m_Context) return;

    m_RenderInterface = std::make_unique<RmlUiRenderInterface>();
    m_SystemInterface = std::make_unique<RmlUiSystemInterface>();

    Rml::SetRenderInterface(m_RenderInterface.get());
    Rml::SetSystemInterface(m_SystemInterface.get());

    if (!Rml::Initialise())
    {
        m_RenderInterface.reset();
        m_SystemInterface.reset();
        return;
    }

    // Reuses the same bundled fonts this engine already ships for its portable GDI-text shim
    // (Core/Platform/BundledFonts.h, "fonts/LiberationSans-*.ttf", copied next to the exe by the
    // same asset-copy step as everything else under src/bin/) rather than adding a new font
    // dependency. fallback_face=true on the regular weight means any RML/RCSS font-family that
    // doesn't match a loaded face still renders with this one instead of silently rendering no
    // text at all -- confirmed missing entirely (shapes rendered, text didn't) from a real
    // screenshot before this fix.
    Rml::LoadFontFace("fonts/LiberationSans-Regular.ttf", true);
    Rml::LoadFontFace("fonts/LiberationSans-Bold.ttf");

    m_Context = Rml::CreateContext("main", Rml::Vector2i(windowWidth, windowHeight));
}

void RmlUiRuntime::Destroy()
{
    if (!m_Context) return;

    // Rml::Shutdown() releases every context it owns, including m_Context -- do not call
    // Rml::RemoveContext/delete it separately first.
    Rml::Shutdown();
    m_Context = nullptr;

    // Per RenderInterface.h/SystemInterface.h's own contract: the application must keep these
    // alive until after Rml::Shutdown() and destroy them itself afterward -- RmlUi never takes
    // ownership.
    m_RenderInterface.reset();
    m_SystemInterface.reset();
}

void RmlUiRuntime::OnResize(int windowWidth, int windowHeight)
{
    if (!m_Context) return;
    m_Context->SetDimensions(Rml::Vector2i(windowWidth, windowHeight));
}

void RmlUiRuntime::Update()
{
    if (!m_Context) return;
    m_Context->Update();
}

bool RmlUiRuntime::ProcessSdlEvent(SDL_Event& event, SDL_Window* window)
{
    if (!m_Context) return true; // nothing to consume it -- let it fall through

    // Mouse button down/up are handled directly here instead of delegating to
    // RmlSDL::InputEventHandler, which also calls SDL_CaptureMouse(true/false) on every button
    // press/release (RmlUi_Platform_SDL.cpp) -- correct for a sample app that owns the whole
    // window, but a real bug in this engine: it has its own cursor rendering and mouse-clip
    // handling (Winmain.cpp's UpdateCursorClip()) that was never built to expect SDL's capture
    // mode being toggled externally, and every mouse click in the game (not just clicks on
    // RmlUi elements) passes through this function. Confirmed by a real user report: the cursor
    // stopped tracking correctly ("does not follow where you drag") starting right after the
    // first click-driven scene transition (server selection -> login) once RmlUi's button-event
    // path started firing. Still reuses RmlSDL::ConvertMouseButton/GetKeyModifierState (pure,
    // side-effect-free helpers) for the actual button index/modifier mapping.
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        return m_Context->ProcessMouseButtonDown(RmlSDL::ConvertMouseButton(event.button.button), RmlSDL::GetKeyModifierState());
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        return m_Context->ProcessMouseButtonUp(RmlSDL::ConvertMouseButton(event.button.button), RmlSDL::GetKeyModifierState());

    // Mouse motion is also handled directly rather than through RmlSDL::InputEventHandler, which
    // multiplies the raw event coordinates by SDL_GetWindowPixelDensity() before forwarding them
    // -- correct for a backend whose Context was created in DPI-independent "points" and needs
    // converting up to real pixels, but this engine's own HandleMouseMotion() (Winmain.cpp)
    // applies no such scaling, and RmlUiRuntime::Create() built the Context directly from
    // WindowWidth/WindowHeight (already real pixels). Multiplying by density on top of that
    // double-applies any non-1.0 scaling, drifting RmlUi's notion of the cursor position away
    // from where the game itself thinks it is -- plausible root cause of a real report ("mouse
    // only seems to work in some areas"). Feed the same raw coordinates HandleMouseMotion() sees.
    if (event.type == SDL_EVENT_MOUSE_MOTION)
        return m_Context->ProcessMouseMove(static_cast<int>(event.motion.x), static_cast<int>(event.motion.y), RmlSDL::GetKeyModifierState());

    return RmlSDL::InputEventHandler(m_Context, window, event);
}

bool RmlUiRuntime::IsMouseOverUI() const
{
    return m_Context && m_Context->IsMouseInteracting();
}

void RmlUiRuntime::Render()
{
    if (!m_Context) return;

    // Mirrors ZzzOpenglUtil.cpp's BeginBitmap()/EndBitmap() pattern (flush IR::, full viewport,
    // disable depth test, swap GlobalUBO's proj/view to an ortho pair, restore after) with one
    // deliberate difference: BeginBitmap() sets up a BOTTOM-UP ortho projection (matching legacy
    // 2D code's manual y = WindowHeight - y flips), but RmlUi's vertices already arrive in
    // top-left-origin, y-down pixel space and must not be re-flipped per-vertex. So this uses a
    // TOP-DOWN ortho (top/bottom swapped relative to BeginBitmap()) instead of reusing
    // BeginBitmap() itself. This is the same technique the in-progress D3D11 branch
    // (janblade/dx-only-port) already uses to reconcile GL-vs-D3D11 texture Y-origin at the
    // projection level rather than per-draw -- reusing GlobalUBO here means that fix is
    // inherited for free once the branches converge. See the RmlUi migration plan's Phase 0.4.
    IR::Flush();
    RHI::SetViewport(0, 0, static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    RHI::SetDepthTestEnabled(false);

    float savedProj[16];
    float savedView[16];
    memcpy(savedProj, GlobalUBO::Instance().GetProj(), sizeof(savedProj));
    memcpy(savedView, GlobalUBO::Instance().GetView(), sizeof(savedView));

    // RmlUiRenderInterface::RenderGeometry calls GlobalUBO::SetModel(origin, scale) once per
    // compiled-geometry draw to translate it to its element's screen position -- necessary, but
    // it leaves Model holding whatever the LAST draw this frame set it to (e.g. the login panel's
    // OK button origin) with nothing to put it back afterward. Legacy 2D/3D rendering (BeginBitmap/
    // BeginOpengl-based sprite, text and world draws) never sets Model itself -- it assumes Model
    // is identity, the way it always was before RmlUi existed. Confirmed as the cause of a real
    // report: RmlUi's own elements rendered in the right place, but every legacy element and the
    // "responsive"/clickable area had shifted toward the upper-right -- exactly what a leaked,
    // never-reset translation does to everything drawn after it. PushModel()/PopModel() (already
    // built for exactly this save/restore shape) brackets the whole call the same way savedProj/
    // savedView do above.
    GlobalUBO::Instance().PushModel();

    // SetOrtho() resets View to identity internally (GlobalUBO.cpp) -- no separate SetView() call
    // needed here. bottom/top are swapped relative to BeginBitmap()'s (0, W, 0, H) call to get the
    // top-down flip described above; confirmed against SetOrtho()'s closed-form matrix, not assumed.
    GlobalUBO::Instance().SetOrtho(0.0f, (float)WindowWidth, (float)WindowHeight, 0.0f);

    m_Context->Render();

    GlobalUBO::Instance().PopModel();

    // Scissor has no BindState-style cache to self-correct a leak (RHI.h's comment on
    // SetScissorEnabled/SetScissorRect) -- force it off here unconditionally rather than trusting
    // that RmlUi's own EnableScissorRegion(false) was the last scissor call this frame, so a
    // stale clip rect can never survive into the next renderer's draws.
    RHI::SetScissorEnabled(false);

    // RmlUiRenderInterface::RenderGeometry leaves RHI::BlendMode::Blend3 active on the last
    // draw call (cull+depthmask OFF, alpha blend ON per RHI.h's enum comment) -- nothing resets
    // this per-frame the way BeginOpengl() forces depth-test/cull/depth-write/alpha-test back on
    // unconditionally every frame (ZzzOpenglUtil.cpp), so restore a known-good opaque state
    // explicitly here rather than assuming whatever runs next sets its own blend mode first.
    RHI::SetBlendMode(RHI::BlendMode::Opaque);

    GlobalUBO::Instance().SetProj(savedProj);
    GlobalUBO::Instance().SetView(savedView);
}
