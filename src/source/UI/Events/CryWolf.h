
#if !defined(AFX_NEWUICRYWOLF_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)
#define AFX_NEWUICRYWOLF_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Events/CryWolfRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The Crywolf HUD: the battle panel (altars, dark elves, Balgass, the time, the statue's
// shield), the ready-state notice and the end-of-battle result. crywolf.rml draws it; C++ keeps
// the state, the animations and the result dialog. The native images stay loaded for
// M34CryWolf1st's unused Render_Mvp_Interface(), which still draws through Render(...).
class CCryWolf : public CObject
{
public:
    enum IMAGE_LIST
    {
        IMAGE_MVP_INTERFACE = BITMAP_INTERFACE_CRYWOLF_BEGIN,
    };

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;
    int m_iHour;
    int m_iMinute;
    int m_iSecond;
    DWORD m_dwSyncTime;
    int m_icntTime;
    bool m_bTimeStart;

public:
    CCryWolf();
    virtual ~CCryWolf();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();
    bool Render(int Posx, int Posy, int nPosx, int nPosy, float u, float v, float su, float sv, int Index,
                bool Scale = false, bool StartScale = false, float Alpha = 1.f);

    float GetLayerDepth(); //. 10.0f

    void OpenningProcess();
    void ClosingProcess();
    void SetTime(int iHour, int iMinute);
    void InitTime();


private:
    void LoadImages();
    void UnloadImages();

    void BuildRmlUi();
    // Advances the result's animation and the time, like the original's Render() did each
    // frame, and syncs crywolf.rml.
    void SyncView();
    void SyncResult(CryWolfRmlModel& updated);
    void SyncHud(CryWolfRmlModel& updated);

    void BindRmlModel(Rml::DataModelConstructor& c, CryWolfRmlModel& model);
    // The original drew the HUD at layer depth 10, over the inventory, the chat and the other panels:
    // the document sits in the main context, pulled to the front when it is shown.
    UI::RmlBridge::ThemedView<CryWolfRmlModel> m_RmlView{"crywolf",
        [this](Rml::DataModelConstructor& c, CryWolfRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/crywolf.rml"}}};
};
}

#endif // !defined(AFX_NEWUIGUILDINFOWINDOW_H__AD267ADA_D799_4033_85B8_6B03E42EFB13__INCLUDED_)
