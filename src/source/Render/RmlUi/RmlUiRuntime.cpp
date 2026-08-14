#include "stdafx.h"
#include "RmlUiRuntime.h"
#include "RmlUiRenderInterface.h"
#include "RmlUiSystemInterface.h"

#include <RmlUi/Core/Core.h>
#include "Render/RHI/RHI.h"
#include "Render/Core/GlobalUBO.h"
#include "Render/Core/ImmediateRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h" // WindowWidth/WindowHeight
#include <cstring>

#include <RmlUi_Platform_SDL.h> // ThirdParty/RmlUi/Backends -- see the CMakeLists.txt addition

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

    // SetOrtho() resets View to identity internally (GlobalUBO.cpp) -- no separate SetView() call
    // needed here. bottom/top are swapped relative to BeginBitmap()'s (0, W, 0, H) call to get the
    // top-down flip described above; confirmed against SetOrtho()'s closed-form matrix, not assumed.
    GlobalUBO::Instance().SetOrtho(0.0f, (float)WindowWidth, (float)WindowHeight, 0.0f);

    m_Context->Render();

    // Scissor has no BindState-style cache to self-correct a leak (RHI.h's comment on
    // SetScissorEnabled/SetScissorRect) -- force it off here unconditionally rather than trusting
    // that RmlUi's own EnableScissorRegion(false) was the last scissor call this frame, so a
    // stale clip rect can never survive into the next renderer's draws.
    RHI::SetScissorEnabled(false);

    GlobalUBO::Instance().SetProj(savedProj);
    GlobalUBO::Instance().SetView(savedView);
}
