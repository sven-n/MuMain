#include "stdafx.h"

#include "DrawSpawnCheck.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <map>
#include <mutex>
#include <set>
#include <tuple>

#if defined(MU_ENABLE_CONTROL_SOCKET) || !defined(NDEBUG)
#define MU_DRAW_SPAWN_LOG
#include "Core/Utilities/Log/ErrorReport.h"
#endif

#if defined(MU_DRAW_SPAWN_LOG) && defined(_WIN32)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

namespace Diagnostics::DrawSpawnCheck
{
namespace
{
constexpr int CallerDepth = 3;
using Callers = std::array<void*, CallerDepth>;

std::mutex s_mutex;
int s_drawDepth = 0;
int s_allowDepth = 0;
int s_violations = 0;
bool s_listing = false;
int s_draws = 0;
std::map<std::tuple<const char*, Callers>, int> s_counts;

#ifdef MU_DRAW_SPAWN_LOG
std::string FunctionAt(void* address)
{
#ifdef _WIN32
    static const bool symbols = []
    {
        SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        return SymInitialize(GetCurrentProcess(), nullptr, TRUE) == TRUE;
    }();
    if (symbols && address != nullptr)
    {
        alignas(SYMBOL_INFO) char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
        auto* symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;
        if (SymFromAddr(GetCurrentProcess(), reinterpret_cast<DWORD64>(address), nullptr, symbol))
            return symbol->Name;
    }
#endif
    char hex[32];
    snprintf(hex, sizeof(hex), "%p", address);
    return hex;
}

void LogViolation(const char* kind, const Callers& callers)
{
    static std::set<void*> logged;
    if (!logged.insert(callers[0]).second)
        return;
    g_ErrorReport.Write(L"[DrawSpawnCheck] the draw spawned a %hs from %hs; spawn it in the update\r\n", kind,
                        FunctionAt(callers[0]).c_str());
}
#endif
} // namespace

DrawScope::DrawScope()
{
    std::lock_guard lock(s_mutex);
    if (s_drawDepth++ == 0 && s_listing)
        ++s_draws;
}

DrawScope::~DrawScope()
{
    std::lock_guard lock(s_mutex);
    --s_drawDepth;
}

AllowScope::AllowScope()
{
    ++s_allowDepth;
}

AllowScope::~AllowScope()
{
    --s_allowDepth;
}

void Note([[maybe_unused]] const char* kind)
{
    if (s_drawDepth == 0)
        return;
    const bool violation = s_allowDepth == 0;
    if (violation)
        ++s_violations;
#ifdef MU_DRAW_SPAWN_LOG
    if (!violation && !s_listing)
        return;
    Callers callers{};
#ifdef _WIN32
    // Skips Note and the spawn function itself.
    CaptureStackBackTrace(2, CallerDepth, callers.data(), nullptr);
#endif
    std::lock_guard lock(s_mutex);
    if (violation)
        LogViolation(kind, callers);
#ifdef MU_ENABLE_CONTROL_SOCKET
    if (s_listing)
        ++s_counts[{kind, callers}];
#endif
#endif
}

int Violations()
{
    return s_violations;
}

void ResetViolations()
{
    s_violations = 0;
}

#ifdef MU_ENABLE_CONTROL_SOCKET
void Start()
{
    std::lock_guard lock(s_mutex);
    s_counts.clear();
    s_draws = 0;
    s_listing = true;
}

std::vector<Site> Report(int& draws)
{
    std::lock_guard lock(s_mutex);
    draws = s_draws;
    std::map<std::pair<std::string, std::string>, int> named;
    for (const auto& [key, count] : s_counts)
    {
        const auto& [kind, callers] = key;
        std::string chain;
        for (void* address : callers)
        {
            if (address == nullptr)
                break;
            if (!chain.empty())
                chain += " < ";
            chain += FunctionAt(address);
        }
        named[{kind, chain}] += count;
    }
    std::vector<Site> sites;
    for (const auto& [key, count] : named)
        sites.push_back({key.first, key.second, count});
    std::ranges::sort(sites, [](const Site& a, const Site& b) { return a.count > b.count; });
    return sites;
}
#endif
} // namespace Diagnostics::DrawSpawnCheck
