#include <doctest.h>

#include <array>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "Core/Platform/ProcessLocale.h"

namespace
{
constexpr const char* kLocaleVariable = "LC_ALL";
constexpr std::array<const char*, 4> kDecimalCommaLocales = {
    "de_DE.UTF-8", "ru_RU.UTF-8", "fr_FR.UTF-8", "pl_PL.UTF-8"};
constexpr float kSampleValue = 1000.5f;
constexpr const char* kSampleText = "1000.5";

void SetLocaleVariable(const char* value)
{
#ifdef _WIN32
    _putenv_s(kLocaleVariable, value ? value : "");
#else
    if (value)
    {
        setenv(kLocaleVariable, value, 1);
    }
    else
    {
        unsetenv(kLocaleVariable);
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
        SetLocaleVariable(decimalCommaLocale);
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
    SetLocaleVariable(previous ? saved.c_str() : nullptr);

    CHECK(parsed == kSampleValue);
    CHECK(formatted == kSampleText);
    CHECK(numericLocale == "C");
}
