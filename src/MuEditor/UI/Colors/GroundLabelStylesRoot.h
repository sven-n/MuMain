#pragma once

#ifdef _EDITOR

#include "GroundLabelStyleDialog.h"
#include "NamedColorSource.h"

#include "../MuEditor/UI/ItemTools/ItemLookUseCounts.h"
#include "Data/GameData/ItemData/ItemGroundLabels.h"

#include <string>

namespace MuEditor::Colors
{
// The Colors window's Ground labels root (D68): the label styles with how many items name each in their own rules; a
// new style, a changed look (the items it reaches are listed before it is applied, and the labels on the ground take it
// at once) and the styles nothing names removed. The styles of the standard rules stay.
class GroundLabelStylesRoot final : public NamedColorSource
{
public:
    NamedColorTexts GetTexts() const override;
    int GetVersion() const override;
    std::size_t GetCount() const override;
    const std::string& GetName(std::size_t index) const override;
    bool Has(const std::string& name) const override;
    void RenderSwatch(const std::string& name) const override;
    int CountUsers(const std::string& name) override;
    std::vector<NamedColorUser> FindUsers(const std::string& name) const override;
    bool IsKept(const std::string& name) const override;
    void Remove(const char* step, std::span<const std::string> names) override;
    const std::vector<Data::Colors::ColorFolder>& GetFolders() const override;
    void SetFolders(const char* step, std::vector<Data::Colors::ColorFolder> folders) override;
    void SetOrder(const char* step, std::span<const std::string> names) override;
    void OpenNew(const std::string& from) override;
    std::optional<std::string> RenderNew(const Data::Colors::ColorFolderPath& folder) override;
    void RenderEdit(const std::string& name) override;
    bool IsEdited(const std::string& name) const override;
    void ApplyEdit(const std::string& name) override;
    void RevertEdit(const std::string& name) override;
    MuEditor::UndoTarget& GetUndoTarget() override;

private:
    // The edited look follows the style (an undo, a redo) until it is changed.
    void UpdateEdit(const Data::Items::GroundLabelStyle& style);

    Data::Items::GroundLabelStyle m_edit;
    Data::Items::GroundLabelStyle m_loaded;
    int m_editVersion = -1;
    MuEditor::Items::ItemLookUseCounts m_counts;
    GroundLabelStyleDialog m_dialog;
};
} // namespace MuEditor::Colors

#endif // _EDITOR
