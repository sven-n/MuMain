#pragma once

#include "w_WindowMessageHandler.h"

namespace input
{
	struct TypedDevice : public WindowMessageHandler
	{
		virtual ~TypedDevice(void) = 0 {}
		virtual void    Reset() = 0;
		virtual void    Process( int delta ) = 0;   
		virtual void    Active( bool b ) = 0;
	};
};