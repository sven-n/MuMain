#include "stdafx.h"

#ifdef _EDITOR

#include "EffectBrowserAssets.h"

#include "Core/Text/Utf8.h"
#include "Data/DataHandler/LoadData.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"

namespace MuEditor::Effects
{
namespace
{
// BMD::Release clears the meshes; the file CLoadData remembers stays.
EffectAsset ProbeModel(int type)
{
    if (Models == nullptr || Models[type].NumMeshs <= 0 || Models[type].Meshs == nullptr)
        return {};
    return {true, Core::Text::ToUtf8(gLoadData.GetModelFile(type).c_str()), gLoadData.GetModelLoadWorld(type)};
}

// FindTexture returns null for a number nothing is loaded with (GetTexture
// would return an error texture instead).
EffectAsset ProbeTexture(int type)
{
    const BITMAP_t* bitmap = Bitmaps.FindTexture(static_cast<GLuint>(type));
    if (bitmap == nullptr)
        return {};
    return {true, Core::Text::ToUtf8(bitmap->FileName), Bitmaps.GetLoadWorld(static_cast<GLuint>(type))};
}
} // namespace

EffectAsset ProbeLoadedAsset(EffectAssetSlot slot, int type)
{
    switch (slot)
    {
    case EffectAssetSlot::Model:
        return ProbeModel(type);
    case EffectAssetSlot::Texture:
    case EffectAssetSlot::DefaultTexture:
        return ProbeTexture(type);
    case EffectAssetSlot::TextureChosenInCode:
        break;
    }
    return {};
}
} // namespace MuEditor::Effects

#endif // _EDITOR
