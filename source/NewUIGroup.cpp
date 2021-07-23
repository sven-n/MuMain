//=============================================================================
//
//	NewUIGroup.cpp
//
//	Copyright 2010 Webzen Mu-Studio
//
//=============================================================================
#pragma once
#include "StdAfx.h"

#ifdef KJW_ADD_NEWUI_GROUP
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

void CNewUIGroup::AddUIObj( CNewUIObj* pUIObj )
{
	m_vecUI.push_back(pUIObj);
}

bool CNewUIGroup::Render()
{
	if( IsVisible() == false )
		return false;

	type_vector_uibase::iterator vi = m_vecUI.begin();
	for(; vi != m_vecUI.end(); vi++)
	{
		if( (*vi)->IsVisible() == true ) 
		{
			(*vi)->Render();
		}
	}

	return true;
}

bool CNewUIGroup::Update()
{
	if( IsEnabled() == false )
		return false;

	type_vector_uibase::iterator vi = m_vecUI.begin();
	for(; vi != m_vecUI.end(); vi++)
	{
		if( (*vi)->IsEnabled() == true ) 
		{
			if((*vi)->Update() == false )
			{
				return false;
			}
		}
	}

	return true;
}

bool CNewUIGroup::UpdateMouseEvent()
{
	type_vector_uibase::iterator vi = m_vecUI.begin();
	
	for(; vi != m_vecUI.end(); vi++)
	{
		if((*vi)->IsVisible()) 
		{
			CNewUIObj *pUIObj = (*vi);
			pUIObj->UpdateMouseEvent();
// 			if( pUIObj->UpdateMouseEvent() == true )
// 				break;
		}
	}

	return true;
}

bool CNewUIGroup::UpdateKeyEvent()
{
	type_vector_uibase::iterator vi = m_vecUI.begin();
	for(; vi != m_vecUI.end(); vi++)
	{
		HWND hRelatedWnd = (*vi)->GetRelatedWnd();
		if(NULL == hRelatedWnd)
		{
			hRelatedWnd = g_hWnd;
		}
		
		HWND hWnd = GetFocus();
		
		if((*vi)->IsEnabled() && hWnd == hRelatedWnd)
		{
			CNewUIObj *pUIObj = (*vi);
			pUIObj->UpdateKeyEvent();
// 			if( pUIObj->UpdateKeyEvent() == true )
// 				break;
		}
	}

	return true;
}

void CNewUIGroup::Release()
{
	type_vector_uibase::iterator vi = m_vecUI.begin();
	for(; vi != m_vecUI.end(); vi++)
	{
		CNewUIObj* pUIObj = (*vi);
		SAFE_DELETE( pUIObj );
	}

	int iCount = 0;

	vi = m_vecUI.begin();
	for( ; vi < m_vecUI.end(); ++vi )
	{
		CNewUIObj* pUIObj = (*vi);
		if( pUIObj != NULL )
		{
#ifdef KWAK_ADD_TRACE_FUNC
			__TraceF( TEXT( "UI클래스 그룹 컴포넌트 중 삭제 안된것(인덱스:%d)이 있습니다. 종종 뮤DEAD의 원인이 됩니다. vecUI \n" ), iCount );
#else // KWAK_ADD_TRACE_FUNC
			char	szOutput[128];
			sprintf( szOutput, "UI클래스 그룹 컴포넌트 중 삭제 안된것(인덱스:%d)이 있습니다. 종종 뮤DEAD의 원인이 됩니다. vecUI \n", iCount );
			OutputDebugString( szOutput );
#endif // KWAK_ADD_TRACE_FUNC
		}
		iCount++;
	}

	m_vecUI.clear();
}

#endif // KJW_ADD_NEWUI_GROUP