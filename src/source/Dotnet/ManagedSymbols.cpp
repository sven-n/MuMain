#include "stdafx.h"

#include "ManagedSymbols.h"

#include "Connection.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "PacketBindings_ChatServer.h"
#include "PacketBindings_ClientToServer.h"
#include "PacketBindings_ConnectServer.h"

namespace DotNetBridge
{
namespace
{
// The exports Connection.cpp and PacketFunctions_Custom.cpp look up; the packet senders' come from the generated
// PacketBindings_*.h. tests/check_network_bridge.py keeps these lists and the lookups the same.
constexpr ManagedExport kManagedExports_Connection[] = {
    {"ConnectionManager_BeginConnect"},
    {"ConnectionManager_Disconnect"},
    {"ConnectionManager_BeginReceive"},
    {"ConnectionManager_Send"},
    {"ConnectionManager_SetLogCallback"},
    // Libraries from before it lack it; the error report then says the version is unknown.
    {"ConnectionManager_GetLibraryVersion", true},
    {"ConnectionManager_SendLogin"},
    {"ConnectionManager_SendAuthenticateExt"},
    {"ConnectionManager_SendChatMessageExt"},
};

std::vector<ManagedExport> CollectExports()
{
    std::vector<ManagedExport> exports(std::begin(kManagedExports_Connection), std::end(kManagedExports_Connection));
    for (const std::span<const char* const> generated : {std::span<const char* const>(kManagedExports_ChatServer),
                                                         std::span<const char* const>(kManagedExports_ConnectServer),
                                                         std::span<const char* const>(kManagedExports_ClientToServer)})
    {
        for (const char* name : generated)
        {
            exports.push_back({name});
        }
    }
    return exports;
}
} // namespace

std::span<const ManagedExport> ManagedExports()
{
    static const std::vector<ManagedExport> exports = CollectExports();
    return exports;
}

ManagedSymbolsState ManagedSymbolTable::Resolve(const ManagedSymbolSource& source,
                                                std::span<const ManagedExport> exports)
{
    if (m_state != ManagedSymbolsState::NotResolved)
    {
        return m_state;
    }

    m_description = source.description;
    if (source.find == nullptr)
    {
        m_state = ManagedSymbolsState::SourceMissing;
        return m_state;
    }

    std::unordered_map<std::string_view, void*> found;
    for (const ManagedExport& managedExport : exports)
    {
        void* const address = source.find(managedExport.name);
        if (address == nullptr && !managedExport.optional)
        {
            mu::log::Get("dotnet")->error("NET: {} has no export {}", m_description, managedExport.name);
            m_missing.emplace_back(managedExport.name);
        }
        found.emplace(managedExport.name, address);
    }

    if (!m_missing.empty())
    {
        m_state = ManagedSymbolsState::ExportsMissing;
        return m_state;
    }
    m_exports = std::move(found);
    m_state = ManagedSymbolsState::Resolved;
    return m_state;
}

void* ManagedSymbolTable::Find(std::string_view name) const
{
    const auto entry = m_exports.find(name);
    return entry != m_exports.end() ? entry->second : nullptr;
}

bool ManagedSymbolTable::Lists(std::string_view name) const
{
    return m_exports.contains(name);
}

ManagedSymbolTable& ProcessManagedSymbols()
{
    static ManagedSymbolTable table;
    return table;
}

void* LookUpManagedSymbol(const char* name)
{
    const ManagedSymbolTable& table = ProcessManagedSymbols();
    switch (table.State())
    {
    case ManagedSymbolsState::NotResolved:
        mu::log::Get("dotnet")->error("NET: {} was looked up before the network library's exports were resolved", name);
        return nullptr;
    case ManagedSymbolsState::SourceMissing:
        ReportDotNetError(nullptr, DotNetErrorKind::LibraryNotFound);
        return nullptr;
    case ManagedSymbolsState::ExportsMissing:
        // Reported at startup.
        return nullptr;
    case ManagedSymbolsState::Resolved:
        break;
    }

    if (!table.Lists(name))
    {
        mu::log::Get("dotnet")->error("NET: {} is in no export list (Dotnet/ManagedSymbols.cpp)", name);
        return nullptr;
    }
    return table.Find(name);
}
} // namespace DotNetBridge
