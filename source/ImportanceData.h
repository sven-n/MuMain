// ImportanceData.h: interface for the ImportanceData class.
//
//////////////////////////////////////////////////////////////////////

#ifdef IMPORTANCE_DATA

#if !defined(AFX_IMPORTANCEDATA_H__EBA9B625_B5A1_4D9A_93E6_BA077D4FE197__INCLUDED_)
#define AFX_IMPORTANCEDATA_H__EBA9B625_B5A1_4D9A_93E6_BA077D4FE197__INCLUDED_

#include "Singleton.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace importance
{
	class ImportanceData : public Singleton<ImportanceData>  
	{

	private:
		void InitData();

	public:
		ImportanceData();
		virtual ~ImportanceData();

	public:
		GATE_ATTRIBUTE&						GetGateAttribute( int index );
		SKILL_ATTRIBUTE&					GetSkillAttribute( int index );
		CHARACTER&							GetCharacterClient( int index );	  
		weak_ptr<CHARACTER_MACHINE>			GetCharacterMachine();
		weak_ptr<CHARACTER_ATTRIBUTE>		GetCharaterAttribute();

	private:
		//array
		shared_array<GATE_ATTRIBUTE>	  m_GateAttribute;
		shared_array<SKILL_ATTRIBUTE>     m_SkillAttribute;
		shared_array<CHARACTER>			  m_ClientCharacter;

		//shared point
		shared_ptr<CHARACTER_MACHINE>	  m_MachineCharacter;

		//weak point
		weak_ptr<CHARACTER_ATTRIBUTE>	  m_CharacterAttribute;
	};
};

inline
GATE_ATTRIBUTE&						importance::ImportanceData::GetGateAttribute( int index )
{
	assert(m_GateAttribute);
	return m_GateAttribute[index];
}

inline
SKILL_ATTRIBUTE&					importance::ImportanceData::GetSkillAttribute( int index )
{
	assert(m_SkillAttribute);
	return m_SkillAttribute[index];
}

inline
CHARACTER&							importance::ImportanceData::GetCharacterClient( int index )
{
	assert(m_ClientCharacter);
	return m_ClientCharacter[index];

}

inline
weak_ptr<CHARACTER_MACHINE>			importance::ImportanceData::GetCharacterMachine()
{
	assert(m_MachineCharacter);
	return m_MachineCharacter;
}

inline
weak_ptr<CHARACTER_ATTRIBUTE>		importance::ImportanceData::GetCharaterAttribute()
{
	assert(m_CharacterAttribute.expired());
	return m_CharacterAttribute;
}

#define g_ImportanceData importance::ImportanceData::GetSingleton ()

#endif // !defined(AFX_IMPORTANCEDATA_H__EBA9B625_B5A1_4D9A_93E6_BA077D4FE197__INCLUDED_)

#endif //IMPORTANCE_DATA