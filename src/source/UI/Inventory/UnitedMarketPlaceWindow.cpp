#include "stdafx.h"
#include "UI/Inventory/UnitedMarketPlaceWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "World/MapInfra/MapManager.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;

CUnitedMarketPlaceWindow::CUnitedMarketPlaceWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iRemainTime = 0;
    m_bIsEnterButtonLocked = FALSE;
}

CUnitedMarketPlaceWindow::~CUnitedMarketPlaceWindow()
{
    Release();
}

bool CUnitedMarketPlaceWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA, this);

    SetPos(x, y);

    BuildRmlUi();

    Show(false);

    return true;
}


void CUnitedMarketPlaceWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void CUnitedMarketPlaceWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CUnitedMarketPlaceWindow::UpdateMouseEvent()
{
    return !UI::RmlBridge::IsPointerOver(m_RmlView.Document());
}

bool CUnitedMarketPlaceWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CUnitedMarketPlaceWindow::Update()
{
    SyncRmlModel();

    // Clicks RmlUi reported (the original's BtnProcess()): Warp asks the server for the market and
    // locks the button; the exit button hides the window.
    const bool warp = m_PendingWarp;
    const bool exit = m_PendingExit;
    m_PendingWarp = m_PendingExit = false;
    if (!IsVisible())
        return true;
    if (warp && m_bIsEnterButtonLocked != TRUE)
    {
        LoadingWorld = 9999999;
        SocketClient->ToGameServer()->SendEnterMarketPlaceRequest();
        m_bIsEnterButtonLocked = true;
        return true;
    }
    if (exit)
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
    return true;
}

bool CUnitedMarketPlaceWindow::IsVisible() const
{
    return CObject::IsVisible();
}

bool CUnitedMarketPlaceWindow::Render()
{
    // Nothing native left: the frame, the texts and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

void CUnitedMarketPlaceWindow::OpeningProcess()
{
    LockEnterButton(FALSE);
}

void CUnitedMarketPlaceWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CUnitedMarketPlaceWindow::GetLayerDepth()
{
    return 5.0f;
}




void CUnitedMarketPlaceWindow::SetRemainTime(int iTime)
{
    m_iRemainTime = iTime;
    if (iTime != 0)
    {
        LockEnterButton(TRUE);
    }
}

void CUnitedMarketPlaceWindow::LockEnterButton(BOOL bLock)
{
    m_bIsEnterButtonLocked = bLock;
}

void CUnitedMarketPlaceWindow::BindRmlModel(Rml::DataModelConstructor& c, UnitedMarketPlaceRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("title", &model.title);
    c.RegisterArray<std::vector<Rml::String>>();
    c.Bind("lines", &model.lines);
    c.Bind("warp_text", &model.warpText);
    c.Bind("label_line_px", &model.labelLinePx);
    c.Bind("warp_locked", &model.warpLocked);
    c.Bind("exit_tooltip", &model.exitTooltip);
    c.BindEventCallback("market_warp", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingWarp = true; });
    c.BindEventCallback("market_exit", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingExit = true; });

    model.title = StringUtils::WideToNarrow(I18N::Game::Julia);
    model.warpText = StringUtils::WideToNarrow(I18N::Game::Warp3016);
    model.exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);
}

void CUnitedMarketPlaceWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CUnitedMarketPlaceWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    // The button label's line height: the native line height in physical px.
    {
        const float labelLinePx = CUIRenderTextSDLTtf::LineHeightPx(UI::Scaling::FontRole::Normal);
        auto& labelModel = m_RmlView.GetModel();
        if (labelModel.labelLinePx != labelLinePx)
        {
            labelModel.labelLinePx = labelLinePx;
            m_RmlView.MarkDirty("label_line_px");
        }
    }
    UnitedMarketPlaceRmlModel& model = m_RmlView.GetModel();
    const float boldPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);
    if (model.boldTextPx != boldPx)
    {
        model.boldTextPx = boldPx;
        m_RmlView.MarkDirty("bold_text_px");
    }
    const bool locked = m_bIsEnterButtonLocked == TRUE;
    if (model.warpLocked != locked)
    {
        model.warpLocked = locked;
        m_RmlView.MarkDirty("warp_locked");
    }

    // In the market, the way back to town; elsewhere, the market (the original's Render()). The
    // theme gives each of the seven slots its own row; going back to town has nothing to say in
    // three of them, which stay empty so the last line keeps its place.
    std::vector<const wchar_t*> texts;
    if (gMapManager.WorldActive == WD_79UNITEDMARKETPLACE)
        texts = {I18N::Game::WillYouBeGoingBackToTownNow,
                 I18N::Game::HaveAnotherGreatDay,
                 I18N::Game::AndStayPositiveAtAllTimes,
                 nullptr,
                 nullptr,
                 nullptr,
                 I18N::Game::WouldYouLikeToGoToTown};
    else
        texts = {I18N::Game::IfYouGoToTheMarketInLorencia,
                 I18N::Game::YouLlFindManyItemsYouNeed,
                 I18N::Game::AvailableForPurchase,
                 I18N::Game::IfYouHaveItemsYouWantToSell,
                 I18N::Game::YouCanSellThem,
                 I18N::Game::AtTheMarket,
                 I18N::Game::WouldYouLikeToGoToTheMarket};
    std::vector<Rml::String> lines;
    for (const wchar_t* text : texts)
        lines.push_back(text != nullptr ? StringUtils::WideToNarrow(text) : Rml::String{});
    if (model.lines != lines)
    {
        model.lines = std::move(lines);
        m_RmlView.MarkDirty("lines");
    }
}
