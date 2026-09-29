#include "UI/RmlBridge/RmlStackingOrder.h"

#include <utility>

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
constexpr float SceneMessageBoxDepth = 31.0f; // the login scene's g_MessageBox, after CUIMng
constexpr float LoadingScreenDepth = 40.0f;   // the loading scene draws nothing else
constexpr float ReconnectDialogDepth = 50.0f; // after the whole scene

// The original's GetLayerDepth() of each window, for its foreground and background
// (_bg) documents alike.
constexpr std::pair<std::string_view, float> Depths[] = {
    {"login_scene.rml", BeforeWindowsDepth},
    {"map_name.rml", BeforeWindowsDepth},
    {"buff_strip.rml", 0.95f},
    {"world_labels.rml", 1.0f},
    {"duel_window.rml", 1.1f},
    {"blood_castle_time.rml", 1.2f},
    {"doppelganger_frame.rml", 1.2f},
    {"empire_guardian_enter.rml", 1.2f},
    {"empire_guardian_enter_bg.rml", 1.2f},
    {"empire_guardian_timer.rml", 1.2f},
    {"chaos_castle_time.rml", 1.3f},
    {"cursed_temple_system.rml", 1.5f},
    {"siege_warfare.rml", 1.6f},
    {"battle_soccer_score.rml", 1.8f},
    {"kanturu_info.rml", 1.92f},
    {"quick_command.rml", 2.0f},
    {"trade.rml", 2.1f},
    {"trade_bg.rml", 2.1f},
    {"storage.rml", 2.2f},
    {"storage_bg.rml", 2.2f},
    {"storage_ext.rml", 2.2f},
    {"storage_ext_bg.rml", 2.2f},
    {"pet_info.rml", 2.3f},
    {"party_info.rml", 2.4f},
    {"npc_dialogue.rml", 3.1f},
    {"npc_quest.rml", 3.1f},
    {"npc_quest_bg.rml", 3.1f},
    {"quest_progress.rml", 3.1f},
    {"quest_progress_etc.rml", 3.1f},
    {"my_shop.rml", 3.2f},
    {"my_shop_bg.rml", 3.2f},
    {"purchase_shop.rml", 3.2f},
    {"purchase_shop_bg.rml", 3.2f},
    {"my_quest_info.rml", 3.3f},
    {"mix_inventory.rml", 3.4f},
    {"mix_inventory_bg.rml", 3.4f},
    {"lucky_item.rml", 3.4f},
    {"lucky_item_bg.rml", 3.4f},
    {"gold_bowman.rml", 3.4f},
    {"gold_bowman_bg.rml", 3.4f},
    {"gold_bowman_lena.rml", 3.4f},
    {"gold_bowman_lena_bg.rml", 3.4f},
    {"mu_helper_config.rml", 3.4f},
    {"mu_helper_detail.rml", 3.4f},
    {"item_endurance.rml", 3.5f},
    {"devil_square_enter.rml", 4.0f},
    {"blood_castle_enter.rml", 4.1f},
    {"my_inventory.rml", 4.2f},
    {"my_inventory_bg.rml", 4.2f},
    {"gens_ranking.rml", 4.2f},
    {"lucky_coin_exchange.rml", 4.2f},
    {"lucky_coin_exchange_bg.rml", 4.2f},
    {"lucky_coin_registration.rml", 4.2f},
    {"lucky_coin_registration_bg.rml", 4.2f},
    {"mu_helper_bar.rml", 4.3f}, // the location bar (CNewUIHeroPositionInfo)
    {"guild_make.rml", 4.3f},
    {"guild_info.rml", 4.5f},
    {"npc_shop.rml", 4.55f},
    {"npc_shop_bg.rml", 4.55f},
    {"inventory_extension.rml", 4.55f},
    {"inventory_extension_bg.rml", 4.55f},
    {"castle_window.rml", 5.0f},
    {"guard_window.rml", 5.0f},
    {"gateman.rml", 5.0f},
    {"gate_switch.rml", 5.0f},
    {"catapult.rml", 5.0f},
    {"doppelganger_enter.rml", 5.0f},
    {"doppelganger_enter_bg.rml", 5.0f},
    {"united_market_place.rml", 5.0f},
    {"duel_watch.rml", 5.0f},
    {"duel_watch_frame.rml", 5.0f},
    {"duel_watch_spectators.rml", 5.0f},
    {"character_info.rml", 5.1f},
    {"mu_helper_skill_picker.rml", 5.2f},
    {"party_list.rml", 5.4f},
    {"friend_window.rml", 6.0f}, // every friends window: CNewUIFriendWindow drew them all
    {"system_log.rml", 6.05f},
    {"chat_log.rml", 6.1f},
    {"chat_input.rml", 6.2f},
    {"item_explanation.rml", 6.5f},
    {"set_item_explanation.rml", 6.6f},
    {"mini_map.rml", 8.1f},
    {"help_window.rml", 8.3f},
    {"move_command.rml", 8.3f},
    {"crywolf.rml", 10.0f},
    {"kanturu_enter.rml", 10.1f},
    {"master_level.rml", 10.1f},
    {"master_level_bg.rml", 10.1f},
    {"cursed_temple_result.rml", 10.2f},
    {"cursed_temple_enter.rml", 10.3f},
    {"window_menu.rml", 10.4f},
    {"option_window.rml", 10.5f},
    {"main_frame.rml", 10.6f},
    {"main_frame_bg.rml", 10.6f},
    {"command_window.rml", 10.65f}, // UI::Layout::ForegroundPanelLayerDepth
    {"chat_command.rml", 10.65f},
    // The original drew each tooltip at its owner's depth (item tooltips at 5.5), where the
    // chat log, the friends window, the full map and the HUD hid its rows: an original
    // defect. Every tooltip draws above the windows instead, still under the message boxes.
    {"tooltip.rml", TooltipDepth},
    {"message_box_view.rml", MessageBoxDepth},
    // New-only dialogs that replace message boxes.
    {"generic_confirm_dialog.rml", MessageBoxDepth},
    {"generic_confirm_dialog_bg.rml", MessageBoxDepth},
    {"generic_menu_dialog.rml", MessageBoxDepth},
    {"notices.rml", NoticesDepth},
    {"char_info_balloon.rml", SceneBalloonDepth},
    {"login.rml", SceneWindowDepth},
    {"login_main.rml", SceneWindowDepth},
    {"server_select.rml", SceneWindowDepth},
    {"credit_win.rml", SceneWindowDepth},
    {"char_sel_main.rml", SceneWindowDepth},
    {"char_make.rml", SceneWindowDepth},
    {"server_msg.rml", SceneWindowDepth},
    {"msg_win.rml", SceneWindowDepth},
    {"sys_menu.rml", SceneWindowDepth},
    {"remember_password_prompt.rml", SceneMessageBoxDepth},
    {"loading.rml", LoadingScreenDepth},
    {"title_scene.rml", LoadingScreenDepth},
    {"reconnect_dialog.rml", ReconnectDialogDepth},
};
} // namespace

std::optional<float> StackingDepthForDocument(std::string_view documentName)
{
    for (const auto& [name, depth] : Depths)
    {
        if (name == documentName)
            return depth;
    }
    return std::nullopt;
}
} // namespace UI::RmlBridge
