
#ifndef WIN32
	
#ifndef _INT64_H_INCLUDED
#define _INT64_H_INCLUDED
	
#ifndef TEST_NO_WIN32
#include <inttypes.h>
	
typedef int64_t __int64;	
typedef uint64_t ulong64;	
#endif

#endif

#endif // WIN32