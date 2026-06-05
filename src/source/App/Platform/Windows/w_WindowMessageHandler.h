#pragma once

namespace util
{
    struct WindowMessageHandler
    {
        virtual ~WindowMessageHandler(void) {}
        virtual bool HandleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result) = 0;
    };
};
