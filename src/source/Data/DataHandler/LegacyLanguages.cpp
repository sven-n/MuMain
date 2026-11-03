#include "stdafx.h"

#include "LegacyLanguages.h"

#include "Data/GameData/Common/LocalizedString.h"

#include <array>

namespace Data
{
std::span<const LegacyLanguage> GetLegacyLanguages()
{
    static const std::array<LegacyLanguage, 3> Languages = {{
        {L"Eng", std::string(LocalizedString::NeutralLocale)},
        {L"Por", "pt"},
        {L"Spn", "es"},
    }};
    return Languages;
}
} // namespace Data
