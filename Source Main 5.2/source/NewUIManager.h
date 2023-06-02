#ifndef _NEWUIMANAGER_H_
#define _NEWUIMANAGER_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "NewUIBase.h"

namespace SEASON3B
{
    class CNewUIManager
    {
        typedef std::vector<CNewUIObj*> type_vector_uibase;
        typedef std::map<DWORD, CNewUIObj*> type_map_uibase;

        type_vector_uibase	m_vecUI;		//. for rendering and updating
        type_map_uibase		m_mapUI;		//. for managing

        CNewUIObj* m_pActiveMouseUIObj, * m_pActiveKeyUIObj;
#ifdef PBG_MOD_STAMINA_UI
        int m_nShowUICnt;
#endif //PBG_MOD_STAMINA_UI
    public:
        CNewUIManager();
        ~CNewUIManager();

        void AddUIObj(DWORD dwKey, CNewUIObj* pUIObj);
        void RemoveUIObj(DWORD dwKey);
        void RemoveUIObj(CNewUIObj* pUIObj);
        void RemoveAllUIObjs();

        void ReleaseAllUIObj();

        CNewUIObj* FindUIObj(DWORD dwKey);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        CNewUIObj* GetActiveMouseUIObj();
        CNewUIObj* GetActiveKeyUIObj();
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
        static bool CompareLayerDepth(INewUIBase* pObj1, INewUIBase* pObj2);
        static bool CompareLayerDepthReverse(INewUIBase* pObj1, INewUIBase* pObj2);
        static bool CompareKeyEventOrder(INewUIBase* pObj1, INewUIBase* pObj2);
    };
}

#endif // _NEWUIMANAGER_H_