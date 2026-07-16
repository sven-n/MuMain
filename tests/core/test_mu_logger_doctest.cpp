#include <doctest.h>

#include <filesystem>

#include "Core/Utilities/Log/MuLogger.h"

TEST_CASE("named loggers write rotating file")
{
    const auto directory = std::filesystem::temp_directory_path() / "mu_logger_test";
    std::filesystem::remove_all(directory);

    mu::log::Init(directory);
    const auto core = mu::log::Get("core");
    const auto network = mu::log::Get("network");
    CHECK(core != nullptr);
    CHECK(network != nullptr);
    CHECK(core != network);
    MU_LOG_ERROR(core, "mu logger self-check");
    core->flush();
    mu::log::Shutdown();

    CHECK(std::filesystem::exists(directory / "MuError.log"));
    std::filesystem::remove_all(directory);
}
