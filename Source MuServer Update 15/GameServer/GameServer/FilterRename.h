// Filter.h: interface for the CFilter class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_FILTERRENAME_SYNTAX 100

struct FILTERRENAME_INFO
{
	char label[11];
};

class CFilterRename
{
public:
	CFilterRename();
	virtual ~CFilterRename();
	void Init();
	void Load(char* path);
	void SetInfo(FILTERRENAME_INFO info);
	bool CheckFilter(char* text);
private:
	FILTERRENAME_INFO m_FilterInfo[MAX_FILTERRENAME_SYNTAX];
	int m_count;
};

extern CFilterRename gFilterRename;
