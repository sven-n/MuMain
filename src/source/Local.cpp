// Local.cpp: implementation of the Local
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Local.h"

bool CheckSpecialText(wchar_t* Text)
{
    for (auto* lpszCheck = (wchar_t*)Text; *lpszCheck; ++lpszCheck)
    {
        if (!(48 <= *lpszCheck && *lpszCheck < 58) && !(65 <= *lpszCheck && *lpszCheck < 91) && !(97 <= *lpszCheck && *lpszCheck < 123))
        {
            return true;
        }
    }

    return false;
}