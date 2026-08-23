//*****************************************************************************
// File: OptionWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/OptionWin.h"
#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Scenes/SceneCore.h"
#include "Audio/DSPlaySound.h"
#include "UI/Legacy/UIControls.h"
#include "UI/NewUI/NewUISystem.h"
#include "I18N/All.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDraggable.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <algorithm>
#include <cmath>

#define	OW_BTN_GAP		25
#define	OW_SLD_GAP		48

namespace
{
    // Matches the legacy CSlider art's real geometry (98px track - 13px thumb, see
    // COptionWin::Create's iiThumb/iiBack) -- the travel distance a hand-rolled RmlUi thumb drags
    // across.
    constexpr float kSliderTravelPx = 85.0f;
}




COptionWin::COptionWin()
{
}

COptionWin::~COptionWin()
{
}

void COptionWin::Create()
{
    CInput rInput = CInput::Instance();
    // RmlUi migration, Batch 2: -2 (was the default -1) -- see this class's header comment.
    CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight(), -2);
    SetMovable(false);

    SImgInfo aiiBack[WE_BG_MAX] =
    {
        { BITMAP_SYS_WIN, 0, 0, 128, 128 },
        { BITMAP_OPTION_WIN, 0, 0, 213, 65 },
        { BITMAP_SYS_WIN + 2, 0, 0, 213, 43 },
        { BITMAP_SYS_WIN + 3, 0, 0, 5, 8 },
        { BITMAP_SYS_WIN + 4, 0, 0, 5, 8 }
    };
    m_winBack.Create(aiiBack, 1, 30);
    m_winBack.SetLine(30);

    for (int i = 0; i <= OW_BTN_SLIDE_HELP; ++i)
    {
        m_aBtn[i].Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);
        CWin::RegisterButton(&m_aBtn[i]);

        // Same double-toggle risk as CLoginWin's checkboxes (see LoginWin.cpp's SetEnable
        // comment) -- these 3 are checkbox-style (HasCheckVisuals()) CButtons too, so their own
        // input polling would fight RmlToggleAutoAttack/WhisperAlarm/SlideHelp's SetCheck() flip.
        m_aBtn[i].SetEnable(false);
    }

    DWORD adwBtnClr[4] = { CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0 };
    m_aBtn[OW_BTN_CLOSE].Create(108, 30, BITMAP_TEXT_BTN, 4, 2, 1);
    m_aBtn[OW_BTN_CLOSE].SetText(I18N::Game::Close388, adwBtnClr);
    CWin::RegisterButton(&m_aBtn[OW_BTN_CLOSE]);

    SImgInfo iiThumb = { BITMAP_SLIDER, 0, 0, 13, 13 };
    SImgInfo iiBack = { BITMAP_SLIDER + 2, 0, 0, 98, 13 };
    SImgInfo iiGauge = { BITMAP_SLIDER + 1, 0, 0, 4, 7 };
    RECT rcGauge = { 3, 3, 95, 10 };

    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].Create(&iiThumb, &iiBack, &iiGauge, &rcGauge);

    m_aSlider[OW_SLD_EFFECT_VOL].SetSlideRange(9);
    m_aSlider[OW_SLD_RENDER_LV].SetSlideRange(4);

    // RmlUi migration, Batch 2 -- see this class's header comment. Guarded the same way
    // CLoginWin::Create() is (CUIMng::RepositionSceneUI() re-runs Create() on resolution change).
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "option",
            [this](Rml::DataModelConstructor& c, OptionRmlModel& model)
            {
                c.Bind("auto_attack_checked", &model.autoAttackChecked);
                c.Bind("whisper_alarm_checked", &model.whisperAlarmChecked);
                c.Bind("slide_help_checked", &model.slideHelpChecked);
                c.Bind("volume_thumb_left", &model.volumeThumbLeft);
                c.Bind("render_thumb_left", &model.renderThumbLeft);
                c.Bind("title_label", &model.titleLabel);
                c.Bind("auto_attack_label", &model.autoAttackLabel);
                c.Bind("whisper_alarm_label", &model.whisperAlarmLabel);
                c.Bind("slide_help_label", &model.slideHelpLabel);
                c.Bind("close_label", &model.closeLabel);
                c.Bind("volume_label", &model.volumeLabel);
                c.Bind("render_label", &model.renderLabel);
                c.Bind("volume_value_text", &model.volumeValueText);
                c.Bind("render_value_text", &model.renderValueText);

                c.BindEventCallback("option_toggle_auto_attack",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleAutoAttack(); });
                c.BindEventCallback("option_toggle_whisper_alarm",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleWhisperAlarm(); });
                c.BindEventCallback("option_toggle_slide_help",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleSlideHelp(); });
                c.BindEventCallback("option_close_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickClose(); });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/option.rml");

        // First production use of UI::RmlBridge::MakeDraggable -- previously built and unit-proven
        // only against a throwaway handle (see its own header comment). Each thumb is its own
        // handle and panel (there's nothing else to move); onMove clamps to the track's real
        // travel distance and snaps to the slider's discrete step count, then pushes the
        // resulting position straight to g_pOption the same way the legacy CSlider path does.
        if (m_pRmlDoc)
        {
            if (Rml::Element* thumb = m_pRmlDoc->GetElementById("volume_thumb"))
                UI::RmlBridge::MakeDraggable(thumb, thumb, [this](float newLeft, float) { OnSliderThumbDragged(OW_SLD_EFFECT_VOL, newLeft); });
            if (Rml::Element* thumb = m_pRmlDoc->GetElementById("render_thumb"))
                UI::RmlBridge::MakeDraggable(thumb, thumb, [this](float newLeft, float) { OnSliderThumbDragged(OW_SLD_RENDER_LV, newLeft); });
        }
    }

    SetPosition((rInput.GetScreenWidth() - m_winBack.GetWidth()) / 2,
        (rInput.GetScreenHeight() - m_winBack.GetHeight()) / 2);

    UpdateDisplay();
}

void COptionWin::PreRelease()
{
    m_winBack.Release();
    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].Release();

    // See CLoginMainWin::PreRelease()'s identical comment -- CUIMng::RemoveWinList() Release()s
    // every window on every scene transition, and CWin's own Release() has no knowledge of
    // m_pRmlDoc, so without this it can keep rendering into whatever scene comes next if this
    // window happened to be open at the moment of transition.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void COptionWin::SetPosition(int nXCoord, int nYCoord)
{
    m_winBack.SetPosition(nXCoord, nYCoord);

    int nBtnPosX = m_winBack.GetXPos() + 52;
    int nBtnGap = OW_BTN_GAP + m_aBtn[0].GetHeight();
    int nBtnPosBaseTop = m_winBack.GetYPos() + 52;
    for (int i = 0; i <= OW_BTN_SLIDE_HELP; ++i)
        m_aBtn[i].SetPosition(nBtnPosX, nBtnPosBaseTop + i * nBtnGap);

    m_aBtn[OW_BTN_CLOSE].SetPosition(m_winBack.GetXPos() + (m_winBack.GetWidth() - m_aBtn[OW_BTN_CLOSE].GetWidth()) / 2, m_winBack.GetYPos() + 301);

    //	int nSldPosX = m_winBack.GetXPos()
    //		+ (m_winBack.GetWidth() - m_aSlider[0].GetWidth()) / 2;
    int nSldGap = OW_SLD_GAP + m_aSlider[0].GetHeight();
    int nSldPosBaseTop = m_aBtn[OW_BTN_SLIDE_HELP].GetYPos()
        + m_aBtn[0].GetHeight() + OW_SLD_GAP;
    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].SetPosition(nBtnPosX, nSldPosBaseTop + i * nSldGap);

    // RmlUi panel: positioned/sized to match m_winBack's real (screen-absolute) geometry, same
    // idiom as CSysMenuWin::SetPosition. Every child below stays at a fixed RCSS offset relative
    // to #panel (option.rcss) rather than being pushed from here -- unlike SysMenuWin's height,
    // OptionWin's layout is never scene-conditional (m_winBack.SetLine(30) is the only value ever
    // used), so there's nothing here that actually varies at runtime worth pushing per-child.
    if (m_pRmlDoc)
    {
        if (Rml::Element* panel = m_pRmlDoc->GetElementById("panel"))
        {
            panel->SetProperty("left", std::to_string(nXCoord) + "px");
            panel->SetProperty("top", std::to_string(nYCoord) + "px");
            panel->SetProperty("width", std::to_string(m_winBack.GetWidth()) + "px");
            panel->SetProperty("height", std::to_string(m_winBack.GetHeight()) + "px");
        }

        // #panel_middle's own inset/size is pure RCSS now -- see CSysMenuWin::SetPosition()'s
        // identical comment for why the C++ push this replaced was unnecessary.
    }
}

void COptionWin::Show(bool bShow)
{
    CWin::Show(bShow);

    m_winBack.Show(bShow);
    for (int i = 0; i < OW_BTN_MAX; ++i)
        m_aBtn[i].Show(bShow);
    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].Show(bShow);

    if (m_pRmlDoc)
    {
        if (bShow) { UpdateDisplay(); SyncRmlModel(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

void COptionWin::UpdateDisplay()
{
    m_aBtn[OW_BTN_AUTO_ATTACK].SetCheck(g_pOption->IsAutoAttack());
    m_aBtn[OW_BTN_WHISPER_ALARM].SetCheck(g_pOption->IsWhisperSound());
    m_aBtn[OW_BTN_SLIDE_HELP].SetCheck(g_pOption->IsSlideHelp());
    m_aSlider[OW_SLD_EFFECT_VOL].SetSlidePos(g_pOption->GetVolumeLevel());
    m_aSlider[OW_SLD_RENDER_LV].SetSlidePos(g_pOption->GetRenderLevel());

    // Mirror into the RmlUi model too -- this keeps the RmlUi thumbs/checkboxes correct even
    // though nothing calls UpdateDisplay() outside Create()/Show() today (this window is
    // currently unreachable, see this class's header comment); a future caller of UpdateDisplay()
    // to reflect an external g_pOption change gets the RmlUi side updated for free.
    if (m_pRmlDoc)
    {
        m_RmlBinder.GetModel().volumeThumbLeft = g_pOption->GetVolumeLevel() / 9.0f * kSliderTravelPx;
        m_RmlBinder.MarkDirty("volume_thumb_left");
        m_RmlBinder.GetModel().renderThumbLeft = g_pOption->GetRenderLevel() / 4.0f * kSliderTravelPx;
        m_RmlBinder.MarkDirty("render_thumb_left");
    }
}

void COptionWin::UpdateWhileActive(double dDeltaTick)
{
    for (int i = 0; i < OW_SLD_MAX; ++i)
        m_aSlider[i].Update(dDeltaTick);

    if (m_aBtn[OW_BTN_AUTO_ATTACK].IsClick() || m_bRmlAutoAttackClicked)
    {
        m_bRmlAutoAttackClicked = false;
        g_pOption->SetAutoAttack(m_aBtn[OW_BTN_AUTO_ATTACK].IsCheck());
        SyncRmlModel();
    }
    else if (m_aBtn[OW_BTN_WHISPER_ALARM].IsClick() || m_bRmlWhisperAlarmClicked)
    {
        m_bRmlWhisperAlarmClicked = false;
        g_pOption->SetWhisperSound(m_aBtn[OW_BTN_WHISPER_ALARM].IsCheck());
        SyncRmlModel();
    }
    else if (m_aBtn[OW_BTN_SLIDE_HELP].IsClick() || m_bRmlSlideHelpClicked)
    {
        m_bRmlSlideHelpClicked = false;
        g_pOption->SetSlideHelp(m_aBtn[OW_BTN_SLIDE_HELP].IsCheck());
        SyncRmlModel();
    }
    else if (m_aBtn[OW_BTN_CLOSE].IsClick() || m_bRmlCloseClicked)
    {
        m_bRmlCloseClicked = false;
        CUIMng::Instance().HideWin(this);
        CUIMng::Instance().SetSysMenuWinShow(false);
    }
    else if (m_aSlider[OW_SLD_EFFECT_VOL].GetState())
    {
        int nSlidePos = m_aSlider[OW_SLD_EFFECT_VOL].GetSlidePos();

        if (g_pOption->GetVolumeLevel() != nSlidePos)
        {
            g_pOption->SetVolumeLevel(nSlidePos);
            ::SetEffectVolumeLevel(g_pOption->GetVolumeLevel());
        }
    }
    else if (m_aSlider[OW_SLD_RENDER_LV].GetState())
    {
        int nSlidePos = m_aSlider[OW_SLD_RENDER_LV].GetSlidePos();
        if (g_pOption->GetRenderLevel() != nSlidePos)
        {
            g_pOption->SetRenderLevel(nSlidePos);
        }
    }
    else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
    {
        ::PlayBuffer(SOUND_CLICK01);
        CUIMng::Instance().HideWin(this);
        CUIMng::Instance().SetSysMenuWinShow(false);
    }
}

void COptionWin::RenderControls()
{
    // m_winBack/g_pRenderText/m_aSlider[i].Render() no longer draw -- RmlUi's #backdrop/#panel
    // own 100% of this window's visuals (see this class's header comment). Legacy widgets stay
    // registered/updated purely for redundant input detection, same as CSysMenuWin.
    SyncRmlModel();
}

void COptionWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto syncBool = [this](bool OptionRmlModel::* field, const char* boundName, bool value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncBool(&OptionRmlModel::autoAttackChecked, "auto_attack_checked", m_aBtn[OW_BTN_AUTO_ATTACK].IsCheck());
    syncBool(&OptionRmlModel::whisperAlarmChecked, "whisper_alarm_checked", m_aBtn[OW_BTN_WHISPER_ALARM].IsCheck());
    syncBool(&OptionRmlModel::slideHelpChecked, "slide_help_checked", m_aBtn[OW_BTN_SLIDE_HELP].IsCheck());

    auto syncLabel = [this](Rml::String OptionRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const std::string utf8 = StringUtils::WideToNarrow(text);
        if (m_RmlBinder.GetModel().*field != utf8)
        {
            m_RmlBinder.GetModel().*field = utf8;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncLabel(&OptionRmlModel::titleLabel, "title_label", I18N::Game::Option385);
    syncLabel(&OptionRmlModel::autoAttackLabel, "auto_attack_label", I18N::Game::AutomaticAttack);
    syncLabel(&OptionRmlModel::whisperAlarmLabel, "whisper_alarm_label", I18N::Game::BeepSoundForWhispering);
    syncLabel(&OptionRmlModel::slideHelpLabel, "slide_help_label", I18N::Game::SlideHelp);
    syncLabel(&OptionRmlModel::closeLabel, "close_label", I18N::Game::Close388);
    syncLabel(&OptionRmlModel::volumeLabel, "volume_label", I18N::Game::Volume);
    syncLabel(&OptionRmlModel::renderLabel, "render_label", I18N::Game::EffectLimitation);

    // Numeric display values -- render-level's is a display-only transform (raw levels 0-4 shown
    // as 5,7,9,11,13), matching the legacy anVal[OW_SLD_RENDER_LV] computation exactly.
    auto syncValueText = [this](Rml::String OptionRmlModel::* field, const char* boundName, int value)
    {
        wchar_t buf[8];
        ::_itow(value, buf, 10);
        const std::string utf8 = StringUtils::WideToNarrow(buf);
        if (m_RmlBinder.GetModel().*field != utf8)
        {
            m_RmlBinder.GetModel().*field = utf8;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncValueText(&OptionRmlModel::volumeValueText, "volume_value_text", g_pOption->GetVolumeLevel());
    syncValueText(&OptionRmlModel::renderValueText, "render_value_text", g_pOption->GetRenderLevel() * 2 + 5);
}

void COptionWin::OnSliderThumbDragged(int sliderIndex, float newLeftPx)
{
    const int steps = (sliderIndex == OW_SLD_EFFECT_VOL) ? 9 : 4;
    const float clamped = std::clamp(newLeftPx, 0.0f, kSliderTravelPx);
    const int pos = int(std::round(clamped / kSliderTravelPx * steps));
    const float snappedPx = pos / float(steps) * kSliderTravelPx;

    const char* thumbId = (sliderIndex == OW_SLD_EFFECT_VOL) ? "volume_thumb" : "render_thumb";
    if (m_pRmlDoc)
    {
        if (Rml::Element* thumb = m_pRmlDoc->GetElementById(thumbId))
        {
            thumb->SetProperty("left", std::to_string(snappedPx) + "px");
            // This is a horizontal-only track -- MakeDraggable tracks both axes (it has no
            // knowledge this handle is constrained), so undo its vertical tracking every tick
            // rather than letting the thumb drift with the cursor's Y movement.
            thumb->SetProperty("top", "0px");
        }
    }

    m_aSlider[sliderIndex].SetSlidePos(pos);   // keep the legacy CSlider mirrored, same as CButton precedent

    if (sliderIndex == OW_SLD_EFFECT_VOL)
    {
        if (g_pOption->GetVolumeLevel() != pos)
        {
            g_pOption->SetVolumeLevel(pos);
            ::SetEffectVolumeLevel(g_pOption->GetVolumeLevel());
        }
    }
    else
    {
        if (g_pOption->GetRenderLevel() != pos)
            g_pOption->SetRenderLevel(pos);
    }
    SyncRmlModel();
}

void COptionWin::RmlToggleAutoAttack()
{
    m_aBtn[OW_BTN_AUTO_ATTACK].SetCheck(!m_aBtn[OW_BTN_AUTO_ATTACK].IsCheck());
    m_bRmlAutoAttackClicked = true;
}

void COptionWin::RmlToggleWhisperAlarm()
{
    m_aBtn[OW_BTN_WHISPER_ALARM].SetCheck(!m_aBtn[OW_BTN_WHISPER_ALARM].IsCheck());
    m_bRmlWhisperAlarmClicked = true;
}

void COptionWin::RmlToggleSlideHelp()
{
    m_aBtn[OW_BTN_SLIDE_HELP].SetCheck(!m_aBtn[OW_BTN_SLIDE_HELP].IsCheck());
    m_bRmlSlideHelpClicked = true;
}