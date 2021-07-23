// w_ShopUIInfo.cpp: implementation of the ShopUIInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_ShopUIInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_BUILDER

#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzLodTerrain.h"
#include "./Utilities/Log/ErrorReport.h"

namespace
{
	const string SHOPUIINFOFILENAEM   = "Data\\Local\\partCharge1\\shopui.bmd";

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

ShopUIInfoPtr ShopUIInfo::Make()
{
	ShopUIInfoPtr info( new ShopUIInfo() );
	return info;
}

ShopUIInfo::ShopUIInfo()
{
	Initialize();
	LoadFromFile( SHOPUIINFOFILENAEM );
}

ShopUIInfo::~ShopUIInfo()
{
	Destroy();
}

void ShopUIInfo::Initialize()
{

}

void ShopUIInfo::Destroy()
{
	m_ShopInfoNoChargeMap.clear();
	m_ShopInfoChargeMap.clear();
	m_ShopInfoNoCharge.clear();
	m_ShopInfoCharge.clear();
}

void ShopUIInfo::LoadFromFile( const string& fileName )
{
	FILE *fp = fopen( fileName.c_str(), "rb" );
	if(fp != NULL)
	{
		DWORD structsize = sizeof(SHOPCATEGORY);

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
				SHOPCATEGORY shopcategory;

				BuxConvert( pSeek, structsize );
				memcpy( &shopcategory, pSeek, structsize );

				if( shopcategory.s_ShopType == eShop_NoCharge )
				{
					m_ShopInfoNoCharge.push_back( shopcategory );
				}
				else
				{
					m_ShopInfoCharge.push_back( shopcategory );
				}

				pSeek += structsize;
			}
		}
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

void ShopUIInfo::QueryHighShopCategory( eShopType shoptype, ShopCategory_List& outdata )
{
	ShopCategory_List& shopcategory = (shoptype==eShop_NoCharge) ? m_ShopInfoNoCharge : m_ShopInfoCharge;

	for( ShopCategory_List::iterator iter = shopcategory.begin(); iter != shopcategory.end(); )
	{
		ShopCategory_List::iterator curiter = iter;
		++iter;
		SHOPCATEGORY& data = *curiter;

		if( data.s_LowCategory == eLowCategory_None )
		{
			outdata.push_back( data );
		}
	}
}

void ShopUIInfo::QueryLowShopCategory( eShopType shoptype, eHighCategoryType highcategory, ShopCategory_List& outdata )
{
	ShopCategory_Map& shopcategorymap = (shoptype==eShop_NoCharge) ? m_ShopInfoNoChargeMap : m_ShopInfoChargeMap;
	ShopCategory_List& shopcategory   = (shoptype==eShop_NoCharge) ? m_ShopInfoNoCharge : m_ShopInfoCharge;

	ShopCategory_Map::iterator iter = shopcategorymap.find( highcategory );

	if( iter == shopcategorymap.end() )
	{
		for( ShopCategory_List::iterator liter = shopcategory.begin(); liter != shopcategory.end(); )
		{
			ShopCategory_List::iterator curiter = liter;
			++liter;
			SHOPCATEGORY& data = *curiter;

			if( data.s_HighCategory == highcategory && data.s_LowCategory != eLowCategory_None )
			{
				outdata.push_back( data );
			}
		}

		shopcategorymap.insert(make_pair(highcategory, outdata));
	}
	else
	{
		outdata = (*iter).second;
	}
}

#endif //NEW_USER_INTERFACE_BUILDER