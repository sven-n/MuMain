#pragma once

#ifdef _EDITOR

#include <cstddef>
#include <cstdio>

// The names the editor's tool windows are known by. ImGui keeps a window's
// place and dock under its id (from "###" on), which stays when the title
// shown before it changes with the language. The editor window's first layout
// docks the tool windows by these names.
namespace MuEditor::WindowIds
{
inline constexpr char ItemEditor[] = "###ItemEditor";
inline constexpr char SkillEditor[] = "###SkillEditor";
inline constexpr char DevEditor[] = "###DevEditor";
inline constexpr char EffectBrowser[] = "###EffectBrowser";
// Titles that are not translated: the whole title is the name.
inline constexpr char MapEditor[] = "Map Editor";
inline constexpr char Console[] = "Console";

inline constexpr std::size_t TitleCapacity = 128;

// The title to Begin() a window with: what it shows, then its id.
template <std::size_t Capacity> const char* Title(char (&buffer)[Capacity], const char* shown, const char* id)
{
    std::snprintf(buffer, Capacity, "%s%s", shown, id);
    return buffer;
}
} // namespace MuEditor::WindowIds

#endif // _EDITOR
