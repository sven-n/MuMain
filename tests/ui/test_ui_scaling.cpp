#include <doctest.h>

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_enum.h"
#include "UI/NewUI/UILayoutPolicy.h"
#include "UI/Scaling/UITransform.h"

using UI::Scaling::FontRole;

TEST_CASE("dialogs stay centered and capped [ui][scaling]")
{
    const auto hd = UI::Scaling::PanelTransform(1280, 720);
    CHECK(hd.scaleX == doctest::Approx(1.5f));
    CHECK(hd.scaleY == doctest::Approx(1.5f));
    CHECK(hd.offsetX == doctest::Approx(160.0f));
    CHECK(hd.offsetY == doctest::Approx(0.0f));

    const auto fullHd = UI::Scaling::PanelTransform(1920, 1080);
    CHECK(fullHd.scaleX == doctest::Approx(1.5f));
    CHECK(fullHd.offsetX == doctest::Approx(480.0f));
    CHECK(fullHd.offsetY == doctest::Approx(180.0f));

    const auto fourK = UI::Scaling::PanelTransform(3840, 2160);
    CHECK(fourK.scaleX == doctest::Approx(1.5f));
    CHECK(fourK.offsetX == doctest::Approx(1440.0f));
    CHECK(fourK.offsetY == doctest::Approx(720.0f));
}

TEST_CASE("HUD fills the viewport while dialogs stay capped [ui][scaling]")
{
    const auto hud = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Hud, 1920, 1080);
    CHECK(UI::Scaling::PositionX(hud, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::PositionY(hud, 480.0f) == doctest::Approx(1080.0f));

    const auto dialog = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Dialog, 1920, 1080);
    CHECK(dialog.scaleX == doctest::Approx(1.5f));
    CHECK(dialog.offsetX == doctest::Approx(480.0f));
    CHECK(dialog.offsetY == doctest::Approx(180.0f));
}

TEST_CASE("right dock anchors existing panel columns to the viewport edge [ui][scaling]")
{
    const auto dock = UI::Scaling::DockRightTransform(1920, 1080);
    CHECK(UI::Scaling::PositionX(dock, 450.0f) == doctest::Approx(1635.0f));
    CHECK(UI::Scaling::PositionX(dock, 640.0f) == doctest::Approx(1920.0f));
    CHECK(UI::Scaling::LogicalX(dock, 1635.0f) == doctest::Approx(450.0f));
}

TEST_CASE("world viewport reaches the rendered dock edge [ui][scaling]")
{
    CHECK(UI::Scaling::WorldViewportWidthForDock(260.0f, 1920, 1200) == doctest::Approx(450.0f));
    CHECK(UI::Scaling::WorldViewportWidthForDock(450.0f, 1920, 1200) == doctest::Approx(545.0f));
    CHECK(UI::Scaling::WorldViewportWidthForDock(640.0f, 1920, 1200) == doctest::Approx(640.0f));
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
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_INVENTORY) == LayoutMode::DockRight);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MOVEMAP) == LayoutMode::DockLeft);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_MESSAGEBOX) == LayoutMode::Dialog);
    CHECK(UI::Layout::ForInterface(SEASON3B::INTERFACE_NAME_WINDOW) == LayoutMode::WorldOverlay);
}

TEST_CASE("positions include offsets and sizes do not [ui][scaling]")
{
    const auto transform = UI::Scaling::PanelTransform(1920, 1080);
    CHECK(UI::Scaling::PositionX(transform, 10.0f) == doctest::Approx(495.0f));
    CHECK(UI::Scaling::PositionY(transform, 20.0f) == doctest::Approx(210.0f));
    CHECK(UI::Scaling::SizeX(transform, 10.0f) == doctest::Approx(15.0f));
    CHECK(UI::Scaling::SizeY(transform, 20.0f) == doctest::Approx(30.0f));
    CHECK(UI::Scaling::LogicalX(transform, 495.0f) == doctest::Approx(10.0f));
    CHECK(UI::Scaling::LogicalY(transform, 210.0f) == doctest::Approx(20.0f));
}

TEST_CASE("screen overlays fill the window [ui][scaling]")
{
    const auto transform = UI::Scaling::ScreenOverlayTransform(1280, 720);
    CHECK(transform.scaleX == doctest::Approx(2.0f));
    CHECK(transform.scaleY == doctest::Approx(1.5f));
    CHECK(transform.offsetX == doctest::Approx(0.0f));
    CHECK(transform.offsetY == doctest::Approx(0.0f));
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

TEST_CASE("layout typography grows gradually and fits bounded controls [ui][scaling]")
{
    const auto reference = UI::Scaling::PanelTransform(640, 480);
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, reference) == 11);
    CHECK(UI::Scaling::FontPointSize(FontRole::Bold, reference) == 11);
    CHECK(UI::Scaling::FontPointSize(FontRole::Big, reference) == 22);
    CHECK(UI::Scaling::FontPointSize(FontRole::Fixed, reference) == 13);

    const auto dialog = UI::Scaling::PanelTransform(1280, 720);
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, dialog) == 13);
    CHECK(UI::Scaling::MaximumFontPointSize(FontRole::Normal) == 13);
    CHECK(UI::Scaling::FontScaleForBounds(FontRole::Normal, dialog, 140.0f, 20.0f, 120.0f, 30.0f)
          == doctest::Approx(120.0f / 140.0f));
    CHECK(UI::Scaling::FontScaleForBounds(FontRole::Normal, dialog, 140.0f, 20.0f, 100.0f, 30.0f)
          == doctest::Approx(11.0f / 13.0f));

    const auto fourK = UI::Scaling::ScreenOverlayTransform(3840, 2160);
    CHECK(UI::Scaling::FontPointSize(FontRole::Normal, fourK) == 13);
    CHECK(UI::Scaling::FontPointSize(FontRole::Big, fourK) == 26);
    CHECK(UI::Scaling::FontPointSize(FontRole::Fixed, fourK) == 15);
}

TEST_CASE("window cursor centers detached content in the active layout [ui][scaling]")
{
    const auto screen = UI::Scaling::ScreenOverlayTransform(1920, 1080);
    const auto screenPosition = UI::Scaling::CenteredLogicalPosition(screen, 960.0f, 540.0f, 40.0f, 60.0f);
    CHECK(screenPosition.x == doctest::Approx(300.0f));
    CHECK(screenPosition.y == doctest::Approx(210.0f));

    const auto dock = UI::Scaling::DockRightTransform(1920, 1080);
    const auto dockPosition = UI::Scaling::CenteredLogicalPosition(dock, 1200.0f, 540.0f, 40.0f, 60.0f);
    CHECK(dockPosition.x == doctest::Approx(140.0f));
    CHECK(dockPosition.y == doctest::Approx(210.0f));
    CHECK(UI::Scaling::PositionX(dock, dockPosition.x + 20.0f) == doctest::Approx(1200.0f));
    CHECK(UI::Scaling::PositionY(dock, dockPosition.y + 30.0f) == doctest::Approx(540.0f));
}
