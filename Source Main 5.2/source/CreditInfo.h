// CreditInfo.h: interface for the CreditInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_CREDITINFO_H__39B91D7E_6041_459E_822C_DEBCAD9281B0__INCLUDED_)
#define AFX_CREDITINFO_H__39B91D7E_6041_459E_822C_DEBCAD9281B0__INCLUDED_

#include "interface.h"

#pragma once

namespace info
{
	BoostSmartPointer(CreditInfo);
	class CreditInfo : public InfoFile
	{
	public:
		static CreditInfoPtr MakeInfo( const std::string& filename );
		virtual ~CreditInfo();

	protected:
		virtual bool isopenfile();
		virtual void clear();

	public:
		const Script_Credit* GetData( int index ) const;

	private:
		bool OpenFile( const std::string& filename );
		CreditInfo( const std::string& filename );

	private:
		typedef std::vector<Script_Credit>		CreditVECTOR;

	private:
		bool				m_IsOpenFile;
		CreditVECTOR		m_Info;// 400

	};
};

inline
bool info::CreditInfo::isopenfile()
{
	return m_IsOpenFile;
}

inline
void info::CreditInfo::clear()
{

}

inline
const info::Script_Credit* info::CreditInfo::GetData( int index ) const
{
	if( index >= m_Info.size() ) 
		assert(0);
	return &m_Info[index];
}

#endif // !defined(AFX_CREDITINFO_H__39B91D7E_6041_459E_822C_DEBCAD9281B0__INCLUDED_)

#endif //INFO_BUILDER