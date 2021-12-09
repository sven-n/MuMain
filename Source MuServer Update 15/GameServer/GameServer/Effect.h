// Effect.h: interface for the CEffect class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CEffect
{
public:
	CEffect();
	virtual ~CEffect();
	void Clear();
	bool IsEffect();
	bool Set(BYTE index,BYTE group,BYTE save,BYTE type,BYTE flag,DWORD time,DWORD count,WORD value1,WORD value2,WORD value3,WORD value4);
public:
	BYTE m_index;
	BYTE m_group;
	BYTE m_save;
	BYTE m_type;
	BYTE m_flag;
	DWORD m_time;
	DWORD m_count;
	WORD m_value[4];
};
