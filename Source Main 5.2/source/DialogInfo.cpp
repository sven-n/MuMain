// DialogInfo.cpp: implementation of the DialogInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DialogInfo.h"

#ifdef INFO_BUILDER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace info;

DialogInfoPtr DialogInfo::MakeInfo( const std::string& filename )
{
	DialogInfoPtr info ( new DialogInfo( filename ) );
	return info;
}

DialogInfo::DialogInfo( const std::string& filename ) : InfoFile( filename ), m_IsOpenFile( false )
{
	m_IsOpenFile = OpenFile( filename );

	if( !m_IsOpenFile )
	{
		assert(0);
	}
}

DialogInfo::~DialogInfo()
{
	clear();
}

bool DialogInfo::OpenFile( const std::string& filename )
{
	return true;
}

#endif //INFO_BUILDER