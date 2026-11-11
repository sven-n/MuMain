//************************************************************************
//
// FILE: StringMethod.cpp
// Removed #ifdef _WIN32 guard (Story 7.6.6)
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "StringMethod.h"

CStringMethod::CStringMethod() {}

CStringMethod::~CStringMethod() {}

void CStringMethod::ConvertStringToDateTime(tm& datetime, std::wstring strdata)
{
    if (strdata.length() >= 4)
    {
        datetime.tm_year = _wtoi(strdata.substr(0, 4).c_str()) - 1900;
    }

    if (strdata.length() >= 6)
    {
        datetime.tm_mon = _wtoi(strdata.substr(4, 2).c_str()) - 1;
    }

    if (strdata.length() >= 8)
    {
        datetime.tm_mday = _wtoi(strdata.substr(6, 2).c_str());
    }

    if (strdata.length() >= 10)
    {
        datetime.tm_hour = _wtoi(strdata.substr(8, 2).c_str());
    }

    if (strdata.length() >= 12)
    {
        datetime.tm_min = _wtoi(strdata.substr(10, 2).c_str());
    }

    if (strdata.length() >= 14)
    {
        datetime.tm_sec = _wtoi(strdata.substr(12, 2).c_str());
    }

    mktime(&datetime);
}
#endif
