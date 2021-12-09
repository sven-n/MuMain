// Warehouse.cpp: implementation of the CWarehouse class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Warehouse.h"
#include "QueryManager.h"
#include "SocketManager.h"

CWarehouse gWarehouse;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWarehouse::CWarehouse() // OK
{
	this->m_WarehouseGuildInfo.clear();
	this->m_ExecFirst = 0;
}

CWarehouse::~CWarehouse() // OK
{

}

void CWarehouse::GDWarehouseItemRecv(SDHP_WAREHOUSE_ITEM_RECV* lpMsg,int index) // OK
{
	SDHP_WAREHOUSE_ITEM_SEND pMsg;

	pMsg.header.set(0x05,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	if(lpMsg->WarehouseNumber == 0)
	{
		if(gQueryManager.ExecQuery("SELECT AccountID FROM warehouse WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("INSERT INTO warehouse (AccountID,Money,EndUseDate,DbVersion) VALUES ('%s',0,getdate(),3)",lpMsg->account);
			gQueryManager.Close();
			gQueryManager.ExecQuery("UPDATE warehouse SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) WHERE AccountID='%s'",sizeof(pMsg.WarehouseItem),sizeof(pMsg.WarehouseItem),lpMsg->account);
			gQueryManager.Close();
			this->DGWarehouseFreeSend(index,lpMsg->index,lpMsg->account);
			return;
		}
		else
		{
			gQueryManager.Close();

			if(gQueryManager.ExecQuery("SELECT Items,Money,pw FROM warehouse WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
			{
				gQueryManager.Close();

				memset(pMsg.WarehouseItem,0xFF,sizeof(pMsg.WarehouseItem));

				pMsg.WarehouseMoney = 0;

				pMsg.WarehousePassword = 0;
			}
			else
			{
				gQueryManager.GetAsBinary("Items",pMsg.WarehouseItem[0],sizeof(pMsg.WarehouseItem));

				pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");

				pMsg.WarehousePassword = gQueryManager.GetAsInteger("pw");

				gQueryManager.Close();
			}
		}
	}
	else
	{
		if(gQueryManager.ExecQuery("SELECT AccountID FROM ExtWarehouse WHERE AccountID='%s' AND Number=%d",lpMsg->account,lpMsg->WarehouseNumber) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("INSERT INTO ExtWarehouse (AccountID,Money,Number) VALUES ('%s',0,%d)",lpMsg->account,lpMsg->WarehouseNumber);
			gQueryManager.Close();
			gQueryManager.ExecQuery("UPDATE ExtWarehouse SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) WHERE AccountID='%s' AND Number=%d",sizeof(pMsg.WarehouseItem),sizeof(pMsg.WarehouseItem),lpMsg->account,lpMsg->WarehouseNumber);
			gQueryManager.Close();
			this->DGWarehouseFreeSend(index,lpMsg->index,lpMsg->account);
			return;
		}
		else
		{
			gQueryManager.Close();

			if(gQueryManager.ExecQuery("SELECT Items,Money FROM ExtWarehouse WHERE AccountID='%s' AND Number=%d",lpMsg->account,lpMsg->WarehouseNumber) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
			{
				gQueryManager.Close();

				memset(pMsg.WarehouseItem,0xFF,sizeof(pMsg.WarehouseItem));

				pMsg.WarehouseMoney = 0;
			}
			else
			{
				gQueryManager.GetAsBinary("Items",pMsg.WarehouseItem[0],sizeof(pMsg.WarehouseItem));

				pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");

				gQueryManager.Close();

				if(gQueryManager.ExecQuery("SELECT pw FROM warehouse WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
				{
					gQueryManager.Close();

					pMsg.WarehousePassword = 0;
				}
				else
				{
					pMsg.WarehousePassword = gQueryManager.GetAsInteger("pw");

					gQueryManager.Close();
				}
			}
		}
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void CWarehouse::GDWarehouseItemSaveRecv(SDHP_WAREHOUSE_ITEM_SAVE_RECV* lpMsg) // OK
{
	if(lpMsg->WarehouseNumber == 0)
	{
		gQueryManager.BindParameterAsBinary(1,lpMsg->WarehouseItem[0],sizeof(lpMsg->WarehouseItem));
		gQueryManager.ExecQuery("UPDATE warehouse SET Items=?,Money=%d WHERE AccountID='%s'",lpMsg->WarehouseMoney,lpMsg->account);
		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE warehouse SET pw=%d WHERE AccountID='%s'",lpMsg->WarehousePassword,lpMsg->account);
		gQueryManager.Close();
	}
	else
	{
		gQueryManager.BindParameterAsBinary(1,lpMsg->WarehouseItem[0],sizeof(lpMsg->WarehouseItem));
		gQueryManager.ExecQuery("UPDATE ExtWarehouse SET Items=?,Money=%d WHERE AccountID='%s' AND Number=%d",lpMsg->WarehouseMoney,lpMsg->account,lpMsg->WarehouseNumber);
		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE warehouse SET pw=%d WHERE AccountID='%s'",lpMsg->WarehousePassword,lpMsg->account);
		gQueryManager.Close();
	}
}

void CWarehouse::DGWarehouseFreeSend(int ServerIndex,WORD index,char* account) // OK
{
	SDHP_WAREHOUSE_FREE_SEND pMsg;

	pMsg.header.set(0x05,0x01,sizeof(pMsg));

	pMsg.index = index;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	gSocketManager.DataSend(ServerIndex,(BYTE*)&pMsg,pMsg.header.size);
}

void CWarehouse::GDWarehouseGuildItemRecv(SDHP_WAREHOUSE_ITEM_RECV* lpMsg,int index) // OK
{
	//if (!this->m_ExecFirst)
	//{
	//	gQueryManager.ExecQuery("UPDATE WarehouseGuild SET InUse=0");
	//	gQueryManager.Close();
	//	this->m_ExecFirst = 1;
	//}

	if(this->WarehouseGuildAddUse(lpMsg->account) == 0)
	{
		SDHP_WAREHOUSEGUILD_CLOSE_SEND pMsg;

		pMsg.header.set(0x05,0x75,sizeof(pMsg));

		pMsg.index = lpMsg->index;

		gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		return;
	}

	SDHP_WAREHOUSE_ITEM_SEND pMsg;

	pMsg.header.set(0x05,0x70,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	if(gQueryManager.ExecQuery("SELECT Guild FROM WarehouseGuild WHERE Guild='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();
		gQueryManager.ExecQuery("INSERT INTO WarehouseGuild (Guild,Money,EndUseDate,DbVersion) VALUES ('%s',0,getdate(),3)",lpMsg->account);
		gQueryManager.Close();
		gQueryManager.ExecQuery("UPDATE WarehouseGuild SET Items=CONVERT(varbinary(%d),REPLICATE(char(0xFF),%d)) WHERE Guild='%s'",sizeof(pMsg.WarehouseItem),sizeof(pMsg.WarehouseItem),lpMsg->account);
		gQueryManager.Close();
		this->DGWarehouseGuildFreeSend(index,lpMsg->index,lpMsg->account);
		return;
	}
	else
	{
		gQueryManager.Close();

		//if(gQueryManager.ExecQuery("SELECT InUse FROM WarehouseGuild WHERE Guild='%s' and InUse = 0",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		//{
		//	gQueryManager.Close();

		//	SDHP_WAREHOUSEGUILD_CLOSE_SEND pMsg;

		//	pMsg.header.set(0x05,0x75,sizeof(pMsg));

		//	pMsg.index = lpMsg->index;

		//	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
		//	return;
		//}
		//else
		//{
		//	gQueryManager.Close();
		//}

		if(gQueryManager.ExecQuery("SELECT Items,Money,pw FROM WarehouseGuild WHERE Guild='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			memset(pMsg.WarehouseItem,0xFF,sizeof(pMsg.WarehouseItem));

			pMsg.WarehouseMoney = 0;

			pMsg.WarehousePassword = 0;

			//gQueryManager.ExecQuery("UPDATE WarehouseGuild SET InUse=1 WHERE Guild='%s'",lpMsg->account);
			//gQueryManager.Close();
		}
		else
		{
			gQueryManager.GetAsBinary("Items",pMsg.WarehouseItem[0],sizeof(pMsg.WarehouseItem));

			pMsg.WarehouseMoney = gQueryManager.GetAsInteger("Money");

			pMsg.WarehousePassword = gQueryManager.GetAsInteger("pw");

			gQueryManager.Close();

			//gQueryManager.ExecQuery("UPDATE WarehouseGuild SET InUse=1 WHERE Guild='%s'",lpMsg->account);
			//gQueryManager.Close();
		}
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void CWarehouse::GDWarehouseGuildItemSaveRecv(SDHP_WAREHOUSE_ITEM_SAVE_RECV* lpMsg) // OK
{
		gQueryManager.BindParameterAsBinary(1,lpMsg->WarehouseItem[0],sizeof(lpMsg->WarehouseItem));
		gQueryManager.ExecQuery("UPDATE WarehouseGuild SET Items=?,Money=%d,pw=%d WHERE Guild='%s'",lpMsg->WarehouseMoney,lpMsg->WarehousePassword,lpMsg->account);
		gQueryManager.Close();

		this->WarehouseGuildDelUse(lpMsg->account);
}

void CWarehouse::DGWarehouseGuildFreeSend(int ServerIndex,WORD index,char* account) // OK
{
	SDHP_WAREHOUSE_FREE_SEND pMsg;

	pMsg.header.set(0x05,0x71,sizeof(pMsg));

	pMsg.index = index;

	memcpy(pMsg.account,account,sizeof(pMsg.account));

	gSocketManager.DataSend(ServerIndex,(BYTE*)&pMsg,pMsg.header.size);
}

bool CWarehouse::WarehouseGuildAddUse(char* guild)
{
	for(std::vector<WAREHOUSEGUILD_INFO>::iterator it=this->m_WarehouseGuildInfo.begin();it != this->m_WarehouseGuildInfo.end();it++)
	{
		if(strcmp(it->guild,guild) == 0)
		{
			return 0;
		}
	}

	WAREHOUSEGUILD_INFO info;

	memcpy(info.guild,guild,sizeof(info.guild));

	this->m_WarehouseGuildInfo.push_back(info);

	return 1;
}

void CWarehouse::WarehouseGuildDelUse(char* guild)
{
	for(std::vector<WAREHOUSEGUILD_INFO>::iterator it=this->m_WarehouseGuildInfo.begin();it != this->m_WarehouseGuildInfo.end();it++)
	{
		if(strcmp(it->guild,guild) == 0)
		{
			this->m_WarehouseGuildInfo.erase(it);
			break;
		}
	}
}