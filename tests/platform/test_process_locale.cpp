#include <doctest.h>

#include <array>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <iterator>
#include <optional>
#include <string>
#ifdef __APPLE__
#include <langinfo.h>
#endif

#include "Core/Platform/ProcessLocale.h"

namespace
{
constexpr const char* kLocaleVariable = "LC_ALL";
// Where setlocale(LC_ALL, "") takes LC_CTYPE from; an app started from the macOS Finder has none
// of them.
constexpr std::array<const char*, 3> kLocaleVariables = {"LC_ALL", "LC_CTYPE", "LANG"};
constexpr std::array<const char*, 4> kDecimalCommaLocales = {
    "de_DE.UTF-8", "ru_RU.UTF-8", "fr_FR.UTF-8", "pl_PL.UTF-8"};
constexpr float kSampleValue = 1000.5f;
constexpr const char* kSampleText = "1000.5";

void SetLocaleVariable(const char* name, const char* value)
{
#ifdef _WIN32
    _putenv_s(name, value ? value : "");
#else
    if (value)
    {
        setenv(name, value, 1);
    }
    else
    {
        unsetenv(name);
    }
#endif
}

// The first decimal-comma locale this machine has installed, or nullptr when none is.
const char* FindDecimalCommaLocale()
{
    for (const char* name : kDecimalCommaLocales)
    {
        if (std::setlocale(LC_ALL, name) != nullptr)
        {
            std::setlocale(LC_ALL, "C");
            return name;
        }
    }
    return nullptr;
}

std::string FormatSample()
{
    char buffer[32] = {};
    std::snprintf(buffer, sizeof(buffer), "%.1f", static_cast<double>(kSampleValue));
    return buffer;
}
}

TEST_CASE("The process locale keeps numbers in the C locale [platform][locale]")
{
    const char* decimalCommaLocale = FindDecimalCommaLocale();
    const char* previous = std::getenv(kLocaleVariable);
    const std::string saved = previous ? previous : "";
    if (decimalCommaLocale)
    {
        SetLocaleVariable(kLocaleVariable, decimalCommaLocale);
    }
    else
    {
        MESSAGE("no decimal-comma locale installed; checking the C numeric locale only");
    }

    Core::Platform::ApplyProcessLocale();
    const float parsed = std::strtof(kSampleText, nullptr);
    const std::string formatted = FormatSample();
    const std::string numericLocale = std::setlocale(LC_NUMERIC, nullptr);

    std::setlocale(LC_ALL, "C");
    SetLocaleVariable(kLocaleVariable, previous ? saved.c_str() : nullptr);

    CHECK(parsed == kSampleValue);
    CHECK(formatted == kSampleText);
    CHECK(numericLocale == "C");
}

TEST_CASE("The process locale writes any language without a locale in the environment [platform][locale]")
{
    std::array<std::optional<std::string>, kLocaleVariables.size()> saved;
    for (size_t i = 0; i < kLocaleVariables.size(); ++i)
    {
        if (const char* value = std::getenv(kLocaleVariables[i]))
        {
            saved[i] = value;
        }
        SetLocaleVariable(kLocaleVariables[i], nullptr);
    }

    Core::Platform::ApplyProcessLocale();
    // Japanese and Cyrillic, above U+00FF: Apple's wide printf functions fail on them in the "C"
    // locale.
    wchar_t text[64] = {};
    const int written =
        std::swprintf(text, std::size(text), L"\u7D4C\u9A13\u5024\uFF1A%d / \u041E\u043F\u044B\u0442", 5);
#ifdef __APPLE__
    const std::string codeset = nl_langinfo(CODESET);
#endif

    std::setlocale(LC_ALL, "C");
    for (size_t i = 0; i < kLocaleVariables.size(); ++i)
    {
        SetLocaleVariable(kLocaleVariables[i], saved[i] ? saved[i]->c_str() : nullptr);
    }

    CHECK(written > 0);
    CHECK(std::wstring(text) == L"\u7D4C\u9A13\u5024\uFF1A5 / \u041E\u043F\u044B\u0442");
#ifdef __APPLE__
    CHECK(codeset == "UTF-8");
#endif
}
