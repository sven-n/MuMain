// w_ClientSystem.h: interface for the ClientSystem class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#if !defined(AFX_W_CLIENTSYSTEM_H__BFCD8B0E_724D_490A_AA71_5E0DA827EC1B__INCLUDED_)
#define AFX_W_CLIENTSYSTEM_H__BFCD8B0E_724D_490A_AA71_5E0DA827EC1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_Task.h"
#include "w_Module.h"

BoostSmartPointer( ClientSystem );
class ClientSystem
{
public:
	static ClientSystemPtr Make();
	virtual ~ClientSystem();

public:
	void ChangeTask( Task::eType type );
    void RegisterModule( Module::eType type, bool overwrite = false );
	void RegisterMessageBoxModule( const MESSAGEBOXDATA& messageboxdata );

public:
	void UnregisterModule( Module::eType type );

public:
	const Task::eType GetCurTaskType() const;

private:
	void Initialize();
	void Destroy();
	ClientSystem();

private:
	typedef array< BoostSmart_Ptr(Module), Module::eCount > Module_Array;

private:
	Module_Array					m_CurModule;
	BoostSmart_Ptr(Task)			m_CurTask;
};

inline
const Task::eType ClientSystem::GetCurTaskType() const
{
	if( m_CurTask )
		return m_CurTask->GetType();
	else
		return Task::eNone;
}

#endif // !defined(AFX_W_CLIENTSYSTEM_H__BFCD8B0E_724D_490A_AA71_5E0DA827EC1B__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM