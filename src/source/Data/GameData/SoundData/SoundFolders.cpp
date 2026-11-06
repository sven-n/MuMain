#include "stdafx.h"

#include "SoundFolders.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <set>

namespace Data::Sounds
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* Folders = "folders";
constexpr const char* Name = "name";
constexpr const char* Sounds = "sounds";
} // namespace Keys

const Items::Json::KeySet FolderKeys{Keys::Name, Keys::Folders, Keys::Sounds};

class FolderReader
{
public:
    FolderReader(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    // False, with a fatal problem added, at the first value of the wrong form.
    bool ReadList(const OrderedJson& json, const std::string& field, std::vector<SoundFolder>& folders)
    {
        if (!json.is_array())
        {
            return Fail(field, "must be a list of folders");
        }
        std::set<std::string> names;
        for (size_t index = 0; index < json.size(); ++index)
        {
            const std::string key = field + "[" + std::to_string(index) + "]";
            SoundFolder folder;
            if (!ReadFolder(json[index], key, folder))
            {
                return false;
            }
            if (!names.insert(folder.name).second)
            {
                return Fail(key + "." + Keys::Name, "\"" + folder.name + "\" is the name of another folder there too");
            }
            folders.push_back(std::move(folder));
        }
        return true;
    }

private:
    bool Fail(const std::string& field, const std::string& message)
    {
        m_issues.push_back(
            {ItemDataIssueSeverity::Fatal, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
        return false;
    }

    void Warn(const std::string& field, const std::string& message)
    {
        m_issues.push_back(
            {ItemDataIssueSeverity::Warning, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    }

    bool ReadFolder(const OrderedJson& json, const std::string& key, SoundFolder& folder)
    {
        if (!json.is_object() || !json.contains(Keys::Name))
        {
            return Fail(key, "must be an object with a name");
        }
        const OrderedJson& name = json[Keys::Name];
        if (!name.is_string() || name.get_ref<const std::string&>().empty())
        {
            return Fail(key + "." + Keys::Name, "must be a text that is not empty");
        }
        folder.name = name.get<std::string>();
        if (const auto folders = json.find(Keys::Folders);
            folders != json.end() && !ReadList(*folders, key + "." + Keys::Folders, folder.folders))
        {
            return false;
        }
        if (const auto sounds = json.find(Keys::Sounds); sounds != json.end() && !ReadSounds(*sounds, key, folder))
        {
            return false;
        }
        Items::Json::WarnAboutUnknownKeys(json, FolderKeys, key + ".",
                                          [&](const std::string& field, const std::string& message)
                                          { Warn(field, message); });
        return true;
    }

    bool ReadSounds(const OrderedJson& json, const std::string& key, SoundFolder& folder)
    {
        const std::string field = key + "." + Keys::Sounds;
        if (!json.is_array())
        {
            return Fail(field, "must be a list of sounds");
        }
        for (size_t index = 0; index < json.size(); ++index)
        {
            const std::string item = field + "[" + std::to_string(index) + "]";
            if (!json[index].is_string() || json[index].get_ref<const std::string&>().empty())
            {
                return Fail(item, "must be a sound's key");
            }
            const std::string sound = json[index].get<std::string>();
            if (!m_seen.insert(sound).second)
            {
                Warn(item, sound + " is in another folder already: it stays there");
                continue;
            }
            folder.sounds.push_back(sound);
        }
        return true;
    }

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
    std::set<std::string> m_seen;
};

OrderedJson WriteList(std::span<const SoundFolder> folders)
{
    OrderedJson list = OrderedJson::array();
    for (const SoundFolder& folder : folders)
    {
        OrderedJson json = OrderedJson::object();
        json[Keys::Name] = folder.name;
        if (!folder.folders.empty())
        {
            json[Keys::Folders] = WriteList(folder.folders);
        }
        if (!folder.sounds.empty())
        {
            json[Keys::Sounds] = folder.sounds;
        }
        list.push_back(std::move(json));
    }
    return list;
}

bool Find(std::span<const SoundFolder> folders, std::string_view key, std::vector<std::string>& path)
{
    for (const SoundFolder& folder : folders)
    {
        path.push_back(folder.name);
        for (const std::string& sound : folder.sounds)
        {
            if (sound == key)
            {
                return true;
            }
        }
        if (Find(folder.folders, key, path))
        {
            return true;
        }
        path.pop_back();
    }
    return false;
}
} // namespace

void ReadSoundFoldersJson(std::string_view text, const std::string& source, std::vector<SoundFolder>& folders,
                          std::vector<Items::ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, SoundFoldersFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::Folders}, "",
                                      [&](const std::string& field, const std::string& message)
                                      {
                                          issues.push_back({ItemDataIssueSeverity::Warning, source,
                                                            ItemDataIssue::NoItem, ItemDataIssue::NoItem, field,
                                                            message});
                                      });
    const auto list = root.find(Keys::Folders);
    if (list == root.end())
    {
        return;
    }
    std::vector<SoundFolder> read;
    if (FolderReader(source, issues).ReadList(*list, Keys::Folders, read))
    {
        folders = std::move(read);
    }
}

std::string WriteSoundFoldersJson(std::span<const SoundFolder> folders)
{
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = SoundFoldersFormatVersion;
    root[Keys::Folders] = WriteList(folders);
    return root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace) + "\n";
}

std::optional<std::vector<std::string>> FindSoundFolder(std::span<const SoundFolder> folders, std::string_view key)
{
    std::vector<std::string> path;
    if (Find(folders, key, path))
    {
        return path;
    }
    return std::nullopt;
}
} // namespace Data::Sounds
