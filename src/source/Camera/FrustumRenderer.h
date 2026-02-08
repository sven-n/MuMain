#pragma once

#ifdef _EDITOR

#include "CameraConfig.h"
#include "Frustum.h"
#include "_types.h"
#include "../_define.h"

float RequestTerrainHeight(float xf, float yf);

#include <cmath>
#include <cstring>

/**
 * @brief Renders the frustum visualization for FreeFly spectator mode
 *
 * Draws:
 * - Color-coded wireframe (green near, red far, yellow sides)
 * - Semi-transparent filled side planes for volume visibility
 * - Green ground projection on terrain surface
 * - Cyan camera position marker
 *
 * Far plane vertices are scaled by RENDER_DISTANCE_MULTIPLIER to match
 * the actual GL projection extent set by BeginOpengl().
 *
 * Vertices layout from Frustum::GetVertices():
 * [0-3] = Near plane (top-left, top-right, bottom-right, bottom-left)
 * [4-7] = Far plane  (top-left, top-right, bottom-right, bottom-left)
 */
inline void RenderFrustumWireframe(const Frustum& frustum)
{
    const vec3_t* orig = frustum.GetVertices();

    // Copy vertices so we can scale far plane
    vec3_t v[8];
    std::memcpy(v, orig, sizeof(vec3_t) * 8);

    // Compute camera position as the intersection point behind the near plane.
    // The near plane is very close to the camera, so average of near corners
    // projected back along the side edges gives a good approximation.
    vec3_t camPos;
    camPos[0] = (v[0][0] + v[1][0] + v[2][0] + v[3][0]) * 0.25f;
    camPos[1] = (v[0][1] + v[1][1] + v[2][1] + v[3][1]) * 0.25f;
    camPos[2] = (v[0][2] + v[1][2] + v[2][2] + v[3][2]) * 0.25f;

    // Scale far-plane vertices outward from camera by RENDER_DISTANCE_MULTIPLIER
    // to match the actual GL projection extent
    for (int i = 4; i < 8; ++i)
    {
        for (int axis = 0; axis < 3; ++axis)
        {
            float dir = v[i][axis] - camPos[axis];
            v[i][axis] = camPos[axis] + dir * RENDER_DISTANCE_MULTIPLIER;
        }
    }

    // Save OpenGL state
    GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    GLboolean texture2D = glIsEnabled(GL_TEXTURE_2D);
    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    GLboolean blend = glIsEnabled(GL_BLEND);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Color-coded wireframe ---
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    // Near plane edges - green
    glColor4f(0.0f, 1.0f, 0.0f, 0.8f);
    glVertex3fv(v[0]); glVertex3fv(v[1]);
    glVertex3fv(v[1]); glVertex3fv(v[2]);
    glVertex3fv(v[2]); glVertex3fv(v[3]);
    glVertex3fv(v[3]); glVertex3fv(v[0]);

    // Far plane edges - red
    glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
    glVertex3fv(v[4]); glVertex3fv(v[5]);
    glVertex3fv(v[5]); glVertex3fv(v[6]);
    glVertex3fv(v[6]); glVertex3fv(v[7]);
    glVertex3fv(v[7]); glVertex3fv(v[4]);

    // Connecting side edges - yellow
    glColor4f(1.0f, 1.0f, 0.0f, 0.8f);
    glVertex3fv(v[0]); glVertex3fv(v[4]);
    glVertex3fv(v[1]); glVertex3fv(v[5]);
    glVertex3fv(v[2]); glVertex3fv(v[6]);
    glVertex3fv(v[3]); glVertex3fv(v[7]);

    glEnd();

    // --- Semi-transparent filled planes ---
    glColor4f(1.0f, 1.0f, 0.0f, 0.08f);
    glBegin(GL_QUADS);

    // Left side (0, 3, 7, 4)
    glVertex3fv(v[0]); glVertex3fv(v[3]); glVertex3fv(v[7]); glVertex3fv(v[4]);
    // Right side (1, 2, 6, 5)
    glVertex3fv(v[1]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[5]);
    // Top (0, 1, 5, 4)
    glVertex3fv(v[0]); glVertex3fv(v[1]); glVertex3fv(v[5]); glVertex3fv(v[4]);
    // Bottom (3, 2, 6, 7)
    glVertex3fv(v[3]); glVertex3fv(v[2]); glVertex3fv(v[6]); glVertex3fv(v[7]);
    // Far (4, 5, 6, 7)
    glVertex3fv(v[4]); glVertex3fv(v[5]); glVertex3fv(v[6]); glVertex3fv(v[7]);

    glEnd();

    // --- Ground projection on terrain ---
    const int hullCount = frustum.Get2DCount();
    if (hullCount >= 3)
    {
        const float* hullX = frustum.Get2DX();
        const float* hullY = frustum.Get2DY();
        const float zOffset = 5.0f;

        glLineWidth(3.0f);
        glColor4f(0.0f, 1.0f, 0.0f, 0.7f);
        glBegin(GL_LINES);

        for (int i = 0; i < hullCount; ++i)
        {
            int next = (i + 1) % hullCount;

            float x0 = hullX[i] * TERRAIN_SCALE;
            float y0 = hullY[i] * TERRAIN_SCALE;
            float x1 = hullX[next] * TERRAIN_SCALE;
            float y1 = hullY[next] * TERRAIN_SCALE;

            // Subdivide edge to follow terrain elevation
            float dx = x1 - x0;
            float dy = y1 - y0;
            float edgeLen = std::sqrt(dx * dx + dy * dy);
            int segments = (int)(edgeLen / (2.0f * TERRAIN_SCALE)) + 1;
            if (segments < 1) segments = 1;
            if (segments > 32) segments = 32;

            for (int s = 0; s < segments; ++s)
            {
                float t0 = (float)s / (float)segments;
                float t1 = (float)(s + 1) / (float)segments;

                float sx0 = x0 + dx * t0;
                float sy0 = y0 + dy * t0;
                float sx1 = x0 + dx * t1;
                float sy1 = y0 + dy * t1;

                float z0 = RequestTerrainHeight(sx0, sy0) + zOffset;
                float z1 = RequestTerrainHeight(sx1, sy1) + zOffset;

                glVertex3f(sx0, sy0, z0);
                glVertex3f(sx1, sy1, z1);
            }
        }

        glEnd();
    }

    // --- Camera position marker (cyan cross at near plane center) ---
    const float markerSize = 50.0f;
    glColor4f(0.0f, 1.0f, 1.0f, 0.9f);
    glBegin(GL_LINES);
    glVertex3f(camPos[0] - markerSize, camPos[1], camPos[2]);
    glVertex3f(camPos[0] + markerSize, camPos[1], camPos[2]);
    glVertex3f(camPos[0], camPos[1] - markerSize, camPos[2]);
    glVertex3f(camPos[0], camPos[1] + markerSize, camPos[2]);
    glVertex3f(camPos[0], camPos[1], camPos[2] - markerSize);
    glVertex3f(camPos[0], camPos[1], camPos[2] + markerSize);
    glEnd();

    // Restore OpenGL state
    glLineWidth(1.0f);
    if (depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (texture2D) glEnable(GL_TEXTURE_2D); else glDisable(GL_TEXTURE_2D);
    if (lighting) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
    if (!blend) glDisable(GL_BLEND);
}

#endif // _EDITOR
