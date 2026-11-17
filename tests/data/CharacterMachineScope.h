#pragma once

#include "Engine/Object/ZzzInfomation.h"

#include <memory>

// A character machine of the test's own (CharacterMachine) with nothing worn, for the scope: what the stat code reads
// as the hero's equipment. The previous machine is back when the scope goes.
class CharacterMachineScope
{
public:
    CharacterMachineScope() : m_previous(CharacterMachine), m_machine(std::make_unique<CHARACTER_MACHINE>())
    {
        for (ITEM& equipped : m_machine->Equipment)
        {
            equipped = ITEM{};
            equipped.Type = -1;
        }
        CharacterMachine = m_machine.get();
    }

    ~CharacterMachineScope()
    {
        CharacterMachine = m_previous;
    }

    CharacterMachineScope(const CharacterMachineScope&) = delete;
    CharacterMachineScope& operator=(const CharacterMachineScope&) = delete;

    CHARACTER_MACHINE& Machine()
    {
        return *m_machine;
    }

private:
    CHARACTER_MACHINE* m_previous;
    std::unique_ptr<CHARACTER_MACHINE> m_machine;
};
