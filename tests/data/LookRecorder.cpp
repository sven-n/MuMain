#include "stdafx.h"

#include "LookRecorder.h"

#include "TestModelSlots.h"

#include "Core/Utilities/Random.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/DataHandler/ItemData/ItemModelLoader.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzOpenData.h"
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
#include <sstream>
#include <string>
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

// Every number of the text as a whole number of steps on the grid. Another platform's sine, or a multiply and add the
// compiler fuses (ARM64), moves a number by a last float step of the sine (about 1e-7 near 1, also for a small number
// the sine was scaled to), far below a fifth of a step: a number near one grid's edge is far from the others'.
std::string Rounded(const std::string& text, double offset)
{
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
        const double steps = std::floor(value / GridStep + offset);
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
}

ShippedItems::~ShippedItems()
{
    g_ItemModelDatabase.Build({}, Data::Effects::GlowColorList{});
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
    {L"Effect\\Lava.jpg", BITMAP_LAVA, GL_CLAMP_TO_EDGE},
    {L"Item\\soketmagic_stape02.jpg", BITMAP_SOCKETSTAFF, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\entrance_R.jpg", BITMAP_FREETICKET_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\juju_R.jpg", BITMAP_CHAOSCARD_R, GL_CLAMP_TO_EDGE},
    {L"Item\\partCharge1\\bujuck01alpa.jpg", BITMAP_LUCKY_CHARM_EFFECT53, GL_CLAMP_TO_EDGE},
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
    Data::Items::ModelLoader::ReopenModels(itemTypes, {});
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

std::vector<int> ItemsUsing(std::string_view renderStyle)
{
    std::vector<int> items;
    for (const Data::Items::ItemModelDefinition& model : ShippedModels().models)
    {
        if (model.renderStyle == renderStyle)
        {
            items.push_back(Data::Items::MakeItemType(model.group, model.number));
        }
    }
    std::ranges::sort(items);
    return items;
}

std::string DescribeItem(int itemType)
{
    return "(" + std::to_string(itemType / MAX_ITEM_INDEX) + "," + std::to_string(itemType % MAX_ITEM_INDEX) + ")";
}

Record RecordItem(int itemType, const Situation& situation)
{
    const int slot = Data::Items::ToModelSlot(itemType);
    BMD& model = Models[slot];
    OBJECT& object = DrawnObject();
    object.Initialize();
    object.Type = slot;
    ItemObjectAttribute(&object);
    object.Alpha = situation.alpha;

    // Each drawing starts from the scrolling mesh the model loads with: a look that leaves one would otherwise draw the
    // next drawing differently.
    model.StreamMesh = -1;
    // Random::DrawRand keys on the object's address, which changes from run to run: the object is keyed as a fixed
    // address, so the baseline holds the same random values in every run (and no draw frame starts in these tests).
    const Random::DrawnAs fixedKey(&object, FixedDrawKey());
    const double previousWorldTime = WorldTime;
    WorldTime = situation.worldTime;
    model.Animation(BoneTransform, 0.f, 0.f, 0, object.Angle, object.HeadAngle, false, false);

    RecordingDrawer drawer;
    Render::Drawing::DrawCallLog log;
    Render::ModelDrawing::SetModelDrawer(&drawer);
    mu::SetRendererOverride(&log);
    Render::Drawing::ResetLegacyState();
    log.calls.clear();
    const vec3_t light = {1.f, 1.f, 1.f};
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

Record RecordItemWithCode(int itemType, const Situation& situation, std::string_view renderStyle)
{
    using Data::Looks::g_LookStore;
    const std::vector<Data::Looks::LookDefinition> looks(g_LookStore.GetRenderStyles().begin(),
                                                         g_LookStore.GetRenderStyles().end());
    std::vector<Data::Looks::LookDefinition> withoutStyle;
    std::ranges::copy_if(looks, std::back_inserter(withoutStyle),
                         [renderStyle](const Data::Looks::LookDefinition& look) { return look.name != renderStyle; });
    g_LookStore.SetRenderStyles(std::move(withoutStyle));
    Record record = RecordItem(itemType, situation);
    g_LookStore.SetRenderStyles(looks);
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
std::string HashOf(const Record& record, double offset)
{
    std::string text;
    for (const std::vector<Field>& draw : record.meshDraws)
    {
        text += Rounded(Join(draw), offset) + "\n";
    }
    text += Rounded(Join(record.modelAfter), offset);
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

std::string Digest(const Record& record)
{
    std::string digest = std::to_string(record.meshDraws.size());
    for (const double offset : GridOffsets)
    {
        digest += " " + HashOf(record, offset);
    }
    return digest;
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
