#include "doctest.h"

#include "Core/Utilities/Log/ReadingLanguageLog.h"

using Core::Log::ReadingLanguage::Describe;
using Core::Log::ReadingLanguage::State;

TEST_CASE("The start-up line names the UI language and the item names [core][log]")
{
    CHECK(Describe(std::nullopt, State{"en", false}) == "> UI language = en, item names in English.");
    CHECK(Describe(std::nullopt, State{"de", true}) == "> UI language = de, item names translated.");
}

TEST_CASE("A later UI language switch gets a line of its own [core][log]")
{
    CHECK(Describe(State{"en", false}, State{"zh-CN", false}) == "> UI language changed to zh-CN.");
    CHECK(Describe(State{"zh-CN", true}, State{"en", true}) == "> UI language changed to en.");
}

TEST_CASE("Turning translated item names on or off gets a line of its own [core][log]")
{
    CHECK(Describe(State{"de", false}, State{"de", true}) == "> Item names now translated.");
    CHECK(Describe(State{"de", true}, State{"de", false}) == "> Item names now in English.");
    CHECK(Describe(State{"en", false}, State{"de", true}) == "> UI language changed to de, item names now translated.");
}

TEST_CASE("Applying the active language again writes nothing [core][log]")
{
    // The editor's language box calls I18N::SetLocale when the player picks the language that is already active.
    CHECK(Describe(State{"en", false}, State{"en", false}).empty());
    CHECK(Describe(State{"de", true}, State{"de", true}).empty());
}
