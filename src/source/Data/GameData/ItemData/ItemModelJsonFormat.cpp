#include "stdafx.h"

#include "ItemModelJsonFormat.h"
#include "ItemJsonCommon.h"

#include <algorithm>
#include <cctype>
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
constexpr const char* File = "file";
constexpr const char* TextureFolders = "textureFolders";
constexpr const char* NoneBlendMeshes = "noneBlendMeshes";
} // namespace Keys

constexpr std::string_view ModelFileExtension = ".bmd";
constexpr char FolderSeparator = '/';
constexpr char WindowsFolderSeparator = '\\';

bool EndsWithIgnoringCase(std::string_view text, std::string_view ending)
{
    return text.size() >= ending.size() && std::equal(ending.rbegin(), ending.rend(), text.rbegin(),
                                                      [](char left, char right)
                                                      {
                                                          return std::tolower(static_cast<unsigned char>(left)) ==
                                                                 std::tolower(static_cast<unsigned char>(right));
                                                      });
}

bool HasWindowsSeparator(std::string_view path)
{
    return path.find(WindowsFolderSeparator) != std::string_view::npos;
}

// ---------------------------------------------------------------- writing

OrderedJson WriteModel(const ItemModelDefinition& model)
{
    OrderedJson json;
    json[Keys::Number] = model.number;
    json[Keys::File] = model.file;
    if (!model.textureFolders.empty())
    {
        json[Keys::TextureFolders] = model.textureFolders;
    }
    if (!model.noneBlendMeshes.empty())
    {
        json[Keys::NoneBlendMeshes] = model.noneBlendMeshes;
    }
    return json;
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

private:
    void AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message);
    void AddError(const std::string& field, const std::string& message)
    {
        AddIssue(ItemDataIssueSeverity::Error, field, message);
    }
    bool ReadFile(const OrderedJson& json, std::string& file);
    void ReadTextureFolders(const OrderedJson& json, std::vector<std::string>& folders);
    void ReadNoneBlendMeshes(const OrderedJson& json, std::vector<int>& meshes);
    void WarnAboutUnknownKeys(const OrderedJson& json);

    const std::string& m_source;
    int m_group;
    int m_number = ItemDataIssue::NoItem;
    bool m_hasErrors = false;
    std::vector<ItemDataIssue>& m_issues;
};

void ItemModelReader::AddIssue(ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
{
    m_issues.push_back({severity, m_source, m_group, m_number, field, message});
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
    if (HasWindowsSeparator(file))
    {
        AddError(Keys::File, "must use / between folders");
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
        if (HasWindowsSeparator(folder) || folder.front() == FolderSeparator || folder.back() == FolderSeparator)
        {
            AddError(Keys::TextureFolders,
                     "\"" + folder + "\" must use / between folders and must not start or end with /");
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

void ItemModelReader::WarnAboutUnknownKeys(const OrderedJson& json)
{
    static const std::set<std::string, std::less<>> KnownKeys{Keys::Number, Keys::File, Keys::TextureFolders,
                                                              Keys::NoneBlendMeshes};
    for (const auto& [key, value] : json.items())
    {
        if (!KnownKeys.contains(key))
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
    if (!Json::ReadNumber(json, m_number, addError) || !ReadFile(json, model.file))
    {
        return false;
    }

    model.group = m_group;
    model.number = m_number;
    ReadTextureFolders(json, model.textureFolders);
    ReadNoneBlendMeshes(json, model.noneBlendMeshes);
    WarnAboutUnknownKeys(json);
    return !m_hasErrors;
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

    const auto modelList = root.find(Keys::Models);
    if (modelList == root.end() || !modelList->is_array())
    {
        Json::AddFileIssue(issues, source, group, Keys::Models, "missing or not a list");
        return;
    }

    for (const OrderedJson& json : *modelList)
    {
        ItemModelDefinition model;
        ItemModelReader reader(source, group, issues);
        if (reader.Read(json, model))
        {
            models.push_back(std::move(model));
        }
    }
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

    std::string text = root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    text = Json::PutListsOnOneLine(text, Keys::TextureFolders);
    text = Json::PutListsOnOneLine(text, Keys::NoneBlendMeshes);
    return text + "\n";
}
} // namespace Data::Items
