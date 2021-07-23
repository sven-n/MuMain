#pragma once

#ifdef NEW_USER_INTERFACE_UTIL

namespace util
{
/*
	template<typename T>
	struct Build
	{
		virtual ~Build(void) = 0 {}

		void SetHandle( BoostWeak_Ptr(T) t )
		{
			m_Handler = t;
		}

		T& GetHandler()
		{
			assert( m_Handler.expired() );
			return (m_Handler.lock().get());
		}

	protected:
		BoostWeak_Ptr(T)		m_Handler;
	};
*/
	struct Build
	{
		virtual ~Build(void) = 0 {}
	};
};

#endif //NEW_USER_INTERFACE_UTIL