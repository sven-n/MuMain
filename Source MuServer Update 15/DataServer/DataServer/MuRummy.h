// MuRummy.h: interface for the CMuRummy class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DataServerProtocol.h"

#define MURUMMY_MAX_CARD 24

struct _tagMuRummyCardInfoDS
{
	_tagMuRummyCardInfoDS()
	{
		this->btColor = -1;
		this->btNumber = -1;
		this->btSlotNum = -1;
		this->btSeq = -1;
		this->btStatus = -1;
	};

	BYTE btColor;
	BYTE btNumber;
	BYTE btSlotNum;
	BYTE btSeq;
	BYTE btStatus;
};

struct _tagMuRummyCardUpdateDS
{
	_tagMuRummyCardUpdateDS()
	{
		this->btSlotNum = -1;
		this->btSeq = -1;
		this->btStatus = -1;
	};

	BYTE btSlotNum;
	BYTE btSeq;
	BYTE btStatus;
};

struct _tagPMSG_REQ_MURUMMY_SELECT_DS
{
	PSBMSG_HEAD h; // C1:12:00
	char AccountID[11];
	char Name[11];
	WORD aIndex;
};

struct _tagPMSG_ANS_MURUMMY_SELECT_DS
{
	PSBMSG_HEAD h; // C1:12:00
	WORD aIndex;
	WORD wScore;
	BYTE btResult;
	_tagMuRummyCardInfoDS stMuRummyCardInfoDS[MURUMMY_MAX_CARD];
};

struct _tagPMSG_REQ_MURUMMY_INSERT_DS
{
	PSBMSG_HEAD h; // C1:12:30
	char AccountID[11];
	char Name[11];
	WORD aIndex;
	_tagMuRummyCardInfoDS stMuRummyCardInfoDS[MURUMMY_MAX_CARD];
};

struct _tagPMSG_REQ_MURUMMY_UPDATE_DS 
{
	PSBMSG_HEAD h; // C1:12:31
	char AccountID[11];
	char Name[11];
	BYTE btSlotNum;
	BYTE btStatus;
	BYTE btSequence;
};

struct _tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS
{
	PSBMSG_HEAD h; // C1:12:32
	char AccountID[11];
	char Name[11];
	WORD wScore;
	_tagMuRummyCardUpdateDS stCardUpdateDS[3];
};

struct _tagPMSG_REQ_MURUMMY_DELETE_DS
{
	PSBMSG_HEAD h; // C1:12:33
	char AccountID[11];
	char Name[11];
};

struct _tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS
{
	PSBMSG_HEAD h; // C1:12:34
	char AccountID[11];
	char Name[11];
	_tagMuRummyCardUpdateDS stCardUpdateDS;
};

struct _tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS
{
	PSBMSG_HEAD h; // C1:12:35
	char AccountID[11];
	char Name[11];
	WORD wScore;
	_tagMuRummyCardUpdateDS stMuRummyCardUpdateDS[MURUMMY_MAX_CARD];
};

class CMuRummy
{
public:
	CMuRummy();
	virtual ~CMuRummy();
	void GDReqCardInfo(_tagPMSG_REQ_MURUMMY_SELECT_DS* lpMsg,int index);
	void GDReqCardInfoInsert(_tagPMSG_REQ_MURUMMY_INSERT_DS* lpMsg);
	void GDReqCardInfoUpdate(_tagPMSG_REQ_MURUMMY_UPDATE_DS* lpMsg);
	void GDReqScoreUpdate(_tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS* lpMsg);
	void GDReqScoreDelete(_tagPMSG_REQ_MURUMMY_DELETE_DS* lpMsg);
	void GDReqSlotInfoUpdate(_tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS* lpMsg);
	void GDReqMuRummyInfoUpdate(_tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS* lpMsg);
};

extern CMuRummy gMuRummy;
