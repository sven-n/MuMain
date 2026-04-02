///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "MuRenderer.h"
#include "ZzzTexture.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInfomation.h"
#include "NewUISystem.h"

// GLU perspective — builds perspective matrix via GLM and multiplies into the renderer's
// projection matrix stack. GLM_FORCE_DEPTH_ZERO_TO_ONE is set (Metal/Vulkan Z [0,1]).
// Right-handed convention (GLM default) — matches original OpenGL game code.
// Here we build the matrix using the same convention and pass it via MultMatrix
// (which does ActiveMatrix = ActiveMatrix * m).
// Note: GLM_FORCE_DEPTH_ZERO_TO_ONE is defined via target_compile_definitions in CMakeLists.txt
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static void gluPerspective(double fovy, double aspect, double zNear, double zFar)
{
    const glm::mat4 persp = glm::perspective(glm::radians(static_cast<float>(fovy)), static_cast<float>(aspect),
                                             static_cast<float>(zNear), static_cast<float>(zFar));
    mu::GetRenderer().MultMatrix(glm::value_ptr(persp));
}

int OpenglWindowX;
int OpenglWindowY;
int OpenglWindowWidth;
int OpenglWindowHeight;
bool CameraTopViewEnable = false;
float CameraViewNear = 20.f;
float CameraViewFar = 2000.f;
float CameraFOV = 55.f;
vec3_t CameraPosition;
vec3_t CameraAngle;
float CameraMatrix[3][4];
vec3_t MousePosition;
vec3_t MouseTarget;
float g_fCameraCustomDistance = 0.f;
bool FogEnable = false;
GLfloat FogDensity = 0.0004f;
GLfloat FogColor[4] = {
    30 / 256.f,
    20 / 256.f,
    10 / 256.f,
};

bool _isVSyncAvailable = false;
bool _isVSyncEnabled = false;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;

unsigned int WindowWidth = 1024;
unsigned int WindowHeight = 768;
int MouseX = WindowWidth / 2;
int MouseY = WindowHeight / 2;
int BackMouseX = MouseX;
int BackMouseY = MouseY;
bool MouseLButton;
bool MouseLButtonPop;
bool MouseLButtonPush;
bool MouseRButton;
bool MouseRButtonPop;
bool MouseRButtonPush;
bool MouseLButtonDBClick;
bool MouseMButton;
bool MouseMButtonPop;
bool MouseMButtonPush;
int MouseWheel;
DWORD MouseRButtonPress = 0;

// bool    showShoppingMall = false;

void OpenExploper(wchar_t* Name, wchar_t* para)
{
    ShellExecute(NULL, L"open", Name, para, L"", SW_SHOW);
}

bool CheckID_HistoryDay(wchar_t* Name, WORD day)
{
    typedef struct __day_history__
    {
        wchar_t ID[MAX_USERNAME_SIZE + 1];
        WORD date;
    } dayHistory;

    FILE* fp;
    dayHistory days[100];
    WORD num = 0;
    bool sameName = false;
    bool update = true;

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
            }
        }
        fclose(fp);
    }

    if (update)
    {
        if (!sameName)
        {
            memcpy(days[num].ID, Name, (MAX_USERNAME_SIZE + 1) * sizeof(wchar_t));
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

    return update;
}

bool GrabEnable = false;
wchar_t GrabFileName[MAX_PATH];
int GrabScreen = 0;

float PerspectiveX;
float PerspectiveY;
int ScreenCenterX;
int ScreenCenterY;
int ScreenCenterYFlip;

void GetOpenGLMatrix(float Matrix[3][4])
{
    float OpenGLMatrix[16];
    mu::GetRenderer().GetMatrix(GL_MODELVIEW_MATRIX, OpenGLMatrix);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            Matrix[i][j] = OpenGLMatrix[j * 4 + i];
        }
    }
}

void gluPerspective2(float Fov, float Aspect, float ZNear, float ZFar)
{
    gluPerspective(Fov, Aspect, ZNear, ZFar);

    ScreenCenterX = OpenglWindowX + OpenglWindowWidth / 2;
    ScreenCenterY = OpenglWindowY + OpenglWindowHeight / 2;
    ScreenCenterYFlip = WindowWidth - ScreenCenterY;

    float AspectY = (float)(WindowHeight) / (float)(OpenglWindowHeight);
    PerspectiveX = tanf(Fov * 0.5f * Q_PI / 180.f) / (float)(OpenglWindowWidth / 2) * Aspect;
    PerspectiveY = tanf(Fov * 0.5f * Q_PI / 180.f) / (float)(OpenglWindowHeight / 2) * AspectY;
}

void CreateScreenVector(int sx, int sy, vec3_t Target, bool bFixView)
{
    sx = sx * WindowWidth / 640;
    sy = sy * WindowHeight / 480;
    vec3_t p1, p2;
    if (bFixView)
    {
        p1[0] = (float)(sx - ScreenCenterX) * CameraViewFar * PerspectiveX;
        p1[1] = -(float)(sy - ScreenCenterY) * CameraViewFar * PerspectiveY;
        p1[2] = -CameraViewFar;
    }
    else
    {
        p1[0] = (float)(sx - ScreenCenterX) * RENDER_ITEMVIEW_FAR * PerspectiveX;
        p1[1] = -(float)(sy - ScreenCenterY) * RENDER_ITEMVIEW_FAR * PerspectiveY;
        p1[2] = -RENDER_ITEMVIEW_FAR;
    }

    p2[0] = -CameraMatrix[0][3];
    p2[1] = -CameraMatrix[1][3];
    p2[2] = -CameraMatrix[2][3];
    VectorIRotate(p2, CameraMatrix, MousePosition);
    VectorIRotate(p1, CameraMatrix, p2);
    VectorAdd(MousePosition, p2, Target);
}

void Projection(vec3_t Position, int* sx, int* sy)
{
    vec3_t TrasformPosition;
    VectorTransform(Position, CameraMatrix, TrasformPosition);
    *sx = -(int)(TrasformPosition[0] / PerspectiveX / TrasformPosition[2]) + ScreenCenterX;
    *sy = (int)(TrasformPosition[1] / PerspectiveY / TrasformPosition[2]) + ScreenCenterY;
    *sx = *sx * 640 / (int)WindowWidth;
    *sy = *sy * 480 / (int)WindowHeight;
}

void TransformPosition(vec3_t Position, vec3_t WorldPosition, int* x, int* y)
{
    vec3_t Temp;
    VectorSubtract(Position, CameraPosition, Temp);
    VectorRotate(Temp, CameraMatrix, WorldPosition);

    *x = (int)(WorldPosition[0] / PerspectiveX / -WorldPosition[2]) + (ScreenCenterX);
    *y = (int)(WorldPosition[1] / PerspectiveY / -WorldPosition[2]) + (ScreenCenterYFlip);
    //*y = (int)(WorldPosition[1]/PerspectiveY/-WorldPosition[2]) + (WindowHeight/2);
}

bool TestDepthBuffer(vec3_t Position)
{
    vec3_t WorldPosition;
    int x, y;
    TransformPosition(Position, WorldPosition, &x, &y);
    if (x < OpenglWindowX || y < OpenglWindowY || x >= (int)OpenglWindowX + OpenglWindowWidth ||
        y >= (int)OpenglWindowY + OpenglWindowHeight)
        return false;

    GLfloat key[3];
    mu::GetRenderer().ReadPixels(x, y, 1, 1, key);

    float z = 1.f - CameraViewNear / -WorldPosition[2] + CameraViewNear / CameraViewFar;
    if (key[0] >= z)
        return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// opengl render util
///////////////////////////////////////////////////////////////////////////////

int CachTexture = -1;
bool TextureEnable;
bool DepthTestEnable;
bool CullFaceEnable;
bool DepthMaskEnable;
bool AlphaTestEnable;
int AlphaBlendType;

void BindTexture(int tex)
{
    if (CachTexture != tex)
    {
        CachTexture = tex;
        mu::GetRenderer().BindTexture(tex);
    }
}

bool TextureStream = false;

extern int test;
void BindTextureStream(int tex)
{
    if (CachTexture != tex)
    {
        CachTexture = tex;
        BITMAP_t* b = &Bitmaps[tex];
        mu::GetRenderer().BindTexture(b->TextureNumber);
        TextureStream = true;
    }
}

void EndTextureStream()
{
    TextureStream = false;
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

void glViewport2(int x, int y, int Width, int Height)
{
    OpenglWindowX = x;
    OpenglWindowY = y;
    OpenglWindowWidth = Width;
    OpenglWindowHeight = Height;
    mu::GetRenderer().SetViewport(x, WindowHeight - (y + Height), Width, Height);
}

float ConvertX(float x)
{
    return x * (float)WindowWidth / 640.f;
}

float ConvertY(float y)
{
    return y * (float)WindowHeight / 480.f;
}

void BeginOpengl(int x, int y, int Width, int Height)
{
    // BeginScene scales from 640×480 design space to physical pixels internally.
    // Pass raw design-space values — do NOT pre-scale here (that would double-scale).
    mu::GetRenderer().BeginScene(x, y, Width, Height);

    // Scale to window pixels for aspect ratio and perspective calculations only.
    const int scaledW = Width * WindowWidth / 640;
    const int scaledH = Height * WindowHeight / 480;

    // Projection matrix — perspective
    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    gluPerspective2(CameraFOV, (float)scaledW / (float)scaledH, CameraViewNear, CameraViewFar * 1.4f);

    // Modelview matrix — camera transform
    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    mu::GetRenderer().Rotate(CameraAngle[1], 0.f, 1.f, 0.f);
    if (!CameraTopViewEnable)
        mu::GetRenderer().Rotate(CameraAngle[0], 1.f, 0.f, 0.f);
    mu::GetRenderer().Rotate(CameraAngle[2], 0.f, 0.f, 1.f);
    mu::GetRenderer().Translate(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]);

    // State tracking flags
    AlphaTestEnable = false;
    TextureEnable = true;
    DepthTestEnable = true;
    CullFaceEnable = true;
    DepthMaskEnable = true;

    mu::GetRenderer().SetDepthTest(true);

    GetOpenGLMatrix(CameraMatrix);
}

void EndOpengl()
{
    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PopMatrix();
    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PopMatrix();
    mu::GetRenderer().EndScene();
}

void UpdateMousePositionn()
{
    vec3_t vPos;

    mu::GetRenderer().LoadIdentity();
    mu::GetRenderer().Translate(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]);
    GetOpenGLMatrix(CameraMatrix);

    Vector(-CameraMatrix[0][3], -CameraMatrix[1][3], -CameraMatrix[2][3], vPos);
    VectorIRotate(vPos, CameraMatrix, MousePosition);
}

BOOL IsGLExtensionSupported(const wchar_t* extension)
{
    return FALSE;
}

bool WGLExtensionSupported(const char* extension_name)
{
    return false;
}

void InitVSync() {}

bool IsVSyncAvailable()
{
    return _isVSyncAvailable;
}

bool IsVSyncEnabled()
{
    return _isVSyncEnabled;
}

void EnableVSync() {}

void DisableVSync() {}

int GetFPSLimit()
{
    return 60;
}

#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
BOOL InitGLMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pfd, int iRequestMSAAValue,
                       int& OutiPixelFormat)
{
    return FALSE;
}

void SetEnableMultisample()
{
    if (TRUE == g_bSupportedMSAA)
    {
        mu::GetRenderer().SetMultisample(true);
    }
}

void SetDisableMultisample()
{
    if (TRUE == g_bSupportedMSAA)
    {
        mu::GetRenderer().SetMultisample(false);
    }
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

    // Story 7-9-2 (AC-5): Port GL_QUADS box to MuRenderer triangle list.
    // 6 faces * 2 triangles * 3 vertices = 36 vertices.
    auto MakeVtx = [&](const vec3_t& pos, float tu, float tv, std::uint32_t c) -> mu::Vertex3D
    { return {pos[0], pos[1], pos[2], 0.f, 0.f, 1.f, tu, tv, c}; };

    // Pack ABGR colors (opaque)
    constexpr std::uint32_t cDark = 0xFF333333u;  // 0.2, 0.2, 0.2
    constexpr std::uint32_t cMid = 0xFF999999u;   // 0.6, 0.6, 0.6
    constexpr std::uint32_t cLight = 0xFF666666u; // 0.4, 0.4, 0.4

    // Helper: emit 2 triangles from a quad (v0,v1,v2,v3) -> (v0,v1,v2) + (v0,v2,v3)
    std::vector<mu::Vertex3D> verts;
    verts.reserve(36);

    auto EmitQuad = [&](const vec3_t& v0, float u0, float v0t, const vec3_t& v1, float u1, float v1t, const vec3_t& v2,
                        float u2, float v2t, const vec3_t& v3, float u3, float v3t, std::uint32_t col)
    {
        verts.push_back(MakeVtx(v0, u0, v0t, col));
        verts.push_back(MakeVtx(v1, u1, v1t, col));
        verts.push_back(MakeVtx(v2, u2, v2t, col));
        verts.push_back(MakeVtx(v0, u0, v0t, col));
        verts.push_back(MakeVtx(v2, u2, v2t, col));
        verts.push_back(MakeVtx(v3, u3, v3t, col));
    };

    // Face 1 (dark): 7,6,4,5
    EmitQuad(TransformVertices[7], 1.f, 1.f, TransformVertices[6], 1.f, 0.f, TransformVertices[4], 0.f, 0.f,
             TransformVertices[5], 0.f, 1.f, cDark);
    // Face 2 (dark): 0,2,3,1
    EmitQuad(TransformVertices[0], 0.f, 1.f, TransformVertices[2], 1.f, 1.f, TransformVertices[3], 1.f, 0.f,
             TransformVertices[1], 0.f, 0.f, cDark);
    // Face 3 (mid): 7,3,2,6
    EmitQuad(TransformVertices[7], 1.f, 1.f, TransformVertices[3], 1.f, 0.f, TransformVertices[2], 0.f, 0.f,
             TransformVertices[6], 0.f, 1.f, cMid);
    // Face 4 (mid): 0,1,5,4
    EmitQuad(TransformVertices[0], 0.f, 1.f, TransformVertices[1], 1.f, 1.f, TransformVertices[5], 1.f, 0.f,
             TransformVertices[4], 0.f, 0.f, cMid);
    // Face 5 (light): 7,5,1,3
    EmitQuad(TransformVertices[7], 1.f, 1.f, TransformVertices[5], 1.f, 0.f, TransformVertices[1], 0.f, 0.f,
             TransformVertices[3], 0.f, 1.f, cLight);
    // Face 6 (light): 0,4,6,2
    EmitQuad(TransformVertices[0], 0.f, 1.f, TransformVertices[4], 1.f, 1.f, TransformVertices[6], 1.f, 0.f,
             TransformVertices[2], 0.f, 0.f, cLight);

    mu::GetRenderer().RenderTriangles(verts, 0);
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

    // Story 7-9-2 (AC-5): Port GL_QUADS plane to MuRenderer triangle list.
    constexpr std::uint32_t white = 0xFFFFFFFFu;
    auto MakeVtx = [&](const vec3_t& pos, float tu, float tv) -> mu::Vertex3D
    { return {pos[0], pos[1], pos[2], 0.f, 0.f, 1.f, tu, tv, white}; };
    std::vector<mu::Vertex3D> verts = {
        MakeVtx(TransformVertices[0], 0.f, 1.f), MakeVtx(TransformVertices[1], 1.f, 1.f),
        MakeVtx(TransformVertices[2], 1.f, 0.f), MakeVtx(TransformVertices[0], 0.f, 1.f),
        MakeVtx(TransformVertices[2], 1.f, 0.f), MakeVtx(TransformVertices[3], 0.f, 0.f),
    };
    mu::GetRenderer().RenderTriangles(verts, 0);
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

void RenderSprite(int Texture, vec3_t Position, float Width, float Height, vec3_t Light, float Rotation, float u,
                  float v, float uWidth, float vHeight)
{
    BindTexture(Texture);

    vec3_t p2;
    VectorTransform(Position, CameraMatrix, p2);
    // VectorCopy(Position,p2);
    float x = p2[0];
    float y = p2[1];
    float z = p2[2];

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

    // Story 7-9-2 (AC-5): Port GL_QUADS sprite to MuRenderer triangle list.
    std::uint32_t spriteColor;
    {
        auto ToByte = [](float f) -> std::uint8_t
        { return static_cast<std::uint8_t>(std::min(std::max(f, 0.f), 1.f) * 255.f + 0.5f); };
        std::uint8_t r = ToByte(Light[0]);
        std::uint8_t g = ToByte(Light[1]);
        std::uint8_t b = ToByte(Light[2]);
        std::uint8_t a = 255;
        if (Bitmaps[Texture].Components != 3)
        {
            if (Texture == BITMAP_BLOOD + 1 || Texture == BITMAP_FONT_HIT)
                a = 255;
            else
                a = r; // alpha = Light[0]
        }
        spriteColor = (static_cast<std::uint32_t>(a) << 24) | (static_cast<std::uint32_t>(b) << 16) |
                      (static_cast<std::uint32_t>(g) << 8) | r;
    }
    auto MakeVtx = [&](const vec3_t& pos, float tu, float tv) -> mu::Vertex3D
    { return {pos[0], pos[1], pos[2], 0.f, 0.f, 1.f, tu, tv, spriteColor}; };
    // Quad (p[0],p[1],p[2],p[3]) -> 2 triangles
    std::vector<mu::Vertex3D> verts = {
        MakeVtx(p[0], c[0][0], c[0][1]), MakeVtx(p[1], c[1][0], c[1][1]), MakeVtx(p[2], c[2][0], c[2][1]),
        MakeVtx(p[0], c[0][0], c[0][1]), MakeVtx(p[2], c[2][0], c[2][1]), MakeVtx(p[3], c[3][0], c[3][1]),
    };
    mu::GetRenderer().RenderTriangles(verts, 0);
}

void RenderSpriteUV(int Texture, vec3_t Position, float Width, float Height, float (*UV)[2], vec3_t Light[4],
                    float Alpha)
{
    BindTexture(Texture);

    vec3_t p2;
    VectorTransform(Position, CameraMatrix, p2);
    float x = p2[0];
    float y = p2[1];
    float z = p2[2];

    Width *= 0.5f;
    Height *= 0.5f;
    vec3_t p[4];
    Vector(x - Width, y - Height, z, p[0]);
    Vector(x + Width, y - Height, z, p[1]);
    Vector(x + Width, y + Height, z, p[2]);
    Vector(x - Width, y + Height, z, p[3]);

    // Story 7-9-2 (AC-5): Port GL_QUADS spriteUV to MuRenderer triangle list.
    auto ToByte = [](float f) -> std::uint8_t
    { return static_cast<std::uint8_t>(std::min(std::max(f, 0.f), 1.f) * 255.f + 0.5f); };
    auto MakeVtx = [&](int idx) -> mu::Vertex3D
    {
        std::uint8_t r = ToByte(Light[idx][0]);
        std::uint8_t g = ToByte(Light[idx][1]);
        std::uint8_t b = ToByte(Light[idx][2]);
        std::uint8_t a = ToByte(Alpha);
        std::uint32_t col = (static_cast<std::uint32_t>(a) << 24) | (static_cast<std::uint32_t>(b) << 16) |
                            (static_cast<std::uint32_t>(g) << 8) | r;
        return {p[idx][0], p[idx][1], p[idx][2], 0.f, 0.f, 1.f, UV[idx][0], UV[idx][1], col};
    };
    // Quad (0,1,2,3) -> 2 triangles
    std::vector<mu::Vertex3D> verts = {
        MakeVtx(0), MakeVtx(1), MakeVtx(2), MakeVtx(0), MakeVtx(2), MakeVtx(3),
    };
    mu::GetRenderer().RenderTriangles(verts, 0);
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

float RenderNumber2D(float x, float y, int Num, float Width, float Height)
{
    wchar_t Text[32];
    _itow(Num, Text, 10);
    int Length = (int)wcslen(Text);
    x -= Width * Length / 2;
    for (int i = 0; i < Length; i++)
    {
        float u = (float)(Text[i] - 48) * 16.f / 256.f;
        // glColor3fv(Color);
        RenderBitmap(BITMAP_FONT + 1, x, y, Width, Height, u, 0.f, 16.f / 256.f, 16.f / 32.f);
        x += Width * 0.7f;
    }
    return x;
}

void BeginBitmap()
{
    // Story 7-9-2 (AC-2): Delegate orthographic setup to the renderer.
    mu::GetRenderer().Begin2DPass();
}

void EndBitmap()
{
    // Story 7-9-2 (AC-2): Delegate matrix restore to the renderer.
    mu::GetRenderer().End2DPass();
}

void RenderColor(float x, float y, float Width, float Height, float Alpha, int Flag)
{
    // Option A (AC-3): textureId=0 sentinel means untextured quad.
    // DisableTexture() is called to disable texturing in the GL pipeline;
    // RenderQuad2D is called with textureId=0 to document the untextured intent.
    DisableTexture();

    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);

    y = WindowHeight - y;

    // Build per-vertex color: packed ABGR
    // Flag==0: white with alpha; Flag==1: black with alpha; no alpha -> opaque white
    //
    // Behavioral note (4-2-2): The original GL implementation emitted no glColor* call
    // when Alpha == 0.f, allowing the current OpenGL vertex color state (from the previous
    // render call) to carry over implicitly. The migration instead always emits opaque white
    // (0xFFFFFFFF) in this case, which is the more correct deterministic behavior.
    // EndRenderColor() already resets to glColor4f(1,1,1,1), so the implicit-state
    // assumption (current color = white) was sound in practice.
    std::uint32_t color = 0xFFFFFFFFu; // default: opaque white (deterministic, see note above)
    if (Alpha > 0.f)
    {
        const float clampedAlpha = (Alpha > 1.0f) ? 1.0f : Alpha;
        const auto a = static_cast<std::uint32_t>(clampedAlpha * 255.0f);
        if (Flag == 0)
        {
            // White with alpha: ABGR = (a<<24) | 0x00FFFFFF
            color = (a << 24) | 0x00FFFFFFu;
        }
        else if (Flag == 1)
        {
            // Black with alpha: ABGR = (a<<24) | 0x00000000
            color = (a << 24) | 0x00000000u;
        }
    }

    const mu::Vertex2D vertices[4] = {
        {x, y, 0.0f, 0.0f, color},
        {x, y - Height, 0.0f, 0.0f, color},
        {x + Width, y - Height, 0.0f, 0.0f, color},
        {x + Width, y, 0.0f, 0.0f, color},
    };
    // textureId=0: untextured sentinel (Option A, AC-3)
    mu::GetRenderer().RenderQuad2D(vertices, 0u);
}
void EndRenderColor()
{
    mu::GetRenderer().SetTexture2D(true);
}

void RenderColorBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth,
                       float vHeight, unsigned int color)
{
    x = ConvertX(x);
    y = ConvertY(y);

    Width = ConvertX(Width);
    Height = ConvertY(Height);

    BindTexture(Texture);

    y = WindowHeight - y;

    // color parameter is already packed ABGR (caller provides it)
    const mu::Vertex2D vertices[4] = {
        {x, y, u, v, color},
        {x, y - Height, u, v + vHeight, color},
        {x + Width, y - Height, u + uWidth, v + vHeight, color},
        {x + Width, y, u + uWidth, v, color},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
}

void RenderBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth,
                  float vHeight, bool Scale, bool StartScale, float Alpha)
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

    // Pack ABGR: opaque white unless Alpha > 0 (per-vertex alpha mode)
    // Clamp Alpha to [0,1] to prevent 8-bit A channel overflow (e.g. Alpha=1.01f -> a=257 overflows)
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
                        float uWidth, float vHeight)
{
    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);
    // x -= Width *0.5f;
    // y -= Height*0.5f;
    BindTexture(Texture);

    vec3_t p[4], p2[4];

    // Coordinate conversion contract (640×480 → screen space):
    // ConvertX/ConvertY above scale from 640×480 logical to screen pixels.
    // Y-axis inversion here flips from OpenGL bottom-up to screen top-down
    // (required because the original coordinate system has Y=0 at bottom).
    // This same conversion is applied in CSprite::Render() (Story 7-9-2, AC-3).
    y = WindowHeight - y;

    Vector(-Width * 0.5f, Height * 0.5f, 0.f, p[0]);
    Vector(-Width * 0.5f, -Height * 0.5f, 0.f, p[1]);
    Vector(Width * 0.5f, -Height * 0.5f, 0.f, p[2]);
    Vector(Width * 0.5f, Height * 0.5f, 0.f, p[3]);

    vec3_t Angle;
    Vector(0.f, 0.f, Rotate, Angle);
    float Matrix[3][4];
    AngleMatrix(Angle, Matrix);

    // Rotation math preserved: compute rotated positions then build Vertex2D
    VectorRotate(p[0], Matrix, p2[0]);
    VectorRotate(p[1], Matrix, p2[1]);
    VectorRotate(p[2], Matrix, p2[2]);
    VectorRotate(p[3], Matrix, p2[3]);

    const mu::Vertex2D vertices[4] = {
        {p2[0][0] + x, p2[0][1] + y, u, v, 0xFFFFFFFFu},
        {p2[1][0] + x, p2[1][1] + y, u, v + vHeight, 0xFFFFFFFFu},
        {p2[2][0] + x, p2[2][1] + y, u + uWidth, v + vHeight, 0xFFFFFFFFu},
        {p2[3][0] + x, p2[3][1] + y, u + uWidth, v, 0xFFFFFFFFu},
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

    // Rotation math preserved: compute rotated positions then build Vertex2D
    // UV layout: unit (0-1) range across all four corners
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

void RenderPointRotate(int Texture, float ix, float iy, float iWidth, float iHeight, float x, float y, float Width,
                       float Height, float Rotate, float Rotate_Loc, float uWidth, float vHeight, int Num)
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

    Vector((ix - (Width * 0.5f)) + ((Width / 2.f) - (Width - x)),
           (iy - (Height * 0.5f)) + ((Height / 2.f) - (Height - y)), 0.f, p);

    Vector(0.f, 0.f, Rotate, Angle);
    AngleMatrix(Angle, Matrix);

    VectorRotate(p, Matrix, p3);

    Vector(-(iWidth * 0.5f), (iHeight * 0.5f), 0.f, p2[0]);
    Vector(-(iWidth * 0.5f), -(iHeight * 0.5f), 0.f, p2[1]);
    Vector((iWidth * 0.5f), -(iHeight * 0.5f), 0.f, p2[2]);
    Vector((iWidth * 0.5f), (iHeight * 0.5f), 0.f, p2[3]);

    Vector(0.f, 0.f, Rotate_Loc, Angle);
    AngleMatrix(Angle, Matrix);

    // Compute the 4 transformed positions (VectorTransform uses Matrix[x][3] as translation)
    Matrix[0][3] = p3[0] + 25;
    Matrix[1][3] = p3[1];
    VectorTransform(p2[0], Matrix, p4[0]);
    VectorTransform(p2[1], Matrix, p4[1]);
    VectorTransform(p2[2], Matrix, p4[2]);
    VectorTransform(p2[3], Matrix, p4[3]);

    const float halfW = WindowWidth / 2.f;
    const float halfH = WindowHeight / 2.f;

    // UV layout: sub-rect [0, uWidth] x [0, vHeight]
    const mu::Vertex2D vertices[4] = {
        {p4[0][0] + halfW, p4[0][1] + halfH, 0.0f, 0.0f, 0xFFFFFFFFu},
        {p4[1][0] + halfW, p4[1][1] + halfH, 0.0f, vHeight, 0xFFFFFFFFu},
        {p4[2][0] + halfW, p4[2][1] + halfH, uWidth, vHeight, 0xFFFFFFFFu},
        {p4[3][0] + halfW, p4[3][1] + halfH, uWidth, 0.0f, 0xFFFFFFFFu},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));

    // Retain minimap button side-effect: update button position based on p4[0]
    if (Num > -1)
    {
        float dx, dy;
        dx = p4[0][0] + halfW;
        dy = p4[0][1] + halfH;
        dx = dx * (640.f / WindowWidth);
        dy = dy * (480.f / WindowHeight);
        if (Num >= 100)
        {
            g_pNewUIMiniMap->SetBtnPos(Num - 100, dx - (iWidth / 2), (480 - dy) - (iHeight / 2), iWidth, iHeight);
        }
        else
        {
            g_pNewUIMiniMap->SetBtnPos(Num, dx, 480 - dy, iWidth / 2, iHeight / 2);
        }
    }
}

void RenderBitmapLocalRotate(int Texture, float x, float y, float Width, float Height, float Rotate, float u, float v,
                             float uWidth, float vHeight)
{
    vec3_t p[4];
    x = ConvertX(x);
    y = ConvertY(y);
    y = WindowHeight - y;
    Width = ConvertX(Width);
    Height = ConvertY(Height);

    BindTexture(Texture);

    // Local pivot rotation math preserved as-is
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

            // Per-vertex alpha gradient (edge fade): 1.0 = opaque, 0.0 = transparent
            float Alpha[4] = {1.f, 1.f, 1.f, 1.f};
            if (x == 0)
            {
                Alpha[0] = 0.f;
                Alpha[1] = 0.f;
            }
            if (x == 3)
            {
                Alpha[2] = 0.f;
                Alpha[3] = 0.f;
            }
            if (y == 0)
            {
                Alpha[0] = 0.f;
                Alpha[3] = 0.f;
            }
            if (y == 3)
            {
                Alpha[1] = 0.f;
                Alpha[2] = 0.f;
            }

            // Pack per-vertex alpha into ABGR: A=(alpha*255)<<24 | 0x00FFFFFF
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
                    float vHeight)
{
    x = ConvertX(x);
    y = ConvertY(y);
    Width = ConvertX(Width);
    Height = ConvertY(Height);
    BindTexture(Texture);

    y = WindowHeight - y;

    // Asymmetric UV warp preserved exactly as original TEXCOORD assignments:
    //   c[0] = (u,                     v + vHeight*0.25f)   -> vertex 0 (TL)
    //   c[1] = (u,                     v+vHeight-vHeight*0.25f) -> vertex 1 (BL)
    //   c[2] = (u+uWidth,              v+vHeight)           -> vertex 2 (BR)
    //   c[3] = (u+uWidth,              v)                   -> vertex 3 (TR)
    const mu::Vertex2D vertices[4] = {
        {x, y, u, v + vHeight * 0.25f, 0xFFFFFFFFu},
        {x, y - Height, u, v + vHeight - vHeight * 0.25f, 0xFFFFFFFFu},
        {x + Width, y - Height, u + uWidth, v + vHeight, 0xFFFFFFFFu},
        {x + Width, y, u + uWidth, v, 0xFFFFFFFFu},
    };
    mu::GetRenderer().RenderQuad2D(vertices, static_cast<std::uint32_t>(Texture));
}

///////////////////////////////////////////////////////////////////////////////
// collision detect util
///////////////////////////////////////////////////////////////////////////////

float absf(float a)
{
    if (a < 0.f)
        return -a;
    return a;
}

float minf(float a, float b)
{
    if (a > b)
        return b;
    return a;
}

float maxf(float a, float b)
{
    if (a > b)
        return a;
    return b;
}

int InsideTest(float x, float y, float z, int n, float* v1, float* v2, float* v3, float* v4, int flag, float type)
{
    if (type > 0.f)
        flag <<= 3;

    int i;
    vec3_t* vtx[4];
    // cppcheck-suppress dangerousTypeCast
    vtx[0] = (vec3_t*)v1;
    // cppcheck-suppress dangerousTypeCast
    vtx[1] = (vec3_t*)v2;
    // cppcheck-suppress dangerousTypeCast
    vtx[2] = (vec3_t*)v3;
    // cppcheck-suppress dangerousTypeCast
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

bool CollisionDetectLineToFace(vec3_t Position, vec3_t Target, int Polygon, float* v1, float* v2, float* v3, float* v4,
                               vec3_t Normal, bool Collision)
{
    vec3_t Direction;
    VectorSubtract(Target, Position, Direction);
    float a = DotProduct(Direction, Normal);
    if (a >= 0.f)
        return false;
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
                (Z >= minf(Position[2], Target[2]) && Z <= maxf(Position[2], Target[2])))
                Count++;
        }
        else if (MIN == absf(Direction[1]))
        {
            if ((Z >= minf(Position[2], Target[2]) && Z <= maxf(Position[2], Target[2])) &&
                (X >= minf(Position[0], Target[0]) && X <= maxf(Position[0], Target[0])))
                Count++;
        }
        else
        {
            if ((X >= minf(Position[0], Target[0]) && X <= maxf(Position[0], Target[0])) &&
                (Y >= minf(Position[1], Target[1]) && Y <= maxf(Position[1], Target[1])))
                Count++;
        }
        if (Count == 0)
            return false;
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
        if (Count == 0)
            return false;
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

    if (Q1 > 0)
        mx2 += Q1;
    else
        mn2 += Q1;
    if (Q2 > 0)
        mx2 += Q2;
    else
        mn2 += Q2;
    if (Q3 > 0)
        mx2 += Q3;
    else
        mn2 += Q3;

    if (mn1 > mx2)
        return false;
    if (mn2 > mx1)
        return false;

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

    if (!ProjectLineBox(eq11, p1, p2, obb))
        return false;
    if (!ProjectLineBox(eq12, p1, p2, obb))
        return false;
    if (!ProjectLineBox(eq13, p1, p2, obb))
        return false;

    if (!ProjectLineBox(obb.XAxis, p1, p2, obb))
        return false;
    if (!ProjectLineBox(obb.YAxis, p1, p2, obb))
        return false;
    if (!ProjectLineBox(obb.ZAxis, p1, p2, obb))
        return false;

    return true;
}