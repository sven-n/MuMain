
#include "stdafx.h"
#include "UI/Inventory/MyShopInventory.h"
#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "I18N/All.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>

const int iMAX_SHOPTITLE_MULTI = 26;

namespace
{
    void RenderText(const wchar_t* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort, HFONT hFont = g_hFont)
    {
        g_pRenderText->SetFont(hFont);

        DWORD backuptextcolor = g_pRenderText->GetTextColor();
        DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

        g_pRenderText->SetTextColor(color);
        g_pRenderText->SetBgColor(backcolor);
        g_pRenderText->RenderText(x, y, text, sx, sy, sort);

        g_pRenderText->SetTextColor(backuptextcolor);
        g_pRenderText->SetBgColor(backuptextbackcolor);
    }
};

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CMyShopInventory::CMyShopInventory() : m_SourceIndex(-1), m_TargetIndex(-1), m_EnablePersonalShop(false)
{
    m_pNewUIMng = NULL;
    m_pNewInventoryCtrl = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_EditBox = NULL;
    m_bIsEnableInputValueTextBox = false;
    m_bOpenLocked = false;
    m_bOpenApplyTooltip = false;
}

mu::ui::window::CMyShopInventory::~CMyShopInventory()
{
    Release();
}

bool mu::ui::window::CMyShopInventory::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MYSHOP_INVENTORY, this);

    SetPos(x, y);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::MYSHOP, g_pNewUI3DRenderMng, g_pNewItemMng, this, m_Pos.x + 16, m_Pos.y + 90, 8, 4, MAX_MY_INVENTORY_EX_INDEX))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    m_pNewInventoryCtrl->SetToolTipType(TOOLTIP_TYPE_MY_SHOP);

    m_EditBox = new CUITextInputBox;

    m_EditBox->Init(g_hWnd, 200, 14, iMAX_SHOPTITLE_MULTI - 1);
    m_EditBox->SetPosition(m_Pos.x + 50, m_Pos.y + 55);
    m_EditBox->SetTextColor(255, 255, 230, 210);
    m_EditBox->SetBackColor(0, 0, 0, 25);
    m_EditBox->SetFont(g_hFont);

    ChangeEditBox(UISTATE_NORMAL);
    ChangePersonal(m_EnablePersonalShop);

    // Guarded so the document/model are created once, even if Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "my_shop",
            [this](Rml::DataModelConstructor& c, MyShopRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("title", &model.title);
                c.Bind("exit_tooltip", &model.exitTooltip);

                c.Bind("open_locked", &model.openLocked);
                c.Bind("open_tooltip", &model.openTooltip);

                c.Bind("close_locked", &model.closeLocked);
                c.Bind("close_tooltip", &model.closeTooltip);

                c.BindEventCallback("my_shop_exit_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
                    });
                c.BindEventCallback("my_shop_open_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        wchar_t shopTitle[MAX_SHOPTITLE + 1]{};
                        GetTitle(shopTitle);
                        if (IsExistUndecidedPrice() == false && wcslen(shopTitle) > 0)
                        {
                            if (m_EnablePersonalShop == false)
                            {
                                mu::ui::window::GenericDialogConfig cfg;
                                cfg.buttons = mu::ui::window::GenericDialogConfig::ButtonSet::OkCancel;
                                cfg.lines.push_back({ I18N::Game::DoYouWantToOpenAStore, false });
                                cfg.onPrimary = [this]
                                {
                                    wchar_t confirmedTitle[MAX_SHOPTITLE]{};
                                    GetTitle(confirmedTitle);
                                    wcscpy(g_szPersonalShopTitle, confirmedTitle);
                                    SocketClient->ToGameServer()->SendPlayerShopOpen(MU_C16(confirmedTitle));

                                    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
                                    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY);
                                };
                                mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                            }
                            else
                            {
                                wcscpy(g_szPersonalShopTitle, shopTitle);
                                SocketClient->ToGameServer()->SendPlayerShopOpen(MU_C16(shopTitle));

                                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
                                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY);
                                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY_EXT);
                            }
                        }
                        else
                        {
                            g_pSystemLogBox->AddText(I18N::Game::ThereSNoStoreNameOrItemPrice, mu::ui::window::TYPE_ERROR_MESSAGE);
                        }
                    });
                c.BindEventCallback("my_shop_close_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        SocketClient->ToGameServer()->SendPlayerShopClose();

                        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
                        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY);
                        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY_EXT);
                    });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/my_shop.rml");

        // Frame background panel uses the background context -- see MyShopBgRmlModel (MyShopInventory.h).
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "my_shop_bg",
                [](Rml::DataModelConstructor& c, MyShopBgRmlModel& model)
                {
                    c.Bind("root_x", &model.rootX);
                    c.Bind("root_y", &model.rootY);
                    c.Bind("root_scale", &model.rootScale);
                });
            if (bgModelCreated)
            {
                // Shown immediately (unlike m_pRmlDoc) -- Render() only runs while this window is
                // visible, so there's no "wrong scene" case to guard against here.
                m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/my_shop_bg.rml");
            }
        }

        // Not Show()n here -- m_pRmlDoc's visibility follows this window's own Show()/Hide() via
        // SyncRmlModel(), not an eager Show() at Create() time.
    }

    Show(false);

    return true;
}

void mu::ui::window::CMyShopInventory::Release()
{
    SAFE_DELETE(m_pNewInventoryCtrl);
    SAFE_DELETE(m_EditBox);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    // Hide explicitly -- Release() has no other way to hide these once created.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();
}

void mu::ui::window::CMyShopInventory::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->SetPos(m_Pos.x + 16, m_Pos.y + 90);
    }
}

void mu::ui::window::CMyShopInventory::GetTitle(wchar_t* titletext)
{
     m_EditBox->GetText(titletext, iMAX_SHOPTITLE_MULTI);
}

void mu::ui::window::CMyShopInventory::SetTitle(wchar_t* titletext)
{
    m_EditBox->SetText(titletext);
}

bool mu::ui::window::CMyShopInventory::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

void mu::ui::window::CMyShopInventory::DeleteItem(int iIndex)
{
    if (m_pNewInventoryCtrl)
    {
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
        if (pItem != NULL)
            m_pNewInventoryCtrl->RemoveItem(pItem);
    }
}

void mu::ui::window::CMyShopInventory::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->RemoveAllItems();
    }
}

ITEM* mu::ui::window::CMyShopInventory::FindItem(int iLinealPos)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItem(iLinealPos);
    return NULL;
}

void mu::ui::window::CMyShopInventory::ChangePersonal(bool state)
{
    m_EnablePersonalShop = state;

    // Close's lock state is purely a function of m_EnablePersonalShop (see SyncRmlModel()); only
    // Open's lock/tooltip need their own state, since OpenButtonLock()/UnLock() can also drive them
    // independently of this call.
    m_bOpenLocked = false;
    m_bOpenApplyTooltip = m_EnablePersonalShop;
}

void mu::ui::window::CMyShopInventory::OpenButtonLock()
{
    m_bOpenLocked = true;
    m_bOpenApplyTooltip = false;
}

void mu::ui::window::CMyShopInventory::OpenButtonUnLock()
{
    m_bOpenLocked = false;
    m_bOpenApplyTooltip = true;
}

const bool mu::ui::window::CMyShopInventory::IsEnablePersonalShop() const
{
    return m_EnablePersonalShop;
}

void mu::ui::window::CMyShopInventory::ChangeEditBox(const UISTATES type)
{
    m_EditBox->SetState(type);

    if (type == UISTATE_NORMAL)
    {
        m_EditBox->GiveFocus();
    }

}

bool mu::ui::window::CMyShopInventory::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MYSHOP_INVENTORY) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool mu::ui::window::CMyShopInventory::MyShopInventoryProcess()
{
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY) == false)
    {
        return false;
    }

    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();

    if (m_pNewInventoryCtrl && pPickedItem && IsRelease(VK_LBUTTON))
    {
        ITEM* pItemObj = pPickedItem->GetItem();
        int iSourceIndex = pPickedItem->GetSourceLinealPos();
        int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

#ifndef KJH_FIX_CHANGE_ITEM_PRICE_IN_PERSONAL_SHOP				// #ifndef
        if (IsPersonalShopBan(pItemObj))
            m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f);
        else
            m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f);
#endif // KJH_FIX_CHANGE_ITEM_PRICE_IN_PERSONAL_SHOP

        if (iTargetIndex == -1)
        {
            return true;
        }

        if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
        {
            if (IsPersonalShopBan(pItemObj) == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::ThisItemIsNotAllowedToUseThePrivateStore, mu::ui::window::TYPE_ERROR_MESSAGE);
                return true;
            }

            if (m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                ChangeSourceIndex(iSourceIndex);
                ChangeTargetIndex(iTargetIndex);

                CreateMessageBox(MSGBOX_LAYOUT_CLASS(CPersonalShopItemValueMsgBoxLayout));
                SetInputValueTextBox(true);

                pPickedItem->HidePickedItem();
                return true;
            }
        }
        else if (pPickedItem->GetOwnerInventory() == NULL)
        {
            if (IsPersonalShopBan(pItemObj) == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::ThisItemIsNotAllowedToUseThePrivateStore, mu::ui::window::TYPE_ERROR_MESSAGE);
                return true;
            }

            if (m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                ChangeSourceIndex(iSourceIndex);
                ChangeTargetIndex(iTargetIndex);

                CreateMessageBox(MSGBOX_LAYOUT_CLASS(CPersonalShopItemValueMsgBoxLayout));
                SetInputValueTextBox(true);

                pPickedItem->HidePickedItem();
                return true;
            }
        }
        else if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
        {
            if (m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                ChangeSourceIndex(iSourceIndex);
                ChangeTargetIndex(iTargetIndex);
                SendRequestEquipmentItem(STORAGE_TYPE::MYSHOP, iSourceIndex, pItemObj,
                    STORAGE_TYPE::MYSHOP, iTargetIndex);
                return true;
            }
        }
    }
    else if (m_pNewInventoryCtrl && !pPickedItem && IsPress(VK_RBUTTON))
    {
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;

        int iCurSquareIndex = m_pNewInventoryCtrl->GetIndexAtPt(MouseX, MouseY);

        if (iCurSquareIndex != -1)
        {
            ITEM* pItem = g_pMyShopInventory->FindItem(iCurSquareIndex);

            if(pItem)
            {
                ChangeSourceIndex(iCurSquareIndex);
                ChangeTargetIndex(-1);
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(CPersonalShopItemValueMsgBoxLayout));
                SetInputValueTextBox(true);
            }
            return true;
        }
    }

    return false;
}

bool mu::ui::window::CMyShopInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
    {
        return false;
    }

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY))
    {
        if (MyShopInventoryProcess() == true)
        {
            return false;
        }

        // Top-right corner close "X" (shared frame): hides + swallows the click.
        if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_MYSHOP_INVENTORY))
        {
            return false;
        }

        if (mu::ui::window::IsRelease(VK_LBUTTON)
            && CheckMouseIn(m_EditBox->GetPosition_x(), m_EditBox->GetPosition_y(), m_EditBox->GetWidth(), m_EditBox->GetHeight()))
        {
            ChangeEditBox(UISTATE_NORMAL);
        }

        if (mu::ui::window::IsRelease(VK_LBUTTON)
            && CheckMouseIn(m_EditBox->GetPosition_x(), m_EditBox->GetPosition_y(), m_EditBox->GetWidth(), m_EditBox->GetHeight()) == false)
        {
            SetFocus(g_hWnd);
            CUITextInputBox::ReleaseFocus();
        }
    }

    m_EditBox->DoAction();

    // The 3 real buttons (Exit/Open/Close) are handled by RmlUi's data-event-click (see Create()).

    if (WindowProcess())
        return false;

    return true;
}

bool mu::ui::window::CMyShopInventory::WindowProcess()
{
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT).Contains(MouseX, MouseY) == false)
    {
        return false;
    }

    if (mu::ui::window::IsPress(VK_RBUTTON))
    {
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;
    }

    return true;
}

bool mu::ui::window::CMyShopInventory::Update()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
    {
        return false;
    }

    SyncRmlModel();
    return true;
}

void mu::ui::window::CMyShopInventory::SyncRmlModel()
{
    if (m_pRmlBgDoc)
    {
        UI::RmlBridge::SyncRootTransform(m_BgRmlBinder, m_Pos);

        // RenderBackgroundLayer() renders whatever's shown in the shared background context
        // regardless of caller, so this Hide()/Show() is what keeps the bg panel hidden when closed.
        if (IsVisible()) m_pRmlBgDoc->Show(); else m_pRmlBgDoc->Hide();
    }

    if (!m_pRmlDoc) return;
    if (IsVisible()) m_pRmlDoc->Show(); else m_pRmlDoc->Hide();

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);

    auto& model = m_RmlBinder.GetModel();
    auto syncBool = [&](bool MyShopRmlModel::* field, const char* boundName, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncWide = [&](Rml::String MyShopRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
    };

    syncWide(&MyShopRmlModel::title, "title", I18N::Game::PersonalStore);
    syncWide(&MyShopRmlModel::exitTooltip, "exit_tooltip", I18N::Game::Close388);

    syncBool(&MyShopRmlModel::openLocked, "open_locked", m_bOpenLocked);
    syncWide(&MyShopRmlModel::openTooltip, "open_tooltip", m_bOpenApplyTooltip ? I18N::Game::Apply : I18N::Game::Open1107);

    syncBool(&MyShopRmlModel::closeLocked, "close_locked", !m_EnablePersonalShop);
    syncWide(&MyShopRmlModel::closeTooltip, "close_tooltip", I18N::Game::Closed);
}

void mu::ui::window::CMyShopInventory::RenderTextInfo()
{
    wchar_t Text[100];

    if (m_EnablePersonalShop)
    {
        RenderText(I18N::Game::StillOpening, m_Pos.x, m_Pos.y + 200, INVENTORY_WIDTH, 0, RGBA(215, 138, 0, 255), 0x00000000, RT3_SORT_CENTER, g_hFontBold);
    }

    memset(&Text, 0, sizeof(wchar_t) * 100);
    mu_swprintf(Text, I18N::Game::Warning);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 230, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    mu_swprintf(Text, I18N::Game::SellingPriceWhenOpeningTheStore);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 250, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    mu_swprintf(Text, I18N::Game::PleaseVerify);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 262, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    mu_swprintf(Text, I18N::Game::AlreadyInThePersonalStore);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 274, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    mu_swprintf(Text, I18N::Game::CancelSoldItem);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 286, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    mu_swprintf(Text, I18N::Game::CanTBeReturned);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 298, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    mu_swprintf(Text, I18N::Game::AllItemTrading);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 320, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    mu_swprintf(Text, I18N::Game::CanOnlyBeDoneUsingZen);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 332, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold);
}

bool mu::ui::window::CMyShopInventory::Render()
{
    EnableAlphaTest();

    // Frame background panel is RmlUi, routed through the background context (see
    // MyShopBgRmlModel), painted by CManager::Render()'s centralized RenderBackgroundLayer() call
    // before this window's own Render()/Render3D() run.
    RenderTextInfo();

    if (m_EditBox)
    {
        m_EditBox->Render();
    }

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->Render();
    }

    DisableAlphaBlend();

    return true;
}

void mu::ui::window::CMyShopInventory::ClosingProcess()
{
    CInventoryCtrl::BackupPickedItem();
    g_pMyInventory->ChangeMyShopButtonStateOpen();
    SetFocus(g_hWnd);
    CUITextInputBox::ReleaseFocus();
}

int mu::ui::window::CMyShopInventory::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

int mu::ui::window::CMyShopInventory::GetItemInventoryIndex(ITEM* pItem)
{
    return m_pNewInventoryCtrl->GetIndexByItem(pItem);
}

void mu::ui::window::CMyShopInventory::ResetSubject()
{
    if (m_EditBox)
    {
        m_EditBox->SetText(NULL);
    }
}

bool mu::ui::window::CMyShopInventory::IsEnableInputValueTextBox()
{
    return m_bIsEnableInputValueTextBox;
}

void mu::ui::window::CMyShopInventory::SetInputValueTextBox(bool bIsEnable)
{
    m_bIsEnableInputValueTextBox = bIsEnable;
}
