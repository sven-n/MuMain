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
