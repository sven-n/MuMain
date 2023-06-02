//=============================================================================
//	NewUIGroup.cpp
//=============================================================================
#pragma once
#include "StdAfx.h"
#include "NewUIGroup.h"

using namespace SEASON3B;

CNewUIGroup::CNewUIGroup()
{
    m_vecUI.clear();
}

CNewUIGroup::~CNewUIGroup()
{
    Release();
}

void CNewUIGroup::AddUIObj(CNewUIObj* pUIObj)
{
    m_vecUI.push_back(pUIObj);
}

bool CNewUIGroup::Render()
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

bool CNewUIGroup::Update()
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

bool CNewUIGroup::UpdateMouseEvent()
{
    auto vi = m_vecUI.begin();

    for (; vi != m_vecUI.end(); vi++)
    {
        if ((*vi)->IsVisible())
        {
            CNewUIObj* pUIObj = (*vi);
            pUIObj->UpdateMouseEvent();
            // 			if( pUIObj->UpdateMouseEvent() == true )
            // 				break;
        }
    }

    return true;
}

bool CNewUIGroup::UpdateKeyEvent()
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
            CNewUIObj* pUIObj = (*vi);
            pUIObj->UpdateKeyEvent();
            // 			if( pUIObj->UpdateKeyEvent() == true )
            // 				break;
        }
    }

    return true;
}

void CNewUIGroup::Release()
{
    auto vi = m_vecUI.begin();
    for (; vi != m_vecUI.end(); vi++)
    {
        CNewUIObj* pUIObj = (*vi);
        SAFE_DELETE(pUIObj);
    }

    int iCount = 0;

    vi = m_vecUI.begin();
    for (; vi < m_vecUI.end(); ++vi)
    {
        CNewUIObj* pUIObj = (*vi);
        if (pUIObj != NULL)
        {
            __TraceF(TEXT("vecUI \n"), iCount);
        }
        iCount++;
    }

    m_vecUI.clear();
}