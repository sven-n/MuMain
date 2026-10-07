
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Inventory/LuckyItemWnd.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzCharacter.h"

#include "Audio/DSPlaySound.h"
#include "GameLogic/Items/MixMgr.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;
CLuckyItemWnd::CLuckyItemWnd()
{
    memset(m_szSubject, 0, 255);
    m_eType = eLuckyItemType_None;
    m_nMixEffectTimer = 0;
    // Otherwise garbage until OpeningProcess()/SetFrame_Text() first run -- SyncRmlModel() now
    // reads this every Update() tick (not only while the window draws), so an
    // uninitialized value here could drive an out-of-bounds m_sText[] loop before the window is
    // ever opened.
    m_nTextMaxLine = 0;
}

CLuckyItemWnd::~CLuckyItemWnd()
{
#ifdef LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
    Release();
#endif // LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
}

int CLuckyItemWnd::GetLuckyItemRate(int _nType)
{
    if (_nType == eLuckyItemType_Trade)		return 100;
    if (_nType == eLuckyItemType_Refinery)	return 50;

    return 0;
}

STORAGE_TYPE CLuckyItemWnd::SetMoveAction()
{
    m_eWndAction = eLuckyItem_Move;
    switch (m_eType)
    {
    case eLuckyItemType_Trade:
        return STORAGE_TYPE::LUCKYITEM_TRADE;
    case eLuckyItemType_Refinery:
        return STORAGE_TYPE::LUCKYITEM_REFINERY;
    }

    return STORAGE_TYPE::UNDEFINED;
}

int CLuckyItemWnd::SetActAction()
{
    m_eWndAction = eLuckyItem_Act;
    switch (m_eType)
    {
        case eLuckyItemType_Trade:
            return 51;
        case eLuckyItemType_Refinery:
            return 52;
        default:
            return -1;
    }
}

void CLuckyItemWnd::RenderMixEffect()
{
    if (m_nMixEffectTimer <= 0)
    {
        return;
    }
    else
    {
        --m_nMixEffectTimer;
    }

    EnableAlphaBlend();
    for (int i = 0; i < (int)m_pNewInventoryCtrl->GetNumberOfItems(); ++i)
    {
        int iWidth = ItemAttribute[m_pNewInventoryCtrl->GetItem(i)->Type].Width;
        int iHeight = ItemAttribute[m_pNewInventoryCtrl->GetItem(i)->Type].Height;

        for (int h = 0; h < iHeight; ++h)
        {
            for (int w = 0; w < iWidth; ++w)
            {
                const BYTE red = static_cast<BYTE>((rand() % 6 + 6) * 0.1f * 255.f);
                const BYTE green = static_cast<BYTE>((rand() % 4 + 4) * 0.1f * 255.f);
                const DWORD sparkleColor = RGBA(red, green, 51, 255);
                float Rotate = (float)((int)(WorldTime) % 100) * 20.f;
                float Scale = 5.f + (rand() % 10);
                const UI::Items::GridRect cell = m_pNewInventoryCtrl->Geometry().CellsRect(
                    m_pNewInventoryCtrl->GetItem(i)->x + w, m_pNewInventoryCtrl->GetItem(i)->y + h, 1, 1);
                float x = cell.x + (rand() % (std::max)(1, static_cast<int>(cell.width)));
                float y = cell.y + (rand() % (std::max)(1, static_cast<int>(cell.height)));
                RenderBitmapRotate(BITMAP_SHINY, x, y, Scale, Scale, 0, 0.f, 0.f, 1.f, 1.f, sparkleColor);
                RenderBitmapRotate(BITMAP_SHINY, x, y, Scale, Scale, Rotate, 0.f, 0.f, 1.f, 1.f, sparkleColor);
                RenderBitmapRotate(BITMAP_SHINY + 1, x, y, Scale * 3.f, Scale * 3.f, Rotate,
                    0.f, 0.f, 1.f, 1.f, sparkleColor);
                RenderBitmapRotate(BITMAP_LIGHT, x, y, Scale * 6.f, Scale * 6.f, 0,
                    0.f, 0.f, 1.f, 1.f, sparkleColor);
            }
        }
    }
    DisableAlphaBlend();
}

void CLuckyItemWnd::GetResult(BYTE _byResult, int _nIndex, std::span<const BYTE> pbyItemPacket)
{
    int		nDefault = -1;
    int		nMessage = nDefault;
    int		nPlaySound = nDefault;
    int		nAddInven = -1;
    bool	bInitInven = false;

    mu::ui::window::CInventoryCtrl::DeletePickedItem();

    switch (m_eWndAction)
    {
    case eLuckyItem_Move:
        bInitInven = true;
        nAddInven = _nIndex;
        break;
    case eLuckyItem_Act:
        if (_byResult == 1)
        {
            bInitInven = true;
            nAddInven = _nIndex;
            nPlaySound = SOUND_JEWEL01;
            m_nMixEffectTimer = 50;
        }
        else
        {
            if (m_eType == eLuckyItemType_Trade)
            {
                nMessage = 3303;
            }
            else if (m_eType == eLuckyItemType_Refinery)
            {
                bInitInven = true;
            }
        }

        m_eEnd = eLuckyItem_End;
        SetFrame_Text(m_eEnd);
        break;
    }

    if (nMessage > nDefault)	g_pChatListBox->AddText(L"", I18N::Game::Lookup(nMessage), mu::ui::window::TYPE_ERROR_MESSAGE);
    if (nPlaySound > nDefault)	PlayBuffer(static_cast<ESound>(nPlaySound));
    if (bInitInven)			g_pLuckyItemWnd->Process_InventoryCtrl_DeleteItem(-1);
    if (nAddInven > nDefault)	Process_InventoryCtrl_InsertItem(nAddInven, pbyItemPacket);

    m_eWndAction = eLuckyItem_None;
}

bool CLuckyItemWnd::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_LUCKYITEMWND, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::LUCKYITEM_TRADE, g_pNewItemMng, this, x + 15, y + 110, 8, 4))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }
    m_pNewInventoryCtrl->GetSquareColorNormal(m_fInvenClr);
    m_pNewInventoryCtrl->GetSquareColorWarning(m_fInvenClrWarning);

    SetPos(x, y);

    for (int i = 0; i < LUCKYITEMMAXLINE; i++)
    {
        m_sText[i].s_nTextIndex = -1;
        m_sText[i].s_dwColor = 0;
        m_sText[i].s_nLine = false;
    }

    BuildRmlUi();

    Show(false);

    return true;
}

void CLuckyItemWnd::BindRmlModel(Rml::DataModelConstructor& c, LuckyItemRmlModel& model)
{
    c.Bind("root_x", &model.rootX);
    c.Bind("root_y", &model.rootY);
    c.Bind("root_scale", &model.rootScale);
    UI::Items::RegisterItemGridCells(c);
    c.Bind("grid_cells", &model.gridCells);
    c.Bind("text_px", &model.textPx);
    c.Bind("panel_width", &model.panelWidth);
    c.Bind("title", &model.title);
    c.Bind("mix_tooltip", &model.mixTooltip);
    c.Bind("mix_visible", &model.mixVisible);

    auto luckyLine = c.RegisterStruct<LuckyLine>();
    luckyLine.RegisterMember("text", &LuckyLine::text);
    luckyLine.RegisterMember("color", &LuckyLine::color);
    luckyLine.RegisterMember("align", &LuckyLine::align);
    c.RegisterArray<std::vector<LuckyLine>>();
    c.Bind("text_lines", &model.textLines);

    c.BindEventCallback("lucky_item_mix_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            Process_BTN_Action();
        });
}

void CLuckyItemWnd::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CLuckyItemWnd::Release()
{
    m_ItemTarget.Disable();
#ifdef LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
    SAFE_DELETE(m_pNewInventoryCtrl);
#endif // LEM_FIX_LUCKYITEM_UICLASS_SAFEDELETE
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void CLuckyItemWnd::OpeningProcess(void)
{
    for (int i = 0; i < LUCKYITEMMAXLINE; i++)
    {
        m_sText[i].s_nTextIndex = -1;
        m_sText[i].s_dwColor = 0;
        m_sText[i].s_nLine = false;
    }
    m_nTextMaxLine = 0;
    m_eEnd = eLuckyItem_None;
    switch (m_eType)
    {
    case eLuckyItemType_Trade:
        mu_swprintf(m_szSubject, L"%ls", I18N::Game::ExchangeLuckyItem);
        AddText(3291, 0xFF0000FF, RT3_SORT_LEFT), AddText(0), AddText(0), AddText(3292), AddText(3293), AddText(3294);
        AddText(0), AddText(0);
        AddText(2223, 0xFF00FFFF);
        AddText(0);
        AddText(3295, 0xFF0000FF), AddText(3296, 0xFF0000FF);
        // Mix button tooltip text is now derived from m_eType directly in SyncRmlModel().
        break;
    case eLuckyItemType_Refinery:
        mu_swprintf(m_szSubject, L"%ls", I18N::Game::RefineLuckyItem);
        AddText(2346, 0xFF0000FF, RT3_SORT_LEFT), AddText(0), AddText(0);
        AddText(3300), AddText(3301);
        AddText(0), AddText(0), AddText(0);
        AddText(3302, 0xFF0000FF);
        // Mix button tooltip text is now derived from m_eType directly in SyncRmlModel().
        break;
    }
}

void CLuckyItemWnd::SetFrame_Text(eLUCKYITEM _eType)
{
    switch (_eType)
    {
    case eLuckyItem_End:
        for (int i = 0; i < LUCKYITEMMAXLINE; i++)
        {
            m_sText[i].s_nTextIndex = -1;
            m_sText[i].s_dwColor = 0;
            m_sText[i].s_nLine = false;
        }
        m_nTextMaxLine = 0;
        AddText(0);
        if (m_eType == eLuckyItemType_Trade)			AddText(1888);
        else if (m_eType == eLuckyItemType_Refinery) {}
        break;
    }
}

void CLuckyItemWnd::AddText(int _nGlobalTextIndex, DWORD _dwColor, int _nLine)
{
    if (m_nTextMaxLine >= LUCKYITEMMAXLINE)	return;
    m_sText[m_nTextMaxLine].s_nTextIndex = _nGlobalTextIndex;
    m_sText[m_nTextMaxLine].s_dwColor = _dwColor;
    m_sText[m_nTextMaxLine].s_nLine = _nLine;
    m_nTextMaxLine++;
}

bool CLuckyItemWnd::ClosingProcess(void)
{
    if (GetInventoryCtrl()->GetNumberOfItems() > 0 || CInventoryCtrl::GetPickedItem() != NULL)
    {
        g_pChatListBox->AddText(L"", I18N::Game::CloseInventoryAfterMovingYourItemsInTheInventory, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
    m_eType = eLuckyItemType_None;
    return true;
}

bool CLuckyItemWnd::Process_InventoryCtrl_InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    return false;
}

void CLuckyItemWnd::Process_InventoryCtrl_DeleteItem(int iIndex)
{
    if (m_pNewInventoryCtrl)
    {
        if (iIndex == -1)
        {
            m_pNewInventoryCtrl->RemoveAllItems();
            return;
        }
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
        if (pItem != NULL)
            m_pNewInventoryCtrl->RemoveItem(pItem);
    }
}

bool CLuckyItemWnd::Check_LuckyItem_InWnd(void)
{
    if (GetInventoryCtrl()->GetNumberOfItems() > 0)	return true;
    return false;
}

bool CLuckyItemWnd::Check_LuckyItem(ITEM* _pItem)
{
    switch (m_eType)
    {
    case eLuckyItemType_Trade:
        if (Check_LuckyItem_Trade(_pItem))		return true;
        break;
    case eLuckyItemType_Refinery:
        if (Check_LuckyItem_Refinery(_pItem))	return true;
        break;
    }

    return false;
}

bool CLuckyItemWnd::Check_LuckyItem_Trade(ITEM* _pItem)
{
    if (_pItem->Type >= ITEM_HELPER + 135 && _pItem->Type <= ITEM_HELPER + 145)		return true;

    return false;
}

bool CLuckyItemWnd::Check_LuckyItem_Refinery(ITEM* _pItem)
{
    if (_pItem->Type >= ITEM_ARMOR + 62 && _pItem->Type <= ITEM_ARMOR + 72)		return true;
    else if (_pItem->Type >= ITEM_HELM + 62 && _pItem->Type <= ITEM_HELM + 72)		return true;
    else if (_pItem->Type >= ITEM_BOOTS + 62 && _pItem->Type <= ITEM_BOOTS + 72)		return true;
    else if (_pItem->Type >= ITEM_GLOVES + 62 && _pItem->Type <= ITEM_GLOVES + 72)	return true;
    else if (_pItem->Type >= ITEM_PANTS + 62 && _pItem->Type <= ITEM_PANTS + 72)		return true;

    return false;
}

bool CLuckyItemWnd::Process_InventoryCtrl(void)
{
    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
    if (!m_pNewInventoryCtrl)	return false;
    if (!pPickedItem)			return false;

    ITEM* pItemObj = pPickedItem->GetItem();
    bool	bAct = Check_LuckyItem(pItemObj);

    if (!bAct || Check_LuckyItem_InWnd())
    {
        m_pNewInventoryCtrl->SetSquareColorNormal(m_fInvenClrWarning[0], m_fInvenClrWarning[1], m_fInvenClrWarning[2]);
        return false;
    }

    if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
    {
        if (mu::ui::window::IsPress(VK_LBUTTON))
        {
            int iSourceIndex = pPickedItem->GetSourceLinealPos();
            int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
            if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                auto nMoveIndex = SetMoveAction();
                if (SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, iSourceIndex, pItemObj, nMoveIndex, iTargetIndex))
                    return true;
            }
        }
    }
    else if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
    {
        if (mu::ui::window::IsPress(VK_LBUTTON))
        {
            int iSourceIndex = pPickedItem->GetSourceLinealPos();
            int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);
            if (iTargetIndex != -1 && m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                auto nMoveIndex = SetMoveAction();
                if (SendRequestEquipmentItem(nMoveIndex, iSourceIndex, pItemObj, nMoveIndex, iTargetIndex))
                {
                    return true;
                }
            }
        }
    }

    // InventoryCtrl Background Color
    m_pNewInventoryCtrl->SetSquareColorNormal(m_fInvenClr[0], m_fInvenClr[1], m_fInvenClr[2]);
    return false;
}

CInventoryCtrl* CLuckyItemWnd::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

bool CLuckyItemWnd::Process_BTN_Action(void)
{
    // Invoked directly by the RmlUi mix-button's data-event-click callback (see Create()) now,
    // rather than polled every frame against a native CButton hit-test.
    if (m_eEnd == eLuckyItem_End)
        return false;

    if (CInventoryCtrl::GetPickedItem())
        return false;

    if (!Check_LuckyItem_InWnd())
    {
        g_pChatListBox->AddText(L"", I18N::Game::ItemsForCombinationSystemIsLacking, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }
    if (!Check_LuckyItem(m_pNewInventoryCtrl->GetItem(0)))
    {
        g_pChatListBox->AddText(L"", I18N::Game::CorrespondingItemIsInappropriate, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }
#ifdef LEM_FIX_LUCKYITEM_SLOTCHECK
    if (g_pMyInventory->FindEmptySlot(4, 4) == -1)
#else // LEM_FIX_LUCKYITEM_SLOTCHECK
    if (g_pMyInventory->GetInventoryCtrl()->FindEmptySlot(4, 4) == -1)
#endif // LEM_FIX_LUCKYITEM_SLOTCHECK
    {
        g_pChatListBox->AddText(L"", I18N::Game::InventorySpaceIsInsufficient, mu::ui::window::TYPE_ERROR_MESSAGE);
        return false;
    }

    {
        int nTextIndex[10] = { 0, };
        switch (g_pLuckyItemWnd->GetAct())
        {
        case eLuckyItemType_Trade:
            nTextIndex[0] = 3288;
            nTextIndex[1] = 3297;
            nTextIndex[2] = 3298;
            nTextIndex[3] = 3299;
            break;
        case eLuckyItemType_Refinery:
            nTextIndex[0] = 3289;
            nTextIndex[1] = 539;
            break;
        default:
            return false;
        }

        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines.push_back({ I18N::Game::Lookup(nTextIndex[0]), true });
        for (int i = 1; i < 10; ++i)
        {
            if (nTextIndex[i] <= 0)
                break;
            cfg.lines.push_back({ I18N::Game::Lookup(nTextIndex[i]), false });
        }
        cfg.onPrimary = [] { SocketClient->ToGameServer()->SendChaosMachineMixRequest(static_cast<ChaosMachineMixType>(g_pLuckyItemWnd->SetActAction()), 0); };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
    return true;
}

bool CLuckyItemWnd::UpdateMouseEvent(void)
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;
    Process_InventoryCtrl();

    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_ptPos, mu::ui::window::INTERFACE_LUCKYITEMWND);

    // Mix button click is handled by the RmlUi "lucky_item_mix_click" event callback (see
    // Create()), which calls Process_BTN_Action() directly -- not polled here.

    float panelWidth = 190.f;
    float panelHeight = 429.f;
    UI::RmlBridge::RefreshLogicalPanelSize(m_RmlView.Document(), "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_ptPos.x, m_ptPos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
    {
        if (mu::ui::window::IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (mu::ui::window::IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }

    return true;
}

bool CLuckyItemWnd::UpdateKeyEvent(void)
{
    return true;
}

bool CLuckyItemWnd::Update(void)
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
        return false;

    SyncRmlModel();
    return true;
}

bool CLuckyItemWnd::Render(void)
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();
    return true;
}

void CLuckyItemWnd::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("item_view") : nullptr, IsVisible());
    if (!m_RmlView.Document()) return;
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());

    UI::RmlBridge::SyncRootTransform(m_RmlView.Binder(), m_ptPos);
    UI::RmlBridge::SyncPanelWidth(m_RmlView.Binder(), m_RmlView.Document());
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->FollowGrid(m_RmlView.Document(), "item_grid", m_ptPos, 15, 110);
    if (m_pNewInventoryCtrl && m_RmlView.GetModel().gridCells != m_pNewInventoryCtrl->Cells())
    {
        m_RmlView.GetModel().gridCells = m_pNewInventoryCtrl->Cells();
        m_RmlView.MarkDirty("grid_cells");
    }
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    auto& model = m_RmlView.GetModel();
    auto syncWide = [&](Rml::String LuckyItemRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };
    auto syncBool = [&](bool LuckyItemRmlModel::* field, const char* boundName, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };
    auto syncLines = [&](std::vector<LuckyLine> LuckyItemRmlModel::* field, const char* boundName,
        std::vector<LuckyLine> newLines)
    {
        if (model.*field != newLines) { model.*field = std::move(newLines); m_RmlView.MarkDirty(boundName); }
    };

    // m_szSubject is only re-written by OpeningProcess() (on Trade/Refinery mode switch), but it's
    // cheap to re-check every tick the same change-checked way as every other field here.
    syncWide(&LuckyItemRmlModel::title, "title", m_szSubject);

    const wchar_t* mixTooltipText = (m_eType == eLuckyItemType_Refinery) ? I18N::Game::Refine : I18N::Game::Combining;
    syncWide(&LuckyItemRmlModel::mixTooltip, "mix_tooltip", mixTooltipText);

    // Same m_eEnd != eLuckyItem_End condition RenderItems() checks for the mix-completion
    // sparkle effect -- drives the RmlUi mix button's visibility here.
    syncBool(&LuckyItemRmlModel::mixVisible, "mix_visible", m_eEnd != eLuckyItem_End);

    // Former Render_Frame()'s m_sText[]/AddText() text loop. One entry per slot in
    // [0, m_nTextMaxLine), including blank spacer slots (s_nTextIndex == 0), so line spacing in
    // normal document flow matches the original's fixed per-slot vertical rhythm. s_dwColor is
    // packed the same way CUIRenderTextSDLTtf::SetTextColor(DWORD) unpacks it (mu::sdlttf::
    // PackColorDWORD: alpha<<24 | blue<<16 | green<<8 | red), not classic 0xAARRGGBB.
    std::vector<LuckyLine> textLines;
    for (int i = 0; i < m_nTextMaxLine; ++i)
    {
        if (m_sText[i].s_nTextIndex < 0) break;
        if (m_sText[i].s_nTextIndex == 0) { textLines.push_back({}); continue; }

        const DWORD dwColor = m_sText[i].s_dwColor;
        const int r = dwColor & 0xFF;
        const int g = (dwColor >> 8) & 0xFF;
        const int b = (dwColor >> 16) & 0xFF;
        const int a = (dwColor >> 24) & 0xFF;
        wchar_t colorBuf[32];
        mu_swprintf(colorBuf, L"rgba(%d,%d,%d,%d)", r, g, b, a);

        textLines.push_back({
            StringUtils::WideToNarrow(I18N::Game::Lookup(m_sText[i].s_nTextIndex)),
            StringUtils::WideToNarrow(colorBuf),
            m_sText[i].s_nLine == RT3_SORT_LEFT ? "left" : "center"
        });
    }
    syncLines(&LuckyItemRmlModel::textLines, "text_lines", std::move(textLines));
}

float CLuckyItemWnd::GetLayerDepth(void)
{
    return 3.4f;
}

// Into #item_view (m_ItemTarget), in this window's layout space.
void CLuckyItemWnd::RenderItems()
{
    if (m_pNewInventoryCtrl && m_pNewInventoryCtrl->IsVisible())
        m_pNewInventoryCtrl->Render3D();
    // The mix-completion sparkle over the items.
    if (m_eEnd == eLuckyItem_End)
    {
        DisableDepthTest();
        EnableAlphaTest();
        RenderMixEffect();
        DisableAlphaBlend();
    }
}
