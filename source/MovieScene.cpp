// MovieScene.cpp: implementation of the CMovieScene class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef MOVIE_DIRECTSHOW
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "strmbasd.lib")
#pragma comment(lib, "WMVCORE.lib")

#include <atlbase.h>
#include <dshow.h>
#include <dshowasf.h>

#include "ZzzOpenglUtil.h"

#include "MovieScene.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMovieScene::CMovieScene()
{
	Init();
}

CMovieScene::~CMovieScene()
{

}

void CMovieScene::Init()
{
	m_pVideoWindow	=	NULL;
	m_pGraphBuilder =	NULL;
    m_pMediaControl =	NULL;
    m_pMediaEventEx =	NULL;

	m_pReader		=	NULL;
	m_pFileSource	=	NULL;

	m_bOneTimeInit	=	FALSE;
	m_bOneTimePlay	=	FALSE;
	m_bEndMovie		=	FALSE;
	m_bIsFile		=	TRUE;

	m_bFailDirectShow	=	FALSE;

	m_iPlayNum		=	0;
	OpenMovieTxt(MOVIE_FILE_TXT);
}

void CMovieScene::Destroy()
{
	m_iPlayNum = 1;
	SaveMovieTxt(MOVIE_FILE_TXT);

	SAFE_RELEASE(m_pFileSource);
	SAFE_RELEASE(m_pReader);

	SAFE_RELEASE(m_pVideoWindow);
	SAFE_RELEASE(m_pGraphBuilder);
	SAFE_RELEASE(m_pMediaControl);
	SAFE_RELEASE(m_pMediaEventEx);
}

void CMovieScene::InitOpenGLClear(HDC hDC)
{
	::BeginOpengl();

	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	::glClearColor( 0.f, 0.f, 0.f, 1.f);

	::EndOpengl();
	::glFlush();
	::SwapBuffers(hDC);
}

void CMovieScene::Initialize_DirectShow(HWND hWnd, char* strFileName)
{
	if(m_bOneTimeInit == TRUE)
		return;

	m_bOneTimeInit = TRUE;

	if(IsFileInFolder(strFileName) == FALSE)
		return;

//	ShowWindow(g_hWnd,SW_SHOW);
//	SetForegroundWindow(g_hWnd);
//	SetFocus(g_hWnd);

	USES_CONVERSION;
	HRESULT hr;
	WCHAR wFile[MAX_PATH];
	wcsncpy(wFile, T2W(strFileName), NUMELMS(wFile)-1);
    wFile[MAX_PATH-1] = 0;

	m_hWnd = hWnd;

    if(FAILED(CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IGraphBuilder, (void **)&m_pGraphBuilder)))
	{
		m_bFailDirectShow = TRUE;
		return;
	}
		

	if(FAILED(m_pGraphBuilder->QueryInterface(IID_IMediaEventEx, (void **)&m_pMediaEventEx)))
	{
		m_bFailDirectShow = TRUE;
		return;
	}

	if(FAILED(m_pGraphBuilder->QueryInterface(IID_IMediaControl, (void **)&m_pMediaControl)))
	{
		m_bFailDirectShow = TRUE;
		return;
	}
	
	if(FAILED(m_pGraphBuilder->QueryInterface(IID_IVideoWindow, (void **)&m_pVideoWindow)))
	{
		m_bFailDirectShow = TRUE;
		return;
	}

	m_pMediaEventEx->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0);

	if(IsWindowsMediaFile(strFileName))
	{
		// Load the improved ASF reader filter by CLSID
		if(FAILED(CreateFilter(CLSID_WMAsfReader, &m_pReader)))
		{	
			m_bFailDirectShow = TRUE;
			return;
		}

		// Add the ASF reader filter to the graph.  For ASF/WMV/WMA content,
        // this filter is NOT the default and must be added explicitly.
		if(FAILED(m_pGraphBuilder->AddFilter(m_pReader, L"ASF Reader")))
		{
			m_bFailDirectShow = TRUE;
			return;
		}

		// Create the key provider that will be used to unlock the WM SDK
        //AddKeyProvider(m_pGraphBuilder);

		if(FAILED(m_pReader->QueryInterface(IID_IFileSourceFilter, (void **)&m_pFileSource)))
		{
			m_bFailDirectShow = TRUE;
			return;
		}

		hr = m_pFileSource->Load(wFile, NULL);

		if(hr == NS_E_LICENSE_REQUIRED || hr == NS_E_PROTECTED_CONTENT || FAILED(hr) )
		{
			m_bFailDirectShow = TRUE;
			return;
		}
		
		
		if(SUCCEEDED(hr))
		{
			if(FAILED(RenderOutputPins(m_pGraphBuilder, m_pReader)))
			{
				m_bFailDirectShow = TRUE;
				return;
			}
				
			m_pReader->Release();
			m_pReader = NULL;
		}
	}
	else
	{
		if(FAILED(m_pGraphBuilder->RenderFile(wFile, NULL)))
		{
			m_bFailDirectShow = TRUE;
			return;
		}
	}

	m_pVideoWindow->put_Owner((OAHWND)m_hWnd);
    m_pVideoWindow->put_WindowStyle( WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN );

	if(m_pVideoWindow)
    {
        RECT client;
        GetClientRect(m_hWnd, &client);

		LONG lMovieHeight;
	
		lMovieHeight = (client.right / 3) * 2;

		RECT t_client;
		t_client.left = client.left;
		t_client.right = client.right;
		t_client.top = (client.bottom - lMovieHeight) / 3;
		t_client.bottom = lMovieHeight + t_client.top;
				
        m_pVideoWindow->SetWindowPosition(t_client.left, t_client.top, t_client.right, t_client.bottom);
		//m_pVideoWindow->SetWindowPosition(0, 0, 720, 480);
		//m_pVideoWindow->SetWindowForeground(OATRUE);
    }

	if(FAILED(m_pMediaControl->Run()))
	{
		m_bFailDirectShow = TRUE;
		return;
	}

//	ToggleFullScreen();
}

void CMovieScene::CreateFilter(REFCLSID clsid, IBaseFilter **ppFilter)
{

	CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER,
						IID_IBaseFilter, (void **) ppFilter);
}

BOOL CMovieScene::IsWindowsMediaFile(char* strFileName)
{
	TCHAR t_strFileName[MAX_PATH];

    _tcsncpy(t_strFileName, strFileName, NUMELMS(t_strFileName));
    t_strFileName[MAX_PATH-1] = 0;
    _tcslwr(t_strFileName);

    if (_tcsstr(t_strFileName, TEXT(".asf")) ||
        _tcsstr(t_strFileName, TEXT(".wma")) ||
        _tcsstr(t_strFileName, TEXT(".wmv")))
        return TRUE;
    else
        return FALSE;
}

BOOL CMovieScene::IsFileInFolder(char* strFileName)
{
	FILE *pFile = fopen(strFileName,"rb");
	
	if(pFile == NULL)
	{
		m_bIsFile = FALSE;
	}
	else
	{
		m_bIsFile = TRUE;
		fclose(pFile);
	}

	return m_bIsFile;
}

HRESULT CMovieScene::RenderOutputPins(IGraphBuilder *pGB, IBaseFilter *pFilter)
{
	HRESULT         hr = S_OK;
    IEnumPins *     pEnumPin = NULL;
    IPin *          pConnectedPin = NULL, * pPin = NULL;
    PIN_DIRECTION   PinDirection;
    ULONG           ulFetched;

    // Enumerate all pins on the filter
    hr = pFilter->EnumPins(&pEnumPin);

    if(SUCCEEDED(hr))
    {
        // Step through every pin, looking for the output pins
        while (S_OK == (hr = pEnumPin->Next(1L, &pPin, &ulFetched)))
        {
            // Is this pin connected?  We're not interested in connected pins.
            hr = pPin->ConnectedTo(&pConnectedPin);
            if (pConnectedPin)
            {
                pConnectedPin->Release();
                pConnectedPin = NULL;
            }

            // If this pin is not connected, render it.
            if (VFW_E_NOT_CONNECTED == hr)
            {
                hr = pPin->QueryDirection(&PinDirection);
                if ((S_OK == hr) && (PinDirection == PINDIR_OUTPUT))
                {
                    hr = pGB->Render(pPin);
                }
            }
            pPin->Release();

            // If there was an error, stop enumerating
            if (FAILED(hr))                      
                break;
        }
    }

    // Release pin enumerator
    pEnumPin->Release();
    return hr;
}

void CMovieScene::ToggleFullScreen()
{
    LONG lMode;
    static HWND hDrain=0;

    if(m_pVideoWindow)
    {
		m_pVideoWindow->get_FullScreenMode(&lMode);

		if(lMode == OAFALSE)
		{
			m_pVideoWindow->get_MessageDrain((OAHWND *) &hDrain);
        
			m_pVideoWindow->put_MessageDrain((OAHWND) m_hWnd);
        
			lMode = OATRUE;
			m_pVideoWindow->put_FullScreenMode(lMode);
		}
    }
}

void CMovieScene::PlayMovie()
{
	if(m_bOneTimePlay == FALSE)
	{
		m_bOneTimePlay = TRUE;
		m_pMediaControl->Run();
		SetFocus(m_hWnd);
	}
		
	long evCode, evParam1, evParam2;
	m_pMediaEventEx->GetEvent(&evCode, (LONG_PTR *) &evParam1, (LONG_PTR *) &evParam2, 0 );

	if( evCode == EC_COMPLETE )
	{
		m_bEndMovie =  TRUE;
	}
}

BOOL CMovieScene::IsEndMovie()
{
	return m_bEndMovie;
}

void CMovieScene::OpenMovieTxt(char* strFileName)
{
	FILE *pFile = fopen(strFileName, "rt");

	if(pFile == NULL) return;

	fscanf(pFile, "%d", &m_iPlayNum);
	
	fclose(pFile);
}

void CMovieScene::SaveMovieTxt(char* strFileName)
{
	FILE *pFile = fopen(strFileName, "wt");

	fprintf(pFile,"%d", m_iPlayNum);
	
	fclose(pFile);
}

int CMovieScene::GetPlayNum()
{
	return m_iPlayNum;
}

BOOL CMovieScene::IsFile()
{
	return m_bIsFile;
}

BOOL CMovieScene::IsFailDirectShow()
{
	return m_bFailDirectShow;
}

#endif // MOVIE_DIRECTSHOW