#include "stdafx.h"

#include "DrawPurityCheck.h"

#ifdef MU_ENABLE_CONTROL_SOCKET

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Skills/SkillEffectMgr.h"
#include "Render/Effects/ZzzEffect.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

namespace Diagnostics::DrawPurityCheck
{
namespace
{
constexpr int MaxFields = 32;

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

constexpr const char* ObjectFields[] = {
    "Live",       "Type",       "SubType",      "CurrentAction", "PriorAction", "AnimationFrame", "PriorAnimationFrame",
    "LifeTime",   "Position.x", "Position.y",   "Position.z",    "Angle.x",     "Angle.y",        "Angle.z",
    "Scale",      "Alpha",      "Light.r",      "Light.g",       "Light.b",     "Velocity",       "Gravity",
    "HiddenMesh", "PKKey",      "m_iAnimation", "Owner",         "Direction.x", "Direction.y",    "Direction.z"};
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

void Snapshot(std::unordered_map<const void*, Entry>& into)
{
    into.clear();
    Recorder r(into);
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
        Record(r, &CharactersClient[i].Object, "character");
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
}

bool s_listing = false;
int s_draws = 0;
std::unordered_map<const void*, Entry> s_before;
std::unordered_map<const void*, Entry> s_after;
std::map<std::string, int> s_counts;

void Count(const Entry& e, const char* field)
{
    ++s_counts[std::string(e.kind) + " " + std::to_string(e.type) + " " + field];
}
} // namespace

void BeforeDraw()
{
    if (s_listing)
        Snapshot(s_before);
}

void AfterDraw()
{
    if (!s_listing)
        return;
    ++s_draws;
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
