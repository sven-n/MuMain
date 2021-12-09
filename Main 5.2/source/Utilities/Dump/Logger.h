class CLogger
{
public:
	const static int MAXIMUM_TRY = 5;	// maximum try to open file

	CLogger();
	~CLogger();

	bool				Open();
	void				Close();
	bool				Write(LPCTSTR szLine);
	bool				WriteWithFormat(LPCTSTR szFormat, ...);

private:
	FILE				*m_pFile;	// log file pointer
	CRITICAL_SECTION	m_csLog;	// critical section for logger
	bool				m_bOpen;	// whether file is opened or not
};