#pragma once

#include "w_BuffStateValueControl.h"
#include "w_BuffTimeControl.h"
#include "w_BuffScriptLoader.h"

SmartPointer(BuffStateSystem);
class BuffStateSystem : public util::WindowMessageHandler
{
public:
    static BuffStateSystemPtr Make();
    BuffStateSystem();
    virtual ~BuffStateSystem();

public:
    virtual bool HandleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result);

public:
    BuffScriptLoader& GetBuffInfo();
    BuffTimeControl& GetBuffTimeControl();
    BuffStateValueControl& GetBuffStateValueControl();

private:
    void Initialize();
    void Destroy();

private:
    BuffScriptLoaderPtr				m_BuffInfo;
    BuffTimeControlPtr				m_BuffTimeControl;
    BuffStateValueControlPtr		m_BuffStateValueControl;
};

inline
BuffScriptLoader& BuffStateSystem::GetBuffInfo()
{
    assert(m_BuffInfo);
    return *m_BuffInfo;
}

inline
BuffTimeControl& BuffStateSystem::GetBuffTimeControl()
{
    assert(m_BuffTimeControl);
    return *m_BuffTimeControl;
}

inline
BuffStateValueControl& BuffStateSystem::GetBuffStateValueControl()
{
    assert(m_BuffStateValueControl);
    return *m_BuffStateValueControl;
}
