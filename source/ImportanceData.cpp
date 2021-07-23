// ImportanceData.cpp: implementation of the ImportanceData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImportanceData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef IMPORTANCE_DATA

using namespace importance;

static ImportanceData importanceclient;

ImportanceData::ImportanceData()
{
	try
	{
		InitData();
    }
    catch( ... ) {
        MessageBox( NULL, "ImportanceData Error", "ImportanceData", MB_OK );
        throw;
    }
}

ImportanceData::~ImportanceData()
{

}

void ImportanceData::InitData()
{
	m_GateAttribute =  shared_array<GATE_ATTRIBUTE>( new GATE_ATTRIBUTE[MAX_GATES] );
	m_SkillAttribute =  shared_array<SKILL_ATTRIBUTE>( new SKILL_ATTRIBUTE[MAX_SKILLS] );
	m_ClientCharacter =  shared_array<CHARACTER>( new CHARACTER[MAX_CHARACTERS_CLIENT] );
	m_MachineCharacter = shared_ptr<CHARACTER_MACHINE>( new CHARACTER_MACHINE );

	//weak point 소유권이 없다.
	m_CharacterAttribute = m_MachineCharacter->Character;
}

#endif//IMPORTANCE_DATA