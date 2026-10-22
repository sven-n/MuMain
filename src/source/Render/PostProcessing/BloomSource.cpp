#include "Render/PostProcessing/BloomSource.h"

namespace Render::PostProcessing
{

bool IsBloomSource(bool blendEnabled, mu::BlendMode mode, bool excluded)
{
    if (!blendEnabled || excluded)
    {
        return false;
    }

    switch (mode)
    {
    case mu::BlendMode::Additive:
    case mu::BlendMode::Glow:
    case mu::BlendMode::Luminance:
        return true;
    default:
        return false;
    }
}

} // namespace Render::PostProcessing
