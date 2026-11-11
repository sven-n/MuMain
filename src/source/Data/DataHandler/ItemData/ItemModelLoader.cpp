#include "stdafx.h"

#include "ItemModelLoader.h"
#include "ItemModelProblem.h"

#include "Core/Globals/_enum.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/LoadData.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelGlowJson.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/PartModelSlots.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Data::Items::ModelLoader
{
namespace
{
// The model loader takes Windows paths; the data uses '/' between folders.
constexpr wchar_t DataSeparator = L'/';
constexpr wchar_t LoaderSeparator = L'\\';
// CLoadData::AccessModel appends it to the file name.
constexpr std::wstring_view ModelFileExtension = L".bmd";
// How many errors the player sees; all of them go to the log.
constexpr size_t MaxErrorsInMessage = 10;
constexpr const char* LoggerName = "data";

// The problems of OpenModels and OpenTextures, until TakeProblemMessage.
std::vector<ItemModelProblem> g_problems;

using Data::Paths::ToLoaderPath;

std::string ToDataPath(const std::wstring& path)
{
    std::string result = Core::Text::ToUtf8(path.c_str());
    std::replace(result.begin(), result.end(), static_cast<char>(LoaderSeparator), static_cast<char>(DataSeparator));
    return result;
}

ItemModelProblem MakeProblem(ItemModelProblemType type, const ItemModelDefinition& model)
{
    ItemModelProblem problem;
    problem.type = type;
    problem.group = model.group;
    problem.number = model.number;
    problem.modelFile = model.file;
    problem.sharedModel = model.model;
    return problem;
}

// Logs the problem right away, so it is in the log even when the game ends
// before the problems are shown, and keeps it for the message.
void AddProblem(ItemModelProblem problem)
{
    const auto logger = mu::log::Get(LoggerName);
    if (problem.IsError())
    {
        MU_LOG_ERROR(logger, "Item model {}", problem.ToLogString());
    }
    else
    {
        MU_LOG_WARN(logger, "Item model {}", problem.ToLogString());
    }
    g_problems.push_back(std::move(problem));
}

void MarkNoneBlendMeshes(int itemType, const ItemModelDefinition& model)
{
    BMD& bmd = Models[ToModelSlot(itemType)];
    for (const int mesh : model.noneBlendMeshes)
    {
        if (mesh >= bmd.NumMeshs)
        {
            ItemModelProblem problem = MakeProblem(ItemModelProblemType::NoneBlendMeshMissing, model);
            problem.mesh = mesh;
            problem.meshCount = bmd.NumMeshs;
            AddProblem(std::move(problem));
            continue;
        }
        bmd.Meshs[mesh].NoneBlendMesh = true;
    }
}

void CheckGlowMeshes(int itemType, const ItemModelDefinition& model)
{
    const int meshCount = Models[ToModelSlot(itemType)].NumMeshs;
    GlowJson::ForEachMesh(model.glow,
                          [&](const std::string& field, int mesh)
                          {
                              if (mesh < meshCount)
                              {
                                  return;
                              }
                              ItemModelProblem problem = MakeProblem(ItemModelProblemType::GlowMeshMissing, model);
                              problem.field = field;
                              problem.mesh = mesh;
                              problem.meshCount = meshCount;
                              AddProblem(std::move(problem));
                          });
}

void CheckLookName(const ItemModelDefinition& model, const std::string& name, LookExists exists,
                   ItemModelProblemType unknown)
{
    if (name.empty() || exists(name))
    {
        return;
    }
    ItemModelProblem problem = MakeProblem(unknown, model);
    problem.name = name;
    AddProblem(std::move(problem));
}

// The checks of every item, also of the items that use the data of a shared
// model that another item opened.
void CheckModel(int itemType, const ItemModelDefinition& model, const LookNames& lookNames)
{
    CheckGlowMeshes(itemType, model);
    CheckLookName(model, model.renderStyle, lookNames.renderStyle, ItemModelProblemType::RenderStyleUnknown);
    CheckLookName(model, model.itemEffect, lookNames.itemEffect, ItemModelProblemType::ItemEffectUnknown);
}

// The material values of the item's slot: every slot keeps its own, also one
// that uses the data of a shared model.
void ApplyMaterial(int itemType, const ItemModelDefinition& model)
{
    Models[ToModelSlot(itemType)].SpecularStrength = static_cast<float>(model.material.specular);
}

// Opens the .bmd file into the slot; whether it was opened.
bool AccessModelFile(int modelType, const std::string& file)
{
    const std::wstring path = ToLoaderPath(file);
    const size_t nameStart = path.find_last_of(LoaderSeparator) + 1; // 0 when there is no folder
    const std::wstring folder = path.substr(0, nameStart);
    const std::wstring name = path.substr(nameStart, path.size() - nameStart - ModelFileExtension.size());
    return gLoadData.AccessModel(modelType, folder.c_str(), name.c_str());
}

// The texture folders as the loader takes them.
std::vector<std::wstring> LoaderFolders(const std::vector<std::string>& textureFolders)
{
    std::vector<std::wstring> folders;
    folders.reserve(textureFolders.size());
    for (const std::string& textureFolder : textureFolders)
    {
        folders.push_back(ToLoaderPath(textureFolder) + LoaderSeparator);
    }
    return folders;
}

// Whether the file was opened.
bool OpenModel(int itemType, const ItemModelDefinition& model, const LookNames& lookNames)
{
    if (!AccessModelFile(ToModelSlot(itemType), model.file))
    {
        AddProblem(MakeProblem(ItemModelProblemType::ModelFileMissing, model));
        return false;
    }
    MarkNoneBlendMeshes(itemType, model);
    ApplyMaterial(itemType, model);
    CheckModel(itemType, model, lookNames);
    return true;
}

// The item type whose slot opened the file of a shared model.
constexpr int NotOpened = -1;

// The first item of a shared model opens its file; the others use the data
// of that slot.
void OpenSharedModel(int itemType, const ItemModelDefinition& model, const LookNames& lookNames,
                     std::map<std::string, int, std::less<>>& openedBy)
{
    const auto [opened, isFirst] = openedBy.try_emplace(model.model, NotOpened);
    if (isFirst)
    {
        if (OpenModel(itemType, model, lookNames))
        {
            opened->second = itemType;
        }
        return;
    }
    if (opened->second == NotOpened)
    {
        AddProblem(MakeProblem(ItemModelProblemType::ModelFileMissing, model));
        return;
    }
    gLoadData.ShareModel(ToModelSlot(itemType), ToModelSlot(opened->second));
    ApplyMaterial(itemType, model);
    CheckModel(itemType, model, lookNames);
}

ItemModelProblemType GetTextureProblemType(const TextureProblem& textureProblem)
{
    if (textureProblem.unsupportedType)
    {
        return ItemModelProblemType::TextureTypeUnsupported;
    }
    return textureProblem.usedInstead.empty() ? ItemModelProblemType::TextureMissing
                                              : ItemModelProblemType::TextureOutsideFolders;
}

void AddTextureProblem(const ItemModelDefinition& model, const TextureProblem& textureProblem)
{
    ItemModelProblem problem = MakeProblem(GetTextureProblemType(textureProblem), model);
    problem.mesh = textureProblem.mesh;
    problem.texture = Core::Text::ToUtf8(textureProblem.fileName.c_str());
    problem.searchedFolders = model.textureFolders;
    problem.usedInstead = ToDataPath(textureProblem.usedInstead);
    AddProblem(std::move(problem));
}

void OpenModelTextures(int itemType, const ItemModelDefinition& model)
{
    std::vector<TextureProblem> textureProblems;
    gLoadData.OpenTexture(ToModelSlot(itemType), LoaderFolders(model.textureFolders), textureProblems);
    for (const TextureProblem& textureProblem : textureProblems)
    {
        AddTextureProblem(model, textureProblem);
    }
}

// Calls open(itemType, model) for every item model, in item type order.
template <typename TOpen> void ForEachModel(TOpen&& open)
{
    const std::span<const ItemModelDefinition> models = g_ItemModelDatabase.GetAllSlots();
    for (int itemType = 0; itemType < static_cast<int>(models.size()); ++itemType)
    {
        if (models[itemType].Exists())
        {
            open(itemType, models[itemType]);
        }
    }
}
} // namespace

void OpenModels(const LookNames& lookNames)
{
    std::map<std::string, int, std::less<>> sharedModelsOpenedBy;
    ForEachModel(
        [&](int itemType, const ItemModelDefinition& model)
        {
            if (model.model.empty())
            {
                OpenModel(itemType, model, lookNames);
            }
            else
            {
                OpenSharedModel(itemType, model, lookNames, sharedModelsOpenedBy);
            }
        });
}

void OpenTextures()
{
    ForEachModel(
        [](int itemType, const ItemModelDefinition& model)
        {
            // The textures of a shared model are loaded with the slot that opened it.
            if (!Models[ToModelSlot(itemType)].SharesData())
            {
                OpenModelTextures(itemType, model);
            }
        });
}

void SwapOpenedModels(int firstItemType, int secondItemType)
{
    if (Models == nullptr || !IsValidItemType(firstItemType) || !IsValidItemType(secondItemType))
    {
        return;
    }

    const int firstSlot = ToModelSlot(firstItemType);
    const int secondSlot = ToModelSlot(secondItemType);
    gLoadData.SwapModels(firstSlot, secondSlot);
    // The material values are the model's (ApplyMaterial), so they move with it.
    std::swap(Models[firstSlot].SpecularStrength, Models[secondSlot].SpecularStrength);
}

std::vector<ItemModelProblem> ReopenModels(std::span<const int> itemTypes, const LookNames& lookNames)
{
    if (Models == nullptr)
    {
        return {};
    }

    const size_t problemsBefore = g_problems.size();
    std::map<std::string, int, std::less<>> sharedModelsOpenedBy;
    for (const int itemType : itemTypes)
    {
        const ItemModelDefinition* model = g_ItemModelDatabase.Find(itemType);
        if (model == nullptr)
        {
            if (IsValidItemType(itemType))
            {
                Models[ToModelSlot(itemType)].Release();
            }
            continue;
        }
        const bool opens = model->model.empty() || !sharedModelsOpenedBy.contains(model->model);
        if (model->model.empty())
        {
            OpenModel(itemType, *model, lookNames);
        }
        else
        {
            OpenSharedModel(itemType, *model, lookNames, sharedModelsOpenedBy);
            // A shared model whose file did not open leaves each of its items
            // without a model, not with the one its slot had before.
            if (sharedModelsOpenedBy.find(model->model)->second == NotOpened)
            {
                Models[ToModelSlot(itemType)].Release();
            }
        }
        if (opens && Models[ToModelSlot(itemType)].NumMeshs > 0)
        {
            OpenModelTextures(itemType, *model);
        }
    }

    std::vector<ItemModelProblem> problems(std::make_move_iterator(g_problems.begin() + problemsBefore),
                                           std::make_move_iterator(g_problems.end()));
    g_problems.resize(problemsBefore);
    return problems;
}

void OpenPartModel(int modelType, const LookNames& lookNames)
{
    if (Models == nullptr)
    {
        return;
    }
    const auto logger = mu::log::Get(LoggerName);
    const std::optional<std::string_view> name = PartModelNameOf(modelType);
    const PartModel* part = name ? g_ItemModelDatabase.FindPartModel(*name) : nullptr;
    if (part == nullptr)
    {
        MU_LOG_ERROR(logger, "Part model {}: no entry in {}", name ? std::string(*name) : std::to_string(modelType),
                     PartModelsFileName);
        return;
    }
    const SharedItemModel& model = part->model;
    if (!AccessModelFile(modelType, model.file))
    {
        MU_LOG_ERROR(logger, "Part model {}: the model file {} is missing", model.name, model.file);
        Models[modelType].Release();
        return;
    }
    BMD& bmd = Models[modelType];
    for (const int mesh : model.noneBlendMeshes)
    {
        if (mesh < bmd.NumMeshs)
        {
            bmd.Meshs[mesh].NoneBlendMesh = true;
        }
        else
        {
            MU_LOG_WARN(logger, "Part model {}: noneBlendMeshes names mesh {}, the model has {}", model.name, mesh,
                        bmd.NumMeshs);
        }
    }
    if (!part->renderStyle.empty() && !lookNames.renderStyle(part->renderStyle))
    {
        MU_LOG_WARN(logger, "Part model {}: the render style {} does not exist", model.name, part->renderStyle);
    }
    std::vector<TextureProblem> textureProblems;
    gLoadData.OpenTexture(modelType, LoaderFolders(model.textureFolders), textureProblems);
    for (const TextureProblem& problem : textureProblems)
    {
        MU_LOG_WARN(logger, "Part model {}: texture {} of mesh {} is missing in its texture folders", model.name,
                    Core::Text::ToUtf8(problem.fileName.c_str()), problem.mesh);
    }
}

std::string TakeProblemMessage()
{
    const std::string message = DescribeItemModelErrors(g_problems, g_ItemDatabase, MaxErrorsInMessage);
    g_problems.clear();
    // Warnings are only written out with the next error; the player may quit
    // the game from the message.
    mu::log::Get(LoggerName)->flush();
    return message;
}
} // namespace Data::Items::ModelLoader
