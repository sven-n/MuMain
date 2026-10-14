#include "stdafx.h"
#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/RmlBridge/RmlElementBox.h"

#include <cmath>

#include <RmlUi/Core/Element.h>

#include "Camera/CameraProjection.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"

UI::Items::ItemCameraTarget::ItemCameraTarget(Drawer drawer)
    : m_drawer(std::move(drawer)),
      m_target([this](std::uint32_t width, std::uint32_t height) { Render(width, height); })
{
}

void UI::Items::ItemCameraTarget::Sync(Rml::Element* image, bool enabled)
{
    if (image == nullptr)
    {
        m_target.SetEnabled(false);
        return;
    }
    const bool placed = UI::RmlBridge::DrawnContentBox(*image, m_offset, m_size);
    const bool drawing = enabled && placed && m_size.x >= 1.f && m_size.y >= 1.f;
    m_target.SetEnabled(drawing);
    // Drawn at the size it is shown, never upscaled.
    if (drawing)
        m_target.Resize(static_cast<std::uint32_t>(std::lround(m_size.x)), static_cast<std::uint32_t>(std::lround(m_size.y)));
    // An <img> without a texture still draws its quad, untextured: white. It stays laid out, since
    // the target takes its size from the box.
    const Rml::String source = drawing ? m_target.Source() : Rml::String();
    if (source.empty())
        image->SetProperty("visibility", "hidden");
    else
        image->RemoveProperty("visibility");
    if (!source.empty() && image->GetAttribute<Rml::String>("src", "") != source)
        image->SetAttribute("src", source);
}

void UI::Items::ItemCameraTarget::Render(std::uint32_t width, std::uint32_t height)
{
    if (width == 0 || height == 0 || !m_drawer)
        return;

    const float windowWidth = static_cast<float>(WindowWidth);
    const float windowHeight = static_cast<float>(WindowHeight);
    const float w = m_size.x;
    const float h = m_size.y;
    if (w < 1.f || h < 1.f)
        return;

    // gluPerspective2() and the identity view overwrite g_Camera, which picking reads.
    SaveCameraPerspective();
    {
        const UI::Scaling::ScopedWindowPixels pixels(WindowWidth, WindowHeight);

        auto& renderer = mu::GetRenderer();
        renderer.SetMatrixMode(GL_PROJECTION);
        renderer.PushMatrix();
        renderer.LoadIdentity();
        const float scaleX = windowWidth / w;
        const float scaleY = windowHeight / h;
        const float centerX = (2.f * m_offset.x + w) / windowWidth - 1.f;
        const float centerY = 1.f - (2.f * m_offset.y + h) / windowHeight;
        renderer.Translate(-centerX * scaleX, -centerY * scaleY, 0.f);
        renderer.Scale(scaleX, scaleY, 1.f);
        // gluPerspective2() takes the camera's screen centre from the viewport; the capture brings its own.
        SetRenderViewport(0, 0, WindowWidth, WindowHeight);
        gluPerspective2(m_fieldOfView, windowWidth / windowHeight, RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
        renderer.SetMatrixMode(GL_MODELVIEW);
        renderer.PushMatrix();
        renderer.LoadIdentity();
        CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
        EnableDepthTest();
        EnableDepthMask();
        // Native 2D drawn here lands where it would have on screen too.
        renderer.SetOffscreen2DRect(m_offset.x, m_offset.y, w, h);

        m_drawer(m_offset, m_size);

        renderer.SetMatrixMode(GL_MODELVIEW);
        renderer.PopMatrix();
        renderer.SetMatrixMode(GL_PROJECTION);
        renderer.PopMatrix();
    }
    RestoreCameraPerspective();
}
