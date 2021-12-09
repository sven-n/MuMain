// CashShop.cpp: implementation of the CCashShop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CashShop.h"
#include "QueryManager.h"
#include "SocketManager.h"

CCashShop gCashShop;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCashShop::CCashShop() // OK
{

}

CCashShop::~CCashShop() // OK
{

}

void CCashShop::GDCashShopPointRecv(SDHP_CASH_SHOP_POINT_RECV* lpMsg,int index) // OK
{
	SDHP_CASH_SHOP_POINT_SEND pMsg;

	pMsg.header.set(0x18,0x00,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	if(gQueryManager.ExecQuery("SELECT * FROM CashShopData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		if(gQueryManager.ExecQuery("INSERT INTO CashShopData (AccountID,WCoinC,WCoinP,GoblinPoint) VALUES ('%s',0,0,0)",lpMsg->account) == 0)
		{
			gQueryManager.Close();

			pMsg.result = 1;
		}
		else
		{
			gQueryManager.Close();

			pMsg.WCoinC = 0;

			pMsg.WCoinP = 0;

			pMsg.GoblinPoint = 0;
		}
	}
	else
	{
		pMsg.WCoinC = gQueryManager.GetAsInteger("WCoinC");

		pMsg.WCoinP = gQueryManager.GetAsInteger("WCoinP");

		pMsg.GoblinPoint = gQueryManager.GetAsInteger("GoblinPoint");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void CCashShop::GDCashShopItemBuyRecv(SDHP_CASH_SHOP_ITEM_BUY_RECV* lpMsg,int index) // OK
{
	SDHP_CASH_SHOP_ITEM_BUY_SEND pMsg;

	pMsg.header.set(0x18,0x01,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	pMsg.PackageMainIndex = lpMsg->PackageMainIndex;

	pMsg.Category = lpMsg->Category;

	pMsg.ProductMainIndex = lpMsg->ProductMainIndex;

	pMsg.ItemIndex = lpMsg->ItemIndex;

	pMsg.CoinIndex = lpMsg->CoinIndex;

	pMsg.MileageFlag = lpMsg->MileageFlag;

	if(gQueryManager.ExecQuery("SELECT * FROM CashShopData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.WCoinC = gQueryManager.GetAsInteger("WCoinC");

		pMsg.WCoinP = gQueryManager.GetAsInteger("WCoinP");

		pMsg.GoblinPoint = gQueryManager.GetAsInteger("GoblinPoint");

		gQueryManager.Close();

		if(gQueryManager.ExecQuery("SELECT count(*) FROM CashShopInventory WHERE AccountID='%s' AND InventoryType=%d",lpMsg->account,83) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.result = 1;
		}
		else
		{
			pMsg.ItemCount = gQueryManager.GetResult(0);

			gQueryManager.Close();
		}
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void CCashShop::GDCashShopItemGifRecv(SDHP_CASH_SHOP_ITEM_GIF_RECV* lpMsg,int index) // OK
{
	SDHP_CASH_SHOP_ITEM_GIF_SEND pMsg;

	pMsg.header.set(0x18,0x02,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	pMsg.PackageMainIndex = lpMsg->PackageMainIndex;

	pMsg.Category = lpMsg->Category;

	pMsg.ProductMainIndex = lpMsg->ProductMainIndex;

	pMsg.SaleZone = lpMsg->SaleZone;

	pMsg.ItemIndex = lpMsg->ItemIndex;

	pMsg.CoinIndex = lpMsg->CoinIndex;

	pMsg.MileageFlag = lpMsg->MileageFlag;

	memcpy(pMsg.GiftName,lpMsg->GiftName,sizeof(pMsg.GiftName));

	memcpy(pMsg.GiftText,lpMsg->GiftText,sizeof(pMsg.GiftText));

	gQueryManager.BindParameterAsString(1,lpMsg->GiftName,sizeof(lpMsg->GiftName));

	if(gQueryManager.ExecQuery("SELECT AccountID FROM Character WHERE Name=?") == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		gQueryManager.GetAsString("AccountID",pMsg.GiftAccount,sizeof(pMsg.GiftAccount));

		gQueryManager.Close();

		if(gQueryManager.ExecQuery("SELECT * FROM CashShopData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();

			pMsg.result = 1;
		}
		else
		{
			pMsg.WCoinC = gQueryManager.GetAsInteger("WCoinC");

			pMsg.WCoinP = gQueryManager.GetAsInteger("WCoinP");

			pMsg.GoblinPoint = gQueryManager.GetAsInteger("GoblinPoint");

			gQueryManager.Close();

			if(gQueryManager.ExecQuery("SELECT count(*) FROM CashShopInventory WHERE AccountID='%s' AND InventoryType=%d",pMsg.GiftAccount,71) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
			{
				gQueryManager.Close();

				pMsg.result = 1;
			}
			else
			{
				pMsg.ItemCount = gQueryManager.GetResult(0);

				gQueryManager.Close();
			}
		}
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void CCashShop::GDCashShopItemNumRecv(SDHP_CASH_SHOP_ITEM_NUM_RECV* lpMsg,int index) // OK
{
	SDHP_CASH_SHOP_ITEM_NUM_SEND pMsg;

	pMsg.header.set(0x18,0x03,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	pMsg.InventoryPage = lpMsg->InventoryPage;

	pMsg.InventoryType = lpMsg->InventoryType;

	pMsg.ItemCount = 0;

	pMsg.PageCount = 0;

	if(gQueryManager.ExecQuery("SELECT * FROM CashShopInventory WHERE AccountID='%s' AND InventoryType=%d",lpMsg->account,lpMsg->InventoryType) == 0)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		while(gQueryManager.Fetch() != SQL_NO_DATA)
		{
			if((((pMsg.ItemCount++)/MAX_CASH_SHOP_PAGE_ITEM)+1) == lpMsg->InventoryPage)
			{
				pMsg.ProductInfo[pMsg.PageCount].BaseItemCode = gQueryManager.GetAsInteger("BaseItemCode");

				pMsg.ProductInfo[pMsg.PageCount].MainItemCode = gQueryManager.GetAsInteger("MainItemCode");

				pMsg.ProductInfo[pMsg.PageCount].PackageMainIndex = gQueryManager.GetAsInteger("PackageMainIndex");

				pMsg.ProductInfo[pMsg.PageCount].ProductBaseIndex = gQueryManager.GetAsInteger("ProductBaseIndex");

				pMsg.ProductInfo[pMsg.PageCount].ProductMainIndex = gQueryManager.GetAsInteger("ProductMainIndex");

				pMsg.ProductInfo[pMsg.PageCount].CoinValue = gQueryManager.GetAsFloat("CoinValue");

				pMsg.ProductInfo[pMsg.PageCount].ProductType = gQueryManager.GetAsInteger("ProductType");

				gQueryManager.GetAsString("GiftName",pMsg.ProductInfo[pMsg.PageCount].GiftName,sizeof(pMsg.ProductInfo[pMsg.PageCount].GiftName));

				gQueryManager.GetAsString("GiftText",pMsg.ProductInfo[pMsg.PageCount].GiftText,sizeof(pMsg.ProductInfo[pMsg.PageCount].GiftText));

				pMsg.PageCount++;
			}
		}

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void CCashShop::GDCashShopItemUseRecv(SDHP_CASH_SHOP_ITEM_USE_RECV* lpMsg,int index) // OK
{
	SDHP_CASH_SHOP_ITEM_USE_SEND pMsg;

	pMsg.header.set(0x18,0x04,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.result = 0;

	pMsg.BaseItemCode = lpMsg->BaseItemCode;

	pMsg.MainItemCode = lpMsg->MainItemCode;

	pMsg.ItemIndex = lpMsg->ItemIndex;

	pMsg.ProductType = lpMsg->ProductType;

	if(gQueryManager.ExecQuery("SELECT * FROM CashShopInventory WHERE BaseItemCode=%d",lpMsg->BaseItemCode) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.result = 1;
	}
	else
	{
		pMsg.ProductInfo.BaseItemCode = gQueryManager.GetAsInteger("BaseItemCode");

		pMsg.ProductInfo.MainItemCode = gQueryManager.GetAsInteger("MainItemCode");

		pMsg.ProductInfo.PackageMainIndex = gQueryManager.GetAsInteger("PackageMainIndex");

		pMsg.ProductInfo.ProductBaseIndex = gQueryManager.GetAsInteger("ProductBaseIndex");

		pMsg.ProductInfo.ProductMainIndex = gQueryManager.GetAsInteger("ProductMainIndex");

		pMsg.ProductInfo.CoinValue = gQueryManager.GetAsFloat("CoinValue");

		pMsg.ProductInfo.ProductType = gQueryManager.GetAsInteger("ProductType");

		gQueryManager.GetAsString("GiftName",pMsg.ProductInfo.GiftName,sizeof(pMsg.ProductInfo.GiftName));

		gQueryManager.GetAsString("GiftText",pMsg.ProductInfo.GiftText,sizeof(pMsg.ProductInfo.GiftText));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void CCashShop::GDCashShopPeriodicItemRecv(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV* lpMsg,int index) // OK
{
	BYTE send[4096];

	SDHP_CASH_SHOP_PERIODIC_ITEM_SEND pMsg;

	pMsg.header.set(0x18,0x05,0);

	int size = sizeof(pMsg);

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.count = 0;

	SDHP_CASH_SHOP_PERIODIC_ITEM2 info;

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_CASH_SHOP_PERIODIC_ITEM1* lpInfo = (SDHP_CASH_SHOP_PERIODIC_ITEM1*)(((BYTE*)lpMsg)+sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_RECV)+(sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM1)*n));

		if(gQueryManager.ExecQuery("SELECT Time FROM CashShopPeriodicItem WHERE ItemSerial=%d",lpInfo->serial) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("INSERT INTO CashShopPeriodicItem (ItemSerial,Time) VALUES (%d,%d)",lpInfo->serial,0);
			gQueryManager.Close();

			info.slot = lpInfo->slot;
			info.serial = lpInfo->serial;
			info.time = 0;
		}
		else
		{
			info.slot = lpInfo->slot;
			info.serial = lpInfo->serial;
			info.time = gQueryManager.GetAsInteger("Time");

			gQueryManager.Close();
		}

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	gSocketManager.DataSend(index,send,size);
}

void CCashShop::GDCashShopRecievePointRecv(SDHP_CASH_SHOP_RECIEVE_POINT_RECV* lpMsg,int index) // OK
{
	SDHP_CASH_SHOP_RECIEVE_POINT_SEND pMsg;

	pMsg.header.set(0x18,0x06,sizeof(pMsg));

	pMsg.index = lpMsg->index;

	memcpy(pMsg.account,lpMsg->account,sizeof(pMsg.account));

	pMsg.CallbackFunc = lpMsg->CallbackFunc;

	pMsg.CallbackArg1 = lpMsg->CallbackArg1;

	pMsg.CallbackArg2 = lpMsg->CallbackArg2;

	if(gQueryManager.ExecQuery("SELECT * FROM CashShopData WHERE AccountID='%s'",lpMsg->account) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		pMsg.WCoinC = 0;

		pMsg.WCoinP = 0;

		pMsg.GoblinPoint = 0;
	}
	else
	{
		pMsg.WCoinC = gQueryManager.GetAsInteger("WCoinC");

		pMsg.WCoinP = gQueryManager.GetAsInteger("WCoinP");

		pMsg.GoblinPoint = gQueryManager.GetAsInteger("GoblinPoint");

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index,(BYTE*)&pMsg,sizeof(pMsg));
}

void CCashShop::GDCashShopAddPointSaveRecv(SDHP_CASH_SHOP_ADD_POINT_SAVE_RECV* lpMsg) // OK
{
	char TargetAccount[11];

	memcpy(TargetAccount,((lpMsg->GiftAccount[0]==0)?lpMsg->account:lpMsg->GiftAccount),sizeof(TargetAccount));

	if(gQueryManager.ExecQuery("SELECT WCoinC,WCoinP,GoblinPoint FROM CashShopData WHERE AccountID='%s'",TargetAccount) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO CashShopData (AccountID,WCoinC,WCoinP,GoblinPoint) VALUES ('%s',%d,%d,%d)",TargetAccount,lpMsg->AddWCoinC,lpMsg->AddWCoinP,lpMsg->AddGoblinPoint);

		gQueryManager.Close();
	}
	else
	{
		DWORD WCoinC = gQueryManager.GetAsInteger("WCoinC");

		DWORD WCoinP = gQueryManager.GetAsInteger("WCoinP");

		DWORD GoblinPoint = gQueryManager.GetAsInteger("GoblinPoint");

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE CashShopData SET WCoinC=%d,WCoinP=%d,GoblinPoint=%d WHERE AccountID='%s'",(((WCoinC+lpMsg->AddWCoinC)>0x7FFFFFFF)?0x7FFFFFFF:(WCoinC+lpMsg->AddWCoinC)),(((WCoinP+lpMsg->AddWCoinP)>0x7FFFFFFF)?0x7FFFFFFF:(WCoinP+lpMsg->AddWCoinP)),(((GoblinPoint+lpMsg->AddGoblinPoint)>0x7FFFFFFF)?0x7FFFFFFF:(GoblinPoint+lpMsg->AddGoblinPoint)),TargetAccount);

		gQueryManager.Close();
	}
}

void CCashShop::GDCashShopSubPointSaveRecv(SDHP_CASH_SHOP_SUB_POINT_SAVE_RECV* lpMsg) // OK
{
	char TargetAccount[11];

	memcpy(TargetAccount,((lpMsg->GiftAccount[0]==0)?lpMsg->account:lpMsg->GiftAccount),sizeof(TargetAccount));

	if(gQueryManager.ExecQuery("SELECT WCoinC,WCoinP,GoblinPoint FROM CashShopData WHERE AccountID='%s'",TargetAccount) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
	{
		gQueryManager.Close();

		gQueryManager.ExecQuery("INSERT INTO CashShopData (AccountID,WCoinC,WCoinP,GoblinPoint) VALUES ('%s',%d,%d,%d)",TargetAccount,0,0,0);

		gQueryManager.Close();
	}
	else
	{
		DWORD WCoinC = gQueryManager.GetAsInteger("WCoinC");

		DWORD WCoinP = gQueryManager.GetAsInteger("WCoinP");

		DWORD GoblinPoint = gQueryManager.GetAsInteger("GoblinPoint");

		gQueryManager.Close();

		gQueryManager.ExecQuery("UPDATE CashShopData SET WCoinC=%d,WCoinP=%d,GoblinPoint=%d WHERE AccountID='%s'",((lpMsg->SubWCoinC>WCoinC)?0:(WCoinC-lpMsg->SubWCoinC)),((lpMsg->SubWCoinP>WCoinP)?0:(WCoinP-lpMsg->SubWCoinP)),((lpMsg->SubGoblinPoint>GoblinPoint)?0:(GoblinPoint-lpMsg->SubGoblinPoint)),TargetAccount);

		gQueryManager.Close();

		if (lpMsg->SubWCoinC > 0 )
		{
		gQueryManager.ExecQuery("INSERT INTO LOG_CREDITOS (login,valor,tipo) VALUES ('%s',%d,%d)",TargetAccount,((lpMsg->SubWCoinC>WCoinC)?0:(-lpMsg->SubWCoinC)),4);

		gQueryManager.Close();
		}
	}
}

void CCashShop::GDCashShopInsertItemSaveRecv(SDHP_CASH_SHOP_INSERT_ITEM_SAVE_RECV* lpMsg) // OK
{
	char TargetAccount[11];

	memcpy(TargetAccount,((lpMsg->GiftAccount[0]==0)?lpMsg->account:lpMsg->GiftAccount),sizeof(TargetAccount));

	gQueryManager.BindParameterAsString(1,lpMsg->GiftName,sizeof(lpMsg->GiftName));

	gQueryManager.BindParameterAsString(2,lpMsg->GiftText,sizeof(lpMsg->GiftText));

	gQueryManager.ExecQuery("INSERT INTO CashShopInventory (MainItemCode,AccountID,InventoryType,PackageMainIndex,ProductBaseIndex,ProductMainIndex,CoinValue,ProductType,GiftName,GiftText) VALUES (%d,'%s',%d,%d,%d,%d,%f,%d,?,?)",0,TargetAccount,lpMsg->InventoryType,lpMsg->PackageMainIndex,lpMsg->ProductBaseIndex,lpMsg->ProductMainIndex,lpMsg->CoinValue,lpMsg->ProductType);

	gQueryManager.Close();
}

void CCashShop::GDCashShopDeleteItemSaveRecv(SDHP_CASH_SHOP_DELETE_ITEM_SAVE_RECV* lpMsg) // OK
{
	char TargetAccount[11];

	memcpy(TargetAccount,((lpMsg->GiftAccount[0]==0)?lpMsg->account:lpMsg->GiftAccount),sizeof(TargetAccount));

	gQueryManager.ExecQuery("DELETE FROM CashShopInventory WHERE BaseItemCode=%d",lpMsg->BaseItemCode);
	gQueryManager.Close();
}

void CCashShop::GDCashShopPeriodicItemSaveRecv(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV* lpMsg) // OK
{
	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE* lpInfo = (SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE*)(((BYTE*)lpMsg)+sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE_RECV)+(sizeof(SDHP_CASH_SHOP_PERIODIC_ITEM_SAVE)*n));

		if(gQueryManager.ExecQuery("SELECT ItemSerial FROM CashShopPeriodicItem WHERE ItemSerial=%d",lpInfo->serial) == 0 || gQueryManager.Fetch() == SQL_NO_DATA)
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("INSERT INTO CashShopPeriodicItem (ItemSerial,Time) VALUES (%d,%d)",lpInfo->serial,lpInfo->time);
			gQueryManager.Close();
		}
		else
		{
			gQueryManager.Close();
			gQueryManager.ExecQuery("UPDATE CashShopPeriodicItem SET Time=%d WHERE ItemSerial=%d",lpInfo->time,lpInfo->serial);
			gQueryManager.Close();
		}
	}
}
