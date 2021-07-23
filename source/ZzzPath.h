
#ifdef CSK_DEBUG_MAP_PATHFINDING
	#define SHOW_PATH_INFO
	extern bool g_bShowPath;
#endif // CSK_DEBUG_MAP_PATHFINDING

#ifdef _DEBUG
	//#define SAVE_PATH_TIME
#endif

#include <math.h>
#include "BaseCls.h"
#include "./Utilities/Log/ErrorReport.h"

class PATH
{
public:
	PATH();
	~PATH();

	// 초기화
public:
    void SetMapDimensions( int iWidth, int iHeight, WORD *pbyMap);

	// 맵 정보
private:
	int	m_iWidth, m_iHeight;
	int m_iSize;
	WORD* m_pbyMap;

	// 찾은 길 정보
private:
	int m_iNumPath;
	BYTE m_xPath[MAX_COUNT_PATH];
	BYTE m_yPath[MAX_COUNT_PATH];
public:
	int GetPath( void)	{ return ( min( m_iNumPath, MAX_PATH_FIND)); }
	BYTE* GetPathX( void)	{ return ( m_xPath + MAX_COUNT_PATH - m_iNumPath); }
	BYTE* GetPathY( void)	{ return ( m_yPath + MAX_COUNT_PATH - m_iNumPath); }

	// 데이터 접근
	int GetIndex( int xPos, int yPos)	{ return ( xPos + yPos * m_iWidth); }
	void GetXYPos( int iIndex, int *pxPos, int *pyPos)	{ *pxPos = iIndex % m_iWidth; *pyPos = iIndex / m_iWidth; }
	BOOL CheckXYPos( int xPos, int yPos)	{ return ( xPos >= 0 && yPos >= 0 && xPos < m_iWidth && yPos < m_iHeight);}
	static int s_iDir[8][2];

	// 길찾기 내부 정보
private:
	BYTE *m_pbyClosed;	                // 열렸는가? 닫혔는가? ( 0 : 열린 곳, 1 : 닫힌 곳)
	int m_iMinClosed, m_iMaxClosed;	    // m_pbyClosed 를 초기화할 영역
	int *m_piCostToStart;		        // 도착점까지의 비용
	int *m_pxPrev;		                // 바로 직전 노드
	int *m_pyPrev;
	CBTree<int, int> m_btOpenNodes;	    // 검사해볼 것들의 리스트 ( 인덱스, 예상 총비용)

	void Clear( void);
	bool AddClearPos( int iIndex);
	void Init( void);
	int GetNewNodeToTest( void);

	// 길 찾기
public:
	bool FindPath(int xStart, int yStart, int xEnd, int yEnd, bool bErrorCheck, int iWall, bool Value, float fDistance = 0.0f);

private:
	void SetEndNodes( bool bErrorCheck, int iWall, int xEnd, int yEnd, float fDistance);
	int CalculateCostToStartAddition( int xDir, int yDir);
	int EstimateCostToGoal( int xStart, int yStart, int xNew, int yNew);
	bool GeneratePath( int xStart, int yStart, int xEnd, int yEnd);

	// 디버그용 함수
#ifdef SHOW_PATH_INFO
public:
	BYTE GetClosedStatus( int iIndex)	{ return ( m_pbyClosed[iIndex]); }
#endif // SHOW_PATH_INFO

};


inline PATH::PATH()
{
	m_pbyClosed = NULL;
	m_piCostToStart = NULL;
	m_pxPrev = NULL;
	m_pyPrev = NULL;
}

inline PATH::~PATH()
{
	Clear();
}

inline void PATH::Clear( void)
{
	if ( m_pbyClosed)
	{
		delete [] m_pbyClosed;
		m_pbyClosed = NULL;
	}
	if ( m_piCostToStart)
	{
		delete [] m_piCostToStart;
		m_piCostToStart = NULL;
	}
	if ( m_pxPrev)
	{
		delete [] m_pxPrev;
		m_pxPrev = NULL;
	}
	if ( m_pyPrev)
	{
		delete [] m_pyPrev;
		m_pyPrev = NULL;
	}
	m_iMinClosed = MAX_INT_FORPATH;
	m_iMaxClosed = -1;
}

inline void PATH::SetMapDimensions( int iWidth, int iHeight, WORD* pbyMap)
{
	Clear();

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_pbyMap = pbyMap;
	m_iSize  = m_iWidth*m_iHeight;

	m_pbyClosed = new BYTE [m_iSize];
	m_piCostToStart = new int [m_iSize];
	m_pxPrev = new int [m_iSize];
	m_pyPrev = new int [m_iSize];
	ZeroMemory( m_pbyClosed, m_iSize* sizeof ( BYTE));
}

inline bool PATH::AddClearPos( int iIndex)
{
	if(iIndex < 0 || iIndex >= m_iSize)
	{
		return false;
	}

	m_iMinClosed = min( iIndex, m_iMinClosed);
	m_iMaxClosed = max( iIndex, m_iMaxClosed);

	return true;
}

inline void PATH::Init( void)
{
	if(MAX_INT_FORPATH == m_iMinClosed)
	{
		return;
	}

	ZeroMemory( &( m_pbyClosed[m_iMinClosed]), ( m_iMaxClosed - m_iMinClosed + 1) * sizeof ( BYTE));
	m_iMinClosed = MAX_INT_FORPATH;
	m_iMaxClosed = -1;
}

inline int PATH::GetNewNodeToTest( void)
{	
	// 새로 검사할 노드를 얻어온다.
	CBNode<int, int> *pResult = NULL;
	CBNode<int, int> *pNode = m_btOpenNodes.FindHead();
	while ( pNode)
	{
		pResult = pNode;
		pNode = m_btOpenNodes.GetLeft( pNode);
	}

#if _DEBUG
	if ( !pResult)
	{
		return ( -1);
	}
#endif // _DEBUG

	return ( m_btOpenNodes.RemoveNode( pResult));
}

// bErrorCheck 가 false 이면 가까운데 까지라도 간다.
inline bool PATH::FindPath( int xStart, int yStart, int xEnd, int yEnd, bool bErrorCheck, int iWall, bool Value, float fDistance)
{	
	Init();

	// 1. 잘못된 호출 체크
	if(xStart == 0 || yStart == 0)
	{
		return false;
	}

	// 2. 끝점이 갈 수 있는 곳인지 체크
	if(0.0f == fDistance)
	{
		int iEndIndex = GetIndex(xEnd, yEnd);
		if(iEndIndex < 0 || iEndIndex >= m_iSize)	//. 인덱스가 범위 밖이라면 실패!
		{
			return false;
		}
		
		if(Value == true)
		{
			m_pbyMap[iEndIndex] = 0;
		}

		if(bErrorCheck && (iWall <= m_pbyMap[iEndIndex] && (m_pbyMap[GetIndex(xEnd, yEnd)]&TW_ACTION) != TW_ACTION))
		{
			return false;
		}

		m_pbyClosed[iEndIndex] = PATH_END;
		if(!AddClearPos(iEndIndex))
		{
			return false;
		}
	}
	else
	{
		SetEndNodes( bErrorCheck, iWall, xEnd, yEnd, fDistance);
	}

	// 갈 수 없을 경우 가장 가까운 곳까지라도 가게 하기 위한 변수
	int iCostToGoalOfNearest = MAX_INT_FORPATH;
	int xNearest = xStart;
	int yNearest = yStart;

	// 3. 시작점은 비용이 0 인 것으로 추가한다.
	int iStartIndex = GetIndex( xStart, yStart);
	if (iStartIndex < 0 || iStartIndex >= m_iSize)		//. 인덱스가 범위 밖이라면 실패!
	{
		return false;
	}

	m_btOpenNodes.Add( iStartIndex, 0);
	m_pbyClosed[iStartIndex] |= PATH_INTESTLIST;
	if(!AddClearPos( iStartIndex))
	{
		return false;
	}

	// 4. 하나씩 검사해보면서 그 주변의 것들을 검색 리스트에 넣는다.
	int iMaxCount = bErrorCheck ? 500 : 50;
	for(int iCheckCount = iMaxCount; 0 < m_btOpenNodes.GetCount() && iCheckCount > 0; --iCheckCount)
	{
		// a) 새로 검사할 위치 얻기
		int xTest, yTest;
		int iIndex = GetNewNodeToTest();
		GetXYPos( iIndex, &xTest, &yTest);

		// b) 실제 m_piCostToStart 계산
		m_piCostToStart[iIndex] = ( iCheckCount == iMaxCount) ? 0 : MAX_INT_FORPATH;
		for ( int i = 0; i < 8; i++)
		{	// 주변에서 더 가까운 값이 있으면 그 값으로 세팅
			int xNear = xTest + s_iDir[i][0];
			int yNear = yTest + s_iDir[i][1];
			if ( !CheckXYPos( xNear, yNear))
			{
				continue;
			}
			int iNearIndex = GetIndex( xNear, yNear);
			if ( PATH_TESTED & m_pbyClosed[iNearIndex])
			{
				// ** CostToStart 계산 **
				int iNewCost = m_piCostToStart[iNearIndex] + CalculateCostToStartAddition( s_iDir[i][0], s_iDir[i][1]);
				if ( iNewCost < m_piCostToStart[iIndex])
				{
					m_piCostToStart[iIndex] = iNewCost;
					m_pxPrev[iIndex] = xNear;
					m_pyPrev[iIndex] = yNear;
				}
			}
		}
		m_pbyClosed[iIndex] |= PATH_TESTED;
		// c) 찾은 경우
		if ( PATH_END & m_pbyClosed[iIndex])
		{
			m_btOpenNodes.RemoveAll();
			return ( GeneratePath( xStart, yStart, xTest, yTest));
		}
		// d) 주변의 8 방향을 검색 리스트에 추가
#ifdef _VS2008PORTING
		for (int i = 0; i < 8; i++)
#else // _VS2008PORTING
		for ( i = 0; i < 8; i++)
#endif // _VS2008PORTING
		{
			int xNew = xTest + s_iDir[i][0];
			int yNew = yTest + s_iDir[i][1];
			if ( !CheckXYPos( xNew, yNew))
			{
				continue;
			}
			int iNewIndex = GetIndex( xNew, yNew);
            int byMapAttribute = m_pbyMap[iNewIndex];

            if ( (byMapAttribute&TW_ACTION)==TW_ACTION ) byMapAttribute -= TW_ACTION;
            if ( (byMapAttribute&TW_HEIGHT)==TW_HEIGHT ) byMapAttribute -= TW_HEIGHT;
            if ( (byMapAttribute&TW_CAMERA_UP)==TW_CAMERA_UP ) byMapAttribute -= TW_CAMERA_UP;

			if ( !( PATH_INTESTLIST & m_pbyClosed[iNewIndex]) && iWall > byMapAttribute )
			{
				// ** 예상 총비용 계산 **
				int iNewCost = m_piCostToStart[iIndex] + EstimateCostToGoal( xEnd, yEnd, xNew, yNew);
				m_btOpenNodes.Add( iNewIndex, iNewCost);
				m_pbyClosed[iNewIndex] |= PATH_INTESTLIST;
				if(!AddClearPos( iNewIndex))
					return false;
				m_pxPrev[iNewIndex] = xTest;
				m_pyPrev[iNewIndex] = yTest;
			}
		}
		// e) 완료
		if ( !bErrorCheck)
		{	// 근처까지 가도 되는 경우, 가장 가까운 위치인지 체크
			int iCostToGoal = EstimateCostToGoal( xEnd, yEnd, xTest, yTest);
			if ( iCostToGoal < iCostToGoalOfNearest)
			{
				iCostToGoalOfNearest = iCostToGoal;
				xNearest = xTest;
				yNearest = yTest;
			}
		}
	}
	// 5. 가까운 곳까지라도 간다.
	if(!bErrorCheck)
	{
		m_btOpenNodes.RemoveAll();
		return ( GeneratePath( xStart, yStart, xNearest, yNearest));
	}

	// 6. 찾을 수 없는 경우 실패
	m_btOpenNodes.RemoveAll();

	return false;
}

inline void PATH::SetEndNodes( bool bErrorCheck, int iWall, int xEnd, int yEnd, float fDistance)
{
	int iDistance = ( int)fDistance;
	for ( int j = -iDistance; j <= iDistance; j++)
	{
		// 원 내부의 마름모 부분은 무조건 세팅
		int xRange = iDistance - abs( j);
		for ( int i = -xRange; i <= 0; i++)
		{
			int iEndIndex = GetIndex( xEnd + i, yEnd + j);
			if ( iEndIndex>=0 && iEndIndex<(m_iSize) )
			{
				if ( !( bErrorCheck && iWall <= m_pbyMap[iEndIndex]))
				{
					m_pbyClosed[iEndIndex] = PATH_END;
					AddClearPos( iEndIndex);
				}
			}
            else
            {
        		g_ErrorReport.Write( "잘못된 위치인덱스 : %d \r\n", iEndIndex);
            }
			iEndIndex = GetIndex( xEnd - i, yEnd + j);

            if ( iEndIndex>=0 && iEndIndex<(m_iSize) )
			{
				if ( !( bErrorCheck && iWall <= m_pbyMap[iEndIndex]))
				{
					m_pbyClosed[iEndIndex] = PATH_END;
					AddClearPos( iEndIndex);
				}
			}
            else
            {
        		g_ErrorReport.Write( "잘못된 위치인덱스 : %d \r\n", iEndIndex);
            }
		}
		// 그 좌측과 우측은 거리 계산해서 세팅
#ifdef _VS2008PORTING
		for ( int i = -iDistance; i < -xRange; i++)
#else // _VS2008PORTING
		for ( i = -iDistance; i < -xRange; i++)
#endif // _VS2008PORTING
		{
#ifdef _VS2008PORTING
			if ( ( float)sqrt( (float)(i * i + j * j)) < fDistance)
#else // _VS2008PORTING
			if ( ( float)sqrt( i * i + j * j) < fDistance)
#endif // _VS2008PORTING
			{
				int iEndIndex = GetIndex( xEnd + i, yEnd + j);
				if ( iEndIndex>=0 && iEndIndex<(m_iSize) )
				{
					if ( !( bErrorCheck && iWall <= m_pbyMap[iEndIndex]))
					{
						m_pbyClosed[iEndIndex] = PATH_END;
						AddClearPos( iEndIndex);
					}
				}
                else
                {
        		    g_ErrorReport.Write( "잘못된 위치인덱스 : %d \r\n", iEndIndex);
                }

                iEndIndex = GetIndex( xEnd - i, yEnd + j);
				if ( iEndIndex>=0 && iEndIndex<(m_iSize) )
				{
					if ( !( bErrorCheck && iWall <= m_pbyMap[iEndIndex]))
					{
						m_pbyClosed[iEndIndex] = PATH_END;
						AddClearPos( iEndIndex);
					}
				}
                else
                {
        		    g_ErrorReport.Write( "잘못된 위치인덱스 : %d \r\n", iEndIndex);
                }
			}
		}
	}
}

inline int PATH::CalculateCostToStartAddition( int xDir, int yDir)
{
	return ( ( xDir == 0 || yDir == 0) ? FACTOR_PATH_DIST : FACTOR_PATH_DIST_DIAG);
}

inline int PATH::EstimateCostToGoal( int xStart, int yStart, int xNew, int yNew)
{
	//사각+대각선거리
	int xDist = abs( xNew - xStart);
	int yDist = abs( yNew - yStart);
	if ( xDist == 1 && yDist == 1)
	{	// 기사가 대각선에서 때리지 않는 버그 수정 위함
		yDist = 0;
	}

	return ( abs( xDist - yDist) * FACTOR_PATH_DIST + min( xDist, yDist) * FACTOR_PATH_DIST_DIAG + 1) * 3 / 4;
}

inline bool PATH::GeneratePath( int xStart, int yStart, int xEnd, int yEnd)
{
	int xCurrent = xEnd;
	int yCurrent = yEnd;
	for ( m_iNumPath = 0; m_iNumPath < MAX_COUNT_PATH; m_iNumPath++)
	{
		// 찾은 길을 뒤집어 넣는다.
		m_xPath[( MAX_COUNT_PATH - 1) - m_iNumPath] = xCurrent;
		m_yPath[( MAX_COUNT_PATH - 1) - m_iNumPath] = yCurrent;

		if ( xCurrent == xStart && yCurrent == yStart)
		{
			m_iNumPath++;
			return ( true);
		}

		int iIndex = GetIndex( xCurrent, yCurrent);
		xCurrent = m_pxPrev[iIndex];
		yCurrent = m_pyPrev[iIndex];
	}

	return ( false);
}