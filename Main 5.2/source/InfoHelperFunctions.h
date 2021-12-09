#pragma once

#include "Builder.h"

#ifdef INFO_BUILDER

namespace info
{
	//data
	class ExplanationInfo;
	class ItemInfo;
	class SkillInfo;
	class SlideInfo;
	class DialogInfo;
	class CreditInfo;

	//administer
	class Builder;
}

////////////////////////////////////////////////////builder///////////////////////////////////////////////////////////

inline
info::Builder&						TheBuilderInfo()
{
	assert( info::Builder::IsInitialized() );
	return g_BuilderINFO;
}

////////////////////////////////////////////////////text file///////////////////////////////////////////////////////////

inline
const char*							TheGolbalText( int index )
{
	return TheBuilderInfo().QueryInfo( info::eInfo_Text_File )->getinfodataTEXT( index );
}

////////////////////////////////////////////////////item file/////////////////////////////////////////////////////////////

inline
const info::Script_Item*			TheItemText( int index )
{
	return TheBuilderInfo().QueryInfo( info::eInfo_Item_File )->getinfodataITEM( index );
}

////////////////////////////////////////////////////skill file/////////////////////////////////////////////////////////////

inline
const info::Script_Skill*			TheSkillText( int index )
{
	return g_BuilderINFO.QueryInfo( info::eInfo_Skill_File )->getinfodataSKILL( index );
}

////////////////////////////////////////////////////silde file/////////////////////////////////////////////////////////////

inline
const char*			TheSildeText( int level, int index )
{
	return TheBuilderInfo().QueryInfo( info::eInfo_Slide_File )->getinfodataSILDE( level, index );
}

////////////////////////////////////////////////////credit file/////////////////////////////////////////////////////////////

inline
const info::Script_Credit*			TheCreditText( int index )
{
	return TheBuilderInfo().QueryInfo( info::eInfo_Credit_File )->getinfodataCREDIT( index );
}

////////////////////////////////////////////////////dialog file/////////////////////////////////////////////////////////////

inline
const info::Script_Dialog*			TheDialogText( int index )
{
	return g_BuilderINFO.QueryInfo( info::eInfo_Dialog_File )->getinfodataDIALOG( index );
}


#endif //INFO_BUILDER