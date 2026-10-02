#include "stdafx.h"

#include "doctest.h"

#include "TestFiles.h"

#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Data/GameData/EffectData/EffectTypesJson.h"
#include "Render/Effects/EffectRegistry.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

using namespace Data::Effects;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;

namespace
{
const std::filesystem::path EffectDirectory = std::filesystem::path(MU_TEST_DATA_DIR) / "Effects";
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

const EffectTypesLoadResult& ShippedTypes()
{
    static const EffectTypesLoadResult result = LoadEffectTypeFiles(EffectDirectory);
    return result;
}

bool IsEffectSymbol(int type)
{
    const auto symbols = GetEffectTypeSymbols(EffectKind::Effect);
    return std::any_of(symbols.begin(), symbols.end(),
                       [&](const EffectTypeSymbol& symbol) { return symbol.type == type; });
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
              TestFiles::ReadWholeFile(EffectDirectory / fileName));
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

// The registry of #493 describes effect types by number; each of them must
// have a name, or its data could not reference it.
TEST_CASE("Every effect type of the registry is in the catalogue [data][effects]")
{
    for (int type = 0; type < TypeNumberLimit; ++type)
    {
        if (Render::Effects::Lookup(type) != nullptr)
        {
            INFO(type);
            CHECK(IsEffectSymbol(type));
        }
    }
}
