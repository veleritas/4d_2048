#include <iostream>
#include <random>
using namespace std;

const int CELLS = 16;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(0, 9); // [0, 9]

uint64_t new_tile()
{
    return dist(gen) < 9? 1 : 2;
}

uint64_t get_val(uint64_t value, int pos)
{
    // Return the value at a given position.
    uint64_t mask = 0xFULL << (4 * pos);
    return (value & mask) >> (4 * pos);
}

uint64_t init_board()
{
    // Return starting state of game.

    std::uniform_int_distribution<> temp(0, CELLS-1);

    int pos[2] = {0};
    while (pos[0] == pos[1])
        pos[0] = temp(gen), pos[1] = temp(gen);

    return (new_tile() << (4 * pos[0])) | (new_tile() << (4 * pos[1]));
}

void draw_board(uint64_t state)
{
    // Given a game state value, draw the board with ASCII art.

    puts("+--+--+");
    for (int i=0; i<CELLS; i++)
    {
        if (i % 2 == 0)
            printf("|");

        printf("%x", get_val(state, i));

        if (i % 4 == 3)
            puts("|");

        if (i % 8 == 7)
            puts("+--+--+");
    }
}

int main()
{
    uint64_t state = init_board();
    draw_board(state);

    return 0;
}

