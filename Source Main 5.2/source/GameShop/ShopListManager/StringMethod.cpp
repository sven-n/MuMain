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

void CStringMethod::ConvertStringToDateTime(tm& datetime,std::string strdata) // OK
{
	if(strdata.length()>=4)
	{
		datetime.tm_year = atoi(strdata.substr(0,4).c_str())-1900;
	}

	if(strdata.length()>=6)
	{
		datetime.tm_mon = atoi(strdata.substr(4,2).c_str())-1;
	}

	if(strdata.length()>=8)
	{
		datetime.tm_mday = atoi(strdata.substr(6,2).c_str());
	}

	if(strdata.length()>=10)
	{
		datetime.tm_hour = atoi(strdata.substr(8,2).c_str());
	}

	if(strdata.length()>=12)
	{
		datetime.tm_min = atoi(strdata.substr(10,2).c_str());
	}

	if(strdata.length()>=14)
	{
		datetime.tm_sec = atoi(strdata.substr(12,2).c_str());
	}

	mktime(&datetime);
}
#endif