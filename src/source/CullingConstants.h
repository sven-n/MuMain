///////////////////////////////////////////////////////////////////////////////
// CullingConstants.h
// Centralized culling radius constants used across the codebase
// These values control frustum culling sphere tests for different object types
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Default culling radii - these are fallback values
// Actual values can be overridden at runtime via DevEditor
constexpr float DEFAULT_CULL_RADIUS_TERRAIN = 200.0f;
constexpr float DEFAULT_CULL_RADIUS_CHARACTER = 100.0f;
constexpr float DEFAULT_CULL_RADIUS_ITEM = 100.0f;
constexpr float DEFAULT_CULL_RADIUS_OBJECT = 100.0f;  // Unified radius for all objects
