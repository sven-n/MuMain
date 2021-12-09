// ItemInfo.cpp: implementation of the ItemInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemInfo.h"

#ifdef INFO_BUILDER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace info;

ItemInfoPtr ItemInfo::MakeInfo( const std::string& filename )
{
	ItemInfoPtr info ( new ItemInfo( filename ) );
	return info;
}

ItemInfo::ItemInfo( const std::string& filename ) : InfoFile( filename ), m_IsOpenFile( false )
{
	m_Info           = new Script_Item[(16 * 512) + 1024];

	m_IsOpenFile = OpenFile( filename );

	if( !m_IsOpenFile )
	{
		assert( 0 );
	}
}

ItemInfo::~ItemInfo()
{
	clear();
}

bool ItemInfo::OpenFile( const std::string& filename )
{
	return true;
}

#endif //INFO_BUILDER