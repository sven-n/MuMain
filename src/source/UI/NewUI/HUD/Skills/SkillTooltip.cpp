#include "stdafx.h"
#include "UI/NewUI/HUD/Skills/SkillTooltip.h"

#include "UI/NewUI/HUD/Skills/SkillTooltipModel.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"   // TextList / TextListColor / TextBold externs, STRP_*
#include "GameLogic/Pets/GIPetManager.h"
#include "UI/Legacy/UIControls.h"         // g_pRenderText macro

namespace UI::Skills::Tooltip
{

namespace
{
// Map the renderer-agnostic LineColor enum to the legacy TextList color
// constants used by RenderTipTextList.
int LegacyColor(LineColor c)
{
    switch (c)
    {
    case LineColor::White:    return TEXT_COLOR_WHITE;
    case LineColor::Blue:     return TEXT_COLOR_BLUE;
    case LineColor::Red:      return TEXT_COLOR_RED;
    case LineColor::DarkRed:  return TEXT_COLOR_DARKRED;
    }
    return TEXT_COLOR_WHITE;
}
}

void Render(int sx, int sy, int Type, int /*SkillNum*/, int iRenderPoint /*= STRP_NONE*/)
{
    // Pet command icons get a different UI entirely (delegated to giPetManager).
    if (giPetManager::RenderPetCmdInfo(sx, sy, Type)) return;

    const int skillType = CharacterAttribute->Skill[Type];

    BuildOptions options;
    options.skillType = skillType;
    options.skillSlotIndex = Type;
    options.includeCharacterSpecific = true;

    Model model;
    BuildModel(options, model);

    // Copy the model into the legacy TextList / Color / Bold buffers that
    // RenderTipTextList consumes. Pre-allocated globals, no heap.
    const int lineCount = (model.count < MAX_TOOLTIP_LINES) ? model.count : MAX_TOOLTIP_LINES;
    for (int i = 0; i < lineCount; ++i)
    {
        const Line& src = model.lines[i];
        wcscpy(TextList[i], src.text);
        TextListColor[i] = LegacyColor(src.color);
        TextBold[i] = src.isBold ? 1 : 0;
    }

    SIZE TextSize = { 0, 0 };
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[0], 1, &TextSize);

    if (iRenderPoint == STRP_NONE)
    {
        const int Height =
            ((model.count - model.skipCount) * TextSize.cy + model.skipCount * TextSize.cy / 2)
            / g_fScreenRate_y;
        sy -= Height;
    }

    RenderTipTextList(sx, sy, model.count, 0, RT3_SORT_CENTER, iRenderPoint);
}

}  // namespace UI::Skills::Tooltip
