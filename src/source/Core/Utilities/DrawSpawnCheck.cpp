#include "stdafx.h"

#include "DrawSpawnCheck.h"

#ifdef MU_ENABLE_CONTROL_SOCKET

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <tuple>

#ifdef _WIN32
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
bool s_enabled = false;
int s_drawDepth = 0;
int s_draws = 0;
std::map<std::tuple<const char*, Callers>, int> s_counts;

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
} // namespace

DrawScope::DrawScope()
{
    std::lock_guard lock(s_mutex);
    if (s_drawDepth++ == 0 && s_enabled)
        ++s_draws;
}

DrawScope::~DrawScope()
{
    std::lock_guard lock(s_mutex);
    --s_drawDepth;
}

void Note(const char* kind)
{
    if (!s_enabled || s_drawDepth == 0)
        return;
    Callers callers{};
#ifdef _WIN32
    // Skips Note and the spawn function itself.
    CaptureStackBackTrace(2, CallerDepth, callers.data(), nullptr);
#endif
    std::lock_guard lock(s_mutex);
    ++s_counts[{kind, callers}];
}

void Start()
{
    std::lock_guard lock(s_mutex);
    s_counts.clear();
    s_draws = 0;
    s_enabled = true;
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
} // namespace Diagnostics::DrawSpawnCheck

#endif
