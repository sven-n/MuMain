// w_PetProcess.h: interface for the PetProcess class.
// LDK_2008/07/08
//////////////////////////////////////////////////////////////////////

#ifdef LDK_ADD_NEW_PETPROCESS

#if !defined(AFX_W_PETPROCESS_H__95A7D82D_B7F0_405F_A95B_FD4CBB548E5D__INCLUDED_)
#define AFX_W_PETPROCESS_H__95A7D82D_B7F0_405F_A95B_FD4CBB548E5D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_BasePet.h"
#include "w_PetAction.h"

#define PC4_ELF			1
#define PC4_TEST		2
#define PC4_SATAN		3
#define XMAS_RUDOLPH	4
#ifdef PJH_ADD_PANDA_PET
#define PANDA			5
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
#define	UNICORN			6
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
#define SKELETON		7
#endif	// YDG_ADD_SKELETON_PET

BoostSmartPointer(PetInfo);
class PetInfo
{
public:
	static PetInfoPtr Make();
	virtual ~PetInfo();

private:
	PetInfo();
	void Destroy();

public:
	float GetScale() { return m_scale; }
	void SetScale( float scale = 0.0f ) { m_scale = scale; }

	int GetBlendMesh() { return m_blendMesh; }
	void SetBlendMesh( int blendMesh = -1 ) { m_blendMesh = blendMesh; }

	float* GetSpeeds() { return m_speeds; }
	int* GetActions() { return m_actions; }
	void SetActions(int count, int *actions, float *speeds);

	int GetActionsCount() { return m_count; }

private:
	//create
	int m_blendMesh;
	float m_scale;
	
	//action
	int *m_actions;
	float *m_speeds;
	
	//ex
	int m_count;
};

//------------------------------------------------------------//

BoostSmartPointer(PetProcess);
class PetProcess  
{
public:
	typedef list< BoostSmart_Ptr(PetObject) > PetList;
	typedef map< int, BoostSmart_Ptr(PetAction) > ActionMap;	//actionNum, actionClass
	typedef map< int, BoostSmart_Ptr(PetInfo) > InfoMap;		//PetType, PetInfo

public:
	static PetProcessPtr Make();
	virtual ~PetProcess();
	
private:
#ifdef LDK_MOD_NUMBERING_PETCREATE
	int Register( BoostSmart_Ptr( PetObject ) pPet, int petNum );
	bool UnRegister( CHARACTER *Owner, int petRegNum = -1 );
#else //LDK_MOD_NUMBERING_PETCREATE
	void Register( BoostSmart_Ptr( PetObject ) pPet );
	void UnRegister( CHARACTER *Owner, int itemType, bool isUnregistAll = false );
#endif //LDK_MOD_NUMBERING_PETCREATE

	BoostWeak_Ptr(PetAction) Find( int key );
#ifdef LDK_MOD_NUMBERING_PETCREATE
	BoostWeak_Ptr(PetObject) FindList( int key, PetList::iterator out_iter = NULL );
#endif //LDK_MOD_NUMBERING_PETCREATE
	void Init();
	void Destroy();
	PetProcess();

public:
	bool LoadData();

	bool IsPet( int itemType );
#ifdef LDK_MOD_NUMBERING_PETCREATE
	int CreatePet( int itemType, int modelType, vec3_t Position, CHARACTER *Owner, int SubType=0, int LinkBone=0 );
	bool DeletePet( CHARACTER *Owner, int petRegNum = -1, bool ex = false ); //petRegNum -1이면 같은주인의 모든펫 삭제 3번째 파라메터는 예비용
#else //LDK_MOD_NUMBERING_PETCREATE
	bool CreatePet( int itemType, int modelType, vec3_t Position, CHARACTER *Owner, int SubType=0, int LinkBone=0 );
	void DeletePet( CHARACTER *Owner, int itemType = -1, bool allDelete = false );
#endif //LDK_MOD_NUMBERING_PETCREATE
	void UpdatePets();
	void RenderPets();

#ifdef LDK_MOD_NUMBERING_PETCREATE
	bool SetCommandPet( CHARACTER *Owner, int petRegNum, int targetKey, PetObject::ActionType cmdType ); //petRegNum -1이면 같은주인의 모든펫 행동 변경
#else //LDK_MOD_NUMBERING_PETCREATE
	void SetCommandPet( CHARACTER *Owner, int targetKey, PetObject::ActionType cmdType );
#endif //LDK_MOD_NUMBERING_PETCREATE

private:
	PetList m_petsList;
	ActionMap m_petsAction;
	InfoMap m_petsInfo;
};

extern PetProcessPtr g_petProcess;
extern PetProcess& ThePetProcess();

#endif // !defined(AFX_W_PETPROCESS_H__95A7D82D_B7F0_405F_A95B_FD4CBB548E5D__INCLUDED_)
#endif //LDK_ADD_NEW_PETPROCESS