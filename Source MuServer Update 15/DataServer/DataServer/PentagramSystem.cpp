// PentagramSystem.cpp: implementation of the CPentagramSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PentagramSystem.h"
#include "QueryManager.h"
#include "SocketManager.h"

CPentagramSystem gPentagramSystem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPentagramSystem::CPentagramSystem() // OK
{

}

CPentagramSystem::~CPentagramSystem() // OK
{

}

void CPentagramSystem::GDPentagramJewelInfoRecv(SDHP_PENTAGRAM_JEWEL_INFO_RECV* lpMsg,int index) // OK
{
	#if(DATASERVER_UPDATE>=701)

	BYTE send[8192];

	SDHP_PENTAGRAM_JEWEL_INFO_SEND pMsg;

	pMsg.header.set(0x23,0x00,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpMsg->name,sizeof(pMsg.name));

	pMsg.type = lpMsg->type;

	pMsg.count = 0;

	SDHP_PENTAGRAM_JEWEL_INFO info;

	if(gQueryManager.ExecQuery("SELECT * FROM PentagramJewel WHERE Name='%s' AND Type=%d",lpMsg->name,lpMsg->type) != 0)
	{
		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			info.Type = gQueryManager.GetAsInteger("Type");
			info.Index = gQueryManager.GetAsInteger("Index");
			info.Attribute = gQueryManager.GetAsInteger("Attribute");
			info.ItemSection = gQueryManager.GetAsInteger("ItemSection");
			info.ItemType = gQueryManager.GetAsInteger("ItemType");
			info.ItemLevel = gQueryManager.GetAsInteger("ItemLevel");
			info.OptionIndexRank1 = gQueryManager.GetAsInteger("OptionIndexRank1");
			info.OptionLevelRank1 = gQueryManager.GetAsInteger("OptionLevelRank1");
			info.OptionIndexRank2 = gQueryManager.GetAsInteger("OptionIndexRank2");
			info.OptionLevelRank2 = gQueryManager.GetAsInteger("OptionLevelRank2");
			info.OptionIndexRank3 = gQueryManager.GetAsInteger("OptionIndexRank3");
			info.OptionLevelRank3 = gQueryManager.GetAsInteger("OptionLevelRank3");
			info.OptionIndexRank4 = gQueryManager.GetAsInteger("OptionIndexRank4");
			info.OptionLevelRank4 = gQueryManager.GetAsInteger("OptionLevelRank4");
			info.OptionIndexRank5 = gQueryManager.GetAsInteger("OptionIndexRank5");
			info.OptionLevelRank5 = gQueryManager.GetAsInteger("OptionLevelRank5");

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
	}

	gQueryManager.Close();

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);

	#endif
}

void CPentagramSystem::GDPentagramJewelInfoSaveRecv(SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=701)

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_PENTAGRAM_JEWEL_INFO_SAVE* lpInfo = (SDHP_PENTAGRAM_JEWEL_INFO_SAVE*)(((BYTE*)lpMsg)+sizeof(SDHP_PENTAGRAM_JEWEL_INFO_SAVE_RECV)+(sizeof(SDHP_PENTAGRAM_JEWEL_INFO_SAVE)*n));

		if(gQueryManager.ExecQuery("SELECT Name FROM PentagramJewel WHERE Name='%s' AND Type=%d AND [Index]=%d",lpMsg->name,lpInfo->Type,lpInfo->Index) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("INSERT INTO PentagramJewel (Name,Type,[Index],Attribute,ItemSection,ItemType,ItemLevel,OptionIndexRank1,OptionLevelRank1,OptionIndexRank2,OptionLevelRank2,OptionIndexRank3,OptionLevelRank3,OptionIndexRank4,OptionLevelRank4,OptionIndexRank5,OptionLevelRank5) VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",lpMsg->name,lpInfo->Type,lpInfo->Index,lpInfo->Attribute,lpInfo->ItemSection,lpInfo->ItemType,lpInfo->ItemLevel,lpInfo->OptionIndexRank1,lpInfo->OptionLevelRank1,lpInfo->OptionIndexRank2,lpInfo->OptionLevelRank2,lpInfo->OptionIndexRank3,lpInfo->OptionLevelRank3,lpInfo->OptionIndexRank4,lpInfo->OptionLevelRank4,lpInfo->OptionIndexRank5,lpInfo->OptionLevelRank5);
			gQueryManager.Close();
		}
		else
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("UPDATE PentagramJewel SET Attribute=%d,ItemSection=%d,ItemType=%d,ItemLevel=%d,OptionIndexRank1=%d,OptionLevelRank1=%d,OptionIndexRank2=%d,OptionLevelRank2=%d,OptionIndexRank3=%d,OptionLevelRank3=%d,OptionIndexRank4=%d,OptionLevelRank4=%d,OptionIndexRank5=%d,OptionLevelRank5=%d WHERE Name='%s' AND Type=%d AND [Index]=%d",lpInfo->Attribute,lpInfo->ItemSection,lpInfo->ItemType,lpInfo->ItemLevel,lpInfo->OptionIndexRank1,lpInfo->OptionLevelRank1,lpInfo->OptionIndexRank2,lpInfo->OptionLevelRank2,lpInfo->OptionIndexRank3,lpInfo->OptionLevelRank3,lpInfo->OptionIndexRank4,lpInfo->OptionLevelRank4,lpInfo->OptionIndexRank5,lpInfo->OptionLevelRank5,lpMsg->name,lpInfo->Type,lpInfo->Index);
			gQueryManager.Close();
		}
	}

	#endif
}

void CPentagramSystem::GDPentagramJewelInsertSaveRecv(SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=701)

	if(gQueryManager.ExecQuery("SELECT Name FROM PentagramJewel WHERE Name='%s' AND Type=%d AND [Index]=%d",lpMsg->name,lpMsg->Type,lpMsg->Index) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO PentagramJewel (Name,Type,[Index],Attribute,ItemSection,ItemType,ItemLevel,OptionIndexRank1,OptionLevelRank1,OptionIndexRank2,OptionLevelRank2,OptionIndexRank3,OptionLevelRank3,OptionIndexRank4,OptionLevelRank4,OptionIndexRank5,OptionLevelRank5) VALUES ('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",lpMsg->name,lpMsg->Type,lpMsg->Index,lpMsg->Attribute,lpMsg->ItemSection,lpMsg->ItemType,lpMsg->ItemLevel,lpMsg->OptionIndexRank1,lpMsg->OptionLevelRank1,lpMsg->OptionIndexRank2,lpMsg->OptionLevelRank2,lpMsg->OptionIndexRank3,lpMsg->OptionLevelRank3,lpMsg->OptionIndexRank4,lpMsg->OptionLevelRank4,lpMsg->OptionIndexRank5,lpMsg->OptionLevelRank5);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE PentagramJewel SET Attribute=%d,ItemSection=%d,ItemType=%d,ItemLevel=%d,OptionIndexRank1=%d,OptionLevelRank1=%d,OptionIndexRank2=%d,OptionLevelRank2=%d,OptionIndexRank3=%d,OptionLevelRank3=%d,OptionIndexRank4=%d,OptionLevelRank4=%d,OptionIndexRank5=%d,OptionLevelRank5=%d WHERE Name='%s' AND Type=%d AND [Index]=%d",lpMsg->Attribute,lpMsg->ItemSection,lpMsg->ItemType,lpMsg->ItemLevel,lpMsg->OptionIndexRank1,lpMsg->OptionLevelRank1,lpMsg->OptionIndexRank2,lpMsg->OptionLevelRank2,lpMsg->OptionIndexRank3,lpMsg->OptionLevelRank3,lpMsg->OptionIndexRank4,lpMsg->OptionLevelRank4,lpMsg->OptionIndexRank5,lpMsg->OptionLevelRank5,lpMsg->name,lpMsg->Type,lpMsg->Index);
		gQueryManager.Close();
	}

	#endif
}

void CPentagramSystem::GDPentagramJewelDeleteSaveRecv(SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_RECV* lpMsg) // OK
{
	#if(DATASERVER_UPDATE>=701)

	gQueryManager.ExecQuery("DELETE FROM PentagramJewel WHERE Name='%s' AND Type=%d AND [Index]=%d",lpMsg->name,lpMsg->Type,lpMsg->Index);
	gQueryManager.Close();

	#endif
}
