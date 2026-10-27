#pragma once

#include <span>
#include <string>

namespace Data
{
// A language of the original client's per-language files, which live in
// Data/Local/<folder>/ (Item_<folder>.bmd, NpcName_<folder>.txt), and the UI
// locale code its texts belong to.
struct LegacyLanguage
{
    std::wstring folder;
    std::string locale;
};

// Eng (en), Por (pt), Spn (es), English first.
std::span<const LegacyLanguage> GetLegacyLanguages();
} // namespace Data
