#pragma once

#ifdef _EDITOR

#include <unordered_map>

// Renders loaded world-object models (Models[type]) to small textures for use as
// ImGui thumbnails, so the object palette can show a preview grid instead of a
// name list. Each model is rendered once into a cached texture via an offscreen
// framebuffer; a per-frame budget spreads the first-time cost over a few frames.
class CObjectThumbnail
{
public:
    static CObjectThumbnail& GetInstance();

    // Call once at the start of each editor frame to reset the per-frame budget.
    void BeginFrame();

    // Returns a GL texture id previewing model `type`, rendering it on demand
    // (subject to the per-frame budget). Returns 0 if not ready this frame or the
    // model isn't loaded.
    unsigned int Get(int type);

    // Drops all cached thumbnails (e.g. on map change, since model slots change).
    void Invalidate();

    // Renders the model currently in Models[slot] to a NEW texture (no caching);
    // the caller owns/frees it. Used by the object browser to preview foreign
    // models it loads into a scratch slot. Returns 0 if the slot has no model.
    unsigned int RenderSlotToTexture(int slot);

    // Frees a texture returned by RenderSlotToTexture (caller-owned).
    static void FreeTexture(unsigned int tex);

    int Size() const { return THUMB_SIZE; }

private:
    CObjectThumbnail() = default;
    ~CObjectThumbnail() = default;

    bool EnsureTargets();

    static constexpr int THUMB_SIZE = 112;
    static constexpr int MAX_RENDERS_PER_FRAME = 6;

    bool         m_init = false;
    unsigned int m_fbo = 0;
    unsigned int m_depth = 0;
    int          m_budget = 0;
    std::unordered_map<int, unsigned int> m_cache;  // type -> color texture (0 = failed)
};

#define g_ObjectThumbnail CObjectThumbnail::GetInstance()

#endif // _EDITOR
