#include "stdafx.h"
#include "I18N/All.h"
#include "Render/Text/CUIRenderText.h"

#ifdef PBG_ADD_GENSRANKING
#include "UI/HUD/GensRanking.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"

#include "Core/Utilities/StringUtils.h"
#include "Core/Utilities/UsefulDef.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

using namespace SEASON3B;
using namespace mu::ui::window;

#define TEMP_MAX_TEXT_LENGTH 1024

CGensRanking::CGensRanking() : m_fBooleanSize(0.8f)
{
    Init();
}

CGensRanking::~CGensRanking()
{
    if (m_pTextBox)
        m_pTextBox->Release();

    SAFE_DELETE(m_pTextBox);
    Destroy();
}

void CGensRanking::Init()
{
    m_nContribution = 0;
    memset(m_szRanking, 0, sizeof(m_szRanking));

    m_Pos.x = 0;
    m_Pos.y = 0;

    memset(m_szGensTeam, 0, sizeof(m_szGensTeam));

    m_byGensInfluence = GENSTYPE_NONE;
    m_ptRenderMarkPos.x = 0;
    m_ptRenderMarkPos.y = 0;

    m_nNextContribution = 0;

    // CutStr() copies characters without appending a terminator to each title row.
    memset(m_szTitleName, 0, sizeof(m_szTitleName));
    SetTitleName();
}

void CGensRanking::Destroy()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool CGensRanking::Create(CManager* pNewUIMng, int x, int y)
{
    if (pNewUIMng == NULL)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_GENSRANKING, this);

    m_pTextBox = new CTextBox();
    m_pTextBox->Create(x, y, 200, 110);

    SetPos(x, y);
    BuildRmlUi();

    Show(false);

    return true;
}

void CGensRanking::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
    if (m_pTextBox)
        m_pTextBox->SetPos(x + 20, y + 280, 150, 110);
}

bool CGensRanking::Render()
{
    // Nothing native left: the frame, the mark, the texts, the rewards text box and its scroll
    // bar are RmlUi. Kept because CObject requires the override.
    return true;
}

bool CGensRanking::Update()
{
    // The exit button RmlUi reported (the original's BtnProcess()).
    const bool exit = m_PendingExit;
    m_PendingExit = false;
    if (exit && IsVisible())
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GENSRANKING);

    if (!IsVisible())
    {
        SyncRmlModel();
        return true;
    }

    SyncRmlModel();
    return true;
}

bool CGensRanking::UpdateMouseEvent()
{
    if (!g_pNewUISystem->IsVisible(INTERFACE_GENSRANKING))
        return true;

    if (UI::RmlBridge::IsPointerOver(m_RmlView.Document()))
    {
        // The pane consumes its wheel event; keep the same notch from reaching the camera.
        MouseWheel = 0;
        if (mu::ui::window::IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }
        if (!mu::ui::window::IsNone(VK_LBUTTON))
            return false;
    }
    return true;
}

bool CGensRanking::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(INTERFACE_GENSRANKING))
    {
        if (mu::ui::window::IsPress(VK_ESCAPE))
        {
            g_pNewUISystem->Hide(INTERFACE_GENSRANKING);
            return false;
        }
    }
    return true;
}

void CGensRanking::OpenningProcess()
{
    SocketClient->ToGameServer()->SendGensRankingRequest();
}

void CGensRanking::ClosingProcess()
{
    //n/a
}

void CGensRanking::SetContribution(int _Contribution)
{
    if (_Contribution < 0)
    {
        m_nContribution = 0;
        return;
    }

    m_nContribution = _Contribution;
}

int CGensRanking::GetContribution()
{
    if (m_nContribution <= 0)
        return 0;

    return m_nContribution;
}

bool CGensRanking::SetRanking(int _Ranking)
{
    if (_Ranking <= 0)
    {
        mu_swprintf(m_szRanking, L"-");
        return false;
    }

    _itow(_Ranking, m_szRanking, 10);
    return true;
}

wchar_t* CGensRanking::GetRanking()
{
    return m_szRanking;
}

void CGensRanking::SetNextContribution(int _NextContribution)
{
    if (_NextContribution < 0)
    {
        m_nNextContribution = -1;
        return;
    }
    m_nNextContribution = _NextContribution;
}

int CGensRanking::GetNextContribution()
{
    return m_nNextContribution;
}

bool CGensRanking::SetGensInfo()
{
    m_byGensInfluence = (GENS_TYPE)Hero->m_byGensInfluence;

    if ((m_byGensInfluence & GENSTYPE_DUPRIAN) == GENSTYPE_DUPRIAN)
    {
        SetGensTeamName(I18N::Game::Duprian);
        return true;
    }
    else if ((m_byGensInfluence & GENSTYPE_BARNERT) == GENSTYPE_BARNERT)
    {
        SetGensTeamName(I18N::Game::Vanert);
        return true;
    }
    else
    {
        g_pSystemLogBox->AddText(I18N::Game::YouHaveNotJoinedAGens, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        return false;
    }
    return false;
}

bool CGensRanking::SetGensTeamName(const wchar_t* _pTeamName)
{
    if (_pTeamName)
    {
        wcscpy(m_szGensTeam, _pTeamName);
        return true;
    }
    return false;
}

wchar_t* CGensRanking::GetGensTeamName()
{
    return m_szGensTeam;
}

void CGensRanking::SetTitleName()
{
    wchar_t _szTempText[256] = { 0, };
    mu_swprintf(_szTempText, I18N::Game::GrandDukeDukeMarquisCountViscount);
    ::DivideStringByPixel(&m_szTitleName[0][0], TITLENAME_END, MAX_TITLELENGTH, _szTempText, 240, true, '#');
}

wchar_t* CGensRanking::GetTitleName(BYTE _index)
{
    if (TITLENAME_START <= _index && TITLENAME_END >= _index)
        return m_szTitleName[_index - 1];
    else
        return m_szTitleName[TITLENAME_END - 1];
}

void CGensRanking::RanderMark(float x, float y, GENS_TYPE gensType, BYTE rankIndex, IMAGE_AREA imageArea, float yOffset)
{
    if (gensType == GENSTYPE_NONE)
    {
        return;
    }

    int imageType = (gensType == GENSTYPE_DUPRIAN) ? IMAGE_NEWMARK_DUPRIAN : IMAGE_NEWMARK_BARNERT;

    float _width = GENSMARK_WIDTH;
    float _height = GENSMARK_HEIGHT;
    BITMAP_t* texture = Bitmaps.GetTexture(imageType);
    float imageWidth = texture->Width;
    float imageHeight = texture->Height;

    if (imageArea == MARK_BOOLEAN)
    {
        _width = GENSMARK_WIDTH * m_fBooleanSize;
        _height = GENSMARK_HEIGHT * m_fBooleanSize;
        imageWidth = texture->Width * m_fBooleanSize;
        imageHeight = texture->Height * m_fBooleanSize;
        y = (yOffset - y - _height) / 2 + y;
        x = (float)(x - _width + 1);
    }

    int imageIndex = GetImageIndex(rankIndex);
    float columnIndex = imageIndex % 5;
    float rowIndex = imageIndex / 5;
    float u = columnIndex * _width / imageWidth;
    float v = rowIndex * _height / imageHeight;

    RenderBitmap(imageType, x, y, _width, _height, u, v, _width / imageWidth, _height / imageHeight);
}

int CGensRanking::GetImageIndex(BYTE rankIndex)
{
    if (rankIndex < TITLENAME_START || rankIndex > TITLENAME_END)
    {
        rankIndex = TITLENAME_END;
    }

    if (rankIndex > TITLENAME_END || rankIndex <= TITLENAME_NONE)
    {
        return -1;
    }

    return TITLENAME_END - rankIndex;
}
void CGensRanking::BindRmlModel(Rml::DataModelConstructor& c, GensRankingRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("mark_sprite", &model.markSprite);
    auto lineType = c.RegisterStruct<GensLine>();
    lineType.RegisterMember("text", &GensLine::text);
    lineType.RegisterMember("text_px", &GensLine::textPx);
    c.Bind("title", &model.title);
    c.Bind("gens_label", &model.gensLabel);
    c.Bind("gens_name", &model.gensName);
    c.Bind("level_label", &model.levelLabel);
    c.Bind("title_name", &model.titleName);
    c.Bind("rank_label", &model.rankLabel);
    c.Bind("rank_value", &model.rankValue);
    c.Bind("contrib_label", &model.contribLabel);
    c.Bind("contrib_value", &model.contribValue);
    c.Bind("desc_label", &model.descLabel);
    c.RegisterArray<std::vector<GensLine>>();
    c.Bind("promo_lines", &model.promoLines);
    c.Bind("desc_lines", &model.descLines);
    c.Bind("desc_line_step", &model.descLineStep);
    c.Bind("exit_tooltip", &model.exitTooltip);
    c.BindEventCallback("gens_ranking_exit", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingExit = true; });
}

void CGensRanking::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CGensRanking::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 4.2: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    SyncContent();
}

void CGensRanking::SyncContent()
{

    // One of the window's own lines: the document places it, so only what it says and the size the
    // native renderer would have shrunk it to for its box travel through the model.
    auto line = [&](const wchar_t* text, bool boldFont, float boxWidth) -> GensLine
    {
        if (text == nullptr || text[0] == L'\0')
            return {};
        g_pRenderText->SetFont(boldFont ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = boldFont ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        return {StringUtils::WideToNarrow(text),
                UI::RmlBridge::NativeTextPxInBox(role, static_cast<float>(measured), boxWidth)};
    };

    // The original's RenderTexts().
    wchar_t szText[TEMP_MAX_TEXT_LENGTH] = {};
    GensLine title = line(I18N::Game::GensInfoWindow, true, GENSRANKING_WIDTH);
    GensLine gensLabel = line(I18N::Game::Gens, true, GENSRANKING_WIDTH);
    GensLine gensName = line(GetGensTeamName(), false, GENSRANKING_WIDTH);
    GensLine levelLabel = line(I18N::Game::Level3095, false, GENSRANKING_WIDTH);
    // Past the leading space DivideStringByPixel() inserted.
    const wchar_t* titleText = GetTitleName(Hero->GensRanking);
    GensLine titleName = line(titleText[0] != L'\0' ? titleText + 1 : titleText, false, GENSRANKING_WIDTH - 16);
    GensLine rankLabel = line(I18N::Game::GensRanking, true, 74.f);
    mu_swprintf(szText, I18N::Game::S, GetRanking());
    GensLine rankValue = line(szText, false, GENSRANKING_WIDTH - 20);
    GensLine contribLabel = line(I18N::Game::GainContribution, true, 74.f);
    mu_swprintf(szText, L"%d", GetContribution());
    GensLine contribValue = line(szText, false, GENSRANKING_WIDTH - 20);
    GensLine descLabel = line(I18N::Game::GensDescription, true, 58.f);

    std::vector<GensLine> promoLines;
    if (GetNextContribution() > 0)
    {
        // The original cut into an uninitialised buffer: CutStr() writes no terminator, so a
        // line could keep garbage after it (the new client showed a stray glyph and lost the
        // first line). Zeroed here, as the original's stack happened to be.
        wchar_t tempText[TEMP_MAX_TEXT_LENGTH] = {};
        wchar_t lines[NUM_LINE_CMB][MAX_TEXT_LENGTH] = {};
        mu_swprintf(tempText, I18N::Game::TheAmountOfContributionNeededForPromotionToTheNextRankIsD,
                    GetNextContribution());
        g_pRenderText->SetFont(g_hFont);
        const int lineCount =
            ::DivideStringByPixel(&lines[0][0], NUM_LINE_CMB, MAX_TEXT_LENGTH, tempText, 140, true, '#');
        for (int j = 0; j < lineCount; ++j)
            promoLines.push_back(line(lines[j], false, 180.f));
    }

    // Keep CTextBox's original wrapping and measured row pitch. RmlUi scrolls all rows.
    std::vector<GensLine> descLines;
    float descLineStep = 0.f;
    if (m_pTextBox)
    {
        m_pTextBox->ClearText();
        m_pTextBox->AddText(I18N::Game::GensRankingRewardsAreGivenOut);
        m_pTextBox->AddText(I18N::Game::GensRankingRewardsCanBeClaimed);
        g_pRenderText->SetFont(g_hFont);
        descLineStep = static_cast<float>(g_pRenderText->MeasureText(L"A", 1).cy + 2);
        descLines.reserve(m_pTextBox->GetMaxLine());
        for (int i = 0; i < m_pTextBox->GetMaxLine(); ++i)
        {
            const std::wstring text = m_pTextBox->GetLineText(i);
            descLines.push_back(line(text.c_str(), false, 150.f));
        }
    }

    GensRankingRmlModel& model = m_RmlView.GetModel();
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::title, "title", std::move(title));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::gensLabel, "gens_label", std::move(gensLabel));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::gensName, "gens_name", std::move(gensName));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::levelLabel, "level_label", std::move(levelLabel));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::titleName, "title_name", std::move(titleName));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::rankLabel, "rank_label", std::move(rankLabel));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::rankValue, "rank_value", std::move(rankValue));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::contribLabel, "contrib_label", std::move(contribLabel));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::contribValue, "contrib_value", std::move(contribValue));
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::descLabel, "desc_label", std::move(descLabel));
    if (model.promoLines != promoLines)
    {
        model.promoLines = std::move(promoLines);
        m_RmlView.MarkDirty("promo_lines");
    }
    if (model.descLines != descLines)
    {
        model.descLines = std::move(descLines);
        m_RmlView.MarkDirty("desc_lines");
    }
    SyncField(m_RmlView.Binder(), &GensRankingRmlModel::descLineStep, "desc_line_step", descLineStep);

    // The original's RenderMark(): the rank's 50 x 69 cell of the family's mark sheet.
    Rml::String markSprite;
    const int cell = GetImageIndex(Hero->GensRanking);
    if (m_byGensInfluence == GENSTYPE_DUPRIAN && cell >= 0)
        markSprite = "gens-d-" + std::to_string(cell);
    else if (m_byGensInfluence == GENSTYPE_BARNERT && cell >= 0)
        markSprite = "gens-v-" + std::to_string(cell);
    if (model.markSprite != markSprite)
    {
        model.markSprite = markSprite;
        m_RmlView.MarkDirty("mark_sprite");
    }

    const Rml::String exitTooltip = StringUtils::WideToNarrow(I18N::Game::Close388);
    if (model.exitTooltip != exitTooltip)
    {
        model.exitTooltip = exitTooltip;
        m_RmlView.MarkDirty("exit_tooltip");
    }
}
#endif //PBG_ADD_GENSRANKING
