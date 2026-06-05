#ifndef __ZZZINTERFACE_H__
#define __ZZZINTERFACE_H__

#define TRADELIMITLEVEL		( 6 )

extern bool WhisperEnable;

extern bool LockInputStatus;

extern bool g_bAutoGetItem;

extern bool g_bRenderGameCursor;

extern bool InputEnable;
extern bool TabInputEnable;
extern bool GuildInputEnable;
extern bool GoldInputEnable;
extern bool g_bScratchTicket;

extern int  InputGold;
extern int  InputResidentNumber;
extern int  InputNumber;
extern int  InputIndex;
extern int  InputTextWidth;
extern int  InputTextMax[12];
extern wchar_t InputText[12][256];
extern wchar_t InputTextIME[12][4];
extern int  InputLength[12];
extern char InputTextHide[12];
extern wchar_t MacroText[10][256];
extern uint64_t  RemainingMacroCooldownTime;
extern int  FontHeight;
extern int  ItemHelp;
extern bool DontMove;
extern int  EditFlag;
extern int  SelectMonster;
extern int  SelectModel;
extern int  SelectMapping;
extern bool DebugEnable;
extern bool MouseOnWindow;
extern int  MouseUpdateTime;
extern int  MouseUpdateTimeMax;
extern int  TalkNPC;
extern int  SelectedItem;
extern int  SelectedNpc;
extern int  SelectedCharacter;
extern int  SelectedOperate;
extern int  Attacking;
extern int  ActionTarget;

extern int   g_iFollowCharacter;
extern int  HeroTile;
extern int  LoadingWorld;

extern int ItemKey;
extern bool g_bGMObservation;

void Action(CHARACTER* c, OBJECT* o, bool Now);
void SendRequestAction(OBJECT& obj, BYTE action);

bool PressKey(int Key);
void MoveHero();

void ClearInput(BOOL bClearWhisperTarget = TRUE);
void CutText(const wchar_t* Text, wchar_t* Text1, wchar_t* Text2, size_t MaxLength);
void RenderSwichState();


//  HotKey
int  FindHotKey(int Skill);

void RenderInputText(int x, int y, int Index, int Hide = 0);
void RenderDebugWindow();
void RenderTipText(int sx, int sy, const wchar_t* Text);

extern int g_iWidthEx;

void RenderInterface(bool);
void MoveInterface();
void RenderCursor();
bool CheckArrow();
void ReloadArrow();
int SearchArrowCount();

bool FindText(const wchar_t* Text, const wchar_t* Token, bool First = false);
bool FindTextABS(const wchar_t* Text, const wchar_t* Token, bool First = false);
bool CheckAbuseFilter(wchar_t* Text, bool bCheckSlash = true);
bool CheckAbuseNameFilter(wchar_t* Text);
bool CheckCommand(wchar_t* Text, bool bMacroText = false);
void SetActionClass(CHARACTER* c, OBJECT* o, int Action, int ActionType);
void RenderBar(float x, float y, float Width, float Height, float Bar, bool Disabled = false, bool clipping = true);
void RenderOutSides();
void RenderTimes();
void RenderPartyHP();

void SendMove(CHARACTER* c, OBJECT* o);
void SetCharacterPos(CHARACTER* c, BYTE posX, BYTE posY, vec3_t position);

bool CheckAttack();
bool CheckAttack_Fenrir(CHARACTER* c);
int	 getTargetCharacterKey(CHARACTER* c, int selected);

void MoveTournamentInterface();
void MoveBattleSoccerEffect(CHARACTER* c);
void RenderTournamentInterface();

void GetTime(DWORD time, std::wstring& timeText, bool isSecond = true);

extern int   TargetNpc;

// skill.
bool IsCanBCSkill(int nType);
bool CheckSkillUseCondition(OBJECT* o, int Type);
bool CheckWall(int sx1, int sy1, int sx2, int sy2);
bool CheckTile(CHARACTER* c, OBJECT* o, float Range);
void LetHeroStop(CHARACTER* c = NULL, BOOL bSetMovementFalse = FALSE);
void SendCharacterMove(unsigned short Key, float Angle, unsigned char PathNum, unsigned char* PathX, unsigned char* PathY, unsigned char TargetX, unsigned char TargetY);
void Attack(CHARACTER* c);
void SendRequestMagic(int Type, int Key);
void SendRequestMagicContinue(int Type, int x, int y, int Angle, BYTE Dest, BYTE Tpos, WORD TKey, BYTE* pSkillSerial);
BYTE GetDestValue(int xPos, int yPos, int xDst, int yDst);

bool IsGMCharacter();
bool IsNonAttackGM();
bool IsIllegalMovementByUsingMsg(const wchar_t* szChatText);

#endif //__ZZZINTERFACE_H__
