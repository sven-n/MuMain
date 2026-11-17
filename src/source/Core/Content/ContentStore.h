#pragma once

#include "Core/Content/ContentPath.h"

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

struct SDL_Storage;
struct SDL_IOStream;

// The game's content (Data/, fonts/, shaders/ ...) read by logical path from one read-only source: the client folder
// on desktop, the APK's assets on Android, whatever a port or a test plugs in. Lookups ignore case on every platform.
// Reads are safe from any thread; Open and Close are not, and run while nothing reads.
namespace Content
{
// Takes over `storage` (SDL_OpenTitleStorage, or SDL_OpenStorage with an interface of one's own) and closes the
// previous one. Without one, content is read relative to the working directory.
void Open(SDL_Storage* storage);
void Close();

// The file's bytes; nullopt when it is missing, a folder, or can't be read, with "<path>: <reason>" in `error`.
std::optional<std::string> ReadAll(const Path& path, std::string* error = nullptr);

// True for a file (not a folder).
bool Exists(const Path& path);

struct Entry
{
    std::string name;
    bool folder = false;
};

// What a folder holds, its names as stored, sorted; empty for a missing folder. An empty `folder` is the root.
std::vector<Entry> List(std::string_view folder);

// For code that takes any path (tests and the editor pass absolute ones): a relative path is read as content; an
// absolute one, or one that leaves the content, is a file on disk.
std::optional<std::string> ReadFile(const std::filesystem::path& path, std::string* error = nullptr);

struct StreamCloser
{
    void operator()(SDL_IOStream* stream) const noexcept;
};
// A stream over a file's bytes, for code that reads piece by piece or a library that takes a stream; it owns the
// bytes and frees them when it closes.
using Stream = std::unique_ptr<SDL_IOStream, StreamCloser>;
// Null when the file can't be read, with "<path>: <reason>" in `error`.
Stream OpenIO(const std::filesystem::path& path, std::string* error = nullptr);

enum class Found
{
    Nothing,
    File,
    Folder,
    // A path on disk whose existence can't be checked (an unreadable folder, a name too long).
    Unknown,
};
Found Probe(const std::filesystem::path& path);
// The folder's entries, sorted by name; empty for a missing folder.
std::vector<Entry> ListFolder(const std::filesystem::path& folder);

// Forgets the folder listings it holds, after something wrote or removed content. Writes are rare (editor saves),
// so it drops them all rather than finding the folder under every spelling.
void Refresh();

// Refreshes when it goes out of scope: the first line of a function that writes into the content, so every way out
// of it, a failed write included, leaves the layer seeing what is on disk.
struct RefreshAfterWrite
{
    RefreshAfterWrite() = default;
    RefreshAfterWrite(const RefreshAfterWrite&) = delete;
    RefreshAfterWrite& operator=(const RefreshAfterWrite&) = delete;
    ~RefreshAfterWrite()
    {
        Refresh();
    }
};
} // namespace Content
