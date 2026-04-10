// Flow Code: VS1-NET-ERROR-MESSAGING
// Story: 3.4.1 - Connection Error Messaging & Graceful Degradation
//
// AC-4 reserved message template (not triggered in this story):
//   "Server handshake failed. Check OpenMU version compatibility."
// AC-5 reserved message template (not triggered in this story):
//   "Login failed. Check credentials."

#include "stdafx.h"
#include <map>

#include "Connection.h"
#include "DotNetMessageFormat.h"

// Full definitions required: Connection.cpp allocates these types (new PacketFunctions_*()).
// Connection.h uses only forward declarations to avoid include-order failures in TUs
// that pull it in transitively through WSclient.h.
#include "PacketFunctions_ChatServer.h"
#include "PacketFunctions_ConnectServer.h"
#include "PacketFunctions_ClientToServer.h"

// MEDIUM-4 fix: g_dotnetLibPath defined here (not in anonymous namespace in header) to prevent
// per-TU copies if Connection.h is ever included by a second translation unit.
// Declaration (extern) remains in Connection.h; only Connection.cpp includes Connection.h today.
#ifdef MU_DOTNET_LIB_DIR
const std::string g_dotnetLibPath =
    (std::filesystem::path(MU_DOTNET_LIB_DIR) / ("MUnique.Client.Library" + std::string(MU_DOTNET_LIB_EXT))).string();
#else
const std::string g_dotnetLibPath =
    (std::filesystem::path("MUnique.Client.Library") += MU_DOTNET_LIB_EXT).string();
#endif

// Defined after g_dotnetLibPath so C++ TU initialization order guarantees the path is
// fully constructed before Load() is called. (SIOF fix — was inline in Connection.h)
const mu::platform::LibraryHandle munique_client_library_handle =
    mu::platform::Load(g_dotnetLibPath.c_str());

// PacketBindings_*.h define inline variables via GetSymbol(munique_client_library_handle, ...).
// Due to SIOF, these may be NULL if the linker picks a TU where the handle isn't initialized yet.
#include "PacketBindings_ChatServer.h"
#include "PacketBindings_ConnectServer.h"
#include "PacketBindings_ClientToServer.h"

// Re-resolve any NULL inline binding variables. Called from MuMain() after static init.
// Uses a template helper to avoid repeating the pattern for all ~200 bindings.
namespace
{
template <typename FnPtr>
void ReResolve(FnPtr& var, const char* name)
{
    if (!var && munique_client_library_handle)
    {
        var = reinterpret_cast<FnPtr>(mu::platform::GetSymbol(munique_client_library_handle, name));
    }
}
} // anonymous namespace

void ResolvePacketBindings()
{
    if (!munique_client_library_handle)
    {
        g_ErrorReport.Write(L"NET: ResolvePacketBindings -- library not loaded, skipping\r\n");
        return;
    }

    // ConnectServer
    ReResolve(dotnet_SendConnectionInfoRequest075, "SendConnectionInfoRequest075");
    ReResolve(dotnet_SendConnectionInfoRequest, "SendConnectionInfoRequest");
    ReResolve(dotnet_SendConnectionInfo, "SendConnectionInfo");
    ReResolve(dotnet_SendServerListRequest, "SendServerListRequest");
    ReResolve(dotnet_SendServerListRequestOld, "SendServerListRequestOld");
    ReResolve(dotnet_SendHello, "SendHello");
    ReResolve(dotnet_SendPatchCheckRequest, "SendPatchCheckRequest");
    ReResolve(dotnet_SendPatchVersionOkay, "SendPatchVersionOkay");
    ReResolve(dotnet_SendClientNeedsPatch, "SendClientNeedsPatch");

    // ChatServer
    ReResolve(dotnet_SendAuthenticate, "SendAuthenticate");
    ReResolve(dotnet_SendChatRoomClientJoined, "SendChatRoomClientJoined");
    ReResolve(dotnet_SendLeaveChatRoom, "SendLeaveChatRoom");
    ReResolve(dotnet_SendChatRoomClientLeft, "SendChatRoomClientLeft");
    ReResolve(dotnet_SendChatMessage, "SendChatMessage");
    ReResolve(dotnet_SendKeepAlive, "SendKeepAlive");

    // ClientToServer — ALL 191 bindings resolved to prevent SIOF null pointers.
    // Generated from: grep "^inline.*dotnet_" PacketBindings_ClientToServer.h
    ReResolve(dotnet_SendPing, "SendPing");
    ReResolve(dotnet_SendChecksumResponse, "SendChecksumResponse");
    ReResolve(dotnet_SendPublicChatMessage, "SendPublicChatMessage");
    ReResolve(dotnet_SendWhisperMessage, "SendWhisperMessage");
    ReResolve(dotnet_SendLoginLongPassword, "SendLoginLongPassword");
    ReResolve(dotnet_SendLoginShortPassword, "SendLoginShortPassword");
    ReResolve(dotnet_SendLogin075, "SendLogin075");
    ReResolve(dotnet_SendLogOut, "SendLogOut");
    ReResolve(dotnet_SendLogOutByCheatDetection, "SendLogOutByCheatDetection");
    ReResolve(dotnet_SendResetCharacterPointRequest, "SendResetCharacterPointRequest");
    ReResolve(dotnet_SendPlayerShopSetItemPrice, "SendPlayerShopSetItemPrice");
    ReResolve(dotnet_SendPlayerShopOpen, "SendPlayerShopOpen");
    ReResolve(dotnet_SendPlayerShopClose, "SendPlayerShopClose");
    ReResolve(dotnet_SendPlayerShopItemListRequest, "SendPlayerShopItemListRequest");
    ReResolve(dotnet_SendPlayerShopItemBuyRequest, "SendPlayerShopItemBuyRequest");
    ReResolve(dotnet_SendPlayerShopCloseOther, "SendPlayerShopCloseOther");
    ReResolve(dotnet_SendPickupItemRequest, "SendPickupItemRequest");
    ReResolve(dotnet_SendPickupItemRequest075, "SendPickupItemRequest075");
    ReResolve(dotnet_SendDropItemRequest, "SendDropItemRequest");
    ReResolve(dotnet_SendItemMoveRequest, "SendItemMoveRequest");
    ReResolve(dotnet_SendItemMoveRequestExtended, "SendItemMoveRequestExtended");
    ReResolve(dotnet_SendConsumeItemRequest, "SendConsumeItemRequest");
    ReResolve(dotnet_SendConsumeItemRequest075, "SendConsumeItemRequest075");
    ReResolve(dotnet_SendTalkToNpcRequest, "SendTalkToNpcRequest");
    ReResolve(dotnet_SendCloseNpcRequest, "SendCloseNpcRequest");
    ReResolve(dotnet_SendBuyItemFromNpcRequest, "SendBuyItemFromNpcRequest");
    ReResolve(dotnet_SendSellItemToNpcRequest, "SendSellItemToNpcRequest");
    ReResolve(dotnet_SendRepairItemRequest, "SendRepairItemRequest");
    ReResolve(dotnet_SendWarpCommandRequest, "SendWarpCommandRequest");
    ReResolve(dotnet_SendEnterGateRequest, "SendEnterGateRequest");
    ReResolve(dotnet_SendEnterGateRequest075, "SendEnterGateRequest075");
    ReResolve(dotnet_SendTeleportTarget, "SendTeleportTarget");
    ReResolve(dotnet_SendServerChangeAuthentication, "SendServerChangeAuthentication");
    ReResolve(dotnet_SendCastleSiegeStatusRequest, "SendCastleSiegeStatusRequest");
    ReResolve(dotnet_SendCastleSiegeRegistrationRequest, "SendCastleSiegeRegistrationRequest");
    ReResolve(dotnet_SendCastleSiegeUnregisterRequest, "SendCastleSiegeUnregisterRequest");
    ReResolve(dotnet_SendCastleSiegeRegistrationStateRequest, "SendCastleSiegeRegistrationStateRequest");
    ReResolve(dotnet_SendCastleSiegeMarkRegistration, "SendCastleSiegeMarkRegistration");
    ReResolve(dotnet_SendCastleSiegeDefenseBuyRequest, "SendCastleSiegeDefenseBuyRequest");
    ReResolve(dotnet_SendCastleSiegeDefenseRepairRequest, "SendCastleSiegeDefenseRepairRequest");
    ReResolve(dotnet_SendCastleSiegeDefenseUpgradeRequest, "SendCastleSiegeDefenseUpgradeRequest");
    ReResolve(dotnet_SendCastleSiegeTaxInfoRequest, "SendCastleSiegeTaxInfoRequest");
    ReResolve(dotnet_SendCastleSiegeTaxChangeRequest, "SendCastleSiegeTaxChangeRequest");
    ReResolve(dotnet_SendCastleSiegeTaxMoneyWithdraw, "SendCastleSiegeTaxMoneyWithdraw");
    ReResolve(dotnet_SendToggleCastleGateRequest, "SendToggleCastleGateRequest");
    ReResolve(dotnet_SendCastleGuildCommand, "SendCastleGuildCommand");
    ReResolve(dotnet_SendCastleSiegeHuntingZoneEntranceSetting, "SendCastleSiegeHuntingZoneEntranceSetting");
    ReResolve(dotnet_SendCastleSiegeGateListRequest, "SendCastleSiegeGateListRequest");
    ReResolve(dotnet_SendCastleSiegeStatueListRequest, "SendCastleSiegeStatueListRequest");
    ReResolve(dotnet_SendCastleSiegeRegisteredGuildsListRequest, "SendCastleSiegeRegisteredGuildsListRequest");
    ReResolve(dotnet_SendCastleOwnerListRequest, "SendCastleOwnerListRequest");
    ReResolve(dotnet_SendFireCatapultRequest, "SendFireCatapultRequest");
    ReResolve(dotnet_SendWeaponExplosionRequest, "SendWeaponExplosionRequest");
    ReResolve(dotnet_SendGuildLogoOfCastleOwnerRequest, "SendGuildLogoOfCastleOwnerRequest");
    ReResolve(dotnet_SendCastleSiegeHuntingZoneEnterRequest, "SendCastleSiegeHuntingZoneEnterRequest");
    ReResolve(dotnet_SendCrywolfInfoRequest, "SendCrywolfInfoRequest");
    ReResolve(dotnet_SendCrywolfContractRequest, "SendCrywolfContractRequest");
    ReResolve(dotnet_SendCrywolfChaosRateBenefitRequest, "SendCrywolfChaosRateBenefitRequest");
    ReResolve(dotnet_SendWhiteAngelItemRequest, "SendWhiteAngelItemRequest");
    ReResolve(dotnet_SendEnterOnWerewolfRequest, "SendEnterOnWerewolfRequest");
    ReResolve(dotnet_SendEnterOnGatekeeperRequest, "SendEnterOnGatekeeperRequest");
    ReResolve(dotnet_SendLeoHelperItemRequest, "SendLeoHelperItemRequest");
    ReResolve(dotnet_SendMoveToDeviasBySnowmanRequest, "SendMoveToDeviasBySnowmanRequest");
    ReResolve(dotnet_SendSantaClausItemRequest, "SendSantaClausItemRequest");
    ReResolve(dotnet_SendKanturuInfoRequest, "SendKanturuInfoRequest");
    ReResolve(dotnet_SendKanturuEnterRequest, "SendKanturuEnterRequest");
    ReResolve(dotnet_SendRaklionStateInfoRequest, "SendRaklionStateInfoRequest");
    ReResolve(dotnet_SendCashShopPointInfoRequest, "SendCashShopPointInfoRequest");
    ReResolve(dotnet_SendCashShopOpenState, "SendCashShopOpenState");
    ReResolve(dotnet_SendCashShopItemBuyRequest, "SendCashShopItemBuyRequest");
    ReResolve(dotnet_SendCashShopItemGiftRequest, "SendCashShopItemGiftRequest");
    ReResolve(dotnet_SendCashShopStorageListRequest, "SendCashShopStorageListRequest");
    ReResolve(dotnet_SendCashShopDeleteStorageItemRequest, "SendCashShopDeleteStorageItemRequest");
    ReResolve(dotnet_SendCashShopStorageItemConsumeRequest, "SendCashShopStorageItemConsumeRequest");
    ReResolve(dotnet_SendCashShopEventItemListRequest, "SendCashShopEventItemListRequest");
    ReResolve(dotnet_SendUnlockVault, "SendUnlockVault");
    ReResolve(dotnet_SendSetVaultPin, "SendSetVaultPin");
    ReResolve(dotnet_SendRemoveVaultPin, "SendRemoveVaultPin");
    ReResolve(dotnet_SendVaultClosed, "SendVaultClosed");
    ReResolve(dotnet_SendVaultMoveMoneyRequest, "SendVaultMoveMoneyRequest");
    ReResolve(dotnet_SendLahapJewelMixRequest, "SendLahapJewelMixRequest");
    ReResolve(dotnet_SendPartyListRequest, "SendPartyListRequest");
    ReResolve(dotnet_SendPartyPlayerKickRequest, "SendPartyPlayerKickRequest");
    ReResolve(dotnet_SendPartyInviteRequest, "SendPartyInviteRequest");
    ReResolve(dotnet_SendPartyInviteResponse, "SendPartyInviteResponse");
    ReResolve(dotnet_SendWalkRequest, "SendWalkRequest");
    ReResolve(dotnet_SendWalkRequest075, "SendWalkRequest075");
    ReResolve(dotnet_SendInstantMoveRequest, "SendInstantMoveRequest");
    ReResolve(dotnet_SendAnimationRequest, "SendAnimationRequest");
    ReResolve(dotnet_SendRequestCharacterList, "SendRequestCharacterList");
    ReResolve(dotnet_SendCreateCharacter, "SendCreateCharacter");
    ReResolve(dotnet_SendDeleteCharacter, "SendDeleteCharacter");
    ReResolve(dotnet_SendSelectCharacter, "SendSelectCharacter");
    ReResolve(dotnet_SendFocusCharacter, "SendFocusCharacter");
    ReResolve(dotnet_SendIncreaseCharacterStatPoint, "SendIncreaseCharacterStatPoint");
    ReResolve(dotnet_SendInventoryRequest, "SendInventoryRequest");
    ReResolve(dotnet_SendClientReadyAfterMapChange, "SendClientReadyAfterMapChange");
    ReResolve(dotnet_SendSaveKeyConfiguration, "SendSaveKeyConfiguration");
    ReResolve(dotnet_SendAddMasterSkillPoint, "SendAddMasterSkillPoint");
    ReResolve(dotnet_SendHitRequest, "SendHitRequest");
    ReResolve(dotnet_SendTargetedSkill, "SendTargetedSkill");
    ReResolve(dotnet_SendTargetedSkill075, "SendTargetedSkill075");
    ReResolve(dotnet_SendTargetedSkill095, "SendTargetedSkill095");
    ReResolve(dotnet_SendMagicEffectCancelRequest, "SendMagicEffectCancelRequest");
    ReResolve(dotnet_SendAreaSkill, "SendAreaSkill");
    ReResolve(dotnet_SendAreaSkill075, "SendAreaSkill075");
    ReResolve(dotnet_SendAreaSkill095, "SendAreaSkill095");
    ReResolve(dotnet_SendRageAttackRequest, "SendRageAttackRequest");
    ReResolve(dotnet_SendRageAttackRangeRequest, "SendRageAttackRangeRequest");
    ReResolve(dotnet_SendTradeCancel, "SendTradeCancel");
    ReResolve(dotnet_SendTradeButtonStateChange, "SendTradeButtonStateChange");
    ReResolve(dotnet_SendTradeRequest, "SendTradeRequest");
    ReResolve(dotnet_SendTradeRequestResponse, "SendTradeRequestResponse");
    ReResolve(dotnet_SendSetTradeMoney, "SendSetTradeMoney");
    ReResolve(dotnet_SendLetterDeleteRequest, "SendLetterDeleteRequest");
    ReResolve(dotnet_SendLetterListRequest, "SendLetterListRequest");
    ReResolve(dotnet_SendLetterSendRequest, "SendLetterSendRequest");
    ReResolve(dotnet_SendLetterReadRequest, "SendLetterReadRequest");
    ReResolve(dotnet_SendGuildKickPlayerRequest, "SendGuildKickPlayerRequest");
    ReResolve(dotnet_SendGuildJoinRequest, "SendGuildJoinRequest");
    ReResolve(dotnet_SendGuildJoinResponse, "SendGuildJoinResponse");
    ReResolve(dotnet_SendGuildListRequest, "SendGuildListRequest");
    ReResolve(dotnet_SendGuildCreateRequest, "SendGuildCreateRequest");
    ReResolve(dotnet_SendGuildCreateRequest075, "SendGuildCreateRequest075");
    ReResolve(dotnet_SendGuildMasterAnswer, "SendGuildMasterAnswer");
    ReResolve(dotnet_SendCancelGuildCreation, "SendCancelGuildCreation");
    ReResolve(dotnet_SendGuildWarResponse, "SendGuildWarResponse");
    ReResolve(dotnet_SendGuildInfoRequest, "SendGuildInfoRequest");
    ReResolve(dotnet_SendGuildRoleAssignRequest, "SendGuildRoleAssignRequest");
    ReResolve(dotnet_SendGuildTypeChangeRequest, "SendGuildTypeChangeRequest");
    ReResolve(dotnet_SendGuildRelationshipChangeRequest, "SendGuildRelationshipChangeRequest");
    ReResolve(dotnet_SendGuildRelationshipChangeResponse, "SendGuildRelationshipChangeResponse");
    ReResolve(dotnet_SendRequestAllianceList, "SendRequestAllianceList");
    ReResolve(dotnet_SendRemoveAllianceGuildRequest, "SendRemoveAllianceGuildRequest");
    ReResolve(dotnet_SendPingResponse, "SendPingResponse");
    ReResolve(dotnet_SendItemRepair, "SendItemRepair");
    ReResolve(dotnet_SendChaosMachineMixRequest, "SendChaosMachineMixRequest");
    ReResolve(dotnet_SendCraftingDialogCloseRequest, "SendCraftingDialogCloseRequest");
    ReResolve(dotnet_SendFriendListRequest, "SendFriendListRequest");
    ReResolve(dotnet_SendFriendAddRequest, "SendFriendAddRequest");
    ReResolve(dotnet_SendFriendDelete, "SendFriendDelete");
    ReResolve(dotnet_SendChatRoomCreateRequest, "SendChatRoomCreateRequest");
    ReResolve(dotnet_SendFriendAddResponse, "SendFriendAddResponse");
    ReResolve(dotnet_SendSetFriendOnlineState, "SendSetFriendOnlineState");
    ReResolve(dotnet_SendChatRoomInvitationRequest, "SendChatRoomInvitationRequest");
    ReResolve(dotnet_SendLegacyQuestStateRequest, "SendLegacyQuestStateRequest");
    ReResolve(dotnet_SendLegacyQuestStateSetRequest, "SendLegacyQuestStateSetRequest");
    ReResolve(dotnet_SendPetCommandRequest, "SendPetCommandRequest");
    ReResolve(dotnet_SendPetInfoRequest, "SendPetInfoRequest");
    ReResolve(dotnet_SendIllusionTempleEnterRequest, "SendIllusionTempleEnterRequest");
    ReResolve(dotnet_SendIllusionTempleSkillRequest, "SendIllusionTempleSkillRequest");
    ReResolve(dotnet_SendIllusionTempleRewardRequest, "SendIllusionTempleRewardRequest");
    ReResolve(dotnet_SendLuckyCoinCountRequest, "SendLuckyCoinCountRequest");
    ReResolve(dotnet_SendLuckyCoinRegistrationRequest, "SendLuckyCoinRegistrationRequest");
    ReResolve(dotnet_SendLuckyCoinExchangeRequest, "SendLuckyCoinExchangeRequest");
    ReResolve(dotnet_SendDoppelgangerEnterRequest, "SendDoppelgangerEnterRequest");
    ReResolve(dotnet_SendEnterMarketPlaceRequest, "SendEnterMarketPlaceRequest");
    ReResolve(dotnet_SendMuHelperStatusChangeRequest, "SendMuHelperStatusChangeRequest");
    ReResolve(dotnet_SendMuHelperSaveDataRequest, "SendMuHelperSaveDataRequest");
    ReResolve(dotnet_SendQuestSelectRequest, "SendQuestSelectRequest");
    ReResolve(dotnet_SendQuestProceedRequest, "SendQuestProceedRequest");
    ReResolve(dotnet_SendQuestCompletionRequest, "SendQuestCompletionRequest");
    ReResolve(dotnet_SendQuestCancelRequest, "SendQuestCancelRequest");
    ReResolve(dotnet_SendQuestClientActionRequest, "SendQuestClientActionRequest");
    ReResolve(dotnet_SendActiveQuestListRequest, "SendActiveQuestListRequest");
    ReResolve(dotnet_SendQuestStateRequest, "SendQuestStateRequest");
    ReResolve(dotnet_SendEventQuestStateListRequest, "SendEventQuestStateListRequest");
    ReResolve(dotnet_SendAvailableQuestsRequest, "SendAvailableQuestsRequest");
    ReResolve(dotnet_SendNpcBuffRequest, "SendNpcBuffRequest");
    ReResolve(dotnet_SendEnterEmpireGuardianEvent, "SendEnterEmpireGuardianEvent");
    ReResolve(dotnet_SendGensJoinRequest, "SendGensJoinRequest");
    ReResolve(dotnet_SendGensLeaveRequest, "SendGensLeaveRequest");
    ReResolve(dotnet_SendGensRewardRequest, "SendGensRewardRequest");
    ReResolve(dotnet_SendGensRankingRequest, "SendGensRankingRequest");
    ReResolve(dotnet_SendDevilSquareEnterRequest, "SendDevilSquareEnterRequest");
    ReResolve(dotnet_SendMiniGameOpeningStateRequest, "SendMiniGameOpeningStateRequest");
    ReResolve(dotnet_SendEventChipRegistrationRequest, "SendEventChipRegistrationRequest");
    ReResolve(dotnet_SendMutoNumberRequest, "SendMutoNumberRequest");
    ReResolve(dotnet_SendEventChipExitDialog, "SendEventChipExitDialog");
    ReResolve(dotnet_SendEventChipExchangeRequest, "SendEventChipExchangeRequest");
    ReResolve(dotnet_SendServerImmigrationRequest, "SendServerImmigrationRequest");
    ReResolve(dotnet_SendLuckyNumberRequest, "SendLuckyNumberRequest");
    ReResolve(dotnet_SendBloodCastleEnterRequest, "SendBloodCastleEnterRequest");
    ReResolve(dotnet_SendMiniGameEventCountRequest, "SendMiniGameEventCountRequest");
    ReResolve(dotnet_SendChaosCastleEnterRequest, "SendChaosCastleEnterRequest");
    ReResolve(dotnet_SendChaosCastlePositionSet, "SendChaosCastlePositionSet");
    ReResolve(dotnet_SendDuelStartRequest, "SendDuelStartRequest");
    ReResolve(dotnet_SendDuelStartResponse, "SendDuelStartResponse");
    ReResolve(dotnet_SendDuelStopRequest, "SendDuelStopRequest");
    ReResolve(dotnet_SendDuelChannelJoinRequest, "SendDuelChannelJoinRequest");
    ReResolve(dotnet_SendDuelChannelQuitRequest, "SendDuelChannelQuitRequest");

    g_ErrorReport.Write(L"NET: ResolvePacketBindings done (SendServerListRequest=%p)\r\n",
                        reinterpret_cast<void*>(dotnet_SendServerListRequest));
}

std::map<int32_t, Connection*> connections;

namespace DotNetBridge
{
bool g_dotnetErrorDisplayed = false;

// AC-1 + AC-2 + AC-7: Structured error reporting for DotNet bridge failures.
// Distinguishes library-not-found vs symbol-not-found via DotNetErrorKind.
// Writes to BOTH g_ErrorReport (persistent MuError.log) AND MessageBoxW dialog
// (mapped to SDL_ShowSimpleMessageBox via PlatformCompat.h shim).
// AC-STD-NFR-1: Dialog shown at most ONCE per session via g_dotnetErrorDisplayed guard.
// AC-STD-NFR-2: Called from main game thread only (single-threaded game loop).
void ReportDotNetError(const char* detail, DotNetErrorKind kind)
{
    if (g_dotnetErrorDisplayed)
    {
        return;
    }
    g_dotnetErrorDisplayed = true;

    // Determine platform name for AC-1 message (compile-time, impl file only — acceptable
    // per Dev Notes §Key Design Decisions: diagnostic string in implementation file).
    const char* platformName =
#if defined(__linux__)
        "Linux";
#elif defined(__APPLE__)
        "macOS";
#else
        "Windows";
#endif

    std::string msg;
    if (kind == DotNetErrorKind::LibraryNotFound)
    {
        // AC-1: Library not found — include resolved path and platform
        msg = FormatLibraryNotFoundMessage(g_dotnetLibPath, platformName);
    }
    else
    {
        // AC-2: Symbol not found — include function name
        msg = FormatSymbolNotFoundMessage(detail);
    }

    // Write to MuError.log (persistent diagnostic — g_ErrorReport)
    g_ErrorReport.Write(L"NET: %hs\r\n", msg.c_str());

    // Show user-visible dialog via MessageBoxW shim → SDL_ShowSimpleMessageBox (PlatformCompat.h)
    // ASCII-safe conversion: diagnostic messages contain only ASCII characters
    std::wstring wideMsg(msg.begin(), msg.end());
    MessageBoxW(nullptr, wideMsg.c_str(), L"Network Error", MB_ICONERROR | MB_OK);
}

bool IsManagedLibraryAvailable()
{
    if (munique_client_library_handle)
    {
        return true;
    }

    // AC-1: Pass library path; ReportDotNetError will use g_dotnetLibPath and platform name
    ReportDotNetError(nullptr, DotNetErrorKind::LibraryNotFound);
    return false;
}
} // namespace DotNetBridge

using DotNetBridge::DotNetErrorKind;
using DotNetBridge::IsManagedLibraryAvailable;
using DotNetBridge::ReportDotNetError;

using onPacketReceived = void(int32_t, int32_t, BYTE*);
using onDisconnected = void(int32_t);

typedef int32_t(CORECLR_DELEGATE_CALLTYPE* Connect)(const char16_t*, int32_t, BYTE, onPacketReceived, onDisconnected);
typedef void(CORECLR_DELEGATE_CALLTYPE* Disconnect)(int32_t);
typedef void(CORECLR_DELEGATE_CALLTYPE* BeginReceive)(int32_t);
typedef void(CORECLR_DELEGATE_CALLTYPE* Send)(int32_t, const BYTE*, int32_t);

Connect dotnet_connect = LoadManagedSymbol<Connect>("ConnectionManager_Connect");

Disconnect dotnet_disconnect = LoadManagedSymbol<Disconnect>("ConnectionManager_Disconnect");

BeginReceive dotnet_beginreceive = LoadManagedSymbol<BeginReceive>("ConnectionManager_BeginReceive");

Send dotnet_send = LoadManagedSymbol<Send>("ConnectionManager_Send");

void Connection::OnPacketReceivedS(const int32_t handle, const int32_t size, BYTE* data)
{
    const auto it = connections.find(handle);
    if (it == connections.end())
    {
        return;
    }

    if (Connection* connection = it->second)
    {
        connection->OnPacketReceived(data, size);
    }
}

void Connection::OnDisconnectedS(const int32_t handle)
{
    const auto it = connections.find(handle);
    if (it == connections.end())
    {
        return;
    }

    if (Connection* connection = it->second)
    {
        connection->OnDisconnected();
    }
}

Connection::Connection(const char16_t* host, int32_t port, bool isEncrypted,
                       void (*packetHandler)(int32_t, const BYTE*, int32_t))
{
    this->_packetHandler = packetHandler;
    if (!dotnet_connect)
    {
        ReportDotNetError("ConnectionManager_Connect", DotNetErrorKind::SymbolNotFound);
        this->_handle = 0;
        return;
    }

    this->_handle = dotnet_connect(host, port, isEncrypted ? 1 : 0, &OnPacketReceivedS, &OnDisconnectedS);

    g_ErrorReport.Write(L"NET: dotnet_connect returned handle=%d (encrypted=%d)\r\n",
                        this->_handle, isEncrypted ? 1 : 0);

    if (IsConnected())
    {
        connections[this->_handle] = this;
        if (dotnet_beginreceive)
        {
            dotnet_beginreceive(this->_handle);
            g_ErrorReport.Write(L"NET: BeginReceive started for handle=%d\r\n", this->_handle);
        }

        // cppcheck-suppress [noCopyConstructor, noOperatorEq]
        _chatServer = new PacketFunctions_ChatServer();
        _connectServer = new PacketFunctions_ConnectServer();
        _gameServer = new PacketFunctions_ClientToServer();

        _chatServer->SetHandle(this->_handle);
        _connectServer->SetHandle(this->_handle);
        _gameServer->SetHandle(this->_handle);
    }
}

Connection::~Connection()
{
    if (!IsConnected())
    {
        return;
    }

    if (dotnet_disconnect)
    {
        dotnet_disconnect(_handle);
    }

    SAFE_DELETE(_chatServer);
    SAFE_DELETE(_connectServer);
    SAFE_DELETE(_gameServer);
}

bool Connection::IsConnected()
{
    return this->_handle > 0;
}

void Connection::Send(const BYTE* data, const int32_t size)
{
    if (!IsConnected())
    {
        return;
    }

    if (!dotnet_send)
    {
        ReportDotNetError("ConnectionManager_Send", DotNetErrorKind::SymbolNotFound);
        return;
    }

    dotnet_send(this->_handle, data, size);
}

void Connection::Close()
{
    if (!IsConnected())
    {
        return;
    }

    if (dotnet_disconnect)
    {
        dotnet_disconnect(this->_handle);
    }
}

void Connection::OnDisconnected()
{
    if (!IsConnected())
    {
        g_ErrorReport.Write(L"NET: OnDisconnected called but already disconnected\r\n");
        return;
    }

    g_ErrorReport.Write(L"NET: OnDisconnected — handle=%d, erasing from connection map\r\n", this->_handle);
    connections.erase(this->_handle);
    this->_handle = 0;
}

void Connection::OnPacketReceived(const BYTE* data, const int32_t size)
{
    // Diagnostic: log first packet to confirm .NET→C++ callback path is working.
    static bool s_firstPacketLogged = false;
    if (!s_firstPacketLogged)
    {
        g_ErrorReport.Write(L"NET: First packet received — handle=%d size=%d (callback path working)\r\n",
                            this->_handle, size);
        s_firstPacketLogged = true;
    }
    this->_packetHandler(this->_handle, data, size);
}