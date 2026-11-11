#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

// File helpers shared by the data tests.
namespace TestFiles
{
inline std::string ReadWholeFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}
} // namespace TestFiles
