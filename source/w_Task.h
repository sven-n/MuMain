// Task.h: interface for the Task class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#if !defined(AFX_TASK_H__866348D5_6815_4828_9F8C_447600D2D3B9__INCLUDED_)
#define AFX_TASK_H__866348D5_6815_4828_9F8C_447600D2D3B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Task  
{
public:
	enum eType 
	{
		eNone,
		eGame,
		eCount,
	};

public:
	Task( eType type );
	virtual ~Task();

public:
	const eType GetType() const;

public:
	eType			m_Type;

};

inline
const Task::eType Task::GetType() const
{
	return m_Type;
}

#endif // !defined(AFX_TASK_H__866348D5_6815_4828_9F8C_447600D2D3B9__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM
