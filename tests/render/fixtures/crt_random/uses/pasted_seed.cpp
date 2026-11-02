#include <stdlib.h>
#define CALL(a, b) a##b
void Start() { CALL(sr, and)(7); }
