
#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlElementTooltip.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "GameLogic/Buffs/BuffTooltipLines.h"

#include <cstdint>

#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    // RmlUi-backed active-buff strip, a variable-length list (unlike CMuHelperBar's fixed layout).
    // Right-clicking Infinity Arrow or Swell of Magic Power asks to cancel it, as the original did.
    class CBuffStrip : public CObject
    {
    public:
        CBuffStrip();
        virtual ~CBuffStrip();

        bool Create(CManager* pNewUIMng);
        void Release();

        // Vestigial -- RmlUi/CSS owns this widget's position now (.center-x in buff_strip.rml).
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 0.95f

        void OpenningProcess();
        void ClosingProcess();

        // Same MAIN_SCENE-only Update() gate and fix as CMuHelperBar::SyncDocVisibility() (MuHelperBar.h).
        void SyncDocVisibility(bool sceneAllowsShow);


    private:
        void BuildRmlUi();

        struct BuffEntry
        {
            // Pre-computed pixel grid-slot position, bound as plain values (no arithmetic in RML).
            // Full decorator string, e.g. "image(atlas1-23)", selecting a named @spritesheet rect
            // from buff_strip.rcss (one rect per 20x28 tile; BuildIconDecorator() in BuffStrip.cpp
            // generates the name). Bound wholesale via data-style-decorator.
            //
            // Note: an absolutely-positioned oversized child does NOT get clipped by RmlUi even
            // with overflow:hidden on its container (ContainerBox::Close() computes the clip rect
            // before ClosePositionedElements() places such children) -- don't retry a clipped-atlas
            // approach; named @spritesheet rects are the working mechanism.
            Rml::String decorator;
        };
        struct BuffStripRmlModel
        {
            std::vector<BuffEntry> buffs;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, BuffStripRmlModel& model);
        UI::RmlBridge::ThemedView<BuffStripRmlModel> m_RmlView{"buff_strip",
            [this](Rml::DataModelConstructor& c, BuffStripRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/buff_strip.rml"}}};

        CManager* m_pNewUIMng = nullptr;

        // The buffs shown, in slot order, for mapping a slot's event back to its buff.
        std::vector<int> m_ShownBuffs;
        // The hovered slot's buff, in the shared tooltip.
        UI::RmlBridge::ElementTooltip m_Tooltip;
        void SyncTooltip();
        // The hovered buff's value lines, kept until the buff, the server's values, the texts or the hero's
        // attributes change.
        struct ValueLinesCache
        {
            int buff = -1;
            std::uint32_t sentVersion = 0;
            std::uint32_t textsVersion = 0;
            GameLogic::Buffs::BuffCasterStats stats{};
            std::vector<GameLogic::Buffs::BuffTooltipLine> lines;
        };
        ValueLinesCache m_ValueLines;
        const std::vector<GameLogic::Buffs::BuffTooltipLine>& HoveredValueLines(eBuffState buff);

        void SyncRmlModel();
        void OnBuffRightClick(int slot);
    };
}
