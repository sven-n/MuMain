#include "stdafx.h"

#include "ColorFolders.h"

#include "Core/Text/Utf8.h"

#include <algorithm>
#include <set>
#include <utility>

namespace Data::Colors
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* NameKey = "name";
constexpr const char* FoldersKey = "folders";
constexpr const char* ColorsKey = "colors";

// U+0000 to U+001F and U+007F, which a name can't show.
bool IsControlCharacter(char character)
{
    constexpr unsigned char LastControlCharacter = 0x1F;
    constexpr unsigned char DeleteCharacter = 0x7F;
    const auto byte = static_cast<unsigned char>(character);
    return byte <= LastControlCharacter || byte == DeleteCharacter;
}

bool HasFolderNamed(const std::vector<ColorFolder>& folders, std::string_view name)
{
    return std::any_of(folders.begin(), folders.end(),
                       [name](const ColorFolder& folder) { return folder.name == name; });
}

// Reads the folder trees of one file; remembers the colors placed so far, so a color goes into one folder only.
class FolderReader
{
public:
    FolderReader(const std::string& source, const std::function<bool(std::string_view)>& isColor,
                 std::vector<ItemDataIssue>& issues)
        : m_source(source), m_isColor(isColor), m_issues(issues)
    {
    }

    std::vector<ColorFolder> ReadList(const OrderedJson& list, const std::string& field, std::size_t depth)
    {
        std::vector<ColorFolder> folders;
        if (!list.is_array())
        {
            Warn(field, "must be a list of folders; left out");
            return folders;
        }
        for (std::size_t index = 0; index < list.size(); ++index)
        {
            const std::string entryField = field + "[" + std::to_string(index) + "]";
            std::optional<std::string> name = ReadName(list[index], entryField, depth);
            if (!name)
            {
                continue;
            }
            if (HasFolderNamed(folders, *name))
            {
                Warn(entryField + "." + NameKey, "another folder here has the name \"" + *name + "\"; left out");
                continue;
            }
            folders.push_back(ReadContent(list[index], entryField, depth, std::move(*name)));
        }
        return folders;
    }

private:
    std::optional<std::string> ReadName(const OrderedJson& json, const std::string& field, std::size_t depth)
    {
        if (depth > MaxColorFolderDepth)
        {
            Warn(field, "is deeper than " + std::to_string(MaxColorFolderDepth) + " folders; left out");
            return std::nullopt;
        }
        if (!json.is_object())
        {
            Warn(field, "must be a folder; left out");
            return std::nullopt;
        }
        const auto name = json.find(NameKey);
        if (name == json.end() || !name->is_string() || !IsColorFolderName(name->get<std::string>()))
        {
            Warn(field + "." + NameKey, "must be a name of 1 to " + std::to_string(MaxColorFolderNameLength) +
                                            " characters, without a space at either end or a control character; "
                                            "the folder is left out");
            return std::nullopt;
        }
        return name->get<std::string>();
    }

    ColorFolder ReadContent(const OrderedJson& json, const std::string& field, std::size_t depth, std::string name)
    {
        Items::Json::WarnAboutUnknownKeys(json, {NameKey, FoldersKey, ColorsKey}, field + ".",
                                          [this](const std::string& unknown, const std::string& message)
                                          { Warn(unknown, message); });
        ColorFolder folder;
        folder.name = std::move(name);
        if (const auto folders = json.find(FoldersKey); folders != json.end())
        {
            folder.folders = ReadList(*folders, field + "." + FoldersKey, depth + 1);
        }
        if (const auto colors = json.find(ColorsKey); colors != json.end())
        {
            ReadColors(*colors, field + "." + ColorsKey, folder.colors);
        }
        return folder;
    }

    void ReadColors(const OrderedJson& list, const std::string& field, std::vector<std::string>& colors)
    {
        if (!list.is_array())
        {
            Warn(field, "must be a list of color names; left out");
            return;
        }
        for (std::size_t index = 0; index < list.size(); ++index)
        {
            const std::string entryField = field + "[" + std::to_string(index) + "]";
            if (!list[index].is_string())
            {
                Warn(entryField, "must be a color's name; left out");
                continue;
            }
            std::string color = list[index].get<std::string>();
            if (!m_isColor(color))
            {
                Warn(entryField, "\"" + color + "\" is not a color of the file; left out");
                continue;
            }
            if (!m_placed.insert(color).second)
            {
                Warn(entryField, "\"" + color + "\" is in a folder already; left out");
                continue;
            }
            colors.push_back(std::move(color));
        }
    }

    void Warn(const std::string& field, const std::string& message)
    {
        m_issues.push_back(
            {ItemDataIssueSeverity::Warning, m_source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
    }

    const std::string& m_source;
    const std::function<bool(std::string_view)>& m_isColor;
    std::vector<ItemDataIssue>& m_issues;
    std::set<std::string, std::less<>> m_placed;
};

OrderedJson WriteFolder(const ColorFolder& folder, const std::function<bool(std::string_view)>& isColor)
{
    OrderedJson json = OrderedJson::object();
    json[NameKey] = folder.name;
    if (!folder.folders.empty())
    {
        json[FoldersKey] = WriteColorFolders(folder.folders, isColor);
    }
    std::vector<std::string> colors;
    std::copy_if(folder.colors.begin(), folder.colors.end(), std::back_inserter(colors),
                 [&isColor](const std::string& color) { return isColor(color); });
    if (!colors.empty())
    {
        json[ColorsKey] = std::move(colors);
    }
    return json;
}

// The folders the parent holds (the empty path: the root's), or nullptr.
std::vector<ColorFolder>* FindChildren(std::vector<ColorFolder>& folders, std::span<const std::size_t> parent)
{
    if (parent.empty())
    {
        return &folders;
    }
    ColorFolder* folder = FindColorFolder(folders, parent);
    return folder != nullptr ? &folder->folders : nullptr;
}

// The list the folder at the path is in, or nullptr.
std::vector<ColorFolder>* FindSiblings(std::vector<ColorFolder>& folders, std::span<const std::size_t> path)
{
    if (path.empty() || FindColorFolder(folders, path) == nullptr)
    {
        return nullptr;
    }
    return FindChildren(folders, path.first(path.size() - 1));
}

// How many levels the folder takes: 1, and the deepest of the folders in it.
std::size_t LevelsOf(const ColorFolder& folder)
{
    std::size_t deepest = 0;
    for (const ColorFolder& inner : folder.folders)
    {
        deepest = std::max(deepest, LevelsOf(inner));
    }
    return 1 + deepest;
}

// Puts the folders into the list; one of a name the list has already is joined with that one.
void JoinFolders(std::vector<ColorFolder>& list, std::vector<ColorFolder> incoming)
{
    for (ColorFolder& folder : incoming)
    {
        const auto same = std::find_if(list.begin(), list.end(),
                                       [&folder](const ColorFolder& listed) { return listed.name == folder.name; });
        if (same == list.end())
        {
            list.push_back(std::move(folder));
            continue;
        }
        same->colors.insert(same->colors.end(), folder.colors.begin(), folder.colors.end());
        JoinFolders(same->folders, std::move(folder.folders));
    }
}

bool FindColor(const std::vector<ColorFolder>& folders, std::string_view color, ColorFolderPath& path)
{
    for (std::size_t index = 0; index < folders.size(); ++index)
    {
        path.push_back(index);
        const ColorFolder& folder = folders[index];
        if (std::find(folder.colors.begin(), folder.colors.end(), color) != folder.colors.end() ||
            FindColor(folder.folders, color, path))
        {
            return true;
        }
        path.pop_back();
    }
    return false;
}

void CollectColors(const std::vector<ColorFolder>& folders, std::set<std::string, std::less<>>& colors)
{
    for (const ColorFolder& folder : folders)
    {
        colors.insert(folder.colors.begin(), folder.colors.end());
        CollectColors(folder.folders, colors);
    }
}

bool StartsWith(std::span<const std::size_t> path, std::span<const std::size_t> prefix)
{
    return path.size() >= prefix.size() && std::equal(prefix.begin(), prefix.end(), path.begin());
}
} // namespace

bool IsColorFolderName(std::string_view name)
{
    if (name.empty() || !Core::Text::IsValidUtf8(name))
    {
        return false;
    }
    // Blank, or with spaces around it, a name looks like another one or like none in the tree.
    if (name.front() == ' ' || name.back() == ' ' || std::any_of(name.begin(), name.end(), &IsControlCharacter))
    {
        return false;
    }
    std::string cut(name);
    return !Core::Text::CutToUtf16Length(cut, MaxColorFolderNameLength);
}

std::vector<ColorFolder> ReadColorFolders(const OrderedJson& root, const std::string& source,
                                          const std::function<bool(std::string_view)>& isColor,
                                          std::vector<ItemDataIssue>& issues)
{
    const auto list = root.find(ColorFoldersKey);
    if (list == root.end())
    {
        return {};
    }
    FolderReader reader(source, isColor, issues);
    return reader.ReadList(*list, ColorFoldersKey, 1);
}

OrderedJson WriteColorFolders(std::span<const ColorFolder> folders,
                              const std::function<bool(std::string_view)>& isColor)
{
    OrderedJson list = OrderedJson::array();
    for (const ColorFolder& folder : folders)
    {
        list.push_back(WriteFolder(folder, isColor));
    }
    return list;
}

std::string PutColorFoldersOnLines(const std::string& text)
{
    return Items::Json::PutListEntriesOnOneLine(text, ColorFoldersKey);
}

ColorFolder* FindColorFolder(std::vector<ColorFolder>& folders, std::span<const std::size_t> path)
{
    std::vector<ColorFolder>* level = &folders;
    ColorFolder* folder = nullptr;
    for (const std::size_t index : path)
    {
        if (index >= level->size())
        {
            return nullptr;
        }
        folder = &(*level)[index];
        level = &folder->folders;
    }
    return folder;
}

const ColorFolder* FindColorFolder(const std::vector<ColorFolder>& folders, std::span<const std::size_t> path)
{
    return FindColorFolder(const_cast<std::vector<ColorFolder>&>(folders), path);
}

std::vector<std::string> GetColorFolderNames(const std::vector<ColorFolder>& folders, std::span<const std::size_t> path)
{
    std::vector<std::string> names;
    const std::vector<ColorFolder>* level = &folders;
    for (const std::size_t index : path)
    {
        if (index >= level->size())
        {
            return {};
        }
        names.push_back((*level)[index].name);
        level = &(*level)[index].folders;
    }
    return names;
}

std::optional<ColorFolderPath> FindColorFolderByNames(const std::vector<ColorFolder>& folders,
                                                      std::span<const std::string> names)
{
    ColorFolderPath path;
    const std::vector<ColorFolder>* level = &folders;
    for (const std::string& name : names)
    {
        const auto found = std::find_if(level->begin(), level->end(),
                                        [&name](const ColorFolder& folder) { return folder.name == name; });
        if (found == level->end())
        {
            return std::nullopt;
        }
        path.push_back(static_cast<std::size_t>(found - level->begin()));
        level = &found->folders;
    }
    return path;
}

std::optional<ColorFolderPath> FindFolderOfColor(const std::vector<ColorFolder>& folders, std::string_view color)
{
    ColorFolderPath path;
    if (FindColor(folders, color, path))
    {
        return path;
    }
    return std::nullopt;
}

std::vector<std::string> GetRootColors(const std::vector<ColorFolder>& folders, std::span<const std::string> colors)
{
    std::set<std::string, std::less<>> placed;
    CollectColors(folders, placed);
    std::vector<std::string> root;
    std::copy_if(colors.begin(), colors.end(), std::back_inserter(root),
                 [&placed](const std::string& color) { return !placed.contains(color); });
    return root;
}

bool AddColorFolder(std::vector<ColorFolder>& folders, std::span<const std::size_t> parent, const std::string& name)
{
    std::vector<ColorFolder>* children = FindChildren(folders, parent);
    if (children == nullptr || !IsColorFolderName(name) || parent.size() + 1 > MaxColorFolderDepth ||
        HasFolderNamed(*children, name))
    {
        return false;
    }
    children->push_back({name, {}, {}});
    return true;
}

bool RenameColorFolder(std::vector<ColorFolder>& folders, std::span<const std::size_t> path, const std::string& name)
{
    std::vector<ColorFolder>* siblings = FindSiblings(folders, path);
    if (siblings == nullptr || !IsColorFolderName(name) || HasFolderNamed(*siblings, name))
    {
        return false;
    }
    (*siblings)[path.back()].name = name;
    return true;
}

bool RemoveColorFolder(std::vector<ColorFolder>& folders, std::span<const std::size_t> path)
{
    std::vector<ColorFolder>* siblings = FindSiblings(folders, path);
    if (siblings == nullptr)
    {
        return false;
    }
    ColorFolder removed = std::move((*siblings)[path.back()]);
    siblings->erase(siblings->begin() + static_cast<std::ptrdiff_t>(path.back()));
    // Under the root a color needs no list: one no folder holds sits there.
    if (ColorFolder* parent = FindColorFolder(folders, path.first(path.size() - 1)))
    {
        parent->colors.insert(parent->colors.end(), removed.colors.begin(), removed.colors.end());
    }
    JoinFolders(*siblings, std::move(removed.folders));
    return true;
}

bool MoveColorFolderInto(std::vector<ColorFolder>& folders, std::span<const std::size_t> path,
                         std::span<const std::size_t> parent)
{
    const ColorFolder* folder = FindColorFolder(folders, path);
    std::vector<ColorFolder>* target = FindChildren(folders, parent);
    if (folder == nullptr || target == nullptr || StartsWith(parent, path) ||
        std::equal(parent.begin(), parent.end(), path.begin(), path.end() - 1) ||
        parent.size() + LevelsOf(*folder) > MaxColorFolderDepth || HasFolderNamed(*target, folder->name))
    {
        return false;
    }
    ColorFolder moved = *folder;
    std::vector<ColorFolder>* siblings = FindSiblings(folders, path);
    siblings->erase(siblings->begin() + static_cast<std::ptrdiff_t>(path.back()));
    FindChildren(folders, GetPathAfterMove(path, parent))->push_back(std::move(moved));
    return true;
}

ColorFolderPath GetPathAfterMove(std::span<const std::size_t> path, std::span<const std::size_t> parent)
{
    ColorFolderPath to(parent.begin(), parent.end());
    if (path.empty())
    {
        return to;
    }
    const std::size_t level = path.size() - 1;
    if (to.size() > level && std::equal(path.begin(), path.begin() + static_cast<std::ptrdiff_t>(level), to.begin()) &&
        to[level] > path.back())
    {
        --to[level];
    }
    return to;
}

bool MoveColorFolderBy(std::vector<ColorFolder>& folders, std::span<const std::size_t> path, int delta)
{
    std::vector<ColorFolder>* siblings = FindSiblings(folders, path);
    if (siblings == nullptr)
    {
        return false;
    }
    const std::ptrdiff_t from = static_cast<std::ptrdiff_t>(path.back());
    const std::ptrdiff_t to = from + delta;
    if (delta == 0 || to < 0 || to >= static_cast<std::ptrdiff_t>(siblings->size()))
    {
        return false;
    }
    std::swap((*siblings)[static_cast<std::size_t>(from)], (*siblings)[static_cast<std::size_t>(to)]);
    return true;
}

bool MoveColorInto(std::vector<ColorFolder>& folders, std::string_view color, std::span<const std::size_t> folder)
{
    const std::optional<ColorFolderPath> current = FindFolderOfColor(folders, color);
    if (current && std::equal(current->begin(), current->end(), folder.begin(), folder.end()))
    {
        return false;
    }
    if (folder.empty())
    {
        return RemoveColorFromFolders(folders, color);
    }
    ColorFolder* target = FindColorFolder(folders, folder);
    if (target == nullptr)
    {
        return false;
    }
    RemoveColorFromFolders(folders, color);
    target->colors.emplace_back(color);
    return true;
}

bool MoveColorInFolderBy(std::vector<ColorFolder>& folders, std::string_view color, int delta)
{
    const std::optional<ColorFolderPath> path = FindFolderOfColor(folders, color);
    if (!path)
    {
        return false;
    }
    std::vector<std::string>& colors = FindColorFolder(folders, *path)->colors;
    const auto found = std::find(colors.begin(), colors.end(), color);
    const std::ptrdiff_t from = found - colors.begin();
    const std::ptrdiff_t to = from + delta;
    if (delta == 0 || to < 0 || to >= static_cast<std::ptrdiff_t>(colors.size()))
    {
        return false;
    }
    std::swap(colors[static_cast<std::size_t>(from)], colors[static_cast<std::size_t>(to)]);
    return true;
}

std::optional<std::vector<std::string>> MoveRootColorBy(const std::vector<ColorFolder>& folders,
                                                        std::span<const std::string> colors, std::string_view color,
                                                        int delta)
{
    const std::vector<std::string> roots = GetRootColors(folders, colors);
    const auto found = std::find(roots.begin(), roots.end(), color);
    if (found == roots.end() || delta == 0)
    {
        return std::nullopt;
    }
    const std::ptrdiff_t to = (found - roots.begin()) + delta;
    if (to < 0 || to >= static_cast<std::ptrdiff_t>(roots.size()))
    {
        return std::nullopt;
    }
    std::vector<std::string> order(colors.begin(), colors.end());
    const auto first = std::find(order.begin(), order.end(), color);
    const auto second = std::find(order.begin(), order.end(), roots[static_cast<std::size_t>(to)]);
    std::iter_swap(first, second);
    return order;
}

bool RemoveColorFromFolders(std::vector<ColorFolder>& folders, std::string_view color)
{
    for (ColorFolder& folder : folders)
    {
        const auto found = std::find(folder.colors.begin(), folder.colors.end(), color);
        if (found != folder.colors.end())
        {
            folder.colors.erase(found);
            return true;
        }
        if (RemoveColorFromFolders(folder.folders, color))
        {
            return true;
        }
    }
    return false;
}

void RenameColorInFolders(std::vector<ColorFolder>& folders, std::string_view from, const std::string& to)
{
    for (ColorFolder& folder : folders)
    {
        std::replace(folder.colors.begin(), folder.colors.end(), std::string(from), to);
        RenameColorInFolders(folder.folders, from, to);
    }
}
} // namespace Data::Colors
