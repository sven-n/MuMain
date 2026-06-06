// PartyManager.h: interface for the CPartyManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARTYMANAGER_H__9242DD27_52E9_4E31_AB79_4C761D233527__INCLUDED_)
#define AFX_PARTYMANAGER_H__9242DD27_52E9_4E31_AB79_4C761D233527__INCLUDED_

#pragma once

namespace SEASON3B
{
    class CPartyManager
    {
    protected:
        CPartyManager();

    public:
        virtual ~CPartyManager();

    private:
        static CPartyManager* m_pPartyManager;

    public:
        bool Create();
        void Release();
        bool Update();
        bool Render();

    public:
        void SearchPartyMember();
        bool IsPartyActive();
        bool IsPartyMember(int index);
        bool IsPartyMemberChar(CHARACTER* c);
        CHARACTER* GetPartyMemberChar(PARTY_t* pMember);

    public:
        static CPartyManager* GetInstance();
    };
}

#define g_pPartyManager SEASON3B::CPartyManager::GetInstance()

#endif // !defined(AFX_PARTYMANAGER_H__9242DD27_52E9_4E31_AB79_4C761D233527__INCLUDED_)
