// w_ShopItemInfo.cpp: implementation of the ShopItemInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_ShopItemInfo.h"

#ifdef NEW_USER_INTERFACE_BUILDER

#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzLodTerrain.h"
#include "./Utilities/Log/ErrorReport.h"

namespace
{
	const string SHOPITEMINFOFILENAEM = "Data\\Local\\partCharge1\\ShopCategoryItems.bmd";

	BYTE bBuxCode[3] = {0xfc,0xcf,0xab};
	void BuxConvert(BYTE *Buffer, int Size)
	{
		for(int i=0;i<Size;i++)
			Buffer[i] ^= bBuxCode[i%3];
	}

	wchar_t wBuxCode[3] = {0xcd56, 0x1d93, 0x5b81};
	void BuxConvertW(wchar_t *Buffer, int Size)
	{
		for(int i=0;i<Size;i++)
			Buffer[i] ^= wBuxCode[i%3];
	}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ShopItemInfoPtr ShopItemInfo::Make()
{
	ShopItemInfoPtr info( new ShopItemInfo() );
	return info;
}

ShopItemInfo::ShopItemInfo()
{
	Initialize();
	LoadFromFile( SHOPITEMINFOFILENAEM );
}

ShopItemInfo::~ShopItemInfo()
{
	Destroy();
}

void ShopItemInfo::Initialize()
{

}

void ShopItemInfo::Destroy()
{

}

void ShopItemInfo::LoadFromFile( const string& fileName )
{
	FILE *fp = fopen( fileName.c_str(), "rb" );
	if(fp != NULL)
	{
		DWORD structsize = sizeof(SHOPCATEGORYITEM);

		DWORD listsize;
		fread( &listsize, sizeof(DWORD), 1, fp );

		BYTE *Buffer = new BYTE [structsize*listsize];
		fread( Buffer, structsize*listsize, 1, fp );

		DWORD dwCheckSum;
		fread( &dwCheckSum, sizeof(DWORD), 1, fp );

		fclose(fp);
		if ( dwCheckSum != GenerateCheckSum2( Buffer, structsize*listsize, 0xE2F1))
		{
			char Text[256];
    		sprintf( Text, "%s - File corrupted.", fileName );
			g_ErrorReport.Write( Text );
			MessageBox(g_hWnd,Text,NULL,MB_OK);
			SendMessage(g_hWnd,WM_DESTROY,0,0);
		}
		else
		{
			BYTE *pSeek = Buffer;

			for( int i = 0;i < listsize; i++ )
			{
				SHOPCATEGORYITEM shopcategoryitem;

				BuxConvert( pSeek, structsize );
				memcpy( &shopcategoryitem, pSeek, structsize );

				//좀 더 손을 보자...!
				if( shopcategoryitem.s_bShoplistCode == eShop_NoCharge )
				{
					ShopCategoryItem_Map Temp;
					Temp.insert( make_pair( shopcategoryitem.s_bItemCodeIndex, shopcategoryitem ) );
					m_ShopInfoNoChargeItem.insert( make_pair( shopcategoryitem.s_bItemCodeType, Temp ) );
				}
				else
				{
					ShopCategoryItem_Map Temp;
					Temp.insert( make_pair( shopcategoryitem.s_bItemCodeIndex, shopcategoryitem ) );
					m_ShopInfoChargeItem.insert( make_pair( shopcategoryitem.s_bItemCodeType, Temp ) );
				}

				pSeek += structsize;
			}
		}

		int a = m_ShopInfoChargeItem.size();

		delete [] Buffer;
	}
	else
	{
		char Text[256];
    	sprintf( Text, "%s - File not exist.", fileName );
		g_ErrorReport.Write( Text );
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}
}

SHOPCATEGORYITEM& ShopItemInfo::QueryShopCategoryItem( SHOPCATEGORYITEM& scitem, const eShopType type, const int itemtype, const int itemindex )
{
	ShopItemInfo_Map& shopcategoryitems = (type==eShop_NoCharge) ? m_ShopInfoNoChargeItem : m_ShopInfoChargeItem;

	ShopItemInfo_Map::iterator iter = shopcategoryitems.find( itemtype );

	if( iter == shopcategoryitems.end() )
	{
		assert(0);
		return scitem;
	}
	else
	{
		ShopCategoryItem_Map& shopcategoryitems = (*iter).second;

		ShopCategoryItem_Map::iterator iter2 = shopcategoryitems.find( itemindex );

		if( iter2 == shopcategoryitems.end() )
		{
			assert(0);
			return scitem;
		}
		else
		{
			scitem = (*iter2).second;
			return scitem;
		}
	}
}

#endif //NEW_USER_INTERFACE_BUILDER