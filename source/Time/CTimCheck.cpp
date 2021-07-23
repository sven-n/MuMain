#include "stdafx.h"
#include "CTimCheck.h"

static CTimeCheck CTime;

CTimeCheck::CTimeCheck()
{

}

CTimeCheck::~CTimeCheck()
{

}

int CTimeCheck::CheckIndex(int index)
{
#ifdef _VS2008PORTING
	for(int i = 0; i < (int)stl_Time.size(); i++)
#else // _VS2008PORTING
	for(int i = 0; i < stl_Time.size(); i++)
#endif // _VS2008PORTING
	{
		if(stl_Time[i].iIndex == index)
			return i;
	}

	TimeCheck T;

	T.iIndex = index;
	T.iBackupTime = 0;
	T.bTimeCheck = true;
	
	stl_Time.push_back(T);

	return stl_Time.size()-1;
}

bool CTimeCheck::GetTimeCheck(int index, int DelayTime)
{
	int I = CheckIndex(index);

	int PresentTime = timeGetTime();
	
	if(stl_Time[I].bTimeCheck)
	{
		stl_Time[I].iBackupTime = PresentTime;
		stl_Time[I].bTimeCheck = false;
	}

	if(stl_Time[I].iBackupTime+DelayTime <= PresentTime)
	{
		stl_Time[I].bTimeCheck = true;
		return true;
	}

	return false;
}

void CTimeCheck::DeleteTimeIndex(int index)
{
#ifdef _VS2008PORTING
	for(int i = 0; i < (int)stl_Time.size(); i++)
#else // _VS2008PORTING
	for(int i = 0; i < stl_Time.size(); i++)
#endif // _VS2008PORTING
	{
		if(stl_Time[i].iIndex == index)
		{
			stl_Time.erase(stl_Time.begin()+i);
			break;
		}
	}
}