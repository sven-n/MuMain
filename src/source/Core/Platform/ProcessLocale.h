#pragma once

namespace Core::Platform
{
// Adopts the user's locale for text and character handling while keeping number formatting and
// parsing in the "C" locale. RmlUi (and every strtof/printf-based parser in the client) reads
// "1.5" as 1 under a decimal-comma locale such as de_DE or ru_RU, which breaks RCSS layout.
// On macOS LC_CTYPE is UTF-8 also when the environment's locale is not (an app started from the
// Finder has none), as Apple's wide printf functions cannot write most languages otherwise.
void ApplyProcessLocale();
} // namespace Core::Platform
