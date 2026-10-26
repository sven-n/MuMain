#include "stdafx.h"

#include "doctest.h"

#include "App/Control/ControlCommands.h"
#include "App/Control/ControlProtocol.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

namespace
{
std::string Run(std::string (*command)(const App::Control::Request&, std::unique_ptr<App::Control::Act>&),
                const std::string& raw)
{
    std::unique_ptr<App::Control::Act> act;
    return command(App::Control::Request::Parse(raw), act);
}

bool IsBadRequest(const std::string& response)
{
    return response.find(R"("error":"bad_request")") != std::string::npos;
}
} // namespace

TEST_CASE("tooltip refuses a value its item's fields can't hold [core][control]")
{
    const auto tooltip = [](const std::string& fields)
    { return Run(&App::Control::Commands::ItemTooltip, R"({"cmd":"tooltip","group":0,"number":0,)" + fields + "}"); };
    // Each would wrap around in the item's byte (or its packet's four bits) instead of being refused.
    for (const char* fields :
         {R"("level":-1)", R"("level":16)", R"("durability":-1)", R"("durability":256)", R"("option":-1)",
          R"("option":16)", R"("excellent":64)", R"("excellent":256)", R"("ancient":-1)", R"("ancient":256)",
          R"("harmony":16)", R"("harmony":1,"harmony_level":16)", R"("level":"high")"})
    {
        INFO(fields);
        const std::string response = tooltip(fields);
        CHECK(IsBadRequest(response));
        CHECK(response.find("0 to 15") != std::string::npos);
    }
}

TEST_CASE("tooltip refuses a harmony level without a harmony option [core][control]")
{
    const std::string response = Run(&App::Control::Commands::ItemTooltip,
                                     R"({"cmd":"tooltip","group":0,"number":0,"harmony_level":13})");
    CHECK(IsBadRequest(response));
    CHECK(response.find("`harmony_level` needs `harmony`") != std::string::npos);
}

TEST_CASE("tooltip refuses an item number no item uses [core][control]")
{
    // The tests load no item data, so no item exists.
    const std::string response =
        Run(&App::Control::Commands::ItemTooltip, R"({"cmd":"tooltip","group":15,"number":511})");
    CHECK(IsBadRequest(response));
    CHECK(response.find("no item has group 15 and number 511") != std::string::npos);
}

TEST_CASE("tooltip-snapshot leaves its file alone when the request is refused [core][control]")
{
    const std::filesystem::path file = std::filesystem::temp_directory_path() / "mu_tooltip_snapshot_keep.txt";
    {
        std::ofstream(file, std::ios::binary) << "before";
    }
    const std::string response =
        Run(&App::Control::Commands::ItemTooltipSnapshot,
            R"({"cmd":"tooltip-snapshot","out":")" + file.generic_string() + R"(","group":16})");
    CHECK(IsBadRequest(response));
    std::ostringstream text;
    text << std::ifstream(file, std::ios::binary).rdbuf();
    CHECK(text.str() == "before");
    std::filesystem::remove(file);
}

TEST_CASE("tooltip-snapshot replies the absolute path it wrote [core][control]")
{
    const std::filesystem::path previous = std::filesystem::current_path();
    std::filesystem::current_path(std::filesystem::temp_directory_path());
    const std::string response = Run(&App::Control::Commands::ItemTooltipSnapshot,
                                     R"({"cmd":"tooltip-snapshot","out":"mu_tooltip_snapshot_relative.txt"})");
    const std::filesystem::path written = std::filesystem::absolute("mu_tooltip_snapshot_relative.txt");
    std::filesystem::current_path(previous);
    CHECK(std::filesystem::exists(written));
    CHECK(response.find("mu_tooltip_snapshot_relative.txt") != std::string::npos);
    CHECK(response.find("\"out\":\"mu_tooltip_snapshot_relative.txt\"") == std::string::npos);
    std::filesystem::remove(written);
}

TEST_CASE("tooltip-snapshot fails when it can't write its file [core][control]")
{
    // A folder can't be opened as the file.
    const std::filesystem::path folder = std::filesystem::temp_directory_path();
    const std::string response = Run(&App::Control::Commands::ItemTooltipSnapshot,
                                     R"({"cmd":"tooltip-snapshot","out":")" + folder.generic_string() + R"("})");
    CHECK(response.find(R"("error":"failed")") != std::string::npos);
}
