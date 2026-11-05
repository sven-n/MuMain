#include <cstdlib>
// A weak reference still calls the CRT when it links.
#ifdef __GNUC__
#pragma weak rand
#endif
int Pick() { return &rand != nullptr ? rand() % 3 : 0; }
