// MovereqInfo.h: interface for the MovereqInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_MOVEREQINFO_H__27E37410_1852_4E21_BB35_AA421D61F17C__INCLUDED_)
#define AFX_MOVEREQINFO_H__27E37410_1852_4E21_BB35_AA421D61F17C__INCLUDED_

#include "interface.h"

#pragma once

namespace info
{
	BoostSmartPointer(MovereqInfo);
	class MovereqInfo : public InfoFile
	{
	private:
		bool OpenFile( const std::string& filename );
		MovereqInfo( const std::string& filename );

	public:
		static MovereqInfoPtr MakeInfo( const std::string& filename );
		virtual ~MovereqInfo();
		
	protected:
		virtual bool isopenfile();
		virtual void clear();
		
	public:
		const Script_Movereq* GetData( int index ) const;

	private:
		bool							m_IsOpenFile;
		std::vector<Script_Movereq>		m_Info;
	};
};

inline
bool info::MovereqInfo::isopenfile()
{
	return m_IsOpenFile;
}

inline
void info::MovereqInfo::clear()
{

}

inline
const info::Script_Movereq* info::MovereqInfo::GetData( int index ) const
{
	if( index >= m_Info.size() ) 
		assert(0);

	return &m_Info[index];
}

#endif // !defined(AFX_MOVEREQINFO_H__27E37410_1852_4E21_BB35_AA421D61F17C__INCLUDED_)

#endif //INFO_BUILDER