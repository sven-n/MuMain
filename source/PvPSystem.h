using namespace std;

#ifdef _PVP_BLOCK_PVP_CHAR
bool CanJoinServer(int PK);
#endif	// _PVP_BLOCK_PVP_CHAR

#ifdef _PVP_ADD_MOVE_SCROLL
const int g_ciMurderMoveDelay = 3;

class CMurdererMove
{
public:
	CMurdererMove() { Reset(); }
	virtual ~CMurdererMove() {}
	void Reset();
	void MurdererMove(char * pszMoveText);
	void MurdererMoveByScroll(int iItemIndex);
	void MurdererMoveCheck();
	int CanMove() { return (m_iCheckTime == g_ciMurderMoveDelay); }
	bool CanWalk() { return (m_szMoveText[0] == '\0' && m_iItemIndex == -1); }

	void CancelMove();

protected:
	char m_szMoveText[256];
	int m_iItemIndex;
	DWORD m_dwTimer;
	int m_iCheckTime;
};
#endif	// _PVP_ADD_MOVE_SCROLL

