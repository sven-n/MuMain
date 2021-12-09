// QuestInfo.h: interface for the QuestInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_QUESTINFO_H__DDBFB715_E296_4C77_8E81_5A1EE2ABBF09__INCLUDED_)
#define AFX_QUESTINFO_H__DDBFB715_E296_4C77_8E81_5A1EE2ABBF09__INCLUDED_

#include "interface.h"

#pragma once

namespace info
{
	BoostSmartPointer(QuestInfo);
	class QuestInfo : public InfoFile
	{
	private:
		bool OpenFile( const std::string& filename );
		QuestInfo( const std::string& filename );

	public:
		static QuestInfoPtr MakeInfo( const std::string& filename );
		virtual ~QuestInfo();

	protected:
		virtual bool isopenfile();
		virtual void clear();

	public:
		const Script_Quest* GetData( int index ) const;

	private:
		bool							m_IsOpenFile;
		std::vector<Script_Quest>		m_Info;
	};
};

inline
bool info::QuestInfo::isopenfile()
{
	return m_IsOpenFile;
}

inline
void info::QuestInfo::clear()
{

}

inline
const info::Script_Quest* info::QuestInfo::GetData( int index ) const 
{ 
	if( index >= m_Info.size() ) 
		assert(0);
	return &m_Info[index];
 }

#endif // !defined(AFX_QUESTINFO_H__DDBFB715_E296_4C77_8E81_5A1EE2ABBF09__INCLUDED_)

#endif //INFO_BUILDER