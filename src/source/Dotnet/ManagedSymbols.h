#pragma once

#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace DotNetBridge
{
// Where the network library's exports come from. Each target compiles one adapter (Dotnet/SymbolSources) that makes
// it: the NativeAOT library on desktop and Android, or whatever a port links the exports from.
struct ManagedSymbolSource
{
    // Names the source in messages: the library's path, say.
    std::string description;
    // An export's address, or null when the source lacks it. Null itself when there is no source, a missing library.
    void* (*find)(const char* name) = nullptr;
};

// The target's source, from its adapter. The game's startup passes it to InitializeDotNetBridge.
ManagedSymbolSource TargetManagedSymbolSource();

// An export the client looks up. A missing required one means the library doesn't match this client.
struct ManagedExport
{
    const char* name;
    bool optional = false;
};

// Every export the client looks up: the packet senders' and the connection's.
std::span<const ManagedExport> ManagedExports();

enum class ManagedSymbolsState
{
    NotResolved,
    Resolved,
    SourceMissing,
    ExportsMissing,
};

class ManagedSymbolTable
{
public:
    // Looks up every export once and logs each missing one. Only the first call does anything: the senders keep the
    // addresses they were given for the rest of the process.
    ManagedSymbolsState Resolve(const ManagedSymbolSource& source, std::span<const ManagedExport> exports);

    ManagedSymbolsState State() const
    {
        return m_state;
    }
    const std::string& Description() const
    {
        return m_description;
    }
    const std::vector<std::string>& MissingExports() const
    {
        return m_missing;
    }

    // The export's address; null unless the table resolved, and for a missing optional export.
    void* Find(std::string_view name) const;
    bool Lists(std::string_view name) const;

private:
    ManagedSymbolsState m_state = ManagedSymbolsState::NotResolved;
    std::string m_description;
    std::vector<std::string> m_missing;
    // Keyed by the export lists' names, which live as long as the process.
    std::unordered_map<std::string_view, void*> m_exports;
};

// The process's table, which InitializeDotNetBridge resolves before any connection.
ManagedSymbolTable& ProcessManagedSymbols();

// An export from the process's table, for LoadManagedSymbol. Null, and reported, when the network library is
// missing; null and logged before the table is resolved or for a name no list holds.
void* LookUpManagedSymbol(const char* name);
} // namespace DotNetBridge
