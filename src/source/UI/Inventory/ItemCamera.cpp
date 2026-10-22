#include "stdafx.h"
#include "UI/Inventory/ItemCamera.h"

#include "Camera/CameraProjection.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/Scaling/UITransform.h"

void UI::Items::DrawUnderItemCamera(float left, float top, float width, float height, float fieldOfView,
                                    const std::function<void()>& draw)
{
    const float windowWidth = static_cast<float>(WindowWidth);
    const float windowHeight = static_cast<float>(WindowHeight);

    // gluPerspective2() and the identity view overwrite g_Camera, which picking reads.
    SaveCameraPerspective();
    {
        const UI::Scaling::ScopedWindowPixels pixels(WindowWidth, WindowHeight);

        auto& renderer = mu::GetRenderer();
        renderer.SetMatrixMode(GL_PROJECTION);
        renderer.PushMatrix();
        renderer.LoadIdentity();
        const float scaleX = windowWidth / width;
        const float scaleY = windowHeight / height;
        const float centerX = (2.f * left + width) / windowWidth - 1.f;
        const float centerY = 1.f - (2.f * top + height) / windowHeight;
        renderer.Translate(-centerX * scaleX, -centerY * scaleY, 0.f);
        renderer.Scale(scaleX, scaleY, 1.f);
        // gluPerspective2() takes the camera's screen centre from the viewport; the capture brings its own.
        SetRenderViewport(0, 0, WindowWidth, WindowHeight);
        gluPerspective2(fieldOfView, windowWidth / windowHeight, RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
        renderer.SetMatrixMode(GL_MODELVIEW);
        renderer.PushMatrix();
        renderer.LoadIdentity();
        CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
        EnableDepthTest();
        EnableDepthMask();
        // Native 2D drawn here lands where it would have on screen too.
        renderer.SetOffscreen2DRect(left, top, width, height);

        draw();

        renderer.SetMatrixMode(GL_MODELVIEW);
        renderer.PopMatrix();
        renderer.SetMatrixMode(GL_PROJECTION);
        renderer.PopMatrix();
    }
    RestoreCameraPerspective();
}
