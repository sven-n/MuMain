#include "stdafx.h"

#include "doctest.h"

#include "App/Control/ControlCommands.h"
#include "App/Control/ControlProtocol.h"

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

namespace
{
using Command = std::string (*)(const App::Control::Request&, std::unique_ptr<App::Control::Act>&);

std::string Run(Command command, const std::string& raw)
{
    std::unique_ptr<App::Control::Act> act;
    return command(App::Control::Request::Parse(raw), act);
}

bool IsBadRequest(const std::string& response)
{
    return response.find(R"("error":"bad_request")") != std::string::npos;
}
} // namespace

TEST_CASE("repair-tooltip refuses a value its item's bytes can't hold [core][control]")
{
    const auto repair = [](const std::string& fields)
    {
        return Run(&App::Control::Commands::RepairTooltip,
                   R"({"cmd":"repair-tooltip","group":0,"number":0,"level":0,)" + fields + "}");
    };
    // A durability is a byte, the excellent options six bits: what would wrap around is refused.
    CHECK(IsBadRequest(repair(R"("durability":-1)")));
    CHECK(IsBadRequest(repair(R"("durability":256)")));
    CHECK(IsBadRequest(repair(R"("durability":"full")")));
    CHECK(IsBadRequest(repair(R"("excellent":-1)")));
    CHECK(IsBadRequest(repair(R"("excellent":64)")));
    CHECK(IsBadRequest(repair(R"("excellent":256)")));
}

TEST_CASE("item-name-snapshot fails when it can't write its file [core][control]")
{
    // A folder can't be opened as the file.
    const std::filesystem::path folder = std::filesystem::temp_directory_path();
    const std::string response = Run(&App::Control::Commands::ItemNameSnapshot,
                                     R"({"cmd":"item-name-snapshot","out":")" + folder.generic_string() + R"("})");
    CHECK(response.find(R"("error":"failed")") != std::string::npos);
}

TEST_CASE("item-name and repair-tooltip refuse an item number no item uses [core][control]")
{
    // The tests load no item data, so no item exists.
    const std::pair<Command, const char*> commands[] = {{&App::Control::Commands::ItemName, "item-name"},
                                                        {&App::Control::Commands::RepairTooltip, "repair-tooltip"}};
    for (const auto& [command, name] : commands)
    {
        INFO(name);
        const std::string response = Run(command, std::string(R"({"cmd":")") + name + R"(","group":15,"number":511})");
        CHECK(IsBadRequest(response));
        CHECK(response.find("no item has group 15 and number 511") != std::string::npos);
    }
}

TEST_CASE("item-name-snapshot replies the absolute path it wrote [core][control]")
{
    const std::filesystem::path previous = std::filesystem::current_path();
    std::filesystem::current_path(std::filesystem::temp_directory_path());
    const std::string response = Run(&App::Control::Commands::ItemNameSnapshot,
                                     R"({"cmd":"item-name-snapshot","out":"mu_item_name_snapshot_relative.txt"})");
    const std::filesystem::path written = std::filesystem::absolute("mu_item_name_snapshot_relative.txt");
    std::filesystem::current_path(previous);
    CHECK(std::filesystem::exists(written));
    CHECK(response.find("mu_item_name_snapshot_relative.txt") != std::string::npos);
    CHECK(response.find("\"out\":\"mu_item_name_snapshot_relative.txt\"") == std::string::npos);
    std::filesystem::remove(written);
}
