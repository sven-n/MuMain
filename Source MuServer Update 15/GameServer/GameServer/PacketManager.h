// PacketManager.h: interface for the CPacketManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#if(GAMESERVER_UPDATE>=701)

#include "Util\\cryptopp\\cryptlib.h"
#include "Util\\cryptopp\\modes.h"
#include "\Util\\cryptopp\\des.h"

using namespace CryptoPP;

#endif

struct ENCDEC_HEADER
{
	#pragma pack(1)
	WORD header;
	DWORD size;
	#pragma pack()
};

struct ENCDEC_DATA
{
	DWORD Modulus[4];
	DWORD Key[4];
	DWORD Xor[4];
};

class CPacketManager
{
public:
	CPacketManager();
	virtual ~CPacketManager();
	void Init();
	bool LoadEncryptionKey(char* name);
	bool LoadDecryptionKey(char* name);
	bool LoadKey(char* name,WORD header,bool type);
	int Encrypt(BYTE* lpTarget,BYTE* lpSource,int size);
	int Decrypt(BYTE* lpTarget,BYTE* lpSource,int size);
	int EncryptBlock(BYTE* lpTarget,BYTE* lpSource,int size);
	int DecryptBlock(BYTE* lpTarget,BYTE* lpSource);
	int AddBits(BYTE* lpTarget,int TargetBitPos,BYTE* lpSource,int SourceBitPos,int size);
	int GetByteOfBit(int value);
	void Shift(BYTE* lpBuff,int size,int ShiftSize);
	bool AddData(BYTE* lpBuff,int size);
	bool ExtractPacket(BYTE* lpBuff);
	void XorData(int start,int end);
private:
	#if(GAMESERVER_UPDATE>=701)
	ECB_Mode<DES_XEX3>::Encryption m_Encryption;
	ECB_Mode<DES_XEX3>::Decryption m_Decryption;
	#else
	ENCDEC_DATA m_Encryption;
	ENCDEC_DATA m_Decryption;
	DWORD m_SaveLoadXor[4];
	#endif
	BYTE m_buff[2048];
	DWORD m_size;
public:
	BYTE m_XorFilter[32];
};

extern CPacketManager gPacketManager;
