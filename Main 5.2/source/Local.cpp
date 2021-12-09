// Local.cpp: implementation of the Local
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Local.h"

bool CheckSpecialText(char *Text)
{
	for ( unsigned char *lpszCheck = ( unsigned char*)Text; *lpszCheck; ++lpszCheck)
	{
		if ( 1 == _mbclen( lpszCheck))
		{
			if( !(48 <= *lpszCheck && *lpszCheck < 58) && !(65 <= *lpszCheck && *lpszCheck < 91) && !(97 <= *lpszCheck && *lpszCheck < 123) )
 			{
				return ( true);
			}
		}
		else
		{
			return ( true);
		}
	}
	return false;
}
