#pragma once

#ifdef _EDITOR

#include "NamedColorList.h"
#include "NamedColorSource.h"

#include "../MuEditor/UI/ItemTools/GlowColorDialog.h"
#include "../MuEditor/UI/ItemTools/ItemLookUseCounts.h"
#include "Data/GameData/EffectData/GlowColors.h"

#include <array>
#include <string>

namespace MuEditor::Colors
{
// The Colors window's Glow tab (D26 for the glow colors of the item models): the colors with how many items use each;
// a new color, a changed value (the items it reaches are listed before it is applied) and the colors no item uses
// removed. The model tool picks an item's colors from them.
class GlowColorsTab final : public NamedColorSource
{
public:
    void Render();

    NamedColorTexts GetTexts() const override;
    std::size_t GetCount() const override;
    const std::string& GetName(std::size_t index) const override;
    bool Has(const std::string& name) const override;
    void RenderSwatch(std::size_t index) const override;
    int CountUsers(const std::string& name) override;
    std::vector<int> FindUsers(const std::string& name) const override;
    bool IsKept(const std::string& name) const override;
    void Remove(const char* step, std::span<const std::string> names) override;
    void OpenNew(const std::string& from) override;
    std::optional<std::string> RenderNew() override;
    void RenderEdit(const std::string& name) override;
    bool IsEdited(const std::string& name) const override;
    void ApplyEdit(const std::string& name) override;
    void RevertEdit(const std::string& name) override;

private:
    // The edited value follows the color (an undo, a redo) until it is changed.
    void UpdateEdit(const std::string& name, const Data::Effects::GlowColorValue& value);

    std::array<float, 3> m_edit{};
    Data::Effects::GlowColorValue m_loaded{};
    std::string m_editOf;
    int m_editVersion = -1;
    MuEditor::Items::ItemLookUseCounts m_counts;
    MuEditor::Items::CGlowColorDialog m_dialog;
    NamedColorList m_list;
};
} // namespace MuEditor::Colors

#endif // _EDITOR
