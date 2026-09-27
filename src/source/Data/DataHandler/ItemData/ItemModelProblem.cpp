#include "stdafx.h"

#include "ItemModelProblem.h"
#include "ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <string_view>

namespace Data::Items
{
namespace
{
// The game reads textures from encrypted copies: <name>.OZJ for a .jpg,
// <name>.OZT for a .tga.
struct StoredTextureExtension
{
    std::string_view textureExtension;
    std::string_view storedExtension;
};

constexpr std::array<StoredTextureExtension, 2> StoredTextureExtensions = {{{".jpg", ".OZJ"}, {".tga", ".OZT"}}};
constexpr const char* TextureRootFolder = "Data/";

bool EqualsIgnoringCase(std::string_view left, std::string_view right)
{
    return left.size() == right.size() &&
           std::equal(
               left.begin(), left.end(), right.begin(), [](char a, char b)
               { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
}

// "Sword01.jpg (Sword01.OZJ)": the name in the model and the file the game reads.
std::string DescribeTexture(const std::string& texture)
{
    const size_t dot = texture.rfind('.');
    if (dot == std::string::npos)
    {
        return texture;
    }

    const std::string_view extension = std::string_view(texture).substr(dot);
    for (const StoredTextureExtension& stored : StoredTextureExtensions)
    {
        if (EqualsIgnoringCase(extension, stored.textureExtension))
        {
            return texture + " (" + texture.substr(0, dot) + std::string(stored.storedExtension) + ")";
        }
    }
    return texture;
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
           problem.modelFile + " not found in " + DescribeFolders(problem.searchedFolders);
}

std::string DescribeProblem(const ItemModelProblem& problem)
{
    switch (problem.type)
    {
    case ItemModelProblemType::ModelFileMissing:
        return "model file " + problem.modelFile + " not found";
    case ItemModelProblemType::TextureMissing:
        return DescribeTextureOfMesh(problem);
    case ItemModelProblemType::TextureOutsideFolders:
        return DescribeTextureOfMesh(problem) + "; the one another model loaded (" + problem.usedInstead +
               ") is used, add its folder to textureFolders";
    case ItemModelProblemType::NoneBlendMeshMissing:
        return "noneBlendMeshes has mesh " + std::to_string(problem.mesh) + ", but " + problem.modelFile + " has " +
               std::to_string(problem.meshCount) + " meshes";
    }
    return {};
}
} // namespace

std::string ItemModelProblem::ToString(const ItemDatabase& items) const
{
    const std::string modelEntry = (GetItemModelDataDirectory() / GetItemGroupFileName(group)).generic_string();
    return items.GetLogName(MakeItemType(group, number)) + ": " + DescribeProblem(*this) + " (" + modelEntry + ")";
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
