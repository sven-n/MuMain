#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/DataChecks/DataCheckArea.h"

#include <functional>
#include <memory>

namespace MuEditor::Monsters
{
// The monsters or the monster models in Data Checks: the checks of Save
// Monsters on the tables as the tools have them now. A problem of a monster
// names it; a click selects it and opens the tool of its field. A problem of a
// model names the model in its field.
class MonsterCheckArea final : public DataChecks::DataCheckArea
{
public:
    using CheckTables = std::function<void(std::vector<Data::Items::ItemDataIssue>& issues)>;

    // `name` is a translated text, read when drawn.
    MonsterCheckArea(const char* const* name, CheckTables check);

    const char* GetName() const override;
    void Check(std::vector<Data::Items::ItemDataIssue>& issues) override;
    // "2 Budge Dragon" in the UI language, "76" for a number without an entry
    // (removed since the check), or the file for a problem that is not about
    // one monster.
    void RenderSubject(const Data::Items::ItemDataIssue& issue) const override;
    // The monster is in the What column: "name.de", not "monster 2 name.de".
    std::string_view GetField(const Data::Items::ItemDataIssue& issue) const override;
    // A problem that is not about one monster has nothing to open.
    void Open(const Data::Items::ItemDataIssue& issue, bool clicked) override;

private:
    const char* const* m_name;
    CheckTables m_check;
};

// The monster list and the monsters' attacks.
std::unique_ptr<DataChecks::DataCheckArea> CreateMonsterCheckArea();
// The model list.
std::unique_ptr<DataChecks::DataCheckArea> CreateMonsterModelCheckArea();
} // namespace MuEditor::Monsters

#endif // _EDITOR
