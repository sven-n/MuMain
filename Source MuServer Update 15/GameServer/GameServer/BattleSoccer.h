// BattleSoccer.h: interface for the CBattleSoccer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "BattleGround.h"
#include "User.h"

#define MAX_BATTLESOCCER_GROUND 1

class CBattleSoccer : public CBattleGround
{
public:
	CBattleSoccer();
	virtual ~CBattleSoccer();
	void SetGoalMove(int set_code);
	int GetGoalMove();
	void SetGoalLeftRect(int x, int y, int xl, int yl);
	void SetGoalRightRect(int x, int y, int xl, int yl);
public:
	int m_goalmove;	// 60 - d
	int m_BallIndex;	// 64 - D
	RECT m_GoalLeft;	// 68 - D
	RECT m_GoalRight;	// 78 - D
	GUILD_INFO_STRUCT* m_RedTeam;	// 88 - D
	GUILD_INFO_STRUCT* m_BlueTeam;	// 8C - D
};

extern CBattleSoccer* gBSGround[MAX_BATTLESOCCER_GROUND];
