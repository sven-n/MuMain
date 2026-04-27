#pragma once

#ifdef _EDITOR

#include "Frustum.h"

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
void RenderFrustumWireframe(const Frustum& frustum);

#endif // _EDITOR
