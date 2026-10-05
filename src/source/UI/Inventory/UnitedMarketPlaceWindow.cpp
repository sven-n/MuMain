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
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;

CUnitedMarketPlaceWindow::CUnitedMarketPlaceWindow()
{
    m_pNewUIMng = NULL;
    m_pNewUI3DRenderMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iRemainTime = 0;
    m_bIsEnterButtonLocked = FALSE;
}

CUnitedMarketPlaceWindow::~CUnitedMarketPlaceWindow()
{
    Release();
}

bool CUnitedMarketPlaceWindow::Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}


void CUnitedMarketPlaceWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = NULL;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CUnitedMarketPlaceWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CUnitedMarketPlaceWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    // #panel's own live RCSS size is the source of truth -- INVENTORY_WIDTH/HEIGHT only cover the
    // first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = INVENTORY_WIDTH;
    float panelHeight = INVENTORY_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth),
                                      static_cast<int>(panelHeight))
            .Contains(MouseX, MouseY))
        return false;

    return true;
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

void CUnitedMarketPlaceWindow::Render3D()
{
    //RenderItem3D();
}

void CUnitedMarketPlaceWindow::RenderItem3D()
{
    // 	POINT ptOrigin = { m_Pos.x, m_Pos.y+50 };
    //
    // 	int nItemType = (14*MAX_ITEM_INDEX)+111;
    //     int nItemLevel = 0;
    //
    // 	::RenderItem3D(ptOrigin.x+(190-20)/2, ptOrigin.y+75, 20.f, 27, nItemType, nItemLevel<<3, 0, 0, false);
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




bool CUnitedMarketPlaceWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click. The Warp and exit
    // buttons are RmlUi's (see Update()).
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
    return false;
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

void CUnitedMarketPlaceWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "united_market_place",
        [this](Rml::DataModelConstructor& c, UnitedMarketPlaceRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
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
        });

    if (!modelCreated)
        return;

    UnitedMarketPlaceRmlModel& model = m_RmlBinder.GetModel();
    model.title = StringUtils::WideToNarrow(I18N::Game::Julia);
    model.warpText = StringUtils::WideToNarrow(I18N::Game::Warp3016);
    model.exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);
    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/united_market_place.rml");
}

void CUnitedMarketPlaceWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void CUnitedMarketPlaceWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    // The button label's line height: the native line height in physical px.
    {
        const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
        const float labelLinePx = static_cast<float>(lineHeight) * UI::Scaling::GetActiveTransform().scaleY;
        auto& labelModel = m_RmlBinder.GetModel();
        if (labelModel.labelLinePx != labelLinePx)
        {
            labelModel.labelLinePx = labelLinePx;
            m_RmlBinder.MarkDirty("label_line_px");
        }
    }
    UnitedMarketPlaceRmlModel& model = m_RmlBinder.GetModel();
    const float boldPx =
        UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, UI::Scaling::GetActiveTransform());
    if (model.boldTextPx != boldPx)
    {
        model.boldTextPx = boldPx;
        m_RmlBinder.MarkDirty("bold_text_px");
    }
    const bool locked = m_bIsEnterButtonLocked == TRUE;
    if (model.warpLocked != locked)
    {
        model.warpLocked = locked;
        m_RmlBinder.MarkDirty("warp_locked");
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
        m_RmlBinder.MarkDirty("lines");
    }
}
