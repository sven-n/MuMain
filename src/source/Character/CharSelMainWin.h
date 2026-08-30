//*****************************************************************************
// File: CharSelMainWin.h
//*****************************************************************************

#pragma once

#include <algorithm>
#include <array>
#include <cmath>

#include "UI/Widgets/Win.h"
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
}

namespace Rml { class ElementDocument; }

// RmlUi migration: character-select scene, following CLoginMainWin/CSysMenuWin's established
// hybrid pattern. CWin::Create() now passes nTexID=-2 (was the default -1) -- RmlUi renders 100%
// of this bar's visuals (buttons, the info-bar background, the decorative flourish, and the rare
// account-block message) in every theme; the legacy CSprites/CButtons stay alive purely as
// bookkeeping (button click-detection redundancy, geometry used by ApplyLayout's math), never
// rendered. See docs/rmlui-ui-system/README.md for the shared architecture this follows.
//
// Rebased onto upstream's responsive-scaling work (UI::CharacterSelection::CalculateLayout()
// above): ApplyLayout() is now the single source of truth for every element's screen rect
// (position AND size -- buttons/deco/info-bar can scale up to 2x on larger screens, not just
// move), pushed to both the legacy CButton/CSprite bookkeeping and the RmlUi elements together.
class CCharSelMainWin : public CWin
{
protected:
    CSprite m_asprBack[CSMW_SPR_MAX];
    CButton m_aBtn[CSMW_BTN_MAX];
    bool m_bAccountBlockItem;

public:
    CCharSelMainWin();
    virtual ~CCharSelMainWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    void UpdateDisplay();

    // Invoked from the RmlUi document's data-event-click bindings (see Create()). Polled-and-
    // cleared exactly like the legacy CButton::IsClick() edge triggers they supplement in
    // UpdateWhileActive() -- mirrors CSysMenuWin's RmlClickX() shape, including the same-guard
    // gating for buttons that can be genuinely disabled (Create/Connect/Delete, not Menu).
    void RmlClickCreate() { if (m_bCreateEnabled) m_bRmlCreateClicked = true; }
    void RmlClickMenu() { m_bRmlMenuClicked = true; }
    void RmlClickConnect() { if (m_bConnectEnabled) m_bRmlConnectClicked = true; }
    void RmlClickDelete() { if (m_bDeleteEnabled) m_bRmlDeleteClicked = true; }

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();
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
