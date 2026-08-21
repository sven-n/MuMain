#include "stdafx.h"
#include "GameLogic/Commands/ChatCommandFavourites.h"

#include "Data/GameConfig/GameConfig.h"

#include <algorithm>
#include <vector>

namespace GameLogic::Commands::Favourites
{
bool Contains(const std::wstring& command)
{
    const auto& favourites = GameConfig::GetInstance().GetChatCommandFavourites();
    return std::find(favourites.begin(), favourites.end(), command) != favourites.end();
}

bool Toggle(const std::wstring& command)
{
    if (command.empty())
    {
        return false;
    }

    auto favourites = GameConfig::GetInstance().GetChatCommandFavourites();
    const auto found = std::find(favourites.begin(), favourites.end(), command);
    const bool isFavouriteNow = found == favourites.end();
    if (isFavouriteNow)
    {
        favourites.push_back(command);
    }
    else
    {
        favourites.erase(found);
    }

    GameConfig::GetInstance().SetChatCommandFavourites(favourites);
    return isFavouriteNow;
}
} // namespace GameLogic::Commands::Favourites
