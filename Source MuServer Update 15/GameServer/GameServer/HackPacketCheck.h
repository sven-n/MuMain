// HackPacketCheck.h: interface for the CHackPacketCheck class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_HACK_PACKET_INFO 256

struct HACK_PACKET_INFO
{
	void Reset() // OK
	{
		this->Index = -1;
		this->Value = -1;
		this->Encrypt = -1;
		this->MaxDelay = 0;
		this->MinCount = 0;
		this->MaxCount = 0;
	}

	int Index;
	int Value;
	int Encrypt;
	int MaxDelay;
	int MinCount;
	int MaxCount;
};

struct HACK_PACKET_MAIN_INFO
{
	void ResetIndex() // OK
	{
		this->IndexInfo.Reset();
	}

	void ResetValue() // OK
	{
		for(int n=0;n < MAX_HACK_PACKET_INFO;n++)
		{
			this->ValueInfo[n].Reset();
		}
	}

	HACK_PACKET_INFO IndexInfo;
	HACK_PACKET_INFO ValueInfo[MAX_HACK_PACKET_INFO];
};

class CHackPacketCheck
{
public:
	CHackPacketCheck();
	virtual ~CHackPacketCheck();
	void Init();
	void Load(char* path);
	void SetInfo(HACK_PACKET_INFO info);
	HACK_PACKET_INFO* GetInfo(int index,int value);
	bool CheckPacketHack(int aIndex,int index,int value,int encrypt,int serial);
	bool CheckPacketHackMaxDelay(int aIndex,int index,int MaxDelay);
	bool CheckPacketHackMinCount(int aIndex,int index,int MinCount);
	bool CheckPacketHackMaxCount(int aIndex,int index,int MaxCount);
private:
	HACK_PACKET_MAIN_INFO m_HackPacketInfo[MAX_HACK_PACKET_INFO];
};

extern CHackPacketCheck gHackPacketCheck;
