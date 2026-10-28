#include "stdafx.h"

#include "Data/DataHandler/ModelData/ModelFileDataHandler.h"

#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/LoadData.h"
#include "Render/Models/Gltf/EmbeddedTextures.h"
#include "Render/Models/Gltf/GlbLoader.h"
#include "Render/Models/Gltf/SkinnedModel.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <cstdlib>
#include <span>
#include <vector>

namespace Data::Models
{
namespace
{
constexpr std::string_view ModelFileDataLabel = "Model file data";
constexpr std::string_view ModelFileDataName = "model file table";
constexpr const wchar_t* GameDataPrefix = L"Data\\";

ModelFiles& ModelFileTable()
{
    static ModelFiles table;
    return table;
}

void LogTextureProblems(const BMD& model, const ModelFileEntry& entry, std::span<const TextureProblem> problems)
{
    const Render::Models::Gltf::SkinnedModel* skinned = Render::Models::Gltf::SkinnedModelOf(model);
    for (const TextureProblem& problem : problems)
    {
        const bool hasImage = skinned && problem.mesh < static_cast<int>(skinned->meshImages.size()) &&
                              skinned->meshImages[problem.mesh] >= 0;
        if (hasImage)
        {
            continue;
        }
        MU_LOG_WARN(mu::log::Get("render"), "Model {} ({}): mesh {} has no texture {}{}", entry.model, entry.file,
                    problem.mesh, Core::Text::ToUtf8(problem.fileName.c_str()),
                    problem.usedInstead.empty() ? std::string()
                                                : ", uses " + Core::Text::ToUtf8(problem.usedInstead.c_str()));
    }
}
} // namespace

ModelFiles ReadModelFileTable(const char* overlayValue, const std::filesystem::path& game)
{
    std::vector<Items::ItemDataIssue> overlayIssues;
    const DataDirectories directories = MakeDataDirectories(overlayValue, game, overlayIssues);
    ModelFiles table = LoadModelFiles(directories);
    table.issues.insert(table.issues.begin(), overlayIssues.begin(), overlayIssues.end());
    if (directories.overlay)
    {
        MU_LOG_INFO(mu::log::Get("data"), "Data overlay {} is searched before {}",
                    directories.overlay->generic_string(), game.generic_string());
    }
    return table;
}

bool LoadModelFileTable(std::string& errorMessage)
{
    ModelFiles table = ReadModelFileTable(std::getenv(DataOverlayVariable), "Data");
    LogDataIssues(ModelFileDataLabel, table.issues);
    if (Items::HasErrors(table.issues))
    {
        errorMessage = DescribeDataErrors(ModelFileDataName, table.table.parent_path(), table.issues);
        return false;
    }
    for (const ModelFileEntry& entry : table.models)
    {
        MU_LOG_INFO(mu::log::Get("data"), "Model {} is drawn from {}", entry.model, entry.path.generic_string());
    }
    ModelFileTable() = std::move(table);
    return true;
}

const ModelFiles& GetModelFileTable()
{
    return ModelFileTable();
}

bool ApplyModelFile(BMD& model, int type, const ModelFiles& table)
{
    const ModelFileEntry* entry = table.Find(type);
    if (entry == nullptr)
    {
        return false;
    }
    std::string reason;
    return Render::Models::Gltf::LoadGlbIntoBmd(model, entry->path, entry->actions, reason, entry->lod);
}

std::wstring ModelFileTextureName(const ModelFileEntry& entry)
{
    std::wstring name = GameDataPrefix + Core::Text::FromUtf8(entry.file);
    std::replace(name.begin(), name.end(), L'/', L'\\');
    return name;
}

void OpenModelFileTextures(int type, std::span<const std::wstring> subFolders, const ModelFiles& table)
{
    const ModelFileEntry* entry = table.Find(type);
    if (entry == nullptr)
    {
        return;
    }
    std::vector<TextureProblem> problems;
    gLoadData.OpenTexture(type, subFolders, problems);
    BMD& model = ::Models[type];
    Render::Models::Gltf::OpenEmbeddedTextures(model, ModelFileTextureName(*entry), GL_REPEAT, GL_NEAREST);
    LogTextureProblems(model, *entry, problems);
}
} // namespace Data::Models
