//*****************************************************************************
// File: Observer.h
//*****************************************************************************

#if !defined(AFX_OBSERVER_H__D77AB996_491E_44AD_AE54_179DE70DFC3B__INCLUDED_)
#define AFX_OBSERVER_H__D77AB996_491E_44AD_AE54_179DE70DFC3B__INCLUDED_

#pragma once

class CSubject;

class CObserver
{
public:
    virtual ~CObserver();
    virtual void UpdateData(CSubject* pChangedSubject) = 0;

protected:
    CObserver();
};

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
    CPList m_ObserverList;
};

#endif // !defined(AFX_OBSERVER_H__D77AB996_491E_44AD_AE54_179DE70DFC3B__INCLUDED_)
