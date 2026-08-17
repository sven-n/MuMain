//*****************************************************************************
// File: OptionWin.h
//*****************************************************************************
#pragma once

#include "UI/Widgets/WinEx.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/Slider.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#define	OW_BTN_AUTO_ATTACK		0
#define	OW_BTN_WHISPER_ALARM	1
#define	OW_BTN_SLIDE_HELP		2
#define	OW_BTN_CLOSE			3
#define	OW_BTN_MAX				4

#define OW_SLD_EFFECT_VOL		0
#define OW_SLD_RENDER_LV		1
#define OW_SLD_MAX				2

namespace Rml { class ElementDocument; }

// RmlUi migration, Batch 2. NOTE: this window is currently unreachable in live play -- SysMenuWin's
// Option button opens a different, already-modern window (SEASON3B::CNewUIOptionWindow via
// g_pNewUISystem->Show(INTERFACE_OPTION)) instead of this one, confirmed by exhaustive grep (see
// the migration plan's "Key finding" section). This migration is deliberately faithful anyway
// (the checkboxes/sliders genuinely work) but does NOT rewire that routing -- retiring one of the
// two option-window implementations is a separate product decision left for later.
//
// CWin::Create() now passes nTexID=-2 (was the default -1) -- see CSysMenuWin's identical header
// comment for the #backdrop/m_winBack-geometry-only rationale, which applies here unchanged.
// The 2 sliders are UI::RmlBridge::MakeDraggable's first production use (see OptionWin.cpp).
class COptionWin : public CWin
{
protected:
    CWinEx		m_winBack;
    CButton		m_aBtn[OW_BTN_MAX];
    CSlider		m_aSlider[OW_SLD_MAX];

public:
    COptionWin();
    virtual ~COptionWin();

    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    void UpdateDisplay();

    void RmlToggleAutoAttack();
    void RmlToggleWhisperAlarm();
    void RmlToggleSlideHelp();
    void RmlClickClose() { m_bRmlCloseClicked = true; }

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();

private:
    struct OptionRmlModel
    {
        bool autoAttackChecked = false;
        bool whisperAlarmChecked = false;
        bool slideHelpChecked = false;
        float volumeThumbLeft = 0.0f;
        float renderThumbLeft = 0.0f;
        Rml::String titleLabel;
        Rml::String autoAttackLabel;
        Rml::String whisperAlarmLabel;
        Rml::String slideHelpLabel;
        Rml::String closeLabel;
        Rml::String volumeLabel;
        Rml::String renderLabel;
        Rml::String volumeValueText;
        Rml::String renderValueText;
    };
    RmlModelBinder<OptionRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    bool m_bRmlAutoAttackClicked = false;
    bool m_bRmlWhisperAlarmClicked = false;
    bool m_bRmlSlideHelpClicked = false;
    bool m_bRmlCloseClicked = false;

    void SyncRmlModel();
    // Shared by both sliders' UI::RmlBridge::MakeDraggable onMove callback -- clamps to the
    // track's real travel distance and snaps to the slider's discrete step count, mirroring
    // CSlider::Update's own drag-to-discrete-position math (Slider.cpp) for a hand-rolled RmlUi
    // element instead of the legacy widget.
    void OnSliderThumbDragged(int sliderIndex, float newLeftPx);
};
