#include "stdafx.h"

#include "doctest.h"

// The game's create calls of a type, which the effect browser reads from the
// sources and offers to the world preview (FX1.7b).
#ifdef _EDITOR
#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_enum.h"
#include "UI/EffectBrowser/EffectCallSites.h"

#include <algorithm>
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
} // namespace

TEST_CASE("The effect browser reads the game's create calls with the values they write out [effects][editor]")
{
    const std::string source = R"source(
void Skill(OBJECT* o, vec3_t Position)
{
    vec3_t Light, Angle, Target;
    Vector(0.2f, 0.2f, 1.0f, Light);
    CreateEffect(MODEL_FENRIR_THUNDER, Position, o->Angle, Light, 2, o);
    CreateJoint(BITMAP_JOINT_THUNDER, Position, Target, Angle, 3, NULL, 50.f, 10, 10);
    CreateParticleFpsChecked(BITMAP_FIRE, Position, Angle, o->Light);
    CreateEffect(MODEL_STONE1 + rand() % 2, Position, Angle, Light, 13, o);
    Vector(1.f, 0.5f, .25f, Light);
    CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o, 0.f);
    VectorCopy(o->Light, Light);
    CreateSprite(BITMAP_SHINY, Position, 1.f, Light, o);
    // CreateEffect(MODEL_POISON, Position, Angle, Light);
    Log("CreateEffect(MODEL_POISON, Position, Angle, Light)");
    g_SkillEffects.CreateEffect();
}

void CreateEffect(int Type, vec3_t Position, vec3_t Angle, vec3_t Light)
{
}
)source";
    const std::vector<EffectCallSite> calls = ReadEffectCallSites(source, "Test/Skill.cpp");
    CHECK(calls.size() == 7);
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
    CHECK(joint->pkValue == 10);
    CHECK(joint->skillIndexValue == 10);

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
    // A copy into the light after the last Vector leaves it unknown.
    const EffectCallSite* shiny = FindCall(calls, EffectKind::Sprite, BITMAP_SHINY);
    REQUIRE(shiny != nullptr);
    CHECK_FALSE(shiny->light.has_value());
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

    EffectCallSiteIndex missing;
    CHECK_FALSE(missing.Load(std::filesystem::path(MU_TEST_SOURCE_DIR) / "no such folder"));
    CHECK(missing.IsLoaded());
}
#endif // _EDITOR
