#pragma once

#include <string>

// The `$bench ...` console/chat commands. Kept out of muConsoleDebug.cpp's dispatch chain so the
// benchmark's own vocabulary lives with the rest of the benchmark.

namespace Core::Benchmark::Console
{
    // Returns true when the text was a $bench command and has been handled.
    bool HandleCommand(const std::wstring& command);
}
