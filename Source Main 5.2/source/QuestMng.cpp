//*****************************************************************************
// File: CQuestMng.cpp
//*****************************************************************************

#include "stdafx.h"
#include "QuestMng.h"



#include <crtdbg.h>

#include "NewUISystem.h"
#include "UsefulDef.h"

#define	QM_NPCDIALOGUE_FILE			L"Data\\Local\\NPCDialogue.bmd"
#define	QM_QUESTPROGRESS_FILE		L"Data\\Local\\QuestProgress.bmd"

#define	QM_QUESTWORDS_FILE			std::wstring(L"Data\\Local\\"+g_strSelectedML+L"\\QuestWords_"+g_strSelectedML+L".bmd").c_str()

CQuestMng g_QuestMng;

CQuestMng::CQuestMng()
{
    m_nNPCIndex = 0;
    m_szNPCName = nullptr;
}

CQuestMng::~CQuestMng()
{
}

void CQuestMng::LoadQuestScript()
{
    LoadNPCDialogueScript();
    LoadQuestProgressScript();
    LoadQuestWordsScript();
}

void CQuestMng::LoadNPCDialogueScript()
{
    FILE* fp = ::_wfopen(QM_NPCDIALOGUE_FILE, L"rb");
    if (fp == NULL)
    {
        wchar_t szMessage[256];
        swprintf(szMessage, L"%s file not found.\r\n", QM_NPCDIALOGUE_FILE);
        g_ErrorReport.Write(szMessage);
        ::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
        ::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    const int nSize = sizeof(DWORD) + sizeof(SNPCDialogue);
    BYTE abyBuffer[nSize];
    DWORD dwIndex = 0;
    SNPCDialogue sNPCDialogue;

    while (0 != ::fread(abyBuffer, nSize, 1, fp))
    {
        ::BuxConvert(abyBuffer, nSize);

        ::memcpy(&dwIndex, abyBuffer, sizeof(DWORD));
        ::memcpy(&sNPCDialogue, abyBuffer + sizeof(DWORD), sizeof(SNPCDialogue));

        m_mapNPCDialogue.insert(std::make_pair(dwIndex, sNPCDialogue));
    }

    ::fclose(fp);
}

void CQuestMng::LoadQuestProgressScript()
{
    FILE* fp = ::_wfopen(QM_QUESTPROGRESS_FILE, L"rb");
    if (fp == NULL)
    {
        wchar_t szMessage[256];
        ::swprintf(szMessage, L"%s file not found.\r\n", QM_QUESTPROGRESS_FILE);
        g_ErrorReport.Write(szMessage);
        ::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
        ::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    const int nSize = sizeof(DWORD) + sizeof(SQuestProgress);
    BYTE abyBuffer[nSize];
    DWORD dwIndex = 0;
    SQuestProgress sQuestProgress;

    while (0 != ::fread(abyBuffer, nSize, 1, fp))
    {
        ::BuxConvert(abyBuffer, nSize);

        ::memcpy(&dwIndex, abyBuffer, sizeof(DWORD));
        ::memcpy(&sQuestProgress, abyBuffer + sizeof(DWORD), sizeof(SQuestProgress));

        m_mapQuestProgress.insert(std::make_pair(dwIndex, sQuestProgress));
    }

    ::fclose(fp);
}

void CQuestMng::LoadQuestWordsScript()
{
    FILE* fp = ::_wfopen(QM_QUESTWORDS_FILE, L"rb");
    if (fp == NULL)
    {
        wchar_t szMessage[256];
        ::swprintf(szMessage, L"%s file not found.\r\n", QM_QUESTWORDS_FILE);
        g_ErrorReport.Write(szMessage);
        ::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
        ::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

#pragma pack(push, 1)
    struct SQuestWordsHeader
    {
        int		m_nIndex;
        short	m_nWordsLen;
    };
#pragma pack(pop)

    int nSize = sizeof(SQuestWordsHeader);
    SQuestWordsHeader sQuestWordsHeader;
    char rawWords[1024] { };
    wchar_t szWords[1024] { };

    while (0 != ::fread(&sQuestWordsHeader, nSize, 1, fp))
    {
        ::BuxConvert((BYTE*)&sQuestWordsHeader, nSize);

        ::fread(rawWords, sQuestWordsHeader.m_nWordsLen, 1, fp);
        ::BuxConvert((BYTE*)rawWords, sQuestWordsHeader.m_nWordsLen);
        CMultiLanguage::ConvertFromUtf8(szWords, rawWords, 1024);

        std::wstring strWords = szWords;
        m_mapQuestWords.insert(std::make_pair(sQuestWordsHeader.m_nIndex, strWords));
    }

    ::fclose(fp);
}

void CQuestMng::SetQuestRequestReward(const BYTE* pbyRequestRewardPacket)
{
    auto pRequestRewardPacket
        = (LPPMSG_NPC_QUESTEXP_INFO)pbyRequestRewardPacket;
    DWORD dwQuestIndex = pRequestRewardPacket->m_dwQuestIndex;
    int i;

    const SQuestRequestReward* pOldRequestReward = GetRequestReward(dwQuestIndex);
    if (pOldRequestReward)
    {
        for (i = 0; i < pOldRequestReward->m_byRequestCount; ++i)
            g_pNewItemMng->DeleteItem(pOldRequestReward->m_aRequest[i].m_pItem);
        BYTE byRewardCount
            = pOldRequestReward->m_byGeneralRewardCount + pOldRequestReward->m_byRandRewardCount;
        for (i = 0; i < byRewardCount; ++i)
            g_pNewItemMng->DeleteItem(pOldRequestReward->m_aReward[i].m_pItem);
    }

    SQuestRequestReward sRequestReward;
    ::memset(&sRequestReward, 0, sizeof(SQuestRequestReward));

    auto pRequestPacket
        = (LPNPC_QUESTEXP_REQUEST_INFO)(pbyRequestRewardPacket + sizeof(PMSG_NPC_QUESTEXP_INFO));

    if (pRequestPacket->m_dwType == QUEST_REQUEST_NONE || pRequestRewardPacket->m_byRequestCount == 0)
    {
        sRequestReward.m_byRequestCount = 1;
    }
    else
    {
        sRequestReward.m_byRequestCount = pRequestRewardPacket->m_byRequestCount;
        for (i = 0; i < sRequestReward.m_byRequestCount; ++i)
        {
            sRequestReward.m_aRequest[i].m_dwType = pRequestPacket->m_dwType;
            sRequestReward.m_aRequest[i].m_wIndex = pRequestPacket->m_wIndex;
            sRequestReward.m_aRequest[i].m_dwValue = pRequestPacket->m_dwValue;
#ifdef ASG_ADD_TIME_LIMIT_QUEST
            sRequestReward.m_aRequest[i].m_dwCurValue = pRequestPacket->m_dwCurValue;
#else	// ASG_ADD_TIME_LIMIT_QUEST
            sRequestReward.m_aRequest[i].m_wCurValue = pRequestPacket->m_wCurValue;
#endif	// ASG_ADD_TIME_LIMIT_QUEST
            if (pRequestPacket->m_dwType == QUEST_REQUEST_ITEM)
                sRequestReward.m_aRequest[i].m_pItem
                = g_pNewItemMng->CreateItemOld(pRequestPacket->m_byItemInfo);
            ++pRequestPacket;
        }
    }

    auto pRewardPacket = (LPNPC_QUESTEXP_REWARD_INFO)(pbyRequestRewardPacket + sizeof(PMSG_NPC_QUESTEXP_INFO) + sizeof(NPC_QUESTEXP_REQUEST_INFO) * 5);

    if (pRewardPacket->m_dwType == QUEST_REWARD_NONE || pRequestRewardPacket->m_byRewardCount == 0)
    {
        sRequestReward.m_byGeneralRewardCount = 1;
    }
    else
    {
        sRequestReward.m_byRandGiveCount = pRequestRewardPacket->m_byRandRewardCount;

        BYTE byGeneralCount = 0;
        BYTE byRandCount = 0;
        SQuestReward aTempRandReward[5];
        ::memset(aTempRandReward, 0, sizeof(SQuestReward) * 5);

        for (i = 0; i < pRequestRewardPacket->m_byRewardCount; ++i)
        {
            if (QUEST_REWARD_TYPE(pRewardPacket->m_dwType & 0xFFE0) == QUEST_REWARD_RANDOM)
            {
                aTempRandReward[byRandCount].m_dwType = pRewardPacket->m_dwType & 0x1F;
                aTempRandReward[byRandCount].m_wIndex = pRewardPacket->m_wIndex;
                aTempRandReward[byRandCount].m_dwValue = pRewardPacket->m_dwValue;
                if (aTempRandReward[byRandCount].m_dwType == QUEST_REWARD_ITEM)
                    aTempRandReward[byRandCount].m_pItem
                    = g_pNewItemMng->CreateItemOld(pRewardPacket->m_byItemInfo);
                ++byRandCount;
            }
            else
            {
                sRequestReward.m_aReward[byGeneralCount].m_dwType = pRewardPacket->m_dwType;
                sRequestReward.m_aReward[byGeneralCount].m_wIndex = pRewardPacket->m_wIndex;
                sRequestReward.m_aReward[byGeneralCount].m_dwValue = pRewardPacket->m_dwValue;
                if (pRewardPacket->m_dwType == QUEST_REWARD_ITEM)
                    sRequestReward.m_aReward[byGeneralCount].m_pItem = g_pNewItemMng->CreateItemOld(pRewardPacket->m_byItemInfo);
                ++byGeneralCount;
            }

            ++pRewardPacket;
        }

        sRequestReward.m_byGeneralRewardCount = byGeneralCount;
        sRequestReward.m_byRandRewardCount = byRandCount;

        for (i = 0; i < sRequestReward.m_byRandRewardCount; ++i)
            sRequestReward.m_aReward[byGeneralCount++] = aTempRandReward[i];
    }

    m_mapQuestRequestReward[dwQuestIndex] = sRequestReward;
}

const SQuestRequestReward* CQuestMng::GetRequestReward(DWORD dwQuestIndex)
{
    QuestRequestRewardMap::const_iterator iter = m_mapQuestRequestReward.find(dwQuestIndex);
    if (iter == m_mapQuestRequestReward.end())
        return NULL;

    return &(iter->second);
}

void CQuestMng::SetNPC(int nNPCIndex)
{
    m_nNPCIndex = nNPCIndex;
    m_szNPCName = getMonsterName(nNPCIndex);
}

int CQuestMng::GetNPCIndex()
{
    return m_nNPCIndex;
}

wchar_t* CQuestMng::GetNPCName()
{
    return m_szNPCName;
}

void CQuestMng::SetCurQuestProgress(DWORD dwQuestIndex)
{
    AddCurQuestIndexList(dwQuestIndex);

    if (LOWORD(dwQuestIndex) == 0x00FF)
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS);
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);

        g_pSystemLogBox->AddText(GlobalText[2814], SEASON3B::TYPE_ERROR_MESSAGE);

        return;
    }

    if (0 == m_mapQuestProgress[dwQuestIndex].m_byUIType)
    {
        g_pQuestProgress->SetContents(dwQuestIndex);
        if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
            g_pNewUISystem->Show(SEASON3B::INTERFACE_QUEST_PROGRESS);
    }
    else
    {
        g_pQuestProgressByEtc->SetContents(dwQuestIndex);
        if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
            g_pNewUISystem->Show(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
    }
}

const wchar_t* CQuestMng::GetWords(int nWordsIndex)
{
    QuestWordsMap::const_iterator iter = m_mapQuestWords.find(nWordsIndex);
    if (iter == m_mapQuestWords.end())
        return NULL;

    return iter->second.c_str();
}

const wchar_t* CQuestMng::GetNPCDlgNPCWords(DWORD dwDlgState)
{
    if (0 == m_nNPCIndex)
        return NULL;

    DWORD dwNPCDlgIndex = (DWORD)m_nNPCIndex * 0x10000 + dwDlgState;

    NPCDialogueMap::const_iterator iter = m_mapNPCDialogue.find(dwNPCDlgIndex);
    if (iter == m_mapNPCDialogue.end())
        return NULL;

    return GetWords(iter->second.m_nNPCWords);
}

const wchar_t* CQuestMng::GetNPCDlgAnswer(DWORD dwDlgState, int nAnswer)
{
    if (0 == m_nNPCIndex)
        return NULL;

    _ASSERT(0 <= nAnswer || nAnswer < QM_MAX_ND_ANSWER);

    DWORD dwNPCDlgIndex = (DWORD)m_nNPCIndex * 0x10000 + dwDlgState;

    NPCDialogueMap::const_iterator iter = m_mapNPCDialogue.find(dwNPCDlgIndex);
    if (iter == m_mapNPCDialogue.end())
        return NULL;

    DWORD nNowAnswer = iter->second.m_anAnswer[nAnswer * 2];
    if (0 == nNowAnswer)
        return NULL;

    return GetWords(nNowAnswer);
}

int CQuestMng::GetNPCDlgAnswerResult(DWORD dwDlgState, int nAnswer)
{
    if (0 == m_nNPCIndex)
        return NULL;

    _ASSERT(0 <= nAnswer || nAnswer < QM_MAX_ND_ANSWER);

    DWORD dwNPCDlgIndex = (DWORD)m_nNPCIndex * 0x10000 + dwDlgState;

    NPCDialogueMap::const_iterator iter = m_mapNPCDialogue.find(dwNPCDlgIndex);
    if (iter == m_mapNPCDialogue.end())
        return -1;

    DWORD nNowAnswer = iter->second.m_anAnswer[nAnswer * 2];
    if (0 == nNowAnswer)
        return -1;

    return iter->second.m_anAnswer[nAnswer * 2 + 1];
}

const wchar_t* CQuestMng::GetNPCWords(DWORD dwQuestIndex)
{
    QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
    if (iter == m_mapQuestProgress.end())
        return NULL;

    return GetWords(iter->second.m_nNPCWords);
}

const wchar_t* CQuestMng::GetPlayerWords(DWORD dwQuestIndex)
{
    QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
    if (iter == m_mapQuestProgress.end())
        return NULL;

    return GetWords(iter->second.m_nPlayerWords);
}

const wchar_t* CQuestMng::GetAnswer(DWORD dwQuestIndex, int nAnswer)
{
    _ASSERT(0 <= nAnswer || nAnswer < QM_MAX_ANSWER);

    QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
    if (iter == m_mapQuestProgress.end())
        return NULL;

    DWORD nNowAnswer = iter->second.m_anAnswer[nAnswer];
    if (0 == nNowAnswer)
        return NULL;

    return GetWords(nNowAnswer);
}

const wchar_t* CQuestMng::GetSubject(DWORD dwQuestIndex)
{
    QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
    if (iter == m_mapQuestProgress.end())
        return NULL;

    return GetWords(iter->second.m_nSubject);
}

const wchar_t* CQuestMng::GetSummary(DWORD dwQuestIndex)
{
    QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
    if (iter == m_mapQuestProgress.end())
        return NULL;

    return GetWords(iter->second.m_nSummary);
}

bool CQuestMng::IsRequestRewardQS(DWORD dwQuestIndex)
{
    QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
    _ASSERT(iter != m_mapQuestProgress.end());

    if (0 == iter->second.m_anAnswer[0])
        return true;
    else
        return false;
}

bool CQuestMng::GetRequestRewardText(SRequestRewardText* aDest, int nDestCount, DWORD dwQuestIndex)
{
    SQuestRequestReward* pRequestReward = &m_mapQuestRequestReward[dwQuestIndex];

    if (pRequestReward->m_byRequestCount + pRequestReward->m_byGeneralRewardCount + pRequestReward->m_byRandRewardCount + 3 > nDestCount)
        return false;

    ::memset(aDest, 0, sizeof(SRequestRewardText) * nDestCount);

    int nLine = 0;
    bool bRequestComplete = true;
    int i;

    aDest[nLine].m_hFont = g_hFontBold;
    aDest[nLine].m_dwColor = ARGB(255, 179, 230, 77);
    wcscpy(aDest[nLine++].m_szText, GlobalText[2809]);

    SQuestRequest* pRequestInfo;
    for (i = 0; i < pRequestReward->m_byRequestCount; ++i, ++nLine)
    {
        pRequestInfo = &pRequestReward->m_aRequest[i];

        aDest[nLine].m_hFont = g_hFont;
        aDest[nLine].m_eRequestReward = RRC_REQUEST;
        aDest[nLine].m_dwType = pRequestInfo->m_dwType;
        aDest[nLine].m_wIndex = pRequestInfo->m_wIndex;
        aDest[nLine].m_pItem = pRequestInfo->m_pItem;

        switch (pRequestInfo->m_dwType)
        {
        case QUEST_REQUEST_NONE:
            aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);
            wcscpy(aDest[nLine].m_szText, GlobalText[1361]);
            break;

#ifdef ASG_ADD_TIME_LIMIT_QUEST
        case QUEST_REQUEST_MONSTER:
        case QUEST_REQUEST_ITEM:
        case QUEST_REQUEST_LEVEL:
        case QUEST_REQUEST_ZEN:
        case QUEST_REQUEST_PVP_POINT:
            if (pRequestInfo->m_dwCurValue < pRequestInfo->m_dwValue)
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }
            else
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

            switch (pRequestInfo->m_dwType)
            {
            case QUEST_REQUEST_MONSTER:
                ::swprintf(aDest[nLine].m_szText, L"Mon.: %s x %lu/%lu",
                    ::getMonsterName(int(pRequestInfo->m_wIndex)),
                    MIN(pRequestInfo->m_dwCurValue, pRequestInfo->m_dwValue),
                    pRequestInfo->m_dwValue);
                break;
            case QUEST_REQUEST_ITEM:
            {
                wchar_t szItemName[32];
                ::GetItemName((int)pRequestInfo->m_pItem->Type,
                    (pRequestInfo->m_pItem->Level, szItemName);
                ::swprintf(aDest[nLine].m_szText, L"Item: %s x %lu/%lu", szItemName,
                    MIN(pRequestInfo->m_dwCurValue, pRequestInfo->m_dwValue),
                    pRequestInfo->m_dwValue);
            }
            break;
            case QUEST_REQUEST_LEVEL:
                ::swprintf(aDest[nLine].m_szText, L"Level: %lu %s",
                    pRequestInfo->m_dwValue, GlobalText[2812]);
                break;
            case QUEST_REQUEST_ZEN:
                ::swprintf(aDest[nLine].m_szText, L"Zen : %lu", pRequestInfo->m_dwValue);
                break;
            case QUEST_REQUEST_PVP_POINT:
                swprintf(aDest[nLine].m_szText, GlobalText[3278],
                    MIN(pRequestInfo->m_dwCurValue, pRequestInfo->m_dwValue),
                    pRequestInfo->m_dwValue);
                break;
            }
            break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST

#ifndef ASG_ADD_TIME_LIMIT_QUEST
        case QUEST_REQUEST_MONSTER:
            if ((DWORD)pRequestInfo->m_wCurValue < pRequestInfo->m_dwValue)
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }
            else
            {
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);
            }

            {
                auto text = getMonsterName(int(pRequestInfo->m_wIndex));
                swprintf(aDest[nLine].m_szText, L"Mon.: %s x %lu/%lu",
                    text,
                    MIN((DWORD)pRequestInfo->m_wCurValue, pRequestInfo->m_dwValue),
                    pRequestInfo->m_dwValue);
            }
            break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST

        case QUEST_REQUEST_SKILL:
#ifdef ASG_ADD_TIME_LIMIT_QUEST
            if (0 == pRequestInfo->m_dwCurValue)
#else	// ASG_ADD_TIME_LIMIT_QUEST
            if (0 == pRequestInfo->m_wCurValue)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }
            else
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

            ::swprintf(aDest[nLine].m_szText, L"Skill: %s",
                SkillAttribute[pRequestInfo->m_wIndex].Name);
            break;

#ifndef ASG_ADD_TIME_LIMIT_QUEST
        case QUEST_REQUEST_ITEM:
            if ((DWORD)pRequestInfo->m_wCurValue < pRequestInfo->m_dwValue)
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }
            else
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

            wchar_t szItemName[32];
            ::GetItemName((int)pRequestInfo->m_pItem->Type, pRequestInfo->m_pItem->Level,
                szItemName);
            ::swprintf(aDest[nLine].m_szText, L"Item: %s x %lu/%lu", szItemName,
                MIN((DWORD)pRequestInfo->m_wCurValue, pRequestInfo->m_dwValue),
                pRequestInfo->m_dwValue);
            break;

        case QUEST_REQUEST_LEVEL:
            if ((DWORD)pRequestInfo->m_wCurValue < pRequestInfo->m_dwValue)
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }
            else
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

            ::swprintf(aDest[nLine].m_szText, L"Level: %lu %s",
                pRequestInfo->m_dwValue, GlobalText[2812]);
            break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST

        case QUEST_REQUEST_TUTORIAL:
#ifdef ASG_ADD_TIME_LIMIT_QUEST
            if (pRequestInfo->m_dwCurValue == 1)
#else	// ASG_ADD_TIME_LIMIT_QUEST
            if (pRequestInfo->m_wCurValue == 1)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);
            else
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }

            switch (dwQuestIndex)
            {
            case 0x10009:
                ::swprintf(aDest[nLine].m_szText, L"%s", GlobalText[2819]);
                break;
            case 0x1000F:
                ::swprintf(aDest[nLine].m_szText, L"%s", GlobalText[2820]);
                break;
            }
            break;

        case QUEST_REQUEST_BUFF:
        {
#ifdef ASG_ADD_TIME_LIMIT_QUEST
            if (pRequestInfo->m_dwCurValue == 0)
#else	// ASG_ADD_TIME_LIMIT_QUEST
            if (pRequestInfo->m_wCurValue == 0)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }
            else
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

            const BuffInfo buffinfo = g_BuffInfo((eBuffState)pRequestInfo->m_wIndex);
            ::swprintf(aDest[nLine].m_szText, L"Bonus: %s", buffinfo.s_BuffName);
        }
        break;

        case QUEST_REQUEST_EVENT_MAP_MON_KILL:
        case QUEST_REQUEST_EVENT_MAP_BLOOD_GATE:
        case QUEST_REQUEST_EVENT_MAP_USER_KILL:
        case QUEST_REQUEST_EVENT_MAP_DEVIL_POINT:
        {
#ifdef ASG_ADD_TIME_LIMIT_QUEST
            if (pRequestInfo->m_dwCurValue < pRequestInfo->m_dwValue)
#else	// ASG_ADD_TIME_LIMIT_QUEST
            if ((DWORD)pRequestInfo->m_wCurValue < pRequestInfo->m_dwValue)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }
            else
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

            int nTextIndex = 0;
            switch (pRequestInfo->m_dwType)
            {
            case QUEST_REQUEST_EVENT_MAP_MON_KILL:
                nTextIndex = 3074;
                break;
            case QUEST_REQUEST_EVENT_MAP_BLOOD_GATE:
                nTextIndex = 3077;
                break;
            case QUEST_REQUEST_EVENT_MAP_USER_KILL:
                nTextIndex = 3075;
                break;
            case QUEST_REQUEST_EVENT_MAP_DEVIL_POINT:
                nTextIndex = 3079;
                break;
            }
            swprintf(aDest[nLine].m_szText, GlobalText[nTextIndex], pRequestInfo->m_wIndex,
#ifdef ASG_ADD_TIME_LIMIT_QUEST
                MIN(pRequestInfo->m_dwCurValue, pRequestInfo->m_dwValue),
#else	// ASG_ADD_TIME_LIMIT_QUEST
                MIN((DWORD)pRequestInfo->m_wCurValue, pRequestInfo->m_dwValue),
#endif	// ASG_ADD_TIME_LIMIT_QUEST
                pRequestInfo->m_dwValue);
        }
        break;

        case QUEST_REQUEST_EVENT_MAP_CLEAR_BLOOD:
        case QUEST_REQUEST_EVENT_MAP_CLEAR_CHAOS:
        case QUEST_REQUEST_EVENT_MAP_CLEAR_DEVIL:
        case QUEST_REQUEST_EVENT_MAP_CLEAR_ILLUSION:
        {
#ifdef ASG_ADD_TIME_LIMIT_QUEST
            if (pRequestInfo->m_dwCurValue == 0)
#else	// ASG_ADD_TIME_LIMIT_QUEST
            if (pRequestInfo->m_wCurValue == 0)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
            {
                aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
                bRequestComplete = false;
            }
            else
                aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

            int nTextIndex = 0;
            switch (pRequestInfo->m_dwType)
            {
            case QUEST_REQUEST_EVENT_MAP_CLEAR_BLOOD:
                nTextIndex = 3078;
                break;
            case QUEST_REQUEST_EVENT_MAP_CLEAR_CHAOS:
                nTextIndex = 3076;
                break;
            case QUEST_REQUEST_EVENT_MAP_CLEAR_DEVIL:
                nTextIndex = 3080;
                break;
            case QUEST_REQUEST_EVENT_MAP_CLEAR_ILLUSION:
                nTextIndex = 3081;
                break;
            }
            swprintf(aDest[nLine].m_szText, GlobalText[nTextIndex], pRequestInfo->m_wIndex);
        }
        break;
        }
        aDest[nLine].m_szText[QM_MAX_REQUEST_REWARD_TEXT_LEN - 1] = 0;
    }

    BYTE byRewardCount;
    SQuestReward* pRewardInfo;
    i = 0;
    int j;
    for (j = 0; j < 2; ++j)
    {
        if (0 == j && pRequestReward->m_byGeneralRewardCount)
            ::wcscpy(aDest[nLine].m_szText, GlobalText[2810]);
        else if (1 == j && pRequestReward->m_byRandRewardCount)
            swprintf(aDest[nLine].m_szText, GlobalText[3082], pRequestReward->m_byRandGiveCount);
        else
            continue;
        aDest[nLine].m_hFont = g_hFontBold;
        aDest[nLine++].m_dwColor = ARGB(255, 179, 230, 77);

        byRewardCount = 0 == j
            ? pRequestReward->m_byGeneralRewardCount
            : pRequestReward->m_byGeneralRewardCount + pRequestReward->m_byRandRewardCount;
        for (; i < byRewardCount; ++i, ++nLine)
        {
            pRewardInfo = &pRequestReward->m_aReward[i];

            aDest[nLine].m_hFont = g_hFont;
            aDest[nLine].m_dwColor = 0 == j ? ARGB(255, 223, 191, 103) : ARGB(255, 103, 103, 223);
            aDest[nLine].m_eRequestReward = RRC_REWARD;
            aDest[nLine].m_dwType = pRewardInfo->m_dwType;
            aDest[nLine].m_wIndex = pRewardInfo->m_wIndex;
            aDest[nLine].m_pItem = pRewardInfo->m_pItem;

            switch (pRewardInfo->m_dwType)
            {
            case QUEST_REWARD_NONE:
                ::wcscpy(aDest[nLine].m_szText, GlobalText[1361]);
                break;

            case QUEST_REWARD_EXP:
                ::swprintf(aDest[nLine].m_szText, L"Exp.: %lu", pRewardInfo->m_dwValue);
                break;

            case QUEST_REWARD_ZEN:
                ::swprintf(aDest[nLine].m_szText, L"Zen: %lu", pRewardInfo->m_dwValue);
                break;

            case QUEST_REWARD_ITEM:
                wchar_t szItemName[32];
                ::GetItemName((int)pRewardInfo->m_pItem->Type, pRewardInfo->m_pItem->Level,
                    szItemName);
                ::swprintf(aDest[nLine].m_szText, L"Item: %s x %lu",
                    szItemName, pRewardInfo->m_dwValue);
                break;

            case QUEST_REWARD_BUFF:
            {
                const BuffInfo buffinfo = g_BuffInfo((eBuffState)pRewardInfo->m_wIndex);
                ::swprintf(aDest[nLine].m_szText, L"Bonus: %s x %lu%s", buffinfo.s_BuffName,
                    pRewardInfo->m_dwValue, GlobalText[2300]);
            }
            break;

#ifdef ASG_ADD_GENS_SYSTEM
            case QUEST_REWARD_CONTRIBUTE:
                swprintf(aDest[nLine].m_szText, GlobalText[2994], pRewardInfo->m_dwValue);
                break;
#endif	// ASG_ADD_GENS_SYSTEM
            }
            aDest[nLine].m_szText[QM_MAX_REQUEST_REWARD_TEXT_LEN - 1] = 0;
        }
    }
    return bRequestComplete;
}

void CQuestMng::SetEPRequestRewardState(DWORD dwQuestIndex, bool ProgressState)
{
    m_mapEPRequestRewardState[HIWORD(dwQuestIndex)] = ProgressState;
}

bool CQuestMng::IsEPRequestRewardState(DWORD dwQuestIndex)
{
    WORD wEP = HIWORD(dwQuestIndex);

    std::map<WORD, bool>::const_iterator iter = m_mapEPRequestRewardState.find(wEP);
    if (iter == m_mapEPRequestRewardState.end())
        return false;

    return m_mapEPRequestRewardState[wEP];
}

bool CQuestMng::IsQuestByEtc(DWORD dwQuestIndex)
{
    QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
    _ASSERT(iter != m_mapQuestProgress.end());

    if (iter->second.m_byUIType == 1)
        return true;
    else
        return false;
}

void CQuestMng::SetQuestIndexByEtcList(DWORD* adwSrcQuestIndex, int nIndexCount)
{
    m_listQuestIndexByEtc.clear();

    if (NULL == adwSrcQuestIndex)
        return;

    int i;
    for (i = 0; i < nIndexCount; ++i)
        m_listQuestIndexByEtc.push_back(adwSrcQuestIndex[i]);
}

bool CQuestMng::IsQuestIndexByEtcListEmpty()
{
    return m_listQuestIndexByEtc.empty();
}

void CQuestMng::SendQuestIndexByEtcSelection()
{
    if (IsQuestIndexByEtcListEmpty())
        return;

    auto iter = m_listQuestIndexByEtc.begin();
    const auto questNumber = static_cast<uint16_t>((*iter & 0xFF00) >> 16);
    const auto questGroup = static_cast<uint16_t>(*iter & 0xFF);
    SocketClient->ToGameServer()->SendQuestSelectRequest(questNumber, questGroup, 0);
}

void CQuestMng::DelQuestIndexByEtcList(DWORD dwQuestIndex)
{
    if (0x0000 == LOWORD(dwQuestIndex))
        return;

    DWordList::iterator iter;
    for (iter = m_listQuestIndexByEtc.begin(); iter != m_listQuestIndexByEtc.end(); advance(iter, 1))
    {
        if (HIWORD(*iter) == HIWORD(dwQuestIndex))
        {
            m_listQuestIndexByEtc.erase(iter);
            break;
        }
    }
}

void CQuestMng::SetCurQuestIndexList(DWORD* adwCurQuestIndex, int nIndexCount)
{
    m_listCurQuestIndex.clear();

    int i;
    for (i = 0; i < nIndexCount; ++i)
        if (GetSubject(adwCurQuestIndex[i]) != NULL)
            m_listCurQuestIndex.push_back(adwCurQuestIndex[i]);

    g_pMyQuestInfoWindow->SetCurQuestList(&m_listCurQuestIndex);
}

void CQuestMng::AddCurQuestIndexList(DWORD dwQuestIndex)
{
    WORD wEP = HIWORD(dwQuestIndex);
    WORD wQS = LOWORD(dwQuestIndex);
    bool bNotFound = true;

    DWordList::iterator iter;
    for (iter = m_listCurQuestIndex.begin(); iter != m_listCurQuestIndex.end(); advance(iter, 1))
    {
        if (wEP == HIWORD(*iter))
        {
            if (wQS != 0x0000 && wQS != 0x00ff)
                m_listCurQuestIndex.insert(iter, dwQuestIndex);

            m_listCurQuestIndex.erase(iter);

            bNotFound = false;
            break;
        }
    }

    if (bNotFound)
    {
        if (wQS != 0x0000 && wQS != 0x00ff)
            m_listCurQuestIndex.push_back(dwQuestIndex);
    }

    m_listCurQuestIndex.sort();

    g_pMyQuestInfoWindow->SetCurQuestList(&m_listCurQuestIndex);
}

void CQuestMng::RemoveCurQuestIndexList(DWORD dwQuestIndex)
{
    WORD wEP = HIWORD(dwQuestIndex);

    DWordList::iterator iter;
    for (iter = m_listCurQuestIndex.begin(); iter != m_listCurQuestIndex.end(); advance(iter, 1))
    {
        if (wEP == HIWORD(*iter))
        {
            m_listCurQuestIndex.erase(iter);
            break;
        }
    }

    g_pMyQuestInfoWindow->SetCurQuestList(&m_listCurQuestIndex);
}

bool CQuestMng::IsIndexInCurQuestIndexList(DWORD dwQuestIndex)
{
    DWordList::iterator iter;
    for (iter = m_listCurQuestIndex.begin(); iter != m_listCurQuestIndex.end(); advance(iter, 1))
    {
        if (*iter == dwQuestIndex)
            return true;
    }

    return false;
}