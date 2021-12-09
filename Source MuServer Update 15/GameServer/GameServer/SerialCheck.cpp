// SerialCheck.cpp: implementation of the CSerialCheck class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SerialCheck.h"

CSerialCheck gSerialCheck[MAX_OBJECT];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSerialCheck::CSerialCheck() // OK
{
	this->Init();
}

CSerialCheck::~CSerialCheck() // OK
{

}

void CSerialCheck::Init() // OK
{
	this->m_RecvSerial = (BYTE)-1;
	this->m_SendSerial = 0;
}

bool CSerialCheck::CheckSerial(BYTE serial) // OK
{
	if((BYTE)(this->m_RecvSerial+1) == serial)
	{
		this->m_RecvSerial++;
		return 1;
	}

	return 0;
}

BYTE CSerialCheck::GetRecvSerial() // OK
{
	return this->m_RecvSerial;
}

BYTE CSerialCheck::GetSendSerial() // OK
{
	return this->m_SendSerial++;
}
