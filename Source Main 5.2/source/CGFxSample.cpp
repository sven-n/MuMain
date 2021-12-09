
#include "stdafx.h"

#if defined LDK_ADD_SCALEFORM && defined GFX_UI_TEST_CODE

#include <assert.h>
#include "GSysFile.h"

#include "ZzzTexture.h"
#include "CGFxSample.h"

//open TGA

//open jpg

GFile* Fx3DDemoFileOpener::OpenFile(const char *pfilename)
{
	GFile * temp = new GSysFile(pfilename);

	return temp;
}

GImageInfoBase* CustomImageCreator::CreateImage(const GFxImageCreateInfo &info)
{   
	GPtr<GImage> pimage;

	switch(info.Type)
	{
	case GFxImageCreateInfo::Input_Image:
		// We have to pass correct size; it is required at least
		// when we are initializing image info with a texture.
		return new GImageInfo(info.pImage, info.pImage->Width, info.pImage->Height);

	case GFxImageCreateInfo::Input_File:
		{
//*/
  			GPtr<GTextureOGL>texture = pRenderer->CreateTexture();
  			texture->InitTexture(  Bitmaps.GetTexture(BITMAP_TEST_UI_IMAGE)->BitmapIndex );
  			GImageInfo * imageInfo = new GImageInfo( (GTexture*)texture, info.pFileInfo->TargetWidth, info.pFileInfo->TargetHeight );
			if(!imageInfo)
				return 0;

			return imageInfo;
/*/ 
			// If we got here, we are responsible for loading an image file.
			//GPtr<GFile> pfile  = *info.pFileOpener->OpenFile(info.pFileInfo->FileName.ToCStr());
			GPtr<GFile> pfile  = fileOpener.OpenFile(info.pFileInfo->FileName.ToCStr());

			if (!pfile)
				return 0;

			// Load an image into GImage object.
			pimage = *GFxImageCreator::LoadBuiltinImage(pfile, info.pFileInfo->Format, info.Use,
				info.pLog, info.pJpegSupport, info.pPNGSupport, NULL);

			if (!pimage)
				return 0;

			return new GImageInfo(pimage, info.pFileInfo->TargetWidth, 
				info.pFileInfo->TargetHeight);
//*/
		}        
		break;

		// No input - empty image info.
	case GFxImageCreateInfo::Input_None:
	default:
		break;
	}
	return new GImageInfo();
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GFxSample* GFxSample::Make(GFxRegistType _UIType, mapGFXContainer *_GfxRepo, const char* _pfilename, UInt _loadConstants, UPInt _memoryArena, UInt _renderFlag, GFxMovieView::ScaleModeType _scaleType, FontConfig* _pconfig)
{
	if(_GfxRepo == NULL)
		return NULL;

	GFxSample* temp( new GFxSample );

	if(!temp->InitGFx(_pfilename, _loadConstants, _memoryArena, _renderFlag, _scaleType, _pconfig))
	{
		delete temp;
		temp = NULL;

		//error log

		return NULL;
	}

	_GfxRepo->insert(make_pair(_UIType, temp));

	return temp;
}

GFxSample::GFxSample()
{
	isVisible = FALSE;
	isLock = FALSE;
	isWireFrame = FALSE;
	MovieLastTime = 0;
}

GFxSample::~GFxSample()
{
	isVisible = FALSE;

	pUIMovie = NULL;
	pUIMovieDef = NULL;

	pFSHandler->Release();
	pEIHandler->Release();
}

bool GFxSample::OnCreateDevice(SInt bufw, SInt bufh, SInt left, SInt top, SInt w, SInt h, UInt flags)
{
	if(!pUIMovie)
		return FALSE;

	if(!pRenderer->SetDependentVideoMode())
		return FALSE;

	pUIMovie->SetViewport(bufw, bufh, left, top, w, h, flags);

	return TRUE;
}

bool GFxSample::OnResetDevice()
{
	// openGL 상에서 resetDevice 하는 방법 모르겠음
	//this->InitGFx();
	return TRUE;
}

bool GFxSample::OnLostDevice()
{
	if(pRenderer->ResetVideoMode())
		return FALSE;

	return TRUE;
}

bool GFxSample::OnDestroyDevice()
{
	return TRUE;
}

bool GFxSample::InitGFx(const char* _pfilename, UInt _loadConstants, UPInt _memoryArena, UInt _renderFlag, GFxMovieView::ScaleModeType _scaleType, FontConfig* _pconfig)
{
#ifdef GFX_UI_TEST_CODE
	LoadBitmap("Interface\\GFx\\hpguege_I1.tga", BITMAP_TEST_UI_IMAGE, GL_LINEAR);
#endif //GFX_UI_TEST_CODE

	if (_pconfig)
		_pconfig->Apply(&gfxLoader);

	gfxLoader.SetLog(GPtr<GFxLog>(*new GFxPlayerLog()));

	pFSHandler = new SampleFSCHandler;
	gfxLoader.SetFSCommandHandler(GPtr<GFxFSCommandHandler>(pFSHandler));

	pEIHandler = new SampleEIHandler;
	gfxLoader.SetExternalInterface(GPtr<GFxExternalInterface>(pEIHandler));

	pRenderer = *GRendererOGL::CreateRenderer();
	pRenderer->SetDependentVideoMode();

	pRenderConfig = *new GFxRenderConfig(pRenderer, _renderFlag);
	if(!pRenderer || !pRenderConfig)
		return FALSE;
	gfxLoader.SetRenderConfig(pRenderConfig);

	//pRenderConfig->SetRenderFlags(pRenderConfig->GetRenderFlags()| GFxRenderConfig::RF_EdgeAA | GFxRenderConfig::RF_StrokeNormal);

	GFxMovieInfo newMovieInfo;
	// Get info about the width & height of the movie.
	if (!gfxLoader.GetMovieInfo(_pfilename, &newMovieInfo, 0, _loadConstants)) 
	{
		return FALSE;
	}

// 	GPtr<GFxFileOpener> pfileOpener = *new GFxFileOpener;
	GPtr<GFxFileOpener> pfileOpener = *new Fx3DDemoFileOpener;
	gfxLoader.SetFileOpener(pfileOpener);

	CustomImageCreator *pCimageCreator = new CustomImageCreator;
	pCimageCreator->pRenderer = pRenderer;
	gfxLoader.SetImageCreator(GPtr<GFxImageCreator>(pCimageCreator));

//	GPtr<GFxImageCreator> imageCreator = *new CustomImageCreator;
//	gfxLoader.SetImageCreator(imageCreator);

	pUIMovieDef = *(gfxLoader.CreateMovie(_pfilename, _loadConstants, _memoryArena));
	if(!pUIMovieDef)
		return FALSE;

	pUIMovie = *pUIMovieDef->CreateInstance(GFxMovieDef::MemoryParams(), true);
	if(!pUIMovie)
		return FALSE;

	// Release cached memory used by previous file.
	if (pUIMovie->GetMeshCacheManager())
		pUIMovie->GetMeshCacheManager()->ClearCache();

	pUIMovie->SetViewScaleMode(_scaleType);

	// Set a reference to the app
	pUIMovie->SetUserData(this);

	pUIMovie->Advance(0.0f, 0);
	pUIMovie->SetBackgroundAlpha(0.0f);

	MovieLastTime = timeGetTime();

	return TRUE;
}

bool GFxSample::Update()
{
	if(isVisible == FALSE)
		return TRUE;

	if(!pUIMovie)
		return FALSE;

	// 	char str[256];
	// 	float outX = 0;
	// 	float outY = 0;
	// 	float outZ = 0;
	// 
	// 	pEIHandler->GetAngleX(&outX);
	// 	pEIHandler->GetAngleY(&outY);
	// 	pEIHandler->GetAngleZ(&outZ);
	// 
	// 	sprintf_s(str, "Axis X - %f \nAxis Y - %f \nAxis Z - %f", outX, outY, outZ);
	// 	bool check = pUIMovie->SetVariable("_root.hud.MessageText.text", str);

	return TRUE;
}

bool GFxSample::Render()
{
	if(isVisible == FALSE)
		return TRUE;

	if(!pUIMovie)
		return FALSE;

	DWORD mtime = timeGetTime();
	float deltaTime = ((float)(mtime - MovieLastTime)) / 1000.0f;
	MovieLastTime = mtime;

	if(isWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	pUIMovie->Advance(deltaTime, 0);
	pUIMovie->Display();

	if(isWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	return TRUE;
}

//GPtr<GFxMovieView> GFxSample::GetMovie()
GFxMovieView* GFxSample::GetMovie()
{
	if(!pUIMovie)
		return NULL;

	return pUIMovie; 
}

//--------------------------------------------------------------------------------------
// FSCommand Handler
//--------------------------------------------------------------------------------------
void SampleFSCHandler::Callback(GFxMovieView* pmovie, const char* pcommand, const char* parg)
{
	//GFxPrintf("FSCommand: %s, Args: %s\n", pcommand, parg);
}

//--------------------------------------------------------------------------------------
// ExternalInterface Handler
//--------------------------------------------------------------------------------------
void SampleEIHandler::Callback(GFxMovieView* pmovieView, const char* methodName, const GFxValue* args, UInt argCount)
{
	//GFxPrintf("ExternalInterface: %s, %d args: ", methodName, argCount);
	for(UInt i = 0; i < argCount; i++)
	{
		switch(args[i].GetType())
		{
		case GFxValue::VT_Null:
			//GFxPrintf("NULL");
			break;
		case GFxValue::VT_Boolean:
			//GFxPrintf("%s", args[i].GetBool() ? "true" : "false");
			break;
		case GFxValue::VT_Number:
			//GFxPrintf("%3.3f", args[i].GetNumber());
			break;
		case GFxValue::VT_String:
			//GFxPrintf("%s", args[i].GetString());
			break;
		default:;
			//GFxPrintf("unknown");
			break;
		}
		//GFxPrintf("%s", (i == argCount - 1) ? "" : ", ");
	}
	//GFxPrintf("\n");

	if(strcmp(methodName, "axisX") == 0 && argCount == 1 && args[0].GetType() == GFxValue::VT_Number)
	{
		angleX = (FLOAT)args[0].GetNumber();
	}
	else if(strcmp(methodName, "axisY") == 0 && argCount == 1 && args[0].GetType() == GFxValue::VT_Number)
	{
		angleY = (FLOAT)args[0].GetNumber();
	}
	else if(strcmp(methodName, "axisZ") == 0 && argCount == 1 && args[0].GetType() == GFxValue::VT_Number)
	{
		angleZ = (FLOAT)args[0].GetNumber();
	}
}

#endif //LDK_ADD_SCALEFORM && defined GFX_UI_TEST_CODE
