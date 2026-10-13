///////////////////////////////////////////////////////////////////////////////
// LoginSceneOverlay.cpp - the login scene's logo and bottom lines in RmlUi
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LoginSceneOverlay.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "App/Platform/Windows/Winmain.h" // g_hFont
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

extern EGameScene SceneFlag;

namespace
{
// Physical px from the transform the native drawing used; levels 0..1.
struct LoginSceneRmlModel
{
    bool logoVisible = false;
    float logoLevel = 0.f;
    Rml::String logoColor;
    float glowLevel = 0.f;
    float textPx = 0.f;       // the font's cached size, which the native renderer rasterises at
    float fontScale = 1.f;    // the native size / textPx: the lines are scaled down by it
    float lineHeightPx = 0.f; // before the scale
    Rml::String copyright;
    Rml::String rights;
    Rml::String version;
};

void BindModel(Rml::DataModelConstructor& c, LoginSceneRmlModel& model)
{
    c.Bind("logo_visible", &model.logoVisible);
    c.Bind("logo_level", &model.logoLevel);
    c.Bind("logo_color", &model.logoColor);
    c.Bind("glow_level", &model.glowLevel);
    c.Bind("text_px", &model.textPx);
    c.Bind("font_scale", &model.fontScale);
    c.Bind("line_height_px", &model.lineHeightPx);
    c.Bind("copyright", &model.copyright);
    c.Bind("rights", &model.rights);
    c.Bind("version", &model.version);
}

UI::RmlBridge::ThemedView<LoginSceneRmlModel> s_view{"login_scene", BindModel,
                                                     {{"Data/Interface/RmlUi/login_scene.rml"}}};
std::wstring s_copyrightSource;
std::wstring s_rightsSource;
std::wstring s_versionSource;

// A line's text, converted only when the native string changed (or the
// model is new): the overlay syncs every frame of the login scene.
void SyncText(Rml::String LoginSceneRmlModel::* field, const char* name, const wchar_t* text, std::wstring& source)
{
    if (source == text && !(s_view.Binder().GetModel().*field).empty())
        return;
    source = text;
    SyncField(s_view.Binder(), field, name, StringUtils::WideToNarrow(text));
}

// The native levels: a BYTE from the clamped fade value, as RGBA(level, level, level, level).
float Level(float fade)
{
    return static_cast<float>(static_cast<BYTE>(std::clamp(fade, 0.f, 1.f) * 255.f)) / 255.f;
}
} // namespace

namespace Scenes::LoginOverlay
{
bool Render(bool tourMode, float logoAlpha, const wchar_t* copyright, const wchar_t* rights, const wchar_t* version)
{
    s_view.Ensure();
    if (s_view.Document() == nullptr)
        return false;

    UI::RmlBridge::SyncDocumentVisibilityBehind(s_view.Document(), true);

    // The original's 640x480 screen stretched over the window.
    const UI::Scaling::Transform transform =
        UI::Scaling::ScreenOverlayTransform(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));

    // The logo, its glow first; the theme places it.
    SyncField(s_view.Binder(), &LoginSceneRmlModel::logoVisible, "logo_visible", tourMode);
    if (tourMode)
    {
        SyncField(s_view.Binder(), &LoginSceneRmlModel::glowLevel, "glow_level", Level(logoAlpha - 0.3f));
        const float logoLevel = Level(logoAlpha);
        // The colour follows the level; formatted only when the fade moves it.
        if (s_view.Binder().GetModel().logoLevel != logoLevel || s_view.Binder().GetModel().logoColor.empty())
        {
            char color[32];
            const long channel = std::lround(logoLevel * 255.f);
            std::snprintf(color, sizeof(color), "rgb(%ld, %ld, %ld)", channel, channel, channel);
            SyncField(s_view.Binder(), &LoginSceneRmlModel::logoColor, "logo_color", Rml::String(color));
        }
        SyncField(s_view.Binder(), &LoginSceneRmlModel::logoLevel, "logo_level", logoLevel);
    }

    // The lines, which the theme places. The native renderer rasterises at the cached size and
    // scales the glyphs down to the native size; text laid out at that size directly comes out
    // wider (hinting), so the lines do the same.
    g_pRenderText->SetFont(g_hFont);
    const SIZE lineSize = g_pRenderText->MeasureText(L"Q", 1);
    const float cachedPx = static_cast<float>(UI::Scaling::CachedFontPointSize(UI::Scaling::FontRole::Normal));
    const float fontScale = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform) / cachedPx;
    SyncField(s_view.Binder(), &LoginSceneRmlModel::textPx, "text_px", cachedPx);
    SyncField(s_view.Binder(), &LoginSceneRmlModel::fontScale, "font_scale", fontScale);
    SyncField(s_view.Binder(), &LoginSceneRmlModel::lineHeightPx, "line_height_px",
              static_cast<float>(lineSize.cy) * transform.scaleY / fontScale);
    SyncText(&LoginSceneRmlModel::copyright, "copyright", copyright, s_copyrightSource);
    SyncText(&LoginSceneRmlModel::rights, "rights", rights, s_rightsSource);
    SyncText(&LoginSceneRmlModel::version, "version", version, s_versionSource);
    return true;
}

void HideOutsideLoginScene()
{
    if (SceneFlag != LOG_IN_SCENE)
        UI::RmlBridge::SyncDocumentVisibilityBehind(s_view.Document(), false);
}
} // namespace Scenes::LoginOverlay
