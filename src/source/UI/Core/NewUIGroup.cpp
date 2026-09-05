//=============================================================================
//	NewUIGroup.cpp
//=============================================================================
#include "stdafx.h"
#include "UI/Core/NewUIGroup.h"

using namespace SEASON3B;
using namespace mu::ui::window;

// cppcheck-suppress uninitMemberVar
CGroup::CGroup()
{
    m_vecUI.clear();
}

CGroup::~CGroup()
{
    // cppcheck-suppress virtualCallInConstructor
    Release();
}

void CGroup::AddUIObj(CObject* pUIObj)
{
    m_vecUI.push_back(pUIObj);
}

bool CGroup::Render()
{
    if (IsVisible() == false)
        return false;

    auto vi = m_vecUI.begin();
    for (; vi != m_vecUI.end(); vi++)
    {
        if ((*vi)->IsVisible() == true)
        {
            (*vi)->Render();
        }
    }

    return true;
}

bool CGroup::Update()
{
    if (IsEnabled() == false)
        return false;

    auto vi = m_vecUI.begin();
    for (; vi != m_vecUI.end(); vi++)
    {
        if ((*vi)->IsEnabled() == true)
        {
            if ((*vi)->Update() == false)
            {
                return false;
            }
        }
    }

    return true;
}

bool CGroup::UpdateMouseEvent()
{
    auto vi = m_vecUI.begin();

    for (; vi != m_vecUI.end(); vi++)
    {
        if ((*vi)->IsVisible())
        {
            CObject* pUIObj = (*vi);
            pUIObj->UpdateMouseEvent();
            // 			if( pUIObj->UpdateMouseEvent() == true )
            // 				break;
        }
    }

    return true;
}

bool CGroup::UpdateKeyEvent()
{
    auto vi = m_vecUI.begin();
    for (; vi != m_vecUI.end(); vi++)
    {
        HWND hRelatedWnd = (*vi)->GetRelatedWnd();
        if (NULL == hRelatedWnd)
        {
            hRelatedWnd = g_hWnd;
        }

        HWND hWnd = GetFocus();

        if ((*vi)->IsEnabled() && hWnd == hRelatedWnd)
        {
            CObject* pUIObj = (*vi);
            pUIObj->UpdateKeyEvent();
            // 			if( pUIObj->UpdateKeyEvent() == true )
            // 				break;
        }
    }

    return true;
}

void CGroup::Release()
{
    auto vi = m_vecUI.begin();
    for (; vi != m_vecUI.end(); vi++)
    {
        CObject* pUIObj = (*vi);
        SAFE_DELETE(pUIObj);
    }

    int iCount = 0;

    vi = m_vecUI.begin();
    for (; vi < m_vecUI.end(); ++vi)
    {
        CObject* pUIObj = (*vi);
        if (pUIObj != NULL)
        {
            __TraceF(TEXT("vecUI \n"), iCount);
        }
        iCount++;
    }

    m_vecUI.clear();
}