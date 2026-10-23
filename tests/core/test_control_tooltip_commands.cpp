#include "stdafx.h"

#include "doctest.h"

#include "App/Control/ControlCommands.h"
#include "App/Control/ControlProtocol.h"

#include <filesystem>
#include <memory>
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
        CHECK(IsBadRequest(tooltip(fields)));
    }
}

TEST_CASE("tooltip-snapshot fails when it can't write its file [core][control]")
{
    // A folder can't be opened as the file.
    const std::filesystem::path folder = std::filesystem::temp_directory_path();
    const std::string response = Run(&App::Control::Commands::ItemTooltipSnapshot,
                                     R"({"cmd":"tooltip-snapshot","out":")" + folder.generic_string() + R"("})");
    CHECK(response.find(R"("error":"failed")") != std::string::npos);
}
