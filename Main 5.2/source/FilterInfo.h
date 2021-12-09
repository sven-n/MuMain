// FilterInfo.h: interface for the FilterInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_FILTERINFO_H__42939AF8_97D4_4FB3_B22F_4ED1A17E460D__INCLUDED_)
#define AFX_FILTERINFO_H__42939AF8_97D4_4FB3_B22F_4ED1A17E460D__INCLUDED_

#include "interface.h"

#pragma once

namespace info
{
	BoostSmartPointer(FilterInfo);
	class FilterInfo : public InfoFile
	{
	private:
		bool OpenFile( const std::string& filename ); 
		FilterInfo( const std::string& filename );

	public:
		static FilterInfoPtr MakeInfo( const std::string& filename );
		virtual ~FilterInfo();

	protected:
		virtual bool isopenfile();
		virtual void clear();

	public:
		const char*	GetData( int index ) const;

	private:
		char			m_info[1000][20];
		bool			m_IsOpenFile;

	};
};

inline
bool info::FilterInfo::isopenfile()
{
	return m_IsOpenFile;
}

inline
void info::FilterInfo::clear()
{
	
}

inline
const char* info::FilterInfo::GetData( int index ) const
{ 
	if( index >= 1000 ) 
		assert(0);
	return m_info[index]; 
}

#endif // !defined(AFX_FILTERINFO_H__42939AF8_97D4_4FB3_B22F_4ED1A17E460D__INCLUDED_)

#endif //INFO_BUILDER