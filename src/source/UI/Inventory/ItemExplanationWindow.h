
#if !defined(AFX_NEWUIITEMEXPLANATIONWINDOW_H__4029DCB0_6E92_4032_A68B_CE62B878F615__INCLUDED_)
#define AFX_NEWUIITEMEXPLANATIONWINDOW_H__4029DCB0_6E92_4032_A68B_CE62B878F615__INCLUDED_

#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/Inventory/TipTextListView.h"

namespace mu::ui::window
{
// The item help window (/<item name> in chat): the item's levels table, drawn through
// RmlUi from the RenderTipTextList() calls the original made (TipTextListView).
class CItemExplanationWindow : public CObject
{
public:
    CItemExplanationWindow();
    virtual ~CItemExplanationWindow();

    bool Create(CManager* pNewUIMng);
    void Release();


    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth();    //. 6.5f
    float GetKeyEventOrder(); // 10.f;

    void OpenningProcess();
    void ClosingProcess();

private:
    // What the original's Render() drew with RenderTipTextList(), laid out for the document.
    // item_explanation.rml.
    void RecordTable(TipTextListRecord& record);

    CManager* m_pNewUIMng;

    TipTextListView m_View{"item_explanation", "Data/Interface/RmlUi/item_explanation.rml"};
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIITEMEXPLANATIONWINDOW_H__4029DCB0_6E92_4032_A68B_CE62B878F615__INCLUDED_)
