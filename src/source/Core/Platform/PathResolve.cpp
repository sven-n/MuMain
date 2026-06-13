#ifndef _WIN32

#include "Core/Platform/PathResolve.h"

#include <dirent.h>
#include <strings.h>   // strcasecmp
#include <unistd.h>    // access

namespace
{
    // True if `dir`/`name` exists verbatim.
    bool EntryExists(const std::string& dir, const std::string& name)
    {
        const std::string full = dir.empty() ? name : dir + "/" + name;
        return ::access(full.c_str(), F_OK) == 0;
    }

    // Scans `dir` for an entry matching `name` case-insensitively; on a match
    // replaces `name` with the entry's actual spelling and returns true.
    bool FindCaseInsensitive(const std::string& dir, std::string& name)
    {
        DIR* d = ::opendir(dir.empty() ? "." : dir.c_str());
        if (!d) return false;
        bool found = false;
        while (const dirent* entry = ::readdir(d))
        {
            if (::strcasecmp(entry->d_name, name.c_str()) == 0)
            {
                name = entry->d_name;
                found = true;
                break;
            }
        }
        ::closedir(d);
        return found;
    }
}

std::string MuResolvePath(const char* utf8Path)
{
    if (!utf8Path) return {};

    std::string path(utf8Path);
    for (char& c : path)
        if (c == '\\') c = '/';

    // The common case: the normalized path exists as spelled.
    if (::access(path.c_str(), F_OK) == 0)
        return path;

    // Walk the components, case-correcting each against the directory that has
    // been resolved so far. `resolved` holds "" for a relative path so the
    // first component is looked up in the working directory.
    std::string resolved;
    size_t pos = 0;
    if (!path.empty() && path[0] == '/')
    {
        resolved = "/";
        pos = 1;
    }

    while (pos < path.size())
    {
        const size_t sep = path.find('/', pos);
        const size_t end = (sep == std::string::npos) ? path.size() : sep;
        std::string component = path.substr(pos, end - pos);

        if (!component.empty() && component != "." && component != ".." &&
            !EntryExists(resolved, component))
        {
            FindCaseInsensitive(resolved, component);
        }

        if (!resolved.empty() && resolved.back() != '/')
            resolved += '/';
        resolved += component;
        pos = end + 1;
    }

    return resolved;
}

#endif // !_WIN32
