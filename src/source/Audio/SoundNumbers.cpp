#include "stdafx.h"

#include "SoundNumbers.h"

#include <cwctype>
#include <unordered_map>

namespace Audio::SoundNumbers
{
namespace
{
struct Numbers
{
    std::unordered_map<std::wstring, int> byFile;
    // The numbers given, from MAX_BUFFER on.
    int given = 0;
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
    if (numbers.given >= MAX_FILE_SOUNDS)
    {
        // MAX_BUFFER is the first file's number.
        return NoNumber;
    }
    const int number = static_cast<int>(MAX_BUFFER) + numbers.given;
    ++numbers.given;
    numbers.byFile.emplace(key, number);
    return static_cast<ESound>(number);
}

void ReleaseAll()
{
    const int count = GetNumbers().given;
    for (int index = 0; index < count; ++index)
    {
        ReleaseBuffer(static_cast<int>(MAX_BUFFER) + index);
    }
}
} // namespace Audio::SoundNumbers
