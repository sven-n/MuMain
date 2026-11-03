#include "stdafx.h"

#include "MonsterModelLoader.h"

#include "Audio/DSPlaySound.h"
#include "Audio/SoundNumbers.h"
#include "Core/Text/Utf8.h"
#include "Data/DataHandler/LoadData.h"
#include "Data/DataHandler/ModelData/ModelFileDataHandler.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"
#include "Engine/Object/MonsterSlotSound.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Textures/ZzzTexture.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

namespace Data::Monsters::ModelLoader
{
namespace
{
// The defaults a model gets before its own values.
enum class Defaults
{
    MonsterRange,
    NpcRange,
    // A part model: only what its entry lists.
    None,
};

constexpr size_t TextureNameSize = sizeof(Texture_t::FileName);

// "Data/Monster/Monster01.bmd" -> "Data\Monster\Monster01.bmd".
std::wstring ToLoaderPath(const std::string& path)
{
    std::wstring loaderPath = Core::Text::FromUtf8(path.c_str());
    std::replace(loaderPath.begin(), loaderPath.end(), L'/', L'\\');
    return loaderPath;
}

// AccessModel's folder ("Data\Monster\") and file name without ".bmd".
bool OpenFile(int slot, const std::string& file)
{
    const std::wstring path = ToLoaderPath(file);
    const size_t split = path.find_last_of(L'\\');
    const std::wstring folder = path.substr(0, split + 1);
    const std::wstring name = path.substr(split + 1, path.size() - split - 1 - std::wstring(L".bmd").size());
    return gLoadData.AccessModel(slot, folder.c_str(), name.c_str());
}

std::vector<std::wstring> LoaderFolders(const std::vector<std::string>& folders)
{
    std::vector<std::wstring> loaderFolders;
    for (const std::string& folder : folders)
    {
        loaderFolders.push_back(ToLoaderPath(folder) + L'\\');
    }
    return loaderFolders;
}

void OpenTextures(int slot, const std::vector<std::string>& folders)
{
    if (::Models[slot].NumMeshs <= 0)
    {
        return;
    }
    gLoadData.OpenTexture(slot, LoaderFolders(folders));
}

// The texture folders of the map that is loaded.
const std::vector<std::string>& FoldersOnMap(const MonsterModelDefinition& model)
{
    const int map = gMapManager.GetServerMap();
    const int stage = gMapManager.GetServerStage();
    for (const MonsterModelMapFolders& entry : model.mapTextureFolders)
    {
        if (Data::Maps::Names(entry.maps, map, stage))
        {
            return entry.folders;
        }
    }
    return model.load.textureFolders;
}

void ApplyAnimations(BMD& model, const MonsterModelLoad& load, Defaults defaults)
{
    for (int animation = 0; animation < model.NumActions; ++animation)
    {
        Action_t& action = model.Actions[animation];
        if (defaults == Defaults::MonsterRange)
        {
            action.PlaySpeed = GetMonsterRangeDefaultSpeed(animation);
            action.Loop = IsHeldByDefaultInMonsterRange(animation);
        }
        else if (defaults == Defaults::NpcRange)
        {
            action.PlaySpeed = NpcRangeDefaultSpeed;
        }
    }
    for (const auto& [animation, speed] : load.speeds)
    {
        if (animation < model.NumActions)
        {
            model.Actions[animation].PlaySpeed = speed;
        }
    }
    if (load.holdLastFrame)
    {
        for (int animation = 0; animation < model.NumActions; ++animation)
        {
            model.Actions[animation].Loop = std::find(load.holdLastFrame->begin(), load.holdLastFrame->end(),
                                                      animation) != load.holdLastFrame->end();
        }
    }
}

void ApplyTextureNames(BMD& model, const std::vector<std::pair<int, std::string>>& names)
{
    for (const auto& [mesh, name] : names)
    {
        if (mesh < model.NumMeshs)
        {
            std::memset(model.Textures[mesh].FileName, 0, TextureNameSize);
            std::memcpy(model.Textures[mesh].FileName, name.data(), std::min(name.size(), TextureNameSize - 1));
        }
    }
}

std::optional<int> PartSlot(const MonsterModelPart& part)
{
    if (!part.slot.empty())
    {
        return FindMonsterModelSlot(part.slot);
    }
    return g_EffectTypeCatalogue.FindType(Effects::EffectKind::Effect, part.effect);
}

void OpenPart(const MonsterModelPart& part, int slot)
{
    OpenFile(slot, part.load.file);
    OpenTextures(slot, part.load.textureFolders);
    ApplyAnimations(::Models[slot], part.load, Defaults::None);
}

void OpenParts(const MonsterModelDefinition& model)
{
    for (const MonsterModelPart& part : model.parts)
    {
        const std::optional<int> slot = PartSlot(part);
        // Parts of a map are opened with the map.
        if (slot && part.maps.empty())
        {
            OpenPart(part, *slot);
        }
    }
}

void LoadSounds(const MonsterModelDefinition& model)
{
    for (const MonsterModelSound& sound : model.sounds)
    {
        LoadSound(sound);
    }
}

// The sound slot value of a sound the slot names: its number from
// SOUND_MONSTER on; -1 for no sound.
short SlotValue(const MonsterModelDefinition& model, const std::string& value)
{
    if (value.empty())
    {
        return -1;
    }
    const std::optional<int> number = FindSoundNumber(model, value);
    return number ? static_cast<short>(*number - SOUND_MONSTER) : -1;
}

// The entry's sound slots. Without slots in the entry, every slot has no
// sound: a slot keeps no 0, which is the Bull Fighter's first sound
// (SOUND_MONSTER), so a model without slots played it where it was loaded
// (P9).
void SetSoundSlots(BMD& target, const MonsterModelDefinition& model)
{
    std::fill(std::begin(target.Sounds), std::end(target.Sounds), Engine::Object::MonsterSlotSound::NoSound);
    if (!model.soundSlots)
    {
        return;
    }
    for (size_t i = 0; i < SoundSlotSetSize; ++i)
    {
        target.Sounds[i] = SlotValue(model, model.soundSlots->first[i]);
        target.Sounds[SoundSlotSetSize + i] =
            model.soundSlots->second ? SlotValue(model, (*model.soundSlots->second)[i]) : -1;
    }
}

void SetMeshValues(BMD& target, const MonsterModelDefinition& model)
{
    if (model.headBone)
    {
        target.BoneHead = *model.headBone;
    }
    if (model.streamMesh)
    {
        target.StreamMesh = static_cast<char>(*model.streamMesh);
    }
    for (const int mesh : model.noneBlendMeshes)
    {
        if (mesh < target.NumMeshs)
        {
            target.Meshs[mesh].NoneBlendMesh = true;
        }
    }
}

void LoadBitmaps(const MonsterModelDefinition& model)
{
    for (const MonsterModelBitmap& bitmap : model.bitmaps)
    {
        if (const std::optional<int> slot = FindMonsterModelBitmapSlot(bitmap.slot))
        {
            LoadBitmap(ToLoaderPath(bitmap.file).c_str(), static_cast<GLuint>(*slot),
                       bitmap.linearFilter ? GL_LINEAR : GL_NEAREST, bitmap.repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        }
    }
}

} // namespace

bool Open(int slot)
{
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(slot);
    if (model == nullptr)
    {
        return false;
    }
    const bool monsterRange = IsMonsterRangeSlot(slot);
    BMD& target = ::Models[slot];
    // A model whose file is missing has no body, and the game keeps running
    // (P1): AccessModel logs the file, and Data Checks lists it.
    OpenFile(slot, model->load.file);
    // A monster model without meshes gets nothing more, and its sound slots
    // stay without sounds; an NPC model still loads its sounds.
    if (monsterRange && target.NumMeshs == 0)
    {
        std::fill(std::begin(target.Sounds), std::end(target.Sounds), Engine::Object::MonsterSlotSound::NoSound);
        return true;
    }
    // A monster model the model file table names is drawn from its glTF binary (with the BMD's actions);
    // otherwise, or when the file is refused, from the BMD.
    const Data::Models::ModelFiles& modelFiles = Data::Models::GetModelFileTable();
    // The file may order its bones differently, so the head bone is found by the BMD bone's name.
    std::string headBoneName;
    if (model->headBone && *model->headBone >= 0 && *model->headBone < target.NumBones)
    {
        const char* name = target.Bones[*model->headBone].Name;
        headBoneName.assign(name, strnlen(name, sizeof(target.Bones[0].Name)));
    }
    const bool fromModelFile = monsterRange && Data::Models::ApplyModelFile(target, slot, modelFiles);
    if (fromModelFile)
    {
        Data::Models::OpenModelFileTextures(slot, LoaderFolders(FoldersOnMap(*model)), modelFiles);
    }
    else
    {
        ApplyTextureNames(target, model->textureNames);
        OpenTextures(slot, FoldersOnMap(*model));
    }
    OpenParts(*model);
    ApplyAnimations(target, model->load, monsterRange ? Defaults::MonsterRange : Defaults::NpcRange);
    SetMeshValues(target, *model);
    if (fromModelFile && !headBoneName.empty())
    {
        target.BoneHead = target.FindBoneOrRoot(headBoneName);
    }
    LoadSounds(*model);
    SetSoundSlots(target, *model);
    LoadBitmaps(*model);
    return true;
}

std::optional<int> FindSoundNumber(const MonsterModelDefinition& model, const std::string& value)
{
    for (const MonsterModelSound& sound : model.sounds)
    {
        if (SoundNameOf(sound) == value)
        {
            return SoundNumber(sound);
        }
    }
    return FindMonsterSoundId(value);
}

int SoundNumber(const MonsterModelSound& sound)
{
    if (!sound.id.empty())
    {
        if (const std::optional<int> id = FindMonsterSoundId(sound.id))
        {
            return *id;
        }
    }
    return static_cast<int>(Audio::SoundNumbers::ForFile(ToLoaderPath(sound.file)));
}

void LoadSound(const MonsterModelSound& sound)
{
    const ESound number = static_cast<ESound>(SoundNumber(sound));
    const std::wstring file = ToLoaderPath(sound.file);
    if (!IsSoundLoaded(number, file.c_str(), sound.channels, sound.spatial))
    {
        LoadWaveFile(number, file.c_str(), sound.channels, sound.spatial);
    }
}

bool IsOpen(int slot)
{
    if (::Models == nullptr || (!IsMonsterRangeSlot(slot) && !IsNpcRangeSlot(slot)))
    {
        return false;
    }
    const BMD& model = ::Models[slot];
    return model.NumActions > 0 || model.NumMeshs > 0;
}

#ifdef _EDITOR
std::vector<int> Reopen(int slot)
{
    std::vector<int> reopened;
    if (!IsOpen(slot))
    {
        return reopened;
    }
    BMD& target = ::Models[slot];
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(slot);
    if (model == nullptr)
    {
        target.Release();
        reopened.push_back(slot);
        return reopened;
    }
    Open(slot);
    reopened.push_back(slot);
    for (const MonsterModelPart& part : model->parts)
    {
        const std::optional<int> partSlot = PartSlot(part);
        if (!partSlot)
        {
            continue;
        }
        // The map's parts that the loaded map opened; Open opened the others.
        if (!part.maps.empty())
        {
            const bool onThisMap =
                Data::Maps::Names(part.maps, gMapManager.GetServerMap(), gMapManager.GetServerStage());
            const BMD& partModel = ::Models[*partSlot];
            if (!onThisMap || (partModel.NumActions <= 0 && partModel.NumMeshs <= 0))
            {
                continue;
            }
            OpenPart(part, *partSlot);
        }
        reopened.push_back(*partSlot);
    }
    return reopened;
}
#endif

void OpenMapParts(int map, int stage)
{
    for (const MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
    {
        for (const MonsterModelPart& part : model->parts)
        {
            const std::optional<int> slot = PartSlot(part);
            if (slot && Data::Maps::Names(part.maps, map, stage))
            {
                OpenPart(part, *slot);
            }
        }
    }
}
} // namespace Data::Monsters::ModelLoader
