#include "stdafx.h"
#include "RmlUiRuntime.h"
#include "RmlUiRenderInterface.h"
#include "RmlUiSystemInterface.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi_Platform_SDL.h> // ThirdParty/RmlUi/Backends -- see the CMakeLists.txt addition
#include "Render/Renderer/MuRenderer.h"
#include "Data/GameConfig/GameConfig.h"
#include "UI/Scaling/UITransform.h"

namespace
{
    // Global UI scale (docs/rmlui-ui-system/layout-and-scaling.md) -- the one RmlUi-native call
    // site every `dp`-authored RCSS dimension responds to. Two multipliers, composed the same way
    // UI::Scaling's own legacy transforms compose them (UITransform.cpp):
    //   - GameConfig::GetUIScalePercent() -- the user's own preference dial.
    //   - UI::Scaling::ViewportFitScale() -- auto-fit-to-window-size (the same formula the still-
    //     legacy HUD band's bars_scale uses via BottomHudScale/BottomHudCenterTransform), which
    //     already folds in UI::Scaling::GetWindowContentScale() (OS display-scale/pixel-density)
    //     internally -- do NOT also multiply GetWindowContentScale() here, it would double-count.
    // Before 2026-09-03 this ratio had no auto-fit term at all, so every dp-authored window (mu
    // helper bar, buff strip, the HUD's top-right button row, and -- since dialogs were deliberately
    // un-exempted the same day -- login/sys_menu/msg_win/etc.) stayed a fixed size regardless of
    // window resolution while the still-legacy HUD bars right next to them grew with it. This
    // makes every dp window participate in the same auto-fit the legacy transforms already have.
    //
    // NOT yet verified on real mismatched-density hardware -- Rml::Context's dimensions are set
    // from SDL's window-coordinate size (RmlUiRuntime::OnResize, see its own comment), not
    // SDL_GetWindowSizeInPixels(), and the SDL window requests SDL_WINDOW_HIGH_PIXEL_DENSITY
    // (SDLWindowFlags.h). Whether RenderInterface_SDL_GPU's viewport already stretches that
    // window-coordinate-sized canvas across the full pixel framebuffer (in which case the
    // content-scale term folded into ViewportFitScale would double-scale) or renders it 1:1 (in
    // which case it's the missing piece) needs a real scaled display or a
    // UI::Scaling::SetWindowContentScale() debug override to confirm directly -- see
    // docs/rmlui-ui-system/layout-and-scaling.md. This was already an open question before this
    // change; adding the auto-fit term doesn't add a new one, just carries the existing one forward.
    //
    // Re-applied on resize too: SetDensityIndependentPixelRatio() sets an absolute ratio, not a
    // relative one, so it doesn't drift on its own, but re-asserting it here costs nothing and
    // removes any doubt about whether some other code path could have reset it in between.
    void ApplyUIScale(Rml::Context* context, int windowWidth, int windowHeight)
    {
        if (!context) return;
        const int percent = GameConfig::GetInstance().GetUIScalePercent();
        const float autoFit =
            UI::Scaling::ViewportFitScale(windowWidth, windowHeight, UI::Scaling::MaximumPanelScale);
        context->SetDensityIndependentPixelRatio((static_cast<float>(percent) / 100.0f) * autoFit);
    }
}

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

    auto* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
    auto* window = mu::GetRenderer().GetWindow();
    if (!device || !window)
    {
        // Matches GetDevice()/GetWindow()'s own "not initialized" nullptr contract -- Create()
        // must run after the SDL_GPU renderer's Init(), same ordering requirement the RHI-based
        // version had (RHI::Init() before RmlUiRuntime::Create()).
        return;
    }

    m_RenderInterface = std::make_unique<RmlUiRenderInterface>(device, window);
    m_SystemInterface = std::make_unique<RmlUiSystemInterface>();

    Rml::SetRenderInterface(m_RenderInterface.get());
    Rml::SetSystemInterface(m_SystemInterface.get());

    if (!Rml::Initialise())
    {
        m_RenderInterface.reset();
        m_SystemInterface.reset();
        return;
    }

    // Reuses the same bundled fonts this engine already ships for its portable text shim
    // (fonts/LiberationSans-*.ttf, copied next to the exe by the same asset-copy step as
    // everything else under src/bin/) rather than adding a new font dependency. fallback_face=
    // true on the regular weight means any RML/RCSS font-family that doesn't match a loaded face
    // still renders with this one instead of silently rendering no text at all.
    Rml::LoadFontFace("fonts/LiberationSans-Regular.ttf", true);
    Rml::LoadFontFace("fonts/LiberationSans-Bold.ttf");

    m_Context = Rml::CreateContext("main", Rml::Vector2i(windowWidth, windowHeight));
    ApplyUIScale(m_Context, windowWidth, windowHeight);

    // See m_BackgroundContext's own comment (RmlUiRuntime.h) -- a second, independent context,
    // same dimensions/scale as "main", named distinctly so RmlUi's own context registry and any
    // future debug tooling can tell them apart.
    m_BackgroundContext = Rml::CreateContext("background", Rml::Vector2i(windowWidth, windowHeight));
    ApplyUIScale(m_BackgroundContext, windowWidth, windowHeight);

    // Renders once per frame, after this frame's game content is recorded onto the command
    // buffer but before it's submitted -- see SetPreSubmitCallback's own comment (MuRenderer.h)
    // for why this exact seam is required instead of a per-scene Update()/Render() call site
    // the way the RHI-based version had it. This also means every scene gets RmlUi rendering
    // for free, including Webzen (which had no hook at all before) -- an incidental fix, not
    // something this port set out to change.
    mu::GetRenderer().SetPreSubmitCallback([]() { RmlUiRuntime::Instance().RenderFrame(); });

    // Register as the active UI input consumer (UiInputRouter.h) -- Winmain.cpp's event pump and
    // gameplay's mouse-gating checks (Selection.cpp, ZzzInterface.cpp) go through the router, not
    // this concrete type, from this point on.
    Core::Input::SetUiInputConsumer(this);
}

void RmlUiRuntime::Destroy()
{
    if (!m_Context) return;

    Core::Input::SetUiInputConsumer(nullptr);
    mu::GetRenderer().SetPreSubmitCallback(nullptr);

    // Rml::Shutdown() releases every context it owns, including m_Context -- do not call
    // Rml::RemoveContext/delete it separately first. It also releases every outstanding
    // compiled-geometry/texture handle via RmlUiRenderInterface, which must still be able to
    // reach a live SDL_GPUDevice/RenderInterface_SDL_GPU while that happens -- do not reset
    // m_RenderInterface before this call.
    Rml::Shutdown();
    m_Context = nullptr;
    m_BackgroundContext = nullptr; // released by the same Rml::Shutdown() call above

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
    ApplyUIScale(m_Context, windowWidth, windowHeight);

    if (m_BackgroundContext)
    {
        m_BackgroundContext->SetDimensions(Rml::Vector2i(windowWidth, windowHeight));
        ApplyUIScale(m_BackgroundContext, windowWidth, windowHeight);
    }
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
    // RmlUi elements) passes through this function. Still reuses RmlSDL::ConvertMouseButton/
    // GetKeyModifierState (pure, side-effect-free helpers) for the actual button index/modifier
    // mapping.
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        return m_Context->ProcessMouseButtonDown(RmlSDL::ConvertMouseButton(event.button.button), RmlSDL::GetKeyModifierState());
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        return m_Context->ProcessMouseButtonUp(RmlSDL::ConvertMouseButton(event.button.button), RmlSDL::GetKeyModifierState());

    // Mouse motion is also handled directly rather than through RmlSDL::InputEventHandler, which
    // multiplies the raw event coordinates by SDL_GetWindowPixelDensity() before forwarding them
    // -- correct for a backend whose Context was created in DPI-independent "points" and needs
    // converting up to real pixels, but this engine's own HandleMouseMotion() (Winmain.cpp)
    // applies no such scaling, and RmlUiRuntime::Create() built the Context directly from real
    // window pixel dimensions. Multiplying by density on top of that would double-apply any
    // non-1.0 scaling. Feed the same raw coordinates HandleMouseMotion() sees.
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

    // Pulls the current frame's still-open command buffer + swapchain texture straight from the
    // renderer -- valid here because SetPreSubmitCallback (see Create() above) only ever invokes
    // this before the frame's command buffer is submitted. RenderInterface_SDL_GPU::BeginFrame
    // starts its own render pass on that same buffer/texture (so it composites on top of
    // everything already recorded, not into a separate image), Context::Render() records RmlUi's
    // draws into it via the base class's own compile/replay pipeline, and EndFrame() closes that
    // render pass. Nothing here needs to save/restore any renderer state the way the RHI-based
    // version had to (GlobalUBO's proj/view/model stack, RHI::BlendMode) -- SDL_GPU pipelines
    // carry their own state per draw, so there is nothing global left to leak into whatever
    // renders next.
    const mu::FrameGpuContext ctx = mu::GetRenderer().GetFrameGpuContext();
    if (!ctx.commandBuffer || !ctx.swapchainTexture) return;

    m_RenderInterface->BeginFrame(ctx.commandBuffer, ctx.swapchainTexture, ctx.width, ctx.height);
    m_Context->Render();
    m_RenderInterface->EndFrame();
}

void RmlUiRuntime::RenderFrame()
{
    Update();
    Render();
}

void RmlUiRuntime::RenderBackgroundLayer()
{
    if (!m_BackgroundContext) return;

    // Opens a real render pass now, replaying whatever the caller's own legacy content has
    // recorded so far this frame -- see FlushRenderCommands()'s own comment (MuRenderer.h) for
    // why this is required before this context can render into the same command buffer at this
    // point, rather than waiting for the frame's one pre-existing pass. A no-op (returns
    // immediately) if nothing new has been recorded since the last flush -- harmless either way,
    // this context still renders below regardless.
    mu::GetRenderer().FlushRenderCommands();

    // Same seam Render() uses for "main" -- see that method's own comment for why this is only
    // valid here (still inside Begin/EndFrame's command buffer) and needs no state save/restore.
    const mu::FrameGpuContext ctx = mu::GetRenderer().GetFrameGpuContext();
    if (!ctx.commandBuffer || !ctx.swapchainTexture) return;

    m_BackgroundContext->Update();
    m_RenderInterface->BeginFrame(ctx.commandBuffer, ctx.swapchainTexture, ctx.width, ctx.height);
    m_BackgroundContext->Render();
    m_RenderInterface->EndFrame();
}
