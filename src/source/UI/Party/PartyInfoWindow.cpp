
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Party/PartyInfoWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Scaling/UITransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "Audio/DSPlaySound.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "World/MapInfra/MapManager.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

using namespace SEASON3B;
using namespace mu::ui::window;

CPartyInfoWindow::CPartyInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_bParty = false;
    m_iSelectedCharID = -1;
}

CPartyInfoWindow::~CPartyInfoWindow()
{
    Release();
}

bool CPartyInfoWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_PARTY, this);

    SetPos(x, y);

    LoadImages();

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    Show(false);

    return true;
}

void CPartyInfoWindow::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "party_info",
            [this](Rml::DataModelConstructor& c, PartyInfoRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("has_party", &model.hasParty);
                c.Bind("window_title", &model.windowTitle);
                c.Bind("exit_tooltip", &model.exitTooltip);

                auto textLine = c.RegisterStruct<TextLine>();
                textLine.RegisterMember("text", &TextLine::text);
                c.RegisterArray<std::vector<TextLine>>();
                c.Bind("empty_state_lines", &model.emptyStateLines);

                auto member = c.RegisterStruct<PartyMemberRow>();
                member.RegisterMember("name", &PartyMemberRow::name);
                member.RegisterMember("name_color", &PartyMemberRow::nameColor);
                member.RegisterMember("map_text", &PartyMemberRow::mapText);
                member.RegisterMember("coord_text", &PartyMemberRow::coordText);
                member.RegisterMember("hp_text", &PartyMemberRow::hpText);
                member.RegisterMember("hp_percent", &PartyMemberRow::hpPercent);
                member.RegisterMember("is_leader", &PartyMemberRow::isLeader);
                member.RegisterMember("show_kick", &PartyMemberRow::showKick);
                member.RegisterMember("index", &PartyMemberRow::index);
                c.RegisterArray<std::vector<PartyMemberRow>>();
                c.Bind("members", &model.members);

                c.BindEventCallback("party_click_exit",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickExit(); });
                c.BindEventCallback("party_kick_member",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                    {
                        if (arguments.size() == 1)
                            RmlClickKickMember(arguments[0].Get<int>(-1));
                    });
            });

        if (modelCreated)
        {
            auto& model = m_RmlBinder.GetModel();
            model.windowTitle = StringUtils::WideToNarrow(I18N::Game::Party);
            model.exitTooltip = StringUtils::WideToNarrow(I18N::Game::ClosePartyWindowP);

            model.emptyStateLines = {
                { StringUtils::WideToNarrow(I18N::Game::TypePartyWithTheMouseCursorOn) },
                { StringUtils::WideToNarrow(I18N::Game::ThePlayerYouWouldLike) },
                { StringUtils::WideToNarrow(I18N::Game::ToCreateAPartyWith) },
                { StringUtils::WideToNarrow(I18N::Game::AndYouCanCreate) },
                { StringUtils::WideToNarrow(I18N::Game::APartyWithThem) },
                { StringUtils::WideToNarrow(I18N::Game::YouCanShareMoreExpWith) },
                { StringUtils::WideToNarrow(I18N::Game::YourPartyMembersBasedOnLevel) },
            };
        }

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
        "Data/Interface/RmlUi/party_info.rml");
}

void CPartyInfoWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    // Next frame's SyncRmlModel() self-corrects visibility/live model state.
}

void CPartyInfoWindow::Release()
{
    UnloadImages();

    if (m_pRmlDoc)
    {
        m_pRmlDoc->Close();
        m_pRmlDoc = nullptr;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = NULL;
    }
}

void CPartyInfoWindow::OpenningProcess()
{
    SocketClient->ToGameServer()->SendPartyListRequest();
}

void CPartyInfoWindow::ClosingProcess()
{
}

void CPartyInfoWindow::RmlClickExit()
{
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_PARTY);
}

void CPartyInfoWindow::RmlClickKickMember(int index)
{
    if (index < 0 || index >= PartyNumber)
        return;

    LeaveParty(index);
}

bool CPartyInfoWindow::UpdateMouseEvent()
{
    // Top-right corner close "X" (shared frame). Hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_PARTY))
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, PARTY_INFO_WINDOW_WIDTH, PARTY_INFO_WINDOW_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CPartyInfoWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_PARTY) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_PARTY);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool CPartyInfoWindow::Update()
{
    SetParty(PartyNumber > 0);
    SyncRmlModel();
    return true;
}

bool CPartyInfoWindow::Render()
{
    // RmlUi's #panel owns all chrome/text/button rendering now; nothing left to draw natively.
    return true;
}

bool CPartyInfoWindow::LeaveParty(const int iIndex)
{
    if (!gMapManager.IsCursedTemple())
    {
        PlayBuffer(SOUND_CLICK01);
        SocketClient->ToGameServer()->SendPartyPlayerKickRequest(Party[iIndex].Number);
    }

    SetParty(false);

    return true;
}

void CPartyInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void CPartyInfoWindow::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else m_pRmlDoc->Hide();
    }
}

void CPartyInfoWindow::SetParty(bool bParty)
{
    m_bParty = bParty;
}

float CPartyInfoWindow::GetLayerDepth()
{
    return 2.4f;
}

void CPartyInfoWindow::SyncRmlModel()
{
    if (!m_pRmlDoc)
        return;

    auto& model = m_RmlBinder.GetModel();

    const auto transform = UI::Scaling::GetActiveTransform();
    model.rootX = static_cast<float>(m_Pos.x) * transform.scaleX + transform.offsetX;
    model.rootY = static_cast<float>(m_Pos.y) * transform.scaleY + transform.offsetY;
    model.rootScale = transform.scaleX;
    m_RmlBinder.MarkDirty("root_x");
    m_RmlBinder.MarkDirty("root_y");
    m_RmlBinder.MarkDirty("root_scale");

    if (model.hasParty != m_bParty)
    {
        model.hasParty = m_bParty;
        m_RmlBinder.MarkDirty("has_party");
    }

    if (!m_bParty)
        return;

    wchar_t szText[256] = { 0, };
    std::vector<PartyMemberRow> members;
    members.reserve(PartyNumber);

    for (int i = 0; i < PartyNumber; i++)
    {
        PARTY_t* pMember = &Party[i];

        PartyMemberRow row;
        row.index = i;
        row.name = StringUtils::WideToNarrow(pMember->Name);
        row.nameColor = (i == 0) ? "rgba(0,255,0,255)" : "rgba(255,255,255,255)";
        row.mapText = StringUtils::WideToNarrow(gMapManager.GetMapName(pMember->Map));

        mu_swprintf(szText, L"(%d,%d)", pMember->x, pMember->y);
        row.coordText = StringUtils::WideToNarrow(szText);

        row.hpPercent = pMember->maxHP > 0 ? (pMember->currHP * 100.f) / pMember->maxHP : 0.f;
        mu_swprintf(szText, L"%d %ls %d", pMember->currHP, I18N::Game::Text2374, pMember->maxHP);
        row.hpText = StringUtils::WideToNarrow(szText);

        row.isLeader = (i == 0);
        row.showKick = !wcscmp(Party[0].Name, Hero->ID) || !wcscmp(Party[i].Name, Hero->ID);

        members.push_back(row);
    }

    model.members = std::move(members);
    m_RmlBinder.MarkDirty("members");
}

void CPartyInfoWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_PARTY_BASE_WINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_PARTY_BASE_WINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_PARTY_BASE_WINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_PARTY_BASE_WINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_PARTY_BASE_WINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_PARTY_BASE_WINDOW_BTN_EXIT, GL_LINEAR);		// Exit Button

    LoadBitmap(L"Interface\\newui_item_table01(L).tga", IMAGE_PARTY_TABLE_TOP_LEFT);
    LoadBitmap(L"Interface\\newui_item_table01(R).tga", IMAGE_PARTY_TABLE_TOP_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table02(L).tga", IMAGE_PARTY_TABLE_BOTTOM_LEFT);
    LoadBitmap(L"Interface\\newui_item_table02(R).tga", IMAGE_PARTY_TABLE_BOTTOM_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table03(Up).tga", IMAGE_PARTY_TABLE_TOP_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(Dw).tga", IMAGE_PARTY_TABLE_BOTTOM_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(L).tga", IMAGE_PARTY_TABLE_LEFT_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(R).tga", IMAGE_PARTY_TABLE_RIGHT_PIXEL);

    LoadBitmap(L"Interface\\newui_party_lifebar01.jpg", IMAGE_PARTY_HPBAR_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_party_lifebar02.jpg", IMAGE_PARTY_HPBAR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_party_flag.tga", IMAGE_PARTY_FLAG, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_party_x.tga", IMAGE_PARTY_EXIT, GL_LINEAR);
}

void CPartyInfoWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_BACK);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_TOP);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_LEFT);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_RIGHT);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_BOTTOM);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_BTN_EXIT);

    DeleteBitmap(IMAGE_PARTY_TABLE_RIGHT_PIXEL);
    DeleteBitmap(IMAGE_PARTY_TABLE_LEFT_PIXEL);
    DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_PIXEL);
    DeleteBitmap(IMAGE_PARTY_TABLE_TOP_PIXEL);
    DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_RIGHT);
    DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_LEFT);
    DeleteBitmap(IMAGE_PARTY_TABLE_TOP_RIGHT);
    DeleteBitmap(IMAGE_PARTY_TABLE_TOP_LEFT);

    DeleteBitmap(IMAGE_PARTY_HPBAR_BACK);
    DeleteBitmap(IMAGE_PARTY_HPBAR);
    DeleteBitmap(IMAGE_PARTY_FLAG);
    DeleteBitmap(IMAGE_PARTY_EXIT);
}
