//*****************************************************************************
// File: NewUINPCQuest.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Quests/NPCQuest.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Quests/DialogStructure.h"
#include "I18N/All.h"

#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "UI/Scaling/UITransform.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlRootTransform.h"
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
    m_pNewUI3DRenderMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CNPCQuest::~CNPCQuest()
{
    Release();
}

bool CNPCQuest::Create(CManager* pNewUIMng,
    C3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng
        || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_NPCQUEST, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    SetPos(x, y);

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    Show(false);

    return true;
}

void CNPCQuest::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "npc_quest",
        [this](Rml::DataModelConstructor& c, NPCQuestRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
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
        });

    if (modelCreated)
    {
        m_RmlBinder.GetModel().completeLabel = StringUtils::WideToNarrow(I18N::Game::ProceedWithQuest);
        m_RmlBinder.GetModel().costLabel = StringUtils::WideToNarrow(I18N::Game::Cost);
        m_RmlBinder.GetModel().exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);
    }

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
        "Data/Interface/RmlUi/npc_quest.rml");

    // Frame background panel uses the background context -- see NPCQuestBgRmlModel (NPCQuest.h).
    if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
    {
        const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "npc_quest_bg",
            [](Rml::DataModelConstructor& c, NPCQuestBgRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);
            });
        if (bgModelCreated)
        {
            m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/npc_quest_bg.rml");
        }
    }
}

void CNPCQuest::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;

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
    // Next frame's SyncRmlModel() self-corrects visibility for both docs.
}

void CNPCQuest::Release()
{
    if (m_pRmlDoc)
    {
        m_pRmlDoc->Close();
        m_pRmlDoc = nullptr;
    }

    if (m_pRmlBgDoc)
    {
        m_pRmlBgDoc->Close();
        m_pRmlBgDoc = nullptr;
    }

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = NULL;
    }

    if (m_pNewUIMng)
    {
        UI::RmlBridge::UnregisterForThemeReload(this);
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
    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else m_pRmlDoc->Hide();
    }
}

bool CNPCQuest::UpdateMouseEvent()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_NPCQUEST))
        return false;

    // Frame chrome (and thus #panel's real size) lives in the background-context doc, not
    // m_pRmlDoc -- see BuildRmlUi()'s own comment on the fg/bg split this window needs for its
    // still-native live-3D quest-item preview.
    float panelWidth = NPCQUEST_WIDTH;
    float panelHeight = NPCQUEST_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlBgDoc, "panel", panelWidth, panelHeight);

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
        return false;

    return true;
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
    // RmlUi's #panel owns all chrome/text/list rendering now; only the live quest-condition item
    // preview is still a native per-frame call, via Render3D() (I3DRenderObj's own separate draw
    // pass, unchanged interface).
    return true;
}

void CNPCQuest::RenderItem3D()
{
    BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    BYTE byCurQuestState = g_csQuest.getQuestState2(int(byCurQuestIndex));

    if (QUEST_ING != byCurQuestState)
        return;

    // #conditions_anchor (npc_quest.rml) and its own live child-row height replace a hardcoded
    // m_Pos+235 origin / 32px step -- the RmlUi-rendered condition ROW TEXT lives in these exact
    // rows, so reading their real geometry (instead of duplicating npc_quest.rcss's
    // .nq-condition-row height in C++) is what keeps this still-native item-icon preview aligned
    // with them across a theme change, rather than only by both themes happening to agree today.
    auto x = float(m_Pos.x + 30);
    auto y = float(m_Pos.y + 235);
    float rowStep = 32.f;

    if (m_pRmlDoc)
    {
        if (Rml::Element* conditionsEl = m_pRmlDoc->GetElementById("conditions_anchor"))
        {
            const auto transform = UI::Scaling::TransformForLayout(GetLayoutMode(), WindowWidth, WindowHeight);
            if (transform.scaleX > 0.0f && transform.scaleY > 0.0f)
            {
                // Icon sits 22px left / 9px above the row's own text origin -- a native rendering
                // choice, not theme geometry, so it stays a fixed offset from whatever the anchor's
                // live position resolves to.
                const auto offset = conditionsEl->GetAbsoluteOffset();
                x = UI::Scaling::LogicalX(transform, offset.x) - 22.f;
                y = UI::Scaling::LogicalY(transform, offset.y) - 9.f;

                // A row's box height is in the panel's own (logical) units -- a transform does not
                // change box sizes -- unless a theme lays the rows out in physical pixels inside a
                // counter-scaled text layer (legacy .sharp-text): then it is divided back.
                if (Rml::Element* firstRow = conditionsEl->GetChild(0))
                {
                    float rowHeight = firstRow->GetBox().GetSize(Rml::BoxArea::Border).y;
                    if (conditionsEl->GetComputedValues().has_local_transform())
                        rowHeight /= transform.scaleY;
                    if (rowHeight > 0.0f)
                        rowStep = rowHeight;
                }
            }
        }
    }

    const float Height = 27.f;

    QUEST_ATTRIBUTE* pQuest = g_csQuest.GetCurQuestAttribute();
    int nClass = gCharacterManager.GetBaseClass(Hero->Class);

    for (int i = 0; i < pQuest->shQuestConditionNum; ++i)
    {
        if (!pQuest->QuestAct[i].byRequestClass[nClass])
            continue;

        if (QUEST_ITEM == pQuest->QuestAct[i].byQuestType)
        {
            int nItemType = (pQuest->QuestAct[i].wItemType * MAX_ITEM_INDEX)
                + pQuest->QuestAct[i].byItemSubType;

            int nItemLevel = pQuest->QuestAct[i].byItemLevel;

            ::RenderItem3D(x, y, 20.f, Height, nItemType, nItemLevel, 0, 0, false);

            y += rowStep;
        }
    }
}

void CNPCQuest::Render3D()
{
    RenderItem3D();
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
    if (m_pRmlBgDoc)
    {
        UI::RmlBridge::SyncRootTransform(m_BgRmlBinder, m_Pos);
        // RenderBackgroundLayer() renders whatever's shown in the shared background context
        // regardless of caller, so this Hide()/Show() is what keeps the bg panel hidden when closed.
        UI::RmlBridge::SyncDocumentVisibility(m_pRmlBgDoc, IsVisible());
    }

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
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    const BYTE byCurQuestIndex = g_csQuest.GetCurrQuestIndex();
    const BYTE byCurQuestState = g_csQuest.getQuestState2(int(byCurQuestIndex));

    const bool bDarkNpcCheck = (Hero->Class == CLASS_DARK_LORD || Hero->Class == CLASS_DARK
        || Hero->Class == CLASS_RAGEFIGHTER) && bCheckNPC;

    model.npcName = StringUtils::WideToNarrow(bDarkNpcCheck ? g_csQuest.GetNPCName(2) : g_csQuest.GetNPCName(byCurQuestIndex));
    m_RmlBinder.MarkDirty("npc_name");

    model.showQuestTitle = !bDarkNpcCheck;
    m_RmlBinder.MarkDirty("show_quest_title");
    if (model.showQuestTitle)
    {
        model.questTitle = StringUtils::WideToNarrow(g_csQuest.getQuestTitle());
        m_RmlBinder.MarkDirty("quest_title");
    }

    model.showConditions = (QUEST_ING == byCurQuestState);
    m_RmlBinder.MarkDirty("show_conditions");
    if (model.showConditions)
    {
        std::vector<NPCQuestConditionRow> rows;
        m_bCompleteEnabled = BuildConditionRows(rows);
        model.conditions = std::move(rows);
        model.completeEnabled = m_bCompleteEnabled;
        m_RmlBinder.MarkDirty("conditions");
        m_RmlBinder.MarkDirty("complete_enabled");
    }
    else
    {
        m_bCompleteEnabled = false;
    }

    model.showCost = (QUEST_NO == byCurQuestState);
    m_RmlBinder.MarkDirty("show_cost");
    if (model.showCost)
    {
        wchar_t szTemp[128];
        ::ConvertGold(g_csQuest.GetNeedZen(), szTemp);
        model.costAmount = StringUtils::WideToNarrow(szTemp);

        model.costTier = UI::RmlBridge::GoldTierKey(GameLogic::Items::ClassifyGoldAmount(g_csQuest.GetNeedZen()));

        m_RmlBinder.MarkDirty("cost_amount");
        m_RmlBinder.MarkDirty("cost_tier");
    }

    model.messageLines.clear();
    for (int i = 0; i < g_iNumLineMessageBoxCustom; ++i)
        model.messageLines.push_back({ StringUtils::WideToNarrow(g_lpszMessageBoxCustom[i]) });
    m_RmlBinder.MarkDirty("message_lines");

    model.answers.clear();
    for (int j = 0; j < g_iNumAnswer; ++j)
    {
        if (0 == g_lpszDialogAnswer[j][0][0])
            break;
        model.answers.push_back({ StringUtils::WideToNarrow(g_lpszDialogAnswer[j][0]), j });
    }
    m_RmlBinder.MarkDirty("answers");

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
    m_RmlBinder.MarkDirty("dialogue_top");
    m_RmlBinder.MarkDirty("message_top");
    m_RmlBinder.MarkDirty("answers_top");
}
