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
// The id the item table had, so a saved layout puts the browser where it was.
inline constexpr char ItemBrowser[] = "###ItemEditor";
inline constexpr char ItemBasics[] = "###ItemBasics";
inline constexpr char ItemRequirements[] = "###ItemRequirements";
inline constexpr char ItemRules[] = "###ItemRules";
inline constexpr char ItemCategories[] = "###ItemCategories";
inline constexpr char ItemModel[] = "###ItemModel";
inline constexpr char ItemGlowColors[] = "###ItemGlowColors";
inline constexpr char ItemFormulas[] = "###ItemFormulas";
inline constexpr char ItemSync[] = "###ItemSync";
inline constexpr char ItemChecks[] = "###ItemChecks";
inline constexpr char SkillEditor[] = "###SkillEditor";
inline constexpr char DevEditor[] = "###DevEditor";
inline constexpr char EffectBrowser[] = "###EffectBrowser";
inline constexpr char ItemTranslations[] = "###ItemTranslations";
inline constexpr char ServerList[] = "###ServerList";
inline constexpr char MonsterBrowser[] = "###MonsterBrowser";
inline constexpr char MonsterTranslations[] = "###MonsterTranslations";
inline constexpr char MonsterChecks[] = "###MonsterChecks";
inline constexpr char MonsterModel[] = "###MonsterModel";
inline constexpr char MonsterPreview[] = "###MonsterPreview";
inline constexpr char MonsterAttacks[] = "###MonsterAttacks";
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
