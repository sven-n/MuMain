#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewSubject.h"

#include <algorithm>
#include <cmath>

namespace MuEditor::Effects
{
namespace
{
constexpr int TextureSizeStep = 32;
constexpr int MinTextureSize = 64;
constexpr int MaxTextureSize = 2048;

int RoundTextureSide(float points, float scale)
{
    const int pixels = static_cast<int>(std::ceil(points * scale));
    const int stepped = (pixels + TextureSizeStep - 1) / TextureSizeStep * TextureSizeStep;
    return std::clamp(stepped, MinTextureSize, MaxTextureSize);
}

bool IsNotDrawnByGame(Data::Effects::EffectKind kind, const EffectStages& stages)
{
    return kind == Data::Effects::EffectKind::Effect && stages.render == RenderStage::NotDrawn &&
           !stages.drawnOnGround && !stages.drawnAfterCharacters;
}
} // namespace

PreviewSubject DescribePreviewSubject(Data::Effects::EffectKind kind, int type, EffectAssetSlot slot, bool loaded,
                                      const EffectStages& stages)
{
    if (slot == EffectAssetSlot::TextureChosenInCode)
        return {PreviewDraw::None, NoteTextureChosenInCode};
    if (!loaded)
        return {PreviewDraw::None, NoteNothingLoaded};
    switch (slot)
    {
    case EffectAssetSlot::Model:
    {
        std::uint16_t notes = IsNotDrawnByGame(kind, stages) ? NoteNotDrawnByGame : 0;
        if (IsWorldObjectSlot(slot, type))
            notes |= NoteWorldObjectSlot;
        return {PreviewDraw::Model, notes};
    }
    case EffectAssetSlot::DefaultTexture:
        return {PreviewDraw::Sprite, NoteCodeMayChooseTexture};
    case EffectAssetSlot::Texture:
    case EffectAssetSlot::TextureChosenInCode:
        break;
    }
    const bool onGround = kind == Data::Effects::EffectKind::Effect && stages.drawnOnGround;
    return {onGround ? PreviewDraw::GroundDecal : PreviewDraw::Sprite, 0};
}

std::vector<int> PreviewSubTypes(const std::vector<std::vector<int>>& variantSubTypes)
{
    const auto named = [&](int subType)
    {
        return std::any_of(variantSubTypes.begin(), variantSubTypes.end(), [subType](const std::vector<int>& subTypes)
                           { return std::find(subTypes.begin(), subTypes.end(), subType) != subTypes.end(); });
    };
    int other = 0;
    while (named(other))
        ++other;
    std::vector<int> subTypes = {other};
    for (const std::vector<int>& variant : variantSubTypes)
    {
        if (!variant.empty())
            subTypes.push_back(variant.front());
    }
    return subTypes;
}

PreviewTextureSize ChoosePreviewTextureSize(float width, float height, float scale)
{
    return {RoundTextureSide(width, scale), RoundTextureSide(height, scale)};
}
} // namespace MuEditor::Effects

#endif // _EDITOR
