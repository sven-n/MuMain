#pragma once

#include "stdafx.h"

// DXP-22: single-source-of-truth wrapper monopoly for glUseProgram / glBindTexture(GL_TEXTURE_2D) /
// glBindVertexArray -- the bind-time counterpart to DXP-10's enable/disable state monopoly
// (AlphaBlendType/CullFaceEnable/etc. in ZzzOpenglUtil.cpp). Step 2-3 converted every real call site
// in the tree to call these; step 5 turned on last-bound-id caching (the GL call is skipped when the
// requested id is already bound) now that step 3's conversion makes the cache trustworthy end to end
// -- see .ai-os/memory/tasks/dxp/DXP-22-bind-state-monopoly-perf.md. Note: BindTexture2D caches the
// bound texture id per slot only, NOT the active texture unit itself -- see the comment in
// BindState.cpp for why caching the active unit would be unsafe here.

void BindProgram(GLuint program);
void BindTexture2D(int slot, GLuint texture);
void BindVAO(GLuint vao);

// Explicit program-0 reset. Used immediately before raw fixed-function immediate-mode draws
// (glBegin/glEnd with no shader bound at all) -- BMD::RenderMesh() no longer reflexively unbinds
// after every mesh, so this covers the one load-bearing case that still needs it explicitly. See
// DXP-22 "The actual fix shape" and ZzzLodTerrain.cpp's RenderTerrainFrustrum().
void UnbindAllShaders();

// Call after any glDeleteVertexArrays/glDeleteProgram/glDeleteTextures affecting an object that
// might be cached here. Deleting a currently-bound object implicitly reverts GL's real binding to 0;
// if the freed numeric id later gets reused by a Gen call (routine under GL's id-recycling), this
// wrapper's cache would otherwise still think the reused id is already correctly bound and skip the
// real bind on the next request -- silently rendering with whatever object is actually left bound
// instead. Conservative by design: invalidates the whole relevant cache rather than trying to match
// the specific deleted id, since deletions are rare relative to binds and the cost of an unnecessary
// rebind is negligible next to a wrong-geometry render. Found and fixed 2026-08-02 after this exact
// bug produced a stretched "infinity shadow" on the character-select screen (BMD model VAOs are
// created/destroyed on every character/equipment switch there).
void InvalidateProgramCache();
void InvalidateTextureCache();
void InvalidateVAOCache();

// GLP-05: resets the cached active texture unit to "unknown", forcing the next BindTexture2D
// call to reissue glActiveTexture regardless of what slot it requests. Called alongside
// InvalidateTextureCache() (RHI_GL_Impl::DestroyTexture) and on context recreation, as defensive
// belt-and-suspenders for the bind-state monopoly rather than because texture deletion itself
// changes the active unit -- it doesn't; glDeleteTextures affects binding, not glActiveTexture
// state.
void InvalidateActiveTextureUnitCache();
