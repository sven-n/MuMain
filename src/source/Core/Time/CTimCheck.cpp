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
    for (int i = 0; i < (int)stl_Time.size(); i++)
    {
        if (stl_Time[i].iIndex == index)
            return i;
    }

    TimeCheck T;

    T.iIndex = index;
    T.iBackupTime = 0;
    T.bTimeCheck = true;

    stl_Time.push_back(T);

    return stl_Time.size() - 1;
}

bool CTimeCheck::GetTimeCheck(int index, int DelayTime)
{
    int I = CheckIndex(index);

    const auto presentTime = WorldTime;

    if (stl_Time[I].bTimeCheck)
    {
        stl_Time[I].iBackupTime = presentTime;
        stl_Time[I].bTimeCheck = false;
    }

    if (stl_Time[I].iBackupTime + DelayTime <= presentTime)
    {
        stl_Time[I].bTimeCheck = true;
        return true;
    }

    return false;
}

void CTimeCheck::DeleteTimeIndex(int index)
{
    for (int i = 0; i < (int)stl_Time.size(); i++)
    {
        if (stl_Time[i].iIndex == index)
        {
            stl_Time.erase(stl_Time.begin() + i);
            break;
        }
    }
}