//////////////////////////////////////////////////////////////////////
// muDebugHelper.h: interface for the CmuConsoleDebug class.
//////////////////////////////////////////////////////////////////////

#pragma once

enum MSG_TYPE
{
	MCD_SEND = 0x01,
	MCD_RECEIVE,
	MCD_ERROR,
	MCD_NORMAL
};

class CmuConsoleDebug  
{
	bool	m_bInit;
public:
	virtual ~CmuConsoleDebug();
	static CmuConsoleDebug* GetInstance();

	void UpdateMainScene();
	bool CheckCommand(const std::string& strCommand);
	void Write(int iType, const char* pStr, ...);

protected:
	CmuConsoleDebug();	//. ban to create instance
};

#define g_ConsoleDebug	CmuConsoleDebug::GetInstance()

class CmuSimpleLog
{
public:
	CmuSimpleLog(void);
	~CmuSimpleLog(void);
	static CmuSimpleLog* getInstance() { static CmuSimpleLog sInstance; return &sInstance; }
	
	void setFilename(const char* strFilename);
	void log(char *str, ...);
	
private:
	bool m_bLogfirst;
	
	std::string m_strFilename;
	
	FILE *m_pFile;
};

#define g_ConsoleDebugLog	CmuSimpleLog::getInstance()


