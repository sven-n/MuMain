#if !defined(AFX_NEWUIDUELWINDOW_H__446BA52D_E675_4B70_8A9B_65A672B9FBEB__INCLUDED_)
#define AFX_NEWUIDUELWINDOW_H__446BA52D_E675_4B70_8A9B_65A672B9FBEB__INCLUDED_

#pragma once

#include "UI/Combat/DuelWindowRmlModel.h"
#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The duel score window. duel_window.rml draws it; C++ keeps its position and feeds the names
// and scores from g_DuelMgr.
class CDuelWindow : public CObject
{
private:
    enum
    {
        DUEL_WND_WIDTH = 131,
        DUEL_WND_HEIGHT = 70,
    };

public:
    CDuelWindow();
    virtual ~CDuelWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth(); //. 1.1f

    void ReloadRmlTheme();

private:
    void BuildRmlUi();
    void SyncRmlModel();

    CManager* m_pNewUIMng;
    POINT m_Pos;

    RmlModelBinder<DuelWindowRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
};
}

#endif // !defined(AFX_NEWUIDUELWINDOW_H__446BA52D_E675_4B70_8A9B_65A672B9FBEB__INCLUDED_)
