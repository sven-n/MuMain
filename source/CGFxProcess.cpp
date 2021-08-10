#include "stdafx.h"

#ifdef LDK_ADD_SCALEFORM

#include "CGFxProcess.h"

//gfxExpress 추가
#include "CGFxMainUi.h"
#include "CGFxInfoPopup.h"

#ifdef GFX_UI_TEST_CODE
	#include "CGFxSample.h"
#endif // GFX_UI_TEST_CODE

#define UI_FILEPATH "./Data/Interface/GFx/"

/////////////////////////////////////////////////////////////////////
static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};
static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
CGFXBase* GFxProcess::Find(GFxRegistType key)
{
	//m_mapGfxContainer 사용
	mapGFXContainer::iterator iter = m_mapGfxContainer.find(key);

	if( iter != m_mapGfxContainer.end() )
	{
		return (*iter).second;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// GFxProcess* GFxProcess::Make()
// {
// 	GFxProcess* process( new GFxProcess );
// 	return process;
// }
GFxProcess* GFxProcess::GetInstancePtr()
{
	static GFxProcess* _pGFx = new GFxProcess();
	return _pGFx;
}

GFxProcess::GFxProcess()
{
	m_isCompletedLoad = FALSE;

	m_iNowSceneFlag = NON_SCENE;

	m_iWidth = 0;
	m_iHeight = 0;

	m_iUISelection = 0;

	m_bNoFontConfig = FALSE;
	m_FontConfigIndex = -1;
}

GFxProcess::~GFxProcess()
{
 	GFxDestroy();
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void GFxProcess::OnCreateDevice(SInt bufw, SInt bufh, SInt left, SInt top, SInt w, SInt h, UInt flags)
{
	m_iWidth = w;
	m_iHeight = h;

	if(!m_isCompletedLoad)
		return;

	for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter ++ )
	{
		CGFXBase* temp = (*iter).second;

		if(temp->OnCreateDevice(bufw, bufh, left, top, w, h, flags))
		{
			//error log
		}
	}
}

void GFxProcess::OnResetDevice()
{
	if(!m_isCompletedLoad)
		return;

	for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter ++ )
	{
		CGFXBase* temp = (*iter).second;

		if(temp->OnResetDevice())
		{
			//error log
		}
	}
}

void GFxProcess::OnLostDevice()
{
	if(!m_isCompletedLoad)
		return;

	for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter ++ )
	{
		CGFXBase* temp = (*iter).second;

		if(temp->OnLostDevice())
		{
			//error log
		}
	}
}

void GFxProcess::OnDestroyDevice()
{
	if(!m_isCompletedLoad)
		return;

	for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter ++ )
	{
		CGFXBase* temp = (*iter).second;

		if(temp->OnDestroyDevice())
		{
			//error log
		}
	}
}

void GFxProcess::LoadFontConfigs(ConfigParser *parser)
{
	m_FontConfigs.Parse(parser);
	if (m_FontConfigs.GetSize() > 0)
	{
		m_FontConfigIndex = 0;
	}
	else
	{
		m_FontConfigIndex = -1;
	}
}
int GFxProcess::GetFontConfigIndexByName(const char* pname)
{
	for (UInt i = 0; i < m_FontConfigs.GetSize(); i++)
	{
		if (G_stricmp(pname, m_FontConfigs[i]->ConfigName) == 0)
			return i;
	}   
	return -1;
}

FontConfig* GFxProcess::GetCurrentFontConfig()
{
	// we are skipping over invalid fontconfigs until one is found.
	// else return NULL.

	if (m_FontConfigIndex == -1)
		return NULL;

	FontConfig* fc = NULL;
	SInt sIdx = m_FontConfigIndex;
	bool ok = false;

	while (!ok)
	{
		ok = true;
		fc = m_FontConfigs[m_FontConfigIndex];
		// check if all fontlib files exist
		for (UInt i=0; i < fc->FontLibFiles.GetSize(); i++)
		{
			// check if file exists
			GSysFile file(fc->FontLibFiles[i]);
			if (!file.IsValid())
			{
				ok = false;
				fprintf(stderr, "Fontlib file '%s' cannot be found. Skipping config '%s'..\n", fc->FontLibFiles[i].ToCStr(), fc->ConfigName.ToCStr());
				break;
			}
		}

		if (!ok)
		{
			m_FontConfigIndex++;
			m_FontConfigIndex %= (SInt)m_FontConfigs.GetSize();
			if (m_FontConfigIndex == sIdx)
				return NULL;
		}
	}

	return m_FontConfigs[m_FontConfigIndex];
}

bool GFxProcess::LoadDefaultFontConfigFromPath( const GString& path )
{
	if (!m_bNoFontConfig)
	{
		// load fontconfig.txt if it exists in the movie's path
		GString fontConfigFilePath;

		// [PPS] The absolute check is unneccessary
		// ExtractFilePath will return the correct parent path:
		// C:/folder/filename.ext - C:/folder/
		// folder/filename.ext - folder/
		// filename.ext - empty string
		//if (GFxURLBuilder::IsPathAbsolute(path))
		//{
		fontConfigFilePath.AppendString(path);
		if ( !GFxURLBuilder::ExtractFilePath(&fontConfigFilePath) )
		{
			fontConfigFilePath = "";
		}
		//}

		fontConfigFilePath += "fontconfig.txt";
		bool maintainIndex = false;

		// store font config file related info
		if (m_FontConfigFilePath.GetLength() == 0)   // if no file was previously loaded
		{
			GFileStat fileStats;
			if ( GSysFile::GetFileStat(&fileStats, fontConfigFilePath.ToCStr()) )
			{
				m_FontConfigFilePath = fontConfigFilePath;
				m_FontConfigFileStats = fileStats;
			}
		}
		else // if the file was previously loaded and is modified
		{
			if (fontConfigFilePath == m_FontConfigFilePath)
			{
				// if modified time is not the same, then reload config file
				GFileStat fileStats;
				if ( GSysFile::GetFileStat(&fileStats, fontConfigFilePath.ToCStr()) )
				{
					if ( !(fileStats == m_FontConfigFileStats) )
					{
						m_FontConfigFileStats = fileStats;
						maintainIndex = true;
					}
				}
			}
		}

		// parse the config file
		ConfigParser parser(fontConfigFilePath.ToCStr());
		SInt oldIdx = m_FontConfigIndex;
		LoadFontConfigs(&parser);

		// try to maintain previous font config index
		if ( maintainIndex &&
			(m_FontConfigIndex == 0) &&
			(oldIdx != -1) )
		{
			m_FontConfigIndex = oldIdx;
			m_FontConfigIndex %= (SInt)m_FontConfigs.GetSize();
		}

		return true;
	}
	return false;
}

bool GFxProcess::GFxInit()
{
 	if(!m_isCompletedLoad)
 	{
		GString filepath = UI_FILEPATH;
		GString filename = "";

		// font 
		if (!LoadDefaultFontConfigFromPath(filepath) ) 
		{
			//ERROR log
		}

#ifdef ADD_SELETED_LANGUAGE_FONT
		//폰트 국가 설정
		m_FontConfigIndex = SELECTED_LANGUAGE;
		m_FontConfigIndex %= (SInt)m_FontConfigs.GetSize();
#endif //ADD_SELETED_LANGUAGE_FONT

		// set the font config
		FontConfig* pconfig = NULL;
		if (m_FontConfigIndex != -1) 
		{
			pconfig = GetCurrentFontConfig();
		} 

		//폰트설정
		const char* language = (m_FontConfigIndex == -1) ? "Default" : m_FontConfigs[m_FontConfigIndex]->ConfigName.ToCStr();
		
		// CGFXBase Class 생성, 초기화
		// m_mapGfxContainer 등록

		// create main frame swf
		filename = "mainFrame.gfx";
		CGFxMainUi* tempMainUI = CGFxMainUi::Make(GFxRegistType::eGFxRegist_MainUI, &m_mapGfxContainer, filepath+filename, GFxLoader::LoadAll | GFxLoader::LoadKeepBindData | GFxLoader::LoadWaitFrame1, 0 ,GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeMask, GFxMovieView::SM_NoScale, pconfig, language);
		if(tempMainUI == NULL)
		{
			delete tempMainUI;
			tempMainUI = NULL;
			//error log
		}

		// info Popup
		filename = "infoPopup.gfx";
		CGFxInfoPopup* tempPopup =  CGFxInfoPopup::Make(GFxRegistType::eGFxRegist_InfoPupup, &m_mapGfxContainer, filepath+filename, GFxLoader::LoadAll | GFxLoader::LoadKeepBindData | GFxLoader::LoadWaitFrame1, 0 ,GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeMask, GFxMovieView::SM_NoScale, pconfig, language);
		if(tempPopup == NULL)
		{
			delete tempPopup;
			tempPopup = NULL;
			//error log
		}

#ifdef GFX_UI_TEST_CODE
		// image Test
		filename = "hpguege.gfx";
		GFxSample* tempImageTest = GFxSample::Make(GFxRegistType::eGFxRegist_ImageTest, &m_mapGfxContainer, filepath+filename, GFxLoader::LoadAll | GFxLoader::LoadKeepBindData | GFxLoader::LoadWaitFrame1, 0 ,GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeMask, GFxMovieView::SM_NoScale, pconfig);
		if(tempImageTest == NULL)
		{
			delete tempImageTest;
			tempImageTest = NULL;
			//error log
		}
#endif //GFX_UI_TEST_CODE

 		m_isCompletedLoad = TRUE;
 	}

	return m_isCompletedLoad;
}

void GFxProcess::GFxDestroy()
{
	//m_mapGfxContainer 초기화, 삭제
	for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter++)
	{
		CGFXBase* temp = (*iter).second;
		delete temp;
	}
	m_mapGfxContainer.clear();
}

void GFxProcess::GFxUpdate(int _scene)
{
	if(!m_isCompletedLoad)
		return;

	//scene 변경시 초기화
	if(m_iNowSceneFlag != _scene)
	{
		for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter ++ )
		{
			CGFXBase* temp = (*iter).second;

			// scene update 초기화
			switch (_scene)
			{
			case LOG_IN_SCENE:		// 로그인 씬.
				GFxSceneInit_LogIn((*iter).first, temp);
				break;

			case CHARACTER_SCENE:	// 캐릭터 선택, 생성 씬.
				GFxSceneInit_Char((*iter).first, temp);
				break;

			case MAIN_SCENE:		// 게임 씬.
				GFxSceneInit_Main((*iter).first, temp);
				break;
			}
		}
		m_iNowSceneFlag = _scene;
	}

	for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter ++ )
	{
		CGFXBase* temp = (*iter).second;

		// scene별 설정
		switch (_scene)
		{
		case LOG_IN_SCENE:		// 로그인 씬.
			GFxSceneControll_LogIn((*iter).first, temp);
			break;

		case CHARACTER_SCENE:	// 캐릭터 선택, 생성 씬.
			GFxSceneControll_Char((*iter).first, temp);
			break;

		case MAIN_SCENE:		// 게임 씬.
			GFxSceneControll_Main((*iter).first, temp);
			break;
		}
	}//for
}

void GFxProcess::GFxRender()
{
	if(!m_isCompletedLoad)
		return;

	for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter ++ )
	{
		CGFXBase* temp = (*iter).second;

		if(temp->IsVisible())
		{
			//GFxRendering--------------------------------------------------------------------------//
			//설정 초기화
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glDisable(GL_FRAGMENT_PROGRAM_ARB);
			glDisable(GL_VERTEX_PROGRAM_ARB);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			if(temp->Render())
			{
				//error log
			}

			//설정 복구
			glPopAttrib();

			//3d model rendering--------------------------------------------------------------------//
			//. 2D를 그리던 중이였으므로 EndBitmap를 호출한다.
			EndBitmap();

			/******************** 3D 그리기 루틴 ********************/
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glViewport2(0,0,m_iWidth,m_iHeight);
			gluPerspective2(1.f, (float)(m_iWidth)/(float)(m_iHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			GetOpenGLMatrix(CameraMatrix);	// 카메라 매트릭스 초기화
			EnableDepthTest();
			EnableDepthMask();

			// 기존 3D와 꼬이는 것을 방지하기 위해
			// 깊이 버퍼를 클리어 시켜준다.
			glClear(GL_DEPTH_BUFFER_BIT);

			if(temp->RenderModel())
			{
				//error log
			}

			// 현재 카메라의 매트릭스를 가지고 MousePosition 업데이트
			UpdateMousePositionn();

			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();

			//. 다시 2D를 그려야 하므로 BeginBitmap를 호출한다.
			BeginBitmap();

			//. 3D 위에 그려지는 2D Effect를 그린다.
			// 	while(!m_deque2DEffects.empty())
			// 	{
			// 		UI_2DEFFECT_INFO& UI2DEffectInfo = m_deque2DEffects.front();
			// 		if(UI2DEffectInfo.pCallbackFunc)
			// 		{
			// 			(*UI2DEffectInfo.pCallbackFunc)(UI2DEffectInfo.pClass, UI2DEffectInfo.dwParamA, UI2DEffectInfo.dwParamB);
			// 		}
			// 		m_deque2DEffects.pop_front();
			// 	}

		}
	}
}

void GFxProcess::GFxProcessEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool *pbNoFurtherProcessing)
{
	if(!m_isCompletedLoad)
		return;

	int mx = LOWORD(lParam), my = HIWORD(lParam);

	for( mapGFXContainer::iterator iter = m_mapGfxContainer.begin(); iter != m_mapGfxContainer.end(); iter ++ )
	{
		CGFXBase* temp = (*iter).second;

		if(temp->IsVisible())
		{
			GFxMovieView* pMove = temp->GetMovie();

			bool processedMouseEvent = false;
			if(uMsg == WM_MOUSEMOVE)
			{
				GFxMouseEvent mevent(GFxEvent::MouseMove, 0, (Float)mx, (Float)my);
				pMove->HandleEvent(mevent);
				processedMouseEvent = true;
			}
			else if(uMsg == WM_LBUTTONDOWN)
			{
				::SetCapture(hWnd);
				GFxMouseEvent mevent(GFxEvent::MouseDown, 0, (Float)mx, (Float)my);
				pMove->HandleEvent(mevent);
				processedMouseEvent = true;
			}
			else if(uMsg == WM_LBUTTONUP)
			{
				::ReleaseCapture();
				GFxMouseEvent mevent(GFxEvent::MouseUp, 0, (Float)mx, (Float)my);
				pMove->HandleEvent(mevent);
				processedMouseEvent = true;
			}
			else if(uMsg == WM_RBUTTONDOWN)
			{
				::SetCapture(hWnd);
				GFxMouseEvent mevent(GFxEvent::MouseDown, 1, (Float)mx, (Float)my);
				pMove->HandleEvent(mevent);
				processedMouseEvent = true;
			}
			else if(uMsg == WM_RBUTTONUP)
			{
				::ReleaseCapture();
				GFxMouseEvent mevent(GFxEvent::MouseUp, 1, (Float)mx, (Float)my);
				pMove->HandleEvent(mevent);
				processedMouseEvent = true;
			}

			//if(processedMouseEvent && pMove->HitTest((Float)mx, (Float)my, GFxMovieView::HitTest_ButtonEvents))
			if(processedMouseEvent && pMove->HitTest((Float)mx, (Float)my, GFxMovieView::HitTest_ShapesNoInvisible))
				*pbNoFurtherProcessing = true;

			if(uMsg == WM_SYSKEYDOWN || uMsg == WM_SYSKEYUP || uMsg == WM_KEYDOWN || uMsg == WM_KEYUP || uMsg == WM_CHAR)
			{
				ProcessKeyEvent(pMove, uMsg, wParam, lParam);
			}
		}//isAlive

	}//for
}

//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////
void GFxProcess::GFxSceneInit_LogIn(GFxRegistType _type, CGFXBase* _gfx)
{
	if(_type == GFxRegistType::eGFxRegist_MAX_COUNT)
	{
		//error log
		return;
	}

	// Setting Once ------------------------------------------------//
	// scene 변경시 잠겨있는 설정을 풀어줌.
	_gfx->SetUnLockVisible();

	switch(_type)
	{
	case GFxRegistType::eGFxRegist_NONE:
		//error log??
		break;

		// scene에서 초기 열어놓음
	case GFxRegistType::eGFxRegist_ImageTest:
		_gfx->SetVisible(TRUE);
		break;

		// scene에서 초기 닫아놓음
	default:
		_gfx->SetVisible(FALSE);
		break;
	}
}

void GFxProcess::GFxSceneControll_LogIn(GFxRegistType _type, CGFXBase* _gfx)
{
	if(_type == GFxRegistType::eGFxRegist_MAX_COUNT)
	{
		//error log
		return;
	}

	// show/hide ------------------------------------------------//
// 	switch(_type)
// 	{
// 		// 해당씬에서 절대로 안닫힘
// 	case GFxRegistType::eGFxRegist_TEST_0:
// 	case GFxRegistType::eGFxRegist_TEST_1:
// 		{
// 			_gfx->SetLockVisible(TRUE);
// 		}
// 		break;
// 
// 		// 해당씬에서 절대로 안열림
// 	case GFxRegistType::eGFxRegist_MainButton:
// 		{
// 			_gfx->SetLockVisible(FALSE);
// 		}
// 		break;
// 	}

	// update ------------------------------------------------//
	if(_gfx->IsVisible())
	{
		if(_gfx->Update())
		{
			//error log
		}
	}
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void GFxProcess::GFxSceneInit_Char(GFxRegistType _type, CGFXBase* _gfx)
{
	if(_type == GFxRegistType::eGFxRegist_MAX_COUNT)
	{
		//error log
		return;
	}

	// Setting Once ------------------------------------------------//
	// scene 변경시 잠겨있는 설정을 풀어줌.
	_gfx->SetUnLockVisible();

	switch(_type)
	{
	case GFxRegistType::eGFxRegist_NONE:
		//error log??
		break;
		// scene에서 초기 열어놓음

		// scene에서 초기 닫아놓음
	default:
		_gfx->SetVisible(FALSE);
		break;
	}
}

void GFxProcess::GFxSceneControll_Char(GFxRegistType _type, CGFXBase* _gfx)
{
	if(_type == GFxRegistType::eGFxRegist_MAX_COUNT)
	{
		//error log
		return;
	}

	// show/hide ------------------------------------------------//
// 	switch(_type)
// 	{
// 		// 해당씬에서 절대로 안닫힘
// 	case GFxRegistType::eGFxRegist_TEST_2:
// 		{
// 			_gfx->SetLockVisible(TRUE);
// 		}
// 		break;
// 
// 		// 해당씬에서 절대로 안열림
// 	case GFxRegistType::eGFxRegist_TEST_3:
// 	case GFxRegistType::eGFxRegist_MainButton:
// 		{
// 			_gfx->SetLockVisible(FALSE);
// 		}
// 		break;
// 	}

	// update ------------------------------------------------//
	if(_gfx->IsVisible())
	{
		if(_gfx->Update())
		{
			//error log
		}
	}
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////
void GFxProcess::GFxSceneInit_Main(GFxRegistType _type, CGFXBase* _gfx)
{
	if(_type == GFxRegistType::eGFxRegist_MAX_COUNT)
	{
		//error log
		return;
	}

	// Setting Once ------------------------------------------------//
	// scene 변경시 잠겨있는 설정을 풀어줌.
	_gfx->SetUnLockVisible();

	switch(_type)
	{
	case GFxRegistType::eGFxRegist_NONE:
		//error log??
		break;
		// scene에서 초기 열어놓음
// 	case GFxRegistType::eGFxRegist_TEST_0:
// 		{
// 			_gfx->SetVisible(TRUE);
// 		}
// 		break;

	case GFxRegistType::eGFxRegist_MainUI:
	case GFxRegistType::eGFxRegist_InfoPupup:
		{

			_gfx->SetVisible(TRUE);
		}
		break;


		// scene에서 초기 닫아놓음
	default:
		_gfx->SetVisible(FALSE);
		break;
	}

	if(_gfx->IsVisible())
	{
		if(_gfx->Init())
		{
			//error log
		}
	}

}

void GFxProcess::GFxSceneControll_Main(GFxRegistType _type, CGFXBase* _gfx)
{
	if(_type == GFxRegistType::eGFxRegist_MAX_COUNT)
	{
		//error log
		return;
	}

	// show/hide ------------------------------------------------//
//	switch(_type)
//	{
//		// 해당씬에서 절대로 안닫힘
// 	case GFxRegistType::eGFxRegist_MainUI:
// 		{
// 			_gfx->SetLockVisible(TRUE);
// 		}
// 		break;
//
//		// 해당씬에서 절대로 안열림
//		//_gfx->SetLockVisible(FALSE);
//	}

	// update ------------------------------------------------//
	if(_gfx->IsVisible())
	{
		if(_gfx->Update())
		{
			//error log
		}
	}
}

//-----------------------------------------------------
void GFxProcess::SetSkillClearHotKey()
{
	CGFxMainUi* temp = (CGFxMainUi*)Find(GFxRegistType::eGFxRegist_MainUI);

	if(temp)
		temp->SetSkillClearHotKey();
}

void GFxProcess::SetSkillHotKey(int iHotKey, int _skillType)
{
	CGFxMainUi* temp = (CGFxMainUi*)Find(GFxRegistType::eGFxRegist_MainUI);

	if(temp)
		temp->SetSkillHotKey(iHotKey, _skillType);
}

int GFxProcess::GetSkillHotKey(int iHotKey)
{
	CGFxMainUi* temp = (CGFxMainUi*)Find(GFxRegistType::eGFxRegist_MainUI);

	if(temp)
	{
		return temp->GetSkillHotKey(iHotKey);
	}

	return -1;
}


void GFxProcess::SetItemClearHotKey()
{
	CGFxMainUi* temp = (CGFxMainUi*)Find(GFxRegistType::eGFxRegist_MainUI);

	if(temp)
		temp->SetItemClearHotKey();
}

void GFxProcess::SetItemHotKey(int iHotKey, int iItemType, int iItemLevel)
{
	CGFxMainUi* temp = (CGFxMainUi*)Find(GFxRegistType::eGFxRegist_MainUI);

	if(temp)
		temp->SetItemHotKey(iHotKey, iItemType, iItemLevel);
}

int GFxProcess::GetItemHotKey(int iHotKey)
{
	CGFxMainUi* temp = (CGFxMainUi*)Find(GFxRegistType::eGFxRegist_MainUI);

	if(temp)
	{
		return temp->GetItemHotKey(iHotKey);
	}

	return -1;
}

int GFxProcess::GetItemHotKeyLevel(int iHotKey)
{
	CGFxMainUi* temp = (CGFxMainUi*)Find(GFxRegistType::eGFxRegist_MainUI);

	if(temp)
	{
		return temp->GetItemHotKeyLevel(iHotKey);
	}

	return 0;
}


#endif //LDK_ADD_SCALEFORM
