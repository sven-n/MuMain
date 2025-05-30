//////////////////////////////////////////////////////////////////////////
//
//  UIManager.cpp
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "GIPetManager.h"
#include "PersonalShopTitleImp.h"
#include "CComGem.h"
#include "DSPlaySound.h"
#include "NewUICustomMessageBox.h"
#include "NewUINPCShop.h"

extern int g_iKeyPadEnable;
extern int g_iChatInputType;
extern DWORD g_dwMouseUseUIID;

CUIGateKeeper* g_pUIGateKeeper = NULL;
JewelHarmonyInfo* g_pUIJewelHarmonyinfo = NULL;
ItemAddOptioninfo* g_pItemAddOptioninfo = NULL;
CUIPopup* g_pUIPopup = NULL;




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
    glColor4f(1.0f, 1.0f, 1.0f, 0.8f);

    float fPosX = 320 - nWidth / 2;
    float fPosY;
    if (nOriginY == -1)
    {
        fPosY = 130 - nHeight / 2;
        if (fPosY < 40.0f)		fPosY = 40.0f;
    }
    else
        fPosY = nOriginY;
    POINT ptPos = { fPosX, fPosY };

    RenderBitmap(BITMAP_INTERFACE + 22, fPosX, fPosY, nWidth, 5, 0.f, 0.f, nWidth / 256.f, 5.f / 8.f);
    fPosY += 5;

    int nBodyHeight = nHeight - 10;
    int nPatternCount = nBodyHeight / 40;
    for (int i = 0; i < nPatternCount; ++i)
    {
        RenderBitmap(BITMAP_INTERFACE + 21, fPosX, fPosY, nWidth, 40, 0.f, 0.f, nWidth / 256.f, 40.f / 64.f);
        fPosY += 40;
    }

    if (nBodyHeight > nPatternCount * 40)
    {
        float fRate = (float)(nBodyHeight - nPatternCount * 40) / 40.0f;
        RenderBitmap(BITMAP_INTERFACE + 21, fPosX, fPosY, nWidth, 40 * fRate, 0.f, 0.f, nWidth / 256.f, (40.f / 64.f) * fRate);
        fPosY += 40 * fRate;
    }

    RenderBitmap(BITMAP_INTERFACE + 22, fPosX, fPosY, nWidth, 5, 0.f, 0.f, nWidth / 256.f, 5.f / 8.f);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
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
    for (DWORD dwInterface = INTERFACE_FRIEND; dwInterface < INTERFACE_MAX_COUNT; ++dwInterface)
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
        for (DWORD dwInterface = INTERFACE_FRIEND; dwInterface < INTERFACE_MAX_COUNT; ++dwInterface)
        {
            if (IsOpen(dwInterface))
                return true;
        }
    }

    switch (dwInterface)
    {
    case INTERFACE_INVENTORY:
        return HeroInventoryEnable;
    case INTERFACE_STORAGE:
        return StorageInventoryEnable;
    case INTERFACE_PERSONALSHOPSALE:
    case INTERFACE_PERSONALSHOPPURCHASE:
        return g_bPersonalShopWnd;
    case INTERFACE_SERVERDIVISION:
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
    for (DWORD flag = INTERFACE_FRIEND; flag < INTERFACE_MAX_COUNT; ++flag)
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
    if (IsOpen(INTERFACE_REFINERYINFO))
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
    GetDeleteInterface(closeinterfaceflag, INTERFACE_FRIEND);

    switch (dwInterface)
    {
    case INTERFACE_INVENTORY:
    {
        bool bResult = CloseInterface(closeinterfaceflag);
        if (bResult)
        {
            HeroInventoryEnable = true;
        }
    }
    break;
    case INTERFACE_PERSONALSHOPSALE:
    {
        GetDeleteInterface(closeinterfaceflag, INTERFACE_INVENTORY);

        bool bResult = CloseInterface(closeinterfaceflag);
        if (bResult)
        {
            Open(INTERFACE_INVENTORY);

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
    case INTERFACE_PERSONALSHOPPURCHASE:
    {
        bool bResult = CloseInterface(closeinterfaceflag);
        if (bResult)
        {
            Open(INTERFACE_INVENTORY);

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
    case INTERFACE_SERVERDIVISION:
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
    case INTERFACE_INVENTORY:
    {
        bool bResult = true;
        if (bResult)
        {
            std::list<DWORD> closeinterfaceflag;

            GetInsertInterface(closeinterfaceflag, INTERFACE_TRADE);
            GetInsertInterface(closeinterfaceflag, INTERFACE_STORAGE);
            GetInsertInterface(closeinterfaceflag, INTERFACE_GUILDSTORAGE);
            GetInsertInterface(closeinterfaceflag, INTERFACE_MIXINVENTORY);
            GetInsertInterface(closeinterfaceflag, INTERFACE_PERSONALSHOPSALE);
            GetInsertInterface(closeinterfaceflag, INTERFACE_NPCBREEDER);
            GetInsertInterface(closeinterfaceflag, INTERFACE_NPCSHOP);
            GetInsertInterface(closeinterfaceflag, INTERFACE_SENATUS);
            GetInsertInterface(closeinterfaceflag, INTERFACE_REFINERY);
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
    case INTERFACE_STORAGE:
    case INTERFACE_GUILDSTORAGE:
    case INTERFACE_MIXINVENTORY:
    case INTERFACE_TRADE:
    case INTERFACE_NPCBREEDER:
    case INTERFACE_NPCSHOP:
    case INTERFACE_GUARDSMAN:
    {
        std::list<DWORD> closeinterfaceflag;
        GetInsertInterface(closeinterfaceflag, dwInterface);
        bool bResult = CloseInterface(closeinterfaceflag, dwExtraData);
        if (bResult)
        {
            std::list<DWORD> closeflag;
            GetInsertInterface(closeflag, INTERFACE_INVENTORY);
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