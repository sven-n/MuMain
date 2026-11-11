#include "stdafx.h"

#include "Render/Drawing/LegacyCalls.h"

#include "Render/Renderer/MuRenderer.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/GameMaps/GMHellas.h"

#include <bit>
#include <cstddef>
#include <vector>

namespace Render::Drawing
{
namespace
{
LegacyCallRecorder* s_recorder = nullptr;
// The marks of the open reorderable ranges, innermost last.
std::vector<std::size_t> s_reorderableMarks;
} // namespace

void SetLegacyCallRecorder(LegacyCallRecorder* recorder)
{
    s_recorder = recorder;
}

LegacyCallRecorder* ActiveLegacyCallRecorder()
{
    return s_recorder;
}

bool RecordsLegacyCall(LegacyCall call, int argument)
{
    if (s_recorder == nullptr)
        return false;
    s_recorder->RecordLegacyCall(call, argument);
    return true;
}

void MakeLegacyCall(LegacyCall call, int argument)
{
    switch (call)
    {
    case LegacyCall::BindTexture:
        BindTexture(argument);
        break;
    case LegacyCall::EnableDepthTest:
        EnableDepthTest();
        break;
    case LegacyCall::DisableDepthTest:
        DisableDepthTest();
        break;
    case LegacyCall::EnableDepthMask:
        EnableDepthMask();
        break;
    case LegacyCall::DisableDepthMask:
        DisableDepthMask();
        break;
    case LegacyCall::EnableCullFace:
        EnableCullFace();
        break;
    case LegacyCall::DisableCullFace:
        DisableCullFace();
        break;
    case LegacyCall::DisableTexture:
        DisableTexture(argument != 0);
        break;
    case LegacyCall::DisableAlphaBlend:
        DisableAlphaBlend();
        break;
    case LegacyCall::EnableAlphaTest:
        EnableAlphaTest(argument != 0);
        break;
    case LegacyCall::EnableAlphaBlend:
        EnableAlphaBlend();
        break;
    case LegacyCall::EnableAlphaBlendMinus:
        EnableAlphaBlendMinus();
        break;
    case LegacyCall::EnableAlphaBlend2:
        EnableAlphaBlend2();
        break;
    case LegacyCall::EnableAlphaBlend3:
        EnableAlphaBlend3();
        break;
    case LegacyCall::EnableAlphaBlend4:
        EnableAlphaBlend4();
        break;
    case LegacyCall::EnableLightMap:
        EnableLightMap();
        break;
    case LegacyCall::SetAlphaFuncRef:
        SetAlphaFuncRef(std::bit_cast<float>(argument));
        break;
    case LegacyCall::BeginSprite:
        BeginSprite();
        break;
    case LegacyCall::EndSprite:
        EndSprite();
        break;
    case LegacyCall::BeginReorderableDraws:
        if (!RecordsLegacyCall(call, argument))
            s_reorderableMarks.push_back(mu::GetRenderer().BeginReorderableDraws());
        break;
    case LegacyCall::EndReorderableDraws:
        if (!RecordsLegacyCall(call, argument) && !s_reorderableMarks.empty())
        {
            mu::GetRenderer().EndReorderableDraws(s_reorderableMarks.back());
            s_reorderableMarks.pop_back();
        }
        break;
    case LegacyCall::BeginCharacterDraws:
        if (!RecordsLegacyCall(call, argument))
            mu::GetRenderer().BeginCharacterDraws();
        break;
    case LegacyCall::EndCharacterDraws:
        if (!RecordsLegacyCall(call, argument))
            mu::GetRenderer().EndCharacterDraws();
        break;
    case LegacyCall::BeginNoSunCasters:
        if (!RecordsLegacyCall(call, argument))
            mu::GetRenderer().BeginNoSunCasters();
        break;
    case LegacyCall::EndNoSunCasters:
        if (!RecordsLegacyCall(call, argument))
            mu::GetRenderer().EndNoSunCasters();
        break;
    case LegacyCall::RenderTerrain:
        RenderTerrain(argument != 0);
        break;
    case LegacyCall::RenderWaterTerrain:
        RenderWaterTerrain();
        break;
    }
}
} // namespace Render::Drawing
