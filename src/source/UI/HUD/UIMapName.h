#pragma once

#include "UI/HUD/MapNameRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

class CUIMapName
{
    enum SHOW_STATE
    {
        HIDE,
        FADEIN,
        SHOW,
        FADEOUT
    };

protected:
    SHOW_STATE m_eState;
    DWORD m_dwOldTime;
    DWORD m_dwDeltaTickSum;
    float m_fAlpha;
#ifdef ASG_ADD_GENS_SYSTEM
    bool m_bStrife;
#endif

public:
    CUIMapName();
    virtual ~CUIMapName();
    // Registered with the locale observers by address: a copy or a move would leave the list
    // pointing at the wrong object.
    CUIMapName(const CUIMapName&) = delete;
    CUIMapName& operator=(const CUIMapName&) = delete;

    void Init();
    void ShowMapName();
    void Update();
    void Render();

protected:
    // An unavailable RmlUi document hides the banner; there is no native/bitmap fallback.
    void BuildRmlUi();
    void SyncView();
    void RefreshName();
    void RefreshIcon();
    static void OnLocaleChanged(void* context) noexcept;

    void BindRmlModel(Rml::DataModelConstructor& c, UI::MapName::MapNameRmlModel& model);
    UI::RmlBridge::ThemedView<UI::MapName::MapNameRmlModel> m_RmlView{
        "map_name",
        [this](Rml::DataModelConstructor& c, UI::MapName::MapNameRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/map_name.rml"}}};
};
