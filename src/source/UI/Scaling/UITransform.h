#pragma once

namespace UI::Scaling
{
    inline constexpr int DockLogicalBottom = 432;
    // Shared ceiling for "general" (non-HUD-band, non-dock) uniform auto-fit -- PanelTransform's
    // own cap, and RmlUiRuntime.cpp's dp-ratio auto-fit reuses the same number (single source of
    // truth) for every RmlUi document's `dp` unit (i.e. every migrated panel's own text size).
    // Raising this does NOT fix an RmlUi dialog's text looking small: at a *reported* 1024x768
    // the raw fit is only 1.6, well under where this cap ever saturates, so the ceiling never
    // enters the computation. A dialog's text looking small relative to its own theme's sibling
    // windows at a shared resolution is a per-document `font-size` choice, not a global ceiling
    // problem -- don't reach for this constant again without a specific higher-resolution report
    // to test against.
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

    // Where a window's 640x480 reference units land on the screen, and at what scale. Every scale
    // but ScreenOverlay's and Pixels' is uniform and follows the UI scale.
    enum class LayoutMode
    {
        // The original screen centred on the whole window at the panel scale: NPC panels and
        // dialogs (the default).
        Stage,
        // The original screen at the bottom HUD's scale, centred like the HUD and standing on the
        // window's bottom: the fixed-place windows that stand on or open from the HUD.
        HudBoard,
        // A part placed by the workspace at the HUD's scale (the event HUDs, the bottom HUD's
        // parts, the chat): no offset, so it is drawn at the size its slot was given.
        HudFrame,
        // The docked panels, standing on the HUD at the dock scale; the workspace packs them.
        DockLeft,
        DockRight,
        // A window that keeps its own position, at the dock scale (the friend list).
        FloatingWorkspace,
        // The original screen stretched over the whole window (W/640 x H/480, no UI scale): what
        // must cover the screen or follow the world -- the notice band, the full map, names and
        // balloons over characters.
        ScreenOverlay,
        // Real screen pixels: a window whose own rendering already computes them (its own scale
        // against whatever resolution it assumes, e.g. CCreditWin's 800x600). Any other mode would
        // rescale it twice and remap MouseX/MouseY into the wrong space.
        Pixels,
        // Placed by the theme's workspace: the window's own transform maps it onto its slot
        // (CObject::PlaceInSlot()). TransformForLayout() cannot know it.
        Slot,
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
    // The one scale native text grows by, whatever a window's layout: the UI scale (the panel
    // scale, RmlUi's dp ratio). Every layout's transform carries it as its typographyScale.
    float TypographyScale(int windowWidth, int windowHeight);
    float ViewportFitScale(int windowWidth, int windowHeight, float maximumScale);
    float CompanionRatio(int windowWidth, int windowHeight);
    float BottomHudScale(int windowWidth, int windowHeight);
    Transform HudBoardTransform(int windowWidth, int windowHeight);
    Transform DockLeftTransform(int windowWidth, int windowHeight);
    Transform DockRightTransform(int windowWidth, int windowHeight);
    Transform FloatingWorkspaceTransform(int windowWidth, int windowHeight);
    Viewport FloatingWorkspaceBounds(int windowWidth, int windowHeight);
    // The whole window's height in ScreenOverlayTransform()'s units: a map's weather or a screen dim
    // covers the strip beside a HUD narrower than the window too, and the HUD draws over the rest.
    float ScreenOverlayFullHeight(int windowWidth, int windowHeight);
    float FloatingWorkspaceContentHeight(int windowWidth, int windowHeight);
    Viewport WorldViewport(int windowWidth, int windowHeight, bool topViewEnabled);
    float WorldViewportAspect(int windowWidth, int windowHeight, bool topViewEnabled);
    Transform TransformForLayout(LayoutMode mode, int windowWidth, int windowHeight);
    float PositionX(const Transform& transform, float x);
    float PositionY(const Transform& transform, float y);
    float SizeX(const Transform& transform, float width);
    float SizeY(const Transform& transform, float height);
    float LogicalX(const Transform& transform, float windowX);
    float LogicalY(const Transform& transform, float windowY);
    int MinimumFontPointSize(FontRole role);
    int MaximumFontPointSize(FontRole role);
    int CachedFontPointSize(FontRole role);
    int FontPointSize(FontRole role, const Transform& transform);
    // Physical pixel size the native text renderer draws `role` text at under `transform` -- what a
    // legacy-theme RmlUi text element must use to match it, independent of the panel's own scale.
    float NativeTextPixelSize(FontRole role, const Transform& transform);
    // The same at TypographyScale(), which every layout shares: no window's transform needed.
    float NativeTextPixelSize(FontRole role, int windowWidth, int windowHeight);
    // NativeTextPixelSize() for a text drawn into a box (RenderText() with a box width): the
    // renderer shrinks a text wider than its box to fit it, down to the role's minimum size.
    // `measuredWidth` is the text's unconstrained width and `boxWidth` the box's, both in the
    // transform's logical units (what MeasureText() returns).
    float NativeTextPixelSizeInBox(FontRole role, const Transform& transform, float measuredWidth, float boxWidth);
    // Physical pixel size of the renderer's smallest `role` text (MinimumFontPointSize()).
    float MinimumTextPixelSize(FontRole role);
    // The box rule behind NativeTextPixelSizeInBox() for any text size: `textPx` scaled by
    // boxWidth / measuredWidth when the text is wider than its box, but not below `minimumPx`
    // (nor above `textPx`). `measuredWidth` is the text's width at `textPx`, in the box's units.
    float FitTextPixelSizeToWidth(float textPx, float measuredWidth, float boxWidth, float minimumPx);
    // How much a window the original drew at fixed pixels (login form, server list, system menu,
    // login/character scene buttons) grows in the legacy theme: as much as the native dialog text
    // (NativeTextPixelSize(), LayoutMode::Stage) has grown against its size at 1024x768, never
    // below 1 -- the original's own size up to 1280x720, larger only where the text is larger.
    float SceneWindowScale(int windowWidth, int windowHeight);
    // The rule behind SceneWindowScale() for any text size: textPx / referenceTextPx, not below 1.
    float TextGrowthScale(float textPx, float referenceTextPx);
    // How the original scaled its character scene button bar (laid out for 800x600): by
    // min(W/800, H/600), clamped to [1, 2] (UI::CharacterSelection::CalculateLayout()).
    float SceneBarScale(int windowWidth, int windowHeight);
    // The same for a box with a height too (RenderText() with a box height the text is taller than,
    // e.g. the Devil Square rank headers' height of 3): the smaller of the two fits, down to the minimum.
    float NativeTextPixelSizeInBounds(FontRole role, const Transform& transform, float measuredWidth,
                                      float measuredHeight, float boxWidth, float boxHeight);
    float FontScaleForBounds(FontRole role, const Transform& transform, float measuredWidth, float measuredHeight,
                             float boxWidth, float boxHeight);
    float ContentScaleFromMetrics(float displayScale, float pixelDensity);
    float GetWindowContentScale();
    void SetWindowContentScale(float contentScale);
    Transform GetActiveTransform();
    void SetActiveTransform(const Transform& transform);
}
