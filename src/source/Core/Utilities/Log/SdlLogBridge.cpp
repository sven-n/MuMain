#include "stdafx.h"

#include "SdlLogBridge.h"

#include "Core/Utilities/Log/MuLogger.h"

#include <SDL3/SDL.h>

namespace
{
SDL_LogOutputFunction g_previousSDLLogOutput = nullptr;
void* g_previousSDLLogUserdata = nullptr;
std::shared_ptr<spdlog::logger> g_sdlLogger;

// SDL_LOGGING selects the diagnostics to retain, so spdlog must not filter them again.
constexpr spdlog::level::level_enum kSDLLogForwardingLevel = spdlog::level::trace;

spdlog::level::level_enum ToSpdlogLogLevel(SDL_LogPriority priority)
{
    switch (priority)
    {
    case SDL_LOG_PRIORITY_TRACE:
        return spdlog::level::trace;
    case SDL_LOG_PRIORITY_VERBOSE:
    case SDL_LOG_PRIORITY_DEBUG:
        return spdlog::level::debug;
    case SDL_LOG_PRIORITY_INFO:
        return spdlog::level::info;
    case SDL_LOG_PRIORITY_WARN:
        return spdlog::level::warn;
    case SDL_LOG_PRIORITY_ERROR:
        return spdlog::level::err;
    case SDL_LOG_PRIORITY_CRITICAL:
        return spdlog::level::critical;
    case SDL_LOG_PRIORITY_INVALID:
    case SDL_LOG_PRIORITY_COUNT:
    default:
        return spdlog::level::debug;
    }
}

const char* GetSDLLogCategoryName(int category)
{
    // This covers SDL3's predefined categories; custom categories use the numeric fallback.
    switch (category)
    {
    case SDL_LOG_CATEGORY_APPLICATION:
        return "app";
    case SDL_LOG_CATEGORY_ERROR:
        return "error";
    case SDL_LOG_CATEGORY_ASSERT:
        return "assert";
    case SDL_LOG_CATEGORY_SYSTEM:
        return "system";
    case SDL_LOG_CATEGORY_AUDIO:
        return "audio";
    case SDL_LOG_CATEGORY_VIDEO:
        return "video";
    case SDL_LOG_CATEGORY_RENDER:
        return "render";
    case SDL_LOG_CATEGORY_INPUT:
        return "input";
    case SDL_LOG_CATEGORY_TEST:
        return "test";
    case SDL_LOG_CATEGORY_GPU:
        return "gpu";
    default:
        return nullptr;
    }
}

// Deliberately unlocked: SDL holds its log function lock across this call and
// across SDL_SetLogOutputFunction(), so invocations are serialized and none can
// run once the callback is restored. Taking a lock here would invert the order
// against SDL_SetLogOutputFunction() in the scoped installer and deadlock.
void SDLCALL ForwardSDLLog(void*, int category, SDL_LogPriority priority, const char* message)
{
    if (g_previousSDLLogOutput != nullptr)
        g_previousSDLLogOutput(g_previousSDLLogUserdata, category, priority, message);

    if (g_sdlLogger == nullptr)
        return;

    const char* categoryName = GetSDLLogCategoryName(category);
    if (categoryName != nullptr)
        g_sdlLogger->log(ToSpdlogLogLevel(priority), "{}: {}", categoryName, message);
    else
        g_sdlLogger->log(ToSpdlogLogLevel(priority), "SDL category {}: {}", category, message);
}
} // namespace

namespace Core::Log::Sdl
{
ScopedLogOutput::ScopedLogOutput()
{
    // The bridge has one owner: a nested instance stays inert instead of tearing
    // the outer one down when it goes out of scope.
    if (g_sdlLogger != nullptr)
        return;

    SDL_GetLogOutputFunction(&g_previousSDLLogOutput, &g_previousSDLLogUserdata);
    g_sdlLogger = mu::log::Get("sdl");
    g_sdlLogger->set_level(kSDLLogForwardingLevel);
    // Publishes the globals: SDL's lock orders this after any in-flight callback.
    SDL_SetLogOutputFunction(ForwardSDLLog, nullptr);
    m_installed = true;
}

ScopedLogOutput::~ScopedLogOutput()
{
    if (!m_installed)
        return;

    // Restore first so the last messages still reach the forwarder, then release.
    SDL_SetLogOutputFunction(g_previousSDLLogOutput, g_previousSDLLogUserdata);
    g_sdlLogger.reset();
    g_previousSDLLogOutput = nullptr;
    g_previousSDLLogUserdata = nullptr;
}
} // namespace Core::Log::Sdl
