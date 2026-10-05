#include "stdafx.h"

#include "doctest.h"

#include "EffectSourceCases.h"
#include "EffectSourceCondition.h"
#include "EffectTestData.h"

#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Render/Effects/EffectRegistry.h"

#ifdef _EDITOR
#include "Data/GameData/EffectData/EffectCreateParamsJson.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Render/Effects/Behaviors/EffectBehaviors.h"
#include "Core/Utilities/AssetLoadWorld.h"
#include "UI/EffectBrowser/EffectBrowserModel.h"
#include "UI/EffectBrowser/EffectLegacyCases.h"
#include "UI/EffectBrowser/EffectPreviewObject.h"
#include "UI/EffectBrowser/EffectWorldPreview.h"
#include "World/MapInfra/MapManager.h"
#endif

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace Data::Effects;
using EffectSourceCases::Stage;

namespace
{
// The macros of the #ifdef blocks in CreateEffect, MoveEffect, RenderEffects,
// RenderEffectShadows and RenderAfterEffects, as the game is built
// (Defined_Global.h through stdafx.h).
#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
constexpr bool InfluenceGroundEffect = true;
#else
constexpr bool InfluenceGroundEffect = false;
#endif
#ifdef ASG_ADD_KARUTAN_MONSTERS
constexpr bool KarutanMonsters = true;
#else
constexpr bool KarutanMonsters = false;
#endif
#ifdef PJH_ADD_PANDA_CHANGERING
constexpr bool PandaChangeRing = true;
#else
constexpr bool PandaChangeRing = false;
#endif
#ifdef PBG_ADD_CHARACTERSLOT
constexpr bool CharacterSlot = true;
#else
constexpr bool CharacterSlot = false;
#endif
#ifdef ENABLE_POTION_EFFECT
constexpr bool PotionEffect = true;
#else
constexpr bool PotionEffect = false;
#endif
#ifdef GUILD_WAR_EVENT
constexpr bool GuildWarEvent = true;
#else
constexpr bool GuildWarEvent = false;
#endif

constexpr std::array<EffectSourceCases::MacroState, 6> EffectMacros = {{
    {"ASG_ADD_INFLUENCE_GROUND_EFFECT", InfluenceGroundEffect},
    {"ASG_ADD_KARUTAN_MONSTERS", KarutanMonsters},
    {"PJH_ADD_PANDA_CHANGERING", PandaChangeRing},
    {"PBG_ADD_CHARACTERSLOT", CharacterSlot},
    {"ENABLE_POTION_EFFECT", PotionEffect},
    {"GUILD_WAR_EVENT", GuildWarEvent},
}};

constexpr std::array<Stage, 5> Stages = {Stage::Create, Stage::Move, Stage::Render, Stage::Ground,
                                         Stage::AfterCharacters};

const char* NameOf(Stage stage)
{
    switch (stage)
    {
    case Stage::Create:
        return "CreateEffect";
    case Stage::Move:
        return "MoveEffect";
    case Stage::Render:
        return "RenderEffects";
    case Stage::Ground:
        return "RenderEffectShadows";
    case Stage::AfterCharacters:
        break;
    }
    return "RenderAfterEffects";
}

// The number of a label: the code of an effect symbol, or a number of an
// effect symbol written as a literal (`case 9:`); -1 for anything else.
int ResolveLabel(const std::string& label)
{
    const std::span<const EffectTypeSymbol> symbols = GetEffectTypeSymbols(EffectKind::Effect);
    const auto byCode = std::find_if(symbols.begin(), symbols.end(),
                                     [&](const EffectTypeSymbol& symbol) { return symbol.code == label; });
    if (byCode != symbols.end())
        return byCode->type;
    int number = -1;
    const auto [end, error] = std::from_chars(label.data(), label.data() + label.size(), number);
    const bool isNumber = error == std::errc() && end == label.data() + label.size();
    const bool isSymbol = std::any_of(symbols.begin(), symbols.end(),
                                      [&](const EffectTypeSymbol& symbol) { return symbol.type == number; });
    return isNumber && isSymbol ? number : -1;
}

// The code of an effect type, for the messages of failed checks.
std::string CodeOf(int type)
{
    const std::span<const EffectTypeSymbol> symbols = GetEffectTypeSymbols(EffectKind::Effect);
    const auto found = std::find_if(symbols.begin(), symbols.end(),
                                    [type](const EffectTypeSymbol& symbol) { return symbol.type == type; });
    return found != symbols.end() ? std::string(found->code) : std::to_string(type);
}

// The effect types with a case in the switch of `stage`.
std::set<int> ReadCases(const std::string& source, Stage stage)
{
    const EffectSourceCases::SwitchLabels labels = EffectSourceCases::ReadSwitchLabels(source, stage, EffectMacros);
    INFO(std::string(NameOf(stage)));
    for (const std::string& problem : labels.problems)
    {
        FAIL_CHECK(problem);
    }
    CHECK_FALSE(labels.labels.empty());
    std::set<int> types;
    for (const std::string& label : labels.labels)
    {
        INFO(label);
        const int type = ResolveLabel(label);
        CHECK(type >= 0);
        types.insert(type);
    }
    return types;
}

std::map<Stage, std::set<int>> ReadAllCases()
{
    const std::string source =
        EffectSourceCases::ReadEffectSource(std::filesystem::path(MU_TEST_SOURCE_DIR) / "Render/Effects/ZzzEffect.cpp");
    REQUIRE_FALSE(source.empty());
    std::map<Stage, std::set<int>> cases;
    for (const Stage stage : Stages)
    {
        cases[stage] = ReadCases(source, stage);
    }
    return cases;
}

// Whether the registry handles the stage of `type`, so its switch case would
// never run. RenderEffectShadows never asks the registry. RenderAfterEffects
// draws what the case of RenderEffects asked for, so a draw handler leaves its
// case behind: the handler has to take that drawing along.
bool RegistryHandles(Stage stage, int type)
{
    const Render::Effects::EffectDescriptor* descriptor = Render::Effects::Lookup(type);
    if (descriptor == nullptr)
        return false;
    switch (stage)
    {
    case Stage::Create:
        return descriptor->create.has_value() || descriptor->onCreate != nullptr;
    case Stage::Move:
        return descriptor->move != nullptr;
    case Stage::Render:
    case Stage::AfterCharacters:
        return descriptor->render != nullptr;
    case Stage::Ground:
        break;
    }
    return false;
}
} // namespace

// A phase that moves a stage into the registry deletes the case; a case left
// behind would never run. Also: every label names an effect symbol, so the
// symbol list misses no type the code handles.
TEST_CASE("The legacy switches of ZzzEffect.cpp have cases only for stages the registry does not handle "
          "[data][effects]")
{
    EffectTestData::BuildShippedRegistry();
    for (const auto& [stage, types] : ReadAllCases())
    {
        INFO(std::string(NameOf(stage)));
        for (const int type : types)
        {
            INFO(CodeOf(type));
            CHECK_FALSE(RegistryHandles(stage, type));
        }
    }
}

#ifdef _EDITOR
using namespace MuEditor::Effects;

namespace
{
bool IsBelow(const EffectLegacyCases& left, const EffectLegacyCases& right)
{
    return left.type < right.type;
}

std::uint8_t FlagOf(Stage stage)
{
    switch (stage)
    {
    case Stage::Create:
        return CreateCase;
    case Stage::Move:
        return MoveCase;
    case Stage::Render:
        return RenderCase;
    case Stage::Ground:
        return GroundCase;
    case Stage::AfterCharacters:
        break;
    }
    return AfterCharactersCase;
}

EffectTypeCatalogue ShippedCatalogue()
{
    EffectTypeCatalogue catalogue;
    for (const EffectKind kind : EffectKinds)
    {
        catalogue.Build(kind, EffectTestData::ShippedTypes().types[ToIndex(kind)]);
    }
    return catalogue;
}

EffectBrowserModel ShippedModel()
{
    EffectTestData::BuildShippedRegistry();
    EffectBrowserModel model;
    model.Build(ShippedCatalogue(), &Render::Effects::Lookup);
    return model;
}

std::vector<std::string_view> NamesOf(const EffectBrowserModel& model, EffectKind kind, const std::vector<int>& rows)
{
    std::vector<std::string_view> names;
    for (const int row : rows)
    {
        names.push_back(model.GetRows(kind)[row].name);
    }
    return names;
}

// The fields WriteEffectCreateParams writes for `params`, the fields of
// "offset" and "copy" each on its own, as the creation table names them.
std::vector<std::string> WrittenFields(const EffectCreateParams& params)
{
    std::vector<std::string> fields;
    const Data::Items::Json::OrderedJson written = WriteEffectCreateParams(params);
    for (const auto& [key, value] : written.items())
    {
        if (key != "offset" && key != "copy")
        {
            fields.push_back(key);
            continue;
        }
        for (const auto& [field, fieldValue] : value.items())
        {
            fields.push_back(key + "." + field);
        }
    }
    return fields;
}

// Whether the filter lists the effect named `name`.
bool MatchesNames(const EffectBrowserModel& model, const EffectBrowserFilter& filter, std::string_view name)
{
    const std::vector<std::string_view> names =
        NamesOf(model, EffectKind::Effect, model.Filter(EffectKind::Effect, filter));
    return std::find(names.begin(), names.end(), name) != names.end();
}

const EffectCreateTable::Line* FindLine(const EffectCreateTable& table, std::string_view field)
{
    const auto found = std::find_if(table.lines.begin(), table.lines.end(),
                                    [&](const EffectCreateTable::Line& line) { return line.field == field; });
    return found != table.lines.end() ? &*found : nullptr;
}

// The bounds of the type range of MoveEffect's shared code, which are no
// effect types.
constexpr std::array<std::pair<std::string_view, int>, 2> RangeBounds = {{
    {"MODEL_BIRD01", MODEL_BIRD01},
    {"MODEL_SKILL_END", MODEL_SKILL_END},
}};

// A name in the conditions of MoveEffect's shared code: an effect symbol or a
// bound of the range. Looked up in a table, as the test evaluates the
// conditions for many types and SubTypes.
std::optional<int> ValueOfName(std::string_view name)
{
    static const std::map<std::string, int, std::less<>> values = []
    {
        std::map<std::string, int, std::less<>> byName(RangeBounds.begin(), RangeBounds.end());
        for (const EffectTypeSymbol& symbol : GetEffectTypeSymbols(EffectKind::Effect))
            byName.emplace(symbol.code, symbol.type);
        return byName;
    }();
    const auto found = values.find(name);
    return found != values.end() ? std::optional<int>(found->second) : std::nullopt;
}
} // namespace

TEST_CASE("The effect browser's list of the legacy cases is the cases of the switches [data][effects][editor]")
{
    const std::span<const EffectLegacyCases> list = GetEffectLegacyCases();
    CHECK(std::adjacent_find(list.begin(), list.end(),
                             [](const auto& left, const auto& right) { return !IsBelow(left, right); }) == list.end());

    std::map<int, std::uint8_t> fromSource;
    for (const auto& [stage, types] : ReadAllCases())
    {
        for (const int type : types)
        {
            fromSource[type] |= FlagOf(stage);
        }
    }
    for (const auto& [type, cases] : fromSource)
    {
        INFO(CodeOf(type));
        CHECK(FindEffectLegacyCases(type) == cases);
    }
    for (const EffectLegacyCases& entry : list)
    {
        INFO(CodeOf(entry.type));
        const auto found = fromSource.find(entry.type);
        CHECK(found != fromSource.end());
        CHECK((found != fromSource.end() ? found->second : 0) == entry.cases);
    }
    CHECK(FindEffectLegacyCases(MODEL_BLOOD) == 0);
}

// The preview animates a model where the code MoveEffect runs after its switch
// does. The test evaluates the conditions of that code and finds the cases
// that return before it, so a type a phase takes out of them or adds fails
// here until IsAnimatedByMoveEffect follows.
TEST_CASE("The effect preview animates the models that MoveEffect's shared code animates [data][effects][editor]")
{
    const std::string source =
        EffectSourceCases::ReadEffectSource(std::filesystem::path(MU_TEST_SOURCE_DIR) / "Render/Effects/ZzzEffect.cpp");
    REQUIRE_FALSE(source.empty());
    const EffectSourceCases::SharedMoveConditions conditions =
        EffectSourceCases::ReadSharedMoveConditions(source, EffectMacros);
    for (const std::string& problem : conditions.problems)
    {
        FAIL_CHECK(problem);
    }
    REQUIRE(conditions.problems.empty());

    // A case that returns first skips that code.
    const EffectSourceCases::SwitchLabels returning =
        EffectSourceCases::ReadCasesThatReturn(source, Stage::Move, EffectMacros);
    for (const std::string& problem : returning.problems)
    {
        FAIL_CHECK(problem);
    }
    std::set<int> returnsFirst;
    for (const std::string& label : returning.labels)
    {
        INFO(label);
        const int type = ResolveLabel(label);
        CHECK(type >= 0);
        returnsFirst.insert(type);
    }

    // The names do not depend on the type, so one reading finds every problem
    // and every number. A comparison of the SubType with a number changes at
    // that number; the range covers SubTypes only the copy might compare with.
    std::set<int> subTypes;
    for (int subType = -1; subType <= 255; ++subType)
    {
        subTypes.insert(subType);
    }
    for (const std::string& condition : {conditions.skipped, conditions.animated})
    {
        const EffectSourceCases::ConditionValue first =
            EffectSourceCases::EvaluateCondition(condition, 0, 0, ValueOfName);
        for (const std::string& problem : first.problems)
        {
            FAIL_CHECK(problem);
        }
        REQUIRE(first.problems.empty());
        for (const int number : first.numbers)
        {
            subTypes.insert({number - 1, number, number + 1});
        }
    }

    std::set<int> types;
    for (const EffectTypeSymbol& symbol : GetEffectTypeSymbols(EffectKind::Effect))
    {
        types.insert(symbol.type);
    }
    for (const auto& bound : RangeBounds)
    {
        types.insert({bound.second - 1, bound.second});
    }
    for (const int type : types)
    {
        INFO(CodeOf(type));
        for (const int subType : subTypes)
        {
            INFO(subType);
            const bool skipped =
                EffectSourceCases::EvaluateCondition(conditions.skipped, type, subType, ValueOfName).value;
            const bool animated =
                EffectSourceCases::EvaluateCondition(conditions.animated, type, subType, ValueOfName).value;
            CHECK(IsAnimatedByMoveEffect(type, subType) == (!returnsFirst.contains(type) && !skipped && animated));
        }
    }
}

// Code of an effect that changes the character or reaches the server whoever
// owns the effect: the world preview refuses its type. The test finds the
// code in ZzzEffect.cpp's switches and in the move handlers (Move_<code>) and
// checks that each type is refused; elsewhere in the effect code it may not
// be.
TEST_CASE("The world preview refuses every type whose code changes the character or tells the server "
          "[data][effects][editor]")
{
    constexpr std::array<std::string_view, 4> Markers = {"(JOINT*)o->Owner", "SetPlayerStop(Hero",
                                                         "CollisionHeroCharacter(", "SocketClient->"};
    const std::filesystem::path effects = std::filesystem::path(MU_TEST_SOURCE_DIR) / "Render/Effects";
    const std::span<const RefusedWorldPreview> refused = GetRefusedWorldPreviews();
    const auto isRefused = [&](int type)
    {
        return std::any_of(refused.begin(), refused.end(),
                           [type](const RefusedWorldPreview& entry) { return entry.type == type; });
    };
    // Occurrences in a text read by ReadEffectSource, spaces left out.
    const auto count = [](const std::string& source, std::string_view marker)
    {
        std::string compact;
        std::copy_if(source.begin(), source.end(), std::back_inserter(compact),
                     [](char c) { return std::isspace(static_cast<unsigned char>(c)) == 0; });
        int found = 0;
        for (size_t at = compact.find(marker); at != std::string::npos; at = compact.find(marker, at + 1))
            ++found;
        return found;
    };

    const std::string zzzEffect = EffectSourceCases::ReadEffectSource(effects / "ZzzEffect.cpp");
    REQUIRE_FALSE(zzzEffect.empty());
    for (const std::string_view marker : Markers)
    {
        INFO(marker);
        int inCases = 0;
        for (const Stage stage : {Stage::Create, Stage::Move, Stage::Render})
        {
            const EffectSourceCases::SwitchLabels cases =
                EffectSourceCases::ReadCasesContaining(zzzEffect, stage, EffectMacros, marker);
            for (const std::string& problem : cases.problems)
            {
                FAIL_CHECK(problem);
            }
            for (const std::string& label : cases.labels)
            {
                INFO(label);
                CHECK(isRefused(ResolveLabel(label)));
            }
            inCases += static_cast<int>(cases.labels.size());
        }
        // One occurrence per case; more fail here, to be looked at.
        CHECK(count(zzzEffect, marker) == inCases);
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(effects))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".cpp" ||
            entry.path().filename() == "ZzzEffect.cpp")
            continue;
        INFO(entry.path().string());
        const std::string text = EffectSourceCases::ReadEffectSource(entry.path());
        for (const std::string_view marker : Markers)
        {
            INFO(marker);
            if (count(text, marker) == 0)
                continue;
            // Each occurrence lies in the move handler of a refused type.
            std::string compact;
            std::copy_if(text.begin(), text.end(), std::back_inserter(compact),
                         [](char c) { return std::isspace(static_cast<unsigned char>(c)) == 0; });
            for (size_t at = compact.find(marker); at != std::string::npos; at = compact.find(marker, at + 1))
            {
                constexpr std::string_view Handler = "boolMove_";
                const size_t start = compact.rfind(Handler, at);
                REQUIRE(start != std::string::npos);
                const size_t name = start + Handler.size();
                const std::string code = compact.substr(name, compact.find('(', name) - name);
                INFO(code);
                CHECK(isRefused(ResolveLabel(code)));
            }
        }
    }
}

TEST_CASE("The effect browser takes a stage from the registry, then the legacy case [data][effects][editor]")
{
    Render::Effects::EffectDescriptor descriptor;
    const std::uint8_t allCases = CreateCase | MoveCase | RenderCase;
    CHECK(DescribeEffectStages(BITMAP_LIGHT, nullptr, 0) ==
          EffectStages{CreateStage::SetupOnly, MoveStage::SharedCodeOnly, RenderStage::NotDrawn});
    CHECK(DescribeEffectStages(BITMAP_LIGHT, nullptr, allCases) ==
          EffectStages{CreateStage::Switch, MoveStage::Switch, RenderStage::Switch});
    // RenderEffects' default draws the skill models.
    CHECK(DescribeEffectStages(MODEL_SKILL_BEGIN + 1, &descriptor, 0).render == RenderStage::DrawnAsModel);
    CHECK(DescribeEffectStages(MODEL_SKILL_END, &descriptor, 0).render == RenderStage::NotDrawn);
    // RenderEffectShadows draws on the ground, besides any other stage.
    CHECK(DescribeEffectStages(BITMAP_LIGHT, nullptr, GroundCase) ==
          EffectStages{CreateStage::SetupOnly, MoveStage::SharedCodeOnly, RenderStage::OnGround, true});
    CHECK(DescribeEffectStages(BITMAP_LIGHT, nullptr, static_cast<std::uint8_t>(RenderCase | GroundCase)) ==
          EffectStages{CreateStage::SetupOnly, MoveStage::SharedCodeOnly, RenderStage::Switch, true});
    // RenderAfterEffects draws again what the case of RenderEffects asked for.
    CHECK(DescribeEffectStages(BITMAP_LIGHT, nullptr, static_cast<std::uint8_t>(RenderCase | AfterCharactersCase)) ==
          EffectStages{CreateStage::SetupOnly, MoveStage::SharedCodeOnly, RenderStage::Switch, false, true});

    descriptor.create = Render::Effects::CreateParams{};
    descriptor.move = &Render::Effects::Behaviors::MoveSpear;
    descriptor.render = &Render::Effects::Behaviors::RenderDefault;
    CHECK(DescribeEffectStages(BITMAP_LIGHT, &descriptor, allCases) ==
          EffectStages{CreateStage::Data, MoveStage::Handler, RenderStage::Handler});
    descriptor.onCreate = &Render::Effects::Behaviors::CreateMayaStone45;
    CHECK(DescribeEffectStages(BITMAP_LIGHT, &descriptor, 0).create == CreateStage::DataThenHook);
    descriptor.create.reset();
    CHECK(DescribeEffectStages(BITMAP_LIGHT, &descriptor, 0).create == CreateStage::Hook);
}

TEST_CASE("The effect browser lists the types of every kind with name, code, stages and slot [data][effects][editor]")
{
    const EffectBrowserModel model = ShippedModel();
    REQUIRE(model.IsBuilt());
    for (const EffectKind kind : EffectKinds)
    {
        INFO(GetEffectKindName(kind));
        const std::span<const EffectBrowserRow> rows = model.GetRows(kind);
        CHECK(rows.size() == GetEffectTypeSymbols(kind).size());
        CHECK(std::none_of(rows.begin(), rows.end(), [](const EffectBrowserRow& row) { return row.name.empty(); }));
    }

    const EffectBrowserRow* ghost = model.FindRow(EffectKind::Effect, MODEL_CUNDUN_GHOST);
    REQUIRE(ghost != nullptr);
    CHECK(ghost->name == "kundunGhost");
    CHECK(ghost->code == "MODEL_CUNDUN_GHOST");
    CHECK(ghost->stages.create == CreateStage::Data);
    CHECK(ghost->assetSlot == EffectAssetSlot::Model);
    CHECK(model.FindRow(EffectKind::Particle, MODEL_CUNDUN_GHOST) == nullptr);

    CHECK(model.FindRow(EffectKind::Effect, MODEL_MAYASTONE4)->stages.create == CreateStage::Hook);
    CHECK(model.FindRow(EffectKind::Effect, MODEL_DESAIR)->stages.render == RenderStage::Handler);
    const EffectBrowserRow* fallingStone = model.FindRow(EffectKind::Effect, MODEL_KALIMA_FALLING_STONE);
    CHECK(fallingStone->stages == EffectStages{CreateStage::Switch, MoveStage::Switch, RenderStage::Switch});
    CHECK(IsWorldObjectSlot(fallingStone->assetSlot, fallingStone->type));
    CHECK_FALSE(IsWorldObjectSlot(ghost->assetSlot, ghost->type));
    // RenderEffectShadows draws these on the ground; the second one is also a
    // case of RenderEffects.
    CHECK(model.FindRow(EffectKind::Effect, BITMAP_SHOCK_WAVE)->stages.render == RenderStage::OnGround);
    const EffectStages raklionMagic = model.FindRow(EffectKind::Effect, MODEL_RAKLION_BOSS_MAGIC)->stages;
    CHECK(raklionMagic.render == RenderStage::Switch);
    CHECK(raklionMagic.drawnOnGround);
    CHECK(model.FindRow(EffectKind::Effect, MODEL_MAYASTAR)->stages.drawnAfterCharacters);

    // Each row of the catalogue is a type whose creation is data.
    const std::span<const EffectBrowserRow> effects = model.GetRows(EffectKind::Effect);
    const auto withData = std::count_if(effects.begin(), effects.end(), [](const EffectBrowserRow& row)
                                        { return row.stages.create == CreateStage::Data; });
    CHECK(static_cast<size_t>(withData) == ShippedCatalogue().GetCreateParams().size());

    CHECK(model.FindRow(EffectKind::Effect, BITMAP_SHOTGUN)->assetSlot == EffectAssetSlot::Texture);
    CHECK(model.FindRow(EffectKind::Sprite, BITMAP_LIGHT)->assetSlot == EffectAssetSlot::Texture);
    CHECK(model.FindRow(EffectKind::Particle, BITMAP_LIGHT)->assetSlot == EffectAssetSlot::DefaultTexture);
    CHECK(model.FindRow(EffectKind::Joint, MODEL_SPEARSKILL)->assetSlot == EffectAssetSlot::TextureChosenInCode);
}

TEST_CASE("The effect browser filters by search, stage and loaded asset [data][effects][editor]")
{
    EffectBrowserModel model = ShippedModel();
    EffectBrowserFilter filter;
    filter.search = "kundunghost";
    CHECK(NamesOf(model, EffectKind::Effect, model.Filter(EffectKind::Effect, filter)) ==
          std::vector<std::string_view>{"kundunGhost"});
    filter.search = "model_cundun_ghost";
    CHECK(model.Filter(EffectKind::Effect, filter).size() == 1);
    filter.search = std::to_string(BITMAP_SHOTGUN);
    CHECK(NamesOf(model, EffectKind::Particle, model.Filter(EffectKind::Particle, filter)) ==
          std::vector<std::string_view>{"advSmoke2"});

    filter = {};
    filter.create = CreateStage::Hook;
    CHECK(NamesOf(model, EffectKind::Effect, model.Filter(EffectKind::Effect, filter)) ==
          std::vector<std::string_view>{"mayaStone4", "mayaStone5"});
    // Particles have no stages; the stage filters leave them all.
    CHECK(model.Filter(EffectKind::Particle, filter).size() == model.GetRows(EffectKind::Particle).size());
    filter.create.reset();
    filter.render = RenderStage::Handler;
    CHECK(NamesOf(model, EffectKind::Effect, model.Filter(EffectKind::Effect, filter)) ==
          std::vector<std::string_view>{"desair"});
    // "On the ground" also lists the effects drawn there besides another stage.
    filter.render = RenderStage::OnGround;
    const std::span<const EffectLegacyCases> cases = GetEffectLegacyCases();
    const auto onGround = std::count_if(cases.begin(), cases.end(),
                                        [](const EffectLegacyCases& entry) { return (entry.cases & GroundCase) != 0; });
    CHECK(model.Filter(EffectKind::Effect, filter).size() == static_cast<size_t>(onGround));

    // The poison model was loaded by this map, the fire model by an earlier
    // one, the ghost's on the loading screen; slot 64 holds this map's object,
    // which is the wall's model only on the siege map.
    static constexpr int ThisWorld = 0;
    static constexpr int EarlierWorld = 2;
    const auto probe = [](int wallWorld)
    {
        return [wallWorld](EffectAssetSlot slot, int type) -> EffectAsset
        {
            if (slot != EffectAssetSlot::Model)
                return {};
            switch (type)
            {
            case MODEL_CUNDUN_GHOST:
                return {true, "Data/Skill/x.bmd", Core::AssetLoadWorld::LoadingScreen};
            case MODEL_POISON:
                return {true, "Data/Skill/Poison01.bmd", ThisWorld};
            case MODEL_FIRE:
                return {true, "Data/Skill/Fire01.bmd", EarlierWorld};
            case BATTLE_CASTLE_WALL4:
                return {true, "Data/Object/Object65.bmd", wallWorld};
            default:
                return {};
            }
        };
    };
    filter = {};
    filter.assets = AssetFilter::LoadedNow;
    CHECK(model.Filter(EffectKind::Effect, filter).empty());
    const int generation = model.GetAssetGeneration();
    model.RefreshAssets(probe(ThisWorld), ThisWorld);
    CHECK(model.GetAssetGeneration() != generation);
    CHECK(NamesOf(model, EffectKind::Effect, model.Filter(EffectKind::Effect, filter)) ==
          std::vector<std::string_view>{"fireModel", "poison", "kundunGhost"});
    filter.assets = AssetFilter::LoadedAtStart;
    CHECK(NamesOf(model, EffectKind::Effect, model.Filter(EffectKind::Effect, filter)) ==
          std::vector<std::string_view>{"kundunGhost"});
    filter.assets = AssetFilter::LoadedByThisMap;
    CHECK(NamesOf(model, EffectKind::Effect, model.Filter(EffectKind::Effect, filter)) ==
          std::vector<std::string_view>{"poison"});
    CHECK(model.FindRow(EffectKind::Effect, MODEL_FIRE)->assetOrigin == AssetOrigin::EarlierMap);
    CHECK(model.FindRow(EffectKind::Effect, MODEL_CUNDUN_GHOST)->asset.file == "Data/Skill/x.bmd");
    CHECK(model.FindRow(EffectKind::Effect, BATTLE_CASTLE_WALL4)->foreignMapObject);

    // On the siege map the wall's slot holds the wall.
    model.RefreshAssets(probe(WD_30BATTLECASTLE), WD_30BATTLECASTLE);
    const EffectBrowserRow* wall = model.FindRow(EffectKind::Effect, BATTLE_CASTLE_WALL4);
    CHECK_FALSE(wall->foreignMapObject);
    CHECK(wall->assetOrigin == AssetOrigin::ThisMap);
    CHECK(MatchesNames(model, filter, "battleCastleWall4"));
}

TEST_CASE("The effect browser tells what loaded an asset and which map a map-object effect belongs to "
          "[data][effects][editor]")
{
    CHECK(ClassifyAssetOrigin({}, 0) == AssetOrigin::NotLoaded);
    CHECK(ClassifyAssetOrigin({true, "a.bmd", std::nullopt}, 0) == AssetOrigin::Unknown);
    CHECK(ClassifyAssetOrigin({true, "a.bmd", Core::AssetLoadWorld::LoadingScreen}, 0) == AssetOrigin::LoadingScreen);
    CHECK(ClassifyAssetOrigin({true, "a.bmd", 5}, 5) == AssetOrigin::ThisMap);
    CHECK(ClassifyAssetOrigin({true, "a.bmd", 5}, 6) == AssetOrigin::EarlierMap);
    // Without the editor's source every asset counts as loaded on the loading
    // screen.
    CHECK(Core::AssetLoadWorld::Get() == Core::AssetLoadWorld::LoadingScreen);

    CHECK(GetHomeWorld(BATTLE_CASTLE_WALL1) == WD_30BATTLECASTLE);
    CHECK(GetHomeWorld(MODEL_KALIMA_FALLING_STONE) == WD_24HELLAS);
    CHECK_FALSE(GetHomeWorld(MODEL_POISON).has_value());
    CHECK(IsHomeWorld(BATTLE_CASTLE_WALL3, WD_30BATTLECASTLE));
    CHECK_FALSE(IsHomeWorld(BATTLE_CASTLE_WALL3, 0));
    CHECK(IsHomeWorld(MODEL_KALIMA_FALLING_STONE, WD_24HELLAS_7));
    CHECK_FALSE(IsHomeWorld(MODEL_KALIMA_FALLING_STONE, WD_30BATTLECASTLE));
    CHECK_FALSE(IsHomeWorld(MODEL_POISON, 0));
}

TEST_CASE("The effect browser shows the other kinds of a number, shared code and creation values "
          "[data][effects][editor]")
{
    const EffectBrowserModel model = ShippedModel();

    // The only number with two names: effect shotgun, particle advSmoke2.
    const EffectBrowserDetails shotgun = model.Describe({EffectKind::Effect, BITMAP_SHOTGUN});
    CHECK(shotgun.sameNumber == std::vector<EffectTypeRef>{{EffectKind::Particle, BITMAP_SHOTGUN}});
    CHECK(model.FindRow(EffectKind::Particle, BITMAP_SHOTGUN)->name == "advSmoke2");
    CHECK(model.Describe({EffectKind::Sprite, BITMAP_LIGHT}).sameNumber.size() == 3);

    CHECK(model.Describe({EffectKind::Effect, MODEL_SUMMONER_SUMMON_NEIL_NIFE2}).sameMoveHandler ==
          std::vector<int>{MODEL_SUMMONER_SUMMON_NEIL_NIFE1, MODEL_SUMMONER_SUMMON_NEIL_NIFE3});
    CHECK(model.Describe({EffectKind::Effect, MODEL_MAYASTONE4}).sameCreateHook == std::vector<int>{MODEL_MAYASTONE5});
    CHECK(model.Describe({EffectKind::Effect, MODEL_DESAIR}).sameMoveHandler.empty());

    const EffectBrowserDetails ghost = model.Describe({EffectKind::Effect, MODEL_CUNDUN_GHOST});
    REQUIRE(ghost.creation.has_value());
    CHECK(ghost.creation->variantSubTypes.empty());
    REQUIRE(FindLine(*ghost.creation, "lifeTime") != nullptr);
    CHECK(FindLine(*ghost.creation, "lifeTime")->values == std::vector<std::string>{"200"});
    CHECK(FindLine(*ghost.creation, "light")->values == std::vector<std::string>{"[0.5, 0.5, 0.5]"});

    const EffectBrowserDetails skull = model.Describe({EffectKind::Effect, BITMAP_SKULL});
    REQUIRE(skull.creation.has_value());
    CHECK_FALSE(skull.creation->variantSubTypes.empty());
    CHECK(skull.creation->variantSubTypes.front() == std::vector<int>{1});
    CHECK_FALSE(model.Describe({EffectKind::Effect, MODEL_KALIMA_FALLING_STONE}).creation.has_value());
    CHECK_FALSE(model.Describe({EffectKind::Particle, BITMAP_SHOTGUN}).creation.has_value());
}

TEST_CASE("The creation table lists each field once in the order of the file, with what the SubTypes of each "
          "variant get [data][effects][editor]")
{
    EffectCreateParams row;
    row.lifeTime = 20.0;
    row.positionOffset.components[2] = 1.0;
    row.copyPositionToStartPosition = true;
    EffectCreateParams variant;
    variant.scale = 2.0;
    variant.startPosition.components = {1.0, 2.0, 3.0};
    row.variants.push_back({{1, 2}, variant});

    const EffectCreateTable table = BuildEffectCreateTable(row);
    CHECK(table.variantSubTypes == std::vector<std::vector<int>>{{1, 2}});
    std::vector<std::string> fields;
    for (const EffectCreateTable::Line& line : table.lines)
    {
        fields.push_back(line.field);
    }
    CHECK(fields ==
          std::vector<std::string>{"lifeTime", "scale", "startPosition", "offset.position", "copy.startPosition"});
    CHECK(FindLine(table, "lifeTime")->values == std::vector<std::string>{"20", "20"});
    CHECK(FindLine(table, "scale")->values == std::vector<std::string>{"", "2"});
    // A value of the variant replaces the copy of the row.
    CHECK(FindLine(table, "copy.startPosition")->values == std::vector<std::string>{"position", ""});
    CHECK(FindLine(table, "offset.position")->values == std::vector<std::string>{"{\"z\": 1}", "{\"z\": 1}"});
}
// A field only a later variant sets keeps its place in the file: alpha after
// scale, and a value before the copy another variant makes into the same field.
TEST_CASE("The creation table keeps the order of the file across variants [data][effects][editor]")
{
    EffectCreateParams first;
    first.lifeTime = 5.0;
    first.copyCallScaleToScale = true;
    EffectCreateParams second;
    second.lifeTime = 6.0;
    second.scale = 2.0;
    second.alpha = 0.5;
    EffectCreateParams row;
    row.variants = {{{0}, first}, {{1}, second}};

    const EffectCreateTable table = BuildEffectCreateTable(row);
    std::vector<std::string> fields;
    for (const EffectCreateTable::Line& line : table.lines)
    {
        fields.push_back(line.field);
    }
    CHECK(fields == std::vector<std::string>{"lifeTime", "scale", "alpha", "copy.scale"});
    CHECK(FindLine(table, "scale")->values == std::vector<std::string>{"", "", "2"});
    CHECK(FindLine(table, "copy.scale")->values == std::vector<std::string>{"", "callScale", ""});
}

TEST_CASE("The creation table of every shipped row has the fields of each column in the order of the file "
          "[data][effects][editor]")
{
    const EffectTypeCatalogue catalogue = ShippedCatalogue();
    for (const EffectTypeCreateParams& entry : catalogue.GetCreateParams())
    {
        INFO(CodeOf(entry.type));
        const EffectCreateTable table = BuildEffectCreateTable(entry.params);
        EffectCreateParams common = entry.params;
        common.variants.clear();
        for (size_t column = 0; column <= entry.params.variants.size(); ++column)
        {
            INFO(column);
            const EffectCreateParams params =
                column == 0 ? common : ResolveVariant(entry.params, entry.params.variants[column - 1].params);
            std::vector<std::string> listed;
            for (const EffectCreateTable::Line& line : table.lines)
            {
                if (!line.values[column].empty())
                    listed.push_back(line.field);
            }
            CHECK(listed == WrittenFields(params));
        }
    }
}
#endif // _EDITOR
