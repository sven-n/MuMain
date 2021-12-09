// CreditInfo.cpp: implementation of the CreditInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CreditInfo.h"

#ifdef INFO_BUILDER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace info;

CreditInfoPtr CreditInfo::MakeInfo( const std::string& filename )
{
	CreditInfo* info ( new CreditInfo( filename ) );
	return info;
}

CreditInfo::CreditInfo( const std::string& filename ) : InfoFile( filename ), m_IsOpenFile( false )
{
	m_IsOpenFile = OpenFile( filename );

	if( !m_IsOpenFile )
	{
		assert(0);
	}
}

CreditInfo::~CreditInfo()
{
	clear();
}

bool CreditInfo::OpenFile( const std::string& filename )
{
	return true;
}

#endif //INFO_BUILDER