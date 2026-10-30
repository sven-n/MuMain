#include <doctest.h>
#include "stdafx.h"
#include "UI/RmlBridge/ThemeInheritance.h"
#include "UI/RmlBridge/RmlStackingOrder.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <chrono>
#include <fstream>

using UI::RmlBridge::ThemeInheritance;
using UI::RmlBridge::ResolveThemeResource;
using UI::RmlBridge::StackingDepthForDocument;
using UI::RmlBridge::ThemedStackingDepth;

namespace
{
struct ThemeFiles
{
    std::filesystem::path temporary = std::filesystem::temp_directory_path() /
        ("mu_theme_inheritance_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::path root = temporary / "Data/Interface/RmlUi/themes";
    ~ThemeFiles() { std::filesystem::remove_all(temporary); }
    void Write(const std::string& path, const std::string& contents) const
    {
        std::filesystem::create_directories((root / path).parent_path());
        std::ofstream(root / path, std::ios::binary) << contents;
    }
};
}

TEST_CASE("theme chains merge settings and replace files from child to ancestor [ui][rml-theme]")
{
    ThemeFiles files;
    files.Write("base/theme.ini", "[Capabilities]\nNativeTextSize=1\n[Fonts]\nbody=fonts/body.ttf\n");
    files.Write("base/tokens.ini", "[Tokens]\naccent=red\nmuted=gray\n");
    files.Write("base/base.rcss", "base");
    files.Write("base/panel.rcss", "parent");
    files.Write("middle/theme.ini", "[Theme]\nBase=base\n");
    files.Write("child/theme.ini", "\xef\xbb\xbf[THEME]\nbase=\"MIDDLE\"\n[Capabilities]\nNativeTextSize=0\n");
    files.Write("child/tokens.ini", "[Tokens]\naccent=blue\n");
    files.Write("child/panel.rcss", "child");
    const ThemeInheritance theme(files.root / "child");
    REQUIRE(theme.IsValid());
    CHECK(theme.Directories().size() == 3);
    REQUIRE(theme.GetSetting("Tokens", "accent"));
    CHECK(theme.GetSetting("Tokens", "accent")->value == "blue");
    CHECK(theme.GetSetting("Tokens", "muted")->value == "gray");
    CHECK(theme.GetSetting("Capabilities", "NativeTextSize")->value == "0");
    CHECK(theme.GetSetting("Fonts", "body")->directory == files.root / "base");
    CHECK(theme.ResolveFile("base.rcss") == files.root / "base/base.rcss");
    CHECK(theme.ResolveFile("panel.rcss") == files.root / "child/panel.rcss");
    CHECK_FALSE(theme.ResolveFile("../base/base.rcss"));
    CHECK_FALSE(theme.ResolveFile(files.root / "base/base.rcss"));
}

TEST_CASE("invalid ancestry fails without partial settings or file fallback [ui][rml-theme]")
{
    ThemeFiles files;
    files.Write("a/theme.ini", "[Theme]\nBase=b\n");
    files.Write("b/theme.ini", "[Theme]\nBase=a\n");
    files.Write("a/base.rcss", "exists");
    CHECK_FALSE(ThemeInheritance(files.root / "a").IsValid());
    CHECK_FALSE(ThemeInheritance(files.root / "a").ResolveFile("base.rcss"));
    CHECK(ThemeInheritance(files.root / "a").Section("Theme").empty());
    files.Write("b/theme.ini", "[Theme]\nBase=b\n");
    CHECK_FALSE(ThemeInheritance(files.root / "b").IsValid());
    files.Write("b/theme.ini", "[Theme]\nBase=missing\n");
    CHECK_FALSE(ThemeInheritance(files.root / "a").IsValid());
    files.Write("b/theme.ini", "[Theme]\nBase=../a\n");
    CHECK_FALSE(ThemeInheritance(files.root / "b").IsValid());
    CHECK_FALSE(ThemeInheritance::IsValidName("../a"));
    CHECK_FALSE(ThemeInheritance::IsValidName("a/b"));
}

TEST_CASE("inherited images resolve container files but keep logical extensions [ui][rml-theme]")
{
    ThemeFiles files;
    files.Write("base/theme.ini", "");
    files.Write("base/images/panel.OZT", "image");
    files.Write("base/images/photo.OZJ", "image");
    files.Write("child/theme.ini", "[Theme]\nBase=base\n");
    const auto virtualImage = files.root / "child/images/panel.tga";
    CHECK(ResolveThemeResource(virtualImage.generic_string(), true) == (files.root / "base/images/panel.tga").generic_string());
    CHECK(ResolveThemeResource((files.root / "child/images/photo.jpg").generic_string(), true) ==
          (files.root / "base/images/photo.jpg").generic_string());
    files.Write("child/images/panel.OZT", "override");
    CHECK(ResolveThemeResource(virtualImage.generic_string(), true) == virtualImage.generic_string());
    CHECK(ResolveThemeResource("Data/Interface/ordinary.tga", true) == "Data/Interface/ordinary.tga");
}

TEST_CASE("a theme restacks documents through [Stacking], inherited like any setting [ui][rml-theme]")
{
    ThemeFiles files;
    files.Write("base/theme.ini", "[Stacking]\nchat_input.rml=10.65\nchat_log.rml=over\nmain_frame_top.rml=nan\ntooltip.rml=inf\n");
    files.Write("child/theme.ini", "[Theme]\nBase=base\n");
    const ThemeInheritance theme(files.root / "child");
    REQUIRE(theme.IsValid());
    const auto input = ThemedStackingDepth(theme, "chat_input.rml");
    REQUIRE(input);
    CHECK(*input == doctest::Approx(10.65f));
    // Not listed, not a number, or not finite: the stacking table's depth.
    CHECK(ThemedStackingDepth(theme, "main_frame.rml") == StackingDepthForDocument("main_frame.rml"));
    CHECK(ThemedStackingDepth(theme, "chat_log.rml") == StackingDepthForDocument("chat_log.rml"));
    CHECK(ThemedStackingDepth(theme, "main_frame_top.rml") == StackingDepthForDocument("main_frame_top.rml"));
    CHECK(ThemedStackingDepth(theme, "tooltip.rml") == StackingDepthForDocument("tooltip.rml"));
    CHECK_FALSE(ThemedStackingDepth(theme, "no_such_document.rml"));
}

TEST_CASE("the classic theme stacks its chat input over the HUD and under the tooltips [ui][rml-theme]")
{
    const ThemeInheritance classic(std::filesystem::path(MU_RMLUI_DIR) / "themes/classic");
    REQUIRE(classic.IsValid());
    const auto input = ThemedStackingDepth(classic, "chat_input.rml");
    REQUIRE(input);
    CHECK(*input > *StackingDepthForDocument("main_frame.rml"));
    // Over the command windows it can overlap, never at their depth (equal depths order by focus).
    CHECK(*input > *StackingDepthForDocument("command_window.rml"));
    CHECK(*input > *StackingDepthForDocument("chat_command.rml"));
    CHECK(*input < *StackingDepthForDocument("option_window.rml"));
    CHECK(*input < *StackingDepthForDocument("tooltip.rml"));
    // The HUD keeps its own depth, over the windows, so the skill list beside them stays reachable.
    CHECK(ThemedStackingDepth(classic, "main_frame.rml") == StackingDepthForDocument("main_frame.rml"));

    const ThemeInheritance legacy(std::filesystem::path(MU_RMLUI_DIR) / "themes/legacy");
    REQUIRE(legacy.IsValid());
    CHECK(ThemedStackingDepth(legacy, "chat_input.rml") == StackingDepthForDocument("chat_input.rml"));
}
