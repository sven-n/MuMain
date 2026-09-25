#include "stdafx.h"
#include "Core/Input/SyntheticInput.h"
#include "RmlUiRuntime.h"
#include "RmlUiRenderInterface.h"
#include "RmlUiSystemInterface.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi_Platform_SDL.h> // ThirdParty/RmlUi/Backends -- see the CMakeLists.txt addition
#include "Render/Renderer/MuRenderer.h"
#include "Data/GameConfig/GameConfig.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/FrameProfiler.h"

namespace
{
    // Global UI scale -- the one RmlUi-native call site every `dp`-authored RCSS dimension
    // responds to. Two multipliers, composed the same way
    // UI::Scaling's own legacy transforms compose them (UITransform.cpp):
    //   - GameConfig::GetUIScalePercent() -- the user's own preference dial.
    //   - UI::Scaling::ViewportFitScale() -- auto-fit-to-window-size (the same formula the still-
    //     legacy HUD band's bars_scale uses via BottomHudScale/BottomHudCenterTransform), which
    //     already folds in UI::Scaling::GetWindowContentScale() (OS display-scale/pixel-density)
    //     internally -- do NOT also multiply GetWindowContentScale() here, it would double-count.
    //
    // Confirmed live on a 125%-scaled (non-high-DPI-pixel-density) display: contentScale folded
    // into ViewportFitScale's UPPER bound only (see that function's own comment -- it used to also
    // raise the lower bound, which overflowed every reference-pixel layout at/near the reference
    // resolution). No double-scaling seen at the reference size after that fix. Whether a genuine
    // high-pixel-density panel (SDL_GetWindowPixelDensity() > 1, not just an OS scale preference)
    // needs anything different here is still unconfirmed.
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
        // must run after the SDL_GPU renderer's Init().
        return;
    }

    m_RenderInterface = std::make_unique<RmlUiRenderInterface>(device, window);
    m_SystemInterface = std::make_unique<RmlUiSystemInterface>(window);

    Rml::SetRenderInterface(m_RenderInterface.get());
    Rml::SetSystemInterface(m_SystemInterface.get());

    if (!Rml::Initialise())
    {
        m_RenderInterface.reset();
        m_SystemInterface.reset();
        return;
    }

    // See m_TextInputMethodEditor's own header comment -- installs RmlUi's own vendored SDL IME
    // bridge globally, once, for the lifetime of this runtime. Must run after Rml::Initialise()
    // (matches every vendored sample backend's own ordering).
    m_TextInputMethodEditor = std::make_unique<TextInputMethodEditor_SDL>();
    Rml::SetTextInputHandler(m_TextInputMethodEditor.get());

    // Reuses the same bundled fonts this engine already ships for its portable text shim
    // (fonts/LiberationSans-*.ttf, copied next to the exe by the same asset-copy step as
    // everything else under src/bin/) rather than adding a new font dependency. fallback_face=
    // true on the regular weight means any RML/RCSS font-family that doesn't match a loaded face
    // still renders with this one instead of silently rendering no text at all.
    Rml::LoadFontFace("fonts/LiberationSans-Regular.ttf", true);
    Rml::LoadFontFace("fonts/LiberationSans-Bold.ttf");

    // Second, explicitly-named face (not a fallback_face) -- CCreditWin's scrolling credit text is
    // the first RmlUi content to need it, matching the font family the legacy GDI path names via
    // I18N::Game::Gulim ("NanumGothic" is this file's own internal name-table family name, not a
    // guess -- CCreditWin.rcss's font-family must match it exactly). Broader CJK/Cyrillic RmlUi
    // text coverage beyond this one face is a separate, pre-existing gap (every other ported
    // window's legacy theme still hardcodes "Liberation Sans"), not something this addresses.
    Rml::LoadFontFace("fonts/NanumGothic-Regular.ttf");

    // Third, explicitly-named face -- already bundled for the legacy GDI text shim's own font
    // picker (BundledFonts.h) but never previously registered with RmlUi. Loaded for its Unicode
    // symbol coverage: the modern theme's login/char-select icon buttons (base.rcss's
    // .btn-icon-label) use glyphs like U+2261/U+2726/U+2699/U+21C4/U+2715 that Liberation Sans and
    // NanumGothic mostly don't have, but DejaVu Sans does.
    Rml::LoadFontFace("fonts/DejaVuSans.ttf");
    Rml::LoadFontFace("fonts/DejaVuSans-Bold.ttf");

    m_Context = Rml::CreateContext("main", Rml::Vector2i(windowWidth, windowHeight));
    ApplyUIScale(m_Context, windowWidth, windowHeight);

    // See m_BackgroundContext's own comment (RmlUiRuntime.h) -- a second, independent context,
    // same dimensions/scale as "main", named distinctly so RmlUi's own context registry and any
    // future debug tooling can tell them apart.
    m_BackgroundContext = Rml::CreateContext("background", Rml::Vector2i(windowWidth, windowHeight));
    ApplyUIScale(m_BackgroundContext, windowWidth, windowHeight);

    // See GetDialogBackgroundContext()'s own comment -- a third context, exclusively for
    // CGenericConfirmDialog's own panel, rendered at a different point in the frame than
    // m_BackgroundContext's own documents.
    m_DialogBackgroundContext = Rml::CreateContext("dialog_background", Rml::Vector2i(windowWidth, windowHeight));
    ApplyUIScale(m_DialogBackgroundContext, windowWidth, windowHeight);

    // Renders once per frame, after this frame's game content is recorded onto the command
    // buffer but before it's submitted -- see SetPreSubmitCallback's own comment (MuRenderer.h)
    // for why this exact seam is required. A single choke point every scene renders through
    // uniformly, including Webzen.
    mu::GetRenderer().SetPreSubmitCallback([]() { RmlUiRuntime::Instance().RenderFrame(); });

    // Register as the active UI input consumer (UiInputRouter.h) -- Winmain.cpp's event pump and
    // gameplay's mouse-gating checks (Selection.cpp, ZzzInterface.cpp) go through the router, not
    // this concrete type, from this point on.
    Core::Input::SetUiInputConsumer(this);
}

void RmlUiRuntime::Destroy()
{
    if (!m_Context) return;

    Core::Input::Synthetic::CancelDelivery();
    Core::Input::SetUiInputConsumer(nullptr);
    mu::GetRenderer().SetPreSubmitCallback(nullptr);

    // Clears RmlUi's global registration before Shutdown() tears down contexts/documents/elements
    // -- matches the vendored Win32 backends' own teardown guard (RmlUi_Backend_Win32_*.cpp:
    // "if (Rml::GetTextInputHandler() == &data->text_input_method_editor) SetTextInputHandler
    // (nullptr)"). A live WidgetTextInputContext already holds its own captured handler pointer
    // from focus time, not a live lookup, so this alone doesn't protect m_TextInputMethodEditor
    // from being called during Shutdown() -- it must still stay alive until after that call
    // returns (see below), same as m_RenderInterface/m_SystemInterface.
    Rml::SetTextInputHandler(nullptr);

    // Rml::Shutdown() releases every context it owns, including m_Context -- do not call
    // Rml::RemoveContext/delete it separately first. It also releases every outstanding
    // compiled-geometry/texture handle via RmlUiRenderInterface, which must still be able to
    // reach a live SDL_GPUDevice/RenderInterface_SDL_GPU while that happens -- do not reset
    // m_RenderInterface before this call.
    Rml::Shutdown();
    m_Context = nullptr;
    m_BackgroundContext = nullptr; // released by the same Rml::Shutdown() call above
    m_DialogBackgroundContext = nullptr; // ditto

    // Per RenderInterface.h/SystemInterface.h's own contract: the application must keep these
    // alive until after Rml::Shutdown() and destroy them itself afterward -- RmlUi never takes
    // ownership. Same contract applies to m_TextInputMethodEditor (TextInputHandler.h has no
    // explicit statement of this, but WidgetTextInputContext's teardown path calls back into it
    // during element/document destruction, i.e. during the Shutdown() call above).
    m_RenderInterface.reset();
    m_SystemInterface.reset();
    m_TextInputMethodEditor.reset();
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

    if (m_DialogBackgroundContext)
    {
        m_DialogBackgroundContext->SetDimensions(Rml::Vector2i(windowWidth, windowHeight));
        ApplyUIScale(m_DialogBackgroundContext, windowWidth, windowHeight);
    }
}

void RmlUiRuntime::Update()
{
    if (!m_Context) return;
    FRAME_PROFILE(RmlUiUpdate);
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

void RmlUiRuntime::CancelSyntheticMousePress(unsigned char button, SDL_Window* window)
{
    if (!m_Context || button != SDL_BUTTON_LEFT)
        return; // RmlUi retains an active chain only for the primary button.
    // MouseLeave clears hover before Up, so vendored Context cannot dispatch
    // Mouseup/Click on the pressed element, but still clears its active chain.
    m_Context->ProcessMouseLeave();
    m_Context->ProcessMouseButtonUp(0, RmlSDL::GetKeyModifierState());
    if (window && SDL_GetMouseFocus() == window)
    {
        float x = 0.0f, y = 0.0f;
        SDL_GetMouseState(&x, &y);
        m_Context->ProcessMouseMove(static_cast<int>(x), static_cast<int>(y), RmlSDL::GetKeyModifierState());
    }
}

bool RmlUiRuntime::IsMouseOverUI() const
{
    return m_Context && m_Context->IsMouseInteracting();
}

bool RmlUiRuntime::IsTextInputActive() const
{
    return m_SystemInterface && m_SystemInterface->IsTextInputActive();
}

void RmlUiRuntime::ProcessTextEditing(const SDL_Event& event)
{
    if (m_TextInputMethodEditor)
        m_TextInputMethodEditor->HandleEdit(event.edit);
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
    // render pass. SDL_GPU pipelines carry their own state per draw, so there's no global
    // renderer state to save/restore around this.
    const mu::FrameGpuContext ctx = mu::GetRenderer().GetFrameGpuContext();
    if (!ctx.commandBuffer || !ctx.swapchainTexture) return;

    FRAME_PROFILE(RmlUiRender);
    m_RenderInterface->BeginFrame(ctx.commandBuffer, ctx.swapchainTexture, ctx.width, ctx.height);
    m_Context->Render();
    m_RenderInterface->EndFrame();
}

void RmlUiRuntime::RenderFrame()
{
    Update();
    Render();

    // Arms the RenderBackgroundLayer() once-per-frame guard for the next frame -- see
    // m_backgroundLayerRenderedThisFrame's own header comment (RmlUiRuntime.h) for why this,
    // not BeginFrame(), is the correct reset point.
    m_backgroundLayerRenderedThisFrame = false;
    m_dialogBackgroundLayerRenderedThisFrame = false;
}

void RmlUiRuntime::RenderBackgroundLayer()
{
    if (!m_BackgroundContext) return;
    if (m_backgroundLayerRenderedThisFrame) return;

    // Opens a real render pass now, replaying whatever the caller's own legacy content has
    // recorded so far this frame -- see FlushRenderCommands()'s own comment (MuRenderer.h) for
    // why this is required before this context can render into the same command buffer at this
    // point, rather than waiting for the frame's one pre-existing pass. A no-op (returns
    // immediately) if nothing new has been recorded since the last flush -- harmless either way,
    // this context still renders below regardless.
    mu::GetRenderer().FlushRenderCommands();

    // Same seam Render() uses for "main" -- see that method's own comment for why this is only
    // valid here (still inside Begin/EndFrame's command buffer) and needs no state save/restore.
    // GetFrameGpuContext() can legitimately come back null on a given frame (SDL_GPU swapchain
    // texture not yet available -- MuRendererSDLGpu.cpp's own BeginFrame(), not just a minimized
    // window), and the once-per-frame guard below must NOT latch on a frame where that happens:
    // this call site owns the shared background context's ONLY render for the whole frame (see
    // m_backgroundLayerRenderedThisFrame's own header comment), so latching here regardless of
    // whether a render actually happened would silently drop the background panel behind whatever
    // native 3D icon content still draws unconditionally later this same frame -- exactly the
    // "background blinks/looks transparent" symptom this comment is here to prevent regressing.
    const mu::FrameGpuContext ctx = mu::GetRenderer().GetFrameGpuContext();
    if (!ctx.commandBuffer || !ctx.swapchainTexture) return;

    m_backgroundLayerRenderedThisFrame = true;

    {
        FRAME_PROFILE(RmlUiUpdate);
        m_BackgroundContext->Update();
    }
    {
        FRAME_PROFILE(RmlUiRender);
        m_RenderInterface->BeginFrame(ctx.commandBuffer, ctx.swapchainTexture, ctx.width, ctx.height);
        m_BackgroundContext->Render();
        m_RenderInterface->EndFrame();
    }
}

void RmlUiRuntime::RenderDialogBackgroundLayer()
{
    if (!m_DialogBackgroundContext) return;
    if (m_dialogBackgroundLayerRenderedThisFrame) return;

    // Same reasoning as RenderBackgroundLayer()'s own FlushRenderCommands() call -- see that
    // method's comment. Called from a different point in the frame (CManager::Render(), right
    // before the first object at/past the 3D camera's own z-order, not before the first visible
    // object overall), so this flushes whatever every ordinary window's own Render() recorded in
    // between, not just whatever RenderBackgroundLayer() itself already flushed earlier this frame.
    mu::GetRenderer().FlushRenderCommands();

    const mu::FrameGpuContext ctx = mu::GetRenderer().GetFrameGpuContext();
    if (!ctx.commandBuffer || !ctx.swapchainTexture) return;

    m_dialogBackgroundLayerRenderedThisFrame = true;

    {
        FRAME_PROFILE(RmlUiUpdate);
        m_DialogBackgroundContext->Update();
    }
    {
        FRAME_PROFILE(RmlUiRender);
        m_RenderInterface->BeginFrame(ctx.commandBuffer, ctx.swapchainTexture, ctx.width, ctx.height);
        m_DialogBackgroundContext->Render();
        m_RenderInterface->EndFrame();
    }
}
