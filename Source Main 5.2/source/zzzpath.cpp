#include "stdafx.h"
#include "zzzpath.h"

#ifdef CSK_DEBUG_MAP_PATHFINDING
bool g_bShowPath = false;
#endif // CSK_DEBUG_MAP_PATHFINDING

int PATH::s_iDir[8][2] =
{
    { -1, -1},	{ 0, -1},	{ 1, -1},
    { -1, 0},				{ 1, 0},
    { -1, 1},	{ 0, 1},	{ 1, 1}
};
