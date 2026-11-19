#include "stdafx.h"

#include "ItemModelJsonFormat.h"
#include "ItemJsonCommon.h"
#include "ItemModelDisplayJson.h"
#include "ItemModelGlowJson.h"
#include "ItemModelMaterialJson.h"
#include "ItemTextureFiles.h"

#include "Data/GameData/ColorFolders/ColorFolders.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/Common/MeshLimits.h"

#include <algorithm>
#include <limits>
#include <set>

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

namespace Keys
{
using namespace Json::Keys;
constexpr const char* Models = "models";
constexpr const char* PartModels = "partModels";
constexpr const char* Model = "model";
constexpr const char* Name = "name";
constexpr const char* File = "file";
constexpr const char* TextureFolders = "textureFolders";
constexpr const char* NoneBlendMeshes = "noneBlendMeshes";
constexpr const char* MeshNames = "meshNames";
constexpr const char* RenderStyle = "renderStyle";
constexpr const char* ItemEffect = "itemEffect";
constexpr const char* RenderStyleValues = "renderStyleValues";
constexpr const char* ItemEffectValues = "itemEffectValues";
} // namespace Keys

// The fields of a model entry and of a shared model; others are warned about.
const std::set<std::string, std::less<>> ItemModelKeys{Keys::Number,
                                                       Keys::Model,
                                                       Keys::File,
                                                       Keys::TextureFolders,
                                                       Keys::NoneBlendMeshes,
                                                       Keys::MeshNames,
                                                       DisplayJson::InventoryKey,
                                                       DisplayJson::GroundKey,
                                                       DisplayJson::ClothKey,
                                                       GlowJson::GlowKey,
                                                       MaterialJson::MaterialKey,
                                                       Keys::RenderStyle,
                                                       Keys::RenderStyleValues,
                                                       Keys::ItemEffect,
                                                       Keys::ItemEffectValues};
const std::set<std::string, std::less<>> SharedModelKeys{Keys::Name, Keys::File, Keys::TextureFolders,
                                                         Keys::NoneBlendMeshes, Keys::MeshNames};
const std::set<std::string, std::less<>> PartModelKeys{
    Keys::Name, Keys::File, Keys::TextureFolders, Keys::NoneBlendMeshes, Keys::MeshNames, Keys::RenderStyle};

constexpr std::string_view ModelFileExtension = ".bmd";
using Data::Paths::FindPathProblem;

// ---------------------------------------------------------------- writing

// The file, texture folders, none-blend meshes and mesh names of a model entry or a shared model.
void WriteModelFile(const std::string& file, const std::vector<std::string>& textureFolders,
                    const std::vector<int>& noneBlendMeshes, const std::map<int, std::string>& meshNames,
                    OrderedJson& json)
{
    json[Keys::File] = file;
    if (!textureFolders.empty())
    {
        json[Keys::TextureFolders] = textureFolders;
    }
    if (!noneBlendMeshes.empty())
    {
        json[Keys::NoneBlendMeshes] = noneBlendMeshes;
    }
    if (!meshNames.empty())
    {
        OrderedJson names = OrderedJson::object();
        for (const auto& [mesh, name] : meshNames)
        {
            names[std::to_string(mesh)] = name;
        }
        json[Keys::MeshNames] = std::move(names);
    }
}

OrderedJson WriteModel(const ItemModelDefinition& model)
{
    OrderedJson json;
    json[Keys::Number] = model.number;
    if (!model.model.empty())
    {
        // The file, texture folders and none-blend meshes are the shared model's.
        json[Keys::Model] = model.model;
    }
    else
    {
        WriteModelFile(model.file, model.textureFolders, model.noneBlendMeshes, model.meshNames, json);
    }
    DisplayJson::Write(model, json);
    GlowJson::Write(model, json);
    MaterialJson::Write(model, json);
    if (!model.renderStyle.empty())
    {
        json[Keys::RenderStyle] = model.renderStyle;
    }
    const auto putValues = [&json](const char* key, const std::map<std::string, std::string>& values)
    {
        if (values.empty())
        {
            return;
        }
        OrderedJson written = OrderedJson::object();
        for (const auto& [name, value] : values)
        {
            written[name] = OrderedJson::parse(value, nullptr, false);
        }
        json[key] = std::move(written);
    };
    putValues(Keys::RenderStyleValues, model.renderStyleValues);
    if (!model.itemEffect.empty())
    {
        json[Keys::ItemEffect] = model.itemEffect;
    }
    putValues(Keys::ItemEffectValues, model.itemEffectValues);
    return json;
}

// The text of a model file, with the lists of numbers and names on one line.
std::string DumpModelFile(const OrderedJson& root)
{
    std::string text = root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    for (const char* key :
         {Keys::TextureFolders, Keys::NoneBlendMeshes, DisplayJson::AnchorKey, DisplayJson::OffsetKey,
          DisplayJson::RotationKey, GlowJson::LevelKey, GlowJson::MeshesKey, GlowJson::ShineMeshesKey})
    {
        text = Json::PutListsOnOneLine(text, key);
    }
    text = Json::PutObjectsOnOneLine(text, Keys::MeshNames);
    text = Json::PutObjectsOnOneLine(text, Keys::RenderStyleValues);
    text = Json::PutObjectsOnOneLine(text, Keys::ItemEffectValues);
    return text + "\n";
}

// ---------------------------------------------------------------- reading

// Reads the model currently being read and collects its problems.
class ItemModelReader
{
public:
    ItemModelReader(const std::string& source, int group, std::vector<ItemDataIssue>& issues)
        : m_source(source), m_group(group), m_issues(issues)
    {
    }

    bool Read(const OrderedJson& json, ItemModelDefinition& model);
    // `index`: the position in the list, which names the entry until its
    // name is read.
    bool ReadShared(const OrderedJson& json, size_t index, SharedItemModel& model);
    // A part model: a shared model's fields and a render style.
    bool ReadPart(const OrderedJson& json, size_t index, PartModel& part);

private:
    bool ReadNamedModel(const OrderedJson& json, const std::string& position, SharedItemModel& model,
                        const std::set<std::string, std::less<>>& knownKeys);
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message);
    void AddFatal(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Fatal, field, message);
    }
    bool ReadFile(const OrderedJson& json, std::string& file);
    void ReadTextureFolders(const OrderedJson& json, std::vector<std::string>& folders);
    void ReadNoneBlendMeshes(const OrderedJson& json, std::vector<int>& meshes);
    void ReadMeshNames(const OrderedJson& json, std::map<int, std::string>& names);
    void ReadLookName(const OrderedJson& json, const char* key, std::string& name);
    void ReadLookValues(const OrderedJson& json, const char* key, std::map<std::string, std::string>& values);
    void WarnAboutUnknownKeys(const OrderedJson& json, const std::set<std::string, std::less<>>& knownKeys);

    const std::string& m_source;
    int m_group;
    int m_number = ItemDataIssue::NoItem;
    // Shared models: their name before the fields in messages.
    std::string m_fieldPrefix;
    bool m_hasFatal = false;
    std::vector<ItemDataIssue>& m_issues;
};

void ItemModelReader::AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
{
    m_issues.push_back({severity, m_source, m_group, m_number, m_fieldPrefix + field, message});
    m_hasFatal = m_hasFatal || severity == ItemDataIssueSeverity::Fatal;
}

bool ItemModelReader::ReadFile(const OrderedJson& json, std::string& file)
{
    const auto field = json.find(Keys::File);
    if (field == json.end() || !field->is_string() || field->get_ref<const std::string&>().empty())
    {
        AddFatal(Keys::File, "missing or not a text");
        return false;
    }

    file = field->get<std::string>();
    if (!EndsWithIgnoringCase(file, ModelFileExtension))
    {
        AddFatal(Keys::File, "must name a " + std::string(ModelFileExtension) + " file");
        return false;
    }
    const std::string pathProblem = FindPathProblem(file);
    if (!pathProblem.empty())
    {
        AddFatal(Keys::File, pathProblem);
        return false;
    }
    return true;
}

void ItemModelReader::ReadTextureFolders(const OrderedJson& json, std::vector<std::string>& folders)
{
    const auto list = json.find(Keys::TextureFolders);
    if (list == json.end())
    {
        return;
    }
    if (!list->is_array())
    {
        AddFatal(Keys::TextureFolders, "must be a list of folder names");
        return;
    }

    for (const OrderedJson& entry : *list)
    {
        const bool isFolder = entry.is_string() && !entry.get_ref<const std::string&>().empty();
        if (!isFolder)
        {
            AddFatal(Keys::TextureFolders, entry.dump() + " is not a folder name");
            continue;
        }

        const std::string& folder = entry.get_ref<const std::string&>();
        const std::string pathProblem = FindPathProblem(folder);
        if (!pathProblem.empty())
        {
            AddFatal(Keys::TextureFolders, "\"" + folder + "\" " + pathProblem);
            continue;
        }
        folders.push_back(folder);
    }
}

void ItemModelReader::ReadNoneBlendMeshes(const OrderedJson& json, std::vector<int>& meshes)
{
    const auto list = json.find(Keys::NoneBlendMeshes);
    if (list == json.end())
    {
        return;
    }
    if (!list->is_array())
    {
        AddFatal(Keys::NoneBlendMeshes, "must be a list of mesh indexes");
        return;
    }

    for (const OrderedJson& entry : *list)
    {
        long long mesh = 0;
        if (!Json::ReadWholeNumber(entry, mesh) || mesh < 0 || mesh > std::numeric_limits<int>::max())
        {
            AddFatal(Keys::NoneBlendMeshes, entry.dump() + " is not a mesh index");
            continue;
        }
        meshes.push_back(static_cast<int>(mesh));
    }
}

// The names are only shown, so a problem leaves the name out and is a warning.
void ItemModelReader::ReadMeshNames(const OrderedJson& json, std::map<int, std::string>& names)
{
    const auto list = json.find(Keys::MeshNames);
    if (list == json.end())
    {
        return;
    }
    if (!list->is_object())
    {
        AddIssue(ItemDataIssueSeverity::Warning, Keys::MeshNames,
                 "must be an object of mesh numbers and names, e.g. {\"0\": \"Shaft\"}; left out");
        return;
    }
    for (const auto& [key, value] : list->items())
    {
        const std::string field = std::string(Keys::MeshNames) + "." + key;
        long long mesh = -1;
        const bool number = !key.empty() && key.size() <= 2 &&
                            std::all_of(key.begin(), key.end(), [](char c) { return c >= '0' && c <= '9'; });
        if (number)
        {
            mesh = std::stoll(key);
        }
        if (!number || mesh >= MaxModelMeshes)
        {
            AddIssue(ItemDataIssueSeverity::Warning, field,
                     "is not a mesh number (0 to " + std::to_string(MaxModelMeshes - 1) + "); left out");
            continue;
        }
        if (!value.is_string() || !Colors::IsColorFolderName(value.get_ref<const std::string&>()))
        {
            AddIssue(ItemDataIssueSeverity::Warning, field,
                     "must be a name of 1 to " + std::to_string(Colors::MaxColorFolderNameLength) +
                         " characters, without a space at either end or a control character; left out");
            continue;
        }
        names[static_cast<int>(mesh)] = value.get<std::string>();
    }
}

// A shared model, a render style or an item effect; whether it exists is
// checked when the models are loaded (the shared models) or opened.
void ItemModelReader::ReadLookName(const OrderedJson& json, const char* key, std::string& name)
{
    const auto field = json.find(key);
    if (field == json.end())
    {
        return;
    }
    if (!field->is_string() || !Json::IsName(field->get_ref<const std::string&>()))
    {
        AddFatal(key, "must be a name of letters and digits");
        return;
    }
    name = field->get<std::string>();
}

// The values an item sets for its look (design L12): names of letters and digits, each with a value the look reads
// when the item is drawn (and Data Checks checks there).
void ItemModelReader::ReadLookValues(const OrderedJson& json, const char* key,
                                     std::map<std::string, std::string>& values)
{
    const auto field = json.find(key);
    if (field == json.end())
    {
        return;
    }
    if (!field->is_object())
    {
        AddFatal(key, "must be an object of the look's value names and the item's values, e.g. {\"color\": [1, 0, 0]}");
        return;
    }
    for (const auto& [name, value] : field->items())
    {
        if (!Json::IsName(name))
        {
            AddFatal(std::string(key) + "." + name, "must be the name of a value of letters and digits");
            continue;
        }
        values[name] = value.dump();
    }
}

void ItemModelReader::WarnAboutUnknownKeys(const OrderedJson& json, const std::set<std::string, std::less<>>& knownKeys)
{
    Json::WarnAboutUnknownKeys(json, knownKeys, "", [this](const std::string& field, const std::string& message)
                               { AddIssue(ItemDataIssueSeverity::Warning, field, message); });
}

bool ItemModelReader::Read(const OrderedJson& json, ItemModelDefinition& model)
{
    if (!json.is_object())
    {
        AddFatal("", "a model must be an object");
        return false;
    }

    const auto addError = [this](const std::string& field, const std::string& message) { AddFatal(field, message); };
    if (!Json::ReadNumber(json, m_number, addError))
    {
        return false;
    }

    model.group = m_group;
    model.number = m_number;
    if (json.contains(Keys::Model))
    {
        // The file, texture folders and none-blend meshes are the shared model's.
        ReadLookName(json, Keys::Model, model.model);
        for (const char* key : {Keys::File, Keys::TextureFolders, Keys::NoneBlendMeshes, Keys::MeshNames})
        {
            if (json.contains(key))
            {
                AddFatal(key, std::string("is the shared model's; leave it out with \"") + Keys::Model + "\"");
            }
        }
    }
    else if (ReadFile(json, model.file))
    {
        ReadTextureFolders(json, model.textureFolders);
        ReadNoneBlendMeshes(json, model.noneBlendMeshes);
        ReadMeshNames(json, model.meshNames);
    }
    else
    {
        return false;
    }
    const auto report = [this](ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    { AddIssue(severity, field, message); };
    DisplayJson::Read(json, model, report);
    GlowJson::Read(json, model, report);
    MaterialJson::Read(json, model, report);
    ReadLookName(json, Keys::RenderStyle, model.renderStyle);
    ReadLookValues(json, Keys::RenderStyleValues, model.renderStyleValues);
    ReadLookName(json, Keys::ItemEffect, model.itemEffect);
    ReadLookValues(json, Keys::ItemEffectValues, model.itemEffectValues);
    WarnAboutUnknownKeys(json, ItemModelKeys);
    return !m_hasFatal;
}

bool ItemModelReader::ReadShared(const OrderedJson& json, size_t index, SharedItemModel& model)
{
    return ReadNamedModel(json, std::string(Keys::Models) + "[" + std::to_string(index) + "]", model, SharedModelKeys);
}

bool ItemModelReader::ReadPart(const OrderedJson& json, size_t index, PartModel& part)
{
    if (!ReadNamedModel(json, std::string(Keys::PartModels) + "[" + std::to_string(index) + "]", part.model,
                        PartModelKeys))
    {
        return false;
    }
    ReadLookName(json, Keys::RenderStyle, part.renderStyle);
    return !m_hasFatal;
}

bool ItemModelReader::ReadNamedModel(const OrderedJson& json, const std::string& position, SharedItemModel& model,
                                     const std::set<std::string, std::less<>>& knownKeys)
{
    if (!json.is_object())
    {
        AddFatal(position, "must be an object");
        return false;
    }

    const auto name = json.find(Keys::Name);
    if (name == json.end() || !name->is_string() || !Json::IsName(name->get_ref<const std::string&>()))
    {
        AddFatal(position + "." + Keys::Name, "missing or not a name of letters and digits");
        return false;
    }
    model.name = name->get<std::string>();
    m_fieldPrefix = model.name + ".";
    if (!ReadFile(json, model.file))
    {
        return false;
    }
    ReadTextureFolders(json, model.textureFolders);
    ReadNoneBlendMeshes(json, model.noneBlendMeshes);
    ReadMeshNames(json, model.meshNames);
    WarnAboutUnknownKeys(json, knownKeys);
    return !m_hasFatal;
}

// Calls read(entry, index) for every entry of the "models" list of a model
// file; a file without the list is an error.
template <typename TRead>
void ReadModelList(const OrderedJson& root, const std::string& source, int group, std::vector<ItemDataIssue>& issues,
                   TRead&& read)
{
    const auto modelList = root.find(Keys::Models);
    if (modelList == root.end() || !modelList->is_array())
    {
        Json::AddFileIssue(issues, source, group, Keys::Models, "missing or not a list");
        return;
    }
    for (size_t index = 0; index < modelList->size(); ++index)
    {
        read((*modelList)[index], index);
    }
}
} // namespace

void ReadItemModelGroupJson(std::string_view text, const std::string& source, std::vector<ItemModelDefinition>& models,
                            std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    int group = 0;
    if (!Json::ReadFileHeader(text, source, ItemModelJsonFormatVersion, root, group, issues))
    {
        return;
    }

    ReadModelList(root, source, group, issues,
                  [&](const OrderedJson& json, size_t)
                  {
                      ItemModelDefinition model;
                      ItemModelReader reader(source, group, issues);
                      if (reader.Read(json, model))
                      {
                          models.push_back(std::move(model));
                      }
                  });
}

std::string WriteItemModelGroupJson(int group, std::span<const ItemModelDefinition> models)
{
    std::vector<const ItemModelDefinition*> groupModels;
    for (const ItemModelDefinition& model : models)
    {
        if (model.group == group && model.Exists())
        {
            groupModels.push_back(&model);
        }
    }
    std::sort(groupModels.begin(), groupModels.end(),
              [](const ItemModelDefinition* left, const ItemModelDefinition* right)
              { return left->number < right->number; });

    OrderedJson root;
    root[Keys::FormatVersion] = ItemModelJsonFormatVersion;
    root[Keys::Group] = group;
    root[Keys::Models] = OrderedJson::array();
    for (const ItemModelDefinition* model : groupModels)
    {
        root[Keys::Models].push_back(WriteModel(*model));
    }
    return DumpModelFile(root);
}

void ReadSharedItemModelsJson(std::string_view text, const std::string& source, std::vector<SharedItemModel>& models,
                              std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, ItemModelJsonFormatVersion, root, issues))
    {
        return;
    }

    ReadModelList(root, source, ItemDataIssue::NoItem, issues,
                  [&](const OrderedJson& json, size_t index)
                  {
                      SharedItemModel model;
                      ItemModelReader reader(source, ItemDataIssue::NoItem, issues);
                      if (reader.ReadShared(json, index, model))
                      {
                          models.push_back(std::move(model));
                      }
                  });
}

std::string WriteSharedItemModelsJson(std::span<const SharedItemModel> models)
{
    std::vector<const SharedItemModel*> sorted;
    for (const SharedItemModel& model : models)
    {
        sorted.push_back(&model);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const SharedItemModel* left, const SharedItemModel* right) { return left->name < right->name; });

    OrderedJson root;
    root[Keys::FormatVersion] = ItemModelJsonFormatVersion;
    root[Keys::Models] = OrderedJson::array();
    for (const SharedItemModel* model : sorted)
    {
        OrderedJson json;
        json[Keys::Name] = model->name;
        WriteModelFile(model->file, model->textureFolders, model->noneBlendMeshes, model->meshNames, json);
        root[Keys::Models].push_back(std::move(json));
    }
    return DumpModelFile(root);
}

void ReadPartModelsJson(std::string_view text, const std::string& source, std::vector<PartModel>& parts,
                        std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, ItemModelJsonFormatVersion, root, issues))
    {
        return;
    }
    const auto list = root.find(Keys::PartModels);
    if (list == root.end() || !list->is_array())
    {
        Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::PartModels, "missing or not a list");
        return;
    }
    std::set<std::string, std::less<>> names;
    for (size_t index = 0; index < list->size(); ++index)
    {
        PartModel part;
        ItemModelReader reader(source, ItemDataIssue::NoItem, issues);
        if (!reader.ReadPart((*list)[index], index, part))
        {
            continue;
        }
        if (!names.insert(part.model.name).second)
        {
            Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, part.model.name + "." + Keys::Name,
                               "is in the list more than once");
            continue;
        }
        parts.push_back(std::move(part));
    }
}

std::string WritePartModelsJson(std::span<const PartModel> parts)
{
    std::vector<const PartModel*> sorted;
    for (const PartModel& part : parts)
    {
        sorted.push_back(&part);
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const PartModel* left, const PartModel* right) { return left->model.name < right->model.name; });

    OrderedJson root;
    root[Keys::FormatVersion] = ItemModelJsonFormatVersion;
    root[Keys::PartModels] = OrderedJson::array();
    for (const PartModel* part : sorted)
    {
        OrderedJson json;
        json[Keys::Name] = part->model.name;
        WriteModelFile(part->model.file, part->model.textureFolders, part->model.noneBlendMeshes, part->model.meshNames,
                       json);
        if (!part->renderStyle.empty())
        {
            json[Keys::RenderStyle] = part->renderStyle;
        }
        root[Keys::PartModels].push_back(std::move(json));
    }
    return DumpModelFile(root);
}
} // namespace Data::Items
