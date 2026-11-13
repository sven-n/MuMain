#pragma once

#include <SDL3/SDL_storage.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>

// Content from a source of the test's own, as an SDL storage: what a port's backend (an APK, an archive, a download
// cache) looks like to the game. Either files held in memory, or the names of a folder's files, whose bytes are read
// only when the game asks for one. Content::Open takes the storage over; it frees this when it closes.
class ContentMemoryStorage
{
public:
    using Files = std::map<std::string, std::string>;

    // A storage over `files`, which the test may still change; storages may share them.
    static SDL_Storage* Open(std::shared_ptr<Files> files)
    {
        auto* self = new ContentMemoryStorage;
        self->m_files = std::move(files);
        return OpenSelf(self);
    }

    // A storage over the files under `root`: their names are listed now, their bytes read when asked for.
    static SDL_Storage* OpenFolder(const std::filesystem::path& root)
    {
        auto* self = new ContentMemoryStorage;
        self->m_root = root;
        std::error_code error;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(root, error))
        {
            std::error_code fileError;
            if (entry.is_regular_file(fileError))
            {
                const std::u8string name = entry.path().lexically_relative(root).generic_u8string();
                self->m_sizes[std::string(name.begin(), name.end())] = entry.file_size(fileError);
            }
        }
        return OpenSelf(self);
    }

private:
    ContentMemoryStorage() = default;

    static SDL_Storage* OpenSelf(ContentMemoryStorage* self)
    {
        static const SDL_StorageInterface iface = MakeInterface();
        return SDL_OpenStorage(&iface, self);
    }

    static std::string Prefix(const char* path)
    {
        return (path == nullptr || *path == '\0') ? std::string() : std::string(path) + "/";
    }

    // Calls `visit(name)` for each file name that starts with `prefix`, in order; stops when it returns false.
    template <typename Visit>
    void ForEachUnder(const std::string& prefix, Visit&& visit) const
    {
        const auto walk = [&](const auto& names)
        {
            for (auto it = names.lower_bound(prefix); it != names.end() && it->first.starts_with(prefix); ++it)
            {
                if (!visit(it->first))
                {
                    return;
                }
            }
        };
        if (m_files)
        {
            walk(*m_files);
        }
        else
        {
            walk(m_sizes);
        }
    }

    std::optional<Uint64> SizeOf(const char* path) const
    {
        if (m_files)
        {
            const auto found = m_files->find(path);
            return found == m_files->end() ? std::nullopt : std::optional<Uint64>(found->second.size());
        }
        const auto found = m_sizes.find(path);
        return found == m_sizes.end() ? std::nullopt : std::optional<Uint64>(found->second);
    }

    bool Read(const char* path, void* destination, Uint64 length) const
    {
        if (m_files)
        {
            const std::string& bytes = m_files->at(path);
            std::memcpy(destination, bytes.data(), bytes.size());
            return true;
        }
        const std::string name(path);
        const std::u8string utf8(name.begin(), name.end());
        std::ifstream file(m_root / std::filesystem::path(utf8), std::ios::binary);
        return file.read(static_cast<char*>(destination), static_cast<std::streamsize>(length)).good();
    }

    static SDL_StorageInterface MakeInterface()
    {
        SDL_StorageInterface iface;
        SDL_INIT_INTERFACE(&iface);
        iface.close = [](void* userdata)
        {
            delete static_cast<ContentMemoryStorage*>(userdata);
            return true;
        };
        iface.ready = [](void*) { return true; };
        iface.enumerate = [](void* userdata, const char* path, SDL_EnumerateDirectoryCallback callback,
                             void* callbackUserdata)
        {
            const auto& self = *static_cast<ContentMemoryStorage*>(userdata);
            const std::string prefix = Prefix(path);
            std::set<std::string> names;
            self.ForEachUnder(prefix,
                              [&](const std::string& name)
                              {
                                  const std::string rest = name.substr(prefix.size());
                                  names.insert(rest.substr(0, rest.find('/')));
                                  return true;
                              });
            for (const std::string& name : names)
            {
                if (callback(callbackUserdata, prefix.c_str(), name.c_str()) != SDL_ENUM_CONTINUE)
                {
                    break;
                }
            }
            return true;
        };
        iface.info = [](void* userdata, const char* path, SDL_PathInfo* info)
        {
            const auto& self = *static_cast<ContentMemoryStorage*>(userdata);
            *info = {};
            if (const std::optional<Uint64> size = self.SizeOf(path))
            {
                info->type = SDL_PATHTYPE_FILE;
                info->size = *size;
                return true;
            }
            bool folder = false;
            self.ForEachUnder(Prefix(path),
                              [&](const std::string&)
                              {
                                  folder = true;
                                  return false;
                              });
            if (folder)
            {
                info->type = SDL_PATHTYPE_DIRECTORY;
                return true;
            }
            return SDL_SetError("%s: no such file", path);
        };
        iface.read_file = [](void* userdata, const char* path, void* destination, Uint64 length)
        {
            const auto& self = *static_cast<ContentMemoryStorage*>(userdata);
            const std::optional<Uint64> size = self.SizeOf(path);
            if (!size || *size != length || !self.Read(path, destination, length))
            {
                return SDL_SetError("%s: cannot be read", path);
            }
            return true;
        };
        return iface;
    }

    std::shared_ptr<Files> m_files;
    std::filesystem::path m_root;
    std::map<std::string, Uint64> m_sizes;
};
