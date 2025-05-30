#pragma once

#include "UIGuildInfo.h"
#include "UIGuildMaster.h"
#include "UISenatus.h"
#include "UIGateKeeper.h"
#include "UIPopup.h"
#include "UIJewelHarmony.h"
#include "ItemAddOptioninfo.h"

extern CUIGateKeeper* g_pUIGateKeeper;
extern CUIPopup* g_pUIPopup;
extern JewelHarmonyInfo* g_pUIJewelHarmonyinfo;
extern ItemAddOptioninfo* g_pItemAddOptioninfo;
class CUIManager;
extern CUIManager* g_pUIManager;

enum
{
    INTERFACE_NONE = 0,
    INTERFACE_FRIEND,
    INTERFACE_MOVEMAP,
    INTERFACE_PARTY,
    INTERFACE_QUEST,
    INTERFACE_GUILDINFO,
    INTERFACE_TRADE,
    INTERFACE_STORAGE,
    INTERFACE_GUILDSTORAGE,
    INTERFACE_MIXINVENTORY,
    INTERFACE_COMMAND,
    INTERFACE_PET,
    INTERFACE_PERSONALSHOPSALE,
    INTERFACE_DEVILSQUARE,
    INTERFACE_SERVERDIVISION,
    INTERFACE_BLOODCASTLE,
    INTERFACE_NPCBREEDER,
    INTERFACE_NPCSHOP,
    INTERFACE_PERSONALSHOPPURCHASE,
    INTERFACE_NPCGUILDMASTER,
    INTERFACE_GUARDSMAN,
    INTERFACE_SENATUS,
    INTERFACE_GATEKEEPER,
    INTERFACE_CATAPULTATTACK,
    INTERFACE_CATAPULTDEFENSE,
    INTERFACE_GATESWITCH,
    INTERFACE_CHARACTER,
    INTERFACE_INVENTORY,
    INTERFACE_REFINERY,
    INTERFACE_REFINERYINFO,
    INTERFACE_KANTURU2ND_ENTERNPC,
    INTERFACE_MAP_ENTRANCE,
    INTERFACE_MAX_COUNT,
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