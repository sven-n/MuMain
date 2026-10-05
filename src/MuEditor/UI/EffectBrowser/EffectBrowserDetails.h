#pragma once

#ifdef _EDITOR

#include "EffectBrowserModel.h"
#include "EffectPreviewView.h"

#include <array>
#include <optional>
#include <string>

// The details of the type selected in the effect browser: name, code and
// number, a preview of what its slot holds, the types of other kinds with
// that number, what its slot holds, its stages, its creation values and the
// data that names it.
class CEffectBrowserDetails
{
public:
    // Returns a type clicked in a list of other types, to show it.
    std::optional<MuEditor::Effects::EffectTypeRef> Render(const MuEditor::Effects::EffectBrowserModel& model,
                                                           std::optional<MuEditor::Effects::EffectTypeRef> selected);

    // Called between frames.
    void BeforeFrame()
    {
        m_preview.BeforeFrame();
    }

private:
    void Refresh(const MuEditor::Effects::EffectBrowserModel& model, MuEditor::Effects::EffectTypeRef selected);
    void RenderIdentity(const MuEditor::Effects::EffectBrowserRow& row, Data::Effects::EffectKind kind);
    std::optional<MuEditor::Effects::EffectTypeRef>
    RenderSameNumber(const MuEditor::Effects::EffectBrowserModel& model);
    void RenderAsset(const MuEditor::Effects::EffectBrowserRow& row);
    void RenderMapObject(const MuEditor::Effects::EffectBrowserRow& row);
    std::optional<MuEditor::Effects::EffectTypeRef> RenderStages(const MuEditor::Effects::EffectBrowserModel& model,
                                                                 const MuEditor::Effects::EffectBrowserRow& row,
                                                                 Data::Effects::EffectKind kind);
    void RenderCreationValues(Data::Effects::EffectKind kind);
    void RenderUsedBy(Data::Effects::EffectKind kind);

    // Built when the selection, the assets or the language change, not every
    // frame.
    std::optional<MuEditor::Effects::EffectTypeRef> m_ref;
    int m_assetGeneration = -1;
    const char* m_locale = nullptr;
    MuEditor::Effects::EffectBrowserDetails m_details;
    // What loaded the slot's asset, and the map a map-object effect belongs
    // to (empty for the others).
    std::string m_assetOrigin;
    std::string m_homeMap;
    // The SubTypes of each variant column, written once ("1, 2").
    std::vector<std::string> m_variantSubTypes;
    // The catalogue file of each kind, written once.
    std::array<std::string, Data::Effects::EffectKindCount> m_files;
    CEffectPreviewView m_preview;
};

#endif // _EDITOR
