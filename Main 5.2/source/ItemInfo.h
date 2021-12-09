// ItemInfo.h: interface for the ItemInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_ITEMINFO_H__FC15E73F_9DCB_4CDD_B324_59A99FFA73BB__INCLUDED_)
#define AFX_ITEMINFO_H__FC15E73F_9DCB_4CDD_B324_59A99FFA73BB__INCLUDED_

#include "interface.h"

#pragma once

namespace info
{

BoostSmartPointer(ItemInfo);
class ItemInfo : public InfoFile
{
public:
	static ItemInfoPtr MakeInfo( const std::string& filename );
	virtual ~ItemInfo();
	
protected:
	virtual bool isopenfile();
	virtual void clear();
	
public:
	const Script_Item* GetData( int index ) const;

private:
	bool OpenFile( const std::string& filename );
	ItemInfo( const std::string& filename );

private:
	bool					m_IsOpenFile;
	Script_Item				m_Info[ITEM_ETC+MAX_ITEM_INDEX];
};

};

inline
bool info::ItemInfo::isopenfile()
{
	return m_IsOpenFile;
}

inline
void info::ItemInfo::clear()
{

}

inline
const info::Script_Item* info::ItemInfo::GetData( int index ) const
{
	//예외 처리 해둘것.
	if( index >= ITEM_ETC+MAX_ITEM_INDEX ) {
		assert(0);
	}
	return &m_Info[index];
}

#endif // !defined(AFX_ITEMINFO_H__FC15E73F_9DCB_4CDD_B324_59A99FFA73BB__INCLUDED_)

#endif //INFO_BUILDER