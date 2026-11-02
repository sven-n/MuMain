#include <cstdlib>
#define TEXT(include) (#include + std::rand())
const char* Pick() { return TEXT(abc); }
