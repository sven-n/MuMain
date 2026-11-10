#include "stdafx.h"

#include "DrawSpawnCheck.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <set>
#include <tuple>

#include "Core/Utilities/Log/ErrorReport.h"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#else
#include <cxxabi.h>
#include <dlfcn.h>
#if defined(__APPLE__) || defined(__GLIBC__)
#include <execinfo.h>
#define MU_HAS_BACKTRACE 1
#endif
#endif

namespace Diagnostics::DrawSpawnCheck
{
namespace
{
constexpr int CallerDepth = 3;
using Callers = std::array<void*, CallerDepth>;

std::mutex s_mutex;
// Per thread: the frame draws on one thread, and what other threads do meanwhile is theirs.
thread_local int s_drawDepth = 0;
int s_violations = 0;
const char* s_lastKind = "";
void* s_lastCaller = nullptr;
bool s_listing = false;
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
#else
    Dl_info info{};
    if (address != nullptr && dladdr(address, &info) != 0 && info.dli_sname != nullptr)
    {
        int status = 0;
        char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
        std::string name = status == 0 ? demangled : info.dli_sname;
        free(demangled);
        return name;
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
    g_ErrorReport.Write(L"[DrawSpawnCheck] the draw did a %hs from %hs; do it in the update\r\n", kind,
                        FunctionAt(callers[0]).c_str());
}
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

void Note([[maybe_unused]] const char* kind)
{
    if (s_drawDepth == 0)
        return;
    // The noting function's callers, nearest first: skips Note and the noting function itself.
    Callers callers{};
#ifdef _WIN32
    CaptureStackBackTrace(2, CallerDepth, callers.data(), nullptr);
#elif defined(MU_HAS_BACKTRACE)
    std::array<void*, CallerDepth + 2> frames{};
    const int count = backtrace(frames.data(), static_cast<int>(frames.size()));
    for (int i = 2; i < count; ++i)
        callers[i - 2] = frames[i];
#endif
    std::lock_guard lock(s_mutex);
    ++s_violations;
    s_lastKind = kind;
    s_lastCaller = callers[0];
    LogViolation(kind, callers);
#ifdef MU_ENABLE_CONTROL_SOCKET
    if (s_listing)
        ++s_counts[{kind, callers}];
#endif
}

int Violations()
{
    std::lock_guard lock(s_mutex);
    return s_violations;
}

void ResetViolations()
{
    std::lock_guard lock(s_mutex);
    s_violations = 0;
    s_lastKind = "";
    s_lastCaller = nullptr;
}

Violation LastViolation()
{
    std::lock_guard lock(s_mutex);
    return {s_lastKind, s_lastCaller != nullptr ? FunctionAt(s_lastCaller) : std::string(), s_lastCaller};
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
