#include "stdafx.h"

#include "LookJson.h"

#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/Common/MeshLimits.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterJsonCommon.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
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
constexpr const char* ItemValuesBefore = "itemValuesBefore";
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
constexpr const char* Light = "light";
constexpr const char* Tint = "tint";
constexpr const char* ScrollingMesh = "scrollingMesh";
constexpr const char* Shape = "shape";
constexpr const char* From = "from";
constexpr const char* To = "to";
constexpr const char* Seconds = "seconds";
constexpr const char* PerSecond = "perSecond";
constexpr const char* Step = "step";
constexpr const char* Count = "count";
constexpr const char* SecondsEach = "secondsEach";
} // namespace Keys

constexpr const char* RenderStyleKind = "renderStyle";
constexpr const char* Drawing = "drawing";
constexpr const char* None = "none";
constexpr const char* All = "all";

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

constexpr std::array<std::pair<ShapeKind, const char*>, 6> ShapeNames{{
    {ShapeKind::Wave, "wave"},
    {ShapeKind::Bounce, "bounce"},
    {ShapeKind::Scroll, "scroll"},
    {ShapeKind::Ramp, "ramp"},
    {ShapeKind::Frames, "frames"},
    {ShapeKind::Random, "random"},
}};

// The fields of each shape, in the order they are written.
std::vector<const char*> ShapeFields(ShapeKind kind)
{
    switch (kind)
    {
    case ShapeKind::Wave:
    case ShapeKind::Bounce:
    case ShapeKind::Ramp:
        return {Keys::From, Keys::To, Keys::Seconds};
    case ShapeKind::Scroll:
        return {Keys::PerSecond};
    case ShapeKind::Frames:
        return {Keys::Count, Keys::Step, Keys::SecondsEach};
    case ShapeKind::Random:
        return {Keys::From, Keys::To, Keys::Step};
    }
    return {};
}

// The number a shape keeps for a field of ShapeFields other than its count.
double& ShapeField(Shape& shape, std::string_view key)
{
    if (key == Keys::From)
    {
        return shape.from;
    }
    if (key == Keys::To)
    {
        return shape.to;
    }
    if (key == Keys::PerSecond)
    {
        return shape.perSecond;
    }
    if (key == Keys::Step)
    {
        return shape.step;
    }
    return shape.seconds;
}

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
    bool ReadNumber(const OrderedJson& json, const std::string& field, Number& number);
    bool ReadShape(const OrderedJson& json, const std::string& field, Shape& shape);
    void ReadGlowMesh(const OrderedJson& json, const std::string& field, GlowMesh& mesh);
    void ReadLight(const OrderedJson& json, const std::string& field, std::optional<LayerLight>& light);
    void ReadItemValues(const OrderedJson& json, const std::string& field, ItemValues& values);
    void ReadItemNumber(const OrderedJson& json, const char* key, const std::string& field,
                        std::optional<Number>& value);
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
    if (json.contains(Keys::ItemValuesBefore))
    {
        AddFatal(field + "." + Keys::ItemValuesBefore,
                 "is a part of item effects; a render style leaves its item values after drawing (itemValuesAfter)");
    }
    WarnAboutUnknownKeys(json, {Keys::Name, Keys::Layers, Keys::ItemValuesAfter, Keys::ItemValuesBefore}, field);
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
    ReadLight(json, field, layer.light);
    if (const auto scrolling = json.find(Keys::ScrollingMesh); scrolling != json.end())
    {
        int mesh = 0;
        if (ReadMesh(*scrolling, field + "." + Keys::ScrollingMesh, mesh))
        {
            layer.scrollingMesh = mesh;
        }
    }
    WarnAboutUnknownKeys(json,
                         {Keys::Kind, Keys::Motion, Keys::Blend, Keys::Meshes, Keys::Texture, Keys::Opacity,
                          Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU, Keys::ScrollV, Keys::Light,
                          Keys::ScrollingMesh},
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
    if (!Items::Json::ReadWholeNumber(json, number) || number < 0 || number >= MaxModelMeshes)
    {
        AddFatal(field, json.dump() + " is not a mesh number (0 to " + std::to_string(MaxModelMeshes - 1) + ")");
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
    if (drawing && entry->is_string() && entry->get_ref<const std::string&>() == Drawing)
    {
        value = FromDrawing{};
        return;
    }
    if (entry->is_object())
    {
        Shape shape;
        if (ReadShape(*entry, field + "." + key, shape))
        {
            value = shape;
        }
        return;
    }
    if (entry->is_number())
    {
        value = entry->get<float>();
        return;
    }
    AddFatal(field + "." + key, drawing ? "must be a number, \"drawing\" or a shape" : "must be a number or a shape");
}

// A number, or a shape: {"shape": "wave", ...}.
bool LookReader::ReadNumber(const OrderedJson& json, const std::string& field, Number& number)
{
    if (json.is_number())
    {
        number = json.get<float>();
        return true;
    }
    if (json.is_object())
    {
        Shape shape;
        if (ReadShape(json, field, shape))
        {
            number = shape;
            return true;
        }
        return false;
    }
    AddFatal(field, "must be a number or a shape");
    return false;
}

bool LookReader::ReadShape(const OrderedJson& json, const std::string& field, Shape& shape)
{
    if (json.find(Keys::Shape) == json.end())
    {
        AddFatal(field + "." + Keys::Shape, "is missing: " + ListOf(ShapeNames));
        return false;
    }
    if (!ReadName(json, Keys::Shape, field, ShapeNames, shape.kind))
    {
        return false;
    }
    const std::string kind = NameOf(ShapeNames, shape.kind);
    bool valid = true;
    Items::Json::KeySet known{Keys::Shape};
    for (const char* key : ShapeFields(shape.kind))
    {
        known.insert(key);
        const auto entry = json.find(key);
        if (entry == json.end() || !entry->is_number())
        {
            AddFatal(field + "." + key, "must be a number: a " + kind + " has " + key);
            valid = false;
            continue;
        }
        if (std::string_view(key) == Keys::Count)
        {
            long long count = 0;
            if (!Items::Json::ReadWholeNumber(*entry, count) || count < 1 || count > 1000)
            {
                AddFatal(field + "." + key, "must be a whole number from 1 to 1000");
                valid = false;
            }
            shape.count = static_cast<int>(count);
            continue;
        }
        ShapeField(shape, key) = entry->get<double>();
    }
    WarnAboutUnknownKeys(json, known, field);
    if (!valid)
    {
        return false;
    }
    const bool timed = shape.kind != ShapeKind::Scroll && shape.kind != ShapeKind::Random;
    if (timed && !(shape.seconds > 0.0))
    {
        AddFatal(field + "." + (shape.kind == ShapeKind::Frames ? Keys::SecondsEach : Keys::Seconds),
                 "must be more than 0");
        return false;
    }
    if (shape.kind == ShapeKind::Random && !(shape.step > 0.0))
    {
        AddFatal(field + "." + Keys::Step, "must be more than 0");
        return false;
    }
    if (shape.kind == ShapeKind::Random && shape.to < shape.from)
    {
        AddFatal(field + "." + Keys::To, "must not be below from");
        return false;
    }
    return true;
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
    if (entry->is_string() && entry->get_ref<const std::string&>() == All)
    {
        mesh = AllGlowMeshes{};
        return;
    }
    int number = 0;
    if (ReadMesh(*entry, field + "." + Keys::GlowMesh, number))
    {
        mesh = number;
    }
}

// [red, green, blue], or {"tint": [red, green, blue]}; each a number or a shape.
void LookReader::ReadLight(const OrderedJson& json, const std::string& field, std::optional<LayerLight>& light)
{
    const auto entry = json.find(Keys::Light);
    if (entry == json.end())
    {
        return;
    }
    const std::string lightField = field + "." + Keys::Light;
    LayerLight read;
    const OrderedJson* color = &*entry;
    std::string colorField = lightField;
    if (entry->is_object())
    {
        const auto tint = entry->find(Keys::Tint);
        if (tint == entry->end() || entry->size() != 1)
        {
            AddFatal(lightField, "must be [red, green, blue] or {\"tint\": [red, green, blue]}");
            return;
        }
        read.tint = true;
        color = &*tint;
        colorField += std::string(".") + Keys::Tint;
    }
    if (!color->is_array() || color->size() != 3)
    {
        AddFatal(colorField, "must be [red, green, blue]: three numbers or shapes");
        return;
    }
    bool valid = true;
    for (std::size_t i = 0; i < 3; ++i)
    {
        valid = ReadNumber((*color)[i], colorField + "[" + std::to_string(i) + "]", read.color[i]) && valid;
    }
    if (valid)
    {
        light = read;
    }
}

void LookReader::ReadItemValues(const OrderedJson& json, const std::string& field, ItemValues& values)
{
    if (!json.is_object())
    {
        AddFatal(field, "must be an object of item values");
        return;
    }
    ReadItemNumber(json, Keys::Opacity, field, values.opacity);
    ReadMeshOrNone(json, Keys::GlowMesh, field, values.glowMesh);
    ReadItemNumber(json, Keys::GlowStrength, field, values.glowStrength);
    ReadItemNumber(json, Keys::ScrollU, field, values.scrollU);
    ReadItemNumber(json, Keys::ScrollV, field, values.scrollV);
    ReadMeshOrNone(json, Keys::HiddenMesh, field, values.hiddenMesh);
    ReadMeshOrNone(json, Keys::ScrollingMesh, field, values.scrollingMesh);
    if (values.IsEmpty())
    {
        AddIssue(ItemDataIssueSeverity::Warning, field, "sets no item value");
    }
    WarnAboutUnknownKeys(json,
                         {Keys::Opacity, Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU, Keys::ScrollV,
                          Keys::HiddenMesh, Keys::ScrollingMesh},
                         field);
}

void LookReader::ReadItemNumber(const OrderedJson& json, const char* key, const std::string& field,
                                std::optional<Number>& value)
{
    const auto entry = json.find(key);
    if (entry == json.end())
    {
        return;
    }
    Number number;
    if (ReadNumber(*entry, field + "." + key, number))
    {
        value = number;
    }
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

// A whole number without a fraction, as WriteFloat writes one.
OrderedJson WriteDouble(double value)
{
    if (value == std::floor(value) && std::abs(value) < static_cast<double>(1LL << 53))
    {
        return static_cast<long long>(value);
    }
    return value;
}

OrderedJson WriteShape(Shape shape)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Shape] = NameOf(ShapeNames, shape.kind);
    for (const char* key : ShapeFields(shape.kind))
    {
        json[key] =
            std::string_view(key) == Keys::Count ? OrderedJson(shape.count) : WriteDouble(ShapeField(shape, key));
    }
    return json;
}

OrderedJson WriteNumber(const Number& number)
{
    if (const Shape* shape = std::get_if<Shape>(&number))
    {
        return WriteShape(*shape);
    }
    return WriteFloat(std::get<float>(number));
}

OrderedJson WriteValue(const Value& value)
{
    if (std::holds_alternative<FromDrawing>(value))
    {
        return Drawing;
    }
    if (const Shape* shape = std::get_if<Shape>(&value))
    {
        return WriteShape(*shape);
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
    else if (std::holds_alternative<AllGlowMeshes>(layer.glowMesh))
    {
        json[Keys::GlowMesh] = All;
    }
    else if (const int* mesh = std::get_if<int>(&layer.glowMesh))
    {
        json[Keys::GlowMesh] = *mesh;
    }
    PutValue(json, Keys::GlowStrength, layer.glowStrength);
    PutValue(json, Keys::ScrollU, layer.scrollU);
    PutValue(json, Keys::ScrollV, layer.scrollV);
    if (layer.light)
    {
        OrderedJson color = OrderedJson::array();
        for (const Number& channel : layer.light->color)
        {
            color.push_back(WriteNumber(channel));
        }
        if (layer.light->tint)
        {
            json[Keys::Light] = OrderedJson{{Keys::Tint, std::move(color)}};
        }
        else
        {
            json[Keys::Light] = std::move(color);
        }
    }
    if (layer.scrollingMesh)
    {
        json[Keys::ScrollingMesh] = *layer.scrollingMesh;
    }
    return json;
}

OrderedJson WriteItemValues(const ItemValues& values)
{
    OrderedJson json = OrderedJson::object();
    if (values.opacity)
    {
        json[Keys::Opacity] = WriteNumber(*values.opacity);
    }
    if (values.glowMesh)
    {
        json[Keys::GlowMesh] = WriteMeshOrNone(*values.glowMesh);
    }
    if (values.glowStrength)
    {
        json[Keys::GlowStrength] = WriteNumber(*values.glowStrength);
    }
    if (values.scrollU)
    {
        json[Keys::ScrollU] = WriteNumber(*values.scrollU);
    }
    if (values.scrollV)
    {
        json[Keys::ScrollV] = WriteNumber(*values.scrollV);
    }
    if (values.hiddenMesh)
    {
        json[Keys::HiddenMesh] = WriteMeshOrNone(*values.hiddenMesh);
    }
    if (values.scrollingMesh)
    {
        json[Keys::ScrollingMesh] = WriteMeshOrNone(*values.scrollingMesh);
    }
    return json;
}
} // namespace

std::string_view GetRenderStylesFileName()
{
    return "RenderStyles.json";
}

void ReadRenderStylesFile(std::string_view text, const std::string& source, RenderStylesFile& file,
                          std::vector<Items::ItemDataIssue>& issues)
{
    std::vector<LookDefinition>& looks = file.looks;
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
    std::set<std::string, std::less<>> names;
    for (const LookDefinition& look : looks)
    {
        names.insert(look.name);
    }
    file.folders = Colors::ReadColorFolders(
        root, source, [&names](std::string_view name) { return names.contains(name); }, issues, Colors::LookFolderKeys);
    Items::Json::WarnAboutUnknownKeys(
        root, {Items::Json::Keys::FormatVersion, Keys::Kind, Keys::LookFolders, Keys::Looks}, "",
        [&](const std::string& field, const std::string& message)
        {
            issues.push_back(
                {ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
        });
}

void ReadRenderStylesJson(std::string_view text, const std::string& source, std::vector<LookDefinition>& looks,
                          std::vector<Items::ItemDataIssue>& issues)
{
    RenderStylesFile file;
    ReadRenderStylesFile(text, source, file, issues);
    looks.insert(looks.end(), std::make_move_iterator(file.looks.begin()), std::make_move_iterator(file.looks.end()));
}

std::vector<std::string> ReadLookNames(std::string_view text)
{
    std::vector<std::string> names;
    const OrderedJson root = OrderedJson::parse(text, nullptr, false);
    if (!root.is_object())
    {
        return names;
    }
    const auto list = root.find(Keys::Looks);
    if (list == root.end() || !list->is_array())
    {
        return names;
    }
    for (const OrderedJson& look : *list)
    {
        const auto name = look.is_object() ? look.find(Keys::Name) : look.end();
        names.push_back(name != look.end() && name->is_string() ? name->get<std::string>() : "");
    }
    return names;
}

std::string WriteRenderStylesJson(std::vector<LookDefinition> looks, std::span<const Colors::ColorFolder> folders)
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
    if (!folders.empty())
    {
        const auto isLook = [&looks](std::string_view name)
        { return std::ranges::any_of(looks, [name](const LookDefinition& look) { return look.name == name; }); };
        root[Colors::LookFolderKeys.list] = Colors::WriteColorFolders(folders, isLook, Colors::LookFolderKeys);
    }
    root[Keys::Looks] = std::move(list);
    std::string text = root.dump(Items::Json::Indent);
    text = Colors::PutColorFoldersOnLines(text, Colors::LookFolderKeys);
    text = Items::Json::PutListEntriesOnOneLine(text, Keys::Layers);
    text = Items::Json::PutObjectsOnOneLine(text, Keys::ItemValuesAfter);
    return text + "\n";
}
} // namespace Data::Looks
