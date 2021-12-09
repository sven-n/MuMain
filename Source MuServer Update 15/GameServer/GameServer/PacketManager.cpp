// PacketManager.cpp: implementation of the CPacketManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PacketManager.h"

CPacketManager gPacketManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPacketManager::CPacketManager() // OK
{
	this->Init();
}

CPacketManager::~CPacketManager() // OK
{

}

void CPacketManager::Init() // OK
{
	#if(GAMESERVER_UPDATE>=701)

	BYTE DES_XEX3[24] = {0x0C,0xB0,0x66,0xCC,0xEF,0x92,0x8C,0x5C,0x65,0xF4,0xAC,0x3F,0x71,0xF2,0x7B,0xCE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

	this->m_Encryption.SetKey(DES_XEX3,sizeof(DES_XEX3));
	this->m_Decryption.SetKey(DES_XEX3,sizeof(DES_XEX3));

	#else

	memset(&this->m_Encryption,0,sizeof(this->m_Encryption));
	memset(&this->m_Decryption,0,sizeof(this->m_Decryption));

	this->m_SaveLoadXor[0] = 0x3F08A79B;
	this->m_SaveLoadXor[1] = 0xE25CC287;
	this->m_SaveLoadXor[2] = 0x93D27AB9;
	this->m_SaveLoadXor[3] = 0x20DEA7BF;

	#endif

	memset(this->m_buff,0,sizeof(this->m_buff));

	this->m_size = 0;

	//#if(GAMESERVER_UPDATE>=601)

	//this->m_XorFilter[0] = 0xAB;
	//this->m_XorFilter[1] = 0x11;
	//this->m_XorFilter[2] = 0xCD;
	//this->m_XorFilter[3] = 0xFE;
	//this->m_XorFilter[4] = 0x18;
	//this->m_XorFilter[5] = 0x23;
	//this->m_XorFilter[6] = 0xC5;
	//this->m_XorFilter[7] = 0xA3;
	//this->m_XorFilter[8] = 0xCA;
	//this->m_XorFilter[9] = 0x33;
	//this->m_XorFilter[10] = 0xC1;
	//this->m_XorFilter[11] = 0xCC;
	//this->m_XorFilter[12] = 0x66;
	//this->m_XorFilter[13] = 0x67;
	//this->m_XorFilter[14] = 0x21;
	//this->m_XorFilter[15] = 0xF3;
	//this->m_XorFilter[16] = 0x32;
	//this->m_XorFilter[17] = 0x12;
	//this->m_XorFilter[18] = 0x15;
	//this->m_XorFilter[19] = 0x35;
	//this->m_XorFilter[20] = 0x29;
	//this->m_XorFilter[21] = 0xFF;
	//this->m_XorFilter[22] = 0xFE;
	//this->m_XorFilter[23] = 0x1D;
	//this->m_XorFilter[24] = 0x44;
	//this->m_XorFilter[25] = 0xEF;
	//this->m_XorFilter[26] = 0xCD;
	//this->m_XorFilter[27] = 0x41;
	//this->m_XorFilter[28] = 0x26;
	//this->m_XorFilter[29] = 0x3C;
	//this->m_XorFilter[30] = 0x4E;
	//this->m_XorFilter[31] = 0x4D;

	//#else

	this->m_XorFilter[0] = 0xE7;
	this->m_XorFilter[1] = 0x6D;
	this->m_XorFilter[2] = 0x3A;
	this->m_XorFilter[3] = 0x89;
	this->m_XorFilter[4] = 0xBC;
	this->m_XorFilter[5] = 0xB2;
	this->m_XorFilter[6] = 0x9F;
	this->m_XorFilter[7] = 0x73;
	this->m_XorFilter[8] = 0x23;
	this->m_XorFilter[9] = 0xA8;
	this->m_XorFilter[10] = 0xFE;
	this->m_XorFilter[11] = 0xB6;
	this->m_XorFilter[12] = 0x49;
	this->m_XorFilter[13] = 0x5D;
	this->m_XorFilter[14] = 0x39;
	this->m_XorFilter[15] = 0x5D;
	this->m_XorFilter[16] = 0x8A;
	this->m_XorFilter[17] = 0xCB;
	this->m_XorFilter[18] = 0x63;
	this->m_XorFilter[19] = 0x8D;
	this->m_XorFilter[20] = 0xEA;
	this->m_XorFilter[21] = 0x7D;
	this->m_XorFilter[22] = 0x2B;
	this->m_XorFilter[23] = 0x5F;
	this->m_XorFilter[24] = 0xC3;
	this->m_XorFilter[25] = 0xB1;
	this->m_XorFilter[26] = 0xE9;
	this->m_XorFilter[27] = 0x83;
	this->m_XorFilter[28] = 0x29;
	this->m_XorFilter[29] = 0x51;
	this->m_XorFilter[30] = 0xE8;
	this->m_XorFilter[31] = 0x56;

	//#endif
}

bool CPacketManager::LoadEncryptionKey(char* name) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	return 1;

	#else

	return this->LoadKey(name,4370,0);

	#endif
}

bool CPacketManager::LoadDecryptionKey(char* name) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	return 1;

	#else

	return this->LoadKey(name,4370,1);

	#endif
}

bool CPacketManager::LoadKey(char* name,WORD header,bool type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	return 1;

	#else

	ENCDEC_HEADER HeaderInfo;

	HANDLE file = CreateFile(name,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

	if(file == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD size;

	ReadFile(file,&HeaderInfo,sizeof(HeaderInfo),&size,0);

	if(HeaderInfo.header != header || HeaderInfo.size != (sizeof(HeaderInfo)+sizeof(ENCDEC_DATA)))
	{
		CloseHandle(file);
		return 0;
	}

	ENCDEC_DATA* lpData;

	if(type == 0)
	{
		lpData = &this->m_Encryption;
	}
	else
	{
		lpData = &this->m_Decryption;
	}

	DWORD table[4];

	ReadFile(file,table,sizeof(table),&size,0);

	for(int n=0;n < 4;n++)
	{
		lpData->Modulus[n] = this->m_SaveLoadXor[n]^table[n];
	}

	ReadFile(file,table,sizeof(table),&size,0);

	for(int n=0;n < 4;n++)
	{
		lpData->Key[n] = this->m_SaveLoadXor[n]^table[n];
	}

	ReadFile(file,table,sizeof(table),&size,0);

	for(int n=0;n < 4;n++)
	{
		lpData->Xor[n] = this->m_SaveLoadXor[n]^table[n];
	}

	CloseHandle(file);
	return 1;

	#endif
}

int CPacketManager::Encrypt(BYTE* lpTarget,BYTE* lpSource,int size) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int OriSize = size;

	BYTE* lpTempSource = lpSource;
	BYTE* lpTempTarget = lpTarget;

	if((size%8) != 0)
	{
		size = (1+(size/8))*8;
	}

	this->m_Encryption.ProcessData(lpTempTarget,lpTempSource,size);

	lpTempTarget[size] = size-OriSize;

	return (++size);

	#else

	int OriSize = size;
	int TempSize1,TempSize2;

	BYTE* lpTempSource = lpSource;
	BYTE* lpTempTarget = lpTarget;

	int dec = (size+7)/8;

	size = ((dec+(dec*4))*2)+dec;

	if(lpTarget != 0)
	{
		TempSize1 = OriSize;

		for(int n=0;n < TempSize1;n+=8,OriSize-=8,lpTempTarget+=11)
		{
			TempSize2 = OriSize;

			if(OriSize >= 8)
			{
				TempSize2 = 8;
			}

			this->EncryptBlock(lpTempTarget,&lpTempSource[n],TempSize2);
		}
	}

	return size;

	#endif
}

int CPacketManager::Decrypt(BYTE* lpTarget,BYTE* lpSource,int size) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int OriSize = size;

	BYTE* lpTempSource = lpSource;
	BYTE* lpTempTarget = lpTarget;

	if(((--size)%8) != 0)
	{
		size = (1+(size/8))*8;
	}

	this->m_Decryption.ProcessData(lpTempTarget,lpTempSource,size);

	return (OriSize-lpTempSource[(OriSize-1)]);

	#else

	int result = (size*8)/11;
	int DecSize = 0;

	BYTE* lpTempSource = lpSource;
	BYTE* lpTempTarget = lpTarget;

	if(lpTarget != 0 && size > 0)
	{
		result = 0;

		while(DecSize < size)
		{
			int TempResult = this->DecryptBlock(lpTempTarget,lpTempSource);

			if(result < 0)
			{
				return result;
			}

			result += TempResult;
			DecSize += 11;
			lpTempSource += 11;
			lpTempTarget += 8;
		}
	}

	return result;

	#endif
}

int CPacketManager::EncryptBlock(BYTE* lpTarget,BYTE* lpSource,int size) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	return 0;

	#else

	DWORD EncBuffer[4] = {0};
	DWORD EncValue = 0;

	BYTE* lpTempSource = lpSource;
	BYTE* lpTempTarget = lpTarget;

	memset(lpTempTarget,0,11);

	for(int n=0;n < 4;n++)
	{
		EncBuffer[n] = (((this->m_Encryption.Xor[n]^((WORD*)lpTempSource)[n])^EncValue)*this->m_Encryption.Key[n])%this->m_Encryption.Modulus[n];
		EncValue = (WORD)EncBuffer[n];
	}

	for(int n=0;n < 3;n++)
	{
		EncBuffer[n] = (EncBuffer[n]^this->m_Encryption.Xor[n])^(WORD)EncBuffer[n+1];
	}

	int BitPos = 0;

	for(int n=0;n < 4;n++)
	{
		BitPos = this->AddBits(lpTempTarget,BitPos,(BYTE*)&EncBuffer[n],0,16);
		BitPos = this->AddBits(lpTempTarget,BitPos,(BYTE*)&EncBuffer[n],22,2);
	}

	BYTE CheckSum = 0xF8;

	for(int n=0;n < 8;n++)
	{
		CheckSum ^= lpTempSource[n];
	}

	((BYTE*)&EncValue)[0] = (CheckSum^size)^0x3D;
	((BYTE*)&EncValue)[1] = CheckSum;

	return this->AddBits(lpTempTarget,BitPos,(BYTE*)&EncValue,0,16);

	#endif
}

int CPacketManager::DecryptBlock(BYTE* lpTarget,BYTE* lpSource) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	return 0;

	#else

	DWORD DecBuffer[4] = {0};

	BYTE* lpTempSource = lpSource;
	BYTE* lpTempTarget = lpTarget;

	memset(lpTempTarget,0,8);

	int BitPos = 0;

	for(int n=0;n < 4;n++)
	{
		this->AddBits((BYTE*)&DecBuffer[n],0,lpTempSource,BitPos,16);
		BitPos += 16;
		this->AddBits((BYTE*)&DecBuffer[n],22,lpTempSource,BitPos,2);
		BitPos += 2;
	}

	for(int n=2;n >= 0;n--)
	{
		DecBuffer[n] = (DecBuffer[n]^this->m_Decryption.Xor[n])^(WORD)DecBuffer[n+1];
	}

	DWORD value = 0;

	for(int n=0;n < 4;n++)
	{
		((WORD*)lpTempTarget)[n] = (WORD)((((this->m_Decryption.Key[n]*DecBuffer[n])%this->m_Decryption.Modulus[n])^this->m_Decryption.Xor[n])^value);
		value = (WORD)DecBuffer[n];
	}

	DecBuffer[0] = 0;

	this->AddBits((BYTE*)DecBuffer,0,lpTempSource,BitPos,16);

	((BYTE*)DecBuffer)[0] = (((BYTE*)DecBuffer)[0]^((BYTE*)DecBuffer)[1])^0x3D;

	BYTE CheckSum = 0xF8;

	for(int n=0;n < 8;n++)
	{
		CheckSum ^= lpTempTarget[n];
	}

	if(CheckSum != ((BYTE*)DecBuffer)[1])
	{
		return -1;
	}
	
	return ((BYTE*)DecBuffer)[0];

	#endif
}

int CPacketManager::AddBits(BYTE* lpTarget,int TargetBitPos,BYTE* lpSource,int SourceBitPos,int size) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	return 0;

	#else

	int SourceBitSize = SourceBitPos+size;
	int TempSize1 = this->GetByteOfBit(SourceBitSize-1)+(1-this->GetByteOfBit(SourceBitPos));

	BYTE* lpTempBuff = new BYTE[TempSize1+1];

	memset(lpTempBuff,0,TempSize1+1);

	memcpy(lpTempBuff,&lpSource[this->GetByteOfBit(SourceBitPos)],TempSize1);

	if((SourceBitSize%8) != 0)
	{
		lpTempBuff[TempSize1-1] &= 0xFF << (8-(SourceBitSize%8));
	}

	int ShiftLeft = (SourceBitPos%8);
	int ShiftRight = (TargetBitPos%8);

	this->Shift(lpTempBuff,TempSize1,-ShiftLeft);
	this->Shift(lpTempBuff,TempSize1+1,ShiftRight);

	int TempSize2 = ((ShiftRight<=ShiftLeft)?0:1)+TempSize1;

	BYTE* lpTempTarget = &lpTarget[this->GetByteOfBit(TargetBitPos)];

	for(int n=0;n < TempSize2;n++)
	{
		lpTempTarget[n] |= lpTempBuff[n];
	}

	delete[] lpTempBuff;

	return TargetBitPos+size;

	#endif
}

int CPacketManager::GetByteOfBit(int value) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	return 0;

	#else

	return value >> 3;

	#endif
}

void CPacketManager::Shift(BYTE* lpBuff,int size,int ShiftSize) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	return;

	#else

	BYTE* lpTempBuff = lpBuff;

	if(ShiftSize != 0)
	{
		if(ShiftSize > 0)
		{
			if((size-1) > 0)
			{
				for(int n=(size-1);n > 0;n--)
				{
					lpTempBuff[n] = (lpTempBuff[n-1] << (8-ShiftSize)) | (lpTempBuff[n] >> ShiftSize);
				}
			}

			lpTempBuff[0] >>= ShiftSize;
		}
		else
		{
			ShiftSize = -ShiftSize;

			if((size-1) > 0)
			{
				for(int n=0;n < (size-1);n++)
				{
					lpTempBuff[n] = (lpTempBuff[n+1] >> (8-ShiftSize)) | (lpTempBuff[n] << ShiftSize);
				}
			}

			lpTempBuff[size-1] <<= ShiftSize;
		}
	}

	#endif
}

bool CPacketManager::AddData(BYTE* lpBuff,int size) // OK
{
	if(size <= 0 || size >= 2048)
	{
		return 0;
	}

	memcpy(this->m_buff,lpBuff,size);
	this->m_size = size;
	return 1;
}

bool CPacketManager::ExtractPacket(BYTE* lpBuff) // OK
{
	int size,end;

	switch(this->m_buff[0])
	{
		case 0xC1:
			size = this->m_buff[1];
			end = 2;
			break;
		case 0xC2:
			size = MAKEWORD(this->m_buff[2],this->m_buff[1]);
			end = 3;
			break;
		default:
			return 0;
	}

	if(this->m_size < ((DWORD)size))
	{
		return 0;
	}

	this->XorData((size-1),end);

	memcpy(lpBuff,this->m_buff,size);

	return 1;
}

void CPacketManager::XorData(int start,int end) // OK
{
	if(start < end)
	{
		return;
	}

	for(int n=start;n > end;n--)
	{
		this->m_buff[n] ^= this->m_buff[n-1]^this->m_XorFilter[n%32];
	}
}
