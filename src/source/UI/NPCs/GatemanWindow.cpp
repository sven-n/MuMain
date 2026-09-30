
#include "stdafx.h"
#include "UI/NPCs/GatemanWindow.h"
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
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/NPCs/UIGateKeeper.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

extern CUIGateKeeper* g_pUIGateKeeper;

using namespace SEASON3B;
using namespace mu::ui::window;

CGatemanWindow::CGatemanWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CGatemanWindow::~CGatemanWindow()
{
    Release();
}

bool CGatemanWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GATEKEEPER, this);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CGatemanWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CGatemanWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CGatemanWindow::UpdateMouseEvent()
{
    // The guild master's public toggle stays a native hit test (the document leaves that area to
    // the pointer); the buttons are RmlUi's (see Update()).
    if (g_pUIGateKeeper->GetType() == TOUCH_TYPE_GUILD_MASTER)
    {
        const POINT ptOrigin = {m_Pos.x, m_Pos.y + 50};
        if (mu::ui::window::IsPress(VK_LBUTTON) &&
            mu::ui::window::CheckMouseIn(ptOrigin.x + 35, ptOrigin.y + 60, 100, 16))
            g_pUIGateKeeper->SendPublicSetting();
    }

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

bool CGatemanWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GATEKEEPER) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CGatemanWindow::Update()
{
    // A button RmlUi reported (the original's CButton handling in UpdateMouseEvent()).
    const GATEMAN_BUTTON button = m_PendingButton;
    m_PendingButton = GATEMAN_BUTTON_NONE;
    if (IsVisible() && button == GATEMAN_BUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
    }
    else if (IsVisible() && button != GATEMAN_BUTTON_NONE)
    {
        switch (g_pUIGateKeeper->GetType())
        {
        case TOUCH_TYPE_PERSON:
            UpdateGuestMode(button);
            break;
        case TOUCH_TYPE_GUILD_STAFF:
            UpdateGuildMemeberMode(button);
            break;
        case TOUCH_TYPE_GUILD_MASTER:
            UpdateGuildMasterMode(button);
            break;
        }
    }

    SyncRmlModel();
    return true;
}

bool CGatemanWindow::Render()
{
    // Nothing native left: the frame, the page and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

void CGatemanWindow::OpeningProcess()
{
}

void CGatemanWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CGatemanWindow::GetLayerDepth()
{
    return 5.0f;
}

bool CGatemanWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_GATEKEEPER);

    return false;
}

void CGatemanWindow::UpdateGuildMasterMode(GATEMAN_BUTTON button)
{
    if (button == GATEMAN_BUTTON_SET)
    {
        g_pUIGateKeeper->SendEnteranceFee();
    }
    else if (button == GATEMAN_BUTTON_FEE_UP)
    {
        g_pUIGateKeeper->EnteranceFeeUp();
    }
    else if (button == GATEMAN_BUTTON_FEE_DOWN)
    {
        g_pUIGateKeeper->EnteranceFeeDown();
    }
    else if (button == GATEMAN_BUTTON_ENTER)
    {
        g_pUIGateKeeper->SendEnter();
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
    }
}

void CGatemanWindow::UpdateGuildMemeberMode(GATEMAN_BUTTON button)
{
    if (button == GATEMAN_BUTTON_ENTER)
    {
        g_pUIGateKeeper->SendEnter();
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
    }
}

void CGatemanWindow::UpdateGuestMode(GATEMAN_BUTTON button)
{
    // Enter is locked (no click) while the castle is closed to guests.
    if (button == GATEMAN_BUTTON_ENTER && g_pUIGateKeeper->IsPublic())
    {
        if ((int)CharacterMachine->Gold >= g_pUIGateKeeper->GetEnteranceFee())
        {
            g_pUIGateKeeper->SendEnter();
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GATEKEEPER);
        }
        else
        {
            mu::ui::window::GenericDialogConfig cfg;
            cfg.lines = {
                { I18N::Game::EnteringIsNotAllowed, false },
                { I18N::Game::InsufficientZenForEntering, false },
            };
            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
        }
    }
}

void CGatemanWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "gateman",
        [this](Rml::DataModelConstructor& c, GatemanRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("line_height_px", &model.lineHeightPx);
            c.Bind("label_top", &model.labelTop);
            auto text = c.RegisterStruct<GatemanTextEntry>();
            text.RegisterMember("text", &GatemanTextEntry::text);
            text.RegisterMember("left", &GatemanTextEntry::left);
            text.RegisterMember("top", &GatemanTextEntry::top);
            text.RegisterMember("width", &GatemanTextEntry::width);
            text.RegisterMember("text_px", &GatemanTextEntry::textPx);
            text.RegisterMember("align", &GatemanTextEntry::align);
            text.RegisterMember("bold", &GatemanTextEntry::bold);
            text.RegisterMember("color", &GatemanTextEntry::color);
            c.RegisterArray<std::vector<GatemanTextEntry>>();
            c.Bind("texts", &model.texts);
            auto button = c.RegisterStruct<GatemanButtonEntry>();
            button.RegisterMember("label", &GatemanButtonEntry::label);
            button.RegisterMember("id", &GatemanButtonEntry::id);
            button.RegisterMember("left", &GatemanButtonEntry::left);
            button.RegisterMember("top", &GatemanButtonEntry::top);
            button.RegisterMember("locked", &GatemanButtonEntry::locked);
            c.RegisterArray<std::vector<GatemanButtonEntry>>();
            c.Bind("buttons", &model.buttons);
            c.Bind("master_mode", &model.masterMode);
            c.Bind("is_public", &model.isPublic);
            c.Bind("exit_tooltip", &model.exitTooltip);
            c.BindEventCallback("gateman_button",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PendingButton = static_cast<GATEMAN_BUTTON>(arguments[0].Get<int>(-1));
                                });
        });
    if (!modelCreated)
        return;

    m_pRmlDoc =
        UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/gateman.rml");
}

void CGatemanWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void CGatemanWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth 5: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
    SyncContent();
}

void CGatemanWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    std::vector<GatemanTextEntry> texts;
    std::vector<GatemanButtonEntry> buttons;
    // RenderText(x, y, text, width, 0, sort) in panel coordinates, in the font and colour the
    // original had set at that point (its draws leak them from one call to the next).
    bool bold = true;
    DWORD color = RGBA(220, 220, 220, 255);
    auto addText = [&](const wchar_t* text, float x, float y, float width, int align)
    {
        if (text == nullptr || text[0] == L'\0')
            return;
        g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const float px =
            width > 0.f ? UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured), width)
                        : UI::Scaling::NativeTextPixelSize(role, transform);
        texts.push_back(
            {StringUtils::WideToNarrow(text), x, y, width, px, align, bold, UI::RmlBridge::RgbaToCss(color)});
    };
    auto addButton = [&](GATEMAN_BUTTON id, const wchar_t* label, float top, bool locked)
    {
        buttons.push_back(
            {StringUtils::WideToNarrow(label), id, static_cast<float>(INVENTORY_WIDTH / 2 - 27), top, locked});
    };

    // RenderFrame(): the title, bold (220, 220, 220), which the page then inherits.
    addText(I18N::Game::GuardNPC, 15, 13, 160, 1);

    wchar_t szText[256] = {};
    wchar_t szGold[64] = {};
    const BYTE type = g_pUIGateKeeper->GetType();
    float y = 50;
    if (type == TOUCH_TYPE_GUILD_MASTER)
    {
        addText(I18N::Game::EntranceRestriction, 0, y, 190, 1);
        bold = false;
        y += 20;
        addText(I18N::Game::OnlyTheGuildMembers, 0, y, 190, 1);
        y += 10;
        addText(I18N::Game::AreAllowedToEnter, 0, y, 190, 1);
        y += 10;
        addText(I18N::Game::IsAllowed, 0, y, 190, 1);
        y += 20;
        addText(I18N::Game::OpenItToNonMembers, 55, y, 0, 0);
        y += 18;
        ConvertGold(g_pUIGateKeeper->GetEnteranceFee(), szGold);
        mu_swprintf(szText, I18N::Game::EntranceFeeSZen, szGold);
        addText(szText, 35, y, 0, 0);
        y += 30;
        bold = true;
        addText(I18N::Game::EntranceFeeSetting, 0, y, 190, 1);
        bold = false;
        ConvertGold(g_pUIGateKeeper->GetViewEnteranceFee(), szGold);
        mu_swprintf(szText, L"%ls %ls", szGold, I18N::Game::Zen);
        // RT3_WRITE_RIGHT_TO_LEFT: the text ends at x 80.
        addText(szText, 80 - 200, y + 32, 200, 2);
        addButton(GATEMAN_BUTTON_SET, I18N::Game::Confirm, 220, false);
        // Past the fee arrows and Confirm, as the original's ptOrigin advanced (20 + 20 + 25 + 25).
        y += 90;
        color = RGBA(255, 255, 255, 255);
        ConvertGold(g_pUIGateKeeper->GetMaxEnteranceFee(), szGold);
        mu_swprintf(szText, I18N::Game::EntranceFeeRange0SZen, szGold);
        addText(szText, 0, y, 190, 1);
        y += 13;
        addText(I18N::Game::ForSetting, 0, y, 190, 1);
        y += 13;
        ConvertGold(g_pUIGateKeeper->GetAddEnteranceFee(), szGold);
        mu_swprintf(szText, I18N::Game::IncreaseUnitSZen, szGold);
        addText(szText, 0, y, 190, 1);
        addButton(GATEMAN_BUTTON_ENTER, I18N::Game::Enter, 320, false);
    }
    else if (type == TOUCH_TYPE_GUILD_STAFF)
    {
        addText(I18N::Game::WouldYouLikeToEnter, 0, y, 190, 1);
        addButton(GATEMAN_BUTTON_ENTER, I18N::Game::Enter, 100, false);
    }
    else if (type == TOUCH_TYPE_PERSON)
    {
        if (g_pUIGateKeeper->IsPublic())
        {
            ConvertGold(g_pUIGateKeeper->GetEnteranceFee(), szGold);
            mu_swprintf(szText, I18N::Game::EntranceFeeSzen, szGold);
            color = g_pUIGateKeeper->GetEnteranceFee() > (int)CharacterMachine->Gold ? RGBA(255, 100, 50, 255)
                                                                                     : RGBA(255, 255, 100, 255);
            addText(szText, 0, y, 190, 1);
            color = RGBA(255, 255, 255, 255);
            addText(I18N::Game::PayEntranceFeeToEnter, 0, y + 10, 190, 1);
            addText(I18N::Game::WouldYouLikeToEnter, 0, y + 20, 190, 1);
        }
        else
        {
            addText(I18N::Game::EnteringIsNotAllowed, 0, y, 190, 1);
            addText(I18N::Game::ApprovalFromTheLordOfACastleIsRequired, 0, y + 10, 190, 1);
            addText(I18N::Game::ForEntering, 0, y + 20, 190, 1);
            addText(I18N::Game::PleaseGoBack, 0, y + 30, 190, 1);
        }
        addButton(GATEMAN_BUTTON_ENTER, I18N::Game::Enter, 100, !g_pUIGateKeeper->IsPublic());
    }

    GatemanRmlModel& model = m_RmlBinder.GetModel();
    const bool sameTexts = model.texts.size() == texts.size() &&
                           std::equal(model.texts.begin(), model.texts.end(), texts.begin(),
                                      [](const GatemanTextEntry& a, const GatemanTextEntry& b)
                                      {
                                          return a.text == b.text && a.left == b.left && a.top == b.top &&
                                                 a.width == b.width && a.textPx == b.textPx && a.align == b.align &&
                                                 a.bold == b.bold && a.color == b.color;
                                      });
    if (!sameTexts)
    {
        model.texts = std::move(texts);
        m_RmlBinder.MarkDirty("texts");
    }
    const bool sameButtons = model.buttons.size() == buttons.size() &&
                             std::equal(model.buttons.begin(), model.buttons.end(), buttons.begin(),
                                        [](const GatemanButtonEntry& a, const GatemanButtonEntry& b)
                                        {
                                            return a.label == b.label && a.id == b.id && a.left == b.left &&
                                                   a.top == b.top && a.locked == b.locked;
                                        });
    if (!sameButtons)
    {
        model.buttons = std::move(buttons);
        m_RmlBinder.MarkDirty("buttons");
    }
    SyncField(m_RmlBinder, &GatemanRmlModel::masterMode, "master_mode", type == TOUCH_TYPE_GUILD_MASTER);
    SyncField(m_RmlBinder, &GatemanRmlModel::isPublic, "is_public", g_pUIGateKeeper->IsPublic() == TRUE);
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    SyncField(m_RmlBinder, &GatemanRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineHeight) * transform.scaleY);
    SyncField(m_RmlBinder, &GatemanRmlModel::labelTop, "label_top", static_cast<float>(23 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &GatemanRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}
