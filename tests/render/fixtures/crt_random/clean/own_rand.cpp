// Functions of the game's own that are named rand.
namespace game
{
int rand()
{
    return 7;
}
}
struct Dice
{
    static int rand();
    static void srand(int seed);
};
int Roll()
{
    Dice::srand(3);
    return game::rand() + Dice::rand();
}
