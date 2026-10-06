#include "stdafx.h"

#include "doctest.h"

// The game's create calls of a type, which the effect browser reads from the
// sources and offers to the world preview (FX1.7b).
#ifdef _EDITOR
#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"
#include "UI/EffectBrowser/EffectCallSites.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

using namespace MuEditor::Effects;
using Data::Effects::EffectKind;

namespace
{
constexpr float Tolerance = 1e-3f;

const EffectCallSite* FindCall(const std::vector<EffectCallSite>& calls, EffectKind kind, int type)
{
    const auto found = std::find_if(calls.begin(), calls.end(),
                                    [&](const EffectCallSite& call) { return call.kind == kind && call.type == type; });
    return found != calls.end() ? &*found : nullptr;
}

bool IsLight(const std::optional<PreviewVector>& light, float r, float g, float b)
{
    return light && std::abs((*light)[0] - r) < Tolerance && std::abs((*light)[1] - g) < Tolerance &&
           std::abs((*light)[2] - b) < Tolerance;
}

// Whether one of `calls` lies in `file`, with this SubType and light.
bool HasCall(const std::vector<const EffectCallSite*>& calls, std::string_view file, int subType, float r, float g,
             float b)
{
    return std::any_of(
        calls.begin(), calls.end(), [&](const EffectCallSite* call)
        { return call->file == file && call->subTypeValue == subType && IsLight(call->light, r, g, b); });
}
} // namespace

TEST_CASE("The effect browser reads the game's create calls with the values they write out [effects][editor]")
{
    const std::string source = R"source(
void Skill(OBJECT* o, vec3_t Position)
{
    vec3_t Light, Angle, Target;
    Vector(0.2f, 0.2f, 1.0f, Light);
    CreateEffect(MODEL_FENRIR_THUNDER, Position, o->Angle, Light, 2, o);
    CreateJoint(BITMAP_JOINT_THUNDER, Position, Target, Angle, 3, NULL, 50.f, 7, 12);
    CreateParticleFpsChecked(BITMAP_FIRE, Position, Angle, o->Light);
    CreateEffect(MODEL_STONE1 + rand() % 2, Position, Angle, Light, 13, o);
    Vector(1.f, 0.5f, .25f, Light);
    CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o, 0.f);
    VectorCopy(o->Light, Light);
    CreateSprite(BITMAP_SHINY, Position, 1.f, Light, o);
    Vector(0.4f, 0.4f, 1.0f, vLight);
    CreateJointFpsChecked(BITMAP_JOINT_ENERGY, Position, Target, Angle, 9, o, 30.f, -1, 0, 0, -1, vLight);
    CreateParticle(BITMAP_SMOKE, Position, Angle, Light, 4, 2.5f, o);
    CreateEffect(MODEL_FIRE, Position, Angle, Light, 1, o, -1, 0, 0, 0, 1.5f);
    CreateSprite(BITMAP_FLARE, Position, 0.75f, Light, NULL, 0.f, 2);
    // CreateEffect(MODEL_POISON, Position, Angle, Light);
    Log("CreateEffect(MODEL_POISON, Position, Angle, Light)");
    g_SkillEffects.CreateEffect();
}

void CreateEffect(int Type, vec3_t Position, vec3_t Angle, vec3_t Light)
{
}
)source";
    const std::vector<EffectCallSite> calls = ReadEffectCallSites(source, "Test/Skill.cpp");
    CHECK(calls.size() == 11);
    CHECK(FindCall(calls, EffectKind::Effect, MODEL_POISON) == nullptr);

    const EffectCallSite* thunder = FindCall(calls, EffectKind::Effect, MODEL_FENRIR_THUNDER);
    REQUIRE(thunder != nullptr);
    CHECK(thunder->file == "Test/Skill.cpp");
    CHECK(thunder->line == 6);
    CHECK(thunder->subTypeValue == 2);
    CHECK(thunder->owner == "o");
    CHECK_FALSE(thunder->withoutOwner);
    REQUIRE(thunder->light.has_value());
    CHECK((*thunder->light)[0] == doctest::Approx(0.2f).epsilon(Tolerance));
    CHECK((*thunder->light)[2] == doctest::Approx(1.0f).epsilon(Tolerance));

    const EffectCallSite* joint = FindCall(calls, EffectKind::Joint, BITMAP_JOINT_THUNDER);
    REQUIRE(joint != nullptr);
    CHECK(joint->subTypeValue == 3);
    CHECK(joint->scaleValue == doctest::Approx(50.0f));
    CHECK(joint->withoutOwner);
    CHECK(joint->pkValue == 7);
    CHECK(joint->skillIndexValue == 12);
    // Lightning's light is the colour it passes; most calls pass none.
    CHECK_FALSE(joint->light.has_value());
    const EffectCallSite* energy = FindCall(calls, EffectKind::Joint, BITMAP_JOINT_ENERGY);
    REQUIRE(energy != nullptr);
    REQUIRE(energy->light.has_value());
    CHECK((*energy->light)[0] == doctest::Approx(0.4f).epsilon(Tolerance));
    CHECK((*energy->light)[2] == doctest::Approx(1.0f).epsilon(Tolerance));

    // Arguments left out get the create function's defaults.
    const EffectCallSite* fire = FindCall(calls, EffectKind::Particle, BITMAP_FIRE);
    REQUIRE(fire != nullptr);
    CHECK(fire->subTypeValue == 0);
    CHECK_FALSE(fire->scaleValue.has_value());
    CHECK(fire->withoutOwner);
    CHECK_FALSE(fire->light.has_value());

    const EffectCallSite* stone = FindCall(calls, EffectKind::Effect, MODEL_STONE1 + 1);
    REQUIRE(stone != nullptr);
    CHECK(stone->randomTypes == 2);
    CHECK(FindCall(calls, EffectKind::Effect, MODEL_STONE1) != nullptr);

    const EffectCallSite* light = FindCall(calls, EffectKind::Sprite, BITMAP_LIGHT);
    REQUIRE(light != nullptr);
    CHECK(light->scaleValue == doctest::Approx(1.5f));
    REQUIRE(light->light.has_value());
    CHECK((*light->light)[1] == doctest::Approx(0.5f).epsilon(Tolerance));
    CHECK((*light->light)[2] == doctest::Approx(0.25f).epsilon(Tolerance));
    // Every argument written: each is read from its place.
    const EffectCallSite* smoke = FindCall(calls, EffectKind::Particle, BITMAP_SMOKE);
    REQUIRE(smoke != nullptr);
    CHECK(smoke->subTypeValue == 4);
    CHECK(smoke->scaleValue == doctest::Approx(2.5f));
    CHECK(smoke->owner == "o");
    const EffectCallSite* fireEffect = FindCall(calls, EffectKind::Effect, MODEL_FIRE);
    REQUIRE(fireEffect != nullptr);
    CHECK(fireEffect->subTypeValue == 1);
    CHECK(fireEffect->scaleValue == doctest::Approx(1.5f));
    const EffectCallSite* flare = FindCall(calls, EffectKind::Sprite, BITMAP_FLARE);
    REQUIRE(flare != nullptr);
    CHECK(flare->subTypeValue == 2);
    CHECK(flare->scaleValue == doctest::Approx(0.75f));
    CHECK(flare->withoutOwner);
    CHECK(light->owner == "o");
    // A copy into the light after the last Vector leaves it unknown.
    const EffectCallSite* shiny = FindCall(calls, EffectKind::Sprite, BITMAP_SHINY);
    REQUIRE(shiny != nullptr);
    CHECK_FALSE(shiny->light.has_value());

    // None only with the SubType of a call that passes no owner.
    const std::vector<const EffectCallSite*> thunderCalls = {joint};
    CHECK(CreatesWithoutOwner(thunderCalls, 3));
    CHECK_FALSE(CreatesWithoutOwner(thunderCalls, 4));
    const std::vector<const EffectCallSite*> stoneCalls = {stone};
    CHECK_FALSE(CreatesWithoutOwner(stoneCalls, 13));
}

TEST_CASE("The effect browser takes the light that reaches the call, in its function [effects][editor]")
{
    const std::string source = R"source(
namespace Effects
{
    void Moved(OBJECT* o, vec3_t Position, vec3_t Angle, int kind)
    {
        vec3_t Light, vColour;
        vec3_t vLight = { 0.45f, 0.45f, 0.7f };
        CreateEffect(MODEL_FIRE, Position, Angle, vLight, 1, o);
        Vector(1.f, 1.f, 1.f, Light);
        switch (kind)
        {
        case 0:
            Vector(0.2f, 0.2f, 0.2f, Light);
            CreateEffect(MODEL_POISON, Position, Angle, Light, 0, o);
            break;
        case 1:
            CreateEffect(MODEL_STONE1, Position, Angle, Light, 0, o);
            break;
        }
        if (kind > 2)
        {
            Vector(0.5f, 0.5f, 0.5f, Light);
        }
        CreateEffect(MODEL_STONE2, Position, Angle, Light, 0, o);
        Vector(0.3f, 0.3f, 0.3f, Light);
        VectorScale(Light, 0.5f, Light);
        CreateEffect(MODEL_BIG_STONE1, Position, Angle, Light, 0, o);
        Vector(0.6f, 0.6f, 0.6f, Light);
        Light[0] = 1.f;
        CreateEffect(MODEL_BIG_STONE2, Position, Angle, Light, 0, o);
        Vector(0.4f, 0.4f, 1.0f, vColour);
        CreateJoint(BITMAP_JOINT_ENERGY, Position, Position, Angle, 0, o, 20.f, -1, 0, 0, -1, vColour);
        CreateJoint(BITMAP_JOINT_THUNDER, Position, Position, Angle, 0, o, 20.f, -1, 0, 0, -1, vColour);
    }

    void Next(OBJECT* o, vec3_t Position, vec3_t Angle)
    {
        vec3_t Light;
        CreateEffect(MODEL_ICE, Position, Angle, Light, 0, o);
    }
}
)source";
    const std::vector<EffectCallSite> calls = ReadEffectCallSites(source, "Test/Moved.cpp");
    const auto lightOf = [&](int type) -> std::optional<PreviewVector>
    {
        const EffectCallSite* call = FindCall(calls, EffectKind::Effect, type);
        REQUIRE(call != nullptr);
        return call->light;
    };
    // A declaration with r, g, b, and a Vector of the same case.
    CHECK(IsLight(lightOf(MODEL_FIRE), 0.45f, 0.45f, 0.7f));
    CHECK(IsLight(lightOf(MODEL_POISON), 0.2f, 0.2f, 0.2f));
    // What an earlier case wrote does not reach a later case.
    CHECK(IsLight(lightOf(MODEL_STONE1), 1.0f, 1.0f, 1.0f));
    // Written on some paths only, changed after the Vector, or written in
    // another function: unknown.
    CHECK_FALSE(lightOf(MODEL_STONE2).has_value());
    CHECK_FALSE(lightOf(MODEL_BIG_STONE1).has_value());
    CHECK_FALSE(lightOf(MODEL_BIG_STONE2).has_value());
    CHECK_FALSE(lightOf(MODEL_ICE).has_value());
    // A create call that passes the colour last only reads it.
    const EffectCallSite* energy = FindCall(calls, EffectKind::Joint, BITMAP_JOINT_ENERGY);
    const EffectCallSite* thunder = FindCall(calls, EffectKind::Joint, BITMAP_JOINT_THUNDER);
    REQUIRE(energy != nullptr);
    REQUIRE(thunder != nullptr);
    CHECK(IsLight(energy->light, 0.4f, 0.4f, 1.0f));
    CHECK(IsLight(thunder->light, 0.4f, 0.4f, 1.0f));
}

TEST_CASE("The effect browser leaves out the calls in code the build skips [effects][editor]")
{
    const std::string source = R"source(
void Skipped(OBJECT* o, vec3_t Position, vec3_t Angle, vec3_t Light)
{
#ifdef NEVER_DEFINED
    CreateEffect(MODEL_FIRE, Position, Angle, Light, 0, o);
#else
    CreateEffect(MODEL_POISON, Position, Angle, Light, 0, o);
#endif
#if 0
    CreateEffect(MODEL_STONE1, Position, Angle, Light, 0, o);
#endif
#if defined(NEVER_DEFINED)
    CreateEffect(MODEL_ICE, Position, Angle, Light, 0, o);
#endif
#ifdef SOMETIMES_DEFINED
    CreateEffect(MODEL_STONE2, Position, Angle, Light, 0, o);
#endif
}
)source";
    const std::vector<EffectCallSite> calls = ReadEffectCallSites(source, "Test/Skipped.cpp", {"NEVER_DEFINED"});
    CHECK(FindCall(calls, EffectKind::Effect, MODEL_FIRE) == nullptr);
    CHECK(FindCall(calls, EffectKind::Effect, MODEL_STONE1) == nullptr);
    CHECK(FindCall(calls, EffectKind::Effect, MODEL_ICE) == nullptr);
    CHECK(FindCall(calls, EffectKind::Effect, MODEL_STONE2) != nullptr);
    const EffectCallSite* poison = FindCall(calls, EffectKind::Effect, MODEL_POISON);
    REQUIRE(poison != nullptr);
    CHECK(poison->line == 7);
}

TEST_CASE("The effect browser finds the game's calls of a type in the sources, the Fenrir's plasma storm among them "
          "[effects][editor]")
{
    EffectCallSiteIndex index;
    CHECK_FALSE(index.IsLoaded());
    REQUIRE(index.Load(std::filesystem::path(MU_TEST_SOURCE_DIR)));
    CHECK(index.Count() > 3000);

    const std::vector<const EffectCallSite*> bolts = index.Find(EffectKind::Joint, MODEL_FENRIR_SKILL_THUNDER);
    const auto plasmaStorm = std::find_if(bolts.begin(), bolts.end(),
                                          [](const EffectCallSite* call)
                                          {
                                              return call->file == "Engine/Object/ZzzCharacter.cpp" &&
                                                     call->subType == "0 + fenrirType" && call->owner == "p_to";
                                          });
    REQUIRE(plasmaStorm != bolts.end());
    CHECK((*plasmaStorm)->scaleValue == doctest::Approx(100.0f));
    CHECK_FALSE((*plasmaStorm)->subTypeValue.has_value());

    const std::vector<const EffectCallSite*> thunders = index.Find(EffectKind::Effect, MODEL_FENRIR_THUNDER);
    CHECK(std::any_of(thunders.begin(), thunders.end(),
                      [](const EffectCallSite* call) { return call->subTypeValue == 2 && call->light.has_value(); }));
    CHECK(index.Find(EffectKind::Particle, MODEL_FENRIR_SKILL_THUNDER).empty());

    // Lightning with a colour: the bolts of a monster's skill in the move
    // handlers, blue.
    const std::vector<const EffectCallSite*> thunderBolts = index.Find(EffectKind::Joint, BITMAP_JOINT_THUNDER);
    CHECK(std::any_of(thunderBolts.begin(), thunderBolts.end(),
                      [](const EffectCallSite* call)
                      {
                          return call->file == "Render/Effects/Behaviors/MoveHandlers.cpp" && call->light.has_value() &&
                                 (*call->light)[2] == doctest::Approx(1.0f) &&
                                 (*call->light)[0] == doctest::Approx(0.4f);
                      }));

    // The light that reaches the call: a declaration in a move handler,
    // where the functions are indented in a namespace, and the light of the
    // function rather than that of an earlier case.
    CHECK(HasCall(index.Find(EffectKind::Effect, MODEL_STONE1), "Render/Effects/Behaviors/MoveHandlers.cpp", 13, 0.45f,
                  0.45f, 0.7f));
    CHECK(HasCall(index.Find(EffectKind::Particle, BITMAP_SMOKE), "Render/Effects/Behaviors/MoveHandlers.cpp", 54,
                  0.45f, 0.45f, 0.7f));
    CHECK(HasCall(index.Find(EffectKind::Effect, MODEL_BIG_STONE_PART1), "Render/Models/ZzzBMD.cpp", 1, 1.0f, 1.0f,
                  1.0f));
    // Code under a macro nothing defines is left out.
    const std::vector<const EffectCallSite*> shiny = index.Find(EffectKind::Sprite, BITMAP_SHINY + 6);
    CHECK(std::none_of(shiny.begin(), shiny.end(), [](const EffectCallSite* call)
                       { return call->file == "World/GameMaps/GMEmpireGuardian2.cpp"; }));
    // The blade skill is created without an owner with SubType 0 only; its
    // SubType 1 reads its owner.
    const std::vector<const EffectCallSite*> blade = index.Find(EffectKind::Effect, MODEL_BLADE_SKILL);
    CHECK(CreatesWithoutOwner(blade, 0));
    CHECK_FALSE(CreatesWithoutOwner(blade, 1));

    EffectCallSiteIndex missing;
    CHECK_FALSE(missing.Load(std::filesystem::path(MU_TEST_SOURCE_DIR) / "no such folder"));
    CHECK(missing.IsLoaded());
}
#endif // _EDITOR
