#include "doctest.h"

#include "UI/Chat/IncomingFilters.h"

#include <string>
#include <utility>

using namespace UI::Chat;

namespace
{
// Removes the filter again, also when a check fails.
struct ScopedFilter
{
    int id;

    explicit ScopedFilter(IncomingFilter filter) : id(AddIncomingFilter(std::move(filter))) {}

    ~ScopedFilter()
    {
        RemoveIncomingFilter(id);
    }
};
} // namespace

TEST_CASE("A message without filters is shown as it came [chat][filter]")
{
    wchar_t text[16] = L"Welcome";

    CHECK(FilterIncoming(IncomingKind::System, L"", text));
    CHECK(std::wstring(text) == L"Welcome");
}

TEST_CASE("A filter can hide a message and sees its kind and sender [chat][filter]")
{
    const ScopedFilter filter([](IncomingMessage& message)
                              { return message.kind != IncomingKind::Whisper || message.sender != L"Spammer"; });
    wchar_t text[16] = L"buy now";

    CHECK_FALSE(FilterIncoming(IncomingKind::Whisper, L"Spammer", text));
    CHECK(FilterIncoming(IncomingKind::Whisper, L"Friend", text));
    CHECK(FilterIncoming(IncomingKind::System, L"", text));
}

TEST_CASE("Filters run in the order they were added until one hides [chat][filter]")
{
    int runsOfSecond = 0;
    const ScopedFilter first(
        [](IncomingMessage& message)
        {
            message.text += L"1";
            return message.text != L"hide1";
        });
    const ScopedFilter second(
        [&runsOfSecond](IncomingMessage& message)
        {
            ++runsOfSecond;
            message.text += L"2";
            return true;
        });
    wchar_t shown[16] = L"a";
    wchar_t hidden[16] = L"hide";

    CHECK(FilterIncoming(IncomingKind::System, L"", shown));
    CHECK(std::wstring(shown) == L"a12");
    CHECK_FALSE(FilterIncoming(IncomingKind::System, L"", hidden));
    CHECK(std::wstring(hidden) == L"hide");
    CHECK(runsOfSecond == 1);
}

TEST_CASE("A longer new text is cut to the receive buffer [chat][filter]")
{
    const ScopedFilter filter(
        [](IncomingMessage& message)
        {
            message.text = L"0123456789";
            return true;
        });
    wchar_t text[5] = L"x";

    CHECK(FilterIncoming(IncomingKind::Banner, L"", text));
    CHECK(std::wstring(text) == L"0123");
}

TEST_CASE("A removed filter does not run [chat][filter]")
{
    const int id = AddIncomingFilter([](IncomingMessage&) { return false; });
    RemoveIncomingFilter(id);
    wchar_t text[8] = L"hello";

    CHECK(FilterIncoming(IncomingKind::System, L"", text));
}
