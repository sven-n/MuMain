#ifndef _NEWUIMANAGER_H_
#define _NEWUIMANAGER_H_

#pragma once

#pragma warning(disable : 4786)
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "UI/Core/WindowObject.h"

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

        // Opt-in for Render()'s centralized RmlUiRuntime::RenderBackgroundLayer() call (see that
        // method, WindowManager.cpp). RmlUiRuntime's background context is a single app-lifetime
        // singleton shared by every CManager instance -- more than one instance exists
        // (CSceneUICoordinator's m_NewStyleMng is a second, scene-scoped one for login/character-
        // scene windows, separate from CSystem's app-lifetime m_pNewUIMng), but only windows
        // registered on m_pNewUIMng ever load documents into that background context. Defaults to
        // false so a new/other CManager instance doesn't silently start painting m_pNewUIMng's
        // windows' background docs into its own scene -- see SetDrivesBackgroundLayer().
        bool m_bDrivesBackgroundLayer = false;
    public:
        CManager();
        ~CManager();

        // Call once, right after constructing the one CManager instance that owns the windows
        // which actually load documents into RmlUiRuntime's background context (currently
        // CSystem::m_pNewUIMng only -- see m_bDrivesBackgroundLayer's own comment). Every other
        // CManager instance should leave this false.
        void SetDrivesBackgroundLayer(bool drives) { m_bDrivesBackgroundLayer = drives; }

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
