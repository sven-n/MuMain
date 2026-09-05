// NewUIHelpWindow.h: interface for the CHelpWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_)
#define AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_

#pragma once

#include "UI/Core/NewUIManager.h"

namespace mu::ui::window
{
class CHelpWindow : public CObject
{
public:
    CHelpWindow();
    virtual ~CHelpWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth();    //. 7.1f
    float GetKeyEventOrder(); // 10.f;

    void OpenningProcess();
    void ClosingProcess();

    void AutoUpdateIndex();

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;

    int m_iIndex;
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIHELPWINDOW_H__9A918DE0_7707_456C_9E5B_89503F1936D1__INCLUDED_)
