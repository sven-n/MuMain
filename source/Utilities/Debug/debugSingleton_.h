#pragma once

#ifdef LDS_ADD_DEBUGINGMODULES


template <typename T> class CdebugSingleton_
{
	static T*		m_pThis;

public:

	CdebugSingleton_()
	{
		if (m_pThis) return;

		int offset = (int)(T*)1 - (int)(CdebugSingleton_<T>*)(T*)1;
		m_pThis = (T*)((int)this + offset);

	}

	virtual ~CdebugSingleton_()
	{
		if (!m_pThis) return;

		m_pThis = 0;
	}

	static T&	GetThis()
	{
		if (!m_pThis) 
		{
			m_pThis = new T();
		}
		return *m_pThis;
	}

	static T*	GetThis_()
	{
		if (!m_pThis) 
		{
			m_pThis = new T();
		}
		return m_pThis;
	}

	void DestroyThis()
	{
		if(m_pThis)
		{
			delete m_pThis;
		}

		m_pThis = 0;
	}
};

template <typename T> T* CdebugSingleton_ <T>::m_pThis;

#endif // LDS_ADD_DEBUGINGMODULES