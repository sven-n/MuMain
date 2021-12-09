// ExplanationInfo.cpp: implementation of the ExplanationInfo class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ExplanationInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

using namespace info;

ExplanationInfoPtr ExplanationInfo::MakeInfo( const std::string& filename )
{
	ExplanationInfoPtr info ( new ExplanationInfo( filename ) );
	return info;
}

ExplanationInfo::ExplanationInfo( const std::string& filename ) : InfoFile( filename ), m_IsOpenFile( false )
{
	m_IsOpenFile = OpenFile( filename );

	if(  !m_IsOpenFile )
	{
		assert( 0 );
	}
}

ExplanationInfo::~ExplanationInfo()
{
	clear();
}

bool ExplanationInfo::OpenFile( const std::string& filename )
{
	return true;
}

#endif //INFO_BUILDER