#include "stdafx.h"
#include "Core/Utilities/Random.h"

#include "ItemEffects.h"
#include "ItemModelTable.h"

#include "LookDrawing.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookJson.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace Render
{
DrawLook DrawLook::Of(const OBJECT& o)
{
    return {o.Alpha, o.BlendMesh, o.BlendMeshLight, o.BlendMeshTexCoordU, o.BlendMeshTexCoordV, o.HiddenMesh};
}
} // namespace Render

namespace Render::Items::ItemEffects
{
namespace
{
// An effect runs before the model is drawn, and may change the level the
// model glows like and the alpha it is drawn with. Drawn: it drew the model
// itself, nothing more is drawn.
using EffectFunction = Result (*)(BMD* b, const OBJECT* o, DrawLook& look, int Type, float& Alpha, int& Level);
// Spawns the effect's sprites, particles and lightning from the model's pose, in the update.
using SpawnFunction = void (*)(BMD* b, OBJECT* o, vec34_t* bones);

struct ItemEffect
{
    const char* name;
    EffectFunction apply;
    SpawnFunction spawn = nullptr;
};

// ------------------------------------------------ the effects

// For effects that only spawn: the model is drawn as usual.
Result ApplyNothing(BMD*, const OBJECT*, DrawLook&, int, float&, int&)
{
    return Result::Applied;
}

void SpawnDevilsKey(BMD* b, OBJECT* o, vec34_t* bones)
{
    float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
    vec3_t p, Position, EffLight;
    Vector(0.f, 0.f, 0.f, p);

    float Scale = Luminosity * 0.8f;
    Vector(Luminosity * 2, Luminosity * 0.32f, Luminosity * 0.32f, EffLight);

    b->TransformPosition(bones[1], p, Position);
    VectorAdd(Position, o->Position, Position);
    CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);

    b->TransformPosition(bones[2], p, Position);
    VectorAdd(Position, o->Position, Position);
    CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);
}

void SpawnDevilsInvitation(BMD* b, OBJECT* o, vec34_t* bones)
{
    float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
    vec3_t p, Position, EffLight;
    Vector(0.f, 0.f, 0.f, p);

    float Scale = Luminosity * 0.8f;
    Vector(Luminosity * 2, Luminosity * 0.32f, Luminosity * 0.32f, EffLight);

    b->TransformPosition(bones[9], p, Position);
    VectorAdd(Position, o->Position, Position);
    CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);

    b->TransformPosition(bones[10], p, Position);
    VectorAdd(Position, o->Position, Position);
    CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);
}

void SpawnRena(BMD* b, OBJECT* o, vec34_t* bones)
{
    float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.25f + 0.75f;
    vec3_t EffLight;

    Vector(Luminosity * 1.f, Luminosity * 0.5f, Luminosity * 0.f, EffLight);
    CreateSprite(BITMAP_SPARK + 1, o->Position, 2.5f, EffLight, o);
}

void SpawnWingsOfDarkness(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t posCenter, p, Position, Light;
    float Scale = sinf(WorldTime * 0.004f) * 0.3f + 0.3f;

    Scale = (Scale * 10.f) + 20.f;

    Vector(0.6f, 0.3f, 0.8f, Light);

    Vector(0.f, 0.f, 0.f, p);

    for (int i = 0; i < 5; ++i)
    {
        b->TransformPosition(bones[22 - i], p, posCenter, true);
        b->TransformPosition(bones[30 - i], p, Position, true);
        if (rand_fps_check(1))
        {
            CreateJoint(BITMAP_JOINT_THUNDER, Position, posCenter, o->Angle, 14, o, Scale);
            CreateJoint(BITMAP_JOINT_SPIRIT, posCenter, Position, o->Angle, 4, o, Scale + 5);
        }

        CreateSprite(BITMAP_FLARE_BLUE, posCenter, Scale / 28.f, Light, o);
    }

    for (int i = 0; i < 5; ++i)
    {
        b->TransformPosition(bones[7 - i], p, posCenter, true);
        b->TransformPosition(bones[11 + i], p, Position, true);
        if (rand_fps_check(1))
        {
            CreateJoint(BITMAP_JOINT_THUNDER, Position, posCenter, o->Angle, 14, o, Scale);
            CreateJoint(BITMAP_JOINT_SPIRIT, posCenter, Position, o->Angle, 4, o, Scale + 5);
        }

        CreateSprite(BITMAP_FLARE_BLUE, posCenter, Scale / 28.f, Light, o);
    }
}

void SpawnWingOfStorm(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t vRelativePos, vPos, vLight;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    Vector(0.f, 0.f, 0.f, vPos);
    Vector(0.f, 0.f, 0.f, vLight);

    float fLuminosity = absf(sinf(WorldTime * 0.0004f)) * 0.4f;
    Vector(0.5f + fLuminosity, 0.5f + fLuminosity, 0.5f + fLuminosity, vLight);
    int iBone[] = {9, 20, 19, 10, 18, 28, 27, 36, 35, 38, 37, 53, 48, 62, 70, 72, 71, 78, 79, 80, 87, 90, 91, 106, 102};
    float fScale = 0.f;

    for (int i = 0; i < 25; ++i)
    {
        b->TransformPosition(bones[iBone[i]], vRelativePos, vPos, true);
        fScale = 0.5f; // (Random::Rand()%10) * 0.05f + 0.3f;
        CreateSprite(BITMAP_CLUD64, vPos, fScale, vLight, o, WorldTime * 0.01f, 1);
    }

    int iBoneThunder[] = {11, 21, 29, 63, 81, 89};
    if (rand_fps_check(2))
    {
        for (int i = 0; i < 6; ++i)
        {
            b->TransformPosition(bones[iBoneThunder[i]], vRelativePos, vPos, true);
            if (Random::Rand() % 20 == 0)
            {
                Vector(0.6f, 0.6f, 0.9f, vLight);
                CreateEffect(MODEL_FENRIR_THUNDER, vPos, o->Angle, vLight, 1, o);
            }
        }
    }

    int iBoneLight[] = {64, 61, 69, 77, 86, 98, 97, 99, 104, 103, 105, 12, 8, 17, 26, 34, 52, 44, 51, 50, 49, 45};

    fScale = absf(sinf(WorldTime * 0.003f)) * 0.2f;

    for (int i = 0; i < 22; ++i)
    {
        b->TransformPosition(bones[iBoneLight[i]], vRelativePos, vPos, true);
        if (iBoneLight[i] == 12 || iBoneLight[i] == 64 || iBoneLight[i] == 98 || iBoneLight[i] == 52)
        {
            Vector(0.9f, 0.0f, 0.0f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, fScale + 1.4f, vLight, o);
        }
        else
        {
            Vector(0.8f, 0.5f, 0.2f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, fScale + 0.3f, vLight, o);
        }
    }
}

void SpawnWingOfEternal(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t p, Position, Light;
    Vector(0.f, 0.f, 0.f, p);
    float Scale = absf(sinf(WorldTime * 0.003f)) * 0.2f;
    float Luminosity = absf(sinf(WorldTime * 0.003f)) * 0.3f;

    Vector(0.5f + Luminosity, 0.5f + Luminosity, 0.6f + Luminosity, Light);
    // int iRedFlarePos[] = { 25, 32, 53, 15, 9, 35 };
    int iRedFlarePos[] = {24, 31, 15, 8, 53, 35};
    for (int i = 0; i < 6; ++i)
    {
        b->TransformPosition(bones[iRedFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale + 1.3f, Light, o);
    }

    Vector(0.1f, 0.1f, 0.9f, Light);
    // int iGreenFlarePos[] = { 23, 22, 24, 34, 5, 31, 14, 12, 27, 8, 6, 7, 16, 13, 56, 37, 58, 40, 39, 38 };
    int iGreenFlarePos[] = {22, 23, 25, 29, 30, 28, 32, 13, 16, 14, 12, 9, 7, 6, 57, 58, 40, 39};

    for (int i = 0; i < 18; ++i)
    {
        b->TransformPosition(bones[iGreenFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale + 1.5f, Light, o);
    }
    int iGreenFlarePos2[] = {56, 38, 51, 45};

    for (int i = 0; i < 4; ++i)
    {
        b->TransformPosition(bones[iGreenFlarePos2[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale + 0.5f, Light, o);
    }
}

void SpawnWingOfIllusion(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t p, Position, Light;
    Vector(0.f, 0.f, 0.f, p);
    float Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
    float Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;

    Vector(0.5f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, Light);
    int iRedFlarePos[] = {5, 6, 7, 8, 18, 19, 23, 24, 25, 27, 37, 38};
    for (int i = 0; i < 12; ++i)
    {
        b->TransformPosition(bones[iRedFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_FLARE, Position, Scale + 0.6f, Light, o);
    }

    Vector(0.0f + Luminosity, 0.5f + Luminosity, 0.0f + Luminosity, Light);
    int iGreenFlarePos[] = {4, 9, 13, 14, 26, 32, 31, 33};

    for (int i = 0; i < 8; ++i)
    {
        b->TransformPosition(bones[iGreenFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
    }

    Vector(1.0f, 1.0f, 1.0f, Light);
    float fLumi = (sinf(WorldTime * 0.004f) + 1.0f) * 0.05f;
    Vector(0.8f + fLumi, 0.8f + fLumi, 0.3f + fLumi, Light);
    CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o, 0.5f);
    if (rand_fps_check(2))
    {
        b->TransformPosition(bones[13], p, Position, true);
        CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 5, 0.5f);
        b->TransformPosition(bones[31], p, Position, true);
        CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 5, 0.5f);
    }
}

void SpawnWingOfRuin(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t p, Position, Light;
    Vector(0.f, 0.f, 0.f, p);
    float Scale = absf(sinf(WorldTime * 0.003f)) * 0.2f;
    float Luminosity = absf(sinf(WorldTime * 0.003f)) * 0.3f;

    Vector(0.7f + Luminosity, 0.5f + Luminosity, 0.8f + Luminosity, Light);
    int iRedFlarePos[] = {6, 15, 24, 56, 47, 38};
    for (int i = 0; i < 6; ++i)
    {
        b->TransformPosition(bones[iRedFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale + 1.5f, Light, o);
    }

    Vector(0.6f, 0.4f, 0.7f, Light);
    int iSparkPos[] = {7,  16, 25, 57, 48, 39, 11, 22, 31, 63, 54, 40, 10, 21, 30, 62, 53, 41, 9,
                       20, 29, 61, 52, 42, 8,  19, 28, 60, 51, 43, 18, 27, 59, 50, 17, 26, 58, 49};
    int iNumParticle = 1;

    for (int i = 0; i < 6; ++i)
    {
        b->TransformPosition(bones[iSparkPos[i]], p, Position, true);
        for (int j = 0; j < iNumParticle; ++j)
            if (rand_fps_check(1))
                CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.1f);
    }

    for (int i = 6; i < 18; ++i)
    {
        b->TransformPosition(bones[iSparkPos[i]], p, Position, true);
        for (int j = 0; j < iNumParticle; ++j)
            if (rand_fps_check(1))
                CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.3f);
    }

    for (int i = 18; i < 30; ++i)
    {
        b->TransformPosition(bones[iSparkPos[i]], p, Position, true);
        for (int j = 0; j < iNumParticle; ++j)
            if (rand_fps_check(1))
                CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.5f);
    }

    for (int i = 30; i < 38; ++i)
    {
        b->TransformPosition(bones[iSparkPos[i]], p, Position, true);
        for (int j = 0; j < iNumParticle; ++j)
            if (rand_fps_check(1))
                CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.7f);
    }
}

void SpawnWingOfDimension(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t p, Position, Light;
    Vector(0.f, 0.f, 0.f, p);
    float Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
    float Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;

    Vector((1.0f + Luminosity) / 2.f, (0.7f + Luminosity) / 2.f, (0.2f + Luminosity) / 2.f, Light);
    int iFlarePos0[] = {7, 30, 31, 43, 8, 20};

    int icnt;
    for (icnt = 0; icnt < 2; ++icnt)
    {
        b->TransformPosition(bones[iFlarePos0[icnt]], p, Position, true);
        CreateSprite(BITMAP_FLARE, Position, Scale + 2.0f, Light, o);
    }
    Vector((1.0f + Luminosity) / 4.f, (0.7f + Luminosity) / 4.f, (0.2f + Luminosity) / 4.f, Light);
    for (; icnt < 6; ++icnt)
    {
        b->TransformPosition(bones[iFlarePos0[icnt]], p, Position, true);
        CreateSprite(BITMAP_FLARE, Position, Scale + 0.5f, Light, o);
    }

    Vector((0.5f + Luminosity) / 2.f, (0.1f + Luminosity) / 2.f, (0.4f + Luminosity) / 2.f, Light);
    int iGreenFlarePos[] = {29, 38, 42, 19, 15, 6};

    for (int i = 0; i < 6; ++i)
    {
        b->TransformPosition(bones[iGreenFlarePos[i]], p, Position, true);
        CreateSprite(BITMAP_FLARE, Position, Scale + 2.0f, Light, o);
    }
}

// ------------------------------------------------ the names

const ItemEffect ItemEffects[] = {
    {"devilsKey", ApplyNothing, SpawnDevilsKey},
    {"devilsInvitation", ApplyNothing, SpawnDevilsInvitation},
    {"rena", ApplyNothing, SpawnRena},
    {"wingsOfDarkness", ApplyNothing, SpawnWingsOfDarkness},
    {"wingOfStorm", ApplyNothing, SpawnWingOfStorm},
    {"wingOfEternal", ApplyNothing, SpawnWingOfEternal},
    {"wingOfIllusion", ApplyNothing, SpawnWingOfIllusion},
    {"wingOfRuin", ApplyNothing, SpawnWingOfRuin},
    {"wingOfDimension", ApplyNothing, SpawnWingOfDimension},
};

const ItemEffect* FindEffect(std::string_view name)
{
    const auto found = std::find_if(std::begin(ItemEffects), std::end(ItemEffects),
                                    [name](const ItemEffect& effect) { return effect.name == name; });
    return found != std::end(ItemEffects) ? found : nullptr;
}

// The item effect of an item: one still in code, or one that is data as the item has it.
struct ItemEffectOf
{
    const ItemEffect* code = nullptr;
    const Data::Looks::LookDefinition* data = nullptr;
};

// An item effect that is data as an item that sets values of it has it (design L12); those are made again with the
// effect table, after a change of the item models or of the looks.
const Data::Looks::LookDefinition* DataEffectOfItem(const Data::Items::ItemModelDefinition& model,
                                                    const Data::Looks::LookDefinition& effect)
{
    if (model.itemEffectValues.empty() || !Data::Looks::SetsEachItem(effect))
    {
        return &effect;
    }
    static ItemLooks<Data::Looks::LookDefinition> itemEffects;
    // A value that can't be one keeps the look's; Data Checks names the problem.
    std::vector<Data::Items::ItemDataIssue> issues;
    return itemEffects.Keep(
        Data::Items::MakeItemType(model.group, model.number),
        Data::Looks::ResolveForItem(effect, model.itemEffectValues, "", issues, Data::Looks::LookKind::ItemEffect));
}

// The effect of every item type, taken from the item model database after
// each build of it and after each change of the looks that are data.
ItemModelTable<ItemEffectOf> g_itemEffects{[](const Data::Items::ItemModelDefinition& model) -> ItemEffectOf
                                           {
                                               if (model.itemEffect.empty())
                                               {
                                                   return {};
                                               }
                                               if (const Data::Looks::LookDefinition* data =
                                                       Data::Looks::g_LookStore.FindItemEffect(model.itemEffect))
                                               {
                                                   return {nullptr, DataEffectOfItem(model, *data)};
                                               }
                                               // Names that are not effects are drawn without one; the
                                               // model loader reports them.
                                               return {FindEffect(model.itemEffect), nullptr};
                                           },
                                           [] { return Data::Looks::g_LookStore.GetVersion(); }};

#ifdef _EDITOR
// PreviewEffect's model and effect, while one lives.
int s_previewModel = -1;
const Data::Looks::LookDefinition* s_previewEffect = nullptr;
#endif

// The model's effect, or the one the Style Editor previews on it.
ItemEffectOf EffectOfModel(int modelType)
{
#ifdef _EDITOR
    if (s_previewEffect != nullptr && modelType == s_previewModel)
    {
        return {nullptr, s_previewEffect};
    }
#endif
    return g_itemEffects.Find(modelType);
}
} // namespace

#ifdef _EDITOR
PreviewEffect::PreviewEffect(int modelType, const Data::Looks::LookDefinition& effect)
{
    s_previewModel = modelType;
    s_previewEffect = &effect;
}

PreviewEffect::~PreviewEffect()
{
    s_previewModel = -1;
    s_previewEffect = nullptr;
}
#endif

bool Exists(std::string_view name)
{
    return Data::Looks::g_LookStore.FindItemEffect(name) != nullptr || FindEffect(name) != nullptr;
}

bool ExistsInCode(std::string_view name)
{
    return FindEffect(name) != nullptr;
}

std::span<const std::string_view> GetNames()
{
    static std::vector<std::string_view> names;
    static int version = -1;
    if (version != Data::Looks::g_LookStore.GetVersion())
    {
        version = Data::Looks::g_LookStore.GetVersion();
        names.clear();
        for (const ItemEffect& effect : ItemEffects)
        {
            names.emplace_back(effect.name);
        }
        for (const Data::Looks::LookDefinition& look : Data::Looks::g_LookStore.GetItemEffects())
        {
            names.emplace_back(look.name);
        }
        std::ranges::sort(names);
        names.erase(std::unique(names.begin(), names.end()), names.end());
    }
    return names;
}

Result Apply(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float& alpha, int& level)
{
    const ItemEffectOf effect = EffectOfModel(modelType);
    if (effect.data != nullptr)
    {
        Looks::ApplyItemEffect(*effect.data, b, o, look, alpha, level);
        return Result::Applied;
    }
    return effect.code != nullptr ? effect.code->apply(b, o, look, modelType, alpha, level) : Result::None;
}

bool Spawns(int modelType)
{
    const ItemEffectOf effect = EffectOfModel(modelType);
    if (effect.data != nullptr)
    {
        return effect.data->objectLight.has_value();
    }
    return effect.code != nullptr && effect.code->spawn != nullptr;
}

void Spawn(BMD* b, OBJECT* o, int modelType, float (*bones)[3][4])
{
    const ItemEffectOf effect = EffectOfModel(modelType);
    if (effect.data != nullptr)
    {
        Looks::SpawnItemEffect(*effect.data, o);
    }
    else if (effect.code != nullptr && effect.code->spawn != nullptr)
    {
        effect.code->spawn(b, o, bones);
    }
}
} // namespace Render::Items::ItemEffects
