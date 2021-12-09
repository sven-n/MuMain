// SkillInfo.h: interface for the SkillInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_SKILLINFO_H__3FF63040_0D46_4FFC_BFD9_86B9CE56C47A__INCLUDED_)
#define AFX_SKILLINFO_H__3FF63040_0D46_4FFC_BFD9_86B9CE56C47A__INCLUDED_

#include "interface.h"
#include "ZzzInfomation.h"

#pragma once

namespace info
{
	BoostSmartPointer(SkillInfo);
	class SkillInfo : public InfoFile
	{
	public:
		static SkillInfoPtr MakeInfo( const std::string& filename );
		virtual ~SkillInfo();

	protected:
		virtual bool isopenfile();
		virtual void clear();

	public:
		const Script_Skill* GetData( int index ) const;

	private:
		bool OpenFile( const std::string& filename );
		SkillInfo( const std::string& filename );

	private:
		typedef std::vector<Script_Skill>	SkillVECTOR;

	private:
		bool			m_IsOpenFile;
		SkillVECTOR     m_Info;
	};
};

inline
bool info::SkillInfo::isopenfile()
{
	return m_IsOpenFile;
}

inline
void info::SkillInfo::clear()
{

}

inline
const info::Script_Skill* info::SkillInfo::GetData( int index ) const
{
	if( index >= m_Info.size() ) 
		assert(0);
	return &m_Info[index];
}

#endif // !defined(AFX_SKILLINFO_H__3FF63040_0D46_4FFC_BFD9_86B9CE56C47A__INCLUDED_)

#endif //INFO_BUILDER