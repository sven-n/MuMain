#include "stdafx.h"

#ifdef _EDITOR

#include "SoundFolderTree.h"

#include <algorithm>
#include <iterator>
#include <set>

namespace MuEditor::SoundEditor
{
namespace
{
using Data::Sounds::SoundFolder;

bool HasName(const std::vector<SoundFolder>& siblings, const std::string& name)
{
    return std::any_of(siblings.begin(), siblings.end(),
                       [&name](const SoundFolder& folder) { return folder.name == name; });
}

// The folders that hold the path's last folder: the top for a path of one.
std::vector<SoundFolder>* SiblingsOf(std::vector<SoundFolder>& folders, const FolderPath& path)
{
    if (path.empty())
    {
        return nullptr;
    }
    if (path.size() == 1)
    {
        return &folders;
    }
    SoundFolder* parent = FindFolder(folders, FolderPath(path.begin(), path.end() - 1));
    return parent != nullptr ? &parent->folders : nullptr;
}

std::vector<SoundFolder>::iterator Position(std::vector<SoundFolder>& siblings, const std::string& name)
{
    return std::find_if(siblings.begin(), siblings.end(),
                        [&name](const SoundFolder& folder) { return folder.name == name; });
}

bool TakeSound(std::vector<SoundFolder>& folders, const std::string& key)
{
    for (SoundFolder& folder : folders)
    {
        const auto found = std::find(folder.sounds.begin(), folder.sounds.end(), key);
        if (found != folder.sounds.end())
        {
            folder.sounds.erase(found);
            return true;
        }
        if (TakeSound(folder.folders, key))
        {
            return true;
        }
    }
    return false;
}

size_t DropStale(std::vector<SoundFolder>& folders, const std::set<std::string>& keys)
{
    size_t dropped = 0;
    for (SoundFolder& folder : folders)
    {
        const size_t before = folder.sounds.size();
        std::erase_if(folder.sounds, [&keys](const std::string& key) { return !keys.contains(key); });
        dropped += before - folder.sounds.size() + DropStale(folder.folders, keys);
    }
    return dropped;
}

// An entry of a key a renamed sound takes (`taken`) that was not renamed is
// one of a sound that is gone: it goes, so the sound is in one folder.
bool Rename(std::vector<SoundFolder>& folders, const std::map<std::string, std::string>& renamed,
            const std::set<std::string>& taken)
{
    bool any = false;
    for (SoundFolder& folder : folders)
    {
        for (auto sound = folder.sounds.begin(); sound != folder.sounds.end();)
        {
            const auto found = renamed.find(*sound);
            if (found != renamed.end())
            {
                *sound = found->second;
                ++sound;
                any = true;
            }
            else if (taken.contains(*sound))
            {
                sound = folder.sounds.erase(sound);
                any = true;
            }
            else
            {
                ++sound;
            }
        }
        any = Rename(folder.folders, renamed, taken) || any;
    }
    return any;
}
} // namespace

SoundFolder* FindFolder(std::vector<SoundFolder>& folders, const FolderPath& path)
{
    std::vector<SoundFolder>* level = &folders;
    SoundFolder* found = nullptr;
    for (const std::string& name : path)
    {
        const auto next = Position(*level, name);
        if (next == level->end())
        {
            return nullptr;
        }
        found = &*next;
        level = &found->folders;
    }
    return found;
}

std::string FreeFolderName(const std::vector<SoundFolder>& siblings, const std::string& base)
{
    if (!HasName(siblings, base))
    {
        return base;
    }
    // "Old 2" taken: "Old 3", not "Old 2 2".
    std::string stem = base;
    int number = 2;
    const size_t space = base.find_last_of(' ');
    if (space != std::string::npos && space + 1 < base.size() &&
        base.find_first_not_of("0123456789", space + 1) == std::string::npos && base.size() - space <= 6)
    {
        stem = base.substr(0, space);
        number = std::stoi(base.substr(space + 1)) + 1;
    }
    for (;; ++number)
    {
        const std::string name = stem + " " + std::to_string(number);
        if (!HasName(siblings, name))
        {
            return name;
        }
    }
}

FolderPath AddFolder(std::vector<SoundFolder>& folders, const FolderPath& parent, const std::string& base)
{
    std::vector<SoundFolder>* siblings = &folders;
    if (!parent.empty())
    {
        SoundFolder* folder = FindFolder(folders, parent);
        if (folder == nullptr)
        {
            return {};
        }
        siblings = &folder->folders;
    }
    SoundFolder added;
    added.name = FreeFolderName(*siblings, base);
    FolderPath path = parent;
    path.push_back(added.name);
    siblings->push_back(std::move(added));
    return path;
}

bool RenameFolder(std::vector<SoundFolder>& folders, const FolderPath& path, const std::string& name)
{
    std::vector<SoundFolder>* siblings = SiblingsOf(folders, path);
    if (siblings == nullptr || name.empty() || HasName(*siblings, name))
    {
        return false;
    }
    const auto folder = Position(*siblings, path.back());
    if (folder == siblings->end())
    {
        return false;
    }
    folder->name = name;
    return true;
}

bool RemoveFolder(std::vector<SoundFolder>& folders, const FolderPath& path)
{
    std::vector<SoundFolder>* siblings = SiblingsOf(folders, path);
    if (siblings == nullptr)
    {
        return false;
    }
    const auto at = Position(*siblings, path.back());
    if (at == siblings->end())
    {
        return false;
    }
    SoundFolder removed = std::move(*at);
    const auto index = static_cast<size_t>(at - siblings->begin());
    siblings->erase(at);
    // Its folders take its place, then its sounds go to the parent (the top
    // has no sounds: they are not sorted then).
    std::vector<SoundFolder> moved;
    for (SoundFolder& child : removed.folders)
    {
        // Free among the siblings and the folders moved before it.
        std::vector<SoundFolder> taken = *siblings;
        taken.insert(taken.end(), moved.begin(), moved.end());
        child.name = FreeFolderName(taken, child.name);
        moved.push_back(std::move(child));
    }
    siblings->insert(siblings->begin() + static_cast<std::ptrdiff_t>(index), std::make_move_iterator(moved.begin()),
                     std::make_move_iterator(moved.end()));
    if (path.size() > 1)
    {
        SoundFolder* parent = FindFolder(folders, FolderPath(path.begin(), path.end() - 1));
        parent->sounds.insert(parent->sounds.end(), removed.sounds.begin(), removed.sounds.end());
    }
    return true;
}

bool MoveFolder(std::vector<SoundFolder>& folders, const FolderPath& path, int by)
{
    std::vector<SoundFolder>* siblings = SiblingsOf(folders, path);
    if (siblings == nullptr)
    {
        return false;
    }
    const auto at = Position(*siblings, path.back());
    if (at == siblings->end())
    {
        return false;
    }
    const auto index = at - siblings->begin();
    const auto to = index + by;
    if (to < 0 || to >= static_cast<std::ptrdiff_t>(siblings->size()))
    {
        return false;
    }
    std::swap((*siblings)[static_cast<size_t>(index)], (*siblings)[static_cast<size_t>(to)]);
    return true;
}

bool MoveSound(std::vector<SoundFolder>& folders, const std::string& key, const FolderPath& to)
{
    SoundFolder* target = to.empty() ? nullptr : FindFolder(folders, to);
    if (!to.empty() && target == nullptr)
    {
        return false;
    }
    if (target != nullptr && std::find(target->sounds.begin(), target->sounds.end(), key) != target->sounds.end())
    {
        return false;
    }
    const bool taken = TakeSound(folders, key);
    if (target == nullptr)
    {
        return taken;
    }
    // FindFolder again: taking the sound did not move any folder.
    FindFolder(folders, to)->sounds.push_back(key);
    return true;
}

bool RenameSounds(std::vector<SoundFolder>& folders, const std::map<std::string, std::string>& renamed)
{
    std::set<std::string> taken;
    for (const auto& entry : renamed)
    {
        taken.insert(entry.second);
    }
    return Rename(folders, renamed, taken);
}

size_t DropStaleSounds(std::vector<SoundFolder>& folders, const std::vector<std::string>& keys)
{
    return DropStale(folders, std::set<std::string>(keys.begin(), keys.end()));
}

size_t CountStaleSounds(const std::vector<SoundFolder>& folders, const std::set<std::string>& keys)
{
    size_t stale = 0;
    for (const SoundFolder& folder : folders)
    {
        stale += static_cast<size_t>(std::count_if(folder.sounds.begin(), folder.sounds.end(),
                                                   [&keys](const std::string& key) { return !keys.contains(key); })) +
                 CountStaleSounds(folder.folders, keys);
    }
    return stale;
}
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
