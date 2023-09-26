#ifndef _IEXPLORER_H_
#define _IEXPLORER_H_

#pragma once

namespace leaf {
    /* Open URL for default web-browser    */
    /* This function return process handle */
    inline bool OpenExplorer(const std::wstring& url)
    {
        // SW_SHOW
        if (32 < (UINT)ShellExecute(NULL, L"open", url.c_str(), NULL, NULL, SW_NORMAL))
            return true;
        return false;
    }
}

#endif // _IEXPLORER_H_