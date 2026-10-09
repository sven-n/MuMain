
#include "stdafx.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/Events/DoppelGangerFrame.h"
#include "UI/Core/WindowSystem.h"
#include "I18N/All.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cstdio>

using namespace SEASON3B;
using namespace mu::ui::window;

CDoppelGangerFrame::CDoppelGangerFrame()
{
    m_pNewUIMng = NULL;
    m_iEnteredMonsters = 0;
    m_iMaxMonsters = 0;
    m_fMonsterGauge = 0.0f;
    m_fMonsterGaugeRcvd = 0.0f;
    m_iTime = 1800;
    m_bStopTimer = FALSE;
    m_bIceWalkerEnabled = FALSE;
    m_fIceWalkerPositionRcvd = 0.0f;
    m_fIceWalkerPosition = 0.0f;
}

CDoppelGangerFrame::~CDoppelGangerFrame()
{
    Release();
}

bool CDoppelGangerFrame::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DOPPELGANGER_FRAME, this);

    BuildRmlUi();

    Show(false);

    return true;
}

void CDoppelGangerFrame::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

bool CDoppelGangerFrame::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;
    return true;
}

bool CDoppelGangerFrame::UpdateKeyEvent()
{
    return true;
}

bool CDoppelGangerFrame::Update()
{
    SyncView();

    return true;
}

bool CDoppelGangerFrame::Render()
{
    // Nothing native left: the frame, the texts, the gauge and the markers are RmlUi (SyncView()).
    // Kept because CObject requires the override.
    return true;
}

namespace
{
// One step of the original's per-frame approach of `value` towards `target`, 0.01 at a time.
void Approach(float& value, float target)
{
    const float speed = 0.01f;
    if (value + speed <= target)
        value += speed;
    else if (value - speed >= target)
        value -= speed;
    else
        value = target;
}

// A gauge piece: the right-aligned `fill` of Double_bar (165 texels of it) over 167 units from x 59.
DoppelGangerFrameBarEntry BarPiece(const char* src, float fill)
{
    char rect[64];
    std::snprintf(rect, sizeof(rect), "%g 0 %g 6", 165.f * (1.0f - fill), 165.f * fill);
    return {src, rect, 59 + 167.f * (1.0f - fill), 167.f * fill};
}
} // namespace

void CDoppelGangerFrame::BindRmlModel(Rml::DataModelConstructor& c, DoppelGangerFrameRmlModel& model)
{
    auto text = c.RegisterStruct<DoppelGangerFrameTextEntry>();
    text.RegisterMember("text", &DoppelGangerFrameTextEntry::text);
    text.RegisterMember("text_px", &DoppelGangerFrameTextEntry::textPx);
    c.Bind("passed_line", &model.passedLine);
    c.Bind("passed_state", &model.passedState);
    c.Bind("time_label", &model.timeLabel);
    c.Bind("time_line", &model.timeLine);
    auto bar = c.RegisterStruct<DoppelGangerFrameBarEntry>();
    bar.RegisterMember("src", &DoppelGangerFrameBarEntry::src);
    bar.RegisterMember("rect", &DoppelGangerFrameBarEntry::rect);
    bar.RegisterMember("left", &DoppelGangerFrameBarEntry::left);
    bar.RegisterMember("width", &DoppelGangerFrameBarEntry::width);
    c.RegisterArray<std::vector<DoppelGangerFrameBarEntry>>();
    c.Bind("bars", &model.bars);
    c.Bind("ice_walker_visible", &model.iceWalkerVisible);
    c.Bind("ice_walker_left", &model.iceWalkerLeft);
    auto marker = c.RegisterStruct<DoppelGangerFrameMarkerEntry>();
    marker.RegisterMember("left", &DoppelGangerFrameMarkerEntry::left);
    marker.RegisterMember("hero", &DoppelGangerFrameMarkerEntry::hero);
    c.RegisterArray<std::vector<DoppelGangerFrameMarkerEntry>>();
    c.Bind("markers", &model.markers);
}

void CDoppelGangerFrame::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CDoppelGangerFrame::StepGauges()
{
    // The original's Render() moved the monster gauge, the Ice Walker and every party marker 0.01
    // towards the received position each frame it drew.
    Approach(m_fMonsterGauge, m_fMonsterGaugeRcvd);
    if (m_bIceWalkerEnabled == TRUE)
        Approach(m_fIceWalkerPosition, m_fIceWalkerPositionRcvd);
    for (auto& [index, position] : m_PartyPositionMap)
    {
        if (position.m_fPositionRcvd != -1)
            Approach(position.m_fPosition, position.m_fPositionRcvd);
    }
}

void CDoppelGangerFrame::SyncView()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibilityBehind(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    StepGauges();


    // The original's texts: the monsters that passed (orange, red-orange after one, red after two),
    // "Time left" and the time, each centred on 110 units and shrunk to them; the themes place
    // and colour them.
    auto textEntry = [&](const wchar_t* text, bool big)
    {
        const auto role = big ? UI::Scaling::FontRole::Big : UI::Scaling::FontRole::Normal;
        g_pRenderText->SetFont(big ? g_hFontBig : g_hFont);
        const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        return DoppelGangerFrameTextEntry{StringUtils::WideToNarrow(text),
                                          UI::RmlBridge::NativeTextPxInBox(role, static_cast<float>(width), 110.f)};
    };
    const char* passedState = "none";
    if (m_iEnteredMonsters == 1)
        passedState = "one";
    else if (m_iEnteredMonsters >= 2)
        passedState = "several";
    wchar_t szText[256] = {};
    mu_swprintf(szText, I18N::Game::MonstersPassedDD, m_iEnteredMonsters, m_iMaxMonsters);
    SyncField(m_RmlView.Binder(), &DoppelGangerFrameRmlModel::passedLine, "passed_line", textEntry(szText, false));
    SyncField(m_RmlView.Binder(), &DoppelGangerFrameRmlModel::passedState, "passed_state", Rml::String(passedState));
    SyncField(m_RmlView.Binder(), &DoppelGangerFrameRmlModel::timeLabel, "time_label",
              textEntry(I18N::Game::TimeLeft, false));
    const int iMinute = m_iTime / 60;
    const int iSecond = m_bStopTimer == TRUE ? 0 : 99 - static_cast<int>(WorldTime) % 100;
    mu_swprintf(szText, L"%.2d:%.2d:%.2d", iMinute, m_iTime % 60, iSecond);
    SyncField(m_RmlView.Binder(), &DoppelGangerFrameRmlModel::timeLine, "time_line", textEntry(szText, true));
    DoppelGangerFrameRmlModel& model = m_RmlView.GetModel();

    // The gauge: yellow up to the first monster, then yellow under orange, then orange under red.
    const char* yellow = "../../../Double_bar(Y).jpg";
    const char* orangeBar = "../../../Double_bar(O).jpg";
    const char* red = "../../../Double_bar(R).jpg";
    std::vector<DoppelGangerFrameBarEntry> bars;
    if (m_iEnteredMonsters == 0)
    {
        bars.push_back(BarPiece(yellow, m_fMonsterGauge));
    }
    else if (m_iEnteredMonsters == 1)
    {
        bars.push_back(BarPiece(yellow, 1.0f));
        bars.push_back(BarPiece(orangeBar, m_fMonsterGauge));
    }
    else
    {
        bars.push_back(BarPiece(orangeBar, 1.0f));
        bars.push_back(BarPiece(red, m_fMonsterGauge));
    }
    const bool sameBars =
        model.bars.size() == bars.size() &&
        std::equal(model.bars.begin(), model.bars.end(), bars.begin(),
                   [](const DoppelGangerFrameBarEntry& a, const DoppelGangerFrameBarEntry& b)
                   { return a.src == b.src && a.rect == b.rect && a.left == b.left && a.width == b.width; });
    if (!sameBars)
    {
        model.bars = std::move(bars);
        m_RmlView.MarkDirty("bars");
    }

    SyncField(m_RmlView.Binder(), &DoppelGangerFrameRmlModel::iceWalkerVisible, "ice_walker_visible",
              m_bIceWalkerEnabled == TRUE);
    SyncField(m_RmlView.Binder(), &DoppelGangerFrameRmlModel::iceWalkerLeft, "ice_walker_left",
              59 - 6.5f + 167 * m_fIceWalkerPosition);

    std::vector<DoppelGangerFrameMarkerEntry> markers;
    for (const auto& [index, position] : m_PartyPositionMap)
    {
        if (position.m_fPositionRcvd == -1)
            continue;
        markers.push_back({59 - 4.5f + 167 * position.m_fPosition, index == Hero->Key});
    }
    const bool sameMarkers = model.markers.size() == markers.size() &&
                             std::equal(model.markers.begin(), model.markers.end(), markers.begin(),
                                        [](const DoppelGangerFrameMarkerEntry& a, const DoppelGangerFrameMarkerEntry& b)
                                        { return a.left == b.left && a.hero == b.hero; });
    if (!sameMarkers)
    {
        model.markers = std::move(markers);
        m_RmlView.MarkDirty("markers");
    }
}

bool CDoppelGangerFrame::BtnProcess()
{
    return false;
}

float CDoppelGangerFrame::GetLayerDepth()
{
    return 1.2f;
}

void CDoppelGangerFrame::OpenningProcess()
{
    m_iEnteredMonsters = 0;
    m_iMaxMonsters = 0;
    m_fMonsterGauge = 0.0f;
    m_fMonsterGaugeRcvd = 0.0f;
    m_iTime = 600;
    m_bStopTimer = FALSE;
    ResetPartyMemberInfo();
    m_fIceWalkerPositionRcvd = 0.0f;
    m_fIceWalkerPosition = 0.0f;
    m_bIceWalkerEnabled = FALSE;

    EnabledDoppelGangerEvent(TRUE);
}

void CDoppelGangerFrame::ClosingProcess()
{
    EnabledDoppelGangerEvent(FALSE);
}

void CDoppelGangerFrame::SetMonsterGauge(float fValue)
{
    m_fMonsterGaugeRcvd = fValue;
}

void CDoppelGangerFrame::SetRemainTime(int iSeconds)
{
    m_iTime = iSeconds;
}

void CDoppelGangerFrame::StopTimer(BOOL bFlag)
{
    m_bStopTimer = bFlag;
}

void CDoppelGangerFrame::ResetPartyMemberInfo()
{
    m_PartyPositionMap.clear();
}

void CDoppelGangerFrame::SetPartyMemberRcvd()
{
    for (std::map<WORD, PARTY_POSITION>::iterator iter = m_PartyPositionMap.begin(); iter != m_PartyPositionMap.end(); ++iter)
    {
        iter->second.m_fPositionRcvd = -1.0f;
    }
}

void CDoppelGangerFrame::SetPartyMemberInfo(WORD wIndex, float fPosition)
{
    auto iter = m_PartyPositionMap.find(wIndex);
    if (iter == m_PartyPositionMap.end())
    {
        PARTY_POSITION party_pos;
        party_pos.m_fPosition = 0;
        party_pos.m_fPositionRcvd = fPosition;
        m_PartyPositionMap.insert(std::pair<WORD, PARTY_POSITION>(wIndex, party_pos));
    }
    else
    {
        iter->second.m_fPositionRcvd = fPosition;
    }
}

void CDoppelGangerFrame::SetIceWalkerMap(BOOL bEnable, float fPosition)
{
    if (bEnable == TRUE)
    {
        if (m_bIceWalkerEnabled == TRUE)
        {
            m_fIceWalkerPositionRcvd = fPosition;
        }
        else
        {
            m_fIceWalkerPositionRcvd = fPosition;
            m_fIceWalkerPosition = fPosition;
        }
        m_bIceWalkerEnabled = TRUE;
    }
    else
    {
        m_bIceWalkerEnabled = FALSE;
        m_fIceWalkerPositionRcvd = 0.0f;
    }
}
