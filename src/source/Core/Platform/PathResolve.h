// Case-correcting path resolution for POSIX filesystems (issue #462, Phase 4).
//
// The engine's asset paths are Windows-style: backslash separators and a case
// that does not always match the files on disk, which a case-sensitive
// filesystem rejects. The file-open shims funnel paths through here, which
// flips the separators and, when a component does not exist verbatim, falls
// back to a case-insensitive directory scan for it.
#pragma once

#ifndef _WIN32

#include <string>

// Returns `utf8Path` with separators normalized to '/' and each component
// case-corrected to an existing directory entry where the verbatim spelling
// does not exist. Components with no case-insensitive match are kept as-is, so
// paths for files being created resolve their directory and keep the new name,
// and genuinely missing files still fail at open time.
std::string MuResolvePath(const char* utf8Path);

#endif // !_WIN32
