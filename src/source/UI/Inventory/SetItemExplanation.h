
#if !defined(AFX_NEWUISETITEMEXPLANATION_H__31F3D8C3_34A7_45F8_BEC6_A915E8B5B6BF__INCLUDED_)
#define AFX_NEWUISETITEMEXPLANATION_H__31F3D8C3_34A7_45F8_BEC6_A915E8B5B6BF__INCLUDED_

#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/Inventory/ItemHelpView.h"

namespace mu::ui::window
{
// The set item help window (/<set name> in chat): the set's options, listed as the inventory
// tooltip lists them, in RmlUi (ItemHelpView).
class CSetItemExplanation : public CObject
{
public:
    CSetItemExplanation();
    virtual ~CSetItemExplanation();

    bool Create(CManager* pNewUIMng);
    void Release();


    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth();    //. 6.6f
    float GetKeyEventOrder(); // 10.f;

    void OpenningProcess();
    void ClosingProcess();

private:
    CManager* m_pNewUIMng;

    ItemHelpView m_View{"set_item_explanation", "Data/Interface/RmlUi/set_item_explanation.rml"};
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUISETITEMEXPLANATION_H__31F3D8C3_34A7_45F8_BEC6_A915E8B5B6BF__INCLUDED_)
