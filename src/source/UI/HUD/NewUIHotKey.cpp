#include "stdafx.h"
#include "GameLogic/Commands/ChatCommandCatalog.h"
#include "I18N/All.h"

#include "UI/HUD/NewUIHotKey.h"
#include "UI/Core/NewUISystem.h"
#include "UI/Dialogs/NewUICommonMessageBox.h"
#include "UI/Dialogs/NewUICustomMessageBox.h"
#include "Audio/DSPlaySound.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "Engine/Object/ZzzInterface.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "UI/Scaling/UITransform.h"

#include "Render/Effects/ZzzEffect.h"
#include "UI/Widgets/UIControls.h"
#include "UI/Core/NewUIManager.h"
#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "GameShop/InGameShopSystem.h"
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "MUHelper/MuHelper.h"

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CHotKey::CHotKey() : m_pNewUIMng(NULL), m_bStateGameOver(false)
{
}

mu::ui::window::CHotKey::~CHotKey()
{
    Release();
}

bool mu::ui::window::CHotKey::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_HOTKEY, this);
    Show(true);
    return true;
}

void mu::ui::window::CHotKey::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CHotKey::UpdateMouseEvent()
{
    if (g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
    {
        return true;
    }

    if (SelectedCharacter >= 0)
    {
        if (mu::ui::window::IsRepeat(VK_MENU) && mu::ui::window::IsRelease(VK_RBUTTON)
            && gMapManager.InChaosCastle() == false
            && gMapManager.IsCursedTemple() == false
            )
        {
            CHARACTER* pCha = &CharactersClient[SelectedCharacter];

            if (pCha->Object.Kind != KIND_PLAYER)
            {
                return false;
            }

            if ((pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_DEER)
                || (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_SNOWMAN)
                || (pCha->Object.SubType == MODEL_XMAS_EVENT_CHA_SSANTA)
                )
            {
                return false;
            }

            if (::IsStrifeMap(gMapManager.WorldActive) && Hero->m_byGensInfluence != pCha->m_byGensInfluence)
                return false;

            float fPos_x = pCha->Object.Position[0] - Hero->Object.Position[0];
            float fPos_y = pCha->Object.Position[1] - Hero->Object.Position[1];
            float fDistance = sqrtf((fPos_x * fPos_x) + (fPos_y * fPos_y));

            if (fDistance < 300.f)
            {
                int x, y;
                x = MouseX + 10;
                y = MouseY - 50;
                if (y < 0)
                {
                    y = 0;
                }
                g_pQuickCommand->OpenQuickCommand(pCha->ID, SelectedCharacter, x, y);
            }
            else
            {
                g_pSystemLogBox->AddText(I18N::Game::ItCannotBeUsedDueToTheDistance, mu::ui::window::TYPE_ERROR_MESSAGE);
                g_pQuickCommand->CloseQuickCommand();
            }

            return false;
        }
    }

    return true;
}

bool mu::ui::window::CHotKey::UpdateKeyEvent()
{
    if (mu::ui::window::IsPress(VK_ESCAPE) == true)
    {
        if (g_MessageBox->IsEmpty())
        {
            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CSystemMenuMsgBoxLayout));
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    if (m_bStateGameOver == true)
    {
        return false;
    }

    if (mu::ui::window::IsPress(VK_TAB) == false && g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MINI_MAP) == true)
    {
        return false;
    }

    if (g_isCharacterBuff((&Hero->Object), eBuff_DuelWatch))
    {
        if (mu::ui::window::IsPress('M') == true)
        {
            g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_MOVEMAP);
            PlayBuffer(SOUND_CLICK01);
        }
        return false;
    }

    if (AutoGetItem() == true)
    {
        return false;
    }

    if (CanUpdateKeyEventRelatedMyInventory() == true)
    {
        if (mu::ui::window::IsPress('I') || mu::ui::window::IsPress('V'))
        {
            if (g_pNPCShop->IsSellingItem() == false)
            {
                g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_INVENTORY);
                PlayBuffer(SOUND_CLICK01);
                return false;
            }
        }

        return true;
    }
    else if (CanUpdateKeyEvent() == false)
    {
        return true;
    }

    if (mu::ui::window::IsPress('F'))
    {
        if (gMapManager.InChaosCastle() == true)
        {
            return true;
        }

        int iLevel = CharacterAttribute->Level;

        if (iLevel < 6)
        {
            if (g_pSystemLogBox->CheckChatRedundancy(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction) == FALSE)
            {
                g_pSystemLogBox->AddText(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction, mu::ui::window::TYPE_SYSTEM_MESSAGE);
            }
        }
        else
        {
            g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_FRIEND);
        }

        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress('I') || mu::ui::window::IsPress('V'))
    {
        if (g_pNPCShop->IsSellingItem() == false)
        {
            g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_INVENTORY);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    else if (mu::ui::window::IsPress('C'))
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_CHARACTER);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress('T'))
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_MYQUEST);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress('P'))
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_PARTY);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress('G'))
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_GUILDINFO);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress('A'))
    {
        if (gCharacterManager.IsMasterLevel(Hero->Class) == true
            && Hero->Class != CLASS_TEMPLENIGHT)
        {
            g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_MASTER_LEVEL);
        }

        PlayBuffer(SOUND_CLICK01);

        return false;
    }
    else if (mu::ui::window::IsPress('U'))
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_WINDOW_MENU);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (gMapManager.InChaosCastle() == false && mu::ui::window::IsPress('D'))
    {
        if (::IsStrifeMap(gMapManager.WorldActive))
        {
            if (g_pSystemLogBox->CheckChatRedundancy(I18N::Game::TheCommandWindowCannotBeActivatedInBattleZone) == FALSE)
                g_pSystemLogBox->AddText(I18N::Game::TheCommandWindowCannotBeActivatedInBattleZone, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        }
        else
        {
            g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_COMMAND);
            PlayBuffer(SOUND_CLICK01);
        }

        return false;
    }
    else if (mu::ui::window::IsPress(VK_F1) == true)
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_HELP);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress('J') == true)
    {
        // Only servers which offer their chat commands have something to show.
        if (GameLogic::Commands::Catalog().IsAvailable())
        {
            g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_COMMAND_LIST);
            PlayBuffer(SOUND_CLICK01);
        }

        return false;
    }
    else if (mu::ui::window::IsPress('M') == true)
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_MOVEMAP);
        PlayBuffer(SOUND_CLICK01);

        return false;
    }
    else if (mu::ui::window::IsPress(VK_TAB) == true && gMapManager.InBattleCastle() == true)
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_SIEGEWARFARE);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress(VK_TAB) == true)
    {
        if (g_pNewUIMiniMap->m_bSuccess == false)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MINI_MAP);
        }
        else
            g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_MINI_MAP);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    else if (mu::ui::window::IsPress('X') == true)
    {
        g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt CallStack - CHotKey.UpdateKeyEvent()");
        if (g_pInGameShop->IsInGameShopOpen() == false)
            return false;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
        if (g_InGameShopSystem->IsScriptDownload() == true)
        {
            if (g_InGameShopSystem->ScriptDownload() == false)
                return false;
        }
        if (g_InGameShopSystem->IsBannerDownload() == true)
        {
            if (g_InGameShopSystem->BannerDownload() == true)
            {
                g_pInGameShop->InitBanner(g_InGameShopSystem->GetBannerFileName(), g_InGameShopSystem->GetBannerURL());
            }
        }
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_INGAMESHOP) == false)
        {
            if (g_InGameShopSystem->GetIsRequestShopOpenning() == false)
            {
                SocketClient->ToGameServer()->SendCashShopOpenState(0);
                g_InGameShopSystem->SetIsRequestShopOpenning(true);
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
                g_pMainFrame->SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
            }
        }
        else
        {
            SocketClient->ToGameServer()->SendCashShopOpenState(1);
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INGAMESHOP);
        }

        return false;
    }
#endif // PBG_ADD_INGAMESHOP_UI_MAINFRAME
    else if (mu::ui::window::IsPress('B'))
    {
        if (!g_pNewUIGensRanking->SetGensInfo())
            return false;

        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_GENSRANKING);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress(VK_HOME) && !g_pChatInputBox->HaveFocus())
    {
        MUHelper::g_MuHelper.Toggle();
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    else if (mu::ui::window::IsPress('Z'))
    {
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_MUHELPER);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    return true;
}

bool mu::ui::window::CHotKey::Update()
{
    return true;
}

bool mu::ui::window::CHotKey::Render()
{
    return true;
}

bool mu::ui::window::CHotKey::CanUpdateKeyEventRelatedMyInventory()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MIXINVENTORY)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_STORAGE)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCSHOP)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MYSHOP_INVENTORY)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYITEMWND)
        )
    {
        return true;
    }
    return false;
}

bool mu::ui::window::CHotKey::CanUpdateKeyEvent()
{
    if (CUITextInputBox::IsAnyInputBoxFocused())
    {
        return false;
    }

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_KANTURU2ND_ENTERNPC)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CATAPULT)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCQUEST)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_SENATUS)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GATEKEEPER)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GUARDSMAN)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GATESWITCH)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCGUILDMASTER)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_BLOODCASTLE)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_DEVILSQUARE)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MASTER_LEVEL)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_DUELWATCH)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_DOPPELGANGER_NPC)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPC_DIALOGUE)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUEST_PROGRESS)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUEST_PROGRESS_ETC)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GOLD_BOWMAN)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GOLD_BOWMAN_LENA)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_LUCKYCOIN_REGISTRATION)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_EXCHANGE_LUCKYCOIN)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_EMPIREGUARDIAN_NPC)
        || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA)
        )
    {
        return false;
    }

    return true;
}

float mu::ui::window::CHotKey::GetLayerDepth()
{
    return 1.0f;
}

float mu::ui::window::CHotKey::GetKeyEventOrder()
{
    return 1.0f;
}

void mu::ui::window::CHotKey::SetStateGameOver(bool bGameOver)
{
    m_bStateGameOver = bGameOver;
}

bool mu::ui::window::CHotKey::IsStateGameOver()
{
    return m_bStateGameOver;
}

bool mu::ui::window::CHotKey::AutoGetItem()
{
    if (
        CInventoryCtrl::GetPickedItem() == NULL
        && mu::ui::window::IsPress(VK_SPACE)
        && g_pChatInputBox->HaveFocus() == false
        && !UI::Scaling::BottomHudContainsWindowPoint(WindowWidth, WindowHeight,
                                                       g_fWindowMouseX, g_fWindowMouseY)
        )
    {
        for (int i = 0; i < MAX_ITEMS; ++i)
        {
            OBJECT* pObj = &Items[i].Object;
            if (pObj->Live && pObj->Visible)
            {
                vec3_t vDir;
                VectorSubtract(pObj->Position, Hero->Object.Position, vDir);
                if (VectorLength(vDir) < 300)
                {
                    Hero->MovementType = MOVEMENT_GET;
                    ItemKey = i;
                    g_bAutoGetItem = true;
                    Action(Hero, pObj, true);
                    Hero->MovementType = MOVEMENT_MOVE;
                    g_bAutoGetItem = false;

                    return true;
                }
            }
        }
    }

    return false;
}
