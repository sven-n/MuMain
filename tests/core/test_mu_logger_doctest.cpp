#include <doctest.h>

#include <filesystem>
#include <fstream>
#include <iterator>

#include "Core/Platform/WinCompat.h"
#include "Core/Platform/SecureCrt.h"
#include "Core/Platform/WinApiShims.h"
#include "Core/Utilities/Log/MuLogger.h"

std::shared_ptr<spdlog::logger> GetEarlyLogger();

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

    const std::string logContents = ReadFile(directory / "MuError.log");
    CHECK(logContents.find("mu logger early-init self-check") != std::string::npos);
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
