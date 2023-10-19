#ifndef _BONEMANAGER_H_
#define _BONEMANAGER_H_

#pragma once

#include "zzzinfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

namespace BoneManager {
    void RegisterBone(CHARACTER* pCharacter, const std::wstring& name, int nBone);
    void UnregisterBone(CHARACTER* pCharacter, const std::wstring& name);
    void UnregisterBone(CHARACTER* pCharacter);
    void UnregisterAll();

    CHARACTER* GetOwnCharacter(OBJECT* pObject, const std::wstring& name);
    int GetBoneNumber(OBJECT* pObject, const std::wstring& name);
    bool GetBonePosition(OBJECT* pObject, const std::wstring& name, OUT vec3_t Position);
    bool GetBonePosition(OBJECT* pObject, const std::wstring& name, IN vec3_t Relative, OUT vec3_t Position);
}

class CBoneManager {
    typedef struct __BONEINFO
    {
        std::wstring	name;	// bone name

        CHARACTER* pCharacter;
        BMD* pModel;
        int			nBone;
    } BONEINFO, * LPBONEINFO;

    typedef std::list<LPBONEINFO> t_bone_list;
    t_bone_list	m_listBone;

public:
    ~CBoneManager();

    void RegisterBone(CHARACTER* pCharacter, const std::wstring& name, int nBone);
    void UnregisterBone(CHARACTER* pCharacter, const std::wstring& name);
    void UnregisterBone(CHARACTER* pCharacter);
    void UnregisterAll();

    CHARACTER* GetOwnCharacter(OBJECT* pObject, const std::wstring& name);
    int GetBoneNumber(OBJECT* pObject, const std::wstring& name);
    bool GetBonePosition(OBJECT* pObject, const std::wstring& name, OUT vec3_t Position);
    bool GetBonePosition(OBJECT* pObject, const std::wstring& name, IN vec3_t Relative, OUT vec3_t Position);

    static CBoneManager* GetInstance();

protected:
    CBoneManager();		//. ban create instance

    LPBONEINFO FindBone(OBJECT* pObject, const std::wstring& name);
};

#endif // _BONEMANAGER_H_