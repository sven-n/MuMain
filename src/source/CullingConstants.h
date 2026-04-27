///////////////////////////////////////////////////////////////////////////////
// CullingConstants.h
// Centralized culling radius constants used across the codebase
// These values control frustum culling sphere tests for different object types
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Default culling radii - these are fallback values
// Actual values can be overridden at runtime via DevEditor.
// The item radius matches the legacy `TestFrustrum(o->Position, 400.f)` value;
// large item models (weapons, armor sets) extend further than 100 from their
// centre, so a tighter radius caused them to flicker at the edge of view as
// the centre point exited the frustum before the geometry did.
constexpr float DEFAULT_CULL_RADIUS_ITEM = 400.0f;
constexpr float DEFAULT_CULL_RADIUS_OBJECT = 100.0f;  // Unified radius for all objects
