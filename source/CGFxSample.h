#pragma once

#ifdef LDK_ADD_SCALEFORM && defined GFX_UI_TEST_CODE

#include "GTimer.h"
#include "GFile.h"
#include "GFxLoader.h"
#include "GFxEvent.h"
#include "GFxPlayer.h"
#include "GFxFontLib.h"

#include "FxPlayerLog.h"
#include "GrendererOGL.h"

#include "CGFxProcess.h"

#include "GFxImageResource.h"

//--------------------------------------------------------------------------------------
// FSCommand Handler
//--------------------------------------------------------------------------------------
class SampleFSCHandler : public GFxFSCommandHandler
{
public:
	virtual void Callback(GFxMovieView* pmovie, const char* pcommand, const char* parg);
};

//--------------------------------------------------------------------------------------
// ExternalInterface Handler
//--------------------------------------------------------------------------------------
class SampleEIHandler : public GFxExternalInterface
{
private:
	float angleX;
	float angleY;
	float angleZ;

public:
	void GetAngleX(float *outX) { *outX = angleX; }
	void GetAngleY(float *outY) { *outY = angleY; }
	void GetAngleZ(float *outZ) { *outZ = angleZ; }

public:
	virtual void Callback(GFxMovieView* pmovieView, const char* methodName, const GFxValue* args, UInt argCount);
};

//--------------------------------------------------------------------------------------
// GFxTutorial Definition
//--------------------------------------------------------------------------------------

class Fx3DDemoFileOpener : public GFxFileOpener
{
public:
	virtual GFile* OpenFile(const char *pfilename);
};

class CustomImageCreator : public GFxImageCreator
{
public:
	Fx3DDemoFileOpener fileOpener;

	GPtr<GRendererOGL>	pRenderer;
public:
	GImageInfoBase* CreateImage(const GFxImageCreateInfo &info);
};


class GFxSample : public CGFXBase
{
public:
	static GFxSample* Make(GFxRegistType _UIType, mapGFXContainer *_GfxRepo, const char* _pfilename, UInt _loadConstants = GFxLoader::LoadKeepBindData, UPInt _memoryArena = 0, UInt _renderFlag = GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeMask, GFxMovieView::ScaleModeType _scaleType = GFxMovieView::SM_NoScale, FontConfig* _pconfig = NULL);

	virtual ~GFxSample();

	virtual bool OnCreateDevice(SInt bufw, SInt bufh, SInt left, SInt top, SInt w, SInt h, UInt flags = 0);
	virtual bool OnResetDevice();
	virtual bool OnLostDevice();
	virtual bool OnDestroyDevice();

	virtual bool Update();
	virtual bool Render();

//	virtual GPtr<GFxMovieView> GetMovie();
	virtual GFxMovieView* GetMovie();

	virtual bool IsVisible()				{ return isVisible; }
	virtual void SetVisible(bool _visible)	{ if(!isLock && isVisible!= _visible) isVisible = _visible; }

	virtual bool IsLock()					{ return isLock; }
	virtual void SetUnLockVisible()			{ isLock = FALSE; }
	virtual void SetLockVisible(bool _visible) { isLock = TRUE; isVisible = _visible; }

	GPtr<GFxLog> GetLog() { return gfxLoader.GetLog(); }
	void LogPrintf(const char *format, va_list al)
	{
		gfxLoader.GetLog()->LogMessageVarg(GFxLog::Log_Channel_General, format, al);
	}

private:
	GFxSample();

	bool InitGFx(	const char* _pfilename, 
		UInt _loadConstants = GFxLoader::LoadKeepBindData,
		UPInt _memoryArena = 0,
		UInt _renderFlag = GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeMask, 
		GFxMovieView::ScaleModeType _scaleType = GFxMovieView::SM_NoScale,
		FontConfig* _pconfig = NULL);

private:
	GPtr<GRendererOGL>		pRenderer;
	GPtr<GFxRenderConfig>	pRenderConfig;
	GPtr<GFxRenderStats>	pRenderStats;

	GFxLoader           gfxLoader;
	GPtr<GFxMovieDef>   pUIMovieDef;
	GPtr<GFxMovieView>  pUIMovie;
	GFxMovieInfo		MovieInfo;

	SampleFSCHandler	*pFSHandler;
	SampleEIHandler *pEIHandler;

	// Movie frame time
	bool isVisible;
	bool isLock;
	bool isWireFrame;
	DWORD MovieLastTime;
};

#endif //LDK_ADD_SCALEFORM && defined GFX_UI_TEST_CODE

