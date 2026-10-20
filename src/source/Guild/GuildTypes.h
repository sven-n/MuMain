//////////////////////////////////////////////////////////////////////////
//  GuildTypes.h
//  The guild values the game reads off a character, extracted from the retired
//  UIGuildInfo/UIGuildMaster widgets so they stop carrying a UI dependency.
//////////////////////////////////////////////////////////////////////////

#pragma once

// Hero->GuildStatus. The server sends these, so the values are protocol, not presentation.
enum GUILD_STATUS
{
    G_NONE = (BYTE)-1,
    G_PERSON = 0,
    G_MASTER = 128,
    G_SUB_MASTER = 64,
    G_BATTLE_MASTER = 32
};

enum GUILD_TYPE
{
    GT_NORMAL = 0x00,
    GT_ANGEL = 0x01
};

// Duplicated, with different spellings, by GuildConstants::RelationshipType in GuildConstants.h --
// same five values. Folding the two together is a separate tidy-up; doing it here would have
// churned the call sites of a header this change is only moving.
enum GUILD_RELATIONSHIP
{
    GR_NONE = 0x00,
    GR_UNION = 0x01,
    GR_UNIONMASTER = 0x04,
    GR_RIVAL = 0x02,
    GR_RIVALUNION = 0x08
};
