// MovieScene.h: interface for the CMovieScene class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOVIESCENE_H__89628B4A_20FB_4DA3_9B34_D25B1026CFAC__INCLUDED_)
#define AFX_MOVIESCENE_H__89628B4A_20FB_4DA3_9B34_D25B1026CFAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef MOVIE_DIRECTSHOW

class CMovieScene  
{
private:
	HWND			m_hWnd;

	IVideoWindow*	m_pVideoWindow;
	IGraphBuilder*	m_pGraphBuilder;
    IMediaControl*	m_pMediaControl;
    IMediaEventEx*	m_pMediaEventEx;

	IBaseFilter*		m_pReader;
	IFileSourceFilter*	m_pFileSource;

	BOOL			m_bOneTimeInit;
	BOOL			m_bOneTimePlay;
	BOOL			m_bEndMovie;
	BOOL			m_bIsFile;			// 파일이 존재하는가 판별하는 변수
	BOOL			m_bFailDirectShow;	// 다이렉트쇼 초기화및 동영상 잘 로드 했는가 판별

	int				m_iPlayNum;			// 플레이를 한번 했는가 판별하는 변수

private:
	void	Init();
	void	ToggleFullScreen();
	BOOL	IsWindowsMediaFile(char* strFileName);
	BOOL	IsFileInFolder(char* strFileName);
	void	CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter);
	HRESULT RenderOutputPins(IGraphBuilder *pGB, IBaseFilter *pFilter);
	
public:
	void	InitOpenGLClear(HDC hDC);
	void	Initialize_DirectShow(HWND hWnd, char* strFileName);
	void	PlayMovie();
	BOOL	IsEndMovie();
	BOOL	IsFile();
	BOOL	IsFailDirectShow();
	int		GetPlayNum();
	void	OpenMovieTxt(char* strFileName);
	void	SaveMovieTxt(char* strFileName);
	void	Destroy();
	CMovieScene();
	virtual ~CMovieScene();

};

#endif // MOVIE_DIRECTSHOW

#endif // !defined(AFX_MOVIESCENE_H__89628B4A_20FB_4DA3_9B34_D25B1026CFAC__INCLUDED_)
