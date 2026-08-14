#pragma once

#include "stdafx.h"
#include <RmlUi/Core/Context.h>
#include <memory>

class RmlUiRenderInterface;
class RmlUiSystemInterface;

// Owns the Rml::Context lifecycle and the one per-frame Update()/Render() entry point. Hooked
// from SceneManager.cpp's RenderScene() -- see the RmlUi migration plan's Phase 0.4 for why that
// call site (not a per-Scene hook) is the single choke point every scene funnels through.
class RmlUiRuntime
{
public:
    static RmlUiRuntime& Instance();

    void Create(int windowWidth, int windowHeight);
    void Destroy();
    bool IsCreated() const { return m_Context != nullptr; }

    void OnResize(int windowWidth, int windowHeight);

    void Update();
    void Render();

    Rml::Context* GetContext() const { return m_Context; }

private:
    RmlUiRuntime() = default;
    ~RmlUiRuntime();

    RmlUiRuntime(const RmlUiRuntime&) = delete;
    RmlUiRuntime& operator=(const RmlUiRuntime&) = delete;

    std::unique_ptr<RmlUiRenderInterface> m_RenderInterface;
    std::unique_ptr<RmlUiSystemInterface> m_SystemInterface;
    Rml::Context* m_Context = nullptr; // owned by Rml::Core, released via Rml::Shutdown()
};
