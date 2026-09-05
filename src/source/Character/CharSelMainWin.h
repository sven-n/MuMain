//*****************************************************************************
// File: CharSelMainWin.h
//*****************************************************************************

#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/Button.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#define CSMW_SPR_DECO 0
#define CSMW_SPR_INFO 1
#define CSMW_SPR_MAX 2

#define CSMW_BTN_CREATE 0
#define CSMW_BTN_MENU 1
#define CSMW_BTN_CONNECT 2
#define CSMW_BTN_DELETE 3
#define CSMW_BTN_MAX 4

namespace UI::CharacterSelection
{
    inline constexpr float NativeWidth = 800.0f;
    inline constexpr float NativeHeight = 600.0f;
    inline constexpr float NativeAnchorY = 567.0f;
    inline constexpr int NativeOuterMargin = 22;
    inline constexpr int NativeButtonWidth = 54;
    inline constexpr int NativeButtonHeight = 30;
    inline constexpr int NativeButtonGap = 1;
    inline constexpr int NativeInformationGap = 2;
    inline constexpr int NativeInformationTop = 5;
    inline constexpr int NativeInformationHeight = 21;
    inline constexpr int NativeDecorationWidth = 189;
    inline constexpr int NativeDecorationHeight = 103;
    inline constexpr int NativeDecorationWindowOutset = 22;
    inline constexpr int NativeDecorationTopOffset = 59;
    inline constexpr int NativeBottomGap = 11;

    struct Rect
    {
        int x;
        int y;
        int width;
        int height;
    };

    struct Layout
    {
        float scale;
        Rect window;
        Rect decoration;
        Rect information;
        std::array<Rect, CSMW_BTN_MAX> buttons;
    };

    inline int Scale(int value, float scale)
    {
        return static_cast<int>(std::lround(value * scale));
    }

    inline std::array<Rect, CSMW_BTN_MAX> CalculateButtons(int screenWidth, int outerMargin, int windowTop,
                                                           int buttonWidth, int buttonHeight, int buttonGap)
    {
        std::array<Rect, CSMW_BTN_MAX> buttons{};
        buttons[CSMW_BTN_CREATE] = {outerMargin, windowTop, buttonWidth, buttonHeight};
        buttons[CSMW_BTN_MENU] = {outerMargin + buttonWidth + buttonGap, windowTop, buttonWidth, buttonHeight};
        buttons[CSMW_BTN_DELETE] = {screenWidth - outerMargin - buttonWidth, windowTop, buttonWidth, buttonHeight};
        buttons[CSMW_BTN_CONNECT] = {
            buttons[CSMW_BTN_DELETE].x - buttonGap - buttonWidth, windowTop, buttonWidth, buttonHeight};
        return buttons;
    }

    inline Layout CalculateLayout(int screenWidth, int screenHeight)
    {
        const float scale = std::clamp(
            std::min(static_cast<float>(screenWidth) / NativeWidth,
                     static_cast<float>(screenHeight) / NativeHeight),
            1.0f, 2.0f);
        const int outerMargin = Scale(NativeOuterMargin, scale);
        const int buttonWidth = Scale(NativeButtonWidth, scale);
        const int buttonHeight = Scale(NativeButtonHeight, scale);
        const int buttonGap = Scale(NativeButtonGap, scale);
        const int informationGap = Scale(NativeInformationGap, scale);
        const int anchorY = static_cast<int>(std::lround(NativeAnchorY / NativeHeight * screenHeight));
        const int windowTop = anchorY - buttonHeight - Scale(NativeBottomGap, scale);
        const int windowWidth = std::max(0, screenWidth - 2 * outerMargin);

        Layout layout{};
        layout.scale = scale;
        layout.window = {outerMargin, windowTop, windowWidth, buttonHeight};
        layout.buttons = CalculateButtons(
            screenWidth, outerMargin, windowTop, buttonWidth, buttonHeight, buttonGap);

        const int informationX = layout.buttons[CSMW_BTN_MENU].x + buttonWidth + informationGap;
        const int informationRight = layout.buttons[CSMW_BTN_CONNECT].x - informationGap;
        layout.information = {informationX, windowTop + Scale(NativeInformationTop, scale),
                              std::max(0, informationRight - informationX),
                              Scale(NativeInformationHeight, scale)};

        const int decorationWidth = Scale(NativeDecorationWidth, scale);
        const int windowRight = layout.window.x + layout.window.width;
        layout.decoration = {windowRight - decorationWidth + Scale(NativeDecorationWindowOutset, scale),
                             windowTop - Scale(NativeDecorationTopOffset, scale), decorationWidth,
                             Scale(NativeDecorationHeight, scale)};
        return layout;
    }

    // 133dp inset used by char_sel_main.rcss's .info-bar (both themes) -- see that file's own
    // comment for the derivation (NativeOuterMargin+NativeButtonWidth+NativeButtonGap+
    // NativeButtonWidth+NativeInformationGap). Named here so CalculateFixedAnchorLayout() below
    // and the RCSS stay derived from the same source constants instead of a second hard-coded 133.
    inline constexpr int NativeInformationInset =
        NativeOuterMargin + NativeButtonWidth + NativeButtonGap + NativeButtonWidth + NativeInformationGap;
    // .info-bar's bottom:15dp and .deco's bottom:-3dp in char_sel_main.rcss.
    inline constexpr int NativeInformationBottomGap = 15;
    inline constexpr int NativeDecorationBottomOffset = 3;

    // Fixed, dp-anchored counterpart to CalculateLayout() above, mirroring char_sel_main.rcss's
    // 2026-08-31 layout-and-scaling retrofit exactly (docs/rmlui-ui-system/layout-and-scaling.md).
    // CalculateLayout() (upstream's auto-scale-to-fit-800x600 system) is no longer used by this
    // window: once the RmlUi visuals switched from mirroring that resolution-proportional rect to
    // fixed-dp/anchor-class CSS positioning, continuing to position the legacy CButton/CSprite
    // hit-test objects via CalculateLayout()'s math left them visually detached from the buttons
    // actually on screen at any resolution other than a coincidental match -- a real, confirmed
    // bug, not a cosmetic one: this window's own bounding rect (below) must cover every element
    // positioned above, or hovering the info-bar/deco alone would wrongly fall through to the
    // legacy world-click handler (previously via CUIMng::IsCursorOnUI(), now via
    // UpdateMouseEvent()'s own rect claim -- see CServerSelWin's established pattern
    // (docs/newui-legacy-merger.md) this window's own migration follows). uiScale must be the same
    // combined ratio RmlUi's own `dp` unit uses (Rml::Context::SetDensityIndependentPixelRatio(),
    // RmlUiRuntime.cpp's ApplyUIScale()) -- UI::Scaling::CompanionRatio(screenWidth, screenHeight)
    // (UITransform.cpp) computes it (UIScalePercent times UI::Scaling::ViewportFitScale(), not
    // UIScalePercent alone since 2026-09-03), so these rects always match the RmlUi buttons
    // pixel-for-pixel regardless of screen resolution or UI-scale setting.
    inline Layout CalculateFixedAnchorLayout(int screenWidth, int screenHeight, float uiScale)
    {
        const int buttonWidth = Scale(NativeButtonWidth, uiScale);
        const int buttonHeight = Scale(NativeButtonHeight, uiScale);
        const int buttonGap = Scale(NativeButtonGap, uiScale);
        const int outerMargin = Scale(NativeOuterMargin, uiScale);
        const int bottomGap = Scale(NativeBottomGap, uiScale);
        const int windowTop = screenHeight - bottomGap - buttonHeight;

        Layout layout{};
        layout.scale = uiScale;
        layout.buttons = CalculateButtons(screenWidth, outerMargin, windowTop, buttonWidth, buttonHeight, buttonGap);

        const int infoInset = Scale(NativeInformationInset, uiScale);
        const int infoHeight = Scale(NativeInformationHeight, uiScale);
        const int infoBottomGap = Scale(NativeInformationBottomGap, uiScale);
        layout.information = {infoInset, screenHeight - infoBottomGap - infoHeight,
                              std::max(0, screenWidth - 2 * infoInset), infoHeight};

        const int decorationWidth = Scale(NativeDecorationWidth, uiScale);
        const int decorationHeight = Scale(NativeDecorationHeight, uiScale);
        const int decorationBottomOffset = Scale(NativeDecorationBottomOffset, uiScale);
        layout.decoration = {screenWidth - decorationWidth, screenHeight + decorationBottomOffset - decorationHeight,
                             decorationWidth, decorationHeight};

        // Overall bounding box -- UpdateMouseEvent() below treats any cursor position inside this
        // rect as "on this window's UI", so it must cover every element positioned above, not just
        // the buttons, or hovering the info-bar/deco alone would wrongly fall through to the world.
        const int top = std::min({layout.buttons[CSMW_BTN_CREATE].y, layout.decoration.y, layout.information.y});
        layout.window = {0, top, screenWidth, screenHeight - top};
        return layout;
    }
}

namespace Rml { class ElementDocument; }

// RmlUi migration: character-select scene, following CLoginMainWin/CSysMenuWin's established
// hybrid pattern. RmlUi renders 100% of this bar's visuals (buttons, the info-bar background, the
// decorative flourish, and the rare account-block message) in every theme; the legacy
// CSprites/CButtons stay alive purely as bookkeeping (button click-detection redundancy,
// UpdateMouseEvent()'s own rect-hit-testing), never rendered. See docs/rmlui-ui-system/README.md
// for the shared architecture this follows.
//
// 2026-08-31 layout-and-scaling retrofit (docs/rmlui-ui-system/layout-and-scaling.md): the RmlUi
// visuals position themselves via char_sel_main.rcss's fixed-dp/anchor-class rules, NOT via
// ApplyLayout() pushing a computed rect anymore. ApplyLayout() still runs, but only feeds the
// legacy CButton/CSprite bookkeeping objects, and does so via
// UI::CharacterSelection::CalculateFixedAnchorLayout() (mirroring the RCSS's own fixed-dp math)
// rather than the older CalculateLayout() (upstream's auto-scale-to-fit-800x600 system, kept above
// for reference/potential reuse elsewhere but no longer used by this window) -- see
// CalculateFixedAnchorLayout()'s own comment for why using the mismatched old math was a real,
// user-visible bug (Delete silently no-op'ing) and not just a style inconsistency.
//
// CUIMng/CNewUIManager merger (docs/newui-legacy-merger.md) Phase 2: migrated off CWin onto
// mu::ui::window::CObject. Not modal -- UpdateMouseEvent() claims only within its own bounding rect
// (CServerSelWin's established pattern), not the whole screen, since the world behind this bar
// must stay clickable/rotatable. Its own Update() additionally skips all button-click processing
// while CCharMakeWin/CMsgWin/CSysMenuWin is shown (see their own GetLayerDepth() comments) --
// this is the actual fix for a real, reported bug: the legacy CWin::m_bActive gate this used to
// rely on for that exact purpose doesn't reliably deactivate on a timely basis (same class of
// issue CLoginMainWin/CSysMenuWin's own "act immediately" RmlClick*() methods already document),
// letting the Menu button fire while CCharMakeWin was still open.
class CCharSelMainWin : public mu::ui::window::CObject
{
protected:
    CSprite m_asprBack[CSMW_SPR_MAX];
    CButton m_aBtn[CSMW_BTN_MAX];
    bool m_bAccountBlockItem;

    // Replaces CWin::m_ptPos/m_Size -- no shared rect facility on the CObject side (matching
    // every pre-existing CObject window), so this window keeps its own bounding box, same as
    // CServerSelWin's established pattern.
    POINT m_ptPos;
    SIZE m_Size;

public:
    CCharSelMainWin();
    ~CCharSelMainWin() override;

    void Create();
    void Release(); // was CWin::PreRelease() (an override hook CWin::Release() called
                     // automatically) -- called explicitly now, same as CCreditWin's own Release().
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow) override;
    void UpdateDisplay();

    // Invoked from the RmlUi document's data-event-click bindings (see Create()). Polled-and-
    // cleared exactly like the legacy CButton::IsClick() edge triggers they supplement in
    // Update() -- mirrors CSysMenuWin's RmlClickX() shape, including the same-guard gating for
    // buttons that can be genuinely disabled (Create/Connect/Delete, not Menu).
    void RmlClickCreate() { if (m_bCreateEnabled) m_bRmlCreateClicked = true; }
    void RmlClickMenu() { m_bRmlMenuClicked = true; }
    void RmlClickConnect() { if (m_bConnectEnabled) m_bRmlConnectClicked = true; }
    void RmlClickDelete() { if (m_bDeleteEnabled) m_bRmlDeleteClicked = true; }

    // mu::ui::window::IObject
    bool Render() override;
    bool Update() override;
    // Was CWin::CursorInWin(WA_ALL) -- claims any click within its own bounding box, ported from
    // CServerSelWin's established pattern. Not modal: the world behind this bar stays reachable.
    bool UpdateMouseEvent() override;
    bool UpdateKeyEvent() override
    {
        return true;
    }
    float GetLayerDepth() override
    {
        return 15.0f;
    }

protected:
    void DeleteCharacter();

private:
    void ApplyLayout(const UI::CharacterSelection::Layout& layout);

    struct CharSelMainRmlModel
    {
        bool createDisabled = false;
        bool connectDisabled = false;
        bool deleteDisabled = false;
        bool accountBlockHidden = true;
        Rml::String accountBlockLine1;
        Rml::String accountBlockLine2;
    };
    RmlModelBinder<CharSelMainRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

    bool m_bCreateEnabled = false;
    bool m_bConnectEnabled = false;
    bool m_bDeleteEnabled = false;

    bool m_bRmlCreateClicked = false;
    bool m_bRmlMenuClicked = false;
    bool m_bRmlConnectClicked = false;
    bool m_bRmlDeleteClicked = false;

    void SyncRmlModel();
};

// Replaces CUIMng's old `CCharSelMainWin m_CharSelMainWin;` member, same convention as
// g_CreditWin.
extern CCharSelMainWin g_CharSelMainWin;
