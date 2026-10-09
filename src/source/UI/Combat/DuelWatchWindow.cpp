#include "stdafx.h"
#include "UI/Combat/DuelWatchWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "I18N/All.h"
#include "Audio/DSPlaySound.h"
#include "GameLogic/Combat/DuelMgr.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlWindowClose.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
constexpr int kRoomCount = 4;

// RenderText() shrinks a text wider than its box to fit it: the size it drew `text` at.
float TextPxInBox(UI::Scaling::FontRole role, const UI::Scaling::Transform& transform, const wchar_t* text,
                  int boxWidth)
{
    g_pRenderText->SetFont(role == UI::Scaling::FontRole::Bold ? g_hFontBold : g_hFont);
    const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
    return UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(width),
                                                 static_cast<float>(boxWidth));
}

} // namespace

CDuelWatchWindow::CDuelWatchWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CDuelWatchWindow::~CDuelWatchWindow()
{
    Release();
}

bool CDuelWatchWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DUELWATCH, this);

    SetPos(x, y);

    for (int i = 0; i < kRoomCount; ++i)
        m_bChannelEnable[i] = FALSE;

    BuildRmlUi();

    Show(false);

    return true;
}

void CDuelWatchWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void CDuelWatchWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CDuelWatchWindow::UpdateMouseEvent()
{
    return !UI::RmlBridge::IsPointerOver(m_RmlView.Document());
}

bool CDuelWatchWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_DUELWATCH) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_DUELWATCH);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool CDuelWatchWindow::Update()
{
    if (IsVisible())
    {
        for (int i = 0; i < kRoomCount; ++i)
            m_bChannelEnable[i] = g_DuelMgr.IsDuelChannelEnabled(i) ? TRUE : FALSE;
    }

    SyncRmlModel();

    // A Watch click RmlUi reported: join that colosseum, if its duel is still open to spectators
    // (the original's BtnProcess() on an unlocked button).
    const int join = m_PendingJoin;
    m_PendingJoin = -1;
    if (IsVisible() && join >= 0 && join < kRoomCount && m_bChannelEnable[join] == TRUE &&
        g_DuelMgr.IsDuelChannelJoinable(join))
    {
        SocketClient->ToGameServer()->SendDuelChannelJoinRequest(static_cast<BYTE>(join));
    }

    return true;
}

bool CDuelWatchWindow::Render()
{
    // Nothing native left: the frame, the rooms and the Watch buttons are RmlUi. Kept because
    // CObject requires the override.
    return true;
}

void CDuelWatchWindow::OpeningProcess()
{
}

void CDuelWatchWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CDuelWatchWindow::GetLayerDepth()
{
    return 5.0f;
}

void CDuelWatchWindow::BindRmlModel(Rml::DataModelConstructor& c, DuelWatchRmlModel& model)
{
    UI::RmlBridge::BindWindowClose(c, mu::ui::window::INTERFACE_DUELWATCH);
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("title", &model.title);
    c.Bind("subtitle", &model.subtitle);
    c.Bind("subtitle_px", &model.subtitlePx);
    c.Bind("vs_text", &model.vsText);
    c.Bind("no_duel_text", &model.noDuelText);
    c.Bind("watch_text", &model.watchText);
    c.Bind("label_line_px", &model.labelLinePx);

    auto room = c.RegisterStruct<DuelWatchRoomEntry>();
    room.RegisterMember("index", &DuelWatchRoomEntry::index);
    room.RegisterMember("heading", &DuelWatchRoomEntry::heading);
    room.RegisterMember("running", &DuelWatchRoomEntry::running);
    room.RegisterMember("player1", &DuelWatchRoomEntry::player1);
    room.RegisterMember("player2", &DuelWatchRoomEntry::player2);
    room.RegisterMember("player1_px", &DuelWatchRoomEntry::player1Px);
    room.RegisterMember("player2_px", &DuelWatchRoomEntry::player2Px);
    room.RegisterMember("joinable", &DuelWatchRoomEntry::joinable);
    c.RegisterArray<std::vector<DuelWatchRoomEntry>>();
    c.Bind("rooms", &model.rooms);

    c.BindEventCallback("duelwatch_join",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_PendingJoin = arguments[0].Get<int>(-1);
                        });

    model.title = StringUtils::WideToNarrow(I18N::Game::DoorkeeperTitus);
    model.subtitle = StringUtils::WideToNarrow(I18N::Game::SelectAnColosseumYouDLikeToWatch);
    model.vsText = "VS";
    model.noDuelText = StringUtils::WideToNarrow(I18N::Game::NoDuelOn);
    model.watchText = StringUtils::WideToNarrow(I18N::Game::Watch);
    model.rooms.clear();
    for (int i = 0; i < kRoomCount; ++i)
    {
        wchar_t heading[256] = {};
        mu_swprintf(heading, I18N::Game::ColosseumD, i + 1);
        DuelWatchRoomEntry entry;
        entry.index = i;
        entry.heading = StringUtils::WideToNarrow(heading);
        model.rooms.push_back(std::move(entry));
    }
}

void CDuelWatchWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CDuelWatchWindow::SyncRmlModel()
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
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncField(m_RmlView.Binder(), &DuelWatchRmlModel::boldTextPx, "bold_text_px",
              UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold));
    SyncField(m_RmlView.Binder(), &DuelWatchRmlModel::subtitlePx, "subtitle_px",
              TextPxInBox(UI::Scaling::FontRole::Bold, transform, I18N::Game::SelectAnColosseumYouDLikeToWatch, 190));

    DuelWatchRmlModel& model = m_RmlView.GetModel();
    bool changed = false;
    for (DuelWatchRoomEntry& room : model.rooms)
    {
        const bool running = m_bChannelEnable[room.index] == TRUE;
        const bool joinable = running && g_DuelMgr.IsDuelChannelJoinable(room.index);
        const wchar_t* name1 = running ? g_DuelMgr.GetDuelChannelUserID1(room.index) : L"";
        const wchar_t* name2 = running ? g_DuelMgr.GetDuelChannelUserID2(room.index) : L"";
        Rml::String player1 = StringUtils::WideToNarrow(name1);
        Rml::String player2 = StringUtils::WideToNarrow(name2);
        const float player1Px = TextPxInBox(UI::Scaling::FontRole::Normal, transform, name1, 70);
        const float player2Px = TextPxInBox(UI::Scaling::FontRole::Normal, transform, name2, 70);
        if (room.running != running || room.joinable != joinable || room.player1 != player1 ||
            room.player2 != player2 || room.player1Px != player1Px || room.player2Px != player2Px)
        {
            room.running = running;
            room.joinable = joinable;
            room.player1 = std::move(player1);
            room.player2 = std::move(player2);
            room.player1Px = player1Px;
            room.player2Px = player2Px;
            changed = true;
        }
    }
    if (changed)
        m_RmlView.MarkDirty("rooms");
}
