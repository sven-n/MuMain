#pragma once

// LoginSceneOverlay.h - the login scene's logo and bottom lines in RmlUi (login_scene.rml)

namespace Scenes::LoginOverlay
{
// What NewRenderLogInScene() draws over the world: the MU logo in tour mode at its fade level
// (`logoAlpha`, g_fMULogoAlpha), the copyright, "All Rights Reserved." and version lines. Call
// once per rendered login frame with the native 2D transform active. False when RmlUi is not
// available: the caller then draws them natively.
bool Render(bool tourMode, float logoAlpha, const wchar_t* copyright, const wchar_t* rights, const wchar_t* version);

// Hides the document once the login scene is left; call once per frame before the scene draws.
void HideOutsideLoginScene();
} // namespace Scenes::LoginOverlay
