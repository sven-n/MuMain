#include "stdafx.h"

#include "ItemBmdLanguages.h"

#include "Data/GameData/Common/LocalizedString.h"

#include <array>

namespace Data::Items
{
std::span<const ItemBmdLanguage> GetItemBmdLanguages()
{
    static const std::array<ItemBmdLanguage, 3> Languages = {{
        {L"Eng", std::string(LocalizedString::NeutralLocale)},
        {L"Por", "pt"},
        {L"Spn", "es"},
    }};
    return Languages;
}
} // namespace Data::Items
