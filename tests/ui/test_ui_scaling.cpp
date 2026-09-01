#include "App/stdafx.h"

#include <doctest.h>

#include "Character/CharSelMainWin.h"
#include "Core/Input/Input.h"
#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_enum.h"
#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"
#include "Engine/Object/ZzzInventory.h"
#include "UI/Legacy/UIControls.h"
#include "UI/Legacy/UIMapName.h"
#include "UI/NewUI/Dialogs/NewUIChatCommandWindow.h"
#include "UI/NewUI/HUD/NewUICommandWindow.h"
#include "UI/NewUI/HUD/NewUIMoveCommandWindow.h"
#include "UI/NewUI/Inventory/NewUIInventoryCtrl.h"
#include "UI/NewUI/NewUI3DRenderMng.h"
#include "UI/NewUI/NewUIManager.h"
#include "UI/NewUI/NPCs/NewUINPCShop.h"
#include "UI/NewUI/Options/NewUIOptionWindow.h"
#include "UI/NewUI/UILayoutPolicy.h"
#include "UI/Scaling/UITransform.h"
#include "UI/Widgets/Button.h"

using UI::Scaling::FontRole;
using SEASON3B::CNewUICommandWindow;

namespace
{
class Recording3DObject final : public SEASON3B::INewUI3DRenderObj
{
public:
    explicit Recording3DObject(SEASON3B::CNewUIObj* owner)
        : m_owner(owner)
    {
    }

    void Render3D() override
    {
        mouseX = MouseX;
        mouseY = MouseY;
    }

    bool IsVisible() const override { return true; }
    SEASON3B::CNewUIObj* GetLayoutOwner() const override { return m_owner; }

    int mouseX = -1;
    int mouseY = -1;

private:
    SEASON3B::CNewUIObj* m_owner;
};

class Test3DCamera final : public SEASON3B::CNewUI3DCamera
{
public:
    using CNewUI3DCamera::Render3D;
};

class RecordingUIObject final : public SEASON3B::CNewUIObj
{
public:
    bool Render() override
    {
        renderMouseX = MouseX;
        renderMouseY = MouseY;
        return true;
    }

    bool Update() override { return true; }
    bool UpdateMouseEvent() override { return true; }
    bool UpdateKeyEvent() override { return true; }
    float GetLayerDepth() override { return 1.0f; }

    int renderMouseX = -1;
    int renderMouseY = -1;
};
}

TEST_CASE("teleport layout uses stable width and fits above the dock [ui][scaling]")
{
    const auto layout = UI::MoveCommand::CalculateLayout(1, 14);
    CHECK(layout.windowWidth == 230);
    CHECK(layout.visibleRows == 26);
    CHECK(layout.windowHeight == 424);
    CHECK(layout.listTop == 39);
    CHECK(layout.closeTop == 405);
    CHECK(layout.closeLeft == 2);
    CHECK(layout.closeWidth == 225);
    CHECK(layout.scrollTrackTop == 36);
    CHECK(layout.scrollTrackHeight == 364);
    CHECK(layout.thumbTravel == 334);
    CHECK(1 + layout.windowHeight <= UI::Scaling::DockLogicalBottom);
}

TEST_CASE("teleport width is independent of measured row height [ui][scaling]")
{
    CHECK(UI::MoveCommand::CalculateLayout(1, 12).windowWidth == 230);
    CHECK(UI::MoveCommand::CalculateLayout(1, 14).windowWidth == 230);
    CHECK(UI::MoveCommand::CalculateLayout(1, 18).windowWidth == 230);
}

TEST_CASE("teleport scroll offset handles empty short exact and overflow lists [ui][scaling]")
{
    CHECK(UI::MoveCommand::MaximumScrollOffset(0, 26) == 0);
    CHECK(UI::MoveCommand::MaximumScrollOffset(12, 26) == 0);
    CHECK(UI::MoveCommand::MaximumScrollOffset(26, 26) == 0);
    CHECK(UI::MoveCommand::MaximumScrollOffset(27, 26) == 1);
    CHECK(UI::MoveCommand::MaximumScrollOffset(52, 26) == 26);

    CHECK(UI::MoveCommand::ClampScrollOffset(-5, 52, 26) == 0);
    CHECK(UI::MoveCommand::ClampScrollOffset(13, 52, 26) == 13);
    CHECK(UI::MoveCommand::ClampScrollOffset(99, 52, 26) == 26);
}

TEST_CASE("teleport thumb maps first middle and last offsets [ui][scaling]")
{
    const auto layout = UI::MoveCommand::CalculateLayout(1, 14);
    CHECK(UI::MoveCommand::ThumbYForScrollOffset(0, layout, 52) == 36);
    CHECK(UI::MoveCommand::ThumbYForScrollOffset(13, layout, 52) == 203);
    CHECK(UI::MoveCommand::ThumbYForScrollOffset(26, layout, 52) == 370);

    CHECK(UI::MoveCommand::ScrollOffsetForThumbY(36, layout, 52) == 0);
    CHECK(UI::MoveCommand::ScrollOffsetForThumbY(203, layout, 52) == 13);
    CHECK(UI::MoveCommand::ScrollOffsetForThumbY(370, layout, 52) == 26);
}

TEST_CASE("teleport thumb disables cleanly without overflow [ui][scaling]")
{
    const auto layout = UI::MoveCommand::CalculateLayout(1, 14);
    CHECK(UI::MoveCommand::ThumbYForScrollOffset(9, layout, 0) == layout.scrollTrackTop);
    CHECK(UI::MoveCommand::ScrollOffsetForThumbY(layout.scrollTrackTop + 50, layout, 0) == 0);
}

TEST_CASE("teleport drag release maps, exits, and consumes input [ui][scaling]")
{
    const auto layout = UI::MoveCommand::CalculateLayout(1, 14);
    const auto state = UI::MoveCommand::UpdateDragState(true, true, layout.scrollTrackTop + 999, 0, 13, layout, 52);
    CHECK(state.scrollOffset == 26);
    CHECK_FALSE(state.dragging);
    CHECK(state.releaseConsumed);
}

TEST_CASE("dialogs scale with the viewport and stop at a readable cap [ui][scaling]")
{
    const auto hd = UI::Scaling::PanelTransform(1280, 720);
    CHECK(hd.scaleX == doctest::Approx(1.5f));
    CHECK(hd.scaleY == doctest::Approx(1.5f));
    CHECK(hd.offsetX == doctest::Approx(160.0f));
    CHECK(hd.offsetY == doctest::Approx(0.0f));

    const auto fullHd = UI::Scaling::PanelTransform(1920, 1080);
    CHECK(fullHd.scaleX == doctest::Approx(2.0f));
    CHECK(fullHd.offsetX == doctest::Approx(320.0f));
    CHECK(fullHd.offsetY == doctest::Approx(60.0f));

    const auto fourK = UI::Scaling::PanelTransform(3840, 2160);
    CHECK(fourK.scaleX == doctest::Approx(2.0f));
    CHECK(fourK.offsetX == doctest::Approx(1280.0f));
    CHECK(fourK.offsetY == doctest::Approx(600.0f));
}

TEST_CASE("HUD fills the viewport while dialogs stay capped [ui][scaling]")
{
    const auto hud = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Hud, 1920, 1080);
    CHECK(UI::Scaling::PositionX(hud, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::PositionY(hud, 480.0f) == doctest::Approx(1080.0f));

    const auto dialog = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Dialog, 1920, 1080);
    CHECK(dialog.scaleX == doctest::Approx(2.0f));
    CHECK(dialog.offsetX == doctest::Approx(320.0f));
    CHECK(dialog.offsetY == doctest::Approx(60.0f));
}

TEST_CASE("docks use a moderate large-screen cap without changing dialogs [ui][scaling]")
{
    const auto smallDock = UI::Scaling::DockLeftTransform(1280, 720);
    CHECK(smallDock.scaleX == doctest::Approx(1.5f));
    CHECK(smallDock.scaleY == doctest::Approx(1.5f));

    const auto fullHdDock = UI::Scaling::DockLeftTransform(1920, 1080);
    CHECK(fullHdDock.scaleX == doctest::Approx(2.25f));
    CHECK(fullHdDock.scaleY == doctest::Approx(2.25f));
    CHECK(UI::Scaling::PositionY(fullHdDock, UI::Scaling::DockLogicalBottom) == doctest::Approx(978.0f));

    const auto fourKDock = UI::Scaling::DockLeftTransform(3840, 2160);
    CHECK(fourKDock.scaleX == doctest::Approx(2.25f));
    CHECK(fourKDock.scaleY == doctest::Approx(2.25f));

    const auto fourKDialog = UI::Scaling::PanelTransform(3840, 2160);
    CHECK(fourKDialog.scaleX == doctest::Approx(2.0f));
    CHECK(fourKDialog.scaleY == doctest::Approx(2.0f));
}

TEST_CASE("inventory drag keeps the clicked point anchored to the item [ui][inventory]")
{
    const POINT offset = UI::Items::Drag::PickupOffset(180, 260, 40, 60, 183, 317, true);
    CHECK(offset.x == 3);
    CHECK(offset.y == 57);

    const POINT itemTopLeft = UI::Items::Drag::ItemTopLeft(183, 317, offset);
    CHECK(itemTopLeft.x == 180);
    CHECK(itemTopLeft.y == 260);
}

TEST_CASE("inventory item hover animation ignores world input capture [ui][inventory]")
{
    CHECK(UI::Items::ShouldAnimatePreview(true, false, false));
    CHECK(UI::Items::ShouldAnimatePreview(true, true, true));
    CHECK_FALSE(UI::Items::ShouldAnimatePreview(true, true, false));
    CHECK_FALSE(UI::Items::ShouldAnimatePreview(false, false, false));
}

TEST_CASE("display resolution options use unique supported sizes [ui][options]")
{
    std::vector<UI::Options::DisplayResolution> modes = {
        {1920, 1080}, {1280, 720}, {1920, 1080}, {0, 1080}, {3840, 2160},
    };

    const auto resolutions = UI::Options::NormalizeDisplayResolutions(std::move(modes));

    REQUIRE(resolutions.size() == 3);
    CHECK(resolutions[0] == UI::Options::DisplayResolution(1280, 720));
    CHECK(resolutions[1] == UI::Options::DisplayResolution(1920, 1080));
    CHECK(resolutions[2] == UI::Options::DisplayResolution(3840, 2160));
    CHECK(UI::Options::FindExactDisplayResolutionIndex(resolutions, 1920, 1080) == 1);
    CHECK(UI::Options::FindExactDisplayResolutionIndex(resolutions, 1600, 900) == -1);
    CHECK(UI::Options::FindClosestDisplayResolutionIndex(resolutions, 1366, 768) == 0);
}

TEST_CASE("VSync preference defaults on and remains mutable [config][render]")
{
    CHECK(CfgDefaults::CfgDefaultVSync);

    auto& config = GameConfig::GetInstance();
    const bool previous = config.GetVSyncEnabled();

    config.SetVSyncEnabled(false);
    CHECK_FALSE(config.GetVSyncEnabled());
    config.SetVSyncEnabled(true);
    CHECK(config.GetVSyncEnabled());

    config.SetVSyncEnabled(previous);
}

TEST_CASE("inventory drag centers items without a grid pickup anchor [ui][inventory]")
{
    const POINT offset = UI::Items::Drag::PickupOffset(0, 0, 40, 60, 183, 317, false);
    CHECK(offset.x == 20);
    CHECK(offset.y == 30);
}

TEST_CASE("inventory drag keeps border drops in their original slots [ui][inventory]")
{
    constexpr int gridLeft = 100;
    constexpr int gridTop = 200;

    const POINT leftOffset = UI::Items::Drag::PickupOffset(gridLeft, gridTop, 40, 40, 101, 201, true);
    const POINT leftTopLeft = UI::Items::Drag::ItemTopLeft(101, 201, leftOffset);
    CHECK(leftTopLeft.x == gridLeft);
    CHECK(leftTopLeft.y == gridTop);
    CHECK((leftTopLeft.x - gridLeft) / SEASON3B::INVENTORY_SQUARE_WIDTH == 0);
    CHECK((leftTopLeft.y - gridTop) / SEASON3B::INVENTORY_SQUARE_HEIGHT == 0);

    constexpr int rightItemLeft = gridLeft + 6 * SEASON3B::INVENTORY_SQUARE_WIDTH;
    const POINT rightOffset = UI::Items::Drag::PickupOffset(rightItemLeft, gridTop, 40, 40,
                                                            rightItemLeft + 39, gridTop + 39, true);
    const POINT rightTopLeft = UI::Items::Drag::ItemTopLeft(rightItemLeft + 39, gridTop + 39, rightOffset);
    CHECK(rightTopLeft.x == rightItemLeft);
    CHECK(rightTopLeft.y == gridTop);
    CHECK((rightTopLeft.x - gridLeft) / SEASON3B::INVENTORY_SQUARE_WIDTH == 6);
    CHECK((rightTopLeft.y - gridTop) / SEASON3B::INVENTORY_SQUARE_HEIGHT == 0);
}

TEST_CASE("inventory drag anchor survives dock scaling [ui][inventory]")
{
    const auto dock = UI::Scaling::DockRightTransform(1920, 1080);
    constexpr int itemLeft = 465;
    constexpr int itemTop = 200;
    constexpr int pointerOffsetX = 38;
    constexpr int pointerOffsetY = 37;
    const float windowPointerX = UI::Scaling::PositionX(dock, itemLeft + pointerOffsetX);
    const float windowPointerY = UI::Scaling::PositionY(dock, itemTop + pointerOffsetY);
    const int logicalPointerX = static_cast<int>(std::floor(UI::Scaling::LogicalX(dock, windowPointerX)));
    const int logicalPointerY = static_cast<int>(std::floor(UI::Scaling::LogicalY(dock, windowPointerY)));

    const POINT offset = UI::Items::Drag::PickupOffset(itemLeft, itemTop, 40, 40,
                                                       logicalPointerX, logicalPointerY, true);
    const POINT itemTopLeft = UI::Items::Drag::ItemTopLeft(logicalPointerX, logicalPointerY, offset);
    CHECK(itemTopLeft.x == itemLeft);
    CHECK(itemTopLeft.y == itemTop);
}

TEST_CASE("inventory rejects item footprints beyond the final column [ui][inventory]")
{
    constexpr int columnCount = 8;
    constexpr int rowCount = 8;
    constexpr int itemWidth = 2;
    constexpr int itemHeight = 1;

    CHECK(UI::Items::Grid::Fits(columnCount - itemWidth, itemWidth, itemHeight, columnCount, rowCount));
    CHECK_FALSE(UI::Items::Grid::Fits(columnCount - 1, itemWidth, itemHeight, columnCount, rowCount));
}

TEST_CASE("inventory consumes picked-item presses before world input [ui][inventory]")
{
    CHECK(UI::Items::Drag::ShouldConsumePanelPress(true, true));
    CHECK_FALSE(UI::Items::Drag::ShouldConsumePanelPress(true, false));
    CHECK_FALSE(UI::Items::Drag::ShouldConsumePanelPress(false, true));
}

TEST_CASE("store window consumes passive hover before world selection [ui][store]")
{
    const int previousMouseX = MouseX;
    const int previousMouseY = MouseY;
    MouseX = 100;
    MouseY = 200;

    {
        SEASON3B::CNewUINPCShop shop;
        shop.SetSellingItem(true);
        CHECK_FALSE(shop.UpdateMouseEvent());
    }

    MouseX = previousMouseX;
    MouseY = previousMouseY;
}

TEST_CASE("right dock anchors existing panel columns to the viewport edge [ui][scaling]")
{
    const auto dock = UI::Scaling::DockRightTransform(1920, 1080);
    CHECK(UI::Scaling::PositionX(dock, 450.0f) == doctest::Approx(1492.5f));
    CHECK(UI::Scaling::PositionX(dock, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::LogicalX(dock, 1492.5f) == doctest::Approx(450.0f));
}

TEST_CASE("docked command window ends at the bottom HUD top [ui][scaling]")
{
    CHECK(UI::Scaling::PositionY(
        UI::Scaling::DockRightTransform(1280, 1024),
        CNewUICommandWindow::COMMAND_WINDOW_HEIGHT) == doctest::Approx(922.0f));
}

TEST_CASE("dockable command windows share the HUD boundary [ui][scaling]")
{
    CHECK(UI::Scaling::DockLogicalBottom == 432);
    CHECK(CNewUICommandWindow::COMMAND_WINDOW_HEIGHT == UI::Scaling::DockLogicalBottom);
    CHECK(SEASON3B::CNewUIChatCommandWindow::WindowHeight == UI::Scaling::DockLogicalBottom);
}

TEST_CASE("command windows render between HUD and modal layers [ui][scaling]")
{
    CNewUICommandWindow commandWindow;
    SEASON3B::CNewUIChatCommandWindow commandListWindow;

    CHECK(UI::Layout::ForegroundPanelLayerDepth > 10.6f);
    CHECK(UI::Layout::ForegroundPanelLayerDepth < 10.7f);
    CHECK(commandWindow.GetLayerDepth() == doctest::Approx(UI::Layout::ForegroundPanelLayerDepth));
    CHECK(commandListWindow.GetLayerDepth()
          == doctest::Approx(UI::Layout::ForegroundPanelLayerDepth));
}

TEST_CASE("input screen bounds accept only positive resize dimensions [ui][scaling]")
{
    class TestInput final : public CInput
    {
    };

    TestInput input;
    input.SetScreenSize(800, 600);
    CHECK(input.GetScreenWidth() == 800);
    CHECK(input.GetScreenHeight() == 600);

    input.SetScreenSize(0, 720);
    CHECK(input.GetScreenWidth() == 800);
    CHECK(input.GetScreenHeight() == 600);

    input.SetScreenSize(1280, 1024);
    CHECK(input.GetScreenWidth() == 1280);
    CHECK(input.GetScreenHeight() == 1024);
}

TEST_CASE("character selection controls scale from 800x600 and cap at 2x [ui][scaling]")
{
    CHECK(UI::CharacterSelection::CalculateLayout(640, 480).scale == doctest::Approx(1.0f));
    CHECK(UI::CharacterSelection::CalculateLayout(800, 600).scale == doctest::Approx(1.0f));
    CHECK(UI::CharacterSelection::CalculateLayout(1280, 1024).scale == doctest::Approx(1.6f));
    CHECK(UI::CharacterSelection::CalculateLayout(1920, 1080).scale == doctest::Approx(1.8f));
    CHECK(UI::CharacterSelection::CalculateLayout(3840, 2160).scale == doctest::Approx(2.0f));
}

TEST_CASE("character selection layout keeps scaled controls and symmetric margins [ui][scaling]")
{
    const auto native = UI::CharacterSelection::CalculateLayout(800, 600);
    CHECK(native.window.x == 22);
    CHECK(native.window.y == 526);
    CHECK(native.window.width == 756);
    CHECK(native.window.height == 30);
    CHECK(native.buttons[CSMW_BTN_CREATE].width == 54);
    CHECK(native.buttons[CSMW_BTN_CREATE].height == 30);
    CHECK(native.information.width == 534);
    CHECK(native.decoration.x == 611);
    CHECK(native.decoration.width == 189);

    const auto sxga = UI::CharacterSelection::CalculateLayout(1280, 1024);
    CHECK(sxga.window.x == 35);
    CHECK(sxga.window.y == 902);
    CHECK(sxga.window.width == 1210);
    CHECK(sxga.buttons[CSMW_BTN_CREATE].width == 86);
    CHECK(sxga.buttons[CSMW_BTN_CREATE].height == 48);
    CHECK(sxga.information.width == 856);
    CHECK(sxga.decoration.x == 978);
    CHECK(sxga.decoration.width == 302);

    const auto fullHd = UI::CharacterSelection::CalculateLayout(1920, 1080);
    CHECK(fullHd.window.x == 40);
    CHECK(fullHd.window.y == 947);
    CHECK(fullHd.window.width == 1840);
    CHECK(fullHd.buttons[CSMW_BTN_CREATE].width == 97);
    CHECK(fullHd.buttons[CSMW_BTN_CREATE].height == 54);
    CHECK(fullHd.information.width == 1440);

    const auto fourK = UI::CharacterSelection::CalculateLayout(3840, 2160);
    CHECK(fourK.window.x == 44);
    CHECK(fourK.window.width == 3752);
    CHECK(fourK.buttons[CSMW_BTN_CREATE].width == 108);
    CHECK(fourK.buttons[CSMW_BTN_CREATE].height == 60);
    CHECK(fourK.information.width == 3308);
}

TEST_CASE("resized character button rectangle is its click rectangle [ui][scaling]")
{
    const unsigned int previousHeight = WindowHeight;
    WindowHeight = 600;

    CButton button;
    button.Create(54, 30, -1);
    button.SetSize(108, 60);
    button.SetPosition(100, 200);
    button.Show();

    CHECK(button.GetWidth() == 108);
    CHECK(button.GetHeight() == 60);
    CHECK(button.PtInSprite(207, 259));
    CHECK_FALSE(button.PtInSprite(208, 259));
    CHECK_FALSE(button.PtInSprite(207, 260));

    WindowHeight = previousHeight;
}

TEST_CASE("3D item rendering uses its owner layout for hover input [ui][scaling]")
{
    const unsigned int previousWidth = WindowWidth;
    const unsigned int previousHeight = WindowHeight;
    const int previousMouseX = MouseX;
    const int previousMouseY = MouseY;
    const float previousWindowMouseX = g_fWindowMouseX;
    const float previousWindowMouseY = g_fWindowMouseY;
    const auto previousTransform = UI::Scaling::GetActiveTransform();

    WindowWidth = 1920;
    WindowHeight = 1080;
    g_fWindowMouseX = 1605.0f;
    g_fWindowMouseY = 456.0f;
    UI::Scaling::SetActiveTransform(UI::Scaling::ScreenOverlayTransform(WindowWidth, WindowHeight));
    MouseX = 546;
    MouseY = 228;

    CNewUICommandWindow owner;
    owner.SetLayoutMode(UI::Scaling::LayoutMode::DockRight);
    Recording3DObject object(&owner);
    Test3DCamera camera;
    camera.Add3DRenderObj(&object);

    camera.Render3D();

    CHECK(object.mouseX == 500);
    CHECK(object.mouseY == 200);
    CHECK(MouseX == 546);
    CHECK(MouseY == 228);
    CHECK(UI::Scaling::GetActiveTransform().scaleX == doctest::Approx(3.0f));
    CHECK(UI::Scaling::GetActiveTransform().offsetX == doctest::Approx(0.0f));

    UI::Scaling::SetActiveTransform(previousTransform);
    WindowWidth = previousWidth;
    WindowHeight = previousHeight;
    MouseX = previousMouseX;
    MouseY = previousMouseY;
    g_fWindowMouseX = previousWindowMouseX;
    g_fWindowMouseY = previousWindowMouseY;
}

TEST_CASE("managed rendering uses its layout mouse coordinates [ui][scaling]")
{
    const unsigned int previousWidth = WindowWidth;
    const unsigned int previousHeight = WindowHeight;
    const int previousMouseX = MouseX;
    const int previousMouseY = MouseY;
    const float previousWindowMouseX = g_fWindowMouseX;
    const float previousWindowMouseY = g_fWindowMouseY;
    const auto previousTransform = UI::Scaling::GetActiveTransform();

    WindowWidth = 1920;
    WindowHeight = 1080;
    g_fWindowMouseX = 1605.0f;
    g_fWindowMouseY = 456.0f;
    UI::Scaling::SetActiveTransform(UI::Scaling::ScreenOverlayTransform(WindowWidth, WindowHeight));
    MouseX = 535;
    MouseY = 202;

    RecordingUIObject object;
    SEASON3B::CNewUIManager manager;
    manager.AddUIObj(SEASON3B::INTERFACE_INVENTORY, &object);

    manager.Render();

    CHECK(object.renderMouseX == 500);
    CHECK(object.renderMouseY == 200);
    CHECK(MouseX == 535);
    CHECK(MouseY == 202);
    CHECK(UI::Scaling::GetActiveTransform().scaleX == doctest::Approx(3.0f));
    CHECK(UI::Scaling::GetActiveTransform().offsetX == doctest::Approx(0.0f));

    UI::Scaling::SetActiveTransform(previousTransform);
    WindowWidth = previousWidth;
    WindowHeight = previousHeight;
    MouseX = previousMouseX;
    MouseY = previousMouseY;
    g_fWindowMouseX = previousWindowMouseX;
    g_fWindowMouseY = previousWindowMouseY;
}

TEST_CASE("map splash centers in physical window pixels [ui][scaling]")
{
    CHECK(UI::MapName::ImageWidth == doctest::Approx(166.0f));
    CHECK(UI::MapName::ImageHeight == doctest::Approx(90.0f));
    CHECK(UI::MapName::PhysicalLeft(1280) == doctest::Approx(557.0f));
    CHECK(UI::MapName::PhysicalLeft(1920) == doctest::Approx(877.0f));
}

TEST_CASE("bottom HUD regions reconstruct at 640x480 and 1024x768 [ui][scaling]")
{
    const auto referenceLeft = UI::Scaling::BottomHudLeftTransform(640, 480);
    const auto referenceCenter = UI::Scaling::BottomHudCenterTransform(640, 480);
    const auto referenceRight = UI::Scaling::BottomHudRightTransform(640, 480);
    CHECK(referenceLeft.scaleX == doctest::Approx(1.0f));
    CHECK(referenceCenter.offsetX == doctest::Approx(0.0f));
    CHECK(referenceRight.offsetX == doctest::Approx(0.0f));
    CHECK(UI::Scaling::PositionX(referenceLeft, 152.0f) == doctest::Approx(152.0f));
    CHECK(UI::Scaling::PositionX(referenceCenter, 152.0f) == doctest::Approx(152.0f));
    CHECK(UI::Scaling::PositionX(referenceCenter, 488.0f) == doctest::Approx(488.0f));
    CHECK(UI::Scaling::PositionX(referenceRight, 488.0f) == doctest::Approx(488.0f));

    const auto left = UI::Scaling::BottomHudLeftTransform(1024, 768);
    const auto center = UI::Scaling::BottomHudCenterTransform(1024, 768);
    const auto right = UI::Scaling::BottomHudRightTransform(1024, 768);
    CHECK(left.scaleX == doctest::Approx(1.6f));
    CHECK(UI::Scaling::PositionX(left, 152.0f) == doctest::Approx(243.2f));
    CHECK(UI::Scaling::PositionX(center, 152.0f) == doctest::Approx(243.2f));
    CHECK(UI::Scaling::PositionX(center, 488.0f) == doctest::Approx(780.8f));
    CHECK(UI::Scaling::PositionX(right, 488.0f) == doctest::Approx(780.8f));
}

TEST_CASE("bottom HUD uses symmetric wide gaps and caps at 2x [ui][scaling]")
{
    const auto hdLeft = UI::Scaling::BottomHudLeftTransform(1280, 720);
    const auto hdCenter = UI::Scaling::BottomHudCenterTransform(1280, 720);
    const auto hdRight = UI::Scaling::BottomHudRightTransform(1280, 720);
    CHECK(hdCenter.scaleX == doctest::Approx(1.5f));
    CHECK(UI::Scaling::PositionX(hdLeft, 152.0f) == doctest::Approx(228.0f));
    CHECK(UI::Scaling::PositionX(hdCenter, 152.0f) == doctest::Approx(388.0f));
    CHECK(UI::Scaling::PositionX(hdCenter, 320.0f) == doctest::Approx(640.0f));
    CHECK(UI::Scaling::PositionX(hdCenter, 488.0f) == doctest::Approx(892.0f));
    CHECK(UI::Scaling::PositionX(hdRight, 488.0f) == doctest::Approx(1052.0f));

    const auto wideLeft = UI::Scaling::BottomHudLeftTransform(1920, 1200);
    const auto wideCenter = UI::Scaling::BottomHudCenterTransform(1920, 1200);
    const auto wideRight = UI::Scaling::BottomHudRightTransform(1920, 1200);
    CHECK(wideCenter.scaleX == doctest::Approx(2.0f));
    CHECK(UI::Scaling::PositionX(wideLeft, 0.0f) == doctest::Approx(0.0f));
    CHECK(UI::Scaling::PositionX(wideCenter, 320.0f) == doctest::Approx(960.0f));
    CHECK(UI::Scaling::PositionX(wideRight, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::PositionY(wideCenter, 429.0f) == doctest::Approx(1098.0f));
}

TEST_CASE("bottom HUD regional transforms round trip window positions [ui][scaling]")
{
    const auto left = UI::Scaling::BottomHudLeftTransform(1920, 1200);
    const auto center = UI::Scaling::BottomHudCenterTransform(1920, 1200);
    const auto right = UI::Scaling::BottomHudRightTransform(1920, 1200);
    CHECK(UI::Scaling::LogicalX(left, UI::Scaling::PositionX(left, 80.0f)) == doctest::Approx(80.0f));
    CHECK(UI::Scaling::LogicalX(center, UI::Scaling::PositionX(center, 320.0f)) == doctest::Approx(320.0f));
    CHECK(UI::Scaling::LogicalX(right, UI::Scaling::PositionX(right, 560.0f)) == doctest::Approx(560.0f));
    CHECK(UI::Scaling::LogicalY(center, UI::Scaling::PositionY(center, 450.0f)) == doctest::Approx(450.0f));
}

TEST_CASE("experience transform spans the window with HUD vertical scale [ui][scaling]")
{
    const auto experience = UI::Scaling::BottomHudExperienceTransform(1920, 1200);
    CHECK(experience.scaleX == doctest::Approx(3.0f));
    CHECK(experience.scaleY == doctest::Approx(2.0f));
    CHECK(UI::Scaling::PositionX(experience, 0.0f) == doctest::Approx(0.0f));
    CHECK(UI::Scaling::PositionX(experience, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::PositionY(experience, 480.0f) == doctest::Approx(1200.0f));
}

TEST_CASE("world viewport spans the window while docks remain at the rounded HUD top [ui][scaling]")
{
    const auto hd = UI::Scaling::WorldViewport(1280, 720, false);
    CHECK(hd.width == 1280);
    CHECK(hd.height == 720);
    CHECK(UI::Scaling::WorldViewportAspect(1280, 720, false) == doctest::Approx(1280.0f / 720.0f));
    const auto hdDock = UI::Scaling::DockLeftTransform(1280, 720);
    CHECK(UI::Scaling::PositionY(hdDock, 432.0f) == doctest::Approx(644.0f));

    const auto sxga = UI::Scaling::WorldViewport(1280, 1024, false);
    CHECK(sxga.width == 1280);
    CHECK(sxga.height == 1024);
    const auto sxgaDock = UI::Scaling::DockLeftTransform(1280, 1024);
    CHECK(UI::Scaling::PositionY(sxgaDock, 432.0f) == doctest::Approx(922.0f));

    const auto topView = UI::Scaling::WorldViewport(1920, 1200, true);
    CHECK(topView.width == 1920);
    CHECK(topView.height == 1200);
}

TEST_CASE("world viewport clamps zero and tiny dimensions before deriving aspect [ui][scaling]")
{
    const auto zeroNormal = UI::Scaling::WorldViewport(0, 0, false);
    CHECK(zeroNormal.width == 1);
    CHECK(zeroNormal.height == 1);
    CHECK(UI::Scaling::WorldViewportAspect(0, 0, false) == doctest::Approx(1.0f));

    const auto tinyNormal = UI::Scaling::WorldViewport(640, 1, false);
    CHECK(tinyNormal.width == 640);
    CHECK(tinyNormal.height == 1);
    CHECK(UI::Scaling::WorldViewportAspect(640, 1, false) == doctest::Approx(640.0f));

    const auto zeroTopView = UI::Scaling::WorldViewport(0, 0, true);
    CHECK(zeroTopView.width == 1);
    CHECK(zeroTopView.height == 1);
    CHECK(UI::Scaling::WorldViewportAspect(0, 0, true) == doctest::Approx(1.0f));

    const auto tinyTopView = UI::Scaling::WorldViewport(640, 1, true);
    CHECK(tinyTopView.width == 640);
    CHECK(tinyTopView.height == 1);
    CHECK(UI::Scaling::WorldViewportAspect(640, 1, true) == doctest::Approx(640.0f));
}

TEST_CASE("bottom HUD hit-region edges block controls and preserve wide gaps [ui][scaling]")
{
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 100.0f, 643.49f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 100.0f, 643.5f));

    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 227.99f, 660.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 228.0f, 660.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 300.0f, 660.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 387.99f, 660.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 388.0f, 660.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 891.99f, 660.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 892.0f, 660.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 980.0f, 660.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 1051.99f, 660.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 1052.0f, 660.0f));

    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 0.0f, 705.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 300.0f, 710.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 980.0f, 710.0f));
    CHECK(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 1279.99f, 719.99f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 1280.0f, 710.0f));
    CHECK_FALSE(UI::Scaling::BottomHudContainsWindowPoint(1280, 720, 640.0f, 720.0f));
}

TEST_CASE("legacy UI preserves logical input and world-overlay coordinates [ui][scaling]")
{
    const auto legacy = UI::Scaling::LegacyUiTransform(1280, 720);
    CHECK(UI::Scaling::PositionX(legacy, 123.0f) == doctest::Approx(246.0f));
    CHECK(UI::Scaling::PositionY(legacy, 45.0f) == doctest::Approx(67.5f));
    CHECK(UI::Scaling::SizeX(legacy, 190.0f) == doctest::Approx(380.0f));
    CHECK(UI::Scaling::SizeY(legacy, 13.0f) == doctest::Approx(19.5f));
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, legacy) == 11);
    CHECK(UI::Scaling::FontPointSize(FontRole::Fixed, legacy) == 13);
}

TEST_CASE("interface policy selects viewport dock and dialog layouts [ui][scaling]")
{
    using UI::Scaling::LayoutMode;
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MAINFRAME) == LayoutMode::Hud);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_SKILL_LIST) == LayoutMode::HudCenter);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_HOTKEY) == LayoutMode::Hud);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_INVENTORY) == LayoutMode::DockRight);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MOVEMAP) == LayoutMode::DockLeft);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_FRIEND) == LayoutMode::FloatingWorkspace);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MESSAGEBOX) == LayoutMode::Dialog);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_NAME_WINDOW) == LayoutMode::WorldOverlay);
}

TEST_CASE("floating windows keep uniform scale across the full viewport [ui][scaling]")
{
    const auto transform = UI::Scaling::FloatingWorkspaceTransform(3840, 2160);
    const auto bounds = UI::Scaling::FloatingWorkspaceBounds(3840, 2160);

    CHECK(transform.scaleX == doctest::Approx(2.25f));
    CHECK(transform.scaleY == doctest::Approx(2.25f));
    CHECK(transform.offsetX == doctest::Approx(0.0f));
    CHECK(transform.offsetY == doctest::Approx(0.0f));
    CHECK(bounds.width == 1706);
    CHECK(bounds.height == 960);
    CHECK(UI::Scaling::PositionX(transform, static_cast<float>(bounds.width)) <= 3840.0f);
    CHECK(UI::Scaling::PositionY(transform, static_cast<float>(bounds.height)) <= 2160.0f);
}

TEST_CASE("screen coverage follows the capped HUD boundary [ui][scaling]")
{
    const auto screen = UI::Scaling::ScreenOverlayTransform(3840, 2160);
    const float screenHeight = UI::Scaling::ScreenOverlayContentHeight(3840, 2160);

    CHECK(screenHeight == doctest::Approx(457.333333f));
    CHECK(UI::Scaling::SizeY(screen, screenHeight) == doctest::Approx(2058.0f));
    CHECK(UI::Scaling::ScreenOverlayContentHeight(640, 480) == doctest::Approx(429.0f));
    CHECK(UI::Scaling::FloatingWorkspaceContentHeight(3840, 2160) == doctest::Approx(914.666667f));
}

TEST_CASE("positions include offsets and sizes do not [ui][scaling]")
{
    const auto transform = UI::Scaling::PanelTransform(1920, 1080);
    CHECK(UI::Scaling::PositionX(transform, 10.0f) == doctest::Approx(340.0f));
    CHECK(UI::Scaling::PositionY(transform, 20.0f) == doctest::Approx(100.0f));
    CHECK(UI::Scaling::SizeX(transform, 10.0f) == doctest::Approx(20.0f));
    CHECK(UI::Scaling::SizeY(transform, 20.0f) == doctest::Approx(40.0f));
    CHECK(UI::Scaling::LogicalX(transform, 340.0f) == doctest::Approx(10.0f));
    CHECK(UI::Scaling::LogicalY(transform, 100.0f) == doctest::Approx(20.0f));
}

TEST_CASE("letter preview viewport includes active layout offsets [ui][scaling]")
{
    const UI::Scaling::Transform transform{2.25f, 2.25f, 480.0f, 6.0f, 2.25f};
    const auto viewport = UI::Scaling::ViewportForLogicalRect(transform, 351.0f, 151.0f, 119.0f, 141.0f);

    CHECK(viewport.x == 1270);
    CHECK(viewport.y == 346);
    CHECK(viewport.width == 268);
    CHECK(viewport.height == 317);
}

TEST_CASE("focused letter input owns its parent window selection [ui][input]")
{
    CUITextInputBox input;
    input.SetParentUIID(42);
    input.GiveFocus(FALSE);

    CHECK(CUITextInputBox::IsFocusedForParent(42));
    CHECK_FALSE(CUITextInputBox::IsFocusedForParent(41));

    CUITextInputBox::ReleaseFocus();
}

TEST_CASE("screen overlays fill the window [ui][scaling]")
{
    const auto transform = UI::Scaling::ScreenOverlayTransform(1280, 720);
    CHECK(transform.scaleX == doctest::Approx(2.0f));
    CHECK(transform.scaleY == doctest::Approx(1.5f));
    CHECK(transform.offsetX == doctest::Approx(0.0f));
    CHECK(transform.offsetY == doctest::Approx(0.0f));
}

TEST_CASE("Windows content scaling enlarges capped UI without double-scaling Retina [ui][scaling]")
{
    CHECK(UI::Scaling::ContentScaleFromMetrics(2.0f, 1.0f) == doctest::Approx(2.0f));
    CHECK(UI::Scaling::ContentScaleFromMetrics(2.0f, 2.0f) == doctest::Approx(1.0f));
    CHECK(UI::Scaling::ContentScaleFromMetrics(0.0f, 0.0f) == doctest::Approx(1.0f));

    const float previousScale = UI::Scaling::GetWindowContentScale();
    UI::Scaling::SetWindowContentScale(2.0f);

    const auto dialog = UI::Scaling::PanelTransform(3840, 2160);
    CHECK(dialog.scaleX == doctest::Approx(4.0f));
    CHECK(dialog.scaleY == doctest::Approx(4.0f));
    CHECK(dialog.offsetX == doctest::Approx(640.0f));
    CHECK(dialog.offsetY == doctest::Approx(120.0f));
    CHECK(UI::Scaling::CachedFontPointSize(FontRole::Normal) == 32);

    UI::Scaling::SetWindowContentScale(previousScale);
}

TEST_CASE("login and character scenes use the full reference viewport [ui][scaling]")
{
    const auto viewport = UI::Scaling::FullReferenceViewport();
    CHECK(viewport.x == 0);
    CHECK(viewport.y == 0);
    CHECK(viewport.width == 640);
    CHECK(viewport.height == 480);
}

TEST_CASE("active transform round trips [ui][scaling]")
{
    const auto previous = UI::Scaling::GetActiveTransform();
    const UI::Scaling::Transform transform{1.25f, 1.5f, 30.0f, 40.0f, 1.2f};

    UI::Scaling::SetActiveTransform(transform);
    const auto active = UI::Scaling::GetActiveTransform();
    CHECK(active.scaleX == doctest::Approx(transform.scaleX));
    CHECK(active.scaleY == doctest::Approx(transform.scaleY));
    CHECK(active.offsetX == doctest::Approx(transform.offsetX));
    CHECK(active.offsetY == doctest::Approx(transform.offsetY));
    CHECK(active.typographyScale == doctest::Approx(transform.typographyScale));

    UI::Scaling::SetActiveTransform(previous);
}

TEST_CASE("scoped active transform restores transform and logical mouse [ui][scaling]")
{
    const auto previousTransform = UI::Scaling::GetActiveTransform();
    const int previousMouseX = MouseX;
    const int previousMouseY = MouseY;
    const float previousWindowMouseX = g_fWindowMouseX;
    const float previousWindowMouseY = g_fWindowMouseY;

    const UI::Scaling::Transform base{1.0f, 1.0f, 0.0f, 0.0f, 1.0f};
    const UI::Scaling::Transform regional{2.0f, 2.0f, 100.0f, 200.0f, 2.0f};
    UI::Scaling::SetActiveTransform(base);
    MouseX = 7;
    MouseY = 9;
    g_fWindowMouseX = 300.0f;
    g_fWindowMouseY = 500.0f;

    {
        UI::Scaling::ScopedActiveTransform scoped(regional, true);
        CHECK(UI::Scaling::GetActiveTransform().offsetX == doctest::Approx(100.0f));
        CHECK(MouseX == 100);
        CHECK(MouseY == 150);
    }

    CHECK(UI::Scaling::GetActiveTransform().offsetX == doctest::Approx(0.0f));
    CHECK(MouseX == 7);
    CHECK(MouseY == 9);

    UI::Scaling::SetActiveTransform(previousTransform);
    MouseX = previousMouseX;
    MouseY = previousMouseY;
    g_fWindowMouseX = previousWindowMouseX;
    g_fWindowMouseY = previousWindowMouseY;
}

TEST_CASE("layout typography grows gradually and fits bounded controls [ui][scaling]")
{
    const auto reference = UI::Scaling::PanelTransform(640, 480);
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, reference) == 11);
    CHECK(UI::Scaling::FontPointSize(FontRole::Bold, reference) == 11);
    CHECK(UI::Scaling::FontPointSize(FontRole::Big, reference) == 22);
    CHECK(UI::Scaling::FontPointSize(FontRole::Fixed, reference) == 13);

    const auto dialog = UI::Scaling::PanelTransform(1280, 720);
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, dialog) == 13);
    CHECK(UI::Scaling::MaximumFontPointSize(FontRole::Normal) == 16);
    CHECK(UI::Scaling::FontScaleForBounds(FontRole::Normal, dialog, 160.0f, 20.0f, 200.0f, 30.0f)
          == doctest::Approx(13.0f / 16.0f));
    CHECK(UI::Scaling::FontScaleForBounds(FontRole::Normal, dialog, 160.0f, 20.0f, 100.0f, 30.0f)
          == doctest::Approx(11.0f / 16.0f));

    const auto dock = UI::Scaling::DockRightTransform(1920, 1080);
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, dock) == 16);
    CHECK(UI::Scaling::FontPointSize(FontRole::Big, dock) == 32);
    CHECK(UI::Scaling::FontPointSize(FontRole::Fixed, dock) == 18);

    const auto fourK = UI::Scaling::ScreenOverlayTransform(3840, 2160);
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, fourK) == 16);
    CHECK(UI::Scaling::FontPointSize(FontRole::Big, fourK) == 32);
    CHECK(UI::Scaling::FontPointSize(FontRole::Fixed, fourK) == 18);
}

TEST_CASE("window cursor centers detached content in the active layout [ui][scaling]")
{
    const auto screen = UI::Scaling::ScreenOverlayTransform(1920, 1080);
    const auto screenPosition = UI::Scaling::CenteredLogicalPosition(screen, 960.0f, 540.0f, 40.0f, 60.0f);
    CHECK(screenPosition.x == doctest::Approx(300.0f));
    CHECK(screenPosition.y == doctest::Approx(210.0f));

    const auto dock = UI::Scaling::DockRightTransform(1920, 1080);
    const auto dockPosition = UI::Scaling::CenteredLogicalPosition(dock, 1200.0f, 540.0f, 40.0f, 60.0f);
    CHECK(dockPosition.x == doctest::Approx(300.0f));
    CHECK(UI::Scaling::PositionX(dock, dockPosition.x + 20.0f) == doctest::Approx(1200.0f));
    CHECK(UI::Scaling::PositionY(dock, dockPosition.y + 30.0f) == doctest::Approx(540.0f));
}
