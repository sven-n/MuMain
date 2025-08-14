#include "stdafx.h"

#include "BoneManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzInterface.h"


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

            auto _lpBoneInfo = new BONEINFO;
            _lpBoneInfo->name = name;
            _lpBoneInfo->pCharacter = pCharacter;
            _lpBoneInfo->pModel = pModel;
            _lpBoneInfo->nBone = nBone;

            m_listBone.push_back(_lpBoneInfo);
        }
    }
}
void CBoneManager::UnregisterBone(CHARACTER* pCharacter, const std::wstring& name)
{
    if (pCharacter != NULL) {
        LPBONEINFO _lpBoneInfo = FindBone(&pCharacter->Object, name);
        if (_lpBoneInfo) {
            delete _lpBoneInfo;
            m_listBone.remove(_lpBoneInfo);
        }
    }
}
void CBoneManager::UnregisterBone(CHARACTER* pCharacter)
{
    auto iter = m_listBone.begin();
    for (; iter != m_listBone.end(); ) {
        LPBONEINFO _lpBoneInfo = (*iter);
        if (_lpBoneInfo->pCharacter == pCharacter) {
            delete _lpBoneInfo;
            iter = m_listBone.erase(iter);
        }
        else
            iter++;
    }
}
void CBoneManager::UnregisterAll()
{
    auto iter = m_listBone.begin();
    for (; iter != m_listBone.end(); iter++)
        delete (*iter);
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
    auto iter = m_listBone.begin();
    for (; iter != m_listBone.end(); iter++)
    {
        LPBONEINFO _lpBoneInfo = (*iter);
        if ((&_lpBoneInfo->pCharacter->Object == pObject) && (_lpBoneInfo->name == name))
            return _lpBoneInfo;
    }
    return NULL;
}