#include "stdafx.h"
#include "App/Control/ControlCommands.h"

#include "App/Control/ControlEvents.h"
#include "App/Control/ControlState.h"
#include "Core/Input/SyntheticInput.h"
#include "Core/Text/Utf8.h"
#include "Network/Server/WSclient.h"
#include "Scenes/SceneCore.h"
#include "Scenes/SceneManager.h"

#include "json.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace
{
using App::Control::Act;
using App::Control::ErrorCode;
using App::Control::Request;
using nlohmann::json;

// Reported when the executable carries no version resource, which is the
// ordinary case outside Windows: the compile stamp still tells two builds
// of the same source tree apart.
constexpr const char* UnversionedBuildIdentifier = __DATE__ " " __TIME__;
constexpr const char* UnknownVersion = "unknown";

// Two frames are enough for the readback; the allowance covers a stalled
// GPU queue without letting a caller wait indefinitely.
constexpr std::chrono::milliseconds ScreenshotDeadline{5000};

// `wait-for` without a timeout of its own.
constexpr double DefaultWaitForSeconds = 30.0;
// Longest wait a caller may ask for. An hour is far past any scenario step,
// and a bound is what keeps the conversion below defined: a deadline is
// milliseconds in a `long long`, and a double that no such count can hold
// converts to nothing meaningful.
constexpr double MaxWaitForSeconds = 3600.0;
// The widest window pixel a click may name. Far past any resolution, and
// small enough that the cast to `float` is exact.
constexpr double MaxWindowPixel = 100000.0;

// An injected key or click spans three rendered frames; the allowance
// covers a client that renders slowly without letting a caller hang.
constexpr std::chrono::milliseconds SyntheticInputDeadline{5000};

// One recorded event as the protocol reports it.
json EventObject(const App::Control::Events::Record& record)
{
    json event = json::parse(record.fields, nullptr, false);
    if (event.is_discarded() || !event.is_object())
    {
        event = json::object();
    }
    event["seq"] = record.seq;
    event["time"] = record.utc;
    event["event"] = record.name;
    return event;
}

// Every `key=value` of `--match` compared against the event's own fields,
// as text, so a caller can match a name or a number the same way.
bool EventMatches(const json& event, const std::map<std::string, std::string>& match)
{
    for (const auto& [key, expected] : match)
    {
        if (!event.contains(key))
        {
            return false;
        }

        const json& field = event.at(key);
        const std::string actual = field.is_string() ? field.get<std::string>() : field.dump();
        if (actual != expected)
        {
            return false;
        }
    }
    return true;
}

// events --follow: streams new events as they are recorded, one line each,
// until the caller disconnects.
class EventStreamAct : public Act
{
public:
    explicit EventStreamAct(std::uint64_t since) : m_since(since) {}

    [[nodiscard]] std::string_view Name() const override
    {
        return "events";
    }
    [[nodiscard]] bool IsAct() const override
    {
        return false;
    }
    // A follower runs until its caller goes away: no deadline at all.
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return std::nullopt;
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (m_delivered == m_pending.size())
        {
            // Visiting the ring costs its whole capacity, so take everything
            // that is new in one pass and hand it out from there; the ring is
            // only visited again once this batch has been delivered.
            m_pending.clear();
            App::Control::Events::ForEachSince(m_since,
                                               [this](const App::Control::Events::Record& record)
                                               {
                                                   m_pending.push_back(record);
                                                   return true;
                                               });
            m_delivered = 0;
            if (m_pending.empty())
            {
                return Status::Running;
            }
            m_since = m_pending.back().seq;
        }

        json result;
        result["event"] = EventObject(m_pending[m_delivered]);
        ++m_delivered;
        response = App::Control::EncodeResult(EncodedId(), result.dump());
        // One line at a time keeps the order and the framing simple; the
        // dispatcher asks again in the same frame while a backlog is left.
        return Status::Emitted;
    }

private:
    std::uint64_t m_since;
    std::vector<App::Control::Events::Record> m_pending;
    std::size_t m_delivered = 0;
};

// wait-for: watches the ring until a matching event arrives.
class WaitForAct : public Act
{
public:
    WaitForAct(std::string wanted, std::map<std::string, std::string> match, std::uint64_t since,
               std::chrono::milliseconds timeout)
        : m_wanted(std::move(wanted)), m_match(std::move(match)), m_since(since), m_timeout(timeout)
    {
    }

    [[nodiscard]] std::string_view Name() const override
    {
        return "wait-for";
    }
    [[nodiscard]] bool IsAct() const override
    {
        return false;
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return m_timeout;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["event"] = m_wanted;
        progress["last_seq"] = App::Control::Events::LastSequence();
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        // A `wait-for` is pending for seconds at a time and usually finds
        // nothing, so it visits the ring rather than copying it: no
        // allocation per frame, and it stops at its match.
        bool matched = false;
        App::Control::Events::ForEachSince(m_since,
                                           [this, &matched, &response](const App::Control::Events::Record& record)
                                           {
                                               m_since = record.seq;
                                               if (record.name != m_wanted)
                                               {
                                                   return true;
                                               }

                                               const json event = EventObject(record);
                                               if (!EventMatches(event, m_match))
                                               {
                                                   return true;
                                               }

                                               json result;
                                               result["event"] = event;
                                               response = App::Control::EncodeResult(EncodedId(), result.dump());
                                               matched = true;
                                               return false;
                                           });

        return matched ? Status::Finished : Status::Running;
    }

private:
    std::string m_wanted;
    std::map<std::string, std::string> m_match;
    std::uint64_t m_since;
    std::chrono::milliseconds m_timeout;
};

std::string& MutableBuildIdentifier()
{
    static std::string identifier = UnversionedBuildIdentifier;
    return identifier;
}

std::string NotImplemented(const Request& request, std::string_view what)
{
    return App::Control::EncodeError(request.EncodedId(), ErrorCode::Failed,
                                     std::string(what) + " is not implemented yet");
}

// Capture in flight. Shared with the completion callback so an act that is
// interrupted or times out cannot be written to after it is gone.
struct ScreenshotState
{
    bool finished = false;
    ScreenshotOutcome outcome;
};

class ScreenshotAct : public Act
{
public:
    explicit ScreenshotAct(std::shared_ptr<ScreenshotState> state) : m_state(std::move(state)) {}

    [[nodiscard]] std::string_view Name() const override
    {
        return "screenshot";
    }
    [[nodiscard]] bool IsAct() const override
    {
        return false;
    }

    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return ScreenshotDeadline;
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (!m_state->finished)
        {
            return Status::Running;
        }

        const ScreenshotOutcome& outcome = m_state->outcome;
        const std::string path = Core::Text::ToUtf8(outcome.path.c_str());
        if (!outcome.saved)
        {
            response = App::Control::EncodeError(EncodedId(), ErrorCode::Failed,
                                                 "the frame could not be written to `" + path + "`");
            return Status::Finished;
        }

        json result;
        result["path"] = path;
        result["width"] = outcome.width;
        result["height"] = outcome.height;
        response = App::Control::EncodeResult(EncodedId(), result.dump());
        return Status::Finished;
    }

private:
    std::shared_ptr<ScreenshotState> m_state;
};

// hotkey / click-ui: answers once the injected press has been released,
// so the caller's next command sees the UI after the game reacted to it.
class SyntheticInputAct : public Act
{
public:
    SyntheticInputAct(std::string_view name, std::string encodedResult)
        : m_name(name), m_encodedResult(std::move(encodedResult)),
          m_generation(Core::Input::Synthetic::CurrentGeneration())
    {
    }

    // Timed out, interrupted, or its caller went away: forget the injection
    // instead of letting it reach the game after its command was answered.
    // Only its own: several of these run at once, so the injector may already
    // be carrying the next caller's press.
    ~SyntheticInputAct() override
    {
        if (IsStillMine())
        {
            Core::Input::Synthetic::Reset();
        }
    }

    [[nodiscard]] std::string_view Name() const override
    {
        return m_name;
    }
    // An injected key or click observes the single act slot rather than
    // taking it: opening the inventory does not cancel a walk. A second
    // injection is still refused, by the idleness check in the handler.
    [[nodiscard]] bool IsAct() const override
    {
        return false;
    }

    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return SyntheticInputDeadline;
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        // Once the injector has moved on to another caller's injection, this
        // one is over: waiting for the injector to be idle would report on a
        // press that is not ours.
        if (IsStillMine() && !Core::Input::Synthetic::IsIdle())
        {
            return Status::Running;
        }
        response = App::Control::EncodeResult(EncodedId(), m_encodedResult);
        return Status::Finished;
    }

private:
    [[nodiscard]] bool IsStillMine() const
    {
        return Core::Input::Synthetic::CurrentGeneration() == m_generation;
    }

    std::string_view m_name;
    std::string m_encodedResult;
    std::uint64_t m_generation;
};

// Absolute path for a caller-given one, so the answer names a file the
// caller can open regardless of the client's working directory.
std::filesystem::path ResolveScreenshotPath(const std::string& requested)
{
    std::error_code failure;
    const std::filesystem::path absolute = std::filesystem::absolute(requested, failure);
    return failure ? std::filesystem::path(requested) : absolute;
}
} // namespace

namespace App::Control::Commands
{
std::string_view CurrentSceneName()
{
    switch (SceneFlag)
    {
    case SERVER_LIST_SCENE:
        return "server_list";
    case WEBZEN_SCENE:
        return "webzen";
    case LOG_IN_SCENE:
        return "login";
    case LOADING_SCENE:
        return "loading";
    case CHARACTER_SCENE:
        return "character_list";
    case MAIN_SCENE:
        return "world";
    }
    return "unknown";
}

void SetBuildIdentifier(std::string identifier)
{
    if (identifier.empty() || identifier == UnknownVersion)
    {
        return;
    }
    MutableBuildIdentifier() = std::move(identifier);
}

const std::string& BuildIdentifier()
{
    return MutableBuildIdentifier();
}

std::string Ping(const Request& request, std::unique_ptr<Act>&)
{
    json result;
    result["build"] = BuildIdentifier();
    result["scene"] = CurrentSceneName();
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string Scene(const Request& request, std::unique_ptr<Act>&)
{
    json result;
    result["scene"] = CurrentSceneName();
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string State(const Request& request, std::unique_ptr<Act>&)
{
    json result;
    result["scene"] = CurrentSceneName();
    // The account is known from the moment the login is sent; before that
    // it is reported as empty rather than omitted, so a caller can read
    // the same field on every scene.
    result["account"] = Core::Text::ToUtf8(LogInID);

    if (SceneFlag != MAIN_SCENE)
    {
        // No character fields before the world is entered, by design: a
        // caller reads `scene` to know which other fields to expect.
        return EncodeResult(request.EncodedId(), result.dump());
    }

    return EncodeResult(request.EncodedId(), WorldStateObject());
}

std::string Nearby(const Request& request, std::unique_ptr<Act>&)
{
    json result;
    result["nearby"] = NearbyArray();
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string EventsSince(const Request& request, std::unique_ptr<Act>& act)
{
    int since = 0;
    (void)request.GetInt("since", since);

    bool follow = false;
    if (request.GetBool("follow", follow) && follow)
    {
        act = std::make_unique<EventStreamAct>(static_cast<std::uint64_t>(std::max(since, 0)));
        return {};
    }

    json events = json::array();
    for (const Events::Record& record : Events::Since(static_cast<std::uint64_t>(std::max(since, 0))))
    {
        events.push_back(EventObject(record));
    }

    json result;
    result["events"] = std::move(events);
    result["last_seq"] = Events::LastSequence();
    return EncodeResult(request.EncodedId(), result.dump());
}

std::string WaitFor(const Request& request, std::unique_ptr<Act>& act)
{
    std::string wanted;
    if (!request.GetString("event", wanted) || wanted.empty())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`wait-for` needs an event name");
    }

    int since = 0;
    if (!request.GetInt("since", since))
    {
        since = static_cast<int>(Events::LastSequence());
    }

    // `timeout` takes the same treatment as every integer argument: out of
    // range or not a number at all is a `bad_request`, not a silent default.
    // A negative one would expire before the watcher's first idle tick, and
    // an enormous one has no defined conversion to a millisecond count.
    double timeoutSeconds = DefaultWaitForSeconds;
    if (request.Has("timeout") && (!request.GetDouble("timeout", timeoutSeconds) ||
                                   !(timeoutSeconds > 0.0 && timeoutSeconds <= MaxWaitForSeconds)))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`timeout` is a positive number of seconds, at most " +
                               std::to_string(static_cast<int>(MaxWaitForSeconds)));
    }

    std::map<std::string, std::string> match;
    (void)request.GetStringMap("match", match);

    act = std::make_unique<WaitForAct>(std::move(wanted), std::move(match),
                                       static_cast<std::uint64_t>(std::max(since, 0)),
                                       std::chrono::milliseconds(static_cast<long long>(timeoutSeconds * 1000)));
    return {};
}

std::string Screenshot(const Request& request, std::unique_ptr<Act>& act)
{
    std::string requestedPath;
    std::wstring targetPath;
    if (request.GetString("out", requestedPath) && !requestedPath.empty())
    {
        targetPath = ResolveScreenshotPath(requestedPath).wstring();
    }

    auto state = std::make_shared<ScreenshotState>();
    if (!RequestScriptedScreenshot(targetPath,
                                   [state](const ScreenshotOutcome& outcome)
                                   {
                                       state->outcome = outcome;
                                       state->finished = true;
                                   }))
    {
        return EncodeError(request.EncodedId(), ErrorCode::Busy, "another screenshot is already being captured");
    }

    act = std::make_unique<ScreenshotAct>(std::move(state));
    return {};
}

std::string Hotkey(const Request& request, std::unique_ptr<Act>& act)
{
    std::string name;
    if (!request.GetString("key", name) || name.empty())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`hotkey` needs a key name");
    }

    const std::optional<int> virtualKey = Core::Input::Synthetic::VirtualKeyFromName(name);
    if (!virtualKey)
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "unknown key `" + name + "`; known: " + std::string(Core::Input::Synthetic::KeyNames()));
    }

    if (!Core::Input::Synthetic::PressKey(*virtualKey))
    {
        return EncodeError(request.EncodedId(), ErrorCode::Busy, "another key or click is still being injected");
    }

    json result;
    result["key"] = name;
    act = std::make_unique<SyntheticInputAct>("hotkey", result.dump());
    return {};
}

std::string ClickUi(const Request& request, std::unique_ptr<Act>& act)
{
    double windowX = 0.0;
    double windowY = 0.0;
    if (!request.GetDouble("x", windowX) || !request.GetDouble("y", windowY))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`click-ui` needs `x` and `y` window pixels");
    }

    // Bounded for the same reason `Request::GetInt` bounds its own reads:
    // the coordinates are cast to `float` below, and a double outside the
    // range of one has no defined conversion.
    if (!(std::abs(windowX) <= MaxWindowPixel && std::abs(windowY) <= MaxWindowPixel))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`x` and `y` are window pixels, at most " +
                               std::to_string(static_cast<int>(MaxWindowPixel)) + " from the origin");
    }

    std::string buttonName = "left";
    (void)request.GetString("button", buttonName);
    const std::optional<Core::Input::Synthetic::MouseButton> button =
        Core::Input::Synthetic::MouseButtonFromName(buttonName);
    if (!button)
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "unknown button `" + buttonName + "`; known: left, right");
    }

    if (!Core::Input::Synthetic::Click(static_cast<float>(windowX), static_cast<float>(windowY), *button))
    {
        return EncodeError(request.EncodedId(), ErrorCode::Busy, "another key or click is still being injected");
    }

    json result;
    result["x"] = windowX;
    result["y"] = windowY;
    result["button"] = buttonName;
    act = std::make_unique<SyntheticInputAct>("click-ui", result.dump());
    return {};
}
} // namespace App::Control::Commands
