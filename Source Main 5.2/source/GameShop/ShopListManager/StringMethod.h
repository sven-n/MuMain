#pragma once

#include "include.h"
#include <time.h>

class CStringMethod
{
public:
    CStringMethod();
    virtual ~CStringMethod();

    //static void ConvertStringToList(std::vector<int>& List, std::wstring strdata);
    //static void ConvertStringToList(std::vector<std::wstring>& List, std::wstring strdata);
    static void ConvertStringToDateTime(tm& datetime, std::wstring strdata);
};
