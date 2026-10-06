
#if !defined(AFX_NEWUICATAPULTWINDOW_H__064BC38C_5F26_4003_A6C7_7270A11DEEBF__INCLUDED_)
#define AFX_NEWUICATAPULTWINDOW_H__064BC38C_5F26_4003_A6C7_7270A11DEEBF__INCLUDED_

#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/Events/CatapultRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The castle siege catapult, docked right: the target areas of the attacking or defending
// side and Shoot. catapult.rml draws it; C++ keeps the chosen area, the corner close, Escape
// and the fire request (and the stone effects the server's answer starts).
class CCatapultWindow : public CObject
{
public:
    enum CATAPULT_TYPE
    {
        CATAPULT_ATTACK = 1,
        CATAPULT_DEFENSE = 2,
    };

public:
    CCatapultWindow();
    virtual ~CCatapultWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth(); //. 5.0f

    void OpenningProcess();
    void ClosingProcess();

    void Init(int iKey, int iType);
    void DoFire(int iKey, int iResult, int iType, int iPositionX, int iPositionY);
    void DoFireFixStartPosition(int iType, int iPositionX, int iPositionY);
    void SetCameraPos(float x = 0.f, float y = 0.f, float z = 0.f);
    void GetCameraPos(vec3_t& vPos);


private:
    bool BtnProcess();
    void BuildRmlUi();
    void SyncRmlModel();

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;

    // The chosen target area (the original's CCatapultGroupButton index), -1 for none; Shoot
    // is locked until one is chosen.
    int m_iAreaIndex = -1;
    bool m_bFireLocked = true;
    void BindRmlModel(Rml::DataModelConstructor& c, CatapultRmlModel& model);
    UI::RmlBridge::ThemedView<CatapultRmlModel> m_RmlView{"catapult",
        [this](Rml::DataModelConstructor& c, CatapultRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/catapult.rml"}}};
    int m_PendingArea = -1;
    bool m_PendingFire = false;
    bool m_PendingExit = false;

    int m_iType;
    int m_iNpcKey;
    vec3_t m_vCameraPos;
};
}

#endif // !defined(AFX_NEWUICATAPULTWINDOW_H__064BC38C_5F26_4003_A6C7_7270A11DEEBF__INCLUDED_)
