// MuRummy.cpp: implementation of the CMuRummy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MuRummy.h"
#include "QueryManager.h"
#include "SocketManager.h"

CMuRummy gMuRummy;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMuRummy::CMuRummy() // OK
{

}

CMuRummy::~CMuRummy() // OK
{

}

void CMuRummy::GDReqCardInfo(_tagPMSG_REQ_MURUMMY_SELECT_DS* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=802)

	_tagPMSG_ANS_MURUMMY_SELECT_DS pMsg;

	pMsg.h.set(0x12,0x00,sizeof(pMsg));

	pMsg.aIndex = lpMsg->aIndex;

	pMsg.btResult = 1;

	if(gQueryManager.ExecQuery("SELECT TotalScore FROM MuRummyData WHERE Name='%s'",lpMsg->Name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.btResult = 0;
	}
	else
	{
		pMsg.wScore = gQueryManager.GetAsInteger("TotalScore");

		gQueryManager.Close();

		if(gQueryManager.ExecQuery("SELECT Color,Number,Slot,Status,Sequence FROM MuRummyCard WHERE Name='%s'",lpMsg->Name) == 0)
		{
			gQueryManager.Close();

			pMsg.btResult = 0;
		}
		else
		{
			while(gQueryManager.Fetch() != SQL_NO_DATA)
			{
				int seq = gQueryManager.GetAsInteger("Sequence");

				pMsg.stMuRummyCardInfoDS[seq].btColor = gQueryManager.GetAsInteger("Color");

				pMsg.stMuRummyCardInfoDS[seq].btNumber = gQueryManager.GetAsInteger("Number");

				pMsg.stMuRummyCardInfoDS[seq].btSlotNum = gQueryManager.GetAsInteger("Slot");

				pMsg.stMuRummyCardInfoDS[seq].btSeq = seq;

				pMsg.stMuRummyCardInfoDS[seq].btStatus = gQueryManager.GetAsInteger("Status");
			}

			gQueryManager.Close();
		}
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,pMsg.h.size);

	#endif
}

void CMuRummy::GDReqCardInfoInsert(_tagPMSG_REQ_MURUMMY_INSERT_DS* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	for(int n=0;n < _countof(lpMsg->stMuRummyCardInfoDS);n++)
	{
		gQueryManager.ExecQuery("INSERT INTO MuRummyCard (Name,Color,Number,Slot,Status,Sequence) VALUES ('%s',%d,%d,%d,%d,%d)",lpMsg->Name,lpMsg->stMuRummyCardInfoDS[n].btColor,lpMsg->stMuRummyCardInfoDS[n].btNumber,lpMsg->stMuRummyCardInfoDS[n].btSlotNum,lpMsg->stMuRummyCardInfoDS[n].btStatus,lpMsg->stMuRummyCardInfoDS[n].btSeq);
		gQueryManager.Close();
	}

	#endif
}

void CMuRummy::GDReqCardInfoUpdate(_tagPMSG_REQ_MURUMMY_UPDATE_DS* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	gQueryManager.ExecQuery("UPDATE MuRummyCard SET Slot=%d,Status=%d WHERE Name='%s' AND Sequence=%d",lpMsg->btSlotNum,lpMsg->btStatus,lpMsg->Name,lpMsg->btSequence);
	gQueryManager.Close();

	#endif
}

void CMuRummy::GDReqScoreUpdate(_tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	if(gQueryManager.ExecQuery("SELECT Name FROM MuRummyData WHERE Name='%s'",lpMsg->Name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO MuRummyData (Name,TotalScore) VALUES ('%s',%d)",lpMsg->Name,lpMsg->wScore);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE MuRummyData SET TotalScore=%d WHERE Name='%s'",lpMsg->wScore,lpMsg->Name);
		gQueryManager.Close();
	}

	for(int n=0;n < _countof(lpMsg->stCardUpdateDS);n++)
	{
		gQueryManager.ExecQuery("UPDATE MuRummyCard SET Slot=%d,Status=%d WHERE Name='%s' AND Sequence=%d",lpMsg->stCardUpdateDS[n].btSlotNum,lpMsg->stCardUpdateDS[n].btStatus,lpMsg->Name,lpMsg->stCardUpdateDS[n].btSeq);
		gQueryManager.Close();
	}

	#endif
}

void CMuRummy::GDReqScoreDelete(_tagPMSG_REQ_MURUMMY_DELETE_DS* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	gQueryManager.ExecQuery("DELETE MuRummyCard WHERE Name='%s'",lpMsg->Name);
	gQueryManager.Close();

	gQueryManager.ExecQuery("DELETE MuRummyData WHERE Name='%s'",lpMsg->Name);
	gQueryManager.Close();

	#endif
}

void CMuRummy::GDReqSlotInfoUpdate(_tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	gQueryManager.ExecQuery("UPDATE MuRummyCard SET Slot=%d,Status=%d WHERE Name='%s' AND Sequence=%d",lpMsg->stCardUpdateDS.btSlotNum,lpMsg->stCardUpdateDS.btStatus,lpMsg->Name,lpMsg->stCardUpdateDS.btSeq);
	gQueryManager.Close();

	#endif
}

void CMuRummy::GDReqMuRummyInfoUpdate(_tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=802)

	if(gQueryManager.ExecQuery("SELECT Name FROM MuRummyData WHERE Name='%s'",lpMsg->Name) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO MuRummyData (Name,TotalScore) VALUES ('%s',%d)",lpMsg->Name,lpMsg->wScore);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE MuRummyData SET TotalScore=%d WHERE Name='%s'",lpMsg->wScore,lpMsg->Name);
		gQueryManager.Close();
	}

	for(int n=0;n < _countof(lpMsg->stMuRummyCardUpdateDS);n++)
	{
		gQueryManager.ExecQuery("UPDATE MuRummyCard SET Slot=%d,Status=%d WHERE Name='%s' AND Sequence=%d",lpMsg->stMuRummyCardUpdateDS[n].btSlotNum,lpMsg->stMuRummyCardUpdateDS[n].btStatus,lpMsg->Name,lpMsg->stMuRummyCardUpdateDS[n].btSeq);
		gQueryManager.Close();
	}

	#endif
}
