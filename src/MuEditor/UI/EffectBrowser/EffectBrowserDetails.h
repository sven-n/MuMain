#pragma once

#ifdef _EDITOR

#include "EffectBrowserModel.h"

#include <array>
#include <optional>
#include <string>

// The details of the type selected in the effect browser: name, code and
// number, the types of other kinds with that number, what its slot holds, its
// stages, its creation values and the data that names it.
class CEffectBrowserDetails
{
public:
    // Returns a type clicked in a list of other types, to show it.
    std::optional<MuEditor::Effects::EffectTypeRef> Render(const MuEditor::Effects::EffectBrowserModel& model,
                                                           std::optional<MuEditor::Effects::EffectTypeRef> selected);

private:
    void Refresh(const MuEditor::Effects::EffectBrowserModel& model, MuEditor::Effects::EffectTypeRef selected);
    void RenderIdentity(const MuEditor::Effects::EffectBrowserRow& row, Data::Effects::EffectKind kind);
    std::optional<MuEditor::Effects::EffectTypeRef>
    RenderSameNumber(const MuEditor::Effects::EffectBrowserModel& model);
    void RenderAsset(const MuEditor::Effects::EffectBrowserRow& row);
    std::optional<MuEditor::Effects::EffectTypeRef> RenderStages(const MuEditor::Effects::EffectBrowserModel& model,
                                                                 const MuEditor::Effects::EffectBrowserRow& row,
                                                                 Data::Effects::EffectKind kind);
    void RenderCreationValues(Data::Effects::EffectKind kind);
    void RenderUsedBy(Data::Effects::EffectKind kind);

    // Built when the selection changes, not every frame.
    std::optional<MuEditor::Effects::EffectTypeRef> m_ref;
    MuEditor::Effects::EffectBrowserDetails m_details;
    // The SubTypes of each variant column, written once ("1, 2").
    std::vector<std::string> m_variantSubTypes;
    // The catalogue file of each kind, written once.
    std::array<std::string, Data::Effects::EffectKindCount> m_files;
};

#endif // _EDITOR
