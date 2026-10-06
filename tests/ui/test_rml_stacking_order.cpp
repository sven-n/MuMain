#include <doctest.h>

#include "UI/RmlBridge/RmlStackingOrder.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
#include <set>
#include <string>

using UI::RmlBridge::ContextForDocument;
using UI::RmlBridge::DocumentContext;
using UI::RmlBridge::DocumentScene;
using UI::RmlBridge::SceneForDocument;
using UI::RmlBridge::StackingDepthForDocument;

namespace
{
float Depth(const char* documentName)
{
    const auto depth = StackingDepthForDocument(documentName);
    REQUIRE_MESSAGE(depth.has_value(), documentName);
    return *depth;
}

// Every "Data/Interface/RmlUi/<name>.rml" the client sources name.
std::set<std::string> DocumentsNamedInSources()
{
    const std::regex documentPath(R"(Data/Interface/RmlUi/([a-z_]+\.rml))");
    std::set<std::string> names;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(MU_CLIENT_SOURCE_DIR))
    {
        const auto extension = entry.path().extension();
        if (!entry.is_regular_file() || (extension != ".cpp" && extension != ".h"))
            continue;
        std::ifstream file(entry.path());
        const std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        for (std::sregex_iterator it(text.begin(), text.end(), documentPath), end; it != end; ++it)
            names.insert((*it)[1].str());
    }
    return names;
}
} // namespace

TEST_CASE("every document the client loads has a stacking depth [ui][stacking]")
{
    const std::set<std::string> names = DocumentsNamedInSources();
    CHECK(names.size() > 100);
    for (const std::string& name : names)
        CHECK_MESSAGE(StackingDepthForDocument(name).has_value(), name);
}

// The background contexts draw before the native windows, so frame art there stays behind live
// item models. They are closed: new native 3D goes into a RenderTarget its document shows. A
// document leaves these lists when its window moves to one; none joins them.
TEST_CASE("the background contexts hold only the documents already in them [ui][stacking]")
{
    const std::set<std::string> background = {
        "battle_soccer_score.rml", "blood_castle_time.rml", "chaos_castle_time.rml",
        "cursed_temple_system.rml", "doppelganger_frame.rml", "duel_window.rml",
        "empire_guardian_timer.rml", "in_game_shop_bg.rml", "inventory_extension_bg.rml",
        "kanturu_info.rml", "lucky_item_bg.rml", "map_name.rml", "master_level_bg.rml",
        "mix_inventory_bg.rml", "my_inventory_bg.rml", "my_shop_bg.rml",
        "npc_shop_bg.rml", "purchase_shop_bg.rml", "siege_warfare.rml", "storage_bg.rml",
        "storage_ext_bg.rml", "trade_bg.rml", "world_labels.rml",
    };

    const std::set<std::string> named = DocumentsNamedInSources();
    for (const std::string& name : background)
        CHECK_MESSAGE(named.contains(name), name);
    for (const std::string& name : named)
    {
        const DocumentContext context = ContextForDocument(name);
        CHECK_MESSAGE((context == DocumentContext::Background) == background.contains(name), name);
    }
}

TEST_CASE("every document the client loads has a scene [ui][stacking]")
{
    for (const std::string& name : DocumentsNamedInSources())
        CHECK_MESSAGE(SceneForDocument(name).has_value(), name);
}

TEST_CASE("only the main scene's windows are suspended outside it [ui][stacking]")
{
    // CNewUIManager's windows, the HUD, the world labels and their tooltip and message boxes.
    for (const char* name : {"my_inventory.rml", "my_inventory_bg.rml", "move_command.rml", "friend_shell.rml",
                             "chat_room.rml", "letter_read.rml", "letter_write.rml",
                             "blood_castle_enter.rml", "duel_window.rml", "siege_warfare.rml", "main_frame.rml",
                             "world_labels.rml", "map_name.rml", "tooltip.rml", "message_box_view.rml",
                             "generic_confirm_dialog.rml"})
        CHECK_MESSAGE(SceneForDocument(name) == DocumentScene::Main, name);
    // Shown and hidden by their own scene's code: the scene windows, the notices, the loading
    // screens and the reconnect dialog.
    for (const char* name : {"login_scene.rml", "login.rml", "server_select.rml", "char_sel_main.rml",
                             "char_make.rml", "char_info_balloon.rml", "msg_win.rml", "sys_menu.rml",
                             "notices.rml", "loading.rml", "title_scene.rml", "reconnect_dialog.rml"})
        CHECK_MESSAGE(SceneForDocument(name) == DocumentScene::Any, name);
}

TEST_CASE("documents stack as the original's windows did [ui][stacking]")
{
    // The original's order: the logs over the friends windows; friends over character and
    // inventory. The family is one document per window now -- the shell, each chat room, and each
    // letter -- where friend_window.rml used to transcribe all of them at one depth.
    for (const char* name : {"friend_shell.rml", "chat_room.rml", "letter_read.rml", "letter_write.rml"})
    {
        CHECK_MESSAGE(Depth("chat_log.rml") > Depth(name), name);
        CHECK_MESSAGE(Depth("system_log.rml") > Depth(name), name);
        CHECK_MESSAGE(Depth(name) > Depth("character_info.rml"), name);
        CHECK_MESSAGE(Depth(name) > Depth("my_inventory.rml"), name);
    }
    // A letter's chrome sits with the letter it frames; it is only in another context.
    // TODO 34: chat over the pet window and the quest journal.
    CHECK(Depth("chat_log.rml") > Depth("pet_info.rml"));
    CHECK(Depth("chat_log.rml") > Depth("my_quest_info.rml"));
    // TODO 33: help, move list and full map over the location bar.
    CHECK(Depth("help_window.rml") > Depth("mu_helper_bar.rml"));
    CHECK(Depth("move_command.rml") > Depth("mu_helper_bar.rml"));
    CHECK(Depth("mini_map.rml") > Depth("mu_helper_bar.rml"));
    // The window menu over the help; the bottom HUD over the full map.
    CHECK(Depth("window_menu.rml") > Depth("help_window.rml"));
    CHECK(Depth("main_frame.rml") > Depth("mini_map.rml"));
    // The modern top-right button row under every window, over the names.
    CHECK(Depth("main_frame_top.rml") > Depth("world_labels.rml"));
    CHECK(Depth("main_frame_top.rml") < Depth("duel_window.rml"));
    // Names under the duel board, the duel board under the panels (TODO 45, 48).
    CHECK(Depth("world_labels.rml") < Depth("duel_window.rml"));
    CHECK(Depth("duel_window.rml") < Depth("my_inventory_bg.rml"));
    CHECK(Depth("world_labels.rml") < Depth("my_inventory_bg.rml"));
    // TODO 44: the message box over the tooltip; the tooltip over every window.
    CHECK(Depth("message_box_view.rml") > Depth("tooltip.rml"));
    CHECK(Depth("tooltip.rml") > Depth("chat_command.rml"));
    // Notices over the message and result boxes, under the loading screen and the scene windows.
    CHECK(Depth("notices.rml") > Depth("message_box_view.rml"));
    CHECK(Depth("notices.rml") > Depth("generic_confirm_dialog.rml"));
    CHECK(Depth("loading.rml") > Depth("notices.rml"));
    CHECK(Depth("sys_menu.rml") > Depth("notices.rml"));
    CHECK(Depth("reconnect_dialog.rml") > Depth("loading.rml"));
    // A window's background document shares its depth.
    CHECK(Depth("my_inventory_bg.rml") == Depth("my_inventory.rml"));
}
