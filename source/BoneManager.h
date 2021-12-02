
#ifndef _BONEMANAGER_H_
#define _BONEMANAGER_H_

#pragma once

#include "zzzinfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

namespace BoneManager {
	void RegisterBone(CHARACTER* pCharacter, const std::string& name, int nBone);
	void UnregisterBone(CHARACTER* pCharacter, const std::string& name);
	void UnregisterBone(CHARACTER* pCharacter);
	void UnregisterAll();

	CHARACTER* GetOwnCharacter(OBJECT* pObject, const std::string& name);
	int GetBoneNumber(OBJECT* pObject, const std::string& name);
	bool GetBonePosition(OBJECT* pObject, const std::string& name, OUT vec3_t Position);
	bool GetBonePosition(OBJECT* pObject, const std::string& name, IN vec3_t Relative, OUT vec3_t Position);
}

class CBoneManager {
	typedef struct __BONEINFO
	{
		std::string	name;	// bone name

		CHARACTER*	pCharacter;
		BMD*		pModel;
		int			nBone;
	} BONEINFO, * LPBONEINFO;

	typedef std::list<LPBONEINFO> t_bone_list;
	t_bone_list	m_listBone;

public:
	~CBoneManager();

	void RegisterBone(CHARACTER* pCharacter, const std::string& name, int nBone);
	void UnregisterBone(CHARACTER* pCharacter, const std::string& name);
	void UnregisterBone(CHARACTER* pCharacter);
	void UnregisterAll();

	CHARACTER* GetOwnCharacter(OBJECT* pObject, const std::string& name);
	int GetBoneNumber(OBJECT* pObject, const std::string& name);
	bool GetBonePosition(OBJECT* pObject, const std::string& name, OUT vec3_t Position);
	bool GetBonePosition(OBJECT* pObject, const std::string& name, IN vec3_t Relative, OUT vec3_t Position);

	static CBoneManager* GetInstance();

protected:
	CBoneManager();		//. ban create instance

	LPBONEINFO FindBone(OBJECT* pObject, const std::string& name);
};

#endif // _BONEMANAGER_H_