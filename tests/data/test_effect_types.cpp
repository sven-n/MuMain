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
         "light": [0.5, 1, 0.25], "copy": {"direction": "light"}}},
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

// The fields FX1.4 added for the cases that set values beyond the first ones.
TEST_CASE("Effect entries are read with vectors, offsets and copies [data][effects]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "kind": "effect", "types": [
        {"name": "dragon", "code": "MODEL_DRAGON", "create": {"lightEnable": false, "alphaEnable": true,
         "kind": 255, "skill": 65535, "pkKey": -1, "timer": 0.5, "distance": 2, "collisionRange": 1,
         "position": {"z": 100}, "angle": [0, 0, 45], "direction": {"x": 1, "y": -35},
         "offset": {"position": {"y": {"value": 200, "timesFrameFactor": true}, "z": 3400},
                    "angle": {"x": {"value": 20, "timesFrameFactor": false}}, "startPosition": [1, 2, 3]},
         "copy": {"startPosition": "position", "headTargetAngle": "callLight", "scale": "callScale"}}}]})",
                                   EffectKind::Effect);
    CHECK(result.issues.empty());
    REQUIRE(result.types.size() == 1);
    REQUIRE(result.types[0].create.has_value());
    const EffectCreateParams& params = *result.types[0].create;
    CHECK(params.lightEnable == false);
    CHECK(params.alphaEnable == true);
    CHECK(params.kind == 255);
    CHECK(params.skill == 65535);
    CHECK(params.pkKey == -1.0);
    CHECK(params.timer == 0.5);
    CHECK(params.distance == 2.0);
    CHECK(params.collisionRange == 1.0);
    CHECK(params.position == EffectCreateVector{{std::nullopt, std::nullopt, 100.0}});
    CHECK(params.angle == EffectCreateVector{{0.0, 0.0, 45.0}});
    CHECK(params.direction == EffectCreateVector{{1.0, -35.0, std::nullopt}});
    CHECK(params.positionOffset == EffectCreateVector{{std::nullopt, 200.0, 3400.0}, {false, true, false}});
    CHECK(params.angleOffset == EffectCreateVector{{20.0, std::nullopt, std::nullopt}});
    CHECK(params.startPositionOffset == EffectCreateVector{{1.0, 2.0, 3.0}});
    CHECK_FALSE(params.copyLightToDirection);
    CHECK(params.copyPositionToStartPosition);
    CHECK(params.copyCallLightToHeadTargetAngle);
    CHECK(params.copyCallScaleToScale);
}

TEST_CASE("Wrong vectors, offsets and copies are errors, unknown parts warnings [data][effects]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "kind": "effect", "types": [
        {"name": "dragon", "code": "MODEL_DRAGON", "create": {"lifeTime": 2, "lightEnable": 1, "kind": 256,
         "skill": -1, "timer": 1e39, "position": [1, 2], "angle": 5,
         "direction": {"y": {"value": 1, "timesFrameFactor": true}, "w": 1},
         "offset": {"position": {"x": {"timesFrameFactor": true}, "y": {"value": "far"}, "z": "up"},
                    "angle": {}, "startPosition": {"y": {"value": 1, "timesFrameFactor": "yes"}},
                    "scale": {"x": 1}},
         "copy": {"startPosition": "angle", "headTargetAngle": 1, "light": "direction"}}},
        {"name": "ghost", "code": "MODEL_CUNDUN_GHOST", "create": {"offset": 5, "copy": []}},
        {"name": "blood", "code": "MODEL_BLOOD", "create": {"scale": 1, "direction": [0, 1, 0], "offset": {},
         "copy": {"scale": "callScale", "direction": "light"}}}]})",
                                   EffectKind::Effect);
    for (const char* field : {"lightEnable", "kind", "skill", "timer", "position", "angle", "direction.y",
                              "offset.position.x.value", "offset.position.y.value", "offset.position.z",
                              "offset.startPosition.y.timesFrameFactor", "copy.startPosition", "copy.headTargetAngle"})
    {
        INFO(field);
        CHECK(HasError(result.issues, std::string("types[0].create.") + field));
    }
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "types[0].create.direction.w"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "types[0].create.offset.angle"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "types[0].create.offset.scale"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "types[0].create.copy.light"));
    CHECK(HasError(result.issues, "types[1].create.offset"));
    CHECK(HasError(result.issues, "types[1].create.copy"));
    // A field gets one value or one copy.
    CHECK(HasError(result.issues, "types[2].create.copy.scale"));
    CHECK(HasError(result.issues, "types[2].create.copy.direction"));
    CHECK(HasIssue(result.issues, ItemDataIssueSeverity::Warning, "types[2].create.offset"));

    REQUIRE(result.types.size() == 3);
    CHECK(result.types[0].create == EffectCreateParams{.lifeTime = 2});
    CHECK(result.types[1].create == EffectCreateParams{});
    CHECK(result.types[2].create == EffectCreateParams{.scale = 1, .direction = EffectCreateVector{{0.0, 1.0, 0.0}}});
}

TEST_CASE("Wrong creation values are errors, unknown ones warnings, and only effects have them [data][effects]")
{
    const ReadResult result = Read(R"({"formatVersion": 1, "kind": "effect", "types": [
        {"name": "dragon", "code": "MODEL_DRAGON", "create": 5},
        {"name": "ghost", "code": "MODEL_CUNDUN_GHOST", "create": {"lifeTime": "long", "blendMesh": 1.5,
         "hiddenMesh": -3, "light": [1, 1], "copy": {"direction": 1}, "size": 2}}]})",
                                   EffectKind::Effect);
    CHECK(HasError(result.issues, "types[0].create"));
    CHECK(HasError(result.issues, "types[1].create.lifeTime"));
    CHECK(HasError(result.issues, "types[1].create.blendMesh"));
    CHECK(HasError(result.issues, "types[1].create.hiddenMesh"));
    CHECK(HasError(result.issues, "types[1].create.light"));
    CHECK(HasError(result.issues, "types[1].create.copy.direction"));
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
                  "        \"copy\": {\n"
                  "          \"direction\": \"light\"\n"
                  "        }\n"
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

// A vector is a list when all its components are set and none is multiplied
// by the frame factor, else an object of the components; both on one line.
TEST_CASE("Vectors, offsets and copies are written in a fixed order, vectors on one line [data][effects]")
{
    EffectTypeEntry dragon{"dragon", "MODEL_DRAGON"};
    dragon.create =
        EffectCreateParams{.lifeTime = 1000,
                           .light = std::array<double, 3>{1, 1, 1},
                           .lightEnable = false,
                           .alphaEnable = true,
                           .kind = 0,
                           .skill = 3,
                           .pkKey = -1,
                           .timer = 0,
                           .distance = 1,
                           .collisionRange = 1,
                           .position = EffectCreateVector{{std::nullopt, std::nullopt, 100.0}},
                           .angle = EffectCreateVector{{0.0, std::nullopt, std::nullopt}},
                           .direction = EffectCreateVector{{0.0, -35.0, 0.5}},
                           .positionOffset = EffectCreateVector{{std::nullopt, 200.0, 3400.0}, {false, true, false}},
                           .angleOffset = EffectCreateVector{{20.0, 0.0, 0.0}, {true, true, true}},
                           .startPositionOffset = EffectCreateVector{{std::nullopt, std::nullopt, 800.0}},
                           .copyPositionToStartPosition = true,
                           .copyCallLightToHeadTargetAngle = true,
                           .copyCallScaleToScale = true};
    const std::vector<EffectTypeEntry> types = {dragon};

    const std::string text = WriteEffectTypesJson(EffectKind::Effect, types);
    CHECK(text == "{\n"
                  "  \"formatVersion\": 1,\n"
                  "  \"kind\": \"effect\",\n"
                  "  \"types\": [\n"
                  "    {\n"
                  "      \"name\": \"dragon\",\n"
                  "      \"code\": \"MODEL_DRAGON\",\n"
                  "      \"create\": {\n"
                  "        \"lifeTime\": 1000,\n"
                  "        \"light\": [1, 1, 1],\n"
                  "        \"lightEnable\": false,\n"
                  "        \"alphaEnable\": true,\n"
                  "        \"kind\": 0,\n"
                  "        \"skill\": 3,\n"
                  "        \"pkKey\": -1,\n"
                  "        \"timer\": 0,\n"
                  "        \"distance\": 1,\n"
                  "        \"collisionRange\": 1,\n"
                  "        \"position\": {\"z\": 100},\n"
                  "        \"angle\": {\"x\": 0},\n"
                  "        \"direction\": [0, -35, 0.5],\n"
                  "        \"offset\": {\n"
                  "          \"position\": {\"y\": {\"value\": 200, \"timesFrameFactor\": true}, \"z\": 3400},\n"
                  "          \"angle\": {\"x\": {\"value\": 20, \"timesFrameFactor\": true}, \"y\": {\"value\": 0, "
                  "\"timesFrameFactor\": true}, \"z\": {\"value\": 0, \"timesFrameFactor\": true}},\n"
                  "          \"startPosition\": {\"z\": 800}\n"
                  "        },\n"
                  "        \"copy\": {\n"
                  "          \"startPosition\": \"position\",\n"
                  "          \"headTargetAngle\": \"callLight\",\n"
                  "          \"scale\": \"callScale\"\n"
                  "        }\n"
                  "      }\n"
                  "    }\n"
                  "  ]\n"
                  "}\n");
    const ReadResult result = Read(text, EffectKind::Effect);
    CHECK(result.issues.empty());
    REQUIRE(result.types.size() == 1);
    CHECK(result.types[0] == dragon);
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
    // FX1.2, the 8 types whose creation cases FX1.3 moved and the 28 of FX1.4.
    CHECK(withCreateParams == 68);

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

    CHECK(params.groups == Render::Effects::CreateParams::Copies);

    OBJECT blood;
    Render::Effects::ApplyCreateParams(&blood, params, {});
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

// The values first, then the offsets, then the copies; an offset of a field a
// copy writes adds to the copy. The values differ from each other and from
// what the effect holds, so a value in the wrong field or step shows up.
TEST_CASE("The effect registry converts and applies vectors, offsets and copies in order [data][effects]")
{
    const EffectTypeCreateParams row{
        MODEL_BLOOD,
        EffectCreateParams{.lightEnable = false,
                           .alphaEnable = true,
                           .kind = 21,
                           .skill = 22,
                           .pkKey = -23,
                           .timer = 24,
                           .distance = 25,
                           .collisionRange = 26,
                           .position = EffectCreateVector{{std::nullopt, std::nullopt, 100.0}},
                           .angle = EffectCreateVector{{std::nullopt, 0.0, std::nullopt}},
                           .direction = EffectCreateVector{{1.0, 2.0, 3.0}},
                           .positionOffset = EffectCreateVector{{10.0, std::nullopt, 3400.0}, {false, false, true}},
                           .angleOffset = EffectCreateVector{{std::nullopt, std::nullopt, 90.0}},
                           .startPositionOffset = EffectCreateVector{{std::nullopt, 800.0, std::nullopt}},
                           .copyPositionToStartPosition = true,
                           .copyCallLightToHeadTargetAngle = true,
                           .copyCallScaleToScale = true}};
    Render::Effects::BuildRegistry(std::span<const EffectTypeCreateParams>(&row, 1));

    const Render::Effects::CreateParams& params = RequireCreateParams(MODEL_BLOOD);
    using Group = Render::Effects::CreateParams::Group;
    CHECK(params.groups == (Group::Flags | Group::Numbers | Group::Vectors | Group::Offsets | Group::Copies));
    CHECK(params.kind == 21);
    CHECK(params.skill == 22);
    CHECK(params.position.components == 0b100);
    CHECK(params.positionOffset.values == std::array<float, 3>{10.f, 0.f, 3400.f});
    CHECK(params.positionOffset.timesFrameFactor == 0b100);

    extern float FPS_ANIMATION_FACTOR;
    const float frameFactor = FPS_ANIMATION_FACTOR;
    FPS_ANIMATION_FACTOR = 0.5f;
    OBJECT blood;
    blood.LightEnable = true;
    blood.AlphaEnable = false;
    Vector(1000.f, 2000.f, 3000.f, blood.Position);
    Vector(11.f, 22.f, 33.f, blood.Angle);
    Vector(-1.f, -2.f, -3.f, blood.StartPosition);
    blood.Scale = 0.9f;
    const vec3_t callLight = {0.25f, 0.5f, 0.75f};
    Render::Effects::ApplyCreateParams(&blood, params, {callLight, 0.f});
    FPS_ANIMATION_FACTOR = frameFactor;

    CHECK_FALSE(blood.LightEnable);
    CHECK(blood.AlphaEnable);
    CHECK(blood.Kind == 21);
    CHECK(blood.Skill == 22);
    CHECK(blood.PKKey == -23.f);
    CHECK(blood.Timer == 24.f);
    CHECK(blood.Distance == 25.f);
    CHECK(blood.CollisionRange == 26.f);
    // Position: z set to 100, then 3400 times the frame factor added; x offset.
    CHECK(blood.Position[0] == 1010.f);
    CHECK(blood.Position[1] == 2000.f);
    CHECK(blood.Position[2] == 1800.f);
    CHECK(blood.Angle[0] == 11.f);
    CHECK(blood.Angle[1] == 0.f);
    CHECK(blood.Angle[2] == 123.f);
    CHECK(blood.Direction[0] == 1.f);
    CHECK(blood.Direction[2] == 3.f);
    // The start position is the position after its offsets, plus its own offset.
    CHECK(blood.StartPosition[0] == 1010.f);
    CHECK(blood.StartPosition[1] == 2800.f);
    CHECK(blood.StartPosition[2] == 1800.f);
    CHECK(blood.HeadTargetAngle[0] == 0.25f);
    CHECK(blood.HeadTargetAngle[2] == 0.75f);
    // The call's scale as it was passed, also 0.
    CHECK(blood.Scale == 0.f);

    BuildShippedRegistry();
}

TEST_CASE("Creation values from the catalogue are applied to new effects [data][effects]")
{
    BuildShippedRegistry();

    OBJECT ghost;
    ghost.Alpha = 0.25f;
    Render::Effects::ApplyCreateParams(&ghost, RequireCreateParams(MODEL_CUNDUN_GHOST), {});
    CHECK(ghost.LifeTime == 200.f);
    CHECK(ghost.Scale == 1.80f);
    CHECK(ghost.Velocity == 0.08f);
    CHECK(ghost.BlendMesh == -2);
    CHECK(ghost.Light[0] == 0.5f);
    CHECK(ghost.Light[2] == 0.5f);
    CHECK(ghost.Alpha == 0.25f);

    OBJECT arrow;
    Render::Effects::ApplyCreateParams(&arrow, RequireCreateParams(MODEL_INFINITY_ARROW4), {});
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
