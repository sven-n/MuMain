// MovieScene.h: interface for the CMovieScene class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOVIESCENE_H__89628B4A_20FB_4DA3_9B34_D25B1026CFAC__INCLUDED_)
#define AFX_MOVIESCENE_H__89628B4A_20FB_4DA3_9B34_D25B1026CFAC__INCLUDED_

#pragma once

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
	BOOL			m_bIsFile;
	BOOL			m_bFailDirectShow;

	int				m_iPlayNum;

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
