// StreamPacketEngine.h: interface for the CStreamPacketEngine class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_SPE_BUFFERSIZE	(2048)
#define SIZE_SPE_XORFILTER	(32)

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
			AddData( &m_wSize, 1, FALSE);
			break;
		case 0xC2:
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
protected:
	__forceinline void XorData( int iStart, int iEnd, int iDir = 1)
	{
		BYTE byXorFilter[SIZE_SPE_XORFILTER] =
		{
			0xE7, 0x6D, 0x3A, 0x89, 0xBC, 0xB2, 0x9F, 0x73,
			0x23, 0xA8, 0xFE, 0xB6, 0x49, 0x5D, 0x39, 0x5D,
			0x8A, 0xCB, 0x63, 0x8D, 0xEA, 0x7D, 0x2B, 0x5F,
			0xC3, 0xB1, 0xE9, 0x83, 0x29, 0x51, 0xE8, 0x56
		};

		for ( int i = iStart; i != iEnd; i += iDir)
		{
			m_byBuffer[i] ^= ( m_byBuffer[i - 1] ^ byXorFilter[i%SIZE_SPE_XORFILTER]);
		}
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
		if ( bXor)
		{
			XorData( ( int)m_wSize, ( int)m_wSize + wSize);
		}
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
	__forceinline void Send( BOOL bEncrypt = FALSE, BOOL bForceC4 = FALSE)
	{
		End();
		SendPacket( ( char*)m_byBuffer, m_wSize, bEncrypt, bForceC4);
	}
};
