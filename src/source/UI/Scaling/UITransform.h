#pragma once

namespace UI::Scaling
{
    struct Transform
    {
        float scaleX;
        float scaleY;
        float offsetX;
        float offsetY;
        float typographyScale;
    };

    struct Viewport
    {
        int x;
        int y;
        int width;
        int height;
    };

    struct Position
    {
        float x;
        float y;
    };

    enum class FontRole
    {
        Normal,
        Bold,
        Big,
        Fixed,
    };

    enum class LayoutMode
    {
        Hud,
        DockLeft,
        DockRight,
        Dialog,
        WorldOverlay,
    };

    Transform ScreenOverlayTransform(int windowWidth, int windowHeight);
    Viewport FullReferenceViewport();
    Transform LegacyUiTransform(int windowWidth, int windowHeight);
    Transform PanelTransform(int windowWidth, int windowHeight);
    Transform DockLeftTransform(int windowWidth, int windowHeight);
    Transform DockRightTransform(int windowWidth, int windowHeight);
    float WorldViewportWidthForDock(float layoutWidth, int windowWidth, int windowHeight);
    Transform TransformForLayout(LayoutMode mode, int windowWidth, int windowHeight);
    float PositionX(const Transform& transform, float x);
    float PositionY(const Transform& transform, float y);
    float SizeX(const Transform& transform, float width);
    float SizeY(const Transform& transform, float height);
    float LogicalX(const Transform& transform, float windowX);
    float LogicalY(const Transform& transform, float windowY);
    Position CenteredLogicalPosition(const Transform& transform, float windowX, float windowY, float width,
                                     float height);
    int MinimumFontPointSize(FontRole role);
    int MaximumFontPointSize(FontRole role);
    int FontPointSize(FontRole role, const Transform& transform);
    float FontScaleForBounds(FontRole role, const Transform& transform, float measuredWidth, float measuredHeight,
                             float boxWidth, float boxHeight);
    Transform GetActiveTransform();
    void SetActiveTransform(const Transform& transform);
}
