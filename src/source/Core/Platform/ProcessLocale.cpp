#include "ProcessLocale.h"

#include <clocale>
#ifdef __APPLE__
#include <cstring>
#include <langinfo.h>
#endif

namespace Core::Platform
{
void ApplyProcessLocale()
{
    std::setlocale(LC_ALL, "");
#ifdef __APPLE__
    // Apple's wide printf functions (swprintf, so mu_swprintf) write through LC_CTYPE's
    // multibyte encoding: in the "C" locale any character above U+00FF fails the whole call,
    // which leaves Japanese, Chinese, Russian, Ukrainian, Polish and other texts unwritten. An
    // app started from the Finder gets no LANG, so "" gives "C" there.
    if (std::strcmp(nl_langinfo(CODESET), "UTF-8") != 0 && std::setlocale(LC_CTYPE, "UTF-8") == nullptr)
    {
        std::setlocale(LC_CTYPE, "en_US.UTF-8");
    }
#endif
    std::setlocale(LC_NUMERIC, "C");
}
} // namespace Core::Platform
