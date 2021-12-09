// KanturuObjInfo.h: interface for the CKanturuObjInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_KANTURU_OBJ_INDEX 800

class CKanturuObjInfo
{
public:
	CKanturuObjInfo();
	virtual ~CKanturuObjInfo();
	void Reset();
	bool AddObj(int aIndex);
	int GetCount();
public:
	int m_ObjCount;
	int m_ObjIndex[MAX_KANTURU_OBJ_INDEX];
};
