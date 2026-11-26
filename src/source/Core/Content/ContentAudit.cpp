#include "Core/Content/ContentAudit.h"

#include "Core/Content/ContentStore.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include <atomic>
#include <mutex>
#include <optional>

namespace Content::Audit
{
namespace
{
using Files = std::set<std::string>;

struct State
{
    std::mutex mutex;
    Files shared;
    std::map<std::string, Files> monsters;
    // The files of the monsters spawned so far, and of the current map (`map`).
    Files spawned;
    Files mapFiles;
    int map = -1;
    // What was logged already.
    Files reported;
};

State& GetState()
{
    static State state;
    return state;
}

std::atomic<bool> s_active{false};

// Lower case, with '/' between folders: the lists' spelling does not decide a match.
std::string Unify(std::string_view path)
{
    std::string unified(path);
    for (char& c : unified)
    {
        if (c >= 'A' && c <= 'Z')
        {
            c = static_cast<char>(c - 'A' + 'a');
        }
        else if (c == '\\')
        {
            c = '/';
        }
    }
    return unified;
}

// Not in the lists by design: the lists themselves; sounds and music, which the repository does not hold, so a port
// ships both folders whole; an installed texture pack; the shaders, which the renderer reads.
bool IsExempt(const std::string& path)
{
    static constexpr std::string_view Prefixes[] = {"data/maps/files/", "data/sound/", "data/music/",
                                                    "data/texturepacks/", "shaders/"};
    for (const std::string_view prefix : Prefixes)
    {
        if (path.compare(0, prefix.size(), prefix) == 0)
        {
            return true;
        }
    }
    return false;
}

std::map<std::string, Files> ReadLists(const std::string& file)
{
    const std::optional<Path> path = Path::From(file);
    const std::optional<std::string> text = path ? ReadAll(*path) : std::nullopt;
    return text ? ParseLists(*text) : std::map<std::string, Files>{};
}

bool IsSpace(char c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}
} // namespace

std::map<std::string, std::set<std::string>> ParseLists(std::string_view text)
{
    std::map<std::string, Files> lists;
    std::string key;
    std::size_t at = 0;
    while (at < text.size())
    {
        if (text[at] != '"')
        {
            ++at;
            continue;
        }
        std::string value;
        for (++at; at < text.size() && text[at] != '"'; ++at)
        {
            if (text[at] == '\\' && at + 1 < text.size())
            {
                ++at;
            }
            value += text[at];
        }
        ++at;
        std::size_t next = at;
        while (next < text.size() && IsSpace(text[next]))
        {
            ++next;
        }
        if (next < text.size() && text[next] == ':')
        {
            key = value;
            lists[key];
        }
        else if (!key.empty())
        {
            lists[key].insert(Unify(value));
        }
    }
    return lists;
}

bool Start()
{
    Stop();
    std::map<std::string, Files> shared = ReadLists("Data/Maps/Files/Shared.json");
    if (shared["files"].empty())
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Content audit: Data/Maps/Files/Shared.json can't be read; no audit");
        return false;
    }
    std::map<std::string, Files> monsters = ReadLists("Data/Maps/Files/Monsters.json");

    State& state = GetState();
    {
        std::lock_guard lock(state.mutex);
        state.shared = std::move(shared["files"]);
        state.monsters = std::move(monsters);
    }
    s_active = true;
    SDL_Log("Content audit: on, a read missing from the file lists is logged once per file");
    return true;
}

bool StartFromEnvironment()
{
    const char* mode = SDL_getenv("MU_CONTENT_LISTS");
    return mode != nullptr && SDL_strcmp(mode, "audit") == 0 && Start();
}

void Stop()
{
    s_active = false;
    State& state = GetState();
    std::lock_guard lock(state.mutex);
    state.shared.clear();
    state.monsters.clear();
    state.spawned.clear();
    state.mapFiles.clear();
    state.reported.clear();
    state.map = -1;
}

bool Active()
{
    return s_active;
}

void EnterMap(int map)
{
    if (!s_active)
    {
        return;
    }
    // Read before the lock: the read is itself a content read.
    std::map<std::string, Files> lists = ReadLists("Data/Maps/Files/Map" + std::to_string(map) + ".json");
    State& state = GetState();
    std::lock_guard lock(state.mutex);
    state.map = map;
    state.mapFiles = std::move(lists["files"]);
}

void NoteMonster(int type)
{
    if (!s_active)
    {
        return;
    }
    State& state = GetState();
    std::lock_guard lock(state.mutex);
    const auto found = state.monsters.find(std::to_string(type));
    if (found != state.monsters.end())
    {
        state.spawned.insert(found->second.begin(), found->second.end());
    }
}

void NoteRead(std::string_view path)
{
    if (!s_active)
    {
        return;
    }
    const std::string key = Unify(path);
    if (IsExempt(key))
    {
        return;
    }
    State& state = GetState();
    int map = -1;
    {
        std::lock_guard lock(state.mutex);
        if (state.shared.count(key) || state.mapFiles.count(key) || state.spawned.count(key) ||
            !state.reported.insert(key).second)
        {
            return;
        }
        map = state.map;
    }
    if (map < 0)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Content audit: %s is read outside the file lists (no map yet)",
                    std::string(path).c_str());
    }
    else
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Content audit: %s is read outside the file lists on map %d",
                    std::string(path).c_str(), map);
    }
}
} // namespace Content::Audit
