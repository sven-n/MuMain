
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Inventory/Trade.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"

#include "GameLogic/Items/CComGem.h"
#include "Audio/DSPlaySound.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

CTrade::CTrade()
{
    m_pNewUIMng = NULL;
    m_pYourInvenCtrl = m_pMyInvenCtrl = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CTrade::~CTrade()
{
    Release();
}

bool CTrade::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng
        || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_TRADE, this);

    m_pYourInvenCtrl = new CInventoryCtrl;
    if (false == m_pYourInvenCtrl->Create(STORAGE_TYPE::UNDEFINED, g_pNewUI3DRenderMng, g_pNewItemMng,
        this, x + 16, y + 68, COLUMN_TRADE_INVEN, ROW_TRADE_INVEN))
    {
        SAFE_DELETE(m_pYourInvenCtrl);
        return false;
    }

    m_pMyInvenCtrl = new CInventoryCtrl;
    if (false == m_pMyInvenCtrl->Create(STORAGE_TYPE::TRADE, g_pNewUI3DRenderMng, g_pNewItemMng,
        this, x + 16, y + 274, COLUMN_TRADE_INVEN, ROW_TRADE_INVEN))
    {
        SAFE_DELETE(m_pMyInvenCtrl);
        return false;
    }

    SetPos(x, y);

    LoadImages();

    ::memset(m_szYourID, 0, MAX_USERNAME_SIZE + 1);
    m_bTradeAlert = false;

    InitTradeInfo();
    InitYourInvenBackUp();

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CTrade::BuildRmlUi()
{
    // Guarded so the document/model are created once, even if Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "trade",
            [this](Rml::DataModelConstructor& c, TradeRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("title", &model.title);

                c.Bind("your_id_text", &model.yourIdText);
                c.Bind("your_guild_visible", &model.yourGuildVisible);
                c.Bind("your_guild_name", &model.yourGuildName);
                c.Bind("your_level_text", &model.yourLevelText);
                c.Bind("your_level_color", &model.yourLevelColor);
                c.Bind("your_gold_text", &model.yourGoldText);
                c.Bind("your_gold_color", &model.yourGoldColor);
                c.Bind("your_confirm_checked", &model.yourConfirmChecked);

                c.Bind("my_id_text", &model.myIdText);
                c.Bind("my_gold_text", &model.myGoldText);
                c.Bind("my_gold_color", &model.myGoldColor);
                c.Bind("my_confirm_checked", &model.myConfirmChecked);
                c.Bind("my_confirm_waiting", &model.myConfirmWaiting);

                c.Bind("warning_label", &model.warningLabel);
                c.Bind("notice_line1", &model.noticeLine1);
                c.Bind("notice_line2", &model.noticeLine2);
                c.Bind("notice_line3", &model.noticeLine3);
                c.Bind("warning_opacity", &model.warningOpacity);

                c.Bind("close_tooltip", &model.closeTooltip);
                c.Bind("zen_tooltip", &model.zenTooltip);

                c.Bind("item_warning_text", &model.itemWarningText);
                auto itemWarningBadge = c.RegisterStruct<TradeRmlModel::ItemWarningBadge>();
                itemWarningBadge.RegisterMember("x", &TradeRmlModel::ItemWarningBadge::x);
                itemWarningBadge.RegisterMember("y", &TradeRmlModel::ItemWarningBadge::y);
                itemWarningBadge.RegisterMember("width", &TradeRmlModel::ItemWarningBadge::width);
                c.RegisterArray<std::vector<TradeRmlModel::ItemWarningBadge>>();
                c.Bind("item_warning_badges", &model.itemWarningBadges);

                c.BindEventCallback("trade_exit_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        ::PlayBuffer(SOUND_CLICK01);
                        ProcessCloseBtn();
                    });
                c.BindEventCallback("trade_zen_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        // Numeric Mode::Text amount entry, same shape as every other zen-input dialog.
                        mu::ui::window::GenericDialogConfig cfg;
                        cfg.showCancel = true;
                        cfg.lines = { { I18N::Game::EnterTheAmountOfZenYouWouldLikeToTrade, false } };
                        cfg.input = mu::ui::window::GenericDialogConfig::InputField{};
                        cfg.input->mode = mu::ui::window::GenericDialogConfig::InputField::Mode::Text;
                        cfg.input->maxLength = 8;
                        cfg.input->numericOnly = true;
                        cfg.onPrimary = [this]
                        {
                            const std::wstring strText = mu::ui::window::g_pGenericConfirmDialog->GetInputText();
                            const int iInputZen = strText.empty() ? 0 : _wtoi(strText.c_str());
                            if (iInputZen == 0)
                            {
                                mu::ui::window::g_pGenericConfirmDialog->KeepOpen();
                                return;
                            }
                            SendRequestMyGoldInput(iInputZen);
                        };
                        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                        ::PlayBuffer(SOUND_CLICK01);
                    });
                c.BindEventCallback("trade_my_confirm_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        if (m_nMyTradeWait > 0 || CInventoryCtrl::GetPickedItem() != NULL)
                            return;

                        ::PlayBuffer(SOUND_CLICK01);

                        if (m_bTradeAlert && !m_bMyConfirm)
                        {
                            // GenericDialogConfig only has bold/not-bold, not per-line color, so all
                            // 4 lines (3 warning + 1 red in the native layout) collapse to bold here.
                            mu::ui::window::GenericDialogConfig cfg;
                            cfg.showCancel = true;
                            for (int i = 0; i < 4; ++i)
                                cfg.lines.push_back({ I18N::Game::Lookup(371 + i), true });
                            cfg.onPrimary = [this] { AlertTrade(); };
                            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                        }
                        else
                        {
                            AlertTrade();
                        }
                    });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/trade.rml");

        // Frame background panel uses the background context -- see TradeBgRmlModel (Trade.h).
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "trade_bg",
                [](Rml::DataModelConstructor& c, TradeBgRmlModel& model)
                {
                    c.Bind("root_x", &model.rootX);
                    c.Bind("root_y", &model.rootY);
                    c.Bind("root_scale", &model.rootScale);
                });
            if (bgModelCreated)
            {
                // Starts hidden -- CreateBackgroundDocument() no longer Show()s eagerly (see its
                // own comment, RmlTheme.h); SyncRmlModel() below is what shows/hides it.
                m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/trade_bg.rml");
            }
        }

        // Not Show()n here -- m_pRmlDoc's visibility follows this window's own Show()/Hide() via
        // SyncRmlModel(), not an eager Show() at Create() time.
    }
}

void CTrade::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    if (m_pRmlBgDoc)
    {
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            m_BgRmlBinder.Destroy(bgContext);
            bgContext->UnloadDocument(m_pRmlBgDoc);
        }
        m_pRmlBgDoc = nullptr;
    }

    BuildRmlUi();
    // Next frame's Update()/SyncRmlModel() self-corrects live state/visibility for both docs.
}

void CTrade::InitTradeInfo()
{
    m_nYourLevel = 0;
    m_nYourGuildType = -1;
    m_nYourTradeGold = 0;
    m_nMyTradeGold = 0;
    m_nMyTradeWait = 0;
    m_bYourConfirm = m_bMyConfirm = false;
}

void CTrade::InitYourInvenBackUp()
{
    for (int i = 0; i < MAX_TRADE_INVEN; ++i)
        m_aYourInvenBackUp[i].Type = -1;
}

void CTrade::Release()
{
    UnloadImages();

    SAFE_DELETE(m_pMyInvenCtrl);
    SAFE_DELETE(m_pYourInvenCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = NULL;
    }

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->DeleteUI2DEffectObject(UI2DEffectCallback);

    // Hide explicitly -- Release() has no other way to hide these once created.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();
}

void CTrade::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CTrade::UpdateMouseEvent()
{
    if ((m_pYourInvenCtrl && false == m_pYourInvenCtrl->UpdateMouseEvent())
        || (m_pMyInvenCtrl && false == m_pMyInvenCtrl->UpdateMouseEvent()))
    {
        if (mu::ui::window::IsPress(VK_LBUTTON)
            && CInventoryCtrl::GetPickedItem()->GetOwnerInventory() == m_pMyInvenCtrl
            && m_bMyConfirm)
        {
            m_bMyConfirm = false;
            SocketClient->ToGameServer()->SendTradeButtonStateChange(TradeButtonState::Unchecked);
        }

        return false;
    }

    ProcessMyInvenCtrl();

    if (ProcessBtns())
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, TRADE_WIDTH, TRADE_HEIGHT).Contains(MouseX, MouseY))
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

bool CTrade::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            SocketClient->ToGameServer()->SendTradeCancel();
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_TRADE);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

bool CTrade::Update()
{
    if ((m_pYourInvenCtrl && false == m_pYourInvenCtrl->Update())
        || (m_pMyInvenCtrl && false == m_pMyInvenCtrl->Update()))
        return false;

    SyncRmlModel();
    return true;
}

bool CTrade::Render()
{
    ::EnableAlphaTest();

    // Frame background panel is RmlUi, routed through the background context (see
    // TradeBgRmlModel), painted by CManager::Render()'s centralized RenderBackgroundLayer() call
    // before this window's own Render()/Render3D() run.
    RenderGuildMark();

    if (m_pYourInvenCtrl)
        m_pYourInvenCtrl->Render();
    if (m_pMyInvenCtrl)
        m_pMyInvenCtrl->Render();

    if (g_pNewUI3DRenderMng)
        g_pNewUI3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER,
            UI2DEffectCallback, this, 0, 0);

    ::DisableAlphaBlend();

    return true;
}

void CTrade::UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    if (pClass)
    {
        auto* pNewUITrade = (CTrade*)pClass;
        pNewUITrade->RenderWarningArrow();
    }
}

// Dynamically-generated guild-emblem bitmap (::CreateGuildMark() builds it fresh from the guild's
// live mark data) -- a live render like the paperdoll/inventory item icons, not static chrome, so
// it stays native. The guild NAME text next to it moved to RmlUi (see SyncRmlModel()'s
// your_guild_name/your_guild_visible); the two don't overlap (name sits above the icon), so mixing
// a native icon with an RmlUi label here is safe, same as MyInventory mixing its native paperdoll
// with RmlUi frame text at the same m_Pos-relative coordinates.
void CTrade::RenderGuildMark()
{
    for (int i = 0; i < MAX_MARKS; ++i)
    {
        if (GuildMark[i].Key != -1 && GuildMark[i].Key == m_nYourGuildType)
        {
            ::CreateGuildMark(i, false);
            ::RenderBitmap(BITMAP_GUILD, (float)m_Pos.x + 15, (float)m_Pos.y + 42, 16, 16);
            break;
        }
    }
}

void CTrade::RenderWarningArrow()
{
    // Animated cursor-tracking arrow glyph only -- a texture-atlas crop with a color tint (using an
    // intentional GL_CLAMP UV overflow past v=1.0 to extend the sprite's bottom edge), genuinely a
    // rendering technique rather than expressible chrome, so it stays native. The "Warning" text
    // badge that used to render alongside it is RmlUi now (TradeRmlModel::itemWarningBadges,
    // item_warning_badges in trade.rml) -- see SyncRmlModel().
    ::EnableAlphaTest();

    int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
    ITEM* pYourItemObj;
    float fX, fY;
    POINT ptYourInvenCtrl = m_pYourInvenCtrl->GetPos();

    for (int i = 0; i < nYourItems; ++i)
    {
        pYourItemObj = m_pYourInvenCtrl->GetItem(i);
        if (ITEM_COLOR_TRADE_WARNING == pYourItemObj->byColorState)
        {
            fX = (float)ptYourInvenCtrl.x
                + (pYourItemObj->x * INVENTORY_SQUARE_WIDTH);
            fY = (float)ptYourInvenCtrl.y
                + (pYourItemObj->y * INVENTORY_SQUARE_WIDTH)
                + sinf(WorldTime * 0.015f);

            const DWORD warningArrowColor = RGBA(0, 255, 255, 255);
            ::RenderColorBitmap(IMAGE_TRADE_WARNING_ARROW, fX, fY + 5, 24.f, 24.f,
                0.f, 0.4f, 1.f, 1.f, warningArrowColor);
        }
    }

    ::DisableAlphaBlend();
}

void CTrade::ConvertYourLevel(int& rnLevel, DWORD& rdwColor)
{
    if (m_nYourLevel >= 400)
    {
        rnLevel = 400;
        rdwColor = (255 << 24) + (153 << 16) + (153 << 8) + (255);
    }
    else if (m_nYourLevel >= 300)
    {
        rnLevel = 300;
        rdwColor = (255 << 24) + (255 << 16) + (153 << 8) + (255);
    }
    else if (m_nYourLevel >= 200)
    {
        rnLevel = 200;
        rdwColor = (255 << 24) + (255 << 16) + (230 << 8) + (210);
    }
    else if (m_nYourLevel >= 100)
    {
        rnLevel = 100;
        rdwColor = (255 << 24) + (24 << 16) + (201 << 8) + (0);
    }
    else if (m_nYourLevel >= 50)
    {
        rnLevel = 50;
        rdwColor = (255 << 24) + (0 << 16) + (150 << 8) + (255);
    }
    else							//  빨간색.
    {
        rnLevel = 10;
        rdwColor = (255 << 24) + (0 << 16) + (0 << 8) + (255);
    }
}

void CTrade::SyncRmlModel()
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

    auto syncBool = [this](bool TradeRmlModel::* field, const char* boundName, bool value)
    {
        if (m_RmlBinder.GetModel().*field != value) { m_RmlBinder.GetModel().*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncFloat = [this](float TradeRmlModel::* field, const char* boundName, float value)
    {
        if (m_RmlBinder.GetModel().*field != value) { m_RmlBinder.GetModel().*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncText = [this](Rml::String TradeRmlModel::* field, const char* boundName, const Rml::String& value)
    {
        if (m_RmlBinder.GetModel().*field != value) { m_RmlBinder.GetModel().*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncWide = [&](Rml::String TradeRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        syncText(field, boundName, StringUtils::WideToNarrow(text));
    };
    auto argbToRgba = [](DWORD argb) -> Rml::String
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "rgba(%u,%u,%u,%u)",
            (argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF, (argb >> 24) & 0xFF);
        return Rml::String(buf);
    };

    syncWide(&TradeRmlModel::title, "title", I18N::Game::Trade);

    syncWide(&TradeRmlModel::yourIdText, "your_id_text", m_szYourID);

    Rml::String guildName;
    for (int i = 0; i < MAX_MARKS; ++i)
    {
        if (GuildMark[i].Key != -1 && GuildMark[i].Key == m_nYourGuildType)
        {
            guildName = StringUtils::WideToNarrow(GuildMark[i].GuildName);
            break;
        }
    }
    syncBool(&TradeRmlModel::yourGuildVisible, "your_guild_visible", !guildName.empty());
    syncText(&TradeRmlModel::yourGuildName, "your_guild_name", guildName);

    int nLevel;
    DWORD dwLevelColor;
    ConvertYourLevel(nLevel, dwLevelColor);
    wchar_t levelValueBuf[128];
    if (nLevel == 400)
        mu_swprintf(levelValueBuf, L"%d", nLevel);
    else
        mu_swprintf(levelValueBuf, I18N::Game::AboutD, nLevel);
    wchar_t levelBuf[160];
    mu_swprintf(levelBuf, L"Lv.%ls", levelValueBuf);
    syncWide(&TradeRmlModel::yourLevelText, "your_level_text", levelBuf);
    syncText(&TradeRmlModel::yourLevelColor, "your_level_color", argbToRgba(dwLevelColor));

    wchar_t goldBuf[256];
    ::ConvertGold(m_nYourTradeGold, goldBuf);
    syncWide(&TradeRmlModel::yourGoldText, "your_gold_text", goldBuf);
    syncText(&TradeRmlModel::yourGoldColor, "your_gold_color", argbToRgba(::getGoldColor(m_nYourTradeGold)));

    ::ConvertGold(m_nMyTradeGold, goldBuf);
    syncWide(&TradeRmlModel::myGoldText, "my_gold_text", goldBuf);
    syncText(&TradeRmlModel::myGoldColor, "my_gold_color", argbToRgba(::getGoldColor(m_nMyTradeGold)));

    syncWide(&TradeRmlModel::myIdText, "my_id_text", Hero->ID);

    syncBool(&TradeRmlModel::yourConfirmChecked, "your_confirm_checked", m_bYourConfirm);
    syncBool(&TradeRmlModel::myConfirmChecked, "my_confirm_checked", m_bMyConfirm);
    syncBool(&TradeRmlModel::myConfirmWaiting, "my_confirm_waiting", m_nMyTradeWait > 0);

    syncWide(&TradeRmlModel::warningLabel, "warning_label", I18N::Game::Warning);
    syncWide(&TradeRmlModel::noticeLine1, "notice_line1", I18N::Game::NoticePleaseCheckOut);
    syncWide(&TradeRmlModel::noticeLine2, "notice_line2", I18N::Game::TheLevelOfThePlayer);
    syncWide(&TradeRmlModel::noticeLine3, "notice_line3", I18N::Game::AndTheItemsBeforeTrading);

    // sin(WorldTime)-based alpha pulse on the "Warning" word.
    const int nAlpha = int(std::min<int>(255, sin(WorldTime / 200) * 200 + 275));
    syncFloat(&TradeRmlModel::warningOpacity, "warning_opacity", nAlpha / 255.f);

    syncWide(&TradeRmlModel::closeTooltip, "close_tooltip", I18N::Game::Close388);
    syncWide(&TradeRmlModel::zenTooltip, "zen_tooltip", I18N::Game::ZenTrade);

    syncWide(&TradeRmlModel::itemWarningText, "item_warning_text", I18N::Game::Warning);

    // Former RenderWarningArrow()'s "Warning" text badge, one per your-side item flagged
    // ITEM_COLOR_TRADE_WARNING. Coordinates are panel-relative (subtracting m_Pos), matching the
    // same sinf() wobble the native arrow glyph still animates with, since #panel is itself
    // positioned at root_x/root_y (m_Pos) -- see RenderWarningArrow() for the native arrow.
    std::vector<TradeRmlModel::ItemWarningBadge> itemWarningBadges;
    if (m_pYourInvenCtrl)
    {
        const POINT ptYourInvenCtrl = m_pYourInvenCtrl->GetPos();
        const int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
        for (int i = 0; i < nYourItems; ++i)
        {
            ITEM* pYourItemObj = m_pYourInvenCtrl->GetItem(i);
            if (ITEM_COLOR_TRADE_WARNING != pYourItemObj->byColorState) continue;

            const float fX = (float)ptYourInvenCtrl.x + (pYourItemObj->x * INVENTORY_SQUARE_WIDTH);
            const float fY = (float)ptYourInvenCtrl.y + (pYourItemObj->y * INVENTORY_SQUARE_WIDTH) + sinf(WorldTime * 0.015f);
            const float fWidth = (float)((int)ItemAttribute[pYourItemObj->Type].Width * INVENTORY_SQUARE_WIDTH);

            itemWarningBadges.push_back({ fX - m_Pos.x, fY - m_Pos.y, fWidth });
        }
    }
    if (m_RmlBinder.GetModel().itemWarningBadges != itemWarningBadges)
    {
        m_RmlBinder.GetModel().itemWarningBadges = std::move(itemWarningBadges);
        m_RmlBinder.MarkDirty("item_warning_badges");
    }
}

float CTrade::GetLayerDepth()
{
    return 2.1f;
}

void CTrade::LoadImages()
{
    LoadBitmap(L"Interface\\CursorSitDown.tga", IMAGE_TRADE_WARNING_ARROW, GL_LINEAR, GL_CLAMP);
}

void CTrade::UnloadImages()
{
    DeleteBitmap(IMAGE_TRADE_WARNING_ARROW);
}

void CTrade::ProcessClosing()
{
    m_pYourInvenCtrl->RemoveAllItems();
    m_pMyInvenCtrl->RemoveAllItems();

    if (m_bTradeAlert)
        InitYourInvenBackUp();
}

void CTrade::ProcessMyInvenCtrl()
{
    if (NULL == m_pMyInvenCtrl)
        return;

    if (mu::ui::window::IsPress(VK_LBUTTON))
    {
        CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
        if (NULL == pPickedItem)
            return;

        ITEM* pItemObj = pPickedItem->GetItem();
        if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
        {
            int nSrcIndex = pPickedItem->GetSourceLinealPos();
            int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
            if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
                SendRequestItemToTrade(pItemObj, nSrcIndex, nDstIndex);
        }
        else if (pPickedItem->GetOwnerInventory() == m_pMyInvenCtrl)
        {
            int nSrcIndex = pPickedItem->GetSourceLinealPos();
            int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
            if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
            {
                SendRequestEquipmentItem(STORAGE_TYPE::TRADE, nSrcIndex, pItemObj, STORAGE_TYPE::TRADE, nDstIndex);
            }
        }
        else if (pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
        {
            int nSrcIndex = pPickedItem->GetSourceLinealPos();
            int nDstIndex = pPickedItem->GetTargetLinealPos(m_pMyInvenCtrl);
            if (nDstIndex != -1 && m_pMyInvenCtrl->CanMove(nDstIndex, pItemObj))
                SendRequestItemToTrade(pItemObj, nSrcIndex, nDstIndex);
        }
    }
}

void CTrade::SendRequestItemToTrade(ITEM* pItemObj, int nInvenIndex,
    int nTradeIndex)
{
    if (::IsTradeBan(pItemObj))
    {
        g_pSystemLogBox->AddText(I18N::Game::TheseItemsCannotBeTraded, mu::ui::window::TYPE_ERROR_MESSAGE);
    }
    else
    {
        m_bMyConfirm = false;
        SocketClient->ToGameServer()->SendTradeButtonStateChange(TradeButtonState::Unchecked);

        SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, nInvenIndex,
            pItemObj, STORAGE_TYPE::TRADE, nTradeIndex);
    }
}

void CTrade::SendRequestItemToMyInven(ITEM* pItemObj, int nTradeIndex, int nInvenIndex)
{
    SendRequestEquipmentItem(STORAGE_TYPE::TRADE, nTradeIndex, pItemObj, STORAGE_TYPE::INVENTORY, nInvenIndex);

    if (m_bMyConfirm)
    {
        AlertTrade();
    }
    m_nMyTradeWait = 150;
}

void CTrade::SendRequestMyGoldInput(int nInputGold)
{
    if (nInputGold <= (int)CharacterMachine->Gold + m_nMyTradeGold)
    {
        if (m_bMyConfirm)
        {
            m_bMyConfirm = false;
            SocketClient->ToGameServer()->SendTradeButtonStateChange(TradeButtonState::Unchecked);
        }

        if (m_nMyTradeGold > 0)
            m_nMyTradeWait = 150;

        m_nTempMyTradeGold = nInputGold;
        SocketClient->ToGameServer()->SendSetTradeMoney(nInputGold);
    }
    else
    {
        mu::ui::window::CreateOkMessageBox(I18N::Game::YouAreShortOfZen);
    }
}

void CTrade::ProcessCloseBtn()
{
    if (CInventoryCtrl::GetPickedItem() == NULL)
    {
        m_bTradeAlert = false;
        SocketClient->ToGameServer()->SendTradeCancel();
    }
}

bool CTrade::ProcessBtns()
{
    if (m_nMyTradeWait > 0)
        --m_nMyTradeWait;

    // Top-right corner close "X" baked into the frame art: hides + swallows the click. Same
    // hit-box g_pNewUISystem->HandleFrameCornerClose() uses elsewhere (WindowSystem.cpp), kept
    // inline here rather than refactored, matching this window's pre-existing shape -- the real
    // Close/Zen-input buttons are now handled by RmlUi's data-event-click (see Create()).
    if (mu::ui::window::IsPress(VK_LBUTTON)
        && CheckMouseIn(m_Pos.x + 169, m_Pos.y + 7, 13, 12))
    {
        ::PlayBuffer(SOUND_CLICK01);
        ProcessCloseBtn();
        return true;
    }

    return false;
}

void CTrade::AlertTrade()
{
    m_bMyConfirm = !m_bMyConfirm;

    m_bTradeAlert = true;
    SocketClient->ToGameServer()->SendTradeButtonStateChange(m_bMyConfirm ? TradeButtonState::Checked : TradeButtonState::Unchecked);
}

void CTrade::GetYourID(wchar_t* pszYourID)
{
    ::wcscpy(pszYourID, m_szYourID);
}

void CTrade::ProcessToReceiveTradeRequest(char* pbyYourID)
{
    if (g_pNewUISystem->IsImpossibleTradeInterface())
    {
        SocketClient->ToGameServer()->SendTradeRequestResponse(false);
        return;
    }

    CMultiLanguage::ConvertFromUtf8(m_szYourID, pbyYourID);

    mu::ui::window::GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.lines = {
        { m_szYourID, false },
        { I18N::Game::WouldLikeToTradeWithYou, false },
    };
    cfg.onPrimary = [] { SocketClient->ToGameServer()->SendTradeRequestResponse(true); };
    cfg.onSecondary = [] { SocketClient->ToGameServer()->SendTradeRequestResponse(false); };
    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));

    mu::ui::window::CInventoryCtrl::BackupPickedItem();
}

void CTrade::ProcessToReceiveTradeResult(LPPTRADE pTradeData)
{
    switch (pTradeData->SubCode)
    {
    case 0:
        g_pSystemLogBox->AddText(I18N::Game::YourTradeHasBeenCanceled, mu::ui::window::TYPE_ERROR_MESSAGE);
        break;

    case 2:
        g_pSystemLogBox->AddText(I18N::Game::YouCannotTradeRightNow, mu::ui::window::TYPE_ERROR_MESSAGE);
        break;

    case 1:
        g_pNewUISystem->Show(mu::ui::window::INTERFACE_TRADE);

        InitTradeInfo();

        int x = 260 * MouseX / REFERENCE_WIDTH;
        SetCursorPos(x * WindowWidth / REFERENCE_WIDTH, MouseY * WindowHeight / REFERENCE_HEIGHT);

        wchar_t szTempID[MAX_USERNAME_SIZE + 1]{ };
        CMultiLanguage::ConvertFromUtf8(szTempID, pTradeData->ID, MAX_USERNAME_SIZE);

        if (!m_bTradeAlert && ::wcscmp(m_szYourID, szTempID))
            InitYourInvenBackUp();

        m_bTradeAlert = false;
        m_nYourGuildType = pTradeData->GuildKey;
        wcsncpy(m_szYourID, szTempID, MAX_USERNAME_SIZE);
        m_nYourLevel = pTradeData->Level;   //  상대방 레벨.
        break;
    }
}

void CTrade::ProcessToReceiveYourItemDelete(BYTE byYourInvenIndex)
{
    BackUpYourInven(int(byYourInvenIndex));
    ITEM* pYourItemObj = m_pYourInvenCtrl->FindItem(int(byYourInvenIndex));
    m_pYourInvenCtrl->RemoveItem(pYourItemObj);
    AlertYourTradeInven();
    ::PlayBuffer(SOUND_GET_ITEM01);
}

void CTrade::BackUpYourInven(int nYourInvenIndex)
{
    ITEM* pYourItemObj = m_pYourInvenCtrl->FindItem(nYourInvenIndex);
    BackUpYourInven(pYourItemObj);
}

void CTrade::BackUpYourInven(ITEM* pYourItemObj)
{
    if ((pYourItemObj->Type >= ITEM_HELPER && pYourItemObj->Type <= ITEM_DARK_HORSE_ITEM)
        || (pYourItemObj->Type == ITEM_JEWEL_OF_BLESS || pYourItemObj->Type == ITEM_JEWEL_OF_SOUL || pYourItemObj->Type == ITEM_JEWEL_OF_LIFE)
        || (pYourItemObj->Type >= ITEM_JEWEL_OF_GUARDIAN)
        || (COMGEM::isCompiledGem(pYourItemObj))
        || (pYourItemObj->Type >= ITEM_WING && pYourItemObj->Type <= ITEM_WINGS_OF_DARKNESS)
        || (pYourItemObj->Type >= ITEM_CAPE_OF_LORD)
        || (pYourItemObj->Type >= ITEM_WING_OF_STORM && pYourItemObj->Type <= ITEM_WING_OF_DIMENSION)
        || (pYourItemObj->Type == ITEM_JEWEL_OF_CHAOS)
        || (pYourItemObj->Type >= ITEM_CAPE_OF_FIGHTER && pYourItemObj->Type <= ITEM_CAPE_OF_OVERRULE)
        || ((pYourItemObj->Level > 4 && pYourItemObj->Type < ITEM_WING) || pYourItemObj->ExcellentFlags > 0))
    {
        int nCompareValue;
        bool bSameItem = false;

        for (int i = 0; i < MAX_TRADE_INVEN; ++i)
        {
            if (-1 == m_aYourInvenBackUp[i].Type)
                continue;

            nCompareValue = ::CompareItem(m_aYourInvenBackUp[i], *pYourItemObj);
            if (0 == nCompareValue)
            {
                bSameItem = true;
                break;
            }
            else if (-1 == nCompareValue)
            {
                bSameItem = true;
                m_aYourInvenBackUp[i] = *pYourItemObj;
                break;
            }
            else if (2 != nCompareValue)
            {
                bSameItem = true;
            }
        }

        if (!bSameItem)
        {
            for (int i = 0; i < MAX_TRADE_INVEN; ++i)
            {
                if (-1 == m_aYourInvenBackUp[i].Type)
                {
                    m_aYourInvenBackUp[i] = *pYourItemObj;
                    break;
                }
            }
        }
    }
}

void CTrade::ProcessToReceiveYourItemAdd(BYTE byYourInvenIndex, std::span<const BYTE> pbyItemPacket)
{
    m_pYourInvenCtrl->AddItem(byYourInvenIndex, pbyItemPacket);
    AlertYourTradeInven();
    ::PlayBuffer(SOUND_GET_ITEM01);
}

void CTrade::AlertYourTradeInven()
{
    int nCount = 0;
    int nCompareItemType[10];

    m_bTradeAlert = false;

    int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
    ITEM* pYourItemObj;
    int nCompareValue;

    for (int i = 0; i < nYourItems; ++i)
    {
        pYourItemObj = m_pYourInvenCtrl->GetItem(i);
        for (int j = 0; j < MAX_TRADE_INVEN; ++j)
        {
            if (m_aYourInvenBackUp[j].Type == pYourItemObj->Type)
            {
                nCompareValue = ::CompareItem(m_aYourInvenBackUp[j], *pYourItemObj);
                if (1 == nCompareValue)
                {
                    m_bTradeAlert = true;
                    pYourItemObj->byColorState = ITEM_COLOR_TRADE_WARNING;
                }
                else
                {
                    if (0 == nCompareValue)
                        nCompareItemType[nCount++] = m_aYourInvenBackUp[j].Type;

                    pYourItemObj->byColorState = ITEM_COLOR_NORMAL;
                    break;
                }
            }
        }
    }

    if (nCount > 0)
    {
        m_bTradeAlert = false;
        for (int i = 0; i < nCount; ++i)
        {
            for (int j = 0; j < nYourItems; ++j)
            {
                pYourItemObj = m_pYourInvenCtrl->GetItem(j);
                if (nCompareItemType[i] == pYourItemObj->Type)
                    pYourItemObj->byColorState = ITEM_COLOR_NORMAL;
            }
        }
    }
}

void CTrade::ProcessToReceiveMyTradeGold(BYTE bySuccess)
{
    m_nMyTradeGold = bySuccess ? m_nTempMyTradeGold : 0;
}

void CTrade::ProcessToReceiveYourConfirm(BYTE byState)
{
    switch (byState)
    {
    case 0:
        m_bYourConfirm = false;
        break;
    case 1:
        m_bYourConfirm = true;
        break;
    case 2:
        m_bMyConfirm = false;
        m_bYourConfirm = false;
        m_nMyTradeWait = 150;
        break;
    case 3:
        break;
    }

    PlayBuffer(SOUND_CLICK01);
}

void CTrade::ProcessToReceiveTradeExit(BYTE byState)
{
    switch (byState)
    {
    case 0:
    {
        g_pSystemLogBox->AddText(I18N::Game::YourTradeHasBeenCanceled, mu::ui::window::TYPE_ERROR_MESSAGE);

        m_bTradeAlert = false;

        int nYourItems = m_pYourInvenCtrl->GetNumberOfItems();
        for (int i = 0; i < nYourItems; ++i)
            BackUpYourInven(m_pYourInvenCtrl->GetItem(i));
    }
    break;

    case 2:
        g_pSystemLogBox->AddText(I18N::Game::YourTradeHasBeenCanceledBecauseYourInventoryIsFull, mu::ui::window::TYPE_ERROR_MESSAGE);
        break;

    case 3:
        g_pSystemLogBox->AddText(I18N::Game::TradeRequestIsCanceled, mu::ui::window::TYPE_ERROR_MESSAGE);
        break;

    case 4:
        g_pSystemLogBox->AddText(I18N::Game::ReinforcedItemCanTBeTraded, mu::ui::window::TYPE_ERROR_MESSAGE);
        break;
    }

    mu::ui::window::CInventoryCtrl::DeletePickedItem();

    g_MessageBox->PopMessageBox();

    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_TRADE);
}

void CTrade::ProcessToReceiveTradeItems(int nIndex, std::span<const BYTE> pbyItemPacket)
{
    mu::ui::window::CInventoryCtrl::DeletePickedItem();

    if (nIndex >= 0 && nIndex < (m_pMyInvenCtrl->GetNumberOfColumn()
        * m_pMyInvenCtrl->GetNumberOfRow()))
        m_pMyInvenCtrl->AddItem(nIndex, pbyItemPacket);
}

int mu::ui::window::CTrade::GetPointedItemIndexMyInven()
{
    return m_pMyInvenCtrl->GetPointedSquareIndex();
}

int mu::ui::window::CTrade::GetPointedItemIndexYourInven()
{
    return m_pYourInvenCtrl->GetPointedSquareIndex();
}
