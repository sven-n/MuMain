#include "stdafx.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "Guild/GuildMakeWindow.h"
#include "Guild/GuildInfoWindow.h"
#include "UI/Inventory/MyInventory.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Guild/GuildTypes.h"
#include "UI/Core/UIManager.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "GameLogic/Items/CComGem.h"
#include "GameLogic/Items/MixMgr.h"
#include "GameLogic/Quests/CSQuest.h"
#include "World/MapInfra/PortalMgr.h"
#include "GameLogic/Social/GambleSystem.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "UI/Inventory/LuckyItemWnd.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Engine/Object/ZzzInterface.h"
#include "I18N/All.h"
#include "Core/Text/TextLineWrap.h"
#include "Render/Text/CUIRenderText.h"

using namespace SEASON3B;
using namespace mu::ui::window;

extern wchar_t DeleteID[];
extern int DeleteIndex, AppointStatus;
extern int Button_Down;
extern int BackUp_Key;
extern BYTE m_AltarState[];

extern BYTE Rank;
extern int Exp;
extern BYTE Ranking[5];
extern CLASS_TYPE HeroClass[5];
extern int HeroScore[5];
extern wchar_t HeroName[5][MAX_USERNAME_SIZE + 1];
extern char	View_Suc_Or_Fail;

extern int BuyCost;

mu::ui::window::CMessageBoxButton::CMessageBoxButton()
{
    m_bEnable = true;

    m_dwTexType = 0;
    m_x = m_y = m_width = m_height = 0.f;

    m_dwSizeType = MSGBOX_BTN_SIZE_OK;

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_iMoveTextPosX = 0;
    m_iMoveTextPosY = 0;
    m_bClickEffect = false;
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

mu::ui::window::CMessageBoxButton::~CMessageBoxButton()
{
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CMessageBoxButton::MoveTextPos(int iX, int iY)
{
    m_iMoveTextPosX = iX;
    m_iMoveTextPosY = iY;
}

void mu::ui::window::CMessageBoxButton::SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType, bool bClickEffect)
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
void mu::ui::window::CMessageBoxButton::SetInfo(DWORD dwTexType, float x, float y, float width, float height, DWORD dwSizeType)
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
{
    m_dwTexType = dwTexType;
    m_dwSizeType = dwSizeType;
    m_x = x; m_y = y;
    m_width = width; m_height = height;
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
    m_bClickEffect = bClickEffect;

    if (m_dwSizeType == MSGBOX_BTN_SIZE_OK)
    {
        m_fButtonWidth = MSGBOX_BTN_WIDTH;
        m_fButtonHeight = MSGBOX_BTN_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY)
    {
        m_fButtonWidth = MSGBOX_BTN_EMPTY_WIDTH;
        m_fButtonHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY_SMALL)
    {
        m_fButtonWidth = MSGBOX_BTN_EMPTY_SMALL_WIDTH;
        m_fButtonHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    else if (m_dwSizeType == MSGBOX_BTN_SIZE_EMPTY_BIG)
    {
        m_fButtonWidth = MSGBOX_BTN_EMPTY_BIG_WIDTH;
        m_fButtonHeight = MSGBOX_BTN_EMPTY_HEIGHT;
    }
    else
    {
        m_fButtonWidth = width; m_fButtonHeight = height;
    }
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
}

void mu::ui::window::CMessageBoxButton::SetText(const wchar_t* strText)
{
    if (wcslen(strText) > 0)
    {
        m_strText = strText;
    }
}

void mu::ui::window::CMessageBoxButton::AddBlank(int iAddBlank)
{
    m_y += iAddBlank;
}

// CPersonalShopItemValueCheckMsgBoxLayout is now CGenericConfirmDialog-based
// (MyShopInventory.cpp's ShowPersonalShopItemValueDialog()).


