#include "stdafx.h"

#include "SoundNumbers.h"

#include <cwctype>
#include <unordered_map>
#include <vector>

namespace Audio::SoundNumbers
{
namespace
{
struct Numbers
{
    std::unordered_map<std::wstring, int> byFile;
    // The file of each number, from MAX_BUFFER on.
    std::vector<std::wstring> files;
};

Numbers& GetNumbers()
{
    static Numbers numbers;
    return numbers;
}

std::wstring FileKey(std::wstring_view file)
{
    std::wstring key(file);
    for (wchar_t& c : key)
    {
        c = c == L'\\' ? L'/' : static_cast<wchar_t>(std::towlower(c));
    }
    return key;
}
} // namespace

ESound ForFile(std::wstring_view file)
{
    Numbers& numbers = GetNumbers();
    const std::wstring key = FileKey(file);
    if (const auto found = numbers.byFile.find(key); found != numbers.byFile.end())
    {
        return static_cast<ESound>(found->second);
    }
    if (numbers.files.size() >= static_cast<size_t>(MAX_FILE_SOUNDS))
    {
        // MAX_BUFFER is the first file's number.
        return NoNumber;
    }
    const int number = static_cast<int>(MAX_BUFFER) + static_cast<int>(numbers.files.size());
    numbers.files.emplace_back(file);
    numbers.byFile.emplace(key, number);
    return static_cast<ESound>(number);
}

std::wstring FileOf(int number)
{
    const Numbers& numbers = GetNumbers();
    const int index = number - static_cast<int>(MAX_BUFFER);
    if (index < 0 || index >= static_cast<int>(numbers.files.size()))
    {
        return {};
    }
    return numbers.files[static_cast<size_t>(index)];
}

void ReleaseAll()
{
    const int count = static_cast<int>(GetNumbers().files.size());
    for (int index = 0; index < count; ++index)
    {
        ReleaseBuffer(static_cast<int>(MAX_BUFFER) + index);
    }
}
} // namespace Audio::SoundNumbers
