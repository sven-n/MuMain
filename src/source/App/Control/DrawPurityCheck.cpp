#include "stdafx.h"

#include "DrawPurityCheck.h"

#ifdef MU_ENABLE_CONTROL_SOCKET

#include "Camera/CameraState.h"
#include "Core/Utilities/Fnv1aHash.h"
#include "Core/Utilities/Random.h"
#include "Engine/Object/CharacterAttachments.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Physics/PhysicsManager.h"
#include "GameLogic/Skills/SkillEffectMgr.h"
#include "GameLogic/Social/MonkSystem.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Lighting/DynamicLightHistory.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <string>
#include <unordered_map>

extern float FrustrumX[];
extern float FrustrumY[];
extern int FrustrumCount;
extern int FrustrumBoundMinX;
extern int FrustrumBoundMinY;
extern int FrustrumBoundMaxX;
extern int FrustrumBoundMaxY;

namespace Diagnostics::DrawPurityCheck
{
namespace
{
constexpr int MaxFields = 48;

struct Entry
{
    const char* kind = nullptr;
    const char* const* names = nullptr;
    int type = 0;
    int count = 0;
    std::array<std::uint64_t, MaxFields> values{};
};

class Recorder
{
public:
    explicit Recorder(std::unordered_map<const void*, Entry>& into) : m_into(into) {}

    Entry& Begin(const void* key, const char* kind, const char* const* names, int type)
    {
        Entry& entry = m_into[key];
        entry = Entry{kind, names, type};
        return entry;
    }

private:
    std::unordered_map<const void*, Entry>& m_into;
};

void Put(Entry& e, float v)
{
    e.values[e.count++] = std::bit_cast<std::uint32_t>(v);
}
void Put(Entry& e, int v)
{
    e.values[e.count++] = static_cast<std::uint32_t>(v);
}
void Put(Entry& e, std::uint64_t v)
{
    e.values[e.count++] = v;
}
void Put(Entry& e, const void* v)
{
    e.values[e.count++] = reinterpret_cast<std::uintptr_t>(v);
}
void Put(Entry& e, const vec3_t v)
{
    Put(e, v[0]);
    Put(e, v[1]);
    Put(e, v[2]);
}

constexpr const char* ObjectFields[] = {"Live",
                                        "Type",
                                        "SubType",
                                        "CurrentAction",
                                        "PriorAction",
                                        "AnimationFrame",
                                        "PriorAnimationFrame",
                                        "LifeTime",
                                        "Position.x",
                                        "Position.y",
                                        "Position.z",
                                        "Angle.x",
                                        "Angle.y",
                                        "Angle.z",
                                        "Scale",
                                        "Alpha",
                                        "Light.r",
                                        "Light.g",
                                        "Light.b",
                                        "Velocity",
                                        "Gravity",
                                        "HiddenMesh",
                                        "PKKey",
                                        "m_iAnimation",
                                        "Owner",
                                        "Direction.x",
                                        "Direction.y",
                                        "Direction.z",
                                        "Visible",
                                        "DrawnFrame",
                                        "BlendMesh",
                                        "BlendMeshLight",
                                        "BlendMeshTexCoordU",
                                        "BlendMeshTexCoordV",
                                        "AlphaTarget",
                                        "StartPosition.x",
                                        "StartPosition.y",
                                        "StartPosition.z",
                                        "ScreenX",
                                        "ScreenY",
                                        "OBB.StartPos.x",
                                        "OBB.StartPos.y",
                                        "OBB.StartPos.z",
                                        "OBB.XAxis.x",
                                        "OBB.YAxis.y",
                                        "OBB.ZAxis.z",
                                        "Buffs"};
constexpr const char* ParticleFields[] = {
    "Live",     "Type",     "SubType", "Scale",   "Position.x", "Position.y", "Position.z",
    "Angle.x",  "Angle.y",  "Angle.z", "Light.r", "Light.g",    "Light.b",    "Alpha",
    "LifeTime", "Rotation", "Frame",   "Gravity", "Velocity.x", "Velocity.y", "Velocity.z"};
constexpr const char* JointFields[] = {
    "Live",     "Type",        "SubType",     "Scale",       "Position.x",       "Position.y",       "Position.z",
    "Angle.x",  "Angle.y",     "Angle.z",     "Light.r",     "Light.g",          "Light.b",          "LifeTime",
    "Velocity", "Direction.x", "Direction.y", "Direction.z", "TargetPosition.x", "TargetPosition.y", "TargetPosition.z",
    "NumTails", "Weapon",      "MultiUse"};

void Record(Recorder& r, const OBJECT* o, const char* kind)
{
    Entry& e = r.Begin(o, kind, ObjectFields, o->Type);
    Put(e, o->Live ? 1 : 0);
    Put(e, o->Type);
    Put(e, o->SubType);
    Put(e, static_cast<int>(o->CurrentAction));
    Put(e, static_cast<int>(o->PriorAction));
    Put(e, o->AnimationFrame);
    Put(e, o->PriorAnimationFrame);
    Put(e, o->LifeTime);
    Put(e, o->Position);
    Put(e, o->Angle);
    Put(e, o->Scale);
    Put(e, o->Alpha);
    Put(e, o->Light);
    Put(e, o->Velocity);
    Put(e, o->Gravity);
    Put(e, o->HiddenMesh);
    Put(e, static_cast<int>(o->PKKey));
    Put(e, o->m_iAnimation);
    Put(e, static_cast<const void*>(o->Owner));
    Put(e, o->Direction);
    Put(e, o->Visible ? 1 : 0);
    Put(e, static_cast<int>(o->DrawnFrame));
    Put(e, o->BlendMesh);
    Put(e, o->BlendMeshLight);
    Put(e, o->BlendMeshTexCoordU);
    Put(e, o->BlendMeshTexCoordV);
    Put(e, o->AlphaTarget);
    Put(e, o->StartPosition);
    Put(e, o->ScreenX);
    Put(e, o->ScreenY);
    Put(e, o->OBB.StartPos);
    Put(e, o->OBB.XAxis[0]);
    Put(e, o->OBB.YAxis[1]);
    Put(e, o->OBB.ZAxis[2]);
    Fnv1aHash buffs;
    for (DWORD i = 0; i < o->m_BuffMap.GetBuffSize(); ++i)
        buffs.Add(o->m_BuffMap.GetBuff(static_cast<int>(i)));
    Put(e, buffs.Value());
}

constexpr const char* CharacterFields[] = {"Light.r",    "Light.g",  "Light.b",
                                           "HideShadow", "SafeZone", "TargetCharacter"};
constexpr const char* PartFields[] = {
    "Type", "Level", "CurrentAction", "PriorAction", "AnimationFrame", "PriorAnimationFrame", "PlaySpeed", "LinkBone"};
constexpr const char* HashFields[] = {"state"};

void Record(Recorder& r, const PART_t* part, int type)
{
    Entry& e = r.Begin(part, "character part", PartFields, type);
    Put(e, static_cast<int>(part->Type));
    Put(e, static_cast<int>(part->Level));
    Put(e, static_cast<int>(part->CurrentAction));
    Put(e, static_cast<int>(part->PriorAction));
    Put(e, part->AnimationFrame);
    Put(e, part->PriorAnimationFrame);
    Put(e, part->PlaySpeed);
    Put(e, static_cast<int>(part->LinkBone));
}

void RecordHash(Recorder& r, const void* key, const char* kind, std::uint64_t value)
{
    Entry& e = r.Begin(key, kind, HashFields, 0);
    Put(e, value);
}

void RecordCloth(Recorder& r, const CPhysicsCloth* cloth)
{
    Fnv1aHash state;
    cloth->AddStateTo(state);
    RecordHash(r, cloth, "cloth", state.Value());
}

// The character's own state beside its object: its draw colour, its flags, what it targets, its parts and its cloth.
void Record(Recorder& r, const CHARACTER* c)
{
    Entry& e = r.Begin(&c->Light, "character state", CharacterFields, c->Object.Type);
    Put(e, c->Light);
    Put(e, c->HideShadow ? 1 : 0);
    Put(e, c->SafeZone ? 1 : 0);
    Put(e, static_cast<int>(c->TargetCharacter));
    const int type = c->Object.Type;
    for (const PART_t& part : c->BodyPart)
        Record(r, &part, type);
    for (const PART_t& part : c->Weapon)
        Record(r, &part, type);
    Record(r, &c->Wing, type);
    Record(r, &c->Helper, type);

    // The cloth the character owns, and the cloth each part owns.
    if (const auto* cloth = static_cast<const CPhysicsCloth*>(c->Object.m_pCloth))
    {
        for (int i = 0; i < c->Object.m_byNumCloth; ++i)
            RecordCloth(r, &cloth[i]);
    }
    const auto partCloth = [&r](const PART_t& part)
    {
        for (int i = 0; i < part.m_byNumCloth && i < 2; ++i)
        {
            if (part.m_pCloth[i] != nullptr)
                RecordCloth(r, static_cast<const CPhysicsCloth*>(part.m_pCloth[i]));
        }
    };
    for (const PART_t& part : c->BodyPart)
        partCloth(part);
    for (const PART_t& part : c->Weapon)
        partCloth(part);
    partCloth(c->Wing);
    partCloth(c->Helper);
}

// What the update keeps for the whole world: terrain collision, the frame's terrain light and light history, and
// Dark Side's skill state.
void RecordWorld(Recorder& r)
{
    constexpr int cells = TERRAIN_SIZE * TERRAIN_SIZE;
    Fnv1aHash walls;
    Fnv1aHash light;
    for (int i = 0; i < cells; ++i)
    {
        walls.Add(TerrainWall[i]);
        for (const float channel : PrimaryTerrainLight[i])
            light.Add(channel);
    }
    RecordHash(r, TerrainWall, "terrain walls", walls.Value());
    RecordHash(r, PrimaryTerrainLight, "terrain light", light.Value());
    RecordHash(r, &Render::Lighting::FrameLightHistory(), "light history",
               Render::Lighting::FrameLightHistory().StateHash());
    RecordHash(r, &g_CMonkSystem, "dark side", g_CMonkSystem.DarksideStateHash());
}

void Record(Recorder& r, const PARTICLE* p)
{
    Entry& e = r.Begin(p, "particle", ParticleFields, p->Type);
    Put(e, p->Live ? 1 : 0);
    Put(e, p->Type);
    Put(e, p->SubType);
    Put(e, p->Scale);
    Put(e, p->Position);
    Put(e, p->Angle);
    Put(e, p->Light);
    Put(e, p->Alpha);
    Put(e, p->LifeTime);
    Put(e, p->Rotation);
    Put(e, p->Frame);
    Put(e, p->Gravity);
    Put(e, p->Velocity);
}

void Record(Recorder& r, const JOINT* j)
{
    Entry& e = r.Begin(j, "joint", JointFields, j->Type);
    Put(e, j->Live ? 1 : 0);
    Put(e, j->Type);
    Put(e, j->SubType);
    Put(e, j->Scale);
    Put(e, j->Position);
    Put(e, j->Angle);
    Put(e, j->Light);
    Put(e, j->LifeTime);
    Put(e, j->Velocity);
    Put(e, j->Direction);
    Put(e, j->TargetPosition);
    Put(e, j->NumTails);
    Put(e, j->Weapon);
    Put(e, j->MultiUse);
}

constexpr const char* ViewFields[] = {
    "Position.x",   "Position.y",   "Position.z",    "Angle.x",       "Angle.y",           "Angle.z",
    "Matrix[0][0]", "Matrix[0][1]", "Matrix[0][2]",  "Matrix[0][3]",  "Matrix[1][0]",      "Matrix[1][1]",
    "Matrix[1][2]", "Matrix[1][3]", "Matrix[2][0]",  "Matrix[2][1]",  "Matrix[2][2]",      "Matrix[2][3]",
    "PerspectiveX", "PerspectiveY", "ScreenCenterX", "ScreenCenterY", "ScreenCenterYFlip", "FOV",
    "ViewNear",     "ViewFar",      "TopViewEnable"};
constexpr const char* FrustumFields[] = {"Count", "BoundMinX", "BoundMinY", "BoundMaxX", "BoundMaxY"};
constexpr const char* HullFields[] = {"0", "1", "2",  "3",  "4",  "5",  "6",  "7",
                                      "8", "9", "10", "11", "12", "13", "14", "15"};
constexpr const char* MouseRayFields[] = {"MouseTarget.x",   "MouseTarget.y",   "MouseTarget.z",
                                          "MousePosition.x", "MousePosition.y", "MousePosition.z"};

// The view the update built: the draw only reads it.
void RecordView(Recorder& r)
{
    Entry& view = r.Begin(&g_Camera, "view", ViewFields, 0);
    Put(view, g_Camera.Position);
    Put(view, g_Camera.Angle);
    for (const auto& row : g_Camera.Matrix)
    {
        for (const float value : row)
            Put(view, value);
    }
    Put(view, g_Camera.PerspectiveX);
    Put(view, g_Camera.PerspectiveY);
    Put(view, g_Camera.ScreenCenterX);
    Put(view, g_Camera.ScreenCenterY);
    Put(view, g_Camera.ScreenCenterYFlip);
    Put(view, g_Camera.FOV);
    Put(view, g_Camera.ViewNear);
    Put(view, g_Camera.ViewFar);
    Put(view, g_Camera.TopViewEnable ? 1 : 0);

    Entry& frustum = r.Begin(&FrustrumCount, "frustum", FrustumFields, 0);
    Put(frustum, FrustrumCount);
    Put(frustum, FrustrumBoundMinX);
    Put(frustum, FrustrumBoundMinY);
    Put(frustum, FrustrumBoundMaxX);
    Put(frustum, FrustrumBoundMaxY);
    Entry& hullX = r.Begin(FrustrumX, "frustum hull x", HullFields, 0);
    Entry& hullY = r.Begin(FrustrumY, "frustum hull y", HullFields, 0);
    for (int i = 0; i < 16; ++i)
    {
        Put(hullX, FrustrumX[i]);
        Put(hullY, FrustrumY[i]);
    }

    Entry& mouse = r.Begin(MouseTarget, "mouse ray", MouseRayFields, 0);
    Put(mouse, MouseTarget);
    Put(mouse, MousePosition);
}

void Snapshot(std::unordered_map<const void*, Entry>& into)
{
    into.clear();
    Recorder r(into);
    for (int i = 0; CharactersClient != nullptr && i < MAX_CHARACTERS_CLIENT; ++i)
    {
        Record(r, &CharactersClient[i].Object, "character");
        Record(r, &CharactersClient[i]);
    }
    Engine::Object::Attachments::ForEachTracked(
        [&r](const CHARACTER* preview)
        {
            Record(r, &preview->Object, "preview");
            Record(r, preview);
        });
    for (int i = 0; i < MAX_SPRITES; ++i)
        Record(r, &Sprites[i], "sprite");
    for (int i = 0; i < MAX_EFFECTS; ++i)
        Record(r, &Effects[i], "effect");
    for (int i = 0; i < g_SkillEffects.GetSize(); ++i)
        Record(r, g_SkillEffects.GetEffect(i), "effect");
    for (int i = 0; i < MAX_ITEMS; ++i)
        Record(r, &Items[i].Object, "item");
    for (int i = 0; i < MAX_MOUNTS; ++i)
        Record(r, &Mounts[i], "mount");
    for (int i = 0; i < MAX_BOIDS; ++i)
        Record(r, &Boids[i], "boid");
    for (int i = 0; i < MAX_FISHS; ++i)
        Record(r, &Fishs[i], "fish");
    for (const OBJECT_BLOCK& block : ObjectBlock)
    {
        for (const OBJECT* o = block.Head; o != nullptr; o = o->Next)
            Record(r, o, "map object");
    }
    for (int i = 0; i < MAX_PARTICLES; ++i)
        Record(r, &Particles[i]);
    for (int i = 0; i < MAX_JOINTS; ++i)
        Record(r, &Joints[i]);
    RecordView(r);
    RecordWorld(r);
}

bool s_listing = false;
int s_draws = 0;
std::unordered_map<const void*, Entry> s_before;
std::unordered_map<const void*, Entry> s_after;
std::map<std::string, int> s_counts;
// Both sequences are compared by their state; reading it draws nothing from them.
std::uint32_t s_randState = 0;
std::mt19937 s_engine;

void Count(const Entry& e, const char* field)
{
    ++s_counts[std::string(e.kind) + " " + std::to_string(e.type) + " " + field];
}
} // namespace

void BeforeDraw()
{
    if (!s_listing)
        return;
    Snapshot(s_before);
    s_randState = Random::RandState();
    s_engine = Random::Snapshot();
}

void AfterDraw()
{
    if (!s_listing)
        return;
    ++s_draws;
    if (Random::RandState() != s_randState)
        ++s_counts["random legacy sequence"];
    if (Random::Snapshot() != s_engine)
        ++s_counts["random engine"];
    Snapshot(s_after);
    for (const auto& [key, after] : s_after)
    {
        const auto found = s_before.find(key);
        if (found == s_before.end())
        {
            Count(after, "added");
            continue;
        }
        const Entry& before = found->second;
        for (int f = 0; f < after.count; ++f)
        {
            if (before.values[f] != after.values[f])
                Count(before, after.names[f]);
        }
    }
    for (const auto& [key, before] : s_before)
    {
        if (!s_after.contains(key))
            Count(before, "removed");
    }
}

void Start()
{
    s_counts.clear();
    s_draws = 0;
    s_listing = true;
}

std::vector<Change> Report(int& draws)
{
    draws = s_draws;
    std::vector<Change> changes;
    for (const auto& [what, count] : s_counts)
        changes.push_back({what, count});
    std::ranges::sort(changes, [](const Change& a, const Change& b) { return a.count > b.count; });
    return changes;
}
} // namespace Diagnostics::DrawPurityCheck

#endif
