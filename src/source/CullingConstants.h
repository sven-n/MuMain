///////////////////////////////////////////////////////////////////////////////
// CullingConstants.h
// Centralized culling radius constants used across the codebase
// These values control frustum culling sphere tests for different object types
///////////////////////////////////////////////////////////////////////////////

#pragma once

// Terrain culling
constexpr float CULL_RADIUS_TERRAIN = 50.0f;  // Terrain tile culling sphere radius

// Character culling
constexpr float CULL_RADIUS_CHARACTER = 100.0f;  // Characters, NPCs, monsters

// Item culling
constexpr float CULL_RADIUS_ITEM = 50.0f;  // Dropped items on ground

// Static object culling (world objects like walls, trees, etc.)
constexpr float CULL_RADIUS_OBJECT_SMALL = 50.0f;   // Normal sized objects
constexpr float CULL_RADIUS_OBJECT_MEDIUM = 100.0f;  // Medium objects
constexpr float CULL_RADIUS_OBJECT_LARGE = 200.0f;  // Large objects (towers, big structures)

// Object block culling (16x16 tile blocks)
constexpr float CULL_RADIUS_OBJECT_BLOCK = 400.0f;  // 180 tiles * 100 units/tile
