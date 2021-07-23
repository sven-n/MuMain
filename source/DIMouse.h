//-----------------------------------------------------------------------------
// File: MouseNon.h
//
// Desc: Header file for for DirectInput sample
//
// Copyright (c) 1997-1999 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef MOUSE_NON_H
#define MOUSE_NON_H

#include <dinput.h>

//-----------------------------------------------------------------------------
// External function-prototypes
//-----------------------------------------------------------------------------
extern HRESULT InitDirectInput( HWND hDlg );
extern HRESULT SetAcquire();
extern HRESULT FreeDirectInput();
extern HRESULT UpdateInputState();

extern BOOL g_bActive;		
extern HINSTANCE g_hInst;		

extern int	DIMouseX;
extern int	DIMouseY;
extern int	DIMouseWheel;
extern bool	DIMouseLButton;
extern bool	DIMouseRButton;

#endif // !defined(MOUSE_NON_H)
