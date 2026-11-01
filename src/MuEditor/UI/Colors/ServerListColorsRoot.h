#pragma once

#ifdef _EDITOR

#include "NamedColorSource.h"

#include "Data/GameData/ServerListData/CustomColor.h"

#include <string>

namespace MuEditor::Colors
{
// The Colors window's Server list root: the colors of Data/Local/ServerList.json's own, each with a shade per theme
// folder, which the button color rules and the load name by their ID. Its changes are steps of the Server List
// window's history, the login screen shows them at once, and the Server List window's Save writes them.
class ServerListColorsRoot final : public NamedColorSource
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
    std::string GetNameAfterEdit(const std::string& name) const override;
    MuEditor::UndoTarget& GetUndoTarget() override;

private:
    // The edited color follows the list's (an undo, a redo) until it is changed.
    void UpdateEdit(const Data::ServerList::CustomColor& color);
    // Whether the edited ID can be the color's: its own, or a free one.
    bool IsEditedIdUsable(const std::string& name) const;

    Data::ServerList::CustomColor m_edit;
    Data::ServerList::CustomColor m_loaded;
    std::string m_editOf;
    // The new color dialog: its name, and the color it copies the shades of.
    std::string m_newName;
    std::string m_newFrom;
};
} // namespace MuEditor::Colors

#endif // _EDITOR
