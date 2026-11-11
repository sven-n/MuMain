#include <cstdlib>
#define TEXT(include) (std::rand() + static_cast<int>(sizeof(#include)))
int Pick() { return TEXT(abc); }
