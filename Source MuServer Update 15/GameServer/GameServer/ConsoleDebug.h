#pragma once

#define CSK_LH_DEBUG_CONSOLE

enum MSG_TYPE
{
	MCD_SEND = 0x01,
	MCD_RECEIVE,
	MCD_ERROR,
	MCD_NORMAL
};

class CmuConsoleDebug
{
	bool	m_bInit = true;
public:
	virtual ~CmuConsoleDebug();
	static CmuConsoleDebug* GetInstance();

	void Write(int iType, const char* pStr, ...);

protected:
	CmuConsoleDebug();	//. ban to create instance
};

#define g_ConsoleDebug	CmuConsoleDebug::GetInstance()
