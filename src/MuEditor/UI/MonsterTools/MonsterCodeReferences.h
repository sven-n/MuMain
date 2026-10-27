#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/Common/CodeReference.h"

#include <filesystem>
#include <span>
#include <vector>

namespace MuEditor::Monsters
{
// The lines of a source folder that name a monster number. The monster names
// are the MONSTER_ constants of enum EMonsterType in Core/Globals/_enum.h;
// comments and string literals name nothing. Not found: a number compared
// with a variable whose name does not end in MonsterIndex, a number passed to
// a function or in a case label, and code keyed by the monster's model.
class MonsterCodeReferences
{
public:
    // Reads enum EMonsterType of _enum.h (Core/Globals/_enum.h below the
    // folder) for the monster names, then every .cpp and .h file below the
    // folder except _enum.h. False when the folder or _enum.h is missing; then
    // nothing is known.
    bool Load(const std::filesystem::path& sourceFolder);
    bool IsLoaded() const
    {
        return m_loaded;
    }

    // The places that name the number, by file and line (see
    // ReadMonsterMentions); empty outside 0-1023.
    std::span<const CodeReference> Find(int number) const;

private:
    // By monster number; empty until loaded.
    std::vector<std::vector<CodeReference>> m_references;
    bool m_loaded = false;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
