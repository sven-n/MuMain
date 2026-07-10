#include "Render/Renderer/GLCompatShim.h"

#include "Render/Renderer/MuRenderer.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace
{
constexpr MUCompatGLenum kGLDepthTest = 0x0B71;
constexpr MUCompatGLenum kGLAlphaTest = 0x0BC0;
constexpr MUCompatGLenum kGLTexture2D = 0x0DE1;
constexpr MUCompatGLenum kGLBlend = 0x0BE2;
constexpr MUCompatGLenum kGLCullFace = 0x0B44;
constexpr MUCompatGLenum kGLFog = 0x0B60;
constexpr MUCompatGLenum kGLStencilTest = 0x0B90;
constexpr MUCompatGLenum kGLQuads = 0x0007;
constexpr MUCompatGLenum kGLTriangles = 0x0004;
constexpr MUCompatGLenum kGLLines = 0x0001;
constexpr MUCompatGLenum kGLTriangleFan = 0x0006;
constexpr MUCompatGLenum kGLQuadStrip = 0x0008;
constexpr MUCompatGLenum kGLModelViewMatrix = 0x0BA6;
constexpr MUCompatGLenum kGLProjectionMatrix = 0x0BA7;
constexpr MUCompatGLenum kGLSrcAlpha = 0x0302;
constexpr MUCompatGLenum kGLOneMinusSrcAlpha = 0x0303;
constexpr MUCompatGLenum kGLOne = 1;
constexpr MUCompatGLenum kGLZero = 0;
constexpr MUCompatGLenum kGLOneMinusSrcColor = 0x0301;
constexpr MUCompatGLenum kGLOneMinusDstColor = 0x0306;
constexpr MUCompatGLenum kGLSrcColor = 0x0300;
constexpr MUCompatGLenum kGLUnsignedByte = 0x1401;
constexpr MUCompatGLenum kGLRgb = 0x1907;
constexpr MUCompatGLenum kGLRgba = 0x1908;

struct ImmediateVertex
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float u = 0.0f;
    float v = 0.0f;
    std::uint32_t color = 0xFFFFFFFFu;
};

MUCompatGLenum s_mode = 0;
std::vector<ImmediateVertex> s_vertices;
float s_u = 0.0f;
float s_v = 0.0f;
std::uint32_t s_color = 0xFFFFFFFFu;
std::uint32_t s_boundTexture = 0;
constexpr MUCompatGLuint kFirstDynamicTextureId = 0x40000000u;
constexpr MUCompatGLuint kLastDynamicTextureId = 0x7FFFFFFFu;
MUCompatGLuint s_nextTexture = kFirstDynamicTextureId;
bool s_depthTest = false;
bool s_texture2D = true;
bool s_blend = false;
bool s_alphaTest = false;
bool s_cullFace = false;
bool s_fog = false;
bool s_stencil = false;

[[nodiscard]] std::uint8_t ByteFromFloat(float value)
{
    return static_cast<std::uint8_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
}

[[nodiscard]] std::uint32_t PackColor(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
{
    return (static_cast<std::uint32_t>(a) << 24) | (static_cast<std::uint32_t>(b) << 16) |
           (static_cast<std::uint32_t>(g) << 8) | static_cast<std::uint32_t>(r);
}

[[nodiscard]] MUCompatGLuint AllocateDynamicTextureId()
{
    const MUCompatGLuint firstCandidate = s_nextTexture;
    do
    {
        const MUCompatGLuint candidate = s_nextTexture;
        s_nextTexture = candidate == kLastDynamicTextureId ? kFirstDynamicTextureId : candidate + 1u;
        if (!mu::GetRenderer().IsTextureRegistered(candidate))
        {
            return candidate;
        }
    } while (s_nextTexture != firstCandidate);

    return 0u;
}

void PushVertex(float x, float y, float z)
{
    s_vertices.push_back({x, y, z, s_u, s_v, s_color});
}

void SubmitTriangles(const std::vector<ImmediateVertex>& vertices)
{
    std::vector<mu::Vertex3D> out;
    out.reserve(vertices.size());
    for (const ImmediateVertex& v : vertices)
    {
        out.push_back({v.x, v.y, v.z, 0.0f, 0.0f, 1.0f, v.u, v.v, v.color});
    }
    mu::GetRenderer().RenderTriangles(out, s_texture2D ? s_boundTexture : 0u);
}

void SubmitQuadsAsTriangles()
{
    std::vector<ImmediateVertex> tris;
    tris.reserve((s_vertices.size() / 4) * 6);
    for (std::size_t i = 0; i + 3 < s_vertices.size(); i += 4)
    {
        tris.push_back(s_vertices[i + 0]);
        tris.push_back(s_vertices[i + 1]);
        tris.push_back(s_vertices[i + 2]);
        tris.push_back(s_vertices[i + 0]);
        tris.push_back(s_vertices[i + 2]);
        tris.push_back(s_vertices[i + 3]);
    }
    SubmitTriangles(tris);
}

void SubmitTriangleFan()
{
    std::vector<ImmediateVertex> tris;
    if (s_vertices.size() < 3)
    {
        return;
    }
    tris.reserve((s_vertices.size() - 2) * 3);
    for (std::size_t i = 1; i + 1 < s_vertices.size(); ++i)
    {
        tris.push_back(s_vertices[0]);
        tris.push_back(s_vertices[i]);
        tris.push_back(s_vertices[i + 1]);
    }
    SubmitTriangles(tris);
}

void ApplyBlend(MUCompatGLenum sfactor, MUCompatGLenum dfactor)
{
    if (sfactor == kGLSrcAlpha && dfactor == kGLOneMinusSrcAlpha)
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Alpha);
    else if (sfactor == kGLSrcAlpha && dfactor == kGLOne)
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Additive);
    else if (sfactor == kGLZero && dfactor == kGLOneMinusSrcColor)
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Subtract);
    else if (sfactor == kGLOneMinusDstColor && dfactor == kGLZero)
        mu::GetRenderer().SetBlendMode(mu::BlendMode::InverseColor);
    else if (sfactor == kGLOne && dfactor == kGLOneMinusSrcAlpha)
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Mixed);
    else if (sfactor == kGLZero && dfactor == kGLSrcColor)
        mu::GetRenderer().SetBlendMode(mu::BlendMode::LightMap);
    else if (sfactor == kGLOne && dfactor == kGLOne)
        mu::GetRenderer().SetBlendMode(mu::BlendMode::Glow);
}

[[nodiscard]] const void* PrepareTextureUpload(MUCompatGLenum format, MUCompatGLenum type, const void* pixels,
                                               MUCompatGLsizei width, MUCompatGLsizei height,
                                               std::vector<std::uint8_t>& rgbaScratch)
{
    if (!pixels || width <= 0 || height <= 0 || type != kGLUnsignedByte)
    {
        return pixels;
    }

    if (format == kGLRgba)
    {
        return pixels;
    }

    if (format != kGLRgb)
    {
        return pixels;
    }

    const auto pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    const auto* src = static_cast<const std::uint8_t*>(pixels);
    rgbaScratch.resize(pixelCount * 4u);
    for (std::size_t i = 0; i < pixelCount; ++i)
    {
        rgbaScratch[i * 4u + 0u] = src[i * 3u + 0u];
        rgbaScratch[i * 4u + 1u] = src[i * 3u + 1u];
        rgbaScratch[i * 4u + 2u] = src[i * 3u + 2u];
        rgbaScratch[i * 4u + 3u] = 255u;
    }
    return rgbaScratch.data();
}
} // namespace

void mu_glBegin(MUCompatGLenum mode)
{
    s_mode = mode;
    s_vertices.clear();
}

void mu_glEnd()
{
    switch (s_mode)
    {
    case kGLQuads:
        SubmitQuadsAsTriangles();
        break;
    case kGLTriangles:
        SubmitTriangles(s_vertices);
        break;
    case kGLLines: {
        std::vector<mu::Vertex3D> lines;
        lines.reserve(s_vertices.size());
        for (const ImmediateVertex& v : s_vertices)
            lines.push_back({v.x, v.y, v.z, 0.0f, 0.0f, 1.0f, v.u, v.v, v.color});
        mu::GetRenderer().RenderLines(lines, s_texture2D ? s_boundTexture : 0u);
        break;
    }
    case kGLTriangleFan:
        SubmitTriangleFan();
        break;
    case kGLQuadStrip: {
        std::vector<mu::Vertex3D> strip;
        strip.reserve(s_vertices.size());
        for (const ImmediateVertex& v : s_vertices)
            strip.push_back({v.x, v.y, v.z, 0.0f, 0.0f, 1.0f, v.u, v.v, v.color});
        mu::GetRenderer().RenderQuadStrip(strip, s_texture2D ? s_boundTexture : 0u);
        break;
    }
    default:
        break;
    }
    s_mode = 0;
    s_vertices.clear();
}

void mu_glVertex2f(MUCompatGLfloat x, MUCompatGLfloat y) { PushVertex(x, y, 0.0f); }
void mu_glVertex3f(MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z) { PushVertex(x, y, z); }
void mu_glVertex3fv(const MUCompatGLfloat* v) { PushVertex(v[0], v[1], v[2]); }
void mu_glTexCoord2f(MUCompatGLfloat u, MUCompatGLfloat v) { s_u = u; s_v = v; }
void mu_glColor3f(MUCompatGLfloat r, MUCompatGLfloat g, MUCompatGLfloat b) { mu_glColor4f(r, g, b, 1.0f); }
void mu_glColor3fv(const MUCompatGLfloat* c) { mu_glColor4f(c[0], c[1], c[2], 1.0f); }
void mu_glColor3ub(MUCompatGLubyte r, MUCompatGLubyte g, MUCompatGLubyte b) { mu_glColor4ub(r, g, b, 255); }
void mu_glColor4f(MUCompatGLfloat r, MUCompatGLfloat g, MUCompatGLfloat b, MUCompatGLfloat a)
{
    s_color = PackColor(ByteFromFloat(r), ByteFromFloat(g), ByteFromFloat(b), ByteFromFloat(a));
}
void mu_glColor4ub(MUCompatGLubyte r, MUCompatGLubyte g, MUCompatGLubyte b, MUCompatGLubyte a)
{
    s_color = PackColor(r, g, b, a);
}

void mu_glEnable(MUCompatGLenum cap)
{
    if (cap == kGLDepthTest) { s_depthTest = true; mu::GetRenderer().SetDepthTest(true); }
    else if (cap == kGLTexture2D) { s_texture2D = true; mu::GetRenderer().SetTexture2D(true); }
    else if (cap == kGLBlend) { s_blend = true; mu::GetRenderer().SetBlendMode(mu::BlendMode::Alpha); }
    else if (cap == kGLAlphaTest) { s_alphaTest = true; mu::GetRenderer().SetAlphaTest(true); }
    else if (cap == kGLCullFace) { s_cullFace = true; mu::GetRenderer().SetCullFace(true); }
    else if (cap == kGLFog) { s_fog = true; mu::GetRenderer().SetFogEnabled(true); }
    else if (cap == kGLStencilTest) { s_stencil = true; mu::GetRenderer().SetStencilTest(true); }
}

void mu_glDisable(MUCompatGLenum cap)
{
    if (cap == kGLDepthTest) { s_depthTest = false; mu::GetRenderer().SetDepthTest(false); }
    else if (cap == kGLTexture2D) { s_texture2D = false; mu::GetRenderer().SetTexture2D(false); }
    else if (cap == kGLBlend) { s_blend = false; mu::GetRenderer().DisableBlend(); }
    else if (cap == kGLAlphaTest) { s_alphaTest = false; mu::GetRenderer().SetAlphaTest(false); }
    else if (cap == kGLCullFace) { s_cullFace = false; mu::GetRenderer().SetCullFace(false); }
    else if (cap == kGLFog) { s_fog = false; mu::GetRenderer().SetFogEnabled(false); }
    else if (cap == kGLStencilTest) { s_stencil = false; mu::GetRenderer().SetStencilTest(false); }
}

MUCompatGLboolean mu_glIsEnabled(MUCompatGLenum cap)
{
    if (cap == kGLDepthTest) return s_depthTest;
    if (cap == kGLTexture2D) return s_texture2D;
    if (cap == kGLBlend) return s_blend;
    if (cap == kGLAlphaTest) return s_alphaTest;
    if (cap == kGLCullFace) return s_cullFace;
    if (cap == kGLFog) return s_fog;
    if (cap == kGLStencilTest) return s_stencil;
    return 0;
}

void mu_glBlendFunc(MUCompatGLenum sfactor, MUCompatGLenum dfactor) { ApplyBlend(sfactor, dfactor); }
void mu_glClear(MUCompatGLbitfield mask) { (mask & 0x00000100u) ? mu::GetRenderer().ClearDepthBuffer() : mu::GetRenderer().ClearScreen(); }
void mu_glClearColor(MUCompatGLfloat r, MUCompatGLfloat g, MUCompatGLfloat b, MUCompatGLfloat a) { mu::GetRenderer().SetClearColor(r, g, b, a); }
void mu_glDepthFunc(MUCompatGLenum func) { mu::GetRenderer().SetDepthFunc(static_cast<int>(func)); }
void mu_glDepthMask(MUCompatGLboolean flag) { mu::GetRenderer().SetDepthMask(flag != 0); }
void mu_glMatrixMode(MUCompatGLenum mode) { mu::GetRenderer().SetMatrixMode(static_cast<int>(mode)); }
void mu_glPushMatrix() { mu::GetRenderer().PushMatrix(); }
void mu_glPopMatrix() { mu::GetRenderer().PopMatrix(); }
void mu_glLoadIdentity() { mu::GetRenderer().LoadIdentity(); }
void mu_glTranslatef(MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z) { mu::GetRenderer().Translate(x, y, z); }
void mu_glRotatef(MUCompatGLfloat angle, MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z) { mu::GetRenderer().Rotate(angle, x, y, z); }
void mu_glScalef(MUCompatGLfloat x, MUCompatGLfloat y, MUCompatGLfloat z) { mu::GetRenderer().Scale(x, y, z); }
void mu_glGetFloatv(MUCompatGLenum pname, MUCompatGLfloat* data)
{
    if (pname == kGLModelViewMatrix || pname == kGLProjectionMatrix) mu::GetRenderer().GetMatrix(static_cast<int>(pname), data);
}
void mu_glGetIntegerv(MUCompatGLenum, MUCompatGLint* data)
{
    if (data != nullptr)
    {
        data[0] = 0;
        data[1] = 0;
    }
}
void mu_glViewport(MUCompatGLint x, MUCompatGLint y, MUCompatGLsizei width, MUCompatGLsizei height) { mu::GetRenderer().SetViewport(x, y, width, height); }
void mu_glLineWidth(MUCompatGLfloat) {}
void mu_glBindTexture(MUCompatGLenum, MUCompatGLuint texture) { s_boundTexture = texture; mu::GetRenderer().BindTexture(static_cast<int>(texture)); }
void mu_glGenTextures(MUCompatGLsizei n, MUCompatGLuint* textures)
{
    if (n <= 0 || textures == nullptr)
    {
        return;
    }

    for (int i = 0; i < n; ++i)
    {
        textures[i] = AllocateDynamicTextureId();
        if (textures[i] == 0u)
        {
            std::fill(textures + i, textures + n, 0u);
            return;
        }
    }
}
void mu_glDeleteTextures(MUCompatGLsizei n, const MUCompatGLuint* textures)
{
    if (!textures)
    {
        return;
    }

    for (int i = 0; i < n; ++i)
    {
        mu::GetRenderer().ReleaseTexture(textures[i]);
    }
}
void mu_glTexImage2D(MUCompatGLenum, MUCompatGLint, MUCompatGLint, MUCompatGLsizei width, MUCompatGLsizei height, MUCompatGLint, MUCompatGLenum format, MUCompatGLenum type, const void* pixels)
{
    mu::GetRenderer().EnsureTexture(s_boundTexture, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
    std::vector<std::uint8_t> rgbaScratch;
    const void* uploadPixels = PrepareTextureUpload(format, type, pixels, width, height, rgbaScratch);
    mu::GetRenderer().QueueTextureUpdate(s_boundTexture, uploadPixels, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
}
void mu_glTexSubImage2D(MUCompatGLenum, MUCompatGLint, MUCompatGLint, MUCompatGLint, MUCompatGLsizei width, MUCompatGLsizei height, MUCompatGLenum format, MUCompatGLenum type, const void* pixels)
{
    std::vector<std::uint8_t> rgbaScratch;
    const void* uploadPixels = PrepareTextureUpload(format, type, pixels, width, height, rgbaScratch);
    mu::GetRenderer().QueueTextureUpdate(s_boundTexture, uploadPixels, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
}
void mu_glTexParameteri(MUCompatGLenum target, MUCompatGLenum pname, MUCompatGLint param) { mu::GetRenderer().SetTexParameter(static_cast<int>(target), static_cast<int>(pname), param); }
void mu_glTexEnvi(MUCompatGLenum target, MUCompatGLenum pname, MUCompatGLint param) { mu::GetRenderer().SetTexEnv(static_cast<int>(target), static_cast<int>(pname), param); }
void mu_glTexEnvf(MUCompatGLenum target, MUCompatGLenum pname, MUCompatGLfloat param) { mu::GetRenderer().SetTexEnv(static_cast<int>(target), static_cast<int>(pname), static_cast<int>(param)); }
void mu_glAlphaFunc(MUCompatGLenum func, MUCompatGLfloat ref) { mu::GetRenderer().SetAlphaFunc(static_cast<int>(func), ref); }
void mu_glStencilFunc(MUCompatGLenum func, MUCompatGLint ref, MUCompatGLuint mask) { mu::GetRenderer().SetStencilFunc(static_cast<int>(func), ref, mask); }
void mu_glStencilOp(MUCompatGLenum sfail, MUCompatGLenum dpfail, MUCompatGLenum dppass) { mu::GetRenderer().SetStencilOp(static_cast<int>(sfail), static_cast<int>(dpfail), static_cast<int>(dppass)); }
void mu_glColorMask(MUCompatGLboolean r, MUCompatGLboolean g, MUCompatGLboolean b, MUCompatGLboolean a) { mu::GetRenderer().SetColorMask(r, g, b, a); }
void mu_glPolygonMode(MUCompatGLenum face, MUCompatGLenum mode) { mu::GetRenderer().SetPolygonMode(static_cast<int>(face), static_cast<int>(mode)); }
void mu_glFrontFace(MUCompatGLenum mode) { mu::GetRenderer().SetFrontFace(static_cast<int>(mode)); }
void mu_glFogf(MUCompatGLenum, MUCompatGLfloat) {}
void mu_glFogi(MUCompatGLenum, MUCompatGLint) {}
void mu_glFogfv(MUCompatGLenum, const MUCompatGLfloat*) {}
void mu_glReadPixels(MUCompatGLint x, MUCompatGLint y, MUCompatGLsizei width, MUCompatGLsizei height, MUCompatGLenum, MUCompatGLenum, void* pixels) { mu::GetRenderer().ReadPixels(x, y, width, height, pixels); }
const MUCompatGLubyte* mu_glGetString(MUCompatGLenum)
{
    return reinterpret_cast<const MUCompatGLubyte*>(mu::GetRenderer().GetGPUDriverName());
}
void mu_gluPerspective(MUCompatGLdouble fovy, MUCompatGLdouble aspect, MUCompatGLdouble zNear, MUCompatGLdouble zFar)
{
    const glm::mat4 perspective = glm::perspective(glm::radians(static_cast<float>(fovy)),
                                                   static_cast<float>(aspect),
                                                   static_cast<float>(zNear),
                                                   static_cast<float>(zFar));
    mu::GetRenderer().MultMatrix(glm::value_ptr(perspective));
}
void mu_gluOrtho2D(MUCompatGLdouble left, MUCompatGLdouble right, MUCompatGLdouble bottom, MUCompatGLdouble top)
{
    const glm::mat4 ortho = glm::ortho(static_cast<float>(left), static_cast<float>(right),
                                       static_cast<float>(bottom), static_cast<float>(top));
    mu::GetRenderer().MultMatrix(glm::value_ptr(ortho));
}
void* mu_gluNewQuadric() { return nullptr; }
void mu_gluSphere(void*, MUCompatGLdouble, MUCompatGLint, MUCompatGLint) {}
void mu_glEnableClientState(MUCompatGLenum) {}
void mu_glDisableClientState(MUCompatGLenum) {}
void mu_glVertexPointer(MUCompatGLint, MUCompatGLenum, MUCompatGLsizei, const void*) {}
void mu_glTexCoordPointer(MUCompatGLint, MUCompatGLenum, MUCompatGLsizei, const void*) {}
void mu_glColorPointer(MUCompatGLint, MUCompatGLenum, MUCompatGLsizei, const void*) {}
void mu_glDrawArrays(MUCompatGLenum, MUCompatGLint, MUCompatGLsizei) {}
void mu_glReadBuffer(MUCompatGLenum) {}
void mu_glFlush() {}
void mu_glPixelStorei(MUCompatGLenum, MUCompatGLint) {}
void mu_glPushAttrib(MUCompatGLbitfield) {}
void mu_glPopAttrib() {}
void mu_glNormal3f(MUCompatGLfloat, MUCompatGLfloat, MUCompatGLfloat) {}
void mu_glCopyTexImage2D(MUCompatGLenum, MUCompatGLint, MUCompatGLenum, MUCompatGLint, MUCompatGLint, MUCompatGLsizei, MUCompatGLsizei, MUCompatGLint) {}
