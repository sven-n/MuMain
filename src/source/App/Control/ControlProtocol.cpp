#include "App/Control/ControlProtocol.h"

#include "Network/Server/WSclient.h"

#include "json.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <utility>

namespace
{
using nlohmann::json;

constexpr char ResponseTerminator = '\n';

struct ErrorCodeEntry
{
    App::Control::ErrorCode code;
    std::string_view name;
};

constexpr std::array<ErrorCodeEntry, 21> ErrorCodeNames = {{
    {App::Control::ErrorCode::BadRequest, "bad_request"},
    {App::Control::ErrorCode::UnknownCommand, "unknown_command"},
    {App::Control::ErrorCode::WrongScene, "wrong_scene"},
    {App::Control::ErrorCode::Busy, "busy"},
    {App::Control::ErrorCode::Interrupted, "interrupted"},
    {App::Control::ErrorCode::Timeout, "timeout"},
    {App::Control::ErrorCode::NotConnected, "not_connected"},
    {App::Control::ErrorCode::LoginFailed, "login_failed"},
    {App::Control::ErrorCode::NoSuchCharacter, "no_such_character"},
    {App::Control::ErrorCode::NoSuchSkill, "no_such_skill"},
    {App::Control::ErrorCode::NotInView, "not_in_view"},
    {App::Control::ErrorCode::NotAttackable, "not_attackable"},
    {App::Control::ErrorCode::NoPath, "no_path"},
    {App::Control::ErrorCode::NotAllowed, "not_allowed"},
    {App::Control::ErrorCode::WarpRefused, "warp_refused"},
    {App::Control::ErrorCode::SkillRefused, "skill_refused"},
    {App::Control::ErrorCode::InsufficientMana, "insufficient_mana"},
    {App::Control::ErrorCode::NotPickable, "not_pickable"},
    {App::Control::ErrorCode::EmptySlot, "empty_slot"},
    {App::Control::ErrorCode::MoveRefused, "move_refused"},
    {App::Control::ErrorCode::Failed, "failed"},
}};

struct LoginFailureEntry
{
    int messageCode;
    std::string_view reason;
};

// The client's own failure vocabulary, by the named macros only: the
// numbers are upstream's and may be renumbered.
constexpr std::array<LoginFailureEntry, 19> LoginFailures = {{
    {RECEIVE_LOG_IN_FAIL_PASSWORD, "password is incorrect"},
    {RECEIVE_LOG_IN_FAIL_ID, "account is invalid"},
    {RECEIVE_LOG_IN_FAIL_ID_CONNECTED, "account is already connected"},
    {RECEIVE_LOG_IN_FAIL_SERVER_BUSY, "server is full"},
    {RECEIVE_LOG_IN_FAIL_ID_BLOCK, "account is blocked"},
    {RECEIVE_LOG_IN_FAIL_VERSION, "client version is rejected"},
    {RECEIVE_LOG_IN_FAIL_CONNECT, "connection error"},
    {RECEIVE_LOG_IN_FAIL_ERROR, "connection closed after three failed attempts"},
    {RECEIVE_LOG_IN_FAIL_USER_TIME1, "individual subscription term is over"},
    {RECEIVE_LOG_IN_FAIL_USER_TIME2, "individual subscription time is over"},
    {RECEIVE_LOG_IN_FAIL_PC_TIME1, "subscription term is over for this address"},
    {RECEIVE_LOG_IN_FAIL_PC_TIME2, "subscription time is over for this address"},
    {RECEIVE_LOG_IN_FAIL_DATE, "subscription date is over"},
    {RECEIVE_LOG_IN_FAIL_POINT_DATE, "point subscription date is over"},
    {RECEIVE_LOG_IN_FAIL_POINT_HOUR, "point subscription hours are used up"},
    {RECEIVE_LOG_IN_FAIL_INVALID_IP, "address is not allowed"},
    {RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO, "no charge information"},
    {RECEIVE_LOG_IN_FAIL_ONLY_OVER_15, "account is below the age limit"},
    {RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL, "this channel requires a paid subscription"},
}};

App::Control::Value FromJson(const json& value)
{
    if (value.is_boolean())
    {
        return App::Control::Value(value.get<bool>());
    }
    if (value.is_number())
    {
        return App::Control::Value(value.get<double>());
    }
    if (value.is_string())
    {
        return App::Control::Value(value.get<std::string>());
    }
    if (value.is_object())
    {
        std::map<std::string, std::string> entries;
        for (const auto& [key, entry] : value.items())
        {
            entries[key] = entry.is_string() ? entry.get<std::string>() : entry.dump();
        }
        return App::Control::Value(std::move(entries));
    }
    // Arrays and null carry no request field the command set reads; they
    // arrive as a null value rather than an error, so an unknown extra
    // field never fails a valid command.
    return App::Control::Value();
}
} // namespace

namespace App::Control
{
Value::Value(bool value) : m_kind(Kind::Bool), m_bool(value) {}

Value::Value(double value) : m_kind(Kind::Number), m_number(value) {}

Value::Value(std::string value) : m_kind(Kind::String), m_string(std::move(value)) {}

Value::Value(std::map<std::string, std::string> value) : m_kind(Kind::StringMap), m_map(std::move(value)) {}

bool Value::AsBool(bool fallback) const
{
    if (m_kind == Kind::Bool)
    {
        return m_bool;
    }
    if (m_kind == Kind::Number)
    {
        return m_number != 0.0;
    }
    return fallback;
}

double Value::AsNumber(double fallback) const
{
    if (m_kind == Kind::Number)
    {
        return m_number;
    }
    if (m_kind == Kind::Bool)
    {
        return m_bool ? 1.0 : 0.0;
    }
    return fallback;
}

std::string_view ErrorCodeName(ErrorCode code)
{
    for (const auto& entry : ErrorCodeNames)
    {
        if (entry.code == code)
        {
            return entry.name;
        }
    }
    return "failed";
}

const std::vector<std::string>& CommandNames()
{
    static const std::vector<std::string> names = {
        "ping",   "scene", "state",   "nearby", "events",   "wait-for", "screenshot", "login",  "select-char",
        "logout", "quit",  "move",    "warp",   "teleport", "attack",   "skill",      "pickup", "use",
        "equip",  "say",   "whisper", "party",  "halt",     "hotkey",   "click-ui",
    };
    return names;
}

bool IsKnownCommand(std::string_view command)
{
    const auto& names = CommandNames();
    return std::find(names.begin(), names.end(), command) != names.end();
}

Request Request::Parse(std::string_view line)
{
    Request request;

    const bool blank =
        std::all_of(line.begin(), line.end(), [](unsigned char character) { return std::isspace(character) != 0; });
    if (blank)
    {
        request.m_empty = true;
        return request;
    }

    const json parsed = json::parse(line, nullptr, false);
    if (parsed.is_discarded() || !parsed.is_object())
    {
        request.m_error = ErrorCode::BadRequest;
        request.m_message = "request is not a JSON object";
        return request;
    }

    if (parsed.contains("id"))
    {
        request.m_encodedId = parsed.at("id").dump();
    }

    const auto command = parsed.find("cmd");
    if (command == parsed.end() || !command->is_string())
    {
        request.m_error = ErrorCode::BadRequest;
        request.m_message = "request has no `cmd` string";
        return request;
    }

    request.m_command = command->get<std::string>();
    if (!IsKnownCommand(request.m_command))
    {
        request.m_error = ErrorCode::UnknownCommand;
        request.m_message = "unknown command `" + request.m_command + "`";
        return request;
    }

    for (const auto& [key, value] : parsed.items())
    {
        if (key == "cmd" || key == "id")
        {
            continue;
        }
        request.m_fields.emplace(key, FromJson(value));
    }

    request.m_valid = true;
    return request;
}

bool Request::Has(std::string_view key) const
{
    const auto field = m_fields.find(key);
    return field != m_fields.end() && field->second.GetKind() != Value::Kind::Null;
}

bool Request::GetString(std::string_view key, std::string& out) const
{
    const auto field = m_fields.find(key);
    if (field == m_fields.end() || field->second.GetKind() != Value::Kind::String)
    {
        return false;
    }
    out = field->second.AsString();
    return true;
}

bool Request::GetDouble(std::string_view key, double& out) const
{
    const auto field = m_fields.find(key);
    if (field == m_fields.end() || field->second.GetKind() != Value::Kind::Number)
    {
        return false;
    }
    out = field->second.AsNumber();
    return true;
}

bool Request::GetInt(std::string_view key, int& out) const
{
    double number = 0.0;
    if (!GetDouble(key, number))
    {
        return false;
    }
    out = static_cast<int>(number);
    return true;
}

bool Request::GetBool(std::string_view key, bool& out) const
{
    const auto field = m_fields.find(key);
    if (field == m_fields.end())
    {
        return false;
    }
    const Value::Kind kind = field->second.GetKind();
    if (kind != Value::Kind::Bool && kind != Value::Kind::Number)
    {
        return false;
    }
    out = field->second.AsBool();
    return true;
}

bool Request::GetStringMap(std::string_view key, std::map<std::string, std::string>& out) const
{
    const auto field = m_fields.find(key);
    if (field == m_fields.end() || field->second.GetKind() != Value::Kind::StringMap)
    {
        return false;
    }
    out = field->second.AsStringMap();
    return true;
}

std::string EncodeResult(std::string_view encodedId, std::string_view resultObject)
{
    json response;
    response["ok"] = true;
    if (!encodedId.empty())
    {
        response["id"] = json::parse(encodedId, nullptr, false);
    }

    json result = resultObject.empty() ? json::object() : json::parse(resultObject, nullptr, false);
    if (result.is_discarded())
    {
        result = json::object();
    }
    response["result"] = std::move(result);

    return response.dump() + ResponseTerminator;
}

std::string EncodeError(std::string_view encodedId, ErrorCode code, std::string_view message,
                        std::string_view detailsObject)
{
    json response;
    response["ok"] = false;
    if (!encodedId.empty())
    {
        response["id"] = json::parse(encodedId, nullptr, false);
    }
    response["error"] = ErrorCodeName(code);
    response["message"] = message;

    if (!detailsObject.empty())
    {
        json details = json::parse(detailsObject, nullptr, false);
        if (!details.is_discarded() && !details.empty())
        {
            response["result"] = std::move(details);
        }
    }

    return response.dump() + ResponseTerminator;
}

std::string_view LoginFailureReason(int messageCode)
{
    for (const auto& entry : LoginFailures)
    {
        if (entry.messageCode == messageCode)
        {
            return entry.reason;
        }
    }
    return "server refused the login";
}

bool IsLoginFailureCode(int messageCode)
{
    for (const auto& entry : LoginFailures)
    {
        if (entry.messageCode == messageCode)
        {
            return true;
        }
    }
    return false;
}
} // namespace App::Control
