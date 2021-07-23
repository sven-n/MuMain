//*****************************************************************************
// File: Observer.h
//
// Desc: interface for the CObserver, CSubject class.
//		 옵져버, 서브젝트 클래스.(옵져버 패턴)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_OBSERVER_H__D77AB996_491E_44AD_AE54_179DE70DFC3B__INCLUDED_)
#define AFX_OBSERVER_H__D77AB996_491E_44AD_AE54_179DE70DFC3B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//*****************************************************************************
// CObserver
// 다큐멘트/뷰 구조의 뷰라고 생각하면 됨.
//*****************************************************************************

class CSubject;

class CObserver  
{
public:
	virtual ~CObserver();

	// CSubject::Notify()에서 UpdateData()를 호출.
	virtual void UpdateData(CSubject* pChangedSubject) = 0;

protected:
	CObserver();
};


//*****************************************************************************
// CSubject
// 다큐멘트/뷰 구조의 다큐멘트라고 생각하면 됨.
//*****************************************************************************

#include "PList.h"

class CSubject  
{
public:
	virtual ~CSubject();

	virtual void Attach(CObserver* pObserver);
	virtual void Detach(CObserver* pObserver);
	virtual void Notify();

protected:
	CSubject();

private:
	CPList m_ObserverList;	// 옵져버 클래스 상속받은 오브젝트들의 포인터 리스트.
};

#endif // !defined(AFX_OBSERVER_H__D77AB996_491E_44AD_AE54_179DE70DFC3B__INCLUDED_)
