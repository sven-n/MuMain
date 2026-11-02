// rand() and srand() are the CRT's; the game draws from Random::Rand().
/* rand() */
#include <cstdlib> // rand
namespace Random
{
int Rand();
}
bool rand_fps_check(int rate);
/*
 * rand() and srand(7) are only words here,
 * and so is std::rand().
 */
const char* Text()
{
    return "rand() % 3";
}
int operand = 0;
constexpr long Timeout = 10'000;
constexpr long Count = 1'000'000;
const char* Raw = R"(a /* "quoted" rand() )";
const char* Tagged = R"tag(a "quoted" /* rand() */ )" tag)tag";
bool Check(int rand_fps)
{
    return rand_fps_check(rand_fps) && Random::Rand() % 2 == 0;
}
