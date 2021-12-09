// SlideInfo.cpp: implementation of the SlideInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SlideInfo.h"

#ifdef INFO_BUILDER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace info;

SlideInfoPtr SlideInfo::MakeInfo( const std::string& filename )
{
	SlideInfoPtr info ( new SlideInfo( filename ) );
	return info;
}

SlideInfo::SlideInfo( const std::string& filename ) : InfoFile( filename ), m_IsOpenFile( false )
{
	m_IsOpenFile = OpenFile( filename );

	if( !m_IsOpenFile )
	{
		assert(0);
	}
}

SlideInfo::~SlideInfo()
{
	clear();
}

bool SlideInfo::OpenFile( const std::string& filename )
{
	return true;
}

#endif //INFO_BUILDER
