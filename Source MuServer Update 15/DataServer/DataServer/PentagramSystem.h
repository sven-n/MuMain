// PentagramSystem.h: interface for the CPentagramSystem class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_PENTAGRAM_JEWEL_INFO_RECV
{
	PSBMSG_HEAD header; // C1:23:00
	WORD index;
	char account[11];
	char name[11];
	BYTE type;
};

struct SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV
{
	PSWMSG_HEAD header; // C2:23:30
	WORD index;
	char account[11];
	char name[11];
	BYTE count;
};

struct SDHP_PENTAGRAM_JEWEL_INFO_SAVE
{
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

struct SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:23:31
	WORD index;
	char account[11];
	char name[11];
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

struct SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_RECV
{
	PSBMSG_HEAD header; // C1:23:32
	WORD index;
	char account[11];
	char name[11];
	BYTE Type;
	BYTE Index;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_PENTAGRAM_JEWEL_INFO_SEND
{
	PSWMSG_HEAD header; // C2:23:00
	WORD index;
	char account[11];
	char name[11];
	BYTE type;
	BYTE count;
};

struct SDHP_PENTAGRAM_JEWEL_INFO
{
	BYTE Type;
	BYTE Index;
	BYTE Attribute;
	BYTE ItemSection;
	WORD ItemType;
	BYTE ItemLevel;
	BYTE OptionIndexRank1;
	BYTE OptionLevelRank1;
	BYTE OptionIndexRank2;
	BYTE OptionLevelRank2;
	BYTE OptionIndexRank3;
	BYTE OptionLevelRank3;
	BYTE OptionIndexRank4;
	BYTE OptionLevelRank4;
	BYTE OptionIndexRank5;
	BYTE OptionLevelRank5;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CPentagramSystem
{
public:
	CPentagramSystem();
	virtual ~CPentagramSystem();
	void GDPentagramJewelInfoRecv(SDHP_PENTAGRAM_JEWEL_INFO_RECV* lpMsg,int index);
	void GDPentagramJewelInfoSaveRecv(SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV* lpMsg);
	void GDPentagramJewelInsertSaveRecv(SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_RECV* lpMsg);
	void GDPentagramJewelDeleteSaveRecv(SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_RECV* lpMsg);
};

extern CPentagramSystem gPentagramSystem;
