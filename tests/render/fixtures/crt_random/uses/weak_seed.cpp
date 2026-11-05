#include <cstdlib>
// A weak reference still calls the CRT when it links.
#ifdef __GNUC__
#pragma weak srand
#endif
void Reseed() { if (&srand != nullptr) srand(7); }
