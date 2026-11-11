// An open C file stream that closes itself.
#pragma once

#include <cstdio>
#include <memory>

namespace Core::Utilities
{
struct FileCloser
{
    void operator()(std::FILE* file) const noexcept
    {
        std::fclose(file);
    }
};

// A std::unique_ptr only calls the closer for a file that was opened.
using FileHandle = std::unique_ptr<std::FILE, FileCloser>;
} // namespace Core::Utilities
