#include "stdafx.h"

#include "ItemModelLoader.h"
#include "ItemModelProblem.h"

#include "Core/Globals/_enum.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/LoadData.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelGlowJson.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <string>
#include <string_view>
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

std::wstring ToLoaderPath(const std::string& path)
{
    std::wstring result = Core::Text::FromUtf8(path);
    std::replace(result.begin(), result.end(), DataSeparator, LoaderSeparator);
    return result;
}

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
    BMD& bmd = Models[MODEL_ITEM + itemType];
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
    const int meshCount = Models[MODEL_ITEM + itemType].NumMeshs;
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

void OpenModel(int itemType, const ItemModelDefinition& model)
{
    const std::wstring path = ToLoaderPath(model.file);
    const size_t nameStart = path.find_last_of(LoaderSeparator) + 1; // 0 when there is no folder
    const std::wstring folder = path.substr(0, nameStart);
    const std::wstring name = path.substr(nameStart, path.size() - nameStart - ModelFileExtension.size());

    if (!gLoadData.AccessModel(MODEL_ITEM + itemType, folder.c_str(), name.c_str()))
    {
        AddProblem(MakeProblem(ItemModelProblemType::ModelFileMissing, model));
        return;
    }
    MarkNoneBlendMeshes(itemType, model);
    CheckGlowMeshes(itemType, model);
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
    std::vector<std::wstring> folders;
    folders.reserve(model.textureFolders.size());
    for (const std::string& textureFolder : model.textureFolders)
    {
        folders.push_back(ToLoaderPath(textureFolder) + LoaderSeparator);
    }

    std::vector<TextureProblem> textureProblems;
    gLoadData.OpenTexture(MODEL_ITEM + itemType, folders, textureProblems);
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

void OpenModels()
{
    ForEachModel(OpenModel);
}

void OpenTextures()
{
    ForEachModel(OpenModelTextures);
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
