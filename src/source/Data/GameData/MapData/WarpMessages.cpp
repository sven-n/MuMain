#include "stdafx.h"

#include "WarpMessages.h"

#include "I18N/All.h"

#include <algorithm>
#include <array>

namespace Data::Maps
{
namespace
{
struct WarpMessage
{
    std::string_view key;
    // The text's pointer, which follows the language.
    const wchar_t** text;
};

const std::array<WarpMessage, 3> Messages{{
    {GeneralWarpMessageKey, &I18N::Game::WarpBlockedByEquipment},
    {"YouCannotGoToAtlansWhileRidingAUnicorn", &I18N::Game::YouCannotGoToAtlansWhileRidingAUnicorn},
    {"YouCanEnterIcarusOnlyWithWingsDinorantFenrirr", &I18N::Game::YouCanEnterIcarusOnlyWithWingsDinorantFenrirr},
}};

const std::array<std::string_view, Messages.size()> Keys = []
{
    std::array<std::string_view, Messages.size()> keys{};
    std::transform(Messages.begin(), Messages.end(), keys.begin(),
                   [](const WarpMessage& message) { return message.key; });
    return keys;
}();
} // namespace

std::span<const std::string_view> GetWarpMessageKeys()
{
    return Keys;
}

bool IsWarpMessageKey(std::string_view key)
{
    return std::find(Keys.begin(), Keys.end(), key) != Keys.end();
}

const wchar_t* GetWarpMessageText(std::string_view key)
{
    const auto found =
        std::find_if(Messages.begin(), Messages.end(), [&](const WarpMessage& message) { return message.key == key; });
    return *(found != Messages.end() ? found : Messages.begin())->text;
}
} // namespace Data::Maps
