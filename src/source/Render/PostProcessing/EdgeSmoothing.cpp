#include "Render/PostProcessing/EdgeSmoothing.h"

#include "Core/Text/NumberParse.h"

#include <cwctype>
#include <string>

namespace Render::PostProcessing
{

bool UsesPostProcessedFrame(bool postProcessing, bool edgeSmoothing)
{
    return postProcessing || edgeSmoothing;
}

mu::PostProcessingLook CompositedLook(bool postProcessing, const mu::PostProcessingLook& look)
{
    if (postProcessing)
    {
        return look;
    }
    mu::PostProcessingLook neutral = look;
    neutral.bloomStrength = 0.0f;
    neutral.gradingStrength = 0.0f;
    neutral.ambientOcclusionStrength = 0.0f;
    return neutral;
}

std::optional<bool> ParseEdgeSmoothingCommand(std::wstring_view argument)
{
    std::wstring word(Core::Text::TrimWhitespace(argument));
    for (wchar_t& character : word)
    {
        character = static_cast<wchar_t>(std::towlower(static_cast<wint_t>(character)));
    }
    if (word == L"on")
    {
        return true;
    }
    if (word == L"off")
    {
        return false;
    }
    return std::nullopt;
}

} // namespace Render::PostProcessing
