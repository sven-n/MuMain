// rand() and srand() are the CRT's; the game draws from Random::Rand().
/* rand() */
const char* Text()
{
    return "rand() % 3";
}
int operand = 0;
bool Check(int rand_fps)
{
    return rand_fps_check(rand_fps) && Random::Rand() % 2 == 0;
}
/*
 * rand() and srand(7) are only words here,
 * and so is std::rand().
 */
constexpr long Timeout = 10'000;
constexpr long Count = 1'000'000;
const char* Raw = R"(a /* "quoted" rand() )";
int After()
{
    return Random::Rand();
}
const char* Tagged = R"tag(a "quoted" /* rand() */ )" tag)tag";
