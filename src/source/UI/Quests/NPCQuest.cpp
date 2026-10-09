//*****************************************************************************
// File: NewUINPCQuest.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Quests/NPCQuest.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Quests/DialogStructure.h"
#include "I18N/All.h"

#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "UI/Scaling/UITransform.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlStyleKeys.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

extern bool bCheckNPC;
extern int g_iNumLineMessageBoxCustom;
extern int g_iNumAnswer;
extern wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];
extern wchar_t g_lpszDialogAnswer[MAX_ANSWER_FOR_DIALOG][NUM_LINE_DA][MAX_LENGTH_CMB];
extern int g_iCurrentDialogScript;

using namespace SEASON3B;
using namespace mu::ui::window;

CNPCQuest::CNPCQuest()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CNPCQuest::~CNPCQuest()
{
    Release();
}

bool CNPCQuest::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_NPCQUEST, this);

    SetPos(x, y);

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
    }

    Show(false);

    return true;
}

void CNPCQuest::BindRmlModel(Rml::DataModelConstructor& c, NPCQuestRmlModel& model)
{
    c.Bind("text_px", &model.textPx);

    c.Bind("npc_name", &model.npcName);
    c.Bind("quest_title", &model.questTitle);
    c.Bind("show_quest_title", &model.showQuestTitle);

    c.Bind("show_conditions", &model.showConditions);
    auto conditionRow = c.RegisterStruct<NPCQuestConditionRow>();
    conditionRow.RegisterMember("text", &NPCQuestConditionRow::text);
    conditionRow.RegisterMember("color", &NPCQuestConditionRow::color);
    c.RegisterArray<std::vector<NPCQuestConditionRow>>();
    c.Bind("conditions", &model.conditions);
    c.Bind("complete_enabled", &model.completeEnabled);

    c.Bind("show_cost", &model.showCost);
    c.Bind("cost_amount", &model.costAmount);
    c.Bind("cost_tier", &model.costTier);

    auto textLine = c.RegisterStruct<NPCQuestTextLine>();
    textLine.RegisterMember("text", &NPCQuestTextLine::text);
    c.RegisterArray<std::vector<NPCQuestTextLine>>();
    c.Bind("message_lines", &model.messageLines);

    auto answer = c.RegisterStruct<NPCQuestAnswerEntry>();
    answer.RegisterMember("text", &NPCQuestAnswerEntry::text);
    answer.RegisterMember("index", &NPCQuestAnswerEntry::index);
    c.RegisterArray<std::vector<NPCQuestAnswerEntry>>();
    c.Bind("answers", &model.answers);

    c.Bind("dialogue_top", &model.dialogueTop);
    c.Bind("message_top", &model.messageTop);
    c.Bind("answers_top", &model.answersTop);

    c.Bind("complete_label", &model.completeLabel);
    c.Bind("cost_label", &model.costLabel);
    c.Bind("exit_tooltip", &model.exitTooltip);

    c.BindEventCallback("npcquest_click_close",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickClose(); });
    c.BindEventCallback("npcquest_select_answer",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
        {
            if (arguments.size() == 1)
                RmlClickAnswer(arguments[0].Get<int>(-1));
        });
    c.BindEventCallback("npcquest_complete",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickComplete(); });

    model.completeLabel = StringUtils::WideToNarrow(I18N::Game::ProceedWithQuest);
    model.costLabel = StringUtils::WideToNarrow(I18N::Game::Cost);
    model.exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);
}

void CNPCQuest::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CNPCQuest::Release()
{
    m_ItemTarget.Disable();
    m_RmlView.Release();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNPCQuest::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void CNPCQuest::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
    if (m_RmlView.Document())
    {
        if (bShow) m_RmlView.Document()->Show();
        else m_RmlView.Document()->Hide();
    }
}

bool CNPCQuest::UpdateMouseEvent()
{
    return !UI::RmlBridge::IsPointerOver(m_RmlView.Document());
}

bool CNPCQuest::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCQUEST) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPCQUEST);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool CNPCQuest::Update()
{
    SyncRmlModel();
    return true;
}

bool CNPCQuest::Render()
{
    return true;
}

// Into #item_view (m_ItemTarget), in window pixels: each item condition's icon beside its row of
// #conditions_anchor, 22 left of and 9 above the row's text at the panel's drawn scale, as the
// original drew it beside its own text.
void CNPCQuest::RenderItem3D()
{
    BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    BYTE byCurQuestState = g_csQuest.getQuestState2(int(byCurQuestIndex));

    if (QUEST_ING != byCurQuestState)
        return;

    Rml::ElementDocument* document = m_RmlView.Document();
    Rml::Element* root = document != nullptr ? document->GetElementById("content_root") : nullptr;
    Rml::Element* conditions = document != nullptr ? document->GetElementById("conditions_anchor") : nullptr;
    if (root == nullptr || conditions == nullptr)
        return;
    const float scale = UI::RmlBridge::DrawnScale(*root);

    QUEST_ATTRIBUTE* pQuest = g_csQuest.GetCurQuestAttribute();
    int nClass = gCharacterManager.GetBaseClass(Hero->Class);

    int row = 0;
    for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
    {
        if (!pQuest->QuestAct[i].byRequestClass[nClass])
            continue;

        if (QUEST_ITEM == pQuest->QuestAct[i].byQuestType)
        {
            Rml::Element* rowElement = conditions->GetChild(row++);
            Rml::Vector2f origin;
            if (rowElement == nullptr || !UI::RmlBridge::DrawnTopLeft(*rowElement, origin))
                break;

            int nItemType = (pQuest->QuestAct[i].wItemType * MAX_ITEM_INDEX)
                + pQuest->QuestAct[i].byItemSubType;

            int nItemLevel = pQuest->QuestAct[i].byItemLevel;

            ::RenderItem3D(origin.x - 22.f * scale, origin.y - 9.f * scale, 20.f * scale, 27.f * scale, nItemType,
                           nItemLevel, 0, 0, false);
        }
    }
}

bool CNPCQuest::IsVisible() const
{
    return CObject::IsVisible();
}

float CNPCQuest::GetLayerDepth()
{
    return 3.1f;
}

void CNPCQuest::ProcessOpening()
{
    g_csQuest.ShowQuestNpcWindow();
}

bool CNPCQuest::ProcessClosing()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
    return true;
}

bool CNPCQuest::BuildConditionRows(std::vector<NPCQuestConditionRow>& outRows)
{
    bool bCompletion = true;
    outRows.clear();

    QUEST_ATTRIBUTE* pQuest = g_csQuest.GetCurQuestAttribute();
    int nClass = gCharacterManager.GetBaseClass(Hero->Class);

    for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
    {
        if (!pQuest->QuestAct[i].byRequestClass[nClass])
            continue;

        wchar_t szTemp[128];
        Rml::String color;

        switch (pQuest->QuestAct[i].byQuestType)
        {
        case QUEST_ITEM:
        {
            int nItemType = (pQuest->QuestAct[i].wItemType * MAX_ITEM_INDEX)
                + pQuest->QuestAct[i].byItemSubType;
            int nItemNum = pQuest->QuestAct[i].byItemNum;
            int nItemLevel = pQuest->QuestAct[i].byItemLevel;

            // Same color/completion mapping RenderItemMobText() always used -- not reinterpreted here.
            if (!g_csQuest.FindQuestItemsInInven(nItemType, nItemNum, nItemLevel))
                color = "rgba(223,191,103,255)";
            else
            {
                color = "rgba(255,30,30,255)";
                bCompletion = false;
            }

            wchar_t szItemName[128];
            GetItemName(nItemType, nItemLevel, szItemName);
            mu_swprintf(szTemp, L"%ls x %d", szItemName, nItemNum);

            outRows.push_back({ StringUtils::WideToNarrow(szTemp), color });
        }
        break;

        case QUEST_MONSTER:
        {
            int nKillMobCount = g_csQuest.GetKillMobCount(int(pQuest->QuestAct[i].wItemType));

            if (int(pQuest->QuestAct[i].byItemNum) <= nKillMobCount)
            {
                color = "rgba(223,191,103,255)";
                nKillMobCount = int(pQuest->QuestAct[i].byItemNum);
            }
            else
            {
                color = "rgba(255,30,30,255)";
                bCompletion = false;
            }

            auto name = getMonsterName(int(pQuest->QuestAct[i].wItemType));
            mu_swprintf(szTemp, L"%ls x %d/%d", name, nKillMobCount, int(pQuest->QuestAct[i].byItemNum));

            outRows.push_back({ StringUtils::WideToNarrow(szTemp), color });
        }
        break;

        default:
            break; // no other quest-act type is ever produced in practice; nothing to add.
        }
    }

    return bCompletion;
}

void CNPCQuest::RmlClickClose()
{
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPCQUEST);
}

void CNPCQuest::RmlClickAnswer(int nAnswerIndex)
{
    const auto& entry = GameLogic::Quests::Dialog::GetEntry(g_iCurrentDialogScript);
    if (nAnswerIndex < 0 || nAnswerIndex >= entry.numAnswer)
        return;

    BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    int nAnswer = entry.answers[nAnswerIndex].returnCode;

    bool bErrorMessage = false;
    if (1 == nAnswer)
        bErrorMessage = g_csQuest.ProcessNextProgress();
    else if (2 == nAnswer)
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPCQUEST);
    else if (3 == nAnswer)
        SocketClient->ToGameServer()->SendLegacyQuestStateSetRequest(byCurQuestIndex, LegacyQuestState::Active);

    ::PlayBuffer(SOUND_INTERFACE01);

    int nNextDialogIndex = entry.answers[nAnswerIndex].link;
    if (0 < nNextDialogIndex && !bErrorMessage)
        g_csQuest.ShowDialogText(nNextDialogIndex);
}

void CNPCQuest::RmlClickComplete()
{
    if (!g_csQuest.BeQuestItem() || !m_bCompleteEnabled)
        return;

    SocketClient->ToGameServer()->SendLegacyQuestStateSetRequest(g_csQuest.GetCurrQuestIndex(), LegacyQuestState::Active);
    ::PlayBuffer(SOUND_INTERFACE01);
}

void CNPCQuest::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("nq_item") : nullptr, IsVisible());
    if (!m_RmlView.Document())
        return;

    auto& model = m_RmlView.GetModel();

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    const BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    const BYTE byCurQuestState = g_csQuest.getQuestState2(int(byCurQuestIndex));

    const bool bDarkNpcCheck = (Hero->Class == CLASS_DARK_LORD || Hero->Class == CLASS_DARK
        || Hero->Class == CLASS_RAGEFIGHTER) && bCheckNPC;

    model.npcName = StringUtils::WideToNarrow(bDarkNpcCheck ? g_csQuest.GetNPCName(2) : g_csQuest.GetNPCName(byCurQuestIndex));
    m_RmlView.MarkDirty("npc_name");

    model.showQuestTitle = !bDarkNpcCheck;
    m_RmlView.MarkDirty("show_quest_title");
    if (model.showQuestTitle)
    {
        model.questTitle = StringUtils::WideToNarrow(g_csQuest.getQuestTitle());
        m_RmlView.MarkDirty("quest_title");
    }

    model.showConditions = (QUEST_ING == byCurQuestState);
    m_RmlView.MarkDirty("show_conditions");
    if (model.showConditions)
    {
        std::vector<NPCQuestConditionRow> rows;
        m_bCompleteEnabled = BuildConditionRows(rows);
        model.conditions = std::move(rows);
        model.completeEnabled = m_bCompleteEnabled;
        m_RmlView.MarkDirty("conditions");
        m_RmlView.MarkDirty("complete_enabled");
    }
    else
    {
        m_bCompleteEnabled = false;
    }

    model.showCost = (QUEST_NO == byCurQuestState);
    m_RmlView.MarkDirty("show_cost");
    if (model.showCost)
    {
        wchar_t szTemp[128];
        ::ConvertGold(g_csQuest.GetNeedZen(), szTemp);
        model.costAmount = StringUtils::WideToNarrow(szTemp);

        model.costTier = UI::RmlBridge::GoldTierKey(GameLogic::Items::ClassifyGoldAmount(g_csQuest.GetNeedZen()));

        m_RmlView.MarkDirty("cost_amount");
        m_RmlView.MarkDirty("cost_tier");
    }

    model.messageLines.clear();
    for (int i = 0; i < g_iNumLineMessageBoxCustom; ++i)
        model.messageLines.push_back({ StringUtils::WideToNarrow(g_lpszMessageBoxCustom[i]) });
    m_RmlView.MarkDirty("message_lines");

    model.answers.clear();
    for (int j = 0; j < g_iNumAnswer; ++j)
    {
        if (0 == g_lpszDialogAnswer[j][0][0])
            break;
        model.answers.push_back({ StringUtils::WideToNarrow(g_lpszDialogAnswer[j][0]), j });
    }
    m_RmlView.MarkDirty("answers");

    // Same vertical-centering formula RenderText() used natively; the QUEST_ING branch depends on
    // how many message+answer lines are present this instance (a real per-instance value), the other
    // branch is a fixed lower anchor (room for the cost banner above it).
    constexpr float kLineAdvance = 18.f;
    constexpr float kAnswersAnchorTop = 250.f;
    const int iTotalLine = g_iNumLineMessageBoxCustom + g_iNumAnswer;
    model.messageTop = 66.f + static_cast<float>(NUM_LINE_CMB - iTotalLine) * kLineAdvance / 2.f;
    const bool questInProgress = QUEST_ING == byCurQuestState;
    model.answersTop = questInProgress
                           ? model.messageTop + static_cast<float>(g_iNumLineMessageBoxCustom) * kLineAdvance
                           : kAnswersAnchorTop;
    model.dialogueTop = questInProgress ? model.messageTop : kAnswersAnchorTop;
    m_RmlView.MarkDirty("dialogue_top");
    m_RmlView.MarkDirty("message_top");
    m_RmlView.MarkDirty("answers_top");
}
