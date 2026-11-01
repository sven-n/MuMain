#pragma once

#ifdef _EDITOR

#include "NamedColorSource.h"

#include "../MuEditor/UI/ServerList/ServerListColors.h"
#include "Data/GameData/ServerListData/CustomColor.h"

#include <map>
#include <string>

namespace MuEditor::Colors
{
// The Colors window's Server list root: the theme colors the list uses (Data::ServerList::IsThemeColorEntry), each a
// token of every theme's tokens.ini, and the colors of Data/Local/ServerList.json's own, each with a shade per theme
// folder, which the button color rules and the load name by their ID. A change of the list's own colors is a step
// of the Server List window's history, the login screen shows it at once, and the Server List window's Save writes
// it. A theme color can't be removed; Apply writes it into the changed themes' tokens.ini at once and reloads the
// game's theme.
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
    // The theme color entry `name`: its token in each theme folder, read again when it changed (an Apply).
    void UpdateThemeEdit(const std::string& name);
    void RenderThemeEdit(const std::string& name);
    void ApplyThemeEdit(const std::string& name);

    Data::ServerList::CustomColor m_edit;
    Data::ServerList::CustomColor m_loaded;
    std::string m_editOf;
    // The picked theme color's value in each theme folder, as edited and as read, and the folders a write failed for.
    std::map<std::string, MuEditor::ServerList::Colors::TokenColor> m_themeEdit;
    std::map<std::string, MuEditor::ServerList::Colors::TokenColor> m_themeLoaded;
    std::string m_themeEditOf;
    bool m_themeReread = false;
    std::string m_themeWriteFailed;
    // The new color dialog: its name, and the color it copies the shades of.
    std::string m_newName;
    std::string m_newFrom;
};
} // namespace MuEditor::Colors

#endif // _EDITOR
