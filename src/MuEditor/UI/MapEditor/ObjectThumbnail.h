#pragma once

#ifdef _EDITOR

#include <unordered_map>
#include <vector>

// Renders loaded world-object models (Models[type]) to small textures for use as
// ImGui thumbnails, so the object palette can show a preview grid instead of a
// name list.
//
// Get()/RequestSlotPreview() are called from ImGui widget-building code
// (CMuEditorCore::Update(), via CMapEditorUI/CMapObjectBrowser), which runs
// BEFORE mu::GetRenderer().BeginFrame() for the frame - the renderer's offscreen
// capture needs state (s_frameActive, the per-frame command buffer) that only
// exists between BeginFrame()/EndFrame(). So neither function renders directly:
// they queue a request and return either a cached result or "not ready yet".
// ProcessPendingRequests(), called from RenderObjects() (which does run inside
// BeginFrame/EndFrame, same as the selection outline), performs the actual
// renders - one frame of latency, invisible in practice given the grid is
// redrawn every frame anyway.
//
// Returned ids are renderer texture ids (mu::GetRenderer()'s own id space), not
// raw GPU handles - pass them to mu::GetRenderer().GetTexturePointer() to get the
// real pointer for ImGui::Image/ImageButton.
class CObjectThumbnail
{
public:
    static CObjectThumbnail& GetInstance();

    // Call once at the start of each editor frame to reset the per-frame budget.
    void BeginFrame();

    // Returns a texture id previewing model `type`, queuing a render if not
    // already cached (subject to the per-frame budget). Returns 0 if not ready
    // yet or the model isn't loaded.
    unsigned int Get(int type);

    // Drops all cached thumbnails (e.g. on map change, since model slots change).
    void Invalidate();

    // Foreign-model (object browser / O.Browse) preview path. Models[] has
    // exactly one scratch slot shared by every candidate file (see
    // MapObjectImport::LoadForPreview) - loading a new model into it overwrites
    // whatever the previous one put there, so at most one request can be in
    // flight at a time. Usage:
    //   1. After loading a model into the scratch slot, call
    //      RequestSlotPreview(slot). If it returns false, a previous request
    //      hasn't been delivered yet - do NOT overwrite the scratch slot's
    //      model; retry the same request next frame instead.
    //   2. On later frames, call PollSlotPreview() until IsScratchBusy()
    //      becomes false; its return value is then the result (0 = the model
    //      failed to render). The caller is responsible for remembering which
    //      of its own candidates the in-flight request belongs to.
    bool IsScratchBusy() const { return m_scratchPending; }
    bool RequestSlotPreview(int slot);
    unsigned int PollSlotPreview();

    // Frees a texture returned by Get() or PollSlotPreview() (caller-owned for
    // the latter; Get()'s cache owns its own and frees via Invalidate()).
    static void FreeTexture(unsigned int tex);

    // Actually performs queued renders. Must be called once per frame from
    // inside mu::GetRenderer().BeginFrame()/EndFrame() (see RenderObjects()).
    void ProcessPendingRequests();

    int Size() const { return THUMB_SIZE; }

private:
    CObjectThumbnail() = default;
    ~CObjectThumbnail() = default;

    unsigned int RenderNow(int type);

    static constexpr int THUMB_SIZE = 112;
    static constexpr int MAX_RENDERS_PER_FRAME = 6;
    // RenderNow() can fail transiently (e.g. BeginOffscreenCapture() refusing
    // because the renderer hasn't started a frame yet at the moment this ran -
    // seen in bursts early in a session). Retry that many times before giving
    // up and caching it as a genuine failure, so a transient burst doesn't
    // permanently blacklist a thumbnail that would have rendered fine a moment
    // later.
    static constexpr int MAX_TRANSIENT_RETRIES = 60;

    int m_budget = 0;
    std::unordered_map<int, unsigned int> m_cache;      // type -> renderer texture id (0 = failed)
    std::unordered_map<int, int> m_failCount;           // type -> consecutive RenderNow() failures so far
    std::vector<int> m_pendingTypes;                    // Get() requests awaiting ProcessPendingRequests()

    bool m_scratchPending = false;    // a RequestSlotPreview() call awaits ProcessPendingRequests()
    int  m_scratchSlot = -1;          // Models[] slot to render for that pending request
    int  m_scratchFailCount = 0;      // consecutive RenderNow() failures for the current scratch request
    bool m_scratchHasResult = false;  // m_scratchResult holds an undelivered completed render
    unsigned int m_scratchResult = 0; // most recently completed scratch render
};

#define g_ObjectThumbnail CObjectThumbnail::GetInstance()

#endif // _EDITOR
