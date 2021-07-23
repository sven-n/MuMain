#include "stdafx.h"

#ifdef LDK_ADD_SCALEFORM

#include <assert.h>
#include "CGFxInfoPopup.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGFxInfoPopup* CGFxInfoPopup::Make(GFxRegistType _UIType, mapGFXContainer *_GfxRepo, const char* _pfilename, UInt _loadConstants, UPInt _memoryArena, UInt _renderFlag, GFxMovieView::ScaleModeType _scaleType, FontConfig* _pconfig, const char* _languag)
{
	if(_GfxRepo == NULL)
		return NULL;

	CGFxInfoPopup* temp( new CGFxInfoPopup );

	if(!temp->InitGFx(_pfilename, _loadConstants, _memoryArena, _renderFlag, _scaleType, _pconfig, _languag))
	{
		delete temp;
		temp = NULL;

		//error log

		return NULL;
	}

	_GfxRepo->insert(make_pair(_UIType, temp));

	return temp;
}

CGFxInfoPopup::CGFxInfoPopup()
{
	m_bVisible = FALSE;
	m_bLock = FALSE;
	m_bWireFrame = FALSE;
	m_dwMovieLastTime = 0;
}

CGFxInfoPopup::~CGFxInfoPopup()
{
	m_pUIMovie = NULL;
	m_pUIMovieDef = NULL;

	m_pFSHandler->Release();
	m_pEIHandler->Release();
}

bool CGFxInfoPopup::OnCreateDevice(SInt bufw, SInt bufh, SInt left, SInt top, SInt w, SInt h, UInt flags)
{
	if(!m_pUIMovie)
		return FALSE;

	if(!m_pRenderer->SetDependentVideoMode())
		return FALSE;

	m_pUIMovie->SetViewport(bufw, bufh, left, top, w, h, flags);

	//해상도별 설정하기
	if(bufw == 800 && bufh == 600)
	{
		m_iViewType = 1;
		m_pUIMovie->Invoke("_root.scene.SetViewSize", "%d", 1);
	}
	else if(bufw == 1024 && bufh == 768)
	{
		m_iViewType = 2;
		m_pUIMovie->Invoke("_root.scene.SetViewSize", "%d", 2);
	}
	else if(bufw == 1280 && bufh == 1024)
	{
		m_iViewType = 3;
		m_pUIMovie->Invoke("_root.scene.SetViewSize", "%d", 3);
	}

	return TRUE;
}

bool CGFxInfoPopup::OnResetDevice()
{
	// openGL 상에서 resetDevice 하는 방법 모르겠음
	//this->InitGFx();
	return TRUE;
}

bool CGFxInfoPopup::OnLostDevice()
{
	if(m_pRenderer->ResetVideoMode())
		return FALSE;

	return TRUE;
}

bool CGFxInfoPopup::OnDestroyDevice()
{
	return TRUE;
}

bool CGFxInfoPopup::InitGFx(const char* _pfilename, UInt _loadConstants, UPInt _memoryArena, UInt _renderFlag, GFxMovieView::ScaleModeType _scaleType, FontConfig* _pconfig, const char* _languag)
{

	if (_pconfig)
	{
		_pconfig->Apply(&m_gfxLoader);
	}
	// 	else 
	//	{
	//		// Create and load a file into GFxFontLib if requested.
	// 		GPtr<GFxFontLib> fontLib = *new GFxFontLib;
	// 		m_gfxLoader.SetFontLib(fontLib);
	// 
	// 		GPtr<GFxMovieDef> pmovieDef = *m_gfxLoader.CreateMovie(Arguments.GetString("FontLibFile").ToCStr());
	// 		fontLib->AddFontsFrom(pmovieDef);
	//	}

	m_gfxLoader.SetLog(GPtr<GFxLog>(*new GFxPlayerLog()));

	m_pFSHandler = new CInfoPopupFSCHandler;
	m_gfxLoader.SetFSCommandHandler(GPtr<GFxFSCommandHandler>(m_pFSHandler));

	m_pEIHandler = new CInfoPopupEIHandler;
	m_pEIHandler->SetMainUi(this);
	m_gfxLoader.SetExternalInterface(GPtr<GFxExternalInterface>(m_pEIHandler));

	m_pRenderer = *GRendererOGL::CreateRenderer();
	m_pRenderer->SetDependentVideoMode();

	m_pRenderConfig = *new GFxRenderConfig(m_pRenderer, _renderFlag);
	if(!m_pRenderer || !m_pRenderConfig)
		return FALSE;
	m_gfxLoader.SetRenderConfig(m_pRenderConfig);

	m_pRenderStats = *new GFxRenderStats();
	m_gfxLoader.SetRenderStats(m_pRenderStats);

	GPtr<GFxFileOpener> pfileOpener = *new GFxFileOpener;
	m_gfxLoader.SetFileOpener(pfileOpener);

	if (!m_gfxLoader.GetMovieInfo(_pfilename, &m_gfxMovieInfo, 0, _loadConstants)) 
	{
		//fprintf(stderr, "Error: Failed to get info about %s\n", pfilename.ToCStr());
		return FALSE;
	}

	m_pUIMovieDef = *(m_gfxLoader.CreateMovie(_pfilename, _loadConstants, _memoryArena));
	if(!m_pUIMovieDef)
		return FALSE;

	m_pUIMovie = *m_pUIMovieDef->CreateInstance(GFxMovieDef::MemoryParams(), false);
	if(!m_pUIMovie)
		return FALSE;

	// Release cached memory used by previous file.
	if (m_pUIMovie->GetMeshCacheManager())
		m_pUIMovie->GetMeshCacheManager()->ClearCache();

	m_pUIMovie->SetViewScaleMode(_scaleType);

	// Set a reference to the app
	m_pUIMovie->SetUserData(this);

	if(_languag == NULL) _languag = "Default";
	m_pUIMovie->SetVariable("_global.gfxLanguage", GFxValue(_languag));

	m_pUIMovie->Advance(0.0f, 0);
	m_pUIMovie->SetBackgroundAlpha(0.0f);

	m_dwMovieLastTime = timeGetTime();

	m_pUIMovie->Invoke("_root.scene.SetClearSkillSlot", "");

	return TRUE;
}

// 캐릭터 선택창에서 넘어올때 호출
bool CGFxInfoPopup::Init()
{
	return TRUE;
}

bool CGFxInfoPopup::Update()
{
	if(m_bVisible == FALSE)
		return TRUE;

	if(!m_pUIMovie)
		return FALSE;

	//GFx update
	DWORD mtime = timeGetTime();
	float deltaTime = ((float)(mtime - m_dwMovieLastTime)) / 1000.0f;
	m_dwMovieLastTime = mtime;
	m_pUIMovie->Advance(deltaTime, 0);

	return TRUE;
}

bool CGFxInfoPopup::RenderModel()
{
	if(m_bVisible == FALSE)
		return TRUE;

	if(!m_pUIMovie)
		return FALSE;

	return TRUE;
}

bool CGFxInfoPopup::Render()
{
	if(m_bVisible == FALSE)
		return TRUE;

	if(!m_pUIMovie)
		return FALSE;

	//랜더링
	if(m_bWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//GFx Render
	m_pUIMovie->Display();

	if(m_bWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	return TRUE;
}

GFxMovieView* CGFxInfoPopup::GetMovie()
{
	if(!m_pUIMovie)
		return NULL;

	return m_pUIMovie; 
}

//--------------------------------------------------------------------------------------
// FSCommand Handler
//--------------------------------------------------------------------------------------
void CInfoPopupFSCHandler::Callback(GFxMovieView* pmovie, const char* pcommand, const char* parg)
{
	//GFxPrintf("FSCommand: %s, Args: %s\n", pcommand, parg);

//	if(strcmp(pcommand, "onClickShopBtn") == 0)
//	{
//	}
}

//--------------------------------------------------------------------------------------
// ExternalInterface Handler
//--------------------------------------------------------------------------------------
void CInfoPopupEIHandler::Callback(GFxMovieView* pmovieView, const char* methodName, const GFxValue* args, UInt argCount)
{
	m_pInfoPopup->GetLog()->LogMessage("\nCallback! %s, nargs = %d\n", (methodName)?methodName:"(null)", argCount);
	for (UInt i = 0; i < argCount; ++i)
	{
		switch(args[i].GetType())
		{
		case GFxValue::VT_Null:		m_pInfoPopup->GetLog()->LogMessage("NULL"); break;
		case GFxValue::VT_String:	m_pInfoPopup->GetLog()->LogMessage("%s", args[i].GetString()); break;
		case GFxValue::VT_Number:	m_pInfoPopup->GetLog()->LogMessage("%f", args[i].GetNumber()); break;
		case GFxValue::VT_Boolean:	m_pInfoPopup->GetLog()->LogMessage("%s", (args[i].GetBool())?"true":"false"); break;
			// etc...
		default:
			m_pInfoPopup->GetLog()->LogMessage("unknown"); break;
			break;
		}
		m_pInfoPopup->GetLog()->LogMessage("%s", (i == argCount - 1) ? "" : ", ");
	}
	m_pInfoPopup->GetLog()->LogMessage("\n");

	if(strcmp(methodName, "error") == 0 /*&& argCount == 1 && args[0].GetType() == GFxValue::VT_String*/)
	{
		//error log
		int aa = -1;

		for(int i=0; i<argCount; i++)
		{
			switch(args[i].GetType())
			{
			case GFxValue::VT_String:
				break;

			case GFxValue::VT_Number:
				aa = args[i].GetNumber();
				break;
			}
		}
		//char temp[1024] = args[0].GetString();
	}
}

#endif //LDK_ADD_SCALEFORM
