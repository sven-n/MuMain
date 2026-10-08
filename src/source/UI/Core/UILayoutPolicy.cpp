#include "UI/Core/UILayoutPolicy.h"

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_enum.h"

UI::Scaling::LayoutMode UI::Layout::ForInterface(std::uint32_t interfaceKey)
{
    using namespace mu::ui::window;
    using Scaling::LayoutMode;

    switch (interfaceKey)
    {
    // Windows below compute real screen pixels themselves rather than reference-space
    // coordinates meant to be rescaled by this table. Note: AddUIObj() (WindowManager.cpp)
    // applies this table unconditionally, overriding any SetLayoutMode() the window's own
    // constructor set.
    case INTERFACE_CREDITS:
    case INTERFACE_SERVER_MESSAGE:
    case INTERFACE_SERVER_SELECT:
    case INTERFACE_MSG_WINDOW:
    case INTERFACE_SYS_MENU:
    case INTERFACE_CHAR_SEL_MAIN:
    case INTERFACE_CHAR_MAKE:
    case INTERFACE_LOGIN_MAIN:
    case INTERFACE_LOGIN:
    // Not modal (world clicks/character movement must still work around it, unlike
    // CGenericMenuDialog) -- its own UpdateMouseEvent() needs a real WindowGeometry hit-test
    // against MouseX/MouseY in the same real-device-pixel space its window_shell content and
    // RestoreDefaultOrUserPosition() already use. Falling through to the default Dialog mode
    // (PanelTransform's 640x480-reference rescale) remapped MouseX/MouseY into a completely
    // different coordinate space than that hit-test rect, so it almost never matched and clicks
    // fell through to the world underneath -- found live (see OptionWindow.cpp's own history).
    case INTERFACE_OPTION:
        return LayoutMode::Pixels;

    case INTERFACE_NAME_WINDOW:
    case INTERFACE_ITEM_TOOLTIP:
    // Must be ScreenOverlay, not Pixels: CCharInfoBalloon::Render() scales its WorldToScreen()
    // position by g_fScreenRate_x/y, which Pixels leaves at 1.0 -- using Pixels here once made
    // balloons drift at any resolution other than 640x480 (found via live testing).
    case INTERFACE_CHAR_INFO_BALLOON:
        return LayoutMode::ScreenOverlay;

    case INTERFACE_MOVEMAP:
        return LayoutMode::DockLeft;

    case INTERFACE_FRIEND:
        return LayoutMode::FloatingWorkspace;

    // The event HUDs, at the size the workspace's event-hud region gives their slots.
    case INTERFACE_KANTURU_INFO:
    case INTERFACE_BLOODCASTLE_TIME:
    case INTERFACE_CHAOSCASTLE_TIME:
    case INTERFACE_BATTLE_SOCCER_SCORE:
    case INTERFACE_DUEL_WINDOW:
    case INTERFACE_DUELWATCH_USERLIST:
    case INTERFACE_DUELWATCH_MAINFRAME:
    case INTERFACE_DOPPELGANGER_FRAME:
    case INTERFACE_EMPIREGUARDIAN_TIMER:
    // The bottom HUD's parts and the chat, which the workspace places at the HUD's scale.
    case INTERFACE_CHATINPUTBOX:
    case INTERFACE_CHATLOGWINDOW:
    case INTERFACE_MU_HELPER_BAR:
    case INTERFACE_MAINFRAME:
    case INTERFACE_BUFF_WINDOW:
    case INTERFACE_HOTKEY:
    case INTERFACE_SYSTEMLOGWINDOW:
        return LayoutMode::HudFrame;

    // Drawn over the whole screen: the notice band and the full map.
    case INTERFACE_SLIDEWINDOW:
    case INTERFACE_MINI_MAP:
        return LayoutMode::ScreenOverlay;

    // The original's 640x480 screen at the bottom HUD's scale, centred like it: the fixed-place
    // windows that stand on or open from the HUD.
    case INTERFACE_SKILL_LIST:
    case INTERFACE_WINDOW_MENU:
    case INTERFACE_CRYWOLF:
    case INTERFACE_SIEGEWARFARE:
    case INTERFACE_MASTER_LEVEL:
    case INTERFACE_CURSEDTEMPLE_GAMESYSTEM:
        return LayoutMode::HudBoard;

    case INTERFACE_PARTY:
    case INTERFACE_MYQUEST:
    case INTERFACE_NPCQUEST:
    case INTERFACE_GUILDINFO:
    case INTERFACE_TRADE:
    case INTERFACE_STORAGE:
    case INTERFACE_STORAGE_EXT:
    case INTERFACE_MIXINVENTORY:
    case INTERFACE_COMMAND:
    case INTERFACE_PET:
    case INTERFACE_NPCSHOP:
    case INTERFACE_INVENTORY:
    case INTERFACE_INVENTORY_EXT:
    case INTERFACE_MYSHOP_INVENTORY:
    case INTERFACE_PURCHASESHOP_INVENTORY:
    case INTERFACE_CHARACTER:
    case INTERFACE_DEVILSQUARE:
    case INTERFACE_BLOODCASTLE:
    case INTERFACE_NPCGUILDMASTER:
    case INTERFACE_GUARDSMAN:
    case INTERFACE_SENATUS:
    case INTERFACE_GATEKEEPER:
    case INTERFACE_GATESWITCH:
    case INTERFACE_CATAPULT:
    case INTERFACE_GOLD_BOWMAN:
    case INTERFACE_GOLD_BOWMAN_LENA:
    case INTERFACE_LUCKYCOIN_REGISTRATION:
    case INTERFACE_EXCHANGE_LUCKYCOIN:
    case INTERFACE_DUELWATCH:
    case INTERFACE_DOPPELGANGER_NPC:
    case INTERFACE_QUEST_PROGRESS:
    case INTERFACE_QUEST_PROGRESS_ETC:
    case INTERFACE_EMPIREGUARDIAN_NPC:
    case INTERFACE_NPC_DIALOGUE:
    case INTERFACE_UNITEDMARKETPLACE_NPC_JULIA:
    case INTERFACE_LUCKYITEMWND:
    case INTERFACE_MUHELPER:
    case INTERFACE_MUHELPER_EXT:
    case INTERFACE_MUHELPER_SKILL_LIST:
    case INTERFACE_COMMAND_LIST:
    case INTERFACE_ITEM_ENDURANCE_INFO:
    case INTERFACE_PARTY_INFO_WINDOW:
    case INTERFACE_GENSRANKING:
        return LayoutMode::DockRight;

    default:
        return LayoutMode::Stage;
    }
}
