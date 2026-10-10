#include "App/stdafx.h"

#include <doctest.h>

#include <cwchar>

#include "I18N/All.h"

TEST_CASE("experience text shows both 64-bit values in every locale [ui][i18n]")
{
    const std::string previousLocale = I18N::GetCurrentLocale();
    for (const char* locale : I18N::GetAvailableLocales())
    {
        CAPTURE(locale);
        I18N::SetLocale(locale);

        wchar_t text[128] = {};
        const int written = mu_swprintf(text, I18N::Game::EXPI64dI64d, static_cast<unsigned long long>(0),
                                        static_cast<unsigned long long>(5000000000ULL));
        CHECK(written > 0);
        CHECK(std::wcsstr(text, L"0") != nullptr);
        CHECK(std::wcsstr(text, L"5000000000") != nullptr);
        // glibc reads MSVC's %I64d as a 64-character field width; the text must stay compact.
        CHECK(std::wcsstr(text, L"   ") == nullptr);
    }
    I18N::SetLocale(previousLocale.c_str());
}
