#include <doctest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <cstdio>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "Core/Platform/WinCompat.h"
#include "Core/Platform/SecureCrt.h"
#include "Core/Platform/WinApiShims.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Utilities/Log/SdlLogBridge.h"

#include <SDL3/SDL.h>

std::shared_ptr<spdlog::logger> GetEarlyLogger();
std::string GetEarlyLine();

namespace
{
std::filesystem::path TestDirectory(const char* name)
{
    return std::filesystem::temp_directory_path() /
           (std::string(name) + "_" + std::to_string(GetCurrentProcessId()));
}

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream file(path);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

class StreamCapture
{
public:
    explicit StreamCapture(FILE* stream = stdout)
        : m_stream(stream),
          m_capture(std::tmpfile())
    {
        REQUIRE(m_capture != nullptr);
        std::fflush(m_stream);
#ifdef _WIN32
        m_streamCopy = _dup(_fileno(m_stream));
        REQUIRE(m_streamCopy != -1);
        REQUIRE(_dup2(_fileno(m_capture), _fileno(m_stream)) == 0);
#else
        m_streamCopy = dup(fileno(m_stream));
        REQUIRE(m_streamCopy != -1);
        REQUIRE(dup2(fileno(m_capture), fileno(m_stream)) != -1);
#endif
    }

    ~StreamCapture()
    {
        std::fflush(m_stream);
#ifdef _WIN32
        _dup2(m_streamCopy, _fileno(m_stream));
        _close(m_streamCopy);
#else
        dup2(m_streamCopy, fileno(m_stream));
        close(m_streamCopy);
#endif
        std::fclose(m_capture);
    }

    std::string Read() const
    {
        std::fflush(m_stream);
        std::fseek(m_capture, 0, SEEK_END);
        const long size = std::ftell(m_capture);
        std::rewind(m_capture);
        std::string output(static_cast<std::size_t>(size), '\0');
        output.resize(std::fread(output.data(), 1, output.size(), m_capture));
        return output;
    }

private:
    FILE* m_stream = nullptr;
    FILE* m_capture = nullptr;
    int m_streamCopy = -1;
};
} // namespace

TEST_CASE("early logger adopts explicit log directory")
{
    const auto directory = TestDirectory("mu_logger_test");
    const auto ignoredDirectory = TestDirectory("mu_logger_test_ignored");
    std::filesystem::remove_all(directory);
    std::filesystem::remove_all(ignoredDirectory);

    const auto early = GetEarlyLogger();
    REQUIRE(early != nullptr);
    mu::log::Init(directory);
    mu::log::Init(ignoredDirectory);
    const auto core = mu::log::Get("core");
    const auto network = mu::log::Get("network");
    CHECK(core != nullptr);
    CHECK(network != nullptr);
    CHECK(core == early);
    CHECK(core != network);
    MU_LOG_ERROR(early, "mu logger early-init self-check");
    early->flush();
    mu::log::Shutdown();

    // The line logged during static init reaches the folder Init names, ahead of later lines, and nothing else.
    const std::string staticInitLine = GetEarlyLine();
    const std::string logContents = ReadFile(directory / "MuError.log");
    const auto earlyAt = logContents.find(staticInitLine);
    CHECK(earlyAt != std::string::npos);
    CHECK(earlyAt < logContents.find("mu logger early-init self-check"));
    CHECK(logContents.find("mu logger early-init self-check") != std::string::npos);
    CHECK(ReadFile(std::filesystem::current_path() / "MuError.log").find(staticInitLine) == std::string::npos);
    CHECK_FALSE(std::filesystem::exists(ignoredDirectory));
    std::filesystem::remove_all(directory);
}

TEST_CASE("logger shutdown allows explicit reinitialization")
{
    const auto firstDirectory = TestDirectory("mu_logger_reinit_first");
    const auto secondDirectory = TestDirectory("mu_logger_reinit_second");
    std::filesystem::remove_all(firstDirectory);
    std::filesystem::remove_all(secondDirectory);

    mu::log::Init(firstDirectory);
    mu::log::Shutdown();
    mu::log::Init(secondDirectory);
    const auto logger = mu::log::Get("core");
    REQUIRE(logger != nullptr);
    MU_LOG_ERROR(logger, "mu logger reinit self-check");
    logger->flush();
    mu::log::Shutdown();

    const std::string logContents = ReadFile(secondDirectory / "MuError.log");
    CHECK(logContents.find("mu logger reinit self-check") != std::string::npos);
    std::filesystem::remove_all(firstDirectory);
    std::filesystem::remove_all(secondDirectory);
}

TEST_CASE("logger writes to MuError.log without console output")
{
    const auto directory = TestDirectory("mu_logger_file_only");
    std::filesystem::remove_all(directory);

    std::string consoleOutput;
    {
        StreamCapture capture;
        mu::log::Init(directory);
        const auto logger = mu::log::Get("core");
        MU_LOG_ERROR(logger, "file-only logger self-check");
        logger->flush();
        mu::log::Shutdown();
        consoleOutput = capture.Read();
    }

    const std::string logContents = ReadFile(directory / "MuError.log");
    CHECK(logContents.find("file-only logger self-check") != std::string::npos);
    CHECK(consoleOutput.empty());
    std::filesystem::remove_all(directory);
}

TEST_CASE("SDL bridge writes named diagnostics to MuError.log")
{
    const auto directory = TestDirectory("sdl_log_bridge");
    std::filesystem::remove_all(directory);

    mu::log::Init(directory);
    {
        SDL_SetLogPriority(SDL_LOG_CATEGORY_VIDEO, SDL_LOG_PRIORITY_INFO);
        Core::Log::Sdl::ScopedLogOutput logOutput;
        SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "SDL log bridge self-check");
        {
            Core::Log::Sdl::ScopedLogOutput nested;
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "SDL log bridge survives a nested guard");
    }
    mu::log::Shutdown();

    SDL_ResetLogPriorities();
    const std::string logContents = ReadFile(directory / "MuError.log");
    CHECK(logContents.find("[sdl] [info] video: SDL log bridge self-check") != std::string::npos);
    CHECK(logContents.find("[sdl] [info] video: SDL log bridge survives a nested guard") != std::string::npos);
    std::filesystem::remove_all(directory);
}

TEST_CASE("logger keeps the first early lines and counts the rest")
{
    const auto directory = TestDirectory("mu_logger_early_cap");
    std::filesystem::remove_all(directory);
    // Start with no early lines: run alone, this case would also hold the static-init line.
    mu::log::Init(directory / "before");
    mu::log::Shutdown();

    std::string echoed;
    {
        StreamCapture capture(stderr);
        const auto logger = mu::log::Get("core");
        for (int line = 0; line < 1030; ++line)
            MU_LOG_INFO(logger, "early line {:04}", line);
        mu::log::Init(directory);
        mu::log::Shutdown();
        echoed = capture.Read();
    }

    const std::string logContents = ReadFile(directory / "MuError.log");
    REQUIRE(!logContents.empty());
    CHECK(logContents.find("early line 1023") != std::string::npos);
    CHECK(logContents.find("early line 0000") < logContents.find("early line 1023"));
    CHECK(logContents.find("early line 1024") == std::string::npos);
    CHECK(logContents.find("6 log lines from before the log opened were dropped") != std::string::npos);
    CHECK(echoed.find("early line 1029") != std::string::npos);
    std::filesystem::remove_all(directory);
}

TEST_CASE("an unwritable log folder costs the file log, not the process")
{
    const auto directory = TestDirectory("mu_logger_unwritable");
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    // A folder can't be created under a regular file, on any OS.
    const auto blocker = directory / "blocker";
    std::ofstream(blocker) << "not a folder";

    std::string warning;
    {
        StreamCapture capture(stderr);
        CHECK_NOTHROW(mu::log::Init(blocker / "logs"));
        const auto logger = mu::log::Get("core");
        CHECK_NOTHROW(MU_LOG_ERROR(logger, "logged without a log file"));
        logger->flush();
        mu::log::Shutdown();
        warning = capture.Read();
    }

    CHECK(warning.find("No log file") != std::string::npos);
    CHECK(warning.find("logged without a log file") != std::string::npos);
    CHECK_FALSE(std::filesystem::exists(blocker / "logs" / "MuError.log"));
    std::filesystem::remove_all(directory);
}
