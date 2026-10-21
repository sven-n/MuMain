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

    void Init();
    void ShowMapName();
    void Update();
    void Render();

protected:
    // RmlUi owns this presentation, including recovery after a missing document.
    // There is no native/per-map bitmap fallback.
    void BuildRmlUi();
    void SyncView();
    void RefreshName();
    static void OnLocaleChanged(void* context) noexcept;

    void BindRmlModel(Rml::DataModelConstructor& c, UI::MapName::MapNameRmlModel& model);
    UI::RmlBridge::ThemedView<UI::MapName::MapNameRmlModel> m_RmlView{"map_name",
        [this](Rml::DataModelConstructor& c, UI::MapName::MapNameRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/map_name.rml"}}};
};
