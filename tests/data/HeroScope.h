#pragma once

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"

// Points the game at a test-owned hero (CharacterAttribute and ::Hero) and puts the previous one back when it goes:
// the item help window marks the levels its attributes can equip, the tooltip compares its class and stats, and
// options such as the excellent damage level and the Dark Horse's defense come from its level and agility.
class HeroScope
{
public:
    HeroScope() : m_previousHero(CharacterAttribute), m_previousHeroCharacter(::Hero)
    {
        CharacterAttribute = &m_hero;
        ::Hero = &m_heroCharacter;
    }

    ~HeroScope()
    {
        CharacterAttribute = m_previousHero;
        ::Hero = m_previousHeroCharacter;
    }

    HeroScope(const HeroScope&) = delete;
    HeroScope& operator=(const HeroScope&) = delete;

    CHARACTER_ATTRIBUTE& Hero()
    {
        return m_hero;
    }

    CHARACTER& HeroCharacter()
    {
        return m_heroCharacter;
    }

private:
    CHARACTER_ATTRIBUTE m_hero{};
    CHARACTER m_heroCharacter;
    CHARACTER_ATTRIBUTE* m_previousHero;
    CHARACTER* m_previousHeroCharacter;
};

// Points the game at no hero (CharacterAttribute is null), as in an editor preview or before entering the world, and
// puts the previous one back when it goes.
class NoHeroScope
{
public:
    NoHeroScope() : m_previousHero(CharacterAttribute)
    {
        CharacterAttribute = nullptr;
    }

    ~NoHeroScope()
    {
        CharacterAttribute = m_previousHero;
    }

    NoHeroScope(const NoHeroScope&) = delete;
    NoHeroScope& operator=(const NoHeroScope&) = delete;

private:
    CHARACTER_ATTRIBUTE* m_previousHero;
};
