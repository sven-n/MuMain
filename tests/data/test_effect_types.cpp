#include "stdafx.h"

#include "doctest.h"

#include "EffectTestData.h"
#include "TestFiles.h"

#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Data/GameData/EffectData/EffectTypesJson.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Effects/Behaviors/EffectBehaviors.h"
#include "Render/Effects/EffectRegistry.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

using namespace Data::Effects;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using EffectTestData::BuildShippedRegistry;
using EffectTestData::ShippedTypes;

namespace
{
const std::string Source = "ParticleTypes.json";
// Larger than every MODEL_* and BITMAP_* number.
constexpr int TypeNumberLimit = 40000;

struct ReadResult
{
    std::vector<EffectTypeEntry> types;
    std::vector<ItemDataIssue> issues;
};

ReadResult Read(const std::string& text, EffectKind kind = EffectKind::Particle)
{
    ReadResult result;
    ReadEffectTypesJson(text, Source, kind, result.types, result.issues);
    return result;
}

bool HasIssue(const std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& field)
{
    return std::any_of(issues.begin(), issues.end(),
                       [&](const ItemDataIssue& issue) { return issue.severity == severity && issue.field == field; });
}

bool HasError(const std::vector<ItemDataIssue>& issues, const std::string& field)
{
    return HasIssue(issues, ItemDataIssueSeverity::Error, field);
}

// A valid list of the particle types: every symbol with the name "typeN".
std::vector<EffectTypeEntry> NameEveryParticle()
{
    std::vector<EffectTypeEntry> types;
    for (const EffectTypeSymbol& symbol : GetEffectTypeSymbols(EffectKind::Particle))
    {
        types.push_back({"type" + std::to_string(types.size()), std::string(symbol.code)});
    }
    return types;
}

std::vector<ItemDataIssue> Validate(const std::vector<EffectTypeEntry>& types)
{
    std::vector<ItemDataIssue> issues;
    ValidateEffectTypes(EffectKind::Particle, types, Source, issues);
    return issues;
}

bool IsEffectSymbol(int type)
{
    const auto symbols = GetEffectTypeSymbols(EffectKind::Effect);
    return std::any_of(symbols.begin(), symbols.end(),
                       [&](const EffectTypeSymbol& symbol) { return symbol.type == type; });
}

const Render::Effects::CreateParams& RequireCreateParams(int type)
{
    const Render::Effects::EffectDescriptor* descriptor = Render::Effects::Lookup(type);
    REQUIRE(descriptor != nullptr);
    REQUIRE(descriptor->create.has_value());
    return *descriptor->create;
}
} // namespace

TEST_CASE("Effect types are read with their name and code [data][effects]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "kind": "particle", "types": [
        {"name": "smoke", "code": "BITMAP_SMOKE"}, {"name": "smoke2", "code": "BITMAP_SMOKE+1"}]})");
    CHECK(result.issues.empty());
    REQUIRE(result.types.size() == 2);
    CHECK(result.types[0] == EffectTypeEntry{"smoke", "BITMAP_SMOKE"});
    CHECK(result.types[1] == EffectTypeEntry{"smoke2", "BITMAP_SMOKE+1"});
}

TEST_CASE("Effect type files of the wrong kind or a newer format are errors [data][effects]")
{
    CHECK(HasError(Read(R"({"formatVersion": 1, "kind": "joint", "types": []})").issues, "kind"));
    CHECK(HasError(Read(R"({"formatVersion": 1, "types": []})").issues, "kind"));
    CHECK(Data::Items::HasErrors(Read(R"({"formatVersion": 2, "kind": "particle", "types": []})").issues));
    CHECK(HasError(Read(R"({"formatVersion": 1, "kind": "particle"})").issues, "types"));
}

TEST_CASE("Effect type entries without name or code are errors, unknown fields warnings [data][effects]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "kind": "particle", "extra": 1, "types": [
        {"name": "smoke"}, {"code": "BITMAP_SMOKE"}, 5, {"name": "smoke", "code": "BITMAP_SMOKE", "scale": 2}]})");
    CHECK(HasError(result.issues, "types[0].code"));
    CHECK(HasError(result.issues, "types[1].name"));
    CHECK(HasError(result.issues, "types[2]"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "types[3].scale"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "extra"));
    CHECK(result.types.size() == 1);
}

TEST_CASE("Every effect type symbol needs exactly one valid name [data][effects]")
{
    CHECK(Validate(NameEveryParticle()).empty());

    std::vector<EffectTypeEntry> types = NameEveryParticle();
    const std::string missingCode = types.back().code;
    types.pop_back();
    CHECK(HasError(Validate(types), missingCode));

    types = NameEveryParticle();
    types[0].name = "2line";
    types[1].name = "smoke-2";
    types[3].name = types[2].name;
    const std::vector<ItemDataIssue> issues = Validate(types);
    CHECK(HasError(issues, "2line"));
    CHECK(HasError(issues, "smoke-2"));
    CHECK(HasError(issues, types[2].name));

    types = NameEveryParticle();
    types.push_back({"unknownType", "BITMAP_SMOKE+99"});
    types.push_back({"secondSmoke", types[0].code});
    const std::vector<ItemDataIssue> codeIssues = Validate(types);
    CHECK(HasError(codeIssues, "unknownType"));
    CHECK(HasError(codeIssues, "secondSmoke"));
}

TEST_CASE("Effect type files are written sorted by name with a fixed field order [data][effects]")
{
    const std::vector<EffectTypeEntry> types = {{"smoke2", "BITMAP_SMOKE+1"}, {"smoke", "BITMAP_SMOKE"}};
    const std::string text = WriteEffectTypesJson(EffectKind::Particle, types);
    CHECK(text == "{\n"
                  "  \"formatVersion\": 1,\n"
                  "  \"kind\": \"particle\",\n"
                  "  \"types\": [\n"
                  "    {\n"
                  "      \"name\": \"smoke\",\n"
                  "      \"code\": \"BITMAP_SMOKE\"\n"
                  "    },\n"
                  "    {\n"
                  "      \"name\": \"smoke2\",\n"
                  "      \"code\": \"BITMAP_SMOKE+1\"\n"
                  "    }\n"
                  "  ]\n"
                  "}\n");
    const ReadResult result = Read(text);
    CHECK(result.issues.empty());
    CHECK(result.types.size() == 2);
}

TEST_CASE("Effect entries are read with their creation values [data][effects]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "kind": "effect", "types": [
        {"name": "ghost", "code": "MODEL_CUNDUN_GHOST", "create": {"lifeTime": 200, "scale": 1.8, "velocity": 0.08,
         "gravity": -2, "hiddenMesh": 1, "blendMesh": -2, "blendMeshLight": 0.5, "alpha": 0,
         "light": [0.5, 1, 0.25], "copyLightToDirection": true}},
        {"name": "dragon", "code": "MODEL_DRAGON"}]})",
                                   EffectKind::Effect);
    CHECK(result.issues.empty());
    REQUIRE(result.types.size() == 2);
    REQUIRE(result.types[0].create.has_value());
    const EffectCreateParams& params = *result.types[0].create;
    CHECK(params.lifeTime == 200.0);
    CHECK(params.scale == 1.8);
    CHECK(params.velocity == 0.08);
    CHECK(params.gravity == -2.0);
    CHECK(params.hiddenMesh == 1);
    CHECK(params.blendMesh == -2);
    CHECK(params.blendMeshLight == 0.5);
    CHECK(params.alpha == 0.0);
    CHECK(params.light == std::array<double, 3>{0.5, 1.0, 0.25});
    CHECK(params.copyLightToDirection);
    CHECK_FALSE(result.types[1].create.has_value());
}

TEST_CASE("Wrong creation values are errors, unknown ones warnings, and only effects have them [data][effects]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "kind": "effect", "types": [
        {"name": "dragon", "code": "MODEL_DRAGON", "create": 5},
        {"name": "ghost", "code": "MODEL_CUNDUN_GHOST", "create": {"lifeTime": "long", "blendMesh": 1.5,
         "hiddenMesh": -3, "light": [1, 1], "copyLightToDirection": 1, "size": 2}}]})",
                                   EffectKind::Effect);
    CHECK(HasError(result.issues, "types[0].create"));
    CHECK(HasError(result.issues, "types[1].create.lifeTime"));
    CHECK(HasError(result.issues, "types[1].create.blendMesh"));
    CHECK(HasError(result.issues, "types[1].create.hiddenMesh"));
    CHECK(HasError(result.issues, "types[1].create.light"));
    CHECK(HasError(result.issues, "types[1].create.copyLightToDirection"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "types[1].create.size"));
    REQUIRE(result.types.size() == 2);
    CHECK_FALSE(result.types[0].create.has_value());
    CHECK(result.types[1].create == EffectCreateParams{});

    // The game keeps the values as float; alpha goes from 0 to 1; mesh numbers are whole numbers.
    const ReadResult ranges = Read(R"({"formatVersion": 1, "kind": "effect", "types": [
        {"name": "fire", "code": "BITMAP_FIRE", "create": {"lifeTime": 2, "scale": 1e39, "alpha": 5,
         "light": [1, 1e39, 0], "blendMesh": 0.0}}]})",
                                   EffectKind::Effect);
    CHECK(HasError(ranges.issues, "types[0].create.scale"));
    CHECK(HasError(ranges.issues, "types[0].create.alpha"));
    CHECK(HasError(ranges.issues, "types[0].create.light"));
    CHECK(HasError(ranges.issues, "types[0].create.blendMesh"));
    REQUIRE(ranges.types.size() == 1);
    CHECK(ranges.types[0].create == EffectCreateParams{.lifeTime = 2});

    // A create that sets nothing still skips the creation code of the type.
    const ReadResult empty = Read(R"({"formatVersion": 1, "kind": "effect", "types": [
        {"name": "dragon", "code": "MODEL_DRAGON", "create": {}},
        {"name": "ghost", "code": "MODEL_CUNDUN_GHOST", "create": {"lifetime": 2}}]})",
                                  EffectKind::Effect);
    CHECK(HasIssue(empty.issues, ItemDataIssueSeverity::Warning, "types[0].create"));
    CHECK(HasIssue(empty.issues, ItemDataIssueSeverity::Warning, "types[1].create"));
    CHECK(HasIssue(empty.issues, ItemDataIssueSeverity::Warning, "types[1].create.lifetime"));
    CHECK_FALSE(Data::Items::HasErrors(empty.issues));

    const ReadResult particles = Read(R"({"formatVersion": 1, "kind": "particle", "types": [
        {"name": "smoke", "code": "BITMAP_SMOKE", "create": {"lifeTime": 2}}]})");
    CHECK(HasIssue(particles.issues, ItemDataIssueSeverity::Warning, "types[0].create"));
    REQUIRE(particles.types.size() == 1);
    CHECK_FALSE(particles.types[0].create.has_value());
}

TEST_CASE("Creation values are written in a fixed order, unset ones left out [data][effects]")
{
    EffectTypeEntry ghost{"ghost", "MODEL_CUNDUN_GHOST"};
    ghost.create = EffectCreateParams{
        .lifeTime = 200, .scale = 1.8, .blendMesh = -2, .light = std::array<double, 3>{0.5, 0.5, 0.5}};
    EffectTypeEntry arrow{"arrow", "MODEL_INFINITY_ARROW4"};
    arrow.create = EffectCreateParams{.lifeTime = 15,
                                      .scale = 1,
                                      .velocity = 0.3,
                                      .gravity = 2,
                                      .hiddenMesh = 1,
                                      .blendMesh = 0,
                                      .blendMeshLight = 0.5,
                                      .alpha = 0,
                                      .light = std::array<double, 3>{1, 0.5, 0.3},
                                      .copyLightToDirection = true};
    const std::vector<EffectTypeEntry> types = {ghost, arrow};

    const std::string text = WriteEffectTypesJson(EffectKind::Effect, types);
    CHECK(text == "{\n"
                  "  \"formatVersion\": 1,\n"
                  "  \"kind\": \"effect\",\n"
                  "  \"types\": [\n"
                  "    {\n"
                  "      \"name\": \"arrow\",\n"
                  "      \"code\": \"MODEL_INFINITY_ARROW4\",\n"
                  "      \"create\": {\n"
                  "        \"lifeTime\": 15,\n"
                  "        \"scale\": 1,\n"
                  "        \"velocity\": 0.3,\n"
                  "        \"gravity\": 2,\n"
                  "        \"hiddenMesh\": 1,\n"
                  "        \"blendMesh\": 0,\n"
                  "        \"blendMeshLight\": 0.5,\n"
                  "        \"alpha\": 0,\n"
                  "        \"light\": [1, 0.5, 0.3],\n"
                  "        \"copyLightToDirection\": true\n"
                  "      }\n"
                  "    },\n"
                  "    {\n"
                  "      \"name\": \"ghost\",\n"
                  "      \"code\": \"MODEL_CUNDUN_GHOST\",\n"
                  "      \"create\": {\n"
                  "        \"lifeTime\": 200,\n"
                  "        \"scale\": 1.8,\n"
                  "        \"blendMesh\": -2,\n"
                  "        \"light\": [0.5, 0.5, 0.5]\n"
                  "      }\n"
                  "    }\n"
                  "  ]\n"
                  "}\n");
    const ReadResult result = Read(text, EffectKind::Effect);
    CHECK(result.issues.empty());
    REQUIRE(result.types.size() == 2);
    CHECK(result.types[0] == arrow);
    CHECK(result.types[1] == ghost);

    // Only effects have creation values, so other kinds leave them out.
    EffectTypeEntry smoke{"smoke", "BITMAP_SMOKE"};
    smoke.create = EffectCreateParams{.lifeTime = 2};
    const std::vector<EffectTypeEntry> particles = {smoke};
    CHECK(WriteEffectTypesJson(EffectKind::Particle, particles).find("create") == std::string::npos);
}

// Every code is an enum symbol, so the data holds no raw numbers (D28).
TEST_CASE("Each kind lists every type number once, sorted, by its symbol [data][effects]")
{
    for (const EffectKind kind : EffectKinds)
    {
        INFO(GetEffectKindName(kind));
        const auto symbols = GetEffectTypeSymbols(kind);
        CHECK_FALSE(symbols.empty());
        std::set<std::string_view> codes;
        for (size_t i = 0; i < symbols.size(); ++i)
        {
            INFO(symbols[i].code);
            CHECK(std::isupper(static_cast<unsigned char>(symbols[i].code.front())) != 0);
            CHECK(codes.insert(symbols[i].code).second);
            if (i > 0)
            {
                CHECK(symbols[i - 1].type < symbols[i].type);
            }
        }
    }
}

TEST_CASE("Shipped effect type files load without problems [data][effects]")
{
    for (const ItemDataIssue& issue : ShippedTypes().issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
    for (const EffectKind kind : EffectKinds)
    {
        INFO(GetEffectKindName(kind));
        CHECK(ShippedTypes().types[ToIndex(kind)].size() == GetEffectTypeSymbols(kind).size());
    }
}

// Nothing in the client saves the catalogue yet; this checks that the shipped
// files are in the format the writer produces.
TEST_CASE("Shipped effect type files are in the written format [data][effects]")
{
    for (const EffectKind kind : EffectKinds)
    {
        const std::string fileName(GetEffectTypesFileName(kind));
        INFO(fileName);
        CHECK(WriteEffectTypesJson(kind, ShippedTypes().types[ToIndex(kind)]) ==
              TestFiles::ReadWholeFile(EffectTestData::ShippedEffectDirectory() / fileName));
    }
}

TEST_CASE("The effect type catalogue finds types by name and names by number [data][effects]")
{
    EffectTypeCatalogue catalogue;
    for (const EffectKind kind : EffectKinds)
    {
        catalogue.Build(kind, ShippedTypes().types[ToIndex(kind)]);
        CHECK(catalogue.GetTypeCount(kind) == GetEffectTypeSymbols(kind).size());
    }
    CHECK(catalogue.FindType(EffectKind::Particle, "smoke2") == BITMAP_SMOKE + 1);
    CHECK(catalogue.FindType(EffectKind::Effect, "earthQuake1") == MODEL_SKILL_FURY_STRIKE + 1);
    CHECK(catalogue.FindType(EffectKind::Joint, "jointThunder") == BITMAP_JOINT_THUNDER);
    CHECK_FALSE(catalogue.FindType(EffectKind::Joint, "smoke2").has_value());
    CHECK(catalogue.GetName(EffectKind::Sprite, BITMAP_SHINY + 1) == "shiny2");
    CHECK(catalogue.GetName(EffectKind::Effect, MODEL_CUNDUN_GHOST) == "kundunGhost");
    CHECK(catalogue.FindType(EffectKind::Effect, "kalimaFallingStone") == MODEL_KALIMA_FALLING_STONE);
    CHECK(catalogue.GetName(EffectKind::Particle, MODEL_CUNDUN_GHOST).empty());
}

// The creation values that EffectRegistry.cpp held as C++ rows until FX1.2.
TEST_CASE("The effect type catalogue keeps the creation values of effects, sorted by number [data][effects]")
{
    const std::vector<EffectTypeEntry>& effects = ShippedTypes().types[ToIndex(EffectKind::Effect)];
    const auto withCreateParams = static_cast<size_t>(std::count_if(
        effects.begin(), effects.end(), [](const EffectTypeEntry& entry) { return entry.create.has_value(); }));
    // The 32 types of the 22 rows that EffectRegistry.cpp held as C++ until
    // FX1.2, and the 8 types whose creation cases FX1.3 moved.
    CHECK(withCreateParams == 40);

    EffectTypeCatalogue catalogue;
    catalogue.Build(EffectKind::Effect, effects);
    const auto createParams = catalogue.GetCreateParams();
    CHECK(createParams.size() == withCreateParams);
    CHECK(std::is_sorted(createParams.begin(), createParams.end(),
                         [](const EffectTypeCreateParams& left, const EffectTypeCreateParams& right)
                         { return left.type < right.type; }));

    // Building another kind keeps them.
    catalogue.Build(EffectKind::Particle, ShippedTypes().types[ToIndex(EffectKind::Particle)]);
    CHECK(catalogue.GetCreateParams().size() == withCreateParams);
}

TEST_CASE("The effect registry takes creation values from the catalogue, handlers from the code [data][effects]")
{
    BuildShippedRegistry();

    // The values as the old C++ rows wrote them.
    const Render::Effects::CreateParams& ghost = RequireCreateParams(MODEL_CUNDUN_GHOST);
    CHECK(ghost.lifeTime == 200.f);
    CHECK(ghost.scale == 1.80f);
    CHECK(ghost.velocity == 0.08f);
    CHECK(ghost.blendMesh == -2);
    CHECK(ghost.light == std::array<float, 3>{0.5f, 0.5f, 0.5f});
    CHECK_FALSE(ghost.copyLightToDirection);

    const Render::Effects::CreateParams& arrow = RequireCreateParams(MODEL_INFINITY_ARROW4);
    CHECK(arrow.light == std::array<float, 3>{1.f, 0.5f, 0.3f});
    CHECK(arrow.copyLightToDirection);
    CHECK(Render::Effects::Lookup(MODEL_INFINITY_ARROW4)->move == &Render::Effects::Behaviors::MoveInfinityArrow4);

    // Creation values only.
    const Render::Effects::EffectDescriptor* blood = Render::Effects::Lookup(MODEL_BLOOD);
    REQUIRE(blood != nullptr);
    CHECK(blood->create.has_value());
    CHECK(blood->onCreate == nullptr);
    CHECK(blood->move == nullptr);
    CHECK(blood->render == nullptr);

    // A creation hook and a move handler, no creation values.
    const Render::Effects::EffectDescriptor* mayaStone = Render::Effects::Lookup(MODEL_MAYASTONE4);
    REQUIRE(mayaStone != nullptr);
    CHECK_FALSE(mayaStone->create.has_value());
    CHECK(mayaStone->onCreate == &Render::Effects::Behaviors::CreateMayaStone45);
    CHECK(mayaStone->move != nullptr);

    CHECK(Render::Effects::Lookup(MODEL_KALIMA_FALLING_STONE) == nullptr);
    CHECK(Render::Effects::Lookup(-1) == nullptr);
    CHECK(Render::Effects::Lookup(TypeNumberLimit) == nullptr);
}

// One made-up row with every field set to a different value, so a value that
// lands in the wrong field shows up.
TEST_CASE("The effect registry converts and applies every creation value [data][effects]")
{
    const EffectTypeCreateParams row{MODEL_BLOOD, EffectCreateParams{.lifeTime = 11,
                                                                     .scale = 12,
                                                                     .velocity = 13,
                                                                     .gravity = 14,
                                                                     .hiddenMesh = 15,
                                                                     .blendMesh = 16,
                                                                     .blendMeshLight = 17,
                                                                     .alpha = 0.5,
                                                                     .light = std::array<double, 3>{0.1, 0.2, 0.3},
                                                                     .copyLightToDirection = true}};
    Render::Effects::BuildRegistry(std::span<const EffectTypeCreateParams>(&row, 1));

    const Render::Effects::CreateParams& params = RequireCreateParams(MODEL_BLOOD);
    CHECK(params.lifeTime == 11.f);
    CHECK(params.scale == 12.f);
    CHECK(params.velocity == 13.f);
    CHECK(params.gravity == 14.f);
    CHECK(params.hiddenMesh == 15);
    CHECK(params.blendMesh == 16);
    CHECK(params.blendMeshLight == 17.f);
    CHECK(params.alpha == 0.5f);
    CHECK(params.light == std::array<float, 3>{0.1f, 0.2f, 0.3f});
    CHECK(params.copyLightToDirection);

    OBJECT blood;
    Render::Effects::ApplyCreateParams(&blood, params);
    CHECK(blood.LifeTime == 11.f);
    CHECK(blood.Scale == 12.f);
    CHECK(blood.Velocity == 13.f);
    CHECK(blood.Gravity == 14.f);
    CHECK(blood.HiddenMesh == 15);
    CHECK(blood.BlendMesh == 16);
    CHECK(blood.BlendMeshLight == 17.f);
    CHECK(blood.Alpha == 0.5f);
    CHECK(blood.Light[0] == 0.1f);
    CHECK(blood.Light[1] == 0.2f);
    CHECK(blood.Light[2] == 0.3f);
    CHECK(blood.Direction[0] == 0.1f);
    CHECK(blood.Direction[1] == 0.2f);
    CHECK(blood.Direction[2] == 0.3f);

    // The handlers are code: they are there without any creation values.
    Render::Effects::BuildRegistry({});
    CHECK(Render::Effects::Lookup(MODEL_BLOOD) == nullptr);
    REQUIRE(Render::Effects::Lookup(MODEL_DESAIR) != nullptr);
    CHECK(Render::Effects::Lookup(MODEL_DESAIR)->move == &Render::Effects::Behaviors::MoveDesair);

    BuildShippedRegistry();
}

TEST_CASE("Creation values from the catalogue are applied to new effects [data][effects]")
{
    BuildShippedRegistry();

    OBJECT ghost;
    ghost.Alpha = 0.25f;
    Render::Effects::ApplyCreateParams(&ghost, RequireCreateParams(MODEL_CUNDUN_GHOST));
    CHECK(ghost.LifeTime == 200.f);
    CHECK(ghost.Scale == 1.80f);
    CHECK(ghost.Velocity == 0.08f);
    CHECK(ghost.BlendMesh == -2);
    CHECK(ghost.Light[0] == 0.5f);
    CHECK(ghost.Light[2] == 0.5f);
    CHECK(ghost.Alpha == 0.25f);

    OBJECT arrow;
    Render::Effects::ApplyCreateParams(&arrow, RequireCreateParams(MODEL_INFINITY_ARROW4));
    CHECK(arrow.Light[1] == 0.5f);
    CHECK(arrow.Direction[0] == 1.f);
    CHECK(arrow.Direction[1] == 0.5f);
    CHECK(arrow.Direction[2] == 0.3f);
}

// The registry of #493 describes effect types by number; each of them must
// have a name, or its data could not reference it.
TEST_CASE("Every effect type of the registry is in the catalogue [data][effects]")
{
    BuildShippedRegistry();
    for (int type = 0; type < TypeNumberLimit; ++type)
    {
        if (Render::Effects::Lookup(type) != nullptr)
        {
            INFO(type);
            CHECK(IsEffectSymbol(type));
        }
    }
}
