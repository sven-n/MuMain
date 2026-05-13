#include "stdafx.h"

#include <algorithm>

#include "BoneManager.h"


void BoneManager::RegisterBone(CHARACTER* pCharacter, const std::wstring& name, int nBone)
{
    CBoneManager::GetInstance()->RegisterBone(pCharacter, name, nBone);
}
void BoneManager::UnregisterBone(CHARACTER* pCharacter, const std::wstring& name)
{
    CBoneManager::GetInstance()->UnregisterBone(pCharacter, name);
}
void BoneManager::UnregisterBone(CHARACTER* pCharacter)
{
    CBoneManager::GetInstance()->UnregisterBone(pCharacter);
}
void BoneManager::UnregisterAll()
{
    CBoneManager::GetInstance()->UnregisterAll();
}

CHARACTER* BoneManager::GetOwnCharacter(OBJECT* pObject, const std::wstring& name)
{
    return CBoneManager::GetInstance()->GetOwnCharacter(pObject, name);
}
int BoneManager::GetBoneNumber(OBJECT* pObject, const std::wstring& name)
{
    return CBoneManager::GetInstance()->GetBoneNumber(pObject, name);
}
bool BoneManager::GetBonePosition(OBJECT* pObject, const std::wstring& name, OUT vec3_t Position)
{
    return CBoneManager::GetInstance()->GetBonePosition(pObject, name, Position);
}
bool BoneManager::GetBonePosition(OBJECT* pObject, const std::wstring& name, IN vec3_t Relative, OUT vec3_t Position)
{
    return CBoneManager::GetInstance()->GetBonePosition(pObject, name, Relative, Position);
}

CBoneManager::CBoneManager()
{ }
CBoneManager::~CBoneManager()
{
    UnregisterAll();
}

void CBoneManager::RegisterBone(CHARACTER* pCharacter, const std::wstring& name, int nBone)
{
    if (pCharacter != NULL)
    {
        BMD* pModel = &Models[pCharacter->Object.Type];
        if (pModel != NULL) {
            if (FindBone(&pCharacter->Object, name) != NULL)
                UnregisterBone(pCharacter, name);

            auto boneInfo = std::make_unique<BONEINFO>();
            boneInfo->name = name;
            boneInfo->pCharacter = pCharacter;
            boneInfo->pModel = pModel;
            boneInfo->nBone = nBone;

            m_listBone.push_back(std::move(boneInfo));
        }
    }
}
void CBoneManager::UnregisterBone(CHARACTER* pCharacter, const std::wstring& name)
{
    if (pCharacter != NULL) {
        auto iter = FindBoneIterator(&pCharacter->Object, name);
        if (iter != m_listBone.end())
        {
            m_listBone.erase(iter);
        }
    }
}
void CBoneManager::UnregisterBone(CHARACTER* pCharacter)
{
    m_listBone.remove_if(
        [pCharacter](const std::unique_ptr<BONEINFO>& boneInfo)
        {
            return boneInfo && boneInfo->pCharacter == pCharacter;
        });
}
void CBoneManager::UnregisterAll()
{
    m_listBone.clear();
}

CHARACTER* CBoneManager::GetOwnCharacter(OBJECT* pObject, const std::wstring& name)
{
    LPBONEINFO _lpBoneInfo = FindBone(pObject, name);
    if (_lpBoneInfo)
        return _lpBoneInfo->pCharacter;
    return NULL;
}
int CBoneManager::GetBoneNumber(OBJECT* pObject, const std::wstring& name)
{
    LPBONEINFO _lpBoneInfo = FindBone(pObject, name);
    if (_lpBoneInfo)
        return _lpBoneInfo->nBone;
    return -1;
}
bool CBoneManager::GetBonePosition(OBJECT* pObject, const std::wstring& name, OUT vec3_t Position)
{
    LPBONEINFO _lpBoneInfo = FindBone(pObject, name);
    if (_lpBoneInfo) {
        vec3_t LocalPos, Relative;
        Vector(0.f, 0.f, 0.f, Relative);
        _lpBoneInfo->pModel->TransformPosition(pObject->BoneTransform[_lpBoneInfo->nBone], Relative, LocalPos, false);
        VectorScale(LocalPos, pObject->Scale, LocalPos);
        VectorAdd(LocalPos, pObject->Position, Position);
        return true;
    }
    return false;
}
bool CBoneManager::GetBonePosition(OBJECT* pObject, const std::wstring& name, IN vec3_t Relative, OUT vec3_t Position)
{
    LPBONEINFO _lpBoneInfo = FindBone(pObject, name);
    if (_lpBoneInfo) {
        vec3_t LocalPos;
        _lpBoneInfo->pModel->TransformPosition(pObject->BoneTransform[_lpBoneInfo->nBone], Relative, LocalPos, false);
        VectorScale(LocalPos, pObject->Scale, LocalPos);
        VectorAdd(LocalPos, pObject->Position, Position);
        return true;
    }
    return false;
}

CBoneManager* CBoneManager::GetInstance()
{
    static CBoneManager s_Instace;
    return &s_Instace;
}

CBoneManager::LPBONEINFO CBoneManager::FindBone(OBJECT* pObject, const std::wstring& name)
{
    auto iter = FindBoneIterator(pObject, name);
    if (iter == m_listBone.end())
    {
        return nullptr;
    }

    return iter->get();
}

CBoneManager::t_bone_list::iterator CBoneManager::FindBoneIterator(OBJECT* pObject, const std::wstring& name)
{
    return std::find_if(
        m_listBone.begin(),
        m_listBone.end(),
        [pObject, &name](const std::unique_ptr<BONEINFO>& boneInfo)
        {
            return (&boneInfo->pCharacter->Object == pObject) &&
                (boneInfo->name == name);
        });
}

CBoneManager::t_bone_list::const_iterator CBoneManager::FindBoneIterator(OBJECT* pObject, const std::wstring& name) const
{
    return std::find_if(
        m_listBone.cbegin(),
        m_listBone.cend(),
        [pObject, &name](const std::unique_ptr<BONEINFO>& boneInfo)
        {
            return (&boneInfo->pCharacter->Object == pObject) &&
                (boneInfo->name == name);
        });
}