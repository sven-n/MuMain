#pragma once

namespace UI::Scaling
{
    inline constexpr int DockLogicalBottom = 432;
    // Shared ceiling for "general" (non-HUD-band, non-dock) uniform auto-fit -- PanelTransform's
    // own cap, and RmlUiRuntime.cpp's dp-ratio auto-fit reuses the same number (single source of
    // truth: docs/rmlui-ui-system/layout-and-scaling.md's "Global UI scale" section).
    inline constexpr float MaximumPanelScale = 2.0f;

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
        HudLeft,
        HudCenter,
        HudRight,
        HudExperience,
        DockLeft,
        DockRight,
        FloatingWorkspace,
        Dialog,
        WorldOverlay,
        // CUIMng/CNewUIManager merger (docs/rmlui-ui-system) -- for a migrated window whose own
        // rendering (CSprite-based sprites, raw g_pRenderText calls) already computes real screen
        // pixels itself (its own fScaleX/fScaleY against whatever resolution it assumes, e.g.
        // CCreditWin's 800x600) rather than reference-space coordinates meant to be rescaled by
        // this transform system. Every other LayoutMode rescales against kReferenceWidth/Height
        // (640x480) -- applying any of them to this kind of window doubly (and wrongly) rescales
        // it, and (via CNewUIManager's UpdateMouseEvent() -- transformMouse=true) remaps the
        // global MouseX/MouseY into that same wrong reference space, breaking click hit-testing
        // too. TransformForLayout() maps this to a genuine identity transform instead.
        Legacy,
    };

    class ScopedActiveTransform
    {
    public:
        explicit ScopedActiveTransform(const Transform& transform, bool transformMouse = false);
        ~ScopedActiveTransform();
        ScopedActiveTransform(const ScopedActiveTransform&) = delete;
        ScopedActiveTransform& operator=(const ScopedActiveTransform&) = delete;

    private:
        Transform m_previousTransform;
        int m_previousMouseX;
        int m_previousMouseY;
        bool m_restoreMouse;
    };

    Transform ScreenOverlayTransform(int windowWidth, int windowHeight);
    Viewport FullReferenceViewport();
    Transform LegacyUiTransform(int windowWidth, int windowHeight);
    Transform PanelTransform(int windowWidth, int windowHeight);
    float ViewportFitScale(int windowWidth, int windowHeight, float maximumScale);
    float CompanionRatio(int windowWidth, int windowHeight);
    float BottomHudScale(int windowWidth, int windowHeight);
    Transform BottomHudLeftTransform(int windowWidth, int windowHeight);
    Transform BottomHudCenterTransform(int windowWidth, int windowHeight);
    Transform BottomHudRightTransform(int windowWidth, int windowHeight);
    Transform BottomHudExperienceTransform(int windowWidth, int windowHeight);
    Transform DockLeftTransform(int windowWidth, int windowHeight);
    Transform DockRightTransform(int windowWidth, int windowHeight);
    Transform FloatingWorkspaceTransform(int windowWidth, int windowHeight);
    Viewport FloatingWorkspaceBounds(int windowWidth, int windowHeight);
    float ScreenOverlayContentHeight(int windowWidth, int windowHeight);
    float FloatingWorkspaceContentHeight(int windowWidth, int windowHeight);
    Viewport WorldViewport(int windowWidth, int windowHeight, bool topViewEnabled);
    float WorldViewportAspect(int windowWidth, int windowHeight, bool topViewEnabled);
    bool BottomHudContainsWindowPoint(int windowWidth, int windowHeight, float windowX, float windowY);
    Transform TransformForLayout(LayoutMode mode, int windowWidth, int windowHeight);
    float PositionX(const Transform& transform, float x);
    float PositionY(const Transform& transform, float y);
    float SizeX(const Transform& transform, float width);
    float SizeY(const Transform& transform, float height);
    Viewport ViewportForLogicalRect(const Transform& transform, float x, float y, float width, float height);
    float LogicalX(const Transform& transform, float windowX);
    float LogicalY(const Transform& transform, float windowY);
    Position CenteredLogicalPosition(const Transform& transform, float windowX, float windowY, float width,
                                     float height);
    int MinimumFontPointSize(FontRole role);
    int MaximumFontPointSize(FontRole role);
    int CachedFontPointSize(FontRole role);
    int FontPointSize(FontRole role, const Transform& transform);
    float FontScaleForBounds(FontRole role, const Transform& transform, float measuredWidth, float measuredHeight,
                             float boxWidth, float boxHeight);
    float ContentScaleFromMetrics(float displayScale, float pixelDensity);
    float GetWindowContentScale();
    void SetWindowContentScale(float contentScale);
    Transform GetActiveTransform();
    void SetActiveTransform(const Transform& transform);
}
