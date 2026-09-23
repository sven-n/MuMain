//*****************************************************************************
// File: NewUIQuestProgress.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Quests/QuestProgress.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Scaling/UITransform.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Engine/Object/ZzzInventory.h" // ::RenderItemInfo
#include "Network/Server/WSclient.h"    // QUEST_REQUEST_ITEM / QUEST_REWARD_ITEM
#include "Core/Utilities/StringUtils.h"
#include "Core/Utilities/UsefulDef.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

using namespace SEASON3B;
using namespace mu::ui::window;

#define QP_NPC_MAX_LINE_PER_PAGE	7

CQuestProgress::CQuestProgress()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CQuestProgress::~CQuestProgress()
{
    Release();
}

bool CQuestProgress::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_QUEST_PROGRESS, this);

    SetPos(x, y);

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    Show(false);

    return true;
}

void CQuestProgress::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "quest_progress",
        [this](Rml::DataModelConstructor& c, QuestProgressRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);

            c.Bind("subject", &model.subject);
            c.Bind("npc_name", &model.npcName);
            c.Bind("player_name", &model.playerName);

            auto textLine = c.RegisterStruct<QuestProgressTextLine>();
            textLine.RegisterMember("text", &QuestProgressTextLine::text);
            c.RegisterArray<std::vector<QuestProgressTextLine>>();
            c.Bind("npc_lines", &model.npcLines);

            c.Bind("prev_enabled", &model.prevEnabled);
            c.Bind("next_enabled", &model.nextEnabled);
            c.Bind("active_view", &model.activeView);
            c.Bind("player_words_text", &model.playerWordsText);

            auto answer = c.RegisterStruct<QuestProgressAnswerEntry>();
            answer.RegisterMember("text", &QuestProgressAnswerEntry::text);
            answer.RegisterMember("index", &QuestProgressAnswerEntry::index);
            c.RegisterArray<std::vector<QuestProgressAnswerEntry>>();
            c.Bind("answers", &model.answers);

            auto reward = c.RegisterStruct<UI::Quests::RewardModel::Entry>();
            reward.RegisterMember("text", &UI::Quests::RewardModel::Entry::text);
            reward.RegisterMember("color", &UI::Quests::RewardModel::Entry::color);
            reward.RegisterMember("bold", &UI::Quests::RewardModel::Entry::bold);
            reward.RegisterMember("index", &UI::Quests::RewardModel::Entry::index);
            reward.RegisterMember("clickable", &UI::Quests::RewardModel::Entry::clickable);
            c.RegisterArray<std::vector<UI::Quests::RewardModel::Entry>>();
            c.Bind("reward_rows", &model.rewardRows);

            c.Bind("request_complete", &model.requestComplete);
            c.Bind("ok_label", &model.okLabel);
            c.Bind("exit_tooltip", &model.exitTooltip);

            c.BindEventCallback("questprogress_click_close",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickClose(); });
            c.BindEventCallback("questprogress_prev_page",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickPrevPage(); });
            c.BindEventCallback("questprogress_next_page",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickNextPage(); });
            c.BindEventCallback("questprogress_select_answer",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                {
                    if (arguments.size() == 1)
                        RmlClickSelectAnswer(arguments[0].Get<int>(-1));
                });
            c.BindEventCallback("questprogress_select_reward",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                {
                    if (arguments.size() == 1)
                        RmlClickSelectReward(arguments[0].Get<int>(-1));
                });
            c.BindEventCallback("questprogress_complete",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickComplete(); });
        });

    if (modelCreated)
    {
        m_RmlBinder.GetModel().okLabel = StringUtils::WideToNarrow(I18N::Game::OK);
        m_RmlBinder.GetModel().exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);
    }

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
        "Data/Interface/RmlUi/quest_progress.rml");
}

void CQuestProgress::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    // Next frame's SyncRmlModel() self-corrects visibility/live model state.
}

void CQuestProgress::Release()
{
    if (m_pRmlDoc)
    {
        m_pRmlDoc->Close();
        m_pRmlDoc = nullptr;
    }

    if (m_pNewUIMng)
    {
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CQuestProgress::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void CQuestProgress::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else m_pRmlDoc->Hide();
    }
}

bool CQuestProgress::UpdateMouseEvent()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_QUEST_PROGRESS))
        return false;

    float panelWidth = QP_WIDTH;
    float panelHeight = QP_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CQuestProgress::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUEST_PROGRESS))
    {
        if (mu::ui::window::IsPress(VK_ESCAPE))
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_QUEST_PROGRESS);
            return false;
        }
    }

    return true;
}

bool CQuestProgress::Update()
{
    SyncRmlModel();

    // Mirrors Render()'s own condition for drawing the reward-item info popup: whenever it
    // wouldn't be (re)shown this frame, make sure the persistent shared tooltip doesn't linger --
    // same pattern CMyQuestInfoWindow's own Update() already uses.
    if (IsVisible() && !(m_eLowerView == REQUEST_REWARD_MODE && m_pSelectedRewardItem))
        UI::RmlBridge::Tooltip::Hide();

    return true;
}

bool CQuestProgress::Render()
{
    // RmlUi's #panel owns all chrome/text/list rendering; only the selected reward item's info
    // popup is still a native per-frame call here (see m_pSelectedRewardItem).
    if (m_eLowerView == REQUEST_REWARD_MODE && m_pSelectedRewardItem)
    {
        // Reference-pixel, not screen pixel -- RenderItemInfo() converts internally via the
        // ambient transform (same convention every other caller uses).
        ::RenderItemInfo(m_Pos.x + 95, m_Pos.y + 360, m_pSelectedRewardItem, false, 0, true);
    }

    return true;
}

bool CQuestProgress::IsVisible() const
{
    return CObject::IsVisible();
}

float CQuestProgress::GetLayerDepth()
{
    return 3.1f;
}

void CQuestProgress::ProcessOpening()
{
    ::PlayBuffer(SOUND_INTERFACE01);
}

bool CQuestProgress::ProcessClosing()
{
    m_dwCurQuestIndex = 0;
    m_pSelectedRewardItem = nullptr;
    SocketClient->ToGameServer()->SendCloseNpcRequest();
    ::PlayBuffer(SOUND_CLICK01);
    return true;
}

void CQuestProgress::SetContents(DWORD dwQuestIndex)
{
    if (0 == dwQuestIndex)
        return;

    m_dwCurQuestIndex = dwQuestIndex;
    m_pSelectedRewardItem = nullptr;

    SetCurNPCWords();
    m_bCanClick = true;

    if (NULL != g_QuestMng.GetAnswer(m_dwCurQuestIndex, 0))
    {
        m_eLowerView = NON_PLAYER_WORDS_MODE;
    }
    else
    {
        SetCurRequestReward();
        m_eLowerView = REQUEST_REWARD_MODE;
    }
}

void CQuestProgress::SetCurNPCWords()
{
    if (0 == m_dwCurQuestIndex)
        return;

    memset(m_aszNPCWords, 0, sizeof m_aszNPCWords);

    g_pRenderText->SetFont(g_hFont);
    int nLine = ::DivideStringByPixel(&m_aszNPCWords[0][0],
        QP_NPC_LINE_MAX, QP_WORDS_ROW_MAX, g_QuestMng.GetNPCWords(m_dwCurQuestIndex), 160);

    if (1 > nLine)
        return;

    m_nMaxNPCPage = (nLine - 1) / QP_NPC_MAX_LINE_PER_PAGE;
    m_nSelNPCPage = 0;
}

void CQuestProgress::SetCurRequestReward()
{
    if (0 == m_dwCurQuestIndex)
        return;

    m_RewardRows = UI::Quests::RewardModel::BuildRows(m_dwCurQuestIndex, m_bRequestComplete);
}

void CQuestProgress::EnableCompleteBtn(bool bEnable)
{
    m_bRequestComplete = bEnable;
}

void CQuestProgress::RmlClickClose()
{
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_QUEST_PROGRESS);
}

void CQuestProgress::RmlClickPrevPage()
{
    if (m_nSelNPCPage <= 0)
        return; // already at the first page -- next_enabled/prev_enabled guard a stray click.

    --m_nSelNPCPage;
    ::PlayBuffer(SOUND_CLICK01);
}

void CQuestProgress::RmlClickNextPage()
{
    if (m_nSelNPCPage < m_nMaxNPCPage)
    {
        ++m_nSelNPCPage;
    }
    else if (m_eLowerView == NON_PLAYER_WORDS_MODE)
    {
        // Last NPC page reached for the first time -- reveal the numbered answers, matching the
        // native "R" button's double duty (RenderTipTextList()'s old ProcessBtns()).
        m_eLowerView = PLAYER_WORDS_MODE;
    }
    else
    {
        return; // nothing left to reveal.
    }

    ::PlayBuffer(SOUND_CLICK01);
}

void CQuestProgress::RmlClickSelectAnswer(int nAnswerIndex)
{
    if (m_eLowerView != PLAYER_WORDS_MODE || !m_bCanClick)
        return;
    if (nAnswerIndex < 0 || nAnswerIndex >= QM_MAX_ANSWER)
        return;

    const auto questNumber = static_cast<uint16_t>(LOWORD(m_dwCurQuestIndex));
    const auto questGroup = static_cast<uint16_t>(HIWORD(m_dwCurQuestIndex));
    SocketClient->ToGameServer()->SendQuestProceedRequest(questNumber, questGroup,
        static_cast<QuestProceedAction>(nAnswerIndex + 1));
    ::PlayBuffer(SOUND_CLICK01);
    m_bCanClick = false;
}

void CQuestProgress::RmlClickSelectReward(int nRewardIndex)
{
    if (nRewardIndex < 0 || static_cast<size_t>(nRewardIndex) >= m_RewardRows.size())
        return;

    const UI::Quests::RewardModel::RowData& row = m_RewardRows[nRewardIndex];
    if ((row.dwType == QUEST_REQUEST_ITEM || row.dwType == QUEST_REWARD_ITEM) && row.pItem)
        m_pSelectedRewardItem = row.pItem;
    else
        m_pSelectedRewardItem = nullptr;
}

void CQuestProgress::RmlClickComplete()
{
    if (!m_bRequestComplete || !m_bCanClick)
        return;

    const auto questNumber = static_cast<uint16_t>(LOWORD(m_dwCurQuestIndex));
    const auto questGroup = static_cast<uint16_t>(HIWORD(m_dwCurQuestIndex));
    SocketClient->ToGameServer()->SendQuestCompletionRequest(questNumber, questGroup);
    ::PlayBuffer(SOUND_CLICK01);
    m_bCanClick = false;
}

void CQuestProgress::SyncRmlModel()
{
    if (!m_pRmlDoc)
        return;

    auto& model = m_RmlBinder.GetModel();

    const auto transform = UI::Scaling::GetActiveTransform();
    const float rootX = static_cast<float>(m_Pos.x) * transform.scaleX + transform.offsetX;
    const float rootY = static_cast<float>(m_Pos.y) * transform.scaleY + transform.offsetY;
    if (model.rootX != rootX || model.rootY != rootY || model.rootScale != transform.scaleX)
    {
        model.rootX = rootX;
        model.rootY = rootY;
        model.rootScale = transform.scaleX;
        m_RmlBinder.MarkDirty("root_x");
        m_RmlBinder.MarkDirty("root_y");
        m_RmlBinder.MarkDirty("root_scale");
    }

    if (0 == m_dwCurQuestIndex)
        return; // never populated yet (window not yet opened this session) -- nothing else to sync.

    model.subject = StringUtils::WideToNarrow(g_QuestMng.GetSubject(m_dwCurQuestIndex));
    m_RmlBinder.MarkDirty("subject");
    model.npcName = StringUtils::WideToNarrow(g_QuestMng.GetNPCName());
    m_RmlBinder.MarkDirty("npc_name");

    // Current page's up-to-7 lines, already wrapped by SetCurNPCWords()'s DivideStringByPixel()
    // call -- bound as literal non-wrapping lines (see QuestProgressRmlModel.h's own npcLines
    // comment), not re-flowed by RmlUi.
    model.npcLines.clear();
    for (int i = 0; i < QP_NPC_MAX_LINE_PER_PAGE; ++i)
    {
        const wchar_t* line = m_aszNPCWords[i + QP_NPC_MAX_LINE_PER_PAGE * m_nSelNPCPage];
        if (line[0] == L'\0')
            break;
        model.npcLines.push_back({ StringUtils::WideToNarrow(line) });
    }
    m_RmlBinder.MarkDirty("npc_lines");

    // Both computed fresh from state every sync (not toggled imperatively at each mutation point
    // the way the native CButton::Lock()/UnLock() calls were) -- always correct by construction,
    // see RmlClickNextPage()'s own comment for the "reveal answers on last page" rule this mirrors.
    model.prevEnabled = (m_nSelNPCPage > 0);
    model.nextEnabled = (m_nSelNPCPage < m_nMaxNPCPage) || (m_eLowerView == NON_PLAYER_WORDS_MODE);
    m_RmlBinder.MarkDirty("prev_enabled");
    m_RmlBinder.MarkDirty("next_enabled");

    model.activeView = static_cast<int>(m_eLowerView);
    m_RmlBinder.MarkDirty("active_view");

    // Player-name label shows for both NON_PLAYER_WORDS_MODE and PLAYER_WORDS_MODE (hidden only in
    // reward view, quest_progress.rml's own data-class-hidden="active_view == 2") -- always
    // populated here, visibility is the RML's job.
    model.playerName = StringUtils::WideToNarrow(CharacterAttribute->Name);
    m_RmlBinder.MarkDirty("player_name");

    if (m_eLowerView == PLAYER_WORDS_MODE)
    {
        model.playerWordsText = StringUtils::WideToNarrow(g_QuestMng.GetPlayerWords(m_dwCurQuestIndex));

        model.answers.clear();
        for (int i = 0; i < QM_MAX_ANSWER; ++i)
        {
            const wchar_t* pszAnswer = g_QuestMng.GetAnswer(m_dwCurQuestIndex, i);
            if (NULL == pszAnswer)
                break;
            wchar_t szAnswer[2 * QP_WORDS_ROW_MAX];
            mu_swprintf(szAnswer, L"%d.%ls", i + 1, pszAnswer);
            model.answers.push_back({ StringUtils::WideToNarrow(szAnswer), i });
        }
    }
    else
    {
        model.playerWordsText.clear();
        model.answers.clear();
    }
    m_RmlBinder.MarkDirty("player_words_text");
    m_RmlBinder.MarkDirty("answers");

    if (m_eLowerView == REQUEST_REWARD_MODE)
    {
        model.rewardRows.clear();
        for (size_t i = 0; i < m_RewardRows.size(); ++i)
            model.rewardRows.push_back(UI::Quests::RewardModel::ToEntry(m_RewardRows[i], static_cast<int>(i)));
    }
    else
    {
        model.rewardRows.clear();
    }
    m_RmlBinder.MarkDirty("reward_rows");

    if (model.requestComplete != m_bRequestComplete)
    {
        model.requestComplete = m_bRequestComplete;
        m_RmlBinder.MarkDirty("request_complete");
    }
}
