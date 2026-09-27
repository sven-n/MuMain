#include "stdafx.h"

#include "ItemModelLoader.h"

#include "Core/Globals/_enum.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/LoadData.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
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

std::wstring ToLoaderPath(const std::string& path)
{
    std::wstring result = Core::Text::FromUtf8(path);
    std::replace(result.begin(), result.end(), DataSeparator, LoaderSeparator);
    return result;
}

void MarkNoneBlendMeshes(int itemType, const ItemModelDefinition& model)
{
    BMD& bmd = Models[MODEL_ITEM + itemType];
    for (const int mesh : model.noneBlendMeshes)
    {
        if (mesh >= bmd.NumMeshs)
        {
            MU_LOG_WARN(mu::log::Get("data"),
                        "Item model ({},{}) {}: noneBlendMeshes has mesh {}, but the model has {}", model.group,
                        model.number, model.file, mesh, bmd.NumMeshs);
            continue;
        }
        bmd.Meshs[mesh].NoneBlendMesh = true;
    }
}

void OpenModel(int itemType, const ItemModelDefinition& model)
{
    const std::wstring path = ToLoaderPath(model.file);
    const size_t nameStart = path.find_last_of(LoaderSeparator) + 1; // 0 when there is no folder
    const std::wstring folder = path.substr(0, nameStart);
    const std::wstring name = path.substr(nameStart, path.size() - nameStart - ModelFileExtension.size());

    gLoadData.AccessModel(MODEL_ITEM + itemType, folder.c_str(), name.c_str());
    MarkNoneBlendMeshes(itemType, model);
}

void OpenModelTextures(int itemType, const ItemModelDefinition& model)
{
    std::vector<std::wstring> folders;
    folders.reserve(model.textureFolders.size());
    for (const std::string& textureFolder : model.textureFolders)
    {
        folders.push_back(ToLoaderPath(textureFolder) + LoaderSeparator);
    }
    gLoadData.OpenTexture(MODEL_ITEM + itemType, folders);
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
} // namespace Data::Items::ModelLoader
