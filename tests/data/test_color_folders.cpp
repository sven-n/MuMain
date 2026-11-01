// doctest unit tests for the folders MuEditor's Colors window sorts named colors in (D69, ColorFolders.h): the
// "colorFolders" of a file read and written, the edits of the window, and the file that keeps them
// (GlowColors.json).

#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/EffectData/GlowColors.h"

#include <algorithm>
#include <set>
#include <string>
#include <string_view>
#include <vector>

using Data::Colors::ColorFolder;
using Data::Colors::ColorFolderPath;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Items::Json::OrderedJson;

namespace
{
const std::set<std::string, std::less<>> FileColors{"red", "orange", "teal", "gold", "white"};

bool IsFileColor(std::string_view name)
{
    return FileColors.contains(name);
}

std::vector<ColorFolder> Read(std::string_view json, std::vector<ItemDataIssue>& issues)
{
    return Data::Colors::ReadColorFolders(OrderedJson::parse(json), "test.json", &IsFileColor, issues);
}

bool HasWarningAbout(const std::vector<ItemDataIssue>& issues, std::string_view field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [field](const ItemDataIssue& issue)
                       { return issue.severity == ItemDataIssueSeverity::Warning && issue.field == field; });
}

// Warm (Reds (red), orange), Cold (teal); gold and white in no folder.
std::vector<ColorFolder> Sample()
{
    return {{"Warm", {{"Reds", {}, {"red"}}}, {"orange"}}, {"Cold", {}, {"teal"}}};
}
} // namespace

TEST_SUITE("Color folders")
{
    TEST_CASE("a file without folders has none")
    {
        std::vector<ItemDataIssue> issues;
        CHECK(Read(R"({"colors": {}})", issues).empty());
        CHECK(issues.empty());
    }

    TEST_CASE("folders in folders are read with their colors")
    {
        std::vector<ItemDataIssue> issues;
        const std::vector<ColorFolder> folders = Read(
            R"({"colorFolders": [{"name": "Warm", "folders": [{"name": "Reds", "colors": ["red"]}], "colors": ["orange"]},
                                 {"name": "Cold", "colors": ["teal"]}]})",
            issues);
        CHECK(issues.empty());
        CHECK(folders == Sample());
    }

    TEST_CASE("what can't be used is left out with a warning")
    {
        std::vector<ItemDataIssue> issues;
        const std::vector<ColorFolder> folders = Read(R"({"colorFolders": [
            {"name": "Warm", "colors": ["red", "plum", "red", 3]},
            {"name": "Warm", "colors": ["teal"]},
            {"name": "", "colors": ["gold"]},
            {"name": "Cold", "colors": ["red", "white"], "color": "x"}
        ]})",
                                                      issues);
        REQUIRE(folders.size() == 2);
        CHECK(folders[0].colors == std::vector<std::string>{"red"});
        CHECK(folders[1].name == "Cold");
        // A color is in one folder: the first that names it.
        CHECK(folders[1].colors == std::vector<std::string>{"white"});
        CHECK(HasWarningAbout(issues, "colorFolders[0].colors[1]"));
        CHECK(HasWarningAbout(issues, "colorFolders[0].colors[2]"));
        CHECK(HasWarningAbout(issues, "colorFolders[0].colors[3]"));
        CHECK(HasWarningAbout(issues, "colorFolders[1].name"));
        CHECK(HasWarningAbout(issues, "colorFolders[2].name"));
        CHECK(HasWarningAbout(issues, "colorFolders[3].colors[0]"));
        CHECK(HasWarningAbout(issues, "colorFolders[3].color"));
        CHECK_FALSE(Data::Items::HasFatal(issues));
    }

    TEST_CASE("folders deeper than the limit are left out")
    {
        std::string json = R"({"name": "deepest"})";
        for (std::size_t level = 1; level < Data::Colors::MaxColorFolderDepth + 1; ++level)
        {
            json = R"({"name": "level", "folders": [)" + json + "]}";
        }
        std::vector<ItemDataIssue> issues;
        const std::vector<ColorFolder> folders = Read(R"({"colorFolders": [)" + json + "]}", issues);
        const ColorFolder* folder = &folders.at(0);
        for (std::size_t level = 1; level < Data::Colors::MaxColorFolderDepth; ++level)
        {
            REQUIRE(folder->folders.size() == 1);
            folder = &folder->folders[0];
        }
        CHECK(folder->folders.empty());
        CHECK(issues.size() == 1);
    }

    TEST_CASE("folders are written as they are read, without names that are no colors")
    {
        std::vector<ColorFolder> folders = Sample();
        folders[1].colors.push_back("gone");
        folders.push_back({"Empty", {}, {}});
        const OrderedJson written = Data::Colors::WriteColorFolders(folders, &IsFileColor);
        OrderedJson root = OrderedJson::object();
        root[Data::Colors::ColorFoldersKey] = written;
        std::vector<ItemDataIssue> issues;
        std::vector<ColorFolder> expected = Sample();
        expected.push_back({"Empty", {}, {}});
        CHECK(Data::Colors::ReadColorFolders(root, "test.json", &IsFileColor, issues) == expected);
        CHECK(issues.empty());
        CHECK(written[2].dump() == R"({"name":"Empty"})");
    }

    TEST_CASE("the colors no folder holds are under the root, in the file's order")
    {
        const std::vector<std::string> colors{"white", "red", "gold", "teal", "orange"};
        CHECK(Data::Colors::GetRootColors(Sample(), colors) == std::vector<std::string>{"white", "gold"});
        CHECK(Data::Colors::FindFolderOfColor(Sample(), "red") == ColorFolderPath{0, 0});
        CHECK(Data::Colors::FindFolderOfColor(Sample(), "teal") == ColorFolderPath{1});
        CHECK_FALSE(Data::Colors::FindFolderOfColor(Sample(), "gold").has_value());
    }

    TEST_CASE("a folder is added, renamed and moved")
    {
        std::vector<ColorFolder> folders = Sample();
        CHECK(Data::Colors::AddColorFolder(folders, ColorFolderPath{0}, "Yellows"));
        CHECK(folders[0].folders[1].name == "Yellows");
        CHECK_FALSE(Data::Colors::AddColorFolder(folders, ColorFolderPath{0}, "Reds"));
        CHECK_FALSE(Data::Colors::AddColorFolder(folders, {}, ""));
        CHECK_FALSE(Data::Colors::AddColorFolder(folders, ColorFolderPath{5}, "Lost"));

        CHECK(Data::Colors::RenameColorFolder(folders, ColorFolderPath{0, 1}, "Golds"));
        CHECK_FALSE(Data::Colors::RenameColorFolder(folders, ColorFolderPath{0, 1}, "Reds"));
        CHECK_FALSE(Data::Colors::RenameColorFolder(folders, ColorFolderPath{0, 1}, "Golds"));

        CHECK(Data::Colors::MoveColorFolderBy(folders, ColorFolderPath{0, 1}, -1));
        CHECK(folders[0].folders[0].name == "Golds");
        CHECK_FALSE(Data::Colors::MoveColorFolderBy(folders, ColorFolderPath{0, 0}, -1));

        // Into another folder; never into itself or a folder in it.
        CHECK(Data::Colors::MoveColorFolderInto(folders, ColorFolderPath{0, 0}, ColorFolderPath{1}));
        CHECK(folders[1].folders[0].name == "Golds");
        CHECK_FALSE(Data::Colors::MoveColorFolderInto(folders, ColorFolderPath{0}, ColorFolderPath{0, 0}));
        CHECK_FALSE(Data::Colors::MoveColorFolderInto(folders, ColorFolderPath{1, 0}, ColorFolderPath{1}));
        // A folder before the target in the same list: the target's place shifts up by one.
        CHECK(Data::Colors::MoveColorFolderInto(folders, ColorFolderPath{0}, ColorFolderPath{1}));
        REQUIRE(folders.size() == 1);
        CHECK(folders[0].name == "Cold");
        CHECK(folders[0].folders[1].name == "Warm");
    }

    TEST_CASE("a removed folder leaves what it holds to the folder above it")
    {
        std::vector<ColorFolder> folders = Sample();
        folders[1].folders.push_back({"Blues", {}, {"gold"}});
        CHECK(Data::Colors::RemoveColorFolder(folders, ColorFolderPath{1}));
        // Cold's color goes under the root, its folder among the root's folders.
        REQUIRE(folders.size() == 2);
        CHECK(folders[0].name == "Warm");
        CHECK(folders[1].name == "Blues");
        CHECK_FALSE(Data::Colors::FindFolderOfColor(folders, "teal").has_value());

        std::vector<ColorFolder> nested = Sample();
        nested[0].folders.push_back({"Inner", {{"Reds", {}, {"white"}}}, {"gold"}});
        CHECK(Data::Colors::RemoveColorFolder(nested, ColorFolderPath{0, 1}));
        // Inner's colors go to Warm, its Reds is joined with Warm's Reds.
        CHECK(nested[0].colors == std::vector<std::string>{"orange", "gold"});
        REQUIRE(nested[0].folders.size() == 1);
        CHECK(nested[0].folders[0].colors == std::vector<std::string>{"red", "white"});
    }

    TEST_CASE("a color moves between folders and within one")
    {
        std::vector<ColorFolder> folders = Sample();
        CHECK(Data::Colors::MoveColorInto(folders, "gold", ColorFolderPath{0}));
        CHECK(folders[0].colors == std::vector<std::string>{"orange", "gold"});
        CHECK(Data::Colors::MoveColorInFolderBy(folders, "gold", -1));
        CHECK(folders[0].colors == std::vector<std::string>{"gold", "orange"});
        CHECK_FALSE(Data::Colors::MoveColorInFolderBy(folders, "gold", -1));
        CHECK(Data::Colors::MoveColorInto(folders, "red", ColorFolderPath{1}));
        CHECK(folders[0].folders[0].colors.empty());
        CHECK(folders[1].colors == std::vector<std::string>{"teal", "red"});
        CHECK_FALSE(Data::Colors::MoveColorInto(folders, "red", ColorFolderPath{1}));
        CHECK(Data::Colors::MoveColorInto(folders, "red", {}));
        CHECK_FALSE(Data::Colors::FindFolderOfColor(folders, "red").has_value());
        CHECK_FALSE(Data::Colors::MoveColorInto(folders, "red", {}));
    }

    TEST_CASE("a color under the root moves past the next color under the root")
    {
        const std::vector<std::string> colors{"white", "red", "gold", "teal", "orange"};
        const auto order = Data::Colors::MoveRootColorBy(Sample(), colors, "gold", -1);
        REQUIRE(order.has_value());
        CHECK(*order == std::vector<std::string>{"gold", "red", "white", "teal", "orange"});
        CHECK_FALSE(Data::Colors::MoveRootColorBy(Sample(), colors, "gold", 1).has_value());
        CHECK_FALSE(Data::Colors::MoveRootColorBy(Sample(), colors, "red", 1).has_value());
    }

    TEST_CASE("a removed or renamed color leaves or changes its folder")
    {
        std::vector<ColorFolder> folders = Sample();
        Data::Colors::RenameColorInFolders(folders, "red", "crimson");
        CHECK(folders[0].folders[0].colors == std::vector<std::string>{"crimson"});
        CHECK(Data::Colors::RemoveColorFromFolders(folders, "crimson"));
        CHECK(folders[0].folders[0].colors.empty());
        CHECK_FALSE(Data::Colors::RemoveColorFromFolders(folders, "crimson"));
    }

    TEST_CASE("folder names are 1 to 32 characters")
    {
        CHECK(Data::Colors::IsColorFolderName("Load"));
        CHECK(Data::Colors::IsColorFolderName(std::string(32, 'a')));
        CHECK_FALSE(Data::Colors::IsColorFolderName(std::string(33, 'a')));
        CHECK_FALSE(Data::Colors::IsColorFolderName(""));
    }

    TEST_CASE("GlowColors.json keeps its folders")
    {
        const std::vector<Data::Effects::GlowColor> colors{{"orange", {1, 0.5, 0}}, {"red", {1, 0.2, 0}}};
        const std::vector<ColorFolder> folders{{"Warm", {}, {"red"}}};
        const std::string text = Data::Effects::WriteGlowColorsJson(colors, folders);
        CHECK(text.find(R"("colorFolders": [)") < text.find(R"("colors": {)"));

        std::vector<Data::Effects::GlowColor> readColors;
        std::vector<ColorFolder> readFolders;
        std::vector<ItemDataIssue> issues;
        Data::Effects::ReadGlowColorsJson(text, "GlowColors.json", readColors, readFolders, issues);
        CHECK(issues.empty());
        CHECK(readColors == colors);
        CHECK(readFolders == folders);
        // Without folders the file is as before.
        CHECK(Data::Effects::WriteGlowColorsJson(colors).find("colorFolders") == std::string::npos);
    }
}
