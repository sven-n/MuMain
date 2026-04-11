// MuConsoleCommands — handles in-game `$` commands from the chat input.
// Story 7.10.1: Extracted from CmuConsoleDebug::CheckCommand, which is being deleted.
// [VS0-CORE-MIGRATE-LOGGING]

#include "stdafx.h"

#include "Core/MuConsoleCommands.h" // self

#include "MuLogger.h"

#include <charconv>

#include "RenderFX/ZzzOpenglUtil.h"
#include "Scenes/SceneManager.h"

#include "Data/GlobalBitmap.h"

#ifdef CSK_DEBUG_MAP_PATHFINDING
#include "ZzzPath.h"
#endif

// Forward declarations — avoids pulling in heavy headers
extern void SetMaxMessagePerCycle(int messages);
extern int EditFlag;
extern CGlobalBitmap Bitmaps;

#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
extern bool g_bRenderBoundingBox;
#endif

namespace mu::console
{

// Map a user-typed level string to spdlog enum.
static bool ParseLogLevel(const std::wstring& str, spdlog::level::level_enum& out)
{
    if (str == L"trace")
    {
        out = spdlog::level::trace;
    }
    else if (str == L"debug")
    {
        out = spdlog::level::debug;
    }
    else if (str == L"info")
    {
        out = spdlog::level::info;
    }
    else if (str == L"warn")
    {
        out = spdlog::level::warn;
    }
    else if (str == L"error" || str == L"err")
    {
        out = spdlog::level::err;
    }
    else if (str == L"critical" || str == L"crit")
    {
        out = spdlog::level::critical;
    }
    else if (str == L"off")
    {
        out = spdlog::level::off;
    }
    else
    {
        return false;
    }
    return true;
}

bool CheckCommand(const std::wstring& command)
{
    // --- FPS / vsync / overlay commands (always available) ---

    if (command == L"$fpscounter on")
    {
        SetShowFpsCounter(true);
        return true;
    }
    if (command == L"$fpscounter off")
    {
        SetShowFpsCounter(false);
        return true;
    }
    if (command == L"$details on")
    {
        SetShowDebugInfo(true);
        return true;
    }
    if (command == L"$details off")
    {
        SetShowDebugInfo(false);
        return true;
    }
    if (command.compare(0, 4, L"$fps") == 0 && command.size() > 5)
    {
        auto fps_str = command.substr(5);
        std::string narrow(fps_str.begin(), fps_str.end());
        char* end = nullptr;
        float target_fps = std::strtof(narrow.c_str(), &end);
        if (end == narrow.c_str())
        {
            mu::log::Get("core")->warn("Invalid FPS value: {}", narrow);
            return true;
        }
        SetTargetFps(target_fps);
        return true;
    }
    if (command == L"$vsync on")
    {
        EnableVSync();
        SetTargetFps(-1); // unlimited
        ResetFrameStats();
        return true;
    }
    if (command == L"$vsync off")
    {
        DisableVSync();
        ResetFrameStats();
        return true;
    }
    if (command.compare(0, 7, L"$winmsg") == 0 && command.size() > 8)
    {
        auto str_limit = command.substr(8);
        std::string narrow(str_limit.begin(), str_limit.end());
        int message_limit = 0;
        auto [ptr, ec] = std::from_chars(narrow.data(), narrow.data() + narrow.size(), message_limit);
        if (ec != std::errc{})
        {
            mu::log::Get("core")->warn("Invalid message limit: {}", narrow);
            return true;
        }
        SetMaxMessagePerCycle(message_limit);
        return true;
    }

    // --- Debug commands (conditional on feature flags) ---

#ifdef CSK_DEBUG_MAP_ATTRIBUTE
    if (command == L"$mapatt on")
    {
        EditFlag = EDIT_WALL;
        return true;
    }
    if (command == L"$mapatt off")
    {
        EditFlag = EDIT_NONE;
        return true;
    }
#endif

#ifdef CSK_DEBUG_MAP_PATHFINDING
    if (command == L"$path on")
    {
        g_bShowPath = true;
        return true;
    }
    if (command == L"$path off")
    {
        g_bShowPath = false;
        return true;
    }
#endif

#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
    if (command == L"$bb on")
    {
        g_bRenderBoundingBox = true;
        return true;
    }
    if (command == L"$bb off")
    {
        g_bRenderBoundingBox = false;
        return true;
    }
#endif

    // --- Texture info ---

    if (command == L"$texture_info")
    {
        mu::log::Get("render")->info("Texture count: {}, memory: {} KB", Bitmaps.GetNumberOfTexture(),
                                     Bitmaps.GetUsedTextureMemory() / 1024);
        return true;
    }

    // --- Logging commands (Task 8: runtime log-level control) ---

    // $loglevel <logger> <level>
    // Example: $loglevel render debug
    if (command.compare(0, 9, L"$loglevel") == 0 && command.size() > 10)
    {
        auto args = command.substr(10);
        auto space = args.find(L' ');
        if (space == std::wstring::npos)
        {
            mu::log::Get("core")->warn("Usage: $loglevel <logger> <level>");
            return true;
        }

        std::wstring wLoggerName = args.substr(0, space);
        std::wstring wLevel = args.substr(space + 1);

        // Convert wstring to string for spdlog API
        std::string loggerName(wLoggerName.begin(), wLoggerName.end());
        spdlog::level::level_enum level;
        if (!ParseLogLevel(wLevel, level))
        {
            mu::log::Get("core")->warn("Unknown log level '{}'. Use: trace, debug, info, warn, error, critical, off",
                                       std::string(wLevel.begin(), wLevel.end()));
            return true;
        }

        if (mu::log::SetLevel(loggerName, level))
        {
            mu::log::Get("core")->info("Logger '{}' level set to '{}'", loggerName,
                                       std::string(wLevel.begin(), wLevel.end()));
        }
        else
        {
            mu::log::Get("core")->warn("Logger '{}' not found. Use $loggers to list available loggers", loggerName);
        }
        return true;
    }

    // $loggers — list all registered loggers and their current levels
    if (command == L"$loggers")
    {
        auto loggers = mu::log::ListLoggers();
        mu::log::Get("core")->info("Registered loggers ({}):", loggers.size());
        for (const auto& [name, level] : loggers)
        {
            mu::log::Get("core")->info("  {} = {}", name, level);
        }
        return true;
    }

    return false;
}

} // namespace mu::console
