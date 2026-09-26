//*****************************************************************************
// File: NewUINPCDialogue.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/NPCs/NPCDialogue.h"
#include "I18N/All.h"

#include "Core/Platform/CrtDbg.h"
#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Scaling/UITransform.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Core/Utilities/StringUtils.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#include <algorithm>
#include <string>

using namespace SEASON3B;
using namespace mu::ui::window;

CNPCDialogue::CNPCDialogue()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_dwContributePoint = 0;

    // Update()/SyncRmlModel() runs every frame regardless of visibility (same as CQuestProgress's
    // own pattern), so these must be safely zeroed before the first SetContents()/
    // ProcessQuestListReceive() call, not left as uninitialized array members the way the native
    // Render()-gated-by-visibility version could get away with.
    memset(m_aszNPCWords, 0, sizeof m_aszNPCWords);
    m_nSelNPCPage = 0;
    m_nMaxNPCPage = 0;

    memset(m_aszSelTexts, 0, sizeof m_aszSelTexts);
    m_nSelTextCount = 0;
    memset(m_anSelTextLine, 0, sizeof m_anSelTextLine);
    m_nSelSelText = 0;
    m_nSelSelTextPage = 0;
    m_nMaxSelTextPage = 0;
    memset(m_anSelTextLinePerPage, 0, sizeof m_anSelTextLinePerPage);
    memset(m_anSelTextCountPerPage, 0, sizeof m_anSelTextCountPerPage);

    m_eLowerView = NON_SEL_TEXTS_MODE;
    m_bQuestListMode = false;
    m_dwCurDlgIndex = 0;
    memset(m_adwQuestIndex, 0, sizeof m_adwQuestIndex);

    m_bCanClick = false;
}

CNPCDialogue::~CNPCDialogue()
{
    Release();
}

bool CNPCDialogue::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_NPC_DIALOGUE, this);

    SetPos(x, y);

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    m_nSelTextCount = 0;
    m_bQuestListMode = false;

    Show(false);

    return true;
}

void CNPCDialogue::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "npc_dialogue",
        [this](Rml::DataModelConstructor& c, NPCDialogueRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);

            c.Bind("npc_name", &model.npcName);

            auto textLine = c.RegisterStruct<NPCDialogueTextLine>();
            textLine.RegisterMember("text", &NPCDialogueTextLine::text);
            c.RegisterArray<std::vector<NPCDialogueTextLine>>();
            c.Bind("npc_lines", &model.npcLines);

            c.Bind("npc_prev_enabled", &model.npcPrevEnabled);
            c.Bind("npc_next_enabled", &model.npcNextEnabled);

            auto answer = c.RegisterStruct<NPCDialogueAnswerEntry>();
            answer.RegisterMember("text", &NPCDialogueAnswerEntry::text);
            answer.RegisterMember("index", &NPCDialogueAnswerEntry::index);
            c.RegisterArray<std::vector<NPCDialogueAnswerEntry>>();
            c.Bind("answers", &model.answers);

            c.Bind("ans_prev_enabled", &model.ansPrevEnabled);
            c.Bind("ans_next_enabled", &model.ansNextEnabled);
            c.Bind("show_answers", &model.showAnswers);

            c.Bind("show_contribute", &model.showContribute);
            c.Bind("contribute_text", &model.contributeText);

            c.Bind("exit_tooltip", &model.exitTooltip);

            c.BindEventCallback("npcdialogue_click_close",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickClose(); });
            c.BindEventCallback("npcdialogue_npc_prev_page",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickNpcPrevPage(); });
            c.BindEventCallback("npcdialogue_npc_next_page",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickNpcNextPage(); });
            c.BindEventCallback("npcdialogue_ans_prev_page",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickAnsPrevPage(); });
            c.BindEventCallback("npcdialogue_ans_next_page",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickAnsNextPage(); });
            c.BindEventCallback("npcdialogue_select_answer",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                {
                    if (arguments.size() == 1)
                        RmlClickSelectAnswer(arguments[0].Get<int>(-1));
                });
        });

    if (modelCreated)
    {
        m_RmlBinder.GetModel().exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);
    }

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
        "Data/Interface/RmlUi/npc_dialogue.rml");
}

void CNPCDialogue::ReloadRmlTheme()
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

void CNPCDialogue::Release()
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

void CNPCDialogue::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void CNPCDialogue::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else m_pRmlDoc->Hide();
    }
}

bool CNPCDialogue::UpdateMouseEvent()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_NPC_DIALOGUE))
        return false;

    float panelWidth = ND_WIDTH;
    float panelHeight = ND_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool CNPCDialogue::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPC_DIALOGUE))
    {
        if (mu::ui::window::IsPress(VK_ESCAPE))
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPC_DIALOGUE);
            return false;
        }
    }

    return true;
}

bool CNPCDialogue::Update()
{
    SyncRmlModel();
    return true;
}

bool CNPCDialogue::Render()
{
    // RmlUi's #panel owns all chrome/text/list rendering now; nothing left to draw natively.
    return true;
}

bool CNPCDialogue::IsVisible() const
{
    return CObject::IsVisible();
}

float CNPCDialogue::GetLayerDepth()
{
    return 3.1f;
}

void CNPCDialogue::ProcessOpening()
{
    m_bQuestListMode = false;
    SetContents(0);
    ::PlayBuffer(SOUND_INTERFACE01);
}

bool CNPCDialogue::ProcessClosing()
{
    m_dwCurDlgIndex = 0;
    m_dwContributePoint = 0;
    m_bQuestListMode = false;
    SocketClient->ToGameServer()->SendCloseNpcRequest();
    ::PlayBuffer(SOUND_CLICK01);
    return true;
}

void CNPCDialogue::SetContents(DWORD dwDlgIndex)
{
    m_dwCurDlgIndex = dwDlgIndex;
    SetCurNPCWords();
    SetCurSelTexts();
    m_bCanClick = true;
    m_nSelSelText = 0;
}

namespace
{
    // Wrap budget, in native logical units, for text drawn inside `container` (see
    // ResolveDialogueWrapGeometry()); `fallback` when the container is not laid out yet.
    float NativeWrapWidth(Rml::Element& container, const UI::Scaling::Transform& panelTransform, float fallback)
    {
        const float nativeTextPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, panelTransform);
        const float width = container.GetBox().GetSize(Rml::BoxArea::Border).x;
        // A counter-scaled container (legacy .sharp-text) is laid out in physical pixels: its
        // width and font-size are both multiplied by the panel scale, so the ratio is unchanged.
        const float drawnTextPx = container.GetComputedValues().font_size() * panelTransform.scaleX;
        if (width <= 0.0f || drawnTextPx <= 0.0f)
            return fallback;
        return width * nativeTextPx / drawnTextPx;
    }
}

void CNPCDialogue::ResolveDialogueWrapGeometry(float& npcWrapWidth, int& npcLinesPerPage,
    float& answerWrapWidth, int& answerLinesPerPage) const
{
    npcWrapWidth = 160.f;
    npcLinesPerPage = ND_NPC_MAX_LINE_PER_PAGE;
    answerWrapWidth = 160.f;
    answerLinesPerPage = ND_SEL_TEXT_MAX_LINE_PER_PAGE;

    if (!m_pRmlDoc)
        return;

    const auto transform = UI::Scaling::TransformForLayout(GetLayoutMode(), WindowWidth, WindowHeight);
    if (transform.scaleX <= 0.0f || transform.scaleY <= 0.0f)
        return;

    // DivideStringByPixel() wraps using g_pRenderText's own native pixel metrics (in this window's
    // logical units, at the native text size), a different measurement system than RmlUi's font
    // rendering. The RmlUi container's box width is in #panel's own layout units (a transform does
    // not change box sizes), and its text is drawn at its own font-size there: scaling the width
    // by native text size / drawn text size gives the budget that fills the container.
    Rml::Element* npcContainer = m_pRmlDoc->GetElementById("npc_lines_container");
    Rml::Element* answerContainer = m_pRmlDoc->GetElementById("answers_container");
    if (npcContainer)
        npcWrapWidth = NativeWrapWidth(*npcContainer, transform, npcWrapWidth);
    if (answerContainer)
        answerWrapWidth = NativeWrapWidth(*answerContainer, transform, answerWrapWidth);

    // One physical text line's real rendered height -- shared below by both the NPC-words area and
    // the answers area (both use the same font-size in npc_dialogue.rcss), sampled from .nd-line
    // specifically because it's always exactly one physical line (white-space:nowrap); an
    // .nd-answer-row can be several physical lines joined by '\n' (SyncRmlModel()'s own comment),
    // so its own box height isn't a reliable single-line measurement. Only ever populated from
    // whatever's currently bound -- empty on the very first call this session, before
    // SetContents() has ever run once (this window always has NPC words whenever it's open, so
    // every call after the first one finds a real line here).
    float linePitchPx = 0.f;
    if (npcContainer)
    {
        if (Rml::Element* firstLine = npcContainer->GetChild(0))
            linePitchPx = firstLine->GetBox().GetSize(Rml::BoxArea::Border).y;
        // A theme may lay the text out in physical pixels and scale it back (a counter-scaled
        // layer, legacy's .sharp-text): its line pitch is then in those units, not #panel's.
        if (npcContainer->GetComputedValues().has_local_transform())
            linePitchPx /= transform.scaleX;
    }

    if (linePitchPx <= 0.0f)
        return;

    // .nd-npc-lines/.nd-answers have no fixed height of their own (they size to content, which is
    // exactly what's being decided here) -- the real per-theme budget is the gap to the next fixed
    // anchor below each: the pager row it must not run into, not the (theme-owned but purely
    // decorative) divider further below.
    if (npcContainer)
    {
        if (Rml::Element* npcBoundary = m_pRmlDoc->GetElementById("btn_npc_next"))
        {
            const float availableHeightPx = npcBoundary->GetAbsoluteOffset().y - npcContainer->GetAbsoluteOffset().y;
            if (availableHeightPx > 0.0f)
                npcLinesPerPage = std::max(1, static_cast<int>(availableHeightPx / linePitchPx));
        }
    }
    if (answerContainer)
    {
        if (Rml::Element* answerBoundary = m_pRmlDoc->GetElementById("btn_ans_next"))
        {
            const float availableHeightPx = answerBoundary->GetAbsoluteOffset().y - answerContainer->GetAbsoluteOffset().y;
            if (availableHeightPx > 0.0f)
                answerLinesPerPage = std::max(1, static_cast<int>(availableHeightPx / linePitchPx));
        }
    }
}

void CNPCDialogue::SetCurNPCWords(int nQuestListCount)
{
    memset(m_aszNPCWords[0], 0, sizeof(wchar_t) * ND_NPC_LINE_MAX * ND_WORDS_ROW_MAX);

    g_pRenderText->SetFont(g_hFont);
    const wchar_t* pszSrc;
    if (m_bQuestListMode)
        pszSrc = 0 < nQuestListCount ? g_QuestMng.GetWords(1501) : g_QuestMng.GetWords(1502);
    else
        pszSrc = g_QuestMng.GetNPCDlgNPCWords(m_dwCurDlgIndex);

    // Wrap in this window's own layout space: the budget below is in its logical units, and there
    // MeasureText() reports the width the text is drawn at (the native text size).
    const UI::Scaling::ScopedActiveTransform wrapSpace(
        UI::Scaling::TransformForLayout(GetLayoutMode(), WindowWidth, WindowHeight));
    float npcWrapWidth, answerWrapWidth;
    int npcLinesPerPage, answerLinesPerPage;
    ResolveDialogueWrapGeometry(npcWrapWidth, npcLinesPerPage, answerWrapWidth, answerLinesPerPage);

    int nLine = ::DivideStringByPixel(&m_aszNPCWords[0][0], ND_NPC_LINE_MAX, ND_WORDS_ROW_MAX,
        pszSrc, static_cast<int>(npcWrapWidth));

    if (1 > nLine)
        return;

    // Stored, not just used locally -- SyncRmlModel() slices m_aszNPCWords by this same value when
    // it later binds the current page's lines, so the two never disagree on where a page boundary
    // falls (see m_nNpcLinesPerPage's own comment).
    m_nNpcLinesPerPage = npcLinesPerPage;
    m_nMaxNPCPage = (nLine - 1) / m_nNpcLinesPerPage;
    m_eLowerView = (1 <= m_nMaxNPCPage) ? NON_SEL_TEXTS_MODE : SEL_TEXTS_MODE;

    m_nSelNPCPage = 0;
}

void CNPCDialogue::SetCurSelTexts()
{
    ::memset(m_aszSelTexts[0], 0, sizeof(wchar_t) * ND_SEL_TEXT_LINE_MAX * ND_WORDS_ROW_MAX);
    ::memset(m_anSelTextLine, 0, sizeof(int) * (ND_QUEST_INDEX_MAX_COUNT + 1));

    g_pRenderText->SetFont(g_hFont);

    // Wrap in this window's own layout space: the budget below is in its logical units, and there
    // MeasureText() reports the width the text is drawn at (the native text size).
    const UI::Scaling::ScopedActiveTransform wrapSpace(
        UI::Scaling::TransformForLayout(GetLayoutMode(), WindowWidth, WindowHeight));
    float npcWrapWidth, answerWrapWidth;
    int npcLinesPerPage, answerLinesPerPage;
    ResolveDialogueWrapGeometry(npcWrapWidth, npcLinesPerPage, answerWrapWidth, answerLinesPerPage);

    wchar_t szAnswer[2 * ND_WORDS_ROW_MAX];
    const wchar_t* pszAnswer;
    int nSelTextLineSum = 0;
    int i;
    for (i = 0; i < QM_MAX_ND_ANSWER; ++i)
    {
        ::mu_swprintf(szAnswer, L"%d. ", i + 1);
        pszAnswer = g_QuestMng.GetNPCDlgAnswer(m_dwCurDlgIndex, i);
        if (NULL == pszAnswer)
            break;
        ::wcscat(szAnswer, pszAnswer);

        m_anSelTextLine[i] = ::DivideStringByPixel(&m_aszSelTexts[nSelTextLineSum][0], 2, ND_WORDS_ROW_MAX, szAnswer, static_cast<int>(answerWrapWidth), false);

        nSelTextLineSum += m_anSelTextLine[i];

        if (ND_SEL_TEXT_LINE_MAX <= nSelTextLineSum)
            break;
    }

    m_nSelTextCount = i;

    CalculateSelTextMaxPage(i, answerLinesPerPage);
}

void CNPCDialogue::CalculateSelTextMaxPage(int nSelTextCount, int nMaxLinePerPage)
{
    m_nSelSelTextPage = 0;
    m_nMaxSelTextPage = 0;
    ::memset(m_anSelTextLinePerPage, 0, sizeof(int) * ND_SEL_TEXT_PAGE_LIMIT);
    ::memset(m_anSelTextCountPerPage, 0, sizeof(int) * ND_SEL_TEXT_PAGE_LIMIT);

    int i;
    for (i = 0; i < nSelTextCount; ++i)
    {
        ++m_anSelTextCountPerPage[m_nMaxSelTextPage];
        m_anSelTextLinePerPage[m_nMaxSelTextPage] += m_anSelTextLine[i];

        if (m_anSelTextLinePerPage[m_nMaxSelTextPage] > nMaxLinePerPage)
        {
            --m_anSelTextCountPerPage[m_nMaxSelTextPage];
            m_anSelTextLinePerPage[m_nMaxSelTextPage] -= m_anSelTextLine[i];
            if (ND_SEL_TEXT_PAGE_LIMIT == ++m_nMaxSelTextPage)
                break;
            ++m_anSelTextCountPerPage[m_nMaxSelTextPage];
            m_anSelTextLinePerPage[m_nMaxSelTextPage] += m_anSelTextLine[i];
        }
    }
}

void CNPCDialogue::SetQuestListText(DWORD* adwSrcQuestIndex, int nIndexCount)
{
    _ASSERT(0 <= nIndexCount && nIndexCount <= ND_QUEST_INDEX_MAX_COUNT);

    ::memset(m_adwQuestIndex, 0, sizeof(DWORD) * ND_QUEST_INDEX_MAX_COUNT);
    ::memcpy(m_adwQuestIndex, adwSrcQuestIndex, sizeof(DWORD) * nIndexCount);

    m_nSelTextCount = nIndexCount + 1;

    ::memset(m_aszSelTexts[0], 0, sizeof(wchar_t) * ND_SEL_TEXT_LINE_MAX * ND_WORDS_ROW_MAX);
    ::memset(m_anSelTextLine, 0, sizeof(int) * (ND_QUEST_INDEX_MAX_COUNT + 1));

    // Wrap in this window's own layout space: the budget below is in its logical units, and there
    // MeasureText() reports the width the text is drawn at (the native text size).
    const UI::Scaling::ScopedActiveTransform wrapSpace(
        UI::Scaling::TransformForLayout(GetLayoutMode(), WindowWidth, WindowHeight));
    float npcWrapWidth, answerWrapWidth;
    int npcLinesPerPage, answerLinesPerPage;
    ResolveDialogueWrapGeometry(npcWrapWidth, npcLinesPerPage, answerWrapWidth, answerLinesPerPage);

    wchar_t szSelText[2 * ND_WORDS_ROW_MAX];
    const wchar_t* pszSelText;
    int nSelTextRow = 0;
    g_pRenderText->SetFont(g_hFont);
    int i;
    for (i = 0; i < m_nSelTextCount; ++i)
    {
        if (m_nSelTextCount - 1 == i)
        {
            ::mu_swprintf(szSelText, L"%d. ", i + 1);
            pszSelText = g_QuestMng.GetWords(1007);
        }
        else
        {
            ::mu_swprintf(szSelText, L"%d. [Q]", i + 1);
            pszSelText = g_QuestMng.GetSubject(m_adwQuestIndex[i]);
        }

        if (NULL == pszSelText)
            break;
        ::wcscat(szSelText, pszSelText);

        m_anSelTextLine[i] = ::DivideStringByPixel(&m_aszSelTexts[nSelTextRow][0],
            2, ND_WORDS_ROW_MAX, szSelText, static_cast<int>(answerWrapWidth), false);

        nSelTextRow += m_anSelTextLine[i];

        if (ND_SEL_TEXT_LINE_MAX < nSelTextRow)
            break;
    }

    CalculateSelTextMaxPage(i, answerLinesPerPage);
}

void CNPCDialogue::SetContributePoint(DWORD dwContributePoint)
{
    if (543 == g_QuestMng.GetNPCIndex() || 544 == g_QuestMng.GetNPCIndex())
        m_dwContributePoint = dwContributePoint;
}

void CNPCDialogue::ProcessSelTextResult()
{
    if (m_bQuestListMode)
    {
        if (m_nSelSelText == m_nSelTextCount)
        {
            m_bQuestListMode = false;
            SetContents(0);
        }
        else
        {
            const DWORD dwSelectedQuest = m_adwQuestIndex[m_nSelSelText - 1];
            const auto questNumber = static_cast<uint16_t>(LOWORD(dwSelectedQuest));
            const auto questGroup = static_cast<uint16_t>(HIWORD(dwSelectedQuest));
            SocketClient->ToGameServer()->SendQuestSelectRequest(questNumber, questGroup, (BYTE)m_nSelSelText);
        }
    }
    else
    {
        int nAnswerResult = g_QuestMng.GetNPCDlgAnswerResult(m_dwCurDlgIndex, m_nSelSelText - 1);
        if (900 >= nAnswerResult)
        {
            SetContents(nAnswerResult);
        }
        else
        {
            switch (nAnswerResult)
            {
            case 901:
                SocketClient->ToGameServer()->SendAvailableQuestsRequest();
                break;

            case 902:
                SocketClient->ToGameServer()->SendNpcBuffRequest();
                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPC_DIALOGUE);
                break;
            case 903:
                SocketClient->ToGameServer()->SendGensJoinRequest(GensType::Duprian);
                break;
            case 904:
                SocketClient->ToGameServer()->SendGensJoinRequest(GensType::Vanert);
                break;
            case 905:
                SocketClient->ToGameServer()->SendGensLeaveRequest();
                break;
            case 906:
                SocketClient->ToGameServer()->SendGensRewardRequest(GensType::Duprian);
                break;
            case 907:
                SocketClient->ToGameServer()->SendGensRewardRequest(GensType::Vanert);
                break;

            default:
                SetContents(999);
            }
        }
    }
}

void CNPCDialogue::ProcessQuestListReceive(DWORD* adwSrcQuestIndex, int nIndexCount)
{
    m_bQuestListMode = true;
    SetCurNPCWords(nIndexCount);
    SetQuestListText(adwSrcQuestIndex, nIndexCount);
    m_bCanClick = true;
    m_nSelSelText = 0;
}

enum GENS_JOINING_ERR_CODE
{
    GJEC_NONE_ERR = 0,
    GJEC_REG_GENS_ERR,
    GJEC_GENS_SECEDE_DAY_ERR,
    GJEC_REG_GENS_LV_ERR,
    GJEC_REG_GENS_NOT_EQL_GUILDMA_ERR,
    GJEC_NONE_REG_GENS_GUILDMA_ERR,
    GJEC_PARTY,
    GJEC_GUILD_UNION_MASTER
};

void CNPCDialogue::ProcessGensJoiningReceive(BYTE byResult, BYTE byInfluence)
{
    switch (byResult)
    {
    case GJEC_NONE_ERR:
        Hero->m_byGensInfluence = byInfluence;
        SetContents(5);
        break;
    case GJEC_REG_GENS_ERR:
        SetContents(9);
        break;
    case GJEC_GENS_SECEDE_DAY_ERR:
        SetContents(11);
        break;
    case GJEC_REG_GENS_LV_ERR:
        SetContents(8);
        break;
    case GJEC_REG_GENS_NOT_EQL_GUILDMA_ERR:
        SetContents(10);
        break;
    case GJEC_NONE_REG_GENS_GUILDMA_ERR:
        SetContents(12);
        break;
    case GJEC_PARTY:
        SetContents(18);
        break;
    case GJEC_GUILD_UNION_MASTER:
        SetContents(19);
        break;
    }
}

enum GENS_SECEDE_ERR_CODE
{
    GSEC_NONE_ERR = 0,
    GSEC_IS_NOT_REG_GENS,
    GSEC_GUILD_MASTER_CAN_NOT_SECEDE,
    GSEC_IS_NOT_INFLUENCE_NPC
};

enum GENS_REWARD_ERR_CODE
{
    GENS_REWARD_CALL = 0,
    GENS_REWARD_TERM,
    GENS_REWARD_TARGET,
    GENS_REWARD_SPACE,
    GENS_REWARD_ALREADY,
    GENS_REWARD_DIFFERENT,
    GENS_REWARD_NOT_REG,
};

void CNPCDialogue::ProcessGensSecessionReceive(BYTE byResult)
{
    switch (byResult)
    {
    case GSEC_NONE_ERR:
        Hero->m_byGensInfluence = 0;
        SetContents(16);
        break;
    case GSEC_IS_NOT_REG_GENS:
        SetContents(15);
        break;
    case GSEC_GUILD_MASTER_CAN_NOT_SECEDE:
        SetContents(14);
        break;
    case GSEC_IS_NOT_INFLUENCE_NPC:
        SetContents(17);
        break;
    }
}

void CNPCDialogue::ProcessGensRewardReceive(BYTE byResult)
{
    switch (byResult)
    {
    case GENS_REWARD_CALL:
        SetContents(20);
        break;
    case GENS_REWARD_TERM:
        SetContents(21);
        break;
    case GENS_REWARD_TARGET:
        SetContents(22);
        break;
    case GENS_REWARD_SPACE:
        SetContents(23);
        break;
    case GENS_REWARD_ALREADY:
        SetContents(24);
        break;
    case GENS_REWARD_DIFFERENT:
        SetContents(17);
        break;
    case GENS_REWARD_NOT_REG:
        SetContents(25);
        break;
    }
}

void CNPCDialogue::RmlClickClose()
{
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPC_DIALOGUE);
}

void CNPCDialogue::RmlClickNpcPrevPage()
{
    if (m_nSelNPCPage <= 0)
        return; // already at the first page -- native L button was already locked here.

    --m_nSelNPCPage;
    ::PlayBuffer(SOUND_CLICK01);
}

void CNPCDialogue::RmlClickNpcNextPage()
{
    if (m_nSelNPCPage < m_nMaxNPCPage)
    {
        ++m_nSelNPCPage;
        if (m_nSelNPCPage == m_nMaxNPCPage && m_eLowerView == NON_SEL_TEXTS_MODE)
        {
            // Last NPC page reached for the first time -- reveal the sel-text list, matching the
            // native "R" button's double duty (old ProcessBtns()'s own comment).
            m_eLowerView = SEL_TEXTS_MODE;
        }
    }
    else
    {
        return; // last page already reached -- native R button was already locked here.
    }

    ::PlayBuffer(SOUND_CLICK01);
}

void CNPCDialogue::RmlClickAnsPrevPage()
{
    if (m_nSelSelTextPage <= 0)
        return;

    --m_nSelSelTextPage;
    ::PlayBuffer(SOUND_CLICK01);
}

void CNPCDialogue::RmlClickAnsNextPage()
{
    if (m_nSelSelTextPage >= m_nMaxSelTextPage)
        return;

    ++m_nSelSelTextPage;
    ::PlayBuffer(SOUND_CLICK01);
}

void CNPCDialogue::RmlClickSelectAnswer(int nIndex)
{
    if (m_eLowerView != SEL_TEXTS_MODE || !m_bCanClick)
        return;
    if (nIndex < 0 || nIndex >= m_nSelTextCount)
        return;

    // ProcessSelTextResult() reads the 1-based m_nSelSelText -- kept exactly as the native hit-test
    // populated it, so that function needed no changes at all for this port.
    m_nSelSelText = nIndex + 1;
    m_bCanClick = false;
    ProcessSelTextResult();
    ::PlayBuffer(SOUND_CLICK01);
}

void CNPCDialogue::SyncRmlModel()
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
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    model.npcName = StringUtils::WideToNarrow(g_QuestMng.GetNPCName());
    m_RmlBinder.MarkDirty("npc_name");

    // Current page's up-to-m_nNpcLinesPerPage lines, already wrapped by SetCurNPCWords()'s
    // DivideStringByPixel() call -- bound as literal non-wrapping lines (see NPCDialogueRmlModel.h's
    // own npcLines comment). Slices by m_nNpcLinesPerPage, not a hardcoded macro, so this always
    // agrees with whatever value SetCurNPCWords() actually used to compute m_nMaxNPCPage.
    model.npcLines.clear();
    for (int i = 0; i < m_nNpcLinesPerPage; ++i)
    {
        const wchar_t* line = m_aszNPCWords[i + m_nNpcLinesPerPage * m_nSelNPCPage];
        if (line[0] == L'\0')
            break;
        model.npcLines.push_back({ StringUtils::WideToNarrow(line) });
    }
    m_RmlBinder.MarkDirty("npc_lines");

    model.npcPrevEnabled = (m_nSelNPCPage > 0);
    model.npcNextEnabled = (m_nSelNPCPage < m_nMaxNPCPage);
    m_RmlBinder.MarkDirty("npc_prev_enabled");
    m_RmlBinder.MarkDirty("npc_next_enabled");

    model.showAnswers = (m_eLowerView == SEL_TEXTS_MODE);
    m_RmlBinder.MarkDirty("show_answers");

    if (m_eLowerView == SEL_TEXTS_MODE)
    {
        // Same two-level slice RenderText()/UpdateSelTextMouseEvent() used natively: entries (the
        // clickable unit) per page from m_anSelTextCountPerPage, physical lines per entry from
        // m_anSelTextLine -- joined here since one Entry now shows a whole logical entry, not one
        // physical line at a time.
        int nStartSelText = 0;
        for (int p = 0; p < m_nSelSelTextPage; ++p)
            nStartSelText += m_anSelTextCountPerPage[p];
        int nStartLine = 0;
        for (int p = 0; p < m_nSelSelTextPage; ++p)
            nStartLine += m_anSelTextLinePerPage[p];

        model.answers.clear();
        int nLineCursor = nStartLine;
        for (int i = 0; i < m_anSelTextCountPerPage[m_nSelSelTextPage]; ++i)
        {
            const int entryIndex = nStartSelText + i;
            std::wstring joined;
            for (int l = 0; l < m_anSelTextLine[entryIndex]; ++l)
            {
                if (l > 0)
                    joined += L'\n';
                joined += m_aszSelTexts[nLineCursor + l];
            }
            nLineCursor += m_anSelTextLine[entryIndex];
            model.answers.push_back({ StringUtils::WideToNarrow(joined.c_str()), entryIndex });
        }
    }
    else
    {
        model.answers.clear();
    }
    m_RmlBinder.MarkDirty("answers");

    model.ansPrevEnabled = (m_nSelSelTextPage > 0);
    model.ansNextEnabled = (m_nSelSelTextPage < m_nMaxSelTextPage);
    m_RmlBinder.MarkDirty("ans_prev_enabled");
    m_RmlBinder.MarkDirty("ans_next_enabled");

    // Same gate RenderContributePoint() used natively.
    const bool showContribute = (543 == g_QuestMng.GetNPCIndex() && 1 == Hero->m_byGensInfluence)
        || (544 == g_QuestMng.GetNPCIndex() && 2 == Hero->m_byGensInfluence);
    model.showContribute = showContribute;
    m_RmlBinder.MarkDirty("show_contribute");

    if (showContribute)
    {
        wchar_t szContribute[32];
        mu_swprintf(szContribute, I18N::Game::GainContributionU, m_dwContributePoint);
        model.contributeText = StringUtils::WideToNarrow(szContribute);
    }
    else
    {
        model.contributeText.clear();
    }
    m_RmlBinder.MarkDirty("contribute_text");
}
