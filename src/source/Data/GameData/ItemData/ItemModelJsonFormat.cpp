#include "stdafx.h"

#include "ItemModelJsonFormat.h"
#include "ItemJsonCommon.h"
#include "ItemModelDisplayJson.h"
#include "ItemModelGlowJson.h"
#include "ItemTextureFiles.h"

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
constexpr const char* Model = "model";
constexpr const char* Name = "name";
constexpr const char* File = "file";
constexpr const char* TextureFolders = "textureFolders";
constexpr const char* NoneBlendMeshes = "noneBlendMeshes";
constexpr const char* RenderStyle = "renderStyle";
constexpr const char* ItemEffect = "itemEffect";
} // namespace Keys

// The fields of a model entry and of a shared model; others are warned about.
const std::set<std::string, std::less<>> ItemModelKeys{
    Keys::Number,           Keys::Model,           Keys::File,
    Keys::TextureFolders,   Keys::NoneBlendMeshes, DisplayJson::InventoryKey,
    DisplayJson::GroundKey, DisplayJson::ClothKey, GlowJson::GlowKey,
    Keys::RenderStyle,      Keys::ItemEffect};
const std::set<std::string, std::less<>> SharedModelKeys{Keys::Name, Keys::File, Keys::TextureFolders,
                                                         Keys::NoneBlendMeshes};

constexpr std::string_view ModelFileExtension = ".bmd";
constexpr char FolderSeparator = '/';
constexpr char WindowsFolderSeparator = '\\';

constexpr std::string_view ParentFolder = "..";
constexpr char DriveSeparator = ':';

// Paths in the model files stay inside the game folder: relative, with '/'
// between folders, without ".." and without empty folder names. Returns
// what is wrong, or nothing when the path is fine.
std::string FindPathProblem(std::string_view path)
{
    if (path.find(WindowsFolderSeparator) != std::string_view::npos)
    {
        return "must use / between folders";
    }
    if (path.front() == FolderSeparator || path.find(DriveSeparator) != std::string_view::npos)
    {
        return "must be a relative path";
    }

    size_t partStart = 0;
    while (partStart <= path.size())
    {
        const size_t partEnd = std::min(path.find(FolderSeparator, partStart), path.size());
        const std::string_view part = path.substr(partStart, partEnd - partStart);
        if (part.empty())
        {
            return "must not have empty folder names or end with /";
        }
        if (part == ParentFolder)
        {
            return "must not contain \"..\"";
        }
        partStart = partEnd + 1;
    }
    return {};
}

// ---------------------------------------------------------------- writing

// The file, texture folders and none-blend meshes of a model entry or a
// shared model.
void WriteModelFile(const std::string& file, const std::vector<std::string>& textureFolders,
                    const std::vector<int>& noneBlendMeshes, OrderedJson& json)
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
        WriteModelFile(model.file, model.textureFolders, model.noneBlendMeshes, json);
    }
    DisplayJson::Write(model, json);
    GlowJson::Write(model, json);
    if (!model.renderStyle.empty())
    {
        json[Keys::RenderStyle] = model.renderStyle;
    }
    if (!model.itemEffect.empty())
    {
        json[Keys::ItemEffect] = model.itemEffect;
    }
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

private:
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message);
    void AddError(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Error, field, message);
    }
    bool ReadFile(const OrderedJson& json, std::string& file);
    void ReadTextureFolders(const OrderedJson& json, std::vector<std::string>& folders);
    void ReadNoneBlendMeshes(const OrderedJson& json, std::vector<int>& meshes);
    void ReadLookName(const OrderedJson& json, const char* key, std::string& name);
    void WarnAboutUnknownKeys(const OrderedJson& json, const std::set<std::string, std::less<>>& knownKeys);

    const std::string& m_source;
    int m_group;
    int m_number = ItemDataIssue::NoItem;
    // Shared models: their name before the fields in messages.
    std::string m_fieldPrefix;
    bool m_hasErrors = false;
    std::vector<ItemDataIssue>& m_issues;
};

void ItemModelReader::AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
{
    m_issues.push_back({severity, m_source, m_group, m_number, m_fieldPrefix + field, message});
    m_hasErrors = m_hasErrors || severity == ItemDataIssueSeverity::Error;
}

bool ItemModelReader::ReadFile(const OrderedJson& json, std::string& file)
{
    const auto field = json.find(Keys::File);
    if (field == json.end() || !field->is_string() || field->get_ref<const std::string&>().empty())
    {
        AddError(Keys::File, "missing or not a text");
        return false;
    }

    file = field->get<std::string>();
    if (!EndsWithIgnoringCase(file, ModelFileExtension))
    {
        AddError(Keys::File, "must name a " + std::string(ModelFileExtension) + " file");
        return false;
    }
    const std::string pathProblem = FindPathProblem(file);
    if (!pathProblem.empty())
    {
        AddError(Keys::File, pathProblem);
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
        AddError(Keys::TextureFolders, "must be a list of folder names");
        return;
    }

    for (const OrderedJson& entry : *list)
    {
        const bool isFolder = entry.is_string() && !entry.get_ref<const std::string&>().empty();
        if (!isFolder)
        {
            AddError(Keys::TextureFolders, entry.dump() + " is not a folder name");
            continue;
        }

        const std::string& folder = entry.get_ref<const std::string&>();
        const std::string pathProblem = FindPathProblem(folder);
        if (!pathProblem.empty())
        {
            AddError(Keys::TextureFolders, "\"" + folder + "\" " + pathProblem);
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
        AddError(Keys::NoneBlendMeshes, "must be a list of mesh indexes");
        return;
    }

    for (const OrderedJson& entry : *list)
    {
        long long mesh = 0;
        if (!Json::ReadWholeNumber(entry, mesh) || mesh < 0 || mesh > std::numeric_limits<int>::max())
        {
            AddError(Keys::NoneBlendMeshes, entry.dump() + " is not a mesh index");
            continue;
        }
        meshes.push_back(static_cast<int>(mesh));
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
        AddError(key, "must be a name of letters and digits");
        return;
    }
    name = field->get<std::string>();
}

void ItemModelReader::WarnAboutUnknownKeys(const OrderedJson& json, const std::set<std::string, std::less<>>& knownKeys)
{
    for (const auto& [key, value] : json.items())
    {
        if (!knownKeys.contains(key))
        {
            AddIssue(ItemDataIssueSeverity::Warning, key, "unknown field, ignored");
        }
    }
}

bool ItemModelReader::Read(const OrderedJson& json, ItemModelDefinition& model)
{
    if (!json.is_object())
    {
        AddError("", "a model must be an object");
        return false;
    }

    const auto addError = [this](const std::string& field, const std::string& message) { AddError(field, message); };
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
        for (const char* key : {Keys::File, Keys::TextureFolders, Keys::NoneBlendMeshes})
        {
            if (json.contains(key))
            {
                AddError(key, std::string("is the shared model's; leave it out with \"") + Keys::Model + "\"");
            }
        }
    }
    else if (ReadFile(json, model.file))
    {
        ReadTextureFolders(json, model.textureFolders);
        ReadNoneBlendMeshes(json, model.noneBlendMeshes);
    }
    else
    {
        return false;
    }
    const auto report = [this](ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
    { AddIssue(severity, field, message); };
    DisplayJson::Read(json, model, report);
    GlowJson::Read(json, model, report);
    ReadLookName(json, Keys::RenderStyle, model.renderStyle);
    ReadLookName(json, Keys::ItemEffect, model.itemEffect);
    WarnAboutUnknownKeys(json, ItemModelKeys);
    return !m_hasErrors;
}

bool ItemModelReader::ReadShared(const OrderedJson& json, size_t index, SharedItemModel& model)
{
    const std::string position = std::string(Keys::Models) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        AddError(position, "a shared model must be an object");
        return false;
    }

    const auto name = json.find(Keys::Name);
    if (name == json.end() || !name->is_string() || !Json::IsName(name->get_ref<const std::string&>()))
    {
        AddError(position + "." + Keys::Name, "missing or not a name of letters and digits");
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
    WarnAboutUnknownKeys(json, SharedModelKeys);
    return !m_hasErrors;
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
        WriteModelFile(model->file, model->textureFolders, model->noneBlendMeshes, json);
        root[Keys::Models].push_back(std::move(json));
    }
    return DumpModelFile(root);
}
} // namespace Data::Items
