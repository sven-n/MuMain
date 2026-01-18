#include "UIManager.h"

typedef std::pair<int, BYTE> INTBYTEPAIR;

namespace COMGEM
{
    extern	BOOL	m_bType;
    extern	char	m_cGemType;
    extern	char	m_cComType;
    extern	BYTE	m_cCount;
    extern	int		m_iValue;
    extern	BYTE	m_cPercent;
    extern	char	m_cState;
    extern	char	m_cErr;
    extern	CUIUnmixgemList	m_UnmixTarList;

    void	Init();
    void	Exit();
    void	GetBack();
    void	RenderUnMixList();
    void	MoveUnMixList();
    void	CalcGen();
    int		CalcItemValue(const ITEM*);
    char	CalcCompiledCount(const ITEM*);
    int		CalcEmptyInv();
    char	CheckOneItem(const ITEM*);
    bool	CheckMyInvValid();
    bool	CheckInv();

    bool	FindWantedList();
    void	ResetWantedList();
    void	SelectFromList(int iIndex, int iLevel);
    int		GetUnMixGemLevel();

    void	SendReqMix();
    void	SendReqUnMix();
    void	ProcessCSAction();

    int		GetJewelRequireCount(int i);
    int		GetJewelIndex(int _nJewel, int _nType);
    int		Check_Jewel(int _nJewel, int _nType = 0, bool _bModel = false);

    inline	void	SetMode(BOOL mode) { m_bType = mode; }
    inline	void	SetGem(char gem) { m_cGemType = gem; }
    inline	void	SetComType(char type) { m_cComType = type; }
    inline	void	SetState(char state) { m_cState = state; }
    inline	void	SetError(char err) { m_cErr = err; }
    inline	char	GetError() { return m_cErr; }
    inline	bool	isComMode() { return (m_bType == ATTACH); }

    __inline int	Check_Jewel_Unit(int _nJewel, bool _bModel = false) { return Check_Jewel(_nJewel, 1, _bModel); }
    __inline int	Check_Jewel_Com(int _nJewel, bool _bModel = false) { return Check_Jewel(_nJewel, 2, _bModel); }
    __inline bool	isCompiledGem(const ITEM* p) { return (Check_Jewel_Com(p->Type) != NOGEM ? true : false); }
    inline	bool	isAble() { return (m_cState == STATE_READY); }
}
