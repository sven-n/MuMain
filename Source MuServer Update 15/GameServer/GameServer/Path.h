// Path.h: interface for the CPath class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAIN_PATH "..\\Data\\"

class CPath
{
public:
	CPath();
	virtual ~CPath();
	void SetMainPath(char* path);
	char* GetFullPath(char* file);
private:
	char m_MainPath[100];
	char m_FullPath[100];
};

extern CPath gPath;
