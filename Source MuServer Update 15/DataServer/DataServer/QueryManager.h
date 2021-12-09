// QueryManager.h: interface for the CQueryManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_COLUMNS 100

class CQueryManager
{
public:
	CQueryManager();
	virtual ~CQueryManager();
	bool Connect(char* odbc,char* user,char* pass);
	void Disconnect();
	void Diagnostic(char* query);
	bool ExecQuery(char* query,...);
	void Close();
	SQLRETURN Fetch();
	int FindIndex(char* ColName);
	int GetResult(int index);
	int GetAsInteger(char* ColName);
	float GetAsFloat(char* ColName);
	__int64 GetAsInteger64(char* ColName);
	void GetAsString(char* ColName,char* OutBuffer,int OutBufferSize);
	void GetAsBinary(char* ColName,BYTE* OutBuffer,int OutBufferSize);
	void BindParameterAsString(int ParamNumber,void* InBuffer,int ColumnSize);
	void BindParameterAsBinary(int ParamNumber,void* InBuffer,int ColumnSize);
	void ConvertStringToBinary(char* InBuff,int InSize,BYTE* OutBuff,int OutSize);
	void ConvertBinaryToString(BYTE* InBuff,int InSize,char* OutBuff,int OutSize);
private:
	SQLHANDLE m_SQLEnvironment;
	SQLHANDLE m_SQLConnection;
	SQLHANDLE m_STMT;
	char m_odbc[32];
	char m_user[32];
	char m_pass[32];
	SQLINTEGER m_RowCount;
	SQLSMALLINT m_ColCount;
	SQLCHAR m_SQLColName[MAX_COLUMNS][30];
	char m_SQLData[MAX_COLUMNS][8192];
	SQLINTEGER m_SQLDataLen[MAX_COLUMNS];
	SQLINTEGER m_SQLBindValue[MAX_COLUMNS];
};

extern CQueryManager gQueryManager;
