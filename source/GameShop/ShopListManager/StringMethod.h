
#pragma once

#include "include.h"
#include <time.h>

class CStringMethod
{
public:
	CStringMethod();
	virtual ~CStringMethod();

	//static void ConvertStringToList(std::vector<int>& List, std::string strdata);
	//static void ConvertStringToList(std::vector<std::string>& List, std::string strdata);
	static void ConvertStringToDateTime(tm& datetime, std::string strdata);
};
