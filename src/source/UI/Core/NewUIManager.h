#ifndef _NEWUIMANAGER_H_
#define _NEWUIMANAGER_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "UI/Core/NewUIBase.h"

namespace mu::ui::window
{
    class CManager
    {
        typedef std::vector<CObject*> type_vector_uibase;
        typedef std::map<DWORD, CObject*> type_map_uibase;

        type_vector_uibase	m_vecUI;		//. for rendering and updating
        type_map_uibase		m_mapUI;		//. for managing

        CObject* m_pActiveMouseUIObj, * m_pActiveKeyUIObj;
#ifdef PBG_MOD_STAMINA_UI
        int m_nShowUICnt;
#endif //PBG_MOD_STAMINA_UI
    public:
        CManager();
        ~CManager();

        void AddUIObj(DWORD dwKey, CObject* pUIObj);
        void RemoveUIObj(DWORD dwKey);
        void RemoveUIObj(CObject* pUIObj);
        void RemoveAllUIObjs();

        void ReleaseAllUIObj();

        CObject* FindUIObj(DWORD dwKey);
        CObject* FindUIObjByRelatedWnd(HWND hWnd) const;

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        CObject* GetActiveMouseUIObj();
        CObject* GetActiveKeyUIObj();
        void ResetActiveUIObj();

        bool IsInterfaceVisible(DWORD dwKey);
        bool IsInterfaceEnabled(DWORD dwKey);

        void ShowInterface(DWORD dwKey, bool bShow = true);

        void EnableInterface(DWORD dwKey, bool bEnable = true);
        void ShowAllInterfaces(bool bShow = true);
        void EnableAllInterfaces(bool bEnable = true);

#ifdef PBG_MOD_STAMINA_UI
        int GetShowUICnt();
#endif //PBG_MOD_STAMINA_UI

    protected:
        static bool CompareLayerDepth(IObject* pObj1, IObject* pObj2);
        static bool CompareLayerDepthReverse(IObject* pObj1, IObject* pObj2);
        static bool CompareKeyEventOrder(IObject* pObj1, IObject* pObj2);
    };
}

#endif // _NEWUIMANAGER_H_
