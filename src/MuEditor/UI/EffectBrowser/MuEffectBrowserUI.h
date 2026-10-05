#pragma once

#ifdef _EDITOR

#include "EffectBrowserDetails.h"
#include "EffectBrowserList.h"
#include "EffectBrowserModel.h"

#include <array>
#include <optional>
#include <string>

// The effect browser (read only, docs/effect-data.md): a tab per kind with
// the list of its types, and the details of the selected type.
class CMuEffectBrowserUI
{
public:
    static CMuEffectBrowserUI& GetInstance();

    void Render(bool* open);

    // Called between frames, before the renderer's frame starts: the preview
    // releases its texture only then.
    void BeforeFrame()
    {
        m_details.BeforeFrame();
    }

private:
    CMuEffectBrowserUI() = default;

    // The catalogue is loaded on the loading screen; the browser builds its
    // list once it is.
    bool BuildWhenLoaded();
    // What the slots hold changes with the map.
    void RefreshAssetsWhenMapChanges();
    void RefreshAssets();
    void NameMap();
    void RenderMapLine();
    void RenderTabs();
    void RenderKind(Data::Effects::EffectKind kind);
    void Show(MuEditor::Effects::EffectTypeRef type);

    MuEditor::Effects::EffectBrowserModel m_model;
    CEffectBrowserList m_list;
    CEffectBrowserDetails m_details;
    // The selected type of each kind, -1 for none.
    std::array<int, Data::Effects::EffectKindCount> m_selected = {-1, -1, -1, -1};
    // The tab to select at the next frame.
    std::optional<Data::Effects::EffectKind> m_tabToSelect;
    // The map the assets were read on, and its name in the language of
    // m_mapNameLocale.
    std::optional<int> m_assetWorld;
    std::string m_mapName;
    const char* m_mapNameLocale = nullptr;
};

#define g_MuEffectBrowserUI CMuEffectBrowserUI::GetInstance()

#endif // _EDITOR
