#include "Antialiasing.h"

#include <cwctype>
#include <string>

namespace Render::Antialiasing
{

int RequestedSampleCount(int configured)
{
    if (configured >= MaxSampleCount)
    {
        return MaxSampleCount;
    }
    if (configured >= 4)
    {
        return 4;
    }
    if (configured >= 2)
    {
        return 2;
    }
    return OffSampleCount;
}

std::optional<int> ParseCommandArgument(std::wstring_view argument)
{
    const auto first = argument.find_first_not_of(L" \t");
    if (first == std::wstring_view::npos)
    {
        return std::nullopt;
    }
    const auto last = argument.find_last_not_of(L" \t");
    std::wstring word(argument.substr(first, last - first + 1));
    for (wchar_t& character : word)
    {
        character = static_cast<wchar_t>(std::towlower(static_cast<wint_t>(character)));
    }

    if (word == L"off" || word == L"0" || word == L"1")
    {
        return OffSampleCount;
    }
    if (word == L"2")
    {
        return 2;
    }
    if (word == L"4")
    {
        return 4;
    }
    if (word == L"8")
    {
        return 8;
    }
    return std::nullopt;
}

int SelectSampleCount(int requested, SupportedSampleCounts supported)
{
    if (requested >= 8 && supported.eight)
    {
        return 8;
    }
    if (requested >= 4 && supported.four)
    {
        return 4;
    }
    if (requested >= 2 && supported.two)
    {
        return 2;
    }
    return OffSampleCount;
}

int HighestSupportedSampleCount(SupportedSampleCounts supported)
{
    return SelectSampleCount(MaxSampleCount, supported);
}

bool UsesAlphaToCoverage(const CutoutDraw& draw)
{
    if (!draw.multisampled || !draw.alphaTest || !draw.depthTest || !draw.depthWrite || !draw.enhancedTexture)
    {
        return false;
    }
    switch (draw.blending)
    {
    case Blending::Off:
        return true;
    case Blending::Alpha:
        return draw.opaqueVertices;
    case Blending::Other:
    default:
        return false;
    }
}

} // namespace Render::Antialiasing
