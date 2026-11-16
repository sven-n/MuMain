#include "stdafx.h"

#include "doctest.h"

#include "LookRecorder.h"
#include "ItemModelDatabaseScope.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Items/ItemRenderStyles.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// The draw recorder (LookRecorder.h) on real items of the test data, and the baseline of the render styles moved
// into data.
namespace
{
using namespace Data::Looks;
using LookRecorder::Field;
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
    "# situation, the look, the item and the situation, then the number of mesh draws and five digests of them and of\n"
    "# what the drawing left on the model, rounded on five grids (tests/data/LookRecorder.h). Written when each look\n"
    "# moved, once its data drew as its old code (MU_LOOK_RECORDER_WRITE=<names>), so a moved look still draws as\n"
    "# its code did.\n";

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

// The record with each number of its fields moved by `delta`, up and down by turns, as another platform's sine or a
// fused multiply and add moves them: the opacities, glow strengths, scrolls and lights.
Record Moved(Record record, double delta)
{
    double sign = 1.0;
    const auto move = [&](Field& field)
    {
        const std::string_view name = field.first;
        if (name != "alpha" && name != "blendMeshAlpha" && name != "blendMeshU" && name != "blendMeshV" &&
            name != "bodyLight")
        {
            return;
        }
        std::istringstream numbers(field.second);
        std::string moved;
        double value = 0.0;
        while (numbers >> value)
        {
            char text[40];
            std::snprintf(text, sizeof(text), "%.9g", value + sign * delta);
            moved += (moved.empty() ? "" : " ") + std::string(text);
            sign = -sign;
        }
        field.second = moved;
    };
    for (std::vector<Field>& draw : record.meshDraws)
    {
        std::ranges::for_each(draw, move);
    }
    std::ranges::for_each(record.modelAfter, move);
    return record;
}

// The baseline lines of a render style's items in every situation, as they draw now.
// A look of the shipped files, by its kind and name; its baseline lines start with the name, and an item effect's
// with "effect <name>" (a name can be in both lists, design L13).
struct ShippedLook
{
    LookKind kind;
    std::string name;

    [[nodiscard]] std::string Key() const
    {
        return kind == LookKind::ItemEffect ? "effect " + name : name;
    }
};

// The render styles and item effects of the shipped files in g_LookStore while it lives; none afterwards.
struct ShippedLooksScope
{
    ShippedLooksScope()
    {
        for (const LookKind kind : {LookKind::RenderStyle, LookKind::ItemEffect})
        {
            RenderStylesLoadResult shipped = LoadLookFile(DataDirectory / "Effects", kind);
            REQUIRE_FALSE(Data::Items::HasFatal(shipped.issues));
            for (const LookDefinition& look : shipped.looks)
            {
                looks.push_back({kind, look.name});
            }
            g_LookStore.SetLooks(kind, std::move(shipped.looks));
        }
    }
    ~ShippedLooksScope()
    {
        g_LookStore.SetRenderStyles({});
        g_LookStore.SetItemEffects({});
    }
    ShippedLooksScope(const ShippedLooksScope&) = delete;
    ShippedLooksScope& operator=(const ShippedLooksScope&) = delete;

    std::vector<ShippedLook> looks;
};

std::map<std::string, std::string> RecordLook(const ShippedLook& look)
{
    std::map<std::string, std::string> lines;
    const std::vector<int> items = LookRecorder::ItemsUsing(look.name, look.kind);
    LookRecorder::ShippedItems shipped;
    shipped.Open(items);
    for (const int item : items)
    {
        for (const Situation& situation : LookRecorder::SituationsFor(look.name, look.kind))
        {
            lines[look.Key() + " " + LookRecorder::DescribeItem(item) + " " + situation.Describe()] =
                LookRecorder::Digest(LookRecorder::RecordItem(item, situation));
        }
    }
    return lines;
}

// "effect:potion" names the item effect, any other name the render style.
ShippedLook LookNamed(const std::string& name)
{
    constexpr std::string_view Effect = "effect:";
    if (name.starts_with(Effect))
    {
        return {LookKind::ItemEffect, name.substr(Effect.size())};
    }
    return {LookKind::RenderStyle, name};
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

#ifdef _EDITOR
TEST_CASE("The Style Editor's preview without a look draws an item as one that names no render style (design L41) "
          "[looks][recorder]")
{
    const std::vector<int> items = LookRecorder::ItemsUsing("chromeMesh0");
    REQUIRE_FALSE(items.empty());
    const LookStoreScope looks({ChromeMesh0()});
    LookRecorder::ShippedItems shipped;
    shipped.Open(items);
    const int item = items.front();
    const std::vector<Situation> situations = LookRecorder::Situations();
    std::vector<Record> withLook;
    std::vector<Record> withoutLook;
    for (const Situation& situation : situations)
    {
        withLook.push_back(LookRecorder::RecordItem(item, situation));
        const Render::Items::Styles::PreviewLook preview(Data::Items::ToModelSlot(item));
        withoutLook.push_back(LookRecorder::RecordItem(item, situation));
    }
    // The look draws more: its chrome pass on mesh 0.
    CHECK_FALSE(LookRecorder::FirstDifference(withLook.front(), withoutLook.front()).empty());

    // The same item with a model entry that names no render style, its glow colors taken from the shipped file as
    // ShippedItems takes them.
    Data::Effects::GlowColorList glowColors;
    glowColors.Build(Data::Items::LoadGlowColorsFile(Data::Items::GetGlowColorsFile()).colors);
    const Data::Items::ItemModelDefinition entry = *g_ItemModelDatabase.Find(item);
    Data::Items::ItemModelDefinition plain = entry;
    plain.renderStyle.clear();
    g_ItemModelDatabase.Set(plain, glowColors);
    for (std::size_t index = 0; index < situations.size(); ++index)
    {
        CAPTURE(situations[index].Describe());
        CHECK(LookRecorder::FirstDifference(LookRecorder::RecordItem(item, situations[index]), withoutLook[index]) ==
              "");
    }
    g_ItemModelDatabase.Set(entry, glowColors);
}
#endif

TEST_CASE("A number a float step from a rounding edge keeps one of the digests [looks][recorder]")
{
    // 0.12345 is at an edge of the grid that rounds to steps of 0.0001 from half a step (1234 or 1235 by its last float
    // step) and a fifth of a step or more from the other grids' edges.
    Record below;
    Record above;
    below.modelAfter = {{"x", "0.12344999999"}};
    above.modelAfter = {{"x", "0.12345000001"}};
    const std::string first = LookRecorder::Digest(below);
    const std::string second = LookRecorder::Digest(above);
    CHECK(first != second);
    CHECK(LookRecorder::SameDigest(first, second));

    // A number near another grid's edge keeps the other hashes.
    Record lower;
    Record upper;
    lower.modelAfter = {{"x", "-0.09348999999"}};
    upper.modelAfter = {{"x", "-0.09349000001"}};
    CHECK(LookRecorder::Digest(lower) != LookRecorder::Digest(upper));
    CHECK(LookRecorder::SameDigest(LookRecorder::Digest(lower), LookRecorder::Digest(upper)));

    Record other;
    other.modelAfter = {{"x", "0.2"}};
    CHECK_FALSE(LookRecorder::SameDigest(first, LookRecorder::Digest(other)));
    // Another number of mesh draws is never the same.
    Record drawn = below;
    drawn.meshDraws.push_back({{"x", "1"}});
    CHECK_FALSE(LookRecorder::SameDigest(LookRecorder::Digest(drawn), first));
}

TEST_CASE("Numbers a float step moves across the edges of several grids keep the digest of a grid none is at "
          "[looks][recorder]")
{
    // A drawing has many numbers, and those a platform moves (a sine's light channels, its opacity) can each sit at
    // an edge of another grid: one at each of four grids' edges, and three more at the edges of steps of 0.00001 from
    // half, a quarter and three quarters of a step.
    const double edges[] = {0.12345, 0.12343, 0.12341, 0.12349, 0.123455, 0.1234525, 0.1234575};
    constexpr double FloatStep = 1.0e-11;
    Record below;
    Record above;
    for (const double edge : edges)
    {
        char text[32];
        std::snprintf(text, sizeof(text), "%.13f", edge - FloatStep);
        below.modelAfter.push_back({"x", text});
        std::snprintf(text, sizeof(text), "%.13f", edge + FloatStep);
        above.modelAfter.push_back({"x", text});
    }
    CHECK(LookRecorder::SameDigest(LookRecorder::Digest(below), LookRecorder::Digest(above)));
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
        const ShippedLooksScope looks;
        for (const std::string& name : SplitNames(write))
        {
            const ShippedLook look = LookNamed(name);
            // A line is "<key> (group,number) <situation>": an item follows the key.
            std::erase_if(baseline, [&](const auto& line) { return line.first.rfind(look.Key() + " (", 0) == 0; });
            baseline.merge(RecordLook(look));
        }
        WriteBaseline(baseline);
        MESSAGE("Wrote " << Baseline.string());
        return;
    }

    const ShippedLooksScope looks;
    std::set<std::string> moved;
    for (const ShippedLook& look : looks.looks)
    {
        moved.insert(look.Key());
    }

    // Each baseline line belongs to a look in the data, and each look in the data has its lines.
    std::set<std::string> withLines;
    for (const auto& [key, digest] : baseline)
    {
        const std::string name = key.substr(0, key.find(" ("));
        CAPTURE(key);
        CHECK(moved.contains(name));
        withLines.insert(name);
    }
    for (const ShippedLook& look : looks.looks)
    {
        CAPTURE(look.Key());
        CHECK(withLines.contains(look.Key()));
        for (const auto& [key, digest] : RecordLook(look))
        {
            CAPTURE(key);
            const auto line = baseline.find(key);
            REQUIRE(line != baseline.end());
            INFO(digest << " drawn, " << line->second << " kept");
            CHECK(LookRecorder::SameDigest(digest, line->second));
        }
    }
}

TEST_CASE("Every baseline line keeps one of its digests when its numbers move by a platform's float steps "
          "[looks][recorder]")
{
    // A sine is off by its last float step on another platform (about 1e-7 near 1), and a multiply and add the compiler
    // fuses (ARM64) by a step of the result. Twice that, up and down by turns, keeps one of the hashes on every line.
    constexpr double PlatformError = 2.0e-7;
    const ShippedLooksScope looks;
    for (const ShippedLook& look : looks.looks)
    {
        const std::vector<int> items = LookRecorder::ItemsUsing(look.name, look.kind);
        LookRecorder::ShippedItems shippedItems;
        shippedItems.Open(items);
        for (const int item : items)
        {
            for (const Situation& situation : LookRecorder::SituationsFor(look.name, look.kind))
            {
                CAPTURE(look.Key() + " " + LookRecorder::DescribeItem(item) + " " + situation.Describe());
                const Record record = LookRecorder::RecordItem(item, situation);
                const std::string digest = LookRecorder::Digest(record);
                CHECK(LookRecorder::SameDigest(LookRecorder::Digest(Moved(record, PlatformError)), digest));
                CHECK(LookRecorder::SameDigest(LookRecorder::Digest(Moved(record, -PlatformError)), digest));
            }
        }
    }
}

TEST_CASE("Each render style in the data draws as its code while the code still has it [looks][recorder]")
{
    // A look being moved: added to the data and compared with its code here, field by field, before the code is
    // deleted (then the baseline keeps what the code drew).
    const ShippedLooksScope looks;
    for (const ShippedLook& look : looks.looks)
    {
        const bool inCode = look.kind == LookKind::ItemEffect ? Render::Items::ItemEffects::ExistsInCode(look.name)
                                                              : Render::Items::Styles::ExistsInCode(look.name);
        if (!inCode)
        {
            continue;
        }
        const std::vector<int> items = LookRecorder::ItemsUsing(look.name, look.kind);
        LookRecorder::ShippedItems shippedItems;
        shippedItems.Open(items);
        for (const int item : items)
        {
            for (const Situation& situation : LookRecorder::SituationsFor(look.name, look.kind))
            {
                CAPTURE(look.Key() + " " + LookRecorder::DescribeItem(item) + " " + situation.Describe());
                const Record code = LookRecorder::RecordItemWithCode(item, situation, look.name, look.kind);
                const Record data = LookRecorder::RecordItem(item, situation);
                CHECK(LookRecorder::FirstDifference(code, data) == "");
            }
        }
    }
}

TEST_CASE("No render style or item effect is both in the data and in the code (design L14) [looks]")
{
    const RenderStylesLoadResult styles = LoadRenderStyleFile(DataDirectory / "Effects");
    for (const LookDefinition& look : styles.looks)
    {
        CAPTURE(look.name);
        CHECK_FALSE(Render::Items::Styles::ExistsInCode(look.name));
    }
    const RenderStylesLoadResult effects = LoadLookFile(DataDirectory / "Effects", LookKind::ItemEffect);
    REQUIRE_FALSE(Data::Items::HasFatal(effects.issues));
    for (const LookDefinition& look : effects.looks)
    {
        CAPTURE(look.name);
        CHECK_FALSE(Render::Items::ItemEffects::ExistsInCode(look.name));
    }
}

TEST_CASE("The Devil's Eye puts the pulse of its code on its object in the update, within float steps (design L45) "
          "[looks][recorder]")
{
    const ShippedLooksScope looks;
    REQUIRE(g_LookStore.FindItemEffect("devilsEye") != nullptr);
    ItemModelDatabaseScope database;
    Data::Items::ItemModelDefinition eye;
    eye.group = ITEM_DEVILS_EYE / MAX_ITEM_INDEX;
    eye.number = ITEM_DEVILS_EYE % MAX_ITEM_INDEX;
    eye.file = "Data/Item/devil_eye.bmd";
    eye.itemEffect = "devilsEye";
    g_ItemModelDatabase.Build(std::span(&eye, 1), Data::Effects::GlowColorList{});
    const int slot = Data::Items::ToModelSlot(ITEM_DEVILS_EYE);
    REQUIRE(Render::Items::ItemEffects::Spawns(slot));

    const double previousWorldTime = WorldTime;
    for (const Situation& situation : LookRecorder::Situations())
    {
        WorldTime = situation.worldTime;
        CAPTURE(WorldTime);
        BMD model;
        OBJECT object;
        Render::Items::ItemEffects::Spawn(&model, &object, slot, nullptr);
        // SpawnDevilsEye as the code wrote it; the data's wave rounds in another order, a few float steps away.
        const float luminosity = (float)sinf(WorldTime * 0.002f) * 10.f + 15.65f;
        const float floatStep = std::nextafter(5.f, 6.f) - 5.f;
        for (int channel = 0; channel < 3; ++channel)
        {
            CHECK(std::fabs(object.Light[channel] - luminosity / 5.0f) <= 4.f * floatStep);
        }
    }
    WorldTime = previousWorldTime;
}
