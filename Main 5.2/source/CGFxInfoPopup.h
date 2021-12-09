#pragma once

#ifdef LDK_ADD_SCALEFORM

#include "GTimer.h"
#include "GFile.h"
#include "GFxLoader.h"
#include "GFxEvent.h"
#include "GFxPlayer.h"
#include "GFxFontLib.h"

#include "FxPlayerLog.h"
#include "GrendererOGL.h"

#include "CGFxProcess.h"

class CGFxInfoPopup;

//--------------------------------------------------------------------------------------
// FSCommand Handler
//--------------------------------------------------------------------------------------
class CInfoPopupFSCHandler : public GFxFSCommandHandler
{
public:
	virtual void Callback(GFxMovieView* pmovie, const char* pcommand, const char* parg);
};

//--------------------------------------------------------------------------------------
// ExternalInterface Handler
//--------------------------------------------------------------------------------------
class CInfoPopupEIHandler : public GFxExternalInterface
{
private:
	CGFxInfoPopup *m_pInfoPopup;

public:
	void SetMainUi(CGFxInfoPopup* _infoPopup) { m_pInfoPopup = _infoPopup; }

public:
	virtual void Callback(GFxMovieView* pmovieView, const char* methodName, const GFxValue* args, UInt argCount);
};

//--------------------------------------------------------------------------------------
// GFxTutorial Definition
//--------------------------------------------------------------------------------------
class CGFxInfoPopup : public CGFXBase
{
public:
	static CGFxInfoPopup* Make(GFxRegistType _UIType, mapGFXContainer *_GfxRepo, const char* _pfilename, UInt _loadConstants = GFxLoader::LoadKeepBindData, UPInt _memoryArena = 0, UInt _renderFlag = GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeMask, GFxMovieView::ScaleModeType _scaleType = GFxMovieView::SM_NoScale, FontConfig* _pconfig = NULL,const char* _languag = NULL);

	virtual ~CGFxInfoPopup();

	virtual bool OnCreateDevice(SInt bufw, SInt bufh, SInt left, SInt top, SInt w, SInt h, UInt flags = 0);
	virtual bool OnResetDevice();
	virtual bool OnLostDevice();
	virtual bool OnDestroyDevice();

	virtual bool Init();
	virtual bool Update();
	virtual bool Render();
	virtual bool RenderModel();

	//	virtual GPtr<GFxMovieView> GetMovie();
	virtual GFxMovieView* GetMovie();

	virtual bool IsVisible()				{ return m_bVisible; }
	virtual void SetVisible(bool _visible)	{ if(!m_bLock && m_bVisible!= _visible) m_bVisible = _visible; }

	virtual bool IsLock()					{ return m_bLock; }
	virtual void SetUnLockVisible()			{ m_bLock = FALSE; }
	virtual void SetLockVisible(bool _visible) { m_bLock = TRUE; m_bVisible = _visible; }

	GPtr<GFxLog> GetLog() { return m_gfxLoader.GetLog(); }
	void LogPrintf(const char *format, va_list al)
	{
		m_gfxLoader.GetLog()->LogMessageVarg(GFxLog::Log_Channel_General, format, al);
	}

private:
	CGFxInfoPopup();

	bool InitGFx(	const char* _pfilename, 
		UInt _loadConstants = GFxLoader::LoadKeepBindData,
		UPInt _memoryArena = 0,
		UInt _renderFlag = GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeMask, 
		GFxMovieView::ScaleModeType _scaleType = GFxMovieView::SM_NoScale,
		FontConfig* _pconfig = NULL, const char* _languag = NULL);

private:
	GPtr<GRendererOGL>		m_pRenderer;
	GPtr<GFxRenderConfig>	m_pRenderConfig;
	GPtr<GFxRenderStats>	m_pRenderStats;

	GFxLoader           m_gfxLoader;
	GPtr<GFxMovieDef>   m_pUIMovieDef;
	GPtr<GFxMovieView>  m_pUIMovie;
	GFxMovieInfo		m_gfxMovieInfo;

	CInfoPopupFSCHandler *m_pFSHandler;
	CInfoPopupEIHandler	*m_pEIHandler;

	// Movie frame time
	bool m_bVisible;
	bool m_bLock;
	bool m_bWireFrame;
	DWORD m_dwMovieLastTime;

	int m_iViewType;
};

#endif //LDK_ADD_SCALEFORM
