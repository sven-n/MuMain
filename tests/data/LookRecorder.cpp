#include "stdafx.h"

#include "LookRecorder.h"

#include "TestModelSlots.h"

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
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <iterator>
#include <filesystem>
#include <memory>

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
    if ((flags & ReflectionFlags) != 0)
    {
        flags &= ~RENDER_TEXTURE;
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
        {"streamMesh", std::to_string(draw.streamMesh)},
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

// The one object every drawing draws: Random::DrawRand keys on its address.
OBJECT& DrawnObject()
{
    static OBJECT object;
    return object;
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

// Every number of the text with 6 significant digits.
std::string Rounded(const std::string& text)
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
        result += Number(value, 6);
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

void ShippedItems::Open(std::span<const int> itemTypes)
{
    Data::Items::ModelLoader::ReopenModels(itemTypes, {});
    // As the game loads them after the item models: the item effect textures at their indices, which looks in code
    // name, then the textures the looks in data name.
    static const bool effectTexturesLoaded = []
    {
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
    const size_t draws = std::min(code.meshDraws.size(), data.meshDraws.size());
    for (size_t i = 0; i < draws; ++i)
    {
        const std::vector<Field>& a = code.meshDraws[i];
        const std::vector<Field>& b = data.meshDraws[i];
        for (size_t f = 0; f < a.size(); ++f)
        {
            if (a[f].second != b[f].second)
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
        if (code.modelAfter[f].second != data.modelAfter[f].second)
        {
            return std::string("the model after the drawing: ") + code.modelAfter[f].first + " " +
                   code.modelAfter[f].second + " (code) vs " + data.modelAfter[f].second + " (data)";
        }
    }
    const size_t calls = std::min(code.gpuCalls.size(), data.gpuCalls.size());
    for (size_t i = 0; i < calls; ++i)
    {
        if (code.gpuCalls[i] != data.gpuCalls[i])
        {
            return "GPU call " + std::to_string(i + 1) + " of " + std::to_string(code.gpuCalls.size()) +
                   ":\n  code: " + code.gpuCalls[i] + "\n  data: " + data.gpuCalls[i];
        }
    }
    if (code.gpuCalls.size() != data.gpuCalls.size())
    {
        return std::to_string(code.gpuCalls.size()) + " GPU calls (code) vs " + std::to_string(data.gpuCalls.size()) +
               " (data)";
    }
    return {};
}

std::string Digest(const Record& record)
{
    std::string text;
    for (const std::vector<Field>& draw : record.meshDraws)
    {
        text += Rounded(Join(draw)) + "\n";
    }
    text += Rounded(Join(record.modelAfter));
    const std::uint64_t hash = Render::Drawing::Hash(text.data(), text.size());
    char digest[24];
    std::snprintf(digest, sizeof(digest), "%016llx", static_cast<unsigned long long>(hash));
    return std::to_string(record.meshDraws.size()) + " " + digest;
}
} // namespace LookRecorder
