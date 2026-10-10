#include "doctest.h"

#include "Data/GameData/Common/LocalizedString.h"

using Data::LocalizedString;

TEST_CASE("Localized strings parse the OpenMU format [data]")
{
    const LocalizedString text = LocalizedString::Parse("Kris||pt=Cris||es=Kris espa\xC3\xB1ol");

    CHECK(text.GetNeutral() == "Kris");
    CHECK(text.Get("pt") == "Cris");
    CHECK(text.Get("es") == "Kris espa\xC3\xB1ol");
    CHECK(text.Get("en") == "Kris");
}

TEST_CASE("Localized strings fall back to the neutral text [data]")
{
    const LocalizedString text = LocalizedString::Parse("Kris||pt=Cris");

    CHECK(text.Get("de") == "Kris");
    CHECK(text.Get("") == "Kris");
}

TEST_CASE("Localized strings serialize sorted by locale [data]")
{
    LocalizedString text;
    text.Set("pt", "Cris");
    text.Set("en", "Kris");
    text.Set("es", "Kris");

    CHECK(text.Serialize() == "Kris||es=Kris||pt=Cris");
    CHECK(LocalizedString::Parse(text.Serialize()) == text);
}

TEST_CASE("Setting an empty translation removes it [data]")
{
    LocalizedString text = LocalizedString::Parse("Kris||pt=Cris");

    text.Set("pt", "");

    CHECK(text.GetTranslations().empty());
    CHECK(text.Serialize() == "Kris");
}

TEST_CASE("A plain string is a neutral text [data]")
{
    const LocalizedString text = LocalizedString::Parse("Kris");

    CHECK(text.GetNeutral() == "Kris");
    CHECK(text.GetTranslations().empty());
    CHECK_FALSE(text.IsEmpty());
    CHECK(LocalizedString::Parse("").IsEmpty());
}
