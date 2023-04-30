// StreamPacketEngine.h: interface for the CStreamPacketEngine class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_SPE_BUFFERSIZE	(2048)

class CStreamPacketEngine  
{
public:
	CStreamPacketEngine()
	{
		Clear();
	}
	virtual ~CStreamPacketEngine()	{}
	void Clear( void)
	{
		m_wSize = 0;
	}

protected:
	WORD m_wSize;
	BYTE m_byBuffer[MAX_SPE_BUFFERSIZE];

	// Generating packet
public:
	__forceinline void Init(BYTE byType, BYTE byHeadCode)
	{
		Clear();
		AddData( &byType, sizeof ( BYTE), FALSE);
		switch ( byType)
		{
		case 0xC1:
		case 0xC3:
			AddData( &m_wSize, 1, FALSE);
			break;
		case 0xC2:
		case 0xC4:
			AddData( &m_wSize, 2, FALSE);
			break;
		default:
			assert( "StreamPacketEngine type error(Start)");
			return;
		}
		AddData( &byHeadCode, sizeof ( BYTE), FALSE);
	}
	__forceinline void* End( void)
	{
		switch ( m_byBuffer[0])
		{
		case 0xC1:
			memcpy( &m_byBuffer[1], &m_wSize, 1);
			break;
		case 0xC2:
			memcpy( &m_byBuffer[1], ( ( BYTE*)&m_wSize)+1, 1);
			memcpy( &m_byBuffer[2], &m_wSize, 1);
			break;
		default:
			assert( "StreamPacketEngine type error(End)");
			return ( NULL);
		}

		return ( GetBuffer());
	}
public:
	__forceinline void AddData( void *pSrc, WORD wSize, BOOL bXor = TRUE)
	{
		if ( m_wSize + wSize > MAX_SPE_BUFFERSIZE)
		{
			assert( "StreamPacketEngine buffer overflow");
			return;
		}
		memcpy( &( m_byBuffer[m_wSize]), pSrc, wSize);

		m_wSize += wSize;
	}
	__forceinline void AddNullData( WORD wSize)
	{
		BYTE byBuffer[MAX_SPE_BUFFERSIZE];
		ZeroMemory( byBuffer, wSize);
		AddData( byBuffer, wSize);
	}
template <class T>
	__forceinline CStreamPacketEngine& operator << ( T Data)
	{
		AddData( &Data, sizeof ( T));
		return ( *this);
	}

	// Getting information
	int GetSize( void)
	{
		return ( m_wSize);
	}
	void* GetBuffer( void)
	{
		return ( m_byBuffer);
	}

	// Sending packet
	__forceinline void Send()
	{
		End();
		SendPacket(m_byBuffer, m_wSize);
	}
};
