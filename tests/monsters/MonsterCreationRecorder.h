#pragma once

#include "MonsterCreationScope.h"
#include "MonsterModelRecorder.h"

#include <string>
#include <vector>

// The monster recorder's creation part (monsters design, section 10, D50):
// what CreateMonster writes, recorded so that moving a creator's values from
// code into the monster data can be compared with the old code, case by case
// in one build.
//
// A record holds every field of the character (CHARACTER with its Object,
// parts, path and pet information) that differs from before the creation,
// the named bones it registered, the effects, joints and particles it
// created, the objects it placed beside it, the other characters it brought
// to life and the sounds it played.
namespace MonsterRecorder
{
// How the character slot the monster is created in starts.
enum class CreationSlot
{
    // A slot no character has used.
    Clean,
    // A free slot whose fields all hold values of their own, as another
    // character leaves it: a field the creation does not write shows up.
    Dirty,
    // The slot of a living character with the key the monster comes with
    // (CreateCharacter's same-key path).
    SameKey,
};

const char* Describe(CreationSlot slot);

// The slot the recorder creates in (the first after the hero), prepared as
// `slot` says, with the other characters after it freed and the named bones
// dropped; the next creation takes it. A same-key slot has the key.
CHARACTER& PrepareCreationSlot(CreationSlot slot, int key = CreationScope::TestMonsterKey);

// Creates the monster with CreateMonster on the scope's world in the first
// free slot after the hero, prepared as `slot` says, and records it.
Record RecordCreation(CreationScope& scope, int number, CreationSlot slot);
// The same with the random draws seeded by `seed`: a random look shows as a
// record that changes with it.
Record RecordCreation(CreationScope& scope, int number, CreationSlot slot, unsigned seed);
// The same by the path: through a packet's handler or a cutscene.
Record RecordCreation(CreationScope& scope, int number, CreationSlot slot, unsigned seed, CreationPath path);

// Runs one frame on screen of the character's visual move
// (MoveCharacterVisual) with the random draws seeded by `seed`, and records
// the effects, joints and particles it made and the sounds it played: what a
// monster brings at its first frame (the spawn effects at bones).
Record RecordVisualFrame(CreationScope& scope, CHARACTER& character, unsigned seed);

// "path = value" lines.
std::string ToText(const Record& record);

} // namespace MonsterRecorder
