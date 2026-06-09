// Portable Winsock shim (issue #462, Phase 3).
//
// The reconnect reachability probe is the only Winsock user. Most of what it
// needs (socket/connect/getsockopt/select/sockaddr_in/AF_INET/...) is standard
// on POSIX, so on Windows this includes <ws2tcpip.h> and elsewhere it provides
// just the Winsock-specific names mapped onto BSD sockets.
#pragma once

#ifdef _WIN32

#include <ws2tcpip.h>

#else  // ---- non-Windows ----------------------------------------------------

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>    // htons
#include <netdb.h>        // getaddrinfo / addrinfo
#include <unistd.h>       // close
#include <sys/ioctl.h>    // ioctl, FIONBIO
#include <sys/select.h>   // select, fd_set
#include <cerrno>
#include <cwchar>         // wcstombs

#include "Core/Platform/WinCompat.h"  // WORD, MAKEWORD

typedef int SOCKET;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif

// Winsock startup is a no-op on BSD sockets.
typedef struct { WORD wVersion; } WSADATA, * LPWSADATA;
inline int WSAStartup(WORD, LPWSADATA) { return 0; }
inline int WSACleanup() { return 0; }
inline int WSAGetLastError() { return errno; }

inline int closesocket(SOCKET s) { return ::close(s); }

inline int ioctlsocket(SOCKET s, long cmd, u_long* argp)
{
    // Linux ioctl(FIONBIO) expects an int*, not Winsock's u_long*; convert so
    // the flag is read correctly (size/endianness safe).
    if (cmd == FIONBIO)
    {
        int val = argp ? static_cast<int>(*argp) : 0;
        return ::ioctl(s, FIONBIO, &val);
    }
    return ::ioctl(s, static_cast<unsigned long>(cmd), argp);
}

// Wide name resolution. The probe only reads ai_addr/ai_family, so map the wide
// addrinfo onto POSIX addrinfo and convert the node/service to the locale bytes.
typedef struct addrinfo addrinfoW, ADDRINFOW, * PADDRINFOW;

inline int GetAddrInfoW(const wchar_t* node, const wchar_t* service,
                        const addrinfoW* hints, addrinfoW** result)
{
    char n[256] = { 0 };
    char sv[64] = { 0 };
    if (node && wcstombs(n, node, sizeof(n) - 1) == static_cast<size_t>(-1)) return EAI_FAIL;
    if (service && wcstombs(sv, service, sizeof(sv) - 1) == static_cast<size_t>(-1)) return EAI_FAIL;
    return ::getaddrinfo(node ? n : nullptr, service ? sv : nullptr, hints, result);
}
inline void FreeAddrInfoW(addrinfo* p) { ::freeaddrinfo(p); }

#endif // _WIN32
