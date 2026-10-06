
#if !defined(AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_)
#define AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Events/KanturuEnterRmlModel.h"
#include "UI/Events/KanturuUpdates.h"
#include "UI/Events/KanturuInfoRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// Kanturu's entry window (the gateway NPC). kanturu_enter.rml draws it; C++ keeps the state
// texts, the button locks, the equipment checks, Escape and every request it sends.
class CKanturu2ndEnterNpc : public CObject
{
public:
    static constexpr float KANTURU2ND_ENTER_WINDOW_WIDTH = 230.0f;
    static constexpr float KANTURU2ND_ENTER_WINDOW_HEIGHT = 267.0f;

    enum MSGBOX_TYPE
    {
        POPUP_NONE = 0,
        POPUP_USER_OVER,
        POPUP_NOT_MUNSTONE,
        POPUP_FAILED,
        POPUP_FAILED2,
        POPUP_UNIRIA = 5,
        POPUP_CHANGERING,
        POPUP_NOT_HELPER,
    };

public:
    CKanturu2ndEnterNpc();
    virtual ~CKanturu2ndEnterNpc();

    bool Create(CManager* pNewUIMng, int x, int y);
    Rml::ElementDocument* GetFillDocument() const override { return m_RmlView.Document(); }
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth(); //. 10.1f


    void SetNpcObject(OBJECT* pObj);
    bool IsNpcAnimation();
    void SetNpcAnimation(bool bValue);
    bool IsEnterRequest();
    void SetEnterRequest(bool bValue);
    void CreateMessageBox(MSGBOX_TYPE result);

    void ReceiveKanturu3rdInfo(UI::Kanturu::Stage stage, UI::Kanturu::Detail detail, bool canEnter,
                                  BYTE userCount, int remainingSeconds);
    void ReceiveKanturu3rdEnter(UI::Kanturu::EntryResult result);
    void SendRequestKanturu3rdInfo();
    void SendRequestKanturu3rdEnter();

private:
    void Initialize();

    void ProcessRefresh();
    void ProcessEnter();

    void BuildRmlUi();
    void SyncRmlModel();
    void SyncContent();

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;

    BYTE m_byState;

    bool m_bNpcAnimation;
    OBJECT* m_pNpcObject;

    bool m_bEnterRequest;

    DWORD m_dwRefreshTime;
    DWORD m_dwRefreshButtonGapTime;

    wchar_t m_strSubject[MAX_GLOBAL_TEXT_STRING];
    wchar_t m_strStateText[KANTURU2ND_STATETEXT_MAX][MAX_GLOBAL_TEXT_STRING];
    int m_iStateTextNum;

    // The original's CButton locks: Enter by the server's answer, Refresh for a second after a
    // click.
    bool m_EnterLocked = false;
    bool m_RefreshLocked = false;

    void BindRmlModel(Rml::DataModelConstructor& c, KanturuEnterRmlModel& model);
    UI::RmlBridge::ThemedView<KanturuEnterRmlModel> m_RmlView{"kanturu_enter",
        [this](Rml::DataModelConstructor& c, KanturuEnterRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/kanturu_enter.rml"}}};
    bool m_PendingRefresh = false;
    bool m_PendingEnter = false;
    bool m_PendingClose = false;
};

// The Kanturu boss-battle HUD. kanturu_info.rml draws it; C++ keeps the counts, the time and
// when it is shown.
class CKanturuInfoWindow : public CObject
{
public:
    enum IMAGE_LIST
    {
        IMAGE_KANTURUINFO_WINDOW = BITMAP_KANTURU_INFO_BEGIN,
    };
    enum
    {
        KANTURUINFO_WINDOW_WIDTH = 99,
        KANTURUINFO_WINDOW_HEIGHT = 78,
    };

public:
    CKanturuInfoWindow();
    virtual ~CKanturuInfoWindow();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth();    //. 1.92f
    float GetKeyEventOrder(); //. 9.1f

    void SetTime(int iTimeLimit);


private:
    void BuildRmlUi();
    void SyncView();

private:
    CManager* m_pNewUIMng;
    POINT m_Pos;

    int m_iMinute;
    int m_iSecond;
    DWORD m_dwSyncTime;

    DWORD m_dwColonTime = 0; // the colon's last blink
    bool m_bColonVisible = true;
    static void BindRmlModel(Rml::DataModelConstructor& c, KanturuInfoRmlModel& model);
    // The original drew the HUD under every panel (layer depth 1.92): the document sits in the
    // background context, behind its other documents.
    UI::RmlBridge::ThemedView<KanturuInfoRmlModel> m_RmlView{"kanturu_info", BindRmlModel,
        {{"Data/Interface/RmlUi/kanturu_info.rml"}}};
};
} // namespace mu::ui::window

#endif // !defined(AFX_NEWUIKANTURU2NDENTERNPC_H__4CDE30B6_3570_47BA_9401_0EA282BA1949__INCLUDED_)
