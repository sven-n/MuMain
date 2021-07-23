//////////////////////////////////////////////////////////////////////////
//  
//  CSChaosCastle.h
//
//  내  용 : 카오스 캐슬 경기.
//
//  날  짜 : 2004/04/22
//
//  작성자 : 조 규 하.
//
//////////////////////////////////////////////////////////////////////////
#ifndef __CSCHAOS_CASTLE_H__
#define __CSCHAOS_CASTLE_H__

//////////////////////////////////////////////////////////////////////////
//  FUNCTION.
//////////////////////////////////////////////////////////////////////////
bool InChaosCastle(int iMap = World);
bool InBloodCastle(int iMap = World);
bool InDevilSquare();


void    ClearChaosCastleHelper ( CHARACTER* c );
void    ChangeChaosCastleUnit ( CHARACTER* c );
bool    CreateChaosCastleObject ( OBJECT* o );
bool    MoveChaosCastleObjectSetting ( int& objCount, int object );
bool    MoveChaosCastleObject ( OBJECT* o, int& object, int& visibleObject );
bool    MoveChaosCastleAllObject ( OBJECT* o );
bool    RenderChaosCastleVisual ( OBJECT* o, BMD* b );
void    RenderTerrainVisual ( int xi, int yi );

#endif// __CSCHAOS_CASTLE_H__


