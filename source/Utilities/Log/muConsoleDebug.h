// muDebugHelper.h: interface for the CmuConsoleDebug class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MUDEBUGHELPER_H__B8E3064A_5158_4124_8177_01D96D619220__INCLUDED_)
#define AFX_MUDEBUGHELPER_H__B8E3064A_5158_4124_8177_01D96D619220__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef CSK_LH_DEBUG_CONSOLE


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
	bool m_bLogfirst;		// 로그 시작이냐?
	
	std::string m_strFilename;	// 파일 이름
	
	FILE *m_pFile;			// 로그 남기는 파일
};

#define g_ConsoleDebugLog	CmuSimpleLog::getInstance()

#endif // CSK_LH_DEBUG_CONSOLE

#endif // !defined(AFX_MUDEBUGHELPER_H__B8E3064A_5158_4124_8177_01D96D619220__INCLUDED_)
