#include "stdafx.h"

#include "MonsterModelsJson.h"

#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <optional>
#include <set>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::KeySet;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* Models = "models";
constexpr const char* Slot = "slot";
constexpr const char* Effect = "effect";
constexpr const char* File = "file";
constexpr const char* TextureFolders = "textureFolders";
constexpr const char* MapTextureFolders = "mapTextureFolders";
constexpr const char* Maps = "maps";
constexpr const char* Folders = "folders";
constexpr const char* TextureNames = "textureNames";
constexpr const char* Animations = "animations";
constexpr const char* HoldLastFrame = "holdLastFrame";
constexpr const char* HeadBone = "headBone";
constexpr const char* StreamMesh = "streamMesh";
constexpr const char* NoneBlendMeshes = "noneBlendMeshes";
constexpr const char* Cues = "cues";
constexpr const char* Name = "name";
constexpr const char* Animation = "animation";
constexpr const char* Frames = "frames";
constexpr const char* Bitmaps = "bitmaps";
constexpr const char* Filter = "filter";
constexpr const char* Wrap = "wrap";
constexpr const char* Parts = "parts";
constexpr const char* BoundingBox = "boundingBox";
constexpr const char* Min = "min";
constexpr const char* Max = "max";
constexpr const char* LinkBones = "linkBones";
// The sound fields that moved into the sound sets of Sounds.json (monsters
// design, Q-MN78); a file that has them is refused.
constexpr const char* OldSounds = "sounds";
constexpr const char* OldSoundSlots = "soundSlots";
constexpr const char* OldSoundEvents = "soundEvents";
constexpr const char* OldSoundEventMaps = "soundEventMaps";
// The names of the map lists before maps were the server's numbers (monsters
// design, D67); a file that has them is refused with the new name.
constexpr const char* OldWorldTextureFolders = "worldTextureFolders";
constexpr const char* OldWorlds = "worlds";
constexpr const char* OldSoundEventWorlds = "soundEventWorlds";
} // namespace Keys

constexpr const char* LinearFilter = "linear";
constexpr const char* NearestFilter = "nearest";
constexpr const char* RepeatWrap = "repeat";
constexpr const char* ClampWrap = "clamp";

// With the old keys: RefuseRenamedKeys and RefuseSoundFields report them, they
// are not unknown.
const KeySet ModelKeys{Keys::Slot,
                       Keys::File,
                       Keys::TextureFolders,
                       Keys::MapTextureFolders,
                       Keys::TextureNames,
                       Keys::Animations,
                       Keys::HoldLastFrame,
                       Keys::HeadBone,
                       Keys::StreamMesh,
                       Keys::NoneBlendMeshes,
                       Keys::Cues,
                       Keys::Bitmaps,
                       Keys::Parts,
                       Keys::BoundingBox,
                       Keys::LinkBones,
                       Keys::OldSounds,
                       Keys::OldSoundSlots,
                       Keys::OldSoundEvents,
                       Keys::OldSoundEventMaps,
                       Keys::OldWorldTextureFolders,
                       Keys::OldSoundEventWorlds};
const KeySet PartKeys{Keys::Slot,       Keys::Effect,        Keys::File, Keys::TextureFolders,
                      Keys::Animations, Keys::HoldLastFrame, Keys::Maps, Keys::OldWorlds};
const KeySet BitmapKeys{Keys::File, Keys::Slot, Keys::Filter, Keys::Wrap};
const KeySet MapFolderKeys{Keys::Maps, Keys::Folders};
const KeySet BoxKeys{Keys::Min, Keys::Max};
const KeySet CueKeys{Keys::Name, Keys::Animation, Keys::Frames};

constexpr std::string_view ModelFileExtension = ".bmd";
// A texture name fits Texture_t::FileName with its terminating zero.
constexpr size_t MaxTextureNameLength = 31;
constexpr int MaxMeshIndex = 49;
constexpr int MaxLinkBones = 2;

using ModelIssues = Json::EntryIssues;
using Json::RangeText;
using Json::ReadAnimationList;
using Json::ReadFrames;
using Json::ReadMaps;
using Json::ReadPath;
using Json::ReadText;
using Json::ReadWholeIn;
using Json::ReadWholeList;
using Json::WriteAnimationList;
using Json::WriteFloat;

bool EndsWithIgnoringCase(std::string_view text, std::string_view end)
{
    if (text.size() < end.size())
    {
        return false;
    }
    const std::string_view tail = text.substr(text.size() - end.size());
    return std::equal(tail.begin(), tail.end(), end.begin(),
                      [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == b; });
}

bool ReadModelFile(const OrderedJson& json, std::string& file, ModelIssues& issues)
{
    const auto value = json.find(Keys::File);
    if (value == json.end())
    {
        issues.Fatal(Keys::File, "missing");
        return false;
    }
    if (!ReadPath(*value, Keys::File, file, issues))
    {
        return false;
    }
    if (!EndsWithIgnoringCase(file, ModelFileExtension))
    {
        issues.Fatal(Keys::File, "must be a .bmd file");
        return false;
    }
    return true;
}

bool ReadFolders(const OrderedJson& json, const std::string& key, std::vector<std::string>& folders,
                 ModelIssues& issues)
{
    if (!json.is_array() || json.empty())
    {
        issues.Fatal(key, "must be a list of one folder or more");
        return false;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        std::string folder;
        if (!ReadPath(json[i], key + "[" + std::to_string(i) + "]", folder, issues))
        {
            return false;
        }
        folders.push_back(std::move(folder));
    }
    return true;
}

bool ReadSpeeds(const OrderedJson& json, std::vector<MonsterAnimationSpeed>& speeds, ModelIssues& issues)
{
    if (!json.is_object())
    {
        issues.Fatal(Keys::Animations, "must be an object of animation speeds");
        return false;
    }
    for (const auto& [name, value] : json.items())
    {
        const std::string key = std::string(Keys::Animations) + "." + name;
        const std::optional<int> animation = FindMonsterAnimation(name);
        if (!animation)
        {
            issues.Fatal(key, "is not an animation (stop1, stop2, walk, attack1, ... or a number from 12)");
            return false;
        }
        if (!value.is_number() || value.get<double>() < 0.0 || !std::isfinite(value.get<double>()))
        {
            issues.Fatal(key, "must be a number of 0 or more");
            return false;
        }
        speeds.emplace_back(*animation, static_cast<float>(value.get<double>()));
    }
    std::sort(speeds.begin(), speeds.end());
    return true;
}

// The fields a model and a part both have.
void ReadLoad(const OrderedJson& json, MonsterModelLoad& load, ModelIssues& issues)
{
    ReadModelFile(json, load.file, issues);
    const auto folders = json.find(Keys::TextureFolders);
    if (folders == json.end())
    {
        issues.Fatal(Keys::TextureFolders, "missing");
    }
    else
    {
        ReadFolders(*folders, Keys::TextureFolders, load.textureFolders, issues);
    }
    if (const auto speeds = json.find(Keys::Animations); speeds != json.end())
    {
        ReadSpeeds(*speeds, load.speeds, issues);
    }
    if (const auto hold = json.find(Keys::HoldLastFrame); hold != json.end())
    {
        std::vector<int> animations;
        if (ReadAnimationList(*hold, Keys::HoldLastFrame, animations, issues))
        {
            load.holdLastFrame = std::move(animations);
        }
    }
}

void ReadMapFolders(const OrderedJson& json, std::vector<MonsterModelMapFolders>& mapFolders, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(Keys::MapTextureFolders, "must be a list");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::MapTextureFolders) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object() || !entry.contains(Keys::Maps) || !entry.contains(Keys::Folders))
        {
            issues.Fatal(key, "must be an object with maps and folders");
            return;
        }
        MonsterModelMapFolders folders;
        if (!ReadMaps(entry[Keys::Maps], key + "." + Keys::Maps, folders.maps, issues) ||
            !ReadFolders(entry[Keys::Folders], key + "." + Keys::Folders, folders.folders, issues))
        {
            return;
        }
        Items::Json::WarnAboutUnknownKeys(entry, MapFolderKeys, key + ".",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message); });
        mapFolders.push_back(std::move(folders));
    }
}

void ReadTextureNames(const OrderedJson& json, std::vector<std::pair<int, std::string>>& names, ModelIssues& issues)
{
    if (!json.is_object())
    {
        issues.Fatal(Keys::TextureNames, "must be an object of texture names by mesh");
        return;
    }
    for (const auto& [mesh, value] : json.items())
    {
        const std::string key = std::string(Keys::TextureNames) + "." + mesh;
        int number = -1;
        const auto [end, error] = std::from_chars(mesh.data(), mesh.data() + mesh.size(), number);
        if (error != std::errc() || end != mesh.data() + mesh.size() || number < 0 || number > MaxMeshIndex)
        {
            issues.Fatal(key, "the mesh must be a number from 0 to " + std::to_string(MaxMeshIndex));
            return;
        }
        std::string name;
        if (!ReadText(value, key, name, issues))
        {
            return;
        }
        if (name.size() > MaxTextureNameLength)
        {
            issues.Fatal(key, "must have at most " + std::to_string(MaxTextureNameLength) + " characters");
            return;
        }
        names.emplace_back(number, std::move(name));
    }
    std::sort(names.begin(), names.end());
}

bool ReadChoice(const OrderedJson& entry, const char* key, const std::string& field, const char* yes, const char* no,
                bool& value, ModelIssues& issues)
{
    const auto choice = entry.find(key);
    if (choice == entry.end())
    {
        value = false;
        return true;
    }
    if (choice->is_string() && choice->get_ref<const std::string&>() == yes)
    {
        value = true;
        return true;
    }
    if (choice->is_string() && choice->get_ref<const std::string&>() == no)
    {
        value = false;
        return true;
    }
    issues.Fatal(field + "." + key, std::string("must be ") + yes + " or " + no);
    return false;
}

void ReadBitmaps(const OrderedJson& json, std::vector<MonsterModelBitmap>& bitmaps, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(Keys::Bitmaps, "must be a list of bitmaps");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::Bitmaps) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object() || !entry.contains(Keys::File) || !entry.contains(Keys::Slot))
        {
            issues.Fatal(key, "must be an object with file and slot");
            return;
        }
        MonsterModelBitmap bitmap;
        if (!ReadPath(entry[Keys::File], key + "." + Keys::File, bitmap.file, issues) ||
            !ReadText(entry[Keys::Slot], key + "." + Keys::Slot, bitmap.slot, issues) ||
            !ReadChoice(entry, Keys::Filter, key, LinearFilter, NearestFilter, bitmap.linearFilter, issues) ||
            !ReadChoice(entry, Keys::Wrap, key, RepeatWrap, ClampWrap, bitmap.repeat, issues))
        {
            return;
        }
        Items::Json::WarnAboutUnknownKeys(entry, BitmapKeys, key + ".",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message); });
        bitmaps.push_back(std::move(bitmap));
    }
}

// A key renamed when maps became the server's numbers is fatal with its new
// name: as an unknown key it would only warn, and its maps would be lost (a
// part without maps opens with its model on every map).
void RefuseRenamedKeys(const OrderedJson& json, std::initializer_list<std::pair<const char*, const char*>> renamed,
                       const std::string& prefix, ModelIssues& issues)
{
    for (const auto& [oldKey, newKey] : renamed)
    {
        if (json.contains(oldKey))
        {
            issues.Fatal(prefix + oldKey,
                         std::string("was renamed to ") + newKey + ", with the map numbers the server sends");
        }
    }
}

void ReadParts(const OrderedJson& json, std::vector<MonsterModelPart>& parts, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(Keys::Parts, "must be a list of part models");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::Parts) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object())
        {
            issues.Fatal(key, "must be an object");
            return;
        }
        MonsterModelPart part;
        const bool hasSlot = entry.contains(Keys::Slot);
        const bool hasEffect = entry.contains(Keys::Effect);
        if (hasSlot == hasEffect)
        {
            issues.Fatal(key, "must have a slot or an effect, not both");
            return;
        }
        if ((hasSlot && !ReadText(entry[Keys::Slot], key + "." + Keys::Slot, part.slot, issues)) ||
            (hasEffect && !ReadText(entry[Keys::Effect], key + "." + Keys::Effect, part.effect, issues)))
        {
            return;
        }
        ReadLoad(entry, part.load, issues);
        RefuseRenamedKeys(entry, {{Keys::OldWorlds, Keys::Maps}}, key + ".", issues);
        if (const auto maps = entry.find(Keys::Maps); maps != entry.end())
        {
            ReadMaps(*maps, key + "." + Keys::Maps, part.maps, issues);
        }
        Items::Json::WarnAboutUnknownKeys(entry, PartKeys, key + ".",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message); });
        parts.push_back(std::move(part));
    }
}

bool ReadPoint(const OrderedJson& json, const std::string& key, std::array<float, 3>& point, ModelIssues& issues)
{
    if (!json.is_array() || json.size() != point.size() ||
        !std::all_of(json.begin(), json.end(), [](const OrderedJson& value) { return value.is_number(); }))
    {
        issues.Fatal(key, "must be a list of three numbers");
        return false;
    }
    for (size_t i = 0; i < point.size(); ++i)
    {
        point[i] = static_cast<float>(json[i].get<double>());
    }
    return true;
}

void ReadBox(const OrderedJson& json, MonsterModelBody& body, ModelIssues& issues)
{
    if (!json.is_object())
    {
        issues.Fatal(Keys::BoundingBox, "must be an object with min and max");
        return;
    }
    const std::string prefix = std::string(Keys::BoundingBox) + ".";
    std::array<float, 3> point{};
    if (const auto min = json.find(Keys::Min); min != json.end() && ReadPoint(*min, prefix + Keys::Min, point, issues))
    {
        body.boxMin = point;
    }
    if (const auto max = json.find(Keys::Max); max != json.end() && ReadPoint(*max, prefix + Keys::Max, point, issues))
    {
        body.boxMax = point;
    }
    Items::Json::WarnAboutUnknownKeys(json, BoxKeys, prefix, [&](const std::string& unknown, const std::string& message)
                                      { issues.Warning(unknown, message); });
}

void ReadLinkBones(const OrderedJson& json, MonsterModelBody& body, ModelIssues& issues)
{
    if (!json.is_array() || json.size() > MaxLinkBones)
    {
        issues.Fatal(Keys::LinkBones, "must be a list of at most two bones");
        return;
    }
    std::vector<int> linkBones;
    // Both hands may hang on one bone.
    constexpr bool RepeatsAllowed = true;
    if (ReadWholeList(json, Keys::LinkBones, 0, std::numeric_limits<unsigned char>::max(), linkBones, issues,
                      RepeatsAllowed))
    {
        body.linkBones = std::move(linkBones);
    }
}

void ReadOptionalWhole(const OrderedJson& json, const char* key, long long low, long long high,
                       std::optional<int>& value, ModelIssues& issues)
{
    const auto field = json.find(key);
    if (field == json.end())
    {
        return;
    }
    const std::optional<int> number = ReadWholeIn(*field, low, high);
    if (!number)
    {
        issues.Fatal(key, RangeText(low, high));
        return;
    }
    value = number;
}

// Format 2 has no sounds: they are in the sound sets of Sounds.json, which
// the monsters name (monsters design, Q-MN78).
void RefuseSoundFields(const OrderedJson& json, ModelIssues& issues)
{
    for (const char* key : {Keys::OldSounds, Keys::OldSoundSlots, Keys::OldSoundEvents, Keys::OldSoundEventMaps,
                            Keys::OldSoundEventWorlds})
    {
        if (json.contains(key))
        {
            issues.Fatal(key, "is gone since format 2: sounds are in the sound sets of Sounds.json, which the "
                              "monsters name (soundSet)");
        }
    }
}

// The points of the model's animations that sound events play on (Q-MN79).
void ReadCues(const OrderedJson& json, std::vector<MonsterModelCue>& cues, ModelIssues& issues)
{
    if (!json.is_array())
    {
        issues.Fatal(Keys::Cues, "must be a list of cues");
        return;
    }
    for (size_t i = 0; i < json.size(); ++i)
    {
        const std::string key = std::string(Keys::Cues) + "[" + std::to_string(i) + "]";
        const OrderedJson& entry = json[i];
        if (!entry.is_object() || !entry.contains(Keys::Name) || !entry.contains(Keys::Animation) ||
            !entry.contains(Keys::Frames))
        {
            issues.Fatal(key, "must be an object with name, animation and frames");
            return;
        }
        MonsterModelCue cue;
        if (!ReadText(entry[Keys::Name], key + "." + Keys::Name, cue.name, issues))
        {
            return;
        }
        const OrderedJson& animation = entry[Keys::Animation];
        const std::optional<int> found =
            animation.is_string() ? FindMonsterAnimation(animation.get<std::string>()) : std::nullopt;
        if (!found)
        {
            issues.Fatal(key + "." + Keys::Animation, "is not an animation");
            return;
        }
        cue.animation = *found;
        if (!ReadFrames(entry[Keys::Frames], key + "." + Keys::Frames, cue.frames, issues))
        {
            return;
        }
        if (std::any_of(cues.begin(), cues.end(), [&](const MonsterModelCue& other) { return other.name == cue.name; }))
        {
            issues.Fatal(key + "." + Keys::Name, "\"" + cue.name + "\" is the name of another cue of the model");
            return;
        }
        issues.WarnAboutUnknownKeys(entry, CueKeys, key + ".");
        cues.push_back(std::move(cue));
    }
}

void ReadModelFields(const OrderedJson& json, MonsterModelDefinition& model, ModelIssues& issues)
{
    ReadLoad(json, model.load, issues);
    RefuseRenamedKeys(json, {{Keys::OldWorldTextureFolders, Keys::MapTextureFolders}}, "", issues);
    if (const auto folders = json.find(Keys::MapTextureFolders); folders != json.end())
    {
        ReadMapFolders(*folders, model.mapTextureFolders, issues);
    }
    if (const auto names = json.find(Keys::TextureNames); names != json.end())
    {
        ReadTextureNames(*names, model.textureNames, issues);
    }
    ReadOptionalWhole(json, Keys::HeadBone, -1, std::numeric_limits<int>::max(), model.headBone, issues);
    ReadOptionalWhole(json, Keys::StreamMesh, std::numeric_limits<char>::min(), std::numeric_limits<char>::max(),
                      model.streamMesh, issues);
    if (const auto meshes = json.find(Keys::NoneBlendMeshes); meshes != json.end())
    {
        ReadWholeList(*meshes, Keys::NoneBlendMeshes, 0, MaxMeshIndex, model.noneBlendMeshes, issues);
    }
    RefuseSoundFields(json, issues);
    if (const auto cues = json.find(Keys::Cues); cues != json.end())
    {
        ReadCues(*cues, model.cues, issues);
    }
    if (const auto bitmaps = json.find(Keys::Bitmaps); bitmaps != json.end())
    {
        ReadBitmaps(*bitmaps, model.bitmaps, issues);
    }
    if (const auto parts = json.find(Keys::Parts); parts != json.end())
    {
        ReadParts(*parts, model.parts, issues);
    }
    if (const auto box = json.find(Keys::BoundingBox); box != json.end())
    {
        ReadBox(*box, model.body, issues);
    }
    if (const auto bones = json.find(Keys::LinkBones); bones != json.end())
    {
        ReadLinkBones(*bones, model.body, issues);
    }
}

std::optional<MonsterModelDefinition> ReadModel(const OrderedJson& json, size_t index, const std::string& source,
                                                std::vector<ItemDataIssue>& issues)
{
    const std::string entry = std::string(Keys::Models) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, entry,
                          "must be an object"});
        return std::nullopt;
    }
    const auto slot = json.find(Keys::Slot);
    if (slot == json.end() || !slot->is_string() || !FindMonsterModelSlot(slot->get<std::string>()))
    {
        issues.push_back({ItemDataIssueSeverity::Fatal, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          entry + "." + Keys::Slot,
                          "missing or not a monster model type (MONSTER_MODEL_...) or an NPC model slot"});
        return std::nullopt;
    }
    MonsterModelDefinition model;
    model.slot = slot->get<std::string>();
    ModelIssues modelIssues(source, "model " + model.slot, issues);
    ReadModelFields(json, model, modelIssues);
    Items::Json::WarnAboutUnknownKeys(json, ModelKeys, "", [&](const std::string& unknown, const std::string& message)
                                      { modelIssues.Warning(unknown, message); });
    if (!modelIssues.IsValid())
    {
        return std::nullopt;
    }
    return model;
}

// ---------------------------------------------------------------- writing

OrderedJson WriteSpeeds(const std::vector<MonsterAnimationSpeed>& speeds)
{
    OrderedJson json = OrderedJson::object();
    for (const auto& [animation, speed] : speeds)
    {
        json[GetMonsterAnimationName(animation)] = WriteFloat(speed);
    }
    return json;
}

void WriteLoad(const MonsterModelLoad& load, OrderedJson& json)
{
    json[Keys::File] = load.file;
    json[Keys::TextureFolders] = load.textureFolders;
}

void WriteAnimations(const MonsterModelLoad& load, OrderedJson& json)
{
    if (!load.speeds.empty())
    {
        json[Keys::Animations] = WriteSpeeds(load.speeds);
    }
    if (load.holdLastFrame)
    {
        json[Keys::HoldLastFrame] = WriteAnimationList(*load.holdLastFrame);
    }
}

OrderedJson WriteBitmap(const MonsterModelBitmap& bitmap)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::File] = bitmap.file;
    json[Keys::Slot] = bitmap.slot;
    json[Keys::Filter] = bitmap.linearFilter ? LinearFilter : NearestFilter;
    json[Keys::Wrap] = bitmap.repeat ? RepeatWrap : ClampWrap;
    return json;
}

OrderedJson WritePart(const MonsterModelPart& part)
{
    OrderedJson json = OrderedJson::object();
    if (!part.slot.empty())
    {
        json[Keys::Slot] = part.slot;
    }
    else
    {
        json[Keys::Effect] = part.effect;
    }
    WriteLoad(part.load, json);
    WriteAnimations(part.load, json);
    if (!part.maps.empty())
    {
        json[Keys::Maps] = Data::Maps::WriteMapRefs(part.maps);
    }
    return json;
}

OrderedJson WritePoint(const std::array<float, 3>& point)
{
    OrderedJson json = OrderedJson::array();
    for (const float value : point)
    {
        json.push_back(WriteFloat(value));
    }
    return json;
}

void WriteBody(const MonsterModelBody& body, OrderedJson& json)
{
    if (body.boxMin || body.boxMax)
    {
        OrderedJson box = OrderedJson::object();
        if (body.boxMin)
        {
            box[Keys::Min] = WritePoint(*body.boxMin);
        }
        if (body.boxMax)
        {
            box[Keys::Max] = WritePoint(*body.boxMax);
        }
        json[Keys::BoundingBox] = std::move(box);
    }
    if (body.linkBones)
    {
        json[Keys::LinkBones] = *body.linkBones;
    }
}

OrderedJson WriteCues(const std::vector<MonsterModelCue>& cues)
{
    OrderedJson json = OrderedJson::array();
    for (const MonsterModelCue& cue : cues)
    {
        OrderedJson entry = OrderedJson::object();
        entry[Keys::Name] = cue.name;
        entry[Keys::Animation] = GetMonsterAnimationName(cue.animation);
        entry[Keys::Frames] = Json::WriteFrames(cue.frames);
        json.push_back(std::move(entry));
    }
    return json;
}

OrderedJson WriteModel(const MonsterModelDefinition& model)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Slot] = model.slot;
    WriteLoad(model.load, json);
    if (!model.mapTextureFolders.empty())
    {
        OrderedJson folders = OrderedJson::array();
        for (const MonsterModelMapFolders& entry : model.mapTextureFolders)
        {
            OrderedJson folder = OrderedJson::object();
            folder[Keys::Maps] = Data::Maps::WriteMapRefs(entry.maps);
            folder[Keys::Folders] = entry.folders;
            folders.push_back(std::move(folder));
        }
        json[Keys::MapTextureFolders] = std::move(folders);
    }
    if (!model.textureNames.empty())
    {
        OrderedJson names = OrderedJson::object();
        for (const auto& [mesh, name] : model.textureNames)
        {
            names[std::to_string(mesh)] = name;
        }
        json[Keys::TextureNames] = std::move(names);
    }
    WriteAnimations(model.load, json);
    if (model.headBone)
    {
        json[Keys::HeadBone] = *model.headBone;
    }
    if (model.streamMesh)
    {
        json[Keys::StreamMesh] = *model.streamMesh;
    }
    if (!model.noneBlendMeshes.empty())
    {
        json[Keys::NoneBlendMeshes] = model.noneBlendMeshes;
    }
    if (!model.cues.empty())
    {
        json[Keys::Cues] = WriteCues(model.cues);
    }
    if (!model.bitmaps.empty())
    {
        OrderedJson bitmaps = OrderedJson::array();
        for (const MonsterModelBitmap& bitmap : model.bitmaps)
        {
            bitmaps.push_back(WriteBitmap(bitmap));
        }
        json[Keys::Bitmaps] = std::move(bitmaps);
    }
    if (!model.parts.empty())
    {
        OrderedJson parts = OrderedJson::array();
        for (const MonsterModelPart& part : model.parts)
        {
            parts.push_back(WritePart(part));
        }
        json[Keys::Parts] = std::move(parts);
    }
    WriteBody(model.body, json);
    return json;
}

// Short lists and objects on one line, each cue and bitmap on its own line.
std::string FormatText(std::string text)
{
    using namespace Items::Json;
    for (const char* key :
         {Keys::TextureFolders, Keys::HoldLastFrame, Keys::NoneBlendMeshes, Keys::Maps, Keys::Folders, Keys::LinkBones})
    {
        text = PutListsOnOneLine(text, key);
    }
    for (const char* key : {Keys::Animations, Keys::TextureNames, Keys::BoundingBox})
    {
        text = PutObjectsOnOneLine(text, key);
    }
    for (const char* key : {Keys::Cues, Keys::Bitmaps, Keys::MapTextureFolders})
    {
        text = PutListEntriesOnOneLine(text, key);
    }
    return text;
}

int SortSlot(const MonsterModelDefinition& model)
{
    return FindMonsterModelSlot(model.slot).value_or(std::numeric_limits<int>::max());
}
} // namespace

std::string MonsterModelField(std::string_view slot, std::string_view key)
{
    return "model " + std::string(slot) + " " + std::string(key);
}

void ReadMonsterModelsJson(std::string_view text, const std::string& source,
                           std::vector<MonsterModelDefinition>& models, std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, MonsterModelsFormatVersion, root, issues))
    {
        return;
    }
    const auto addFileIssue = [&](ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    { issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message}); };
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::Models}, "",
                                      [&](const std::string& unknown, const std::string& message)
                                      { addFileIssue(ItemDataIssueSeverity::Warning, unknown, message); });
    const auto entries = root.find(Keys::Models);
    if (entries == root.end() || !entries->is_array())
    {
        addFileIssue(ItemDataIssueSeverity::Fatal, Keys::Models, "missing or not a list of models");
        return;
    }
    std::set<int> slots;
    for (size_t index = 0; index < entries->size(); ++index)
    {
        std::optional<MonsterModelDefinition> model = ReadModel((*entries)[index], index, source, issues);
        if (!model)
        {
            continue;
        }
        if (!slots.insert(*FindMonsterModelSlot(model->slot)).second)
        {
            addFileIssue(ItemDataIssueSeverity::Fatal, MonsterModelField(model->slot, Keys::Slot),
                         "is in the list more than once");
            continue;
        }
        models.push_back(std::move(*model));
    }
}

std::string WriteMonsterModelsJson(std::span<const MonsterModelDefinition> models)
{
    std::vector<const MonsterModelDefinition*> sorted;
    sorted.reserve(models.size());
    for (const MonsterModelDefinition& model : models)
    {
        sorted.push_back(&model);
    }
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const MonsterModelDefinition* left, const MonsterModelDefinition* right)
                     { return SortSlot(*left) < SortSlot(*right); });

    OrderedJson entries = OrderedJson::array();
    for (const MonsterModelDefinition* model : sorted)
    {
        entries.push_back(WriteModel(*model));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = MonsterModelsFormatVersion;
    root[Keys::Models] = std::move(entries);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return FormatText(text) + "\n";
}
} // namespace Data::Monsters
