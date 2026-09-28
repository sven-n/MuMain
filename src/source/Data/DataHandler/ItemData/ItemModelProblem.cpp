#include "stdafx.h"

#include "ItemModelProblem.h"
#include "ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <string_view>

namespace Data::Items
{
namespace
{
constexpr const char* TextureRootFolder = "Data/";

// "Sword01.jpg (Sword01.OZJ)": the name in the model and the file the game reads.
std::string DescribeTexture(const std::string& texture)
{
    const std::optional<std::string> storedName = GetStoredTextureFileName(texture);
    return storedName ? texture + " (" + *storedName + ")" : texture;
}

// "Data/Item/ or Data/Skill/"
std::string DescribeFolders(const std::vector<std::string>& folders)
{
    std::string text;
    for (size_t i = 0; i < folders.size(); ++i)
    {
        text += i == 0 ? "" : (i + 1 == folders.size() ? " or " : ", ");
        text += TextureRootFolder + folders[i] + "/";
    }
    return text;
}

std::string DescribeTextureOfMesh(const ItemModelProblem& problem)
{
    return "texture " + DescribeTexture(problem.texture) + " of mesh " + std::to_string(problem.mesh) + " of " +
           problem.modelFile;
}

// The file may be missing, or it exists but could not be read (e.g. it is
// damaged); the loader cannot tell these apart.
constexpr const char* NotFoundOrNotReadable = " not found or not readable in ";

std::string DescribeProblem(const ItemModelProblem& problem)
{
    switch (problem.type)
    {
    case ItemModelProblemType::ModelFileMissing:
        return "model file " + problem.modelFile + " could not be opened (missing or not a valid .bmd file)";
    case ItemModelProblemType::TextureMissing:
        return DescribeTextureOfMesh(problem) + NotFoundOrNotReadable + DescribeFolders(problem.searchedFolders);
    case ItemModelProblemType::TextureTypeUnsupported:
        return DescribeTextureOfMesh(problem) + " is not a .jpg or .tga texture, which the game cannot load";
    case ItemModelProblemType::TextureOutsideFolders:
        return DescribeTextureOfMesh(problem) + NotFoundOrNotReadable + DescribeFolders(problem.searchedFolders) +
               "; the one another model loaded (" + problem.usedInstead + ") is used, add its folder to textureFolders";
    case ItemModelProblemType::NoneBlendMeshMissing:
        return "noneBlendMeshes has mesh " + std::to_string(problem.mesh) + ", but " + problem.modelFile + " has " +
               std::to_string(problem.meshCount) + " meshes";
    case ItemModelProblemType::GlowMeshMissing:
        return problem.field + " has mesh " + std::to_string(problem.mesh) + ", but " + problem.modelFile + " has " +
               std::to_string(problem.meshCount) + " meshes";
    }
    return {};
}

std::string GetModelEntry(int group)
{
    return (GetItemModelDataDirectory() / GetItemGroupFileName(group)).generic_string();
}
} // namespace

std::string ItemModelProblem::ToString(const ItemDatabase& items) const
{
    return items.GetLogName(MakeItemType(group, number)) + ": " + DescribeProblem(*this) + " (" + GetModelEntry(group) +
           ")";
}

std::string ItemModelProblem::ToLogString() const
{
    return "(" + std::to_string(group) + "," + std::to_string(number) + "): " + DescribeProblem(*this) + " (" +
           GetModelEntry(group) + ")";
}

std::string DescribeItemModelErrors(std::span<const ItemModelProblem> problems, const ItemDatabase& items,
                                    size_t maxShown)
{
    std::string lines;
    size_t errorCount = 0;
    for (const ItemModelProblem& problem : problems)
    {
        if (!problem.IsError())
        {
            continue;
        }
        if (++errorCount <= maxShown)
        {
            lines += "\n" + problem.ToString(items);
        }
    }
    if (errorCount == 0)
    {
        return {};
    }

    std::string message = "Some item models could not be loaded completely:\n" + lines;
    if (errorCount > maxShown)
    {
        message += "\n... and " + std::to_string(errorCount - maxShown) + " more";
    }
    return message + "\n\nAll problems are listed in MuError.log.";
}
} // namespace Data::Items
