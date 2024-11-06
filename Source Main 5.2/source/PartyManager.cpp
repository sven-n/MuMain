// PartyManager.cpp: implementation of the CPartyManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PartyManager.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ZzzInventory.h"

using namespace SEASON3B;

CPartyManager::CPartyManager()
{
}

CPartyManager::~CPartyManager()
{
    Release();
}

bool CPartyManager::Create()
{
    return true;
}

void CPartyManager::Release()
{
}

bool CPartyManager::Update()
{
    return true;
}

bool CPartyManager::Render()
{
    return true;
}

CPartyManager* CPartyManager::GetInstance()
{
    static CPartyManager sPartyManager;
    return &sPartyManager;
}

void CPartyManager::SearchPartyMember()
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if (o->Type == MODEL_PLAYER && o->Kind == KIND_PLAYER && o->Live && o->Visible && o->Alpha > 0.f && c->Dead == 0)
        {
            for (int j = 0; j < PartyNumber; ++j)
            {
                PARTY_t* p = &Party[j];

                if (p->index != -2) continue;
                if (p->index > -1) continue;

                int length = max(wcslen(p->Name), max(1, wcslen(c->ID)));

                if (!wcsncmp(p->Name, c->ID, length))
                {
                    p->index = i;
                    break;
                }
            }
        }
    }

    for (int j = 0; j < PartyNumber; ++j)
    {
        PARTY_t* p = &Party[j];

        if (p->index >= 0) continue;

        int length = max(wcslen(p->Name), max(1, wcslen(Hero->ID)));

        if (!wcsncmp(p->Name, Hero->ID, length))
        {
            p->index = -3;
        }
        else
        {
            p->index = -1;
        }
    }
}

bool CPartyManager::IsPartyActive()
{
    int iMemberCount = 0;

    for (int i = 0; i < ((sizeof(Party) / sizeof(Party[0])) - 1); i++)
    {
        PARTY_t* pMember = &Party[i];
        if (pMember->Name[0] == L'\0')
        {
            continue;
        }

        CHARACTER* pChar = FindCharacterByID(pMember->Name);
        if (pChar != NULL)
        {
            iMemberCount++;
        }
    }

    return iMemberCount > 1;
}

bool CPartyManager::IsPartyMember(int index)
{
    CHARACTER* c = &CharactersClient[index];
    return IsPartyMemberChar(c);
}

bool CPartyManager::IsPartyMemberChar(CHARACTER* c)
{
    for (int i = 0; i < PartyNumber; ++i)
    {
        PARTY_t* p = &Party[i];

        int length = max(1, wcslen(c->ID));
        if (!wcsncmp(p->Name, c->ID, length)) return true;
    }

    return false;
}

CHARACTER* CPartyManager::GetPartyMemberChar(PARTY_t* pMember)
{
    if (pMember == nullptr || pMember->Name[0] == L'\0') {
        return NULL;
    }

    return FindCharacterByID(pMember->Name);
}