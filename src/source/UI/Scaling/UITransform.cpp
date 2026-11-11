#include "UI/Scaling/UITransform.h"

#include <algorithm>
#include <cmath>

#include "App/stdafx.h"
#include "Data/GameConfig/GameConfig.h"

namespace
{
constexpr int kReferenceWidth = 640;
constexpr int kReferenceHeight = 480;
constexpr float kHudFrameHeight = 51.0f;
// ponytail: 2x HUD ceiling; raise only if native screenshots show unreadable controls.
constexpr float kMaximumHudScale = 2.0f;
// ponytail: 2x ceiling; raise only if native screenshots still show unreadable UI. Public copy of
// this value lives at UI::Scaling::MaximumPanelScale (UITransform.h) -- RmlUiRuntime.cpp's dp-ratio
// auto-fit reuses it, so it can't stay anonymous-namespace-only anymore.
// ponytail: 2.25x dock ceiling; adjust only from rebuilt native screenshots.
constexpr float kMaximumDockScale = 2.25f;
constexpr float kMaximumTypographyScale = 2.25f;
constexpr int kNormalFontPointSize = 11;
constexpr int kMaximumNormalFontPointSize = 16;
constexpr int kBigFontPointSize = 22;
constexpr int kMaximumBigFontPointSize = 32;
constexpr int kFixedFontPointSize = 13;
constexpr int kMaximumFixedFontPointSize = 18;
// ponytail: one gameplay window; move scale into a window context if multi-window rendering is added.
float g_windowContentScale = 1.0f;

// GameConfig::GetUIScalePercent() is an in-memory singleton read (no disk I/O per call, unlike
// the SDL queries GetWindowContentScale() caches), so this reads it directly rather than adding a
// second cached global. Applied after the fit scale's own ceiling (WithUIScalePercent()), so it still
// grows the UI where the auto-fit already sits at that ceiling, up to what the window holds.
float UIScalePercentMultiplier()
{
    return static_cast<float>(GameConfig::GetInstance().GetUIScalePercent()) / 100.0f;
}

struct FontPointRange
{
    int minimum;
    int maximum;
};

FontPointRange GetFontPointRange(UI::Scaling::FontRole role)
{
    if (role == UI::Scaling::FontRole::Big)
        return {kBigFontPointSize, kMaximumBigFontPointSize};
    if (role == UI::Scaling::FontRole::Fixed)
        return {kFixedFontPointSize, kMaximumFixedFontPointSize};
    return {kNormalFontPointSize, kMaximumNormalFontPointSize};
}

int RoundedBottomHudTop(int windowWidth, int windowHeight)
{
    const float hudTop =
        static_cast<float>(windowHeight) - kHudFrameHeight * UI::Scaling::BottomHudScale(windowWidth, windowHeight);
    return std::max(static_cast<int>(std::lround(hudTop)), 1);
}

// The player's UI scale on top of a fit scale, grown only as far as the original 640x480 screen
// still fits the window: a centred panel or a dock standing on the HUD is never cut off. Never
// below the fit scale itself, so a window smaller than the reference keeps its floor.
float WithUIScalePercent(int windowWidth, int windowHeight, float fitScale)
{
    const float fitsWindow = std::min(static_cast<float>(windowWidth) / kReferenceWidth,
                                      static_cast<float>(windowHeight) / kReferenceHeight);
    return std::min(fitScale * UIScalePercentMultiplier(), std::max(fitScale, fitsWindow));
}

float CappedUniformScale(int windowWidth, int windowHeight, float maximumScale)
{
    return WithUIScalePercent(windowWidth, windowHeight,
                              UI::Scaling::ViewportFitScale(windowWidth, windowHeight, maximumScale));
}

UI::Scaling::Transform DockTransform(int windowWidth, int windowHeight)
{
    const float scale = CappedUniformScale(windowWidth, windowHeight, kMaximumDockScale);
    const float offsetY = static_cast<float>(RoundedBottomHudTop(windowWidth, windowHeight))
                          - UI::Scaling::DockLogicalBottom * scale;
    return {scale, scale, 0.0f, offsetY, UI::Scaling::TypographyScale(windowWidth, windowHeight)};
}
}

UI::Scaling::Transform UI::Scaling::ScreenOverlayTransform(int windowWidth, int windowHeight)
{
    return {
        static_cast<float>(windowWidth) / kReferenceWidth,
        static_cast<float>(windowHeight) / kReferenceHeight,
        0.0f,
        0.0f,
        TypographyScale(windowWidth, windowHeight),
    };
}

UI::Scaling::Viewport UI::Scaling::FullReferenceViewport()
{
    return {0, 0, kReferenceWidth, kReferenceHeight};
}

UI::Scaling::Transform UI::Scaling::LegacyUiTransform(int windowWidth, int windowHeight)
{
    Transform transform = ScreenOverlayTransform(windowWidth, windowHeight);
    transform.typographyScale = 1.0f;
    return transform;
}

UI::Scaling::Transform UI::Scaling::PanelTransform(int windowWidth, int windowHeight)
{
    const float scale = CappedUniformScale(windowWidth, windowHeight, MaximumPanelScale);
    return {
        scale,
        scale,
        (static_cast<float>(windowWidth) - kReferenceWidth * scale) * 0.5f,
        (static_cast<float>(windowHeight) - kReferenceHeight * scale) * 0.5f,
        TypographyScale(windowWidth, windowHeight),
    };
}

float UI::Scaling::TypographyScale(int windowWidth, int windowHeight)
{
    return CappedUniformScale(windowWidth, windowHeight, MaximumPanelScale);
}

// Pure geometry + WindowContentScale, deliberately NOT including UIScalePercent -- every caller
// that needs the user's preference multiplies UIScalePercentMultiplier() in itself, once, so it's
// never double-counted.
// The one shared "fit the reference size to the real window, clamped" core, used by both the
// legacy UI::Scaling transforms (via CappedUniformScale/BottomHudScale below) and RmlUiRuntime.cpp's
// dp-ratio auto-fit.
float UI::Scaling::ViewportFitScale(int windowWidth, int windowHeight, float maximumScale)
{
    const float widthScale = static_cast<float>(windowWidth) / kReferenceWidth;
    const float heightScale = static_cast<float>(windowHeight) / kReferenceHeight;
    const float contentScale = GetWindowContentScale();
    // contentScale only widens headroom above the reference size -- it must NOT raise the floor
    // below it. Folding it into minBound too forces scale above 1.0 even exactly AT the reference
    // resolution whenever contentScale > 1 (any OS display-scale preference, e.g. Windows set to
    // 125%, not just genuine high-DPI pixel density), which overflows every reference-pixel layout
    // that assumes unity scale there with zero margin (found live: HUD + CMyInventory panel both
    // clipped at 640x480 on a 125%-scaled display).
    const float minBound = 1.0f;
    const float maxBound = maximumScale * contentScale;
    // Linear between the reference size and the ceiling -- the same ramp the original client's
    // UI::Scaling used, so UIScalePercent=100 reproduces the legacy layout exactly at every
    // resolution and a screenshot of a migrated window can be compared pixel for pixel against the
    // original. A user who finds this too large at a modest resolution turns UIScalePercent down;
    // that dial multiplies this value post-clamp (UIScalePercentMultiplier), so the curve itself
    // stays the single, predictable auto-fit every caller shares.
    return std::clamp(std::min(widthScale, heightScale), minBound, maxBound);
}

// Combined ratio every legacy "Type-2 companion" object -- a real, functional non-RmlUi widget
// (a CUITextInputBox, or a CButton scaled to match a dp-sized RmlUi sibling) kept in sync with a
// migrated window's now-RCSS-owned layout -- must scale its own fixed reference-pixel offsets by,
// to stay pixel-for-pixel aligned with the RmlUi element it's shadowing. Same composition
// RmlUiRuntime::RefreshScale() uses for RmlUi's own dp ratio (UIScalePercent x
// ViewportFitScale(MaximumPanelScale)) -- the single shared implementation, used by
// CharSelMainWin.cpp's GetUIScaleRatio(), LoginMainWin.cpp, and LoginWin.cpp's
// LoginUIScaleRatio(). Don't hand-copy this formula per window: a hand-copy risks reintroducing
// the same staleness bug (reading CInput::Instance().GetScreenWidth()/GetScreenHeight() instead of
// the WindowWidth/WindowHeight globals RmlUiRuntime::OnResize() actually uses). Callers must pass
// WindowWidth/WindowHeight (ZzzOpenglUtil.cpp), not a separate copy of the screen size.
float UI::Scaling::CompanionRatio(int windowWidth, int windowHeight)
{
    return CappedUniformScale(windowWidth, windowHeight, MaximumPanelScale);
}

float UI::Scaling::BottomHudScale(int windowWidth, int windowHeight)
{
    return WithUIScalePercent(windowWidth, windowHeight, ViewportFitScale(windowWidth, windowHeight, kMaximumHudScale));
}

UI::Scaling::Transform UI::Scaling::DockRightTransform(int windowWidth, int windowHeight)
{
    Transform transform = DockTransform(windowWidth, windowHeight);
    transform.offsetX = static_cast<float>(windowWidth) - kReferenceWidth * transform.scaleX;
    return transform;
}

float UI::Scaling::ScreenOverlayFullHeight(int windowWidth, int windowHeight)
{
    const Transform transform = ScreenOverlayTransform(windowWidth, windowHeight);
    return static_cast<float>(windowHeight) / transform.scaleY;
}

UI::Scaling::Viewport UI::Scaling::WorldViewport(int windowWidth, int windowHeight, bool)
{
    const int physicalWidth = std::max(windowWidth, 1);
    return {0, 0, physicalWidth, std::max(windowHeight, 1)};
}

float UI::Scaling::WorldViewportAspect(int windowWidth, int windowHeight, bool topViewEnabled)
{
    const Viewport viewport = WorldViewport(windowWidth, windowHeight, topViewEnabled);
    return static_cast<float>(viewport.width) / viewport.height;
}

float UI::Scaling::PositionX(const Transform& transform, float x)
{
    return x * transform.scaleX + transform.offsetX;
}

float UI::Scaling::PositionY(const Transform& transform, float y)
{
    return y * transform.scaleY + transform.offsetY;
}

float UI::Scaling::SizeX(const Transform& transform, float width)
{
    return width * transform.scaleX;
}

float UI::Scaling::SizeY(const Transform& transform, float height)
{
    return height * transform.scaleY;
}

float UI::Scaling::LogicalX(const Transform& transform, float windowX)
{
    return (windowX - transform.offsetX) / transform.scaleX;
}

float UI::Scaling::LogicalY(const Transform& transform, float windowY)
{
    return (windowY - transform.offsetY) / transform.scaleY;
}

int UI::Scaling::MinimumFontPointSize(FontRole role)
{
    return GetFontPointRange(role).minimum;
}

int UI::Scaling::MaximumFontPointSize(FontRole role)
{
    return GetFontPointRange(role).maximum;
}

int UI::Scaling::CachedFontPointSize(FontRole role)
{
    return std::max(static_cast<int>(std::lround(MaximumFontPointSize(role) * GetWindowContentScale())), 1);
}

int UI::Scaling::FontPointSize(FontRole role, const Transform& transform)
{
    const FontPointRange range = GetFontPointRange(role);
    const float typographyScale = transform.typographyScale / GetWindowContentScale();
    const float growth =
        std::clamp((typographyScale - 1.0f) / (kMaximumTypographyScale - 1.0f), 0.0f, 1.0f);
    const float pointSize = static_cast<float>(range.minimum) +
                            static_cast<float>(range.maximum - range.minimum) * growth;
    return static_cast<int>(std::lround(pointSize));
}

float UI::Scaling::NativeTextPixelSize(FontRole role, const Transform& transform)
{
    // CUIRenderTextSDLTtf opens every font at CachedFontPointSize() and draws it at
    // FontPointSize() / MaximumFontPointSize() of that (FontScaleForBounds() without a box).
    return static_cast<float>(CachedFontPointSize(role)) * static_cast<float>(FontPointSize(role, transform)) /
           static_cast<float>(MaximumFontPointSize(role));
}

float UI::Scaling::NativeTextPixelSize(FontRole role, int windowWidth, int windowHeight)
{
    return NativeTextPixelSize(role, WindowPixelTransform(windowWidth, windowHeight));
}

UI::Scaling::Transform UI::Scaling::WindowPixelTransform(int windowWidth, int windowHeight)
{
    return {1.0f, 1.0f, 0.0f, 0.0f, TypographyScale(windowWidth, windowHeight)};
}

UI::Scaling::ScopedWindowPixels::ScopedWindowPixels(int windowWidth, int windowHeight)
    : m_scope(WindowPixelTransform(windowWidth, windowHeight), true)
{
}

UI::Scaling::Transform UI::Scaling::TypographyUnitsTransform(int windowWidth, int windowHeight)
{
    const float scale = TypographyScale(windowWidth, windowHeight);
    return {scale, scale, 0.0f, 0.0f, scale};
}

UI::Scaling::ScopedScreenStretch::ScopedScreenStretch(int windowWidth, int windowHeight)
    : m_scope(ScreenOverlayTransform(windowWidth, windowHeight), true)
{
}

float UI::Scaling::PointerScale(int windowWidth, int windowHeight)
{
    return CappedUniformScale(windowWidth, windowHeight, MaximumPanelScale);
}

namespace
{
UI::Scaling::Transform PointerUnitsTransform(int windowWidth, int windowHeight, float pointerX, float pointerY,
                                             int logicalX, int logicalY)
{
    const float scale = UI::Scaling::PointerScale(windowWidth, windowHeight);
    return {scale, scale, pointerX - static_cast<float>(logicalX) * scale,
            pointerY - static_cast<float>(logicalY) * scale, UI::Scaling::TypographyScale(windowWidth, windowHeight)};
}
} // namespace

UI::Scaling::ScopedPointerUnits::ScopedPointerUnits(int windowWidth, int windowHeight, float pointerX, float pointerY,
                                                    int logicalX, int logicalY)
    : m_scope(PointerUnitsTransform(windowWidth, windowHeight, pointerX, pointerY, logicalX, logicalY))
{
}

float UI::Scaling::NativeTextPixelSizeInBox(FontRole role, const Transform& transform, float measuredWidth,
                                            float boxWidth)
{
    return FitTextPixelSizeToWidth(NativeTextPixelSize(role, transform), measuredWidth, boxWidth,
                                   MinimumTextPixelSize(role));
}

float UI::Scaling::NativeTextPixelSizeInBox(FontRole role, int windowWidth, int windowHeight, float measuredWidth,
                                            float boxWidth)
{
    return FitTextPixelSizeToWidth(NativeTextPixelSize(role, windowWidth, windowHeight), measuredWidth, boxWidth,
                                   MinimumTextPixelSize(role));
}

float UI::Scaling::MinimumTextPixelSize(FontRole role)
{
    return static_cast<float>(CachedFontPointSize(role)) * static_cast<float>(MinimumFontPointSize(role)) /
           static_cast<float>(MaximumFontPointSize(role));
}

float UI::Scaling::FitTextPixelSizeToWidth(float textPx, float measuredWidth, float boxWidth, float minimumPx)
{
    if (measuredWidth <= boxWidth || measuredWidth <= 0.0f)
        return textPx;

    return std::max(textPx * boxWidth / measuredWidth, std::min(minimumPx, textPx));
}

float UI::Scaling::SceneWindowScale(int windowWidth, int windowHeight)
{
    const float textPx = NativeTextPixelSize(FontRole::Normal, windowWidth, windowHeight);
    const float referenceTextPx = NativeTextPixelSize(FontRole::Normal, 1024, 768);
    return TextGrowthScale(textPx, referenceTextPx);
}

float UI::Scaling::TextGrowthScale(float textPx, float referenceTextPx)
{
    if (referenceTextPx <= 0.0f)
        return 1.0f;

    return std::max(textPx / referenceTextPx, 1.0f);
}

float UI::Scaling::SceneBarScale(int windowWidth, int windowHeight)
{
    return std::clamp(std::min(static_cast<float>(windowWidth) / 800.0f, static_cast<float>(windowHeight) / 600.0f),
                      1.0f, 2.0f);
}

float UI::Scaling::NativeTextPixelSizeInBounds(FontRole role, int windowWidth, int windowHeight, float measuredWidth,
                                               float measuredHeight, float boxWidth, float boxHeight)
{
    const float textPx = NativeTextPixelSize(role, windowWidth, windowHeight);
    float fit = 1.0f;
    if (boxWidth > 0.0f && measuredWidth > boxWidth)
        fit = std::min(fit, boxWidth / measuredWidth);
    if (boxHeight > 0.0f && measuredHeight > boxHeight)
        fit = std::min(fit, boxHeight / measuredHeight);
    if (fit >= 1.0f)
        return textPx;

    return std::max(textPx * fit, std::min(MinimumTextPixelSize(role), textPx));
}

float UI::Scaling::FontScaleForBounds(FontRole role, const Transform& transform, float measuredWidth,
                                      float measuredHeight, float boxWidth, float boxHeight)
{
    const float maximum = static_cast<float>(MaximumFontPointSize(role));
    const float minimumScale = static_cast<float>(MinimumFontPointSize(role)) / maximum;
    float scale = static_cast<float>(FontPointSize(role, transform)) / maximum;

    if (boxWidth > 0.0f && measuredWidth > 0.0f)
        scale = std::min(scale, boxWidth / measuredWidth);
    if (boxHeight > 0.0f && measuredHeight > 0.0f)
        scale = std::min(scale, boxHeight / measuredHeight);

    return std::clamp(scale, minimumScale, 1.0f);
}

float UI::Scaling::ContentScaleFromMetrics(float displayScale, float pixelDensity)
{
    if (!std::isfinite(displayScale) || !std::isfinite(pixelDensity) || displayScale <= 0.0f || pixelDensity <= 0.0f)
        return 1.0f;
    return displayScale / pixelDensity;
}

float UI::Scaling::GetWindowContentScale()
{
    return g_windowContentScale;
}

void UI::Scaling::SetWindowContentScale(float contentScale)
{
    g_windowContentScale = std::isfinite(contentScale) && contentScale > 0.0f ? contentScale : 1.0f;
}

UI::Scaling::Transform UI::Scaling::GetActiveTransform()
{
    return {g_fScreenRate_x, g_fScreenRate_y, g_fScreenOffset_x, g_fScreenOffset_y, g_fTypographyScale};
}

void UI::Scaling::SetActiveTransform(const Transform& transform)
{
    g_fScreenRate_x = transform.scaleX;
    g_fScreenRate_y = transform.scaleY;
    g_fScreenOffset_x = transform.offsetX;
    g_fScreenOffset_y = transform.offsetY;
    g_fTypographyScale = transform.typographyScale;
}

UI::Scaling::ScopedActiveTransform::ScopedActiveTransform(const Transform& transform, bool transformMouse)
    : m_previousTransform(GetActiveTransform()),
      m_previousMouseX(MouseX),
      m_previousMouseY(MouseY),
      m_restoreMouse(transformMouse)
{
    SetActiveTransform(transform);
    if (!m_restoreMouse)
        return;

    MouseX = static_cast<int>(std::floor(LogicalX(transform, g_fWindowMouseX)));
    MouseY = static_cast<int>(std::floor(LogicalY(transform, g_fWindowMouseY)));
}

UI::Scaling::ScopedActiveTransform::~ScopedActiveTransform()
{
    if (m_restoreMouse)
    {
        MouseX = m_previousMouseX;
        MouseY = m_previousMouseY;
    }
    SetActiveTransform(m_previousTransform);
}
