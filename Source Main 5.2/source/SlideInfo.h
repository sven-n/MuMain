// SlideInfo.h: interface for the SlideInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_SLIDEINFO_H__4DEEE415_8F00_411D_8EDF_3AC277D1DC4C__INCLUDED_)
#define AFX_SLIDEINFO_H__4DEEE415_8F00_411D_8EDF_3AC277D1DC4C__INCLUDED_

#include "interface.h"

#pragma once

namespace info
{
	BoostSmartPointer(SlideInfo);
	class SlideInfo : public InfoFile 
	{
	public:
		static SlideInfoPtr MakeInfo( const std::string& filename );
		virtual ~SlideInfo();

	protected:
		virtual bool isopenfile();
		virtual void clear();

	public:
		const char* GetData( int level, int index );

	private:
		bool OpenFile( const std::string& filename );
		SlideInfo( const std::string& filename );

	private:
		typedef std::map<int, Script_Silde>		SildeMAP;

	private:
		bool				m_IsOpenFile;
		SildeMAP			m_info;

	};
};

inline
bool info::SlideInfo::isopenfile()
{
	return m_IsOpenFile;
}

inline
void info::SlideInfo::clear()
{

}

inline
const char* info::SlideInfo::GetData( int level, int index )
{
	info::SlideInfo::SildeMAP::iterator iter = m_info.find( level );
	if( iter != m_info.end() ) {
		Script_Silde datalist = (*iter).second;
		if( index < datalist.Sildelist.size() )
			return datalist.Sildelist[index];
	}
	assert( 0 );
	return NULL;
}

#endif // !defined(AFX_SLIDEINFO_H__4DEEE415_8F00_411D_8EDF_3AC277D1DC4C__INCLUDED_)

#endif //INFO_BUILDER