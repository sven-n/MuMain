#include "stdafx.h"
#include "App/Control/ControlCommands.h"

#include "App/Control/ControlTargets.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Network/Server/WSclient.h"

#include "json.hpp"

#include <cstdlib>
#include <string>

namespace
{
using App::Control::ErrorCode;
using App::Control::Request;
using nlohmann::json;

// The /trade command asks for the partner at most one tile away.
constexpr int TradeDistance = 1;

[[nodiscard]] bool IsNextToHero(int targetKey)
{
    const int index = FindCharacterIndex(targetKey);
    if (Hero == nullptr || index >= MAX_CHARACTERS_CLIENT)
    {
        return false;
    }
    const CHARACTER& target = CharactersClient[index];
    return std::abs(target.PositionX - Hero->PositionX) <= TradeDistance &&
           std::abs(target.PositionY - Hero->PositionY) <= TradeDistance;
}

std::string RequestTrade(const Request& request)
{
    int targetKey = -1;
    const std::string targetFailure = App::Control::Targets::TargetError(
        request, App::Control::Targets::ResolveTarget(request, targetKey), "trade request", true);
    if (!targetFailure.empty())
    {
        return targetFailure;
    }
    if (!IsNextToHero(targetKey))
    {
        return App::Control::EncodeError(request.EncodedId(), ErrorCode::NotAllowed,
                                         "the trade partner has to stand at most one tile away");
    }

    SocketClient->ToGameServer()->SendTradeRequest(static_cast<uint16_t>(targetKey));
    json result;
    result["action"] = "request";
    result["target"] = targetKey;
    return App::Control::EncodeResult(request.EncodedId(), result.dump());
}

std::string CancelTrade(const Request& request)
{
    SocketClient->ToGameServer()->SendTradeCancel();
    json result;
    result["action"] = "cancel";
    return App::Control::EncodeResult(request.EncodedId(), result.dump());
}
} // namespace

namespace App::Control::Commands
{
std::string Trade(const Request& request, std::unique_ptr<Act>&)
{
    std::string action;
    if (!request.GetString("action", action) || action.empty())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`trade` needs `action`: request or cancel");
    }

    if (action == "request")
    {
        return RequestTrade(request);
    }
    if (action == "cancel")
    {
        return CancelTrade(request);
    }
    return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                       "unknown `action` `" + action + "`; known: request, cancel");
}
} // namespace App::Control::Commands
