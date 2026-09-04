#include "UI/NewUI/UILayoutPolicy.h"

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_enum.h"

UI::Scaling::LayoutMode UI::Layout::ForInterface(std::uint32_t interfaceKey)
{
    using namespace SEASON3B;
    using Scaling::LayoutMode;

    switch (interfaceKey)
    {
    // CUIMng/CNewUIManager merger (docs/rmlui-ui-system) -- CCreditWin computes real screen
    // pixels itself (fScaleX/fScaleY against an assumed 800x600), not reference-space coordinates
    // meant to be rescaled by this policy table's other entries. AddUIObj() (NewUIManager.cpp)
    // calls this unconditionally on first registration, silently overwriting whatever
    // SetLayoutMode() a window's own constructor called -- this table entry, not the constructor,
    // is what actually governs a registered window's layout mode.
    case INTERFACE_CREDITS:
    // CServerMsgWin/CServerSelWin (Phase 2) compute real screen pixels themselves too (position
    // derived from real WindowWidth/Height in CUIMng::CreateLoginScene()/CreateCharacterScene()),
    // same reasoning as INTERFACE_CREDITS.
    case INTERFACE_SERVER_MESSAGE:
    case INTERFACE_SERVER_SELECT:
        return LayoutMode::Legacy;

    case INTERFACE_NAME_WINDOW:
    case INTERFACE_ITEM_TOOLTIP:
        return LayoutMode::WorldOverlay;

    case INTERFACE_MOVEMAP:
        return LayoutMode::DockLeft;

    case INTERFACE_FRIEND:
        return LayoutMode::FloatingWorkspace;

    case INTERFACE_CHATINPUTBOX:
    case INTERFACE_WINDOW_MENU:
    case INTERFACE_KANTURU_INFO:
    case INTERFACE_CHATLOGWINDOW:
    case INTERFACE_BLOODCASTLE_TIME:
    case INTERFACE_CHAOSCASTLE_TIME:
    case INTERFACE_BATTLE_SOCCER_SCORE:
    case INTERFACE_SLIDEWINDOW:
    case INTERFACE_MU_HELPER_BAR:
    case INTERFACE_DUEL_WINDOW:
    case INTERFACE_CRYWOLF:
    case INTERFACE_SIEGEWARFARE:
    case INTERFACE_MAINFRAME:
    case INTERFACE_BUFF_WINDOW:
    case INTERFACE_MASTER_LEVEL:
    case INTERFACE_DUELWATCH_MAINFRAME:
    case INTERFACE_DUELWATCH_USERLIST:
    case INTERFACE_DOPPELGANGER_FRAME:
    case INTERFACE_EMPIREGUARDIAN_TIMER:
    case INTERFACE_MINI_MAP:
    case INTERFACE_GENSRANKING:
    case INTERFACE_HOTKEY:
    case INTERFACE_SYSTEMLOGWINDOW:
    case INTERFACE_CURSEDTEMPLE_GAMESYSTEM:
        return LayoutMode::Hud;

    case INTERFACE_SKILL_LIST:
        return LayoutMode::HudCenter;

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
        return LayoutMode::DockRight;

    default:
        return LayoutMode::Dialog;
    }
}
