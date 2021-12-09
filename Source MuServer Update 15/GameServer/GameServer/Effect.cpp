// Effect.cpp: implementation of the CEffect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Effect.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEffect::CEffect() // OK
{
	this->Clear();
}

CEffect::~CEffect() // OK
{

}

void CEffect::Clear() // OK
{
	this->m_index = 0xFF;
	this->m_group = 0;
	this->m_save = 0;
	this->m_type = 0;
	this->m_flag = 0;
	this->m_time = 0;
	this->m_count = 0;
	this->m_value[0] = 0;
	this->m_value[1] = 0;
	this->m_value[2] = 0;
	this->m_value[3] = 0;
}

bool CEffect::IsEffect() // OK
{
	if(this->m_index == 0xFF)
	{
		return 0;
	}

	return 1;
}

bool CEffect::Set(BYTE index,BYTE group,BYTE save,BYTE type,BYTE flag,DWORD time,DWORD count,WORD value1,WORD value2,WORD value3,WORD value4) // OK
{
	if(this->IsEffect() != 0)
	{
		return 0;
	}

	if(type == 2 && ((int)(count=((DWORD)difftime((time=count),::time(0))))) <= 0)
	{
		return 0;
	}

	this->m_index = index;
	this->m_group = group;
	this->m_save = save;
	this->m_type = type;
	this->m_flag = flag;
	this->m_time = time;
	this->m_count = count;
	this->m_value[0] = value1;
	this->m_value[1] = value2;
	this->m_value[2] = value3;
	this->m_value[3] = value4;
	return 1;
}
