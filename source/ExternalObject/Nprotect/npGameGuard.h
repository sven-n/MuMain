//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __NPGAMEGUARD_H__
#define __NPGAMEGUARD_H__

#ifdef NP_GAME_GUARD

///////////////////////////////////////
//  INCLUDE.
///////////////////////////////////////
#include "Winmain.h"
#include "NPGameLib.h"

namespace npGameGuard
{
    bool    init ( void );
    void    SetHwnd ( HWND hWnd );
    void    CheckGameMon ( void );
    void    loginID ( char* strID );
    void    showErrorMessage ( HWND hWnd, DWORD dwResult );
}


#ifdef GAMEGUARD_AUTH25
BOOL    CALLBACK NPGameMonCallback ( DWORD dwMsg, DWORD dwArg );
#else
bool    CALLBACK NPGameMonCallback ( DWORD dwMsg, DWORD dwArg );
#endif //GAMEGUARD_AUTH25

using namespace npGameGuard;


#endif

#endif