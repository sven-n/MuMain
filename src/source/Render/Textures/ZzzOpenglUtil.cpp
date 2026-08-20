///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Renderer/RenderUtils.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "UI/NewUI/NewUISystem.h"
#include <SDL3/SDL.h>
#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
#include "wglext.h"  // legacy WGL multisample pixel-format path (disabled by default)
#endif
#include "Camera/CameraProjection.h"
#include "Camera/CameraManager.h"
#include "Camera/CameraMode.h"
#include "Camera/CameraConfig.h"

#ifdef _EDITOR
extern "C" bool DevEditor_IsCameraOverrideEnabled(const char* cameraName);
extern "C" void DevEditor_GetCameraFogRange(const char* cameraName, float viewFar, float* outStart, float* outEnd);
#endif

extern "C" CameraManager& CameraManager_Instance();

int     OpenglWindowX = 0;
int     OpenglWindowY = 0;
int     OpenglWindowWidth = 1024;
int     OpenglWindowHeight = 768;
vec3_t  MousePosition;
vec3_t  MouseTarget;
bool    FogEnable = true;
GLfloat FogDensity = 0.0004f;
GLfloat FogColor[4] = { 30 / 256.f,20 / 256.f,10 / 256.f, };

bool _isVSyncAvailable = false;
bool _isVSyncEnabled = false;

unsigned int WindowWidth = 1024;
unsigned int WindowHeight = 768;
int          MouseX = WindowWidth / 2;
int          MouseY = WindowHeight / 2;
int          BackMouseX = MouseX;
int          BackMouseY = MouseY;
bool         MouseLButton;
bool 		 MouseLButtonPop;
bool 		 MouseLButtonPush;
bool         MouseRButton;
bool 		 MouseRButtonPop;
bool 		 MouseRButtonPush;
bool 	   	 MouseLButtonDBClick;
bool         MouseMButton;
bool         MouseMButtonPop;
bool         MouseMButtonPush;
int          MouseWheel;
DWORD		 MouseRButtonPress = 0;

//bool    showShoppingMall = false;

void OpenExploper(wchar_t* Name, wchar_t* para)
{
    ShellExecute(NULL, L"open", Name, para, L"", SW_SHOW);
}

bool CheckID_HistoryDay(wchar_t* Name, WORD day)
{
    typedef struct  __day_history__
    {
        wchar_t ID[MAX_USERNAME_SIZE + 1];
        WORD date;
    }dayHistory;

    FILE* fp;
    dayHistory days[100];
    int   count = 0;
    WORD  num = 0;
    bool  sameName = false;
    bool  update = true;

    if ((fp = _wfopen(L"dconfig.ini", L"rb")) != NULL)
    {
        fread(&num, sizeof(WORD), 1, fp);

        if (num > 100)
        {
            num = 0;
        }
        else
        {
            for (int i = 0; i < num; ++i)
            {
                fread(days[i].ID, sizeof(char), MAX_USERNAME_SIZE + 1, fp);
                fread(&days[i].date, sizeof(WORD), 1, fp);

                if (!wcscmp(days[i].ID, Name))
                {
                    sameName = true;
                    if (days[i].date == day)
                    {
                        update = false;
                        break;
                    }
                    days[i].date = day;
                }
                count++;
            }
        }
        fclose(fp);
    }

    if (update)
    {
        if (!sameName)
        {
            memcpy(days[num].ID, Name, (MAX_USERNAME_SIZE + 1) * sizeof(char));
            days[num].date = day;

            num++;
        }

        fp = _wfopen(L"dconfig.ini", L"wb");

        fwrite(&num, sizeof(WORD), 1, fp);
        for (int i = 0; i < num; ++i)
        {
            fwrite(days[i].ID, sizeof(char), MAX_USERNAME_SIZE + 1, fp);
            fwrite(&days[i].date, sizeof(WORD), 1, fp);
        }

        fclose(fp);
    }

    //    showShoppingMall = update;

    return  update;
}

bool GrabEnable = false;
wchar_t GrabFileName[MAX_PATH];
int  GrabScreen = 0;

///////////////////////////////////////////////////////////////////////////////
// opengl render util
///////////////////////////////////////////////////////////////////////////////

int  CachTexture = -1;
bool TextureEnable;
bool DepthTestEnable;
bool CullFaceEnable;
bool DepthMaskEnable;
bool AlphaTestEnable;
int  AlphaBlendType;

void BindTexture(int tex)
{
    // Always forward to the renderer. In the deferred SDL renderer this only
    // updates logical state, and caching can desync when callers bind directly.
    mu::GetRenderer().BindTexture(tex);
}

bool TextureStream = false;

extern  int test;
void BindTextureStream(int tex)
{
    mu::GetRenderer().BindTexture(tex);
    TextureStream = true;
}

void EndTextureStream()
{
    TextureStream = false;
}

void SetAlphaFuncRef(float ref)
{
    glAlphaFunc(GL_GREATER, ref);
}

void EnableTexture2D()
{
    glEnable(GL_TEXTURE_2D);
}
void DisableTexture2D()
{
    glDisable(GL_TEXTURE_2D);
}
void EnableAlphaTestRaw()
{
    glEnable(GL_ALPHA_TEST);
}
void DisableAlphaTestRaw()
{
    glDisable(GL_ALPHA_TEST);
}
void EnableFog()
{
    glEnable(GL_FOG);
}
void DisableFog()
{
    glDisable(GL_FOG);
}
void EnableBlend()
{
    glEnable(GL_BLEND);
}
void DisableBlend()
{
    glDisable(GL_BLEND);
}
void SetBlendFuncAlpha()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
void SetDepthFuncLEqual()
{
    glDepthFunc(GL_LEQUAL);
}
void ClearColorBuffer()
{
    glClear(GL_COLOR_BUFFER_BIT);
}
void ClearDepthBuffer()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}
void ClearColorAndDepthBuffers()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void SetClearColor(float r, float g, float b, float a)
{
    glClearColor(r, g, b, a);
}
void FlushGL()
{
    glFlush();
}

void EnableDepthTest()
{
    if (!DepthTestEnable)
    {
        DepthTestEnable = true;
        mu::GetRenderer().SetDepthTest(true);
    }
}

void DisableDepthTest()
{
    if (DepthTestEnable)
    {
        DepthTestEnable = false;
        mu::GetRenderer().SetDepthTest(false);
    }
}

void EnableDepthMask()
{
    if (!DepthMaskEnable)
    {
        DepthMaskEnable = true;
        mu::GetRenderer().SetDepthMask(true);
    }
}

void DisableDepthMask()
{
    if (DepthMaskEnable)
    {
        DepthMaskEnable = false;
        mu::GetRenderer().SetDepthMask(false);
    }
}

void EnableCullFace()
{
    if (!CullFaceEnable)
    {
        CullFaceEnable = true;
        mu::GetRenderer().SetCullFace(true);
    }
}

void DisableCullFace()
{
    if (CullFaceEnable)
    {
        CullFaceEnable = false;
        mu::GetRenderer().SetCullFace(false);
    }
}

void DisableTexture(bool AlphaTest)
{
    EnableDepthMask();
    if (AlphaTest == true)
    {
        if (!AlphaTestEnable)
        {
            AlphaTestEnable = true;
            mu::GetRenderer().SetAlphaTest(true);
        }
    }
    else
    {
        if (AlphaTestEnable)
        {
            AlphaTestEnable = false;
            mu::GetRenderer().SetAlphaTest(false);
        }
    }
    if (TextureEnable)
    {
        TextureEnable = false;
        mu::GetRenderer().SetTexture2D(false);
    }
}

void DisableAlphaBlend()
{
    if (AlphaBlendType != 0)
    {
        AlphaBlendType = 0;
        mu::GetRenderer().DisableBlend();
    }
    EnableCullFace();
    EnableDepthMask();
    if (AlphaTestEnable)
    {
        AlphaTestEnable = false;
        mu::GetRenderer().SetAlphaTest(false);
    }
    if (!TextureEnable)
    {
        TextureEnable = true;
        mu::GetRenderer().SetTexture2D(true);
    }
    if (FogEnable)
        mu::GetRenderer().SetFogEnabled(true);
}

void EnableAlphaTest(bool DepthMask)
{
    if (AlphaBlendType != 2)
    {
        AlphaBlendType = 2;
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Alpha);
    }
    DisableCullFace();
    if (DepthMask)
        EnableDepthMask();
    if (!AlphaTestEnable)
    {
        AlphaTestEnable = true;
        mu::GetRenderer().SetAlphaTest(true);
    }
    if (!TextureEnable)
    {
        TextureEnable = true;
        mu::GetRenderer().SetTexture2D(true);
    }
    if (FogEnable)
        mu::GetRenderer().SetFogEnabled(true);
}

void EnableAlphaBlend()
{
    if (AlphaBlendType != 3)
    {
        AlphaBlendType = 3;
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Glow);
    }
    DisableCullFace();
    DisableDepthMask();
    if (AlphaTestEnable)
    {
        AlphaTestEnable = false;
        mu::GetRenderer().SetAlphaTest(false);
    }
    if (!TextureEnable)
    {
        TextureEnable = true;
        mu::GetRenderer().SetTexture2D(true);
    }
    if (FogEnable)
        mu::GetRenderer().SetFogEnabled(false);
}

void EnableAlphaBlendMinus()
{
    if (AlphaBlendType != 4)
    {
        AlphaBlendType = 4;
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Subtract);
    }
    DisableCullFace();
    DisableDepthMask();
    if (AlphaTestEnable)
    {
        AlphaTestEnable = false;
        mu::GetRenderer().SetAlphaTest(false);
    }
    if (!TextureEnable)
    {
        TextureEnable = true;
        mu::GetRenderer().SetTexture2D(true);
    }
    if (FogEnable)
        mu::GetRenderer().SetFogEnabled(true);
}

void EnableAlphaBlend2()
{
    if (AlphaBlendType != 5)
    {
        AlphaBlendType = 5;
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Luminance);
    }
    DisableCullFace();
    DisableDepthMask();
    if (AlphaTestEnable)
    {
        AlphaTestEnable = false;
        mu::GetRenderer().SetAlphaTest(false);
    }
    if (!TextureEnable)
    {
        TextureEnable = true;
        mu::GetRenderer().SetTexture2D(true);
    }
    if (FogEnable)
        mu::GetRenderer().SetFogEnabled(true);
}

void EnableAlphaBlend3()
{
    if (AlphaBlendType != 6)
    {
        AlphaBlendType = 6;
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Alpha);
    }
    DisableCullFace();
    DisableDepthMask();
    if (AlphaTestEnable)
    {
        AlphaTestEnable = false;
        mu::GetRenderer().SetAlphaTest(false);
    }
    if (!TextureEnable)
    {
        TextureEnable = true;
        mu::GetRenderer().SetTexture2D(true);
    }
    if (FogEnable)
        mu::GetRenderer().SetFogEnabled(true);
}

void EnableAlphaBlend4()
{
    if (AlphaBlendType != 7)
    {
        AlphaBlendType = 7;
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Mixed);
    }
    DisableCullFace();
    DisableDepthMask();
    if (AlphaTestEnable)
    {
        AlphaTestEnable = false;
        mu::GetRenderer().SetAlphaTest(false);
    }
    if (!TextureEnable)
    {
        TextureEnable = true;
        mu::GetRenderer().SetTexture2D(true);
    }
    if (FogEnable)
        mu::GetRenderer().SetFogEnabled(true);
}

void EnableLightMap()
{
    if (AlphaBlendType != 1)
    {
        AlphaBlendType = 1;
        mu::GetRenderer().SetBlendMode(mu::BlendMode::LightMap);
    }
    EnableCullFace();
    EnableDepthMask();
    if (AlphaTestEnable)
    {
        AlphaTestEnable = false;
        mu::GetRenderer().SetAlphaTest(false);
    }
    if (!TextureEnable)
    {
        TextureEnable = true;
        mu::GetRenderer().SetTexture2D(true);
    }
    if (FogEnable)
        mu::GetRenderer().SetFogEnabled(true);
}

void SetRenderViewport(int x, int y, int Width, int Height)
{
    OpenglWindowX = x;
    OpenglWindowY = y;
    // NOTE: Do NOT update OpenglWindowWidth/Height here!
    // These represent the FULL window dimensions for UI rendering,
    // while Width/Height here are the game viewport (which may be smaller due to UI bars)
    // OpenglWindowWidth/Height are set once at startup and updated only on window resize
    CameraProjection::SetViewport(x, y, Width, Height);
}

// Saved camera state for save/restore around item rendering blocks.
// Item rendering calls gluPerspective2 (corrupts PerspectiveX/Y/ScreenCenter)
// and GetOpenGLMatrix(g_Camera.Matrix) (corrupts the camera matrix). Both must
// be restored so ScreenToWorldRay reads correct values for click detection.
static struct
{
    float PerspectiveX, PerspectiveY;
    int ScreenCenterX, ScreenCenterY, ScreenCenterYFlip;
    float Matrix[3][4];
} s_SavedCameraState;

void SaveCameraPerspective()
{
    s_SavedCameraState.PerspectiveX     = g_Camera.PerspectiveX;
    s_SavedCameraState.PerspectiveY     = g_Camera.PerspectiveY;
    s_SavedCameraState.ScreenCenterX    = g_Camera.ScreenCenterX;
    s_SavedCameraState.ScreenCenterY    = g_Camera.ScreenCenterY;
    s_SavedCameraState.ScreenCenterYFlip = g_Camera.ScreenCenterYFlip;
    memcpy(s_SavedCameraState.Matrix, g_Camera.Matrix, sizeof(g_Camera.Matrix));
}

void RestoreCameraPerspective()
{
    g_Camera.PerspectiveX     = s_SavedCameraState.PerspectiveX;
    g_Camera.PerspectiveY     = s_SavedCameraState.PerspectiveY;
    g_Camera.ScreenCenterX    = s_SavedCameraState.ScreenCenterX;
    g_Camera.ScreenCenterY    = s_SavedCameraState.ScreenCenterY;
    g_Camera.ScreenCenterYFlip = s_SavedCameraState.ScreenCenterYFlip;
    memcpy(g_Camera.Matrix, s_SavedCameraState.Matrix, sizeof(g_Camera.Matrix));
}

// Perspective setup for item/3D-UI rendering. Sets GL perspective AND updates
// g_Camera perspective cache so item rendering can compute screen positions.
// Callers should wrap the entire item-rendering block in SaveCameraPerspective /
// RestoreCameraPerspective to avoid leaking FOV=1 values to ScreenToWorldRay.
void gluPerspective2(float Fov, float Aspect, float ZNear, float ZFar)
{
    gluPerspective(Fov, Aspect, ZNear, ZFar);

    g_Camera.ScreenCenterX = OpenglWindowX + OpenglWindowWidth / 2;
    g_Camera.ScreenCenterY = OpenglWindowY + OpenglWindowHeight / 2;
    g_Camera.ScreenCenterYFlip = WindowHeight - g_Camera.ScreenCenterY;

    float fovRad = Fov * 0.5f * Q_PI / 180.0f;
    g_Camera.PerspectiveX = tanf(fovRad) / (float)(OpenglWindowWidth / 2) * Aspect;
    g_Camera.PerspectiveY = tanf(fovRad) / (float)(OpenglWindowHeight / 2);
}

float ConvertX(float x)
{
    return x * (float)WindowWidth / (float)REFERENCE_WIDTH;
}

float ConvertY(float y)
{
    return y * (float)WindowHeight / (float)REFERENCE_HEIGHT;
}

void BeginOpengl(int x, int y, int Width, int Height)
{
    x = x * WindowWidth / REFERENCE_WIDTH;
    y = y * WindowHeight / REFERENCE_HEIGHT;
    Width = Width * WindowWidth / REFERENCE_WIDTH;
    Height = Height * WindowHeight / REFERENCE_HEIGHT;

    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    SetRenderViewport(x, y, Width, Height);

    // Calculate aspect ratio dynamically from viewport dimensions
    // This ensures camera adapts to window resizing (WM_SIZE updates WindowWidth/WindowHeight)
    // FOV stays constant, aspect ratio scales the horizontal view accordingly
    float aspectRatio = (float)Width / (float)Height;

    // Apply RENDER_DISTANCE_MULTIPLIER for consistent rendering distance across all systems
    CameraProjection::SetupPerspective(g_Camera, g_Camera.FOV, aspectRatio, g_Camera.ViewNear, g_Camera.ViewFar * RENDER_DISTANCE_MULTIPLIER);

    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    mu::GetRenderer().Rotate(g_Camera.Angle[1], 0.f, 1.f, 0.f);
    if (g_Camera.TopViewEnable == false)
        mu::GetRenderer().Rotate(g_Camera.Angle[0], 1.f, 0.f, 0.f);
    mu::GetRenderer().Rotate(g_Camera.Angle[2], 0.f, 0.f, 1.f);
    mu::GetRenderer().Translate(-g_Camera.Position[0], -g_Camera.Position[1], -g_Camera.Position[2]);

    mu::GetRenderer().SetAlphaTest(false);
    mu::GetRenderer().SetTexture2D(true);
    mu::GetRenderer().SetDepthTest(true);
    mu::GetRenderer().SetCullFace(true);
    mu::GetRenderer().SetDepthMask(true);

    AlphaTestEnable = false;
    TextureEnable = true;
    DepthTestEnable = true;
    CullFaceEnable = true;
    DepthMaskEnable = true;
    mu::GetRenderer().SetDepthFunc(GL_LEQUAL);
    mu::GetRenderer().SetAlphaFunc(GL_GREATER, 0.25f);
    if (FogEnable)
    {
        // Fog scales dynamically with view distance (g_Camera.ViewFar) so it
        // stays at consistent percentages when zooming. The actual GL far clip
        // plane is at g_Camera.ViewFar * RENDER_DISTANCE_MULTIPLIER (1.4 — see
        // SetupPerspective), so fogStart=1.0×/fogEnd=1.25× both sit inside
        // the visible range and fog completes before the clip plane.
        float fogStart = g_Camera.ViewFar * 1.00f;
        float fogEnd   = g_Camera.ViewFar * 1.25f;

#ifdef _EDITOR
        // Allow DevEditor to override fog percentages for the active camera.
        // Active camera name is used so Default/Orbital have independent fog
        // start/end percentages.
        if (ICamera* active = CameraManager_Instance().GetActiveCamera())
        {
            const char* name = active->GetName();
            if (DevEditor_IsCameraOverrideEnabled(name))
                DevEditor_GetCameraFogRange(name, g_Camera.ViewFar, &fogStart, &fogEnd);
        }
#endif

        mu::FogParams fogParams{};
        fogParams.mode = GL_LINEAR;
        fogParams.start = fogStart;
        fogParams.end = fogEnd;
        fogParams.density = FogDensity;
        fogParams.color[0] = FogColor[0];
        fogParams.color[1] = FogColor[1];
        fogParams.color[2] = FogColor[2];
        fogParams.color[3] = FogColor[3];
        mu::GetRenderer().SetFog(fogParams);
    }
    else
    {
        mu::GetRenderer().SetFogEnabled(false);
    }

    CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
}

void EndOpengl()
{
    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PopMatrix();
    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PopMatrix();
}

void UpdateMousePositionn()
{
    vec3_t vPos;

    mu::GetRenderer().LoadIdentity();
    mu::GetRenderer().Translate(-g_Camera.Position[0], -g_Camera.Position[1], -g_Camera.Position[2]);
    CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);

    Vector(-g_Camera.Matrix[0][3], -g_Camera.Matrix[1][3], -g_Camera.Matrix[2][3], vPos);
    VectorIRotate(vPos, g_Camera.Matrix, MousePosition);
}

#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
BOOL IsGLExtensionSupported(const wchar_t* extension)
{
    (void)extension;
    return FALSE;
}

#endif // LDS_ADD_MULTISAMPLEANTIALIASING

void InitVSync()
{
    // SDL GPU swapchains always support VSync. The renderer applies present-mode changes.
    _isVSyncAvailable = true;
}

bool IsVSyncAvailable()
{
    return _isVSyncAvailable;
}

bool IsVSyncEnabled()
{
    return _isVSyncEnabled;
}

bool EnableVSync()
{
    if (mu::GetRenderer().SetVSyncEnabled(true))
    {
        _isVSyncEnabled = true;
        return true;
    }

    _isVSyncEnabled = false;
    return false;
}

bool DisableVSync()
{
    if (mu::GetRenderer().SetVSyncEnabled(false))
    {
        _isVSyncEnabled = false;
        return true;
    }

    _isVSyncEnabled = true;
    return false;
}

// GetFPSLimit() lives in the platform layer (Winmain.cpp): it queries the
// monitor refresh rate, which SDL exposes per display (issue #442).

#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
BOOL InitGLMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pfd, int iRequestMSAAValue, int& OutiPixelFormat)
{
    (void)hInstance;
    (void)hWnd;
    (void)pfd;
    (void)iRequestMSAAValue;
    (void)OutiPixelFormat;
    return FALSE;
}

void SetEnableMultisample()
{
    if (TRUE == g_bSupportedMSAA)
    {
        mu::GetRenderer().SetMultisample(true);
    }

#if defined(_DEBUG)
    CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
}

void SetDisableMultisample()
{
    if (TRUE == g_bSupportedMSAA)
    {
        mu::GetRenderer().SetMultisample(false);
    }

#if defined(_DEBUG)
    CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
}

#endif // LDS_ADD_MULTISAMPLEANTIALIASING

///////////////////////////////////////////////////////////////////////////////
// render util
///////////////////////////////////////////////////////////////////////////////

void RenderBox(float Matrix[3][4])
{
    vec3_t BoundingBoxMin;
    vec3_t BoundingBoxMax;
    Vector(-10.f, -30.f, -10.f, BoundingBoxMin);
    Vector(10.f, 0.f, 10.f, BoundingBoxMax);

    vec3_t BoundingVertices[8];
    Vector(BoundingBoxMax[0], BoundingBoxMax[1], BoundingBoxMax[2], BoundingVertices[0]);
    Vector(BoundingBoxMax[0], BoundingBoxMax[1], BoundingBoxMin[2], BoundingVertices[1]);
    Vector(BoundingBoxMax[0], BoundingBoxMin[1], BoundingBoxMax[2], BoundingVertices[2]);
    Vector(BoundingBoxMax[0], BoundingBoxMin[1], BoundingBoxMin[2], BoundingVertices[3]);
    Vector(BoundingBoxMin[0], BoundingBoxMax[1], BoundingBoxMax[2], BoundingVertices[4]);
    Vector(BoundingBoxMin[0], BoundingBoxMax[1], BoundingBoxMin[2], BoundingVertices[5]);
    Vector(BoundingBoxMin[0], BoundingBoxMin[1], BoundingBoxMax[2], BoundingVertices[6]);
    Vector(BoundingBoxMin[0], BoundingBoxMin[1], BoundingBoxMin[2], BoundingVertices[7]);

    vec3_t TransformVertices[8];
    for (int j = 0; j < 8; j++)
    {
        VectorTransform(BoundingVertices[j], Matrix, TransformVertices[j]);
    }

    const auto MakeVtx = [](const vec3_t& pos, float u, float v, std::uint32_t color) -> mu::Vertex3D
    { return {pos[0], pos[1], pos[2], 0.f, 0.f, 1.f, u, v, color}; };
    std::vector<mu::Vertex3D> verts;
    verts.reserve(36);
    const auto EmitQuad = [&](const vec3_t& a, const vec3_t& b, const vec3_t& c, const vec3_t& d, std::uint32_t color)
    {
        verts.push_back(MakeVtx(a, 1.f, 1.f, color));
        verts.push_back(MakeVtx(b, 1.f, 0.f, color));
        verts.push_back(MakeVtx(c, 0.f, 0.f, color));
        verts.push_back(MakeVtx(a, 1.f, 1.f, color));
        verts.push_back(MakeVtx(c, 0.f, 0.f, color));
        verts.push_back(MakeVtx(d, 0.f, 1.f, color));
    };
    EmitQuad(TransformVertices[7], TransformVertices[6], TransformVertices[4], TransformVertices[5], mu::PackABGR(.2f, .2f, .2f, 1.f));
    EmitQuad(TransformVertices[0], TransformVertices[2], TransformVertices[3], TransformVertices[1], mu::PackABGR(.2f, .2f, .2f, 1.f));
    EmitQuad(TransformVertices[7], TransformVertices[3], TransformVertices[2], TransformVertices[6], mu::PackABGR(.6f, .6f, .6f, 1.f));
    EmitQuad(TransformVertices[0], TransformVertices[1], TransformVertices[5], TransformVertices[4], mu::PackABGR(.6f, .6f, .6f, 1.f));
    EmitQuad(TransformVertices[7], TransformVertices[5], TransformVertices[1], TransformVertices[3], mu::PackABGR(.4f, .4f, .4f, 1.f));
    EmitQuad(TransformVertices[0], TransformVertices[4], TransformVertices[6], TransformVertices[2], mu::PackABGR(.4f, .4f, .4f, 1.f));
    mu::GetRenderer().RenderTriangles(verts, 0u);
}

void RenderPlane3D(float Width, float Height, float Matrix[3][4])
{
    vec3_t BoundingVertices[4];
    Vector(-Width, -Width, Height, BoundingVertices[3]);
    Vector(Width, Width, Height, BoundingVertices[2]);
    Vector(Width, Width, -Height, BoundingVertices[1]);
    Vector(-Width, -Width, -Height, BoundingVertices[0]);

    vec3_t TransformVertices[4];
    for (int j = 0; j < 4; j++)
    {
        VectorTransform(BoundingVertices[j], Matrix, TransformVertices[j]);
    }

    const mu::Vertex3D verts[6] = {
        {TransformVertices[0][0], TransformVertices[0][1], TransformVertices[0][2], 0.f, 0.f, 1.f, 0.f, 1.f, 0xFFFFFFFFu},
        {TransformVertices[1][0], TransformVertices[1][1], TransformVertices[1][2], 0.f, 0.f, 1.f, 1.f, 1.f, 0xFFFFFFFFu},
        {TransformVertices[2][0], TransformVertices[2][1], TransformVertices[2][2], 0.f, 0.f, 1.f, 1.f, 0.f, 0xFFFFFFFFu},
        {TransformVertices[0][0], TransformVertices[0][1], TransformVertices[0][2], 0.f, 0.f, 1.f, 0.f, 1.f, 0xFFFFFFFFu},
        {TransformVertices[2][0], TransformVertices[2][1], TransformVertices[2][2], 0.f, 0.f, 1.f, 1.f, 0.f, 0xFFFFFFFFu},
        {TransformVertices[3][0], TransformVertices[3][1], TransformVertices[3][2], 0.f, 0.f, 1.f, 0.f, 0.f, 0xFFFFFFFFu},
    };
    mu::GetRenderer().RenderTriangles(verts, 0u);
}

void BeginSprite()
{
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
}

void EndSprite()
{
    mu::GetRenderer().PopMatrix();
}

void RenderSprite(int Texture, vec3_t Position, float Width, float Height, vec3_t Light, float Rotation, float u, float v, float uWidth, float vHeight)
{
    BindTexture(Texture);

    vec3_t p2;
    VectorTransform(Position, g_Camera.Matrix, p2);
    //VectorCopy(Position,p2);
    float x = p2[0];
    float y = p2[1];
    float z = p2[2];

    if (z >= -1.0f)
    {
        return;
    }

    Width *= 0.5f;
    Height *= 0.5f;

    vec3_t p[4];
    if (Rotation == 0)
    {
        Vector(x - Width, y - Height, z, p[0]);
        Vector(x + Width, y - Height, z, p[1]);
        Vector(x + Width, y + Height, z, p[2]);
        Vector(x - Width, y + Height, z, p[3]);
    }
    else
    {
        vec3_t p2[4];
        Vector(-Width, -Height, z, p2[0]);
        Vector(Width, -Height, z, p2[1]);
        Vector(Width, Height, z, p2[2]);
        Vector(-Width, Height, z, p2[3]);
        vec3_t Angle;
        Vector(0.f, 0.f, Rotation, Angle);
        float Matrix[3][4];
        AngleMatrix(Angle, Matrix);
        for (int i = 0; i < 4; i++)
        {
            VectorRotate(p2[i], Matrix, p[i]);
            p[i][0] += x;
            p[i][1] += y;
        }
    }

    float c[4][2];
    TEXCOORD(c[3], u, v);
    TEXCOORD(c[2], u + uWidth, v);
    TEXCOORD(c[1], u + uWidth, v + vHeight);
    TEXCOORD(c[0], u, v + vHeight);

    std::uint32_t color;
    if (Bitmaps[Texture].Components == 3)
    {
        color = mu::PackABGR(Light[0], Light[1], Light[2], 1.0f);
    }
    else
    {
        if (Texture == BITMAP_BLOOD + 1 || Texture == BITMAP_FONT_HIT)
        {
            color = mu::PackABGR(Light[0], Light[1], Light[2], 1.0f);
        }
        else
        {
            color = mu::PackABGR(Light[0], Light[1], Light[2], Light[0]);
        }
    }

    const mu::Vertex3D vertices[6] = {
        {p[0][0], p[0][1], p[0][2], 0.f, 0.f, 0.f, c[0][0], c[0][1], color},
        {p[1][0], p[1][1], p[1][2], 0.f, 0.f, 0.f, c[1][0], c[1][1], color},
        {p[2][0], p[2][1], p[2][2], 0.f, 0.f, 0.f, c[2][0], c[2][1], color},
        {p[0][0], p[0][1], p[0][2], 0.f, 0.f, 0.f, c[0][0], c[0][1], color},
        {p[2][0], p[2][1], p[2][2], 0.f, 0.f, 0.f, c[2][0], c[2][1], color},
        {p[3][0], p[3][1], p[3][2], 0.f, 0.f, 0.f, c[3][0], c[3][1], color},
    };
    mu::GetRenderer().RenderTriangles(vertices, static_cast<std::uint32_t>(Texture));
}

void RenderSpriteUV(int Texture, vec3_t Position, float Width, float Height, float(*UV)[2], vec3_t Light[4], float Alpha)
{
    BindTexture(Texture);

    vec3_t p2;
    VectorTransform(Position, g_Camera.Matrix, p2);
    float x = p2[0];
    float y = p2[1];
    float z = p2[2];

    if (z >= -1.0f)
    {
        return;
    }

    Width *= 0.5f;
    Height *= 0.5f;
    vec3_t p[4];
    Vector(x - Width, y - Height, z, p[0]);
    Vector(x + Width, y - Height, z, p[1]);
    Vector(x + Width, y + Height, z, p[2]);
    Vector(x - Width, y + Height, z, p[3]);

    const mu::Vertex3D vertices[6] = {
        {p[0][0], p[0][1], p[0][2], 0.f, 0.f, 0.f, UV[0][0], UV[0][1], mu::PackABGR(Light[0][0], Light[0][1], Light[0][2], Alpha)},
        {p[1][0], p[1][1], p[1][2], 0.f, 0.f, 0.f, UV[1][0], UV[1][1], mu::PackABGR(Light[1][0], Light[1][1], Light[1][2], Alpha)},
        {p[2][0], p[2][1], p[2][2], 0.f, 0.f, 0.f, UV[2][0], UV[2][1], mu::PackABGR(Light[2][0], Light[2][1], Light[2][2], Alpha)},
        {p[0][0], p[0][1], p[0][2], 0.f, 0.f, 0.f, UV[0][0], UV[0][1], mu::PackABGR(Light[0][0], Light[0][1], Light[0][2], Alpha)},
        {p[2][0], p[2][1], p[2][2], 0.f, 0.f, 0.f, UV[2][0], UV[2][1], mu::PackABGR(Light[2][0], Light[2][1], Light[2][2], Alpha)},
        {p[3][0], p[3][1], p[3][2], 0.f, 0.f, 0.f, UV[3][0], UV[3][1], mu::PackABGR(Light[3][0], Light[3][1], Light[3][2], Alpha)},
    };
    mu::GetRenderer().RenderTriangles(vertices, static_cast<std::uint32_t>(Texture));
}

void RenderNumber(vec3_t Position, int Num, vec3_t Color, float Alpha, float Scale)
{
    vec3_t p;
    VectorCopy(Position, p);
    vec3_t Light[4];
    VectorCopy(Color, Light[0]);
    VectorCopy(Color, Light[1]);
    VectorCopy(Color, Light[2]);
    VectorCopy(Color, Light[3]);
    if (Num == -1)
    {
        float UV[4][2];
        TEXCOORD(UV[0], 0.f, 32.f / 32.f);
        TEXCOORD(UV[1], 32.f / 256.f, 32.f / 32.f);
        TEXCOORD(UV[2], 32.f / 256.f, 17.f / 32.f);
        TEXCOORD(UV[3], 0.f, 17.f / 32.f);
        RenderSpriteUV(BITMAP_FONT + 1, p, 45, 20, UV, Light, Alpha);
    }
    else if (Num == -2)
    {
        RenderSprite(BITMAP_FONT_HIT, p, 32 * Scale, 20 * Scale, Light[0], 0.f, 0.f, 0.f, 27.f / 32.f, 15.f / 16.f);
    }
    else
    {
        wchar_t Text[32];
        _itow(Num, Text, 10);
        p[0] -= wcslen(Text) * 5.f;
        unsigned int Length = wcslen(Text);
        p[0] -= Length * Scale * 0.125f;
        p[1] -= Length * Scale * 0.125f;
        for (unsigned int i = 0; i < Length; i++)
        {
            float UV[4][2];
            float u = (float)(Text[i] - 48) * 16.f / 256.f;
            TEXCOORD(UV[0], u, 16.f / 32.f);
            TEXCOORD(UV[1], u + 16.f / 256.f, 16.f / 32.f);
            TEXCOORD(UV[2], u + 16.f / 256.f, 0.f);
            TEXCOORD(UV[3], u, 0.f);
            RenderSpriteUV(BITMAP_FONT + 1, p, Scale, Scale, UV, Light, Alpha);
            p[0] += Scale * 0.5f;
            p[1] += Scale * 0.5f;
        }
    }
}

float RenderNumber2D(float x, float y, int Num, float Width, float Height, unsigned int color)
{
    wchar_t Text[32];
    _itow(Num, Text, 10);
    int Length = (int)wcslen(Text);
    x -= Width * Length / 2;
    for (int i = 0; i < Length; i++)
    {
        float u = (float)(Text[i] - 48) * 16.f / 256.f;
        RenderColorBitmap(BITMAP_FONT + 1, x, y, Width, Height, u, 0.f,
            16.f / 256.f, 16.f / 32.f, color);
        x += Width * 0.7f;
    }
    return x;
}

void BeginBitmap()
{
    mu::GetRenderer().Begin2DPass();
    DisableDepthTest();
}

void EndBitmap()
{
    mu::GetRenderer().End2DPass();
}

namespace
{
std::uint32_t g_renderColor = 0xFFFFFFFFu;
}

void SetRenderColor(BYTE red, BYTE green, BYTE blue, BYTE alpha)
{
    g_renderColor = (static_cast<std::uint32_t>(alpha) << 24) |
                    (static_cast<std::uint32_t>(blue) << 16) |
                    (static_cast<std::uint32_t>(green) << 8) |
                    static_cast<std::uint32_t>(red);
}

void RenderColor(float x, float y, float Width, float Height, float Alpha, int Flag)
{
    DisableTexture();

    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);

    y = WindowHeight - y;

    std::uint32_t color;
    if (Alpha > 0.f)
    {
        const float clampedAlpha = (Alpha > 1.0f) ? 1.0f : Alpha;
        const auto a = static_cast<std::uint32_t>(clampedAlpha * 255.0f);
        if (Flag == 0)
        {
            color = (a << 24) | 0x00FFFFFFu;
        }
        else
        {
            color = (a << 24);
        }
    }
    else
    {
        color = g_renderColor;
    }

    const mu::Vertex2D vertices[4] = {
        {x, y, 0.0f, 0.0f, color},
        {x, y - Height, 0.0f, 0.0f, color},
        {x + Width, y - Height, 0.0f, 0.0f, color},
        {x + Width, y, 0.0f, 0.0f, color},
    };
    mu::GetRenderer().RenderQuad2D(vertices, 0u);
}
void EndRenderColor()
{
    mu::GetRenderer().SetTexture2D(true);
}

static inline std::uint32_t ArgbToAbgr(unsigned int argb)
{
    const std::uint32_t a = (argb >> 24) & 0xffu;
    const std::uint32_t r = (argb >> 16) & 0xffu;
    const std::uint32_t g = (argb >> 8) & 0xffu;
    const std::uint32_t b = argb & 0xffu;
    return (a << 24) | (b << 16) | (g << 8) | r;
}

void RenderColorQuadARGB(float x, float y, float Width, float Height, unsigned int argbColor)
{
    DisableTexture();

    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);
    y = WindowHeight - y;

    const std::uint32_t color = ArgbToAbgr(argbColor);
    const mu::Vertex2D vertices[4] = {
        {x, y, 0.0f, 0.0f, color},
        {x, y - Height, 0.0f, 0.0f, color},
        {x + Width, y - Height, 0.0f, 0.0f, color},
        {x + Width, y, 0.0f, 0.0f, color},
    };
    mu::GetRenderer().RenderQuad2D(vertices, 0u);
}

void RenderColorLineARGB(float x1, float y1, float x2, float y2, float thickness, unsigned int argbColor)
{
    DisableTexture();

    x1 = ConvertX(x1);
    y1 = ConvertY(y1);
    x2 = ConvertX(x2);
    y2 = ConvertY(y2);
    y1 = WindowHeight - y1;
    y2 = WindowHeight - y2;

    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float len = sqrtf(dx * dx + dy * dy);
    if (len < 1e-6f)
    {
        return;
    }

    const float halfT = thickness * 0.5f;
    const float ox = -dy / len * halfT;
    const float oy = dx / len * halfT;

    const std::uint32_t color = ArgbToAbgr(argbColor);
    const mu::Vertex2D vertices[4] = {
        {x1 + ox, y1 + oy, 0.0f, 0.0f, color},
        {x1 - ox, y1 - oy, 0.0f, 0.0f, color},
        {x2 - ox, y2 - oy, 0.0f, 0.0f, color},
        {x2 + ox, y2 + oy, 0.0f, 0.0f, color},
    };
    mu::GetRenderer().RenderQuad2D(vertices, 0u);
}

void RenderColorBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, unsigned int color)
{
    x = ConvertX(x);
    y = ConvertY(y);

    Width = ConvertX(Width);
    Height = ConvertY(Height);

    BindTexture(Texture);

    y = WindowHeight - y;

    const mu::Vertex2D vertices[4] = {
        {x, y, u, v, color},
        {x, y - Height, u, v + vHeight, color},
        {x + Width, y - Height, u + uWidth, v + vHeight, color},
        {x + Width, y, u + uWidth, v, color},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
}

void RenderBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, bool Scale, bool StartScale, float Alpha)
{
    if (StartScale)
    {
        x = ConvertX(x);
        y = ConvertY(y);
    }
    if (Scale)
    {
        Width = ConvertX(Width);
        Height = ConvertY(Height);
    }

    BindTexture(Texture);

    y = WindowHeight - y;

    const float clampedAlpha = (Alpha > 1.0f) ? 1.0f : Alpha;
    const std::uint32_t color =
        (clampedAlpha > 0.0f) ? (static_cast<std::uint32_t>(clampedAlpha * 255.0f) << 24) | 0x00FFFFFFu : 0xFFFFFFFFu;

    const mu::Vertex2D vertices[4] = {
        {x, y, u, v, color},
        {x, y - Height, u, v + vHeight, color},
        {x + Width, y - Height, u + uWidth, v + vHeight, color},
        {x + Width, y, u + uWidth, v, color},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
}

void RenderBitmapRotate(int Texture, float x, float y, float Width, float Height, float Rotate, float u, float v,
    float uWidth, float vHeight, unsigned int color)
{
    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);
    // x -= Width *0.5f;
    // y -= Height*0.5f;
    BindTexture(Texture);

    vec3_t p[4], p2[4];

    y = WindowHeight - y;

    Vector(-Width * 0.5f, Height * 0.5f, 0.f, p[0]);
    Vector(-Width * 0.5f, -Height * 0.5f, 0.f, p[1]);
    Vector(Width * 0.5f, -Height * 0.5f, 0.f, p[2]);
    Vector(Width * 0.5f, Height * 0.5f, 0.f, p[3]);

    vec3_t Angle;
    Vector(0.f, 0.f, Rotate, Angle);
    float Matrix[3][4];
    AngleMatrix(Angle, Matrix);

    VectorRotate(p[0], Matrix, p2[0]);
    VectorRotate(p[1], Matrix, p2[1]);
    VectorRotate(p[2], Matrix, p2[2]);
    VectorRotate(p[3], Matrix, p2[3]);

    const mu::Vertex2D vertices[4] = {
        {p2[0][0] + x, p2[0][1] + y, u, v, color},
        {p2[1][0] + x, p2[1][1] + y, u, v + vHeight, color},
        {p2[2][0] + x, p2[2][1] + y, u + uWidth, v + vHeight, color},
        {p2[3][0] + x, p2[3][1] + y, u + uWidth, v, color},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
}

void RenderBitRotate(int Texture, float x, float y, float Width, float Height, float Rotate)
{
    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);

    BindTexture(Texture);

    vec3_t p[4], p2[4];

    y = Height - y;

    float cx = (Width / 2.f) - (Width - x);
    float cy = (Height / 2.f) - (Height - y);

    float ax = (-Width * 0.5f) + cx;
    float bx = (Width * 0.5f) + cx;
    float ay = (-Height * 0.5f) + cy;
    float by = (Height * 0.5f) + cy;

    Vector(ax, by, 0.f, p[0]);
    Vector(ax, ay, 0.f, p[1]);
    Vector(bx, ay, 0.f, p[2]);
    Vector(bx, by, 0.f, p[3]);

    vec3_t Angle;
    Vector(0.f, 0.f, Rotate, Angle);
    float Matrix[3][4];
    AngleMatrix(Angle, Matrix);

    VectorRotate(p[0], Matrix, p2[0]);
    VectorRotate(p[1], Matrix, p2[1]);
    VectorRotate(p[2], Matrix, p2[2]);
    VectorRotate(p[3], Matrix, p2[3]);

    const float halfW = WindowWidth / 2.f;
    const float halfH = WindowHeight / 2.f;

    const mu::Vertex2D vertices[4] = {
        {p2[0][0] + halfW, p2[0][1] + halfH, 0.0f, 0.0f, 0xFFFFFFFFu},
        {p2[1][0] + halfW, p2[1][1] + halfH, 0.0f, 1.0f, 0xFFFFFFFFu},
        {p2[2][0] + halfW, p2[2][1] + halfH, 1.0f, 1.0f, 0xFFFFFFFFu},
        {p2[3][0] + halfW, p2[3][1] + halfH, 1.0f, 0.0f, 0xFFFFFFFFu},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
}

void RenderPointRotate(int Texture, float ix, float iy, float iWidth, float iHeight, float x, float y, float Width, float Height, float Rotate, float Rotate_Loc, float uWidth, float vHeight, int Num)
{
    vec3_t p, p2[4], p3, p4[4], Angle;
    float Matrix[3][4];

    ix = ConvertX(ix);
    iy = ConvertY(iy);
    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);

    BindTexture(Texture);

    y = Height - y;
    iy = Height - iy;

    Vector((ix - (Width * 0.5f)) + ((Width / 2.f) - (Width - x)), (iy - (Height * 0.5f)) + ((Height / 2.f) - (Height - y)), 0.f, p);

    Vector(0.f, 0.f, Rotate, Angle);
    AngleMatrix(Angle, Matrix);

    VectorRotate(p, Matrix, p3);

    Vector(-(iWidth * 0.5f), (iHeight * 0.5f), 0.f, p2[0]);
    Vector(-(iWidth * 0.5f), -(iHeight * 0.5f), 0.f, p2[1]);
    Vector((iWidth * 0.5f), -(iHeight * 0.5f), 0.f, p2[2]);
    Vector((iWidth * 0.5f), (iHeight * 0.5f), 0.f, p2[3]);

    Vector(0.f, 0.f, Rotate_Loc, Angle);
    AngleMatrix(Angle, Matrix);

    Matrix[0][3] = p3[0] + 25;
    Matrix[1][3] = p3[1];
    VectorTransform(p2[0], Matrix, p4[0]);
    VectorTransform(p2[1], Matrix, p4[1]);
    VectorTransform(p2[2], Matrix, p4[2]);
    VectorTransform(p2[3], Matrix, p4[3]);

    const float halfW = WindowWidth / 2.f;
    const float halfH = WindowHeight / 2.f;

    const mu::Vertex2D vertices[4] = {
        {p4[0][0] + halfW, p4[0][1] + halfH, 0.0f, 0.0f, 0xFFFFFFFFu},
        {p4[1][0] + halfW, p4[1][1] + halfH, 0.0f, vHeight, 0xFFFFFFFFu},
        {p4[2][0] + halfW, p4[2][1] + halfH, uWidth, vHeight, 0xFFFFFFFFu},
        {p4[3][0] + halfW, p4[3][1] + halfH, uWidth, 0.0f, 0xFFFFFFFFu},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));

    if (Num > -1)
    {
        float dx, dy;
        dx = p4[0][0] + halfW;
        dy = p4[0][1] + halfH;
        dx = dx * (float)((float)REFERENCE_WIDTH / WindowWidth);
        dy = dy * (float)((float)REFERENCE_HEIGHT / WindowHeight);
        if (Num >= 100)
        {
            g_pNewUIMiniMap->SetBtnPos(Num - 100, dx - (iWidth / 2), (REFERENCE_HEIGHT - dy) - (iHeight / 2), iWidth, iHeight);
        }
        else
        {
            g_pNewUIMiniMap->SetBtnPos(Num, dx, REFERENCE_HEIGHT - dy, iWidth / 2, iHeight / 2);
        }
    }
}

void RenderBitmapLocalRotate(int Texture, float x, float y, float Width, float Height, float Rotate, float u, float v, float uWidth, float vHeight)
{
    vec3_t p[4];
    x = ConvertX(x);
    y = ConvertY(y);
    y = WindowHeight - y;
    Width = ConvertX(Width);
    Height = ConvertY(Height);

    BindTexture(Texture);

    const float sinR = sinf(Rotate);
    const float cosR = cosf(Rotate);
    vec3_t vCenter, vDir;
    Vector(x, y, 0, vCenter);
    Vector(Width * 0.5f, -Height * 0.5f, 0, vDir);
    p[0][0] = vCenter[0] + (vDir[0]) * cosR;
    p[0][1] = vCenter[1] + (vDir[1]) * sinR;
    p[1][0] = vCenter[0] + (vDir[0]) * sinR;
    p[1][1] = vCenter[1] - (vDir[1]) * cosR;
    p[2][0] = vCenter[0] - (vDir[0]) * cosR;
    p[2][1] = vCenter[1] - (vDir[1]) * sinR;
    p[3][0] = vCenter[0] - (vDir[0]) * sinR;
    p[3][1] = vCenter[1] + (vDir[1]) * cosR;

    const mu::Vertex2D vertices[4] = {
        {p[0][0], p[0][1], u, v, 0xFFFFFFFFu},
        {p[1][0], p[1][1], u, v + vHeight, 0xFFFFFFFFu},
        {p[2][0], p[2][1], u + uWidth, v + vHeight, 0xFFFFFFFFu},
        {p[3][0], p[3][1], u + uWidth, v, 0xFFFFFFFFu},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
}

void RenderBitmapAlpha(int Texture, float sx, float sy, float Width, float Height)
{
    EnableAlphaTest();
    BindTexture(Texture);

    sy = WindowHeight - sy;
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            float p[4][2];
            p[0][0] = sx + (static_cast<float>(x) * Width) * 0.25f;
            p[0][1] = sy - (static_cast<float>(y) * Height) * 0.25f;
            p[1][0] = sx + (static_cast<float>(x) * Width) * 0.25f;
            p[1][1] = sy - (static_cast<float>(y + 1) * Height) * 0.25f;
            p[2][0] = sx + (static_cast<float>(x + 1) * Width) * 0.25f;
            p[2][1] = sy - (static_cast<float>(y + 1) * Height) * 0.25f;
            p[3][0] = sx + (static_cast<float>(x + 1) * Width) * 0.25f;
            p[3][1] = sy - (static_cast<float>(y) * Height) * 0.25f;

            const float u0 = static_cast<float>(x) * 0.25f;
            const float v0 = static_cast<float>(y) * 0.25f;
            const float u1 = static_cast<float>(x + 1) * 0.25f;
            const float v1 = static_cast<float>(y + 1) * 0.25f;

            float Alpha[4] = { 1.f,1.f,1.f,1.f };
            if (x == 0) { Alpha[0] = 0.f; Alpha[1] = 0.f; }
            if (x == 3) { Alpha[2] = 0.f; Alpha[3] = 0.f; }
            if (y == 0) { Alpha[0] = 0.f; Alpha[3] = 0.f; }
            if (y == 3) { Alpha[1] = 0.f; Alpha[2] = 0.f; }

            const mu::Vertex2D vertices[4] = {
                {p[0][0], p[0][1], u0, v0, (static_cast<std::uint32_t>(Alpha[0] * 255.0f) << 24) | 0x00FFFFFFu},
                {p[1][0], p[1][1], u0, v1, (static_cast<std::uint32_t>(Alpha[1] * 255.0f) << 24) | 0x00FFFFFFu},
                {p[2][0], p[2][1], u1, v1, (static_cast<std::uint32_t>(Alpha[2] * 255.0f) << 24) | 0x00FFFFFFu},
                {p[3][0], p[3][1], u1, v0, (static_cast<std::uint32_t>(Alpha[3] * 255.0f) << 24) | 0x00FFFFFFu},
            };
            mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
        }
    }
}

void RenderBitmapUV(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth,
    float vHeight, unsigned int color)
{
    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);
    BindTexture(Texture);

    y = WindowHeight - y;

    const mu::Vertex2D vertices[4] = {
        {x, y, u, v + vHeight * 0.25f, color},
        {x, y - Height, u, v + vHeight - vHeight * 0.25f, color},
        {x + Width, y - Height, u + uWidth, v + vHeight, color},
        {x + Width, y, u + uWidth, v, color},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
}

///////////////////////////////////////////////////////////////////////////////
// collision detect util
///////////////////////////////////////////////////////////////////////////////

float absf(float a)
{
    if (a < 0.f) return -a;
    return a;
}

float minf(float a, float b)
{
    if (a > b) return b;
    return a;
}

float maxf(float a, float b)
{
    if (a > b) return a;
    return b;
}

int InsideTest(float x, float y, float z, int n, float* v1, float* v2, float* v3, float* v4, int flag, float type)
{
    if (type > 0.f)
        flag <<= 3;

    int i;
    vec3_t* vtx[4];
    vtx[0] = (vec3_t*)v1;
    vtx[1] = (vec3_t*)v2;
    vtx[2] = (vec3_t*)v3;
    vtx[3] = (vec3_t*)v4;

    int j = n - 1;
    switch (flag)
    {
    case 1:
        for (i = 0; i < n; j = i, i++)
        {
            float d = ((*vtx[i])[1] - y) * ((*vtx[j])[2] - z) - ((*vtx[j])[1] - y) * ((*vtx[i])[2] - z);
            if (d <= 0.f)
                return false;
        }
        break;
    case 2:
        for (i = 0; i < n; j = i, i++)
        {
            float d = ((*vtx[i])[2] - z) * ((*vtx[j])[0] - x) - ((*vtx[j])[2] - z) * ((*vtx[i])[0] - x);
            if (d <= 0.f)
                return false;
        }
        break;
    case 4:
        for (i = 0; i < n; j = i, i++)
        {
            float d = ((*vtx[i])[0] - x) * ((*vtx[j])[1] - y) - ((*vtx[j])[0] - x) * ((*vtx[i])[1] - y);
            if (d <= 0.f)
                return false;
        }
        break;
    case 8:
        for (i = 0; i < n; j = i, i++)
        {
            float d = ((*vtx[i])[1] - y) * ((*vtx[j])[2] - z) - ((*vtx[j])[1] - y) * ((*vtx[i])[2] - z);
            if (d >= 0.f)
                return false;
        }
        break;
    case 16:
        for (i = 0; i < n; j = i, i++)
        {
            float d = ((*vtx[i])[2] - z) * ((*vtx[j])[0] - x) - ((*vtx[j])[2] - z) * ((*vtx[i])[0] - x);
            if (d >= 0.f)
                return false;
        }
        break;
    case 32:
        for (i = 0; i < n; j = i, i++)
        {
            float d = ((*vtx[i])[0] - x) * ((*vtx[j])[1] - y) - ((*vtx[j])[0] - x) * ((*vtx[i])[1] - y);
            if (d >= 0.f)
                return false;
        }
        break;
    }

    return true;
}

float Distance;

void InitCollisionDetectLineToFace()
{
    Distance = 9999999.f;
}

vec3_t CollisionPosition;

bool CollisionDetectLineToFace(vec3_t Position, vec3_t Target, int Polygon, float* v1, float* v2, float* v3, float* v4, vec3_t Normal, bool Collision)
{
    vec3_t Direction;
    VectorSubtract(Target, Position, Direction);
    float a = DotProduct(Direction, Normal);
    if (a >= 0.f) return false;
    float b = DotProduct(Position, Normal) - DotProduct(v1, Normal);
    float t = -b / a;
    if (t >= 0.f && t <= Distance)
    {
        float X = Direction[0] * t + Position[0];
        float Y = Direction[1] * t + Position[1];
        float Z = Direction[2] * t + Position[2];
        int Count = 0;
        float MIN = minf(minf(absf(Direction[0]), absf(Direction[1])), absf(Direction[2]));
        if (MIN == absf(Direction[0]))
        {
            if ((Y >= minf(Position[1], Target[1]) && Y <= maxf(Position[1], Target[1])) &&
                (Z >= minf(Position[2], Target[2]) && Z <= maxf(Position[2], Target[2]))) Count++;
        }
        else if (MIN == absf(Direction[1]))
        {
            if ((Z >= minf(Position[2], Target[2]) && Z <= maxf(Position[2], Target[2])) &&
                (X >= minf(Position[0], Target[0]) && X <= maxf(Position[0], Target[0]))) Count++;
        }
        else
        {
            if ((X >= minf(Position[0], Target[0]) && X <= maxf(Position[0], Target[0])) &&
                (Y >= minf(Position[1], Target[1]) && Y <= maxf(Position[1], Target[1]))) Count++;
        }
        if (Count == 0) return false;
        Count = 0;
        if (Normal[0] <= -0.5f || Normal[0] >= 0.5f)
        {
            Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 1, Normal[0]);
        }
        else if (Normal[1] <= -0.5f || Normal[1] >= 0.5f)
        {
            Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 2, Normal[1]);
        }
        else
        {
            Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 4, Normal[2]);
        }
        if (Count == 0) return false;
        if (Collision)
        {
            Distance = t;
            Vector(X, Y, Z, CollisionPosition);
        }
        return true;
    }
    return false;
}

bool ProjectLineBox(vec3_t ax, vec3_t p1, vec3_t p2, OBB_t obb)
{
    float P1 = DotProduct(ax, p1);
    float P2 = DotProduct(ax, p2);

    float mx1 = maxf(P1, P2);
    float mn1 = minf(P1, P2);

    float ST = DotProduct(ax, obb.StartPos);
    float Q1 = DotProduct(ax, obb.XAxis);
    float Q2 = DotProduct(ax, obb.YAxis);
    float Q3 = DotProduct(ax, obb.ZAxis);

    float mx2 = ST;
    float mn2 = ST;

    if (Q1 > 0)	mx2 += Q1; else mn2 += Q1;
    if (Q2 > 0)	mx2 += Q2; else mn2 += Q2;
    if (Q3 > 0) mx2 += Q3; else mn2 += Q3;

    if (mn1 > mx2) return false;
    if (mn2 > mx1) return false;

    return true;
}

bool CollisionDetectLineToOBB(vec3_t p1, vec3_t p2, OBB_t obb)
{
    vec3_t e1;
    vec3_t eq11, eq12, eq13;

    VectorSubtract(p2, p1, e1);

    CrossProduct(e1, obb.XAxis, eq11);
    CrossProduct(e1, obb.YAxis, eq12);
    CrossProduct(e1, obb.ZAxis, eq13);

    if (!ProjectLineBox(eq11, p1, p2, obb)) return false;
    if (!ProjectLineBox(eq12, p1, p2, obb)) return false;
    if (!ProjectLineBox(eq13, p1, p2, obb)) return false;

    if (!ProjectLineBox(obb.XAxis, p1, p2, obb)) return false;
    if (!ProjectLineBox(obb.YAxis, p1, p2, obb)) return false;
    if (!ProjectLineBox(obb.ZAxis, p1, p2, obb)) return false;

    return true;
}
