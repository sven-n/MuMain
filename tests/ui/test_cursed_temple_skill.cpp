#include "stdafx.h"
#include <doctest.h>

#include "RmlLayoutFixture.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInterface.h"
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
#include <memory>

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
constexpr DWORD MissingSelection = static_cast<DWORD>(-1);

// End the real character table at an inaccessible page. Index400 must not silently read padding.
class GuardedCharacters
{
public:
    GuardedCharacters()
    {
        SYSTEM_INFO info{};
        GetSystemInfo(&info);
        const std::size_t bytes = sizeof(CHARACTER) * MAX_CHARACTERS_CLIENT;
        const std::size_t committed = (bytes + info.dwPageSize - 1) / info.dwPageSize * info.dwPageSize;
        m_storage = VirtualAlloc(nullptr, committed + info.dwPageSize, MEM_RESERVE, PAGE_NOACCESS);
        REQUIRE(m_storage != nullptr);
        REQUIRE(VirtualAlloc(m_storage, committed, MEM_COMMIT, PAGE_READWRITE) == m_storage);
        characters = reinterpret_cast<CHARACTER*>(static_cast<BYTE*>(m_storage) + committed - bytes);
        std::uninitialized_value_construct_n(characters, MAX_CHARACTERS_CLIENT);
    }

    ~GuardedCharacters()
    {
        std::destroy_n(characters, MAX_CHARACTERS_CLIENT);
        VirtualFree(m_storage, 0, MEM_RELEASE);
    }

    CHARACTER* characters = nullptr;

private:
    void* m_storage = nullptr;
};

class TempleSkillFixture : public UI::Tests::RmlLayoutFixture
{
public:
    TempleSkillFixture()
        : m_previousHero(Hero), m_previousCharacters(CharactersClient), m_previousModels(Models),
          m_previousMachine(CharacterMachine), m_previousAttribute(CharacterAttribute),
          m_previousSkills(SkillAttribute), m_previousSocket(SocketClient), m_previousScene(SceneFlag),
          m_previousWorld(gMapManager.WorldActive), m_previousSelection(SelectedCharacter), m_previousTargetX(TargetX),
          m_previousTargetY(TargetY), m_previousMousePush(MouseRButtonPush), m_previousActive(g_bWndActive),
          m_previousLanguage(g_strSelectedML), m_previousDirectory(std::filesystem::current_path()),
          m_previousTextureCollector(Render::Textures::GetTextureErrorCollector()),
          m_models(std::make_unique<BMD[]>(MODEL_PLAYER + 1)), m_skills(std::make_unique<SKILL_ATTRIBUTE[]>(MAX_SKILLS))
    {
        std::filesystem::current_path(MU_BIN_DIR);
        UI::RmlBridge::SetContextResolver([this](std::string_view) { return context; });
        Render::Sprites::SetGpuUploadsSkipped(true);
        Render::Textures::SetTextureErrorCollector([](const std::string&) {});
        CharactersClient = m_table.characters;
        Hero = &CharactersClient[0];
        CharacterMachine = &m_machine;
        CharacterAttribute = &m_machine.Character;
        CharacterAttribute->Level = 400;
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
        TargetX = TargetY = 40;
        SelectedCharacter = -1;

        SetUpTemple();
        // Rejected targets must return before any transport access.
        SocketClient = nullptr;
    }

    ~TempleSkillFixture()
    {
        m_window.reset();
        g_pNewUISystem->Release();
        Bitmaps.UnloadAllImages();
        SocketClient = m_previousSocket;
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
        MouseRButtonPush = m_previousMousePush;
        g_bWndActive = m_previousActive;
        g_strSelectedML = m_previousLanguage;
        std::filesystem::current_path(m_previousDirectory);
    }

    void Cast(int skill, DWORD selection)
    {
        Hero->m_CursedTempleCurSkill = skill;
        Hero->m_CursedTempleCurSkillPacket = false;
        MouseRButtonPush = true;
        m_window->SetCursedTempleSkill(Hero, &Hero->Object, selection);
    }

    void ExpectRejected(int skill, DWORD selection)
    {
        Cast(skill, selection);
        CHECK_FALSE(Hero->m_CursedTempleCurSkillPacket);
        CHECK_FALSE(MouseRButtonPush);
        CHECK(SocketClient == nullptr);
    }

private:
    void SetUpTemple()
    {
        for (int i = 0; i < 4; ++i)
        {
            const int skill = AT_SKILL_CURSED_TEMPLE_PRODECTION + i;
            SkillAttribute[skill].Distance = SkillRange;
            SkillAttribute[skill].KillCount = SkillCost;
        }

        REQUIRE(g_pNewUISystem->Create());
        m_window = std::make_unique<mu::ui::window::CCursedTempleSystem>();
        REQUIRE(m_window->Create(g_pNewUIMng));
        const std::array<UI::CursedTemple::PartyPosition, 2> team{
            UI::CursedTemple::PartyPosition{HeroId, WD_45CURSEDTEMPLE_LV1, 40, 40},
            UI::CursedTemple::PartyPosition{TeamId, WD_45CURSEDTEMPLE_LV1, 40, 40}};
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
        character.Object.Position[0] = character.Object.Position[1] = (40.f + 0.5f) * TERRAIN_SCALE;
    }

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
    bool m_previousMousePush;
    bool m_previousActive;
    std::wstring m_previousLanguage;
    std::filesystem::path m_previousDirectory;
    Render::Textures::TextureErrorCollector m_previousTextureCollector;
    GuardedCharacters m_table;
    CHARACTER_MACHINE m_machine{};
    std::unique_ptr<BMD[]> m_models;
    std::unique_ptr<SKILL_ATTRIBUTE[]> m_skills;
    std::unique_ptr<mu::ui::window::CCursedTempleSystem> m_window;
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
