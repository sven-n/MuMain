#include "stdafx.h"
#ifdef MU_TEST_TEMPLE_AOT_TRANSPORT
#include <winsock2.h>
#endif
#include <doctest.h>

#include "RmlLayoutFixture.h"
#ifdef MU_TEST_TEMPLE_AOT_TRANSPORT
#include "Dotnet/Connection.h"
#endif
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Events/w_CursedTemple.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/TextureFailure.h"
#include "Scenes/SceneCore.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Events/CursedTempleSystem.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "World/MapInfra/MapManager.h"

#include <array>
#include <chrono>
#include <memory>
#include <vector>

#ifndef _WIN32
#include <sys/mman.h>
#include <unistd.h>
#endif

extern int TargetX;
extern int TargetY;
extern bool g_bWndActive;

namespace
{
constexpr short HeroId = 512;
constexpr short TeamId = 513;
constexpr short EnemyId = 514;
constexpr int TeamSlot = 1;
constexpr int EnemySlot = MAX_CHARACTERS_CLIENT - 1;
constexpr BYTE SkillRange = 5;
constexpr BYTE SkillCost = 5;
constexpr BYTE SkillBudget = 20;
constexpr int TestTile = 40;
constexpr int TestCharacterLevel = 400;
constexpr int TempleSkillCount = AT_SKILL_CURSED_TEMPLE_SUBLIMATION - AT_SKILL_CURSED_TEMPLE_PRODECTION + 1;
constexpr DWORD MissingSelection = static_cast<DWORD>(-1);

#ifdef MU_TEST_TEMPLE_AOT_TRANSPORT
class SkillPacketPeer
{
public:
    SkillPacketPeer()
    {
        WSADATA data{};
        REQUIRE(WSAStartup(MAKEWORD(2, 2), &data) == 0);
        m_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        REQUIRE(m_listener != INVALID_SOCKET);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        REQUIRE(bind(m_listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
        REQUIRE(listen(m_listener, 1) == 0);
        int size = sizeof(address);
        REQUIRE(getsockname(m_listener, reinterpret_cast<sockaddr*>(&address), &size) == 0);
        port = ntohs(address.sin_port);
    }

    ~SkillPacketPeer()
    {
        if (m_peer != INVALID_SOCKET)
            closesocket(m_peer);
        if (m_listener != INVALID_SOCKET)
            closesocket(m_listener);
        WSACleanup();
    }

    void Accept()
    {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(m_listener, &readable);
        timeval wait{2, 0};
        REQUIRE(select(0, &readable, nullptr, nullptr, &wait) == 1);
        m_peer = accept(m_listener, nullptr, nullptr);
        REQUIRE(m_peer != INVALID_SOCKET);
    }

    std::vector<BYTE> Collect()
    {
        std::vector<BYTE> bytes;
        const auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
        while (std::chrono::steady_clock::now() < until)
        {
            fd_set readable;
            FD_ZERO(&readable);
            FD_SET(m_peer, &readable);
            timeval wait{0, 10000};
            const int ready = select(0, &readable, nullptr, nullptr, &wait);
            REQUIRE(ready != SOCKET_ERROR);
            if (ready == 0)
                continue;
            char buffer[256];
            const int count = recv(m_peer, buffer, sizeof(buffer), 0);
            REQUIRE(count > 0);
            bytes.insert(bytes.end(), buffer, buffer + count);
        }
        return bytes;
    }

    int port = 0;

private:
    SOCKET m_listener = INVALID_SOCKET;
    SOCKET m_peer = INVALID_SOCKET;
};
#endif

// End the real character table at an inaccessible page so one-past-end reads always fault.
class GuardedCharacters
{
public:
    GuardedCharacters()
    {
        const std::size_t bytes = sizeof(CHARACTER) * MAX_CHARACTERS_CLIENT;
#ifdef _WIN32
        SYSTEM_INFO info{};
        GetSystemInfo(&info);
        const std::size_t pageSize = info.dwPageSize;
#else
        const long systemPageSize = sysconf(_SC_PAGESIZE);
        REQUIRE(systemPageSize > 0);
        const std::size_t pageSize = static_cast<std::size_t>(systemPageSize);
#endif
        const std::size_t committed = (bytes + pageSize - 1) / pageSize * pageSize;
        m_storageSize = committed + pageSize;
#ifdef _WIN32
        m_storage = VirtualAlloc(nullptr, m_storageSize, MEM_RESERVE, PAGE_NOACCESS);
        REQUIRE(m_storage != nullptr);
        const bool writable = VirtualAlloc(m_storage, committed, MEM_COMMIT, PAGE_READWRITE) == m_storage;
#else
        m_storage = mmap(nullptr, m_storageSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        REQUIRE(m_storage != MAP_FAILED);
        const bool writable = mprotect(m_storage, committed, PROT_READ | PROT_WRITE) == 0;
#endif
        if (!writable)
            ReleaseStorage();
        REQUIRE(writable);
        characters = reinterpret_cast<CHARACTER*>(static_cast<BYTE*>(m_storage) + committed - bytes);
        std::uninitialized_value_construct_n(characters, MAX_CHARACTERS_CLIENT);
    }

    ~GuardedCharacters()
    {
        std::destroy_n(characters, MAX_CHARACTERS_CLIENT);
        ReleaseStorage();
    }

    CHARACTER* characters = nullptr;

private:
    void ReleaseStorage()
    {
#ifdef _WIN32
        VirtualFree(m_storage, 0, MEM_RELEASE);
#else
        munmap(m_storage, m_storageSize);
#endif
    }

    void* m_storage = nullptr;
    std::size_t m_storageSize = 0;
};

// This member is constructed before any global is replaced, including when setup throws.
class TempleSkillState
{
public:
    TempleSkillState()
        : m_previousHero(Hero), m_previousCharacters(CharactersClient), m_previousModels(Models),
          m_previousMachine(CharacterMachine), m_previousAttribute(CharacterAttribute),
          m_previousSkills(SkillAttribute), m_previousSocket(SocketClient), m_previousScene(SceneFlag),
          m_previousWorld(gMapManager.WorldActive), m_previousSelection(SelectedCharacter), m_previousTargetX(TargetX),
          m_previousTargetY(TargetY), m_previousMouse(MouseRButton), m_previousMousePush(MouseRButtonPush),
          m_previousActive(g_bWndActive), m_previousLanguage(g_strSelectedML),
          m_previousDirectory(std::filesystem::current_path()),
          m_previousTextureCollector(Render::Textures::GetTextureErrorCollector())
    {
    }

    ~TempleSkillState()
    {
        Restore();
    }

    void Activate()
    {
        m_active = true;
    }

    void Restore()
    {
        if (!m_active)
            return;
        g_pNewUISystem->Release();
        Bitmaps.UnloadAllImages();
        SocketClient = m_previousSocket;
        // This isolated binary owns the resolver and upload-skip test services.
        UI::RmlBridge::SetContextResolver({});
        Render::Textures::SetTextureErrorCollector(m_previousTextureCollector);
        Render::Sprites::SetGpuUploadsSkipped(false);
        SkillAttribute = m_previousSkills;
        Hero = m_previousHero;
        CharactersClient = m_previousCharacters;
        Models = m_previousModels;
        CharacterMachine = m_previousMachine;
        CharacterAttribute = m_previousAttribute;
        SceneFlag = m_previousScene;
        gMapManager.WorldActive = m_previousWorld;
        SelectedCharacter = m_previousSelection;
        TargetX = m_previousTargetX;
        TargetY = m_previousTargetY;
        MouseRButton = m_previousMouse;
        MouseRButtonPush = m_previousMousePush;
        g_bWndActive = m_previousActive;
        g_strSelectedML = m_previousLanguage;
        std::filesystem::current_path(m_previousDirectory);
        m_active = false;
    }

private:
    CHARACTER* m_previousHero;
    CHARACTER* m_previousCharacters;
    BMD* m_previousModels;
    CHARACTER_MACHINE* m_previousMachine;
    decltype(CharacterAttribute) m_previousAttribute;
    SKILL_ATTRIBUTE* m_previousSkills;
    decltype(SocketClient) m_previousSocket;
    EGameScene m_previousScene;
    int m_previousWorld;
    int m_previousSelection;
    int m_previousTargetX;
    int m_previousTargetY;
    bool m_previousMouse;
    bool m_previousMousePush;
    bool m_previousActive;
    std::wstring m_previousLanguage;
    std::filesystem::path m_previousDirectory;
    Render::Textures::TextureErrorCollector m_previousTextureCollector;
    bool m_active = false;
};

class TempleSkillFixture : public UI::Tests::RmlLayoutFixture
{
public:
    TempleSkillFixture()
        : m_models(std::make_unique<BMD[]>(MODEL_PLAYER + 1)), m_skills(std::make_unique<SKILL_ATTRIBUTE[]>(MAX_SKILLS))
    {
        m_state.Activate();
        std::filesystem::current_path(MU_BIN_DIR);
        UI::RmlBridge::SetContextResolver([this](std::string_view) { return context; });
        Render::Sprites::SetGpuUploadsSkipped(true);
        Render::Textures::SetTextureErrorCollector([](const std::string&) {});
        CharactersClient = m_table.characters;
        Hero = &CharactersClient[0];
        CharacterMachine = &m_machine;
        CharacterAttribute = &m_machine.Character;
        CharacterAttribute->Level = TestCharacterLevel;
        SkillAttribute = m_skills.get();
        g_bWndActive = true;
        g_strSelectedML = L"Eng";
        Models = m_models.get();
        Models[MODEL_PLAYER].NumActions = MAX_PLAYER_ACTION;
        Models[MODEL_PLAYER].Actions = new Action_t[MAX_PLAYER_ACTION]{};
        SceneFlag = MAIN_SCENE;
        gMapManager.WorldActive = WD_45CURSEDTEMPLE_LV1;
        Place(*Hero, HeroId);
        Place(CharactersClient[TeamSlot], TeamId);
        Place(CharactersClient[EnemySlot], EnemyId);
        TargetX = TargetY = TestTile;
        SelectedCharacter = -1;

        SetUpTemple();
        // Rejected targets must return before any transport access.
        SocketClient = nullptr;
#ifdef MU_TEST_TEMPLE_AOT_TRANSPORT
        m_connection = std::make_unique<Connection>(u"127.0.0.1", peer.port, false, nullptr);
        REQUIRE(m_connection->IsConnected());
        peer.Accept();
        SocketClient = m_connection.get();
#endif
    }

    ~TempleSkillFixture()
    {
        m_window.reset();
        m_state.Restore();
    }

    void Cast(int skill, DWORD selection)
    {
        Hero->m_CursedTempleCurSkill = skill;
        Hero->m_CursedTempleCurSkillPacket = false;
        MouseRButtonPush = true;
        m_window->SetCursedTempleSkill(Hero, &Hero->Object, selection);
    }

#ifdef MU_TEST_TEMPLE_AOT_TRANSPORT
    void BeginHeldCast(int skill, DWORD selection)
    {
        Hero->m_CursedTempleCurSkill = skill;
        Hero->m_CursedTempleCurSkillPacket = false;
        MouseRButton = true;
        MouseRButtonPush = true;
        m_window->SetCursedTempleSkill(Hero, &Hero->Object, selection);
    }

    void RetryHeldCast(DWORD selection)
    {
        m_window->SetCursedTempleSkill(Hero, &Hero->Object, selection);
    }

    void ResolveRejected(int skill)
    {
        m_window->ResolveSkill({static_cast<WORD>(skill), static_cast<WORD>(HeroId), static_cast<WORD>(HeroId), false});
    }

    void ReleaseRightButton()
    {
        MouseRButton = false;
        m_window->Update();
    }

    void ExpectRequestCount(std::size_t expectedCount)
    {
        const std::vector<BYTE> bytes = peer.Collect();
        std::size_t offset = 0;
        std::size_t requestCount = 0;
        while (offset < bytes.size())
        {
            const std::size_t remaining = bytes.size() - offset;
            REQUIRE(remaining >= 2);
            std::size_t packetLength = 0;
            std::size_t headerLength = 0;
            if (bytes[offset] == 0xC1)
            {
                headerLength = 2;
                packetLength = bytes[offset + 1];
            }
            else if (bytes[offset] == 0xC2)
            {
                REQUIRE(remaining >= 3);
                headerLength = 3;
                packetLength = (static_cast<std::size_t>(bytes[offset + 1]) << 8) | bytes[offset + 2];
            }
            else
            {
                FAIL("Unexpected MU packet header");
            }

            REQUIRE(packetLength >= headerLength + 2);
            REQUIRE(packetLength <= remaining);
            CHECK(bytes[offset + headerLength] == 0xBF);
            CHECK(bytes[offset + headerLength + 1] == 0x02);
            ++requestCount;
            offset += packetLength;
        }
        CHECK(requestCount == expectedCount);
    }
#endif

    void ExpectRejected(int skill, DWORD selection)
    {
        Cast(skill, selection);
        CHECK_FALSE(Hero->m_CursedTempleCurSkillPacket);
        CHECK_FALSE(MouseRButtonPush);
    }

private:
    void SetUpTemple()
    {
        for (int i = 0; i < TempleSkillCount; ++i)
        {
            const int skill = AT_SKILL_CURSED_TEMPLE_PRODECTION + i;
            SkillAttribute[skill].Distance = SkillRange;
            SkillAttribute[skill].KillCount = SkillCost;
        }

        REQUIRE(g_pNewUISystem->Create());
        m_window = std::make_unique<mu::ui::window::CCursedTempleSystem>();
        REQUIRE(m_window->Create(g_pNewUIMng));
        const std::array<UI::CursedTemple::PartyPosition, 2> team{
            UI::CursedTemple::PartyPosition{HeroId, WD_45CURSEDTEMPLE_LV1, TestTile, TestTile},
            UI::CursedTemple::PartyPosition{TeamId, WD_45CURSEDTEMPLE_LV1, TestTile, TestTile}};
        UI::CursedTemple::MatchStatus status{};
        status.relicHolderIndex = EnemyId;
        status.localTeam = SEASON3A::eTeam_Allied;
        status.party = team;
        m_window->SetMatchStatus(status);
        m_window->SetSkillPoints(SkillBudget);
    }

    static void Place(CHARACTER& character, short key)
    {
        character.Key = key;
        character.Helper.Type = -1;
        character.Object.Live = true;
        character.Object.Type = MODEL_PLAYER;
        character.Object.Kind = KIND_PLAYER;
        character.Object.Position[0] = character.Object.Position[1] = (TestTile + 0.5f) * TERRAIN_SCALE;
    }

    TempleSkillState m_state;
    GuardedCharacters m_table;
    CHARACTER_MACHINE m_machine{};
    std::unique_ptr<BMD[]> m_models;
    std::unique_ptr<SKILL_ATTRIBUTE[]> m_skills;
    std::unique_ptr<mu::ui::window::CCursedTempleSystem> m_window;
#ifdef MU_TEST_TEMPLE_AOT_TRANSPORT
public:
    SkillPacketPeer peer;

private:
    std::unique_ptr<Connection> m_connection;
#endif
};
} // namespace

// Each dangerous case is a separate discovered test/process so the old AV cannot mask other cases.
TEST_CASE("Restraint rejects missing character selection [ui][temple-skill][restraint-missing]")
{
    TempleSkillFixture fixture;
    fixture.ExpectRejected(AT_SKILL_CURSED_TEMPLE_RESTRAINT, MissingSelection);
}

TEST_CASE("Sublimation rejects missing character selection [ui][temple-skill][sublimation-missing]")
{
    TempleSkillFixture fixture;
    fixture.ExpectRejected(AT_SKILL_CURSED_TEMPLE_SUBLIMATION, MissingSelection);
}

TEST_CASE("Restraint rejects index at character table boundary [ui][temple-skill][restraint-boundary]")
{
    TempleSkillFixture fixture;
    fixture.ExpectRejected(AT_SKILL_CURSED_TEMPLE_RESTRAINT, MAX_CHARACTERS_CLIENT);
}

TEST_CASE("Sublimation rejects index at character table boundary [ui][temple-skill][sublimation-boundary]")
{
    TempleSkillFixture fixture;
    fixture.ExpectRejected(AT_SKILL_CURSED_TEMPLE_SUBLIMATION, MAX_CHARACTERS_CLIENT);
}

TEST_CASE("Temple hostile skills reject absent target keys and teammates [ui][temple-skill]")
{
    TempleSkillFixture fixture;
    for (const int skill : {AT_SKILL_CURSED_TEMPLE_RESTRAINT, AT_SKILL_CURSED_TEMPLE_SUBLIMATION})
    {
        CAPTURE(skill);
        fixture.ExpectRejected(skill, TeamSlot);
        CharactersClient[EnemySlot].Key = -1;
        fixture.ExpectRejected(skill, EnemySlot);
    }
}

#ifdef MU_TEST_TEMPLE_AOT_TRANSPORT
TEST_CASE("Temple response before RMB release does not repeat a held gesture [ui][temple-skill]")
{
    TempleSkillFixture fixture;
    const int skill = AT_SKILL_CURSED_TEMPLE_PRODECTION;

    fixture.BeginHeldCast(skill, MissingSelection);
    CHECK(Hero->m_CursedTempleCurSkillPacket);
    fixture.ExpectRequestCount(1);

    fixture.ResolveRejected(skill);
    CHECK_FALSE(Hero->m_CursedTempleCurSkillPacket);
    fixture.RetryHeldCast(MissingSelection);
    CHECK_FALSE(Hero->m_CursedTempleCurSkillPacket);
    fixture.ExpectRequestCount(0);

    fixture.ReleaseRightButton();
    fixture.BeginHeldCast(skill, MissingSelection);
    CHECK(Hero->m_CursedTempleCurSkillPacket);
    fixture.ExpectRequestCount(1);
}
#endif

namespace
{
class PartyMembershipFixture
{
public:
    PartyMembershipFixture() : m_previousCharacters(CharactersClient), m_previousCount(PartyNumber)
    {
        std::copy_n(Party, MAX_PARTYS, m_previousParty.begin());
        CharactersClient = m_table.characters;
        PartyNumber = 1;
        wcscpy(Party[0].Name, L"Teammate");
        wcscpy(CharactersClient[TeamSlot].ID, L"Teammate");
        wcscpy(CharactersClient[EnemySlot].ID, L"Enemy");
    }

    ~PartyMembershipFixture()
    {
        CharactersClient = m_previousCharacters;
        PartyNumber = m_previousCount;
        std::copy(m_previousParty.begin(), m_previousParty.end(), Party);
    }

private:
    GuardedCharacters m_table;
    CHARACTER* m_previousCharacters;
    int m_previousCount;
    std::array<PARTY_t, MAX_PARTYS> m_previousParty{};
};
} // namespace

TEST_CASE("Temple party membership rejects missing selection [ui][temple-skill][party-missing]")
{
    PartyMembershipFixture fixture;
    CHECK_FALSE(g_CursedTemple->IsPartyMember(MissingSelection));
}

TEST_CASE("Temple party membership rejects character table boundary [ui][temple-skill][party-boundary]")
{
    PartyMembershipFixture fixture;
    CHECK_FALSE(g_CursedTemple->IsPartyMember(MAX_CHARACTERS_CLIENT));
}

TEST_CASE("Temple party membership recognizes teammates and enemies [ui][temple-skill][party-controls]")
{
    PartyMembershipFixture fixture;
    CHECK(g_CursedTemple->IsPartyMember(TeamSlot));
    CHECK_FALSE(g_CursedTemple->IsPartyMember(EnemySlot));
}
