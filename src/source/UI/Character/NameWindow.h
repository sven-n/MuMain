
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

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void PrepareBackgroundLayer() override;
        void Show(bool bShow) override;

        float GetLayerDepth();		// 1.0f

    private:
        // Everything this window draws; recorded into m_labelLayer (RmlUi) when it is available,
        // drawn natively otherwise.
        void RenderLabels();
        void RenderName();

        UI::Character::WorldLabelLayer m_labelLayer;

        CManager* m_pNewUIMng;		// UI manager
        POINT m_Pos;					// window position

        bool m_bShowItemName;
        bool m_bShowMonsterHealthBar;

        void RenderMonsterHealthBars();
    };
}

#endif // !defined(AFX_NEWUINAMEWINDOW_H__76B140FF_46CB_4DB6_9DA2_5F84F294D212__INCLUDED_)
