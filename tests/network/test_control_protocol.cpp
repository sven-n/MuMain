// doctest unit tests for the control socket's request/response encoding.
//
// The protocol is pure text handling: no window, no renderer, no connection.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Control protocol"

#include "doctest.h"

#include "App/Control/ControlProtocol.h"
#include "Network/Server/WSclient.h"

#include <cctype>
#include <string>
#include <vector>

using App::Control::ErrorCode;
using App::Control::Request;

TEST_CASE("Control protocol decodes a well-formed request [network][control-protocol]")
{
    const Request request = Request::Parse(R"({"cmd":"move","id":7,"x":140,"y":130,"label":"spawn"})");

    REQUIRE(request.IsValid());
    CHECK_FALSE(request.IsEmpty());
    CHECK(request.Command() == "move");
    CHECK(request.EncodedId() == "7");

    int x = 0;
    int y = 0;
    REQUIRE(request.GetInt("x", x));
    REQUIRE(request.GetInt("y", y));
    CHECK(x == 140);
    CHECK(y == 130);

    std::string label;
    REQUIRE(request.GetString("label", label));
    CHECK(label == "spawn");

    // A field of the wrong type is reported as absent rather than guessed at.
    CHECK_FALSE(request.GetInt("label", x));
    CHECK_FALSE(request.GetString("x", label));
    CHECK_FALSE(request.Has("times"));
}

TEST_CASE("Control protocol rejects malformed lines without poisoning the next [network][control-protocol]")
{
    const Request malformed = Request::Parse("not json");
    CHECK_FALSE(malformed.IsValid());
    CHECK(malformed.Error() == ErrorCode::BadRequest);
    CHECK_FALSE(malformed.Message().empty());

    // A JSON value that is not an object is equally bad.
    CHECK(Request::Parse("[1,2,3]").Error() == ErrorCode::BadRequest);
    CHECK(Request::Parse(R"("ping")").Error() == ErrorCode::BadRequest);
    CHECK(Request::Parse("{}").Error() == ErrorCode::BadRequest);
    CHECK(Request::Parse(R"({"cmd":5})").Error() == ErrorCode::BadRequest);

    // The very next line parses normally: nothing is carried over.
    const Request good = Request::Parse(R"({"cmd":"ping"})");
    CHECK(good.IsValid());
    CHECK(good.Command() == "ping");
    CHECK(good.EncodedId().empty());
}

TEST_CASE("Control protocol reports an unknown command [network][control-protocol]")
{
    const Request request = Request::Parse(R"({"cmd":"nonsense","id":"abc"})");

    CHECK_FALSE(request.IsValid());
    CHECK(request.Error() == ErrorCode::UnknownCommand);
    CHECK(request.Message().find("nonsense") != std::string::npos);
    // The id is still echoed, so the caller can match the failure to its request.
    CHECK(request.EncodedId() == R"("abc")");
}

TEST_CASE("Control protocol treats a blank line as no request [network][control-protocol]")
{
    CHECK(Request::Parse("").IsEmpty());
    CHECK(Request::Parse("   \t ").IsEmpty());
    CHECK_FALSE(Request::Parse("   \t ").IsValid());
}

TEST_CASE("Control protocol serves the documented command vocabulary [network][control-protocol]")
{
    const std::vector<std::string> expected = {
        "ping",    "scene", "state", "nearby",   "events",   "wait-for", "screenshot", "login", "select-char", "logout",
        "quit",    "move",  "warp",  "teleport", "attack",   "skill",    "pickup",     "use",   "equip",       "say",
        "whisper", "party", "halt",  "hotkey",   "click-ui", "ui",       "slot-pixel", "trade",
    };

    for (const std::string& command : expected)
    {
        CAPTURE(command);
        CHECK(App::Control::IsKnownCommand(command));
    }
    CHECK(App::Control::CommandNames().size() == expected.size());
    CHECK_FALSE(App::Control::IsKnownCommand("shutdown"));
    CHECK_FALSE(App::Control::IsKnownCommand(""));
}

TEST_CASE("Control protocol encodes responses and echoes the id [network][control-protocol]")
{
    const std::string result = App::Control::EncodeResult("7", R"({"scene":"login"})");
    CHECK(result.back() == '\n');
    CHECK(result.find(R"("ok":true)") != std::string::npos);
    CHECK(result.find(R"("id":7)") != std::string::npos);
    CHECK(result.find(R"("scene":"login")") != std::string::npos);

    const std::string withoutId = App::Control::EncodeResult("", "");
    CHECK(withoutId.find(R"("id")") == std::string::npos);
    CHECK(withoutId.find(R"("result":{})") != std::string::npos);

    const std::string failure =
        App::Control::EncodeError(R"("abc")", ErrorCode::WrongScene, "client is on the login screen");
    CHECK(failure.back() == '\n');
    CHECK(failure.find(R"("ok":false)") != std::string::npos);
    CHECK(failure.find(R"("id":"abc")") != std::string::npos);
    CHECK(failure.find(R"("error":"wrong_scene")") != std::string::npos);
    CHECK(failure.find("login screen") != std::string::npos);
    // One object per line, always.
    CHECK(failure.find('\n') == failure.size() - 1);
}

TEST_CASE("Control protocol names every error code [network][control-protocol]")
{
    const std::vector<ErrorCode> codes = {
        ErrorCode::BadRequest,       ErrorCode::UnknownCommand, ErrorCode::WrongScene,   ErrorCode::Busy,
        ErrorCode::Interrupted,      ErrorCode::Timeout,        ErrorCode::NotConnected, ErrorCode::LoginFailed,
        ErrorCode::NoSuchCharacter,  ErrorCode::NoSuchSkill,    ErrorCode::NotInView,    ErrorCode::NotAttackable,
        ErrorCode::NoPath,           ErrorCode::NotAllowed,     ErrorCode::WarpRefused,  ErrorCode::SkillRefused,
        ErrorCode::InsufficientMana, ErrorCode::NotPickable,    ErrorCode::EmptySlot,    ErrorCode::MoveRefused,
        ErrorCode::NotOpen,          ErrorCode::Failed,
    };

    for (const ErrorCode code : codes)
    {
        const std::string_view name = App::Control::ErrorCodeName(code);
        CAPTURE(name);
        CHECK_FALSE(name.empty());
        // Wire names are lower snake case so a test script reads them as they are.
        for (const char character : name)
        {
            CHECK((std::islower(static_cast<unsigned char>(character)) != 0 || character == '_'));
        }
    }
}

TEST_CASE("Control protocol maps every login failure macro to a reason [network][control-protocol]")
{
    const std::vector<int> failureCodes = {
        RECEIVE_LOG_IN_FAIL_PASSWORD,
        RECEIVE_LOG_IN_FAIL_ID,
        RECEIVE_LOG_IN_FAIL_ID_CONNECTED,
        RECEIVE_LOG_IN_FAIL_SERVER_BUSY,
        RECEIVE_LOG_IN_FAIL_ID_BLOCK,
        RECEIVE_LOG_IN_FAIL_VERSION,
        RECEIVE_LOG_IN_FAIL_CONNECT,
        RECEIVE_LOG_IN_FAIL_ERROR,
        RECEIVE_LOG_IN_FAIL_USER_TIME1,
        RECEIVE_LOG_IN_FAIL_USER_TIME2,
        RECEIVE_LOG_IN_FAIL_PC_TIME1,
        RECEIVE_LOG_IN_FAIL_PC_TIME2,
        RECEIVE_LOG_IN_FAIL_DATE,
        RECEIVE_LOG_IN_FAIL_POINT_DATE,
        RECEIVE_LOG_IN_FAIL_POINT_HOUR,
        RECEIVE_LOG_IN_FAIL_INVALID_IP,
        RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO,
        RECEIVE_LOG_IN_FAIL_ONLY_OVER_15,
        RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL,
    };

    for (const int code : failureCodes)
    {
        CAPTURE(code);
        CHECK(App::Control::IsLoginFailureCode(code));
        // Not just non-empty: an unknown code also gets a reason (the
        // fallback below), so the test would pass for a code that fell out
        // of the table.
        CHECK_FALSE(App::Control::LoginFailureReason(code).empty());
        CHECK(App::Control::LoginFailureReason(code) != App::Control::LoginFailureReason(RECEIVE_LOG_IN_SUCCESS));
    }

    // The password failure is the one the spec's wrong-password scenario shows.
    CHECK(App::Control::LoginFailureReason(RECEIVE_LOG_IN_FAIL_PASSWORD).find("password") != std::string_view::npos);

    // A success or an unrelated message code is not a login failure.
    CHECK_FALSE(App::Control::IsLoginFailureCode(RECEIVE_LOG_IN_SUCCESS));
    CHECK_FALSE(App::Control::IsLoginFailureCode(RECEIVE_CHARACTERS_LIST));
    // Unknown codes still get a usable reason rather than an empty message.
    CHECK_FALSE(App::Control::LoginFailureReason(-1).empty());
}
