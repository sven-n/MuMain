#include "UI/Scaling/UITransform.h"

#include <algorithm>
#include <cmath>

#include "App/stdafx.h"

namespace
{
constexpr int kReferenceWidth = 640;
constexpr int kReferenceHeight = 480;
constexpr float kMinimumPanelScale = 1.0f;
constexpr float kMaximumPanelScale = 1.5f;
constexpr int kNormalFontPointSize = 11;
constexpr int kMaximumNormalFontPointSize = 13;
constexpr int kBigFontPointSize = 22;
constexpr int kMaximumBigFontPointSize = 26;
constexpr int kFixedFontPointSize = 13;
constexpr int kMaximumFixedFontPointSize = 15;

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
    const float widthScale = static_cast<float>(windowWidth) / kReferenceWidth;
    const float heightScale = static_cast<float>(windowHeight) / kReferenceHeight;
    const float scale = std::clamp(std::min(widthScale, heightScale), kMinimumPanelScale, kMaximumPanelScale);
    return {
        scale,
        scale,
        (static_cast<float>(windowWidth) - kReferenceWidth * scale) * 0.5f,
        (static_cast<float>(windowHeight) - kReferenceHeight * scale) * 0.5f,
        scale,
    };
}

UI::Scaling::Transform UI::Scaling::DockLeftTransform(int windowWidth, int windowHeight)
{
    Transform transform = PanelTransform(windowWidth, windowHeight);
    transform.offsetX = 0.0f;
    return transform;
}

UI::Scaling::Transform UI::Scaling::DockRightTransform(int windowWidth, int windowHeight)
{
    Transform transform = PanelTransform(windowWidth, windowHeight);
    transform.offsetX = static_cast<float>(windowWidth) - kReferenceWidth * transform.scaleX;
    return transform;
}

float UI::Scaling::WorldViewportWidthForDock(float layoutWidth, int windowWidth, int windowHeight)
{
    const Transform screen = ScreenOverlayTransform(windowWidth, windowHeight);
    const Transform dock = DockRightTransform(windowWidth, windowHeight);
    return LogicalX(screen, PositionX(dock, layoutWidth));
}

UI::Scaling::Transform UI::Scaling::TransformForLayout(LayoutMode mode, int windowWidth, int windowHeight)
{
    if (mode == LayoutMode::Hud || mode == LayoutMode::WorldOverlay)
        return ScreenOverlayTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::DockLeft)
        return DockLeftTransform(windowWidth, windowHeight);
    if (mode == LayoutMode::DockRight)
        return DockRightTransform(windowWidth, windowHeight);
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

int UI::Scaling::FontPointSize(FontRole role, const Transform& transform)
{
    const FontPointRange range = GetFontPointRange(role);
    const float growth = std::clamp((transform.typographyScale - kMinimumPanelScale) /
                                        (kMaximumPanelScale - kMinimumPanelScale),
                                    0.0f, 1.0f);
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
