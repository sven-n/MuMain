#pragma once

#include "UI/HUD/MapNameRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

typedef std::map<int, std::wstring> ImgPathMap;

namespace UI::MapName
{
    inline constexpr float ImageWidth = 166.0f;
    inline constexpr float ImageHeight = 90.0f;

    inline float PhysicalLeft(int windowWidth)
    {
        return (static_cast<float>(windowWidth) - ImageWidth) * 0.5f;
    }
}

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
    ImgPathMap m_mapImgPath;
    short m_nOldWorld;
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
    void InitImgPathMap();

    // The banner in RmlUi (map_name.rml): background context, behind every other document (the
    // original drew it before every window). Render() fills it; the native drawing is the fallback
    // when RmlUi is not available.
    void BuildRmlUi();
    void SyncView();
    void RenderNative();

    void BindRmlModel(Rml::DataModelConstructor& c, UI::MapName::MapNameRmlModel& model);
    UI::RmlBridge::ThemedView<UI::MapName::MapNameRmlModel> m_RmlView{"map_name",
        [this](Rml::DataModelConstructor& c, UI::MapName::MapNameRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/map_name.rml", UI::RmlBridge::BackgroundOrMainContext}}};
};
