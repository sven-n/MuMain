#pragma once

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

namespace input
{
enum eKeyType
{
	//문자키
    eKey_None,
    eKey_A,
    eKey_B,
    eKey_C,
    eKey_D,
    eKey_E,
    eKey_F,
    eKey_G,
    eKey_H,
    eKey_I,
    eKey_J,
    eKey_K,
    eKey_L,
    eKey_M,
    eKey_N,
    eKey_O,
    eKey_P,
    eKey_Q,
    eKey_R,
    eKey_S,
    eKey_T,
    eKey_U,
    eKey_V,
    eKey_W,
    eKey_X,
    eKey_Y,
    eKey_Z,

	//숫자키
    eKey_1,
    eKey_2,
    eKey_3,
    eKey_4,
    eKey_5,
    eKey_6,
    eKey_7,
    eKey_8,
    eKey_9,
    eKey_0,

	//특수키
    eKey_F1,
    eKey_F2,
    eKey_F3,
    eKey_F4,
    eKey_F5,
    eKey_F6,
    eKey_F7,
    eKey_F8,
    eKey_F9,
    eKey_F10,
    eKey_F11,
    eKey_F12,
    
	//기능키
    eKey_Escape,
    eKey_Space,
	eKey_Delete,

	//방향키
    eKey_Up,
    eKey_Down,
    eKey_Left,
    eKey_Right,

	//기능키
	eKey_Enter,
	eKey_Tab,

	//마우스
    eKey_LButton,
    eKey_MButton,
    eKey_RButton,
    eKey_Wheel,
    eKey_MouseMove,
};
	
enum eKeyStateType
{
    eKeyState_None,
    //키 상태
    eKeyState_Up,
    eKeyState_Down,
	ekeyState_UpDown,
    eKeyState_DblClk,
};

enum eIMEMessageType
{
    eIMEMessage_StartComposition,
    eIMEMessage_EndComposition,
    eIMEMessage_Composition,
    eIMEMessage_Complete,
    eIMEMessage_Char,
    eIMEMessage_Back,
    eIMEMessage_Left,
    eIMEMessage_Right,
    eIMEMessage_Escape,
    eIMEMessage_Delete,
    eIMEMessage_Enter,
	eIMEMessage_LShift,
	eIMEMessage_RShift,
	eIMEMessage_ShiftUp,
};

};

#endif //NEW_USER_INTERFACE_INPUTSYSTEM