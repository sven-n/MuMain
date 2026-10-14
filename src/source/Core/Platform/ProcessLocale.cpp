#include "ProcessLocale.h"

#include <clocale>

namespace Core::Platform
{
void ApplyProcessLocale()
{
    std::setlocale(LC_ALL, "");
    std::setlocale(LC_NUMERIC, "C");
}
} // namespace Core::Platform
