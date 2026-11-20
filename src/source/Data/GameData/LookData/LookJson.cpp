#include "stdafx.h"

#include "LookJson.h"

#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/Common/MeshLimits.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/LookData/BoneThingTypes.h"
#include "Data/GameData/MonsterData/MonsterJsonCommon.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
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
constexpr const char* Where = "where";
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
constexpr const char* Brightness = "brightness";
constexpr const char* ScrollingMesh = "scrollingMesh";
constexpr const char* Shape = "shape";
constexpr const char* From = "from";
constexpr const char* To = "to";
constexpr const char* Seconds = "seconds";
constexpr const char* PerSecond = "perSecond";
constexpr const char* Step = "step";
constexpr const char* Count = "count";
constexpr const char* SecondsEach = "secondsEach";
constexpr const char* TimesItem = "timesItem";
constexpr const char* NotFor = "notFor";
constexpr const char* OnlyWhen = "onlyWhen";
constexpr const char* Place = "place";
constexpr const char* Level = "level";
constexpr const char* Buff = "buff";
constexpr const char* WithoutBuff = "withoutBuff";
constexpr const char* EachItem = "eachItem";
constexpr const char* Default = "default";
constexpr const char* DrawingOpacity = "drawingOpacity";
constexpr const char* GlowsLikeLevel = "glowsLikeLevel";
constexpr const char* ObjectLight = "objectLight";
constexpr const char* DrawsAlone = "drawsAlone";
constexpr const char* OnBones = "onBones";
constexpr const char* SubType = "subType";
constexpr const char* Bones = "bones";
constexpr const char* Between = "between";
constexpr const char* At = "at";
constexpr const char* Origin = "origin";
constexpr const char* Scale = "scale";
constexpr const char* Color = "color";
constexpr const char* Rotation = "rotation";
constexpr const char* Reversed = "reversed";
constexpr const char* Chance = "chance";
constexpr const char* OneIn = "oneIn";
constexpr const char* Draw = "draw";
constexpr const char* Per = "per";
constexpr const char* Together = "together";
constexpr const char* Things = "things";
constexpr const char* ItemTimes = "itemTimes";
constexpr const char* AnimationSpeed = "animationSpeed";
} // namespace Keys

constexpr const char* RenderStyleKind = "renderStyle";
constexpr const char* ItemEffectKind = "itemEffect";

const char* KindName(LookKind kind)
{
    return kind == LookKind::ItemEffect ? ItemEffectKind : RenderStyleKind;
}
constexpr const char* Drawing = "drawing";
constexpr const char* None = "none";
constexpr const char* All = "all";

constexpr std::array<std::pair<LayerKind, const char*>, 3> LayerKindNames{{
    {LayerKind::Texture, "texture"},
    {LayerKind::Reflection, "reflection"},
    {LayerKind::FlatColor, "flatColor"},
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

constexpr std::array<std::pair<Blend, const char*>, 4> BlendNames{{
    {Blend::Normal, "normal"},
    {Blend::Glow, "glow"},
    {Blend::Darken, "darken"},
    {Blend::AsDrawing, "asDrawing"},
}};

constexpr std::array<std::pair<Where, const char*>, 2> WhereNames{{
    {Where::Over, "over"},
    {Where::UnderHoles, "underHoles"},
}};

constexpr std::array<std::pair<ShapeKind, const char*>, 8> ShapeNames{{
    {ShapeKind::Wave, "wave"},
    {ShapeKind::Bounce, "bounce"},
    {ShapeKind::Scroll, "scroll"},
    {ShapeKind::Ramp, "ramp"},
    {ShapeKind::Zigzag, "zigzag"},
    {ShapeKind::Turn, "turn"},
    {ShapeKind::Frames, "frames"},
    {ShapeKind::Random, "random"},
}};

// The drawings a render style can be not for (NotFor).
enum class NotForDrawing
{
    Doppelganger,
    OutsideMonsterHands,
};

constexpr std::array<std::pair<NotForDrawing, const char*>, 2> NotForNames{{
    {NotForDrawing::Doppelganger, "doppelganger"},
    {NotForDrawing::OutsideMonsterHands, "outsideMonsterHands"},
}};

constexpr std::array<std::pair<Place, const char*>, 4> PlaceNames{{
    {Place::Worn, "worn"},
    {Place::InventoryAndGround, "inventoryAndGround"},
    {Place::Inventory, "inventory"},
    {Place::Ground, "ground"},
}};

constexpr std::array<std::pair<LookBuff, const char*>, 1> BuffNames{{
    {LookBuff::InfinityArrow, "infinityArrow"},
}};

constexpr std::array<std::pair<ChanceDraw, const char*>, 2> ChanceDrawNames{{
    {ChanceDraw::Fps, "fps"},
    {ChanceDraw::Plain, "plain"},
}};

constexpr const char* PerEach = "each";
constexpr const char* PerOnce = "once";

// The highest bone number a thing on bones names; a model's own bones are checked when it is created.
constexpr int MaxBone = MaxModelBones - 1;

// The highest item level, for a layer's level condition.
constexpr long long MaxItemLevel = 15;

// The fields of each shape, in the order they are written.
std::vector<const char*> ShapeFields(ShapeKind kind)
{
    switch (kind)
    {
    case ShapeKind::Wave:
    case ShapeKind::Bounce:
    case ShapeKind::Ramp:
    case ShapeKind::Zigzag:
        return {Keys::From, Keys::To, Keys::Seconds};
    case ShapeKind::Scroll:
        return {Keys::PerSecond};
    case ShapeKind::Frames:
        return {Keys::Count, Keys::Step, Keys::SecondsEach};
    case ShapeKind::Random:
        return {Keys::From, Keys::To, Keys::Step};
    case ShapeKind::Turn:
        return {Keys::From, Keys::To};
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
    LookReader(const std::string& source, LookKind kind, std::vector<ItemDataIssue>& issues)
        : m_source(source), m_kind(kind), m_issues(issues)
    {
    }

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
    void ReadRenderStyleParts(const OrderedJson& json, const std::string& field, LookDefinition& look);
    void ReadItemEffectParts(const OrderedJson& json, const std::string& field, LookDefinition& look);
    void ReadItemValuesBefore(const OrderedJson& json, const std::string& field, ItemValuesBefore& entry);
    void ReadItemValueFields(const OrderedJson& json, const std::string& field, ItemValues& values);
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
    void ReadNotFor(const OrderedJson& json, const std::string& field, NotFor& notFor);
    OrderedJson TakeEachItem(const OrderedJson& json, const std::string& field, EachItemFields& eachItem);
    void ReadOnlyWhen(const OrderedJson& json, const std::string& field, OnlyWhen& onlyWhen);
    void ReadOnBones(const OrderedJson& json, const std::string& field, std::vector<OnBones>& onBones);
    bool ReadOnBonesEntry(const OrderedJson& json, const std::string& field, OnBones& entry);
    bool ReadBoneThing(const OrderedJson& json, const std::string& field, bool inGroup, BoneThing& thing);
    bool ReadBonePlaces(const OrderedJson& json, const std::string& field, BonePlaces& places);
    bool ReadChance(const OrderedJson& json, const std::string& field, std::optional<Chance>& chance,
                    bool ofPlaces = true);
    bool ReadColor(const OrderedJson& json, const std::string& field, std::array<Number, 3>& color);
    // Whether the thing's kind fits the places: lightning goes between pairs, everything else at bones or the origin.
    bool PlacesFit(const BoneThing& thing, const BonePlaces& places, const std::string& field);
    bool ChanceFitsPlaces(const OrderedJson& json, const std::string& field, const BonePlaces& places);

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
    LookKind m_kind;
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
    if (m_kind == LookKind::ItemEffect)
    {
        ReadItemEffectParts(json, field, look);
        return true;
    }
    const auto layers = json.find(Keys::Layers);
    if (layers == json.end() || !layers->is_array())
    {
        AddFatal(field + "." + Keys::Layers, "must be a list of layers");
        return false;
    }
    ReadRenderStyleParts(json, field, look);
    return true;
}

void LookReader::ReadRenderStyleParts(const OrderedJson& json, const std::string& field, LookDefinition& look)
{
    ReadNotFor(json, field, look.notFor);
    if (const auto alone = json.find(Keys::DrawsAlone); alone != json.end())
    {
        if (!alone->is_boolean())
        {
            AddFatal(field + "." + Keys::DrawsAlone, "must be true or false");
        }
        else
        {
            look.drawsAlone = alone->get<bool>();
        }
    }
    const auto layers = json.find(Keys::Layers);
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
    for (const char* part : {Keys::ObjectLight, Keys::OnBones, Keys::AnimationSpeed})
    {
        if (json.contains(part))
        {
            AddFatal(field + "." + part, "is a part of item effects");
        }
    }
    WarnAboutUnknownKeys(json,
                         {Keys::Name, Keys::NotFor, Keys::DrawsAlone, Keys::Layers, Keys::ItemValuesAfter,
                          Keys::ItemValuesBefore, Keys::ObjectLight, Keys::OnBones, Keys::AnimationSpeed},
                         field);
}

// An item effect (design L13): the item values it sets before the drawing, and the light it puts on the object.
void LookReader::ReadItemEffectParts(const OrderedJson& json, const std::string& field, LookDefinition& look)
{
    for (const char* part : {Keys::Layers, Keys::NotFor, Keys::DrawsAlone, Keys::ItemValuesAfter})
    {
        if (json.contains(part))
        {
            AddFatal(field + "." + part,
                     "is a part of render styles; an item effect does what comes before the drawing");
        }
    }
    if (const auto before = json.find(Keys::ItemValuesBefore); before != json.end())
    {
        const std::string beforeField = field + "." + Keys::ItemValuesBefore;
        if (!before->is_array())
        {
            AddFatal(beforeField, "must be a list of item values, each with its Only when");
        }
        else
        {
            for (std::size_t i = 0; i < before->size(); ++i)
            {
                ItemValuesBefore entry;
                ReadItemValuesBefore((*before)[i], beforeField + "[" + std::to_string(i) + "]", entry);
                look.itemValuesBefore.push_back(std::move(entry));
            }
        }
    }
    if (const auto light = json.find(Keys::ObjectLight); light != json.end())
    {
        const std::string lightField = field + "." + Keys::ObjectLight;
        std::array<Number, 3> color{1.f, 1.f, 1.f};
        bool valid = light->is_array() && light->size() == 3;
        if (!valid)
        {
            AddFatal(lightField, "must be [red, green, blue]: three numbers or shapes");
        }
        for (std::size_t i = 0; valid && i < 3; ++i)
        {
            valid = ReadNumber((*light)[i], lightField + "[" + std::to_string(i) + "]", color[i]);
        }
        if (valid)
        {
            look.objectLight = color;
        }
    }
    ReadOnBones(json, field, look.onBones);
    if (const auto speed = json.find(Keys::AnimationSpeed); speed != json.end())
    {
        if (!speed->is_number() || speed->get<float>() <= 0.f)
        {
            AddFatal(field + "." + Keys::AnimationSpeed,
                     "must be a number above 0: how fast the object's animation plays");
        }
        else
        {
            look.animationSpeed = speed->get<float>();
        }
    }
    if (look.itemValuesBefore.empty() && !look.objectLight && look.onBones.empty() && !look.animationSpeed)
    {
        AddIssue(ItemDataIssueSeverity::Warning, field,
                 "does nothing: no item values before the drawing, no object light, nothing on bones, no animation");
    }
    WarnAboutUnknownKeys(json,
                         {Keys::Name, Keys::ItemValuesBefore, Keys::ObjectLight, Keys::OnBones, Keys::AnimationSpeed,
                          Keys::Layers, Keys::NotFor, Keys::DrawsAlone, Keys::ItemValuesAfter},
                         field);
}

// The things an item effect creates on the item's bones (section 7b): a list of entries, each one thing or a group.
void LookReader::ReadOnBones(const OrderedJson& json, const std::string& field, std::vector<OnBones>& onBones)
{
    const auto list = json.find(Keys::OnBones);
    if (list == json.end())
    {
        return;
    }
    const std::string listField = field + "." + Keys::OnBones;
    if (!list->is_array())
    {
        AddFatal(listField, "must be a list of things on bones");
        return;
    }
    for (std::size_t i = 0; i < list->size(); ++i)
    {
        OnBones entry;
        if (ReadOnBonesEntry((*list)[i], listField + "[" + std::to_string(i) + "]", entry))
        {
            onBones.push_back(std::move(entry));
        }
    }
}

bool LookReader::PlacesFit(const BoneThing& thing, const BonePlaces& places, const std::string& field)
{
    const bool joint = thing.kind == BoneThingKind::Joint;
    if (joint == !places.between.empty())
    {
        return true;
    }
    AddFatal(field, joint ? "is lightning: it goes between pairs of bones (\"between\")"
                          : "goes at bones or at the origin: only lightning goes between pairs of bones");
    return false;
}

// The item's place is one place: a chance there rolls once, so it has no "per".
bool LookReader::ChanceFitsPlaces(const OrderedJson& json, const std::string& field, const BonePlaces& places)
{
    const auto chance = json.find(Keys::Chance);
    if (!places.origin || chance == json.end() || !chance->is_object() || !chance->contains(Keys::Per))
    {
        return true;
    }
    AddFatal(field + "." + Keys::Chance + "." + Keys::Per,
             "is for bones and pairs: the item's place is one place, its chance rolls once");
    return false;
}

bool LookReader::ReadOnBonesEntry(const OrderedJson& json, const std::string& field, OnBones& entry)
{
    if (!json.is_object())
    {
        AddFatal(field, "must be a thing on bones, e.g. {\"sprite\": \"light\", \"bones\": [8]}, or a group");
        return false;
    }
    const auto together = json.find(Keys::Together);
    if (together == json.end())
    {
        // One thing: its places and its chance are the entry's.
        BoneThing thing;
        if (!ReadBoneThing(json, field, false, thing))
        {
            return false;
        }
        if (!ReadBonePlaces(json, field, entry.places) || !PlacesFit(thing, entry.places, field) ||
            !ChanceFitsPlaces(json, field, entry.places))
        {
            return false;
        }
        entry.chance = thing.chance;
        thing.chance.reset();
        entry.things.push_back(std::move(thing));
        return true;
    }
    // A group: its places and chance in "together", its things in "things".
    entry.together = true;
    const std::string groupField = field + "." + Keys::Together;
    if (!together->is_object())
    {
        AddFatal(groupField, "must hold the group's places (\"bones\", \"between\" or \"at\") and its chance");
        return false;
    }
    const bool placesRead = ReadBonePlaces(*together, groupField, entry.places);
    const bool chanceRead = ReadChance(*together, groupField, entry.chance);
    bool valid = placesRead && chanceRead;
    // Against places and a chance that were read: a fault in either is named already, as the single form stops there.
    if (valid && !ChanceFitsPlaces(*together, groupField, entry.places))
    {
        valid = false;
    }
    WarnAboutUnknownKeys(*together, {Keys::Bones, Keys::Between, Keys::At, Keys::Chance}, groupField);
    WarnAboutUnknownKeys(json, {Keys::Together, Keys::Things}, field);
    const auto things = json.find(Keys::Things);
    const std::string thingsField = field + "." + Keys::Things;
    if (things == json.end() || !things->is_array() || things->empty())
    {
        AddFatal(thingsField, "must be a list of the things the group creates together");
        return false;
    }
    for (std::size_t i = 0; i < things->size(); ++i)
    {
        const std::string thingField = thingsField + "[" + std::to_string(i) + "]";
        BoneThing thing;
        if (!ReadBoneThing((*things)[i], thingField, true, thing))
        {
            valid = false;
            continue;
        }
        // Against places that were read (a fault in them is named already), whatever failed before this thing.
        if (placesRead)
        {
            valid = PlacesFit(thing, entry.places, thingField) && valid;
        }
        entry.things.push_back(std::move(thing));
    }
    return valid;
}

// The kind and type, sub type, values and (in a group) the thing's own chance; places are read by the caller.
bool LookReader::ReadBoneThing(const OrderedJson& json, const std::string& field, bool inGroup, BoneThing& thing)
{
    if (!json.is_object())
    {
        AddFatal(field, "must be a thing on bones, e.g. {\"sprite\": \"light\"}");
        return false;
    }
    int kinds = 0;
    for (const auto& [kind, key] : BoneThingKeys)
    {
        const auto type = json.find(key);
        if (type == json.end())
        {
            continue;
        }
        ++kinds;
        thing.kind = kind;
        if (!type->is_string() || type->get_ref<const std::string&>().empty())
        {
            AddFatal(field + "." + key, "must be the name of a type in the effect catalogue");
            return false;
        }
        thing.type = type->get<std::string>();
    }
    if (kinds != 1)
    {
        AddFatal(field, "must name one type: " + ListOf(BoneThingKeys));
        return false;
    }
    const auto addFatal = [this](const std::string& at, const std::string& message) { AddFatal(at, message); };
    bool valid = true;
    if (const auto subType = json.find(Keys::SubType); subType != json.end())
    {
        valid = Items::Json::ReadWholeNumberInRange(*subType, field + "." + Keys::SubType, 0, MaxBoneThingSubType,
                                                    thing.subType, addFatal);
    }
    if (const auto scale = json.find(Keys::Scale); scale != json.end())
    {
        valid = ReadNumber(*scale, field + "." + Keys::Scale, thing.scale) && valid;
    }
    if (const auto color = json.find(Keys::Color); color != json.end())
    {
        valid = ReadColor(*color, field + "." + Keys::Color, thing.color) && valid;
    }
    if (const auto rotation = json.find(Keys::Rotation); rotation != json.end())
    {
        if (thing.kind != BoneThingKind::Sprite)
        {
            AddFatal(field + "." + Keys::Rotation, "is only for a sprite");
            valid = false;
        }
        valid = ReadNumber(*rotation, field + "." + Keys::Rotation, thing.rotation) && valid;
    }
    if (const auto reversed = json.find(Keys::Reversed); reversed != json.end())
    {
        if (!reversed->is_boolean() || thing.kind != BoneThingKind::Joint)
        {
            AddFatal(field + "." + Keys::Reversed, "is true or false, only for lightning");
            valid = false;
        }
        else
        {
            thing.reversed = reversed->get<bool>();
        }
    }
    valid = ReadChance(json, field, thing.chance, !inGroup) && valid;
    Items::Json::KeySet known{Keys::SubType, Keys::Scale, Keys::Color, Keys::Rotation, Keys::Reversed, Keys::Chance};
    for (const auto& [kind, key] : BoneThingKeys)
    {
        known.insert(key);
    }
    if (inGroup)
    {
        for (const char* key : {Keys::Bones, Keys::Between, Keys::At})
        {
            if (json.contains(key))
            {
                AddFatal(field + "." + key, "is the group's: the things of a group share its places");
                valid = false;
            }
        }
    }
    else
    {
        known.insert({Keys::Bones, Keys::Between, Keys::At});
    }
    WarnAboutUnknownKeys(json, known, field);
    return valid;
}

// "bones": [8, 9], "between": [[30, 22]] or "at": "origin", exactly one of them.
bool LookReader::ReadBonePlaces(const OrderedJson& json, const std::string& field, BonePlaces& places)
{
    const auto bones = json.find(Keys::Bones);
    const auto between = json.find(Keys::Between);
    const auto at = json.find(Keys::At);
    const int named = (bones != json.end() ? 1 : 0) + (between != json.end() ? 1 : 0) + (at != json.end() ? 1 : 0);
    if (named != 1)
    {
        AddFatal(field, "must name its places once: \"bones\", \"between\" or \"at\": \"origin\"");
        return false;
    }
    const auto addFatal = [this](const std::string& at, const std::string& message) { AddFatal(at, message); };
    if (at != json.end())
    {
        if (!at->is_string() || at->get_ref<const std::string&>() != Keys::Origin)
        {
            AddFatal(field + "." + Keys::At, "must be \"origin\": the item object's own place");
            return false;
        }
        places.origin = true;
        return true;
    }
    if (bones != json.end())
    {
        const std::string bonesField = field + "." + Keys::Bones;
        if (!bones->is_array() || bones->empty())
        {
            AddFatal(bonesField, "must be a list of bone numbers");
            return false;
        }
        bool valid = true;
        for (std::size_t i = 0; i < bones->size(); ++i)
        {
            int bone = 0;
            valid = Items::Json::ReadWholeNumberInRange((*bones)[i], bonesField + "[" + std::to_string(i) + "]", 0,
                                                        MaxBone, bone, addFatal) &&
                    valid;
            places.bones.push_back(bone);
        }
        return valid;
    }
    const std::string betweenField = field + "." + Keys::Between;
    if (!between->is_array() || between->empty())
    {
        AddFatal(betweenField, "must be a list of pairs of bones, [from, to]");
        return false;
    }
    bool valid = true;
    for (std::size_t i = 0; i < between->size(); ++i)
    {
        const std::string pairField = betweenField + "[" + std::to_string(i) + "]";
        const OrderedJson& pair = (*between)[i];
        if (!pair.is_array() || pair.size() != 2)
        {
            AddFatal(pairField, "must be a pair of bones, [from, to]");
            valid = false;
            continue;
        }
        std::pair<int, int> bonePair;
        valid = Items::Json::ReadWholeNumberInRange(pair[0], pairField + "[0]", 0, MaxBone, bonePair.first, addFatal) &&
                valid;
        valid =
            Items::Json::ReadWholeNumberInRange(pair[1], pairField + "[1]", 0, MaxBone, bonePair.second, addFatal) &&
            valid;
        places.between.push_back(bonePair);
    }
    return valid;
}

// "chance": {"oneIn": 2, "draw": "plain", "per": "once"}; without draw and per: rand_fps_check for each place. A
// thing's own chance in a group (not ofPlaces) rolls once each time the group's holds, so it has no "per".
bool LookReader::ReadChance(const OrderedJson& json, const std::string& field, std::optional<Chance>& chance,
                            bool ofPlaces)
{
    const auto entry = json.find(Keys::Chance);
    if (entry == json.end())
    {
        return true;
    }
    const std::string chanceField = field + "." + Keys::Chance;
    if (!entry->is_object())
    {
        AddFatal(chanceField, "must be {\"oneIn\": N}: once in N frames");
        return false;
    }
    const auto oneIn = entry->find(Keys::OneIn);
    if (oneIn == entry->end())
    {
        AddFatal(chanceField + "." + Keys::OneIn, "is missing: once in how many frames");
        return false;
    }
    const auto addFatal = [this](const std::string& at, const std::string& message) { AddFatal(at, message); };
    Chance read;
    bool valid = Items::Json::ReadWholeNumberInRange(*oneIn, chanceField + "." + Keys::OneIn, 1, MaxBoneThingOneIn,
                                                     read.oneIn, addFatal);
    if (entry->contains(Keys::Draw))
    {
        valid = ReadName(*entry, Keys::Draw, chanceField, ChanceDrawNames, read.draw) && valid;
    }
    if (const auto per = entry->find(Keys::Per); per != entry->end() && !ofPlaces)
    {
        AddFatal(chanceField + "." + Keys::Per,
                 "is the group's: a thing's own chance rolls once each time the group's chance holds");
        valid = false;
    }
    else if (per != entry->end())
    {
        const std::string text = per->is_string() ? per->get<std::string>() : std::string();
        read.once = text == PerOnce;
        if (text != PerEach && text != PerOnce)
        {
            AddFatal(chanceField + "." + Keys::Per,
                     std::string("must be \"") + PerEach + "\" (each place) or \"" + PerOnce + "\" (all at once)");
            valid = false;
        }
    }
    WarnAboutUnknownKeys(*entry, {Keys::OneIn, Keys::Draw, Keys::Per}, chanceField);
    if (valid)
    {
        chance = read;
    }
    return valid;
}

bool LookReader::ReadColor(const OrderedJson& json, const std::string& field, std::array<Number, 3>& color)
{
    if (!json.is_array() || json.size() != 3)
    {
        AddFatal(field, "must be [red, green, blue]: three numbers or shapes");
        return false;
    }
    bool valid = true;
    for (std::size_t i = 0; i < 3; ++i)
    {
        valid = ReadNumber(json[i], field + "[" + std::to_string(i) + "]", color[i]) && valid;
    }
    return valid;
}

void LookReader::ReadItemValuesBefore(const OrderedJson& marked, const std::string& field, ItemValuesBefore& entry)
{
    if (!marked.is_object())
    {
        AddFatal(field, "must be an object of item values");
        return;
    }
    const OrderedJson json = TakeEachItem(marked, field, entry.values.eachItem);
    ReadOnlyWhen(json, field, entry.onlyWhen);
    if (json.contains(Keys::Light))
    {
        AddFatal(field + "." + Keys::Light, "can't be set before the drawing: the drawing sets the model's light");
    }
    ReadItemValueFields(json, field, entry.values);
    entry.values.light.reset();
    ReadItemNumber(json, Keys::DrawingOpacity, field, entry.drawingOpacity);
    if (const auto level = json.find(Keys::GlowsLikeLevel); level != json.end())
    {
        long long glowsLike = 0;
        if (!Items::Json::ReadWholeNumber(*level, glowsLike) || glowsLike < 0 || glowsLike > MaxItemLevel)
        {
            AddFatal(field + "." + Keys::GlowsLikeLevel,
                     "must be an item level from 0 to " + std::to_string(MaxItemLevel));
        }
        else
        {
            entry.glowsLikeLevel = static_cast<int>(glowsLike);
        }
    }
    if (entry.IsEmpty())
    {
        AddIssue(ItemDataIssueSeverity::Warning, field, "sets no item value");
    }
    WarnAboutUnknownKeys(json,
                         {Keys::OnlyWhen, Keys::Opacity, Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU,
                          Keys::ScrollV, Keys::HiddenMesh, Keys::ScrollingMesh, Keys::DrawingOpacity,
                          Keys::GlowsLikeLevel, Keys::Light},
                         field);
}

void LookReader::ReadLayer(const OrderedJson& marked, const std::string& field, Layer& layer)
{
    if (!marked.is_object())
    {
        AddFatal(field, "must be a layer: an object with a kind");
        return;
    }
    const OrderedJson json = TakeEachItem(marked, field, layer.eachItem);
    if (json.find(Keys::Kind) == json.end())
    {
        AddFatal(field + "." + Keys::Kind, "is missing: " + ListOf(LayerKindNames));
    }
    ReadName(json, Keys::Kind, field, LayerKindNames, layer.kind);
    if (ReadName(json, Keys::Motion, field, MotionNames, layer.motion) && layer.kind != LayerKind::Reflection)
    {
        AddFatal(field + "." + Keys::Motion, "is only for a reflection");
    }
    if (ReadName(json, Keys::Where, field, WhereNames, layer.where) && layer.kind != LayerKind::Reflection)
    {
        AddFatal(field + "." + Keys::Where, "is only for a reflection");
    }
    ReadName(json, Keys::Blend, field, BlendNames, layer.blend);
    ReadMeshes(json, field, layer.meshes);
    ReadOnlyWhen(json, field, layer.onlyWhen);
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
                         {Keys::Kind, Keys::Motion, Keys::Where, Keys::Blend, Keys::Meshes, Keys::OnlyWhen,
                          Keys::Texture, Keys::Opacity, Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU,
                          Keys::ScrollV, Keys::Light, Keys::ScrollingMesh},
                         field);
}

// The fields each item sets: {"eachItem": "chromeMeshes", "default": [1]} is read as the field's value [1], or as no
// value without a default, and its name is kept (design L12).
OrderedJson LookReader::TakeEachItem(const OrderedJson& json, const std::string& field, EachItemFields& eachItem)
{
    OrderedJson fields = json;
    for (const auto& [key, value] : json.items())
    {
        if (!value.is_object() || !value.contains(Keys::EachItem))
        {
            continue;
        }
        const std::string keyField = field + "." + key;
        const OrderedJson& name = value[Keys::EachItem];
        if (!name.is_string() || !Items::Json::IsName(name.get_ref<const std::string&>()))
        {
            AddFatal(keyField + "." + Keys::EachItem, "must be the name of the item's value, of letters and digits");
            fields.erase(key);
            continue;
        }
        eachItem[key] = name.get<std::string>();
        WarnAboutUnknownKeys(value, {Keys::EachItem, Keys::Default}, keyField);
        if (const auto fallback = value.find(Keys::Default); fallback != value.end())
        {
            fields[key] = *fallback;
        }
        else
        {
            fields.erase(key);
        }
    }
    return fields;
}

// ["doppelganger", "outsideMonsterHands"]: the drawings the look is not for.
void LookReader::ReadNotFor(const OrderedJson& json, const std::string& field, NotFor& notFor)
{
    const auto entry = json.find(Keys::NotFor);
    if (entry == json.end())
    {
        return;
    }
    const std::string notForField = field + "." + Keys::NotFor;
    if (!entry->is_array())
    {
        AddFatal(notForField, "must be a list of " + ListOf(NotForNames));
        return;
    }
    for (std::size_t i = 0; i < entry->size(); ++i)
    {
        const OrderedJson& name = (*entry)[i];
        const std::optional<NotForDrawing> drawing =
            name.is_string() ? ValueOf(NotForNames, name.get_ref<const std::string&>()) : std::nullopt;
        if (!drawing)
        {
            AddFatal(notForField + "[" + std::to_string(i) + "]", "must be one of " + ListOf(NotForNames));
            continue;
        }
        (*drawing == NotForDrawing::Doppelganger ? notFor.doppelganger : notFor.outsideMonsterHands) = true;
    }
}

// {"place": "worn", "level": [0, 0], "hiddenMesh": 1, "buff": "infinityArrow"}: when the layer draws.
void LookReader::ReadOnlyWhen(const OrderedJson& json, const std::string& field, OnlyWhen& onlyWhen)
{
    const auto entry = json.find(Keys::OnlyWhen);
    if (entry == json.end())
    {
        return;
    }
    const std::string whenField = field + "." + Keys::OnlyWhen;
    if (!entry->is_object())
    {
        AddFatal(whenField, "must be an object of conditions, e.g. {\"place\": \"worn\"}");
        return;
    }
    Place place = Place::Worn;
    if (ReadName(*entry, Keys::Place, whenField, PlaceNames, place))
    {
        onlyWhen.place = place;
    }
    if (const auto level = entry->find(Keys::Level); level != entry->end())
    {
        long long from = 0;
        long long to = 0;
        if (!level->is_array() || level->size() != 2 || !Items::Json::ReadWholeNumber((*level)[0], from) ||
            !Items::Json::ReadWholeNumber((*level)[1], to) || from < 0 || to > MaxItemLevel || from > to)
        {
            AddFatal(whenField + "." + Keys::Level, "must be [from, to]: item levels from 0 to " +
                                                        std::to_string(MaxItemLevel) + ", from not above to");
        }
        else
        {
            onlyWhen.level = std::pair<int, int>(static_cast<int>(from), static_cast<int>(to));
        }
    }
    if (const auto hidden = entry->find(Keys::HiddenMesh); hidden != entry->end())
    {
        int mesh = 0;
        if (hidden->is_string() && hidden->get_ref<const std::string&>() == None)
        {
            onlyWhen.hiddenMesh = -1;
        }
        else if (ReadMesh(*hidden, whenField + "." + Keys::HiddenMesh, mesh))
        {
            onlyWhen.hiddenMesh = mesh;
        }
    }
    LookBuff buff = LookBuff::InfinityArrow;
    if (ReadName(*entry, Keys::Buff, whenField, BuffNames, buff))
    {
        onlyWhen.buff = buff;
    }
    if (ReadName(*entry, Keys::WithoutBuff, whenField, BuffNames, buff))
    {
        onlyWhen.withoutBuff = buff;
    }
    if (entry->empty())
    {
        AddIssue(ItemDataIssueSeverity::Warning, whenField, "sets no condition: the layer always draws");
    }
    WarnAboutUnknownKeys(*entry, {Keys::Place, Keys::Level, Keys::HiddenMesh, Keys::Buff, Keys::WithoutBuff},
                         whenField);
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
    if (entry->is_string() && entry->get_ref<const std::string&>() == None)
    {
        meshes.none = true;
        return;
    }
    if (entry->is_object())
    {
        const auto allBut = entry->find(Keys::AllBut);
        if (allBut == entry->end() || entry->size() != 1)
        {
            AddFatal(meshesField, "must be a list of meshes, {\"allBut\": [meshes]}, \"allEvenHidden\" or \"none\"");
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
    if (entry->is_object() && entry->contains(Keys::ItemTimes))
    {
        // The item's value times a number (design L59).
        const OrderedJson& times = (*entry)[Keys::ItemTimes];
        if (!times.is_number())
        {
            AddFatal(field + "." + key + "." + Keys::ItemTimes,
                     "must be a number: the item's value is multiplied by it");
            return;
        }
        WarnAboutUnknownKeys(*entry, {Keys::ItemTimes}, field + "." + key);
        value = FromItem{times.get<float>()};
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
    AddFatal(field + "." + key, drawing ? "must be a number, \"drawing\", a shape or {\"itemTimes\": N}"
                                        : "must be a number, a shape or {\"itemTimes\": N}");
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
            // Only a layer's value that can be the item's is the item's value times a shape (design L16).
            if (shape.timesItem)
            {
                AddIssue(ItemDataIssueSeverity::Warning, field + "." + Keys::TimesItem,
                         "is only for a layer's value that can be the item's; left out");
                shape.timesItem = false;
            }
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
    Items::Json::KeySet known{Keys::Shape, Keys::TimesItem};
    if (const auto times = json.find(Keys::TimesItem); times != json.end())
    {
        if (!times->is_boolean())
        {
            AddFatal(field + "." + Keys::TimesItem, "must be true or false");
            valid = false;
        }
        else
        {
            shape.timesItem = times->get<bool>();
        }
    }
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
    const bool timed =
        shape.kind != ShapeKind::Scroll && shape.kind != ShapeKind::Random && shape.kind != ShapeKind::Turn;
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
    if (entry->is_object() && entry->size() == 1 && entry->contains(Keys::Brightness))
    {
        Number brightness;
        if (ReadNumber((*entry)[Keys::Brightness], lightField + "." + Keys::Brightness, brightness))
        {
            light = LayerLight{.brightness = brightness};
        }
        return;
    }
    if (entry->is_object())
    {
        const auto tint = entry->find(Keys::Tint);
        if (tint == entry->end() || entry->size() != 1)
        {
            AddFatal(lightField,
                     "must be [red, green, blue], {\"tint\": [red, green, blue]} or {\"brightness\": a number}");
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

void LookReader::ReadItemValues(const OrderedJson& marked, const std::string& field, ItemValues& values)
{
    if (!marked.is_object())
    {
        AddFatal(field, "must be an object of item values");
        return;
    }
    const OrderedJson json = TakeEachItem(marked, field, values.eachItem);
    ReadItemValueFields(json, field, values);
    if (values.IsEmpty())
    {
        AddIssue(ItemDataIssueSeverity::Warning, field, "sets no item value");
    }
    WarnAboutUnknownKeys(json,
                         {Keys::Opacity, Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU, Keys::ScrollV,
                          Keys::HiddenMesh, Keys::ScrollingMesh, Keys::Light},
                         field);
}

void LookReader::ReadItemValueFields(const OrderedJson& json, const std::string& field, ItemValues& values)
{
    ReadItemNumber(json, Keys::Opacity, field, values.opacity);
    // "all": every mesh glowing (the drawing reads a glow mesh below -1 so).
    if (const auto glow = json.find(Keys::GlowMesh);
        glow != json.end() && glow->is_string() && glow->get_ref<const std::string&>() == All)
    {
        values.glowMesh = -2;
    }
    else
    {
        ReadMeshOrNone(json, Keys::GlowMesh, field, values.glowMesh);
    }
    ReadItemNumber(json, Keys::GlowStrength, field, values.glowStrength);
    ReadItemNumber(json, Keys::ScrollU, field, values.scrollU);
    ReadItemNumber(json, Keys::ScrollV, field, values.scrollV);
    ReadMeshOrNone(json, Keys::HiddenMesh, field, values.hiddenMesh);
    ReadMeshOrNone(json, Keys::ScrollingMesh, field, values.scrollingMesh);
    ReadLight(json, field, values.light);
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
    if (shape.timesItem)
    {
        json[Keys::TimesItem] = true;
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

OrderedJson WriteColor(const std::array<Number, 3>& color)
{
    OrderedJson json = OrderedJson::array();
    for (const Number& channel : color)
    {
        json.push_back(WriteNumber(channel));
    }
    return json;
}

void WriteBonePlaces(const BonePlaces& places, OrderedJson& json)
{
    if (places.origin)
    {
        json[Keys::At] = Keys::Origin;
    }
    else if (!places.between.empty())
    {
        OrderedJson pairs = OrderedJson::array();
        for (const auto& [from, to] : places.between)
        {
            pairs.push_back(OrderedJson::array({from, to}));
        }
        json[Keys::Between] = std::move(pairs);
    }
    else
    {
        json[Keys::Bones] = places.bones;
    }
}

void WriteChance(const std::optional<Chance>& chance, OrderedJson& json)
{
    if (!chance)
    {
        return;
    }
    OrderedJson written = OrderedJson::object();
    written[Keys::OneIn] = chance->oneIn;
    if (chance->draw != ChanceDraw::Fps)
    {
        written[Keys::Draw] = NameOf(ChanceDrawNames, chance->draw);
    }
    if (chance->once)
    {
        written[Keys::Per] = PerOnce;
    }
    json[Keys::Chance] = std::move(written);
}

// A thing's kind and type, sub type and values that are not the defaults; its places go after, or to the group.
OrderedJson WriteBoneThing(const BoneThing& thing)
{
    OrderedJson json = OrderedJson::object();
    json[NameOf(BoneThingKeys, thing.kind)] = thing.type;
    if (thing.subType != 0)
    {
        json[Keys::SubType] = thing.subType;
    }
    if (thing.reversed)
    {
        json[Keys::Reversed] = true;
    }
    return json;
}

void WriteBoneThingValues(const BoneThing& thing, OrderedJson& json)
{
    if (thing.scale != Number{1.f})
    {
        json[Keys::Scale] = WriteNumber(thing.scale);
    }
    if (thing.color != std::array<Number, 3>{1.f, 1.f, 1.f})
    {
        json[Keys::Color] = WriteColor(thing.color);
    }
    if (thing.rotation != Number{0.f})
    {
        json[Keys::Rotation] = WriteNumber(thing.rotation);
    }
}

OrderedJson WriteOnBones(const OnBones& entry)
{
    if (!entry.together && entry.things.size() == 1)
    {
        const BoneThing& thing = entry.things.front();
        OrderedJson json = WriteBoneThing(thing);
        WriteBonePlaces(entry.places, json);
        WriteBoneThingValues(thing, json);
        WriteChance(entry.chance, json);
        return json;
    }
    OrderedJson group = OrderedJson::object();
    WriteBonePlaces(entry.places, group);
    WriteChance(entry.chance, group);
    OrderedJson things = OrderedJson::array();
    for (const BoneThing& thing : entry.things)
    {
        OrderedJson json = WriteBoneThing(thing);
        WriteBoneThingValues(thing, json);
        WriteChance(thing.chance, json);
        things.push_back(std::move(json));
    }
    OrderedJson json = OrderedJson::object();
    json[Keys::Together] = std::move(group);
    json[Keys::Things] = std::move(things);
    return json;
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
    if (const FromItem* item = std::get_if<FromItem>(&value))
    {
        OrderedJson json = OrderedJson::object();
        json[Keys::ItemTimes] = WriteFloat(item->times);
        return json;
    }
    return WriteFloat(std::get<float>(value));
}

void PutValue(OrderedJson& json, const char* key, const Value& value)
{
    // The item's value as it is is a layer's default.
    if (value != Value{FromItem{}})
    {
        json[key] = WriteValue(value);
    }
}

OrderedJson WriteMeshOrNone(int mesh)
{
    return mesh < 0 ? OrderedJson(None) : OrderedJson(mesh);
}

// A field each item sets: {"eachItem": name, "default": the look's value}, without a default when it has none.
// Wraps the fields each item sets, {"eachItem": name, "default": value}, and keeps the fields in `order`: a field
// the look leaves out is written at its place, not last.
void PutEachItem(OrderedJson& json, const EachItemFields& eachItem, std::initializer_list<const char*> order)
{
    if (eachItem.empty())
    {
        return;
    }
    for (const auto& [key, name] : eachItem)
    {
        OrderedJson marked = OrderedJson::object();
        marked[Keys::EachItem] = name;
        if (const auto fallback = json.find(key); fallback != json.end())
        {
            marked[Keys::Default] = *fallback;
        }
        json[key] = std::move(marked);
    }
    OrderedJson ordered = OrderedJson::object();
    for (const char* key : order)
    {
        if (const auto field = json.find(key); field != json.end())
        {
            ordered[key] = *field;
        }
    }
    for (const auto& [key, value] : json.items())
    {
        if (!ordered.contains(key))
        {
            ordered[key] = value;
        }
    }
    json = std::move(ordered);
}

OrderedJson WriteOnlyWhen(const OnlyWhen& onlyWhen)
{
    OrderedJson json = OrderedJson::object();
    if (onlyWhen.place)
    {
        json[Keys::Place] = NameOf(PlaceNames, *onlyWhen.place);
    }
    if (onlyWhen.level)
    {
        json[Keys::Level] = OrderedJson::array({onlyWhen.level->first, onlyWhen.level->second});
    }
    if (onlyWhen.hiddenMesh)
    {
        json[Keys::HiddenMesh] = WriteMeshOrNone(*onlyWhen.hiddenMesh);
    }
    if (onlyWhen.buff)
    {
        json[Keys::Buff] = NameOf(BuffNames, *onlyWhen.buff);
    }
    if (onlyWhen.withoutBuff)
    {
        json[Keys::WithoutBuff] = NameOf(BuffNames, *onlyWhen.withoutBuff);
    }
    return json;
}

OrderedJson WriteNotFor(const NotFor& notFor)
{
    OrderedJson json = OrderedJson::array();
    if (notFor.doppelganger)
    {
        json.push_back(NameOf(NotForNames, NotForDrawing::Doppelganger));
    }
    if (notFor.outsideMonsterHands)
    {
        json.push_back(NameOf(NotForNames, NotForDrawing::OutsideMonsterHands));
    }
    return json;
}

// [red, green, blue], or {"tint": [red, green, blue]}.
OrderedJson WriteLight(const LayerLight& light)
{
    if (light.brightness)
    {
        return OrderedJson{{Keys::Brightness, WriteNumber(*light.brightness)}};
    }
    OrderedJson color = OrderedJson::array();
    for (const Number& channel : light.color)
    {
        color.push_back(WriteNumber(channel));
    }
    return light.tint ? OrderedJson{{Keys::Tint, std::move(color)}} : color;
}

OrderedJson WriteLayer(const Layer& layer)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Kind] = NameOf(LayerKindNames, layer.kind);
    if (layer.kind == LayerKind::Reflection)
    {
        json[Keys::Motion] = NameOf(MotionNames, layer.motion);
        if (layer.where != Where::Over)
        {
            json[Keys::Where] = NameOf(WhereNames, layer.where);
        }
    }
    if (layer.blend != Blend::Normal)
    {
        json[Keys::Blend] = NameOf(BlendNames, layer.blend);
    }
    if (layer.meshes.none)
    {
        json[Keys::Meshes] = None;
    }
    else if (layer.meshes.evenHidden)
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
    if (!layer.onlyWhen.IsEmpty())
    {
        json[Keys::OnlyWhen] = WriteOnlyWhen(layer.onlyWhen);
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
        json[Keys::Light] = WriteLight(*layer.light);
    }
    if (layer.scrollingMesh)
    {
        json[Keys::ScrollingMesh] = *layer.scrollingMesh;
    }
    PutEachItem(json, layer.eachItem,
                {Keys::Kind, Keys::Motion, Keys::Where, Keys::Blend, Keys::Meshes, Keys::OnlyWhen, Keys::Texture,
                 Keys::Opacity, Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU, Keys::ScrollV, Keys::Light,
                 Keys::ScrollingMesh});
    return json;
}

OrderedJson WriteItemValueFields(const ItemValues& values)
{
    OrderedJson json = OrderedJson::object();
    if (values.opacity)
    {
        json[Keys::Opacity] = WriteNumber(*values.opacity);
    }
    if (values.glowMesh)
    {
        json[Keys::GlowMesh] = *values.glowMesh < -1 ? OrderedJson(All) : WriteMeshOrNone(*values.glowMesh);
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
    if (values.light)
    {
        json[Keys::Light] = WriteLight(*values.light);
    }
    return json;
}

OrderedJson WriteItemValues(const ItemValues& values)
{
    OrderedJson json = WriteItemValueFields(values);
    PutEachItem(json, values.eachItem,
                {Keys::Opacity, Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU, Keys::ScrollV, Keys::HiddenMesh,
                 Keys::ScrollingMesh, Keys::Light});
    return json;
}

OrderedJson WriteItemValuesBefore(const ItemValuesBefore& entry)
{
    OrderedJson json = OrderedJson::object();
    if (!entry.onlyWhen.IsEmpty())
    {
        json[Keys::OnlyWhen] = WriteOnlyWhen(entry.onlyWhen);
    }
    const OrderedJson fields = WriteItemValueFields(entry.values);
    for (const auto& [key, value] : fields.items())
    {
        json[key] = value;
    }
    if (entry.drawingOpacity)
    {
        json[Keys::DrawingOpacity] = WriteNumber(*entry.drawingOpacity);
    }
    if (entry.glowsLikeLevel)
    {
        json[Keys::GlowsLikeLevel] = *entry.glowsLikeLevel;
    }
    PutEachItem(json, entry.values.eachItem,
                {Keys::OnlyWhen, Keys::Opacity, Keys::GlowMesh, Keys::GlowStrength, Keys::ScrollU, Keys::ScrollV,
                 Keys::HiddenMesh, Keys::ScrollingMesh, Keys::DrawingOpacity, Keys::GlowsLikeLevel});
    return json;
}
} // namespace

std::string_view GetRenderStylesFileName()
{
    return "RenderStyles.json";
}

std::string_view GetLookFileName(LookKind kind)
{
    return kind == LookKind::ItemEffect ? "ItemEffects.json" : GetRenderStylesFileName();
}

void ReadRenderStylesFile(std::string_view text, const std::string& source, RenderStylesFile& file,
                          std::vector<Items::ItemDataIssue>& issues)
{
    ReadLookFile(text, source, LookKind::RenderStyle, file, issues);
}

void ReadLookFile(std::string_view text, const std::string& source, LookKind kind, LookFile& file,
                  std::vector<Items::ItemDataIssue>& issues)
{
    std::vector<LookDefinition>& looks = file.looks;
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, LookFileFormatVersion, root, issues))
    {
        return;
    }
    const auto kindField = root.find(Keys::Kind);
    if (kindField == root.end() || !kindField->is_string() ||
        kindField->get_ref<const std::string&>() != KindName(kind))
    {
        issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          Keys::Kind, std::string("must be \"") + KindName(kind) + "\""});
        return;
    }
    LookReader reader(source, kind, issues);
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

bool SetsEachItem(const LookDefinition& look)
{
    return !EachItemNames(look).empty();
}

std::set<std::string> EachItemNames(const LookDefinition& look)
{
    std::set<std::string> names;
    const auto add = [&names](const EachItemFields& eachItem)
    {
        for (const auto& [key, name] : eachItem)
        {
            names.insert(name);
        }
    };
    for (const Layer& layer : look.layers)
    {
        add(layer.eachItem);
    }
    add(look.itemValuesAfter.eachItem);
    for (const ItemValuesBefore& entry : look.itemValuesBefore)
    {
        add(entry.values.eachItem);
    }
    return names;
}

LookDefinition ResolveForItem(const LookDefinition& look, const std::map<std::string, std::string>& values,
                              const std::string& source, std::vector<Items::ItemDataIssue>& issues, LookKind kind)
{
    if (values.empty() || !SetsEachItem(look))
    {
        return look;
    }
    OrderedJson root = OrderedJson::parse(WriteLookFileJson(kind, {look}));
    OrderedJson& json = root[Keys::Looks][0];
    // A value the item sets takes the place of the look's {"eachItem": ..., "default": ...}; null leaves the field out.
    const auto put = [&values](OrderedJson& fields, const EachItemFields& eachItem)
    {
        for (const auto& [key, name] : eachItem)
        {
            if (const auto value = values.find(name); value != values.end())
            {
                OrderedJson parsed = OrderedJson::parse(value->second, nullptr, false);
                if (parsed.is_null())
                {
                    fields.erase(key);
                }
                else
                {
                    fields[key] = std::move(parsed);
                }
            }
        }
    };
    for (std::size_t i = 0; i < look.layers.size(); ++i)
    {
        put(json[Keys::Layers][i], look.layers[i].eachItem);
    }
    if (!look.itemValuesAfter.eachItem.empty())
    {
        put(json[Keys::ItemValuesAfter], look.itemValuesAfter.eachItem);
    }
    for (std::size_t i = 0; i < look.itemValuesBefore.size(); ++i)
    {
        put(json[Keys::ItemValuesBefore][i], look.itemValuesBefore[i].values.eachItem);
    }
    LookFile file;
    std::vector<Items::ItemDataIssue> read;
    ReadLookFile(root.dump(), source, kind, file, read);
    issues.insert(issues.end(), read.begin(), read.end());
    if (Items::HasFatal(read) || file.looks.size() != 1)
    {
        // A value that can't be one: the item draws with the look's own.
        return look;
    }
    return std::move(file.looks.front());
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
    return WriteLookFileJson(LookKind::RenderStyle, std::move(looks), folders);
}

std::string WriteLookFileJson(LookKind kind, std::vector<LookDefinition> looks,
                              std::span<const Colors::ColorFolder> folders)
{
    std::sort(looks.begin(), looks.end(),
              [](const LookDefinition& a, const LookDefinition& b) { return a.name < b.name; });
    OrderedJson list = OrderedJson::array();
    for (const LookDefinition& look : looks)
    {
        OrderedJson json = OrderedJson::object();
        json[Keys::Name] = look.name;
        if (kind == LookKind::ItemEffect)
        {
            if (!look.itemValuesBefore.empty())
            {
                OrderedJson before = OrderedJson::array();
                for (const ItemValuesBefore& entry : look.itemValuesBefore)
                {
                    before.push_back(WriteItemValuesBefore(entry));
                }
                json[Keys::ItemValuesBefore] = std::move(before);
            }
            if (look.objectLight)
            {
                OrderedJson light = OrderedJson::array();
                for (const Number& channel : *look.objectLight)
                {
                    light.push_back(WriteNumber(channel));
                }
                json[Keys::ObjectLight] = std::move(light);
            }
            if (!look.onBones.empty())
            {
                OrderedJson onBones = OrderedJson::array();
                for (const OnBones& entry : look.onBones)
                {
                    onBones.push_back(WriteOnBones(entry));
                }
                json[Keys::OnBones] = std::move(onBones);
            }
            if (look.animationSpeed)
            {
                json[Keys::AnimationSpeed] = WriteFloat(*look.animationSpeed);
            }
            list.push_back(std::move(json));
            continue;
        }
        if (!look.notFor.IsEmpty())
        {
            json[Keys::NotFor] = WriteNotFor(look.notFor);
        }
        if (look.drawsAlone)
        {
            json[Keys::DrawsAlone] = true;
        }
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
    root[Keys::Kind] = KindName(kind);
    if (!folders.empty())
    {
        const auto isLook = [&looks](std::string_view name)
        { return std::ranges::any_of(looks, [name](const LookDefinition& look) { return look.name == name; }); };
        root[Colors::LookFolderKeys.list] = Colors::WriteColorFolders(folders, isLook, Colors::LookFolderKeys);
    }
    root[Keys::Looks] = std::move(list);
    std::string text = root.dump(Items::Json::Indent);
    text = Colors::PutColorFoldersOnLines(text, Colors::LookFolderKeys);
    text = Items::Json::PutListsOnOneLine(text, Keys::NotFor);
    text = Items::Json::PutListEntriesOnOneLine(text, Keys::Layers);
    text = Items::Json::PutObjectsOnOneLine(text, Keys::ItemValuesAfter);
    text = Items::Json::PutListEntriesOnOneLine(text, Keys::ItemValuesBefore);
    text = Items::Json::PutListsOnOneLine(text, Keys::ObjectLight);
    text = Items::Json::PutListEntriesOnOneLine(text, Keys::OnBones);
    return text + "\n";
}

const char* MotionName(Motion motion)
{
    return NameOf(MotionNames, motion);
}

std::optional<Motion> MotionNamed(std::string_view name)
{
    return ValueOf(MotionNames, name);
}

std::string MotionNameList()
{
    return ListOf(MotionNames);
}
} // namespace Data::Looks
