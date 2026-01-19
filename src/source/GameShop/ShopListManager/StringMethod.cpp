//************************************************************************
//
// Decompiled by @myheart, @synth3r
// <https://forum.ragezone.com/members/2000236254.html>
//
//
// FILE: StringMethod.cpp
//
//

#include "stdafx.h"
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "StringMethod.h"

CStringMethod::CStringMethod() // OK
{
}

CStringMethod::~CStringMethod() // OK
{
}

void CStringMethod::ConvertStringToDateTime(tm& datetime, std::wstring strdata) // OK
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