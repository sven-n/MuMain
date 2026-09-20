// Wire format of the control socket: newline-delimited JSON, one request
// object per line, exactly one response object per line.
//
// The header stays free of the JSON parser on purpose. Game files that only
// record events or poll the server include this (or ControlEvents.h) and must
// not pay for json.hpp; the parser lives in the App/Control translation units.
#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace App::Control
{
// Error codes answered to a caller. The names are the wire values; see
// ErrorCodeName(). Keep them in sync with docs/control-socket.md.
enum class ErrorCode : std::uint8_t
{
    BadRequest,
    UnknownCommand,
    WrongScene,
    Busy,
    Interrupted,
    Timeout,
    NotConnected,
    LoginFailed,
    NoSuchCharacter,
    NoSuchSkill,
    NotInView,
    NotAttackable,
    NoPath,
    NotAllowed,
    WarpRefused,
    SkillRefused,
    InsufficientMana,
    NotPickable,
    EmptySlot,
    MoveRefused,
    Failed,
};

[[nodiscard]] std::string_view ErrorCodeName(ErrorCode code);

// A decoded request field. Only the shapes the command set uses are
// represented, so the header needs no JSON type.
class Value
{
public:
    enum class Kind : std::uint8_t
    {
        Null,
        // A shape no argument of the protocol takes (an array). Kept apart
        // from Null so `Has()` still sees the field and its command answers
        // `bad_request` instead of running with a default.
        Unsupported,
        Bool,
        Number,
        String,
        StringMap,
    };

    Value() = default;
    [[nodiscard]] static Value OfUnsupportedKind();
    explicit Value(bool value);
    explicit Value(double value);
    explicit Value(std::string value);
    explicit Value(std::map<std::string, std::string> value);

    [[nodiscard]] Kind GetKind() const
    {
        return m_kind;
    }
    [[nodiscard]] bool AsBool(bool fallback = false) const;
    [[nodiscard]] double AsNumber(double fallback = 0.0) const;
    [[nodiscard]] const std::string& AsString() const
    {
        return m_string;
    }
    [[nodiscard]] const std::map<std::string, std::string>& AsStringMap() const
    {
        return m_map;
    }

private:
    Kind m_kind = Kind::Null;
    bool m_bool = false;
    double m_number = 0.0;
    std::string m_string;
    std::map<std::string, std::string> m_map;
};

// One decoded request line.
class Request
{
public:
    // Decodes one line. A line that is not a JSON object, carries no `cmd`,
    // or names a command outside the vocabulary yields an invalid request
    // whose Error()/Message() describe why; parsing never throws.
    [[nodiscard]] static Request Parse(std::string_view line);

    // True for a line with nothing but whitespace: no request, no response.
    [[nodiscard]] bool IsEmpty() const
    {
        return m_empty;
    }
    [[nodiscard]] bool IsValid() const
    {
        return m_valid;
    }
    [[nodiscard]] ErrorCode Error() const
    {
        return m_error;
    }
    [[nodiscard]] const std::string& Message() const
    {
        return m_message;
    }

    [[nodiscard]] const std::string& Command() const
    {
        return m_command;
    }
    // The caller's `id` as it arrived (encoded JSON), empty when absent.
    [[nodiscard]] const std::string& EncodedId() const
    {
        return m_encodedId;
    }

    [[nodiscard]] bool Has(std::string_view key) const;
    [[nodiscard]] bool GetString(std::string_view key, std::string& out) const;
    [[nodiscard]] bool GetInt(std::string_view key, int& out) const;
    [[nodiscard]] bool GetDouble(std::string_view key, double& out) const;
    [[nodiscard]] bool GetBool(std::string_view key, bool& out) const;
    [[nodiscard]] bool GetStringMap(std::string_view key, std::map<std::string, std::string>& out) const;

private:
    bool m_empty = false;
    bool m_valid = false;
    ErrorCode m_error = ErrorCode::BadRequest;
    std::string m_message;
    std::string m_command;
    std::string m_encodedId;
    std::map<std::string, Value, std::less<>> m_fields;
};

// Every command the dispatcher serves. Parsing rejects anything else with
// `unknown_command`, so the table is the vocabulary in one place.
[[nodiscard]] const std::vector<std::string>& CommandNames();
[[nodiscard]] bool IsKnownCommand(std::string_view command);

// Response lines, terminated with '\n'.
//
// `resultObject` is an already-serialised JSON object ("{...}"); command
// implementations build it with the parser in their own translation unit.
[[nodiscard]] std::string EncodeResult(std::string_view encodedId, std::string_view resultObject);
// `detailsObject`, when given, is an encoded JSON object merged into the
// response as `result` — how far an interrupted or timed-out act got.
[[nodiscard]] std::string EncodeError(std::string_view encodedId, ErrorCode code, std::string_view message,
                                      std::string_view detailsObject = {});

// Reason text for a login failure. `messageCode` is the RECEIVE_LOG_IN_FAIL_*
// value the client's message window was popped up with; the login state
// machine reads it there because a failed login never changes
// CurrentProtocolState (see design.md, D5 amendment).
[[nodiscard]] std::string_view LoginFailureReason(int messageCode);
[[nodiscard]] bool IsLoginFailureCode(int messageCode);
} // namespace App::Control
