#ifndef _BONEMANAGER_H_
#define _BONEMANAGER_H_

#pragma once

#include <list>
#include <memory>
#include <string>

#include "ZzzInfomation.h"
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
    struct BONEINFO
    {
        std::wstring name; // bone name
        CHARACTER* pCharacter{ nullptr };
        BMD* pModel{ nullptr };
        int nBone{ -1 };
    };
    using LPBONEINFO = BONEINFO*;
    using t_bone_list = std::list<std::unique_ptr<BONEINFO>>;

    t_bone_list m_listBone;

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
    t_bone_list::iterator FindBoneIterator(OBJECT* pObject, const std::wstring& name);
    t_bone_list::const_iterator FindBoneIterator(OBJECT* pObject, const std::wstring& name) const;
};

#endif // _BONEMANAGER_H_