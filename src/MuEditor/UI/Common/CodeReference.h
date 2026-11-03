#pragma once

#ifdef _EDITOR

#include <string>

namespace MuEditor
{
// A source line that names an item or a monster, which the tools show before
// it is removed.
struct CodeReference
{
    // How the line names it; a line that names it in more than one way counts
    // the first of these.
    enum class Kind
    {
        // Its constant (ITEM_KRIS, MONSTER_BUDGE_DRAGON).
        Name,
        // Its number (ITEM_POTION + 100, c->MonsterIndex == 495).
        RawId,
        // A range check whose two bounds enclose it.
        Range,
    };

    // Relative to the source folder, with '/'.
    std::string file;
    // 1-based.
    int line = 0;
    // The line as written, comments included, trimmed.
    std::string text;
    Kind kind = Kind::Name;
};
} // namespace MuEditor

#endif // _EDITOR
