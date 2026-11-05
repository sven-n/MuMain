#include "stdafx.h"

#include "doctest.h"

#include "LookRecorder.h"

#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Render/Items/ItemRenderStyles.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// The draw recorder (LookRecorder.h) on real items of the test data, and the baseline of the render styles moved
// into data.
namespace
{
using namespace Data::Looks;
using LookRecorder::Record;
using LookRecorder::Situation;

const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;

// The looks of g_LookStore for one test; none afterwards.
struct LookStoreScope
{
    explicit LookStoreScope(std::vector<LookDefinition> looks)
    {
        g_LookStore.SetRenderStyles(std::move(looks));
    }
    ~LookStoreScope()
    {
        g_LookStore.SetRenderStyles({});
    }
    LookStoreScope(const LookStoreScope&) = delete;
    LookStoreScope& operator=(const LookStoreScope&) = delete;
};

// chromeMesh0 as data: the model, then a glowing chrome pass on mesh 0.
LookDefinition ChromeMesh0(Value reflectionOpacity = FromItem{})
{
    return {"chromeMesh0",
            {Layer{}, Layer{.kind = LayerKind::Reflection,
                            .blend = Blend::Glow,
                            .meshes = {{0}},
                            .opacity = reflectionOpacity,
                            .glowMesh = 0}},
            {}};
}

// The baseline (tests/data/baseline/RenderStyleDraws.txt): one line per item and situation of each render style moved
// into data, written when it moved, after the comparison test below found its data drawing as its code, before the
// code was deleted. MU_LOOK_RECORDER_WRITE=<names, comma-separated> records those looks as they are drawn now (the data
// look of the name, or its code while the data has none) into the file.
const std::filesystem::path Baseline = MU_RENDER_STYLE_BASELINE;

constexpr const char* BaselineHeader =
    "# How the render styles moved from code into Data/Effects/RenderStyles.json draw: one line per item and\n"
    "# situation, the look, the item and the situation, then the number of mesh draws and a digest of them and of\n"
    "# what the drawing left on the model (tests/data/LookRecorder.h). Written when each look moved, once its data\n"
    "# drew as its old code (MU_LOOK_RECORDER_WRITE=<names>), so a moved look still draws as its code did.\n";

std::map<std::string, std::string> ReadBaseline()
{
    std::map<std::string, std::string> lines;
    std::ifstream in(Baseline);
    std::string line;
    while (std::getline(in, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        const size_t tab = line.rfind('\t');
        if (line.empty() || line[0] == '#' || tab == std::string::npos)
        {
            continue;
        }
        lines[line.substr(0, tab)] = line.substr(tab + 1);
    }
    return lines;
}

void WriteBaseline(const std::map<std::string, std::string>& lines)
{
    std::filesystem::create_directories(Baseline.parent_path());
    std::ofstream out(Baseline, std::ios::binary | std::ios::trunc);
    out << BaselineHeader;
    for (const auto& [key, digest] : lines)
    {
        out << key << '\t' << digest << '\n';
    }
}

std::vector<std::string> SplitNames(const char* text)
{
    std::vector<std::string> names;
    std::stringstream stream(text);
    std::string name;
    while (std::getline(stream, name, ','))
    {
        if (!name.empty())
        {
            names.push_back(name);
        }
    }
    return names;
}

// The baseline lines of a render style's items in every situation, as they draw now.
std::map<std::string, std::string> RecordLook(const std::string& name)
{
    std::map<std::string, std::string> lines;
    const std::vector<int> items = LookRecorder::ItemsUsing(name);
    LookRecorder::ShippedItems shipped;
    shipped.Open(items);
    for (const int item : items)
    {
        for (const Situation& situation : LookRecorder::Situations())
        {
            lines[name + " " + LookRecorder::DescribeItem(item) + " " + situation.Describe()] =
                LookRecorder::Digest(LookRecorder::RecordItem(item, situation));
        }
    }
    return lines;
}
} // namespace

TEST_CASE("The draw recorder draws a real item the same way twice, and the draws reach the renderer [looks][recorder]")
{
    const std::vector<int> items = LookRecorder::ItemsUsing("chromeMesh0");
    REQUIRE_FALSE(items.empty());
    const LookStoreScope looks({ChromeMesh0()});
    LookRecorder::ShippedItems shipped;
    shipped.Open(items);
    for (const Situation& situation : LookRecorder::Situations())
    {
        CAPTURE(situation.Describe());
        const Record first = LookRecorder::RecordItem(items.front(), situation);
        const Record second = LookRecorder::RecordItem(items.front(), situation);
        CHECK(first.meshDraws.size() > 1);
        CHECK(first.gpuCalls.size() > 1);
        CHECK(LookRecorder::FirstDifference(first, second).empty());
        CHECK(LookRecorder::Digest(first) == LookRecorder::Digest(second));
    }
}

TEST_CASE("The draw recorder names the first difference between two drawings [looks][recorder]")
{
    const std::vector<int> items = LookRecorder::ItemsUsing("chromeMesh0");
    const Situation plain = LookRecorder::Situations().front();
    Record before;
    Record after;
    {
        const LookStoreScope looks({ChromeMesh0()});
        LookRecorder::ShippedItems shipped;
        shipped.Open(items);
        before = LookRecorder::RecordItem(items.front(), plain);
    }
    {
        const LookStoreScope looks({ChromeMesh0(0.5f)});
        LookRecorder::ShippedItems shipped;
        shipped.Open(items);
        after = LookRecorder::RecordItem(items.front(), plain);
    }
    const std::string difference = LookRecorder::FirstDifference(before, after);
    INFO(difference);
    CHECK(difference.find("alpha 1 (code) vs 0.5 (data)") != std::string::npos);
    CHECK(difference.find(", mesh 0:") != std::string::npos);
    CHECK(LookRecorder::Digest(before) != LookRecorder::Digest(after));
}

TEST_CASE("Render styles moved into data draw as their old code did (the baseline) [looks][recorder]")
{
    std::map<std::string, std::string> baseline = ReadBaseline();
    if (const char* write = std::getenv("MU_LOOK_RECORDER_WRITE"))
    {
        RenderStylesLoadResult shipped = LoadRenderStyleFile(DataDirectory / "Effects");
        REQUIRE_FALSE(Data::Items::HasFatal(shipped.issues));
        const LookStoreScope looks(std::move(shipped.looks));
        for (const std::string& name : SplitNames(write))
        {
            std::erase_if(baseline, [&](const auto& line) { return line.first.rfind(name + " ", 0) == 0; });
            baseline.merge(RecordLook(name));
        }
        WriteBaseline(baseline);
        MESSAGE("Wrote " << Baseline.string());
        return;
    }

    RenderStylesLoadResult shipped = LoadRenderStyleFile(DataDirectory / "Effects");
    REQUIRE_FALSE(Data::Items::HasFatal(shipped.issues));
    std::set<std::string> moved;
    for (const LookDefinition& look : shipped.looks)
    {
        moved.insert(look.name);
    }
    const LookStoreScope looks(std::move(shipped.looks));

    // Each baseline line belongs to a look in the data, and each look in the data has its lines.
    std::set<std::string> withLines;
    for (const auto& [key, digest] : baseline)
    {
        const std::string name = key.substr(0, key.find(' '));
        CAPTURE(key);
        CHECK(moved.contains(name));
        withLines.insert(name);
    }
    for (const std::string& name : moved)
    {
        CAPTURE(name);
        CHECK(withLines.contains(name));
        for (const auto& [key, digest] : RecordLook(name))
        {
            CAPTURE(key);
            const auto line = baseline.find(key);
            REQUIRE(line != baseline.end());
            CHECK(digest == line->second);
        }
    }
}

TEST_CASE("Each render style in the data draws as its code while the code still has it [looks][recorder]")
{
    // A look being moved: added to the data and compared with its code here, field by field, before the code is
    // deleted (then the baseline keeps what the code drew).
    RenderStylesLoadResult shipped = LoadRenderStyleFile(DataDirectory / "Effects");
    REQUIRE_FALSE(Data::Items::HasFatal(shipped.issues));
    std::vector<std::string> moving;
    for (const LookDefinition& look : shipped.looks)
    {
        if (Render::Items::Styles::ExistsInCode(look.name))
        {
            moving.push_back(look.name);
        }
    }
    const LookStoreScope looks(std::move(shipped.looks));
    for (const std::string& name : moving)
    {
        const std::vector<int> items = LookRecorder::ItemsUsing(name);
        LookRecorder::ShippedItems shippedItems;
        shippedItems.Open(items);
        for (const int item : items)
        {
            for (const Situation& situation : LookRecorder::Situations())
            {
                CAPTURE(name + " " + LookRecorder::DescribeItem(item) + " " + situation.Describe());
                const Record code = LookRecorder::RecordItemWithCode(item, situation, name);
                const Record data = LookRecorder::RecordItem(item, situation);
                CHECK(LookRecorder::FirstDifference(code, data) == "");
            }
        }
    }
}

TEST_CASE("No render style is both in the data and in the code (design L14) [looks]")
{
    const RenderStylesLoadResult shipped = LoadRenderStyleFile(DataDirectory / "Effects");
    for (const LookDefinition& look : shipped.looks)
    {
        CAPTURE(look.name);
        CHECK_FALSE(Render::Items::Styles::ExistsInCode(look.name));
    }
}
