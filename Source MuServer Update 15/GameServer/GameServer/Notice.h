// Notice.h: interface for the CNotice class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

#define MAX_NOTICE 100

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_NOTICE_SEND
{
	PBMSG_HEAD header; // C1:0D
	BYTE type;
	BYTE count;
	BYTE opacity;
	WORD delay;
	DWORD color;
	BYTE speed;
	char message[256];
};

struct PMSG_NOTICE_SEND_NEW
{
	PBMSG_HEAD header; // C1:00
	
	BYTE count;
	BYTE opacity;
	WORD delay;
	DWORD color;
	BYTE speed;
	char message[256];
};

struct PMSG_NOTICE_DEV_SEND
{
	PBMSG_HEAD header; // C1:00
	char message[256];
};

//**********************************************//
//**********************************************//
//**********************************************//

struct NOTICE_INFO
{
	char Message[128];
	int Type;
	int Count;
	int Opacity;
	int Delay;
	int Color;
	int Speed;
	int RepeatTime;
};

class CNotice
{
public:
	CNotice();
	virtual ~CNotice();
	void Load(char* path);
	void SetInfo(NOTICE_INFO info);
	void MainProc();
	void GCNoticeSend(int aIndex,BYTE type,BYTE count,BYTE opacity,WORD delay,DWORD color,BYTE speed,char* message,...);
	void GCNoticeSendToAll(BYTE type,BYTE count,BYTE opacity,WORD delay,DWORD color,BYTE speed,char* message,...);
	void CNotice::NewMessageDevTeam(int aIndex,char* message,...);
	void NewNoticeSend(int aIndex,BYTE count,BYTE opacity,WORD delay,DWORD color,BYTE speed,char* message,...);
private:
	NOTICE_INFO m_NoticeInfo[MAX_NOTICE];
	int m_count;
	int m_NoticeValue;
	DWORD m_NoticeTime;
};

extern CNotice gNotice;
