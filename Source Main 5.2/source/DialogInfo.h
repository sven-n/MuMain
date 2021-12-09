// DialogInfo.h: interface for the DialogInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_DIALOGINFO_H__B38EF39B_F14E_4938_BC3D_D7DE4CE5ADCE__INCLUDED_)
#define AFX_DIALOGINFO_H__B38EF39B_F14E_4938_BC3D_D7DE4CE5ADCE__INCLUDED_

#include "interface.h"

#pragma once

namespace info
{
	BoostSmartPointer(DialogInfo);
	class DialogInfo : public InfoFile
	{
	public:
		static DialogInfoPtr MakeInfo( const std::string& filename );
		virtual ~DialogInfo();

	protected:
		virtual bool isopenfile();
		virtual void clear();

	public:
		const Script_Dialog* GetData( int index ) const;

	private:
		bool OpenFile( const std::string& filename );
		DialogInfo( const std::string& filename );

	private:
		typedef std::vector<Script_Dialog>	DialogVECTOR;

	private:
		bool				m_IsOpenFile;
		DialogVECTOR		m_Info;
		
	};
};

inline
bool info::DialogInfo::isopenfile()
{
	return m_IsOpenFile;
}

inline
void info::DialogInfo::clear()
{

}

inline
const Script_Dialog* info::DialogInfo::GetData( int index ) const
{
	if( index >= m_Info.size() ) 
		assert(0);
	return &m_Info[index];
}

#endif // !defined(AFX_DIALOGINFO_H__B38EF39B_F14E_4938_BC3D_D7DE4CE5ADCE__INCLUDED_)

#endif //INFO_BUILDER