#include "Integration/Discord/RpcCommands.h"

#include "json.hpp"

namespace
{
using nlohmann::json;

constexpr int RpcVersion = 1;

// Serialises for the worker thread, where an exception would end the game:
// text that is not valid UTF-8 is repaired instead of thrown at.
std::string Serialise(const json& payload)
{
    constexpr int Compact = -1;
    return payload.dump(Compact, ' ', false, json::error_handler_t::replace);
}

// Discord refuses text fields shorter than this, so such a field is left out.
constexpr std::size_t MinTextLength = 2;

// Bytes 10xxxxxx continue a UTF-8 sequence; a cut must not land before one.
constexpr unsigned char ContinuationMask = 0xC0;
constexpr unsigned char ContinuationBits = 0x80;

bool IsContinuationByte(char byte)
{
    return (static_cast<unsigned char>(byte) & ContinuationMask) == ContinuationBits;
}

void SetText(json& target, const char* key, std::string_view text)
{
    if (text.size() < MinTextLength)
    {
        return;
    }
    target[key] = Integration::Discord::Rpc::ClampText(text);
}

json DescribeAssets(const Integration::Discord::Activity& activity)
{
    json assets = json::object();
    if (!activity.largeImageKey.empty())
    {
        assets["large_image"] = activity.largeImageKey;
        SetText(assets, "large_text", activity.largeImageText);
    }
    if (!activity.smallImageKey.empty())
    {
        assets["small_image"] = activity.smallImageKey;
        SetText(assets, "small_text", activity.smallImageText);
    }
    return assets;
}

json DescribeActivity(const Integration::Discord::Activity& activity)
{
    json described = json::object();
    SetText(described, "details", activity.details);
    SetText(described, "state", activity.state);
    if (activity.startTimestamp > 0)
    {
        described["timestamps"] = {{"start", activity.startTimestamp}};
    }

    json assets = DescribeAssets(activity);
    if (!assets.empty())
    {
        described["assets"] = std::move(assets);
    }
    return described;
}

json ParseOrNull(std::string_view payload)
{
    return json::parse(payload, nullptr, false);
}

// Discord sends `"evt": null` in its ordinary answers, so a field is only
// taken when it really holds text.
std::string StringField(const json& object, const char* key)
{
    if (!object.is_object())
    {
        return {};
    }
    const auto field = object.find(key);
    return field != object.end() && field->is_string() ? field->get<std::string>() : std::string();
}
} // namespace

namespace Integration::Discord::Rpc
{
std::string Handshake(std::string_view applicationId)
{
    const json handshake = {{"v", RpcVersion}, {"client_id", applicationId}};
    return Serialise(handshake);
}

std::string SetActivity(const std::optional<Activity>& activity, std::uint32_t processId, std::uint64_t nonce)
{
    json args = {{"pid", processId}};
    if (activity.has_value())
    {
        args["activity"] = DescribeActivity(*activity);
    }

    const json command = {{"cmd", "SET_ACTIVITY"}, {"args", std::move(args)}, {"nonce", std::to_string(nonce)}};
    return Serialise(command);
}

bool IsReadyEvent(std::string_view payload)
{
    return StringField(ParseOrNull(payload), "evt") == "READY";
}

std::string ErrorMessage(std::string_view payload)
{
    constexpr const char* UnknownError = "unknown error";
    const json parsed = ParseOrNull(payload);
    if (StringField(parsed, "evt") != "ERROR")
    {
        return {};
    }

    const auto data = parsed.find("data");
    const std::string message = data != parsed.end() ? StringField(*data, "message") : std::string();
    return message.empty() ? UnknownError : message;
}

std::string ClampText(std::string_view text)
{
    if (text.size() <= MaxTextLength)
    {
        return std::string(text);
    }

    std::size_t cut = MaxTextLength;
    while (cut > 0 && IsContinuationByte(text[cut]))
    {
        --cut;
    }
    return std::string(text.substr(0, cut));
}
} // namespace Integration::Discord::Rpc
