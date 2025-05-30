#ifndef __ZZZINTERFACE_H__
#define __ZZZINTERFACE_H__

#define TRADELIMITLEVEL		( 6 )

extern bool WhisperEnable;

extern bool LockInputStatus;

extern bool g_bAutoGetItem;

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
extern int  SelectedHero;
extern int  Attacking;

extern int   g_iFollowCharacter;
extern int  HeroTile;
extern int  LoadingWorld;

extern int ItemKey;
extern bool g_bGMObservation;

void Action(CHARACTER* c, OBJECT* o, bool Now);
void SendRequestAction(OBJECT& obj, BYTE action);

bool PressKey(int Key);
void MoveHero();
void EditObjects();

void ClearInput(BOOL bClearWhisperTarget = TRUE);
void CutText(const wchar_t* Text, wchar_t* Text1, wchar_t* Text2, size_t MaxLength);
void CreateChat(wchar_t* ID, const wchar_t* Text, CHARACTER* c, int Flag = 0, int SetColor = -1);
int  CreateChat(wchar_t* character_name, const wchar_t* chat_text, OBJECT* Owner, int Flag = 0, int SetColor = -1);
void AssignChat(wchar_t* ID, const wchar_t* Text, int Flag = 0);
void MoveChat();
void ClearNotice(void);
void CreateNotice(wchar_t* Text, int Color);
void MoveNotices();
void RenderNotices();
void RenderSwichState();
void CheckChatText(wchar_t* Text);

//  Whisper
bool CheckWhisperLevel(int lvl, wchar_t* text);
void RegistWhisperID(int lvl, wchar_t* text);
void ClearWhisperID(void);

void SaveIME_Status();
void SetIME_Status(bool halfShape);
bool CheckIME_Status(bool change, int mode);
void RenderIME_Status();

//  HotKey
int  FindHotKey(int Skill);

void RenderInputText(int x, int y, int Index, int Hide = 0);
void RenderDebugWindow();
void RenderTipText(int sx, int sy, const wchar_t* Text);

extern int g_iWidthEx;

void SelectObjects();
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
void SetPlayerColor(BYTE PK);
bool CheckCommand(wchar_t* Text, bool bMacroText = false);
void SetActionClass(CHARACTER* c, OBJECT* o, int Action, int ActionType);
void RenderBar(float x, float y, float Width, float Height, float Bar, bool Disabled = false, bool clipping = true);
void RenderOutSides();
void RenderBooleans();
void RenderTimes();
void RenderPartyHP();
void SetPositionIME_Wnd(float x, float y);

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
int ExecuteSkill(CHARACTER* c, ActionSkillType Skill, float Distance);
bool ExecuteSkillComplete(CHARACTER* c);
bool CheckMana(CHARACTER* c, int Skill);
bool SkillElf(CHARACTER* c, ITEM* p);
void SendRequestMagic(int Type, int Key);
void SendRequestMagicContinue(int Type, int x, int y, int Angle, BYTE Dest, BYTE Tpos, WORD TKey, BYTE* pSkillSerial);
bool SkillWarrior(CHARACTER* c, ITEM* p);
void UseSkillWarrior(CHARACTER* c, OBJECT* o);
void UseSkillWizard(CHARACTER* c, OBJECT* o);
void UseSkillElf(CHARACTER* c, OBJECT* o);
void UseSkillSummon(CHARACTER* pCha, OBJECT* pObj);
void UseSkillRagefighter(CHARACTER* pCha, OBJECT* pObj);
void AttackRagefighter(CHARACTER* pCha, int nSkill, float fDistance);
bool UseSkillRagePosition(CHARACTER* pCha);
bool CheckTarget(CHARACTER* c);
void MouseRButtonReset();
void AttackKnight(CHARACTER* c, ActionSkillType Skill, float Distance);

bool IsGMCharacter();
bool IsNonAttackGM();
bool IsIllegalMovementByUsingMsg(const wchar_t* szChatText);

#endif //__ZZZINTERFACE_H__
