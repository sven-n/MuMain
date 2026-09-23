#pragma once

#include "Guild/UIGuildInfo.h"
#include "Guild/UIGuildMaster.h"
#include "UI/Combat/UISenatus.h"
#include "UI/NPCs/UIGateKeeper.h"
#include "UI/Dialogs/UIPopup.h"
#include "UI/Inventory/UIJewelHarmony.h"
#include "GameLogic/Items/ItemAddOptioninfo.h"

extern CUIGateKeeper* g_pUIGateKeeper;
extern CUIPopup* g_pUIPopup;
extern JewelHarmonyInfo* g_pUIJewelHarmonyinfo;
extern ItemAddOptioninfo* g_pItemAddOptioninfo;
class CUIManager;
extern CUIManager* g_pUIManager;

// Named MUTEX_* (not INTERFACE_*) to avoid colliding with the unrelated
// mu::ui::window::INTERFACE_LIST enum -- this one is CUIManager's own open/close
// mutual-exclusion policy.
enum
{
    MUTEX_NONE = 0,
    MUTEX_FRIEND,
    MUTEX_MOVEMAP,
    MUTEX_PARTY,
    MUTEX_QUEST,
    MUTEX_GUILDINFO,
    MUTEX_TRADE,
    MUTEX_STORAGE,
    MUTEX_GUILDSTORAGE,
    MUTEX_MIXINVENTORY,
    MUTEX_COMMAND,
    MUTEX_PET,
    MUTEX_PERSONALSHOPSALE,
    MUTEX_DEVILSQUARE,
    MUTEX_SERVERDIVISION,
    MUTEX_BLOODCASTLE,
    MUTEX_NPCBREEDER,
    MUTEX_NPCSHOP,
    MUTEX_PERSONALSHOPPURCHASE,
    MUTEX_NPCGUILDMASTER,
    MUTEX_GUARDSMAN,
    MUTEX_SENATUS,
    MUTEX_GATEKEEPER,
    MUTEX_CATAPULTATTACK,
    MUTEX_CATAPULTDEFENSE,
    MUTEX_GATESWITCH,
    MUTEX_CHARACTER,
    MUTEX_INVENTORY,
    MUTEX_REFINERY,
    MUTEX_REFINERYINFO,
    MUTEX_KANTURU2ND_ENTERNPC,
    MUTEX_MAP_ENTRANCE,
    MUTEX_MAX_COUNT,
};

class CUIManager
{
public:
    CUIManager();
    virtual ~CUIManager();

protected:
    bool IsCanOpen(DWORD dwInterfaceFlag);
    bool CloseInterface(std::list<DWORD>& dwInterfaceFlag, DWORD dwExtraData = 0);

public:
    void Init();
    POINT RenderWindowBase(int nHeight, int nOriginY = -1);
    bool PressKey(int nKey);
    bool IsInputEnable();
    void UpdateInput();
    void Render();
    void CloseAll();
    bool IsOpen(DWORD dwInterface);
    bool Open(DWORD dwInterface, DWORD dwExtraData = 0);
    bool Close(DWORD dwInterface, DWORD dwExtraData = 0);
    void GetInterfaceAll(std::list<DWORD>& outflag);
    void GetInsertInterface(std::list<DWORD>& outflag, DWORD insertflag);
    void GetDeleteInterface(std::list<DWORD>& outflag, DWORD deleteflag);
};
