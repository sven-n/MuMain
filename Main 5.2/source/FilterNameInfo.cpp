// FilterNameInfo.cpp: implementation of the FilterNameInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FilterNameInfo.h"

#ifdef INFO_BUILDER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace info;

FilterNameInfoPtr FilterNameInfo::MakeInfo( const std::string& filename )
{
	FilterNameInfoPtr info ( new FilterNameInfo( filename ) );
	return info;
}

FilterNameInfo::FilterNameInfo( const std::string& filename ) : InfoFile( filename ), m_IsOpenFile( false )
{
	m_IsOpenFile = OpenFile( filename );

	if( !m_IsOpenFile )
	{
		assert(0);
	}
}

FilterNameInfo::~FilterNameInfo()
{
	clear();
}

bool FilterNameInfo::OpenFile( const std::string& filename )
{
	return true;
}

#endif //INFO_BUILDER