#pragma once

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

#include "stdafx.h"
#include "w_InputTypes.h"

namespace input
{
	struct InputMessageHandler
	{
		virtual ~InputMessageHandler(void) = 0{}
		virtual bool HandleInputMessage( eKeyType type, eKeyStateType state, int value ) = 0;
	};

	struct IMEMessageHandler
	{
		virtual ~IMEMessageHandler(void) = 0{}
		virtual void HandleIMEMessage( eIMEMessageType type, const string& str = string() ) = 0;
	};

};

#endif //NEW_USER_INTERFACE_INPUTSYSTEM