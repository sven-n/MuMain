#include "stdafx.h"

#include "MonsterLookRecorder.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Core/Utilities/Random.h"
#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Engine/Object/ObjectPose.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Items/LookDrawing.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"

#include "doctest.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

extern double WorldTime;

namespace MonsterRecorder
{
namespace
{
const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;
const std::filesystem::path Baseline = MU_MONSTER_LOOK_BASELINE;

// A time well into a session, with a fraction, as the item looks' recorder pins it.
constexpr double PinnedWorldTime = 1234567.25;

// The key Random::DrawRand takes for the drawn monster: an address no object has, the same in every run.
const void* FixedDrawKey()
{
    return reinterpret_cast<const void*>(static_cast<std::uintptr_t>(0x20000));
}

constexpr const char* BaselineHeader =
    "# How the monster looks moved from code into Data/Effects draw (MN8): one line per monster number, map and\n"
    "# situation, then the number of mesh draws and five digests of them and of what the drawing left on the model\n"
    "# (tests/data/LookRecorder.h). Written from the old code before each look moved (MU_MONSTER_LOOK_WRITE=1), so a\n"
    "# moved look still draws as its code did. The frozen (buff 56) and Blow of Destruction (buff 86) lines came\n"
    "# after the code was gone, so they are not the old code's: they are drawn from the data. Both debuffs draw the\n"
    "# body and its second pass in the same blue tint at the monster's opacity (MN8 ML-P19), so a frozen line is the\n"
    "# number's Blow of Destruction line.\n";

// The textures OpenBasicData loads at their indices before the looks find theirs, which the monsters' looks in code
// name.
struct BasicTexture
{
    const wchar_t* file;
    GLuint index;
    GLuint wrap;
};
constexpr BasicTexture BasicTextures[] = {
    {L"Effect\\Chrome01.jpg", BITMAP_CHROME, GL_REPEAT},
    {L"Effect\\bab2.jpg", BITMAP_CHROME + 1, GL_REPEAT},
    {L"Effect\\Chrome03.jpg", BITMAP_CHROME3, GL_CLAMP_TO_EDGE},
    {L"Monster\\BoneSE.jpg", BITMAP_BONE_SCORPION_SKIN_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\KryptaBall2.jpg", BITMAP_KRYPTA_BALL_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\bora_golem_effect.jpg", BITMAP_CONDRA_SKIN_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\bora_golem2_effect.jpg", BITMAP_CONDRA_SKIN_EFFECT2, GL_CLAMP_TO_EDGE},
    {L"Monster\\king_golem01_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\king_golem02_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT2, GL_CLAMP_TO_EDGE},
    {L"Monster\\king_golem03_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT3, GL_CLAMP_TO_EDGE},
    {L"Effect\\Bugbear_R.jpg", BITMAP_BUGBEAR_R, GL_REPEAT},
    {L"Effect\\pk_mon02_fire.jpg", BITMAP_PKMON01, GL_REPEAT},
    {L"Effect\\pk_mon03_red.jpg", BITMAP_PKMON02, GL_REPEAT},
    {L"Effect\\pk_mon02_green.jpg", BITMAP_PKMON03, GL_REPEAT},
    {L"Effect\\pk_mon03_green.jpg", BITMAP_PKMON04, GL_REPEAT},
    {L"Effect\\lavagiantAa_e.jpg", BITMAP_PKMON05, GL_REPEAT},
    {L"Effect\\lavagiantBa_e.jpg", BITMAP_PKMON06, GL_REPEAT},
    {L"Effect\\raymond_sword_R.jpg", BITMAP_RAYMOND_SWORD, GL_REPEAT},
    {L"Monster\\iui03.jpg", BITMAP_JANUSEXT, GL_REPEAT},
    // Tantallos' model loads them (Models.json).
    {L"Monster\\bv01_2.jpg", BITMAP_MONSTER_SKIN, GL_REPEAT},
    {L"Monster\\bv02_2.jpg", BITMAP_MONSTER_SKIN + 1, GL_REPEAT},
    {L"Monster\\DGicewalker_body.jpg", BITMAP_DOPPELGANGER_ICEWALKER0, GL_CLAMP_TO_EDGE},
    {L"Monster\\DGicewalker_R.jpg", BITMAP_DOPPELGANGER_ICEWALKER1, GL_CLAMP_TO_EDGE},
    {L"Monster\\Snake1.jpg", BITMAP_DOPPELGANGER_SNAKE01, GL_CLAMP_TO_EDGE},
    {L"Effect\\Shiny02.jpg", BITMAP_SHINY + 1, GL_CLAMP_TO_EDGE},
    {L"Monster\\King11.jpg", BITMAP_WHITE_WIZARD, GL_CLAMP_TO_EDGE},
    {L"Monster\\bladeeff2_r.jpg", BITMAP_BLADEHUNTER_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\illumi_R.jpg", BITMAP_TWINTAIL_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\prsona_R.jpg", BITMAP_PRSONA_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\prsonass_R.jpg", BITMAP_PRSONA_EFFECT2, GL_CLAMP_TO_EDGE},
    {L"Monster\\buserbody_r.jpg", BITMAP_BERSERK_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\busersword_r.jpg", BITMAP_BERSERK_WP_EFFECT, GL_CLAMP_TO_EDGE},
    {L"Monster\\Kni000.jpg", BITMAP_DEST_ORC_WAR0, GL_CLAMP_TO_EDGE},
    {L"Monster\\Kni011.jpg", BITMAP_DEST_ORC_WAR1, GL_CLAMP_TO_EDGE},
    {L"Monster\\Kni022.jpg", BITMAP_DEST_ORC_WAR2, GL_CLAMP_TO_EDGE},
};

// The textures the monsters' maps load at their slots (MapManager::Load), after the looks found theirs: the looks
// find them again then (RefindTextures).
constexpr BasicTexture MapTextures[] = {
    {L"Monster\\bosswing.tga", BITMAP_HGBOSS_WING, GL_REPEAT},
    {L"Monster\\bossmap1_R.jpg", BITMAP_HGBOSS_PATTERN, GL_CLAMP_TO_EDGE},
    {L"Npc\\khs_kan2gate001.jpg", BITMAP_KANTURU_2ND_NPC1, GL_CLAMP_TO_EDGE},
    {L"Npc\\khs_kan2gate003.jpg", BITMAP_KANTURU_2ND_NPC2, GL_CLAMP_TO_EDGE},
    {L"Npc\\khs_kan2gate004.jpg", BITMAP_KANTURU_2ND_NPC3, GL_CLAMP_TO_EDGE},
    {L"Monster\\nightmare_R.jpg", BITMAP_NIGHTMARE_EFFECT1, GL_CLAMP_TO_EDGE},
    {L"Monster\\nightmaresward_R.jpg", BITMAP_NIGHTMARE_EFFECT2, GL_CLAMP_TO_EDGE},
};

// Loads the textures whose slot is empty or holds another file. A slot loaded by an earlier test case can be empty
// in a later one of the same run (the White Wizard's King11.jpg was), so every recorder setup checks them again.
void LoadMissing(std::span<const BasicTexture> textures)
{
    for (const BasicTexture& texture : textures)
    {
        const BITMAP_t* loaded = Bitmaps.FindTexture(static_cast<GLuint>(texture.index));
        if (loaded == nullptr || std::wstring_view(loaded->FileName) != std::wstring(L"Data\\") + texture.file)
            LoadBitmap(texture.file, texture.index, GL_LINEAR, texture.wrap);
    }
}

std::string Number(double value)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%g", value);
    return text;
}
} // namespace

std::string DrawSituation::Describe() const
{
    char time[32];
    std::snprintf(time, sizeof(time), "%.2f", worldTime);
    std::string text =
        Data::Monsters::GetMonsterAnimationName(action) + " at frame " + Number(frame) + ", " + time + " ms";
    if (alpha < 1.f)
        text += ", alpha " + Number(alpha);
    if (buff)
        text += ", buff " + std::to_string(*buff);
    return text;
}

std::vector<DrawSituation> DrawSituations(int slot)
{
    const int actions = Models[slot].NumActions;
    std::vector<DrawSituation> situations;
    for (const int action : {MONSTER01_STOP1, MONSTER01_WALK, MONSTER01_ATTACK1, MONSTER01_DIE})
    {
        if (action < actions)
            situations.push_back(
                {.action = action, .frame = action == MONSTER01_STOP1 ? 0.f : 1.5f, .worldTime = PinnedWorldTime});
    }
    situations.push_back({.worldTime = PinnedWorldTime + 700.5});
    situations.push_back({.worldTime = PinnedWorldTime, .buff = eDeBuff_Poison});
    // Frozen and hit by Blow of Destruction: the tint and the second body pass over it.
    situations.push_back({.worldTime = PinnedWorldTime, .buff = eDeBuff_Freeze});
    situations.push_back({.worldTime = PinnedWorldTime, .buff = eDeBuff_BlowOfDestruction});
    situations.push_back({.worldTime = PinnedWorldTime, .alpha = 0.5f});
    if (slot == MODEL_BLADE_HUNTER && MONSTER01_DIE < actions)
        situations.push_back({.action = MONSTER01_DIE, .frame = 1.5f, .worldTime = PinnedWorldTime, .alpha = 0.5f});
    return situations;
}

LookRecorder::Record RecordMonster(CHARACTER& character, const DrawSituation& situation)
{
    OBJECT& object = character.Object;
    BMD& model = Models[object.Type];
    // The fields a drawing is set up with, restored afterwards (the object keeps its own bones).
    const unsigned short keptAction = object.CurrentAction;
    const unsigned short keptPriorAction = object.PriorAction;
    const float keptFrame = object.AnimationFrame;
    const float keptPriorFrame = object.PriorAnimationFrame;
    const float keptAlpha = object.Alpha;
    const bool keptVisible = object.Visible;
    vec3_t keptLight;
    VectorCopy(object.Light, keptLight);
    const Buff keptBuffs = object.m_BuffMap;
    const char keptStreamMesh = model.StreamMesh;
    vec3_t keptBodyLight;
    VectorCopy(model.BodyLight, keptBodyLight);
    const bool keptLightEnable = model.LightEnable;
    const double previousWorldTime = WorldTime;

    // The key of the drawn object's randomness stays the same from run to run.
    const Random::DrawnAs fixedKey(&object, FixedDrawKey());
    WorldTime = situation.worldTime;
    object.CurrentAction = static_cast<unsigned short>(situation.action);
    object.PriorAction = static_cast<unsigned short>(situation.action);
    object.AnimationFrame = situation.frame;
    object.PriorAnimationFrame = situation.frame;
    object.Alpha = situation.alpha;
    object.Visible = true;
    // The light the scene puts on it: the tests have no terrain light, and a look that tints the light it is handed
    // must show it.
    object.Light[0] = 0.55f;
    object.Light[1] = 0.45f;
    object.Light[2] = 0.35f;
    object.m_BuffMap.ClearBuff();
    if (situation.buff)
        object.m_BuffMap.RegisterBuff(static_cast<eBuffState>(*situation.buff));

    // As the update poses a character before its draws (AnimateCharacterBones): every pass draws its own pose.
    AnimateCharacterBones(&object);
    LookRecorder::Record record = LookRecorder::RecordDrawing(model, [&] { RenderCharacter(&character, &object, 0); });
    // The render state left after the whole character is no part of its look: each model draw sets its own, and a
    // monster's render style draws its glow passes with its body, before its shadow and weapons, where the code drew
    // them last.
    std::erase_if(record.gpuCalls, [](const std::string& call) { return call.starts_with("final "); });
    // Each draw of a monster sets its model's light and lighting first: only the scrolling mesh it leaves lasts.
    std::erase_if(record.modelAfter,
                  [](const LookRecorder::Field& field) { return std::string_view(field.first) != "streamMesh"; });
    // The model drawing never reads the mark of a monster's hands, which the monsters' glow passes in code carried.
    for (auto& draw : record.meshDraws)
    {
        for (auto& [name, value] : draw)
        {
            if (std::string_view(name) == "flags")
            {
                char text[16];
                std::snprintf(text, sizeof(text), "0x%x",
                              static_cast<unsigned>(std::stoul(value, nullptr, 16) & ~unsigned{RENDER_EXTRA}));
                value = text;
            }
        }
    }

    WorldTime = previousWorldTime;
    object.CurrentAction = keptAction;
    object.PriorAction = keptPriorAction;
    object.AnimationFrame = keptFrame;
    object.PriorAnimationFrame = keptPriorFrame;
    object.Alpha = keptAlpha;
    object.Visible = keptVisible;
    VectorCopy(keptLight, object.Light);
    object.m_BuffMap = keptBuffs;
    Engine::Object::Pose::Invalidate(&object);
    model.StreamMesh = keptStreamMesh;
    VectorCopy(keptBodyLight, model.BodyLight);
    model.LightEnable = keptLightEnable;
    return record;
}

struct ShippedLooks::State
{
    std::vector<Data::Looks::LookDefinition> renderStyles;
    std::vector<Data::Looks::LookDefinition> effects;
};

ShippedLooks::ShippedLooks() : m_state(new State)
{
    using Data::Looks::g_LookStore;
    using Data::Looks::LookKind;
    m_state->renderStyles.assign(g_LookStore.GetRenderStyles().begin(), g_LookStore.GetRenderStyles().end());
    m_state->effects.assign(g_LookStore.GetEffects().begin(), g_LookStore.GetEffects().end());
    for (const LookKind kind : {LookKind::RenderStyle, LookKind::Effect})
    {
        Data::Looks::RenderStylesLoadResult shipped = Data::Looks::LoadLookFile(DataDirectory / "Effects", kind);
        REQUIRE_FALSE(Data::Items::HasFatal(shipped.issues));
        g_LookStore.SetLooks(kind, std::move(shipped.looks));
    }
    LoadMissing(BasicTextures);
    Render::Items::Looks::PrepareTextures();
    // A map loads its textures after the looks found theirs, as when the hero enters it.
    LoadMissing(MapTextures);
    Render::Items::Looks::RefindTextures();
}

ShippedLooks::~ShippedLooks()
{
    Data::Looks::g_LookStore.SetRenderStyles(std::move(m_state->renderStyles));
    Data::Looks::g_LookStore.SetEffects(std::move(m_state->effects));
    delete m_state;
}

std::map<std::string, std::string> ReadBaseline()
{
    std::map<std::string, std::string> lines;
    std::ifstream in(Baseline);
    std::string line;
    while (std::getline(in, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        const size_t tab = line.rfind('\t');
        if (line.empty() || line[0] == '#' || tab == std::string::npos)
            continue;
        lines[line.substr(0, tab)] = line.substr(tab + 1);
    }
    return lines;
}

void WriteBaseline(const std::map<std::string, std::string>& lines)
{
    std::filesystem::create_directories(Baseline.parent_path());
    std::ofstream out(Baseline, std::ios::binary | std::ios::trunc);
    out << BaselineHeader;
    for (const auto& [key, digest] : lines)
        out << key << '\t' << digest << '\n';
}

std::string BaselineKey(int number, const Data::Maps::MapRef& map, const DrawSituation& situation)
{
    std::string text = std::to_string(number) + " on map " + std::to_string(map.map);
    if (map.stage != 0)
        text += " stage " + std::to_string(map.stage);
    return text + ", " + situation.Describe();
}
} // namespace MonsterRecorder
