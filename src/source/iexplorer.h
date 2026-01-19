#ifndef _IEXPLORER_H_
#define _IEXPLORER_H_

#pragma once

namespace leaf {
    /* Open URL for default web-browser    */
    /* This function return process handle */
    inline bool OpenExplorer(const std::wstring& url)
    {
        // SW_SHOW
		INT_PTR result = (INT_PTR)ShellExecute(NULL, L"open", url.c_str(), NULL, NULL, SW_NORMAL);
		if (result > 32)
            return true;
        return false;
    }
}

#endif // _IEXPLORER_H_