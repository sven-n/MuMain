#include "stdafx.h"
#include <memory>
#include "UIManager.h"
#include "GuildCache.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzLodTerrain.h"
#include "ZzzPath.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "ZzzEffect.h"
#include "ZzzOpenglUtil.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"

#include "DSPlaySound.h"

#include "MatchEvent.h"
#include "GOBoid.h"
#include "CSQuest.h"
#include "PersonalShopTitleImp.h"
#include "npcBreeder.h"
#include "GIPetManager.h"
#include "CSMapServer.h"
#include "npcGateSwitch.h"
#include "CComGem.h"
#include "UIMapName.h" // rozy
#include "UIMng.h"
#include "CDirection.h"
#include "CSParts.h"
#include "PhysicsManager.h"
#include "Event.h"
#include "MixMgr.h"
#include "MapManager.h"
#include "UIGuardsMan.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "NewUICustomMessageBox.h"
#include "NewUIInventoryCtrl.h"
#include "w_CursedTemple.h"
#include "SummonSystem.h"

#include "CharacterManager.h"
#include "SkillManager.h"

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "GameShop\InGameShopSystem.h"
#include "GameShop\MsgBoxIGSCommon.h"
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#include "w_MapHeaders.h"

#include "w_PetProcess.h"
#include "PortalMgr.h"
#include "DuelMgr.h"

#include "GambleSystem.h"
#include "QuestMng.h"
#ifdef PBG_ADD_SECRETBUFF
#include "FatigueTimeSystem.h"
#endif //PBG_ADD_SECRETBUFF
#include <codecvt>

#include "ServerListManager.h"
#include "MonkSystem.h"

#include "Dotnet/Connection.h"

#include "MUHelper/MuHelper.h"

#define MAX_DEBUG_MAX 10

extern BYTE m_AltarState[];
extern int g_iChatInputType;
extern BOOL g_bUseChatListBox;

extern BOOL g_bUseWindowMode;
extern CUITextInputBox* g_pSingleTextInputBox;

#ifdef _PVP_ADD_MOVE_SCROLL
extern CMurdererMove g_MurdererMove;
#endif	// _PVP_ADD_MOVE_SCROLL

extern  short   g_shCameraLevel;

extern BYTE DebugText[MAX_DEBUG_MAX][256];
extern int  DebugTextLength[MAX_DEBUG_MAX];
extern char DebugTextCount;
extern int  TotalPacketSize;
extern int g_iKeyPadEnable;

extern BOOL g_bWhileMovingZone;
extern DWORD g_dwLatestZoneMoving;

extern CUIMapName* g_pUIMapName; 

extern bool g_PetEnableDuel;

MASTER_LEVEL_VALUE	Master_Level_Data;

//BYTE Version[SIZE_PROTOCOLVERSION] = {'1'+1, '0'+2, '4'+3, '0'+4, '5'+5};
//BYTE Serial[SIZE_PROTOCOLSERIAL+1] = {"TbYehR2hFUPBKgZj"};

BYTE Version[SIZE_PROTOCOLVERSION] = { '2', '0', '4', '0', '4' };
BYTE Serial[SIZE_PROTOCOLSERIAL + 1] = { "k1Pk2jcET48mxL3b" };
Connection* SocketClient = nullptr;
bool EnableSocket = false;


BOOL    g_bGameServerConnected = FALSE;

MATCH_RESULT	g_wtMatchResult;
PMSG_MATCH_TIMEVIEW	g_wtMatchTimeLeft;
int g_iGoalEffect = 0;

CROWN_SWITCH_INFO* Switch_Info = nullptr;

int     HeroKey;
int     CurrentProtocolState;

int DirTable[16] = { -1,-1,  0,-1,  1,-1,  1,0,  1,1,  0,1,  -1,1,  -1,0 };

wchar_t    Password[MAX_ID_SIZE + 1];
wchar_t    QuestionID[MAX_ID_SIZE + 1];
wchar_t    Question[31];

#define FIRST_CROWN_SWITCH_NUMBER	322

void AddDebugText(const unsigned char* Buffer, int Size)
{
    if (DebugTextCount > MAX_DEBUG_MAX - 1)
    {
        DebugTextCount = MAX_DEBUG_MAX - 1;
        for (int i = 1; i < MAX_DEBUG_MAX; i++)
        {
            memcpy(DebugText[i - 1], DebugText[i], DebugTextLength[i]);
            DebugTextLength[i - 1] = DebugTextLength[i];
        }
    }
    if (Size <= 256)
    {
        memcpy(DebugText[DebugTextCount], Buffer, Size);
        DebugTextLength[DebugTextCount] = Size;
        DebugTextCount++;
    }
}

// Forward declaration
static void HandleIncomingPacket(int32_t Handle, const BYTE* ReceiveBuffer, int32_t Size);

BOOL CreateSocket(wchar_t* IpAddr, unsigned short Port)
{
    BOOL bResult = TRUE;
    g_ConsoleDebug->Write(MCD_NORMAL, L"[Connect to Server] ip address = %s, port = %d", IpAddr, Port);

    // todo: generally, it's a bad idea to assume a specific port number (range).
    const bool isEncrypted = Port > 0xADFF || Port < 0xAD00;
    SocketClient = new Connection(IpAddr, Port, isEncrypted, &HandleIncomingPacket);
    if (!SocketClient->IsConnected())
    {
        bResult = FALSE;
        g_ErrorReport.Write(L"Failed to connect. ");
        g_ErrorReport.WriteCurrentTime();
        free(SocketClient);
        SocketClient = nullptr;
        CUIMng::Instance().PopUpMsgWin(MESSAGE_SERVER_LOST);
    }

    return bResult;
}

void DeleteSocket()
{
    if (SocketClient)
    {
        SocketClient->Close();
        SocketClient = nullptr;
    }
}


int  LogIn = 0;
wchar_t LogInID[MAX_ID_SIZE + 1] = { 0, };

bool First = false;
int FirstTime = 0;

bool LogOut = false;

wchar_t ChatWhisperID[MAX_ID_SIZE + 1];

int MoveCount = 0;

int CurrentSkill = 0;

int BuyCost = 0;

int  EnableUse = 0; // todo: get rid of this, it may cause the stuck client bug, so that players can't use items anymore.

int SendGetItem = -1; // todo: get rid of this, it may cause the stuck client bug, so that players can't pick up anything anymore.
int SendDropItem = -1; // todo: get rid of this, it may cause the stuck client bug, so that players can't drop anything anymore.

int FindGuildName(wchar_t* Name)
{
    for (int i = 0; i < MARK_EDIT; i++)
    {
        MARK_t* p = &GuildMark[i];
        if (wcscmp(p->GuildName, Name) == NULL)
        {
            return i;
        }
    }
    return -1;
}

void GuildTeam(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    if (o->Live)
    {
        c->GuildTeam = 0;

        if (Hero->GuildMarkIndex != -1 && c->GuildMarkIndex == Hero->GuildMarkIndex)
            c->GuildTeam = 1;

        if (EnableGuildWar)
        {
            if (GuildWarIndex == -1 && GuildWarName[0])
            {
                GuildWarIndex = FindGuildName(GuildWarName);
            }
            if (GuildWarIndex >= 0 && c->GuildMarkIndex == GuildWarIndex)
            {
                c->GuildTeam = 2;
            }
        }
    }
}

void InitGuildWar()
{
    EnableGuildWar = false;
    GuildWarIndex = -1;
    GuildWarName[0] = NULL;
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        GuildTeam(c);
    }
}

BOOL Util_CheckOption(std::wstring lpszCommandLine, wchar_t cOption, std::wstring &lpszString);

void ReceiveServerList(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
    int Offset = sizeof(PHEADER_DEFAULT_SUBCODE_WORD);

    BYTE Value2 = *(ReceiveBuffer + Offset++);

    g_ServerListManager->Release();

    g_ServerListManager->SetTotalServer(MAKEWORD(Value2, Data->Value));

    for (int i = 0; i < g_ServerListManager->GetTotalServer(); i++)
    {
        auto Data2 = (LPPRECEIVE_SERVER_LIST)(ReceiveBuffer + Offset);

        g_ServerListManager->InsertServerGroup(Data2->Index, Data2->Percent);

        g_ConsoleDebug->Write(MCD_RECEIVE, L"0xF4 [ReceiveServerList(%d %d %d)]", i, Data2->Index, Data2->Percent);

        Offset += sizeof(PRECEIVE_SERVER_LIST);
    }

    CUIMng& rUIMng = CUIMng::Instance();
    if (!rUIMng.m_CreditWin.IsShow())
    {
        rUIMng.ShowWin(&rUIMng.m_ServerSelWin);
        rUIMng.m_ServerSelWin.UpdateDisplay();
        rUIMng.ShowWin(&rUIMng.m_LoginMainWin);
    }

    g_ErrorReport.Write(L"Success Receive Server List.\r\n");

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0xF4 [ReceiveServerList]");
}
void ReceiveServerConnect(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_SERVER_ADDRESS)ReceiveBuffer;
    wchar_t IP[16];
    CMultiLanguage::ConvertFromUtf8(IP, Data->IP);

    g_ErrorReport.Write(L"[ReceiveServerConnect]");
    if (SocketClient != nullptr)
    {
        SocketClient->Close();
    }

    if (CreateSocket(IP, Data->Port))
    {
        g_bGameServerConnected = TRUE;
    }

    wchar_t Text[100];
    swprintf(Text, GlobalText[481], IP, Data->Port);
    g_pSystemLogBox->AddText( Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

void ReceiveServerConnectBusy(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_SERVER_BUSY)ReceiveBuffer;
    SocketClient->ToConnectServer()->SendServerListRequest();
}

void ReceiveJoinServer(const BYTE* ReceiveBuffer)
{
    auto Data2 = (LPPRECEIVE_JOIN_SERVER)ReceiveBuffer;

    if (LogIn != 0)
    {
        HeroKey = ((int)(Data2->NumberH) << 8) + Data2->NumberL;
        g_csMapServer.SendChangeMapServer();
    }
    else
    {
        CUIMng& rUIMng = CUIMng::Instance();

        switch (Data2->Result)
        {
        case 0x01:
            rUIMng.ShowWin(&rUIMng.m_LoginWin);
            HeroKey = ((int)(Data2->NumberH) << 8) + Data2->NumberL;
            CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
            break;

        default:
            g_ErrorReport.Write(L"Connecting error. ");
            g_ErrorReport.WriteCurrentTime();
            rUIMng.PopUpMsgWin(MESSAGE_SERVER_LOST);
            break;
        }

        u_int64 received = 0;
        u_int64 actual = 0;
        for (int i = 0; i < SIZE_PROTOCOLVERSION; ++i)
        {
            received |= Data2->Version[i];
            received <<= 8;
            actual |= Version[i];
            actual <<= 8;
        }

        if (actual < received)
        {
            rUIMng.HideWin(&rUIMng.m_LoginWin);
            rUIMng.PopUpMsgWin(MESSAGE_VERSION);
            g_ErrorReport.Write(L"Version dismatch - Join server.\r\n");
        }
    }

    g_GuildCache.Reset();

//#if defined _DEBUG || defined FOR_WORK
//    if (Data2->Result == 0x01)
//    {
//        wchar_t lpszTemp[256];
//        if (Util_CheckOption(GetCommandLineW(), L'i', lpszTemp))
//        {
//            g_ErrorReport.Write(L"> Try to Login \"%s\"\r\n", m_ID);
//            SendRequestLogIn(m_ID, lpszTemp);
//        }
//    }
//#endif
}

void ReceiveConfirmPassword(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_CONFIRM_PASSWORD)ReceiveBuffer;
    switch (Data->Result)
    {
    case 0:
        CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD_FAIL_ID;
        break;
    case 1:
        BuxConvert((BYTE*)Data->Question, sizeof(Data->Question));
        CMultiLanguage::ConvertFromUtf8(Question, Data->Question, sizeof Data->Question);
        CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD_SUCCESS;
        break;
    }
}

void ReceiveConfirmPassword2(const std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PRECEIVE_CONFIRM_PASSWORD2>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    switch (Data->Result)
    {
    case 1:
        BuxConvert((BYTE*)Data->Password, sizeof(Data->Password));
        CMultiLanguage::ConvertFromUtf8(Password, Data->Password, sizeof Data->Password);
        CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD2_SUCCESS;
        break;
    case 0:
        CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD2_FAIL_ID;
        break;
    case 2:
        CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD2_FAIL_ANSWER;
        break;
    case 3:
        CurrentProtocolState = RECEIVE_CONFIRM_PASSWORD2_FAIL_RESIDENT;
        break;
    }
}

void ReceiveChangePassword(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
    switch (Data->Value)
    {
    case 1:
        CurrentProtocolState = RECEIVE_CHANGE_PASSWORD_SUCCESS;
        break;
    case 0:
        CurrentProtocolState = RECEIVE_CHANGE_PASSWORD_FAIL_ID;
        break;
    case 2:
        CurrentProtocolState = RECEIVE_CHANGE_PASSWORD_FAIL_RESIDENT;
        break;
    case 3:
        CurrentProtocolState = RECEIVE_CHANGE_PASSWORD_FAIL_PASSWORD;
        break;
    }
}

void ReceiveCharacterListExtended(const BYTE* ReceiveBuffer)
{
    InitGuildWar();

    auto Data = (LPPHEADER_DEFAULT_CHARACTER_LIST)ReceiveBuffer;

    int Offset = sizeof(PHEADER_DEFAULT_CHARACTER_LIST);

#ifdef _DEBUG
    g_ConsoleDebug->Write(MCD_RECEIVE, L"[ReceiveList Count %d Max class %d]", Data->CharacterCount, Data->MaxClass);
#else
    g_ErrorReport.Write(L"[ReceiveList Count %d Max class %d]", Data->CharacterCount, Data->MaxClass);
#endif

    CharacterAttribute->IsVaultExtended = Data->IsVaultExtended;
    for (int i = 0; i < Data->CharacterCount; i++)
    {
        auto Data2 = (LPPRECEIVE_CHARACTER_LIST_EXTENDED)(ReceiveBuffer + Offset);

        auto iClass = gCharacterManager.ChangeServerClassTypeToClientClassType(Data2->Class);
        float fPos[2], fAngle = 0.0f;

        switch (Data2->Index)
        {
            case 0:	fPos[0] = 8008.0f;	fPos[1] = 18885.0f;	fAngle = 115.0f; break;
            case 1:	fPos[0] = 7986.0f;	fPos[1] = 19145.0f;	fAngle = 90.0f; break;
            case 2:	fPos[0] = 8046.0f;	fPos[1] = 19400.0f;	fAngle = 75.0f; break;
            case 3:	fPos[0] = 8133.0f;	fPos[1] = 19645.0f;	fAngle = 60.0f; break;
            case 4:	fPos[0] = 8282.0f;	fPos[1] = 19845.0f;	fAngle = 35.0f; break;
            default: return;
        }

        CHARACTER* c = CreateHero(Data2->Index, iClass, 0, fPos[0], fPos[1], fAngle);

        c->Level = Data2->Level;
        c->CtlCode = Data2->CtlCode;

        memset(c->ID, 0, sizeof(c->ID));

        CMultiLanguage::ConvertFromUtf8(c->ID, Data2->ID, MAX_ID_SIZE);

        ReadEquipmentExtended(Data2->Index, Data2->Flags, Data2->Equipment);

        c->GuildStatus = Data2->byGuildStatus;
        Offset += sizeof(PRECEIVE_CHARACTER_LIST_EXTENDED);
    }

    CurrentProtocolState = RECEIVE_CHARACTERS_LIST;
}

CHARACTER_ENABLE g_CharCardEnable;

void ReceiveCharacterCard_New(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_CHARACTERCARD)ReceiveBuffer;
    g_CharCardEnable.bCharacterEnable[0] = false;
    g_CharCardEnable.bCharacterEnable[1] = false;
    g_CharCardEnable.bCharacterEnable[2] = false;

    if ((Data->CharacterCard & CLASS_DARK_CARD) == CLASS_DARK_CARD)
        g_CharCardEnable.bCharacterEnable[0] = true;

    if ((Data->CharacterCard & CLASS_DARK_LORD_CARD) == CLASS_DARK_LORD_CARD)
        g_CharCardEnable.bCharacterEnable[1] = true;

    if ((Data->CharacterCard & CLASS_SUMMONER_CARD) == CLASS_SUMMONER_CARD)
        g_CharCardEnable.bCharacterEnable[2] = true;

    g_ConsoleDebug->Write(MCD_NORMAL, L"[BOTH MESSAGE] CharacterCard Recv %d = %d %d %d", Data->CharacterCard, g_CharCardEnable.bCharacterEnable[0], g_CharCardEnable.bCharacterEnable[1], g_CharCardEnable.bCharacterEnable[2]);
}

void ReceiveCreateCharacter(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_CREATE_CHARACTER)ReceiveBuffer;
    if (Data->Result == 1)
    {
        float fPos[2] = { 0.0f,0.0f }, fAngle = 0.0f;

        switch (Data->Index)
        {
            case 0:	fPos[0] = 8008.0f;	fPos[1] = 18885.0f;	fAngle = 115.0f; break;
            case 1:	fPos[0] = 7986.0f;	fPos[1] = 19145.0f;	fAngle = 90.0f; break;
            case 2:	fPos[0] = 8046.0f;	fPos[1] = 19400.0f;	fAngle = 75.0f; break;
            case 3:	fPos[0] = 8133.0f;	fPos[1] = 19645.0f;	fAngle = 60.0f; break;
            case 4:	fPos[0] = 8282.0f;	fPos[1] = 19845.0f;	fAngle = 35.0f; break;
        }

        INT		iCharacterKey;
        iCharacterKey = Data->Index;
        DeleteCharacter(iCharacterKey);

        CreateHero(Data->Index, CharacterView.Class, CharacterView.Skin, fPos[0], fPos[1], fAngle);
        CharactersClient[Data->Index].Level = Data->Level;
        auto serverClass = (SERVER_CLASS_TYPE)(Data->Class >> 3);
        auto iClass = gCharacterManager.ChangeServerClassTypeToClientClassType(serverClass);

        CharactersClient[Data->Index].Class = iClass;
        CharactersClient[Data->Index].SkinIndex = gCharacterManager.GetSkinModelIndex(iClass);
        CMultiLanguage::ConvertFromUtf8(CharactersClient[Data->Index].ID, Data->ID, MAX_ID_SIZE);
        CharactersClient[Data->Index].ID[MAX_ID_SIZE] = L'\0';
        CurrentProtocolState = RECEIVE_CREATE_CHARACTER_SUCCESS;
        CUIMng& rUIMng = CUIMng::Instance();
        rUIMng.CloseMsgWin();
        rUIMng.m_CharSelMainWin.UpdateDisplay();
        rUIMng.m_CharInfoBalloonMng.UpdateDisplay();
    }
    else if (Data->Result == 0)
        CUIMng::Instance().PopUpMsgWin(RECEIVE_CREATE_CHARACTER_FAIL);
    else if (Data->Result == 2)
        CUIMng::Instance().PopUpMsgWin(RECEIVE_CREATE_CHARACTER_FAIL2);

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x01 [ReceiveCreateCharacter]");
}

void ReceiveDeleteCharacter(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
    switch (Data->Value)
    {
    case 1:
        INT		iKey;
        iKey = CharactersClient[SelectedHero].Key;
        DeleteCharacter(iKey);
        CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_SUCCESS);
        break;
    case 0:
        CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_GUILDWARNING);
        break;
    case 3:
        CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_ITEM_BLOCK);
        break;
    case 2:
    default:
        CUIMng::Instance().PopUpMsgWin(MESSAGE_STORAGE_RESIDENTWRONG);
        break;
    }
}

int SummonLife = 0;

extern void StopMusic();

void InitGame()
{
    EnableUse = 0;
    SendGetItem = -1;
    SummonLife = 0;
    SoccerTime = 0;
    SoccerObserver = false;

    g_wtMatchResult.Clear();
    g_wtMatchTimeLeft.m_Time = 0;
    g_iGoalEffect = 0;

    g_shCameraLevel = 0;

    SelectedNpc = -1;
    SelectedOperate = -1;
    SelectedCharacter = -1;
    SelectedItem = -1;

    Attacking = -1;
    g_pOption->SetAutoAttack(true);
    g_pOption->SetWhisperSound(false);

    CheckInventory = nullptr;

    SocketClient->ToGameServer()->SendCloseNpcRequest();

    g_iFollowCharacter = -1;

    LockInputStatus = false;

    RepairEnable = 0;
    CheckSkill = -1;

    ClearNotice();

    CharacterAttribute->InventoryExtensions = 0;
    CharacterAttribute->Ability = 0;
    CharacterAttribute->AbilityTime[0] = 0;
    CharacterAttribute->AbilityTime[1] = 0;
    CharacterAttribute->AbilityTime[2] = 0;

    CharacterMachine->StorageGold = 0;
    CharacterMachine->Gold = 0;

    g_shEventChipCount = 0;
    g_shMutoNumber[0] = -1;
    g_shMutoNumber[1] = -1;
    g_shMutoNumber[2] = -1;

    ClearWhisperID();

    matchEvent::ClearMatchInfo();

    InitPartyList();

    g_csQuest.clearQuest();

    g_DuelMgr.Reset();
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUEL_WINDOW);

    if (g_pUIManager)
        g_pUIManager->Init();

    if (g_pSiegeWarfare)
        g_pSiegeWarfare->InitMiniMapUI();

    g_Direction.Init();
    g_Direction.DeleteMonster();

    RemoveAllPerosnalItemPrice(PSHOPWNDTYPE_SALE);
    RemoveAllPerosnalItemPrice(PSHOPWNDTYPE_PURCHASE);

    g_pNewUIHotKey->SetStateGameOver(false);
    g_pMyShopInventory->ResetSubject();
    g_pChatListBox->ResetFilter();

    g_pGuildInfoWindow->NoticeClear();
}

BOOL ReceiveLogOut(const BYTE* ReceiveBuffer, BOOL bEncrypted)
{
    LogOut = false;
    auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:
        PostMessage(g_hWnd, WM_DESTROY, 0, 0);
        break;
    case 1:
        StopMusic();
        AllStopSound();

        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();

        ReleaseMainData();
        CryWolfMVPInit();

        SceneFlag = CHARACTER_SCENE;
        CurrentProtocolState = REQUEST_CHARACTERS_LIST;
        SocketClient->ToGameServer()->SendRequestCharacterList(g_pMultiLanguage->GetLanguage());

        InitCharacterScene = false;
        InitMainScene = false;
        EnableMainRender = false;
        CurrentProtocolState = REQUEST_JOIN_SERVER;
        InitGame();
        break;
    case 2:
        if (SceneFlag == MAIN_SCENE)
        {
            CryWolfMVPInit();
            StopMusic();
            AllStopSound();
            SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
            ReleaseMainData();
        }

        g_ErrorReport.Write(L"[ReceiveLogOut]");
        if (SocketClient != nullptr)
        {
            SocketClient->Close();
            g_bGameServerConnected = false;
        }

        ReleaseCharacterSceneData();
        SceneFlag = LOG_IN_SCENE;

        InitLogIn = false;
        InitCharacterScene = false;
        InitMainScene = false;
        EnableMainRender = false;
        CurrentProtocolState = REQUEST_JOIN_SERVER;

        LogIn = 0;
        g_csMapServer.Init();
        InitGame();
        break;
    }

    g_pWindowMgr->Reset();
    g_pFriendList->ClearFriendList();
    g_pLetterList->ClearLetterList();

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x02 [ReceiveServerList(%d)]", Data->Value);

    return (TRUE);
}

int HeroIndex;

BOOL ReceiveJoinMapServer(std::span<const BYTE> ReceiveBuffer)
{
    MouseLButton = false;
    HeroIndex = rand() % MAX_CHARACTERS_CLIENT;
    CHARACTER* c = &CharactersClient[HeroIndex];

    CharacterAttribute->Ability = 0;
    CharacterAttribute->AbilityTime[0] = 0;
    CharacterAttribute->AbilityTime[1] = 0;
    CharacterAttribute->AbilityTime[2] = 0;

    auto const Data = safe_cast<PRECEIVE_JOIN_MAP_SERVER_EXTENDED>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return false;
    }

    CharacterAttribute->Experience = ntoh64(Data->CurrentExperience);
    CharacterAttribute->NextExperience = ntoh64(Data->ExperienceForNextLevel);
    CharacterAttribute->LevelUpPoint = Data->LevelUpPoint;
    CharacterAttribute->Strength = Data->Strength;
    CharacterAttribute->Dexterity = Data->Dexterity;
    CharacterAttribute->Vitality = Data->Vitality;
    CharacterAttribute->Energy = Data->Energy;
    CharacterAttribute->Charisma = Data->Charisma;
    CharacterAttribute->Life = Data->Life;
    CharacterAttribute->LifeMax = Data->LifeMax;
    CharacterAttribute->Mana = Data->Mana;
    CharacterAttribute->ManaMax = Data->ManaMax;
    CharacterAttribute->SkillMana = Data->SkillMana;
    CharacterAttribute->SkillManaMax = Data->SkillManaMax;
    CharacterAttribute->Shield = Data->Shield;
    CharacterAttribute->ShieldMax = Data->ShieldMax;
    CharacterAttribute->AddPoint = Data->AddPoint;
    CharacterAttribute->MaxAddPoint = Data->MaxAddPoint;
    CharacterAttribute->wMinusPoint = Data->wMinusPoint;
    CharacterAttribute->wMaxMinusPoint = Data->wMaxMinusPoint;
    CharacterAttribute->InventoryExtensions = Data->InventoryExtensions;
    CharacterAttribute->AttackSpeed = Data->AttackSpeed;
    CharacterAttribute->MagicSpeed = Data->MagicSpeed;
    CharacterAttribute->MaxAttackSpeed = Data->MaxAttackSpeed;
    CharacterMachine->Gold = Data->Gold;

    gMapManager.WorldActive = Data->Map;
    gMapManager.LoadWorld(gMapManager.WorldActive);

    if (gMapManager.WorldActive == WD_34CRYWOLF_1ST)
    {
        SocketClient->ToGameServer()->SendCrywolfInfoRequest();
    }

    matchEvent::CreateEventMatch(gMapManager.WorldActive);

    CreateCharacterPointer(c, MODEL_PLAYER, Data->PositionX, Data->PositionY, ((float)Data->Angle - 1.f) * 45.f);
    c->Key = HeroKey;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x03 [ReceiveJoinMapServer] Key: %d Map: %d X: %d Y:%d", c->Key, gMapManager.WorldActive, Data->PositionX, Data->PositionY);
    OBJECT* o = &c->Object;
    c->Class = CharacterAttribute->Class;
    c->SkinIndex = gCharacterManager.GetSkinModelIndex(c->Class);
    c->Skin = 0;
    c->PK = Data->PK;
    c->CtlCode = Data->CtlCode;

    o->Kind = KIND_PLAYER;
    SetCharacterClass(c);

    Hero = c;

    memset(c->ID, 0, sizeof c->ID);
    wcscpy(c->ID, CharacterAttribute->Name);

    for (auto & i : CharacterMachine->Equipment)
    {
        i.Type = -1;
        i.Level = 0;
        i.ExcellentFlags = 0;
    }
    
    CreateEffect(BITMAP_MAGIC + 2, o->Position, o->Angle, o->Light, 0, o);
    CurrentProtocolState = RECEIVE_JOIN_MAP_SERVER;

    LockInputStatus = false;
    CheckIME_Status(true, 0);

    LoadingWorld = 30;
    MouseUpdateTime = 0;
    MouseUpdateTimeMax = 6;

    CreatePetDarkSpirit_Now(Hero);

    CreateEffect(BITMAP_MAGIC + 2, o->Position, o->Angle, o->Light, 0, o);
    o->Alpha = 0.f;

    g_pNewUISystem->HideAll();

    SelectedItem = -1;
    SelectedNpc = -1;
    SelectedCharacter = -1;
    SelectedOperate = -1;
    Attacking = -1;
    RepairEnable = 0;

    Hero->Movement = false;
    SetPlayerStop(Hero);

    if (gMapManager.InBloodCastle() == false)
    {
        StopBuffer(SOUND_BLOODCASTLE, true);
    }

    if (gMapManager.InChaosCastle() == false)
    {
        StopBuffer(SOUND_CHAOSCASTLE, true);
        StopBuffer(SOUND_CHAOS_ENVIR, true);
    }

    if (gMapManager.IsEmpireGuardian1() == false &&
        gMapManager.IsEmpireGuardian2() == false &&
        gMapManager.IsEmpireGuardian3() == false &&
        gMapManager.IsEmpireGuardian4() == false)
    {
        StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, true);
        StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, true);
        StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, true);
        StopBuffer(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, true);
    }

    g_pUIMapName->ShowMapName();

    CreateMyGensInfluenceGroundEffect();

    if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4);
    else
    {
        wchar_t Text[256];
        swprintf(Text, L"%s%s", GlobalText[484], gMapManager.GetMapName(gMapManager.WorldActive));

        g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    if (gMapManager.WorldActive == WD_30BATTLECASTLE)
    {
        if (g_pSiegeWarfare)
            g_pSiegeWarfare->CreateMiniMapUI();
    }

    if (gMapManager.WorldActive < WD_65DOPPLEGANGER1 || gMapManager.WorldActive > WD_68DOPPLEGANGER4)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);
    }

    if (gMapManager.WorldActive < WD_69EMPIREGUARDIAN1 || WD_72EMPIREGUARDIAN4 < gMapManager.WorldActive)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x03 [ReceiveJoinMapServer]");

    return (TRUE);
}

void ReceiveRevival(const BYTE* ReceiveBuffer)
{
    MouseLButton = false;
    Hero->Object.Teleport = false;
    Hero->Object.Live = false;
    auto Data = (LPPRECEIVE_REVIVAL_EXTENDED)ReceiveBuffer;

    CharacterAttribute->Life = Data->Life;
    CharacterAttribute->Mana = Data->Mana;
    CharacterAttribute->Shield = Data->Shield;
    CharacterAttribute->SkillMana = Data->SkillMana;

    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        Master_Level_Data.lMasterLevel_Experince = Data->CurrentExperience;
    }
    else
    {
        CharacterAttribute->Experience = Data->CurrentExperience;
    }

    CharacterMachine->Gold = Data->Gold;
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        c->Object.Live = false;
    }

    BYTE Temp = Hero->PK;
    int TempGuild = Hero->GuildMarkIndex;

    CHARACTER* c = &CharactersClient[HeroIndex];

    BYTE BackUpGuildStatus = c->GuildStatus;
    BYTE BackUpGuildType = c->GuildType;
    BYTE BackUpGuildRelationShip = c->GuildRelationShip;
    BYTE byBackupEtcPart = c->EtcPart;

    CreateCharacterPointer(c, MODEL_PLAYER, Data->PositionX, Data->PositionY, ((float)Data->Angle - 1.f) * 45.f);
    c->Key = HeroKey;
    c->GuildStatus = BackUpGuildStatus;
    c->GuildType = BackUpGuildType;
    c->GuildRelationShip = BackUpGuildRelationShip;
    c->EtcPart = byBackupEtcPart;

    OBJECT* o = &c->Object;
    c->Class = CharacterAttribute->Class;
    c->Skin = 0;
    c->PK = Temp;
    o->Kind = KIND_PLAYER;
    c->GuildMarkIndex = TempGuild;
    c->SafeZone = true;
    SetCharacterClass(c);
    SetPlayerStop(c);
    CreateEffect(BITMAP_MAGIC + 2, o->Position, o->Angle, o->Light, 0, o);
    ClearItems();
    ClearCharacters(HeroKey);
    RemoveAllShopTitleExceptHero();

    if (gMapManager.WorldActive >= WD_45CURSEDTEMPLE_LV1 && gMapManager.WorldActive <= WD_45CURSEDTEMPLE_LV6)
    {
        if (!(Data->Map >= WD_45CURSEDTEMPLE_LV1 && Data->Map <= WD_45CURSEDTEMPLE_LV6))
        {
            g_CursedTemple->ResetCursedTemple();
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
        }
    }

    if (gMapManager.WorldActive != Data->Map)
    {
        int OldWorld = gMapManager.WorldActive;

        gMapManager.WorldActive = Data->Map;
        gMapManager.LoadWorld(gMapManager.WorldActive);

        if ((gMapManager.InChaosCastle(OldWorld) == true && OldWorld != gMapManager.WorldActive)
            || gMapManager.InChaosCastle() == true)
        {
            SetCharacterClass(Hero);
        }

        if (gMapManager.InChaosCastle() == false)
        {
            StopBuffer(SOUND_CHAOSCASTLE, true);
            StopBuffer(SOUND_CHAOS_ENVIR, true);
        }

        if (gMapManager.IsEmpireGuardian1() == false &&
            gMapManager.IsEmpireGuardian2() == false &&
            gMapManager.IsEmpireGuardian3() == false &&
            gMapManager.IsEmpireGuardian4() == false)
        {
            StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, true);
            StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, true);
            StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, true);
            StopBuffer(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, true);
        }

        if (gMapManager.WorldActive == -1 || c->Helper.Type != MODEL_HORN_OF_DINORANT || c->SafeZone)
        {
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
        }
        else
        {
            if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
                o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 90.f;
            else
                o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 30.f;
        }
    }

    CreatePetDarkSpirit_Now(c);
    SummonLife = 0;
    GuildTeam(c);

    g_pUIMapName->ShowMapName();

    CreateMyGensInfluenceGroundEffect();

    if (gMapManager.WorldActive < WD_65DOPPLEGANGER1 || gMapManager.WorldActive > WD_68DOPPLEGANGER4)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);
    }
    if (gMapManager.WorldActive < WD_69EMPIREGUARDIAN1 || WD_72EMPIREGUARDIAN4 < gMapManager.WorldActive)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
    }

    g_pNewUISystem->HideAll();

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x04 [ReceiveRevival]");
}

void ReceiveMagicList(const BYTE* ReceiveBuffer)
{
    int Master_Skill_Bool = -1;
    int Skill_Bool = -1;

    auto Data = (LPPHEADER_MAGIC_LIST_COUNT)ReceiveBuffer;
    int Offset = sizeof(PHEADER_MAGIC_LIST_COUNT);
    if (Data->Value == 0xFF)
    {
        auto Data2 = (LPPRECEIVE_MAGIC_LIST)(ReceiveBuffer + Offset);
        CharacterAttribute->Skill[Data2->Index] = AT_SKILL_UNDEFINED;
        CharacterAttribute->SkillLevel[Data2->Index] = 0;
    }
    else if (Data->Value == 0xFE)
    {
        auto Data2 = (LPPRECEIVE_MAGIC_LIST)(ReceiveBuffer + Offset);
        CharacterAttribute->Skill[Data2->Index] = (ActionSkillType)Data2->Type;
        CharacterAttribute->SkillLevel[Data2->Index] = Data2->Level;
    }
    else if (Data->ListType == 0x02)
    {
        for (int i = 0; i < Data->Value; ++i)
        {
            auto Data2 = (LPPRECEIVE_MAGIC_LIST)(ReceiveBuffer + Offset);
            CharacterAttribute->Skill[Data2->Index] = AT_SKILL_UNDEFINED;
            CharacterAttribute->SkillLevel[Data2->Index] = 0;
        }
    }
    else
    {
        if (Data->ListType == 0x00)
        {
            ZeroMemory(CharacterAttribute->Skill, MAX_SKILLS * sizeof(WORD));
            ZeroMemory(CharacterAttribute->SkillLevel, MAX_SKILLS * sizeof(BYTE));
        }
        for (int i = 0; i < Data->Value; i++)
        {
            auto Data2 = (LPPRECEIVE_MAGIC_LIST)(ReceiveBuffer + Offset);
            CharacterAttribute->Skill[Data2->Index] = (ActionSkillType)Data2->Type;
            CharacterAttribute->SkillLevel[Data2->Index] = Data2->Level;
            Offset += sizeof(PRECEIVE_MAGIC_LIST);
        }

        if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD)
        {
            for (int i = 0; i < PET_CMD_END; ++i)
            {
                CharacterAttribute->Skill[AT_PET_COMMAND_DEFAULT + i] = (ActionSkillType)(AT_PET_COMMAND_DEFAULT + i);
            }
        }
    }

    CharacterAttribute->SkillNumber = 0;
    CharacterAttribute->SkillMasterNumber = 0;

    int SkillType = 0;
    for (auto & i : CharacterAttribute->Skill)
    {
        SkillType = i;
        if (SkillType != 0)
        {
            CharacterAttribute->SkillNumber++;
            BYTE SkillUseType = SkillAttribute[SkillType].SkillUseType;
            if (SkillUseType == SKILL_USE_TYPE_MASTER)
            {
                CharacterAttribute->SkillMasterNumber++;
            }
        }
    }
    if (Hero->CurrentSkill >= CharacterAttribute->SkillNumber)
        Hero->CurrentSkill = 0;
    if (CharacterAttribute->SkillNumber == 1)
        Hero->CurrentSkill = 0;
    if (Hero->CurrentSkill >= 0 && CharacterAttribute->Skill[Hero->CurrentSkill] == AT_SKILL_UNDEFINED)
        Hero->CurrentSkill = 0;
    auto Skill = AT_SKILL_UNDEFINED;

    for (int i = 0; i < MAX_SKILLS; i++)
    {
        Skill = CharacterAttribute->Skill[Hero->CurrentSkill];
        if (Skill >= AT_SKILL_STUN && Skill <= AT_SKILL_REMOVAL_BUFF)
            Hero->CurrentSkill++;
        else
            break;
    }

    for (int i = 0; i < MAX_SKILLS; i++)
    {
        Skill = CharacterAttribute->Skill[i];
        if (AT_SKILL_POWER_SLASH_STR == Skill || AT_SKILL_TRIPLE_SHOT_STR == Skill || AT_SKILL_TRIPLE_SHOT_MASTERY == Skill)
        {
            Master_Skill_Bool = i;
        }
        if (AT_SKILL_POWER_SLASH == Skill || Skill == AT_SKILL_TRIPLE_SHOT)
        {
            Skill_Bool = i;
        }
    }

    if (Master_Skill_Bool > -1 && Skill_Bool > -1)
        CharacterAttribute->Skill[Skill_Bool] = AT_SKILL_UNDEFINED;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x11 [ReceiveMagicList]");
}

void Receive_Master_SetSkillList(PMSG_MASTER_SKILL_LIST_SEND* lpMsg)
{
    auto interface = CNewUISystem::GetInstance()->GetUI_NewMasterLevelInterface();
    interface->SetMasterType(Hero->Class);
    interface->InitMasterSkillPoint();

    memset(CharacterAttribute->MasterSkillInfo, 0, sizeof(CharacterAttribute->MasterSkillInfo));

    for (int n = 0; n < lpMsg->count; n++)
    {
        auto lpInfo = (PMSG_MASTER_SKILL_LIST*)(((BYTE*)lpMsg) + sizeof(PMSG_MASTER_SKILL_LIST_SEND) + (sizeof(PMSG_MASTER_SKILL_LIST) * n));
        interface->SetMasterSkillTreeInfo(lpInfo->SkillIndex, lpInfo->SkillLevel, lpInfo->MainValue, lpInfo->NextValue);
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x53 [Receive_Master_SetSkillList]");
}

void ReceiveMuHelperConfigurationData(std::span<const BYTE> ReceiveBuffer)
{
    auto pMuHelperData = safe_cast<PRECEIVE_MUHELPER_DATA>(ReceiveBuffer.subspan(4));
    if (pMuHelperData == nullptr)
    {
        assert(false);
        return;
    }

    MUHelper::ConfigData config;
    MUHelper::ConfigDataSerDe::Deserialize(*pMuHelperData, config);
    g_pNewUIMuHelper->LoadSavedConfig(config);

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0xAE [ReceiveMuHelperConfigurationData]");
}

void ReceiveMuHelperStatusUpdate(std::span<const BYTE> ReceiveBuffer)
{
    auto pMuHelperStatus = safe_cast<PRECEIVE_MUHELPER_STATUS>(ReceiveBuffer.subspan(4));
    if (pMuHelperStatus == nullptr)
    {
        assert(false);
        return;
    }

    if (pMuHelperStatus->Pause)
    {
        MUHelper::g_MuHelper.Stop();
    }
    else
    {
        MUHelper::g_MuHelper.Start();

        if (pMuHelperStatus->Money > 0 && pMuHelperStatus->ConsumeMoney)
        {
            MUHelper:: g_MuHelper.AddCost(pMuHelperStatus->Money);
            int iTotalCost = MUHelper::g_MuHelper.GetTotalCost();

            wchar_t Text[100];
            swprintf(Text, GlobalText[3586], iTotalCost);
            g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
        }
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x51 [ReceiveMuHelperStatusUpdate]");
}

void ReceiveDeleteInventory(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
    if (Data->SubCode != 0xff)
    {
        int itemindex = Data->SubCode;
        if (itemindex >= 0 && itemindex < MAX_EQUIPMENT_INDEX)
        {
            g_pMyInventory->UnequipItem(itemindex);
        }
        else if (itemindex >= MAX_EQUIPMENT_INDEX && itemindex < MAX_MY_INVENTORY_INDEX)
        {
            g_pMyInventory->DeleteItem(itemindex);
        }
        else if (itemindex > MAX_MY_INVENTORY_INDEX && itemindex < MAX_MY_INVENTORY_EX_INDEX)
        {
            g_pMyInventoryExt->DeleteItem(itemindex);
        }
        else if (itemindex >= MAX_MY_INVENTORY_EX_INDEX && itemindex < MAX_MY_SHOP_INVENTORY_INDEX)
        {
            g_pMyShopInventory->DeleteItem(itemindex);
        }
    }

    if (Data->Value)
    {
        EnableUse = 0;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x28 [ReceiveDeleteInventory(%d %d)]", Data->SubCode, Data->Value);
}

int CalcItemLength(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PITEM_EXTENDED_BASE>(ReceiveBuffer);
    int size = 5;
    if (Data->OptionFlags & ItemOptionFlags::HasOption)
    {
        size++;
    }

    if (Data->OptionFlags & ItemOptionFlags::HasExcellent)
    {
        size++;
    }

    if (Data->OptionFlags & ItemOptionFlags::HasAncient)
    {
        size++;
    }

    if (Data->OptionFlags & ItemOptionFlags::HasHarmony)
    {
        size++;
    }

    if (Data->OptionFlags & ItemOptionFlags::HasSockets)
    {
        auto socketCount = ReceiveBuffer[size] & 0xF;
        size++;
        size += socketCount;
    }

    return size;
}

BOOL ReceiveInventoryExtended(std::span<const BYTE> ReceiveBuffer)
{
    for (auto & i : CharacterMachine->Equipment)
    {
        i.Type = -1;
        i.Number = 0;
        i.ExcellentFlags = 0;
    }

    g_pMyInventory->UnequipAllItems();
    g_pMyInventory->DeleteAllItems();
    g_pMyInventoryExt->DeleteAllItems();
    g_pMyShopInventory->DeleteAllItems();

    auto Data = safe_cast<PHEADER_DEFAULT_SUBCODE_WORD>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return false;
    }

    int Offset = sizeof(PHEADER_DEFAULT_SUBCODE_WORD);
    DeleteMount(&Hero->Object);
    giPetManager::DeletePet(Hero);

    ThePetProcess().DeletePet(Hero);

    for (int i = 0; i < Data->Value; i++)
    {
        auto itemStartData = safe_cast<PRECEIVE_INVENTORY_EXTENDED>(ReceiveBuffer.subspan(Offset));
        if (itemStartData == nullptr)
        {
            assert(false);
            return false;
        }

        SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
        int itemindex = itemStartData->Index;
        Offset++;

        auto itemData = ReceiveBuffer.subspan(Offset);
        int length = CalcItemLength(itemData);
        itemData = itemData.subspan(0, length);

        if (itemindex >= 0 && itemindex < MAX_EQUIPMENT_INDEX)
        {
            g_pMyInventory->EquipItem(itemindex, itemData);
        }
        else if (itemindex >= MAX_EQUIPMENT_INDEX && itemindex < MAX_MY_INVENTORY_INDEX)
        {
            g_pMyInventory->InsertItem(itemindex, itemData);
        }
        else if (itemindex >= MAX_MY_INVENTORY_INDEX && itemindex < MAX_MY_INVENTORY_EX_INDEX)
        {
            g_pMyInventoryExt->InsertItem(itemindex, itemData);
        }
        else if (itemindex >= MAX_MY_INVENTORY_EX_INDEX && itemindex < MAX_MY_SHOP_INVENTORY_INDEX)
        {
            g_pMyShopInventory->InsertItem(itemindex, itemData);
        }

        Offset += length;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x10 [ReceiveInventory]");

    return (TRUE);
}



void ReceiveTradeInventoryExtended(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PHEADER_DEFAULT_SUBCODE_WORD>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    int Offset = sizeof(PHEADER_DEFAULT_SUBCODE_WORD);

    if (Data->SubCode == 3)
    {
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
        PlayBuffer(SOUND_MIX01);
        PlayBuffer(SOUND_BREAK01);
        g_pMixInventory->DeleteAllItems();
    }
    else if (Data->SubCode == 5)
    {
        g_pSystemLogBox->AddText(GlobalText[1208], SEASON3B::TYPE_ERROR_MESSAGE);
        PlayBuffer(SOUND_MIX01);
        PlayBuffer(SOUND_BREAK01);
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
        g_pMixInventory->DeleteAllItems();
    }
    else
    {
        for (auto & i : ShopInventory)
        {
            i.Type = -1;
            i.Number = 0;
        }

        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP))
        {
            g_pNPCShop->DeleteAllItems();
        }
    }

    for (int i = 0; i < Data->Value; i++)
    {
        auto itemStartData = safe_cast<PRECEIVE_INVENTORY_EXTENDED>(ReceiveBuffer.subspan(Offset));
        if (itemStartData == nullptr)
        {
            assert(false);
            return;
        }

        int itemindex = itemStartData->Index;
        Offset++;

        auto itemData = ReceiveBuffer.subspan(Offset);
        int length = CalcItemLength(itemData);
        itemData = itemData.subspan(0, length);

        if (Data->SubCode == 3 || Data->SubCode == 5)
        {
            g_pMixInventory->InsertItem(itemindex, itemData);
        }
        else
        {
            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP))
            {
                g_pNPCShop->InsertItem(itemindex, itemData);
            }
            else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
            {
                if (itemindex < MAX_SHOP_INVENTORY)
                {
                    g_pStorageInventory->InsertItem(itemindex, itemData);
                }
                else
                {
                    g_pStorageInventoryExt->InsertItem(itemindex, itemData);
                }
            }
        }

        Offset += length;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x31 [ReceiveTradeInventoryExtended]");
}

void ReceiveChat(const BYTE* ReceiveBuffer)
{
    if (SceneFlag == LOG_IN_SCENE)
    {
        g_ErrorReport.Write(L"Send Request Server List.\r\n");
        SocketClient->ToConnectServer()->SendServerListRequest();
    }
    else
    {
        auto Data = (LPPCHATING)ReceiveBuffer;

        wchar_t ID[MAX_ID_SIZE + 1] {};
        CMultiLanguage::ConvertFromUtf8(ID, Data->ID, MAX_ID_SIZE);
        ID[MAX_ID_SIZE] = L'\0';

        const auto messageSize = Data->Header.Size - MAX_ID_SIZE - sizeof(PBMSG_HEADER);
        wchar_t Text[MAX_CHAT_SIZE + 1] {};
        CMultiLanguage::ConvertFromUtf8(Text, Data->ChatText);
        Text[MAX_CHAT_SIZE] = L'\0';

        if (Text[0] == L'~')
        {
            for (int i = 0; i < messageSize - 1; i++)
                Text[i] = Text[i + 1];
            g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_PARTY_MESSAGE);
        }
        else if (Text[0] == L'@' && Text[1] == L'@')
        {
            for (int i = 0; i < messageSize - 2; i++)
                Text[i] = Text[i + 2];
            g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_UNION_MESSAGE);
        }
        else if (Text[0] == L'@')
        {
            for (int i = 0; i < messageSize - 1; i++)
                Text[i] = Text[i + 1];
            g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_GUILD_MESSAGE);
        }
        else if (Text[0] == L'$')
        {
            for (int i = 0; i < messageSize - 1; i++)
                Text[i] = Text[i + 2];
            g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_GENS_MESSAGE);
        }
        else if (Text[0] == L'#')
        {
            for (int i = 0; i < messageSize - 1; i++)
                Text[i] = Text[i + 1];

            CHARACTER* pFindGm = nullptr;

            for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            {
                CHARACTER* c = &CharactersClient[i];
                OBJECT* o = &c->Object;
                if (o->Live && o->Kind == KIND_PLAYER && (g_isCharacterBuff((&c->Object), eBuff_GMEffect) || (c->CtlCode == CTLCODE_20OPERATOR) || (c->CtlCode == CTLCODE_08OPERATOR)))
                {
                    if (wcscmp(c->ID, ID) == NULL)
                    {
                        pFindGm = c;
                        break;
                    }
                }
            }
            if (pFindGm)
            {
                AssignChat(ID, Text);
                g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_GM_MESSAGE);
            }
            else
            {
                AssignChat(ID, Text, 1);
            }
        }
        else
        {
            CHARACTER* pFindGm = nullptr;
            for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            {
                CHARACTER* c = &CharactersClient[i];
                OBJECT* o = &c->Object;
                if (o->Live && o->Kind == KIND_PLAYER && g_isCharacterBuff((&c->Object), eBuff_GMEffect) || (c->CtlCode == CTLCODE_20OPERATOR) || (c->CtlCode == CTLCODE_08OPERATOR))
                {
                    if (wcscmp(c->ID, ID) == NULL)
                    {
                        pFindGm = c;
                        break;
                    }
                }
            }
            if (pFindGm)
            {
                AssignChat(ID, Text);
                g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_GM_MESSAGE);
            }
            else
            {
                AssignChat(ID, Text);
                g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_CHAT_MESSAGE);
            }
        }
    }
}

void ReceiveChatWhisper(const BYTE* ReceiveBuffer)
{
    if (g_pChatInputBox->IsBlockWhisper() == true)
    {
        return;
    }

    auto Data = (LPPCHATING)ReceiveBuffer;

    wchar_t ID[MAX_ID_SIZE + 1] {};
    CMultiLanguage::ConvertFromUtf8(ID, Data->ID, MAX_ID_SIZE);
    ID[MAX_ID_SIZE] = L'\0';

    const auto messageSize = Data->Header.Size - MAX_ID_SIZE - sizeof(PBMSG_HEADER);
    wchar_t Text[MAX_CHAT_SIZE + 1] {};
    CMultiLanguage::ConvertFromUtf8(Text, Data->ChatText, messageSize);
    Text[messageSize] = L'\0';

    RegistWhisperID(10, ID);

    if (g_pOption->IsWhisperSound())
    {
        PlayBuffer(SOUND_WHISPER);
    }

    g_pChatListBox->AddText(ID, Text, SEASON3B::TYPE_WHISPER_MESSAGE);
}

void ReceiveChatWhisperResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:
    {
        g_pChatListBox->AddText(ChatWhisperID, GlobalText[482], SEASON3B::TYPE_ERROR_MESSAGE, SEASON3B::TYPE_WHISPER_MESSAGE);
    }
    }
}

void ReceiveChatKey(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPCHATING_KEY)ReceiveBuffer;
    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    int Index = FindCharacterIndex(Key);

    if (Hero->GuildStatus == G_MASTER && wcscmp(CharactersClient[Index].ID, L"길드 마스터") == 0)
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCGUILDMASTER);

        GuildInputEnable = true;
        InputEnable = false;
        ClearInput(FALSE);
        InputTextMax[0] = 8;
        InputNumber = 0;

        if (Hero->GuildMarkIndex == -1)
            Hero->GuildMarkIndex = MARK_EDIT;
        return;
    }

    wchar_t ChatText[sizeof Data->ChatText + 1] {};
    CMultiLanguage::ConvertFromUtf8(ChatText, Data->ChatText, sizeof Data->ChatText);
    CreateChat(CharactersClient[Index].ID, ChatText, &CharactersClient[Index]);
}

void ReceiveNotice(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_NOTICE)ReceiveBuffer;
    wchar_t Text[256]{};
    CMultiLanguage::ConvertFromUtf8(Text, Data->Notice);

    if (Data->Result == 0)
    {
        CreateNotice(Text, 0);
    }
    else if (Data->Result == 1)
    {
        if (CHARACTER_SCENE != SceneFlag)
        {
            g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
            EnableUse = 0;
        }
        else
        {
            CUIMng& rUIMng = CUIMng::Instance();
            rUIMng.AddServerMsg(Text);
        }
    }
    else if (Data->Result == 2)
    {
        wchar_t FullText[300] {0};
        swprintf(FullText, GlobalText[483], Text);
        CreateNotice(FullText, 1);
        g_pGuildInfoWindow->AddGuildNotice(Text);
    }
    else if (Data->Result >= 10 && Data->Result <= 15)
    {
        if (Data->Notice != nullptr && Data->Notice[0] != '\0')
        {
            g_pSlideHelpMgr->AddSlide(Data->Count, Data->Delay, Text, Data->Result - 10, Data->Speed / 10.0f, Data->Color);
        }
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x0D [ReceiveNotice(%s)]", Text);
}

void ReceiveMoveCharacter(std::span<const BYTE> ReceiveBuffer)
{
    auto fixedSize = sizeof(PMOVE_CHARACTER);
    auto Data = safe_cast<PMOVE_CHARACTER>(ReceiveBuffer.subspan(0, fixedSize));
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    auto pathNum = Data->PathMetadata & 0x0F;
    if (pathNum > MAX_PATH_FIND)
    {
        pathNum = MAX_PATH_FIND;
    }

    auto requiredSize = fixedSize + (pathNum + 1) / 2;
    if (ReceiveBuffer.size() < requiredSize)
    {
        assert(false);
        return;
    }

    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    CHARACTER* c = &CharactersClient[FindCharacterIndex(Key)];
    OBJECT* o = &c->Object;

    if (c->Dead > 0 || !o->Live)
    {
        return;
    }

    if (IsMonster(c))
    {
        MUHelper::g_MuHelper.AddTarget(Key, false);
    }

    c->PositionX = Data->SourceX;
    c->PositionY = Data->SourceY;
    c->TargetX = Data->TargetX;
    c->TargetY = Data->TargetY;
    c->TargetAngle = Data->PathMetadata >> 4;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"ID : %s | sX : %d | sY : %d | tX : %d | tY : %d", c->ID, c->PositionX, c->PositionY, c->TargetX, c->TargetY);

    if (Key == HeroKey)
    {
        if (!c->Movement)
        {
            c->PositionX = Data->TargetX;
            c->PositionY = Data->TargetY;
        }
        return;
    }

    if (o->Type == MODEL_CRUST)
    {
        c->PositionX = Data->TargetX;
        c->PositionY = Data->TargetY;
        o->Angle[2] = CreateAngle(o->Position[0], o->Position[1], c->PositionX * 100.f, c->PositionY * 100.f);
        SetPlayerWalk(c);
        c->JumpTime = 1;

        return;
    }

    if (c->Movement)
    {
        c->Movement = false;
        SetPlayerStop(c);
    }

    auto pathData = const_cast<BYTE*>(ReceiveBuffer.subspan(fixedSize).data());

    PATH_t* pCharPath = &c->Path;
    pCharPath->Lock.lock();

    if (c->Appear == 0)
    {
        int iDefaultWall = TW_CHARACTER;

        if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4
            && Key != HeroKey)
        {
            iDefaultWall = TW_NOMOVE;
        }

        BYTE direction = c->TargetAngle;
        POINT nextPos = MovePoint(static_cast<EPathDirection>(direction), { Data->SourceX, Data->SourceY });

        pCharPath->PathX[0] = nextPos.x;
        pCharPath->PathY[0] = nextPos.y;

        for (int i = 0; i < pathNum; ++i)
        {
            int byteIndex = i / 2;

            direction = (i % 2 == 0) ? (pathData[byteIndex] >> 4) & 0x0F : (pathData[byteIndex] & 0x0F);

            nextPos = MovePoint(static_cast<EPathDirection>(direction), nextPos);

            pCharPath->PathX[i + 1] = nextPos.x;
            pCharPath->PathY[i + 1] = nextPos.y;
        }

        pCharPath->PathNum = pathNum + 1;
        pCharPath->Direction = direction;
        pCharPath->CurrentPath = 0;
        pCharPath->CurrentPathFloat = 0;
    }

    pCharPath->Lock.unlock();
}

void ReceiveMovePosition(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_MOVE_POSITION)ReceiveBuffer;
    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    CHARACTER* c = &CharactersClient[FindCharacterIndex(Key)];

    OBJECT* o = &c->Object;
    if (o->Type == MODEL_BALL)
    {
        o->Gravity = 20.f;
        o->Direction[1] = (Data->PositionX - (c->PositionX)) * 400.f;
        o->Direction[0] = -(Data->PositionY - (c->PositionY)) * 400.f;
        PlayBuffer(SOUND_MENU01);
        CreateBomb(o->Position, false);
    }

    c->PositionX = Data->PositionX;
    c->PositionY = Data->PositionY;
    c->TargetX = Data->PositionX;
    c->TargetY = Data->PositionY;
    c->JumpTime = 1;
}

extern int EnableEvent;

BOOL ReceiveTeleport(const BYTE* ReceiveBuffer, BOOL bEncrypted)
{
    SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();

    auto Data = (LPPRECEIVE_TELEPORT_POSITION)ReceiveBuffer;
    Hero->PositionX = Data->PositionX;
    Hero->PositionY = Data->PositionY;

    Hero->JumpTime = 0;

    OBJECT* o = &Hero->Object;
    o->Position[0] = ((float)(Hero->PositionX) + 0.5f) * TERRAIN_SCALE;
    o->Position[1] = ((float)(Hero->PositionY) + 0.5f) * TERRAIN_SCALE;

    if (gMapManager.WorldActive == -1 || Hero->Helper.Type != MODEL_HORN_OF_DINORANT || Hero->SafeZone)
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
    }
    else
    {
        if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 90.f;
        else
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 30.f;
    }

    if (gMapManager.WorldActive >= WD_45CURSEDTEMPLE_LV1 && gMapManager.WorldActive <= WD_45CURSEDTEMPLE_LV6)
    {
        if (!(Data->Map >= WD_45CURSEDTEMPLE_LV1 && Data->Map <= WD_45CURSEDTEMPLE_LV6))
        {
            g_CursedTemple->ResetCursedTemple();
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
        }
    }

    int iEtcPart = Hero->EtcPart;

    o->Angle[2] = ((float)(Data->Angle) - 1.f) * 45.f;
    if (Data->Flag == 0)
    {
        CreateTeleportEnd(o);
    }
    else
    {
        ClearItems();
        ClearCharacters(HeroKey);
        RemoveAllShopTitleExceptHero();
        if (gMapManager.WorldActive != Data->Map)
        {
            int OldWorld = gMapManager.WorldActive;

            gMapManager.WorldActive = Data->Map;
            gMapManager.LoadWorld(gMapManager.WorldActive);

            if (gMapManager.WorldActive == WD_34CRYWOLF_1ST)
            {
                SocketClient->ToGameServer()->SendCrywolfInfoRequest();
            }

            if ((gMapManager.InChaosCastle(OldWorld) == true && OldWorld != gMapManager.WorldActive) || gMapManager.InChaosCastle() == true)
            {
                PlayBuffer(SOUND_CHAOS_ENVIR, nullptr, true);

                g_pNewUISystem->Hide(SEASON3B::INTERFACE_FRIEND);

                SetCharacterClass(Hero);
                DeleteMount(&Hero->Object);
            }
            if (gMapManager.InChaosCastle() == false)
            {
                StopBuffer(SOUND_CHAOSCASTLE, true);
                StopBuffer(SOUND_CHAOS_ENVIR, true);
            }

            if (gMapManager.IsEmpireGuardian1() == false &&
                gMapManager.IsEmpireGuardian2() == false &&
                gMapManager.IsEmpireGuardian3() == false &&
                gMapManager.IsEmpireGuardian4() == false)
            {
                StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, true);
                StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, true);
                StopBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, true);
                StopBuffer(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, true);
            }

            matchEvent::CreateEventMatch(gMapManager.WorldActive);

            if (gMapManager.WorldActive == -1 || Hero->Helper.Type != MODEL_HORN_OF_DINORANT || Hero->SafeZone)
            {
                o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
            }
            else
            {
                if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
                    o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 90.f;
                else
                    o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 30.f;
            }

            if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4);
            else
            {
                wchar_t Text[256];
                swprintf(Text, L"%s%s", GlobalText[484], gMapManager.GetMapName(gMapManager.WorldActive));

                g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
        }

        SocketClient->ToGameServer()->SendClientReadyAfterMapChange();

        g_dwLatestZoneMoving = GetTickCount();
        g_bWhileMovingZone = FALSE;

        LoadingWorld = 30;

        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;

        if (gMapManager.WorldActive < WD_65DOPPLEGANGER1 || gMapManager.WorldActive > WD_68DOPPLEGANGER4)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);
        }
        if (gMapManager.WorldActive < WD_69EMPIREGUARDIAN1 || WD_72EMPIREGUARDIAN4 < gMapManager.WorldActive)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
        }

        g_pNewUISystem->HideAll();

        CreatePetDarkSpirit_Now(Hero);
        CreateEffect(BITMAP_MAGIC + 2, o->Position, o->Angle, o->Light, 0, o);

        o->Alpha = 0.f;
        EnableEvent = 0; //USE_EVENT_ELDORADO

        SelectedItem = -1;
        SelectedNpc = -1;
        SelectedCharacter = -1;
        SelectedOperate = -1;
        Attacking = -1;
        RepairEnable = 0;
    }

    MUHelper::g_MuHelper.TriggerStop();

    Hero->Movement = false;
    SetPlayerStop(Hero);

    if (Data->Flag)
        g_pUIMapName->ShowMapName();	// rozy

    CreateMyGensInfluenceGroundEffect();

    if (gMapManager.InBattleCastle() && battleCastle::IsBattleCastleStart())
    {
        Hero->EtcPart = iEtcPart;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x1C [ReceiveTeleport(%d)]", Data->Flag);

    return (TRUE);
}

void ReceiveEquipment(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PRECEIVE_EQUIPMENT_EXTENDED>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    ReadEquipmentExtended(FindCharacterIndex(Key), 0, Data->Equipment);
}

void ReceiveChangePlayer(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PCHANGE_CHARACTER_EXTENDED>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    int Key = Data->Key;

    CHARACTER* c = &CharactersClient[FindCharacterIndex(Key)];
    OBJECT* o = &c->Object;

    int Type = ((Data->ItemGroup & 0xF) * MAX_ITEM_INDEX) | Data->ItemNumber;
    //BYTE Level = Data->Item[1] & 0xf;
    //BYTE Option = Data->Item[3] & 63;
    //BYTE ExtOption = Data->Item[4];

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x25 ReceiveChangePlayer Key(0x%04X) Item Slot(0x%02X) Group(0x%02X) Number(0x%04X) Level(0x%02X)", 
        Data->Key,
        Data->ItemSlot, 
        Data->ItemGroup,
        Data->ItemNumber,
        Data->ItemLevel
    );

    int maxClass = MAX_CLASS;

    switch (Data->ItemSlot)
    {
    case 0:
        if (Data->ItemGroup == 0xFF)
        {
            c->Weapon[0].Type = -1;
            c->Weapon[0].ExcellentFlags = 0;
        }
        else
        {
            c->Weapon[0].Type = MODEL_ITEM + Type;
            c->Weapon[0].Level = Data->ItemLevel;
            c->Weapon[0].ExcellentFlags = Data->ExcellentFlags;
        }
        break;
    case 1:
        if (Data->ItemGroup == 0xFF)
        {
            c->Weapon[1].Type = -1;
            c->Weapon[1].ExcellentFlags = 0;
            DeletePet(c);
        }
        else
        {
            c->Weapon[1].Type = MODEL_ITEM + Type;
            c->Weapon[1].Level = Data->ItemLevel;
            c->Weapon[1].ExcellentFlags = Data->ExcellentFlags;
            CreatePetDarkSpirit_Now(c);
            g_SummonSystem.RemoveEquipEffects(c);
        }
        break;
    case 2:
        if (Data->ItemGroup == 0xFF)
        {
            c->BodyPart[BODYPART_HELM].Type = MODEL_BODY_HELM + c->SkinIndex;
            c->BodyPart[BODYPART_HELM].Level = 0;
            c->BodyPart[BODYPART_HELM].ExcellentFlags = 0;
            c->BodyPart[BODYPART_HELM].AncientDiscriminator = 0;
        }
        else
        {
            c->BodyPart[BODYPART_HELM].Type = MODEL_ITEM + Type;
            c->BodyPart[BODYPART_HELM].Level = Data->ItemLevel;
            c->BodyPart[BODYPART_HELM].ExcellentFlags = Data->ExcellentFlags;
            c->BodyPart[BODYPART_HELM].AncientDiscriminator = Data->AncientDiscriminator;
        }
        break;
    case 3:
        if (Data->ItemGroup == 0xFF)
        {
            c->BodyPart[BODYPART_ARMOR].Type = MODEL_BODY_ARMOR + c->SkinIndex;
            c->BodyPart[BODYPART_ARMOR].Level = 0;
            c->BodyPart[BODYPART_ARMOR].ExcellentFlags = 0;
            c->BodyPart[BODYPART_ARMOR].AncientDiscriminator = 0;
        }
        else
        {
            c->BodyPart[BODYPART_ARMOR].Type = MODEL_ITEM + Type;
            c->BodyPart[BODYPART_ARMOR].Level = Data->ItemLevel;
            c->BodyPart[BODYPART_ARMOR].ExcellentFlags = Data->ExcellentFlags;
            c->BodyPart[BODYPART_ARMOR].AncientDiscriminator = Data->AncientDiscriminator;
        }
        break;
    case 4:
        if (Data->ItemGroup == 0xFF)
        {
            c->BodyPart[BODYPART_PANTS].Type = MODEL_BODY_PANTS + c->SkinIndex;
            c->BodyPart[BODYPART_PANTS].Level = 0;
            c->BodyPart[BODYPART_PANTS].ExcellentFlags = 0;
            c->BodyPart[BODYPART_PANTS].AncientDiscriminator = 0;
        }
        else
        {
            c->BodyPart[BODYPART_PANTS].Type = MODEL_ITEM + Type;
            c->BodyPart[BODYPART_PANTS].Level = Data->ItemLevel;
            c->BodyPart[BODYPART_PANTS].ExcellentFlags = Data->ExcellentFlags;
            c->BodyPart[BODYPART_PANTS].AncientDiscriminator = Data->AncientDiscriminator;
        }
        break;
    case 5:
        if (Data->ItemGroup == 0xFF)
        {
            c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + c->SkinIndex;
            c->BodyPart[BODYPART_GLOVES].Level = 0;
            c->BodyPart[BODYPART_GLOVES].ExcellentFlags = 0;
            c->BodyPart[BODYPART_GLOVES].AncientDiscriminator = 0;
        }
        else
        {
            c->BodyPart[BODYPART_GLOVES].Type = MODEL_ITEM + Type;
            c->BodyPart[BODYPART_GLOVES].Level = Data->ItemLevel;
            c->BodyPart[BODYPART_GLOVES].ExcellentFlags = Data->ExcellentFlags;
            c->BodyPart[BODYPART_GLOVES].AncientDiscriminator = Data->AncientDiscriminator;
        }
        break;
    case 6:
        if (Data->ItemGroup == 0xFF)
        {
            c->BodyPart[BODYPART_BOOTS].Type = MODEL_BODY_BOOTS + c->SkinIndex;
            c->BodyPart[BODYPART_BOOTS].Level = 0;
            c->BodyPart[BODYPART_BOOTS].ExcellentFlags = 0;
            c->BodyPart[BODYPART_BOOTS].AncientDiscriminator = 0;
        }
        else
        {
            c->BodyPart[BODYPART_BOOTS].Type = MODEL_ITEM + Type;
            c->BodyPart[BODYPART_BOOTS].Level = Data->ItemLevel;
            c->BodyPart[BODYPART_BOOTS].ExcellentFlags = Data->ExcellentFlags;
            c->BodyPart[BODYPART_BOOTS].AncientDiscriminator = Data->AncientDiscriminator;
        }
        break;
    case 7:
        if (Data->ItemGroup == 0xFF)
        {
            if (c->Wing.Type == MODEL_WING_OF_RUIN ||
                c->Wing.Type == MODEL_CAPE_OF_LORD ||
                c->Wing.Type == MODEL_WING + 130 ||
                c->Wing.Type == MODEL_CAPE_OF_FIGHTER ||
                c->Wing.Type == MODEL_CAPE_OF_OVERRULE ||
                c->Wing.Type == MODEL_WING + 135 ||
                c->Wing.Type == MODEL_CAPE_OF_EMPEROR)
            {
                DeleteCloth(c, o);
            }
            c->Wing.Type = -1;
        }
        else
        {
            c->Wing.Type = MODEL_ITEM + Type;
            c->Wing.Level = 0;
            if (c->Wing.Type == MODEL_WING_OF_RUIN ||
                c->Wing.Type == MODEL_CAPE_OF_LORD ||
                c->Wing.Type == MODEL_WING + 130 ||
                c->Wing.Type == MODEL_CAPE_OF_FIGHTER ||
                c->Wing.Type == MODEL_CAPE_OF_OVERRULE ||
                c->Wing.Type == MODEL_WING + 135 ||
                c->Wing.Type == MODEL_CAPE_OF_EMPEROR)
            {
                DeleteCloth(c, o);
            }
        }
        break;
    case 8:
        if (Data->ItemGroup == 0xFF)
        {
            c->Helper.Type = -1;
            DeleteMount(o);
            ThePetProcess().DeletePet(c, c->Helper.Type, true);
        }
        else
        {
            c->Helper.Type = MODEL_ITEM + Type;
            c->Helper.Level = 0;
            switch (Type)
            {
            case ITEM_HELPER:CreateMount(MODEL_HELPER, o->Position, o); break;
            case ITEM_HORN_OF_UNIRIA:CreateMount(MODEL_UNICON, o->Position, o); break;
            case ITEM_HORN_OF_DINORANT:CreateMount(MODEL_PEGASUS, o->Position, o); break;
            case ITEM_DARK_HORSE_ITEM:CreateMount(MODEL_DARK_HORSE, o->Position, o); break;
            case ITEM_HORN_OF_FENRIR:
                c->Helper.ExcellentFlags = Data->ExcellentFlags;
                if (Data->ExcellentFlags == 0x01)
                {
                    CreateMount(MODEL_FENRIR_BLACK, o->Position, o);
                }
                else if (Data->ExcellentFlags == 0x02)
                {
                    CreateMount(MODEL_FENRIR_BLUE, o->Position, o);
                }
                else if (Data->ExcellentFlags == 0x04)
                {
                    CreateMount(MODEL_FENRIR_GOLD, o->Position, o);
                }
                else
                {
                    CreateMount(MODEL_FENRIR_RED, o->Position, o);
                }
                break;
            case ITEM_DEMON:
            case ITEM_SPIRIT_OF_GUARDIAN:
            {
                {
                    ThePetProcess().CreatePet(Type, c->Helper.Type, o->Position, c);
                }
            }
            break;
            case ITEM_PET_RUDOLF:ThePetProcess().CreatePet(Type, c->Helper.Type, o->Position, c); break;
            case ITEM_PET_PANDA:ThePetProcess().CreatePet(Type, c->Helper.Type, o->Position, c); break;
            case ITEM_PET_UNICORN:ThePetProcess().CreatePet(Type, c->Helper.Type, o->Position, c); break;
            case ITEM_PET_SKELETON:ThePetProcess().CreatePet(Type, c->Helper.Type, o->Position, c); break;
            }
        }
        break;
    }

    ChangeChaosCastleUnit(c);

    SetCharacterScale(c);
}

void RegisterBuff(eBuffState buff, OBJECT* o, const int bufftime = 0);

void UnRegisterBuff(eBuffState buff, OBJECT* o);

void ReceiveCreatePlayerViewportExtended(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PCREATE_CHARACTER_EXTENDED>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    WORD Key = Data->Key;
    int CreateFlag = (Key >> 15);
    Key &= 0x7FFF;

    

    //if (Index != MAX_CHARACTERS_CLIENT)
    //{
    //auto BackUpGuildMarkIndex = CharactersClient[Index].GuildMarkIndex;
    //auto BackUpGuildStatus = CharactersClient[Index].GuildStatus;
    //auto BackUpGuildType = CharactersClient[Index].GuildType;
    //auto BackUpGuildRelationShip = CharactersClient[Index].GuildRelationShip;
    //auto BackUpGuildMasterKillCount = CharactersClient[Index].GuildMasterKillCount;
    //auto EtcPart = CharactersClient[Index].EtcPart;
    //BYTE BackupCtlcode = 0;
    //if (&CharactersClient[Index] == Hero)
    //{
    //    BackupCtlcode = CharactersClient[Index].CtlCode;
    //}
    //}

    CHARACTER* c = CreateCharacter(Key, MODEL_PLAYER, Data->PositionX, Data->PositionY, 0);
    memset(c->ID, 0, sizeof c->ID);
    CMultiLanguage::ConvertFromUtf8(c->ID, Data->ID, MAX_ID_SIZE);
    OBJECT* o = &c->Object;
    //DeleteCloth(c, o);
    c->Class = gCharacterManager.ChangeServerClassTypeToClientClassType(Data->Class);
    c->SkinIndex = gCharacterManager.GetSkinModelIndex(c->Class);
    c->Skin = 0;
    c->AttackSpeed = Data->AttackSpeed;
    c->MagicSpeed = Data->MagicSpeed;
    c->PK = Data->RotationAndHeroState & 0xf;
    c->Object.Angle[2] = ((float)(Data->RotationAndHeroState >> 4) - 1.f) * 45.f;
    o->Kind = KIND_PLAYER;
    o->Type = MODEL_PLAYER;
    c->PositionX = Data->PositionX;
    c->PositionY = Data->PositionY;
    c->TargetX = Data->TargetX;
    c->TargetY = Data->TargetY;
    switch (Data->Flags & 0x07)
    {
    case 1:
        CreateTeleportEnd(o);
        // AddDebugText(ReceiveBuffer, Size);
        break;
    case 2:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SIT1);
        else
            SetAction(o, PLAYER_SIT_FEMALE1);
        break;
    case 3:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_POSE1);
        else
            SetAction(o, PLAYER_POSE_FEMALE1);
        break;
    case 4:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_HEALING1);
        else
            SetAction(o, PLAYER_HEALING_FEMALE1);
        break;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"(RCPV)ID : %s | sX : %d | sY : %d | tX : %d | tY : %d", c->ID, c->PositionX, c->PositionY, c->TargetX, c->TargetY);

    if (CreateFlag)
    {
        c->Object.Position[0] = ((c->PositionX) + 0.5f) * TERRAIN_SCALE;
        c->Object.Position[1] = ((c->PositionY) + 0.5f) * TERRAIN_SCALE;
        CreateEffect(BITMAP_MAGIC + 2, o->Position, o->Angle, o->Light, 0, o);
        c->Object.Alpha = 0.f;
    }
    else if (PathFinding2(c->PositionX, c->PositionY, Data->TargetX, Data->TargetY, &c->Path))
    {
        c->Movement = true;
    }

    if (gMapManager.InHellas())
    {
        CreateJoint(BITMAP_FLARE + 1, o->Position, o->Position, o->Angle, 8, o, 20.f);
    }

    int Index = FindCharacterIndex(Key);
    ReadEquipmentExtended(Index, Data->Flags, Data->Equipment);

    //if ((Data->Flags & 0x07) == 1)
    //{
    //    // after teleport between servers, restore some previous values.
    //    c->GuildMarkIndex = BackUpGuildMarkIndex;
    //    c->GuildStatus = BackUpGuildStatus;
    //    c->GuildType = BackUpGuildType;
    //    c->GuildRelationShip = BackUpGuildRelationShip;
    //    c->EtcPart = EtcPart;
    //    c->GuildMasterKillCount = BackUpGuildMasterKillCount;

    //    if (&CharactersClient[Index] == Hero)
    //    {
    //        c->CtlCode = BackupCtlcode;
    //    }
    //}

    if (Data->s_BuffCount > 0)
    {
        auto buffs = ReceiveBuffer.subspan(sizeof(PCREATE_CHARACTER_EXTENDED), Data->s_BuffCount);

        for (int j = 0; j < Data->s_BuffCount; ++j)
        {
            auto buff = static_cast<eBuffState>(buffs[j]);
            RegisterBuff(buff, o);
            battleCastle::SettingBattleFormation(c, buff);
            g_ConsoleDebug->Write(MCD_RECEIVE, L"ID : %s, Buff : %d", c->ID, static_cast<int>(buff));
        }

        if (gMapManager.InBattleCastle() && battleCastle::IsBattleCastleStart())
        {
            //g_pSiegeWarfare->InitSkillUI();
        }
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x12 [ReceiveCreatePlayerViewportExtended]");
}

void ReceiveCreateTransformViewport(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PWHEADER_DEFAULT_WORD>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    int Offset = sizeof(PWHEADER_DEFAULT_WORD);

    for (int i = 0; i < Data->Value; i++)
    {
        auto Data2 = safe_cast<PCREATE_TRANSFORM_EXTENDED>(ReceiveBuffer.subspan(Offset));
        if (Data2 == nullptr)
        {
            assert(false);
            return;
        }

        WORD Key = ((WORD)(Data2->KeyH) << 8) + Data2->KeyL;
        int CreateFlag = (Key >> 15);
        Key &= 0x7FFF;

        wchar_t characterName[MAX_ID_SIZE + 1]{};
        CMultiLanguage::ConvertFromUtf8(characterName, Data2->ID, MAX_ID_SIZE);

        CHARACTER* pCha;
        int iIndex = FindCharacterIndex(Key);
        pCha = &CharactersClient[iIndex];

        short sBackUpGuildMarkIndex = -1;
        BYTE byBackUpGuildStatus = 0;
        BYTE byBackUpGuildType = 0;
        BYTE byBackUpGuildRelationShip = 0;
        BYTE byBackUpGuildMasterKillCount = 0;
        BYTE byEtcPart = 0;
        BYTE byBackupCtlcode = 0;

        if (iIndex != MAX_CHARACTERS_CLIENT)
        {
            sBackUpGuildMarkIndex = pCha->GuildMarkIndex;
            byBackUpGuildStatus = pCha->GuildStatus;
            byBackUpGuildType = pCha->GuildType;
            byBackUpGuildRelationShip = pCha->GuildRelationShip;
            byBackUpGuildMasterKillCount = pCha->GuildMasterKillCount;
            byEtcPart = pCha->EtcPart;
            byBackupCtlcode = pCha->CtlCode;
        }

        if (FindText(characterName, L"webzen") == false)
        {
            auto Class = gCharacterManager.ChangeServerClassTypeToClientClassType(Data2->Class);

            auto Type = (EMonsterType)(((WORD)(Data2->TypeH) << 8) + Data2->TypeL);

            CHARACTER* c = CreateMonster(Type, Data2->PositionX, Data2->PositionY, Key);
            OBJECT* o = &c->Object;

            if (c->MonsterIndex == MONSTER_GIANT)
            {
                o->Scale = 0.8f;
            }

            if (Type == MONSTER_JACK_OLANTERN)
            {
                DeleteCloth(c, o);
            }

            if (Type == MONSTER_MU_ALLIES || Type == MONSTER_ILLUSION_SORCERER)
            {
                DeleteCloth(c, o);
            }

            DeleteCloth(c, o);

            c->GuildMarkIndex = sBackUpGuildMarkIndex;
            c->GuildStatus = byBackUpGuildStatus;
            c->GuildType = byBackUpGuildType;
            c->GuildRelationShip = byBackUpGuildRelationShip;
            c->GuildMasterKillCount = byBackUpGuildMasterKillCount;
            c->EtcPart = byEtcPart;
            c->CtlCode = byBackupCtlcode;
            c->Class = Class;
            c->SkinIndex = gCharacterManager.GetSkinModelIndex(c->Class);
            c->PK = Data2->Path & 0xf;
            o->Kind = KIND_PLAYER;
            c->Change = true;

            for (int j = 0; j < Data2->s_BuffCount; ++j)
            {
                RegisterBuff(static_cast<eBuffState>(Data2->s_BuffEffectState[j]), o);
                battleCastle::SettingBattleFormation(c, static_cast<eBuffState>(Data2->s_BuffEffectState[j]));

                g_ConsoleDebug->Write(MCD_RECEIVE, L"ID : %s, Buff : %d", c->ID, static_cast<int>(Data2->s_BuffEffectState[j]));
            }

            c->PositionX = Data2->PositionX;
            c->PositionY = Data2->PositionY;
            c->TargetX = Data2->PositionX;
            c->TargetY = Data2->PositionY;
            c->Object.Angle[2] = ((float)(Data2->Path >> 4) - 1.f) * 45.f;

            if (CreateFlag)
            {
                c->Object.Position[0] = ((c->PositionX) + 0.5f) * TERRAIN_SCALE;
                c->Object.Position[1] = ((c->PositionY) + 0.5f) * TERRAIN_SCALE;
                c->Object.Alpha = 0.f;
                CreateEffect(MODEL_MAGIC_CIRCLE1, o->Position, o->Angle, o->Light, 0, o);
                CreateParticle(BITMAP_LIGHTNING + 1, o->Position, o->Angle, o->Light, 2, 1.f, o);
            }
            else if (PathFinding2((c->PositionX), (c->PositionY), Data2->TargetX, Data2->TargetY, &c->Path))
            {
                c->Movement = true;
            }

            CMultiLanguage::ConvertFromUtf8(c->ID, Data2->ID, MAX_ID_SIZE);

            ChangeCharacterExt(FindCharacterIndex(Key), Data2->Equipment);
        }

        Offset += (sizeof(PCREATE_TRANSFORM_EXTENDED) - (sizeof(BYTE) * (MAX_BUFF_SLOT_INDEX - Data2->s_BuffCount)));
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x45 [ReceiveCreateTransformViewport(%d)]", Data->Value);
}

void AppearMonster(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    switch (c->MonsterIndex)
    {
    case MONSTER_GOLDEN_DRAGON:
        SetAction(o, MONSTER01_STOP2);
        o->PriorAction = MONSTER01_STOP2;
        c->Object.Alpha = 1.f;
        PlayBuffer(SOUND_MONSTER_BULLATTACK1);
        break;
    case MONSTER_ASSASSIN:
        SetAction(o, MONSTER01_STOP2);
        o->PriorAction = MONSTER01_STOP2;
        c->Object.Alpha = 1.f;
        PlayBuffer(SOUND_ASSASSIN);
        break;
    case MONSTER_GOLDEN_TITAN:
    case MONSTER_GOLDEN_SOLDIER:
        c->Appear = 60;
        SetAction(o, MONSTER01_STOP2);
        o->PriorAction = MONSTER01_STOP2;
        c->Object.Alpha = 1.f;
        //PlayBuffer(SOUND_ASSASSIN);
        break;
    case MONSTER_CHIEF_SKELETON_ARCHER_1:
    case MONSTER_CHIEF_SKELETON_ARCHER_2:
    case MONSTER_CHIEF_SKELETON_ARCHER_3:
    case MONSTER_CHIEF_SKELETON_ARCHER_4:
    case MONSTER_CHIEF_SKELETON_ARCHER_5:
    case MONSTER_CHIEF_SKELETON_ARCHER_6:
        PlayBuffer(SOUND_MONSTER_ORCCAPATTACK1);
        break;
    case MONSTER_DARK_ELF:
    case MONSTER_DARKELF:
    case MONSTER_SORAM:
    case MONSTER_BALRAM:
    case MONSTER_DEATH_SPIRIT:
    {
        if (g_Direction.m_CMVP.m_iCryWolfState == CRYWOLF_STATE_READY)
            c->Object.Alpha = 1.0f;
        else
        {
            c->Object.Alpha = 0.f;

            vec3_t vPos;
            Vector(c->Object.Position[0] + 20.0f, c->Object.Position[1] + 20.0f, c->Object.Position[2], vPos);

            CreateJoint(BITMAP_JOINT_THUNDER + 1, vPos, vPos, c->Object.Angle, 7, nullptr, 60.f + rand() % 10);
            CreateJoint(BITMAP_JOINT_THUNDER + 1, vPos, vPos, c->Object.Angle, 7, nullptr, 50.f + rand() % 10);
            CreateJoint(BITMAP_JOINT_THUNDER + 1, vPos, vPos, c->Object.Angle, 7, nullptr, 50.f + rand() % 10);
            CreateJoint(BITMAP_JOINT_THUNDER + 1, vPos, vPos, c->Object.Angle, 7, nullptr, 60.f + rand() % 10);

            CreateParticle(BITMAP_SMOKE + 4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);
            CreateParticle(BITMAP_SMOKE + 4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);
            CreateParticle(BITMAP_SMOKE + 4, c->Object.Position, c->Object.Angle, c->Object.Light, 1, 5.0f);

            Vector(c->Object.Position[0], c->Object.Position[1], c->Object.Position[2] + 120.0f, vPos);
            CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
            CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
            CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
            CreateJoint(BITMAP_JOINT_THUNDER, c->Object.Position, vPos, c->Object.Angle, 17);
        }
    }
    break;
    case MONSTER_MAYA_HAND_LEFT:
    case MONSTER_MAYA_HAND_RIGHT:
    {
        SetAction(o, MONSTER01_APEAR);
        c->Object.Alpha = 0.f;
    }
    break;
    default:
        if (o->Type == MODEL_PLAYER)
        {
            SetAction(o, PLAYER_COME_UP);
            o->PriorAction = PLAYER_COME_UP;
            c->Object.Alpha = 1.f;
        }
        else
        {
            c->Object.Alpha = 0.f;
        }
        break;
    }
}

void ReceiveCreateMonsterViewport(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
    int Offset = sizeof(PWHEADER_DEFAULT_WORD);
    for (int i = 0; i < Data->Value; i++)
    {
        auto Data2 = (LPPCREATE_MONSTER)(ReceiveBuffer + Offset);
        WORD Key = ((WORD)(Data2->KeyH) << 8) + Data2->KeyL;

        BYTE bMyMob = (Data2->TypeH) & 0x80;
        BYTE byBuildTime = (Data2->TypeH & 0x70) >> 4;
        auto Type = (EMonsterType)(((WORD)(Data2->TypeH & 0x03) << 8) + Data2->TypeL);

        if (Type != 106)
            int a = 0;

        int CreateFlag = (Key >> 15);
        int TeleportFlag = (Data2->KeyH & 0x40) >> 6;

        Key &= 0x7FFF;
        CHARACTER* c = CreateMonster(Type, Data2->PositionX, Data2->PositionY, Key);

        g_ConsoleDebug->Write(MCD_RECEIVE, L"0x13 [ReceiveCreateMonsterViewport(Type : %d | Key : %d)]", Type, Key);

        if (c == nullptr) break;

        OBJECT* o = &c->Object;
        if (IsMonster(c))
        {
            MUHelper::g_MuHelper.AddTarget(Key, false);
        }

        for (int j = 0; j < Data2->s_BuffCount; ++j)
        {
            RegisterBuff(static_cast<eBuffState>(Data2->s_BuffEffectState[j]), o);

            g_ConsoleDebug->Write(MCD_RECEIVE, L"ID : %s, Buff : %d", c->ID, static_cast<int>(Data2->s_BuffEffectState[j]));
        }

        float fAngle = 45.0f;
        if (o->Type == MODEL_BALLISTA)
        {
            if (c->PositionY == 90)
                fAngle = 0.0f;
            if (c->PositionX == 62)
                fAngle = -135.0f;
            if (c->PositionX == 183)
                fAngle = 90.0f;
        }
        else if (o->Type == MODEL_KANTURU2ND_ENTER_NPC)
        {
            o->m_fEdgeScale = 1.0f;
            fAngle = 90.f;
        }

        c->Object.Angle[2] = ((float)(Data2->Path >> 4) - 1.f) * fAngle;

        if (o->Type >= MODEL_LITTLESANTA + 1 && o->Type <= MODEL_LITTLESANTA + 4)
        {
            c->Object.Angle[2] = 90.f;
        }
        else if (o->Type >= MODEL_LITTLESANTA + 6 && o->Type <= MODEL_LITTLESANTA + 7)
        {
            c->Object.Angle[2] = 90.f;
        }
        else if (o->Type == MODEL_XMAS2008_SANTA_NPC)
        {
            c->Object.Angle[2] = 20.f;
        }
        else if (o->Type == MODEL_UNITEDMARKETPLACE_JULIA)
        {
            //c->Object.Angle[2] = 44.0f;
            c->Object.Angle[2] = 49.0f;
            //c->Object.Angle[2] = 90.0f;
        }

        c->PositionX = Data2->PositionX;
        c->PositionY = Data2->PositionY;
        c->TargetX = Data2->PositionX;
        c->TargetY = Data2->PositionY;

        c->m_byFriend = bMyMob;
        o->m_byBuildTime = byBuildTime;

        if (gMapManager.InBattleCastle() && c->MonsterIndex == MONSTER_CASTLE_GATE1)
        {
            if (g_isCharacterBuff((&c->Object), eBuff_CastleGateIsOpen))
            {
                SetAction(&c->Object, 1);
                battleCastle::SetCastleGate_Attribute(Data2->PositionX, Data2->PositionY, 0);
            }
            else
            {
                SetAction(&c->Object, 0);
                battleCastle::SetCastleGate_Attribute(Data2->PositionX, Data2->PositionY, 1);
            }
        }
        if (CreateFlag)
        {
            AppearMonster(c);
        }
        else if (gMapManager.WorldActive == WD_39KANTURU_3RD && o->Type == MODEL_DARK_SKULL_SOLDIER_5 && TeleportFlag)
        {
            vec3_t Light;
            Vector(1.0f, 1.0f, 1.0f, Light);
            o->AlphaTarget = 1.f;
            CreateEffect(BITMAP_SPARK + 1, o->Position, o->Angle, Light);
            CreateEffect(BITMAP_SPARK + 1, o->Position, o->Angle, Light);
            PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_TELE);
        }

        int iDefaultWall = TW_CHARACTER;

        if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4
            && Key != HeroKey)
        {
            iDefaultWall = TW_NOMOVE;
        }

        else if (PathFinding2(c->PositionX, c->PositionY, Data2->TargetX, Data2->TargetY, &c->Path, 0.0f, iDefaultWall))
        {
            c->Movement = true;
        }
        else
        {
            c->Movement = false;
        }

        Offset += (sizeof(PCREATE_MONSTER) - (sizeof(BYTE) * (MAX_BUFF_SLOT_INDEX - Data2->s_BuffCount)));
    }
}

void ReceiveCreateSummonViewport(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
    int Offset = sizeof(PWHEADER_DEFAULT_WORD);
    for (int i = 0; i < Data->Value; i++)
    {
        auto Data2 = (LPPCREATE_SUMMON)(ReceiveBuffer + Offset);
        WORD Key = ((WORD)(Data2->KeyH) << 8) + Data2->KeyL;
        auto Type = (EMonsterType)(((WORD)(Data2->TypeH) << 8) + Data2->TypeL);
        int CreateFlag = (Key >> 15);
        Key &= 0x7FFF;

        CHARACTER* c;

        if (Type >= MONSTER_GATE_TO_KALIMA_1 && Type <= MONSTER_GATE_TO_KALIMA_7)
        {
            c = CreateHellGate(Data2->ID, Key, Type, Data2->PositionX, Data2->PositionY, CreateFlag);
        }
        else
        {
            c = CreateMonster(Type, Data2->PositionX, Data2->PositionY, Key);
        }

        if (c == nullptr) break;

        OBJECT* o = &c->Object;

        for (int j = 0; j < Data2->s_BuffCount; ++j)
        {
            RegisterBuff(static_cast<eBuffState>(Data2->s_BuffEffectState[j]), o);
        }

        c->Object.Angle[2] = ((float)(Data2->Path >> 4) - 1.f) * 45.f;
        c->PositionX = Data2->PositionX;
        c->PositionY = Data2->PositionY;
        c->TargetX = Data2->PositionX;
        c->TargetY = Data2->PositionY;
        o->Kind = KIND_PLAYER;
        c->PK = Data2->Path & 0xf;

        if (c->PK >= PVP_MURDERER2)
            c->Level = 1;

        if (Type < 152 || Type>158)
        {
            wchar_t Temp[100] {};
            wcscat(c->ID, GlobalText[485]);
            CMultiLanguage::ConvertFromUtf8(Temp, Data2->ID, MAX_ID_SIZE);
            wcscat(c->ID, Temp);

            CMultiLanguage::ConvertFromUtf8(c->OwnerID, Data2->ID, MAX_ID_SIZE);
            c->OwnerID[MAX_ID_SIZE] = NULL;
        }

        if (CreateFlag)
        {
            AppearMonster(c);
            CreateEffect(MODEL_MAGIC_CIRCLE1, o->Position, o->Angle, o->Light, 0, o);
            CreateParticle(BITMAP_LIGHTNING + 1, o->Position, o->Angle, o->Light, 2, 1.f, o);
            if (Type >= 152 && Type <= 158)
            {
                o->PriorAnimationFrame = 0.f;
                o->AnimationFrame = 0.f;
                PlayBuffer(SOUND_CHAOS_FALLING_STONE);
            }
        }
        else if (PathFinding2(c->PositionX, c->PositionY, Data2->TargetX, Data2->TargetY, &c->Path))
        {
            c->Movement = true;
        }

        Offset += (sizeof(PCREATE_SUMMON) - (sizeof(BYTE) * (MAX_BUFF_SLOT_INDEX - Data2->s_BuffCount)));
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x1F [ReceiveCreateSummonViewport(%d)]", Data->Value);
}

void ReceiveDeleteCharacterViewport(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    int Offset = sizeof(PHEADER_DEFAULT);
    for (int i = 0; i < Data->Value; i++)
    {
        auto Data2 = (LPPDELETE_CHARACTER)(ReceiveBuffer + Offset);

        if (Switch_Info != nullptr)
        {
            WORD Key = ((WORD)(Data2->KeyH) << 8) + Data2->KeyL;
            if (Key == FIRST_CROWN_SWITCH_NUMBER)
            {
                Switch_Info[0].Reset();
            }
            else
            {
                Switch_Info[1].Reset();
            }
        }

        int Key = ((int)(Data2->KeyH) << 8) + Data2->KeyL;
        int DeleteFlag = (Key >> 15);

        Key &= 0x7FFF;

        int iIndex = g_pPurchaseShopInventory->GetShopCharacterIndex();
        if (iIndex >= 0 && iIndex < MAX_CHARACTERS_CLIENT)
        {
            CHARACTER* pCha = &CharactersClient[iIndex];
            if (pCha && pCha->Key == Key)
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
            }
        }

        iIndex = FindCharacterIndex(Key);
        CHARACTER* pCha = &CharactersClient[iIndex];

        int buffSize = g_CharacterBuffSize((&pCha->Object));

        for (int k = 0; k < buffSize; k++)
        {
            UnRegisterBuff(g_CharacterBuff((&pCha->Object), k), &pCha->Object);
        }

        DeleteCharacter(Key);
        CHARACTER* pPlayer = FindCharacterTagShopTitle(Key);
        if (pPlayer)
        {
            RemoveShopTitle(pPlayer);
        }

        Offset += sizeof(PDELETE_CHARACTER);
    }
}
int AttackPlayer = 0;

void ReceiveDamage(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_DAMAGE)ReceiveBuffer;
    int Damage = ((int)(Data->DamageH) << 8) + Data->DamageL;
    if (CharacterAttribute->Life >= Damage)
        CharacterAttribute->Life -= Damage;
    else
        CharacterAttribute->Life = 0;

    int ShieldDamage = ((int)(Data->ShieldDamageH) << 8) + Data->ShieldDamageL;
    if (CharacterAttribute->Shield >= ShieldDamage)
        CharacterAttribute->Shield = ShieldDamage;
    else
        CharacterAttribute->Shield = 0;
}

void ReceiveAttackDamageCastle(CHARACTER* c, OBJECT* o, const bool success, const int key, const int damage, const int shieldDamage, const int damageType, const bool bRepeatedly, const bool bEndRepeatedly, const bool bDoubleEnable, const bool bComboEnable)
{
    vec3_t Light;
    int accumDamage = shieldDamage + damage;
    int	rstDamage = -1;
    float rstScale = 0.8f;
    Vector(0.5f, 0.5f, 0.5f, Light);
    if (accumDamage > 0)
    {
        rstDamage = -2;
        if (accumDamage > 0 && accumDamage < 1000)
        {
            Vector(1.f, 0.f, 0.f, Light);
        }
        else if (accumDamage >= 1000 && accumDamage < 3000)
        {
            Vector(0.95f, 0.7f, 0.15f, Light);
        }
        else if (accumDamage >= 3000)
        {
            Vector(0.95f, 0.7f, 0.15f, Light);
            rstScale = 1.5f;
        }
    }
    else
    {
        if (key == HeroKey)
        {
            Vector(1.f, 1.f, 1.f, Light);
        }
        else
        {
            Vector(0.5f, 0.5f, 0.5f, Light);
        }
    }

    if (success)
    {
        SetPlayerShock(c, damage);
        Vector(1.f, 0.f, 0.f, Light);
        if (key == HeroKey)
        {
            if (damage >= CharacterAttribute->Life)
                CharacterAttribute->Life = 0;
            else
                CharacterAttribute->Life -= damage;

            if (shieldDamage >= CharacterAttribute->Shield)
                CharacterAttribute->Shield = 0;
            else
                CharacterAttribute->Shield -= shieldDamage;
        }

        CreatePoint(o->Position, rstDamage, Light, rstScale);
    }
    else
    {
        if (key == HeroKey)
        {
            if (damage >= CharacterAttribute->Life)
                CharacterAttribute->Life = 0;
            else
                CharacterAttribute->Life -= damage;

            if (shieldDamage >= CharacterAttribute->Shield)
                CharacterAttribute->Shield = 0;
            else
                CharacterAttribute->Shield -= shieldDamage;

            if (g_isCharacterBuff(o, eBuff_WizDefense) && o->Type == MODEL_PLAYER)
            {
                CHARACTER* cm = &CharactersClient[AttackPlayer];
                OBJECT* om = &cm->Object;
                float fAngle = CreateAngle2D(om->Position, o->Position);
                if (fabs(fAngle - om->Angle[2]) < 10.f)
                {
                    vec3_t Angle = { 0.0f, 0.0f, fAngle + 180.f };
                    CreateEffect(MODEL_MAGIC_CAPSULE2, o->Position, Angle, o->Light, 0, o);
                }
            }
#ifdef _PVP_ADD_MOVE_SCROLL
            if (Damage > 0)
                g_MurdererMove.CancelMove();
#endif	// _PVP_ADD_MOVE_SCROLL
        }
        else
        {
            if (c->MonsterIndex != MONSTER_ILLUSION_OF_KUNDUN_7
                && rand_fps_check(2))
            {
                SetPlayerShock(c, damage);
            }
        }

        if (damageType == 4)
        {
            Vector(1.f, 0.f, 1.f, Light);
        }

        CreatePoint(o->Position, rstDamage, Light, rstScale);
    }

    c->Hit = damage;
}

void ReceiveAttackDamage(CHARACTER* c, OBJECT* o, const bool success, const int key, const int damage, const int shieldDamage, const int damageType, const bool bRepeatedly, const bool bEndRepeatedly, const bool bDoubleEnable, const bool bComboEnable)
{
    vec3_t Light;
    if (success)
    {
        SetPlayerShock(c, damage);
        Vector(1.f, 0.f, 0.f, Light);

        CreatePoint(o->Position, damage, Light);

        if (shieldDamage > 0)
        {
            vec3_t nPosShieldDamage;
            Vector(0.8f, 1.f, 0.f, Light);
            nPosShieldDamage[0] = o->Position[0]; nPosShieldDamage[1] = o->Position[1];
            nPosShieldDamage[2] = o->Position[2] + 25.f;
            CreatePoint(nPosShieldDamage, shieldDamage, Light);
        }

        if (key == HeroKey)
        {
            if (damage >= CharacterAttribute->Life)
                CharacterAttribute->Life = 0;
            else
                CharacterAttribute->Life -= damage;

            if (shieldDamage >= CharacterAttribute->Shield)
                CharacterAttribute->Shield = 0;
            else
                CharacterAttribute->Shield -= shieldDamage;
        }
    }
    else
    {
        if (key == HeroKey)
        {
            if (damage >= CharacterAttribute->Life)
                CharacterAttribute->Life = 0;
            else
                CharacterAttribute->Life -= damage;

            if (shieldDamage >= CharacterAttribute->Shield)
                CharacterAttribute->Shield = 0;
            else
                CharacterAttribute->Shield -= shieldDamage;

            if (g_isCharacterBuff(o, eBuff_WizDefense) && o->Type == MODEL_PLAYER)
            {
                CHARACTER* cm = &CharactersClient[AttackPlayer];
                OBJECT* om = &cm->Object;
                float fAngle = CreateAngle2D(om->Position, o->Position);
                if (fabs(fAngle - om->Angle[2]) < 10.f)
                {
                    vec3_t Angle = { 0.0f, 0.0f, fAngle + 180.f };
                    CreateEffect(MODEL_MAGIC_CAPSULE2, o->Position, Angle, o->Light, 0, o);
                }
            }
        }
        else
        {
            if (c->MonsterIndex != MONSTER_ILLUSION_OF_KUNDUN_7
                && rand_fps_check(2))
            {
                SetPlayerShock(c, damage);
            }
        }
        float scale = 15.f;
        if (damage == 0)
        {
            if (key == HeroKey)
            {
                Vector(1.f, 1.f, 1.f, Light);
            }
            else
            {
                Vector(0.5f, 0.5f, 0.5f, Light);
            }
        }
        else
        {
            switch (damageType)
            {
            case 0:	//	DT_NONE
                if (key == HeroKey)
                {
                    Vector(1.f, 0.f, 0.f, Light);
                }
                else
                {
                    Vector(1.f, 0.6f, 0.f, Light);
                }
                break;
            case 1:	//	DT_PERFECT
                scale = 50.f;
                Vector(0.0f, 1.f, 1.f, Light);
                break;
            case 2:	//	DT_EXCELLENT
                scale = 50.f;
                Vector(0.f, 1.f, 0.6f, Light);
                break;
            case 3:	//	DT_CRITICAL
                scale = 50.f;
                Vector(0.f, 0.6f, 1.f, Light);
                break;
            case 4:	//	DT_MIRROR
                Vector(1.f, 0.f, 1.f, Light);
                break;
            case 5: //	DT_POSION
                Vector(0.f, 1.f, 0.f, Light);
                break;
            case 6:	//	DT_DOT
                Vector(0.7f, 0.4f, 1.0f, Light);
                break;
            default:
                Vector(1.f, 1.f, 1.f, Light);
                break;
            }

            if (bComboEnable)
            {
                scale = 50.f;
            }
        }

        if (bRepeatedly || bEndRepeatedly)
        {
            g_CMonkSystem.SetRepeatedly(damage, damageType, bDoubleEnable, bEndRepeatedly);
            if (bEndRepeatedly)
            {
                g_CMonkSystem.RenderRepeatedly(key, o);
            }
        }
        else if (damage == 0)
            CreatePoint(o->Position, -1, Light);
        else
        {
            if (bComboEnable)
            {
                vec3_t Position, Light2;
                VectorCopy(o->Position, Position);
                Vector(Light[0] - 0.4f, Light[1] - 0.4f, Light[2] - 0.4f, Light2);
                CreatePoint(Position, damage, Light2, scale);
                Position[2] += 10.f;
                Vector(Light[0] - 0.2f, Light[1] - 0.2f, Light[2] - 0.2f, Light2);
                CreatePoint(Position, damage, Light2, scale + 5.f);
                Position[2] += 10.f;
                CreatePoint(Position, damage, Light, scale + 10.f);
            }
            else if (bDoubleEnable)    //  Double Damage
            {
                vec3_t Position, Light2;
                VectorCopy(o->Position, Position);
                Vector(Light[0] - 0.4f, Light[1] - 0.4f, Light[2] - 0.4f, Light2);
                CreatePoint(Position, damage, Light2, scale);
                Position[2] += 10.f;
                Vector(Light[0] - 0.2f, Light[1] - 0.2f, Light[2] - 0.2f, Light2);
                CreatePoint(Position, damage, Light2, scale + 5.f);
            }

            CreatePoint(o->Position, damage, Light, scale);
        }

        if (shieldDamage > 0)
        {
            vec3_t nPosShieldDamage;
            Vector(0.8f, 1.f, 0.f, Light);
            nPosShieldDamage[0] = o->Position[0]; nPosShieldDamage[1] = o->Position[1];
            nPosShieldDamage[2] = o->Position[2] + 25.f;
            CreatePoint(nPosShieldDamage, shieldDamage, Light);
        }
    }
    c->Hit = damage;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x15 [ReceiveAttackDamage(%d %d)]", AttackPlayer, damage);
}

void ReceiveAttackDamageExtended(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_ATTACK_EXTENDED)ReceiveBuffer;

    int Key = Data->TargetId;
    int Success = (Key >> 15);
    Key &= 0x7FFF;

    int Index = FindCharacterIndex(Key);
    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;
    
    auto Damage = Data->HealthDamage;
    // DamageType
    int	 DamageType = (Data->DamageType) & 0x0f;
    bool bRepeatedly = (Data->DamageType >> 4) & 0x01;
    bool bEndRepeatedly = (Data->DamageType >> 5) & 0x01;
    bool bDoubleEnable = (Data->DamageType >> 6) & 0x01;
    bool bComboEnable = (Data->DamageType >> 7) & 0x01;
    auto ShieldDamage = Data->ShieldDamage;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x15 [ReceiveAttackDamageExtended(%d %d)]", AttackPlayer, Damage);
    if (IsMonster(c) || IsPlayer(c))
    {
        MUHelper::g_MuHelper.AddTarget(Key, true);
    }

    if (Data->HealthStatus == 0xFF)
    {
        c->HealthStatus = -1;
    }
    else if (Data->HealthStatus == 0)
    {
        c->HealthStatus = 0;
    }
    else
    {
        c->HealthStatus = static_cast<float>(Data->HealthStatus) / 250.f;
    }

    if (Data->ShieldStatus == 0xFF)
    {
        c->ShieldStatus = -1;
    }
    else if (Data->ShieldStatus == 0)
    {
        c->ShieldStatus = 0;
    }
    else
    {
        c->ShieldStatus = static_cast<float>(Data->HealthStatus) / 250.f;
    }

    if (gMapManager.InChaosCastle())
    {
        ReceiveAttackDamageCastle(c, o, Success, Key, Damage, ShieldDamage, DamageType, bRepeatedly, bEndRepeatedly, bDoubleEnable, bComboEnable);
        
    }
    else
    {
        ReceiveAttackDamage(c, o, Success, Key, Damage, ShieldDamage, DamageType, bRepeatedly, bEndRepeatedly, bDoubleEnable, bComboEnable);
    }
}

void ReceiveAction(const BYTE* ReceiveBuffer, int Size)
{
    auto Data = (LPPRECEIVE_ACTION)ReceiveBuffer;
    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;

    int Index = FindCharacterIndex(Key);
    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;

    int iTargetKey, iTargetIndex;

    iTargetKey = ((int)(Data->TargetKeyH) << 8) + Data->TargetKeyL;
    iTargetIndex = FindCharacterIndex(iTargetKey);

    if (!c->SafeZone && c->Helper.Type == MODEL_DARK_HORSE_ITEM)
    {
        // on a Dark Horse, don't show
        
        return;
    }
    //if(c->Helper.Type == MODEL_HELPER+37) return;

    c->Object.Angle[2] = ((float)(Data->Angle) - 1.f) * 45.f;
    c->Movement = false;

    c->Object.Position[0] = c->TargetX * TERRAIN_SCALE + TERRAIN_SCALE * 0.5f;
    c->Object.Position[1] = c->TargetY * TERRAIN_SCALE + TERRAIN_SCALE * 0.5f;
    switch (Data->Action)
    {
    case AT_STAND1:
    case AT_STAND2:
        SetPlayerStop(c);
        break;
    case AT_ATTACK1:
    case AT_ATTACK2:
        SetPlayerAttack(c);
        c->AttackTime = 1;
        c->Object.AnimationFrame = 0;

        c->TargetCharacter = HeroIndex;
        if (IsMonster(c) || IsPlayer(c))
        {
            MUHelper::g_MuHelper.AddTarget(Key, true);
        }

        AttackPlayer = Index;
        break;
    case AT_SKILL_BLOCKING:
        SetAction(o, PLAYER_DEFENSE1);
        PlayBuffer(SOUND_SKILL_DEFENSE);
        break;
    case AT_SIT1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_SIT1);
        else
            SetAction(&c->Object, PLAYER_SIT_FEMALE1);
        break;
    case AT_POSE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_POSE1);
        else
            SetAction(&c->Object, PLAYER_POSE_FEMALE1);
        break;
    case AT_HEALING1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(&c->Object, PLAYER_HEALING1);
        else
            SetAction(&c->Object, PLAYER_HEALING_FEMALE1);
        break;
    case AT_GREETING1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GREETING1);
        else
            SetAction(o, PLAYER_GREETING_FEMALE1);
        break;
    case AT_GOODBYE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GOODBYE1);
        else
            SetAction(o, PLAYER_GOODBYE_FEMALE1);
        break;
    case AT_CLAP1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CLAP1);
        else
            SetAction(o, PLAYER_CLAP_FEMALE1);
        break;
    case AT_GESTURE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_GESTURE1);
        else
            SetAction(o, PLAYER_GESTURE_FEMALE1);
        break;
    case AT_DIRECTION1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_DIRECTION1);
        else
            SetAction(o, PLAYER_DIRECTION_FEMALE1);
        break;
    case AT_UNKNOWN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_UNKNOWN1);
        else
            SetAction(o, PLAYER_UNKNOWN_FEMALE1);
        break;
    case AT_CRY1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CRY1);
        else
            SetAction(o, PLAYER_CRY_FEMALE1);
        break;
    case AT_AWKWARD1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_AWKWARD1);
        else
            SetAction(o, PLAYER_AWKWARD_FEMALE1);
        break;
    case AT_SEE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SEE1);
        else
            SetAction(o, PLAYER_SEE_FEMALE1);
        break;
    case AT_CHEER1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_CHEER1);
        else
            SetAction(o, PLAYER_CHEER_FEMALE1);
        break;
    case AT_WIN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_WIN1);
        else
            SetAction(o, PLAYER_WIN_FEMALE1);
        break;
    case AT_SMILE1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SMILE1);
        else
            SetAction(o, PLAYER_SMILE_FEMALE1);
        break;
    case AT_SLEEP1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_SLEEP1);
        else
            SetAction(o, PLAYER_SLEEP_FEMALE1);
        break;
    case AT_COLD1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_COLD1);
        else
            SetAction(o, PLAYER_COLD_FEMALE1);
        break;
    case AT_AGAIN1:
        if (!gCharacterManager.IsFemale(c->Class))
            SetAction(o, PLAYER_AGAIN1);
        else
            SetAction(o, PLAYER_AGAIN_FEMALE1);
        break;
    case AT_RESPECT1:
        SetAction(o, PLAYER_RESPECT1);
        break;
    case AT_SALUTE1:
        SetAction(o, PLAYER_SALUTE1);
        break;
    case AT_RUSH1:
        SetAction(o, PLAYER_RUSH1);
        break;
    case AT_SCISSORS:
        SetAction(o, PLAYER_SCISSORS);
        break;
    case AT_ROCK:
        SetAction(o, PLAYER_ROCK);
        break;
    case AT_PAPER:
        SetAction(o, PLAYER_PAPER);
        break;
    case AT_HUSTLE:
        SetAction(o, PLAYER_HUSTLE);
        break;
    case AT_DAMAGE1:
        SetAction(&c->Object, MONSTER01_SHOCK);
        break;
    case AT_PROVOCATION:
        SetAction(o, PLAYER_PROVOCATION);
        break;
    case AT_LOOK_AROUND:
        SetAction(o, PLAYER_LOOK_AROUND);
        break;
    case AT_CHEERS:
        SetAction(o, PLAYER_CHEERS);
        break;
    case AT_JACK1:
        SetAction(o, PLAYER_JACK_1);
        o->m_iAnimation = 0;
        break;
    case AT_JACK2:
        SetAction(o, PLAYER_JACK_2);
        o->m_iAnimation = 0;
        break;
    case AT_SANTA1_1:
    case AT_SANTA1_2:
    case AT_SANTA1_3:
    {
        SetAction(o, PLAYER_SANTA_1);
        int i = Data->Action % AT_SANTA1_1;
        g_XmasEvent->CreateXmasEventEffect(c, o, i);
        o->m_iAnimation = 0;
        PlayBuffer(static_cast<ESound>(SOUND_XMAS_JUMP_SANTA + i));
    }
    break;
    case AT_SANTA2_1:
    case AT_SANTA2_2:
    case AT_SANTA2_3:
    {
        SetAction(o, PLAYER_SANTA_2);
        int i = Data->Action % AT_SANTA2_1;
        g_XmasEvent->CreateXmasEventEffect(c, o, i);
        o->m_iAnimation = 0;
        PlayBuffer(SOUND_XMAS_TURN);
    }
    break;
    case AT_RAGEBUFF_1:
    {
        SetAction(o, PLAYER_SKILL_ATT_UP_OURFORCES);
        PlayBuffer(SOUND_RAGESKILL_BUFF_1);
    }
    break;
    case AT_RAGEBUFF_2:
    {
        SetAction(o, PLAYER_SKILL_HP_UP_OURFORCES);
        PlayBuffer(SOUND_RAGESKILL_BUFF_2);
    }
    break;
    default:
        SetAction(&c->Object, Data->Action);
        break;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x18 [ReceiveAction(%d)]", Data->Angle);
}

void ReceiveSkillStatus(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_VIEWSKILLSTATE)ReceiveBuffer;
    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    CHARACTER* c = &CharactersClient[FindCharacterIndex(Key)];
    OBJECT* o = &c->Object;

    if (Data->State == 1) // add
    {
        auto bufftype = static_cast<eBuffState>(Data->BuffIndex);

        if (bufftype == eBuffNone || bufftype >= eBuff_Count) return;

        if (g_isCharacterBuff(o, bufftype))
        {
            if ((o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
                || MODEL_SMITH || MODEL_NPC_SERBIS || MODEL_MERCHANT_MAN
                || MODEL_STORAGE || MODEL_NPC_BREEDER)
            {
                if (g_isCharacterBuff(o, eBuff_CrywolfHeroContracted))
                {
                    g_TokenCharacterBuff(o, eBuff_CrywolfHeroContracted);
                }
                else
                {
                    g_TokenCharacterBuff(o, eBuff_CrywolfHeroContracted);
                    g_CharacterUnRegisterBuff(o, eBuff_CrywolfHeroContracted);
                }
            }
        }
        else
        {
            RegisterBuff(bufftype, o);
            if (bufftype == eBuff_CastleRegimentDefense || bufftype == eBuff_CastleRegimentAttack1
                || bufftype == eBuff_CastleRegimentAttack2 || bufftype == eBuff_CastleRegimentAttack3)
            {
                battleCastle::SettingBattleFormation(c, bufftype);
            }
            else if (bufftype == eBuff_GMEffect)
            {
                if (c->m_pParts != nullptr)
                {
                    DeleteParts(c);
                }
                c->EtcPart = PARTS_WEBZEN;
            }

            g_ConsoleDebug->Write(MCD_RECEIVE, L"RegisterBuff ID : %s, Buff : %d", c->ID, static_cast<int>(bufftype));
        }
    }
    else // clear
    {
        auto bufftype = static_cast<eBuffState>(Data->BuffIndex);

        if (bufftype == eBuffNone || bufftype >= eBuff_Count) return;

        UnRegisterBuff(bufftype, o);
        if (bufftype == eBuff_CastleRegimentDefense || bufftype == eBuff_CastleRegimentAttack1
            || bufftype == eBuff_CastleRegimentAttack2 || bufftype == eBuff_CastleRegimentAttack3)
        {
            battleCastle::DeleteBattleFormation(c, bufftype);

            g_ConsoleDebug->Write(MCD_RECEIVE, L"UnRegisterBuff ID : %s, Buff : %d", c->ID, static_cast<int>(bufftype));
        }
        else if (bufftype == eBuff_GMEffect)
        {
            DeleteParts(c);
        }
    }
}

void ReceiveMagicFinish(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_VALUE_KEY)ReceiveBuffer;
    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    OBJECT* o = &CharactersClient[FindCharacterIndex(Key)].Object;

    switch ((ActionSkillType)Data->Value) // todo: is this correct? Data->Value is a byte, but ActionSkillType is an int
    {
    case AT_SKILL_POISON:
    case AT_SKILL_POISON_STR:
        UnRegisterBuff(eDeBuff_Poison, o);
        break;
    case AT_SKILL_ICE:
    case AT_SKILL_ICE_STR:
    case AT_SKILL_ICE_STR_MG:
        UnRegisterBuff(eDeBuff_Freeze, o);
        break;
    case AT_SKILL_STRIKE_OF_DESTRUCTION:
    case AT_SKILL_STRIKE_OF_DESTRUCTION_STR:
        UnRegisterBuff(eDeBuff_BlowOfDestruction, o);
        break;
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
        UnRegisterBuff(eBuff_Attack, o);
        break;
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
        UnRegisterBuff(eBuff_Defense, o);
        break;
    case AT_SKILL_STUN:
        UnRegisterBuff(eDeBuff_Stun, o);
        break;
    case AT_SKILL_INVISIBLE:
        UnRegisterBuff(eBuff_Cloaking, o);
        break;
    case AT_SKILL_MANA:
        UnRegisterBuff(eBuff_AddMana, o);
        break;
    case AT_SKILL_BRAND_OF_SKILL:
        UnRegisterBuff(eBuff_AddSkill, o);
        break;
    case AT_SKILL_IMPROVE_AG:
        UnRegisterBuff(eBuff_AddAG, o);
        break;
    case AT_SKILL_ADD_CRITICAL:
    case AT_SKILL_ADD_CRITICAL_STR1:
    case AT_SKILL_ADD_CRITICAL_STR2:
    case AT_SKILL_ADD_CRITICAL_STR3:
        UnRegisterBuff(eBuff_AddCriticalDamage, o);
        break;
    case AT_SKILL_SWELL_LIFE:
    case AT_SKILL_SWELL_LIFE_STR:
    case AT_SKILL_SWELL_LIFE_PROFICIENCY:
        UnRegisterBuff(eBuff_Life, o);
        break;
    case AT_SKILL_ICE_ARROW:
    case AT_SKILL_ICE_ARROW_STR:
        UnRegisterBuff(eDeBuff_Harden, o);
        break;
    case AT_SKILL_FIRE_SLASH:
    case AT_SKILL_FIRE_SLASH_STR:
        UnRegisterBuff(eDeBuff_Defense, o);
        break;
    case AT_SKILL_SOUL_BARRIER:
    case AT_SKILL_SOUL_BARRIER_STR:
    case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
        UnRegisterBuff(eBuff_WizDefense, o);
        break;
    case AT_SKILL_DECAY:
    case AT_SKILL_DECAY_STR:
        UnRegisterBuff(eDeBuff_Poison, o);
        break;
    case AT_SKILL_ICE_STORM:
        UnRegisterBuff(eDeBuff_Freeze, o);
        break;
        //  몬스터.
    case AT_SKILL_MONSTER_MAGIC_DEF:
        SetActionDestroy_Def(o);
        UnRegisterBuff(eBuff_Defense, o);
        break;
    case AT_SKILL_MONSTER_PHY_DEF:
        SetActionDestroy_Def(o);
        UnRegisterBuff(eBuff_WizDefense, o);
        break;
    case AT_SKILL_ATT_UP_OURFORCES:
        UnRegisterBuff(eBuff_Att_up_Ourforces, o);
        break;
    case AT_SKILL_HP_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES_STR:
        UnRegisterBuff(eBuff_Hp_up_Ourforces, o);
        break;
    case AT_SKILL_DEF_UP_OURFORCES:
    case AT_SKILL_DEF_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
        UnRegisterBuff(eBuff_Def_up_Ourforces, o);
        break;
    case AT_SKILL_ALICE_THORNS:
        UnRegisterBuff(eBuff_Thorns, o);
        break;
    case AT_SKILL_ALICE_BERSERKER:
    case AT_SKILL_ALICE_BERSERKER_STR:
        UnRegisterBuff(eBuff_Berserker, o);
        break;
    }
}

void SetPlayerBow(CHARACTER* c)
{
    OBJECT* o = &c->Object;

    if (o->Type != MODEL_PLAYER || gCharacterManager.GetBaseClass(c->Class) != CLASS_ELF || c->SafeZone)
        return;

    SetAttackSpeed();

    switch (gCharacterManager.GetEquipedBowType(c))
    {
    case BOWTYPE_BOW:
    {
        if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
        {
            SetAction(&c->Object, PLAYER_FENRIR_ATTACK_BOW);
        }
        else if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA) || (c->Helper.Type == MODEL_HORN_OF_DINORANT))
        {
            SetAction(&c->Object, PLAYER_ATTACK_RIDE_BOW);
        }
        else if (c->Wing.Type != -1)
        {
            SetAction(&c->Object, PLAYER_ATTACK_FLY_BOW);
        }
        else
        {
            SetAction(&c->Object, PLAYER_ATTACK_BOW);
        }
    }break;
    case BOWTYPE_CROSSBOW:
    {
        if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
        {
            SetAction(&c->Object, PLAYER_FENRIR_ATTACK_CROSSBOW);
        }
        else if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA) || (c->Helper.Type == MODEL_HORN_OF_DINORANT))
        {
            SetAction(&c->Object, PLAYER_ATTACK_RIDE_CROSSBOW);
        }
        else if (c->Wing.Type != -1)
        {
            SetAction(&c->Object, PLAYER_ATTACK_FLY_CROSSBOW);
        }
        else
        {
            SetAction(&c->Object, PLAYER_ATTACK_CROSSBOW);
        }
    }break;
    }
}

void SetPlayerHighBow(CHARACTER* c)
{
    switch (gCharacterManager.GetEquipedBowType(c))
    {
    case BOWTYPE_BOW:
    {
        if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
        {
            SetAction(&c->Object, PLAYER_ATTACK_RIDE_BOW_UP);
        }
        else if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA) || (c->Helper.Type == MODEL_HORN_OF_DINORANT))
        {
            SetAction(&c->Object, PLAYER_ATTACK_RIDE_BOW_UP);
        }
        else if (c->Wing.Type != -1)
        {
            SetAction(&c->Object, PLAYER_ATTACK_FLY_BOW_UP);
        }
        else
        {
            SetAction(&c->Object, PLAYER_ATTACK_BOW_UP);
        }
    }break;
    case BOWTYPE_CROSSBOW:
    {
        if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
        {
            SetAction(&c->Object, PLAYER_ATTACK_RIDE_CROSSBOW_UP);
        }
        else if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA) || (c->Helper.Type == MODEL_HORN_OF_DINORANT))
        {
            SetAction(&c->Object, PLAYER_ATTACK_RIDE_CROSSBOW_UP);
        }
        else if (c->Wing.Type != -1)
        {
            SetAction(&c->Object, PLAYER_ATTACK_FLY_CROSSBOW_UP);
        }
        else
        {
            SetAction(&c->Object, PLAYER_ATTACK_CROSSBOW_UP);
        }
    }break;
    }
}

BOOL ReceiveMonsterSkill(const BYTE* ReceiveBuffer, int Size, BOOL bEncrypted)
{
    auto Data = (LPPRECEIVE_MONSTERSKILL)ReceiveBuffer;

    int SourceKey = Data->SourceKey;
    int TargetKey = Data->TargetKey;
    int Success = (TargetKey >> 15);

    WORD SkillNumber = ((WORD)(Data->MagicH) << 8) + Data->MagicL;

    int Index = FindCharacterIndex(SourceKey);
    int TargetIndex = FindCharacterIndex(TargetKey);
    if (TargetIndex == MAX_CHARACTERS_CLIENT)
        return (TRUE);
    AttackPlayer = Index;

    CHARACTER* sc = &CharactersClient[Index];
    CHARACTER* tc = &CharactersClient[TargetIndex];
    OBJECT* so = &sc->Object;
    OBJECT* to = &tc->Object;

    sc->MonsterSkill = SkillNumber;

    sc->SkillSuccess = (Success != 0);
    sc->TargetCharacter = TargetIndex;

    if (SkillNumber > ATMON_SKILL_BIGIN && SkillNumber < ATMON_SKILL_END)
    {
        sc->Object.Angle[2] = CreateAngle2D(so->Position, to->Position);
        SetPlayerAttack(sc);
        so->AnimationFrame = 0;
        sc->AttackTime = 1;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x69 [ReceiveMonsterSkill(Skill : %d | SKey : %d |TKey : %d)]", SkillNumber, SourceKey, TargetKey);

    return (TRUE);
}

BOOL ReceiveMagic(const BYTE* ReceiveBuffer, int Size, BOOL bEncrypted)
{
    auto Data = (LPPRECEIVE_MAGIC)ReceiveBuffer;
    int SourceKey = ((int)(Data->SourceKeyH) << 8) + Data->SourceKeyL;
    int TargetKey = ((int)(Data->TargetKeyH) << 8) + Data->TargetKeyL;
    int Success = (TargetKey >> 15);

    WORD MagicNumber = ((WORD)(Data->MagicH) << 8) + Data->MagicL;

    if (MagicNumber == AT_SKILL_ATTACK
        || MagicNumber == AT_SKILL_DEFENSE
        || MagicNumber == AT_SKILL_ATTACK_STR
        || MagicNumber == AT_SKILL_DEFENSE_STR
        || MagicNumber == AT_SKILL_ATTACK_MASTERY
        || MagicNumber == AT_SKILL_DEFENSE_MASTERY)
    {
        if (Success == false)
        {
            g_pSystemLogBox->AddText(GlobalText[2249], SEASON3B::TYPE_SYSTEM_MESSAGE);
            return FALSE;
        }
    }

    TargetKey &= 0x7FFF;

    int Index = FindCharacterIndex(SourceKey);
    int TargetIndex = FindCharacterIndex(TargetKey);
    if (TargetIndex == MAX_CHARACTERS_CLIENT)
        return (TRUE);

    AttackPlayer = Index;
    CHARACTER* sc = &CharactersClient[Index];
    CHARACTER* tc = &CharactersClient[TargetIndex];
    OBJECT* so = &sc->Object;
    OBJECT* to = &tc->Object;

    if (MagicNumber != AT_SKILL_COMBO)
    {
        if (sc != Hero && MagicNumber != AT_SKILL_TELEPORT && MagicNumber != AT_SKILL_TELEPORT_ALLY && to->Visible)
            so->Angle[2] = CreateAngle2D(so->Position, to->Position);
        sc->TargetCharacter = TargetIndex;

        if (MagicNumber == AT_SKILL_PLASMA_STORM_FENRIR)
            sc->m_iFenrirSkillTarget = TargetIndex;

        sc->SkillSuccess = (Success != 0);
        sc->Skill = MagicNumber;
    }
    switch (MagicNumber)
    {
    case AT_SKILL_MONSTER_SUMMON:
        SetPlayerAttack(sc);
        break;
    case AT_SKILL_RECOVER:
    {
        OBJECT* oh = &Hero->Object;
        OBJECT* o = so;
        vec3_t Light, Position, P, dp;
        vec3_t vFirePosition;

        float Matrix[3][4];

        if (sc != Hero)
        {
            Vector(0.f, -220.f, 130.f, P);
            AngleMatrix(o->Angle, Matrix);
            VectorRotate(P, Matrix, dp);
            VectorAdd(dp, o->Position, Position);
            Vector(0.7f, 0.6f, 0.f, Light);
            CreateEffect(BITMAP_IMPACT, Position, o->Angle, Light, 0, o);
            SetAction(o, PLAYER_RECOVER_SKILL);
            sc->AttackTime = 1;
        }

        o = to;

        Vector(0.4f, 0.6f, 1.f, Light);
        Vector(0.f, 0.f, 0.f, P);
        AngleMatrix(o->Angle, Matrix);
        VectorRotate(P, Matrix, dp);
        VectorAdd(dp, o->Position, Position);
        Position[2] += 130;
        VectorCopy(o->Position, Position);
        for (int i = 0; i < 19; ++i)
        {
            CreateJoint(BITMAP_FLARE, Position, Position, o->Angle, 47, o, 40, 2);
        }
        Vector(0.3f, 0.2f, 0.1f, Light);
        CreateEffect(MODEL_SUMMON, Position, o->Angle, Light, 0);
        CreateEffect(BITMAP_TWLIGHT, Position, o->Angle, Light, 0);
        CreateEffect(BITMAP_TWLIGHT, Position, o->Angle, Light, 1);
        CreateEffect(BITMAP_TWLIGHT, Position, o->Angle, Light, 2);

        for (int i = 0; i < 2; ++i)
        {
            int iNumBones = Models[o->Type].NumBones;
            Models[o->Type].TransformByObjectBone(vFirePosition, o, rand() % iNumBones);
            CreateEffect(BITMAP_FLARE, vFirePosition, o->Angle, Light, 3);
        }
    }

    break;
    case AT_SKILL_MONSTER_MAGIC_DEF:
        sc->AttackTime = 1;
        g_CharacterRegisterBuff(so, eBuff_WizDefense);
        SetPlayerAttack(sc);
        break;

    case AT_SKILL_MONSTER_PHY_DEF:
        sc->AttackTime = 1;
        g_CharacterRegisterBuff(so, eBuff_Defense);
        SetPlayerAttack(sc);
        break;
    case AT_SKILL_HEALING:
    case AT_SKILL_HEALING_STR:
    case AT_SKILL_ATTACK:
    case AT_SKILL_ATTACK_STR:
    case AT_SKILL_ATTACK_MASTERY:
    case AT_SKILL_DEFENSE:
    case AT_SKILL_DEFENSE_STR:
    case AT_SKILL_DEFENSE_MASTERY:
    case AT_SKILL_SUMMON:
    case AT_SKILL_SUMMON + 1:
    case AT_SKILL_SUMMON + 2:
    case AT_SKILL_SUMMON + 3:
    case AT_SKILL_SUMMON + 4:
    case AT_SKILL_SUMMON + 5:
    case AT_SKILL_SUMMON + 6:
#ifdef ADD_ELF_SUMMON
    case AT_SKILL_SUMMON + 7:
#endif // ADD_ELF_SUMMON
        if (sc->MonsterIndex != MONSTER_DARK_PHOENIX)
        {
            PlayBuffer(SOUND_SKILL_DEFENSE);
        }
    case AT_SKILL_SOUL_BARRIER:
    case AT_SKILL_SOUL_BARRIER_STR:
    case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
        if (sc != Hero)
        {
            SetPlayerMagic(sc);
            so->AnimationFrame = 0;
        }
        sc->AttackTime = 1;
        break;
    case AT_SKILL_LIGHTNING:
    case AT_SKILL_LIGHTNING_STR:
    case AT_SKILL_LIGHTNING_STR_MG:
        if (so->Type != MODEL_QUEEN_BEE)
            PlayBuffer(SOUND_THUNDER01);
        if (to->CurrentAction == PLAYER_POSE1 || to->CurrentAction == PLAYER_POSE_FEMALE1 ||
            to->CurrentAction == PLAYER_SIT1 || to->CurrentAction == PLAYER_SIT_FEMALE1)
            SetPlayerStop(tc);
    case AT_SKILL_FIREBALL:
    case AT_SKILL_METEO:
    case AT_SKILL_ICE:
    case AT_SKILL_ICE_STR:
    case AT_SKILL_ICE_STR_MG:
    case AT_SKILL_ENERGYBALL:
    case AT_SKILL_POWERWAVE:
    case AT_SKILL_POISON:
    case AT_SKILL_POISON_STR:
    case AT_SKILL_FLAME:
    case AT_SKILL_FLAME_STR:
    case AT_SKILL_FLAME_STR_MG:
        if (sc != Hero)
        {
            if (so->Type == MODEL_PLAYER)
            {
                SetPlayerMagic(sc);
                so->AnimationFrame = 0;
            }
            else
            {
                SetPlayerAttack(sc);
                so->AnimationFrame = 0;
            }
        }
        sc->AttackTime = 1;
        break;

    case AT_SKILL_TELEPORT_ALLY:
        CreateTeleportBegin(to);
        CreateTeleportEnd(so);

        PlayBuffer(SOUND_TELEKINESIS, so);
        break;

    case AT_SKILL_TELEPORT:
        if (gMapManager.WorldActive == WD_39KANTURU_3RD && so->Type == MODEL_DARK_SKULL_SOLDIER_5)
        {
            vec3_t Light;
            Vector(1.0f, 1.0f, 1.0f, Light);
            so->AlphaTarget = 0.f;
            CreateEffect(BITMAP_SPARK + 1, so->Position, so->Angle, Light);
            CreateEffect(BITMAP_SPARK + 1, so->Position, so->Angle, Light);
            PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_TELE);
        }
        else
            CreateTeleportBegin(so);
        break;

    case AT_SKILL_FORCE:
    case AT_SKILL_FORCE_WAVE:
    case AT_SKILL_FORCE_WAVE_STR:
        if ((sc->Helper.Type >= MODEL_HORN_OF_UNIRIA && sc->Helper.Type <= MODEL_DARK_HORSE_ITEM) && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_STRIKE);
        }
        else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
        {
            SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
        }
        else
        {
            SetAction(so, PLAYER_ATTACK_STRIKE);
        }
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD1);
        break;
    case AT_SKILL_FIREBURST:
    case AT_SKILL_FIREBURST_STR:
    case AT_SKILL_FIREBURST_MASTERY:
        if ((sc->Helper.Type >= MODEL_HORN_OF_UNIRIA && sc->Helper.Type <= MODEL_DARK_HORSE_ITEM) && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_STRIKE);
        }
        else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
        {
            SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
        }
        else
        {
            SetAction(so, PLAYER_ATTACK_STRIKE);
        }
        sc->AttackTime = 1;
        PlayBuffer(SOUND_ATTACK_FIRE_BUST);
        break;

    case AT_SKILL_FALLING_SLASH:
    case AT_SKILL_FALLING_SLASH_STR:
        SetAction(so, PLAYER_ATTACK_SKILL_SWORD1 + AT_SKILL_FALLING_SLASH - AT_SKILL_FALLING_SLASH);
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD1);
        break;

    case AT_SKILL_LUNGE:
    case AT_SKILL_LUNGE_STR:
        SetAction(so, PLAYER_ATTACK_SKILL_SWORD1 + AT_SKILL_LUNGE - AT_SKILL_FALLING_SLASH);
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD2);
        break;

    case AT_SKILL_UPPERCUT:
        SetAction(so, PLAYER_ATTACK_SKILL_SWORD1 + AT_SKILL_UPPERCUT - AT_SKILL_FALLING_SLASH);
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD3);
        break;

    case AT_SKILL_CYCLONE:
    case AT_SKILL_CYCLONE_STR:
    case AT_SKILL_CYCLONE_STR_MG:
        SetAction(so, PLAYER_ATTACK_SKILL_SWORD1 + AT_SKILL_CYCLONE - AT_SKILL_FALLING_SLASH);
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD4);
        break;

    case AT_SKILL_SLASH://베기
    case AT_SKILL_SLASH_STR:
        if (sc->SwordCount % 2 == 0)
        {
            SetAction(so, PLAYER_ATTACK_SKILL_SWORD1 + AT_SKILL_SLASH - AT_SKILL_FALLING_SLASH);
        }
        else
        {
            SetAction(so, PLAYER_ATTACK_TWO_HAND_SWORD1 + 2);
            //SetAction(so,PLAYER_ATTACK_TWO_HAND_SWORD1+sc->SwordCount%3);
        }
        sc->SwordCount++;
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD4);
        break;
    case AT_SKILL_POWER_SLASH:
    case AT_SKILL_POWER_SLASH_STR:
        SetAction(so, PLAYER_ATTACK_TWO_HAND_SWORD_TWO);
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD4);
        break;

    case AT_SKILL_IMPALE:	// 창찌르기
        if (sc->Helper.Type == MODEL_HORN_OF_FENRIR)
            SetAction(so, PLAYER_FENRIR_ATTACK_SPEAR);
        else
            SetAction(so, PLAYER_ATTACK_SKILL_SPEAR);
        sc->AttackTime = 1;
        break;
    case AT_SKILL_DEATHSTAB:
    case AT_SKILL_DEATHSTAB_STR:
        SetAction(so, PLAYER_ATTACK_DEATHSTAB);
        if (sc != Hero && so->Type == MODEL_PLAYER)
        {
            so->AnimationFrame = 0;
        }
        sc->AttackTime = 1;
        break;
    case AT_SKILL_TRIPLE_SHOT:
    case AT_SKILL_TRIPLE_SHOT_STR:
    case AT_SKILL_TRIPLE_SHOT_MASTERY:
        SetPlayerBow(sc);
        sc->AttackTime = 1;
        break;
    case AT_SKILL_MULTI_SHOT:
        SetPlayerBow(sc);
        if (sc != Hero && so->Type == MODEL_PLAYER)
        {
            so->AnimationFrame = 0;
        }
        sc->AttackTime = 1;
        break;
    case AT_SKILL_BLAST_CROSSBOW4:
        SetPlayerBow(sc);
        sc->AttackTime = 1;
        break;
    case AT_SKILL_FIRE_SLASH:
    case AT_SKILL_FIRE_SLASH_STR:
        if (sc->SkillSuccess)
        {
            DeleteEffect(BITMAP_SKULL, to, 0);
            vec3_t Light = { 1.0f, 1.f, 1.f };
            CreateEffect(BITMAP_SKULL, to->Position, to->Angle, Light, 0, to);

            PlayBuffer(SOUND_BLOODATTACK, to);
            g_CharacterRegisterBuff(to, eDeBuff_Defense);
        }
        if (so->Type != MODEL_PLAYER)
        {
            SetPlayerAttack(sc);
            so->AnimationFrame = 0;
            sc->AttackTime = 1;
        }
        break;

    case AT_SKILL_PENETRATION:
    case AT_SKILL_PENETRATION_STR:
        SetPlayerBow(sc);
        sc->AttackTime = 1;
        break;

    case AT_SKILL_ICE_ARROW:
    case AT_SKILL_ICE_ARROW_STR:
        SetPlayerBow(sc);
        if (sc->SkillSuccess)
        {
            DeleteEffect(MODEL_ICE, to, 1);

            vec3_t Angle;
            VectorCopy(to->Angle, Angle);

            CreateEffect(MODEL_ICE, to->Position, Angle, to->Light, 1, to);

            Angle[2] += 180.f;
            CreateEffect(MODEL_ICE, to->Position, Angle, to->Light, 2, to);

            tc->Movement = false;
            SetPlayerStop(tc);

            g_CharacterRegisterBuff(to, eDeBuff_Harden);
        }
        sc->AttackTime = 1;
        break;

    case AT_SKILL_RUSH:
        SetAction(so, PLAYER_ATTACK_RUSH);
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD2);
        break;

    case AT_SKILL_JAVELIN:
        if (sc != Hero)
        {
            if (so->Type == MODEL_PLAYER)
            {
                SetPlayerMagic(sc);
                so->AnimationFrame = 0;
            }
        }
        so->AnimationFrame = 0;
        sc->AttackTime = 1;
        break;

    case AT_SKILL_DEATH_CANNON:
        SetAction(so, PLAYER_ATTACK_DEATH_CANNON);
        so->AnimationFrame = 0;
        sc->AttackTime = 1;
        break;

    case AT_SKILL_DEEPIMPACT:
        SetPlayerHighBow(sc);
        sc->AttackTime = 1;
        break;

    case AT_SKILL_SPACE_SPLIT:
        if ((sc->Helper.Type >= MODEL_HORN_OF_UNIRIA && sc->Helper.Type <= MODEL_DARK_HORSE_ITEM) && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_STRIKE);
        }
        else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
        {
            SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
        }
        else
        {
            SetAction(so, PLAYER_ATTACK_STRIKE);
        }
        sc->AttackTime = 1;
        PlayBuffer(SOUND_ATTACK_FIRE_BUST);
        break;

    case AT_SKILL_BRAND_OF_SKILL:

        g_CharacterRegisterBuff(to, eBuff_AddSkill);

        if (sc->Helper.Type == MODEL_DARK_HORSE_ITEM && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
        }
        else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
        {
            SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);
        }
        else
        {
            SetAction(so, PLAYER_SKILL_HAND1);
        }
        sc->AttackTime = 1;
        break;

    case AT_SKILL_SPIRAL_SLASH:
        SetAction(so, PLAYER_ATTACK_ONE_FLASH);
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD2);
        break;

    case AT_SKILL_STUN:
        if (sc->Helper.Type == MODEL_DARK_HORSE_ITEM && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
        }
        else
            if (sc->Helper.Type == MODEL_HORN_OF_UNIRIA && !sc->SafeZone)
            {
                SetAction(so, PLAYER_SKILL_RIDER);
            }
            else
                if (sc->Helper.Type == MODEL_HORN_OF_DINORANT && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_SKILL_RIDER_FLY);
                }
                else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);
                }
                else
                {
                    SetAction(so, PLAYER_SKILL_VITALITY);
                }
        sc->AttackTime = 1;
        break;

    case AT_SKILL_REMOVAL_STUN:
        if (sc->Helper.Type == MODEL_DARK_HORSE_ITEM && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
        }
        else
            if (sc->Helper.Type == MODEL_HORN_OF_UNIRIA && !sc->SafeZone)
            {
                SetAction(so, PLAYER_SKILL_RIDER);
            }
            else
                if (sc->Helper.Type == MODEL_HORN_OF_DINORANT && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_SKILL_RIDER_FLY);
                }
                else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);
                }
                else
                {
                    SetAction(so, PLAYER_ATTACK_REMOVAL);
                }

        if (sc->SkillSuccess)
        {
            UnRegisterBuff(eDeBuff_Stun, to);
        }
        sc->AttackTime = 1;
        break;

    case AT_SKILL_INVISIBLE:
        if (sc->Helper.Type == MODEL_DARK_HORSE_ITEM && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
        }
        else
            if (sc->Helper.Type == MODEL_HORN_OF_UNIRIA && !sc->SafeZone)
            {
                SetAction(so, PLAYER_SKILL_RIDER);
            }
            else
                if (sc->Helper.Type == MODEL_HORN_OF_DINORANT && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_SKILL_RIDER_FLY);
                }
                else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);
                }
                else
                    SetAction(so, PLAYER_SKILL_VITALITY);
        if (sc->SkillSuccess)
        {
            if (!g_isCharacterBuff(to, eBuff_Cloaking))
            {
                if (so != to)
                {
                    g_CharacterRegisterBuff(to, eBuff_Cloaking);
                }
            }
        }
        sc->AttackTime = 1;
        break;

    case AT_SKILL_REMOVAL_INVISIBLE:
        if (sc->Helper.Type == MODEL_DARK_HORSE_ITEM && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
        }
        else
            if (sc->Helper.Type == MODEL_HORN_OF_UNIRIA && !sc->SafeZone)
            {
                SetAction(so, PLAYER_SKILL_RIDER);
            }
            else
                if (sc->Helper.Type == MODEL_HORN_OF_DINORANT && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_SKILL_RIDER_FLY);
                }
                else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);
                }
                else
                    SetAction(so, PLAYER_ATTACK_REMOVAL);
        if (sc->SkillSuccess)
        {
            UnRegisterBuff(eBuff_Cloaking, to);
        }
        sc->AttackTime = 1;
        break;

    case AT_SKILL_MANA:
        if (sc->Helper.Type == MODEL_DARK_HORSE_ITEM && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
        }
        else
            if (sc->Helper.Type == MODEL_HORN_OF_UNIRIA && !sc->SafeZone)
            {
                SetAction(so, PLAYER_SKILL_RIDER);
            }
            else
                if (sc->Helper.Type == MODEL_HORN_OF_DINORANT && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_SKILL_RIDER_FLY);
                }
                else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);
                }
                else
                    SetAction(so, PLAYER_SKILL_VITALITY);
        if (sc->SkillSuccess)
        {
            g_CharacterRegisterBuff(so, eBuff_AddMana);

            CreateEffect(MODEL_MANA_RUNE, so->Position, so->Angle, so->Light);
        }
        sc->AttackTime = 1;
        break;

    case AT_SKILL_REMOVAL_BUFF:
        if (sc->SkillSuccess)
        {
            UnRegisterBuff(eBuff_Attack, to);
            UnRegisterBuff(eBuff_Defense, to);
            UnRegisterBuff(eBuff_Life, to);
            UnRegisterBuff(eBuff_WizDefense, to);
            UnRegisterBuff(eBuff_AddCriticalDamage, to);
            UnRegisterBuff(eBuff_AddMana, to);
        }
        if (sc->Helper.Type == MODEL_DARK_HORSE_ITEM && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
        }
        else
            if (sc->Helper.Type == MODEL_HORN_OF_UNIRIA && !sc->SafeZone)
            {
                SetAction(so, PLAYER_SKILL_RIDER);
            }
            else
                if (sc->Helper.Type == MODEL_HORN_OF_DINORANT && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_SKILL_RIDER_FLY);
                }
                else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
                {
                    SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);
                }
                else
                    SetAction(so, PLAYER_SKILL_VITALITY);
        sc->AttackTime = 1;
        break;
    case AT_SKILL_IMPROVE_AG:
        if (!g_isCharacterBuff(to, eBuff_AddAG))
        {
            DeleteEffect(BITMAP_LIGHT, to, 2);
            DeleteJoint(BITMAP_JOINT_HEALING, to, 9);

            CreateEffect(BITMAP_LIGHT, to->Position, to->Angle, to->Light, 2, to);
        }

        g_CharacterRegisterBuff(to, eBuff_AddAG);

        if (sc != Hero)
        {
            SetPlayerMagic(sc);
            so->AnimationFrame = 0;
        }
        sc->AttackTime = 1;
        break;
    case AT_SKILL_ADD_CRITICAL:
        SetAttackSpeed();

        g_CharacterRegisterBuff(to, eBuff_AddCriticalDamage);

        if (sc->Helper.Type == MODEL_DARK_HORSE_ITEM && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_ATTACK_MAGIC);
        }
        else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
        {
            SetAction(so, PLAYER_FENRIR_ATTACK_MAGIC);
        }
        else
        {
            SetAction(so, PLAYER_SKILL_HAND1);
        }
        sc->AttackTime = 1;
        break;
    case AT_SKILL_PARTY_TELEPORT:
        if ((sc->Helper.Type >= MODEL_HORN_OF_UNIRIA && sc->Helper.Type <= MODEL_DARK_HORSE_ITEM) && !sc->SafeZone)
        {
            SetAction(so, PLAYER_ATTACK_RIDE_TELEPORT);
        }
        else if (sc->Helper.Type == MODEL_HORN_OF_FENRIR && !sc->SafeZone)
        {
            SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT);
        }
        else
        {
            SetAction(so, PLAYER_ATTACK_TELEPORT);
        }
        if (so->Type != MODEL_PLAYER)
        {
            SetPlayerAttack(sc);
            so->AnimationFrame = 0;
        }
        sc->AttackTime = 1;
        break;
    case AT_SKILL_SWELL_LIFE:
    case AT_SKILL_SWELL_LIFE_STR:
    case AT_SKILL_SWELL_LIFE_PROFICIENCY:
        if (!g_isCharacterBuff(to, eBuff_Life))
        {
            DeleteEffect(BITMAP_LIGHT, to, 1);

            CreateEffect(BITMAP_LIGHT, to->Position, to->Angle, to->Light, 1, to);
        }

        g_CharacterRegisterBuff(to, eBuff_Life);

        SetAction(so, PLAYER_SKILL_VITALITY);
        sc->AttackTime = 1;
        break;

    case AT_SKILL_RIDER:
        if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
            SetAction(so, PLAYER_SKILL_RIDER_FLY);
        else
            SetAction(so, PLAYER_SKILL_RIDER);
        sc->AttackTime = 1;
        break;

    case AT_SKILL_BOSS:
        sc->AttackTime = 1;
        break;

    case AT_SKILL_COMBO:
        CreateEffect(MODEL_COMBO, so->Position, so->Angle, so->Light);
        PlayBuffer(SOUND_COMBO);
        break;
    case AT_SKILL_NOVA_BEGIN:
        SetAction(so, PLAYER_SKILL_HELL_BEGIN);
        PlayBuffer(SOUND_NUKE1);
        so->m_bySkillCount = 0;
        break;
    case AT_SKILL_NOVA:
        SetAction(so, PLAYER_SKILL_HELL_START);
        if (sc == Hero)
        {
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            MouseRButton = false;

            MouseRButtonPress = 0;
        }
        sc->AttackTime = 1;
        break;
    case AT_SKILL_FIRE_SCREAM:
    case AT_SKILL_FIRE_SCREAM_STR:
    {
        if (sc->Helper.Type == MODEL_HORN_OF_FENRIR)
        {
            SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
        }
        else if ((sc->Helper.Type >= MODEL_HORN_OF_UNIRIA) && (sc->Helper.Type <= MODEL_DARK_HORSE_ITEM))
        {
            SetAction(so, PLAYER_ATTACK_RIDE_STRIKE);
        }
        else
        {
            SetAction(so, PLAYER_ATTACK_STRIKE);
        }
        PlayBuffer(SOUND_FIRE_SCREAM);
    }
    break;
    case AT_SKILL_EXPLODE:
    {
        vec3_t Pos;
        float addx, addz;
        for (int iaa = 0; iaa < 3; iaa++)
        {
            addx = rand() % 160;
            addz = rand() % 160;
            Vector(to->Position[0] + addx, to->Position[1] + addz, to->Position[2], Pos);

            CreateBomb(Pos, true);
        }
    }
    break;
    case AT_SKILL_INFINITY_ARROW:
    case AT_SKILL_INFINITY_ARROW_STR:
    {
        CharacterMachine->InfinityArrowAdditionalMana = 10;
        CreateEffect(MODEL_INFINITY_ARROW, so->Position, so->Angle, so->Light, 0, so);
        PlayBuffer(SOUND_INFINITYARROW);
    }
    break;
    case AT_SKILL_ALICE_CHAINLIGHTNING:
    case AT_SKILL_ALICE_CHAINLIGHTNING_STR:
    {
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_CHAIN_LIGHTNING);
    }
    break;
    case AT_SKILL_ALICE_SLEEP:
    case AT_SKILL_ALICE_SLEEP_STR:
    {
        sc->AttackTime = 1;

        switch (sc->Helper.Type)
        {
        case MODEL_HORN_OF_UNIRIA:
            SetAction(so, PLAYER_SKILL_SLEEP_UNI);
            break;
        case MODEL_HORN_OF_DINORANT:
            SetAction(so, PLAYER_SKILL_SLEEP_DINO);
            break;
        case MODEL_HORN_OF_FENRIR:
            SetAction(so, PLAYER_SKILL_SLEEP_FENRIR);
            break;
        default:
            SetAction(so, PLAYER_SKILL_SLEEP);
            break;
        }

        PlayBuffer(SOUND_SUMMON_SKILL_SLEEP);
    }
    break;
    case AT_SKILL_ALICE_BLIND:
        sc->AttackTime = 1; // todo: what is this?

        SetAttackSpeed();
        SetAction(so, PLAYER_SKILL_SLEEP); // todo: check if same action for this skill

        PlayBuffer(SOUND_SUMMON_SKILL_BLIND);
        break;
    case AT_SKILL_ALICE_THORNS:
        sc->AttackTime = 1; // todo: what is this?

        SetAttackSpeed();
        SetAction(so, PLAYER_SKILL_SLEEP); // todo: check if same action for this skill

        g_CharacterRegisterBuff(to, eBuff_Thorns);

        PlayBuffer(SOUND_SUMMON_SKILL_THORNS);
        break;
    case AT_SKILL_ALICE_BERSERKER:
    case AT_SKILL_ALICE_BERSERKER_STR:
        sc->AttackTime = 1; // todo: what is this?

        SetAttackSpeed();
        SetAction(so, PLAYER_SKILL_SLEEP); // todo: check if same action for this skill

        g_CharacterRegisterBuff(to, eBuff_Berserker);

        PlayBuffer(SOUND_SKILL_BERSERKER);
        break;
    case AT_SKILL_EXPANSION_OF_WIZARDRY:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_STR:
    case AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY:
    {
        SetAttackSpeed();
        SetAction(so, PLAYER_SKILL_SWELL_OF_MP);

        vec3_t vLight;
        Vector(0.3f, 0.2f, 0.9f, vLight);
        CreateEffect(MODEL_SWELL_OF_MAGICPOWER, so->Position, so->Angle, vLight, 0, so);
        PlayBuffer(SOUND_SKILL_SWELL_OF_MAGICPOWER);
    }break;
    case AT_SKILL_DOPPELGANGER_SELFDESTRUCTION:
    {
        SetAction(so, MONSTER01_APEAR);
        so->m_bActionStart = FALSE;
    }
    break;
    case AT_SKILL_GAOTIC:
    {
        if (so->Type == MODEL_PLAYER)
        {
            if (sc->Helper.Type == MODEL_HORN_OF_FENRIR)
            {
                SetAction(&sc->Object, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
            }
            else if ((sc->Helper.Type >= MODEL_HORN_OF_UNIRIA) && (sc->Helper.Type <= MODEL_DARK_HORSE_ITEM))
            {
                SetAction(&sc->Object, PLAYER_ATTACK_RIDE_STRIKE);
            }
            else
            {
                SetAction(&sc->Object, PLAYER_ATTACK_STRIKE);
            }
        }
        else
        {
            SetPlayerAttack(sc);
        }

        OBJECT* o = so;
        vec3_t Light, Position, P, dp;

        float Matrix[3][4];
        Vector(0.f, -20.f, 0.f, P);
        Vector(0.f, 0.f, 0.f, P);
        AngleMatrix(o->Angle, Matrix);
        VectorRotate(P, Matrix, dp);
        VectorAdd(dp, o->Position, Position);

        Vector(0.5f, 0.5f, 0.5f, Light);
        for (int i = 0; i < 5; ++i)
        {
            CreateEffect(BITMAP_SHINY + 6, Position, o->Angle, Light, 3, o, -1, 0, 0, 0, 0.5f);
        }

        VectorCopy(o->Position, Position);

        for (int i = 0; i < 8; i++)
        {
            Position[0] = (o->Position[0] - 119.f) + (float)(rand() % 240);
            Position[2] = (o->Position[2] + 49.f) + (float)(rand() % 60);
            CreateJoint(BITMAP_2LINE_GHOST, Position, o->Position, o->Angle, 0, o, 20.f, o->PKKey, 0, o->m_bySkillSerialNum);
        }
        if (sc == Hero && SelectedCharacter != -1)
        {
            vec3_t Pos;
            CHARACTER* st = &CharactersClient[SelectedCharacter];
            VectorCopy(st->Object.Position, Pos);
            CreateBomb(Pos, true);
        }
        PlayBuffer(SOUND_SKILL_CAOTIC);
    }
    break;
    case AT_SKILL_KILLING_BLOW:
    case AT_SKILL_KILLING_BLOW_STR:
    case AT_SKILL_KILLING_BLOW_MASTERY:
    {
        g_CMonkSystem.SetRageSkillAni(MagicNumber, so);
        so->m_sTargetIndex = TargetIndex;
        sc->AttackTime = 1;

        if (sc != Hero)
        {
            g_CMonkSystem.RageCreateEffect(so, MagicNumber);
        }
    }
    break;
    case AT_SKILL_ATT_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES:
    case AT_SKILL_DEF_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
    {
        if (sc == Hero)
        {
            if (so->CurrentAction == PLAYER_SKILL_ATT_UP_OURFORCES)
            {
                SendRequestAction(sc->Object, AT_RAGEBUFF_1);
            }
            else
            {
                SendRequestAction(sc->Object, AT_RAGEBUFF_2);
            }
        }
        OBJECT* _pObj = to;
        if (sc != Hero)
        {
            _pObj = so;
            g_CMonkSystem.RageCreateEffect(_pObj, MagicNumber);
        }

        if (MagicNumber == AT_SKILL_HP_UP_OURFORCES || MagicNumber == AT_SKILL_HP_UP_OURFORCES_STR)
        {
            DeleteEffect(BITMAP_LIGHT_RED, _pObj, 0);
            CreateEffect(BITMAP_LIGHT_RED, _pObj->Position, _pObj->Angle, _pObj->Light, 0, _pObj, -1, 0, 0, 0, 1.5f);
        }
        else if (MagicNumber == AT_SKILL_DEF_UP_OURFORCES || MagicNumber == AT_SKILL_DEF_UP_OURFORCES_STR || MagicNumber == AT_SKILL_DEF_UP_OURFORCES_MASTERY)
        {
            DeleteEffect(BITMAP_LIGHT_RED, _pObj, 2);
            CreateEffect(BITMAP_LIGHT_RED, _pObj->Position, _pObj->Angle, _pObj->Light, 2, _pObj, -1, 0, 0, 0, 1.5f);

            vec3_t vLight;
            Vector(0.7f, 0.7f, 1.0f, vLight);
            DeleteEffect(MODEL_WINDFOCE, _pObj, 1);
            CreateEffect(MODEL_WINDFOCE, _pObj->Position, _pObj->Angle, vLight, 1, _pObj, -1, 0, 0, 0, 1.0f);
        }
        else if (MagicNumber == AT_SKILL_ATT_UP_OURFORCES)
        {
            DeleteEffect(BITMAP_LIGHT_RED, _pObj, 1);
            CreateEffect(BITMAP_LIGHT_RED, _pObj->Position, _pObj->Angle, _pObj->Light, 1, _pObj, -1, 0, 0, 0, 1.5f);
        }

        switch (MagicNumber)
        {
        case AT_SKILL_ATT_UP_OURFORCES:
            g_CharacterRegisterBuff(_pObj, eBuff_Att_up_Ourforces);
            break;
        case AT_SKILL_HP_UP_OURFORCES:
        case AT_SKILL_HP_UP_OURFORCES_STR:
            g_CharacterRegisterBuff(_pObj, eBuff_Hp_up_Ourforces);
            break;
        case AT_SKILL_DEF_UP_OURFORCES:
        case AT_SKILL_DEF_UP_OURFORCES_STR:
        case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
            g_CharacterRegisterBuff(_pObj, eBuff_Def_up_Ourforces);
            break;
        }
        sc->AttackTime = 1;
    }
    break;
    case AT_SKILL_BEAST_UPPERCUT:
    case AT_SKILL_BEAST_UPPERCUT_STR:
    case AT_SKILL_BEAST_UPPERCUT_MASTERY:
    {
        sc->AttackTime = 1;
    }
    break;
    case AT_SKILL_CHAIN_DRIVE:
    case AT_SKILL_CHAIN_DRIVE_STR:
    case AT_SKILL_DRAGON_KICK:
    {
        sc->AttackTime = 1;
        if (sc != Hero)
            g_CMonkSystem.RageCreateEffect(so, MagicNumber);
    }
    break;
    case AT_SKILL_OCCUPY:
    {
        SetAction(so, PLAYER_ATTACK_RUSH);
        sc->AttackTime = 1;
        PlayBuffer(SOUND_SKILL_SWORD2);
    }
    break;
    case AT_SKILL_DRAGON_ROAR:
    case AT_SKILL_DRAGON_ROAR_STR:
    {
        vec3_t vLight;
        Vector(1.0f, 1.0f, 1.0f, vLight);
        if (g_CMonkSystem.SetLowerEffEct())
        {
            CreateEffect(MODEL_VOLCANO_OF_MONK, to->Position, to->Angle, vLight, g_CMonkSystem.GetLowerEffCnt(), to, -1, 0, 0, 0, 0.8f);
        }

        CreateEffect(MODEL_TARGETMON_EFFECT, to->Position, to->Angle, vLight, 0, to, -1, 0, 0, 0, 1.0f);
        StopBuffer(SOUND_RAGESKILL_DRAGONLOWER_ATTACK, true);
        PlayBuffer(SOUND_RAGESKILL_DRAGONLOWER_ATTACK);
    }
    break;
    case AT_SKILL_DARKSIDE:
    case AT_SKILL_DARKSIDE_STR:
    {
        if (sc != Hero)
            g_CMonkSystem.SetDarksideCnt();
    }
    break;
    }

#ifdef CONSOLE_DEBUG
    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x19 [ReceiveMagic(%d)]", MagicNumber);
#endif // CONSOLE_DEBUG

    return (TRUE);
}

BOOL ReceiveMagicContinue(const BYTE* ReceiveBuffer, int Size, BOOL bEncrypted)
{
    auto Data = (LPPRECEIVE_MAGIC_CONTINUE)ReceiveBuffer;
    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    WORD MagicNumber = ((WORD)(Data->MagicH) << 8) + Data->MagicL;

    CHARACTER* sc = &CharactersClient[FindCharacterIndex(Key)];
    OBJECT* so = &sc->Object;

    sc->Skill = MagicNumber;

    if (MagicNumber == AT_SKILL_PLASMA_STORM_FENRIR)
        sc->m_iFenrirSkillTarget = FindCharacterIndex(Key);

    so->Angle[2] = (Data->Angle / 255.f) * 360.f;

    if (so->Type == MODEL_PLAYER)
    {
        if (sc != Hero)
        {
            switch (MagicNumber)
            {
            case AT_SKILL_TRIPLE_SHOT:
            case AT_SKILL_TRIPLE_SHOT_STR:
            case AT_SKILL_TRIPLE_SHOT_MASTERY:
                SetPlayerAttack(sc);
                break;
            case AT_SKILL_BLAST_CROSSBOW4:
                SetPlayerAttack(sc);
                break;
            case AT_SKILL_DECAY:
            case AT_SKILL_DECAY_STR:
            case AT_SKILL_ICE_STORM:
                SetPlayerMagic(sc);
                break;

            case AT_SKILL_PENETRATION:
            case AT_SKILL_PENETRATION_STR:
                SetPlayerAttack(sc);
                break;

            case AT_SKILL_FLASH:
                SetAction(so, PLAYER_SKILL_FLASH);
                break;

            case AT_SKILL_HELL_FIRE:
            case AT_SKILL_HELL_FIRE_STR:
                SetAction(so, PLAYER_SKILL_HELL);
                break;

            case AT_SKILL_INFERNO:
            case AT_SKILL_INFERNO_STR:
            case AT_SKILL_INFERNO_STR_MG:
                SetAction(so, PLAYER_SKILL_INFERNO);
                break;
            case AT_SKILL_TWISTING_SLASH:
            case AT_SKILL_TWISTING_SLASH_STR:
            case AT_SKILL_TWISTING_SLASH_MASTERY:
            case AT_SKILL_TWISTING_SLASH_STR_MG:
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
                switch (sc->Helper.Type)
                {
                case MODEL_HORN_OF_UNIRIA:
                    SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_UNI);
                    break;
                case MODEL_HORN_OF_DINORANT:
                    SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_DINO);
                    break;
                case MODEL_HORN_OF_FENRIR:
                    SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_FENRIR);
                    break;
                default:
                    SetAction(so, PLAYER_ATTACK_SKILL_WHEEL);
                    break;
                }
#else	// YDG_ADD_SKILL_RIDING_ANIMATIONS
                SetAction(so, PLAYER_ATTACK_SKILL_WHEEL);
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS
                break;

            case AT_SKILL_FIRE_SCREAM:
            case AT_SKILL_FIRE_SCREAM_STR:
            {
                if (sc->Helper.Type == MODEL_HORN_OF_FENRIR)
                {
                    SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
                }
                else if ((sc->Helper.Type >= MODEL_HORN_OF_UNIRIA) && (sc->Helper.Type <= MODEL_DARK_HORSE_ITEM))
                {
                    SetAction(so, PLAYER_ATTACK_RIDE_STRIKE);
                }
                else
                {
                    SetAction(so, PLAYER_ATTACK_STRIKE);
                }
                PlayBuffer(SOUND_FIRE_SCREAM);
            }
            break;

            case AT_SKILL_THUNDER_STRIKE:
                if (sc->Helper.Type == MODEL_HORN_OF_FENRIR)
                    SetAction(so, PLAYER_FENRIR_ATTACK_DARKLORD_FLASH);
                else
                    SetAction(so, PLAYER_SKILL_FLASH);
                break;
            case AT_SKILL_EARTHSHAKE:
            case AT_SKILL_EARTHSHAKE_STR:
            case AT_SKILL_EARTHSHAKE_MASTERY:
                SetAction(so, PLAYER_ATTACK_DARKHORSE);
                PlayBuffer(SOUND_EARTH_QUAKE);
                break;
            case AT_SKILL_RAGEFUL_BLOW:
            case AT_SKILL_RAGEFUL_BLOW_STR:
            case AT_SKILL_RAGEFUL_BLOW_MASTERY:
                
                SetAction(so, PLAYER_ATTACK_SKILL_FURY_STRIKE);
                break;
            case AT_SKILL_STRIKE_OF_DESTRUCTION:
            case AT_SKILL_STRIKE_OF_DESTRUCTION_STR:
                SetAction(so, PLAYER_SKILL_BLOW_OF_DESTRUCTION);
                break;
            case AT_SKILL_IMPALE:
                if (sc->Helper.Type == MODEL_HORN_OF_FENRIR)
                    SetAction(so, PLAYER_FENRIR_ATTACK_SPEAR);
                else
                    SetAction(so, PLAYER_ATTACK_SKILL_SPEAR);
                break;
            case AT_SKILL_FIRE_SLASH:
            case AT_SKILL_FIRE_SLASH_STR:
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
                switch (sc->Helper.Type)
                {
                case MODEL_HORN_OF_UNIRIA:
                    SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_UNI);
                    break;
                case MODEL_HORN_OF_DINORANT:
                    SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_DINO);
                    break;
                case MODEL_HORN_OF_FENRIR:
                    SetAction(so, PLAYER_ATTACK_SKILL_WHEEL_FENRIR);
                    break;
                default:
                    SetAction(so, PLAYER_ATTACK_SKILL_WHEEL);
                    break;
                }
#else	// YDG_ADD_SKILL_RIDING_ANIMATIONS
                SetAction(so, PLAYER_ATTACK_SKILL_WHEEL);
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS
                break;
            case AT_SKILL_POWER_SLASH:
            case AT_SKILL_POWER_SLASH_STR:
                SetAction(so, PLAYER_ATTACK_TWO_HAND_SWORD_TWO);
                break;
            case AT_SKILL_DEATHSTAB:
            case AT_SKILL_DEATHSTAB_STR:
                SetAction(so, PLAYER_ATTACK_DEATHSTAB);
                break;

            case AT_SKILL_STUN:
                SetAction(so, PLAYER_SKILL_VITALITY);
                break;

            case AT_SKILL_INVISIBLE:
                SetAction(so, PLAYER_SKILL_VITALITY);
                break;

            case AT_SKILL_PLASMA_STORM_FENRIR:
                SetAction_Fenrir_Skill(sc, so);
                break;
            case AT_SKILL_GAOTIC:
            {
                if (sc->Helper.Type == MODEL_HORN_OF_FENRIR)
                {
                    SetAction(&sc->Object, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
                }
                else if ((sc->Helper.Type >= MODEL_HORN_OF_UNIRIA) && (sc->Helper.Type <= MODEL_DARK_HORSE_ITEM))
                {
                    SetAction(&sc->Object, PLAYER_ATTACK_RIDE_STRIKE);
                }
                else
                {
                    SetAction(&sc->Object, PLAYER_ATTACK_STRIKE);
                }

                OBJECT* o = so;
                vec3_t Light, Position, P, dp;

                float Matrix[3][4];
                Vector(0.f, -20.f, 0.f, P);
                Vector(0.f, 0.f, 0.f, P);
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(P, Matrix, dp);
                VectorAdd(dp, o->Position, Position);

                Vector(0.5f, 0.5f, 0.5f, Light);
                for (int i = 0; i < 5; ++i)
                {
                    CreateEffect(BITMAP_SHINY + 6, Position, o->Angle, Light, 3, o, -1, 0, 0, 0, 0.5f);
                }

                VectorCopy(o->Position, Position);

                for (int i = 0; i < 8; i++)
                {
                    Position[0] = (o->Position[0] - 119.f) + (float)(rand() % 240);
                    Position[2] = (o->Position[2] + 49.f) + (float)(rand() % 60);
                    CreateJoint(BITMAP_2LINE_GHOST, Position, o->Position, o->Angle, 0, o, 20.f, o->PKKey, 0, o->m_bySkillSerialNum);
                }
                if (sc == Hero && SelectedCharacter != -1)
                {
                    vec3_t Pos;
                    CHARACTER* st = &CharactersClient[SelectedCharacter];
                    VectorCopy(st->Object.Position, Pos);
                    CreateBomb(Pos, true);
                }
                PlayBuffer(SOUND_SKILL_CAOTIC);
            }
            break;
            case AT_SKILL_MULTI_SHOT:
            {
                SetPlayerBow(sc);
                OBJECT* o = so;
                vec3_t Light, Position, P, dp;

                float Matrix[3][4];
                Vector(0.f, 20.f, 0.f, P);
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(P, Matrix, dp);
                VectorAdd(dp, o->Position, Position);
                Vector(0.8f, 0.9f, 1.6f, Light);
                CreateEffect(MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);
                CreateEffect(MODEL_MULTI_SHOT3, Position, o->Angle, Light, 0);

                Vector(0.f, -20.f, 0.f, P);
                Vector(0.f, 0.f, 0.f, P);
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(P, Matrix, dp);
                VectorAdd(dp, o->Position, Position);

                CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
                CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);
                CreateEffect(MODEL_MULTI_SHOT1, Position, o->Angle, Light, 0);

                Vector(0.f, 20.f, 0.f, P);
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(P, Matrix, dp);
                VectorAdd(dp, o->Position, Position);
                CreateEffect(MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);
                CreateEffect(MODEL_MULTI_SHOT2, Position, o->Angle, Light, 0);

                Vector(0.f, -120.f, 145.f, P);
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(P, Matrix, dp);
                VectorAdd(dp, o->Position, Position);

                short Key = -1;
                for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
                {
                    CHARACTER* tc = &CharactersClient[i];
                    if (tc == sc)
                    {
                        Key = i;
                        break;
                    }
                }

                CreateEffect(MODEL_BLADE_SKILL, Position, o->Angle, Light, 1, o, Key);
            }
            break;
            case AT_SKILL_RECOVER:
            {
                //			OBJECT* o = so;
                vec3_t Light, Position, P, dp;
                vec3_t vFirePosition;

                float Matrix[3][4];

                OBJECT* o = so;

                Vector(0.4f, 0.6f, 1.f, Light);
                Vector(0.f, 0.f, 0.f, P);
                AngleMatrix(o->Angle, Matrix);
                VectorRotate(P, Matrix, dp);
                VectorAdd(dp, o->Position, Position);
                Position[2] += 130;
                VectorCopy(o->Position, Position);
                for (int i = 0; i < 19; ++i)
                {
                    CreateJoint(BITMAP_FLARE, Position, Position, o->Angle, 47, o, 40, 2);
                }
                Vector(0.3f, 0.2f, 0.1f, Light);
                CreateEffect(MODEL_SUMMON, Position, o->Angle, Light, 0);
                CreateEffect(BITMAP_TWLIGHT, Position, o->Angle, Light, 0);
                CreateEffect(BITMAP_TWLIGHT, Position, o->Angle, Light, 1);
                CreateEffect(BITMAP_TWLIGHT, Position, o->Angle, Light, 2);

                for (int i = 0; i < 2; ++i)
                {
                    int iNumBones = Models[o->Type].NumBones;
                    Models[o->Type].TransformByObjectBone(vFirePosition, o, rand() % iNumBones);
                    CreateEffect(BITMAP_FLARE, vFirePosition, o->Angle, Light, 3);
                }
                PlayBuffer(SOUND_SKILL_RECOVER);
            }
            break;
            case AT_SKILL_ALICE_LIGHTNINGORB:
            {
                switch (sc->Helper.Type)
                {
                case MODEL_HORN_OF_UNIRIA:
                    SetAction(so, PLAYER_SKILL_LIGHTNING_ORB_UNI);
                    break;
                case MODEL_HORN_OF_DINORANT:
                    SetAction(so, PLAYER_SKILL_LIGHTNING_ORB_DINO);
                    break;
                case MODEL_HORN_OF_FENRIR:
                    SetAction(so, PLAYER_SKILL_LIGHTNING_ORB_FENRIR);
                    break;
                default:
                    SetAction(so, PLAYER_SKILL_LIGHTNING_ORB);
                    break;
                }
            }
            break;
            case AT_SKILL_ALICE_DRAINLIFE:
            case AT_SKILL_ALICE_DRAINLIFE_STR:
            {
                switch (sc->Helper.Type)
                {
                case MODEL_HORN_OF_UNIRIA:
                    SetAction(so, PLAYER_SKILL_DRAIN_LIFE_UNI);
                    break;
                case MODEL_HORN_OF_DINORANT:
                    SetAction(so, PLAYER_SKILL_DRAIN_LIFE_DINO);
                    break;
                case MODEL_HORN_OF_FENRIR:
                    SetAction(so, PLAYER_SKILL_DRAIN_LIFE_FENRIR);
                    break;
                default:
                    SetAction(so, PLAYER_SKILL_DRAIN_LIFE);
                    break;
                }
            }
            break;
            case AT_SKILL_ALICE_WEAKNESS:
            case AT_SKILL_ALICE_ENERVATION:
                switch (sc->Helper.Type)
                {
                case MODEL_HORN_OF_UNIRIA:
                    SetAction(so, PLAYER_SKILL_SLEEP_UNI);
                    break;
                case MODEL_HORN_OF_DINORANT:
                    SetAction(so, PLAYER_SKILL_SLEEP_DINO);
                    break;
                case MODEL_HORN_OF_FENRIR:
                    SetAction(so, PLAYER_SKILL_SLEEP_FENRIR);
                    break;
                default:
                    SetAction(so, PLAYER_SKILL_SLEEP);
                    break;
                }
                break;

            case AT_SKILL_SUMMON_EXPLOSION:
            case AT_SKILL_SUMMON_REQUIEM:
            case AT_SKILL_SUMMON_POLLUTION:
            {
                g_SummonSystem.CastSummonSkill(MagicNumber, sc, so, Data->PositionX, Data->PositionY);
            }
            break;
            case AT_SKILL_FLAME_STRIKE:
                SetAction(so, PLAYER_SKILL_FLAMESTRIKE);
                break;
            case AT_SKILL_GIGANTIC_STORM:
                SetAction(so, PLAYER_SKILL_GIGANTICSTORM);
                break;

            case AT_SKILL_LIGHTNING_SHOCK:
            case AT_SKILL_LIGHTNING_SHOCK_STR:
                SetAction(so, PLAYER_SKILL_LIGHTNING_SHOCK);
                break;
            case AT_SKILL_DRAGON_ROAR:
            case AT_SKILL_DRAGON_ROAR_STR:
            {
                if (sc != Hero)
                    g_CMonkSystem.RageCreateEffect(so, MagicNumber);

                g_CMonkSystem.SetRageSkillAni(MagicNumber, so);
            }
            break;
            default:
                SetPlayerMagic(sc);
                break;
            }
            so->AnimationFrame = 0;
        }
    }
    else
    {
        SetPlayerAttack(sc);
        so->AnimationFrame = 0;
    }
    sc->AttackTime = 1;
    sc->TargetCharacter = -1;
    sc->AttackFlag = ATTACK_FAIL;
    sc->SkillX = Data->PositionX;
    sc->SkillY = Data->PositionY;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x1E [ReceiveMagicContinue(%d)]", MagicNumber);

    return (TRUE);
}

// ChainLightning
void ReceiveChainMagic(const BYTE* ReceiveBuffer)
{
    auto pPacketData = (LPPRECEIVE_CHAIN_MAGIC)ReceiveBuffer;

    CHARACTER* pSourceChar = &CharactersClient[FindCharacterIndex(pPacketData->wUserIndex)];
    OBJECT* pSourceObject = &pSourceChar->Object;
    OBJECT* pTempObject = nullptr;

    //SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING);

    switch (pSourceChar->Helper.Type)
    {
    case MODEL_HORN_OF_UNIRIA:
        SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING_UNI);
        break;
    case MODEL_HORN_OF_DINORANT:
        SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING_DINO);
        break;
    case MODEL_HORN_OF_FENRIR:
        SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING_FENRIR);
        break;
    default:
        SetAction(pSourceObject, PLAYER_SKILL_CHAIN_LIGHTNING);
        break;
    }

    pSourceChar->Skill = ((WORD)(pPacketData->MagicH) << 8) + pPacketData->MagicL;

    pTempObject = pSourceObject;

    int iOffset = sizeof(PRECEIVE_CHAIN_MAGIC);
    for (int i = 0; i < (int)(pPacketData->byCount); i++)
    {
        auto pPacketData2 = (LPPRECEIVE_CHAIN_MAGIC_OBJECT)(ReceiveBuffer + iOffset);
        CHARACTER* pTargetChar = &CharactersClient[FindCharacterIndex(pPacketData2->wTargetIndex)];
        OBJECT* pTargetObject = &pTargetChar->Object;

        if (pTempObject != pTargetObject && pTargetObject != nullptr && pTargetObject->Live == true)
        {
            vec3_t vAngle;
            Vector(-60.f, 0.f, pSourceObject->Angle[2], vAngle);

            //SetPlayerStop(pTargetChar);

            CreateEffect(MODEL_CHAIN_LIGHTNING, pSourceObject->Position, vAngle, pSourceObject->Light, i,
                pTempObject, -1, 0, 0, 0, 0.f, pPacketData2->wTargetIndex);
        }

        pTempObject = pTargetObject;

        iOffset += sizeof(PRECEIVE_CHAIN_MAGIC_OBJECT);
    }
}

void ReceiveMagicPosition(const BYTE* ReceiveBuffer, int Size)
{
    auto Data = (LPPRECEIVE_MAGIC_POSITIONS)ReceiveBuffer;
    int SourceKey = ((int)(Data->KeyH) << 8) + Data->KeyL;
    WORD MagicNumber = ((WORD)(Data->MagicH) << 8) + Data->MagicL;
    int Index = FindCharacterIndex(SourceKey);
    AttackPlayer = Index;

    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;
    CreateMagicShiny(c);
    PlayBuffer(SOUND_MAGIC);
    SetAction(o, PLAYER_SKILL_HELL);
    c->Skill = MagicNumber;
    c->AttackTime = 1;

    int Offset = sizeof(PRECEIVE_MAGIC_POSITIONS);
    for (int i = 0; i < Data->Count; i++)
    {
        auto Data2 = (LPPRECEIVE_MAGIC_POSITION)(ReceiveBuffer + Offset);
        int TargetKey = ((int)(Data2->KeyH) << 8) + Data2->KeyL;
        CHARACTER* tc = &CharactersClient[FindCharacterIndex(TargetKey)];
        OBJECT* to = &tc->Object;
        if (rand_fps_check(2))
            SetPlayerShock(tc, tc->Hit);
        if (tc->Hit > 0)
        {
            vec3_t Light;
            Vector(1.f, 0.6f, 0.f, Light);
            CreatePoint(to->Position, tc->Hit, Light);
            if (TargetKey == HeroKey)
            {
                if (tc->Hit >= CharacterAttribute->Life)
                    CharacterAttribute->Life = 0;
                else
                    CharacterAttribute->Life -= tc->Hit;
            }
        }
        Offset += sizeof(PRECEIVE_MAGIC_POSITION);
    }
}

void ReceiveSkillCount(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_EX_SKILL_COUNT)ReceiveBuffer;
    int TargetKey = ((int)(Data->KeyH) << 8) + Data->KeyL;
    CHARACTER* tc = &CharactersClient[FindCharacterIndex(TargetKey)];
    OBJECT* to = &tc->Object;

    switch (Data->m_byType)
    {
    case AT_SKILL_NOVA_BEGIN:
        break;

    case AT_SKILL_NOVA:
        to->m_bySkillCount = Data->m_byCount;
        break;
    }
}

BOOL ReceiveDieExp(const BYTE* ReceiveBuffer, BOOL bEncrypted)
{
    auto Data = (LPPRECEIVE_DIE)ReceiveBuffer;
    int     Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    DWORD   Exp = ((DWORD)(Data->ExpH) << 8) + Data->ExpL;
    int     Damage = ((int)(Data->DamageH) << 8) + Data->DamageL;
    int     Success = (Key >> 15);
    Key &= 0x7FFF;

    int Index = FindCharacterIndex(Key);
    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;
    vec3_t Light;
    Vector(1.f, 0.6f, 0.f, Light);
    if (Success)
    {
        SetPlayerDie(c);
        CreatePoint(o->Position, Damage, Light);
    }
    else
    {
        Hero->AttackFlag = ATTACK_DIE;
        Hero->Damage = Damage;
        Hero->TargetCharacter = Index;
        CreatePoint(o->Position, Damage, Light);
    }
    c->Dead = 1;
    c->Movement = false;

    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        g_pMainFrame->SetPreExp_Wide(Master_Level_Data.lMasterLevel_Experince);
        g_pMainFrame->SetGetExp_Wide(Exp);
        Master_Level_Data.lMasterLevel_Experince += Exp;
    }
    else
    {
        g_pMainFrame->SetPreExp(CharacterAttribute->Experience & 0xFFFFFFFF);
        g_pMainFrame->SetGetExp(Exp);
        CharacterAttribute->Experience += Exp;
    }

    if (Exp > 0)
    {
        wchar_t Text[100];
        if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
        {
            swprintf(Text, GlobalText[1750], Exp);
        }
        else
            swprintf(Text, GlobalText[486], Exp);
        g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

#ifdef CONSOLE_DEBUG
    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x16 [ReceiveDieExp : %d]", Exp);
#endif // CONSOLE_DEBUG

    return (TRUE);
}

BOOL ReceiveDieExpLarge(const BYTE* ReceiveBuffer, BOOL bEncrypted)
{
    auto Data = (LPPRECEIVE_EXP_EXTENDED)ReceiveBuffer;

    auto addedExperience = Data->AddedExperience;
    auto damageOfLastHit = Data->DamageOfLastHit;
    auto experienceType = Data->ExperienceType;
    auto killedId = Data->KilledObjectId;
    auto killerId = Data->KillerObjectId;

    int Index = FindCharacterIndex(killedId);
    CHARACTER* killedObject = &CharactersClient[Index];
    OBJECT* o = &killedObject->Object;
    vec3_t Light;
    Vector(1.f, 0.6f, 0.f, Light);
    if (Hero->Key == killerId)
    {
        Hero->AttackFlag = ATTACK_DIE;
        Hero->Damage = damageOfLastHit;
        Hero->TargetCharacter = Index;
    }
    else
    {
        SetPlayerDie(killedObject);
        
    }

    if (damageOfLastHit > 0)
    {
        CreatePoint(o->Position, damageOfLastHit, Light);
    }

    killedObject->Dead = 1;
    killedObject->Movement = false;

    switch(experienceType)
    {
    case eExperienceType_MaxLevelReached:
        // TODO: show message "You already reached maximum Level."
        g_pSystemLogBox->AddText(L"You already reached maximum Level.", SEASON3B::TYPE_SYSTEM_MESSAGE);
        return TRUE;
    case eExperienceType_MaxMasterLevelReached:
        // TODO: show message "You already reached maximum master Level."
        g_pSystemLogBox->AddText(L"You already reached maximum master Level.", SEASON3B::TYPE_SYSTEM_MESSAGE);
        return TRUE;
    case eExperienceType_MonsterLevelTooLowForMasterExperience:
        // TODO: You need to kill stronger monsters to gain master experience.
        g_pSystemLogBox->AddText(L"You need to kill stronger monsters to gain master experience.", SEASON3B::TYPE_SYSTEM_MESSAGE);
        return TRUE;
    }

    if (addedExperience == 0)
    {
        return TRUE;
    }

    if (experienceType == eExperienceType_Master)
    {
        g_pMainFrame->SetPreExp_Wide(Master_Level_Data.lMasterLevel_Experince);
        g_pMainFrame->SetGetExp_Wide(addedExperience);
        Master_Level_Data.lMasterLevel_Experince += addedExperience;
    }
    else
    {
        g_pMainFrame->SetPreExp(CharacterAttribute->Experience & 0xFFFFFFFF);
        g_pMainFrame->SetGetExp(addedExperience);
        CharacterAttribute->Experience += addedExperience;
    }

    if (addedExperience > 0)
    {
        wchar_t Text[100];

        if (experienceType == eExperienceType_Master)
        {
            swprintf(Text, GlobalText[1750], addedExperience);
        }
        else
        {
            swprintf(Text, GlobalText[486], addedExperience);
        }

        g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    return TRUE;
}

void FallingStartCharacter(CHARACTER* c, OBJECT* o)
{
    auto positionX = (BYTE)(o->Position[0] / TERRAIN_SCALE);
    auto positionY = (BYTE)(o->Position[1] / TERRAIN_SCALE);
    int WallIndex = TERRAIN_INDEX_REPEAT(positionX, positionY);
    int Wall = TerrainWall[WallIndex] & TW_ACTION;

    o->m_bActionStart = false;

    if (gMapManager.InChaosCastle() == true && (TerrainWall[WallIndex] & TW_NOGROUND) == TW_NOGROUND)
    {
        c->StormTime = 0;
        o->m_bActionStart = true;
        o->Gravity = rand() % 10 + 10.f;
        o->Velocity -= rand() % 3 + 3.f;

        Vector(0.f, 0.f, 1.f, o->Direction);
        VectorCopy(o->Position, o->m_vDeadPosition);

        PlayBuffer(SOUND_CHAOS_FALLING);
    }
    else if (Wall == TW_ACTION)
    {
        c->StormTime = 0;
        o->m_bActionStart = true;
        o->Gravity = rand() % 10 + 10.f;
        o->Velocity = rand() % 20 + 20.f;
        float angle = rand() % 10 + 85.f;

        if ((TerrainWall[WallIndex + 1] & TW_NOGROUND) == TW_NOGROUND)
        {
            Vector(0.f, 0.f, -angle, o->m_vDownAngle);
        }
        else if ((TerrainWall[WallIndex - 1] & TW_NOGROUND) == TW_NOGROUND)
        {
            Vector(0.f, 0.f, angle, o->m_vDownAngle);
        }

        o->Angle[2] = o->m_vDownAngle[2];
        Vector(rand() % 6 + 8.f, -rand() % 2 + 13.f, 5.f, o->Direction);
        VectorCopy(o->Position, o->m_vDeadPosition);
    }

    if (c == Hero)
    {
        matchEvent::ClearMatchInfo();
    }
}

void ReceiveDie(const BYTE* ReceiveBuffer, int Size)
{
    auto Data = (LPPHEADER_DEFAULT_DIE)ReceiveBuffer;

    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;

    int Index = FindCharacterIndex(Key);

    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;
    c->Dead = 1;
    c->Movement = false;

    WORD SkillType = ((int)(Data->MagicH) << 8) + Data->MagicL;

    c->m_byDieType = SkillType;

    if (gMapManager.InBloodCastle() == true)
    {
        FallingStartCharacter(c, o);
    }
    else if (gMapManager.InChaosCastle() == true)
    {
        FallingStartCharacter(c, o);
    }
    else
    {
        if (c == Hero && g_PortalMgr.IsPortalUsable())
        {
            g_PortalMgr.SaveRevivePosition();
        }

        o->m_bActionStart = false;

        switch (SkillType)
        {
        case AT_SKILL_NOVA:
        case AT_SKILL_COMBO:
            o->m_bActionStart = true;
            c->StormTime = 0;
            o->Velocity = (rand() % 5 + 10.f) * 0.1f;
            o->m_bySkillCount = 0;

            Vector(0.f, rand() % 15 + 40.f, 0.f, o->Direction);
            VectorCopy(o->Position, o->m_vDeadPosition);
            break;
        }

        if (SkillType == AT_SKILL_NOVA || SkillType == AT_SKILL_COMBO)
        {
            int TKey = ((int)(Data->TKeyH) << 8) + Data->TKeyL;
            int TIndex = FindCharacterIndex(TKey);
            CHARACTER* tc = &CharactersClient[TIndex];
            OBJECT* to = &tc->Object;

            o->Angle[2] = CreateAngle2D(o->Position, to->Position);

            VectorCopy(o->Angle, o->HeadAngle);
        }
    }

    if (c == Hero)
    {
        MUHelper::g_MuHelper.TriggerStop();
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x17 [ReceiveDie(%d)]", Key);
}

void ReceiveCreateMoney(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PCREATE_MONEY>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }
    
    if (Data->Id < 0 || Data->Id >= MAX_ITEMS)
    {
        // we don't have a free place for it ...
        return;
    }

    vec3_t Position;
    Position[0] = (float)(Data->PositionX + 0.5f)* TERRAIN_SCALE;
    Position[1] = (float)(Data->PositionY + 0.5f) * TERRAIN_SCALE;

    CreateMoneyDrop(&Items[Data->Id], Data->Amount, Position, Data->IsFreshDrop);
    MUHelper::g_MuHelper.AddItem(Data->Id, { Data->PositionX, Data->PositionY });

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x20 [ReceiveCreateMoney]");
}

void ReceiveCreateItemViewportExtended(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PWHEADER_DEFAULT_WORD>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    int Offset = sizeof(PWHEADER_DEFAULT_WORD);
    for (int i = 0; i < Data->Value; i++)
    {
        auto itemStartData = safe_cast<PCREATE_ITEM_EXTENDED>(ReceiveBuffer.subspan(Offset));
        if (itemStartData == nullptr)
        {
            assert(false);
            return;
        }

        auto id = MAKEWORD(itemStartData->IdH, itemStartData->IdL) & 0x7FFF;
        auto isFreshDrop = (itemStartData->IdL & 0x80) > 0;
        if (id < 0 || id >= MAX_ITEMS)
        {
            // we don't have a free place for it ...
            continue;
        }

        Offset += 4;
        auto itemData = ReceiveBuffer.subspan(Offset);
        int length = CalcItemLength(itemData);
        itemData = itemData.subspan(0, length);

        auto params = ParseItemData(itemData);
        vec3_t Position;
        Position[0] = (float)(itemStartData->PositionX + 0.5f) * TERRAIN_SCALE;
        Position[1] = (float)(itemStartData->PositionY + 0.5f) * TERRAIN_SCALE;

        CreateItemDrop(&Items[id], params, Position, isFreshDrop);
        MUHelper::g_MuHelper.AddItem(id, { itemStartData->PositionX, itemStartData->PositionY });

        Offset += length;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x20 [ReceiveCreateItemViewport]");
}

void ReceiveDeleteItemViewport(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
    int Offset = sizeof(PWHEADER_DEFAULT_WORD);
    for (int i = 0; i < Data->Value; i++)
    {
        auto Data2 = (LPPDELETE_CHARACTER)(ReceiveBuffer + Offset);
        int Key = ((int)(Data2->KeyH) << 8) + Data2->KeyL;
        if (Key < 0 || Key >= MAX_ITEMS)
            Key = 0;
        Items[Key].Object.Live = false;
        Offset += sizeof(PDELETE_CHARACTER);

        MUHelper::g_MuHelper.DeleteItem(Key);
    }
}

static  const   BYTE    NOT_GET_ITEM = 0xff;
static  const   BYTE    GET_ITEM_ZEN = 0xfe;
static  const   BYTE    GET_ITEM_MULTI = 0xfd; // received when item was added in a stack
extern int ItemKey;
void ReceiveGetItem(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PHEADER_DEFAULT>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }
    
    if (Data->Value == NOT_GET_ITEM)
    {
    }
    else
    {
        if (Data->Value == GET_ITEM_ZEN)
        {
            auto Data2 = safe_cast<PRECEIVE_INVENTORY_MONEY>(ReceiveBuffer);
            if (Data2 == nullptr)
            {
                assert(false);
                return;
            }

            wchar_t szMessage[128];
            int backupGold = CharacterMachine->Gold;
            CharacterMachine->Gold = (Data2->Money[0] << 24) + (Data2->Money[1] << 16) + (Data2->Money[2] << 8) + (Data2->Money[3]);

            int getGold = CharacterMachine->Gold - backupGold;

            if (getGold > 0)
            {
                swprintf(szMessage, L"%d %s %s", getGold, GlobalText[224], GlobalText[918]);
                g_pSystemLogBox->AddText(szMessage, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
        }
        else
        {
            auto pickedItem = &Items[ItemKey].Item;
            auto itemIndex = Data->Value;
            if (itemIndex != GET_ITEM_MULTI)
            {
                auto Data2 = safe_cast<PRECEIVE_GET_ITEM_EXTENDED>(ReceiveBuffer);
                if (Data2 == nullptr)
                {
                    assert(false);
                    return;
                }

                auto offset = sizeof(PBMSG_HEADER) + 1;
                auto itemData = ReceiveBuffer.subspan(offset);
                int length = CalcItemLength(itemData);
                itemData = itemData.subspan(0, length);

                if (itemIndex >= MAX_EQUIPMENT_INDEX && itemIndex < MAX_MY_INVENTORY_INDEX)
                {
                    if (g_pMyInventory->InsertItem(itemIndex, itemData))
                    {
                        pickedItem = g_pMyInventory->FindItem(itemIndex);
                    }
                }
                else if (itemIndex >= MAX_MY_INVENTORY_INDEX && Data2->Result < MAX_MY_INVENTORY_EX_INDEX)
                {
                    if (g_pMyInventoryExt->InsertItem(itemIndex, itemData))
                    {
                        pickedItem = g_pMyInventoryExt->FindItem(itemIndex);
                    }
                }
            }

            wchar_t szItem[64] = { 0, };
            int level = pickedItem->Level;
            GetItemName(pickedItem->Type, level, szItem);

            wchar_t szMessage[128];
            swprintf(szMessage, L"%s %s", szItem, GlobalText[918]);
            g_pSystemLogBox->AddText(szMessage, SEASON3B::TYPE_SYSTEM_MESSAGE);

            int Type = pickedItem->Type;
            if (Type == ITEM_JEWEL_OF_BLESS || Type == ITEM_JEWEL_OF_SOUL || Type == ITEM_JEWEL_OF_LIFE || Type == ITEM_JEWEL_OF_CHAOS || Type == ITEM_JEWEL_OF_CREATION
                || Type == INDEX_COMPILED_CELE || Type == INDEX_COMPILED_SOUL || Type == ITEM_JEWEL_OF_GUARDIAN)
                PlayBuffer(SOUND_JEWEL01, &Hero->Object);
            else if (Type == ITEM_GEMSTONE)
                PlayBuffer(SOUND_JEWEL02, &Hero->Object);
            else
                PlayBuffer(SOUND_GET_ITEM01, &Hero->Object);
        }
#ifdef FOR_WORK
        Items[ItemKey].Object.Live = false;
#endif
    }
    SendGetItem = -1;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x22 [ReceiveGetItem(%d)]", Data->Value);
}

void ReceiveDropItem(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_KEY)ReceiveBuffer;
    if (Data->KeyH)
    {
        if (Data->KeyL < 12)
        {
            g_pMyInventory->UnequipItem(Data->KeyL);
        }
        else
        {
            g_pMyInventory->DeleteItem(Data->KeyL);
        }

        SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
    }
    else
    {
        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
    }

    SendDropItem = -1;
}

BOOL g_bPacketAfter_EquipmentItem = FALSE;
BYTE g_byPacketAfter_EquipmentItem[256];

void ReceiveTradeExit(const BYTE* ReceiveBuffer);

BOOL ReceiveEquipmentItemExtended(std::span<const BYTE> ReceiveBuffer)
{
    EquipmentItem = false;
    auto Data = safe_cast<PHEADER_DEFAULT_SUBCODE_ITEM_EXTENDED>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return false;
    }

    auto Offset = sizeof(PBMSG_HEADER) + 2;
    auto itemData = ReceiveBuffer.subspan(Offset);
    int length = CalcItemLength(itemData);
    itemData = itemData.subspan(0, length);

    if (Data->SubCode != 255)
    {
        const auto storageType = static_cast<STORAGE_TYPE>(Data->SubCode);
        SEASON3B::CNewUIPickedItem* pPickedItem = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();
        int iSourceIndex = g_pMyShopInventory->GetSourceIndex();
        if (pPickedItem)
        {
            iSourceIndex = pPickedItem->GetSourceLinealPos();
        }

        if (iSourceIndex >= MAX_MY_INVENTORY_EX_INDEX)
        {
            int price = 0;
            if (GetPersonalItemPrice(iSourceIndex, price, g_IsPurchaseShop))
            {
                RemovePersonalItemPrice(iSourceIndex, g_IsPurchaseShop);
                if (Data->Index >= MAX_MY_INVENTORY_EX_INDEX)
                {
                    AddPersonalItemPrice(Data->Index, price, g_IsPurchaseShop);
                }
            }
        }

        if (storageType == STORAGE_TYPE::INVENTORY)
        {
            SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

            int itemindex = Data->Index;

            if (itemindex >= 0 && itemindex < MAX_EQUIPMENT_INDEX)
            {
                g_pMyInventory->EquipItem(itemindex, itemData);
            }
            else if (itemindex >= MAX_EQUIPMENT_INDEX && itemindex < MAX_MY_INVENTORY_INDEX)
            {
                g_pStorageInventory->ProcessStorageItemAutoMoveSuccess();
                g_pStorageInventoryExt->ProcessStorageItemAutoMoveSuccess();
                g_pMyInventory->InsertItem(itemindex, itemData);
            }
            else if (itemindex >= MAX_MY_INVENTORY_INDEX && itemindex < MAX_MY_INVENTORY_EX_INDEX)
            {
                g_pStorageInventory->ProcessStorageItemAutoMoveSuccess();
                g_pStorageInventoryExt->ProcessStorageItemAutoMoveSuccess();
                g_pMyInventoryExt->InsertItem(itemindex, itemData);
            }
            else if (itemindex >= MAX_MY_INVENTORY_EX_INDEX && itemindex < MAX_MY_SHOP_INVENTORY_INDEX)
            {
                g_pMyShopInventory->InsertItem(itemindex, itemData);
            }
        }
        else if (storageType == STORAGE_TYPE::TRADE)
        {
            g_pTrade->ProcessToReceiveTradeItems(Data->Index, itemData);
        }
        else if (storageType == STORAGE_TYPE::VAULT)
        {
            if (Data->Index < MAX_SHOP_INVENTORY)
            {
                g_pStorageInventory->ProcessToReceiveStorageItems(Data->Index, itemData);
            }
            else
            {
                g_pStorageInventoryExt->ProcessToReceiveStorageItems(Data->Index, itemData);
            }
        }
        if (storageType == STORAGE_TYPE::CHAOS_MIX
            || (storageType >= STORAGE_TYPE::TRAINER_MIX && storageType <= STORAGE_TYPE::DETACH_SOCKET_MIX))
        {
            SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
            if (Data->Index >= 0 && Data->Index < MAX_MIX_INVENTORY)
                g_pMixInventory->InsertItem(Data->Index, itemData);
        }
        else if (storageType == STORAGE_TYPE::LUCKYITEM_TRADE || storageType == STORAGE_TYPE::LUCKYITEM_REFINERY)
        {
            g_pLuckyItemWnd->GetResult(1, Data->Index, itemData);
        }

        PlayBuffer(SOUND_GET_ITEM01);
    }
    else
    {
        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
        if (g_pStorageInventory->IsItemAutoMove())
        {
            g_pStorageInventory->ProcessStorageItemAutoMoveFailure();
        }

        if (g_pStorageInventoryExt->IsItemAutoMove())
        {
            g_pStorageInventoryExt->ProcessStorageItemAutoMoveFailure();
        }
    }

    if (g_bPacketAfter_EquipmentItem)
    {
        ReceiveTradeExit(g_byPacketAfter_EquipmentItem);
        g_bPacketAfter_EquipmentItem = FALSE;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x24 [ReceiveEquipmentItem(%d %d)]", Data->SubCode, Data->Index);

    return (TRUE);
}


void ReceiveModifyItemExtended(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PHEADER_DEFAULT_SUBCODE_ITEM_EXTENDED>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    auto Offset = sizeof(PBMSG_HEADER) + 2;
    auto itemData = ReceiveBuffer.subspan(Offset);
    int length = CalcItemLength(itemData);
    itemData = itemData.subspan(0, length);

    if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
    {
        SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
    }

    int itemindex = Data->Index;
    if (g_pMyInventory->FindItem(itemindex))
    {
        g_pMyInventory->DeleteItem(itemindex);
    }

    if (itemindex >= MAX_EQUIPMENT_INDEX && itemindex < MAX_MY_INVENTORY_INDEX)
    {
        g_pMyInventory->InsertItem(itemindex, itemData);
    }
    else if (itemindex > MAX_MY_INVENTORY_INDEX && itemindex < MAX_MY_INVENTORY_EX_INDEX)
    {
        g_pMyInventoryExt->InsertItem(itemindex, itemData);
    }

    int iType = Items[itemindex].Item.Type;
    if (iType == ITEM_LOST_MAP || iType == ITEM_POTION + 111)
    {
        PlayBuffer(SOUND_KUNDUN_ITEM_SOUND);
    }
    else if (GambleSystem::Instance().IsGambleShop())
    {
    }
    else
    {
        PlayBuffer(SOUND_JEWEL01);
    }
}

BOOL ReceiveTalk(const BYTE* ReceiveBuffer, BOOL bEncrypted)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;

    g_pNewUISystem->HideAll();

    switch (Data->Value)
    {
    case 2:
        g_pNewUISystem->Show(SEASON3B::INTERFACE_STORAGE);
        break;

    case 3:
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_GOBLIN_NORMAL);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
        //BYTE *pbyChaosRate = ( &Data->Value) + 1;
        //int iDummyRate[6];	// 광장표 확률을 서버에서 받으나 사용하지 않고 버림
        //for ( int i = 0; i < 6; ++i)
        //	iDummyRate[i] = ( int)pbyChaosRate[i];	// 광장표 확률을 서버에서 받으나 사용하지 않고 버림(스크립트사용)
        break;

    case 4:
        g_pNewUISystem->Show(SEASON3B::INTERFACE_DEVILSQUARE);
        break;

    case 5:
        g_pUIManager->Open(::INTERFACE_SERVERDIVISION);
        break;

    case 6:
        g_pNewUISystem->Show(SEASON3B::INTERFACE_BLOODCASTLE);
        break;

    case 7:
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CTrainerMenuMsgBoxLayout));
        break;

    case INDEX_NPC_LAHAP:
    {
        if (COMGEM::isAble())
        {
            g_pNewUISystem->HideAll();
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationMsgBoxLayout));
        }
    }
    break;

    case 0x0C:
        g_pNewUISystem->Show(SEASON3B::INTERFACE_SENATUS);
        break;

    case 0x0D:
        SocketClient->ToGameServer()->SendCastleSiegeStatusRequest();
        break;
    case 0x11:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CElpisMsgBoxLayout));
    }
    break;
    case 0x12:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::COsbourneMsgBoxLayout));
        // 			BYTE *pbyChaosRate = ( &Data->Value) + 1;
        // 			g_pUIJewelHarmony->SetMixSuccessRate(pbyChaosRate);
    }
    break;
    case 0x13:
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_JERRIDON);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
        // 			BYTE *pbyChaosRate = ( &Data->Value) + 1;
        // 			g_pUIJewelHarmony->SetMixSuccessRate(pbyChaosRate);
    }
    break;
    case 0x14:
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_CURSEDTEMPLE_NPC);

        BYTE* cursedtempleenterinfo = (&Data->Value) + 1;
        g_pCursedTempleEnterWindow->SetCursedTempleEnterInfo(cursedtempleenterinfo);
    }
    break;
    case 0x15:
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_CHAOS_CARD);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
    }
    break;
    case 0x16:
    {
        g_MixRecipeMgr.SetMixType(SEASON3A::MIXTYPE_CHERRYBLOSSOM);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_MIXINVENTORY);
    }
    break;
    case 0x17:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSeedMasterMenuMsgBoxLayout));
    }
    break;
    case 0x18:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSeedInvestigatorMenuMsgBoxLayout));
    }
    break;
    case 0x19:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CResetCharacterPointMsgBoxLayout));
    }
    break;
    case 0x20:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDelgardoMainMenuMsgBoxLayout));
    }
    break;
    case 0x21:
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_DUELWATCH);
    }
    break;
    case 0x22:
    {
        GambleSystem::Instance().SetGambleShop();
        g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCSHOP);
    }
    break;
    case 0x23:
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_DOPPELGANGER_NPC);
        BYTE* pbtRemainTime = (&Data->Value) + 1;
        g_pDoppelGangerWindow->SetRemainTime(*pbtRemainTime);
    }
    break;
    case 0x24:
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
    }
    break;
    case 0x25:
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
    }
    break;
    case 0x26:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CLuckyTradeMenuMsgBoxLayout));
    }
    break;
    default:
    {
        // Data->Value
        g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCSHOP);
    }
    break;
    }
    PlayBuffer(SOUND_CLICK01);
    PlayBuffer(SOUND_INTERFACE01);
#ifndef FOR_WORK
#ifdef WINDOWMODE
    if (g_bUseWindowMode == FALSE)
    {
#endif	// WINDOWMODE
        int x = 260 * MouseX / 640;
        SetCursorPos((x)*WindowWidth / 640, (MouseY)*WindowHeight / 480);
#ifdef WINDOWMODE
    }
#endif	// WINDOWMODE
#endif	// FOR_WORK

    return (TRUE);
}
/*
void ReceiveBuy(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_ITEM)ReceiveBuffer;
    if (Data->Index != 255)
    {
        if (Data->Index >= MAX_EQUIPMENT_INDEX && Data->Index < MAX_MY_INVENTORY_INDEX)
        {
            g_pMyInventory->InsertItem(Data->Index, Data->Item, Old);
        }
        else if (Data->Index >= MAX_MY_INVENTORY_INDEX && Data->Index < MAX_MY_INVENTORY_EX_INDEX)
        {
            g_pMyInventoryExt->InsertItem(Data->Index, Data->Item, Old);
        }
        else
        {
#ifdef _DEBUG
            __asm { int 3 };
#endif // _DEBUG
        }

        PlayBuffer(SOUND_GET_ITEM01);
    }
    if (Data->Index == 0xfe)
    {
        g_pNewUISystem->HideAll();

        g_pChatListBox->AddText(Hero->ID, GlobalText[732], SEASON3B::TYPE_ERROR_MESSAGE);
    }
    BuyCost = 0;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x32 [ReceiveBuy(%d)]", Data->Index);
}*/

void ReceiveBuyExtended(const std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PHEADER_DEFAULT_ITEM_EXTENDED_HEAD>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    constexpr BYTE BUY_FAILED = 0xFE;
    constexpr BYTE BUY_FAILED_SILENT = 0xFF;

    auto Offset = sizeof(PHEADER_DEFAULT_ITEM_EXTENDED_HEAD);
    auto itemData = ReceiveBuffer.subspan(Offset);
    if (!itemData.empty())
    {
        int length = CalcItemLength(itemData);
        itemData = itemData.subspan(0, length);
    }

    if (Data->Index == BUY_FAILED)
    {
        g_pNewUISystem->HideAll();
        g_pChatListBox->AddText(Hero->ID, GlobalText[732], SEASON3B::TYPE_ERROR_MESSAGE);
    }
    else if (Data->Index == BUY_FAILED_SILENT)
    {
        // do nothing, error message is sent separately.
    }
    else
    {
        if (Data->Index >= MAX_EQUIPMENT_INDEX && Data->Index < MAX_MY_INVENTORY_INDEX)
        {
            g_pMyInventory->InsertItem(Data->Index, itemData);
        }
        else if (Data->Index >= MAX_MY_INVENTORY_INDEX && Data->Index < MAX_MY_INVENTORY_EX_INDEX)
        {
            g_pMyInventoryExt->InsertItem(Data->Index, itemData);
        }

        PlayBuffer(SOUND_GET_ITEM01);
    }

    BuyCost = 0;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x32 [ReceiveBuy(%d)]", Data->Index);
}

void ReceiveTradeYourInventoryExtended(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PHEADER_DEFAULT_ITEM_EXTENDED>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    auto Offset = sizeof(PBMSG_HEADER) + 1;
    auto itemData = ReceiveBuffer.subspan(Offset);
    int length = CalcItemLength(itemData);
    itemData = itemData.subspan(0, length);

    g_pTrade->ProcessToReceiveYourItemAdd(Data->Index, itemData);
}

void ReceiveMixExtended(std::span<const BYTE> ReceiveBuffer)
{
    auto Data = safe_cast<PHEADER_DEFAULT_ITEM_EXTENDED>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    auto Offset = sizeof(PBMSG_HEADER) + 1;
    auto itemData = ReceiveBuffer.subspan(Offset);
    int length = CalcItemLength(itemData);
    itemData = itemData.subspan(0, length);

    switch (Data->Index)
    {
    case 0:
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) && g_pLuckyItemWnd->GetAct())
        {
            std::span<const BYTE> empty = {};
            g_pLuckyItemWnd->GetResult(0, Data->Index, empty);
            break;
        }
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
        wchar_t szText[256] = { 0, };
        switch (g_MixRecipeMgr.GetMixInventoryType())
        {
        case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
        case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
        case SEASON3A::MIXTYPE_GOBLIN_ADD380:
        case SEASON3A::MIXTYPE_EXTRACT_SEED:
        case SEASON3A::MIXTYPE_SEED_SPHERE:
            swprintf(szText, GlobalText[594]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
            break;
            // 			case SEASON3A::MIXTYPE_TRAINER:
            // 				wprintf(szText, GlobalText[1208]);	// 부활 실패
            // 				g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
            // 				break;
        case SEASON3A::MIXTYPE_OSBOURNE:
            swprintf(szText, GlobalText[2105], GlobalText[2061]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
            break;
        case SEASON3A::MIXTYPE_JERRIDON:
            swprintf(szText, GlobalText[2105], GlobalText[2062]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
            break;
        case SEASON3A::MIXTYPE_ELPIS:
            swprintf(szText, GlobalText[2112], GlobalText[2063]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
            break;
        case SEASON3A::MIXTYPE_CHAOS_CARD:
            swprintf(szText, GlobalText[2112], GlobalText[2265]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
            break;
        case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
            swprintf(szText, GlobalText[2112], GlobalText[2560]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
            break;
        }
    }
    break;
    case 1:
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) && g_pLuckyItemWnd->GetAct())
        {
            g_pLuckyItemWnd->GetResult(1, 0, itemData);
            break;
        }
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
        wchar_t szText[256] = { 0, };
        switch (g_MixRecipeMgr.GetMixInventoryType())
        {
        case SEASON3A::MIXTYPE_GOBLIN_NORMAL:
        case SEASON3A::MIXTYPE_GOBLIN_CHAOSITEM:
        case SEASON3A::MIXTYPE_GOBLIN_ADD380:
        case SEASON3A::MIXTYPE_EXTRACT_SEED:
        case SEASON3A::MIXTYPE_SEED_SPHERE:
            swprintf(szText, GlobalText[595]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
            break;
            // 			case SEASON3A::MIXTYPE_TRAINER:
            // 				wprintf(szText, GlobalText[1209]);
            // 				g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
            // 				break;
        case SEASON3A::MIXTYPE_OSBOURNE:
            swprintf(szText, GlobalText[2106], GlobalText[2061]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
            break;
        case SEASON3A::MIXTYPE_JERRIDON:
            swprintf(szText, GlobalText[2106], GlobalText[2062]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
            break;
        case SEASON3A::MIXTYPE_ELPIS:
            swprintf(szText, GlobalText[2113], GlobalText[2063]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
            break;
        case SEASON3A::MIXTYPE_CHAOS_CARD:
            swprintf(szText, GlobalText[2113], GlobalText[2265]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
            break;
        case SEASON3A::MIXTYPE_CHERRYBLOSSOM:
            swprintf(szText, GlobalText[2113], GlobalText[2560]);
            g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
            break;
        }

        g_pMixInventory->DeleteAllItems();
        g_pMixInventory->InsertItem(0, itemData);

        PlayBuffer(SOUND_MIX01);
        PlayBuffer(SOUND_JEWEL01);
    }
    break;
    case 2:
    case 0x0B:
    {
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_READY);
        g_pSystemLogBox->AddText(GlobalText[596], SEASON3B::TYPE_ERROR_MESSAGE);
    }
    break;
    case 4:
        SEASON3B::CreateOkMessageBox(GlobalText[649]);
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
        break;

    case 9:
        SEASON3B::CreateOkMessageBox(GlobalText[689]);
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
        break;

    case 100:
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
        g_pMixInventory->DeleteAllItems();
        g_pMixInventory->InsertItem(0, itemData);
        break;
    case 0x20:
        if (g_pLuckyItemWnd->GetAct())
        {
            g_pLuckyItemWnd->GetResult(0, Data->Index, itemData);
        }
        break;
    case 3:
    case 5:
    case 7:
    case 8:
    case 0x0A:
    default:
        g_pMixInventory->SetMixState(SEASON3B::CNewUIMixInventory::MIX_FINISHED);
        break;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x86 [ReceiveMix(%d)]", Data->Index);
}

void ReceiveSell(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_GOLD)ReceiveBuffer;
    if (Data->Flag != 0)
    {
        if (Data->Flag == 0xff)
        {
            SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();

            g_pChatListBox->AddText(Hero->ID, GlobalText[733], SEASON3B::TYPE_ERROR_MESSAGE);
        }
        else if (Data->Flag == 0xfe)
        {
            g_pNewUISystem->HideAll();

            g_pChatListBox->AddText(Hero->ID, GlobalText[733], SEASON3B::TYPE_ERROR_MESSAGE);
        }
        else
        {
            SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();

            CharacterMachine->Gold = Data->Gold;

            PlayBuffer(SOUND_GET_ITEM01);

            g_pNPCShop->SetSellingItem(false);
        }
    }
    else
    {
        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
    }
}

void ReceiveRepair(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_REPAIR_GOLD)ReceiveBuffer;

    if (Data->Gold != 0)
    {
        CharacterMachine->Gold = Data->Gold;
        CharacterMachine->CalculateAll();
        PlayBuffer(SOUND_REPAIR);
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x34 [ReceiveRepair(%d)]", Data->Gold);
}

void ReceiveLevelUp(const BYTE* ReceiveBuffer, int Size)
{
    if (Size >= sizeof(PRECEIVE_LEVEL_UP_EXTENDED))
    {
        auto Data = (LPPRECEIVE_LEVEL_UP_EXTENDED)ReceiveBuffer;
        CharacterAttribute->Level = Data->Level;
        CharacterAttribute->LevelUpPoint = Data->LevelUpPoint;
        CharacterAttribute->LifeMax = Data->MaxLife;
        CharacterAttribute->ManaMax = Data->MaxMana;
        CharacterAttribute->Life = Data->MaxLife;
        CharacterAttribute->Mana = Data->MaxMana;
        CharacterAttribute->ShieldMax = Data->MaxShield;
        CharacterAttribute->SkillManaMax = Data->SkillManaMax;
        CharacterAttribute->AddPoint = Data->AddPoint;
        CharacterAttribute->MaxAddPoint = Data->MaxAddPoint;
        CharacterAttribute->wMinusPoint = Data->wMinusPoint;
        CharacterAttribute->wMaxMinusPoint = Data->wMaxMinusPoint;
    }
    else
    {
        auto Data = (LPPRECEIVE_LEVEL_UP)ReceiveBuffer;
        CharacterAttribute->Level = Data->Level;
        CharacterAttribute->LevelUpPoint = Data->LevelUpPoint;
        CharacterAttribute->LifeMax = Data->MaxLife;
        CharacterAttribute->ManaMax = Data->MaxMana;
        CharacterAttribute->Life = Data->MaxLife;
        CharacterAttribute->Mana = Data->MaxMana;
        CharacterAttribute->ShieldMax = Data->MaxShield;
        CharacterAttribute->SkillManaMax = Data->SkillManaMax;
        CharacterAttribute->AddPoint = Data->AddPoint;
        CharacterAttribute->MaxAddPoint = Data->MaxAddPoint;
        CharacterAttribute->wMinusPoint = Data->wMinusPoint;
        CharacterAttribute->wMaxMinusPoint = Data->wMaxMinusPoint;
    }

    CharacterMachine->CalculateNextExperince();

    OBJECT* o = &Hero->Object;

    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 45, o, 80, 2);
        for (int i = 0; i < 19; ++i)
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 46, o, 80, 2);
        }
    }
    else
    {
        for (int i = 0; i < 15; ++i)
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 0, o, 40, 2);
        }
        CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 0, o);
    }
    PlayBuffer(SOUND_LEVEL_UP);

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x05 [ReceiveLevelUp]");
}

void ReceiveAddPoint(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_ADD_POINT)ReceiveBuffer;
    if (Data->Result >> 4)
    {
        CharacterAttribute->LevelUpPoint--;
        switch (Data->Result & 0xf)
        {
        case 0:
            CharacterAttribute->Strength++;
            break;
        case 1:
            CharacterAttribute->Dexterity++;
            break;
        case 2:
            CharacterAttribute->Vitality++;
            CharacterAttribute->LifeMax = Data->Max;
            break;
        case 3:
            CharacterAttribute->Energy++;
            CharacterAttribute->ManaMax = Data->Max;
            break;
        case 4:
            CharacterAttribute->Charisma++;
            break;
        }
        CharacterAttribute->SkillManaMax = Data->SkillManaMax;
        CharacterAttribute->ShieldMax = Data->ShieldMax;
    }
    CharacterMachine->CalculateAll();
}

void ReceiveAddPointExtended(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_ADD_POINT_EXTENDED)ReceiveBuffer;
    if (Data->AddedAmount == 0)
    {
        return;
    }

    CharacterAttribute->LevelUpPoint -= Data->AddedAmount;
    switch (Data->StatType)
    {
    case 0:
        CharacterAttribute->Strength += Data->AddedAmount;
        break;
    case 1:
        CharacterAttribute->Dexterity += Data->AddedAmount;
        break;
    case 2:
        CharacterAttribute->Vitality += Data->AddedAmount;
        break;
    case 3:
        CharacterAttribute->Energy += Data->AddedAmount;
        break;
    case 4:
        CharacterAttribute->Charisma += Data->AddedAmount;
        break;
    }

    CharacterAttribute->LifeMax = Data->MaxHealth;
    CharacterAttribute->ManaMax = Data->MaxMana;
    CharacterAttribute->SkillManaMax = Data->SkillManaMax;
    CharacterAttribute->ShieldMax = Data->ShieldMax;

    CharacterMachine->CalculateAll();
}

void ReceiveSetPointsExtended(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_SET_POINTS_EXTENDED)ReceiveBuffer;
    CharacterAttribute->Strength = Data->Strength;
    CharacterAttribute->Dexterity = Data->Dexterity;
    CharacterAttribute->Vitality = Data->Vitality;
    CharacterAttribute->Energy = Data->Energy;
    CharacterAttribute->Charisma = Data->Charisma;

    CharacterMachine->CalculateAll();
}

void ReceiveStatsExtended(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_STATS_EXTENDED)ReceiveBuffer;
    switch (Data->Index)
    {
    case 0xff:
        CharacterAttribute->Life = Data->Life;
        CharacterAttribute->Shield = Data->Shield;
        CharacterAttribute->Mana = Data->Mana;
        CharacterAttribute->SkillMana = Data->BP;
        CharacterAttribute->AttackSpeed = Data->AttackSpeed;
        CharacterAttribute->MagicSpeed = Data->MagicSpeed;
        break;
    case 0xfe:
        CharacterAttribute->LifeMax = Data->Life;
        CharacterAttribute->ShieldMax = Data->Shield;
        CharacterAttribute->ManaMax = Data->Mana;
        CharacterAttribute->SkillManaMax = Data->BP;
        if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
        {
            Master_Level_Data.wMaxLife = Data->Life;
            Master_Level_Data.wMaxShield = Data->Shield;
            Master_Level_Data.wMaxMana = Data->Mana;
            Master_Level_Data.wMaxBP = Data->BP;
        }

        break;
    case 0xfd:
        EnableUse = 0;
        break;
    default:
        // todo: is that ever used?
        if (ITEM* pItem = g_pMyInventory->FindItem(Data->Index))
        {
            if (pItem->Durability > 0)
                pItem->Durability--;
            if (pItem->Durability <= 0)
                g_pMyInventory->DeleteItem(Data->Index);
        }

        break;
    }
}

void ReceivePK(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_PK)ReceiveBuffer;
    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    CHARACTER* c = &CharactersClient[FindCharacterIndex(Key)];
    c->PK = Data->PK;

#ifdef PK_ATTACK_TESTSERVER_LOG
    PrintPKLog(c);
#endif // PK_ATTACK_TESTSERVER_LOG

    if (c->PK >= PVP_MURDERER2)
        c->Level = 1;
    else
        c->Level = 0;

    wchar_t message[256]{};
    wcscpy(message, c->ID);
    wcscat(message, L" : ");
    switch (Data->PK)
    {
    case 1: case 2:
    {
        wcscat(message, GlobalText[487]);
        g_pSystemLogBox->AddText(message, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
    break;
    case 3:
    {
        wcscat(message, GlobalText[488]);
        g_pSystemLogBox->AddText(message, SEASON3B::TYPE_ERROR_MESSAGE);
    }
    break;
    case 4:
    {
        wcscat(message, GlobalText[489]);
        g_pSystemLogBox->AddText(message, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
    break;
    case 5:
    {
        wchar_t szTemp[100];
        swprintf(szTemp, L"%s %d%s", GlobalText[490], 1, GlobalText[491]);
        wcscat(message, szTemp);
        g_pSystemLogBox->AddText(message, SEASON3B::TYPE_ERROR_MESSAGE);
    }
    break;
    case 6:
    {
        wchar_t szTemp[100];
        swprintf(szTemp, L"%s %d%s", GlobalText[490], 2, GlobalText[491]);
        wcscat(message, szTemp);
        g_pSystemLogBox->AddText(message, SEASON3B::TYPE_ERROR_MESSAGE);
    }
    break;
    }
}

void ReceiveDurability(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_VALUE_KEY)ReceiveBuffer;

    if (Data->Value < MAX_EQUIPMENT)
    {
        CharacterMachine->Equipment[Data->Value].Durability = Data->KeyH;
    }
    else
    {
        ITEM* pItem = g_pMyInventory->FindItem(Data->Value);

        if (pItem)
        {
            pItem->Durability = Data->KeyH;
        }
    }

    if (Data->KeyL)
    {
        EnableUse = 0;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x2A [ReceiveDurability(%d %d)]", Data->Value, Data->KeyL);
}

BOOL ReceiveHelperItem(const BYTE* ReceiveBuffer, BOOL bEncrypted)
{
    auto Data = (LPPRECEIVE_HELPER_ITEM)ReceiveBuffer;
    CharacterAttribute->AbilityTime[Data->Index] = Data->Time * REFERENCE_FPS;
    switch (Data->Index)
    {
    case 0:
        CharacterAttribute->Ability |= ABILITY_FAST_ATTACK_SPEED;
        break;
    case 1:
        CharacterAttribute->Ability |= ABILITY_PLUS_DAMAGE;
        CharacterMachine->CalculateDamage();
        CharacterMachine->CalculateMagicDamage();
        break;
    case 2:
        CharacterAttribute->Ability |= ABILITY_FAST_ATTACK_SPEED2;
        break;
    }
    EnableUse = 0;

    return (TRUE);
}

void ReceiveWeather(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    int Weather = (Data->Value >> 4);
    if (Weather == 0)
        RainTarget = 0;
    else if (Weather == 1)
        RainTarget = (Data->Value & 15) * 6;
}

void ReceiveEvent(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_EVENT)ReceiveBuffer;

    switch (Data->m_byNumber)
    {
    case 1:
        if (Data->m_byValue) EnableEvent = 1;
        else               EnableEvent = 0;
        break;
    case 3:
        if (Data->m_byValue) EnableEvent = 3;
        else               EnableEvent = 0;
        break;
    }
    DeleteBoids();
}

void ReceiveSummonLife(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
    SummonLife = Data->Value;
}
BOOL ReceiveTrade(const BYTE* ReceiveBuffer, BOOL bEncrypted)
{
    auto Data = (LPPCHATING)ReceiveBuffer;
    g_pTrade->ProcessToReceiveTradeRequest(Data->ID);

    return (TRUE);
}

void ReceiveTradeResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPTRADE)ReceiveBuffer;
    g_pTrade->ProcessToReceiveTradeResult(Data);
}

void ReceiveTradeYourInventoryDelete(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    g_pTrade->ProcessToReceiveYourItemDelete(Data->Value);
}

/*
void ReceiveTradeYourInventory(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_ITEM)ReceiveBuffer;
    g_pTrade->ProcessToReceiveYourItemAdd(Data->Index, Data->Item, Old);
}*/

void ReceiveTradeMyGold(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    g_pTrade->ProcessToReceiveMyTradeGold(Data->Value);
}

void ReceiveTradeYourGold(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_DWORD)ReceiveBuffer;
    g_pTrade->SetYourTradeGold(int(Data->Value));
}

void ReceiveTradeYourResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    g_pTrade->ProcessToReceiveYourConfirm(Data->Value);
}

void ReceiveTradeExit(const BYTE* ReceiveBuffer)
{
    if (EquipmentItem)
    {
        if (!g_bPacketAfter_EquipmentItem)
        {
            memcpy(g_byPacketAfter_EquipmentItem, ReceiveBuffer, sizeof(PHEADER_DEFAULT));
            g_bPacketAfter_EquipmentItem = TRUE;
        }
        return;
    }

    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    g_pTrade->ProcessToReceiveTradeExit(Data->Value);
}

void ReceivePing(const BYTE* ReceiveBuffer)
{
    SocketClient->ToGameServer()->SendPingResponse();
}

void ReceiveStorageGold(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_STORAGE_GOLD)ReceiveBuffer;
    if (Data->Result)
    {
        CharacterMachine->StorageGold = Data->StorageGold;
        CharacterMachine->Gold = Data->Gold;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x81 [ReceiveStorageGold(%d %d %d)]", Data->Result, Data->StorageGold, Data->Gold);
}

void ReceiveStorageExit(const BYTE* ReceiveBuffer)
{
    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x82 [ReceiveStorageExit]");
}

void ReceiveStorageStatus(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;

    g_pStorageInventory->ProcessToReceiveStorageStatus(Data->Value);
}

void ReceiveParty(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_KEY)ReceiveBuffer;
    PartyKey = ((int)(Data->KeyH) << 8) + Data->KeyL;

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPartyMsgBoxLayout));
}

void ReceivePartyResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:g_pSystemLogBox->AddText(GlobalText[497], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 1:g_pSystemLogBox->AddText(GlobalText[498], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 2:g_pSystemLogBox->AddText(GlobalText[499], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 3:g_pSystemLogBox->AddText(GlobalText[500], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 4:g_pSystemLogBox->AddText(GlobalText[501], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 5:g_pSystemLogBox->AddText(GlobalText[502], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 6:g_pSystemLogBox->AddText(GlobalText[2990], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 7:g_pSystemLogBox->AddText(GlobalText[2997], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 8:g_pSystemLogBox->AddText(GlobalText[2998], SEASON3B::TYPE_ERROR_MESSAGE); break;
    }
}

void ReceivePartyList(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_PARTY_LISTS)ReceiveBuffer;
    int Offset = sizeof(PRECEIVE_PARTY_LISTS);
    PartyNumber = Data->Count;
    for (int i = 0; i < Data->Count; i++)
    {
        auto Data2 = (LPPRECEIVE_PARTY_LIST)(ReceiveBuffer + Offset);
        PARTY_t* p = &Party[i];
        CMultiLanguage::ConvertFromUtf8(p->Name, Data2->ID, MAX_ID_SIZE);
        p->Name[MAX_ID_SIZE] = NULL;
        p->Number = Data2->Number;
        p->Map = Data2->Map;
        p->x = Data2->x;
        p->y = Data2->y;
        p->currHP = Data2->currHP;
        p->maxHP = Data2->maxHP;
        Offset += sizeof(PRECEIVE_PARTY_LIST);
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x42 [ReceivePartyList(partynum : %d)]", Data->Count);
}

void ReceivePartyInfo(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_PARTY_INFOS)ReceiveBuffer;
    int Offset = sizeof(PRECEIVE_PARTY_INFOS);
    for (int i = 0; i < Data->Count; i++)
    {
        auto Data2 = (LPPRECEIVE_PARTY_INFO)(ReceiveBuffer + Offset);
        wchar_t stepHP = Data2->value & 0xf;

        PARTY_t* p = &Party[i];

        p->stepHP = min(10, max(0, stepHP));

        Offset += sizeof(PRECEIVE_PARTY_INFO);
    }
}

void ReceivePartyLeave(const BYTE* ReceiveBuffer)
{
    PartyNumber = 0;
    g_pSystemLogBox->AddText(GlobalText[502], SEASON3B::TYPE_ERROR_MESSAGE);

    if (g_iFollowCharacter >= 0)
    {
        bool IsParty = false;
        CHARACTER* c = &CharactersClient[g_iFollowCharacter];
        for (int i = 0; i < PartyNumber; ++i)
        {
            if (wcscmp(Party[i].Name, c->ID) == NULL && wcslen(Party[i].Name) == wcslen(c->ID))
            {
                IsParty = true;
            }
        }

        if (IsParty == false)
        {
            g_iFollowCharacter = -1;
        }
    }
}

void ReceivePartyGetItem(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_GETITEMINFO_FOR_PARTY)ReceiveBuffer;
    int Key = ((int)(Data->KeyH) << 8) + Data->KeyL;
    int Index = FindCharacterIndex(Key);
    CHARACTER* c = &CharactersClient[Index];
    if (Hero == c) return;

    int itemType = Data->ItemInfo & 0x01fff;
    wchar_t itemName[100] = { 0, };
    wchar_t Text[200] = { 0, };

    if ((Data->ItemInfo & 0x10000))      swprintf(itemName, L"%s ", GlobalText[620]);
    else if ((Data->ItemInfo & 0x20000)) swprintf(itemName, L"%s ", GlobalText[1089]);

    int itemLevel = Data->ItemLevel;
    GetItemName(itemType, itemLevel, Text);
    wcscat(itemName, Text);
    if ((Data->ItemInfo & 0x02000)) wcscat(itemName, GlobalText[176]);
    if ((Data->ItemInfo & 0x08000)) wcscat(itemName, GlobalText[177]);
    if ((Data->ItemInfo & 0x04000)) wcscat(itemName, GlobalText[178]);

    swprintf(Text, L"%s : %s %s", c->ID, itemName, GlobalText[918]);

    g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

extern int ErrorMessage;

void ReceiveGuild(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_KEY)ReceiveBuffer;
    GuildPlayerKey = ((int)(Data->KeyH) << 8) + Data->KeyL;

    SEASON3B::CNewUICommonMessageBox* pMsgBox;
    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildRequestMsgBoxLayout), &pMsgBox);
    pMsgBox->AddMsg(CharactersClient[FindCharacterIndex(GuildPlayerKey)].ID);
    pMsgBox->AddMsg(GlobalText[429]);
}

void ReceiveGuildResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:g_pSystemLogBox->AddText(GlobalText[503], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 1:g_pSystemLogBox->AddText(GlobalText[504], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 2:g_pSystemLogBox->AddText(GlobalText[505], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 3:g_pSystemLogBox->AddText(GlobalText[506], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 4:g_pSystemLogBox->AddText(GlobalText[507], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 5:g_pSystemLogBox->AddText(GlobalText[508], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 6:g_pSystemLogBox->AddText(GlobalText[509], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 7:g_pSystemLogBox->AddText(GlobalText[510], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 0xA1:g_pSystemLogBox->AddText(GlobalText[2992], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 0xA2:g_pSystemLogBox->AddText(GlobalText[2995], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 0xA3:g_pSystemLogBox->AddText(GlobalText[2996], SEASON3B::TYPE_ERROR_MESSAGE); break;
    }
}

void ReceiveGuildList(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_GUILD_LISTS)ReceiveBuffer;
    int Offset = sizeof(PRECEIVE_GUILD_LISTS);

    g_nGuildMemberCount = Data->Count;
    GuildTotalScore = Data->TotalScore;
    GuildTotalScore = max(0, GuildTotalScore);

    wchar_t rivalGuildName[sizeof Data->szRivalGuildName + 1] {};
    CMultiLanguage::ConvertFromUtf8(rivalGuildName, Data->szRivalGuildName, sizeof Data->szRivalGuildName);
    g_pGuildInfoWindow->GuildClear();
    g_pGuildInfoWindow->UnionGuildClear();
    g_pGuildInfoWindow->SetRivalGuildName(rivalGuildName);
    for (int i = 0; i < Data->Count; i++)
    {
        auto Data2 = (LPPRECEIVE_GUILD_LIST)(ReceiveBuffer + Offset);
        GUILD_LIST_t* p = &GuildList[i];
        CMultiLanguage::ConvertFromUtf8(p->Name, Data2->ID, MAX_ID_SIZE);
        p->Number = Data2->Number;
        p->Server = (0x80 & Data2->CurrentServer) ? (0x7F & Data2->CurrentServer) : -1;
        p->GuildStatus = Data2->GuildStatus;
        Offset += sizeof(PRECEIVE_GUILD_LIST);
        g_pGuildInfoWindow->AddGuildMember(p);
    }
}

void ReceiveGuildLeave(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:g_pSystemLogBox->AddText(GlobalText[511], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 1:g_pSystemLogBox->AddText(GlobalText[512], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 2:g_pSystemLogBox->AddText(GlobalText[513], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 3:g_pSystemLogBox->AddText(GlobalText[514], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 4:g_pSystemLogBox->AddText(GlobalText[515], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 5:g_pSystemLogBox->AddText(GlobalText[568], SEASON3B::TYPE_ERROR_MESSAGE); break;
    }
    if (Data->Value == 1 || Data->Value == 4)
    {
        if (Data->Value == 4 && Hero->GuildMarkIndex != -1)
        {
            GuildMark[Hero->GuildMarkIndex].Key = -1;
            GuildMark[Hero->GuildMarkIndex].UnionName[0] = NULL;
            GuildMark[Hero->GuildMarkIndex].GuildName[0] = NULL;
            Hero->GuildMarkIndex = -1;
        }

        EnableGuildWar = false;

        g_nGuildMemberCount = -1;
        Hero->GuildStatus = G_NONE;
        Hero->GuildRelationShip = GR_NONE;
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GUILDINFO);

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
        g_pMoveCommandWindow->SetCastleOwner(false);
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
    }
    else if (Data->Value == 5)
    {
        SocketClient->ToGameServer()->SendGuildListRequest();
    }
}

void ReceiveCreateGuildInterface(const BYTE* ReceiveBuffer)
{
    g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCGUILDMASTER);
}

void ReceiveCreateGuildMasterInterface(const BYTE* ReceiveBuffer)
{
}

void ReceiveDeleteGuildViewport(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_KEY)ReceiveBuffer;
    int Key = ((int)(Data->KeyH & 0x7f) << 8) + Data->KeyL;
    int Index = FindCharacterIndex(Key);
    CHARACTER* c = &CharactersClient[Index];

    c->GuildStatus = -1;
    c->GuildType = 0;
    c->ProtectGuildMarkWorldTime = 0.0f;
    c->GuildRelationShip = 0;
    c->GuildMarkIndex = -1;

    g_nGuildMemberCount = -1;

    g_pNewUISystem->Hide(SEASON3B::INTERFACE_GUILDINFO);
}

void ReceiveCreateGuildResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_GUILD_CREATE_RESULT)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:g_pSystemLogBox->AddText(GlobalText[516], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 2:g_pSystemLogBox->AddText(GlobalText[517], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 3:g_pSystemLogBox->AddText(GlobalText[518], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 4:g_pSystemLogBox->AddText(GlobalText[940], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 5:g_pSystemLogBox->AddText(GlobalText[941], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 6:g_pSystemLogBox->AddText(GlobalText[942], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 1:
        memset(InputText[0], 0, MAX_ID_SIZE);
        InputLength[0] = 0;
        InputTextMax[0] = MAX_ID_SIZE;

        g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCGUILDMASTER);

        MouseUpdateTime = 0;
        MouseUpdateTimeMax = 6;
        break;
    }
}

bool EnableGuildWar = false;
int  GuildWarIndex = -1;
wchar_t GuildWarName[8 + 1];
int  GuildWarScore[2];

bool EnableSoccer = false;
BYTE HeroSoccerTeam = 0;
int  SoccerTime;
wchar_t SoccerTeamName[2][8 + 1];
bool SoccerObserver = false;

void ReceiveDeclareWar(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_WAR)ReceiveBuffer;
    memset(GuildWarName, 0, sizeof GuildWarName);
    CMultiLanguage::ConvertFromUtf8(GuildWarName, Data->Name, 8);

    if (Data->Type == 1)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CBattleSoccerMsgBoxLayout));
    }
    else
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGuildWarMsgBoxLayout));
    }
}

void ReceiveDeclareWarResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:g_pSystemLogBox->AddText(GlobalText[519], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 1:g_pSystemLogBox->AddText(GlobalText[520], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 2:g_pSystemLogBox->AddText(GlobalText[521], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 3:g_pSystemLogBox->AddText(GlobalText[522], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 4:g_pSystemLogBox->AddText(GlobalText[523], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 5:g_pSystemLogBox->AddText(GlobalText[524], SEASON3B::TYPE_ERROR_MESSAGE); break;
    case 6:g_pSystemLogBox->AddText(GlobalText[525], SEASON3B::TYPE_ERROR_MESSAGE); break;
    }
    if (Data->Value != 1 && !EnableGuildWar)
    {
        InitGuildWar();
    }
}

void ReceiveGuildBeginWar(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_WAR)ReceiveBuffer;
    EnableGuildWar = true;

    wchar_t Text[100];
    memset(GuildWarName, 0, sizeof GuildWarName);
    CMultiLanguage::ConvertFromUtf8(GuildWarName, Data->Name, 8);

    if (Data->Type == 0)
    {
        swprintf(Text, GlobalText[526], GuildWarName);
    }
    else
    {
        swprintf(Text, GlobalText[533], GuildWarName);
        EnableSoccer = true;
    }

    CreateNotice(Text, 1);
    HeroSoccerTeam = Data->Team;

    for (int i = 0; i < MARK_EDIT; i++)
    {
        MARK_t* p = &GuildMark[i];
        wchar_t Temp[8 + 1];
        memset(Temp, 0, 8);
        memcpy(Temp, (wchar_t*)Data->Name, 8);
        Temp[8] = NULL;
        if (wcscmp(p->GuildName, Temp) == NULL)
        {
            GuildWarIndex = i;
            break;
        }
    }
    if (GuildWarIndex >= 0)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
        {
            CHARACTER* c = &CharactersClient[i];
            GuildTeam(c);
        }
    }
    SetActionClass(Hero, &Hero->Object, PLAYER_RUSH1, AT_RUSH1);
    SendRequestAction(Hero->Object, AT_RUSH1);

    g_pNewUISystem->Show(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x62 [ReceiveGuildBeginWar(%d)]", Data->Team);
}

void ReceiveGuildEndWar(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    wchar_t Text[100];
    int Win = 0;
    switch (Data->Value)
    {
    case 0:wprintf(Text, GlobalText[527]); break;
    case 1:wprintf(Text, GlobalText[528]); Win = 2; break;
    case 2:wprintf(Text, GlobalText[529]); Win = 2; break;
    case 3:wprintf(Text, GlobalText[530]); break;
    case 4:wprintf(Text, GlobalText[531]); Win = 2; break;
    case 5:wprintf(Text, GlobalText[532]); break;
    case 6:wprintf(Text, GlobalText[480]); Win = 1; break;
    default:swprintf(Text, L""); break;
    }

    g_wtMatchTimeLeft.m_Time = 0;

#ifndef GUILD_WAR_EVENT
    CreateNotice(Text, 1);
#endif

    EnableGuildWar = false;
    EnableSoccer = false;

    GuildWarIndex = -1;
    GuildWarName[0] = NULL;
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        GuildTeam(c);
    }
    switch (Win)
    {
    case 2:
        SetActionClass(Hero, &Hero->Object, PLAYER_WIN1, AT_WIN1);
        SendRequestAction(Hero->Object, AT_WIN1);
        break;
    case 0:
        SetActionClass(Hero, &Hero->Object, PLAYER_CRY1, AT_CRY1);
        SendRequestAction(Hero->Object, AT_CRY1);
        break;
    }

    g_pNewUISystem->Hide(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);
}

void ReceiveGuildWarScore(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_WAR_SCORE)ReceiveBuffer;
    EnableGuildWar = true;

#ifdef GUILD_WAR_EVENT
    int t1 = Data->Score1 - GuildWarScore[0];
    int t2 = Data->Score2 - GuildWarScore[1];
    if (t1 == 20 || t2 == 20)
    {
        if (t1 > t2) g_iGoalEffect = 1;
        else g_iGoalEffect = 2;
    }
#endif

    GuildWarScore[0] = Data->Score1;
    GuildWarScore[1] = Data->Score2;
}

void ReceiveGuildIDViewport(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
    int Offset = sizeof(PWHEADER_DEFAULT_WORD);
    for (int i = 0; i < Data->Value; ++i)
    {
        auto Data2 = (LPPRECEIVE_GUILD_ID)(ReceiveBuffer + Offset);
        int GuildKey = Data2->GuildKey;
        int Key = ((int)(Data2->KeyH & 0x7f) << 8) + Data2->KeyL;
        int Index = FindCharacterIndex(Key);
        CHARACTER* c = &CharactersClient[Index];

        c->GuildStatus = Data2->GuildStatus;
        c->GuildType = Data2->GuildType;
        c->GuildRelationShip = Data2->GuildRelationShip;

#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
        g_pMoveCommandWindow->SetCastleOwner((bool)Data2->btCastleOwner);
#endif // CSK_MOD_MOVE_COMMAND_WINDOW

        if (g_GuildCache.IsExistGuildMark(GuildKey))
            c->GuildMarkIndex = g_GuildCache.GetGuildMarkIndex(GuildKey);
        else
        {
            SocketClient->ToGameServer()->SendGuildInfoRequest(GuildKey);
            c->GuildMarkIndex = g_GuildCache.MakeGuildMarkIndex(GuildKey);
        }

        GuildTeam(c);

        if (gMapManager.WorldActive == WD_30BATTLECASTLE)
        {
            if (g_pSiegeWarfare)
            {
                if (g_pSiegeWarfare->IsCreated() == false)
                {
                    g_pSiegeWarfare->InitMiniMapUI();
                    g_pSiegeWarfare->SetGuildData(Hero);
                    g_pSiegeWarfare->CreateMiniMapUI();
                }
            }
        }

        Offset += sizeof(PRECEIVE_GUILD_ID);
    }
}

void ReceiveGuildInfo(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPPRECEIVE_GUILDINFO)ReceiveBuffer;
    int Index = g_GuildCache.SetGuildMark(Data->GuildKey, Data->UnionName, Data->GuildName, Data->Mark);
}

// 길드직책을 임명/변경/해제 결과
void ReceiveGuildAssign(const BYTE* ReceiveBuffer)
{
    wchar_t szTemp[MAX_GLOBAL_TEXT_STRING] = L"Invalid GuildAssign";
    auto pData = (LPPRECEIVE_GUILD_ASSIGN)ReceiveBuffer;
    if (pData->byResult == 0x01)
    {
        switch (pData->byType)
        {
        case 0x01:
            wcscpy(szTemp, GlobalText[1374]);
            break;
        case 0x02:
            wcscpy(szTemp, GlobalText[1375]);
            break;
        case 0x03:
            wcscpy(szTemp, GlobalText[1376]);
            break;
        default:
            assert(!"Packet(0xE1)");
            break;
        }
    }
    else
    {
        switch (pData->byResult)
        {
        case GUILD_ANS_NOTEXIST_GUILD:
            wcscpy(szTemp, GlobalText[522]);
            break;
        case GUILD_ANS_NOTEXIST_PERMISSION:
            wcscpy(szTemp, GlobalText[1386]);
            break;
        case GUILD_ANS_NOTEXIST_EXTRA_STATUS:
            wcscpy(szTemp, GlobalText[1326]);
            break;
        case GUILD_ANS_NOTEXIST_EXTRA_TYPE:
            wcscpy(szTemp, GlobalText[1327]);
            break;
        default:
            assert(!"Packet(0xE1)");
            break;
        }
    }
    g_pSystemLogBox->AddText(szTemp, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

void ReceiveGuildRelationShip(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_GUILD_RELATIONSHIP)ReceiveBuffer;

    g_pGuildInfoWindow->ReceiveGuildRelationShip(pData->byRelationShipType, pData->byRequestType, pData->byTargetUserIndexH, pData->byTargetUserIndexL);
}

void ReceiveGuildRelationShipResult(const BYTE* ReceiveBuffer)
{
    wchar_t szTemp[MAX_GLOBAL_TEXT_STRING] = L"Invalid GuildRelationShipResult";
    auto pData = (LPPMSG_GUILD_RELATIONSHIP_RESULT)ReceiveBuffer;
    if (pData->byResult == 0x01)
    {
        if (pData->byRelationShipType == 0x01)
        {
            if (pData->byRequestType == 0x01)
            {
                wcscpy(szTemp, GlobalText[1381]);
            }
            else if (pData->byRequestType == 0x02)
            {
                wcscpy(szTemp, GlobalText[1382]);
            }
            else if (pData->byRequestType == 0x10)
            {
                wcscpy(szTemp, GlobalText[1635]);
            }
        }
        else
        {
            if (pData->byRequestType == 0x01)	wcscpy(szTemp, GlobalText[1383]);
            else								wcscpy(szTemp, GlobalText[1384]);
        }
    }
    else if (pData->byResult == 0)
    {
        wcscpy(szTemp, GlobalText[1328]);
    }
    else
    {
        switch (pData->byResult)
        {
        case GUILD_ANS_UNIONFAIL_BY_CASTLE:
            wcscpy(szTemp, GlobalText[1637]);
            break;
        case GUILD_ANS_NOTEXIST_PERMISSION:
            wcscpy(szTemp, GlobalText[1386]);
            break;
        case GUILD_ANS_EXIST_RELATIONSHIP_UNION:
            wcscpy(szTemp, GlobalText[1250]);
            break;
        case GUILD_ANS_EXIST_RELATIONSHIP_RIVAL:
            wcscpy(szTemp, GlobalText[1251]);
            break;
        case GUILD_ANS_EXIST_UNION:
            wcscpy(szTemp, GlobalText[1252]);
            break;
        case GUILD_ANS_EXIST_RIVAL:
            wcscpy(szTemp, GlobalText[1253]);
            break;
        case GUILD_ANS_NOTEXIST_UNION:
            wcscpy(szTemp, GlobalText[1254]);
            break;
        case GUILD_ANS_NOTEXIST_RIVAL:
            wcscpy(szTemp, GlobalText[1255]);
            break;
        case GUILD_ANS_NOT_UNION_MASTER:
            wcscpy(szTemp, GlobalText[1333]);
            break;
        case GUILD_ANS_NOT_GUILD_RIVAL:
            wcscpy(szTemp, GlobalText[1329]);
            break;
        case GUILD_ANS_CANNOT_BE_UNION_MASTER_GUILD:
            wcscpy(szTemp, GlobalText[1331]);
            break;
        case GUILD_ANS_EXCEED_MAX_UNION_MEMBER:
            wcscpy(szTemp, GlobalText[1287]);
            break;
        case GUILD_ANS_CANCEL_REQUEST:
            wcscpy(szTemp, GlobalText[1268]);
            break;
#ifdef ASG_ADD_GENS_SYSTEM
        case GUILD_ANS_UNION_MASTER_NOT_GENS:
            wcscpy(szTemp, GlobalText[2991]);
            break;
        case GUILD_ANS_GUILD_MASTER_NOT_GENS:
            wcscpy(szTemp, GlobalText[2992]);
            break;
        case GUILD_ANS_UNION_MASTER_DISAGREE_GENS:
            wcscpy(szTemp, GlobalText[2993]);
            break;
#endif	// ASG_ADD_GENS_SYSTEM
        default:
            assert(!"Packet (0xE6)");
            break;
        }
    }
    g_pSystemLogBox->AddText(szTemp, SEASON3B::TYPE_SYSTEM_MESSAGE);

    int nCharKey = MAKEWORD(pData->byTargetUserIndexL, pData->byTargetUserIndexH);
    if (nCharKey == HeroKey && pData->byResult == 0x01 && pData->byRelationShipType == 0x01 && pData->byRequestType == 0x02)
        GuildMark[Hero->GuildMarkIndex].UnionName[0] = NULL;
}

void ReceiveBanUnionGuildResult(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_BAN_UNIONGUILD)ReceiveBuffer;
    if (pData->byResult == 0x01)
    {
        if (g_pGuildInfoWindow->GetUnionCount() > 2)
        {
            SocketClient->ToGameServer()->SendRequestAllianceList();
        }
        g_pGuildInfoWindow->UnionGuildClear();
    }
    else if (pData->byResult == 0)
    {
        g_pSystemLogBox->AddText(GlobalText[1328], SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
}

void ReceiveUnionViewportNotify(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_UNION_VIEWPORT_NOTIFY_COUNT)ReceiveBuffer;
    int Offset = sizeof(PMSG_UNION_VIEWPORT_NOTIFY_COUNT);
    for (int i = 0; i < pData->byCount; ++i)
    {
        auto pData2 = (LPPMSG_UNION_VIEWPORT_NOTIFY)(ReceiveBuffer + Offset);
        int nGuildMarkIndex = g_GuildCache.GetGuildMarkIndex(pData2->nGuildKey);
        CMultiLanguage::ConvertFromUtf8(GuildMark[nGuildMarkIndex].UnionName, pData2->szUnionName, MAX_GUILDNAME);

        int nCharKey = MAKEWORD(pData2->byKeyL, pData2->byKeyH);

        int nCharIndex = FindCharacterIndex(nCharKey);
        CharactersClient[nCharIndex].GuildRelationShip = pData2->byGuildRelationShip;

        Offset += sizeof(PMSG_UNION_VIEWPORT_NOTIFY);
    }
}

void ReceiveUnionList(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_UNIONLIST_COUNT)ReceiveBuffer;
    g_pGuildInfoWindow->UnionGuildClear();
    if (pData->byResult == 1)
    {
        int Offset = sizeof(PMSG_UNIONLIST_COUNT);
        for (int i = 0; i < pData->byCount; ++i)
        {
            auto pData2 = (LPPMSG_UNIONLIST)(ReceiveBuffer + Offset);

            BYTE tmp[64];
            for (int j = 0; j < 64; ++j)
            {
                if (j % 2 == 0)
                    tmp[j] = (pData2->GuildMark[j / 2] >> 4) & 0x0f;
                else
                    tmp[j] = pData2->GuildMark[j / 2] & 0x0f;
            }

            wchar_t guildName[MAX_GUILDNAME + 1];
            CMultiLanguage::ConvertFromUtf8(guildName, pData2->szGuildName, MAX_GUILDNAME);

            g_pGuildInfoWindow->AddUnionList(tmp, guildName, pData2->byMemberCount);

            Offset += sizeof(PMSG_UNIONLIST);
        }
    }
}

void ReceiveSoccerTime(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_SOCCER_TIME)ReceiveBuffer;
    SoccerTime = Data->Time;
}

void ReceiveSoccerScore(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_SOCCER_SCORE)ReceiveBuffer;
    CMultiLanguage::ConvertFromUtf8(SoccerTeamName[0], Data->Name1, MAX_GUILDNAME);
    CMultiLanguage::ConvertFromUtf8(SoccerTeamName[1], Data->Name2, MAX_GUILDNAME);
    GuildWarScore[0] = Data->Score1;
    GuildWarScore[1] = Data->Score2;

    if (GuildWarScore[0] != 255)
    {
        SoccerObserver = true;
        g_pNewUISystem->Show(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);
    }
    else
    {
        SoccerObserver = false;
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_BATTLE_SOCCER_SCORE);
    }

    vec3_t Position, Angle, Light;
    Position[0] = (float)(Hero->PositionX + 0.5f) * TERRAIN_SCALE;
    Position[1] = (float)(Hero->PositionY + 0.5f) * TERRAIN_SCALE;
    Position[2] = RequestTerrainHeight(Position[0], Position[1]);
    ZeroMemory(Angle, sizeof(Angle));
    Light[0] = Light[1] = Light[2] = 1.0f;
    CreateEffect(BITMAP_FIRECRACKERRISE, Position, Angle, Light);
}

void ReceiveSoccerGoal(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    wchar_t Text[100];
    if (Data->Value == HeroSoccerTeam)
        swprintf(Text, GlobalText[534], GuildMark[Hero->GuildMarkIndex].GuildName);
    else
        swprintf(Text, GlobalText[534], GuildWarName);
    g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

void Receive_Master_LevelUp(const BYTE* ReceiveBuffer, int Size)
{
    auto Data = (LPPMSG_MASTERLEVEL_UP)ReceiveBuffer;
    Master_Level_Data.nMLevel = Data->nMLevel;
    Master_Level_Data.nAddMPoint = Data->nAddMPoint;
    Master_Level_Data.nMLevelUpMPoint = Data->nMLevelUpMPoint;
    //	Master_Level_Data.nTotalMPoint		= Data->nTotalMPoint;
    Master_Level_Data.nMaxPoint = Data->nMaxPoint;

    if (Size >= sizeof(LPPMSG_MASTERLEVEL_UP_EXTENDED))
    {
        auto ExtData = (LPPMSG_MASTERLEVEL_UP_EXTENDED)ReceiveBuffer;
        Master_Level_Data.wMaxLife = ExtData->wMaxLife;
        Master_Level_Data.wMaxMana = ExtData->wMaxMana;
        Master_Level_Data.wMaxShield = ExtData->wMaxShield;
        Master_Level_Data.wMaxBP = ExtData->wMaxBP;
    }
    else
    {
        Master_Level_Data.wMaxLife = Data->wMaxLife;
        Master_Level_Data.wMaxMana = Data->wMaxMana;
        Master_Level_Data.wMaxShield = Data->wMaxShield;
        Master_Level_Data.wMaxBP = Data->wMaxBP;
    }

    wchar_t szText[256] = { NULL, };
    DWORD iExp = Master_Level_Data.lNext_MasterLevel_Experince - Master_Level_Data.lMasterLevel_Experince;
    if (iExp > 0)
    {
        swprintf(szText, GlobalText[1750], iExp);
        g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    CharacterMachine->CalulateMasterLevelNextExperience();

    OBJECT* o = &Hero->Object;

    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 45, o, 80, 2);
        for (int i = 0; i < 19; ++i)
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 46, o, 80, 2);
        }
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x51 [Receive_Master_LevelUp]");
}

void Receive_Master_Level_Exp(const BYTE* ReceiveBuffer, int Size)
{
    auto Data = (LPPMSG_MASTERLEVEL_INFO)ReceiveBuffer;
    Master_Level_Data.nMLevel = Data->nMLevel;
    Master_Level_Data.lMasterLevel_Experince = 0x0000000000000000;
    Master_Level_Data.lMasterLevel_Experince |= Data->btMExp1;
    Master_Level_Data.lMasterLevel_Experince <<= 8;
    Master_Level_Data.lMasterLevel_Experince |= Data->btMExp2;
    Master_Level_Data.lMasterLevel_Experince <<= 8;
    Master_Level_Data.lMasterLevel_Experince |= Data->btMExp3;
    Master_Level_Data.lMasterLevel_Experince <<= 8;
    Master_Level_Data.lMasterLevel_Experince |= Data->btMExp4;
    Master_Level_Data.lMasterLevel_Experince <<= 8;
    Master_Level_Data.lMasterLevel_Experince |= Data->btMExp5;
    Master_Level_Data.lMasterLevel_Experince <<= 8;
    Master_Level_Data.lMasterLevel_Experince |= Data->btMExp6;
    Master_Level_Data.lMasterLevel_Experince <<= 8;
    Master_Level_Data.lMasterLevel_Experince |= Data->btMExp7;
    Master_Level_Data.lMasterLevel_Experince <<= 8;
    Master_Level_Data.lMasterLevel_Experince |= Data->btMExp8;
    Master_Level_Data.lNext_MasterLevel_Experince = 0x0000000000000000;
    Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp1;
    Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
    Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp2;
    Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
    Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp3;
    Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
    Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp4;
    Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
    Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp5;
    Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
    Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp6;
    Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
    Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp7;
    Master_Level_Data.lNext_MasterLevel_Experince <<= 8;
    Master_Level_Data.lNext_MasterLevel_Experince |= Data->btMNextExp8;
    Master_Level_Data.nMLevelUpMPoint = Data->nMLPoint;

    if (Size >= sizeof(LPPMSG_MASTERLEVEL_INFO_EXTENDED))
    {
        auto ExtData = (LPPMSG_MASTERLEVEL_INFO_EXTENDED)ReceiveBuffer;
        Master_Level_Data.wMaxLife = ExtData->wMaxLife;
        Master_Level_Data.wMaxMana = ExtData->wMaxMana;
        Master_Level_Data.wMaxShield = ExtData->wMaxShield;
        Master_Level_Data.wMaxBP = ExtData->wMaxSkillMana;
    }
    else
    {
        Master_Level_Data.wMaxLife = Data->wMaxLife;
        Master_Level_Data.wMaxMana = Data->wMaxMana;
        Master_Level_Data.wMaxShield = Data->wMaxShield;
        Master_Level_Data.wMaxBP = Data->wMaxSkillMana;
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x50 [Receive_Master_Level_Exp]");
}

void Receive_Master_LevelGetSkill(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_MASTERLEVEL_SKILL)ReceiveBuffer;

    if (Data->Result == 1 && Data->SkillNumber > -1)
    {
        const auto newSkill = (ActionSkillType)Data->SkillNumber;
        if (auto search = SKILL_REPLACEMENTS.find(newSkill); search != SKILL_REPLACEMENTS.end())
        {
            const auto replacedSkill = search->second;
            for (auto & i : CharacterAttribute->Skill)
            {
                if (i == replacedSkill)
                {
                    i = AT_SKILL_UNDEFINED;
                    break;
                }
            }
        }

        bool isSkillUpdated = false;

        for (int i = 0; i < MAX_SKILLS; ++i)
        {
            if (CharacterAttribute->Skill[i] == Data->SkillNumber)
            {
                CharacterAttribute->SkillLevel[i] = Data->SkillLevel;
                isSkillUpdated = true;
                break;
            }
        }

        if (!isSkillUpdated)
        {
            // then we need to add it to the next free index
            for (int i = 0; i < MAX_MAGIC; ++i)
            {
                if (CharacterAttribute->Skill[i] == AT_SKILL_UNDEFINED)
                {
                    CharacterAttribute->Skill[i] = (ActionSkillType)Data->SkillNumber;
                    CharacterAttribute->SkillLevel[i] = Data->SkillLevel;
                    break;
                }
            }
        }

        auto interface = CNewUISystem::GetInstance()->GetUI_NewMasterLevelInterface();

        interface->SkillUpgrade(Data->SkillIndex, Data->SkillLevel, Data->DisplayValue, Data->DisplayValueOfNextLevel);
    }
    Master_Level_Data.nMLevelUpMPoint = Data->MasterLevelUpPoints;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x52 [Receive_Master_LevelGetSkill]");
}

void ReceiveServerCommand(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_SERVER_COMMAND)ReceiveBuffer;
    switch (Data->Cmd1)
    {
    case 2:
        PlayBuffer(SOUND_MEDAL);
        break;
    case 0:
    {
        vec3_t Position, Angle, Light;
        Position[0] = (float)(Data->Cmd2 + 0.5f) * TERRAIN_SCALE;
        Position[1] = (float)(Data->Cmd3 + 0.5f) * TERRAIN_SCALE;
        Position[2] = RequestTerrainHeight(Position[0], Position[1]);
        ZeroMemory(Angle, sizeof(Angle));
        Light[0] = Light[1] = Light[2] = 1.0f;
        CreateEffect(BITMAP_FIRECRACKER0001, Position, Angle, Light, 0);
    }
    break;
    case 1:
        if (Data->Cmd2 >= 20)
        {
            SEASON3B::CreateOkMessageBox(GlobalText[830 + Data->Cmd2 - 20]);
        }
        else
        {
            SEASON3B::CreateOkMessageBox(GlobalText[650 + Data->Cmd2]);
        }
        break;

    case 3:
        SEASON3B::CreateOkMessageBox(GlobalText[710 + Data->Cmd2]);
        break;
    case 4:
        SEASON3B::CreateOkMessageBox(GlobalText[725 + Data->Cmd2]);
        break;
    case 5:
    {
        SEASON3B::CDialogMsgBox* pMsgBox = nullptr;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDialogMsgBoxLayout), &pMsgBox);
        if (pMsgBox)
        {
            wchar_t Text[300];
            CMultiLanguage::ConvertFromUtf8(Text, g_DialogScript[Data->Cmd2].m_lpszText);
            pMsgBox->AddMsg(Text);
        }
    }
    break;

    case 6:
        SEASON3B::CreateOkMessageBox(GlobalText[449]);
        break;
    case 13:
        SEASON3B::CreateOkMessageBox(GlobalText[1826]);
        break;
    case 14:
    {
        switch (Data->Cmd2)
        {
        case 0:
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CWhiteAngelEventLayout));
            break;

        case 1:
            SEASON3B::CreateOkMessageBox(GlobalText[2024]);
            break;

        case 2:
            SEASON3B::CreateOkMessageBox(GlobalText[2022]);
            break;

        case 3:
            SEASON3B::CreateOkMessageBox(GlobalText[2023]);
            break;

        case 4:
            //ShowCustomMessageBox(GlobalText[858]);
            break;

        case 5:
            SEASON3B::CreateOkMessageBox(GlobalText[2023]);
            break;
        }
    }
    break;
    case 15:
    {
        switch (Data->Cmd2)
        {
        case 0:
            SEASON3B::CreateOkMessageBox(GlobalText[2022]);
            break;
        case 1:
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CHarvestEventLayout));
            break;
        case 2:
            SEASON3B::CreateOkMessageBox(GlobalText[2023]);
            break;
        }
    }
    break;
    case 16:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox = nullptr;

        switch (Data->Cmd2)
        {
        case 0:
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSantaTownSantaMsgBoxLayout), &pMsgBox);
            pMsgBox->AddMsg(GlobalText[2588]);
            break;
        case 1:
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSantaTownSantaMsgBoxLayout), &pMsgBox);
            pMsgBox->AddMsg(GlobalText[2585]);
            break;
        case 2:
            SEASON3B::CreateOkMessageBox(GlobalText[2587]);
            break;
        case 3:
            SEASON3B::CreateOkMessageBox(GlobalText[2023]);
            break;
        }
    }
    break;
    case 17:
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSantaTownLeaveMsgBoxLayout));
        break;
    case 47:
    case 48:
    case 49:
        SEASON3B::CreateOkMessageBox(GlobalText[1823 + Data->Cmd1 - 47]);
        break;
    case 55:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[2043], GlobalText[39]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    case 56:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[2043], GlobalText[56]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    case 57:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[2043], GlobalText[57]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    case 58:
    {
        WORD Key = ((WORD)(Data->Cmd2) << 8) + Data->Cmd3;
        Key &= 0x7FFF;
        int Index = FindCharacterIndex(Key);
        if (Index >= 0 && Index != MAX_CHARACTERS_CLIENT) {
            OBJECT* to = &CharactersClient[Index].Object;
            if (to != nullptr) {
                CreateEffect(MODEL_EFFECT_SKURA_ITEM, to->Position, to->Angle, to->Light, 0, to);
                PlayBuffer(SOUND_CHERRYBLOSSOM_EFFECT0, to);
            }
        }
    }
    break;
    case 59:
    {
        vec3_t Position, Angle, Light;
        Position[0] = (float)(Data->Cmd2 + 0.5f) * TERRAIN_SCALE;
        Position[1] = (float)(Data->Cmd3 + 0.5f) * TERRAIN_SCALE;
        Position[2] = RequestTerrainHeight(Position[0], Position[1]);
        ZeroMemory(Angle, sizeof(Angle));
        Light[0] = Light[1] = Light[2] = 1.0f;
        CreateEffect(BITMAP_FIRECRACKER0001, Position, Angle, Light, 1);
    }
    break;
    case 60:
        g_PortalMgr.ResetRevivePosition();
        break;
    case 61:
        g_PortalMgr.ResetPortalPosition();
        break;
    }
}

void ReceiveMixExit(const BYTE* ReceiveBuffer)
{
    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x87 [ReceiveMixExit]");
}

void ReceiveGemMixResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_JEWEL_MIX)ReceiveBuffer;
    wchar_t sBuf[256];
    memset(sBuf, 0, 256);
    switch (Data->m_iResult)
    {
    case 0:
    case 2:
    case 3:
    {
        swprintf(sBuf, L"%s%s %s", GlobalText[1801], GlobalText[1816], GlobalText[868]);
        g_pSystemLogBox->AddText(sBuf, SEASON3B::TYPE_SYSTEM_MESSAGE);
        COMGEM::GetBack();
    }
    break;
    case 1:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationUnityResultMsgBoxLayout));
    }
    break;
    case 4:
    {
        g_pSystemLogBox->AddText(GlobalText[1817], SEASON3B::TYPE_SYSTEM_MESSAGE);
        COMGEM::GetBack();
    }
    break;
    case 5:
    {
        g_pSystemLogBox->AddText(GlobalText[1811], SEASON3B::TYPE_SYSTEM_MESSAGE);
        COMGEM::GetBack();
    }
    break;
    }
}

void ReceiveGemUnMixResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_JEWEL_UNMIX)ReceiveBuffer;
    wchar_t sBuf[256];
    memset(sBuf, 0, 256);

    switch (Data->m_iResult)
    {
    case 0:
    case 5:
    {
        swprintf(sBuf, L"%s%s %s", GlobalText[1800], GlobalText[1816], GlobalText[868]);
        g_pSystemLogBox->AddText(sBuf, SEASON3B::TYPE_SYSTEM_MESSAGE);
        COMGEM::GetBack();
    }
    break;
    case 1:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGemIntegrationDisjointResultMsgBoxLayout));
    }
    break;
    case 2:
    case 3:
    case 4:
    case 6:
    {
        g_pSystemLogBox->AddText(GlobalText[1812], SEASON3B::TYPE_SYSTEM_MESSAGE);
        COMGEM::GetBack();
    }
    break;
    case 7:
    {
        g_pSystemLogBox->AddText(GlobalText[1815], SEASON3B::TYPE_SYSTEM_MESSAGE);
        COMGEM::GetBack();
    }
    break;
    case 8:
    {
        g_pSystemLogBox->AddText(GlobalText[1811], SEASON3B::TYPE_SYSTEM_MESSAGE);
        COMGEM::GetBack();
    }
    break;
    }
}

void ReceiveMoveToDevilSquareResult(const BYTE* ReceiveBuffer)
{
    g_pNewUISystem->Hide(SEASON3B::INTERFACE_DEVILSQUARE);

    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:
        break;
    case 1:
        SEASON3B::CreateOkMessageBox(GlobalText[677]);
        break;

    case 2:
        SEASON3B::CreateOkMessageBox(GlobalText[678]);
        break;

    case 3:
        SEASON3B::CreateOkMessageBox(GlobalText[686]);
        break;

    case 4:
        SEASON3B::CreateOkMessageBox(GlobalText[687]);
        break;

    case 5:
        SEASON3B::CreateOkMessageBox(GlobalText[679]);
        break;

    case 6:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[2043], GlobalText[39]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    }
}

void ReceiveDevilSquareOpenTime(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    if (0 == Data->Value)
    {
        SEASON3B::CreateOkMessageBox(GlobalText[643]);
    }
    else
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[644], (int)Data->Value);
        SEASON3B::CreateOkMessageBox(strText);
    }
}

void ReceiveDevilSquareCountDown(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;

    if (gMapManager.IsCursedTemple())
    {
        if (Data->Value == TYPE_MATCH_CURSEDTEMPLE_ENTER_CLOSE
            || Data->Value == TYPE_MATCH_CURSEDTEMPLE_GAME_START)
        {
            g_CursedTemple->SetInterfaceState(false, 0);
            matchEvent::StartMatchCountDown(Data->Value + 1);
        }
    }
    else if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4)
    {
        if (((BYTE)(Data->Value + 1) >= TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && (BYTE)(Data->Value + 1) <= TYPE_MATCH_DOPPELGANGER_CLOSE) || (BYTE)(Data->Value + 1) == TYPE_MATCH_NONE)
        {
            matchEvent::StartMatchCountDown((BYTE)(Data->Value + 1));
        }
    }
    else
    {
        matchEvent::StartMatchCountDown(Data->Value + 1);
    }
}

void ReceiveDevilSquareRank(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPDEVILRANK)ReceiveBuffer;
    matchEvent::SetMatchResult(Data->m_Count, Data->m_MyRank, (MatchResult*)&Data->m_byRank, Data->m_MyRank);
}

void ReceiveMoveToEventMatchResult(const BYTE* ReceiveBuffer)
{
    g_pNewUISystem->HideAll();

    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:
        break;
    case 1:
        SEASON3B::CreateOkMessageBox(GlobalText[854]);
        break;

    case 2:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[852], GlobalText[1146]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;

    case 3:
        SEASON3B::CreateOkMessageBox(GlobalText[686]);
        break;

    case 4:
        SEASON3B::CreateOkMessageBox(GlobalText[687]);
        break;

    case 5:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[853], GlobalText[1146], MAX_BLOOD_CASTLE_MEN);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;

    case 6:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[867], 6);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    case 7:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[2043], GlobalText[56]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    case 8:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[852], GlobalText[1147]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;

    case 9:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[853], GlobalText[1147], MAX_CHAOS_CASTLE_MEN);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    }
}

void ReceiveEventZoneOpenTime(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_MATCH_OPEN_VALUE)ReceiveBuffer;
    if (Data->Value == 1)
    {
        if (0 == Data->KeyH)
        {
            SEASON3B::CreateOkMessageBox(GlobalText[643]);
        }
        else
        {
            wchar_t strText[128];
            swprintf(strText, GlobalText[644], (int)Data->KeyH);
            SEASON3B::CreateOkMessageBox(strText);
        }
    }
    else if (Data->Value == 2)
    {
        wchar_t strText[256];
        if (0 == Data->KeyH)
        {
            swprintf(strText, GlobalText[850], GlobalText[1146]);
        }
        else
        {
            swprintf(strText, GlobalText[851], (int)Data->KeyH, GlobalText[1146]);
        }
        SEASON3B::CreateOkMessageBox(strText);
    }
    else if (Data->Value == 4)
    {
        WORD time = MAKEWORD(Data->KeyL, Data->KeyH);

        if (0 == time)
        {
            wchar_t szOpenTime1[256] = { 0, };
            wchar_t szOpenTime2[256] = { 0, };

            swprintf(szOpenTime1, GlobalText[850], GlobalText[1147]);
            swprintf(szOpenTime2, GlobalText[1156], GlobalText[1147], Data->KeyM, 100);

            GlobalText.Remove(1154);
            GlobalText.Remove(1155);
            GlobalText.Add(1154, szOpenTime1);
            GlobalText.Add(1155, szOpenTime2);

            SEASON3B::CNewUICommonMessageBox* pMsgBox = nullptr;
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CChaosCastleTimeCheckMsgBoxLayout), &pMsgBox);
            if (pMsgBox)
            {
                pMsgBox->AddMsg(GlobalText[1154]);
                pMsgBox->AddMsg(GlobalText[1155]);
            }
        }
        else
        {
            wchar_t Text[256];
            auto Hour = (int)(time / 60);
            int Mini = (int)(time)-(Hour * 60);

            wchar_t szOpenTime[256] = { 0, };

            swprintf(szOpenTime, GlobalText[1164], Hour);
            swprintf(Text, GlobalText[851], Mini, GlobalText[1147]);
            wcscat(szOpenTime, Text);

            GlobalText.Remove(1154);
            GlobalText.Add(1154, szOpenTime);

            SEASON3B::CNewUICommonMessageBox* pMsgBox = nullptr;
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CChaosCastleTimeCheckMsgBoxLayout), &pMsgBox);
            if (pMsgBox)
            {
                pMsgBox->AddMsg(GlobalText[1154]);
            }
        }
    }
    else if (Data->Value == 5)
    {
        wchar_t strText[256];
        if (0 == Data->KeyH)
        {
            swprintf(strText, GlobalText[850], GlobalText[2369]);
        }
        else
        {
            swprintf(strText, GlobalText[851], (int)Data->KeyH, GlobalText[2369]);
        }
        SEASON3B::CreateOkMessageBox(strText);
    }
}

void ReceiveMoveToEventMatchResult2(const BYTE* ReceiveBuffer)
{
    g_pNewUISystem->HideAll();

    auto Data = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
    switch (Data->Value)
    {
    case 0:
        break;
    case 1:
        SEASON3B::CreateOkMessageBox(GlobalText[854]);
        break;

    case 2:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[852], GlobalText[1147]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;

    case 3:
        SEASON3B::CreateOkMessageBox(GlobalText[686]);
        break;

    case 4:
        SEASON3B::CreateOkMessageBox(GlobalText[687]);
        break;

    case 5:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[853], GlobalText[1147], MAX_CHAOS_CASTLE_MEN);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;

    case 6:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[867], 6);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;

    case 7:
        SEASON3B::CreateOkMessageBox(GlobalText[423]);
        break;

    case 8:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[2043], GlobalText[57]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    }
}

void ReceiveSetAttribute(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_SET_MAPATTRIBUTE)ReceiveBuffer;

    g_ErrorReport.Write(L"Type:%d \r\n", Data->m_byType);

    switch (Data->m_byType)
    {
    case 0:
    {
        if (gMapManager.InBloodCastle() && Data->m_byMapAttr == TW_NOGROUND)
        {
            SetActionObject(gMapManager.WorldActive, 36, 0, 1.f);
        }

        for (int k = 0; k < Data->m_byCount; k++)
        {
            int dx = Data->m_vAttribute[(k * 2) + 1].m_byX - Data->m_vAttribute[(k * 2)].m_byX + 1;
            int dy = Data->m_vAttribute[(k * 2) + 1].m_byY - Data->m_vAttribute[(k * 2)].m_byY + 1;

            g_ErrorReport.Write(L"count:%d, x:%d, y:%d \r\n", Data->m_byCount, dx, dy);

            AddTerrainAttributeRange(Data->m_vAttribute[(k * 2)].m_byX, Data->m_vAttribute[(k * 2)].m_byY, dx, dy, Data->m_byMapAttr, 1 - Data->m_byMapSetType);
        }
    }
    break;

    case 1:
        for (int i = 0; i < Data->m_byCount; ++i)
        {
            if (Data->m_byMapSetType)
            {
                g_ErrorReport.Write(L"SubTerrainAttribute - count:%d, x:%d, y:%d \r\n", Data->m_byCount, Data->m_vAttribute[i].m_byX, Data->m_vAttribute[i].m_byY);
                SubTerrainAttribute(Data->m_vAttribute[i].m_byX, Data->m_vAttribute[i].m_byY, Data->m_byMapAttr);
            }
            else
            {
                g_ErrorReport.Write(L"AddTerrainAttribute - count:%d, x:%d, y:%d \r\n", Data->m_byCount, Data->m_vAttribute[i].m_byX, Data->m_vAttribute[i].m_byY);
                AddTerrainAttribute(Data->m_vAttribute[i].m_byX, Data->m_vAttribute[i].m_byY, Data->m_byMapAttr);
            }
        }
        break;
    }
}

void ReceiveMatchGameCommand(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_MATCH_GAME_STATE)ReceiveBuffer;

    matchEvent::SetMatchGameCommand(Data);
}

void ReceiveDuelRequest(const BYTE* ReceiveBuffer)
{
    if (g_MessageBox->IsEmpty() == false)
    {
        return;
    }

    auto Data = (LPPMSG_REQ_DUEL_ANSWER)ReceiveBuffer;
    wchar_t playerName[MAX_ID_SIZE + 1]{};
    CMultiLanguage::ConvertFromUtf8(playerName, Data->szID, MAX_ID_SIZE);

    g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL, Data->bIndexH), playerName);

    if (g_pNewUISystem->IsImpossibleDuelInterface() == true)
    {
        g_DuelMgr.SendDuelRequestAnswer(DUEL_ENEMY, FALSE);
        return;
    }

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelMsgBoxLayout));
    PlayBuffer(SOUND_OPEN_DUELWINDOW);
}

void ReceiveDuelStart(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_DUEL_INVITE)ReceiveBuffer;
    wchar_t szMessage[256];
    wchar_t playerName[MAX_ID_SIZE + 1]{};
    CMultiLanguage::ConvertFromUtf8(playerName, Data->szID, MAX_ID_SIZE);
    if (Data->nResult == 0)
    {
        g_DuelMgr.EnableDuel(TRUE);
        g_DuelMgr.SetHeroAsDuelPlayer(DUEL_HERO);
        g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL, Data->bIndexH), playerName);
        swprintf(szMessage, GlobalText[912], g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY));
        g_pSystemLogBox->AddText(szMessage, SEASON3B::TYPE_ERROR_MESSAGE);

        g_pNewUISystem->Show(SEASON3B::INTERFACE_DUEL_WINDOW);
        PlayBuffer(SOUND_START_DUEL);
    }
    else if (Data->nResult == 15)
    {
        g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL, Data->bIndexH), playerName);
        swprintf(szMessage, GlobalText[913], g_DuelMgr.GetDuelPlayerID(DUEL_ENEMY));
        g_pSystemLogBox->AddText(szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
    }
    else if (Data->nResult == 16)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelCreateErrorMsgBoxLayout));
    }
    else if (Data->nResult == 28)
    {
        g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL, Data->bIndexH), playerName);
        swprintf(szMessage, GlobalText[2704], 30);
        g_pSystemLogBox->AddText(szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
    }
    else if (Data->nResult == 30)
    {
        g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL, Data->bIndexH), playerName);
        swprintf(szMessage, GlobalText[1811]);
        g_pSystemLogBox->AddText(szMessage, SEASON3B::TYPE_ERROR_MESSAGE);
    }
}

void ReceiveDuelEnd(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_DUEL_EXIT)ReceiveBuffer;

    if (Data->nResult == 0)
    {
        wchar_t playerName[MAX_ID_SIZE + 1]{};
        CMultiLanguage::ConvertFromUtf8(playerName, Data->szID, MAX_ID_SIZE);
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUEL_WINDOW);
        g_DuelMgr.EnableDuel(FALSE);
        g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL, Data->bIndexH), playerName);

        g_pSystemLogBox->AddText(GlobalText[914], SEASON3B::TYPE_ERROR_MESSAGE);

        if (g_wtMatchTimeLeft.m_Type == 2)
            g_wtMatchTimeLeft.m_Time = 0;
        g_DuelMgr.EnablePetDuel(FALSE);
    }
    else
    {
    }
}

void ReceiveDuelScore(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DUEL_SCORE_BROADCAST)ReceiveBuffer;

    if (g_DuelMgr.IsDuelPlayer(MAKEWORD(Data->bIndexL1, Data->bIndexH1), DUEL_HERO))
    {
        g_DuelMgr.SetScore(DUEL_HERO, Data->btDuelScore1);
        g_DuelMgr.SetScore(DUEL_ENEMY, Data->btDuelScore2);
    }
    else if (g_DuelMgr.IsDuelPlayer(MAKEWORD(Data->bIndexL2, Data->bIndexH2), DUEL_HERO))
    {
        g_DuelMgr.SetScore(DUEL_HERO, Data->btDuelScore2);
        g_DuelMgr.SetScore(DUEL_ENEMY, Data->btDuelScore1);
    }
}

void ReceiveDuelHP(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DUEL_HP_BROADCAST)ReceiveBuffer;

    if (g_DuelMgr.IsDuelPlayer(MAKEWORD(Data->bIndexL1, Data->bIndexH1), DUEL_HERO))
    {
        g_DuelMgr.SetHP(DUEL_HERO, Data->btHP1);
        g_DuelMgr.SetHP(DUEL_ENEMY, Data->btHP2);
        g_DuelMgr.SetSD(DUEL_HERO, Data->btShield1);
        g_DuelMgr.SetSD(DUEL_ENEMY, Data->btShield2);
    }
    else if (g_DuelMgr.IsDuelPlayer(MAKEWORD(Data->bIndexL2, Data->bIndexH2), DUEL_HERO))
    {
        g_DuelMgr.SetHP(DUEL_HERO, Data->btHP2);
        g_DuelMgr.SetHP(DUEL_ENEMY, Data->btHP1);
        g_DuelMgr.SetSD(DUEL_HERO, Data->btShield2);
        g_DuelMgr.SetSD(DUEL_ENEMY, Data->btShield1);
    }
}

void ReceiveDuelChannelList(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_DUEL_CHANNELLIST)ReceiveBuffer;
    for (int i = 0; i < 4; ++i)
    {
        wchar_t name1[MAX_ID_SIZE + 1]{};
        wchar_t name2[MAX_ID_SIZE + 1]{};

        CMultiLanguage::ConvertFromUtf8(name1, Data->channel[i].szID1, MAX_ID_SIZE);
        CMultiLanguage::ConvertFromUtf8(name2, Data->channel[i].szID2, MAX_ID_SIZE);
        g_DuelMgr.SetDuelChannel(i, Data->channel[i].bStart, Data->channel[i].bWatch, name1, name2);
    }
}

void ReceiveDuelWatchRequestReply(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_DUEL_JOINCNANNEL)ReceiveBuffer;
    if (Data->nResult == 0)
    {
        wchar_t name1[MAX_ID_SIZE + 1]{};
        wchar_t name2[MAX_ID_SIZE + 1]{};

        CMultiLanguage::ConvertFromUtf8(name1, Data->szID1, MAX_ID_SIZE);
        CMultiLanguage::ConvertFromUtf8(name2, Data->szID2, MAX_ID_SIZE);

        g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUELWATCH);

        g_DuelMgr.SetCurrentChannel(Data->nChannelId);
        g_DuelMgr.SetDuelPlayer(DUEL_HERO, MAKEWORD(Data->bIndexL1, Data->bIndexH1), name1);
        g_DuelMgr.SetDuelPlayer(DUEL_ENEMY, MAKEWORD(Data->bIndexL2, Data->bIndexH2), name2);
    }
    else if (Data->nResult == 16)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelCreateErrorMsgBoxLayout));
    }
    else if (Data->nResult == 27)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelWatchErrorMsgBoxLayout));
    }
    else
    {
    }
}

void ReceiveDuelWatcherJoin(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DUEL_JOINCNANNEL_BROADCAST)ReceiveBuffer;

    wchar_t name[MAX_ID_SIZE + 1]{};
    CMultiLanguage::ConvertFromUtf8(name, Data->szID, MAX_ID_SIZE);
    g_DuelMgr.AddDuelWatchUser(name);
}

void ReceiveDuelWatchEnd(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_DUEL_LEAVECNANNEL)ReceiveBuffer;
    if (Data->nResult == 0)
    {
        g_DuelMgr.RemoveAllDuelWatchUser();
    }
    else
    {
    }
}

void ReceiveDuelWatcherQuit(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DUEL_LEAVECNANNEL_BROADCAST)ReceiveBuffer;
    wchar_t name[MAX_ID_SIZE + 1]{};
    CMultiLanguage::ConvertFromUtf8(name, Data->szID, MAX_ID_SIZE);
    g_DuelMgr.RemoveDuelWatchUser(name);
}

void ReceiveDuelWatcherList(const BYTE* ReceiveBuffer)
{
    g_DuelMgr.RemoveAllDuelWatchUser();

    auto Data = (LPPMSG_DUEL_OBSERVERLIST_BROADCAST)ReceiveBuffer;
    for (int i = 0; i < Data->nCount; ++i)
    {
        wchar_t name[MAX_ID_SIZE + 1]{};
        CMultiLanguage::ConvertFromUtf8(name, Data->user[i].szID, MAX_ID_SIZE);
        g_DuelMgr.AddDuelWatchUser(name);
    }
}

void ReceiveDuelResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DUEL_RESULT_BROADCAST)ReceiveBuffer;

    wchar_t szMessage[256];
    swprintf(szMessage, GlobalText[2689], 10);
    g_pSystemLogBox->AddText(szMessage, SEASON3B::TYPE_SYSTEM_MESSAGE);

    SEASON3B::CDuelResultMsgBox* lpMsgBox = nullptr;
    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDuelResultMsgBoxLayout), &lpMsgBox);
    if (lpMsgBox)
    {
        wchar_t winnerName[MAX_ID_SIZE + 1]{};
        wchar_t loserName[MAX_ID_SIZE + 1]{};
        CMultiLanguage::ConvertFromUtf8(winnerName, Data->szWinner, MAX_ID_SIZE);
        CMultiLanguage::ConvertFromUtf8(loserName, Data->szLoser, MAX_ID_SIZE);
        lpMsgBox->SetIDs(winnerName, loserName);
    }
    PlayBuffer(SOUND_OPEN_DUELWINDOW);
}

void ReceiveDuelRound(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DUEL_ROUNDSTART_BROADCAST)ReceiveBuffer;

    if (Data->nFlag == 0)
    {
        g_DuelMgr.SetHP(DUEL_HERO, 100);
        g_DuelMgr.SetHP(DUEL_ENEMY, 100);
        g_DuelMgr.SetSD(DUEL_HERO, 100);
        g_DuelMgr.SetSD(DUEL_ENEMY, 100);
        g_DuelMgr.SetFighterRegenerated(TRUE);
    }
    else if (Data->nFlag == 1)
    {
    }
}

void ReceiveCreateShopTitleViewport(const BYTE* ReceiveBuffer)
{
    auto Header = (LPPSHOPTITLE_HEADERINFO)ReceiveBuffer;

    auto* pShopTitle = (PSHOPTITLE_DATAINFO*)(ReceiveBuffer + sizeof(PSHOPTITLE_HEADERINFO));
    for (int i = 0; i < Header->byCount; i++, pShopTitle++) {
        int key = MAKEWORD(pShopTitle->byIndexL, pShopTitle->byIndexH);
        int index = FindCharacterIndex(key);
        if (index >= 0 && index < MAX_CHARACTERS_CLIENT) {
            CHARACTER* pPlayer = &CharactersClient[index];

            wchar_t szShopTitle[MAX_SHOPTITLE + 1] { };
            CMultiLanguage::ConvertFromUtf8(szShopTitle, pShopTitle->szTitle, MAX_SHOPTITLE);

            if (pPlayer == Hero)
            {
                wcscpy(g_szPersonalShopTitle, szShopTitle);
                g_pMyShopInventory->SetTitle(szShopTitle);
                g_pMyShopInventory->ChangePersonal(true);
            }

            AddShopTitle(key, pPlayer, szShopTitle);
        }
    }
}

void ReceiveShopTitleChange(const BYTE* ReceiveBuffer)
{
    auto Header = (LPPSHOPTITLE_CHANGEINFO)ReceiveBuffer;

    int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
    int index = FindCharacterIndex(key);
    if (index >= 0 && index < MAX_CHARACTERS_CLIENT) {
        CHARACTER* pPlayer = &CharactersClient[index];
        wchar_t szShopTitle[40]{};
        CMultiLanguage::ConvertFromUtf8(szShopTitle, Header->szTitle, MAX_SHOPTITLE);

        wchar_t szID[MAX_ID_SIZE + 1]{};
        CMultiLanguage::ConvertFromUtf8(szID, Header->szId, MAX_ID_SIZE);

        if (wcsncmp(pPlayer->ID, szID, MAX_ID_SIZE) == 0)
            AddShopTitle(key, pPlayer, (const wchar_t*)szShopTitle);
    }
}

void ReceiveSetPriceResult(const BYTE* ReceiveBuffer)
{
    auto Header = (LPPSHOPSETPRICE_RESULTINFO)ReceiveBuffer;

    if (Header->byResult != 0x01 && g_IsPurchaseShop == PSHOPWNDTYPE_SALE)
    {
        // Header->byResult == 0x06
        if (SEASON3B::CNewUIInventoryCtrl::GetPickedItem())
        {
            SEASON3B::CNewUIInventoryCtrl::DeletePickedItem();
        }

        RemovePersonalItemPrice(g_pMyShopInventory->GetTargetIndex(), PSHOPWNDTYPE_SALE);

        SocketClient->ToGameServer()->SendInventoryRequest();

        g_ErrorReport.Write(L"@ [Fault] ReceiveSetPriceResult (result : %d)\n", Header->byResult);
    }
}

void ReceiveCreatePersonalShop(const BYTE* ReceiveBuffer)
{
    auto Header = (LPCREATEPSHOP_RESULSTINFO)ReceiveBuffer;
    if (Header->byResult == 0x01)
    {
        g_pMyShopInventory->ChangePersonal(true);
        AddShopTitle(Hero->Key, Hero, g_szPersonalShopTitle);
    }
    else
    {
        // Header->btResult == 0x03
        g_ErrorReport.Write(L"@ [Fault] ReceiveCreatePersonalShop (result : %d)\n", Header->byResult);
    }
}
void ReceiveDestroyPersonalShop(const BYTE* ReceiveBuffer)
{
    auto Header = (LPDESTROYPSHOP_RESULTINFO)ReceiveBuffer;
    if (Header->byResult == 0x01)
    {
        int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
        int index = FindCharacterIndex(key);
        if (index >= 0 && index < MAX_CHARACTERS_CLIENT)
        {
            CHARACTER* pPlayer = &CharactersClient[index];
            if (pPlayer == Hero)
            {
                g_pMyShopInventory->ChangePersonal(false);
            }
            RemoveShopTitle(pPlayer);
        }
    }
    else
    {
        g_ErrorReport.Write(L"@ [Fault] ReceiveDestroyPersonalShop (result : %d)\n", Header->byResult);
    }
}

void ReceivePersonalShopItemList(std::span<const BYTE> ReceiveBuffer)
{
    auto Header = safe_cast<GETPSHOPITEMLIST_HEADERINFO>(ReceiveBuffer);
    if (Header == nullptr)
    {
        assert(false);
        return;
    }

    if (Header->byResult == Success)
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE))
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_STORAGE);
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_STORAGE_EXT);
        }

        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
        }

        g_PersonalShopSeller.Initialize();

        wchar_t shopName[MAX_SHOPTITLE + 1] {};
        CMultiLanguage::ConvertFromUtf8(shopName, Header->szShopTitle, MAX_SHOPTITLE);
        g_pPurchaseShopInventory->ChangeTitleText(shopName);
        g_pPurchaseShopInventory->GetInventoryCtrl()->RemoveAllItems();

        g_pNewUISystem->Show(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
        g_pMyInventory->ChangeMyShopButtonStateOpen();

        RemoveAllPerosnalItemPrice(PSHOPWNDTYPE_PURCHASE);	//. clear item price table
        int Offset = sizeof(GETPSHOPITEMLIST_HEADERINFO);
        for (int i = 0; i < Header->ItemCount; i++)
        {
            auto pShopItem = safe_cast<GETPSHOPITEM_DATAINFO>(ReceiveBuffer.subspan(Offset));
            if (pShopItem == nullptr)
            {
                assert(false);
                return;
            }

            Offset+=9;
            auto itemData = ReceiveBuffer.subspan(Offset);
            int length = CalcItemLength(itemData);
            itemData = itemData.subspan(0, length);

            // todo: use item prices as well when the UI is ready
            if (pShopItem->MoneyPrice > 0)
            {
                g_pPurchaseShopInventory->InsertItem(pShopItem->ItemSlot, itemData);
                AddPersonalItemPrice(pShopItem->ItemSlot, pShopItem->MoneyPrice, PSHOPWNDTYPE_PURCHASE);
            }
            else
            {
                g_ConsoleDebug->Write(MCD_ERROR, L"[ReceivePersonalShopItemList]Item Count : %d, Item Index : %d, Item Price : %d", Header->ItemCount, i, pShopItem->MoneyPrice);

                g_ErrorReport.Write(L"@ ReceivePersonalShopItemList - item price less than zero(%d)\n", pShopItem->MoneyPrice);

                g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);

                return;
            }

            Offset += length;
        }

        int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
        int index = FindCharacterIndex(key);

        g_pPurchaseShopInventory->ChangeShopCharacterIndex(index);
    }
    else
    {
        switch (Header->byResult)
        {
        case Fail1:
        {
            g_pSystemLogBox->AddText(GlobalText[1120], SEASON3B::TYPE_ERROR_MESSAGE);
        }
        break;
        case Fail2:
        default:
            g_ErrorReport.Write(L"@ [Fault] ReceivePersonalShopItemList (result : %d)\n", Header->byResult);
        }
    }

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x05 [ReceivePersonalShopItemList]");
}

void ReceiveRefreshItemList(std::span<const BYTE> ReceiveBuffer)
{
    auto Header = safe_cast<GETPSHOPITEMLIST_HEADERINFO>(ReceiveBuffer);
    if (Header == nullptr)
    {
        assert(false);
        return;
    }

    int Offset = sizeof(GETPSHOPITEMLIST_HEADERINFO);
    int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
    if (Header->byResult == Success && Hero->Key == key)
    {
        g_pMyShopInventory->GetInventoryCtrl()->RemoveAllItems();
        for (int i = 0; i < Header->ItemCount; i++)
        {
            auto pShopItem = safe_cast<GETPSHOPITEM_DATAINFO>(ReceiveBuffer.subspan(Offset));
            if (pShopItem == nullptr)
            {
                assert(false);
                return;
            }

            Offset += 9;
            auto itemData = ReceiveBuffer.subspan(Offset);
            int length = CalcItemLength(itemData);
            itemData = itemData.subspan(0, length);

            g_pMyShopInventory->InsertItem(pShopItem->ItemSlot, itemData);
            AddPersonalItemPrice(pShopItem->ItemSlot, pShopItem->MoneyPrice, PSHOPWNDTYPE_SALE);

            Offset += length;
        }

        g_pMyShopInventory->ChangePersonal(true);
        g_bEnablePersonalShop = true;
        wchar_t shopName[MAX_SHOPTITLE + 1]{};
        CMultiLanguage::ConvertFromUtf8(shopName, Header->szShopTitle, MAX_SHOPTITLE);
        wcscpy(g_szPersonalShopTitle, shopName);
        AddShopTitle(key, Hero, shopName);
        g_pMyShopInventory->ChangeTitle(shopName);
    }
    else if (Header->byResult == Success && g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE)
    {
        g_pPurchaseShopInventory->GetInventoryCtrl()->RemoveAllItems();

        for (int i = 0; i < Header->ItemCount; i++)
        {
            auto pShopItem = safe_cast<GETPSHOPITEM_DATAINFO>(ReceiveBuffer.subspan(Offset));
            if (pShopItem == nullptr)
            {
                assert(false);
                return;
            }

            Offset += 9;
            auto itemData = ReceiveBuffer.subspan(Offset);
            int length = CalcItemLength(itemData);
            itemData = itemData.subspan(0, length);

            g_pPurchaseShopInventory->InsertItem(pShopItem->ItemSlot, itemData);
            AddPersonalItemPrice(pShopItem->ItemSlot, pShopItem->MoneyPrice, PSHOPWNDTYPE_PURCHASE);

            Offset += length;
        }
    }
    else
    {
        if (Header->byResult == 0x01)
        {
            auto pCurrentInvenCtrl = g_pPurchaseShopInventory->GetInventoryCtrl();

            size_t uiCntInvenCtrl = pCurrentInvenCtrl->GetNumberOfItems();
            g_ErrorReport.Write(L"@ [Notice] ReceiveRefreshItemList (InventoryCtrl Count Items(%d))\n", uiCntInvenCtrl);
        }
        else
        {
            g_ErrorReport.Write(L"@ [Fault] ReceiveRefreshItemList (result : %d)\n", Header->byResult);
        }
    }
}

void ReceivePurchaseItem(std::span<const BYTE> ReceiveBuffer)
{
    auto Header = safe_cast<PURCHASEITEM_RESULTINFO>(ReceiveBuffer);
    if (Header == nullptr)
    {
        assert(false);
        return;
    }

    if (Header->Result == PURCHASEITEM_RESULTINFO::BoughtSuccessfully)
    {
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY))
        {
            RemovePersonalItemPrice(g_pPurchaseShopInventory->GetSourceIndex(), PSHOPWNDTYPE_PURCHASE);
            g_pPurchaseShopInventory->DeleteItem(g_pPurchaseShopInventory->GetSourceIndex());
        }
        else
        {
            RemoveAllPerosnalItemPrice(PSHOPWNDTYPE_PURCHASE);
        }

        auto itemindex = Header->ItemSlot;
        auto offset = sizeof(PURCHASEITEM_RESULTINFO);
        auto itemData = ReceiveBuffer.subspan(offset);

        if (itemindex >= MAX_EQUIPMENT_INDEX && itemindex < MAX_MY_INVENTORY_INDEX)
        {
            g_pMyInventory->InsertItem(itemindex, itemData);
        }
        else if (itemindex > MAX_MY_INVENTORY_INDEX && itemindex < MAX_MY_INVENTORY_EX_INDEX)
        {
            g_pMyInventoryExt->InsertItem(itemindex, itemData);
        }
    }
    else if (Header->Result == PURCHASEITEM_RESULTINFO::NameMismatchOrPriceMissing)
    {
        g_pSystemLogBox->AddText(GlobalText[1166], SEASON3B::TYPE_ERROR_MESSAGE);
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);
    }
    else
    {
        switch (Header->Result)
        {
        case PURCHASEITEM_RESULTINFO::LackOfMoney:
        {
            g_pSystemLogBox->AddText(GlobalText[423], SEASON3B::TYPE_ERROR_MESSAGE);
        }
        break;
        case PURCHASEITEM_RESULTINFO::MoneyOverflowOrNotEnoughSpace:
        {
            g_pSystemLogBox->AddText(GlobalText[375], SEASON3B::TYPE_ERROR_MESSAGE);
        }
        break;
        case PURCHASEITEM_RESULTINFO::ItemBlock:
        default:
            g_ErrorReport.Write(L"@ [Fault] ReceivePurchaseItem (result : %d)\n", Header->Result);
        }
        SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
    }
}

void NotifySoldItem(const BYTE* ReceiveBuffer)
{
    auto Header = (LPSOLDITEM_RESULTINFO)ReceiveBuffer;
    wchar_t szId[MAX_ID_SIZE + 2] = { 0 };

    CMultiLanguage::ConvertFromUtf8(szId, Header->szId, MAX_ID_SIZE);
    wchar_t Text[100];
    swprintf(Text, GlobalText[1122], szId);
    g_pSystemLogBox->AddText(Text, SEASON3B::TYPE_SYSTEM_MESSAGE);
}

void NotifyClosePersonalShop(const BYTE* ReceiveBuffer)
{
    if (g_IsPurchaseShop == PSHOPWNDTYPE_PURCHASE)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY);

        g_pSystemLogBox->AddText(GlobalText[1126], SEASON3B::TYPE_ERROR_MESSAGE);
    }
}

void ReceiveDisplayEffectViewport(const BYTE* ReceiveBuffer)
{
    auto Header = (LPDISPLAYEREFFECT_NOTIFYINFO)ReceiveBuffer;

    int key = MAKEWORD(Header->byIndexL, Header->byIndexH);
    int index = FindCharacterIndex(key);
    if (index >= 0 && index < MAX_CHARACTERS_CLIENT) {
        CHARACTER* pPlayer = &CharactersClient[index];
        OBJECT* o = &pPlayer->Object;
        if (o->Kind == KIND_PLAYER) {
            switch (Header->byType) {
            case 0x01:	//. HP up
            {
#ifdef ENABLE_POTION_EFFECT
                CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 5, o);
#endif // ENABLE_POTION_EFFECT
            }
            break;
            case 0x02:	//. MP up
            {
            }
            break;
            case 0x10:	//. Level up
            {
                if (gCharacterManager.IsMasterLevel(pPlayer->Class) == true)
                {
                    CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 45, o, 80, 2);
                    for (int i = 0; i < 19; ++i)
                    {
                        CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 46, o, 80, 2);
                    }
                }
                else
                {
                    for (int i = 0; i < 15; ++i)
                    {
                        CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 0, o, 40, 2);
                    }
                    CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 0, o);
                }
                PlayBuffer(SOUND_LEVEL_UP);
            }
            break;
            case 0x03:
            {
                CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 5, o);
            }
            break;

            case 0x11:
            {
                if (!gMapManager.InChaosCastle())
                {
                    CreateEffect(MODEL_SHIELD_CRASH, o->Position, o->Angle, o->Light, 0, o);
                    PlayBuffer(SOUND_SHIELDCLASH);
                }
            }
            break;
            }	//. end of switch
        }	//. end of (o->Kind == KIND_PLAYER)
    }
}

int g_iMaxLetterCount = 0;

void ReceiveFriendList(const BYTE* ReceiveBuffer)
{
    g_pWindowMgr->Reset();
    auto Header = (LPFS_FRIEND_LIST_HEADER)ReceiveBuffer;
    int iMoveOffset = sizeof(FS_FRIEND_LIST_HEADER);
    wchar_t szName[MAX_ID_SIZE + 1] = { 0 };
    for (int i = 0; i < Header->Count; ++i)
    {
        auto Data = (LPFS_FRIEND_LIST_DATA)(ReceiveBuffer + iMoveOffset);
        CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_ID_SIZE);
        szName[MAX_ID_SIZE] = '\0';
        g_pFriendList->AddFriend(szName, 0, Data->Server);
        iMoveOffset += sizeof(FS_FRIEND_LIST_DATA);
    }
    g_pFriendList->Sort(0);
    g_pFriendList->Sort(1);
    g_pWindowMgr->RefreshMainWndPalList();

    // 채팅 서버 살아남
    g_pWindowMgr->SetServerEnable(TRUE);
    if (g_iChatInputType == 0)
    {
        SocketClient->ToGameServer()->SendSetFriendOnlineState(2);
    }

    g_iMaxLetterCount = Header->MaxMemo;

    if (Header->MemoCount > 0)
    {
        wchar_t temp[MAX_TEXT_LENGTH + 1];
        swprintf(temp, GlobalText[1072], Header->MemoCount, Header->MaxMemo);
        g_pSystemLogBox->AddText(temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
}

void ReceiveAddFriendResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_FRIEND_RESULT)ReceiveBuffer;

    wchar_t szName[MAX_ID_SIZE + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_ID_SIZE);
    szName[MAX_ID_SIZE] = '\0';

    wchar_t szText[MAX_TEXT_LENGTH + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szText, Data->Name, MAX_ID_SIZE);
    szText[MAX_ID_SIZE] = '\0';

    switch (Data->Result)
    {
    case 0x00:
        wcscat(szText, GlobalText[1047]);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
        break;
    case 0x01:
    {
        g_pSystemLogBox->AddText(GlobalText[1075], SEASON3B::TYPE_SYSTEM_MESSAGE);
        g_pFriendList->AddFriend(szName, 0, Data->Server);
        g_pFriendList->Sort();
        g_pWindowMgr->RefreshMainWndPalList();
        g_pFriendMenu->UpdateAllChatWindowInviteList();
    }
    break;
    case 0x03:
        wcscpy(szText, GlobalText[1048]);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
        break;
    case 0x04:
        wcscat(szText, GlobalText[1049]);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
        break;
    case 0x05:
        wcscpy(szText, GlobalText[1050]);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
        break;
    case 0x06:
        wcscpy(szText, GlobalText[1068]);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText);
        break;
    default:
        break;
    }
}

void ReceiveRequestAcceptAddFriend(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_ACCEPT_ADD_FRIEND_RESULT)ReceiveBuffer;

    wchar_t szName[MAX_ID_SIZE + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_ID_SIZE);
    szName[MAX_ID_SIZE] = '\0';

    wchar_t szText[MAX_TEXT_LENGTH + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szText, Data->Name, MAX_ID_SIZE);
    szText[MAX_ID_SIZE] = '\0';

    swprintf(szText, L"%s %s", szText, GlobalText[1051]); // " has requested to list you as a friend."

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_FRIEND) == false)
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_FRIEND);
    }

    DWORD dwWindowID = g_pWindowMgr->AddWindow(UIWNDTYPE_QUESTION_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, szText, -1);
    if (dwWindowID != 0)
    {
        ((CUIQuestionWindow*)g_pWindowMgr->GetWindow(dwWindowID))->SaveID(szName);
    }
    PlayBuffer(SOUND_FRIEND_LOGIN_ALERT);
}

void ReceiveDeleteFriendResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_FRIEND_RESULT)ReceiveBuffer;

    wchar_t szName[MAX_ID_SIZE + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_ID_SIZE);
    szName[MAX_ID_SIZE] = '\0';

    switch (Data->Result)
    {
    case 0x00:
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1052]);
        break;
    case 0x01:
        g_pFriendList->RemoveFriend(szName);
        g_pWindowMgr->RefreshMainWndPalList();
        break;
    default:
        break;
    }
}

void ReceiveFriendStateChange(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_FRIEND_STATE_CHANGE)ReceiveBuffer;

    wchar_t szName[MAX_ID_SIZE + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_ID_SIZE);
    szName[MAX_ID_SIZE] = '\0';

    if (Data->Server == 0xFC)
    {
        g_pFriendList->UpdateAllFriendState(0, Data->Server);
        g_pFriendList->Sort();
        g_pWindowMgr->RefreshMainWndPalList();
        g_pFriendMenu->LockAllChatWindow();
        g_pWindowMgr->SetServerEnable(FALSE);
        return;
    }
    g_pFriendList->UpdateFriendState(szName, 0, Data->Server);
    g_pFriendList->Sort();
    g_pWindowMgr->RefreshMainWndPalList();
    g_pFriendMenu->UpdateAllChatWindowInviteList();

    DWORD dwChatRoomUIID = g_pFriendMenu->CheckChatRoomDuplication(szName);
    if (dwChatRoomUIID > 0)
    {
        auto* pWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(dwChatRoomUIID);
        if (pWindow == nullptr);
        else if (Data->Server >= 0xFD/* || Data->Server == 0xFB*/)
        {
            pWindow->Lock(TRUE);
        }
        else
        {
            pWindow->Lock(FALSE);
        }
    }
}

void ReceiveLetterSendResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_SEND_LETTER_RESULT)ReceiveBuffer;
    switch (Data->Result)
    {
    case 0x00:
        if (Data->WindowGuid != 0)
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1053]);
        break;
    case 0x01:
    {
        if (Data->WindowGuid != 0)
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, Data->WindowGuid, 0);
        wchar_t temp[MAX_TEXT_LENGTH + 1];
        swprintf(temp, GlobalText[1046], g_cdwLetterCost);
        g_pSystemLogBox->AddText(temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
    break;
    case 0x02:
        if (Data->WindowGuid != 0)
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1061]);
        break;
    case 0x03:
        if (Data->WindowGuid != 0)
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1064]);
        break;
    case 0x04:
        if (Data->WindowGuid != 0)
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1065]);
        break;
    case 0x06:
        if (Data->WindowGuid != 0)
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1068]);
        break;
    case 0x07:
        if (Data->WindowGuid != 0)
            ((CUILetterWriteWindow*)g_pWindowMgr->GetWindow(Data->WindowGuid))->SetSendState(FALSE);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[423]);
        break;
    default:
        break;
    };
}

void ReceiveLetter(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_LETTER_ALERT)ReceiveBuffer;

    wchar_t szDate[MAX_LETTER_DATE_LENGTH + 1] = { };
    CMultiLanguage::ConvertFromUtf8(szDate, Data->Date, MAX_LETTER_DATE_LENGTH);

    wchar_t szTime[MAX_LETTER_TIME_LENGTH + 1] = { };
    CMultiLanguage::ConvertFromUtf8(szTime, Data->Time, MAX_LETTER_TIME_LENGTH);

    wchar_t szName[MAX_ID_SIZE + 1] = { };
    CMultiLanguage::ConvertFromUtf8(szName, Data->Name, MAX_ID_SIZE);
    szName[MAX_ID_SIZE] = '\0';

    wchar_t szSubject[MAX_TEXT_LENGTH + 1] = { };
    CMultiLanguage::ConvertFromUtf8(szSubject, Data->Subject, MAX_ID_SIZE);
    szSubject[MAX_ID_SIZE] = '\0';

    switch (Data->Read)
    {
    case 0x02:
        PlayBuffer(SOUND_FRIEND_MAIL_ALERT);
        g_pFriendMenu->SetNewMailAlert(TRUE);
        g_pSystemLogBox->AddText(GlobalText[1062], SEASON3B::TYPE_SYSTEM_MESSAGE);
        g_pLetterList->AddLetter(Data->Index, szName, szSubject, szDate, szTime, 0x00);
        g_pLetterList->Sort();
        break;
    case 0x00:
    case 0x01:
        g_pLetterList->AddLetter(Data->Index, szName, szSubject, szDate, szTime, Data->Read);
        g_pLetterList->Sort(2);
        break;
    default:
        break;
    };

    g_pWindowMgr->RefreshMainWndLetterList();

    if (g_pLetterList->GetLetterCount() >= g_iMaxLetterCount)
    {
        g_pSystemLogBox->AddText(GlobalText[1073], SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
}

extern int g_iLetterReadNextPos_x, g_iLetterReadNextPos_y;

void ReceiveLetterText(std::span<const BYTE> ReceiveBuffer, bool isCached)
{
    auto Data = safe_cast<FS_LETTER_TEXT_HEADER>(ReceiveBuffer);
    if (Data == nullptr)
    {
        assert(false);
        return;
    }

    if (!isCached)
    {
        // Cache it if you can :)
        auto CopiedData = new FS_LETTER_TEXT();
        memcpy(CopiedData, ReceiveBuffer.data(), ReceiveBuffer.size());
        g_pLetterList->CacheLetterText(Data->Index, CopiedData);
    }

    auto pLetterHead = g_pLetterList->GetLetter(Data->Index);
    if (pLetterHead == nullptr)
    {
        return;
    }

    pLetterHead->m_bIsRead = TRUE;
    g_pWindowMgr->RefreshMainWndLetterList();

    wchar_t tempTxt[MAX_TEXT_LENGTH + 1];
    swprintf(tempTxt, GlobalText[1054], pLetterHead->m_szText);
    DWORD dwUIID = 0;
    if (g_iLetterReadNextPos_x == UIWND_DEFAULT)
    {
        dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_READLETTER, 100, 100, tempTxt);
    }
    else
    {
        dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_READLETTER, g_iLetterReadNextPos_x, g_iLetterReadNextPos_y, tempTxt, 0, UIADDWND_FORCEPOSITION);
        g_iLetterReadNextPos_x = UIWND_DEFAULT;
    }

    auto* pWindow = (CUILetterReadWindow*)g_pWindowMgr->GetWindow(dwUIID);
    auto* pLetterText = (char*)ReceiveBuffer.subspan(sizeof(FS_LETTER_TEXT_HEADER)).data();
    wchar_t letterText[1000 + 1] = { };
    CMultiLanguage::ConvertFromUtf8(letterText, pLetterText, MAX_LETTERTEXT_LENGTH);
    letterText[MAX_LETTERTEXT_LENGTH] = '\0';
    pWindow->SetLetter(pLetterHead, letterText);

    g_pWindowMgr->SetLetterReadWindow(pLetterHead->m_dwLetterID, dwUIID);

    if (wcsnicmp(pLetterHead->m_szID, L"webzen", MAX_ID_SIZE) == 0)
    {
        pWindow->m_Photo.SetWebzenMail(TRUE);
    }
    else
    {
        pWindow->m_Photo.SetClass(gCharacterManager.ChangeServerClassTypeToClientClassType(Data->Class));
        pWindow->m_Photo.SetEquipmentPacket(Data->Equipment);
        pWindow->m_Photo.SetAnimation(Data->PhotoAction + AT_ATTACK1);
        int iAngle = Data->PhotoDir & 0x3F;
        int iZoom = (Data->PhotoDir & 0xC0) >> 6;
        pWindow->m_Photo.SetAngle(iAngle * 6);
        pWindow->m_Photo.SetZoom((iZoom * 10 + 80) / 100.0f);
    }
    pWindow->SendUIMessageDirect(UI_MESSAGE_LISTSCRLTOP, 0, 0);
}

void ReceiveLetterDeleteResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_LETTER_RESULT)ReceiveBuffer;
    switch (Data->Result)
    {
    case 0x00:
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1055]);
        break;
    case 0x01:
        g_pLetterList->RemoveLetter(Data->Index);
        g_pLetterList->RemoveLetterTextCache(Data->Index);
        break;
    default:
        break;
    };

    g_pWindowMgr->RefreshMainWndLetterList();
}

void ReceiveCreateChatRoomResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_CHAT_CREATE_RESULT)ReceiveBuffer;

    wchar_t szName[MAX_ID_SIZE + 1] = { 0 };
    CMultiLanguage::ConvertFromUtf8(szName, Data->ID, MAX_ID_SIZE);

    wchar_t szIP[sizeof(Data->IP) + 1] { };
    CMultiLanguage::ConvertFromUtf8(szIP, Data->IP, sizeof(Data->IP));

    switch (Data->Result)
    {
    case 0x00:
        g_pFriendMenu->RemoveRequestWindow(szName);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1069]);
        break;
    case 0x01:
        g_pFriendMenu->RemoveRequestWindow(szName);
        if (Data->Type == 0)
        {
            DWORD dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_CHAT, 100, 100, GlobalText[994]);
            ((CUIChatWindow*)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer(szIP, Data->RoomNumber, Data->Ticket);
        }
        else if (Data->Type == 1)
        {
            DWORD dwUIID = g_pFriendMenu->CheckChatRoomDuplication(szName);
            if (dwUIID == 0)
            {
                dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_CHAT_READY, 100, 100, GlobalText[994]);
                ((CUIChatWindow*)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer(szIP, Data->RoomNumber, Data->Ticket);
                g_pWindowMgr->GetWindow(dwUIID)->SetState(UISTATE_READY);
                g_pWindowMgr->SendUIMessage(UI_MESSAGE_BOTTOM, dwUIID, 0);

                g_pWindowMgr->GetWindow(dwUIID)->SetState(UISTATE_HIDE);
                g_pWindowMgr->SendUIMessage(UI_MESSAGE_SELECT, dwUIID, 0);
            }
            else if (dwUIID == -1);
            else
            {
                ((CUIChatWindow*)g_pWindowMgr->GetWindow(dwUIID))->DisconnectToChatServer();
                ((CUIChatWindow*)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer(szIP, Data->RoomNumber, Data->Ticket);
            }
        }
        else if (Data->Type == 2)
        {
            DWORD dwUIID = g_pWindowMgr->AddWindow(UIWNDTYPE_CHAT_READY, 100, 100, GlobalText[994]);
            ((CUIChatWindow*)g_pWindowMgr->GetWindow(dwUIID))->ConnectToChatServer(szIP, Data->RoomNumber, Data->Ticket);
            g_pWindowMgr->GetWindow(dwUIID)->SetState(UISTATE_READY);
            g_pWindowMgr->SendUIMessage(UI_MESSAGE_BOTTOM, dwUIID, 0);
        }
        break;
    case 0x02:
        g_pFriendMenu->RemoveRequestWindow(szName);
        g_pWindowMgr->AddWindow(UIWNDTYPE_OK_FORCE, UIWND_DEFAULT, UIWND_DEFAULT, GlobalText[1070]);
        break;
    default:
        break;
    };
}

void ReceiveChatRoomInviteResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPFS_CHAT_INVITE_RESULT)ReceiveBuffer;
    auto* pChatWindow = (CUIChatWindow*)g_pWindowMgr->GetWindow(Data->WindowGuid);
    if (pChatWindow == nullptr) return;

    switch (Data->Result)
    {
    case 0x00:
        pChatWindow->AddChatText(255, GlobalText[1056], 1, 0);
        break;
    case 0x01:
        if (pChatWindow->GetCurrentInvitePal() != nullptr)
        {
            wchar_t szText[MAX_TEXT_LENGTH + 1] = { 0 };
            wcsncpy(szText, pChatWindow->GetCurrentInvitePal()->m_szID, MAX_ID_SIZE);
            szText[MAX_ID_SIZE] = '\0';
            wcscat(szText, GlobalText[1057]);
            pChatWindow->AddChatText(255, szText, 1, 0);
        }
        else
        {
            assert(!"ReceiveChatRoomInviteResult");
        }
        break;
    case 0x03:
        pChatWindow->AddChatText(255, GlobalText[1074], 1, 0);
        break;
    default:
        break;
    };
}

void ReceiveOption(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_OPTION)ReceiveBuffer;

    g_pMainFrame->ResetSkillHotKey();

    int iHotKey;
    for (int i = 0; i < 10; ++i)
    {
        int iIndex = i * 2;
        iHotKey = MAKEWORD(Data->HotKey[iIndex + 1], Data->HotKey[iIndex]);

        if (iHotKey != 0xffff)
        {
            for (int j = 0; j < MAX_SKILLS; ++j)
            {
                if (iHotKey == CharacterAttribute->Skill[j])
                {
                    g_pMainFrame->SetSkillHotKey(i, j);
                    break;
                }
            }
        }
    }

    if ((Data->GameOption & AUTOATTACK_ON) == AUTOATTACK_ON)
    {
        g_pOption->SetAutoAttack(true);
    }
    else
    {
        g_pOption->SetAutoAttack(false);
    }

    if ((Data->GameOption & WHISPER_SOUND_ON) == WHISPER_SOUND_ON)
    {
        g_pOption->SetWhisperSound(true);
    }
    else
    {
        g_pOption->SetWhisperSound(false);
    }

    if ((Data->GameOption & SLIDE_HELP_OFF) == SLIDE_HELP_OFF)
    {
        g_pOption->SetSlideHelp(false);
    }
    else
    {
        g_pOption->SetSlideHelp(true);
    }

    BYTE byQLevel, byWLevel, byELevel, byRLevel;
    byQLevel = (Data->QWERLevel & 0xFF000000) >> 24;
    byWLevel = (Data->QWERLevel & 0x00FF0000) >> 16;
    byELevel = (Data->QWERLevel & 0x0000FF00) >> 8;
    byRLevel = Data->QWERLevel & 0x000000FF;

    g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_Q, Data->KeyQWE[0] + ITEM_POTION, byQLevel);
    g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_W, Data->KeyQWE[1] + ITEM_POTION, byWLevel);
    g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_E, Data->KeyQWE[2] + ITEM_POTION, byELevel);

    BYTE wChatListBoxSize = (Data->ChatLogBox >> 4) * 3;
    BYTE wChatListBoxBackAlpha = Data->ChatLogBox & 0x0F;

    g_pMainFrame->SetItemHotKey(SEASON3B::HOTKEY_R, Data->KeyR + ITEM_POTION, byRLevel);
}

void ReceiveEventChipInfomation(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_EVENT_CHIP_INFO)ReceiveBuffer;

    g_pNewUISystem->HideAll();
    g_bEventChipDialogEnable = Data->m_byType + 1;
    g_shEventChipCount = Data->m_nChipCount;

    if (g_bEventChipDialogEnable == EVENT_SCRATCH_TICKET)
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_GOLD_BOWMAN);
        g_bEventChipDialogEnable = 0;
    }

    if (g_bEventChipDialogEnable == EVENT_LENA)
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA);
        g_bEventChipDialogEnable = 0;

        if (Data->m_shMutoNum[0] != -1 && Data->m_shMutoNum[1] != -1 && Data->m_shMutoNum[2] != -1) {
            memcpy(g_shMutoNumber, Data->m_shMutoNum, sizeof(short) * 3);
        }
    }

    if (g_bEventChipDialogEnable == EVENT_SCRATCH_TICKET)

        if (g_bEventChipDialogEnable == EVENT_SCRATCH_TICKET)
        {
            ZeroMemory(g_strGiftName, sizeof(char) * 64);

            ClearInput(FALSE);
            InputTextMax[0] = 12;
            InputNumber = 1;
            InputEnable = false;
            GoldInputEnable = false;
            InputGold = 0;
            StorageGoldFlag = 0;
            g_bScratchTicket = true;
        }
}

void ReceiveEventChip(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_EVENT_CHIP)ReceiveBuffer;
    if (Data->m_unChipCount != 0xFFFFFFFF)
        g_shEventChipCount = Data->m_unChipCount;
}

void ReceiveBuffState(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ITEMEFFECTCANCEL)ReceiveBuffer;

    auto bufftype = static_cast<eBuffState>(Data->byBuffType);

    if (bufftype == eBuffNone || bufftype >= eBuff_Count) return;

    if (Data->byEffectOption == 0)
    {
        RegisterBuff(bufftype, (&Hero->Object), Data->wEffectTime);

        if (bufftype == eBuff_HelpNpc)
        {
            g_pSystemLogBox->AddText(GlobalText[1828], SEASON3B::TYPE_SYSTEM_MESSAGE);
        }
    }
    else
    {
        UnRegisterBuff(bufftype, (&Hero->Object));
    }
}

void ReceiveMutoNumber(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_MUTONUMBER)ReceiveBuffer;

    if (Data->m_shMutoNum[0] != -1 && Data->m_shMutoNum[1] != -1 && Data->m_shMutoNum[2] != -1)
    {
        memcpy(g_shMutoNumber, Data->m_shMutoNum, sizeof(short) * 3);
    }
}

void ReceiveServerImmigration(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;

    switch (Data->Value)
    {
    case 0:
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CServerImmigrationErrorMsgBoxLayout));
        break;
    case 1:
        SEASON3B::CreateOkMessageBox(L"ReceiveServerImmigration");
        break;
    }
}

void ReceiveScratchResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_SCRATCH_TICKET_EVENT)ReceiveBuffer;

    switch (Data->m_byIsRegistered)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        SEASON3B::CreateOkMessageBox(GlobalText[886 + Data->m_byIsRegistered]);
        break;

    case 5:
        SEASON3B::CreateOkMessageBox(GlobalText[899]);
        break;
    }

    CMultiLanguage::ConvertFromUtf8(g_strGiftName, Data->m_strGiftName, sizeof Data->m_strGiftName);
}

void ReceivePlaySoundEffect(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_PLAY_SOUND_EFFECT)ReceiveBuffer;

    switch (Data->wEffectNum)
    {
    case 0:
        PlayBuffer(SOUND_RING_EVENT_READY);
        break;
    case 1:
        PlayBuffer(SOUND_RING_EVENT_START);
        break;
    case 2:
        PlayBuffer(SOUND_RING_EVENT_END);
        break;
    default:
        break;
    }
}

void ReceiveEventCount(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_EVENT_COUNT)ReceiveBuffer;

    g_csQuest.SetEventCount(Data->m_wEventType, Data->m_wLeftEnterCount);
}

void ReceiveQuestHistory(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_QUEST_HISTORY)ReceiveBuffer;

    g_csQuest.setQuestLists(Data->m_byQuest, Data->m_byCount, Hero->Class);
}

void ReceiveQuestState(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_QUEST_STATE)ReceiveBuffer;

    g_csQuest.setQuestList(Data->m_byQuestIndex, Data->m_byState);
    g_pNewUISystem->HideAll();
    g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCQUEST);
}

void ReceiveQuestResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_QUEST_RESULT)ReceiveBuffer;

    if (Data->m_byResult == 0)
    {
        g_csQuest.setQuestList(Data->m_byQuestIndex, Data->m_byState);
        g_pNewUISystem->HideAll();
        g_pNewUISystem->Show(SEASON3B::INTERFACE_NPCQUEST);
    }
}

void ReceiveQuestPrize(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_QUEST_REPARATION)ReceiveBuffer;

    WORD Key = ((WORD)(Data->m_byKeyH) << 8) + Data->m_byKeyL;
    Key &= 0x7FFF;
    int Index = FindCharacterIndex(Key);

    switch (Data->m_byReparation)
    {
    case 200:
    {
        CHARACTER* c = &CharactersClient[Index];

        if (c == Hero)
        {
            CharacterAttribute->LevelUpPoint += Data->m_byNumber;
        }

        OBJECT* o = &c->Object;

        vec3_t      Position;
        VectorCopy(o->Position, Position);
        for (int i = 0; i < 15; ++i)
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 0, o, 40, 2);
        }
        CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 0, o);
        PlayBuffer(SOUND_CHANGE_UP);
    }
    break;

    case 201:
    {
        // Evolution from 1st to 2nd class
        CHARACTER* c = &CharactersClient[Index];
        OBJECT* o = &c->Object;
        vec3_t      Position;

        auto byClass = gCharacterManager.ChangeServerClassTypeToClientClassType(static_cast<SERVER_CLASS_TYPE>((Data->m_byNumber >> 3)));
        if (2 != gCharacterManager.GetStepClass(byClass))
            break;

        c->Class = byClass;
        c->SkinIndex = gCharacterManager.GetSkinModelIndex(c->Class);

        if (Hero == c)
        {
            CharacterAttribute->Class = c->Class;
        }

        VectorCopy(o->Position, Position);
        Position[2] += 200.f;
        for (int i = 0; i < 15; ++i)
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 0, o, 40, 2);
            CreateJoint(BITMAP_FLARE, Position, Position, o->Angle, 10, o, 40, 2);
        }
        CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 0, o);

        SetChangeClass(c);
        SetAction(o, PLAYER_SALUTE1);

        PlayBuffer(SOUND_CHANGE_UP);
    }
    break;

    case 202:
    {
        CHARACTER* c = &CharactersClient[Index];
        OBJECT* o = &c->Object;
        vec3_t      Position;

        if (Hero == c)
        {
            CharacterAttribute->LevelUpPoint += Data->m_byNumber;
        }

        VectorCopy(o->Position, Position);
        Position[2] += 200.f;
        for (int i = 0; i < 15; ++i)
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 0, o, 40, 2);
            CreateJoint(BITMAP_FLARE, Position, Position, o->Angle, 10, o, 40, 2);
        }
        CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 0, o);

        SetChangeClass(c);
        SetAction(o, PLAYER_SALUTE1);
        PlayBuffer(SOUND_CHANGE_UP);
    }
    break;

    case 203:
    {
        CHARACTER* c = &CharactersClient[Index];
        OBJECT* o = &c->Object;
        vec3_t      Position;

        VectorCopy(o->Position, Position);
        Position[2] += 200.f;
        for (int i = 0; i < 15; ++i)
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 0, o, 40, 2);
            CreateJoint(BITMAP_FLARE, Position, Position, o->Angle, 10, o, 40, 2);
        }
        CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 0, o);

        SetChangeClass(c);
        SetAction(o, PLAYER_SALUTE1);

        PlayBuffer(SOUND_CHANGE_UP);
    }
    break;

    case 204:
    {
        // Evolution from 2nd to 3rd class
        CHARACTER* c = &CharactersClient[Index];

        auto byClass = gCharacterManager.ChangeServerClassTypeToClientClassType(static_cast<SERVER_CLASS_TYPE>((Data->m_byNumber >> 3)));
        if (3 != gCharacterManager.GetStepClass(byClass))
            break;

        c->Class = byClass;
        c->SkinIndex = gCharacterManager.GetSkinModelIndex(c->Class);

        if (Hero == c)
        {
            CharacterAttribute->Class = c->Class;
        }

        CreateEffect(MODEL_CHANGE_UP_EFF, c->Object.Position, c->Object.Angle, c->Object.Light, 0, &c->Object);
        CreateEffect(MODEL_CHANGE_UP_NASA, c->Object.Position, c->Object.Angle, c->Object.Light, 0, &c->Object);
        CreateEffect(MODEL_CHANGE_UP_CYLINDER, c->Object.Position, c->Object.Angle, c->Object.Light, 0, &c->Object);
        CreateEffect(MODEL_INFINITY_ARROW3, c->Object.Position, c->Object.Angle, c->Object.Light, 1, &c->Object);

        SetChangeClass(c);
        SetAction(&c->Object, PLAYER_CHANGE_UP);
        PlayBuffer(SOUND_CHANGE_UP);
    }
    break;

    default:
        break;
    }
}

void ReceiveQuestMonKillInfo(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_QUEST_MONKILL_INFO)ReceiveBuffer;

    g_csQuest.SetKillMobInfo(pData->anKillCountInfo);
}

#ifdef ASG_ADD_TIME_LIMIT_QUEST

void ReceiveQuestLimitResult(const BYTE* ReceiveBuffer)
{
    LPPMSG_ANS_QUESTEXP_RESULT pData = (LPPMSG_ANS_QUESTEXP_RESULT)ReceiveBuffer;

    g_pNewUISystem->HideAll();

    switch (pData->m_byResult)
    {
    case QUEST_RESULT_CNT_LIMIT:
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CQuestCountLimitMsgBoxLayout));
        break;
    }
}

void ReceiveQuestByItemUseEP(const BYTE* ReceiveBuffer)
{
    DWORD* pdwQuestIndex = (DWORD*)(ReceiveBuffer + sizeof(PMSG_NPCTALK_QUESTLIST));
    SendQuestSelection(*pdwQuestIndex, 0);
}
#endif	// ASG_ADD_TIME_LIMIT_QUEST

void ReceiveQuestByEtcEPList(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NPCTALK_QUESTLIST)ReceiveBuffer;
    g_QuestMng.SetQuestIndexByEtcList((DWORD*)(ReceiveBuffer + sizeof(PMSG_NPCTALK_QUESTLIST)),
        pData->m_wQuestCount);
}

void ReceiveQuestByNPCEPList(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NPCTALK_QUESTLIST)ReceiveBuffer;
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
        g_pNPCDialogue->ProcessQuestListReceive(
            (DWORD*)(ReceiveBuffer + sizeof(PMSG_NPCTALK_QUESTLIST)), pData->m_wQuestCount);
}

void ReceiveQuestQSSelSentence(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NPC_QUESTEXP_INFO)ReceiveBuffer;

    g_QuestMng.SetCurQuestProgress(pData->m_dwQuestIndex);
}

void ReceiveQuestQSRequestReward(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NPC_QUESTEXP_INFO)ReceiveBuffer;
    g_QuestMng.SetQuestRequestReward(ReceiveBuffer);
    g_QuestMng.SetCurQuestProgress(pData->m_dwQuestIndex);
    g_QuestMng.SetEPRequestRewardState(pData->m_dwQuestIndex, true);
}

void ReceiveQuestCompleteResult(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_QUESTEXP_COMPLETE)ReceiveBuffer;

    switch (pData->m_byResult)
    {
    case 0:
        break;
    case 1:
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS);
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);

        g_QuestMng.SetEPRequestRewardState(pData->m_dwQuestIndex, false);
        g_QuestMng.RemoveCurQuestIndexList(pData->m_dwQuestIndex);
        break;

    case 2:
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
            g_pQuestProgress->EnableCompleteBtn(false);
        else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
            g_pQuestProgressByEtc->EnableCompleteBtn(false);
        g_pSystemLogBox->AddText(GlobalText[2816], SEASON3B::TYPE_ERROR_MESSAGE);
        break;

    case 3:
        if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
            g_pQuestProgress->EnableCompleteBtn(false);
        else if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
            g_pQuestProgressByEtc->EnableCompleteBtn(false);
        g_pSystemLogBox->AddText(GlobalText[375], SEASON3B::TYPE_ERROR_MESSAGE);
        g_pSystemLogBox->AddText(GlobalText[374], SEASON3B::TYPE_ERROR_MESSAGE);
        break;
    }
}

void ReceiveQuestGiveUp(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_QUESTEXP_GIVEUP)ReceiveBuffer;
    g_QuestMng.RemoveCurQuestIndexList(pData->m_dwQuestGiveUpIndex);
}

void ReceiveProgressQuestList(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_QUESTEXP_PROGRESS_LIST)ReceiveBuffer;
    g_QuestMng.SetCurQuestIndexList((DWORD*)(ReceiveBuffer + sizeof(PMSG_ANS_QUESTEXP_PROGRESS_LIST)),
        int(pData->m_byQuestCount));
}

void ReceiveProgressQuestRequestReward(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NPC_QUESTEXP_INFO)ReceiveBuffer;
    g_QuestMng.SetQuestRequestReward(ReceiveBuffer);
    g_pMyQuestInfoWindow->SetSelQuestRequestReward();
}

void ReceiveProgressQuestListReady(const BYTE* ReceiveBuffer)
{
    g_QuestMng.SetQuestIndexByEtcList(nullptr, 0);
    SocketClient->ToGameServer()->SendActiveQuestListRequest();
    SocketClient->ToGameServer()->SendEventQuestStateListRequest();
}

void ReceiveGensJoining(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_REG_GENS_MEMBER)ReceiveBuffer;
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
        g_pNPCDialogue->ProcessGensJoiningReceive(pData->m_byResult, pData->m_byInfluence);
}

void ReceiveGensSecession(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_SECEDE_GENS_MEMBER)ReceiveBuffer;
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
        g_pNPCDialogue->ProcessGensSecessionReceive(pData->m_byResult);
}

void ReceivePlayerGensInfluence(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_MSG_SEND_GENS_INFO)ReceiveBuffer;
    Hero->m_byGensInfluence = pData->m_byInfluence;
    Hero->GensRanking = pData->m_nGensClass;
    g_pNewUIGensRanking->SetContribution(pData->m_nContributionPoint);
    g_pNewUIGensRanking->SetRanking(pData->m_nRanking);
    g_pNewUIGensRanking->SetNextContribution(pData->m_nNextContributionPoint);
}

void ReceiveOtherPlayerGensInfluenceViewport(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_SEND_GENS_MEMBER_VIEWPORT)ReceiveBuffer;
    int nOffset = sizeof(PMSG_SEND_GENS_MEMBER_VIEWPORT);
    int i;
    for (i = 0; i < Data->m_byCount; ++i)
    {
        auto Data2 = (LPPMSG_GENS_MEMBER_VIEWPORT_INFO)(ReceiveBuffer + nOffset);
        int nKey = ((int)(Data2->m_byNumberH & 0x7f) << 8) + Data2->m_byNumberL;
        int nIndex = FindCharacterIndex(nKey);
        CHARACTER* c = &CharactersClient[nIndex];

        c->m_byGensInfluence = Data2->m_byInfluence;
        c->GensRanking = Data2->m_nGensClass;
        c->GensContributionPoints = Data2->m_nContributionPoint;
        if (::IsStrifeMap(gMapManager.WorldActive))
        {
            vec3_t vTemp = { 0.f, 0.f, 0.f };
            if (Hero->m_byGensInfluence == c->m_byGensInfluence)
                CreateEffect(BITMAP_OUR_INFLUENCE_GROUND, c->Object.Position, vTemp, vTemp, 0, &c->Object);
            else
                CreateEffect(BITMAP_ENEMY_INFLUENCE_GROUND, c->Object.Position, vTemp, vTemp, 0, &c->Object);
        }
        nOffset += sizeof(PMSG_GENS_MEMBER_VIEWPORT_INFO);
    }
}

void ReceiveNPCDlgUIStart(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_NPC_CLICK)ReceiveBuffer;
    if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
    {
        g_QuestMng.SetNPC(pData->m_wNPCIndex);
        g_pNPCDialogue->SetContributePoint(pData->m_dwContributePoint);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_NPC_DIALOGUE);
    }
}

#ifdef PBG_ADD_GENSRANKING
void ReceiveReward(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_GENS_REWARD_CODE)ReceiveBuffer;

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_DIALOGUE))
        g_pNPCDialogue->ProcessGensRewardReceive(pData->m_byRewardResult);
}
#endif //PBG_ADD_GENSRANKING

void ReceiveUseStateItem(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_USE_STAT_FRUIT)ReceiveBuffer;

    BYTE result = Data->result;
    BYTE fruit = Data->btFruitType;
    WORD point = Data->btStatValue;

    wchar_t strText[MAX_GLOBAL_TEXT_STRING];

    switch (result) {
    case 0x00:
        if (fruit >= 0 && fruit <= 4)
        {
            int  index;

            switch (fruit)
            {
            case 0:
                CharacterAttribute->Energy += point;
                index = 168;
                break;

            case 1:
                CharacterAttribute->Vitality += point;
                index = 169;
                break;

            case 2:
                CharacterAttribute->Dexterity += point;
                index = 167;
                break;

            case 3:
                CharacterAttribute->Strength += point;
                index = 166;
                break;
            case 4:
                CharacterAttribute->Charisma += point;
                index = 1900;
                break;
            }

            CharacterAttribute->AddPoint += point;

            swprintf(strText, GlobalText[379], GlobalText[index], point, GlobalText[1412]);
            SEASON3B::CreateOkMessageBox(strText);
        }
        break;

    case 0x01:
        SEASON3B::CreateOkMessageBox(GlobalText[378]);
        break;

    case 0x02:
    {
        swprintf(strText, GlobalText[1904], GlobalText[1412]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    case 0x03:
        if (fruit >= 0 && fruit <= 4)
        {
            int  index;

            switch (fruit)
            {
            case 0:
                CharacterAttribute->Energy -= point;
                index = 168;
                break;

            case 1:
                CharacterAttribute->Vitality -= point;
                index = 169;
                break;

            case 2:
                CharacterAttribute->Dexterity -= point;
                index = 167;
                break;

            case 3:
                CharacterAttribute->Strength -= point;
                index = 166;
                break;
            case 4:
                CharacterAttribute->Charisma -= point;
                index = 1900;
                break;
            }

            CharacterAttribute->LevelUpPoint += point;
            CharacterAttribute->wMinusPoint += point;

            wchar_t strText[128];
            swprintf(strText, GlobalText[379], GlobalText[index], point, GlobalText[1903]);
            SEASON3B::CreateOkMessageBox(strText);
        }
        break;

    case 0x04:
        SEASON3B::CreateOkMessageBox(GlobalText[1906]);
        break;

    case 0x05:
    {
        wchar_t strText[128];
        swprintf(strText, GlobalText[1904], GlobalText[1903]);
        SEASON3B::CreateOkMessageBox(strText);
    }
    break;
    case 0x06:
        if (fruit >= 0 && fruit <= 4)
        {
            wchar_t Text[MAX_GLOBAL_TEXT_STRING];
            int  index;

            switch (fruit)
            {
            case 0:
                CharacterAttribute->Energy -= point;
                index = 168;
                break;

            case 1:
                CharacterAttribute->Vitality -= point;
                index = 169;
                break;

            case 2:
                CharacterAttribute->Dexterity -= point;
                index = 167;
                break;

            case 3:
                CharacterAttribute->Strength -= point;
                index = 166;
                break;
            case 4:
                CharacterAttribute->Charisma -= point;
                index = 1900;
                break;
            }

            CharacterAttribute->LevelUpPoint += point;

            swprintf(Text, GlobalText[379], GlobalText[index], point, GlobalText[1903]);
            SEASON3B::CreateOkMessageBox(Text);
        }
        break;
    case 0x07:
        SEASON3B::CreateOkMessageBox(GlobalText[1906]);
        break;
    case 0x08:
        wchar_t Text[MAX_GLOBAL_TEXT_STRING];
        swprintf(Text, GlobalText[1904], GlobalText[1903]);
        SEASON3B::CreateOkMessageBox(Text);
        break;
    case 0x10:
    {
        SEASON3B::CreateOkMessageBox(GlobalText[1909]);
    }
    break;

    case 0x21:
        SEASON3B::CreateOkMessageBox(GlobalText[1911]);
        break;

    case 0x25:
        SEASON3B::CreateOkMessageBox(GlobalText[1911]);
        break;

    case 0x26:
        SEASON3B::CreateOkMessageBox(GlobalText[1912]);
        break;
    }

    EnableUse = 0;
}

void ReceivePetCommand(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_PET_COMMAND)ReceiveBuffer;
    WORD Key = ((WORD)(Data->m_byKeyH) << 8) + Data->m_byKeyL;

    giPetManager::SetPetCommand(Hero, Key, Data->m_byCommand);
}

void ReceivePetAttack(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_PET_ATTACK)ReceiveBuffer;
    WORD Key = ((WORD)(Data->m_byKeyH) << 8) + Data->m_byKeyL;
    int  index = FindCharacterIndex(Key);
    CHARACTER* sc = &CharactersClient[index];

    Key = ((WORD)(Data->m_byTKeyH) << 8) + Data->m_byTKeyL;

    giPetManager::SetAttack(sc, Key, Data->m_bySkillType);
}

void ReceivePetInfo(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_PET_INFO)ReceiveBuffer;

    PET_INFO Petinfo;
    ZeroMemory(&Petinfo, sizeof(PET_INFO));

    Petinfo.m_dwPetType = Data->m_byPetType;
    Petinfo.m_wLevel = Data->m_byLevel;
    Petinfo.m_wLife = Data->m_byLife;
    Petinfo.m_dwExp1 = Data->m_iExp;

    giPetManager::SetPetInfo(Data->m_byInvType, Data->m_byPos, &Petinfo);
}

void ReceiveWTTimeLeft(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_MATCH_TIMEVIEW)ReceiveBuffer;
    g_wtMatchTimeLeft.m_Time = Data->m_Time;
    g_wtMatchTimeLeft.m_Type = Data->m_Type;
}

void ReceiveWTMatchResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_MATCH_RESULT)ReceiveBuffer;
    if (Data->m_Type >= 0 && Data->m_Type < 3)
    {
        g_wtMatchResult.m_Type = Data->m_Type;
        g_wtMatchResult.m_Score1 = Data->m_Score1;
        g_wtMatchResult.m_Score2 = Data->m_Score2;
        CMultiLanguage::ConvertFromUtf8(g_wtMatchResult.m_MatchTeamName1, Data->m_MatchTeamName1, MAX_ID_SIZE);
        CMultiLanguage::ConvertFromUtf8(g_wtMatchResult.m_MatchTeamName2, Data->m_MatchTeamName2, MAX_ID_SIZE);
    }
}

void ReceiveWTBattleSoccerGoalIn(const BYTE* ReceiveBuffer)
{
    /*	LPPMSG_SOCCER_GOALIN Data = (LPPMSG_SOCCER_GOALIN)ReceiveBuffer;

      vec3_t Position, Angle, Light;
      Position[0]  = (float)(Data->m_x+0.5f)*TERRAIN_SCALE;
      Position[1]  = (float)(Data->m_y+0.5f)*TERRAIN_SCALE;
      Position[2] = RequestTerrainHeight(Position[0],Position[1]);
      ZeroMemory( Angle, sizeof ( Angle));
      Light[0] = Light[1] = Light[2] = 1.0f;
        CreateEffect(BITMAP_FIRECRACKERRISE,Position,Angle,Light);*/
}

void ReceiveChangeMapServerInfo(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_MAP_CHANGESERVER_INFO)ReceiveBuffer;

    if (0 == Data->m_vSvrInfo.m_wMapSvrPort)
    {
        LoadingWorld = 0;
        return;
    }

    g_PortalMgr.Reset();

    g_csMapServer.ConnectChangeMapServer(Data->m_vSvrInfo);
}

void ReceiveChangeMapServerResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT)ReceiveBuffer;

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0xB1 [ReceiveChangeMapServerResult]");
}

void ReceiveBCStatus(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_CASTLESIEGESTATE)ReceiveBuffer;

    switch (Data->btResult)
    {
    case 0x00:
        g_pSystemLogBox->AddText(GlobalText[1500], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x01:
    case 0x02:
        g_pNewUISystem->Show(SEASON3B::INTERFACE_GUARDSMAN);
        g_pGuardWindow->SetData(Data);
        break;
    case 0x03:
        g_pSystemLogBox->AddText(GlobalText[1501], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x04:
        g_pSystemLogBox->AddText(GlobalText[1502], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveBCReg(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_REGCASTLESIEGE)ReceiveBuffer;

    switch (Data->btResult)
    {
    case 0x00:
        g_pSystemLogBox->AddText(GlobalText[1503], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x01:
        g_GuardsMan.SetRegStatus(1);
        g_pSystemLogBox->AddText(GlobalText[1504], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x02:
        g_pSystemLogBox->AddText(GlobalText[1505], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x03:
        g_pSystemLogBox->AddText(GlobalText[1506], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x04:
        g_pSystemLogBox->AddText(GlobalText[1507], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x05:
        g_pSystemLogBox->AddText(GlobalText[1508], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x06:
        g_pSystemLogBox->AddText(GlobalText[1509], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x07:
        g_pSystemLogBox->AddText(GlobalText[1510], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x08:
        g_pSystemLogBox->AddText(GlobalText[1511], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    default:
        assert(!"ReceiveBCReg(0xB2, 0x01)");
        break;
    }
}

void ReceiveBCGiveUp(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_GIVEUPCASTLESIEGE)ReceiveBuffer;

    switch (Data->btResult)
    {
    case 0x00:
        g_pSystemLogBox->AddText(GlobalText[1512], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x01:
        SocketClient->ToGameServer()->SendCastleSiegeRegistrationStateRequest();
        SocketClient->ToGameServer()->SendCastleSiegeRegisteredGuildsListRequest();
        g_GuardsMan.SetRegStatus(0);
        g_pSystemLogBox->AddText(GlobalText[1513], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x02:
        g_pSystemLogBox->AddText(GlobalText[1514], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x03:
        g_pSystemLogBox->AddText(GlobalText[1515], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    default:
        assert(!"ReceiveBCGiveUp(0xB2,0x02)");
        break;
    }
}

void ReceiveBCRegInfo(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_GUILDREGINFO)ReceiveBuffer;

    switch (Data->btResult)
    {
    case 0x00:
        g_GuardsMan.SetRegStatus(0);
        break;
    case 0x01:
    {
        g_GuardsMan.SetRegStatus(!Data->btIsGiveUp);
        DWORD dwMarkCount;
        auto* pMarkCount = (BYTE*)&dwMarkCount;
        *pMarkCount++ = Data->btGuildMark4;
        *pMarkCount++ = Data->btGuildMark3;
        *pMarkCount++ = Data->btGuildMark2;
        *pMarkCount++ = Data->btGuildMark1;
        g_GuardsMan.SetMarkCount(dwMarkCount);
    }
    break;
    case 0x02:
        g_GuardsMan.SetRegStatus(0);
        break;
    }
}

void ReceiveBCRegMark(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_REGGUILDMARK)ReceiveBuffer;

    switch (Data->btResult)
    {
    case 0x00:
        g_pSystemLogBox->AddText(GlobalText[1516], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x01:
    {
        DWORD dwMarkCount;
        auto* pMarkCount = (BYTE*)&dwMarkCount;
        *pMarkCount++ = Data->btGuildMark4;
        *pMarkCount++ = Data->btGuildMark3;
        *pMarkCount++ = Data->btGuildMark2;
        *pMarkCount++ = Data->btGuildMark1;
        g_GuardsMan.SetMarkCount(dwMarkCount);
    }
    break;
    case 0x02:
        g_pSystemLogBox->AddText(GlobalText[1517], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 0x03:
        g_pSystemLogBox->AddText(GlobalText[1518], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveBCNPCBuy(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_NPCBUY)ReceiveBuffer;
    switch (Data->btResult)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[1519], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 1:
        g_SenatusInfo.BuyNewNPC(Data->iNpcNumber, Data->iNpcIndex);
        break;
    case 2:
        g_pSystemLogBox->AddText(GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 3:
        g_pSystemLogBox->AddText(GlobalText[1520], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 4:
        g_pSystemLogBox->AddText(GlobalText[1616], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveBCNPCRepair(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_NPCREPAIR)ReceiveBuffer;
    switch (Data->btResult)
    {
    case 0:
    {
        g_pSystemLogBox->AddText(GlobalText[1519], SEASON3B::TYPE_SYSTEM_MESSAGE);
    }

    break;
    case 1:
    {
        LPPMSG_NPCDBLIST pNPCInfo = nullptr;
        pNPCInfo = g_SenatusInfo.GetNPCInfo(Data->iNpcNumber, Data->iNpcIndex);
        pNPCInfo->iNpcHp = Data->iNpcHP;
        pNPCInfo->iNpcMaxHp = Data->iNpcMaxHP;
    }
    break;
    case 2:
        g_pSystemLogBox->AddText(GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 3:
        g_pSystemLogBox->AddText(GlobalText[1520], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveBCNPCUpgrade(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_NPCUPGRADE)ReceiveBuffer;
    switch (Data->btResult)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[1519], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 1:
    {
        LPPMSG_NPCDBLIST pNPCInfo = nullptr;
        pNPCInfo = g_SenatusInfo.GetNPCInfo(Data->iNpcNumber, Data->iNpcIndex);
        if (Data->iNpcUpType == 1)
            pNPCInfo->iNpcDfLevel = Data->iNpcUpValue;
        else if (Data->iNpcUpType == 2)
            pNPCInfo->iNpcRgLevel = Data->iNpcUpValue;
        else if (Data->iNpcUpType == 3)
            pNPCInfo->iNpcHp = pNPCInfo->iNpcMaxHp = Data->iNpcUpValue;
    }
    break;
    case 2:
        g_pSystemLogBox->AddText(GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 3:
        g_pSystemLogBox->AddText(GlobalText[1520], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 4:
        g_pSystemLogBox->AddText(GlobalText[1521], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 5:
        g_pSystemLogBox->AddText(GlobalText[1522], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 6:
        g_pSystemLogBox->AddText(GlobalText[1523], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 7:
        g_pSystemLogBox->AddText(GlobalText[1524], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveBCGetTaxInfo(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_TAXMONEYINFO)ReceiveBuffer;
    switch (Data->btResult)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[1525], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 1:
        g_SenatusInfo.SetTaxInfo(Data);
        break;
    case 2:
        g_pSystemLogBox->AddText(GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveBCChangeTaxRate(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_TAXRATECHANGE)ReceiveBuffer;
    switch (Data->btResult)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[1526], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 1:
        if (Data->btTaxType == 3)
        {
            g_pUIGateKeeper->SetEntranceFee((Data->btTaxRate1 << 24) | (Data->btTaxRate2 << 16) | (Data->btTaxRate3 << 8) | (Data->btTaxRate4));
        }
        else
        {
            g_SenatusInfo.ChangeTaxInfo(Data);
        }
        break;
    case 2:
        g_pSystemLogBox->AddText(GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveBCWithdraw(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_MONEYDRAWOUT)ReceiveBuffer;
    switch (Data->btResult)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[1527], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 1:
        g_SenatusInfo.ChangeCastleMoney(Data);
        break;
    case 2:
        g_pSystemLogBox->AddText(GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveTaxInfo(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_MAPSVRTAXINFO)ReceiveBuffer;

    if (Data->btTaxType == 1)
    {
        g_nChaosTaxRate = Data->btTaxRate;
    }
    else if (Data->btTaxType == 2)
    {
        g_pNPCShop->SetTaxRate(Data->btTaxRate);
        g_nTaxRate = Data->btTaxRate;
    }
    else
    {
        assert(!"Packet Recv(0xB2(0x1A))");
    }
}

void ReceiveHuntZoneEnter(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_CSHUNTZONEENTER)ReceiveBuffer;

    switch (pData->m_byResult)
    {
    case 0:
    {
        g_pUIPopup->CancelPopup();
        g_pUIPopup->SetPopup(GlobalText[860], 1, 50, POPUP_OK, nullptr);
    }
    break;

    case 1:
        g_pUIGateKeeper->SetPublic(pData->m_byHuntZoneEnter);
        break;

    case 2:
    {
        g_pUIPopup->CancelPopup();
        g_pUIPopup->SetPopup(GlobalText[1386], 1, 50, POPUP_OK, nullptr);
    }
    break;
    }
}

void ReceiveBCNPCList(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_NPCDBLIST)ReceiveBuffer;
    int Offset = sizeof(PMSG_ANS_NPCDBLIST);

    switch (Data->btResult)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[860], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 1:
    {
        for (int i = 0; i < Data->iCount; ++i)
        {
            auto pNpcInfo = (LPPMSG_NPCDBLIST)(ReceiveBuffer + Offset);
            g_SenatusInfo.SetNPCInfo(pNpcInfo);
            Offset += sizeof(PMSG_NPCDBLIST);
        }
    }
    break;
    case 2:
        g_pSystemLogBox->AddText(GlobalText[1386], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveBCDeclareGuildList(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_CSREGGUILDLIST)ReceiveBuffer;
    int Offset = sizeof(PMSG_ANS_CSREGGUILDLIST);

    switch (Data->btResult)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[860], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 1:
    {
        g_pGuardWindow->ClearDeclareGuildList();
        for (int i = 0; i < Data->iCount; ++i)
        {
            auto pData2 = (LPPMSG_CSREGGUILDLIST)(ReceiveBuffer + Offset);

            // WTF - why not just use an int?
            DWORD dwMarkCount;
            auto* pMarkCount = (BYTE*)&dwMarkCount;
            *pMarkCount++ = pData2->btRegMarks4;
            *pMarkCount++ = pData2->btRegMarks3;
            *pMarkCount++ = pData2->btRegMarks2;
            *pMarkCount++ = pData2->btRegMarks1;

            wchar_t guildName[MAX_GUILDNAME + 1]{};
            CMultiLanguage::ConvertFromUtf8(guildName, pData2->szGuildName, MAX_GUILDNAME);
            g_pGuardWindow->AddDeclareGuildList(guildName, dwMarkCount, pData2->btIsGiveUp, pData2->btSeqNum);

            Offset += sizeof(PMSG_CSREGGUILDLIST);
        }
        g_pGuardWindow->SortDeclareGuildList();
    }
    break;
    }
}

void ReceiveBCGuildList(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_CSATTKGUILDLIST)ReceiveBuffer;
    int Offset = sizeof(PMSG_ANS_CSATTKGUILDLIST);

    switch (Data->btResult)
    {
    case 0:
        g_pSystemLogBox->AddText(GlobalText[860], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 1:
    {
        g_pGuardWindow->ClearGuildList();
        for (int i = 0; i < Data->iCount; ++i)
        {
            auto pData2 = (LPPMSG_CSATTKGUILDLIST)(ReceiveBuffer + Offset);
            wchar_t guildName[MAX_GUILDNAME + 1]{};
            CMultiLanguage::ConvertFromUtf8(guildName, pData2->szGuildName, MAX_GUILDNAME);

            g_pGuardWindow->AddGuildList(guildName, pData2->btCsJoinSide, pData2->btGuildInvolved, pData2->iGuildScore);

            Offset += sizeof(PMSG_CSATTKGUILDLIST);
        }
    }
    break;
    case 2:
        g_pSystemLogBox->AddText(GlobalText[1609], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    case 3:
        g_pSystemLogBox->AddText(GlobalText[1609], SEASON3B::TYPE_SYSTEM_MESSAGE);
        break;
    }
}

void ReceiveGateState(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_GATE_STATE)ReceiveBuffer;
    int Key = ((int)(Data->m_byKeyH) << 8) + Data->m_byKeyL;

    switch (Data->m_byResult)
    {
    case 0:
        break;

    case 1:
        npcGateSwitch::DoInterfaceOpen(Key);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_GATESWITCH);
        break;

    case 2:
        break;

    case 3:
        break;

    case 4:
        break;
    }
}

void ReceiveGateOperator(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_GATE_OPERATOR)ReceiveBuffer;
    int Key = ((int)(Data->m_byKeyH) << 8) + Data->m_byKeyL;

    npcGateSwitch::ProcessState(Key, Data->m_byOperator, Data->m_byResult);
}

void ReceiveGateCurrentState(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_GATE_CURRENT_STATE)ReceiveBuffer;
    int Key = ((int)(Data->m_byKeyH) << 8) + Data->m_byKeyL;
    int Index = FindCharacterIndex(Key);

    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;

    if (Data->m_byOperator)
    {
        if (Attacking != -1 && SelectedCharacter == Index)
        {
            Attacking = -1;
        }
        npcGateSwitch::GateOpen(c, o);
    }
    else
    {
        npcGateSwitch::GateClose(c, o);
    }
}

void ReceiveCrownSwitchState(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_SWITCH_PROC)ReceiveBuffer;

    int iKey = ((int)(pData->m_byIndexH) << 8) + pData->m_byIndexL;
    int iIndex = FindCharacterIndex(iKey);

    CHARACTER* CrownSwitch = &CharactersClient[iIndex];

    if (CrownSwitch == nullptr)
    {
        return;
    }
    if (CrownSwitch->ID == nullptr)
    {
        return;
    }

    switch (pData->m_byState)
    {
    case 0:
    {
        int iSwitchIndex = ((int)(pData->m_byIndexH) << 8) + pData->m_byIndexL;
        if (iSwitchIndex == FIRST_CROWN_SWITCH_NUMBER)
        {
            Switch_Info[0].Reset();
        }
        else
        {
            Switch_Info[1].Reset();
        }

        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownSwitchPopLayout));
    }
    break;

    case 1:
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownSwitchPushLayout));
        break;

    case 2:
    {
        int iKey = ((int)(pData->m_byKeyH) << 8) + pData->m_byKeyL;
        int iIndex = FindCharacterIndex(iKey);
        CHARACTER* pCha = &CharactersClient[iIndex];
        wchar_t strText[256];

        SEASON3B::CProgressMsgBox* pMsgBox = nullptr;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownSwitchOtherPushLayout), &pMsgBox);
        if (pMsgBox)
        {
            if (pCha != nullptr && pCha->ID != nullptr)
            {
                swprintf(strText, GlobalText[1486], pCha->ID);
            }
            else
            {
                swprintf(strText, GlobalText[1487]);
            }
            pMsgBox->AddMsg(strText);

            swprintf(strText, GlobalText[1488], CrownSwitch->ID);
            pMsgBox->AddMsg(strText);
        }
    }
    break;
    }
}

int DenyCrownRegistPopupClose(POPUP_RESULT Result)
{
    if (Result & POPUP_RESULT_ESC)
        return 0;
    return 1;
}

void ReceiveCrownRegist(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_CROWN_STATE)ReceiveBuffer;

    g_MessageBox->PopAllMessageBoxes();

    switch (pData->m_byCrownState)
    {
    case 0:
    {
        SEASON3B::CProgressMsgBox* pMsgBox = nullptr;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterStartLayout), &pMsgBox);
        if (pMsgBox)
        {
            wchar_t strText[256];
            int iTime = (pData->m_dwCrownAccessTime / 1000);
            if (iTime >= 59)
                iTime = 59;
            swprintf(strText, GlobalText[1980], GlobalText[1489], iTime);
            pMsgBox->AddMsg(strText);
            pMsgBox->SetElapseTime(60000 - pData->m_dwCrownAccessTime);
        }
    }
    break;

    case 1:
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterSuccessLayout));
        break;

    case 2:
    {
        SEASON3B::CProgressMsgBox* pMsgBox = nullptr;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterFailLayout), &pMsgBox);
        if (pMsgBox)
        {
            wchar_t strText[256];
            int iTime = (pData->m_dwCrownAccessTime / 1000);
            if (iTime >= 59)
                iTime = 59;
            swprintf(strText, GlobalText[1980], GlobalText[1491], iTime);
            pMsgBox->AddMsg(strText);
        }
    }
    break;

    case 3:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterOtherLayout));
    }
    break;
    case 4:
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterOtherCampLayout));
        break;
    }
}

void ReceiveCrownState(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_CROWN_STATE)ReceiveBuffer;

    g_pUIPopup->CancelPopup();
    switch (pData->m_byCrownState)
    {
    case 0:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownDefenseRemoveLayout));

        int Index = FindCharacterIndexByMonsterIndex(216);

        OBJECT* o = &CharactersClient[Index].Object;

        g_CharacterRegisterBuff(o, eBuff_CastleCrown);
    }
    break;

    case 1:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCrownDefenseCreateLayout));

        int Index = FindCharacterIndexByMonsterIndex(216);

        OBJECT* o = &CharactersClient[Index].Object;

        g_CharacterClearBuff(o);
    }
    break;

    case 2:
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSealRegisterSuccessLayout));

        break;
    }
}

void ReceiveBattleCastleRegiment(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_CROWN_STATE)ReceiveBuffer;

    switch (pData->m_byCrownState)
    {
    case 0:
        battleCastle::SettingBattleFormation(Hero, static_cast<eBuffState>(0));
        return;
    case 1:
        battleCastle::SettingBattleFormation(Hero, eBuff_CastleRegimentDefense);
        break;
    case 2:
        battleCastle::SettingBattleFormation(Hero, eBuff_CastleRegimentAttack1);
        break;
    case 3:
        battleCastle::SettingBattleFormation(Hero, eBuff_CastleRegimentAttack2);
        break;
    case 4:
        battleCastle::SettingBattleFormation(Hero, eBuff_CastleRegimentAttack3);
        break;
    }
}

void ReceiveBattleCasleSwitchInfo(const BYTE* ReceiveBuffer)
{
    auto pData = (LPRECEIVE_CROWN_SWITCH_INFO)ReceiveBuffer;
    Check_Switch(pData);
}

bool Check_Switch(PRECEIVE_CROWN_SWITCH_INFO* Data)
{
    if (Switch_Info == nullptr)
    {
        Switch_Info = new CROWN_SWITCH_INFO[2];

        WORD Key = ((WORD)(Data->m_byIndex1) << 8) + Data->m_byIndex2;

        if (Key == FIRST_CROWN_SWITCH_NUMBER)
        {
            Switch_Info[0].m_bySwitchState = Data->m_bySwitchState;
            Switch_Info[0].m_JoinSide = Data->m_JoinSide;
        }
        else
        {
            Switch_Info[1].m_bySwitchState = Data->m_bySwitchState;
            Switch_Info[1].m_JoinSide = Data->m_JoinSide;
        }
    }
    else
    {
        WORD Key = ((WORD)(Data->m_byIndex1) << 8) + Data->m_byIndex2;

        if (Key == FIRST_CROWN_SWITCH_NUMBER)
        {
            Switch_Info[0].m_bySwitchState = Data->m_bySwitchState;
            Switch_Info[0].m_JoinSide = Data->m_JoinSide;
            CMultiLanguage::ConvertFromUtf8(Switch_Info[0].m_szGuildName, Data->m_szGuildName, MAX_GUILDNAME);
            CMultiLanguage::ConvertFromUtf8(Switch_Info[0].m_szUserName, Data->m_szUserName, MAX_ID_SIZE);
        }
        else
        {
            Switch_Info[1].m_bySwitchState = Data->m_bySwitchState;
            Switch_Info[1].m_JoinSide = Data->m_JoinSide;
            CMultiLanguage::ConvertFromUtf8(Switch_Info[1].m_szGuildName, Data->m_szGuildName, MAX_GUILDNAME);
            CMultiLanguage::ConvertFromUtf8(Switch_Info[1].m_szUserName, Data->m_szUserName, MAX_ID_SIZE);
        }
    }
    return true;
}

bool Delete_Switch()
{
    if (Switch_Info != nullptr)
    {
        delete[] Switch_Info;
        Switch_Info = nullptr;
    }
    return true;
}

void ReceiveBattleCastleStart(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_CROWN_STATE)ReceiveBuffer;

    auto bStartBattleCastle = (bool)(pData->m_byCrownState);

    battleCastle::SetBattleCastleStart(bStartBattleCastle);

    if (bStartBattleCastle)
    {
        g_pSiegeWarfare->InitSkillUI();
    }
    else
    {
        g_pSiegeWarfare->ReleaseSkillUI();
    }
}

void ReceiveBattleCastleProcess(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_BC_PROCESS)ReceiveBuffer;

    wchar_t guildName[MAX_GUILDNAME + 1];
    CMultiLanguage::ConvertFromUtf8(guildName, pData->m_szGuildName, MAX_GUILDNAME);

    switch (pData->m_byBasttleCastleState)
    {
    case 0:
    {
        wchar_t Text[100];
        swprintf(Text, GlobalText[1496], guildName);
        CreateNotice(Text, 1);
    }
    break;

    case 1:
    {
        ChangeBattleFormation(guildName, true);
        wchar_t Text2[100];
        swprintf(Text2, GlobalText[1497], guildName);
        CreateNotice(Text2, 1);
    }
    break;
    }
}

void ReceiveKillCount(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
    if (pData->Value == 0)
    {
        if (Hero->BackupCurrentSkill != 255)
        {
            Hero->CurrentSkill = Hero->BackupCurrentSkill;
        }
        Hero->BackupCurrentSkill = 255;
        Hero->GuildMasterKillCount = 0;
    }
    else
    {
        Hero->GuildMasterKillCount = pData->Value;
    }
}

void ReceiveBuildTime(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_MONSTER_BUILD_TIME)ReceiveBuffer;
    int Key = ((int)(pData->m_byKeyH) << 8) + pData->m_byKeyL;
    int Index = FindCharacterIndex(Key);

    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;

    o->m_byBuildTime = pData->m_byBuildTime;
}

void    ReceiveCastleGuildMark(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_CASTLE_FLAG)ReceiveBuffer;

    BYTE GuildMark[64];
    for (int i = 0; i < 64; i++)
    {
        if (i % 2 == 0)
            GuildMark[i] = (pData->m_byGuildMark[i / 2] >> 4) & 0x0f;
        else
            GuildMark[i] = pData->m_byGuildMark[i / 2] & 0x0f;
    }
    CreateCastleMark(BITMAP_INTERFACE_MAP, GuildMark);
}

void ReceiveCastleHuntZoneInfo(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_CASTLE_HUNTZONE_INFO)ReceiveBuffer;

    if (pData->m_byResult == 0)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGatemanFailMsgBoxLayout));
    }
    else
    {
        g_pUIGateKeeper->SetInfo(pData->m_byResult, (bool)pData->m_byEnable, pData->m_iCurrPrice, pData->m_iUnitPrice, pData->m_iMaxPrice);
        g_pNewUISystem->Show(SEASON3B::INTERFACE_GATEKEEPER);
    }
}

void ReceiveCastleHuntZoneResult(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEVIE_CASTLE_HUNTZONE_RESULT)ReceiveBuffer;

    if (pData->m_byResult == 0)
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGatemanFailMsgBoxLayout));
    }
}

void ReceiveCatapultState(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_CATAPULT_STATE)ReceiveBuffer;

    if (pData->m_byResult == 1)
    {
        int Key = ((int)(pData->m_byKeyH) << 8) + pData->m_byKeyL;

        g_pNewUISystem->Show(SEASON3B::INTERFACE_CATAPULT);
        g_pCatapultWindow->Init(Key, pData->m_byWeaponType);
    }
    else if (pData->m_byResult == 0)
    {
        g_pSystemLogBox->AddText(L"ReceiveCatapultState", SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
}

void ReceiveCatapultFire(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_WEAPON_FIRE)ReceiveBuffer;

    if (pData->m_byResult)
    {
        int Key = ((int)(pData->m_byKeyH) << 8) + pData->m_byKeyL;

        g_pCatapultWindow->DoFire(Key, pData->m_byResult, pData->m_byWeaponType, pData->m_byTargetX, pData->m_byTargetY);
    }
    else if (pData->m_byResult == 0)
    {
        g_pSystemLogBox->AddText(L"ReceiveCatapultFire", SEASON3B::TYPE_SYSTEM_MESSAGE);
    }
}

void    ReceiveCatapultFireToMe(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_BOMBING_ALERT)ReceiveBuffer;

    g_pCatapultWindow->DoFireFixStartPosition(pData->m_byWeaponType, pData->m_byTargetX, pData->m_byTargetY);
}

void ReceivePreviewPort(std::span<const BYTE> ReceiveBuffer)
{
    auto pData = safe_cast<PWHEADER_DEFAULT_WORD>(ReceiveBuffer);
    if (pData == nullptr)
    {
        assert(false);
        return;
    }

    int Offset = sizeof(PWHEADER_DEFAULT_WORD);

    for (int i = 0; i < pData->Value; i++)
    {
        auto pData2 = safe_cast<PRECEIVE_PREVIEW_PORT_EXTENDED>(ReceiveBuffer.subspan(Offset));
        if (pData2 == nullptr)
        {
            assert(false);
            return;
        }

        WORD Key = ((WORD)(pData2->m_byKeyH) << 8) + pData2->m_byKeyL;
        Key &= 0x7FFF;

        switch (pData2->m_byObjType)
        {
        case 1:
        {
            CHARACTER* c = CreateCharacter(Key, MODEL_PLAYER, pData2->m_byPosX, pData2->m_byPosY, 0);
            OBJECT* o = &c->Object;

            c->Class = gCharacterManager.ChangeServerClassTypeToClientClassType((SERVER_CLASS_TYPE)pData2->m_byTypeH);
            c->SkinIndex = gCharacterManager.GetSkinModelIndex(c->Class);
            c->Skin = 0;
            c->PK = 0;
            o->Kind = KIND_TMP;

            for (int j = 0; j < pData2->s_BuffCount; ++j)
            {
                RegisterBuff(static_cast<eBuffState>(pData2->s_BuffEffectState[j]), o);
            }

            c->PositionX = pData2->m_byPosX;
            c->PositionY = pData2->m_byPosY;
            c->TargetX = pData2->m_byPosX;
            c->TargetY = pData2->m_byPosY;
            c->Object.Angle[2] = rand() % 360;

            c->m_iDeleteTime = 150;

            ChangeCharacterExt(FindCharacterIndex(Key), pData2->m_byEquipment);

            wcscpy(c->ID, L"   ");
            c->ID[MAX_ID_SIZE] = NULL;
        }
        break;

        case 2:
        case 3:    //   NPC
        {
            auto Type = (EMonsterType)(((WORD)(pData2->m_byTypeH) << 8) + pData2->m_byTypeL);
            CHARACTER* c = CreateMonster(Type, pData2->m_byPosX, pData2->m_byPosY, Key);
            if (c == nullptr) break;
            OBJECT* o = &c->Object;

            for (int j = 0; j < pData2->s_BuffCount; ++j)
            {
                RegisterBuff(static_cast<eBuffState>(pData2->s_BuffEffectState[j]), o);
            }

            c->PositionX = pData2->m_byPosX;
            c->PositionY = pData2->m_byPosY;
            c->TargetX = pData2->m_byPosX;
            c->TargetY = pData2->m_byPosY;
            c->Object.Angle[2] = rand() % 360;

            c->m_iDeleteTime = 150;

            if (gMapManager.InBattleCastle() && c->MonsterIndex == MONSTER_CASTLE_GATE1)
            {
                //  State
                if (g_isCharacterBuff((&c->Object), eBuff_CastleGateIsOpen))
                {
                    SetAction(&c->Object, 1);
                    battleCastle::SetCastleGate_Attribute(pData2->m_byPosX, pData2->m_byPosY, 0);
                }
                else if (g_isNotCharacterBuff((&c->Object)))
                {
                    SetAction(&c->Object, 0);
                    battleCastle::SetCastleGate_Attribute(pData2->m_byPosX, pData2->m_byPosY, 1);
                }
            }
        }
        break;
        }

        Offset += (sizeof(PRECEIVE_PREVIEW_PORT_EXTENDED) - (sizeof(BYTE) * (MAX_BUFF_SLOT_INDEX - pData2->s_BuffCount)));
    }
}

void ReceiveMapInfoResult(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_MAP_INFO_RESULT)ReceiveBuffer;
}

void ReceiveGuildCommand(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_GUILD_COMMAND)ReceiveBuffer;
    GuildCommander            GCmd = { pData->m_byTeam, pData->m_byX, pData->m_byY, pData->m_byCmd };

    if (g_pSiegeWarfare)
    {
        g_pSiegeWarfare->SetMapInfo(GCmd);
    }
}

void ReceiveGuildMemberLocation(const BYTE* ReceiveBuffer)
{
    if (g_pSiegeWarfare->GetCurSiegeWarType() != TYPE_GUILD_COMMANDER)
        return;

    g_pSiegeWarfare->ClearGuildMemberLocation();

    auto pData = (LPPWHEADER_DEFAULT_WORD2)ReceiveBuffer;
    int Offset = sizeof(PWHEADER_DEFAULT_WORD2);

    for (int i = 0; i < pData->Value; i++)
    {
        auto pData2 = (LPPRECEIVE_MEMBER_LOCATION)(ReceiveBuffer + Offset);

        g_pSiegeWarfare->SetGuildMemberLocation(0, pData2->m_byX, pData2->m_byY);

        Offset += sizeof(PRECEIVE_MEMBER_LOCATION);
    }
}

void ReceiveGuildNpcLocation(const BYTE* ReceiveBuffer)
{
    if (g_pSiegeWarfare->GetCurSiegeWarType() != TYPE_GUILD_COMMANDER)
        return;

    auto pData = (LPPWHEADER_DEFAULT_WORD)ReceiveBuffer;
    int Offset = sizeof(PWHEADER_DEFAULT_WORD);

    for (int i = 0; i < pData->Value; i++)
    {
        auto pData2 = (LPPRECEIVE_NPC_LOCATION)(ReceiveBuffer + Offset);
        g_pSiegeWarfare->SetGuildMemberLocation(pData2->m_byType + 1, pData2->m_byX, pData2->m_byY);

        Offset += sizeof(PRECEIVE_NPC_LOCATION);
    }
}

void ReceiveMatchTimer(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPRECEIVE_MATCH_TIMER)ReceiveBuffer;

    g_pSiegeWarfare->SetTime(pData->m_byHour, pData->m_byMinute);
}

void ReceiveCrywolfInfo(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_CRYWOLF_INFO)ReceiveBuffer;

    M34CryWolf1st::CheckCryWolf1stMVP(pData->btOccupationState, pData->btCrywolfState);
    g_Direction.m_CMVP.GetCryWolfState(pData->btCrywolfState);
}

void ReceiveCrywolStateAltarfInfo(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_CRYWOLF_STATE_ALTAR_INFO)ReceiveBuffer;

    M34CryWolf1st::CheckCryWolf1stMVPAltarfInfo(pData->iCrywolfStatueHP,
        pData->btAltarState1,
        pData->btAltarState2,
        pData->btAltarState3,
        pData->btAltarState4,
        pData->btAltarState5);
}

void ReceiveCrywolfAltarContract(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPPMSG_ANS_CRYWOLF_ALTAR_CONTRACT)ReceiveBuffer;

    int Key = ((int)(pData->btObjIndexH) << 8) + pData->btObjIndexL;

    if (pData->bResult == 0)
    {
        g_CharacterUnRegisterBuff((&Hero->Object), eBuff_CrywolfHeroContracted);

        int level = CharacterAttribute->Level;
        if (level < 260)
        {
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Dont_Set_Temple1));
            //			M34CryWolf1st::Set_Message_Box(54,0,0);
            //			M34CryWolf1st::Set_Message_Box(55,1,0);
        }
        else
        {
            //			M34CryWolf1st::Set_Message_Box(58,0,0);
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Wat_Set_Temple1));
        }
    }
    else if (pData->bResult == 1)
    {
        //		M34CryWolf1st::Set_Message_Box(3,0,0);
        //		M34CryWolf1st::Set_Message_Box(4,1,0);
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CCry_Wolf_Set_Temple));

        M34CryWolf1st::Check_AltarState(Key - 316, pData->btAltarState);

        g_CharacterRegisterBuff((&Hero->Object), eBuff_CrywolfHeroContracted);

        SetAction(&Hero->Object, PLAYER_HEALING_FEMALE1);
        SendRequestAction(Hero->Object, AT_HEALING1);
    }
}

void ReceiveCrywolfLifeTime(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPPMSG_ANS_CRYWOLF_LEFTTIME)ReceiveBuffer;

    M34CryWolf1st::SetTime(pData->btHour, pData->btMinute);
    g_pCryWolfInterface->SetTime((int)(pData->btHour), (int)(pData->btMinute));
}

void ReceiveCrywolfTankerHit(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NOTIFY_REGION_MONSTER_ATTACK)ReceiveBuffer;

    M34CryWolf1st::DoTankerFireFixStartPosition(pData->btSourceX, pData->btSourceY, pData->btPointX, pData->btPointY);
}

void ReceiveCrywolfBenefitPlusChaosRate(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_CRYWOLF_BENEFIT_PLUS_CHAOSRATE)ReceiveBuffer;

    g_MixRecipeMgr.SetPlusChaosRate(pData->btPlusChaosRate);
}

void ReceiveCrywolfBossMonsterInfo(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_CRYWOLF_BOSSMONSTER_INFO)ReceiveBuffer;

    M34CryWolf1st::Set_BossMonster(pData->btBossHP, pData->btMonster2);
}

void ReceiveCrywolfPersonalRank(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_CRYWOLF_PERSONAL_RANK)ReceiveBuffer;

    M34CryWolf1st::Set_MyRank(pData->btRank, pData->iGettingExp);
}

void ReceiveCrywolfHeroList(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_CRYWOLF_HERO_LIST_INFO_COUNT)ReceiveBuffer;
    int Offset = sizeof(PMSG_ANS_CRYWOLF_HERO_LIST_INFO_COUNT);
    for (int i = 0; i < pData->btCount; i++)
    {
        auto pData2 = (LPPMSG_ANS_CRYWOLF_HERO_LIST_INFO)(ReceiveBuffer + Offset);
        Offset += sizeof(PMSG_ANS_CRYWOLF_HERO_LIST_INFO);

        wchar_t playerName[MAX_ID_SIZE + 1];
        CMultiLanguage::ConvertFromUtf8(playerName, pData2->szHeroName, MAX_ID_SIZE);
        auto heroClass = gCharacterManager.ChangeServerClassTypeToClientClassType(pData2->btHeroClass);
        M34CryWolf1st::Set_WorldRank(pData2->iRank, heroClass, pData2->iHeroScore, playerName);
    }
}

void ReceiveKanturu3rdStateInfo(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_KANTURU_STATE_INFO)ReceiveBuffer;

    g_pKanturu2ndEnterNpc->ReceiveKanturu3rdInfo(pData->btState, pData->btDetailState, pData->btEnter, pData->btUserCount, pData->iRemainTime);
}

void ReceiveKanturu3rdEnterBossMap(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_ENTER_KANTURU_BOSS_MAP)ReceiveBuffer;
    g_pKanturu2ndEnterNpc->ReceiveKanturu3rdEnter(pData->btResult);
}

void ReceiveKanturu3rdCurrentState(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_KANTURU_CURRENT_STATE)ReceiveBuffer;
    M39Kanturu3rd::Kanturu3rdState(pData->btCurrentState, pData->btCurrentDetailState);
    M39Kanturu3rd::CheckSuccessBattle(pData->btCurrentState, pData->btCurrentDetailState);
}

void ReceiveKanturu3rdState(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_KANTURU_STATE_CHANGE)ReceiveBuffer;

    if (M39Kanturu3rd::IsInKanturu3rd())
    {
        if ((pData->btState == KANTURU_STATE_MAYA_BATTLE
            && (pData->btDetailState == KANTURU_MAYA_DIRECTION_MONSTER1
                || pData->btDetailState == KANTURU_MAYA_DIRECTION_MAYA1
                || pData->btDetailState == KANTURU_MAYA_DIRECTION_MONSTER2
                || pData->btDetailState == KANTURU_MAYA_DIRECTION_MAYA2
                || pData->btDetailState == KANTURU_MAYA_DIRECTION_MONSTER3
                || pData->btDetailState == KANTURU_MAYA_DIRECTION_MAYA3
                ))
            || (pData->btState == KANTURU_STATE_NIGHTMARE_BATTLE
                && (pData->btDetailState == KANTURU_NIGHTMARE_DIRECTION_BATTLE
                    ))
            )
        {
            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU_INFO) == false)
            {
                g_pNewUISystem->Show(SEASON3B::INTERFACE_KANTURU_INFO);
            }
        }
        else
        {
            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_KANTURU_INFO) == true)
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_KANTURU_INFO);
            }
        }
        M39Kanturu3rd::Kanturu3rdState(pData->btState, pData->btDetailState);
        M39Kanturu3rd::CheckSuccessBattle(pData->btState, pData->btDetailState);
    }
}

void ReceiveKanturu3rdResult(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_KANTURU_BATTLE_RESULT)ReceiveBuffer;
    M39Kanturu3rd::Kanturu3rdResult(pData->btResult);
}

void ReceiveKanturu3rdTimer(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_KANTURU_BATTLE_SCENE_TIMELIMIT)ReceiveBuffer;
    g_pKanturuInfoWindow->SetTime(pData->btTimeLimit);
}

void RecevieKanturu3rdMayaSKill(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NOTIFY_KANTURU_WIDE_AREA_ATTACK)ReceiveBuffer;
    M39Kanturu3rd::MayaSceneMayaAction(pData->btType);
}

void RecevieKanturu3rdLeftUserandMonsterCount(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NOTIFY_KANTURU_USER_MONSTER_COUNT)ReceiveBuffer;
    M39Kanturu3rd::Kanturu3rdUserandMonsterCount(pData->bMonsterCount, pData->btUserCount);
}

void ReceiveCursedTempleEnterInfo(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TEMPLE_USER_COUNT)ReceiveBuffer;

    g_pCursedTempleEnterWindow->ReceiveCursedTempleEnterInfo(ReceiveBuffer);
}

void ReceiveCursedTempleEnterResult(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_RESULT_ENTER_CURSED_TEMPLE)ReceiveBuffer;

    if (data->Result == 0)
    {
        g_pNewUISystem->HideAll();
    }
    g_CursedTemple->UpdateTempleSystemMsg(data->Result);
}

void ReceiveCursedTempleInfo(const BYTE* ReceiveBuffer)
{
    g_pCursedTempleWindow->ReceiveCursedTempleInfo(ReceiveBuffer);
    g_CursedTemple->ReceiveCursedTempleInfo(ReceiveBuffer);
}

void ReceiveCursedTempMagicResult(const BYTE* ReceiveBuffer)
{
    g_pCursedTempleWindow->ReceiveCursedTempRegisterSkill(ReceiveBuffer);
}

void ReceiveCursedTempSkillEnd(const BYTE* ReceiveBuffer)
{
    g_pCursedTempleWindow->ReceiveCursedTempUnRegisterSkill(ReceiveBuffer);
}

void ReceiveCursedTempSkillPoint(const BYTE* ReceiveBuffer)
{
    g_pCursedTempleWindow->ReceiveCursedTempSkillPoint(ReceiveBuffer);
}

void ReceiveCursedTempleHolyItemRelics(const BYTE* ReceiveBuffer)
{
    g_pCursedTempleWindow->ReceiveCursedTempleHolyItemRelics(ReceiveBuffer);
}

void ReceiveCursedTempleGameResult(const BYTE* ReceiveBuffer)
{
    g_pNewUISystem->HideAll();

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM))
    {
        g_pCursedTempleResultWindow->ResetGameResultInfo();
        g_pCursedTempleResultWindow->SetMyTeam(g_pCursedTempleWindow->GetMyTeam());

        g_pNewUISystem->Hide(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
    }

    PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM5);

    g_pNewUISystem->Show(SEASON3B::INTERFACE_CURSEDTEMPLE_RESULT);

    g_pCursedTempleResultWindow->ReceiveCursedTempleGameResult(ReceiveBuffer);
}

void ReceiveCursedTempleState(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_ILLUSION_TEMPLE_STATE)ReceiveBuffer;

    // 0: wait, 1: wait->ready, 2: ready->play, 3: play->end
    auto cursedtemple = static_cast<SEASON3A::eCursedTempleState>(data->btIllusionTempleState + 1);

    if (cursedtemple == SEASON3A::eCursedTempleState_Ready)
    {
        g_pNewUISystem->HideAll();

        g_pCursedTempleWindow->ResetCursedTempleSystemInfo();
        g_pCursedTempleWindow->StartTutorialStep();

        PlayBuffer(SOUND_CURSEDTEMPLE_GAMESYSTEM1);

        g_pNewUISystem->Show(SEASON3B::INTERFACE_CURSEDTEMPLE_GAMESYSTEM);
    }

    g_CursedTemple->ReceiveCursedTempleState(cursedtemple);
}

void ReceiveRaklionStateInfo(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_RAKLION_STATE_INFO)ReceiveBuffer;
    g_Raklion.SetState(pData->btState, pData->btDetailState);
}

void ReceiveRaklionCurrentState(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_RAKLION_CURRENT_STATE)ReceiveBuffer;
    g_Raklion.SetState(pData->btCurrentState, pData->btCurrentDetailState);
}

void RecevieRaklionStateChange(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_RAKLION_STATE_CHANGE)ReceiveBuffer;
    g_Raklion.SetState(pData->btState, pData->btDetailState);
}

void RecevieRaklionBattleResult(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_ANS_RAKLION_BATTLE_RESULT)ReceiveBuffer;
}

void RecevieRaklionWideAreaAttack(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NOTIFY_RAKLION_WIDE_AREA_ATTACK)ReceiveBuffer;
}

void RecevieRaklionUserMonsterCount(const BYTE* ReceiveBuffer)
{
    auto pData = (LPPMSG_NOTIFY_RAKLION_USER_MONSTER_COUNT)ReceiveBuffer;
}

void ReceiveCheckSumRequest(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPHEADER_DEFAULT_WORD)ReceiveBuffer;
    DWORD dwCheckSum = GetCheckSum(Data->Value);
    SocketClient->ToGameServer()->SendChecksumResponse(dwCheckSum);

    g_ConsoleDebug->Write(MCD_RECEIVE, L"0x03 [ReceiveCheckSumRequest]");
}

extern int TimeRemain;

bool ReceiveRegistedLuckyCoin(const BYTE* ReceiveBuffer)
{
    auto _pData = (LPPMSG_ANS_GET_COIN_COUNT)ReceiveBuffer;

    if (_pData->nCoinCnt >= 0)
    {
        g_pLuckyCoinRegistration->SetRegistCount(_pData->nCoinCnt);
        return true;
    }
    return false;
}

bool ReceiveRegistLuckyCoin(const BYTE* ReceiveBuffer)
{
    auto _pData = (LPPMSG_ANS_REGEIST_COIN)ReceiveBuffer;

    g_pLuckyCoinRegistration->UnLockLuckyCoinRegBtn();

    switch (_pData->btResult)
    {
    case 0:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CUseRegistLuckyCoinMsgBoxLayout));
    }
    break;
    case 1:
    {
        g_pLuckyCoinRegistration->SetRegistCount(_pData->nCurCoinCnt);
    }
    break;
    case 100:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CRegistOverLuckyCoinMsgBoxLayout));
    }
    break;
    default:
        return false;
    }

    return true;
}

bool ReceiveRequestExChangeLuckyCoin(const BYTE* ReceiveBuffer)
{
    auto _pData = (LPPMSG_ANS_TREADE_COIN)ReceiveBuffer;

    g_pExchangeLuckyCoinWindow->UnLockExchangeBtn();

    switch (_pData->btResult)
    {
    case 0:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CExchangeLuckyCoinMsgBoxLayout));
    }
    break;
    case 1:
    {
        //g_pNewUISystem->Hide(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
        g_pSystemLogBox->AddText(GlobalText[1888], SEASON3B::TYPE_SYSTEM_MESSAGE);
    }break;
    case 2:
    {
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CExchangeLuckyCoinInvenErrMsgBoxLayout));
    }break;
    default:
        return false;
    }

    return true;
}

bool ReceiveEnterDoppelGangerEvent(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_RESULT_ENTER_DOPPELGANGER)ReceiveBuffer;

    wchar_t szText[256] = { 0, };

    switch (Data->btResult)
    {
    case 0:
        break;
    case 1:
        g_pDoppelGangerWindow->LockEnterButton(TRUE);
        break;
    case 2:
        swprintf(szText, GlobalText[2864]);
        g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
        g_pDoppelGangerWindow->LockEnterButton(TRUE);
        break;
    case 3:
        swprintf(szText, GlobalText[2865]);
        g_pSystemLogBox->AddText(szText, SEASON3B::TYPE_ERROR_MESSAGE);
        g_pDoppelGangerWindow->LockEnterButton(TRUE);
        break;
    case 4:
        g_pDoppelGangerWindow->LockEnterButton(FALSE);
        break;
    default:
        return false;
    }

    return true;
}

bool ReceiveDoppelGangerMonsterPosition(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DOPPELGANGER_MONSTER_POSITION)ReceiveBuffer;
    g_pDoppelGangerFrame->SetMonsterGauge((float)Data->btPosIndex / 22.0f);
    return true;
}

bool ReceiveDoppelGangerState(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DOPPELGANGER_STATE)ReceiveBuffer;

    switch (Data->btDoppelgangerState)
    {
    case 0:
        break;
    case 1:		// wait->ready
        break;
    case 2:		// ready->play
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_DOPPELGANGER_FRAME);

        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDoppelGangerMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2763], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        pMsgBox->AddMsg(L" ");
        pMsgBox->AddMsg(GlobalText[2764], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        pMsgBox->AddMsg(L" ");
        pMsgBox->AddMsg(GlobalText[2765], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }
    break;
    case 3:		// play->end
        break;
    }

    return true;
}

bool ReceiveDoppelGangerIcewalkerState(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DOPPELGANGER_ICEWORKER_STATE)ReceiveBuffer;

    switch (Data->btIceworkerState)
    {
    case 0:
        g_pDoppelGangerFrame->SetIceWalkerMap(TRUE, (float)(22 - Data->btPosIndex) / 22.0f);
        break;
    case 1:
        g_pDoppelGangerFrame->SetIceWalkerMap(FALSE, 0);
        break;
    }

    return true;
}

bool ReceiveDoppelGangerTimePartyState(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DOPPELGANGER_PLAY_INFO)ReceiveBuffer;

    g_pDoppelGangerFrame->SetRemainTime(Data->wRemainSec);
    g_pDoppelGangerFrame->SetPartyMemberRcvd();
    auto pUserPos = (LPPMSG_DOPPELGANGER_USER_POS)&Data->UserPosData;
    for (int i = 0; i < Data->btUserCount; ++i)
    {
        g_pDoppelGangerFrame->SetPartyMemberInfo(pUserPos[i].wUserIndex, (float)(22 - pUserPos[i].btPosIndex) / 22.0f);
    }

    return true;
}

bool ReceiveDoppelGangerResult(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DOPPELGANGER_RESULT)ReceiveBuffer;

    PlayBuffer(SOUND_CHAOS_END);

    g_pDoppelGangerFrame->StopTimer(TRUE);
    g_pDoppelGangerFrame->EnabledDoppelGangerEvent(FALSE);

    switch (Data->btResult)
    {
    case 0:
    {
        g_pDoppelGangerFrame->SetRemainTime(0);

        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDoppelGangerMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2769], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        pMsgBox->AddMsg(L" ");
        pMsgBox->AddMsg(GlobalText[2770], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        // 			pMsgBox->AddMsg(L" ");
        // 			pMsgBox->AddMsg(L" ");
        // 			char szText[256] = { 0, };
        // 			wprintf(szText, GlobalText[2771], Data->dwRewardExp);
        // 			pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_BOLD);
    }
    break;
    case 1:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDoppelGangerMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2766], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }
    break;
    case 2:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDoppelGangerMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2767], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        pMsgBox->AddMsg(L" ");
        pMsgBox->AddMsg(GlobalText[2768], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }
    break;
    }

    return true;
}

bool ReceiveDoppelGangerMonsterGoal(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_DOPPELGANGER_MONSTER_GOAL)ReceiveBuffer;

    g_pDoppelGangerFrame->SetMaxMonsters(Data->btMaxGoalCnt);
    g_pDoppelGangerFrame->SetEnteredMonsters(Data->btGoalCnt);

    return true;
}

bool ReceiveMoveMapChecksum(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_MAPMOVE_CHECKSUM)ReceiveBuffer;

    g_pMoveCommandWindow->SetMoveCommandKey(Data->dwKeyValue);

    return true;
}

bool ReceiveRequestMoveMap(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_ANS_MAPMOVE)ReceiveBuffer;

    if (Data->btResult == 0)
    {
    }
    else
    {
    }

    // 	MAPMOVE_SUCCESS
    // 	MAPMOVE_FAILED,
    // 	MAPMOVE_FAILED_TELEPORT,
    // 	MAPMOVE_FAILED_PSHOP_OPEN,
    // 	MAPMOVE_FAILED_RECALLED,
    // 	MAPMOVE_FAILED_NOT_ENOUGH_EQUIP,
    // 	MAPMOVE_FAILED_WEARING_EQUIP,
    // 	MAPMOVE_FAILED_MURDERER,
    // 	MAPMOVE_FAILED_NOT_ENOUGH_LEVEL,
    // 	MAPMOVE_FAILED_NOT_ENOUGH_ZEN,
    // 	MAPMOVE_FORCED_EVENT_END		= 20,
    // 	MAPMOVE_FORCED_GM

    return true;
}

bool ReceiveEnterEmpireGuardianEvent(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_RESULT_ENTER_EMPIREGUARDIAN)ReceiveBuffer;

    switch (Data->Result)
    {
    case 0:
    {
        g_pEmpireGuardianTimer->SetDay((int)Data->Day);
        g_pEmpireGuardianTimer->SetZone((int)Data->Zone);
        g_pEmpireGuardianTimer->SetRemainTime(Data->RemainTick);

        g_EmpireGuardian1.SetWeather((int)Data->Wheather);
    }break;
    case 1:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2798], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        pMsgBox->AddMsg(L" ");
        wchar_t szText[256] = { NULL, };
        swprintf(szText, GlobalText[2799], (Data->RemainTick / 60000));
        pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }break;
    case 2:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2839], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }break;
    case 3:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2841], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }break;
    case 4:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2842], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }break;
    case 5:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2843], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }break;

    default:
        return false;
    }

    return true;
}

bool ReceiveRemainTickEmpireGuardian(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_REMAINTICK_EMPIREGUARDIAN)ReceiveBuffer;

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER) == false)
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
    }

    g_pEmpireGuardianTimer->SetType((int)Data->Type);
    g_pEmpireGuardianTimer->SetRemainTime((int)Data->RemainTick);
    g_pEmpireGuardianTimer->SetMonsterCount((int)Data->MonsterCount);

    return true;
}

bool ReceiveResultEmpireGuardian(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPMSG_CLEAR_RESULT_EMPIREGUARDIAN)ReceiveBuffer;

    switch (Data->Result)
    {
    case 0:
    {
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
        pMsgBox->AddMsg(GlobalText[2803], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        pMsgBox->AddMsg(GlobalText[2804], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }break;
    case 1:
    {
        int day = g_pEmpireGuardianTimer->GetDay();
        int zone = g_pEmpireGuardianTimer->GetZone();
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
        wchar_t szText[256] = { NULL, };
        swprintf(szText, GlobalText[2801], day);
        pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        swprintf(szText, L"%d%s", zone, GlobalText[2840]);
        pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }break;
    case 2:
    {
        int day = g_pEmpireGuardianTimer->GetDay();
        SEASON3B::CNewUICommonMessageBox* pMsgBox;
        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CEmpireGuardianMsgBoxLayout), &pMsgBox);
        wchar_t szText[256] = { NULL, };
        swprintf(szText, GlobalText[2801], day);
        pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        pMsgBox->AddMsg(GlobalText[2802], RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
        swprintf(szText, GlobalText[861], Data->Exp);
        pMsgBox->AddMsg(szText, RGBA(255, 255, 255, 255), SEASON3B::MSGBOX_FONT_NORMAL);
    }break;
    }

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER) == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_TIMER);
    }

    return true;
}

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM

// (0xD2)(0x01)
bool ReceiveIGS_CashPoint(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_CASHPOINT_ANS)pReceiveBuffer;
    g_InGameShopSystem->SetTotalCash((double)Data->dTotalCash);
    g_InGameShopSystem->SetTotalPoint((double)Data->dTotalPoint);
    g_InGameShopSystem->SetCashCreditCard((double)Data->dCashCredit);
    g_InGameShopSystem->SetCashPrepaid((double)Data->dCashPrepaid);
    g_InGameShopSystem->SetTotalMileage((double)Data->dTotalMileage);
    return true;
}

// (0xD2)(0x02)
bool ReceiveIGS_ShopOpenResult(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_SHOPOPEN_ANS)pReceiveBuffer;

    g_InGameShopSystem->SetIsRequestShopOpenning(false);

    if ((BYTE)Data->byShopOpenResult == 0)
    {
        return false;
    }

    SocketClient->ToGameServer()->SendCashShopPointInfoRequest();
    char szCode = g_pInGameShop->GetCurrentStorageCode();
    SocketClient->ToGameServer()->SendCashShopStorageListRequest(1, szCode);

    g_pNewUISystem->Show(SEASON3B::INTERFACE_INGAMESHOP);

    return true;
}

// (0xD2)(0x03)
bool ReceiveIGS_BuyItem(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_BUYITEM_ANS)pReceiveBuffer;

    switch ((BYTE)Data->byResultCode)
    {
    case -2:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[2953]);
    }
    break;
    case -1:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[2954]);
    }
    break;
    case 0:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2900], GlobalText[2901]);

        SocketClient->ToGameServer()->SendCashShopPointInfoRequest();

        char szCode = g_pInGameShop->GetCurrentStorageCode();
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(1, szCode);
    }
    break;
    case 1:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[2903]);
    }
    break;
    case 2:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[2904]);
    }
    break;
    case 3:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[2956]);
    }
    break;
    case 4:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[2957]);
    }
    break;
    case 5:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[2958]);
    }
    break;
    case 6:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[3052]);
    }break;
    case 7:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[3053]);
    }break;
    case 8:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[3054]);
    }break;
    case 9:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2902], GlobalText[3264]);
    }
    break;
    default:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2945], GlobalText[890]);
    }
    break;
    }

    return true;
}

//----------------------------------------------------------------------------
// (0xD2)(0x04)
bool ReceiveIGS_SendItemGift(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_GIFTSEND_ANS)pReceiveBuffer;

    switch ((BYTE)Data->byResultCode)
    {
    case -2:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2953]);
    }
    break;
    case -1:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2954]);
    }
    break;
    case 0:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2910], GlobalText[2911]);

        SocketClient->ToGameServer()->SendCashShopPointInfoRequest();
    }
    break;
    case 1:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2913]);
    }
    break;
    case 2:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2914]);
    }
    break;
    case 3:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2915]);
    }
    break;
    case 4:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2956]);
    }
    break;
    case 5:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2958]);
    }
    break;
    case 6:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2945], GlobalText[2958]);
    }break;
    case 7:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2959]);
    }
    break;
    case 8:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2960]);
    }
    break;
    case 9:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[2961]);
    }
    break;
    case 10:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[3264]);
    }
    break;
    case 20:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2912], GlobalText[3263]);
    }
    break;
    default:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2945], GlobalText[890]);
    }
    break;
    }

    return true;
}

// (0xD2)(0x06)
bool ReceiveIGS_StorageItemListCount(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_STORAGECOUNT)pReceiveBuffer;
    g_pInGameShop->InitStorage((int)Data->wTotalItemCount, (int)Data->wCurrentItemCount, (int)Data->wTotalPage, (int)Data->wPageIndex);
    return true;
}

// (0xD2)(0x0D)
bool ReceiveIGS_StorageItemList(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_STORAGELIST)pReceiveBuffer;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    if (g_InGameShopSystem->IsShopOpen() == false)
        return false;
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

    g_pInGameShop->AddStorageItem((int)Data->lStorageIndex, (int)Data->lItemSeq, (int)Data->lStorageGroupCode, (int)Data->lProductSeq, (int)Data->lPriceSeq, (int)Data->dCashPoint, (char)Data->chItemType);
    return true;
}

// (0xD2)(0x0E)
bool ReceiveIGS_StorageGiftItemList(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_GIFTSTORAGELIST)pReceiveBuffer;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    if (g_InGameShopSystem->IsShopOpen() == false)
        return false;
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

    wchar_t szID[MAX_ID_SIZE + 1];
    wchar_t szMessage[MAX_GIFT_MESSAGE_SIZE];
    CMultiLanguage::ConvertFromUtf8(szID, Data->chSendUserName, MAX_ID_SIZE);
    CMultiLanguage::ConvertFromUtf8(szMessage, Data->chMessage, MAX_GIFT_MESSAGE_SIZE);

    g_pInGameShop->AddStorageItem((int)Data->lStorageIndex, (int)Data->lItemSeq, (int)Data->lStorageGroupCode, (int)Data->lProductSeq, (int)Data->lPriceSeq, (int)Data->dCashPoint, (char)Data->chItemType, szID, szMessage);
    return true;
}

//----------------------------------------------------------------------------
// (0xD2)(0x07)
bool ReceiveIGS_SendCashGift(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_CASHSEND_ANS)pReceiveBuffer;
    return true;
}

//----------------------------------------------------------------------------
// (0xD2)(0x08)
bool ReceiveIGS_PossibleBuy(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_ITEMBUY_CONFIRM_ANS)pReceiveBuffer;
    return true;
}

// (0xD2)(0x09)
bool ReceiveIGS_LeftCountItem(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_ITEMBUY_LEFT_COUNT_ANS)pReceiveBuffer;
    return true;
}

// (0xD2)(0x0B)
bool ReceiveIGS_UseStorageItem(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_STORAGE_ITEM_USE_ANS)pReceiveBuffer;

    switch ((BYTE)Data->byResult)
    {
    case -2:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[2967]);
    }
    break;
    case -1:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[2966]);
    }
    break;
    case 0:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2924], GlobalText[2925]);

        g_pInGameShop->UpdateStorageItemList();
    }
    break;
    case 1:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[2962]);
    }
    break;
    case 2:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[2963]);
    }
    break;
    case 3:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[2964]);
    }
    break;
    case 4:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[2965]);
    }break;
    case 21:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[2284]);
    }
    break;
    case 22:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[3036]);
    }
    break;
#ifdef LEM_FIX_SERVERMSG_SEALITEM
    case 24:
    {
        CMsgBoxIGSCommon* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2928], GlobalText[2610]);
    }
    break;
#endif	// LEM_FIX_SERVERMSG_SEALITEM
    default:
    {
        CMsgBoxIGSCommon* pMsgBox = nullptr;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSCommonLayout), &pMsgBox);
        pMsgBox->Initialize(GlobalText[2945], GlobalText[890]);
    }
    break;
    }
    return true;
}

// (0xD2)(0x0C)
bool ReceiveIGS_UpdateScript(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_VERSION_UPDATE)pReceiveBuffer;

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    g_InGameShopSystem->SetScriptVersion(Data->wSaleZone, Data->wYear, Data->wYearIdentify);
    g_InGameShopSystem->ShopOpenUnLock();
#else // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INGAMESHOP) == true)
    {
        SendRequestIGS_CashShopOpen(1);
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_INGAMESHOP);
    }

    g_InGameShopSystem->Release();

    g_InGameShopSystem->SetScriptVersion(Data->wSaleZone, Data->wYear, Data->wYearIdentify);

    if (g_InGameShopSystem->ScriptDownload() == false)
    {
        return false;
    }

    g_InGameShopSystem->Initalize();
    g_pInGameShop->InitZoneBtn();
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

    return true;
}

// (0xD2)(0x13)
bool ReceiveIGS_EventItemlistCnt(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_EVENTITEM_COUNT)pReceiveBuffer;

    g_InGameShopSystem->InitEventPackage((int)Data->wEventItemListCount);

    return true;
}

//(0xD2)(0x14)
bool ReceiveIGS_EventItemlist(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_CASHSHOP_EVENTITEM_LIST)pReceiveBuffer;
    g_InGameShopSystem->InsertEventPackage((int*)Data->lPackageSeq);
    return true;
}

// (0xD2)(0x15)
bool ReceiveIGS_UpdateBanner(const BYTE* pReceiveBuffer)
{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
    auto Data = (LPPMSG_CASHSHOP_BANNER_UPDATE)pReceiveBuffer;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    g_InGameShopSystem->SetBannerVersion(Data->wBannerZone, Data->wYear, Data->wYearIdentify);
#else // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
    if (g_InGameShopSystem->IsShopOpen() == false)
        return false;

#ifdef _DEBUG
    g_InGameShopSystem->SetBannerVersion(583, 0, 0);
#else // _DEBUG
    g_InGameShopSystem->SetBannerVersion(Data->wBannerZone, Data->wYear, Data->wYearIdentify);
#endif // _DEBUG

    if (g_InGameShopSystem->BannerDownload() == false)
    {
        return false;
    }

    g_pInGameShop->InitBanner(g_InGameShopSystem->GetBannerFileName(), g_InGameShopSystem->GetBannerURL());
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD

    return true;
}

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM

#ifdef PBG_ADD_SECRETBUFF
bool ReceiveFatigueTime(const BYTE* pReceiveBuffer)
{
    LPPMSG_FATIGUEPERCENTAGE Data = (LPPMSG_FATIGUEPERCENTAGE)pReceiveBuffer;

    if (g_FatigueTimeSystem->SetFatiguePercentage(Data->btFatiguePercentage))
    {
        g_FatigueTimeSystem->SetIsFatigueSystem(true);
#ifdef PBG_MOD_STAMINA_UI
        g_pNewUIStamina->SetCaution(Data->btFatiguePercentage);
#endif //PBG_MOD_STAMINA_UI
        return true;
    }
    else
        return false;

    return false;
}
#endif //PBG_ADD_SECRETBUFF

#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
bool ReceiveEquippingInventoryItem(const BYTE* pReceiveBuffer)
{
    LPPMSG_ANS_INVENTORY_EQUIPMENT_ITEM Data = (LPPMSG_ANS_INVENTORY_EQUIPMENT_ITEM)pReceiveBuffer;

    int iResult = (int)Data->btResult;
    int iItemPos = (int)Data->btItemPos;

    if (!(iResult == 254 || iResult == 255))
        return false;

    if (iItemPos < MAX_EQUIPMENT || iItemPos >= MAX_INVENTORY)
        return false;

    ITEM* pItem = g_pMyInventory->FindItem(iItemPos);
    pItem->Durability = iResult;

#ifdef CONSOLE_DEBUG
    g_ConsoleDebug->Write(MCD_RECEIVE, L"[0xBF][0x20]  [ReceiveEquippingInventoryItem]");
#endif // CONSOLE_DEBUG

    return true;
}
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY

#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM

bool ReceivePeriodItemListCount(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_PERIODITEMEX_ITEMCOUNT)pReceiveBuffer;
    return true;
}

bool ReceivePeriodItemList(const BYTE* pReceiveBuffer)
{
    auto Data = (LPPMSG_PERIODITEMEX_ITEMLIST)pReceiveBuffer;

    if (Data->wItemSlotIndex < MAX_EQUIPMENT_INDEX)
    {
        CharacterMachine->Equipment[Data->wItemSlotIndex].lExpireTime = (long)Data->lExpireDate;
    }
    else
    {
        ITEM* pItem = g_pMyInventory->FindItem(Data->wItemSlotIndex);

        if (pItem == nullptr)
            return false;

        pItem->lExpireTime = (long)Data->lExpireDate;
    }

    return true;
}
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

BOOL ReceiveStraightAttack(const BYTE* ReceiveBuffer, int Size, BOOL bEncrypted)
{
    auto Data = (LPPRECEIVE_STRAIGHTATTACK)ReceiveBuffer;
    int SourceKey = ((int)(Data->SourceKeyH) << 8) + Data->SourceKeyL;
    int TargetKey = ((int)(Data->TargetKeyH) << 8) + Data->TargetKeyL;
    int Success = (TargetKey >> 15);

    WORD AttackNumber = ((WORD)(Data->AttackH) << 8) + Data->AttackL;

    TargetKey &= 0x7FFF;
    int Index = FindCharacterIndex(SourceKey);
    int TargetIndex = FindCharacterIndex(TargetKey);
    if (TargetIndex == MAX_CHARACTERS_CLIENT && AttackNumber != AT_SKILL_DARKSIDE && AttackNumber != AT_SKILL_DARKSIDE_STR)
        return (TRUE);

    AttackPlayer = Index;
    CHARACTER* sc = &CharactersClient[Index];
    CHARACTER* tc = &CharactersClient[TargetIndex];
    OBJECT* so = &sc->Object;
    OBJECT* to = &tc->Object;

    if (sc != Hero && to->Visible)
        so->Angle[2] = CreateAngle2D(so->Position, to->Position);

    sc->TargetCharacter = TargetIndex;

    sc->SkillSuccess = (Success != 0);
    sc->Skill = AttackNumber;

    so->m_sTargetIndex = TargetIndex;

    if (g_CMonkSystem.IsRageHalfwaySkillAni(AttackNumber))
    {
        auto isBeastUppercut = AttackNumber == AT_SKILL_BEAST_UPPERCUT || AttackNumber == AT_SKILL_BEAST_UPPERCUT_STR || AttackNumber == AT_SKILL_BEAST_UPPERCUT_MASTERY;
        if (sc != Hero && isBeastUppercut)
        {
            g_CMonkSystem.SetRageSkillAni(AttackNumber, so);
            g_CMonkSystem.RageCreateEffect(so, AttackNumber);
        }
        else if (!isBeastUppercut)
        {
            g_CMonkSystem.SetRageSkillAni(AttackNumber, so);
        }
        sc->AttackTime = 1;
    }

    switch (AttackNumber)
    {
    case AT_SKILL_DARKSIDE:
    case AT_SKILL_DARKSIDE_STR:
    {
        if (sc != Hero)
            g_CMonkSystem.RageCreateEffect(so, AttackNumber);

        g_CMonkSystem.SetRageSkillAni(AttackNumber, so);
        sc->AttackTime = 1;
    }
    break;
    }

    return (TRUE);
}

void ReceiveDarkside(const BYTE* ReceiveBuffer)
{
    auto Data = (LPPRECEIVE_DARKSIDE_INDEX)ReceiveBuffer;

    auto skill = static_cast<ActionSkillType>(Data->MagicNumber);
    if (skill == AT_SKILL_DARKSIDE || skill == AT_SKILL_DARKSIDE_STR)
    {
        g_CMonkSystem.InitDarksideTarget();
        g_CMonkSystem.SetDarksideTargetIndex(Data->TargerIndex, skill);
    }
}

static void ProcessPacket(const BYTE* ReceiveBuffer, int32_t Size)
{
    auto received_span = std::span<const BYTE>(ReceiveBuffer, Size);
    BYTE HeadCode = 0;
    BOOL bEncrypted = ReceiveBuffer[0] >= 0xC3;
    BOOL bIsC1C3 = ReceiveBuffer[0] % 2 == 1;
    if (bIsC1C3) // C1 and C3
    {
        HeadCode = ReceiveBuffer[2];
    }
    else
    {
        HeadCode = ReceiveBuffer[3];
    }
    switch (HeadCode)
    {
    case 0xF1:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
        case 0x00: //receive join server
            ReceiveJoinServer(ReceiveBuffer);
            break;
        case 0x01: //receive log in
            //AddDebugText(ReceiveBuffer,Size);
            switch (Data->Value)
            {
            case 0x20:
                CurrentProtocolState = RECEIVE_LOG_IN_SUCCESS;
                LogIn = 2;
                CheckHack();
                break;
            case 0x00:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PASSWORD);
                break;
            case 0x01:
                CurrentProtocolState = RECEIVE_LOG_IN_SUCCESS;
                LogIn = 2;
                CheckHack();
                break;
            case 0x02:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID);
                break;
            case 0x03:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID_CONNECTED);
                break;
            case 0x04:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_SERVER_BUSY);
                break;
            case 0x05:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ID_BLOCK);
                break;
            case 0x06:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_VERSION);
                g_ErrorReport.Write(L"Version dismatch. - Login\r\n");
                break;
            case 0x07:
            default:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_CONNECT);
                break;
            case 0x08:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ERROR);
                break;
            case 0x09:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO);
                break;
            case 0x0a:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_USER_TIME1);
                break;
            case 0x0b:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_USER_TIME2);
                break;
            case 0x0c:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PC_TIME1);
                break;
            case 0x0d:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_PC_TIME2);
                break;
            case 0x11:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_ONLY_OVER_15);
                break;
            case 0x40:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL);
                break;
            case 0xc0:
            case 0xd0:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_POINT_DATE);
                break;
            case 0xc1:
            case 0xd1:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_POINT_HOUR);
                break;
            case 0xc2:
            case 0xd2:
                CUIMng::Instance().PopUpMsgWin(RECEIVE_LOG_IN_FAIL_INVALID_IP);
                break;
            }
            break;
        case 0x02:
            if (!ReceiveLogOut(ReceiveBuffer, bEncrypted))
            {
                //return ( FALSE);
            }
            break;
        case 0x12: //0x02 receive create account
            switch (Data->Value)
            {
            case 0x00:
                CurrentProtocolState = RECEIVE_CREATE_ACCOUNT_FAIL_ID;
                break;
            case 0x01:
                CurrentProtocolState = RECEIVE_CREATE_ACCOUNT_SUCCESS;
                break;
            case 0x02:
                CurrentProtocolState = RECEIVE_CREATE_ACCOUNT_FAIL_RESIDENT;
                break;
            }
            break;
        case 0x03: //receive confirm password
            //AddDebugText(ReceiveBuffer,Size);
            ReceiveConfirmPassword(ReceiveBuffer);
            break;
        case 0x04: //receive confirm password
            //AddDebugText(ReceiveBuffer,Size);
            ReceiveConfirmPassword2(received_span);
            break;
        case 0x05: //receive change password
            ReceiveChangePassword(ReceiveBuffer);
            break;
        }
        break;
    }
    case 0xF3:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }

        g_ConsoleDebug->Write(MCD_RECEIVE, L"Recv [0xF3][0x%02x]", subcode);

        switch (subcode)
        {
        case 0x00: //receive characters list
            ReceiveCharacterListExtended(ReceiveBuffer);
            break;
        case 0x01: //receive create character
            ReceiveCreateCharacter(ReceiveBuffer);
            break;
        case 0x02: //receive delete character
            ReceiveDeleteCharacter(ReceiveBuffer);
            break;
        case 0x03: //receive join map server
            if (!ReceiveJoinMapServer(received_span))
            {
                //return ( FALSE);
            }
            break;
        case 0x04: //receive revival
            ReceiveRevival(ReceiveBuffer);
            break;
        case 0x10: //receive inventory
            ReceiveInventoryExtended(received_span);
            break;
        case 0x05: //receive level up
            ReceiveLevelUp(ReceiveBuffer, Size);
            break;
        case 0x06: //receive Add Point
            if (Size >= sizeof(PRECEIVE_ADD_POINT_EXTENDED))
            {
                ReceiveAddPointExtended(ReceiveBuffer);
            }
            else
            {
                ReceiveAddPoint(ReceiveBuffer);
            }

            break;
        case 0x07: //receive damage
            ReceiveDamage(ReceiveBuffer);
            break;
        case 0x08:
            ReceivePK(ReceiveBuffer);
            break;
        case 0x11:
            ReceiveMagicList(ReceiveBuffer);
            break;
        case 0x13:
            // not really in use in OpenMU
            ReceiveEquipment(received_span);
            break;
        case 0x14:
            ReceiveModifyItemExtended(received_span);
            break;
        case 0x20:
            ReceiveSummonLife(ReceiveBuffer);
            break;
        case 0x22:
            ReceiveWTTimeLeft(ReceiveBuffer);
            break;

        case 0x24:
            ReceiveWTMatchResult(ReceiveBuffer);
            break;

        case 0x25:
            ReceiveWTBattleSoccerGoalIn(ReceiveBuffer);
            break;
        case 0x23:
            ReceiveSoccerScore(ReceiveBuffer);
            break;
        case 0x30:
            ReceiveOption(ReceiveBuffer);
            break;
        case 0x32:
            ReceiveSetPointsExtended(ReceiveBuffer);
            break;
        case 0x40:
            ReceiveServerCommand(ReceiveBuffer);
            break;
        case 0x50:
            Receive_Master_Level_Exp(ReceiveBuffer, Size);
            break;
        case 0x51:
            Receive_Master_LevelUp(ReceiveBuffer, Size);
            break;
        case 0x52:
            Receive_Master_LevelGetSkill(ReceiveBuffer);
            break;
        case 0x53:
            Receive_Master_SetSkillList((PMSG_MASTER_SKILL_LIST_SEND*)ReceiveBuffer);
            break;
        }
        break;
    }
    case 0xF4:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x06:
            ReceiveServerList(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveServerConnect(ReceiveBuffer);
            break;
        case 0x05:
            ReceiveServerConnectBusy(ReceiveBuffer);
            break;
        }
        break;
    }
    case 0x00://chat
        ReceiveChat(ReceiveBuffer);
        break;
    case 0x01://chat
        ReceiveChatKey(ReceiveBuffer);
        break;
    case 0x02://chat whisper
        ReceiveChatWhisper(ReceiveBuffer);
        break;
    case 0x03:
        ReceiveCheckSumRequest(ReceiveBuffer);
        break;
    case 0x0B:
        AddDebugText(ReceiveBuffer, Size);
        ReceiveEvent(ReceiveBuffer);
        break;
    case 0x0C:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveChatWhisperResult(ReceiveBuffer);
        break;
    case 0x0D://notice
        ReceiveNotice(ReceiveBuffer);
        break;
    case 0x0f:
        ReceiveWeather(ReceiveBuffer);
        break;
    case PACKET_MOVE: //move character
        ReceiveMoveCharacter(received_span);
        break;
    case PACKET_POSITION: //move position
        ReceiveMovePosition(ReceiveBuffer);
        break;
    case 0x12: //create characters
        AddDebugText(ReceiveBuffer, Size);
        ReceiveCreatePlayerViewportExtended(received_span);
        break;
    case 0x13: //create monsters
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveCreateMonsterViewport(ReceiveBuffer);
        break;
    case 0x1F: //create monsters
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveCreateSummonViewport(ReceiveBuffer);
        break;
    case 0x45: //create monsters
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveCreateTransformViewport(received_span);
        break;
    case 0x14: //delete characters & monsters
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveDeleteCharacterViewport(ReceiveBuffer);
        break;
    case 0x20: //create item
        ReceiveCreateItemViewportExtended(received_span);
        break;
    case 0x2F:
        ReceiveCreateMoney(received_span);
        break;
    case 0x21://delete item
        ReceiveDeleteItemViewport(ReceiveBuffer);
        break;
    case 0x22://get item
        ReceiveGetItem(received_span);
        break;
    case 0x23://drop item
        ReceiveDropItem(ReceiveBuffer);
        break;
    case 0x24://equipment item
        AddDebugText(ReceiveBuffer, Size);
        ReceiveEquipmentItemExtended(received_span);
        break;
    case 0x25://change character
        ReceiveChangePlayer(received_span);
        break;
    case PACKET_ATTACK://attack character
        ReceiveAttackDamageExtended(ReceiveBuffer);
        break;
    case 0x18://action character
        ReceiveAction(ReceiveBuffer, Size);
        break;
    case 0x19://magic
        if (!ReceiveMagic(ReceiveBuffer, Size, bEncrypted))
        {
            //return ( FALSE);
        }
        break;
    case 0x69:
        if (!ReceiveMonsterSkill(ReceiveBuffer, Size, bEncrypted))
        {
            //return ( FALSE);
        }
        break;
    case 0x1A://magic
        ReceiveMagicPosition(ReceiveBuffer, Size);
        break;
    case 0x1E://magic
        if (!ReceiveMagicContinue(ReceiveBuffer, Size, bEncrypted))
        {
            //return ( FALSE);
        }
        break;
    case 0x1B://magic
        ReceiveMagicFinish(ReceiveBuffer);
        break;
    case 0x07://setmagicstatus
        ReceiveSkillStatus(ReceiveBuffer);
        break;
    //case 0x16://die character(exp)
    //    if (!ReceiveDieExp(ReceiveBuffer, bEncrypted))
    //    {
    //        //return ( FALSE);
    //    }
    //    break;
    case 0x16://die character(exp)
        if (Size >= sizeof(PRECEIVE_EXP_EXTENDED))
        {
            ReceiveDieExpLarge(ReceiveBuffer, bEncrypted);
        }
        else
        {
            ReceiveDieExp(ReceiveBuffer, bEncrypted);
        }
        break;
    case 0x17://die character
        ReceiveDie(ReceiveBuffer, Size);
        break;
    case 0x2A:
        AddDebugText(ReceiveBuffer, Size);
        ReceiveDurability(ReceiveBuffer);
        break;
    case 0x26:
        ReceiveStatsExtended(ReceiveBuffer);
        break;
    case 0x28:
        ReceiveDeleteInventory(ReceiveBuffer);
        break;
    case 0x29:
        //AddDebugText(ReceiveBuffer,Size);
        if (!ReceiveHelperItem(ReceiveBuffer, bEncrypted))
        {
            //return ( FALSE);
        }
        break;
    case 0x2c:
        ReceiveUseStateItem(ReceiveBuffer);
        break;
    case 0x30:
        if (!ReceiveTalk(ReceiveBuffer, bEncrypted))
        {
            //return ( FALSE);
        }
        break;
    case 0x31:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeInventoryExtended(received_span);
        break;
    case 0x32:
        ReceiveBuyExtended(received_span);
        break;
    case 0x33:
        ReceiveSell(ReceiveBuffer);
        break;
    case 0x34:
        ReceiveRepair(ReceiveBuffer);
        break;
    case 0x36:
        if (!ReceiveTrade(ReceiveBuffer, bEncrypted))
        {
            //return ( FALSE);
        }
        break;
    case 0x37:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeResult(ReceiveBuffer);
        break;
    case 0x38:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeYourInventoryDelete(ReceiveBuffer);
        break;
    case 0x39:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeYourInventoryExtended(received_span);
        break;
    case 0x3A:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeMyGold(ReceiveBuffer);
        break;
    case 0x3B:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeYourGold(ReceiveBuffer);
        break;
    case 0x3C:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeYourResult(ReceiveBuffer);
        break;
    case 0x3D:
        //AddDebugText(ReceiveBuffer,Size);
        ReceiveTradeExit(ReceiveBuffer);
        break;
    case 0x1C:
        AddDebugText(ReceiveBuffer, Size);
        if (!ReceiveTeleport(ReceiveBuffer, bEncrypted))
        {
            //return ( FALSE);
        }
        break;
    case 0x40:
        ReceiveParty(ReceiveBuffer);
        break;
    case 0x41:
        ReceivePartyResult(ReceiveBuffer);
        break;
    case 0x42:
        ReceivePartyList(ReceiveBuffer);
        break;
    case 0x43:
        ReceivePartyLeave(ReceiveBuffer);
        break;
    case 0x44:
        ReceivePartyInfo(ReceiveBuffer);
        break;
    case 0x46:
        ReceiveSetAttribute(ReceiveBuffer);
        break;
    case 0x47:
        ReceivePartyGetItem(ReceiveBuffer);
        break;
    case 0x48:
        ReceiveDisplayEffectViewport(ReceiveBuffer);
        break;
    case 0x50:
        ReceiveGuild(ReceiveBuffer);
        break;
    case 0x51:
        ReceiveGuildResult(ReceiveBuffer);
        break;
    case 0x52:
        ReceiveGuildList(ReceiveBuffer);
        break;
    case 0x53:
        ReceiveGuildLeave(ReceiveBuffer);
        break;
    case 0x54:
        ReceiveCreateGuildInterface(ReceiveBuffer);
        break;
    case 0x55:
        ReceiveCreateGuildMasterInterface(ReceiveBuffer);
        break;
    case 0x56:
        ReceiveCreateGuildResult(ReceiveBuffer);
        break;
    case 0x65:
        ReceiveGuildIDViewport(ReceiveBuffer);
        break;
    case 0x66:
        ReceiveGuildInfo(ReceiveBuffer);
        break;
    case 0x5D:
        ReceiveDeleteGuildViewport(ReceiveBuffer);
        break;
    case 0x60:
        ReceiveDeclareWarResult(ReceiveBuffer);
        break;
    case 0x61:
        ReceiveDeclareWar(ReceiveBuffer);
        break;
    case 0x62:
        ReceiveGuildBeginWar(ReceiveBuffer);
        break;
    case 0x63:
        ReceiveGuildEndWar(ReceiveBuffer);
        break;
    case 0x64:
        ReceiveGuildWarScore(ReceiveBuffer);
        break;
    case 0xE1:
        ReceiveGuildAssign(ReceiveBuffer);
        break;
    case 0xE5:
        ReceiveGuildRelationShip(ReceiveBuffer);
        break;
    case 0xE6:
        ReceiveGuildRelationShipResult(ReceiveBuffer);
        break;
    case 0xEB:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
        case 0x01:
            ReceiveBanUnionGuildResult(ReceiveBuffer);
            break;
        }
    }
    break;
    case 0x67:
        ReceiveUnionViewportNotify(ReceiveBuffer);
        break;
    case 0xE9:
        ReceiveUnionList(ReceiveBuffer);
        break;
    case 0xBC:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
        case 0x00:
            ReceiveGemMixResult(ReceiveBuffer);
            break;
        case 0x01:
            ReceiveGemUnMixResult(ReceiveBuffer);
            break;
        }
    }
    break;
    case 0x68:
        ReceivePreviewPort(received_span);
        break;

    case 0x71:
        ReceivePing(ReceiveBuffer);
        break;
    case 0x81:
        ReceiveStorageGold(ReceiveBuffer);
        break;
    case 0x82:
        ReceiveStorageExit(ReceiveBuffer);
        break;
    case 0x83:
        ReceiveStorageStatus(ReceiveBuffer);
        break;
    case 0x86:
        ReceiveMixExtended(received_span);
        break;
    case 0x87:
        ReceiveMixExit(ReceiveBuffer);
        break;
    case 0x8E:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
        case 0x01:
            ReceiveMoveMapChecksum(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveRequestMoveMap(ReceiveBuffer);
            break;
        }
    }
    break;
    case 0x90:
        ReceiveMoveToDevilSquareResult(ReceiveBuffer);
        break;
    case 0x91:
        ReceiveEventZoneOpenTime(ReceiveBuffer);
        break;
    case 0x92:
        ReceiveDevilSquareCountDown(ReceiveBuffer);
        break;
    case 0x93:
        ReceiveDevilSquareRank(ReceiveBuffer);
        break;
    case 0x9A:
        ReceiveMoveToEventMatchResult(ReceiveBuffer);
        break;
    case 0x9B:
        ReceiveMatchGameCommand(ReceiveBuffer);
        break;
    case 0x94:
        ReceiveEventChipInfomation(ReceiveBuffer);
        break;
    case 0x95:
        ReceiveEventChip(ReceiveBuffer);
        break;
    case 0x96:
        ReceiveMutoNumber(ReceiveBuffer);
        break;
    case 0x99:
        ReceiveServerImmigration(ReceiveBuffer);
        break;
    case 0x9D:
        ReceiveScratchResult(ReceiveBuffer);
        break;
    case 0x9E:
        ReceivePlaySoundEffect(ReceiveBuffer);
        break;
    case 0x9F:
        ReceiveEventCount(ReceiveBuffer);
        break;
    case 0xA0:
        ReceiveQuestHistory(ReceiveBuffer);
        break;
    case 0xA1:
        ReceiveQuestState(ReceiveBuffer);
        break;
    case 0xA2:
        ReceiveQuestResult(ReceiveBuffer);
        break;
    case 0xA3:
        ReceiveQuestPrize(ReceiveBuffer);
        break;
    case 0xA4:
        ReceiveQuestMonKillInfo(ReceiveBuffer);
        break;
    case 0xF6:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
#ifdef ASG_ADD_TIME_LIMIT_QUEST
        case 0x00:
            ReceiveQuestLimitResult(ReceiveBuffer);
            break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST
        case 0x03:
            ReceiveQuestByEtcEPList(ReceiveBuffer);
            break;
#ifdef ASG_ADD_TIME_LIMIT_QUEST
        case 0x04:
            ReceiveQuestByItemUseEP(ReceiveBuffer);
#endif	// ASG_ADD_TIME_LIMIT_QUEST
        case 0x0A:
            ReceiveQuestByNPCEPList(ReceiveBuffer);
            break;
        case 0x0B:
            ReceiveQuestQSSelSentence(ReceiveBuffer);
            break;
        case 0x0C:
            ReceiveQuestQSRequestReward(ReceiveBuffer);
            break;
        case 0x0D:
            ReceiveQuestCompleteResult(ReceiveBuffer);
            break;
        case 0x0F:
            ReceiveQuestGiveUp(ReceiveBuffer);
            break;
        case 0x1A:
            ReceiveProgressQuestList(ReceiveBuffer);
            break;
        case 0x1B:
            ReceiveProgressQuestRequestReward(ReceiveBuffer);
            break;
        case 0x20:
            ReceiveProgressQuestListReady(ReceiveBuffer);
            break;
        }
    }
    break;
#ifdef ASG_ADD_GENS_SYSTEM
    case 0xF8:
    {
        BYTE bySubcode;

        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            bySubcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            bySubcode = Data->SubCode;
        }

        switch (bySubcode)
        {
        case 0x02:
            ReceiveGensJoining(ReceiveBuffer);
            break;
        case 0x04:
            ReceiveGensSecession(ReceiveBuffer);
            break;
        case 0x07:
            ReceivePlayerGensInfluence(ReceiveBuffer);
            break;
        case 0x05:
            ReceiveOtherPlayerGensInfluenceViewport(ReceiveBuffer);
            break;
#ifdef PBG_ADD_GENSRANKING
        case 0x0A:
            ReceiveReward(ReceiveBuffer);
            break;
#endif //PBG_ADD_GENSRANKING
        }
    }
    break;
#endif	// ASG_ADD_GENS_SYSTEM
    case 0xF9:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
        case 0x01:
            ReceiveNPCDlgUIStart(ReceiveBuffer);
            break;
        }
    }
    break;

    case 0xA7:
        ReceivePetCommand(ReceiveBuffer);
        break;
    case 0xA8:
        ReceivePetAttack(ReceiveBuffer);
        break;
    case 0xA9:
        ReceivePetInfo(ReceiveBuffer);
        break;

    case 0xAA:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
        case 0x01:
            ReceiveDuelStart(ReceiveBuffer);
            break;
        case 0x02:
            ReceiveDuelRequest(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveDuelEnd(ReceiveBuffer);
            break;
        case 0x04:
            ReceiveDuelScore(ReceiveBuffer);
            break;
        case 0x05:
            ReceiveDuelHP(ReceiveBuffer);
            break;
        case 0x06:
            ReceiveDuelChannelList(ReceiveBuffer);
            break;
        case 0x07:
            ReceiveDuelWatchRequestReply(ReceiveBuffer);
            break;
        case 0x08:
            ReceiveDuelWatcherJoin(ReceiveBuffer);
            break;
        case 0x09:
            ReceiveDuelWatchEnd(ReceiveBuffer);
            break;
        case 0x0A:
            ReceiveDuelWatcherQuit(ReceiveBuffer);
            break;
        case 0x0B:
            ReceiveDuelWatcherList(ReceiveBuffer);
            break;
        case 0x0C:
            ReceiveDuelResult(ReceiveBuffer);
            break;
        case 0x0D:
            ReceiveDuelRound(ReceiveBuffer);
            break;
        }
    }
    break;
    case 0xF7:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
        case 0x02:
            ReceiveEnterEmpireGuardianEvent(ReceiveBuffer);
            break;

        case 0x04:
            ReceiveRemainTickEmpireGuardian(ReceiveBuffer);
            break;

        case 0x06:
            ReceiveResultEmpireGuardian(ReceiveBuffer);
            break;
        }
    }
    break;
    case 0x3F:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x00:
            ReceiveCreateShopTitleViewport(ReceiveBuffer);
            break;
        case 0x01:
            ReceiveSetPriceResult(ReceiveBuffer);
            break;
        case 0x02:
            ReceiveCreatePersonalShop(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveDestroyPersonalShop(ReceiveBuffer);
            break;
        case 0x05:
            ReceivePersonalShopItemList(received_span);
            break;
        case 0x06:
            ReceivePurchaseItem(received_span);
            break;
        case 0x08:
            NotifySoldItem(ReceiveBuffer);
            break;
        case 0x10:
            ReceiveShopTitleChange(ReceiveBuffer);
            break;
        case 0x12:
            NotifyClosePersonalShop(ReceiveBuffer);
            break;
        case 0x13:
            ReceiveRefreshItemList(received_span);
            break;
        }
    }
    break;
    case 0xAF:
    {
        auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
        switch (Data->SubCode)
        {
        case 0x01:
            ReceiveMoveToEventMatchResult2(ReceiveBuffer);
            break;
        }
    }
    break;
    case 0xB1:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x00:
            ReceiveChangeMapServerInfo(ReceiveBuffer);
            break;

        case 0x01:
            ReceiveChangeMapServerResult(ReceiveBuffer);
            break;
        }
    }
    break;
    case 0xB2:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x00:
            ReceiveBCStatus(ReceiveBuffer);
            break;
        case 0x01:
            ReceiveBCReg(ReceiveBuffer);
            break;
        case 0x02:
            ReceiveBCGiveUp(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveBCRegInfo(ReceiveBuffer);
            break;
        case 0x04:
            ReceiveBCRegMark(ReceiveBuffer);
            break;
        case 0x05:
            ReceiveBCNPCBuy(ReceiveBuffer);
            break;
        case 0x06:
            ReceiveBCNPCRepair(ReceiveBuffer);
            break;
        case 0x07:
            ReceiveBCNPCUpgrade(ReceiveBuffer);
            break;
        case 0x08:
            ReceiveBCGetTaxInfo(ReceiveBuffer);
            break;
        case 0x09:
            ReceiveBCChangeTaxRate(ReceiveBuffer);
            break;
        case 0x10:
            ReceiveBCWithdraw(ReceiveBuffer);
            break;
        case 0x1A:
            ReceiveTaxInfo(ReceiveBuffer);
            break;
        case 0x1F:
            ReceiveHuntZoneEnter(ReceiveBuffer);
            break;

        case 0x11:
            ReceiveGateState(ReceiveBuffer);
            break;

        case 0x12:
            ReceiveGateOperator(ReceiveBuffer);
            break;

        case 0x13:
            ReceiveGateCurrentState(ReceiveBuffer);
            break;

        case 0x14:
            ReceiveCrownSwitchState(ReceiveBuffer);
            break;

        case 0x15:
            ReceiveCrownRegist(ReceiveBuffer);
            break;

        case 0x16:
            ReceiveCrownState(ReceiveBuffer);
            break;

        case 0x17:
            ReceiveBattleCastleStart(ReceiveBuffer);
            break;

        case 0x18:
            ReceiveBattleCastleProcess(ReceiveBuffer);
            break;

        case 0x19:
            ReceiveBattleCastleRegiment(ReceiveBuffer);
            break;

        case 0x20:
            ReceiveBattleCasleSwitchInfo(ReceiveBuffer);
            break;

        case 0x1B:
            ReceiveMapInfoResult(ReceiveBuffer);
            break;

        case 0x1D:
            ReceiveGuildCommand(ReceiveBuffer);
            break;

        case 0x1E:
            ReceiveMatchTimer(ReceiveBuffer);
            break;
        }
    }
    break;

    case 0xB3:
        ReceiveBCNPCList(ReceiveBuffer);
        break;
    case 0xB4:
        ReceiveBCDeclareGuildList(ReceiveBuffer);
        break;
    case 0xB5:
        ReceiveBCGuildList(ReceiveBuffer);
        break;
    case 0xB6:
        ReceiveGuildMemberLocation(ReceiveBuffer);
        break;
    case 0xBB:
        ReceiveGuildNpcLocation(ReceiveBuffer);
        break;
    case 0xB7:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x00:
            ReceiveCatapultState(ReceiveBuffer);
            break;

        case 0x01:
            ReceiveCatapultFire(ReceiveBuffer);
            break;

        case 0x02:
            ReceiveCatapultFireToMe(ReceiveBuffer);
            break;

        case 0x03:
            //                ReceiveDamageFrmeCatapult ( ReceiveBuffer );
            break;
        }
    }
    break;

    case 0xB8:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x01:
            ReceiveKillCount(ReceiveBuffer);
            break;

        case 0x02:
            break;
        }
    }
    break;
    case 0xB9:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x01:
            ReceiveBuildTime(ReceiveBuffer);
            break;

        case 0x02:
            ReceiveCastleGuildMark(ReceiveBuffer);
            break;

        case 0x03:
            ReceiveCastleHuntZoneInfo(ReceiveBuffer);
            break;

        case 0x05:
            ReceiveCastleHuntZoneResult(ReceiveBuffer);
            break;
        }
    }
    break;

    case 0xBA:
        ReceiveSkillCount(ReceiveBuffer);
        break;
    case 0xBD:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x00:
            ReceiveCrywolfInfo(ReceiveBuffer);
            break;
        case 0x02:
            ReceiveCrywolStateAltarfInfo(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveCrywolfAltarContract(ReceiveBuffer);
            break;
        case 0x04:
            ReceiveCrywolfLifeTime(ReceiveBuffer);
            break;
        case 0x0C:
            ReceiveCrywolfTankerHit(ReceiveBuffer);
            break;
        case 0x05:
            ReceiveCrywolfBossMonsterInfo(ReceiveBuffer);
            break;
        case 0x07:
            ReceiveCrywolfPersonalRank(ReceiveBuffer);
            break;
        case 0x09:
            ReceiveCrywolfBenefitPlusChaosRate(ReceiveBuffer);
            break;
        case 0x08:
            ReceiveCrywolfHeroList(ReceiveBuffer);
            break;
        }
        break;
    }
    case 0xC0:
        ReceiveFriendList(ReceiveBuffer);
        break;
    case 0xC1:
        ReceiveAddFriendResult(ReceiveBuffer);
        break;
    case 0xC2:
        ReceiveRequestAcceptAddFriend(ReceiveBuffer);
        break;
    case 0xC3:
        ReceiveDeleteFriendResult(ReceiveBuffer);
        break;
    case 0xC4:
        ReceiveFriendStateChange(ReceiveBuffer);
        break;
    case 0xC5:
        ReceiveLetterSendResult(ReceiveBuffer);
        break;
    case 0xC6:
        ReceiveLetter(ReceiveBuffer);
        break;
    case 0xC7:
        ReceiveLetterText(received_span, false);
        break;
    case 0xC8:
        ReceiveLetterDeleteResult(ReceiveBuffer);
        break;
    case 0xCA:
        ReceiveCreateChatRoomResult(ReceiveBuffer);
        break;
    case 0xCB:
        ReceiveChatRoomInviteResult(ReceiveBuffer);
        break;
    case 0x2D:
        ReceiveBuffState(ReceiveBuffer);
        break;
    case 0xD1:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        switch (subcode)
        {
        case 0x00:
            ReceiveKanturu3rdStateInfo(ReceiveBuffer);
            break;
        case 0x01:
            ReceiveKanturu3rdEnterBossMap(ReceiveBuffer);
            break;
        case 0x02:
            ReceiveKanturu3rdCurrentState(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveKanturu3rdState(ReceiveBuffer);
            break;
        case 0x04:
            ReceiveKanturu3rdResult(ReceiveBuffer);
            break;
        case 0x05:
            ReceiveKanturu3rdTimer(ReceiveBuffer);
            break;
        case 0x06:
            RecevieKanturu3rdMayaSKill(ReceiveBuffer);
            break;
        case 0x07:
            RecevieKanturu3rdLeftUserandMonsterCount(ReceiveBuffer);
            break;
        case 0x10:
            ReceiveRaklionStateInfo(ReceiveBuffer);
            break;
        case 0x11:
            ReceiveRaklionCurrentState(ReceiveBuffer);
            break;
        case 0x12:
            RecevieRaklionStateChange(ReceiveBuffer);
            break;
        case 0x13:
            RecevieRaklionBattleResult(ReceiveBuffer);
            break;
        case 0x14:
            RecevieRaklionWideAreaAttack(ReceiveBuffer);
            break;
        case 0x15:
            RecevieRaklionUserMonsterCount(ReceiveBuffer);
            break;
        }
        break;
    }
    case 0xBF:
    {
        int subcode;
        if (bIsC1C3)
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else
        {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }

        switch (subcode)
        {
        case 0x00:
            ReceiveCursedTempleEnterResult(ReceiveBuffer);
            break;
        case 0x01:
            ReceiveCursedTempleInfo(ReceiveBuffer);
            break;
        case 0x02:
            ReceiveCursedTempMagicResult(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveCursedTempleEnterInfo(ReceiveBuffer);
            break;
        case 0x04:
            ReceiveCursedTempleGameResult(ReceiveBuffer);
            break;
        case 0x06:
            ReceiveCursedTempSkillPoint(ReceiveBuffer);
            break;
        case 0x07:
            ReceiveCursedTempSkillEnd(ReceiveBuffer);
            break;
        case 0x08:
            ReceiveCursedTempleHolyItemRelics(ReceiveBuffer);
            break;
        case 0x09:
            ReceiveCursedTempleState(ReceiveBuffer);
            break;
        case 0x0a:
            ReceiveChainMagic(ReceiveBuffer);
            break;
        case 0x0B:
            ReceiveRegistedLuckyCoin(ReceiveBuffer);
            break;
        case 0x0C:
            ReceiveRegistLuckyCoin(ReceiveBuffer);
            break;
        case 0x0D:
            ReceiveRequestExChangeLuckyCoin(ReceiveBuffer);
            break;
        case 0x0E:
            ReceiveEnterDoppelGangerEvent(ReceiveBuffer);
            break;
        case 0x0F:
            ReceiveDoppelGangerMonsterPosition(ReceiveBuffer);
            break;
        case 0x10:
            ReceiveDoppelGangerState(ReceiveBuffer);
            break;
        case 0x11:
            ReceiveDoppelGangerIcewalkerState(ReceiveBuffer);
            break;
        case 0x12:
            ReceiveDoppelGangerTimePartyState(ReceiveBuffer);
            break;
        case 0x13:
            ReceiveDoppelGangerResult(ReceiveBuffer);
            break;
        case 0x14:
            ReceiveDoppelGangerMonsterGoal(ReceiveBuffer);
            break;
#ifdef PBG_ADD_SECRETBUFF
        case 0x15:
            ReceiveFatigueTime(ReceiveBuffer);
            break;
#endif //PBG_ADD_SECRETBUFF
#ifdef LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        case 0x20:
            ReceiveEquippingInventoryItem(ReceiveBuffer);
            break;
#endif //LJH_ADD_SYSTEM_OF_EQUIPPING_ITEM_FROM_INVENTORY
        case 0x51:
            ReceiveMuHelperStatusUpdate(received_span);
            break;
        }
    }
    break;
    case 0xDE:
    {
        int subcode = 0;

        if (bIsC1C3) {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE)ReceiveBuffer;
            subcode = Data->SubCode;
        }
        else {
            auto Data = (LPPHEADER_DEFAULT_SUBCODE_WORD)ReceiveBuffer;
            subcode = Data->SubCode;
        }

        switch (subcode)
        {
        case 0x00: ReceiveCharacterCard_New(ReceiveBuffer);
            break;
        }
    }
    break;

#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
    case 0xD2:
    {
        auto* Data = (PBMSG_HEADER2*)ReceiveBuffer;
        switch (Data->m_bySubCode)
        {
        case 0x01:
            ReceiveIGS_CashPoint(ReceiveBuffer);
            break;
        case 0x02:
            ReceiveIGS_ShopOpenResult(ReceiveBuffer);
            break;
        case 0x03:
            ReceiveIGS_BuyItem(ReceiveBuffer);
            break;
        case 0x04:
            ReceiveIGS_SendItemGift(ReceiveBuffer);
            break;
        case 0x06:
            ReceiveIGS_StorageItemListCount(ReceiveBuffer);
            break;
        case 0x07:
            ReceiveIGS_SendCashGift(ReceiveBuffer);
            break;
        case 0x08:
            ReceiveIGS_PossibleBuy(ReceiveBuffer);
            break;
        case 0x09:
            ReceiveIGS_LeftCountItem(ReceiveBuffer);
            break;
        case 0x0B:
            ReceiveIGS_UseStorageItem(ReceiveBuffer);
            break;
        case 0x0C:
            ReceiveIGS_UpdateScript(ReceiveBuffer);
            break;
        case 0x0D:
            ReceiveIGS_StorageItemList(ReceiveBuffer);
            break;
        case 0x0E:
            ReceiveIGS_StorageGiftItemList(ReceiveBuffer);
            break;
        case 0x13:
            ReceiveIGS_EventItemlistCnt(ReceiveBuffer);
            break;
        case 0x14:
            ReceiveIGS_EventItemlist(ReceiveBuffer);
            break;
        case 0x15:
            ReceiveIGS_UpdateBanner(ReceiveBuffer);
            break;
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
        case 0x11:
            ReceivePeriodItemListCount(ReceiveBuffer);
            break;
        case 0x12:
            ReceivePeriodItemList(ReceiveBuffer);
            break;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
        }
    }break;

#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM
    case 0x4A:
        ReceiveStraightAttack(ReceiveBuffer, Size, bEncrypted);
        break;
    case 0x4B:
        ReceiveDarkside(ReceiveBuffer);
        break;
    case 0xAE:
        // Received mu helper config from server
        ReceiveMuHelperConfigurationData(received_span);
        break;
    default:
        break;
    }

    //return ( TRUE);
}

void ProcessPacketCallback(const PacketInfo* Packet)
{
    try
    {
        ProcessPacket(Packet->ReceiveBuffer.get(), Packet->Size);
    }
    catch (const std::exception&)
    {
    }
}

static void HandleIncomingPacket(int32_t Handle, const BYTE* ReceiveBuffer, int32_t Size)
{
    auto Packet = std::make_unique<PacketInfo>();
    Packet->ReceiveBuffer = std::make_unique<BYTE[]>(Size);
    std::copy(ReceiveBuffer, ReceiveBuffer + Size, Packet->ReceiveBuffer.get());
    Packet->Size = Size;

    PostMessage(g_hWnd, WM_RECEIVE_BUFFER, reinterpret_cast<WPARAM>(Packet.release()), 0);
}

bool CheckExceptionBuff(eBuffState buff, OBJECT* o, bool iserase)
{
    if (iserase)
    {
        switch (buff)
        {
        case eBuff_SoulPotion:
        {
            if (o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
            {
                g_CharacterUnRegisterBuff(o, eBuff_CrywolfAltarEnable);
                return false;
            }
        }
        break;

        case eBuff_RemovalMagic:
        {
            std::list<eBuffState> bufflist;

            //debuff
            bufflist.push_back(eDeBuff_Poison);
            bufflist.push_back(eDeBuff_Freeze);
            bufflist.push_back(eDeBuff_Harden);
            bufflist.push_back(eDeBuff_Defense);
            bufflist.push_back(eDeBuff_Stun);
            bufflist.push_back(eDeBuff_Sleep);
            bufflist.push_back(eDeBuff_BlowOfDestruction);

            //buff
            bufflist.push_back(eBuff_Life); bufflist.push_back(eBuff_Attack);
            bufflist.push_back(eBuff_Defense); bufflist.push_back(eBuff_AddAG);
            bufflist.push_back(eBuff_Cloaking); bufflist.push_back(eBuff_AddSkill);
            bufflist.push_back(eBuff_WizDefense); bufflist.push_back(eBuff_AddCriticalDamage);
            bufflist.push_back(eBuff_CrywolfAltarOccufied);

            g_CharacterUnRegisterBuffList(o, bufflist);
        }
        break;
        }

        return true;
    }
    
    switch (buff)
        {
        case eBuff_CastleRegimentAttack3:
        {
            g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);
        }
        break;
        case eBuff_SoulPotion:
        {
            if (o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
            {
                g_CharacterRegisterBuff(o, eBuff_CrywolfAltarEnable);
                return false;
            }
        }
        break;
        case eBuff_CastleGateIsOpen:
        {
            if (o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
            {
                g_CharacterRegisterBuff(o, eBuff_CrywolfAltarDisable);
                return false;
            }
        }
        break;
        case eBuff_CastleRegimentDefense:
        {
            if (o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
            {
                g_CharacterRegisterBuff(o, eBuff_CrywolfAltarContracted);
                return false;
            }
            else
            {
                g_TokenCharacterBuff(o, eBuff_CastleRegimentDefense);
                return false;
            }
        }
        break;
        case eBuff_CastleRegimentAttack1:
        {
            if (o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
            {
                g_CharacterRegisterBuff(o, eBuff_CrywolfAltarAttempt);
                return false;
            }
            else if (g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
            {
                g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);
            }
        }
        break;
        case eBuff_RemovalMagic:
        {
            if (o->Type >= MODEL_CRYWOLF_ALTAR1 && o->Type <= MODEL_CRYWOLF_ALTAR5)
            {
                g_CharacterRegisterBuff(o, eBuff_CrywolfAltarOccufied);
                return false;
            }
        }
        break;
        case eBuff_CastleRegimentAttack2:
        {
            if (g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);
        }
        break;
        }
        return true;
}

void InsertBuffLogicalEffect(eBuffState buff, OBJECT* o, const int bufftime)
{
    if (o && o == &Hero->Object)
    {
        switch (buff)
        {
        case eBuff_Hellowin1:
        case eBuff_Hellowin2:
        case eBuff_Hellowin3:
        case eBuff_Hellowin4:
        case eBuff_Hellowin5:
        {
            g_RegisterBuffTime(buff, bufftime);

            if (buff == eBuff_Hellowin2)
            {
                CharacterMachine->CalculateDamage();
            }
            if (buff == eBuff_Hellowin3)
            {
                CharacterMachine->CalculateDefense();
            }
        }
        break;
        case eBuff_PcRoomSeal1:
        case eBuff_PcRoomSeal2:
        case eBuff_PcRoomSeal3:
        {
            g_RegisterBuffTime(buff, bufftime);
        }
        break;
        case eBuff_Seal1:
        case eBuff_Seal2:
        case eBuff_Seal3:
        case eBuff_Seal4:
        case eBuff_Seal_HpRecovery:
        case eBuff_Seal_MpRecovery:
        case eBuff_GuardCharm:
        case eBuff_ItemGuardCharm:
        case eBuff_AscensionSealMaster:
        case eBuff_WealthSealMaster:
        case eBuff_NewWealthSeal:
        case eBuff_AG_Addition:
        case eBuff_SD_Addition:
        case eBuff_PartyExpBonus:
        {
            g_RegisterBuffTime(buff, bufftime);
        }
        break;
        case eBuff_EliteScroll1:
        case eBuff_EliteScroll2:
        case eBuff_EliteScroll3:
        case eBuff_EliteScroll4:
        case eBuff_EliteScroll5:
        case eBuff_EliteScroll6:
        case eBuff_Scroll_Battle:
        case eBuff_Scroll_Strengthen:
        case eBuff_Scroll_Healing:
        {
            g_RegisterBuffTime(buff, bufftime);

            if (buff == eBuff_EliteScroll2)
            {
                CharacterMachine->CalculateDefense();
            }
            else if (buff == eBuff_EliteScroll3)
            {
                CharacterMachine->CalculateDamage();
            }
            else if (buff == eBuff_EliteScroll4)
            {
                CharacterMachine->CalculateMagicDamage();
            }
        }
        break;
        case eBuff_SecretPotion1:
        case eBuff_SecretPotion2:
        case eBuff_SecretPotion3:
        case eBuff_SecretPotion4:
        case eBuff_SecretPotion5:
        {
            g_RegisterBuffTime(buff, bufftime);

            CharacterMachine->CalculateAll();
        }
        break;
        case eBuff_CherryBlossom_Liguor:
        case eBuff_CherryBlossom_RiceCake:
        case eBuff_CherryBlossom_Petal:
        {
            g_RegisterBuffTime(buff, bufftime);

            if (buff == eBuff_CherryBlossom_Petal)
            {
                CharacterMachine->CalculateDamage();
            }
        }
        break;
        case eBuff_SwellOfMagicPower:
        {
            g_RegisterBuffTime(buff, bufftime);

            if (buff == eBuff_SwellOfMagicPower)
            {
                CharacterMachine->CalculateMagicDamage();
            }
        }break;
        case eBuff_BlessingOfXmax:
        case eBuff_CureOfSanta:
        case eBuff_SafeGuardOfSanta:
        case eBuff_StrengthOfSanta:
        case eBuff_DefenseOfSanta:
        case eBuff_QuickOfSanta:
        case eBuff_LuckOfSanta:
        {
            g_RegisterBuffTime(buff, bufftime);

            wchar_t _Temp[64] = { 0, };

            if (buff == eBuff_BlessingOfXmax)
            {
                g_pSystemLogBox->AddText(GlobalText[2591], SEASON3B::TYPE_SYSTEM_MESSAGE);
                CharacterMachine->CalculateDamage();
                CharacterMachine->CalculateDefense();
            }
            else if (buff == eBuff_StrengthOfSanta)
            {
                swprintf(_Temp, GlobalText[2594], 30);
                g_pSystemLogBox->AddText(_Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);

                CharacterMachine->CalculateDamage();
            }
            else if (buff == eBuff_DefenseOfSanta)
            {
                swprintf(_Temp, GlobalText[2595], 100);
                g_pSystemLogBox->AddText(_Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);

                CharacterMachine->CalculateDefense();
            }
            else if (buff == eBuff_QuickOfSanta)
            {
                swprintf(_Temp, GlobalText[2598], 15);
                g_pSystemLogBox->AddText(_Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
            else if (buff == eBuff_LuckOfSanta)
            {
                swprintf(_Temp, GlobalText[2599], 10);
                g_pSystemLogBox->AddText(_Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
            else if (buff == eBuff_CureOfSanta)
            {
                swprintf(_Temp, GlobalText[2592], 500);
                g_pSystemLogBox->AddText(_Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
            else if (buff == eBuff_SafeGuardOfSanta)
            {
                swprintf(_Temp, GlobalText[2593], 500);
                g_pSystemLogBox->AddText(_Temp, SEASON3B::TYPE_SYSTEM_MESSAGE);
            }
        }
        break;
        case eBuff_DuelWatch:
        {
            g_pNewUISystem->HideAll();
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MAINFRAME);
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_BUFF_WINDOW);
            g_pNewUISystem->Show(SEASON3B::INTERFACE_DUELWATCH_MAINFRAME);
            g_pNewUISystem->Show(SEASON3B::INTERFACE_DUELWATCH_USERLIST);
        }
        break;
        case eBuff_HonorOfGladiator:
        {
            g_RegisterBuffTime(buff, bufftime);
        }
        break;
        }
    }
}

void ClearBuffLogicalEffect(eBuffState buff, OBJECT* o)
{
    if (o && o == &Hero->Object)
    {
        switch (buff)
        {
        case eBuff_Hellowin1:
        case eBuff_Hellowin2:
        case eBuff_Hellowin3:
        case eBuff_Hellowin4:
        case eBuff_Hellowin5:
        {
            g_UnRegisterBuffTime(buff);

            if (buff == eBuff_Hellowin2)
            {
                int iBaseClass = gCharacterManager.GetBaseClass(Hero->Class);

                if (iBaseClass == CLASS_WIZARD || iBaseClass == CLASS_DARK || iBaseClass == CLASS_SUMMONER)
                {
                    CharacterMachine->CalculateMagicDamage();
                }
                else
                {
                    CharacterMachine->CalculateDamage();
                }
            }
            else if (buff == eBuff_Hellowin3)
            {
                CharacterMachine->CalculateDefense();
            }
        }
        break;
        case eBuff_PcRoomSeal1:
        case eBuff_PcRoomSeal2:
        case eBuff_PcRoomSeal3:
        {
            g_UnRegisterBuffTime(buff);
        }
        break;
        case eBuff_Seal1:
        case eBuff_Seal2:
        case eBuff_Seal3:
        case eBuff_Seal4:
        case eBuff_Seal_HpRecovery:
        case eBuff_Seal_MpRecovery:
        case eBuff_GuardCharm:
        case eBuff_ItemGuardCharm:
        case eBuff_AscensionSealMaster:
        case eBuff_WealthSealMaster:
        case eBuff_NewWealthSeal:
        case eBuff_AG_Addition:
        case eBuff_SD_Addition:
        case eBuff_PartyExpBonus:
        {
            g_UnRegisterBuffTime(buff);
        }
        break;
        case eBuff_EliteScroll1:
        case eBuff_EliteScroll2:
        case eBuff_EliteScroll3:
        case eBuff_EliteScroll4:
        case eBuff_EliteScroll5:
        case eBuff_EliteScroll6:
        case eBuff_Scroll_Battle:
        case eBuff_Scroll_Strengthen:
        case eBuff_Scroll_Healing:
        {
            g_UnRegisterBuffTime(buff);

            if (buff == eBuff_EliteScroll2)
            {
                CharacterMachine->CalculateDefense();
            }
            else if (buff == eBuff_EliteScroll3)
            {
                CharacterMachine->CalculateDamage();
            }
            else if (buff == eBuff_EliteScroll4)
            {
                CharacterMachine->CalculateMagicDamage();
            }
        }
        break;
        case eBuff_SecretPotion1:
        case eBuff_SecretPotion2:
        case eBuff_SecretPotion3:
        case eBuff_SecretPotion4:
        case eBuff_SecretPotion5:
        {
            g_UnRegisterBuffTime(buff);

            CharacterMachine->CalculateAll();
        }
        break;
        case eBuff_CherryBlossom_Liguor:
        case eBuff_CherryBlossom_RiceCake:
        case eBuff_CherryBlossom_Petal:
        {
            g_UnRegisterBuffTime(buff);

            if (buff == eBuff_CherryBlossom_Petal)
            {
                CharacterMachine->CalculateDamage();
            }
        }
        break;
        case eBuff_SwellOfMagicPower:
        {
            g_UnRegisterBuffTime(buff);

            CharacterMachine->CalculateMagicDamage();
        }
        break;
        case eBuff_BlessingOfXmax:
        case eBuff_CureOfSanta:
        case eBuff_SafeGuardOfSanta:
        case eBuff_StrengthOfSanta:
        case eBuff_DefenseOfSanta:
        case eBuff_QuickOfSanta:
        case eBuff_LuckOfSanta:
        {
            g_UnRegisterBuffTime(buff);

            if (buff == eBuff_BlessingOfXmax)
            {
                CharacterMachine->CalculateDamage();
                CharacterMachine->CalculateDefense();
            }
            else if (buff == eBuff_StrengthOfSanta)
            {
                CharacterMachine->CalculateDamage();
            }
            else if (buff == eBuff_DefenseOfSanta)
            {
                CharacterMachine->CalculateDefense();
            }
        }
        break;
        case eBuff_DuelWatch:
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUELWATCH_MAINFRAME);
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUELWATCH_USERLIST);
            g_pNewUISystem->Show(SEASON3B::INTERFACE_MAINFRAME);
            g_pNewUISystem->Show(SEASON3B::INTERFACE_BUFF_WINDOW);
        }
        break;
        case eBuff_HonorOfGladiator:
        {
            g_UnRegisterBuffTime(buff);
        }
        break;
        }
    }
}

void InsertBuffPhysicalEffect(eBuffState buff, OBJECT* o)
{
    switch (buff)
    {
    case eBuff_CursedTempleProdection:
    {
        if (o->Type == MODEL_PLAYER)
        {
            DeleteEffect(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o);
            DeleteEffect(MODEL_SHIELD_CRASH, o);
            DeleteEffect(BITMAP_SHOCK_WAVE, o);
            vec3_t  Light;
            Vector(0.3f, 0.3f, 0.8f, Light);
            CreateEffect(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o->Position, o->Angle, Light, 0, o);
            CreateEffect(MODEL_SHIELD_CRASH, o->Position, o->Angle, Light, 1, o);
            CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 10, o);
        }
    }
    break;
    case eBuff_WizDefense:
    {
        if (o->Type == MODEL_PLAYER)
        {
            PlayBuffer(SOUND_SOULBARRIER);
            DeleteJoint(MODEL_SPEARSKILL, o, 0);
            for (int j = 0; j < 5; ++j)
            {
                CreateJoint(MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 0, o, 50.0f);
            }
        }
    }
    break;
    case eBuff_AddAG:
    {
        DeleteEffect(BITMAP_LIGHT, o, 2);
        CreateEffect(BITMAP_LIGHT, o->Position, o->Angle, o->Light, 2, o);
    }
    break;

    case eBuff_Life:
    {
        DeleteEffect(BITMAP_LIGHT, o, 1);
        CreateEffect(BITMAP_LIGHT, o->Position, o->Angle, o->Light, 1, o);
    }
    break;
    case eDeBuff_Harden:
    {
        vec3_t Angle;
        VectorCopy(o->Angle, Angle);

        DeleteEffect(MODEL_ICE, o, 1);
        CreateEffect(MODEL_ICE, o->Position, Angle, o->Light, 1, o);

        Angle[2] += 180.f;
        CreateEffect(MODEL_ICE, o->Position, Angle, o->Light, 2, o);
    }
    break;
    case eDeBuff_Defense:
    {
        vec3_t Light = { 1.0f, 1.f, 1.f };
        DeleteEffect(BITMAP_SKULL, o, 0);
        CreateEffect(BITMAP_SKULL, o->Position, o->Angle, Light, 0, o);

        PlayBuffer(SOUND_BLOODATTACK, o);
    }
    break;
    case eDeBuff_Stun:
    {
        DeleteEffect(BITMAP_SKULL, o, 5);

        vec3_t Angle;
        vec3_t Position;
        VectorCopy(o->Position, Position);
        Vector(0.f, 0.f, 90.f, Angle);
        CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 8, o, 30.0f);
        Position[2] -= 10.f;
        Vector(0.f, 0.f, 180.f, Angle);
        CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 8, o, 30.0f);
        Position[2] -= 10.f;
        Vector(0.f, 0.f, 270.f, Angle);
        CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 8, o, 30.0f);
    }
    break;
    case eDeBuff_NeilDOT:
    {
        g_SummonSystem.CreateDamageOfTimeEffect(AT_SKILL_SUMMON_REQUIEM, o);
    }
    break;
    case eDeBuff_SahamuttDOT:
    {
        g_SummonSystem.CreateDamageOfTimeEffect(AT_SKILL_SUMMON_EXPLOSION, o);
    }
    break;
    case eBuff_Thorns:
    {
        vec3_t vLight;
        Vector(0.9f, 0.6f, 0.1f, vLight);
        CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 43, o, 50.f, 0, 0, 0, 0, vLight);
    }
    break;
    case eDeBuff_Blind:
    {
        vec3_t vLight;
        Vector(1.0f, 1.0f, 1.0f, vLight);
        CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o->Position, o->Angle, vLight, 3, o);
    }
    break;
    case eDeBuff_Sleep:
    {
        vec3_t vLight;
        Vector(0.7f, 0.1f, 0.9f, vLight);
        CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o->Position, o->Angle, vLight, 4, o);
    }
    break;
    case eDeBuff_AttackDown:
    {
        vec3_t vLight;
        Vector(1.4f, 0.2f, 0.2f, vLight);
        CreateEffect(BITMAP_SHINY + 6, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 0.5f);
        CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 1.f);
    }
    break;

    case eDeBuff_DefenseDown:
    {
        vec3_t vLight;
        Vector(0.25f, 1.0f, 0.7f, vLight);
        CreateEffect(BITMAP_SHINY + 6, o->Position, o->Angle, vLight, 2, o, -1, 0, 0, 0, 0.5f);
        CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, vLight, 2, o, -1, 0, 0, 0, 1.f);
    }
    break;
    case eBuff_Berserker:
    {
        vec3_t vLight[2];
        Vector(0.9f, 0.0f, 0.1f, vLight[0]);
        Vector(1.0f, 1.0f, 1.0f, vLight[1]);
        for (int i = 0; i < 4; ++i)
        {
            DeleteEffect(BITMAP_ORORA, o, i);
            CreateEffect(BITMAP_ORORA, o->Position, o->Angle, vLight[0], i, o);
            if (i == 2 || i == 3)
            {
                DeleteEffect(BITMAP_SPARK + 2, o, i);
                CreateEffect(BITMAP_SPARK + 2, o->Position, o->Angle, vLight[1], i, o);
            }
        }
        DeleteEffect(BITMAP_LIGHT_MARKS, o);
        CreateEffect(BITMAP_LIGHT_MARKS, o->Position, o->Angle, vLight[0], 0, o);
    }
    break;
    case eDeBuff_FlameStrikeDamage:
    {
        CreateEffect(BITMAP_FIRE_CURSEDLICH, o->Position, o->Angle, o->Light, 2, o);
    }
    break;
    case eDeBuff_GiganticStormDamage:
    {
        DeleteEffect(BITMAP_JOINT_THUNDER, o, 0);
        CreateEffect(BITMAP_JOINT_THUNDER, o->Position, o->Angle, o->Light, 0, o);
    }
    break;
    case eDeBuff_LightningShockDamage:
    {
        CreateEffect(MODEL_LIGHTNING_SHOCK, o->Position, o->Angle, o->Light, 2, o);
    }
    break;
    case eDeBuff_BlowOfDestruction:
    {
        CreateEffect(MODEL_ICE, o->Position, o->Angle, o->Light);
    }
    break;
    case eBuff_Doppelganger_Ascension:
    {
        PlayBuffer(SOUND_SKILL_BERSERKER);
        CreateEffect(MODEL_CHANGE_UP_EFF, o->Position, o->Angle, o->Light, 2, o);
        CreateEffect(MODEL_CHANGE_UP_CYLINDER, o->Position, o->Angle, o->Light, 2, o);
        CreateEffect(MODEL_INFINITY_ARROW3, o->Position, o->Angle, o->Light, 1, o);
    }
    break;
    case eBuff_SD_Addition:
    {
        DeleteEffect(MODEL_EFFECT_SD_AURA, o);
        CreateEffect(MODEL_EFFECT_SD_AURA, o->Position, o->Angle, o->Light, 0, o);
    }
    break;
    case eBuff_AG_Addition:
    {
        DeleteParticle(BITMAP_AG_ADDITION_EFFECT);

        for (int i = 0; i < 5; ++i)
        {
            CreateParticle(BITMAP_AG_ADDITION_EFFECT, o->Position, o->Angle, o->Light, 0, 1.0f, o);
            CreateParticle(BITMAP_AG_ADDITION_EFFECT, o->Position, o->Angle, o->Light, 1, 1.0f, o);
            CreateParticle(BITMAP_AG_ADDITION_EFFECT, o->Position, o->Angle, o->Light, 2, 1.0f, o);
        }
    }
    break;
    case eBuff_Att_up_Ourforces:
    {
        DeleteEffect(BITMAP_LIGHT_RED, o, 1);
        CreateEffect(BITMAP_LIGHT_RED, o->Position, o->Angle, o->Light, 1, o, -1, 0, 0, 0, 1.5f);
    }
    break;
    case eBuff_Hp_up_Ourforces:
    {
        DeleteEffect(BITMAP_LIGHT_RED, o, 0);
        CreateEffect(BITMAP_LIGHT_RED, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, 1.5f);
    }
    break;
    case eBuff_Def_up_Ourforces:
    {
        DeleteEffect(BITMAP_LIGHT_RED, o, 2);
        CreateEffect(BITMAP_LIGHT_RED, o->Position, o->Angle, o->Light, 2, o, -1, 0, 0, 0, 1.5f);
    }
    break;
    }
}

void ClearBuffPhysicalEffect(eBuffState buff, OBJECT* o)
{
    switch (buff)
    {
    case eBuff_CursedTempleProdection:
    {
        if (g_isCharacterBuff(o, eBuff_CursedTempleProdection))
        {
            DeleteEffect(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, o);
        }
    }
    break;

    case eBuff_Defense:
    {
        DeleteJoint(MODEL_SPEARSKILL, o, 4);
        DeleteJoint(MODEL_SPEARSKILL, o, 9);
    }
    break;

    case eBuff_AddAG:
    {
        DeleteEffect(BITMAP_LIGHT, o, 2);
    }
    break;

    case eBuff_Life:
    {
        DeleteEffect(BITMAP_LIGHT, o, 1);
    }
    break;

    case eBuff_WizDefense:
    {
        if (o->Type == MODEL_PLAYER)
        {
            DeleteJoint(MODEL_SPEARSKILL, o, 0);
        }
    }
    break;

    case eBuff_HelpNpc:
    {
        DeleteJoint(MODEL_SPEARSKILL, o, 4);
        DeleteJoint(MODEL_SPEARSKILL, o, 9);
    }
    break;

    case eBuff_RemovalMagic:
    {
        DeleteEffect(BITMAP_LIGHT, o, 2);
        DeleteEffect(BITMAP_LIGHT, o, 1);
        DeleteEffect(BITMAP_SKULL, o, 0);
        DeleteJoint(MODEL_SPEARSKILL, o, 4);
        DeleteJoint(MODEL_SPEARSKILL, o, 9);
        DeleteJoint(MODEL_SPEARSKILL, o, 0);
    }
    break;
    case eDeBuff_CursedTempleRestraint:
    {
        if (g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint))
        {
            DeleteEffect(MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, o);
        }
    }
    break;

    case eDeBuff_Defense:
    {
        DeleteEffect(BITMAP_SKULL, o, 0);
    }
    break;

    case eDeBuff_Stun:
    {
        DeleteEffect(BITMAP_SKULL, o, 5);
    }
    break;

    case eDeBuff_NeilDOT:
    {
        g_SummonSystem.RemoveDamageOfTimeEffect(AT_SKILL_SUMMON_REQUIEM, o);
    }
    break;

    case eDeBuff_SahamuttDOT:
    {
        g_SummonSystem.RemoveDamageOfTimeEffect(AT_SKILL_SUMMON_EXPLOSION, o);
    }
    break;

    case eBuff_Thorns:
    {
        DeleteJoint(BITMAP_FLARE, o, 43);
    }
    break;

    case eDeBuff_Sleep:
    {
        DeleteEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 4);
    }
    break;

    case eDeBuff_Blind:
    {
        DeleteEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 3);
    }
    break;

    case eDeBuff_AttackDown:
        DeleteEffect(BITMAP_SHINY + 6, o, 1);
        DeleteEffect(BITMAP_PIN_LIGHT, o, 1);
        break;

    case eDeBuff_DefenseDown:
        DeleteEffect(BITMAP_SHINY + 6, o, 2);
        DeleteEffect(BITMAP_PIN_LIGHT, o, 2);
        break;
    case eBuff_Berserker:
    {
        for (int i = 0; i < 4; ++i)
        {
            DeleteEffect(BITMAP_ORORA, o, i);
            if (i == 2 || i == 3)
                DeleteEffect(BITMAP_SPARK + 2, o, i);
        }
        DeleteEffect(BITMAP_LIGHT_MARKS, o);
    }
    break;
    case eDeBuff_FlameStrikeDamage:
    {
        // 			DeleteEffect(BITMAP_FIRE_CURSEDLICH, o, 2);
    }
    break;
    case eDeBuff_GiganticStormDamage:
    {
        // 			DeleteEffect(BITMAP_JOINT_THUNDER, o, 0);
    }
    break;
    case eDeBuff_LightningShockDamage:
    {
        // 			DeleteEffect(MODEL_LIGHTNING_SHOCK, o, 2);
    }
    break;
    case eDeBuff_BlowOfDestruction:
    {
    }
    break;
    case eBuff_SwellOfMagicPower:
    {
        DeleteEffect(MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o, 0);
    }break;
    case eBuff_Doppelganger_Ascension:
    {
        DeleteEffect(BITMAP_JOINT_THUNDER, o, 0);
    }
    break;
    case eBuff_SD_Addition:
    {
        DeleteEffect(MODEL_EFFECT_SD_AURA, o);
    }
    break;
    case eBuff_AG_Addition:
    {
        DeleteParticle(BITMAP_AG_ADDITION_EFFECT);
    }
    break;
    case eBuff_Att_up_Ourforces:
    {
        DeleteEffect(BITMAP_LIGHT_RED, o, 1);
    }
    break;
    case eBuff_Hp_up_Ourforces:
    {
        DeleteEffect(BITMAP_LIGHT_RED, o, 0);
    }
    break;
    case eBuff_Def_up_Ourforces:
    {
        DeleteEffect(BITMAP_LIGHT_RED, o, 2);
    }
    break;
    }
}

void RegisterBuff(eBuffState buff, OBJECT* o, const int bufftime)
{
    eBuffClass buffclasstype = g_IsBuffClass(buff);

    if (buffclasstype == eBuffClass_Count) return;

    if (!o)
    {
        return;
    }

    if (gMapManager.InChaosCastle() && (o && o != &Hero->Object))
    {
        return;
    }

    InsertBuffPhysicalEffect(buff, o);

    if (CheckExceptionBuff(buff, o, false))
    {
        g_CharacterRegisterBuff(o, buff);
        InsertBuffLogicalEffect(buff, o, bufftime);
    }
}

void UnRegisterBuff(eBuffState buff, OBJECT* o)
{
    eBuffClass buffclasstype = g_IsBuffClass(buff);

    if (buffclasstype == eBuffClass_Count) return;

    ClearBuffPhysicalEffect(buff, o);

    if (CheckExceptionBuff(buff, o, true))
    {
        g_CharacterUnRegisterBuff(o, buff);
        ClearBuffLogicalEffect(buff, o);
    }
}