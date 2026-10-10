#include "UI/RmlBridge/RmlStackingOrder.h"

namespace UI::RmlBridge
{
namespace
{
// Passes the original drew outside its window list (Scenes/MainScene.cpp RenderMainSceneUI(),
// Scenes/SceneCommon.cpp RenderInfomation(), Scenes/SceneManager.cpp).
constexpr float BeforeWindowsDepth = 0.5f; // RenderObjectDescription(), RenderInterface()
constexpr float TooltipDepth = 10.69f;     // just under the message boxes, see below
constexpr float MessageBoxDepth = 10.7f;   // CNewUIMessageBoxMng
constexpr float NoticesDepth = 20.0f;      // UI::Notices::Render(), after every window
constexpr float SceneBalloonDepth = 29.0f; // CUIMng::Render(): balloons, then its windows
constexpr float SceneWindowDepth = 30.0f;
constexpr float OverSceneWindowsDepth = 30.5f; // a main-scene window opened over CUIMng's windows
constexpr float SceneMessageBoxDepth = 31.0f; // the login scene's g_MessageBox, after CUIMng
constexpr float LoadingScreenDepth = 40.0f;   // the loading scene draws nothing else
constexpr float ReconnectDialogDepth = 50.0f; // after the whole scene

constexpr DocumentScene MainScene = DocumentScene::Main;
constexpr DocumentScene AnyScene = DocumentScene::Any;
constexpr DocumentScene EveryScene = DocumentScene::Every;

struct DocumentPlacement
{
    std::string_view name;
    float depth;
    DocumentScene scene;
};

// The original's GetLayerDepth() of each window and the scene whose windows the document belongs
// to.
constexpr DocumentPlacement Placements[] = {
    {"login_scene.rml", BeforeWindowsDepth, AnyScene},
    // Never drawn: it only lays out where windows go.
    {"workspace.rml", BeforeWindowsDepth, AnyScene},
    {"map_name.rml", BeforeWindowsDepth, MainScene},
    // A guild war's or battle soccer's time and result (RenderTournamentInterface()).
    {"match_status.rml", BeforeWindowsDepth, MainScene},
    {"buff_strip.rml", 0.95f, MainScene},
    {"world_labels.rml", 1.0f, MainScene},
    // The status texts over the world above the HUD, drawn with the name labels.
    {"hud_status.rml", 1.0f, MainScene},
    // The modern theme's top-right button row: under every window, which dock over it.
    {"main_frame_top.rml", 1.05f, MainScene},
    // The duel and soccer boards drew under every panel.
    {"duel_window.rml", 1.1f, MainScene},
    {"blood_castle_time.rml", 1.2f, MainScene},
    {"doppelganger_frame.rml", 1.2f, MainScene},
    {"empire_guardian_enter.rml", 1.2f, MainScene},
    {"empire_guardian_timer.rml", 1.2f, MainScene},
    {"chaos_castle_time.rml", 1.3f, MainScene},
    {"cursed_temple_system.rml", 1.5f, MainScene},
    {"siege_warfare.rml", 1.6f, MainScene},
    {"battle_soccer_score.rml", 1.8f, MainScene},
    {"kanturu_info.rml", 1.92f, MainScene},
    {"quick_command.rml", 2.0f, MainScene},
    {"trade.rml", 2.1f, MainScene},
    {"storage.rml", 2.2f, MainScene},
    {"storage_ext.rml", 2.2f, MainScene},
    {"pet_info.rml", 2.3f, MainScene},
    {"party_info.rml", 2.4f, MainScene},
    {"npc_dialogue.rml", 3.1f, MainScene},
    {"npc_quest.rml", 3.1f, MainScene},
    {"quest_progress.rml", 3.1f, MainScene},
    {"quest_progress_etc.rml", 3.1f, MainScene},
    {"my_shop.rml", 3.2f, MainScene},
    {"purchase_shop.rml", 3.2f, MainScene},
    {"my_quest_info.rml", 3.3f, MainScene},
    {"mix_inventory.rml", 3.4f, MainScene},
    {"lucky_item.rml", 3.4f, MainScene},
    {"gold_bowman.rml", 3.4f, MainScene},
    {"gold_bowman_lena.rml", 3.4f, MainScene},
    {"mu_helper_config.rml", 3.4f, MainScene},
    {"mu_helper_detail.rml", 3.4f, MainScene},
    {"item_endurance.rml", 3.5f, MainScene},
    {"devil_square_enter.rml", 4.0f, MainScene},
    {"blood_castle_enter.rml", 4.1f, MainScene},
    {"my_inventory.rml", 4.2f, MainScene},
    {"gens_ranking.rml", 4.2f, MainScene},
    {"lucky_coin_exchange.rml", 4.2f, MainScene},
    {"lucky_coin_registration.rml", 4.2f, MainScene},
    {"mu_helper_bar.rml", 4.3f, MainScene}, // the location bar (CNewUIHeroPositionInfo)
    {"guild_make.rml", 4.3f, MainScene},
    {"guild_info.rml", 4.5f, MainScene},
    {"npc_shop.rml", 4.55f, MainScene},
    {"inventory_extension.rml", 4.55f, MainScene},
    {"castle_window.rml", 5.0f, MainScene},
    {"guard_window.rml", 5.0f, MainScene},
    {"gateman.rml", 5.0f, MainScene},
    {"gate_switch.rml", 5.0f, MainScene},
    {"catapult.rml", 5.0f, MainScene},
    {"doppelganger_enter.rml", 5.0f, MainScene},
    {"united_market_place.rml", 5.0f, MainScene},
    {"duel_watch.rml", 5.0f, MainScene},
    {"duel_watch_frame.rml", 5.0f, MainScene},
    {"duel_watch_spectators.rml", 5.0f, MainScene},
    {"character_info.rml", 5.1f, MainScene},
    {"mu_helper_skill_picker.rml", 5.2f, MainScene},
    {"party_list.rml", 5.4f, MainScene},
    {"friend_shell.rml", 6.0f, MainScene},
    {"chat_room.rml", 6.0f, MainScene},
    {"letter_read.rml", 6.0f, MainScene},
    {"letter_write.rml", 6.0f, MainScene},
    {"system_log.rml", 6.05f, MainScene},
    {"chat_log.rml", 6.1f, MainScene},
    {"chat_input.rml", 6.2f, MainScene},
    {"item_explanation.rml", 6.5f, MainScene},
    {"set_item_explanation.rml", 6.6f, MainScene},
    // CSlideWindow::GetLayerDepth() is 1.91 -- it sits under the windows, as the band did.
    {"slide_notice.rml", 1.91f, MainScene},
    {"mini_map.rml", 8.1f, MainScene},
    {"help_window.rml", 8.3f, MainScene},
    {"move_command.rml", 8.3f, MainScene},
    {"crywolf.rml", 10.0f, MainScene},
    // CInGameShop::GetLayerDepth(): the shop sits above crywolf and below kanturu_enter.
    {"in_game_shop.rml", 10.08f, MainScene},
    // The shop's own buy dialog, above the shop it opens from.
    {"igs_buy_package.rml", 10.09f, MainScene},
    {"igs_buy_select.rml", 10.09f, MainScene},
    {"igs_send_gift.rml", 10.09f, MainScene},
    {"kanturu_enter.rml", 10.1f, MainScene},
    {"master_level.rml", 10.1f, MainScene},
    {"master_level_band.rml", 10.1f, MainScene},
    {"cursed_temple_result.rml", 10.2f, MainScene},
    {"cursed_temple_enter.rml", 10.3f, MainScene},
    {"window_menu.rml", 10.4f, MainScene},
    {"main_frame.rml", 10.6f, MainScene},
    {"command_window.rml", 10.65f, MainScene}, // UI::RmlBridge::ForegroundPanelLayerDepth
    {"chat_command.rml", 10.65f, MainScene},
    // The original drew it at 10.5, under the HUD; the modern theme's settings screen covers the
    // whole screen, HUD included. Still under the tooltips and message boxes.
    {"option_window.rml", 10.67f, EveryScene},
    // The original drew each tooltip at its owner's depth (item tooltips at 5.5), where the
    // chat log, the friends window, the full map and the HUD hid its rows: an original
    // defect. Every tooltip draws above the windows instead, still under the message boxes.
    {"tooltip.rml", TooltipDepth, MainScene},
    {"message_box_view.rml", MessageBoxDepth, MainScene},
    // New-only dialogs that replace message boxes.
    {"generic_confirm_dialog.rml", MessageBoxDepth, MainScene},
    {"generic_menu_dialog.rml", MessageBoxDepth, MainScene},
    // The item on the cursor: the original's camera at 10.9, over the windows and message boxes.
    {"cursor_item.rml", 10.9f, MainScene},
    {"notices.rml", NoticesDepth, AnyScene},
    {"char_info_balloon.rml", SceneBalloonDepth, AnyScene},
    {"login.rml", SceneWindowDepth, AnyScene},
    {"login_main.rml", SceneWindowDepth, AnyScene},
    {"server_select.rml", SceneWindowDepth, AnyScene},
    {"credit_win.rml", SceneWindowDepth, AnyScene},
    {"char_sel_main.rml", SceneWindowDepth, AnyScene},
    {"char_make.rml", SceneWindowDepth, AnyScene},
    {"server_msg.rml", SceneWindowDepth, AnyScene},
    {"msg_win.rml", SceneWindowDepth, AnyScene},
    {"sys_menu.rml", SceneWindowDepth, AnyScene},
    {"remember_password_prompt.rml", SceneMessageBoxDepth, AnyScene},
    {"loading.rml", LoadingScreenDepth, AnyScene},
    {"title_scene.rml", LoadingScreenDepth, AnyScene},
    {"reconnect_dialog.rml", ReconnectDialogDepth, AnyScene},
    {"diagnostics.rml", ReconnectDialogDepth, AnyScene},
};
} // namespace

namespace
{
const DocumentPlacement* FindPlacement(std::string_view documentName)
{
    for (const DocumentPlacement& placement : Placements)
    {
        if (placement.name == documentName)
            return &placement;
    }
    return nullptr;
}
} // namespace

std::optional<float> StackingDepthForDocument(std::string_view documentName)
{
    const DocumentPlacement* placement = FindPlacement(documentName);
    if (placement == nullptr)
        return std::nullopt;
    return placement->depth;
}

std::optional<DocumentScene> SceneForDocument(std::string_view documentName)
{
    const DocumentPlacement* placement = FindPlacement(documentName);
    if (placement == nullptr)
        return std::nullopt;
    return placement->scene;
}

std::optional<float> StackingDepthOutsideMainScene(std::string_view documentName)
{
    const DocumentPlacement* placement = FindPlacement(documentName);
    if (placement == nullptr || placement->scene != DocumentScene::Every)
        return std::nullopt;
    return OverSceneWindowsDepth;
}

} // namespace UI::RmlBridge
