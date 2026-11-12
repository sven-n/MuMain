#include "Core/Content/ContentStore.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_storage.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>

namespace Content
{
namespace
{
// A folder's names by their lower case, each to its stored spelling.
using Listing = std::unordered_map<std::string, std::string>;

struct State
{
    SDL_Storage* storage = nullptr;
    std::shared_mutex mutex;
    // By the folder's stored spelling ("" for the root).
    std::unordered_map<std::string, Listing> listings;
    // Counts Refresh and Close calls, so a listing taken before one isn't kept after it.
    std::uint64_t generation = 0;
};

State& GetState()
{
    static State state;
    return state;
}

// shortcut: never closed, it lives until exit; only tests and tools read content without opening any.
SDL_Storage* WorkingDirectory()
{
    // "./", not "": SDL lists an empty path's root as the drives on Windows.
    static SDL_Storage* const storage = SDL_OpenTitleStorage("./", 0);
    return storage;
}

SDL_Storage* CurrentStorage(const State& state)
{
    return state.storage ? state.storage : WorkingDirectory();
}

std::optional<Path> AsContent(const std::filesystem::path& path)
{
    if (!path.is_relative())
    {
        return std::nullopt;
    }
    return Path::From(reinterpret_cast<const char*>(path.generic_u8string().c_str()));
}

// ASCII only, whatever the C locale: names are UTF-8, and their other bytes stay as they are.
std::string Lower(std::string_view text)
{
    std::string lower(text);
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](char c) { return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c; });
    return lower;
}

std::string Join(std::string_view folder, std::string_view name)
{
    return folder.empty() ? std::string(name) : std::string(folder) + "/" + std::string(name);
}

std::vector<std::string> EnumerateNames(SDL_Storage* storage, const std::string& folder)
{
    std::vector<std::string> names;
    SDL_EnumerateStorageDirectory(
        storage, folder.c_str(),
        [](void* userdata, const char*, const char* name) -> SDL_EnumerationResult
        {
            static_cast<std::vector<std::string>*>(userdata)->emplace_back(name);
            return SDL_ENUM_CONTINUE;
        },
        &names);
    std::sort(names.begin(), names.end());
    return names;
}

Listing MakeListing(SDL_Storage* storage, const std::string& folder)
{
    Listing listing;
    for (std::string& stored : EnumerateNames(storage, folder))
    {
        // Sorted, so the first spelling wins on every platform.
        const auto [existing, added] = listing.try_emplace(Lower(stored), stored);
        if (!added)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Content: %s and %s differ only in case; reading %s",
                        Join(folder, existing->second).c_str(), Join(folder, stored).c_str(),
                        Join(folder, existing->second).c_str());
        }
    }
    return listing;
}

std::optional<std::string> Lookup(const Listing& listing, const std::string& key)
{
    const auto found = listing.find(key);
    return found == listing.end() ? std::nullopt : std::optional<std::string>(found->second);
}

// The stored spelling of `name` in `folder` (stored spelling), ignoring case. An opened storage's folders are listed
// once; the working directory's every time, since a test may have moved it.
std::optional<std::string> FindStored(State& state, const std::string& folder, std::string_view name)
{
    const std::string key = Lower(name);
    if (state.storage == nullptr)
    {
        return Lookup(MakeListing(WorkingDirectory(), folder), key);
    }

    std::uint64_t generation = 0;
    {
        std::shared_lock lock(state.mutex);
        if (const auto listing = state.listings.find(folder); listing != state.listings.end())
        {
            return Lookup(listing->second, key);
        }
        generation = state.generation;
    }

    Listing listing = MakeListing(state.storage, folder);
    std::optional<std::string> stored = Lookup(listing, key);
    std::unique_lock lock(state.mutex);
    if (state.generation == generation)
    {
        state.listings.try_emplace(folder, std::move(listing));
    }
    return stored;
}

// The stored spelling of a logical path: as written when that exists, else found folder by folder ignoring case.
std::optional<std::string> Resolve(State& state, std::string_view logical, SDL_PathInfo& info)
{
    const std::string written(logical);
    SDL_Storage* const storage = CurrentStorage(state);
    if (storage == nullptr)
    {
        return std::nullopt;
    }
    if (SDL_GetStoragePathInfo(storage, written.c_str(), &info))
    {
        return written;
    }

    std::string resolved;
    std::size_t start = 0;
    while (start < logical.size())
    {
        std::size_t end = logical.find('/', start);
        if (end == std::string_view::npos)
        {
            end = logical.size();
        }
        const std::optional<std::string> stored = FindStored(state, resolved, logical.substr(start, end - start));
        if (!stored)
        {
            return std::nullopt;
        }
        resolved = Join(resolved, *stored);
        start = end + 1;
    }
    if (!SDL_GetStoragePathInfo(storage, resolved.c_str(), &info))
    {
        return std::nullopt;
    }
    return resolved;
}
} // namespace

void Open(SDL_Storage* storage)
{
    Close();
    GetState().storage = storage;
}

void Close()
{
    State& state = GetState();
    if (state.storage != nullptr)
    {
        SDL_CloseStorage(state.storage);
        state.storage = nullptr;
    }
    state.listings.clear();
    ++state.generation;
}

std::optional<std::string> ReadAll(const Path& path, std::string* error)
{
    State& state = GetState();
    SDL_PathInfo info{};
    const std::optional<std::string> stored = Resolve(state, path.str(), info);
    const char* reason = nullptr;
    if (!stored)
    {
        reason = "not found";
    }
    else if (info.type != SDL_PATHTYPE_FILE)
    {
        reason = "not a file";
    }
    else
    {
        std::string bytes(static_cast<std::size_t>(info.size), '\0');
        if (bytes.empty() || SDL_ReadStorageFile(CurrentStorage(state), stored->c_str(), bytes.data(), info.size))
        {
            return bytes;
        }
        reason = SDL_GetError();
    }
    if (error != nullptr)
    {
        *error = path.str() + ": " + reason;
    }
    return std::nullopt;
}

bool Exists(const Path& path)
{
    SDL_PathInfo info{};
    return Resolve(GetState(), path.str(), info) && info.type == SDL_PATHTYPE_FILE;
}

std::vector<Entry> List(std::string_view folder)
{
    State& state = GetState();
    std::string stored;
    if (!folder.empty())
    {
        const std::optional<Path> logical = Path::From(folder);
        SDL_PathInfo info{};
        const std::optional<std::string> resolved = logical ? Resolve(state, logical->str(), info) : std::nullopt;
        if (!resolved || info.type != SDL_PATHTYPE_DIRECTORY)
        {
            return {};
        }
        stored = *resolved;
    }

    SDL_Storage* const storage = CurrentStorage(state);
    std::vector<Entry> entries;
    for (std::string& name : EnumerateNames(storage, stored))
    {
        SDL_PathInfo info{};
        const bool folderEntry =
            SDL_GetStoragePathInfo(storage, Join(stored, name).c_str(), &info) && info.type == SDL_PATHTYPE_DIRECTORY;
        entries.push_back({std::move(name), folderEntry});
    }
    return entries;
}

std::optional<std::string> ReadFile(const std::filesystem::path& path)
{
    if (const std::optional<Path> content = AsContent(path))
    {
        return ReadAll(*content);
    }
    // Only a regular file: on Linux a folder opens, and reading it throws.
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
    {
        return std::nullopt;
    }
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return std::nullopt;
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

Found Probe(const std::filesystem::path& path)
{
    if (const std::optional<Path> content = AsContent(path))
    {
        SDL_PathInfo info{};
        if (!Resolve(GetState(), content->str(), info))
        {
            return Found::Nothing;
        }
        if (info.type == SDL_PATHTYPE_DIRECTORY)
        {
            return Found::Folder;
        }
        return info.type == SDL_PATHTYPE_FILE ? Found::File : Found::Nothing;
    }
    std::error_code error;
    if (!std::filesystem::exists(path, error))
    {
        return error ? Found::Unknown : Found::Nothing;
    }
    if (std::filesystem::is_directory(path, error))
    {
        return Found::Folder;
    }
    return std::filesystem::is_regular_file(path, error) ? Found::File : Found::Unknown;
}

std::vector<Entry> ListFolder(const std::filesystem::path& folder)
{
    if (const std::optional<Path> content = AsContent(folder))
    {
        return List(content->str());
    }
    std::vector<Entry> entries;
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator(folder, error))
    {
        std::error_code typeError;
        entries.push_back(
            {reinterpret_cast<const char*>(entry.path().filename().u8string().c_str()), entry.is_directory(typeError)});
    }
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) { return a.name < b.name; });
    return entries;
}

void Refresh()
{
    State& state = GetState();
    std::unique_lock lock(state.mutex);
    state.listings.clear();
    ++state.generation;
}
} // namespace Content
