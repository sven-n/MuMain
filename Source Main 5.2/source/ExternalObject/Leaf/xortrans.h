#ifndef _XORTRANS_H_
#define _XORTRANS_H_

#include <windows.h>
#include "xstreambuf.h"

namespace leaf {
    class CCyclicXorTrans {
        xfstreambuf	m_xfKeyTable;

    public:
        CCyclicXorTrans();
        virtual ~CCyclicXorTrans();

        size_t GetKeyTableSize() const;

        bool LoadKeyTable(const std::wstring& filename);
        bool LoadKeyTable(const void* pKey, size_t size);
        void UnloadKeyTable();

        bool AppendKeyTable(const std::wstring& filename);
        bool AppendKeyTable(const void* pKey, size_t size);
        void JumbleKeyTable();
        bool SaveKeyTable(const std::wstring& filename);

        bool Trans(const void* src, size_t size, void* dest, DWORD dwStreamKey);
        bool Trans(const void* src, size_t size, xstreambuf& out, DWORD dwStreamKey);
    };
}

#endif // _XORTRANS_H_