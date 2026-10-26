#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <string>
#include <vector>

namespace MuEditor::Monsters
{
// Save and Revert of the monster list file (Data/Monsters/Monsters.json) and
// the model list file (Data/Monsters/Models.json), and what they last
// reported, for every monster tool that offers them.
class MonsterFileActions
{
public:
    static MonsterFileActions& GetInstance();

    // "Save Monsters"; true when the file was written.
    bool RenderSaveButton();
    // "Revert", which reads the file again; true when it was read.
    bool RenderRevertButton();
    // The last save's or revert's result and the problems it found.
    void RenderStatus() const;

    bool Save();
    bool Revert();

private:
    MonsterFileActions() = default;

    std::string m_status;
    bool m_statusIsError = false;
    std::vector<Data::Items::ItemDataIssue> m_issues;
};
} // namespace MuEditor::Monsters

#define g_MonsterFileActions MuEditor::Monsters::MonsterFileActions::GetInstance()

#endif // _EDITOR
