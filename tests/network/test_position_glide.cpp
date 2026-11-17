#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Network/Server/WSclient.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "World/MapInfra/MapManager.h"

#include <memory>
#include <span>
#include <utility>
#include <vector>

extern float FPS_ANIMATION_FACTOR;

namespace
{
constexpr int ObservedKey = 2;
constexpr BYTE StartTile = 50;
constexpr BYTE CorrectedTile = StartTile + 1;
constexpr BYTE WalkTargetTile = StartTile + 6;
constexpr int FramesToWatch = 5;
// A running player's speed (CharacterMoveSpeed), in world units per frame.
constexpr WORD RunSpeed = 15;

float TileCentre(int tile)
{
    return (static_cast<float>(tile) + 0.5f) * TERRAIN_SCALE;
}

// Another player in view of the hero, standing on (StartTile, StartTile).
struct ObservedPlayerFixture
{
    CHARACTER* previousCharacters = CharactersClient;
    CHARACTER* previousHero = Hero;
    BMD* previousModels = Models;
    int previousWorld = gMapManager.WorldActive;
    float previousAnimationFactor = FPS_ANIMATION_FACTOR;
    CHARACTER_MACHINE* previousMachine = CharacterMachine;
    // SetPlayerWalk reads the hero's boots and gloves.
    std::unique_ptr<CHARACTER_MACHINE> machine = std::make_unique<CHARACTER_MACHINE>();
    // FindCharacterIndex returns MAX_CHARACTERS_CLIENT for an unknown key, and the handlers write to that slot.
    std::unique_ptr<CHARACTER[]> characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT + 1);
    std::unique_ptr<BMD[]> models = std::make_unique<BMD[]>(MODEL_PLAYER + 1);
    std::unique_ptr<Action_t[]> playerActions = std::make_unique<Action_t[]>(MAX_PLAYER_ACTION);
    CHARACTER& observed = characters[1];

    ObservedPlayerFixture()
    {
        gMapManager.WorldActive = WD_0LORENCIA;
        FPS_ANIMATION_FACTOR = 1.f;
        CharacterMachine = machine.get();
        InitPath();
        Models = models.get();
        Models[MODEL_PLAYER].NumActions = MAX_PLAYER_ACTION;
        Models[MODEL_PLAYER].Actions = playerActions.get();
        CharactersClient = characters.get();
        Hero = &characters[0];
        Hero->Key = 1;
        Hero->Object.Live = true;

        observed.Key = ObservedKey;
        observed.Object.Live = true;
        observed.Object.Type = MODEL_PLAYER;
        observed.Object.Kind = KIND_PLAYER;
        observed.Helper.Type = -1;
        observed.Weapon[0].Type = -1;
        observed.Weapon[1].Type = -1;
        observed.PositionX = observed.TargetX = StartTile;
        observed.PositionY = observed.TargetY = StartTile;
        observed.Object.Position[0] = TileCentre(StartTile);
        observed.Object.Position[1] = TileCentre(StartTile);
    }

    ~ObservedPlayerFixture()
    {
        Models[MODEL_PLAYER].Actions = nullptr;
        Models = previousModels;
        FPS_ANIMATION_FACTOR = previousAnimationFactor;
        CharacterMachine = previousMachine;
        Hero = previousHero;
        CharactersClient = previousCharacters;
        gMapManager.WorldActive = previousWorld;
    }

    void ReceiveInstantMove(BYTE x, BYTE y)
    {
        PRECEIVE_MOVE_POSITION packet{};
        packet.KeyL = ObservedKey;
        packet.PositionX = x;
        packet.PositionY = y;
        ReceiveMovePosition(reinterpret_cast<const BYTE*>(&packet));
    }

    void ReceiveWalk(BYTE sourceX, BYTE sourceY, BYTE targetX, BYTE targetY)
    {
        PMOVE_CHARACTER packet{};
        packet.KeyL = ObservedKey;
        packet.SourceX = sourceX;
        packet.SourceY = sourceY;
        packet.TargetX = targetX;
        packet.TargetY = targetY;
        ReceiveMoveCharacter(std::span(reinterpret_cast<const BYTE*>(&packet), sizeof(packet)));
    }

    // The movement half of MoveCharacterClient: the walk, then the glide.
    void RunFrames(int count)
    {
        for (int frame = 0; frame < count; ++frame)
        {
            MoveMonsterClient(&observed, &observed.Object);
            PushingCharacter(&observed, &observed.Object);
        }
    }
};
} // namespace

TEST_CASE("An instant move of another player glides it to the new tile [network][movement]")
{
    ObservedPlayerFixture fixture;
    fixture.ReceiveInstantMove(StartTile + 1, StartTile);

    CHECK(fixture.observed.JumpTime > 0);
    CHECK(fixture.observed.TargetX == StartTile + 1);
}

TEST_CASE("A walk right after an instant move walks instead of gliding to the walk target [network][movement]")
{
    ObservedPlayerFixture fixture;
    // Walks and glides run the same code for monsters. A player's speed would ask the cursed temple window
    // whether it carries the holy item, and tests have no windows.
    fixture.observed.Object.Kind = KIND_MONSTER;
    fixture.observed.MoveSpeed = RunSpeed;
    fixture.ReceiveInstantMove(CorrectedTile, StartTile);
    fixture.ReceiveWalk(CorrectedTile, StartTile, WalkTargetTile, StartTile);

    CHECK(fixture.observed.Movement);
    CHECK(fixture.observed.JumpTime == 0);
    CHECK(fixture.observed.TargetX == WalkTargetTile);

    fixture.RunFrames(FramesToWatch);

    // A glide covers a fifth of the remaining way each frame and would be past tile 53 by now.
    const float drawnX = fixture.observed.Object.Position[0];
    CHECK(drawnX > TileCentre(StartTile));
    CHECK(drawnX < TileCentre(CorrectedTile + 1));
    CHECK(fixture.observed.Movement);
}

TEST_CASE("A walk whose path clears a frame later still replaces the glide [network][movement]")
{
    ObservedPlayerFixture fixture;
    fixture.observed.Object.Kind = KIND_MONSTER;
    fixture.observed.MoveSpeed = RunSpeed;
    fixture.ReceiveInstantMove(CorrectedTile, StartTile);

    // When the walk arrives, characters stand on every tile around it, so it has no step to take.
    std::vector<std::pair<int, WORD>> blockedTiles;
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            if (dx == 0 && dy == 0)
            {
                continue;
            }
            const int index = TERRAIN_INDEX_REPEAT(CorrectedTile + dx, StartTile + dy);
            blockedTiles.emplace_back(index, TerrainWall[index]);
            TerrainWall[index] |= TW_CHARACTER;
        }
    }
    fixture.ReceiveWalk(CorrectedTile, StartTile, WalkTargetTile, StartTile);
    for (const auto& [index, wall] : blockedTiles)
    {
        TerrainWall[index] = wall;
    }

    CHECK_FALSE(fixture.observed.Movement);
    CHECK(fixture.observed.JumpTime > 0);

    // They have stepped away, so MoveMonsterClient's retry finds the path.
    fixture.RunFrames(FramesToWatch);

    CHECK(fixture.observed.Movement);
    CHECK(fixture.observed.JumpTime == 0);
    // One frame of glide moved it a fifth of the way to tile 56; walking on from there keeps it short of tile 53.
    CHECK(fixture.observed.Object.Position[0] < TileCentre(CorrectedTile + 2));
}

TEST_CASE("A walk that cannot start yet lets the running glide carry the character to its end [network][movement]")
{
    ObservedPlayerFixture fixture;
    fixture.ReceiveInstantMove(CorrectedTile, StartTile);
    fixture.observed.Appear = 1;
    fixture.ReceiveWalk(CorrectedTile, StartTile, WalkTargetTile, StartTile);

    CHECK_FALSE(fixture.observed.Movement);
    CHECK(fixture.observed.JumpTime > 0);

    fixture.RunFrames(FramesToWatch);

    // The glide carries it on towards the walk's end instead of leaving it frozen where it was drawn.
    CHECK_FALSE(fixture.observed.Movement);
    CHECK(fixture.observed.Object.Position[0] > TileCentre(CorrectedTile + 2));
    CHECK(fixture.observed.Object.Position[0] <= TileCentre(WalkTargetTile));
}
