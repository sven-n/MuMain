//////////////////////////////////////////////////////////////////////////
//
//  UIManager.cpp
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Pets/GIPetManager.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Items/CComGem.h"
#include "Audio/DSPlaySound.h"
#include "UI/Widgets/UIControls.h"  // CUITextInputBox::GetFocusedPortable (issue #447)
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/NPCs/NPCShop.h"

extern int g_iKeyPadEnable;
extern int g_iChatInputType;
extern DWORD g_dwMouseUseUIID;

CUIGateKeeper* g_pUIGateKeeper = NULL;
JewelHarmonyInfo* g_pUIJewelHarmonyinfo = NULL;
ItemAddOptioninfo* g_pItemAddOptioninfo = NULL;
CUIPopup* g_pUIPopup = NULL;

extern int g_iCancelSkillTarget;
extern int TextNum;
extern int g_iItemInfo[16][17];

namespace mu::ui::window
{
	int& MouseY = ::MouseY;
	int& g_iCancelSkillTarget = ::g_iCancelSkillTarget;
	wchar_t (&TextList)[50][100] = ::TextList;
	int (&TextListColor)[50] = ::TextListColor;
	int (&TextBold)[50] = ::TextBold;
	int& TextNum = ::TextNum;
	int (&g_iItemInfo)[16][17] = ::g_iItemInfo;
	int& ItemHelp = ::ItemHelp;
	JewelHarmonyInfo*& g_pUIJewelHarmonyinfo = ::g_pUIJewelHarmonyinfo;
	int& SelectedCharacter = ::SelectedCharacter;
}

extern int   ShopInventoryStartX;
extern int   ShopInventoryStartY;

extern bool g_bTradeEndByOppenent;
extern bool LogOut;

bool HeroInventoryEnable = false;
bool GuildListEnable = false;
bool EnableGuildInterface = false;
bool StorageInventoryEnable = false;
bool MixInventoryEnable = false;
bool g_bPersonalShopWnd = false;
bool g_bServerDivisionEnable = false;

CUIManager::CUIManager()
{
    g_pUIGateKeeper = new CUIGateKeeper;
    g_pUIPopup = new CUIPopup;
    g_pUIJewelHarmonyinfo = JewelHarmonyInfo::MakeInfo();
    g_pItemAddOptioninfo = ItemAddOptioninfo::MakeInfo();

    Init();
}

CUIManager::~CUIManager()
{
    SAFE_DELETE(g_pItemAddOptioninfo);
    SAFE_DELETE(g_pUIJewelHarmonyinfo);
    SAFE_DELETE(g_pUIGateKeeper);
    SAFE_DELETE(g_pUIPopup);
}

void CUIManager::Init()
{
    g_pUIPopup->Init();
    giPetManager::InitPetManager();
    ClearPersonalShop();
}

POINT CUIManager::RenderWindowBase(int nHeight, int nOriginY)
{
    int nWidth = 213;

    EnableAlphaBlend3();
    constexpr float WindowAlpha = 0.8f;

    float fPosX = 320 - nWidth / 2;
    float fPosY;
    if (nOriginY == -1)
    {
        fPosY = 130 - nHeight / 2;
        if (fPosY < 40.0f)		fPosY = 40.0f;
    }
    else
        fPosY = nOriginY;
    POINT ptPos = { static_cast<LONG>(fPosX), static_cast<LONG>(fPosY) };

    RenderBitmap(BITMAP_INTERFACE + 22, fPosX, fPosY, nWidth, 5, 0.f, 0.f,
        nWidth / 256.f, 5.f / 8.f, true, true, WindowAlpha);
    fPosY += 5;

    int nBodyHeight = nHeight - 10;
    int nPatternCount = nBodyHeight / 40;
    for (int i = 0; i < nPatternCount; ++i)
    {
        RenderBitmap(BITMAP_INTERFACE + 21, fPosX, fPosY, nWidth, 40, 0.f, 0.f,
            nWidth / 256.f, 40.f / 64.f, true, true, WindowAlpha);
        fPosY += 40;
    }

    if (nBodyHeight > nPatternCount * 40)
    {
        float fRate = (float)(nBodyHeight - nPatternCount * 40) / 40.0f;
        RenderBitmap(BITMAP_INTERFACE + 21, fPosX, fPosY, nWidth, 40 * fRate, 0.f, 0.f,
            nWidth / 256.f, (40.f / 64.f) * fRate, true, true, WindowAlpha);
        fPosY += 40 * fRate;
    }

    RenderBitmap(BITMAP_INTERFACE + 22, fPosX, fPosY, nWidth, 5, 0.f, 0.f,
        nWidth / 256.f, 5.f / 8.f, true, true, WindowAlpha);

    DisableAlphaBlend();

    return ptPos;
}

bool CUIManager::PressKey(int nKey)
{
    return false;
}

bool CUIManager::IsInputEnable()
{
    if (InputEnable || GuildInputEnable || (g_pUIPopup->GetPopupID() != 0 && g_pUIPopup->IsInputEnable()))
        return true;
    // A focused portable text field captures the keyboard without taking Win32 focus,
    // so report "input active" explicitly to suppress world/camera keys while typing.
    if (CUITextInputBox::GetFocusedPortable() != nullptr)
        return true;
    if (GetFocus() == g_hWnd)
        return false;
    return true;
}

void CUIManager::UpdateInput()
{
}

void CUIManager::Render()
{
}

void CUIManager::CloseAll()
{
    for (DWORD dwInterface = MUTEX_FRIEND; dwInterface < MUTEX_MAX_COUNT; ++dwInterface)
    {
        if (g_pUIManager->IsOpen(dwInterface))
        {
            Close(dwInterface);
        }
    }

    g_pUIPopup->CancelPopup();
}

bool CUIManager::CloseInterface(std::list<DWORD>& dwInterfaceFlag, DWORD dwExtraData)
{
    return true;
}

bool CUIManager::IsOpen(DWORD dwInterface)
{
    if (dwInterface == 0)
    {
        for (DWORD dwInterface = MUTEX_FRIEND; dwInterface < MUTEX_MAX_COUNT; ++dwInterface)
        {
            if (IsOpen(dwInterface))
                return true;
        }
    }

    switch (dwInterface)
    {
    case MUTEX_INVENTORY:
        return HeroInventoryEnable;
    case MUTEX_STORAGE:
        return StorageInventoryEnable;
    case MUTEX_PERSONALSHOPSALE:
    case MUTEX_PERSONALSHOPPURCHASE:
        return g_bPersonalShopWnd;
    case MUTEX_SERVERDIVISION:
        return g_bServerDivisionEnable;
    default:
        return false;
    }
    return false;
}

bool CUIManager::IsCanOpen(DWORD dwInterfaceFlag)
{
    return true;
}

void CUIManager::GetInterfaceAll(std::list<DWORD>& outflag)
{
    for (DWORD flag = MUTEX_FRIEND; flag < MUTEX_MAX_COUNT; ++flag)
    {
        outflag.push_back(flag);
    }
}

void CUIManager::GetInsertInterface(std::list<DWORD>& outflag, DWORD insertflag)
{
    outflag.push_back(insertflag);
}

void CUIManager::GetDeleteInterface(std::list<DWORD>& outflag, DWORD deleteflag)
{
    for (auto iter = outflag.begin(); iter != outflag.end(); )
    {
        auto Tempiter = iter;
        ++iter;
        DWORD Tempflag = *Tempiter;

        if (Tempflag == deleteflag)
        {
            outflag.erase(Tempiter);
        }
    }
}

bool CUIManager::Open(DWORD dwInterface, DWORD dwExtraData)
{
    if (IsOpen(MUTEX_REFINERYINFO))
        return false;
    if (IsOpen(dwInterface))
        return false;
    if (IsCanOpen(dwInterface) == false)
        return false;
    if (LogOut == true)
        return false;

    std::list<DWORD> closeinterfaceflag;
    GetInterfaceAll(closeinterfaceflag);
    GetDeleteInterface(closeinterfaceflag, dwInterface);
    GetDeleteInterface(closeinterfaceflag, MUTEX_FRIEND);

    switch (dwInterface)
    {
    case MUTEX_INVENTORY:
    {
        bool bResult = CloseInterface(closeinterfaceflag);
        if (bResult)
        {
            HeroInventoryEnable = true;
        }
    }
    break;
    case MUTEX_PERSONALSHOPSALE:
    {
        GetDeleteInterface(closeinterfaceflag, MUTEX_INVENTORY);

        bool bResult = CloseInterface(closeinterfaceflag);
        if (bResult)
        {
            Open(MUTEX_INVENTORY);

            if (g_iPShopWndType != PSHOPWNDTYPE_NONE) {
                g_ErrorReport.Write(L"@ OpenPersonalShop : SendRequestInventory\n");
                SocketClient->ToGameServer()->SendInventoryRequest();
            }

            CreatePersonalItemTable();

            g_bPersonalShopWnd = true;
            g_iPShopWndType = PSHOPWNDTYPE_SALE;
        }
    }
    break;
    case MUTEX_PERSONALSHOPPURCHASE:
    {
        bool bResult = CloseInterface(closeinterfaceflag);
        if (bResult)
        {
            Open(MUTEX_INVENTORY);

            if (g_iPShopWndType != PSHOPWNDTYPE_NONE) {
                g_ErrorReport.Write(L"@ OpenPersonalShop : SendRequestInventory\n");
                SocketClient->ToGameServer()->SendInventoryRequest();
            }
            CreatePersonalItemTable();

            g_bPersonalShopWnd = true;
            g_iPShopWndType = PSHOPWNDTYPE_PURCHASE;
        }
    }
    break;
    case MUTEX_SERVERDIVISION:
    {
        bool bResult = CloseInterface(closeinterfaceflag);
        if (bResult)
        {
            g_bServerDivisionEnable = true;
            g_bServerDivisionAccept = false;
        }
    }
    break;
    default:
        return false;
    }

    PlayBuffer(SOUND_CLICK01);
    PlayBuffer(SOUND_INTERFACE01);

    return true;
}

bool CUIManager::Close(DWORD dwInterface, DWORD dwExtraData)
{
    if (!IsOpen(dwInterface))		return false;

    switch (dwInterface)
    {
    case MUTEX_INVENTORY:
    {
        bool bResult = true;
        if (bResult)
        {
            std::list<DWORD> closeinterfaceflag;

            GetInsertInterface(closeinterfaceflag, MUTEX_TRADE);
            GetInsertInterface(closeinterfaceflag, MUTEX_STORAGE);
            GetInsertInterface(closeinterfaceflag, MUTEX_GUILDSTORAGE);
            GetInsertInterface(closeinterfaceflag, MUTEX_MIXINVENTORY);
            GetInsertInterface(closeinterfaceflag, MUTEX_PERSONALSHOPSALE);
            GetInsertInterface(closeinterfaceflag, MUTEX_NPCBREEDER);
            GetInsertInterface(closeinterfaceflag, MUTEX_NPCSHOP);
            GetInsertInterface(closeinterfaceflag, MUTEX_SENATUS);
            GetInsertInterface(closeinterfaceflag, MUTEX_REFINERY);
            bResult = CloseInterface(closeinterfaceflag, dwExtraData);
            if (bResult)
            {
                std::list<DWORD> closeflag;
                GetInsertInterface(closeflag, dwInterface);
                CloseInterface(closeflag, dwExtraData);
            }
        }
    }
    break;
    case MUTEX_STORAGE:
    case MUTEX_GUILDSTORAGE:
    case MUTEX_MIXINVENTORY:
    case MUTEX_TRADE:
    case MUTEX_NPCBREEDER:
    case MUTEX_NPCSHOP:
    case MUTEX_GUARDSMAN:
    {
        std::list<DWORD> closeinterfaceflag;
        GetInsertInterface(closeinterfaceflag, dwInterface);
        bool bResult = CloseInterface(closeinterfaceflag, dwExtraData);
        if (bResult)
        {
            std::list<DWORD> closeflag;
            GetInsertInterface(closeflag, MUTEX_INVENTORY);
            CloseInterface(closeflag, dwExtraData);
        }
    }
    break;
    default:
    {
        std::list<DWORD> closeinterfaceflag;
        GetInsertInterface(closeinterfaceflag, dwInterface);
        CloseInterface(closeinterfaceflag, dwExtraData);
    }
    break;
    }

    PlayBuffer(SOUND_CLICK01);
    PlayBuffer(SOUND_INTERFACE01);

    return true;
}
