// StreamPacketEngine.h: interface for the CStreamPacketEngine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STREAMPACKETENGINE_H__C0F4554E_77C8_4E5A_A27E_10A6E10A1364__INCLUDED_)
#define AFX_STREAMPACKETENGINE_H__C0F4554E_77C8_4E5A_A27E_10A6E10A1364__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000




#define MAX_SPE_BUFFERSIZE	( 2048)
#define SIZE_SPE_XORFILTER	( 32)


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
//!! 본섭/블루 패치때 항상 변경해주어야 한다.(서버와 상의하세요!)
#ifdef KJH_MOD_STREAMPACKETENGINE_FILTER
		BYTE byXorFilter[SIZE_SPE_XORFILTER] =
		{
			0x4D, 0xF1, 0x42, 0xEF, 0xDD, 0xCE, 0x32, 0x3D,
			0x4E, 0xA4, 0xC1, 0xB3, 0x2F, 0x8A, 0xCD, 0x1F,
			0x77, 0x3E, 0x1D, 0x77, 0x80, 0x04, 0x11, 0xC1,
			0xC5, 0x66, 0xD7, 0x7D, 0x8B, 0x18, 0x14, 0x2F	
		};
#else // KJH_MOD_STREAMPACKETENGINE_FILTER
		BYTE byXorFilter[SIZE_SPE_XORFILTER] =
		{
			0xE7, 0x6D, 0x3A, 0x89, 0xBC, 0xB2, 0x9F, 0x73,
			0x23, 0xA8, 0xFE, 0xB6, 0x49, 0x5D, 0x39, 0x5D,
			0x8A, 0xCB, 0x63, 0x8D, 0xEA, 0x7D, 0x2B, 0x5F,
			0xC3, 0xB1, 0xE9, 0x83, 0x29, 0x51, 0xE8, 0x56
		};
#endif // KJH_MOD_STREAMPACKETENGINE_FILTER
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
#ifdef PKD_ADD_ENHANCED_ENCRYPTION
		SendPacket( ( char*)m_byBuffer, m_wSize, TRUE, bForceC4);
#else
		SendPacket( ( char*)m_byBuffer, m_wSize, bEncrypt, bForceC4);
#endif // PKD_ADD_ENHANCED_ENCRYPTION
		//. 패킷보내는 함수는 전부 여기를 통한다
	}
};


/*class CStreamPacketEngine_Server
{
public:
	CStreamPacketEngine_Server()
	{
		Clear();
	}
	virtual ~CStreamPacketEngine_Server()	{}
	void Clear( void)
	{
		m_wSize = 0;
	}

protected:
	WORD m_wSize;
	BYTE m_byBuffer[MAX_SPE_BUFFERSIZE];

protected:
	void XorData( int iStart, int iEnd, int iDir = 1)
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
	void AddData( void *pSrc, WORD wSize)
	{
		assert( m_wSize + wSize <= MAX_SPE_BUFFERSIZE);
		memcpy( &m_byBuffer[m_wSize], pSrc, wSize);
		m_wSize += wSize;
	}
	int ExtractPacket( void *pTar)
	{
		WORD wSize;
		switch ( m_byBuffer[0])
		{
		case 0xC1:
			wSize = ( WORD)m_byBuffer[1];
			break;
		case 0xC2:
			wSize = ( ( WORD)m_byBuffer[1] << 8) + m_byBuffer[2];
			break;
		default:
			m_wSize = 0;	// clear buffer
			return ( 1);	// invalid packet
		}
		if ( m_wSize < wSize)
		{
			return ( 2);	// not complete packet
		}

		// extract
		BYTE byXorFilter[SIZE_SPE_XORFILTER] =
		{
			0xE7, 0x6D, 0x3A, 0x89, 0xBC, 0xB2, 0x9F, 0x73,
			0x23, 0xA8, 0xFE, 0xB6, 0x49, 0x5D, 0x39, 0x5D,
			0x8A, 0xCB, 0x63, 0x8D, 0xEA, 0x7D, 0x2B, 0x5F,
			0xC3, 0xB1, 0xE9, 0x83, 0x29, 0x51, 0xE8, 0x56
		};
		XorData( ( int)wSize - 1, ( int)( ( m_byBuffer[0] == 0xC1) ? 3 : 4) - 1, -1);
		memcpy( pTar, m_byBuffer, wSize);
		// pull
		BYTE byTemp[MAX_SPE_BUFFERSIZE];
		m_wSize -= wSize;
		memcpy( byTemp, &m_byBuffer[wSize], m_wSize);
		memcpy( m_byBuffer, byTemp, m_wSize);
		return ( 0);	// success
	}
};*/


#endif // !defined(AFX_STREAMPACKETENGINE_H__C0F4554E_77C8_4E5A_A27E_10A6E10A1364__INCLUDED_)
