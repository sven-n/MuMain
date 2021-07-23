//////////////////////////////////////////////////////////////////////
//
// Simple Modulus ( Cryptography library ) ver 1.1
//
// by Robbie Hong ( 2003/01/09)
//
// SimpleModulus.h: interface for the CSimpleModulus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLEMODULUS_H__5EC20CDC_CA87_4F9D_927B_24C8DD11ECB7__INCLUDED_)
#define AFX_SIMPLEMODULUS_H__5EC20CDC_CA87_4F9D_927B_24C8DD11ECB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/*# define SIZE_ENCRYPTION_BLOCK	( 32)
# define SIZE_ENCRYPTION_KEY	( 16)
# define SIZE_ENCRYPTED_BLOCK	( 38)*/

/*# define SIZE_ENCRYPTION_BLOCK	( 16)
# define SIZE_ENCRYPTION_KEY	( 8)	// SIZE_ENCRYPTION_BLOCK 의 절반
# define SIZE_ENCRYPTED_BLOCK	( 20)*/

# define SIZE_ENCRYPTION_BLOCK	( 8)
# define SIZE_ENCRYPTION_KEY	( 4)	// SIZE_ENCRYPTION_BLOCK 의 절반
# define SIZE_ENCRYPTED_BLOCK	( 11)


class CSimpleModulus  
{
public:
	CSimpleModulus();
	virtual ~CSimpleModulus();

	void Init( void);

protected:
	DWORD m_dwModulus[SIZE_ENCRYPTION_KEY];
	DWORD m_dwEncryptionKey[SIZE_ENCRYPTION_KEY];
	DWORD m_dwDecryptionKey[SIZE_ENCRYPTION_KEY];
	DWORD m_dwXORKey[SIZE_ENCRYPTION_KEY];

	static DWORD s_dwSaveLoadXOR[SIZE_ENCRYPTION_KEY];


	//////////////////////////////////////////////////////////////////////////
	// 암호화/복호화 ->

	// 암호화
public:
	int Encrypt( void *lpTarget, void *lpSource, int iSize);	// 큰 사이즈 모두, lpTarget 이 NULL이면 필요량 리턴
	int Decrypt( void *lpTarget, void *lpSource, int iSize);	// 큰 사이즈 모두, lpTarget 이 NULL이면 필요량 리턴

protected:
	void EncryptBlock( void *lpTarget, void *lpSource, int nSize);	// 32 byte 를 38 byte 로
	int DecryptBlock( void *lpTarget, void *lpSource);	// 38 byte 를 32 byte 로, Size를 리턴함
	int AddBits( void *lpBuffer, int nNumBufferBits, void *lpBits, int nInitialBit, int nNumBits);
	void Shift( void *lpBuffer, int nByte, int nShift);
	int GetByteOfBit( int nBit);

	// <- 암호화/복호화
	//////////////////////////////////////////////////////////////////////////

	// 저장/읽기
public:
	BOOL SaveAllKey( char *lpszFileName);
	BOOL LoadAllKey( char *lpszFileName);
	BOOL SaveEncryptionKey( char *lpszFileName);
	BOOL LoadEncryptionKey( char *lpszFileName);
	BOOL SaveDecryptionKey( char *lpszFileName);
	BOOL LoadDecryptionKey( char *lpszFileName);
protected:
	BOOL SaveKey( char *lpszFileName, unsigned short sID, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR);
	BOOL LoadKey( char *lpszFileName, unsigned short sID, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR);
public:
	BOOL LoadKeyFromBuffer( BYTE *pbyBuffer, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR);	// 청크 없이 읽는다.
};


#pragma pack ( 1)
typedef struct
{
	unsigned short m_sID;
	unsigned int m_iSize;
} ChunkHeader;
#pragma pack ()

# define CHUNKID_ALLKEY		( 0x1111)
# define CHUNKID_ONEKEY		( 0x1112)


#endif // !defined(AFX_SIMPLEMODULUS_H__5EC20CDC_CA87_4F9D_927B_24C8DD11ECB7__INCLUDED_)
