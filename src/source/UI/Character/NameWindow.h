
#if !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
#define AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_

#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/Character/WorldLabelLayer.h"

namespace mu::ui::window
{
    // item name
    class CNameWindow : public CObject
    {
    public:
        CNameWindow();
        virtual ~CNameWindow();

        bool Create(CManager* pNewUIMng);
        void Release();


        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void PrepareFrame() override;
        void Show(bool bShow) override;

        // True when the world-label layer draws RenderInterface()'s overlays -- the party members'
        // HP bars over their heads, the siege crown switch lines and build-time bars, the Kanturu
        // result banner -- and the Kalima object labels (RenderObjectDescription()), recorded under
        // the name labels as the original drew them before them; the main scene then leaves them out.
        bool RecordsInterfaceOverlays() const;

        float GetLayerDepth();		// 1.0f

    private:
        // Everything this window draws; recorded into m_labelLayer (RmlUi) when it is available,
        // drawn natively otherwise.
        void RenderLabels();
        void RenderName();

        UI::Character::WorldLabelLayer m_labelLayer;

        CManager* m_pNewUIMng;		// UI manager


        bool m_bShowItemName;
        bool m_bShowMonsterHealthBar;

        void RenderMonsterHealthBars();
    };
}

#endif // !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
