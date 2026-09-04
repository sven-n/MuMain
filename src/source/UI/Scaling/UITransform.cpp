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
constexpr float kHudContentTop = 429.0f;
constexpr float kExperienceTop = 470.0f;
constexpr float kLeftBandEnd = 152.0f;
constexpr float kCenterBandStart = 152.0f;
constexpr float kCenterBandEnd = 488.0f;
constexpr float kRightBandStart = 488.0f;
// ponytail: 2x HUD ceiling; raise only if native screenshots show unreadable controls.
constexpr float kMaximumHudScale = 2.0f;
// ponytail: 2x ceiling; raise only if native screenshots still show unreadable UI. Public copy of
// this value lives at UI::Scaling::MaximumPanelScale (UITransform.h) -- RmlUiRuntime.cpp's dp-ratio
// auto-fit reuses it, so it can't stay anonymous-namespace-only anymore.
// ponytail: 2.25x dock ceiling; adjust only from rebuilt native screenshots.
constexpr float kMaximumDockScale = 2.25f;
constexpr float kMaximumTypographyScale = 2.25f;
// ponytail: 2.0 = quadratic damping (a resolution halfway between reference and the ceiling scales
// up about 1/4 as far as a linear fit would, same endpoints preserved either way). Raise toward 1.0
// for less damping (1.0 = no damping, the original linear behavior), or above 2.0 for more, only
// from rebuilt native screenshots at a few real resolutions -- 2026-09-03, added after user feedback
// that a modest ~1024x768 window made every ViewportFitScale-driven dialog/HUD look noticeably
// larger than its authored size, not just "a little scaled." Deliberately doesn't touch
// kReferenceWidth/Height or any of the ceilings above -- same "no scaling" and "fully capped"
// endpoints as before, only the ramp in between changes.
constexpr float kFitDampingExponent = 2.0f;
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
// second cached global -- docs/rmlui-ui-system/layout-and-scaling.md's "Global UI scale" section.
// Applied post-clamp everywhere it's used (see BottomHudScale/CappedUniformScale below), unlike
// GetWindowContentScale()'s clamp-bound fold: a direct user dial needs a proportional, visible
// effect at every window size, including ones where the auto-scale already sits at its ceiling --
// folding it into the clamp bounds instead would silently defeat the setting there.
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

UI::Scaling::Transform BottomHudTransform(int windowWidth, int windowHeight, float offsetX)
{
    const float scale = UI::Scaling::BottomHudScale(windowWidth, windowHeight);
    return {scale, scale, offsetX, static_cast<float>(windowHeight) - kReferenceHeight * scale, scale};
}

bool ContainsLogicalRect(const UI::Scaling::Transform& transform, float windowX, float windowY, float left,
                         float top, float right, float bottom)
{
    const float x = UI::Scaling::LogicalX(transform, windowX);
    const float y = UI::Scaling::LogicalY(transform, windowY);
    return x >= left && x < right && y >= top && y < bottom;
}

int RoundedBottomHudTop(int windowWidth, int windowHeight)
{
    const float hudTop =
        static_cast<float>(windowHeight) - kHudFrameHeight * UI::Scaling::BottomHudScale(windowWidth, windowHeight);
    return std::max(static_cast<int>(std::lround(hudTop)), 1);
}

float CappedUniformScale(int windowWidth, int windowHeight, float maximumScale)
{
    return UI::Scaling::ViewportFitScale(windowWidth, windowHeight, maximumScale) * UIScalePercentMultiplier();
}

UI::Scaling::Transform DockTransform(int windowWidth, int windowHeight)
{
    const float scale = CappedUniformScale(windowWidth, windowHeight, kMaximumDockScale);
    const float offsetY = static_cast<float>(RoundedBottomHudTop(windowWidth, windowHeight))
                          - UI::Scaling::DockLogicalBottom * scale;
    return {scale, scale, 0.0f, offsetY, scale};
}
}

UI::Scaling::Transform UI::Scaling::ScreenOverlayTransform(int windowWidth, int windowHeight)
{
    const float scaleY = static_cast<float>(windowHeight) / kReferenceHeight;
    return {
        static_cast<float>(windowWidth) / kReferenceWidth,
        scaleY,
        0.0f,
        0.0f,
        scaleY,
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
        scale,
    };
}

// Pure geometry + WindowContentScale, deliberately NOT including UIScalePercent -- every caller
// that needs the user's preference multiplies UIScalePercentMultiplier() in itself, once, so it's
// never double-counted (docs/rmlui-ui-system/layout-and-scaling.md's "Global UI scale" section).
// The one shared "fit the reference size to the real window, clamped" core, now used by both the
// legacy UI::Scaling transforms (via CappedUniformScale/BottomHudScale below) and RmlUiRuntime.cpp's
// dp-ratio auto-fit -- previously two near-identical private copies of this same formula.
float UI::Scaling::ViewportFitScale(int windowWidth, int windowHeight, float maximumScale)
{
    const float widthScale = static_cast<float>(windowWidth) / kReferenceWidth;
    const float heightScale = static_cast<float>(windowHeight) / kReferenceHeight;
    const float contentScale = GetWindowContentScale();
    const float minBound = 1.0f * contentScale;
    const float maxBound = maximumScale * contentScale;
    const float raw = std::clamp(std::min(widthScale, heightScale), minBound, maxBound);

    // Dampen the ramp between the two endpoints (2026-09-03) -- raw itself already IS the answer
    // at the reference resolution (minBound, t=0) and at/past the ceiling (maxBound, t=1); only
    // resolutions strictly between the two get pulled down toward minBound, by kFitDampingExponent
    // (see its own comment). Reduces the reference/ceiling gap to a fraction [0,1] first
    // (`range` guards the degenerate case where they're equal, e.g. contentScale collapses both to
    // the same value), applies the curve, then remaps back -- so this stays purely a reshaping of
    // the existing formula's output, not a second independent scale factor.
    const float range = maxBound - minBound;
    if (range <= 0.0f)
        return raw;
    const float t = (raw - minBound) / range;
    return minBound + std::pow(t, kFitDampingExponent) * range;
}

// Combined ratio every legacy "Type-2 companion" object -- a real, functional non-RmlUi widget
// (a CUITextInputBox, or a CButton scaled to match a dp-sized RmlUi sibling) kept in sync with a
// migrated window's now-RCSS-owned layout -- must scale its own fixed reference-pixel offsets by,
// to stay pixel-for-pixel aligned with the RmlUi element it's shadowing. Same composition
// RmlUiRuntime.cpp's ApplyUIScale() uses for RmlUi's own dp ratio (UIScalePercent x
// ViewportFitScale(MaximumPanelScale)) -- extracted 2026-09-03 as the single shared implementation
// of a formula that had been hand-copied per window (CharSelMainWin.cpp's GetUIScaleRatio(),
// LoginMainWin.cpp's inline version, LoginWin.cpp's LoginUIScaleRatio()); the same staleness bug
// (reading CInput::Instance().GetScreenWidth()/GetScreenHeight() instead of the WindowWidth/
// WindowHeight globals RmlUiRuntime::OnResize() actually uses) got independently reintroduced and
// re-fixed in more than one of those copies before this existed. Callers must pass
// WindowWidth/WindowHeight (ZzzOpenglUtil.cpp), not a separate copy of the screen size --
// see docs/rmlui-ui-system/layout-and-scaling.md's "2026-09-03" section.
float UI::Scaling::CompanionRatio(int windowWidth, int windowHeight)
{
    return CappedUniformScale(windowWidth, windowHeight, MaximumPanelScale);
}

float UI::Scaling::BottomHudScale(int windowWidth, int windowHeight)
{
    return ViewportFitScale(windowWidth, windowHeight, kMaximumHudScale) * UIScalePercentMultiplier();
}

UI::Scaling::Transform UI::Scaling::BottomHudLeftTransform(int windowWidth, int windowHeight)
{
    return BottomHudTransform(windowWidth, windowHeight, 0.0f);
}

UI::Scaling::Transform UI::Scaling::BottomHudCenterTransform(int windowWidth, int windowHeight)
{
    const float scale = BottomHudScale(windowWidth, windowHeight);
    return BottomHudTransform(windowWidth, windowHeight,
                              static_cast<float>(windowWidth) * 0.5f - 320.0f * scale);
}

UI::Scaling::Transform UI::Scaling::BottomHudRightTransform(int windowWidth, int windowHeight)
{
    const float scale = BottomHudScale(windowWidth, windowHeight);
    return BottomHudTransform(windowWidth, windowHeight,
                              static_cast<float>(windowWidth) - kReferenceWidth * scale);
}

UI::Scaling::Transform UI::Scaling::BottomHudExperienceTransform(int windowWidth, int windowHeight)
{
    Transform transform = BottomHudLeftTransform(windowWidth, windowHeight);
    transform.scaleX = static_cast<float>(windowWidth) / kReferenceWidth;
    return transform;
}

UI::Scaling::Transform UI::Scaling::DockLeftTransform(int windowWidth, int windowHeight)
{
    Transform transform = DockTransform(windowWidth, windowHeight);
    transform.offsetX = 0.0f;
    return transform;
}

UI::Scaling::Transform UI::Scaling::DockRightTransform(int windowWidth, int windowHeight)
{
    Transform transform = DockTransform(windowWidth, windowHeight);
    transform.offsetX = static_cast<float>(windowWidth) - kReferenceWidth * transform.scaleX;
    return transform;
}

UI::Scaling::Transform UI::Scaling::FloatingWorkspaceTransform(int windowWidth, int windowHeight)
{
    const float scale = CappedUniformScale(windowWidth, windowHeight, kMaximumDockScale);
    return {scale, scale, 0.0f, 0.0f, scale};
}

UI::Scaling::Viewport UI::Scaling::FloatingWorkspaceBounds(int windowWidth, int windowHeight)
{
    const Transform transform = FloatingWorkspaceTransform(windowWidth, windowHeight);
    return {
        0,
        0,
        std::max(static_cast<int>(std::floor(static_cast<float>(windowWidth) / transform.scaleX)), 1),
        std::max(static_cast<int>(std::floor(static_cast<float>(windowHeight) / transform.scaleY)), 1),
    };
}

float UI::Scaling::ScreenOverlayContentHeight(int windowWidth, int windowHeight)
{
    const Transform transform = ScreenOverlayTransform(windowWidth, windowHeight);
    const float physicalHeight =
        static_cast<float>(windowHeight) - kHudFrameHeight * BottomHudScale(windowWidth, windowHeight);
    return physicalHeight / transform.scaleY;
}

float UI::Scaling::FloatingWorkspaceContentHeight(int windowWidth, int windowHeight)
{
    const Transform transform = FloatingWorkspaceTransform(windowWidth, windowHeight);
    const float physicalHeight =
        static_cast<float>(windowHeight) - kHudFrameHeight * BottomHudScale(windowWidth, windowHeight);
    return physicalHeight / transform.scaleY;
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

bool UI::Scaling::BottomHudContainsWindowPoint(int windowWidth, int windowHeight, float windowX, float windowY)
{
    const Transform experience = BottomHudExperienceTransform(windowWidth, windowHeight);
    if (ContainsLogicalRect(experience, windowX, windowY, 0.0f, kExperienceTop,
                            static_cast<float>(kReferenceWidth), static_cast<float>(kReferenceHeight)))
        return true;

    const Transform left = BottomHudLeftTransform(windowWidth, windowHeight);
    if (ContainsLogicalRect(left, windowX, windowY, 0.0f, kHudContentTop, kLeftBandEnd,
                            static_cast<float>(kReferenceHeight)))
        return true;

    const Transform center = BottomHudCenterTransform(windowWidth, windowHeight);
    if (ContainsLogicalRect(center, windowX, windowY, kCenterBandStart, kHudContentTop, kCenterBandEnd,
                            static_cast<float>(kReferenceHeight)))
        return true;

    const Transform right = BottomHudRightTransform(windowWidth, windowHeight);
    return ContainsLogicalRect(right, windowX, windowY, kRightBandStart, kHudContentTop,
                               static_cast<float>(kReferenceWidth), static_cast<float>(kReferenceHeight));
}

UI::Scaling::Transform UI::Scaling::TransformForLayout(LayoutMode mode, int windowWidth, int windowHeight)
{
    if (mode == LayoutMode::Legacy)
        return {1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
    if (mode == LayoutMode::Hud || mode == LayoutMode::WorldOverlay)
        return ScreenOverlayTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::HudLeft)
        return BottomHudLeftTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::HudCenter)
        return BottomHudCenterTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::HudRight)
        return BottomHudRightTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::HudExperience)
        return BottomHudExperienceTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::DockLeft)
        return DockLeftTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::DockRight)
        return DockRightTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::FloatingWorkspace)
        return FloatingWorkspaceTransform(windowWidth, windowHeight);
    return PanelTransform(windowWidth, windowHeight);
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

UI::Scaling::Viewport UI::Scaling::ViewportForLogicalRect(const Transform& transform, float x, float y, float width,
                                                          float height)
{
    return {
        static_cast<int>(std::lround(PositionX(transform, x))),
        static_cast<int>(std::lround(PositionY(transform, y))),
        std::max(static_cast<int>(std::lround(SizeX(transform, width))), 1),
        std::max(static_cast<int>(std::lround(SizeY(transform, height))), 1),
    };
}

float UI::Scaling::LogicalX(const Transform& transform, float windowX)
{
    return (windowX - transform.offsetX) / transform.scaleX;
}

float UI::Scaling::LogicalY(const Transform& transform, float windowY)
{
    return (windowY - transform.offsetY) / transform.scaleY;
}

UI::Scaling::Position UI::Scaling::CenteredLogicalPosition(const Transform& transform, float windowX, float windowY,
                                                            float width, float height)
{
    return {LogicalX(transform, windowX) - width * 0.5f, LogicalY(transform, windowY) - height * 0.5f};
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
