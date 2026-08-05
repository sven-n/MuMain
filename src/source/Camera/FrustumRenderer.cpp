#include "stdafx.h"

#ifdef _EDITOR

#include "FrustumRenderer.h"
#include "CameraConfig.h"
#include "Core/Globals/_types.h"
#include "Core/Globals/_define.h"
#include <cmath>
#include <cstring>
#include "Render/Core/ImmediateRenderer.h"
#include "Render/Shaders/PassthroughShader.h"
#include "Render/Core/RenderConfig.h"

float RequestTerrainHeight(float xf, float yf);

namespace
{
    // Distance threshold for degenerate/orthographic detection when computing camera apex.
    constexpr float DEGENERATE_EPSILON = 0.001f;

    // Vertical offset above terrain surface for the ground-projection line so it
    // doesn't z-fight with the ground itself.
    constexpr float GROUND_LINE_Z_OFFSET = 5.0f;

    // Ground-projection edge subdivision: one segment per (SUBDIVISIONS_PER_TILE * TERRAIN_SCALE)
    // units of edge length, clamped to [1, MAX_EDGE_SEGMENTS].
    constexpr float SUBDIVISIONS_PER_TILE = 2.0f;
    constexpr int   MAX_EDGE_SEGMENTS = 32;

    // Camera position marker is drawn as an axis-aligned cross with arms this long.
    constexpr float CAMERA_MARKER_HALF_LENGTH = 50.0f;

    // Line widths used for the different visualization layers.
    constexpr float WIREFRAME_LINE_WIDTH = 2.0f;
    constexpr float GROUND_LINE_WIDTH    = 3.0f;

    // RAII wrapper that snapshots GL state we touch and restores it on destruction.
    struct GLStateScope
    {
        GLboolean depthTest;
        GLboolean texture2D;
        GLboolean lighting;
        GLboolean blend;

        GLStateScope()
        {
            depthTest = glIsEnabled(GL_DEPTH_TEST);
            texture2D = GL_FALSE;
            lighting  = GL_FALSE;
            blend     = glIsEnabled(GL_BLEND);

            if (!g_CoreProfile)
            {
                texture2D = glIsEnabled(GL_TEXTURE_2D);
                lighting  = glIsEnabled(GL_LIGHTING);
            }

            glDisable(GL_DEPTH_TEST);
            if (!g_CoreProfile) { glDisable(GL_TEXTURE_2D); glDisable(GL_LIGHTING); }
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        ~GLStateScope()
        {
            glLineWidth(1.0f);
            if (depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
            if (!g_CoreProfile)
            {
                if (texture2D) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
                if (lighting)  glEnable(GL_LIGHTING);  else glDisable(GL_LIGHTING);
            }
            if (!blend) glDisable(GL_BLEND);
        }
    };

    // Average of 4 vec3_t points.
    inline void AverageQuad(const vec3_t a, const vec3_t b, const vec3_t c, const vec3_t d, vec3_t out)
    {
        out[0] = (a[0] + b[0] + c[0] + d[0]) * 0.25f;
        out[1] = (a[1] + b[1] + c[1] + d[1]) * 0.25f;
        out[2] = (a[2] + b[2] + c[2] + d[2]) * 0.25f;
    }

    // Distance between two vec3_t points.
    inline float Distance3D(const vec3_t a, const vec3_t b)
    {
        float dx = a[0] - b[0], dy = a[1] - b[1], dz = a[2] - b[2];
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    // Compute the frustum apex (camera eye) by extrapolating the near→far edge back
    // to where the plane width would be zero. Falls back to nearCenter for degenerate cases.
    void ComputeFrustumApex(const vec3_t v[8], vec3_t outApex,
                            vec3_t outNearCenter, vec3_t outFarCenter)
    {
        AverageQuad(v[0], v[1], v[2], v[3], outNearCenter);
        AverageQuad(v[4], v[5], v[6], v[7], outFarCenter);

        float nearHW = Distance3D(v[0], outNearCenter);
        float farHW  = Distance3D(v[4], outFarCenter);

        float dx = outFarCenter[0] - outNearCenter[0];
        float dy = outFarCenter[1] - outNearCenter[1];
        float dz = outFarCenter[2] - outNearCenter[2];
        float planeDist = std::sqrt(dx * dx + dy * dy + dz * dz);

        float denom = farHW - nearHW;
        if (planeDist > DEGENERATE_EPSILON && std::fabs(denom) > DEGENERATE_EPSILON)
        {
            float nearDist = nearHW * planeDist / denom;
            float invPlaneDist = 1.0f / planeDist;
            outApex[0] = outNearCenter[0] - dx * invPlaneDist * nearDist;
            outApex[1] = outNearCenter[1] - dy * invPlaneDist * nearDist;
            outApex[2] = outNearCenter[2] - dz * invPlaneDist * nearDist;
        }
        else
        {
            // Fallback: use near-plane center (orthographic or degenerate)
            outApex[0] = outNearCenter[0];
            outApex[1] = outNearCenter[1];
            outApex[2] = outNearCenter[2];
        }
    }

    // Scale far-plane vertices outward from the camera by RENDER_DISTANCE_MULTIPLIER
    // so the visualization matches the actual GL projection extent.
    void ScaleFarVerticesFromApex(vec3_t v[8], const vec3_t apex)
    {
        for (int i = 4; i < 8; ++i)
        {
            for (int axis = 0; axis < 3; ++axis)
            {
                float dir = v[i][axis] - apex[axis];
                v[i][axis] = apex[axis] + dir * RENDER_DISTANCE_MULTIPLIER;
            }
        }
    }

    void RenderPyramidWireframe(const vec3_t v[8], const vec3_t apex)
    {
        glLineWidth(WIREFRAME_LINE_WIDTH);

        IR::Begin(GL_LINES);
        PassthroughShader::Instance().SetUseTexture(false);
        // Near plane edges - green
        IR::Color4f(0.0f, 1.0f, 0.0f, 0.8f);
        IR::Vertex3fv(v[0]); IR::Vertex3fv(v[1]);
        IR::Vertex3fv(v[1]); IR::Vertex3fv(v[2]);
        IR::Vertex3fv(v[2]); IR::Vertex3fv(v[3]);
        IR::Vertex3fv(v[3]); IR::Vertex3fv(v[0]);

        // Far plane edges - red
        IR::Color4f(1.0f, 0.0f, 0.0f, 0.8f);
        IR::Vertex3fv(v[4]); IR::Vertex3fv(v[5]);
        IR::Vertex3fv(v[5]); IR::Vertex3fv(v[6]);
        IR::Vertex3fv(v[6]); IR::Vertex3fv(v[7]);
        IR::Vertex3fv(v[7]); IR::Vertex3fv(v[4]);

        // Side edges from eye to far corners - yellow
        IR::Color4f(1.0f, 1.0f, 0.0f, 0.8f);
        IR::Vertex3fv(apex); IR::Vertex3fv(v[4]);
        IR::Vertex3fv(apex); IR::Vertex3fv(v[5]);
        IR::Vertex3fv(apex); IR::Vertex3fv(v[6]);
        IR::Vertex3fv(apex); IR::Vertex3fv(v[7]);
        IR::End();
    }

    void RenderPyramidFilled(const vec3_t v[8], const vec3_t apex)
    {
        IR::Begin(GL_TRIANGLES);
        PassthroughShader::Instance().SetUseTexture(false);
        IR::Color4f(1.0f, 1.0f, 0.0f, 0.08f);
        IR::Vertex3fv(apex); IR::Vertex3fv(v[4]); IR::Vertex3fv(v[7]);
        IR::Vertex3fv(apex); IR::Vertex3fv(v[5]); IR::Vertex3fv(v[6]);
        IR::Vertex3fv(apex); IR::Vertex3fv(v[4]); IR::Vertex3fv(v[5]);
        IR::Vertex3fv(apex); IR::Vertex3fv(v[7]); IR::Vertex3fv(v[6]);
        IR::End();

        IR::Begin(GL_QUADS);
        PassthroughShader::Instance().SetUseTexture(false);
        IR::Color4f(1.0f, 1.0f, 0.0f, 0.08f);
        IR::Vertex3fv(v[4]); IR::Vertex3fv(v[5]); IR::Vertex3fv(v[6]); IR::Vertex3fv(v[7]);
        IR::End();
    }

    void RenderGroundProjection(const Frustum& frustum)
    {
        const int hullCount = frustum.Get2DCount();
        if (hullCount < 3)
            return;

        const float* hullX = frustum.Get2DX();
        const float* hullY = frustum.Get2DY();

        glLineWidth(GROUND_LINE_WIDTH);

        IR::Begin(GL_LINES);
        PassthroughShader::Instance().SetUseTexture(false);
        IR::Color4f(0.0f, 1.0f, 0.0f, 0.7f);

        for (int i = 0; i < hullCount; ++i)
        {
            int next = (i + 1) % hullCount;

            float x0 = hullX[i] * TERRAIN_SCALE;
            float y0 = hullY[i] * TERRAIN_SCALE;
            float x1 = hullX[next] * TERRAIN_SCALE;
            float y1 = hullY[next] * TERRAIN_SCALE;

            float dx = x1 - x0;
            float dy = y1 - y0;
            float edgeLen = std::sqrt(dx * dx + dy * dy);

            int segments = static_cast<int>(std::ceil(edgeLen / (SUBDIVISIONS_PER_TILE * TERRAIN_SCALE)));
            segments = std::max(1, std::min(segments, MAX_EDGE_SEGMENTS));

            for (int s = 0; s < segments; ++s)
            {
                float tA = static_cast<float>(s) / static_cast<float>(segments);
                float tB = static_cast<float>(s + 1) / static_cast<float>(segments);

                float px0 = x0 + tA * dx;
                float py0 = y0 + tA * dy;
                float pz0 = RequestTerrainHeight(px0, py0) + GROUND_LINE_Z_OFFSET;

                float px1 = x0 + tB * dx;
                float py1 = y0 + tB * dy;
                float pz1 = RequestTerrainHeight(px1, py1) + GROUND_LINE_Z_OFFSET;

                IR::Vertex3f(px0, py0, pz0);
                IR::Vertex3f(px1, py1, pz1);
            }
        }

        IR::End();
    }

    // Draw a terrain-hugging horizontal line between two ground hit points.
    void DrawGroundSegment(float x0, float y0, float x1, float y1)
    {
        float dx = x1 - x0;
        float dy = y1 - y0;
        float edgeLen = std::sqrt(dx * dx + dy * dy);

        int segments = static_cast<int>(std::ceil(edgeLen / (SUBDIVISIONS_PER_TILE * TERRAIN_SCALE)));
        segments = std::max(1, std::min(segments, MAX_EDGE_SEGMENTS));

        for (int s = 0; s < segments; ++s)
        {
            float tA = static_cast<float>(s) / static_cast<float>(segments);
            float tB = static_cast<float>(s + 1) / static_cast<float>(segments);

            float px0 = x0 + tA * dx;
            float py0 = y0 + tA * dy;
            float pz0 = RequestTerrainHeight(px0, py0) + GROUND_LINE_Z_OFFSET;

            float px1 = x0 + tB * dx;
            float py1 = y0 + tB * dy;
            float pz1 = RequestTerrainHeight(px1, py1) + GROUND_LINE_Z_OFFSET;

            IR::Vertex3f(px0, py0, pz0);
            IR::Vertex3f(px1, py1, pz1);
        }
    }

    // Cast each ray (apex → far-plane corner) onto the ground plane (Z=0)
    // and connect the resulting points to show where the camera's top and
    // bottom FOV edges intersect the ground. Frustum vertex order from
    // CalculateFrustumVertices: 4=far TL, 5=far TR, 6=far BR, 7=far BL.
    void RenderFovGroundIntersect(const vec3_t apex, const vec3_t v[8])
    {
        auto rayToGround = [&](const vec3_t corner, float& outX, float& outY) -> bool
        {
            float dz = corner[2] - apex[2];
            if (dz >= -DEGENERATE_EPSILON) return false;  // ray not going down
            float t = -apex[2] / dz;
            if (t <= 0.0f) return false;
            outX = apex[0] + t * (corner[0] - apex[0]);
            outY = apex[1] + t * (corner[1] - apex[1]);
            return true;
        };

        float topLx, topLy, topRx, topRy, botLx, botLy, botRx, botRy;
        bool tlOk = rayToGround(v[4], topLx, topLy);
        bool trOk = rayToGround(v[5], topRx, topRy);
        bool brOk = rayToGround(v[6], botRx, botRy);
        bool blOk = rayToGround(v[7], botLx, botLy);

        glLineWidth(GROUND_LINE_WIDTH + 1.0f);

        IR::Begin(GL_LINES);
        PassthroughShader::Instance().SetUseTexture(false);

        if (blOk && brOk)
        {
            IR::Color4f(1.0f, 0.0f, 0.0f, 0.9f);
            DrawGroundSegment(botLx, botLy, botRx, botRy);
        }
        if (tlOk && trOk)
        {
            IR::Color4f(1.0f, 1.0f, 0.0f, 0.9f);
            DrawGroundSegment(topLx, topLy, topRx, topRy);
        }

        IR::End();
    }

    void RenderCameraMarker(const vec3_t apex)
    {
        IR::Begin(GL_LINES);
        PassthroughShader::Instance().SetUseTexture(false);
        IR::Color4f(0.0f, 1.0f, 1.0f, 0.9f);
        IR::Vertex3f(apex[0] - CAMERA_MARKER_HALF_LENGTH, apex[1], apex[2]);
        IR::Vertex3f(apex[0] + CAMERA_MARKER_HALF_LENGTH, apex[1], apex[2]);
        IR::Vertex3f(apex[0], apex[1] - CAMERA_MARKER_HALF_LENGTH, apex[2]);
        IR::Vertex3f(apex[0], apex[1] + CAMERA_MARKER_HALF_LENGTH, apex[2]);
        IR::Vertex3f(apex[0], apex[1], apex[2] - CAMERA_MARKER_HALF_LENGTH);
        IR::Vertex3f(apex[0], apex[1], apex[2] + CAMERA_MARKER_HALF_LENGTH);
        IR::End();
    }
}

void RenderFrustumWireframe(const Frustum& frustum)
{
    // Copy vertices so we can scale the far plane without mutating the frustum
    vec3_t v[8];
    std::memcpy(v, frustum.GetVertices(), sizeof(vec3_t) * 8);

    vec3_t apex, nearCenter, farCenter;
    ComputeFrustumApex(v, apex, nearCenter, farCenter);
    ScaleFarVerticesFromApex(v, apex);

    GLStateScope stateScope;  // RAII: restores GL state on destruction
    RenderPyramidWireframe(v, apex);
    RenderPyramidFilled(v, apex);
    RenderGroundProjection(frustum);
    // FOV ground-intersection lines: ray-cast through the scaled far
    // corners (= the cone's yellow apex→corner edges) so the lines end
    // exactly where the visualization cone hits the ground.
    RenderFovGroundIntersect(apex, v);
    RenderCameraMarker(apex);
}

#endif // _EDITOR
