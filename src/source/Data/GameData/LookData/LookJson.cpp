#include "stdafx.h"

#include "LookJson.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterJsonCommon.h"

#include <algorithm>
#include <array>
#include <limits>
#include <set>

namespace Data::Looks
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;
using Monsters::Json::WriteFloat;

namespace Keys
{
constexpr const char* Kind = "kind";
constexpr const char* Looks = "looks";
constexpr const char* LookFolders = "lookFolders";
constexpr const char* Name = "name";
constexpr const char* Layers = "layers";
constexpr const char* ItemValuesAfter = "itemValuesAfter";
constexpr const char* Motion = "motion";
constexpr const char* Blend = "blend";
constexpr const char* Meshes = "meshes";
constexpr const char* AllBut = "allBut";
constexpr const char* AllEvenHidden = "allEvenHidden";
constexpr const char* Texture = "texture";
constexpr const char* Opacity = "opacity";
constexpr const char* GlowMesh = "glowMesh";
constexpr const char* GlowStrength = "glowStrength";
constexpr const char* ScrollU = "scrollU";
constexpr const char* ScrollV = "scrollV";
constexpr const char* HiddenMesh = "hiddenMesh";
} // namespace Keys

constexpr const char* RenderStyleKind = "renderStyle";
constexpr const char* Drawing = "drawing";
constexpr const char* None = "none";

// A model has at most this many meshes (MAX_MESH).
constexpr long long MeshCount = 50;

constexpr std::array<std::pair<LayerKind, const char*>, 2> LayerKindNames{{
    {LayerKind::Texture, "texture"},
    {LayerKind::Reflection, "reflection"},
}};

constexpr std::array<std::pair<Motion, const char*>, 9> MotionNames{{
    {Motion::SlowDrift, "slowDrift"},
    {Motion::FastSweep, "fastSweep"},
    {Motion::FollowsLight, "followsLight"},
    {Motion::Swirl, "swirl"},
    {Motion::StrongSwirl, "strongSwirl"},
    {Motion::Stripes, "stripes"},
    {Motion::VerySlowDrift, "verySlowDrift"},
    {Motion::OilSheen, "oilSheen"},
    {Motion::StillShine, "stillShine"},
}};

constexpr std::array<std::pair<Blend, const char*>, 3> BlendNames{{
    {Blend::Normal, "normal"},
    {Blend::Glow, "glow"},
    {Blend::AsDrawing, "asDrawing"},
}};

template <typename T, std::size_t N> const char* NameOf(const std::array<std::pair<T, const char*>, N>& names, T value)
{
    for (const auto& [entry, name] : names)
    {
        if (entry == value)
        {
            return name;
        }
    }
    return "";
}

template <typename T, std::size_t N>
std::optional<T> ValueOf(const std::array<std::pair<T, const char*>, N>& names, std::string_view text)
{
    for (const auto& [entry, name] : names)
    {
        if (text == name)
        {
            return entry;
        }
    }
    return std::nullopt;
}

template <typename T, std::size_t N> std::string ListOf(const std::array<std::pair<T, const char*>, N>& names)
{
    std::string list;
    for (const auto& [entry, name] : names)
    {
        list += list.empty() ? "" : ", ";
        list += '"';
        list += name;
        list += '"';
    }
    return list;
}

// Reads one file's looks; each problem names the field it is in ("looks[3].layers[1].meshes").
class LookReader
{
public:
    LookReader(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    void ReadLooks(const OrderedJson& root, std::vector<LookDefinition>& looks);

private:
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    {
        m_issues.push_back({severity, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    }
    void AddFatal(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field, message);
    }
    void WarnAboutUnknownKeys(const OrderedJson& json, const Items::Json::KeySet& known, const std::string& field)
    {
        Items::Json::WarnAboutUnknownKeys(json, known, field + ".",
                                          [this](const std::string& unknown, const std::string& message)
                                          { AddIssue(ItemDataIssueSeverity::Warning, unknown, message); });
    }

    bool ReadLook(const OrderedJson& json, const std::string& field, LookDefinition& look);
    void ReadLayer(const OrderedJson& json, const std::string& field, Layer& layer);
    void ReadMeshes(const OrderedJson& json, const std::string& field, MeshSet& meshes);
    bool ReadMeshList(const OrderedJson& json, const std::string& field, std::vector<int>& meshes);
    bool ReadMesh(const OrderedJson& json, const std::string& field, int& mesh);
    void ReadValue(const OrderedJson& json, const char* key, const std::string& field, bool drawing, Value& value);
    void ReadGlowMesh(const OrderedJson& json, const std::string& field, GlowMesh& mesh);
    void ReadItemValues(const OrderedJson& json, const std::string& field, ItemValues& values);
    void ReadNumber(const OrderedJson& json, const char* key, const std::string& field, std::optional<float>& value);
    void ReadMeshOrNone(const OrderedJson& json, const char* key, const std::string& field, std::optional<int>& mesh);

    template <typename T, std::size_t N>
    bool ReadName(const OrderedJson& json, const char* key, const std::string& field,
                  const std::array<std::pair<T, const char*>, N>& names, T& value)
    {
        const auto entry = json.find(key);
        if (entry == json.end())
        {
            return false;
        }
        const std::optional<T> named =
            entry->is_string() ? ValueOf(names, entry->get_ref<const std::string&>()) : std::nullopt;
        if (!named)
        {
            AddFatal(field + "." + key, "must be one of " + ListOf(names));
            return false;
        }
        value = *named;
        return true;
    }

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

void LookReader::ReadLooks(const OrderedJson& root, std::vector<LookDefinition>& looks)
{
    const auto list = root.find(Keys::Looks);
    if (list == root.end() || !list->is_array())
    {
        AddFatal(Keys::Looks, "must be a list of looks");
        return;
    }
    std::set<std::string, std::less<>> names;
    for (std::size_t i = 0; i < list->size(); ++i)
    {
        const std::string field = std::string(Keys::Looks) + "[" + std::to_string(i) + "]";
        LookDefinition look;
        if (!ReadLook((*list)[i], field, look))
        {
            continue;
        }
        if (!names.insert(look.name).second)
        {
            AddFatal(field + "." + Keys::Name, "\"" + look.name + "\" is in the file twice");
            continue;
        }
        looks.push_back(std::move(look));
    }
}

bool LookReader::ReadLook(const OrderedJson& json, const std::string& field, LookDefinition& look)
{
    if (!json.is_object())
    {
        AddFatal(field, "must be a look: an object with a name");
        return false;
    }
    const auto name = json.find(Keys::Name);
    if (name == json.end() || !name->is_string() || !Items::Json::IsName(name->get_ref<const std::string&>()))
    {
        AddFatal(field + "." + Keys::Name, "must be a name of letters and digits");
        return false;
    }
    look.name = name->get<std::string>();

    const auto layers = json.find(Keys::Layers);
    if (layers == json.end() || !layers->is_array())
    {
        AddFatal(field + "." + Keys::Layers, "must be a list of layers");
        return false;
    }
    if (layers->empty())
    {
        AddIssue(ItemDataIssueSeverity::Warning, field + "." + Keys::Layers, "has no layers: the item is not drawn");
    }
    for (std::size_t i = 0; i < layers->size(); ++i)
    {
        Layer layer;
        ReadLayer((*layers)[i], field + "." + Keys::Layers + "[" + std::to_string(i) + "]", layer);
        look.layers.push_back(std::move(layer));
    }
    if (const auto after = json.find(Keys::ItemValuesAfter); after != json.end())
    {
        ReadItemValues(*after, field + "." + Keys::ItemValuesAfter, look.itemValuesAfter);
    }
    WarnAboutUnknownKeys(json, {Keys::Name, Keys::Layers, Keys::ItemValuesAfter}, field);
    return true;
}

void LookReader::ReadLayer(const OrderedJson& json, const std::string& field, Layer& layer)
{
    if (!json.is_object())
    {
        AddFatal(field, "must be a layer: an object with a kind");
        return;
    }
    if (json.find(Keys::Kind) == json.end())
    {
        AddFatal(field + "." + Keys::Kind, "is missing: " + ListOf(LayerKindNames));
    }
    ReadName(json, Keys::Kind, field, LayerKindNames, layer.kind);
    if (ReadName(json, Keys::Motion, field, MotionNames, layer.motion) && layer.kind != LayerKind::Reflection)
    {
        AddFatal(field + "." + Keys::Motion, "is only for a reflection");
    }
    ReadName(json, Keys::Blend, field, BlendNames, layer.blend);
    ReadMeshes(json, field, layer.meshes);
    if (const auto texture = json.find(Keys::Texture); texture != json.end())
    {
        if (!texture->is_string() || texture->get_ref<const std::string&>().empty())
        {
            AddFatal(field + "." + Keys::Texture, "must be a texture file under Data, e.g. \"Effect/Chrome01.jpg\"");
        }
        else
        {
            layer.texture = texture->get<std::string>();
        }
    }
    ReadValue(json, Keys::Opacity, field, true, layer.opacity);
    ReadGlowMesh(json, field, layer.glowMesh);
    ReadValue(json, Keys::GlowStrength, field, false, layer.glowStrength);
    ReadValue(json, Keys::ScrollU, field, false, layer.scrollU);
    ReadValue(json, Keys::ScrollV, field, false, layer.scrollV);
    WarnAboutUnknownKeys(json,
                         {Keys::Kind, Keys::Motion, Keys::Blend, Keys::Meshes, Keys::Texture, Keys::Opacity,
                          Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU, Keys::ScrollV},
                         field);
}

void LookReader::ReadMeshes(const OrderedJson& json, const std::string& field, MeshSet& meshes)
{
    const auto entry = json.find(Keys::Meshes);
    if (entry == json.end())
    {
        return;
    }
    const std::string meshesField = field + "." + Keys::Meshes;
    if (entry->is_string() && entry->get_ref<const std::string&>() == Keys::AllEvenHidden)
    {
        meshes.evenHidden = true;
        return;
    }
    if (entry->is_object())
    {
        const auto allBut = entry->find(Keys::AllBut);
        if (allBut == entry->end() || entry->size() != 1)
        {
            AddFatal(meshesField, "must be a list of meshes, {\"allBut\": [meshes]} or \"allEvenHidden\"");
            return;
        }
        meshes.allBut = ReadMeshList(*allBut, meshesField + "." + Keys::AllBut, meshes.meshes);
        return;
    }
    ReadMeshList(*entry, meshesField, meshes.meshes);
}

bool LookReader::ReadMeshList(const OrderedJson& json, const std::string& field, std::vector<int>& meshes)
{
    if (!json.is_array() || json.empty())
    {
        AddFatal(field, "must be a list of one or more mesh numbers");
        return false;
    }
    for (const OrderedJson& entry : json)
    {
        int mesh = 0;
        if (ReadMesh(entry, field, mesh))
        {
            meshes.push_back(mesh);
        }
    }
    return !meshes.empty();
}

bool LookReader::ReadMesh(const OrderedJson& json, const std::string& field, int& mesh)
{
    long long number = 0;
    if (!Items::Json::ReadWholeNumber(json, number) || number < 0 || number >= MeshCount)
    {
        AddFatal(field, json.dump() + " is not a mesh number (0 to " + std::to_string(MeshCount - 1) + ")");
        return false;
    }
    mesh = static_cast<int>(number);
    return true;
}

void LookReader::ReadValue(const OrderedJson& json, const char* key, const std::string& field, bool drawing,
                           Value& value)
{
    const auto entry = json.find(key);
    if (entry == json.end())
    {
        return;
    }
    if (entry->is_number())
    {
        value = entry->get<float>();
        return;
    }
    if (drawing && entry->is_string() && entry->get_ref<const std::string&>() == Drawing)
    {
        value = FromDrawing{};
        return;
    }
    AddFatal(field + "." + key, drawing ? "must be a number or \"drawing\"" : "must be a number");
}

void LookReader::ReadGlowMesh(const OrderedJson& json, const std::string& field, GlowMesh& mesh)
{
    const auto entry = json.find(Keys::GlowMesh);
    if (entry == json.end())
    {
        return;
    }
    if (entry->is_string() && entry->get_ref<const std::string&>() == None)
    {
        mesh = NoGlowMesh{};
        return;
    }
    int number = 0;
    if (ReadMesh(*entry, field + "." + Keys::GlowMesh, number))
    {
        mesh = number;
    }
}

void LookReader::ReadItemValues(const OrderedJson& json, const std::string& field, ItemValues& values)
{
    if (!json.is_object())
    {
        AddFatal(field, "must be an object of item values");
        return;
    }
    ReadNumber(json, Keys::Opacity, field, values.opacity);
    ReadMeshOrNone(json, Keys::GlowMesh, field, values.glowMesh);
    ReadNumber(json, Keys::GlowStrength, field, values.glowStrength);
    ReadNumber(json, Keys::ScrollU, field, values.scrollU);
    ReadNumber(json, Keys::ScrollV, field, values.scrollV);
    ReadMeshOrNone(json, Keys::HiddenMesh, field, values.hiddenMesh);
    if (values.IsEmpty())
    {
        AddIssue(ItemDataIssueSeverity::Warning, field, "sets no item value");
    }
    WarnAboutUnknownKeys(
        json, {Keys::Opacity, Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU, Keys::ScrollV, Keys::HiddenMesh},
        field);
}

void LookReader::ReadNumber(const OrderedJson& json, const char* key, const std::string& field,
                            std::optional<float>& value)
{
    const auto entry = json.find(key);
    if (entry == json.end())
    {
        return;
    }
    if (!entry->is_number())
    {
        AddFatal(field + "." + key, "must be a number");
        return;
    }
    value = entry->get<float>();
}

// A mesh number, or "none" (-1, what the item starts with).
void LookReader::ReadMeshOrNone(const OrderedJson& json, const char* key, const std::string& field,
                                std::optional<int>& mesh)
{
    const auto entry = json.find(key);
    if (entry == json.end())
    {
        return;
    }
    if (entry->is_string() && entry->get_ref<const std::string&>() == None)
    {
        mesh = -1;
        return;
    }
    int number = 0;
    if (ReadMesh(*entry, field + "." + key, number))
    {
        mesh = number;
    }
}

OrderedJson WriteValue(const Value& value)
{
    if (std::holds_alternative<FromDrawing>(value))
    {
        return Drawing;
    }
    return WriteFloat(std::get<float>(value));
}

void PutValue(OrderedJson& json, const char* key, const Value& value)
{
    if (!std::holds_alternative<FromItem>(value))
    {
        json[key] = WriteValue(value);
    }
}

OrderedJson WriteMeshOrNone(int mesh)
{
    return mesh < 0 ? OrderedJson(None) : OrderedJson(mesh);
}

OrderedJson WriteLayer(const Layer& layer)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Kind] = NameOf(LayerKindNames, layer.kind);
    if (layer.kind == LayerKind::Reflection)
    {
        json[Keys::Motion] = NameOf(MotionNames, layer.motion);
    }
    if (layer.blend != Blend::Normal)
    {
        json[Keys::Blend] = NameOf(BlendNames, layer.blend);
    }
    if (layer.meshes.evenHidden)
    {
        json[Keys::Meshes] = Keys::AllEvenHidden;
    }
    else if (layer.meshes.allBut)
    {
        json[Keys::Meshes] = OrderedJson{{Keys::AllBut, layer.meshes.meshes}};
    }
    else if (!layer.meshes.meshes.empty())
    {
        json[Keys::Meshes] = layer.meshes.meshes;
    }
    if (!layer.texture.empty())
    {
        json[Keys::Texture] = layer.texture;
    }
    PutValue(json, Keys::Opacity, layer.opacity);
    if (std::holds_alternative<NoGlowMesh>(layer.glowMesh))
    {
        json[Keys::GlowMesh] = None;
    }
    else if (const int* mesh = std::get_if<int>(&layer.glowMesh))
    {
        json[Keys::GlowMesh] = *mesh;
    }
    PutValue(json, Keys::GlowStrength, layer.glowStrength);
    PutValue(json, Keys::ScrollU, layer.scrollU);
    PutValue(json, Keys::ScrollV, layer.scrollV);
    return json;
}

OrderedJson WriteItemValues(const ItemValues& values)
{
    OrderedJson json = OrderedJson::object();
    if (values.opacity)
    {
        json[Keys::Opacity] = WriteFloat(*values.opacity);
    }
    if (values.glowMesh)
    {
        json[Keys::GlowMesh] = WriteMeshOrNone(*values.glowMesh);
    }
    if (values.glowStrength)
    {
        json[Keys::GlowStrength] = WriteFloat(*values.glowStrength);
    }
    if (values.scrollU)
    {
        json[Keys::ScrollU] = WriteFloat(*values.scrollU);
    }
    if (values.scrollV)
    {
        json[Keys::ScrollV] = WriteFloat(*values.scrollV);
    }
    if (values.hiddenMesh)
    {
        json[Keys::HiddenMesh] = WriteMeshOrNone(*values.hiddenMesh);
    }
    return json;
}
} // namespace

std::string_view GetRenderStylesFileName()
{
    return "RenderStyles.json";
}

void ReadRenderStylesJson(std::string_view text, const std::string& source, std::vector<LookDefinition>& looks,
                          std::vector<Items::ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, LookFileFormatVersion, root, issues))
    {
        return;
    }
    const auto kind = root.find(Keys::Kind);
    if (kind == root.end() || !kind->is_string() || kind->get_ref<const std::string&>() != RenderStyleKind)
    {
        issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          Keys::Kind, std::string("must be \"") + RenderStyleKind + "\""});
        return;
    }
    LookReader reader(source, issues);
    reader.ReadLooks(root, looks);
    Items::Json::WarnAboutUnknownKeys(
        root, {Items::Json::Keys::FormatVersion, Keys::Kind, Keys::LookFolders, Keys::Looks}, "",
        [&](const std::string& field, const std::string& message)
        {
            issues.push_back(
                {ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
        });
}

std::string WriteRenderStylesJson(std::vector<LookDefinition> looks)
{
    std::sort(looks.begin(), looks.end(),
              [](const LookDefinition& a, const LookDefinition& b) { return a.name < b.name; });
    OrderedJson list = OrderedJson::array();
    for (const LookDefinition& look : looks)
    {
        OrderedJson json = OrderedJson::object();
        json[Keys::Name] = look.name;
        OrderedJson layers = OrderedJson::array();
        for (const Layer& layer : look.layers)
        {
            layers.push_back(WriteLayer(layer));
        }
        json[Keys::Layers] = std::move(layers);
        if (!look.itemValuesAfter.IsEmpty())
        {
            json[Keys::ItemValuesAfter] = WriteItemValues(look.itemValuesAfter);
        }
        list.push_back(std::move(json));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = LookFileFormatVersion;
    root[Keys::Kind] = RenderStyleKind;
    root[Keys::Looks] = std::move(list);
    std::string text = root.dump(Items::Json::Indent);
    text = Items::Json::PutListEntriesOnOneLine(text, Keys::Layers);
    text = Items::Json::PutObjectsOnOneLine(text, Keys::ItemValuesAfter);
    return text + "\n";
}
} // namespace Data::Looks
