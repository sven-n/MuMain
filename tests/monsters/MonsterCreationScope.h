#pragma once

#include "MonsterModelRecorder.h"

#include <memory>

class CHARACTER;

// The monster recorder's creation part: what CreateMonster needs, as the game
// sets it up: a character table with the hero in it, the map process, and
// the model slots of a LoadScope. The models a creation opens stay open for
// the scope, so creating the monsters of many maps opens each model once.
namespace MonsterRecorder
{
class CreationScope
{
public:
    explicit CreationScope(int world);
    ~CreationScope();

    CreationScope(const CreationScope&) = delete;
    CreationScope& operator=(const CreationScope&) = delete;

    void SetWorld(int world);

    // Creates the monster with CreateMonster on the world that is set, and
    // returns the character's model slot. The character is gone afterwards:
    // the next creation finds its slot free.
    int CreatedModel(int number);

    // Creates the monster with CreateMonster on the world that is set and
    // returns the character, which lives until the next Create or
    // CreatedModel; nullptr when it was not created.
    CHARACTER* Create(int number);

    LoadScope& Load()
    {
        return *m_load;
    }

private:
    struct State;
    std::unique_ptr<LoadScope> m_load;
    std::unique_ptr<State> m_state;
};
} // namespace MonsterRecorder
