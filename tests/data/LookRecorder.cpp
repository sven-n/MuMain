#include "stdafx.h"

#include "LookRecorder.h"

#include "TestModelSlots.h"

#include "Core/Globals/_enum.h"
#include "Core/Utilities/Random.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/DataHandler/ItemData/ItemModelLoader.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/PartModelSlots.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Items/ItemRenderStyles.h"
#include "Render/Items/LookDrawing.h"
#include "Render/Drawing/DrawCallLog.h"
#include "Render/Models/ModelDraw.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Textures/ZzzTexture.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <filesystem>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace LookRecorder
{
namespace
{
using Render::ModelDrawing::MeshDraw;

const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;
const std::filesystem::path ClientDirectory = DataDirectory.parent_path();

// A time well into a session, with a fraction, so a look that rounds the clock shows it.
constexpr double PinnedWorldTime = 1234567.25;

constexpr int ReflectionFlags = RENDER_CHROME | RENDER_CHROME2 | RENDER_CHROME3 | RENDER_CHROME4 | RENDER_CHROME5 |
                                RENDER_CHROME6 | RENDER_CHROME7 | RENDER_METAL | RENDER_OIL;

std::string Number(double value, int digits = 9)
{
    char text[40];
    std::snprintf(text, sizeof(text), "%.*g", digits, value);
    return text;
}

std::string Hex(unsigned value)
{
    char text[16];
    std::snprintf(text, sizeof(text), "0x%x", value);
    return text;
}

std::vector<Field> FieldsOf(const MeshDraw& draw)
{
    int flags = draw.flags;
    // BMD::DrawMesh reads the texture bit only for a pass without a reflection, and the light map bit only for one
    // with a reflection.
    flags &= (flags & ReflectionFlags) != 0 ? ~RENDER_TEXTURE : ~RENDER_LIGHTMAP;
    // Nor for a pass on its glow mesh (ResolveMaterial draws it so before it reads the bit): such a pass counts as one
    // with the bit.
    const bool onGlowMesh = (flags & (ReflectionFlags | RENDER_COLOR)) == 0 && draw.model != nullptr &&
                            (draw.blendMesh <= -2 || draw.model->Meshs[draw.mesh].Texture == draw.blendMesh);
    if (onGlowMesh)
    {
        flags |= RENDER_TEXTURE;
    }
    return {
        {"mesh", std::to_string(draw.mesh)},
        {"flags", Hex(static_cast<unsigned>(flags))},
        {"alpha", Number(draw.alpha)},
        {"blendMesh", std::to_string(draw.blendMesh)},
        {"blendMeshAlpha", Number(draw.blendMeshAlpha)},
        {"blendMeshU", Number(draw.blendMeshU)},
        {"blendMeshV", Number(draw.blendMeshV)},
        {"texture", std::to_string(draw.texture)},
        {"bodyLight", Number(draw.bodyLight[0]) + " " + Number(draw.bodyLight[1]) + " " + Number(draw.bodyLight[2])},
        {"lightEnable", draw.lightEnable ? "1" : "0"},
        {"hideSkin", draw.hideSkin ? "1" : "0"},
        // The drawing reads the scrolling mesh only for the mesh it draws.
        {"streamMesh", std::to_string(draw.streamMesh == draw.mesh ? draw.mesh : -1)},
    };
}

// Writes each mesh draw down and hands it on to the normal drawing.
class RecordingDrawer final : public Render::ModelDrawing::ModelDrawer
{
public:
    void DrawMesh(const MeshDraw& draw) override
    {
        draws.push_back(FieldsOf(draw));
        draw.model->DrawMesh(draw);
    }

    void DrawShadow(const Render::ModelDrawing::ShadowDraw&) override {}

    std::vector<std::vector<Field>> draws;
};

const Data::Items::ItemModelDataLoadResult& ShippedModels()
{
    static const Data::Items::ItemModelDataLoadResult result =
        Data::Items::LoadItemModelDataDirectory(DataDirectory / "Items" / "Models");
    return result;
}

// The one object every drawing draws.
OBJECT& DrawnObject()
{
    static OBJECT object;
    return object;
}

// The key Random::DrawRand takes for the drawn object: an address no object has, the same in every run.
const void* FixedDrawKey()
{
    return reinterpret_cast<const void*>(static_cast<std::uintptr_t>(0x10000));
}

std::string Join(const std::vector<Field>& fields)
{
    std::string text;
    for (const auto& [name, value] : fields)
    {
        text += text.empty() ? "" : " ";
        text += std::string(name) + "=" + value;
    }
    return text;
}

// Float steps (units in the last place) between two floats, through their bits in the order of the numbers.
long long FloatSteps(float a, float b)
{
    const auto ordered = [](float value)
    {
        std::int32_t bits = 0;
        std::memcpy(&bits, &value, sizeof(bits));
        return bits >= 0 ? static_cast<long long>(bits) : -static_cast<long long>(bits & 0x7fffffff);
    };
    return std::llabs(ordered(a) - ordered(b));
}

// Two field values that are the same text, or the same count of numbers, each at most `steps` float steps apart.
bool Close(const std::string& a, const std::string& b, int steps)
{
    if (a == b)
    {
        return true;
    }
    if (steps == 0)
    {
        return false;
    }
    const char* x = a.c_str();
    const char* y = b.c_str();
    while (*x != '\0' || *y != '\0')
    {
        char* xEnd = nullptr;
        char* yEnd = nullptr;
        const float first = std::strtof(x, &xEnd);
        const float second = std::strtof(y, &yEnd);
        if (xEnd == x || yEnd == y || FloatSteps(first, second) > steps)
        {
            return false;
        }
        x = xEnd;
        y = yEnd;
    }
    return true;
}

// The grids the digest's numbers are cut on: a number becomes floor(number / step + offset), so a grid's edges are at
// whole steps minus its offset, a fifth of a step or more from the other grids' edges and a tenth of a step or more
// from the whole steps (an exact number such as 1 or 0.5). The steps are 0.0001, finer than any change a look shows:
// a number a float step off near 1 crosses a given grid's edge about once in a thousand.
constexpr double GridStep = 0.0001;
constexpr std::array<double, 5> GridOffsets{0.5, 0.7, 0.9, 0.1, 0.3};
static_assert(GridOffsets.size() == LookRecorder::RelativeFirstEdges.size());

// The size of a number's step on the relative grid `gridIndex`, in steps of GridStep: 1 up to the grid's first edge,
// then the power of two at most twice the number over that edge, so a step is 0.7 to 1.5 ten-thousandths of the
// number. Each grid's edges between those sizes sit at its first edge (RelativeFirstEdges) times powers of two: a fifth
// of a power of two (a factor of 1.15) from the other grids' edges, so a number a float step from one grid's edge
// (computed, as a bone's height above the item) changes that grid's step only. frexp is exact on every platform.
double RelativeScale(double value, std::size_t gridIndex)
{
    const double size = std::fabs(value) / LookRecorder::RelativeFirstEdges[gridIndex];
    if (!(size > 1.0) || !std::isfinite(size))
    {
        return 1.0;
    }
    int exponent = 0;
    std::frexp(size, &exponent);
    return std::ldexp(1.0, exponent);
}

// Every number of the text as a whole number of steps on the grid. Another platform's sine, or a multiply and add the
// compiler fuses (ARM64), moves a number by a last float step of the sine (about 1e-7 near 1, also for a small number
// the sine was scaled to), far below a fifth of a step: a number near one grid's edge is far from the others'. On the
// relative grid a large number (a place in the world, near 13000) has a step its own float steps stay far below too.
std::string Rounded(const std::string& text, std::size_t gridIndex, LookRecorder::Grid grid)
{
    const double offset = GridOffsets[gridIndex];
    std::string result;
    size_t i = 0;
    while (i < text.size())
    {
        const char c = text[i];
        const bool startsNumber =
            (c >= '0' && c <= '9') || (c == '-' && i + 1 < text.size() && text[i + 1] >= '0' && text[i + 1] <= '9');
        if (!startsNumber || (i > 0 && (std::isalnum(static_cast<unsigned char>(text[i - 1])) != 0)))
        {
            result += c;
            ++i;
            continue;
        }
        char* end = nullptr;
        const double value = std::strtod(text.c_str() + i, &end);
        // Beyond this the steps don't fit a whole number; no look's number comes near it.
        constexpr double MaxSteps = 1.0e15;
        const double step =
            grid == LookRecorder::Grid::Relative ? GridStep * RelativeScale(value, gridIndex) : GridStep;
        const double steps = std::floor(value / step + offset);
        result += std::isfinite(steps) && std::fabs(steps) < MaxSteps ? std::to_string(static_cast<long long>(steps))
                                                                      : Number(value, 6);
        i = static_cast<size_t>(end - text.c_str());
    }
    return result;
}
} // namespace

std::string Situation::Describe() const
{
    std::string text = "+" + std::to_string(level) + " at " + Number(worldTime / 1000.0, 7) + " s";
    if ((renderType & RENDER_DOPPELGANGER) != 0)
    {
        text += ", doppelganger";
    }
    if ((renderType & RENDER_EXTRA) != 0)
    {
        text += ", in a monster's hands";
    }
    if (alpha < 1.f)
    {
        text += ", fading " + Number(alpha, 3);
    }
    if (hideSkin)
    {
        text += ", without the skin";
    }
    if (excellent)
    {
        text += ", excellent";
    }
    if (infinityArrow)
    {
        text += ", infinity arrow";
    }
    if (turn != 0.f)
    {
        text += ", turned " + Number(turn, 3);
    }
    if (light != 1.f)
    {
        text += ", lit " + Number(light, 3);
    }
    if (inInventory)
    {
        text += ", in the inventory";
    }
    return text;
}

std::vector<Situation> Situations()
{
    const Situation plain{.worldTime = PinnedWorldTime, .renderType = RENDER_TEXTURE};
    std::vector<Situation> situations{plain};
    for (const int level : {7, 9, 11, 13, 15})
    {
        Situation s = plain;
        s.level = level;
        situations.push_back(s);
    }
    for (const double later : {1250.0, 3700.0})
    {
        Situation s = plain;
        s.worldTime += later;
        situations.push_back(s);
    }
    for (const int extra : {RENDER_DOPPELGANGER, RENDER_EXTRA})
    {
        Situation s = plain;
        s.renderType |= extra;
        situations.push_back(s);
    }
    Situation fading = plain;
    fading.alpha = 0.5f;
    situations.push_back(fading);
    Situation withoutSkin = plain;
    withoutSkin.hideSkin = true;
    situations.push_back(withoutSkin);
    Situation excellent = plain;
    excellent.excellent = true;
    situations.push_back(excellent);
    Situation glowingLater = plain;
    glowingLater.level = 9;
    glowingLater.worldTime += 1250.0;
    situations.push_back(glowingLater);
    return situations;
}

std::vector<Situation> SituationsFor(std::string_view renderStyle, Data::Looks::LookKind kind)
{
    std::vector<Situation> situations = Situations();
    const Data::Looks::LookDefinition* look = Data::Looks::g_LookStore.Find(kind, renderStyle);
    if (look == nullptr)
    {
        return situations;
    }
    // The conditions of the layers and of the item values before the drawing.
    std::vector<Data::Looks::OnlyWhen> conditions;
    for (const Data::Looks::Layer& layer : look->layers)
    {
        conditions.push_back(layer.onlyWhen);
    }
    for (const Data::Looks::ItemValuesBefore& entry : look->itemValuesBefore)
    {
        conditions.push_back(entry.onlyWhen);
    }
    const auto any = [&conditions](auto&& condition) { return std::ranges::any_of(conditions, condition); };
    const Situation plain = situations.front();
    if (any([](const Data::Looks::OnlyWhen& when) { return when.buff || when.withoutBuff; }))
    {
        Situation buffed = plain;
        buffed.infinityArrow = true;
        situations.push_back(buffed);
        buffed.level = 9;
        situations.push_back(buffed);
    }
    // A value of the turning angle: the item also turned a sixth and two thirds round.
    const auto turns = [](const Data::Looks::Value& value)
    {
        const auto* shape = std::get_if<Data::Looks::Shape>(&value);
        return shape != nullptr && shape->kind == Data::Looks::ShapeKind::Turn;
    };
    if (std::ranges::any_of(look->layers,
                            [&turns](const Data::Looks::Layer& layer)
                            {
                                return turns(layer.opacity) || turns(layer.glowStrength) || turns(layer.scrollU) ||
                                       turns(layer.scrollV);
                            }))
    {
        for (const float turn : {60.f, 240.f})
        {
            Situation turned = plain;
            turned.turn = turn;
            situations.push_back(turned);
        }
    }
    const bool inventoryOrGround =
        any([](const Data::Looks::OnlyWhen& when)
            { return when.place == Data::Looks::Place::Inventory || when.place == Data::Looks::Place::Ground; });
    if (inventoryOrGround)
    {
        for (const int level : {0, 1, 3, 7})
        {
            Situation inventory = plain;
            inventory.level = level;
            inventory.hideSkin = true;
            inventory.inInventory = true;
            situations.push_back(inventory);
        }
    }
    if (any([](const Data::Looks::OnlyWhen& when) { return when.level || when.hiddenMesh; }))
    {
        for (int level = 1; level <= 6; ++level)
        {
            Situation s = plain;
            s.level = level;
            situations.push_back(s);
        }
    }
    if (inventoryOrGround)
    {
        // Only the inventory draws in white light.
        for (Situation& situation : situations)
        {
            situation.light = situation.inInventory ? 1.f : 0.8f;
        }
    }
    return situations;
}

struct ShippedItems::State
{
    TestModelSlots slots;
    std::filesystem::path previousDirectory = std::filesystem::current_path();
};

ShippedItems::ShippedItems() : m_state(new State)
{
    // The model entries name their files and texture folders from the client folder.
    std::filesystem::current_path(ClientDirectory);
    Render::Sprites::SetGpuUploadsSkipped(true);
    const Data::Items::GlowColorsLoadResult colors = Data::Items::LoadGlowColorsFile(Data::Items::GetGlowColorsFile());
    Data::Effects::GlowColorList glowColors;
    glowColors.Build(colors.colors);
    g_ItemModelDatabase.Build(ShippedModels().models, glowColors);
    g_ItemModelDatabase.SetPartModels(ShippedModels().partModels);
    // The shines below +3 the glow of the models names.
    g_Shines.Build(Data::Items::LoadShinesFile(Data::Items::GetShinesFile()).shines);
}

ShippedItems::~ShippedItems()
{
    g_ItemModelDatabase.Build({}, Data::Effects::GlowColorList{});
    g_ItemModelDatabase.SetPartModels({});
    g_Shines.Build({});
    Data::Items::ModelLoader::TakeProblemMessage();
    Render::Sprites::SetGpuUploadsSkipped(false);
    std::filesystem::current_path(m_state->previousDirectory);
    delete m_state;
}

// The textures of OpenBasicData (ZzzOpenData.cpp) that looks name, at the indices the game loads them at before the
// looks find theirs: a look in code names the index, the data look of its name the file, so a wrong index here shows
// as a texture difference in the comparison.
struct BasicTexture
{
    const wchar_t* file;
    GLuint index;
    GLuint wrap;
};
constexpr BasicTexture BasicTextures[] = {
    {L"Effect\\Chrome01.jpg", BITMAP_CHROME, GL_REPEAT},
    {L"Effect\\bab2.jpg", BITMAP_CHROME + 1, GL_REPEAT},
    {L"Effect\\Chrome06.jpg", BITMAP_CHROME6, GL_CLAMP_TO_EDGE},
    {L"Effect\\Chrome07.jpg", BITMAP_CHROME7, GL_CLAMP_TO_EDGE},
    {L"Effect\\energy01.jpg", BITMAP_CHROME_ENERGY, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenArmorMale40.tga", BITMAP_INVEN_ARMOR, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenArmorMale41.tga", BITMAP_INVEN_ARMOR + 1, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenArmorMale42.tga", BITMAP_INVEN_ARMOR + 2, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenArmorMale43.tga", BITMAP_INVEN_ARMOR + 3, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenArmorMale44.tga", BITMAP_INVEN_ARMOR + 4, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenArmorMale45.tga", BITMAP_INVEN_ARMOR + 5, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenPantsMale40.tga", BITMAP_INVEN_PANTS, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenPantsMale41.tga", BITMAP_INVEN_PANTS + 1, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenPantsMale42.tga", BITMAP_INVEN_PANTS + 2, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenPantsMale43.tga", BITMAP_INVEN_PANTS + 3, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenPantsMale44.tga", BITMAP_INVEN_PANTS + 4, GL_CLAMP_TO_EDGE},
    {L"Player\\InvenPantsMale45.tga", BITMAP_INVEN_PANTS + 5, GL_CLAMP_TO_EDGE},
    {L"Player\\Item312_Armoritem.tga", BITMAP_SKIN_ARMOR_DEVINE, GL_CLAMP_TO_EDGE},
    {L"Player\\Item312_Pantitem.tga", BITMAP_SKIN_PANTS_DEVINE, GL_CLAMP_TO_EDGE},
    {L"Player\\SkinClass706_upperitem.tga", BITMAP_SKIN_ARMOR_SUCCUBUS, GL_CLAMP_TO_EDGE},
    {L"Player\\SkinClass706_loweritem.tga", BITMAP_SKIN_PANTS_SUCCUBUS, GL_CLAMP_TO_EDGE},
    {L"Item\\Ingameshop\\kacama_R.jpg", BITMAP_CHARACTERCARD_R_MA, GL_CLAMP_TO_EDGE},
    {L"Item\\Ingameshop\\kacada_R.jpg", BITMAP_CHARACTERCARD_R_DA, GL_CLAMP_TO_EDGE},
    {L"Effect\\Lava.jpg", BITMAP_LAVA, GL_CLAMP_TO_EDGE},
    {L"Item\\soketmagic_stape02.jpg", BITMAP_SOCKETSTAFF, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\entrance_R.jpg", BITMAP_FREETICKET_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\juju_R.jpg", BITMAP_CHAOSCARD_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\bujuck01alpa.jpg", BITMAP_LUCKY_CHARM_EFFECT53, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\monmark01a.jpg", BITMAP_LUCKY_SEAL_EFFECT43, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\monmark02a.jpg", BITMAP_LUCKY_SEAL_EFFECT44, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\monmark03a.jpg", BITMAP_LUCKY_SEAL_EFFECT45, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\expensiveitem01_R.jpg", BITMAP_RAREITEM1_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\expensiveitem02a_R.jpg", BITMAP_RAREITEM2_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\expensiveitem02b_R.jpg", BITMAP_RAREITEM3_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\expensiveitem03a_R.jpg", BITMAP_RAREITEM4_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\expensiveitem03b_R.jpg", BITMAP_RAREITEM5_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge3\\alicecard_R.tga", BITMAP_CHARACTERCARD_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge3\\jujug_R.jpg", BITMAP_NEWCHAOSCARD_GOLD_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge3\\jujul_R.jpg", BITMAP_NEWCHAOSCARD_RARE_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge3\\jujum_R.jpg", BITMAP_NEWCHAOSCARD_MINI_R, GL_CLAMP_TO_EDGE},
    {L"Item\\deathbeamstone_R.jpg", BITMAP_ITEM_EFFECT_DBSTONE_R, GL_CLAMP_TO_EDGE},
    {L"Item\\hellhorn_R.jpg", BITMAP_ITEM_EFFECT_HELLHORN_R, GL_CLAMP_TO_EDGE},
    {L"Item\\phoenixfeather_R.tga", BITMAP_ITEM_EFFECT_PFEATHER_R, GL_CLAMP_TO_EDGE},
    {L"Item\\Deye_R.jpg", BITMAP_ITEM_EFFECT_DEYE_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\rareitem_ticket_7_body.jpg", BITMAP_RAREITEM7, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\rareitem_ticket_8_body.jpg", BITMAP_RAREITEM8, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\rareitem_ticket_9_body.jpg", BITMAP_RAREITEM9, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\rareitem_ticket_10_body.jpg", BITMAP_RAREITEM10, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\rareitem_ticket_11_body.jpg", BITMAP_RAREITEM11, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\rareitem_ticket_12_body.jpg", BITMAP_RAREITEM12, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\DoppelCard.jpg", BITMAP_DOPPLEGANGGER_FREETICKET, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\BarcaCard.jpg", BITMAP_BARCA_FREETICKET, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge8\\Barca7Card.jpg", BITMAP_BARCA7TH_FREETICKET, GL_CLAMP_TO_EDGE},
};

void ShippedItems::Open(std::span<const int> itemTypes)
{
    std::vector<int> items;
    for (const int drawn : itemTypes)
    {
        if (drawn >= 0)
        {
            items.push_back(drawn);
        }
        else
        {
            Data::Items::ModelLoader::OpenPartModel(
                PartModelDrawn(drawn), {Render::Items::Styles::Exists, Render::Items::ItemEffects::Exists});
        }
    }
    Data::Items::ModelLoader::ReopenModels(items, {});
    // As the game loads them before the looks find theirs: the textures of OpenBasicData and the item effect textures
    // at their indices, which looks in code name; then the textures the looks in data name.
    static const bool effectTexturesLoaded = []
    {
        for (const BasicTexture& texture : BasicTextures)
        {
            LoadBitmap(texture.file, texture.index, GL_LINEAR, texture.wrap);
        }
        OpenItemEffectTextures();
        return true;
    }();
    (void)effectTexturesLoaded;
    Render::Items::Looks::PrepareTextures();
}

std::vector<int> ItemsUsing(std::string_view renderStyle, Data::Looks::LookKind kind)
{
    std::vector<int> items;
    for (const Data::Items::ItemModelDefinition& model : ShippedModels().models)
    {
        if ((kind == Data::Looks::LookKind::ItemEffect ? model.itemEffect : model.renderStyle) == renderStyle)
        {
            items.push_back(Data::Items::MakeItemType(model.group, model.number));
        }
    }
    std::ranges::sort(items);
    if (kind == Data::Looks::LookKind::RenderStyle)
    {
        for (const Data::Items::PartModelSlot& slot : Data::Items::PartModelSlots)
        {
            const auto part = std::ranges::find_if(ShippedModels().partModels, [&](const Data::Items::PartModel& entry)
                                                   { return entry.model.name == slot.name; });
            if (part != ShippedModels().partModels.end() && part->renderStyle == renderStyle)
            {
                items.push_back(PartModelDrawn(slot.modelType));
            }
        }
    }
    return items;
}

std::string DescribeItem(int itemType)
{
    if (itemType < 0)
    {
        return "(" + std::string(Data::Items::PartModelNameOf(PartModelDrawn(itemType)).value_or("?")) + ")";
    }
    return "(" + std::to_string(itemType / MAX_ITEM_INDEX) + "," + std::to_string(itemType % MAX_ITEM_INDEX) + ")";
}

Record RecordItem(int itemType, const Situation& situation)
{
    const int slot = itemType >= 0 ? Data::Items::ToModelSlot(itemType) : PartModelDrawn(itemType);
    BMD& model = Models[slot];
    OBJECT& object = DrawnObject();
    // Random::DrawRand keys on the object's address, which changes from run to run: the object is keyed as a fixed
    // address, so the baseline holds the same random values in every run (and no draw frame starts in these tests).
    const Random::DrawnAs fixedKey(&object, FixedDrawKey());
    const double previousWorldTime = WorldTime;
    WorldTime = situation.worldTime;
    // Set up at the pinned clock and key, as a held, worn or inventory item is for each drawing: its glow values
    // (ItemObjectAttribute) read both.
    object.Initialize();
    object.Type = slot;
    ItemObjectAttribute(&object);
    object.Alpha = situation.alpha;
    object.Angle[1] = situation.turn;
    object.m_BuffMap.ClearBuff();
    if (situation.infinityArrow)
    {
        object.m_BuffMap.RegisterBuff(eBuff_InfinityArrow);
    }

    // Each drawing starts from the scrolling mesh the model loads with: a look that leaves one would otherwise draw the
    // next drawing differently.
    model.StreamMesh = -1;
    model.Animation(BoneTransform, 0.f, 0.f, 0, object.Angle, object.HeadAngle, false, false);

    RecordingDrawer drawer;
    Render::Drawing::DrawCallLog log;
    Render::ModelDrawing::SetModelDrawer(&drawer);
    mu::SetRendererOverride(&log);
    Render::Drawing::ResetLegacyState();
    log.calls.clear();
    const vec3_t light = {situation.light, situation.light, situation.light};
    std::optional<Render::Items::Looks::InventoryDrawing> inventory;
    if (situation.inInventory)
    {
        inventory.emplace();
    }
    RenderPartObject(&object, slot, nullptr, light, situation.alpha, situation.level, situation.excellent ? 1 : 0, 0,
                     true, situation.hideSkin, false, 0, situation.renderType);
    log.NoteFinalState();
    mu::SetRendererOverride(nullptr);
    Render::ModelDrawing::SetModelDrawer(nullptr);
    WorldTime = previousWorldTime;

    Record record;
    record.meshDraws = std::move(drawer.draws);
    record.gpuCalls = std::move(log.calls);
    record.modelAfter = {
        {"bodyLight", Number(model.BodyLight[0]) + " " + Number(model.BodyLight[1]) + " " + Number(model.BodyLight[2])},
        {"streamMesh", std::to_string(model.StreamMesh)},
        {"lightEnable", model.LightEnable ? "1" : "0"},
    };
    return record;
}

Record RecordItemWithCode(int itemType, const Situation& situation, std::string_view renderStyle,
                          Data::Looks::LookKind kind)
{
    using Data::Looks::g_LookStore;
    const std::vector<Data::Looks::LookDefinition> looks(g_LookStore.GetLooks(kind).begin(),
                                                         g_LookStore.GetLooks(kind).end());
    std::vector<Data::Looks::LookDefinition> withoutStyle;
    std::ranges::copy_if(looks, std::back_inserter(withoutStyle),
                         [renderStyle](const Data::Looks::LookDefinition& look) { return look.name != renderStyle; });
    g_LookStore.SetLooks(kind, std::move(withoutStyle));
    Record record = RecordItem(itemType, situation);
    g_LookStore.SetLooks(kind, looks);
    return record;
}

namespace
{
std::string Numbers(const float* values, int count)
{
    std::string text;
    for (int i = 0; i < count; ++i)
    {
        text += (i == 0 ? "" : " ") + Number(values[i]);
    }
    return text;
}

// Every live slot of the lists a look creates things in, one entry each, its fields in a fixed order.
std::vector<std::vector<Field>> LiveThings(const OBJECT& object)
{
    std::vector<std::vector<Field>> things;
    const auto ownedBy = [&object](const OBJECT* owner)
    {
        return owner == &object ? "item" : owner ? "other" : "none";
    };
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        const OBJECT& o = Sprites[i];
        if (o.Live)
        {
            things.push_back({{"kind", "sprite"},
                              {"type", std::to_string(o.Type)},
                              {"subType", std::to_string(o.SubType)},
                              {"position", Numbers(o.Position, 3)},
                              {"scale", Number(o.Scale)},
                              {"light", Numbers(o.Light, 3)},
                              {"angle", Numbers(o.Angle, 3)},
                              {"owner", ownedBy(o.Owner)}});
        }
    }
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        const PARTICLE& o = Particles[i];
        if (o.Live)
        {
            things.push_back({{"kind", "particle"},
                              {"type", std::to_string(o.Type)},
                              {"subType", std::to_string(o.SubType)},
                              {"position", Numbers(o.Position, 3)},
                              {"scale", Number(o.Scale)},
                              {"light", Numbers(o.Light, 3)},
                              {"angle", Numbers(o.Angle, 3)},
                              {"lifeTime", Number(o.LifeTime)},
                              {"velocity", Numbers(o.Velocity, 3)},
                              {"target", ownedBy(o.Target)}});
        }
    }
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        const JOINT& o = Joints[i];
        if (o.Live)
        {
            things.push_back({{"kind", "joint"},
                              {"type", std::to_string(o.Type)},
                              {"subType", std::to_string(o.SubType)},
                              {"position", Numbers(o.Position, 3)},
                              {"targetPosition", Numbers(o.TargetPosition, 3)},
                              {"scale", Number(o.Scale)},
                              {"light", Numbers(o.Light, 3)},
                              {"angle", Numbers(o.Angle, 3)},
                              {"lifeTime", Number(o.LifeTime)},
                              {"target", ownedBy(o.Target)}});
        }
    }
    for (int i = 0; i < MAX_EFFECTS; ++i)
    {
        const OBJECT& o = Effects[i];
        if (o.Live)
        {
            things.push_back({{"kind", "effect"},
                              {"type", std::to_string(o.Type)},
                              {"subType", std::to_string(o.SubType)},
                              {"position", Numbers(o.Position, 3)},
                              {"scale", Number(o.Scale)},
                              {"light", Numbers(o.Light, 3)},
                              {"angle", Numbers(o.Angle, 3)},
                              {"lifeTime", Number(o.LifeTime)},
                              {"owner", ownedBy(o.Owner)}});
        }
    }
    // By kind and type, and things of one kind and type in the order they were created (their slots): the order of
    // their numbers can swap on another platform when two of them differ by a float step (sparks on a vertical chain of
    // bones).
    std::ranges::stable_sort(things, {},
                             [](const std::vector<Field>& thing)
                             {
                                 return std::tuple(std::string_view(thing[0].second), std::stoi(thing[1].second),
                                                   std::stoi(thing[2].second));
                             });
    return things;
}

void ClearThingLists()
{
    // Whole objects, not only Live: a create call leaves fields it doesn't set as the slot's last thing had them, so a
    // record would depend on what ran before it.
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        Sprites[i] = OBJECT{};
    }
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        Particles[i] = PARTICLE{};
    }
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        Joints[i] = JOINT{};
    }
    for (int i = 0; i < MAX_EFFECTS; ++i)
    {
        Effects[i] = OBJECT{};
    }
}

// A digest of the engine sequence's state: equal when both drew as many numbers. It is taken from the next numbers a
// copy of the engine gives, which the standard fixes for std::mt19937, hashed with 64-bit FNV-1a: the same on every
// platform, unlike the engine's written state and std::hash.
std::string EngineState()
{
    std::mt19937 engine = Random::Snapshot();
    std::string text;
    for (int i = 0; i < 8; ++i)
    {
        text += std::to_string(engine()) + " ";
    }
    std::uint64_t hash = 14695981039346656037ull;
    for (const char c : text)
    {
        hash = (hash ^ static_cast<unsigned char>(c)) * 1099511628211ull;
    }
    return "x" + std::to_string(hash);
}
} // namespace

Record RecordSpawn(int itemType, double worldTime, std::uint32_t seed)
{
    const int slot = Data::Items::ToModelSlot(itemType);
    BMD& model = Models[slot];
    OBJECT& object = DrawnObject();
    const Random::DrawnAs fixedKey(&object, FixedDrawKey());
    const double previousWorldTime = WorldTime;
    const float previousFactor = FPS_ANIMATION_FACTOR;
    WorldTime = worldTime;
    FPS_ANIMATION_FACTOR = 1.f;
    object.Initialize();
    object.Type = slot;
    object.Live = true;
    ItemObjectAttribute(&object);
    Vector(13120.25f, 12480.5f, 140.75f, object.Position);
    ClearThingLists();
    Random::Seed(seed);
    Random::SeedRand(seed);

    // As SpawnGroundItemEffects poses a ground item.
    model.CurrentAction = object.CurrentAction;
    model.Animation(BoneTransform, object.AnimationFrame, object.PriorAnimationFrame, object.PriorAction, object.Angle,
                    object.HeadAngle, false, false);
    model.BodyScale = object.Scale;
    VectorCopy(object.Position, model.BodyOrigin);
    SpawnPartObjectEffect(&model, &object, slot, BoneTransform);

    Record record;
    record.meshDraws = LiveThings(object);
    record.modelAfter = {{"objectLight", Numbers(object.Light, 3)},
                         {"animationFrame", Number(object.AnimationFrame)},
                         {"randState", "x" + std::to_string(Random::RandState())},
                         {"engineState", EngineState()}};
    ClearThingLists();
    WorldTime = previousWorldTime;
    FPS_ANIMATION_FACTOR = previousFactor;
    return record;
}

std::string FirstDifference(const Record& code, const Record& data)
{
    std::string gpuDifference;
    const size_t calls = std::min(code.gpuCalls.size(), data.gpuCalls.size());
    for (size_t i = 0; i < calls && gpuDifference.empty(); ++i)
    {
        if (code.gpuCalls[i] != data.gpuCalls[i])
        {
            gpuDifference = "GPU call " + std::to_string(i + 1) + " of " + std::to_string(code.gpuCalls.size()) +
                            ":\n  code: " + code.gpuCalls[i] + "\n  data: " + data.gpuCalls[i];
        }
    }
    if (gpuDifference.empty() && code.gpuCalls.size() != data.gpuCalls.size())
    {
        gpuDifference = std::to_string(code.gpuCalls.size()) + " GPU calls (code) vs " +
                        std::to_string(data.gpuCalls.size()) + " (data)";
    }
    // Numbers a few float steps apart only while nothing different reaches the renderer.
    const int steps = gpuDifference.empty() ? AllowedFloatSteps : 0;

    const size_t draws = std::min(code.meshDraws.size(), data.meshDraws.size());
    for (size_t i = 0; i < draws; ++i)
    {
        const std::vector<Field>& a = code.meshDraws[i];
        const std::vector<Field>& b = data.meshDraws[i];
        for (size_t f = 0; f < a.size(); ++f)
        {
            if (!Close(a[f].second, b[f].second, steps))
            {
                return "draw " + std::to_string(i + 1) + " of " + std::to_string(code.meshDraws.size()) + ", mesh " +
                       a[0].second + ": " + a[f].first + " " + a[f].second + " (code) vs " + b[f].second + " (data)";
            }
        }
    }
    if (code.meshDraws.size() != data.meshDraws.size())
    {
        const auto& longer = code.meshDraws.size() > data.meshDraws.size() ? code : data;
        return std::to_string(code.meshDraws.size()) + " mesh draws (code) vs " +
               std::to_string(data.meshDraws.size()) + " (data); the first extra one: " + Join(longer.meshDraws[draws]);
    }
    for (size_t f = 0; f < code.modelAfter.size(); ++f)
    {
        if (!Close(code.modelAfter[f].second, data.modelAfter[f].second, steps))
        {
            return std::string("the model after the drawing: ") + code.modelAfter[f].first + " " +
                   code.modelAfter[f].second + " (code) vs " + data.modelAfter[f].second + " (data)";
        }
    }
    return gpuDifference;
}

namespace
{
// The fields of a mesh draw or thing, their numbers on the grid. On the relative grid a thing's type and subType stay
// exact: a type is a number near 32000 (BITMAP_SPARK), where a relative step is about 3, and one off is another type.
std::string RoundedFields(const std::vector<Field>& fields, std::size_t gridIndex, Grid grid)
{
    if (grid == Grid::Fixed)
    {
        return Rounded(Join(fields), gridIndex, grid);
    }
    std::string text;
    for (const auto& [name, value] : fields)
    {
        const std::string_view key = name;
        text += text.empty() ? "" : " ";
        text += std::string(key) + "=" + (key == "type" || key == "subType" ? value : Rounded(value, gridIndex, grid));
    }
    return text;
}

std::string HashOf(const Record& record, std::size_t gridIndex, Grid grid)
{
    std::string text;
    for (const std::vector<Field>& draw : record.meshDraws)
    {
        text += RoundedFields(draw, gridIndex, grid) + "\n";
    }
    text += Rounded(Join(record.modelAfter), gridIndex, grid);
    const std::uint64_t hash = Render::Drawing::Hash(text.data(), text.size());
    char digest[24];
    std::snprintf(digest, sizeof(digest), "%016llx", static_cast<unsigned long long>(hash));
    return digest;
}

std::vector<std::string> Words(const std::string& text)
{
    std::vector<std::string> words;
    std::istringstream stream(text);
    std::string word;
    while (stream >> word)
    {
        words.push_back(word);
    }
    return words;
}
} // namespace

std::string Digest(const Record& record, Grid grid)
{
    std::string digest = std::to_string(record.meshDraws.size());
    for (std::size_t gridIndex = 0; gridIndex < GridOffsets.size(); ++gridIndex)
    {
        digest += " " + HashOf(record, gridIndex, grid);
    }
    return digest;
}

std::string Describe(const Record& record)
{
    std::string text;
    for (const std::vector<Field>& draw : record.meshDraws)
    {
        text += Join(draw) + "\n";
    }
    return text + "after " + Join(record.modelAfter);
}

bool SameDigest(const std::string& digest, const std::string& baseline)
{
    const std::vector<std::string> drawn = Words(digest);
    const std::vector<std::string> kept = Words(baseline);
    if (drawn.size() < 2 || kept.size() < 2 || drawn[0] != kept[0])
    {
        return false;
    }
    for (std::size_t i = 1; i < std::min(drawn.size(), kept.size()); ++i)
    {
        if (drawn[i] == kept[i])
        {
            return true;
        }
    }
    return false;
}
} // namespace LookRecorder
