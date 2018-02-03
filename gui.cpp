#include <iostream>
#include <random>
using namespace std;

static const int CELLS = 16;
static const int MAX_VALS = 65536; // 2^16

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(0, 9); // [0, 9]

// Store move lookup table for moving left arrow key
uint16_t move_L[MAX_VALS];
uint16_t move_R[MAX_VALS];

uint64_t new_tile()
{
    return dist(gen) < 9? 1 : 2;
}

uint64_t get_val(uint64_t value, int pos)
{
    // Return the value at a given position.
    return (value >> (4 * pos)) & 0xF;
}

uint64_t spawn_tile(uint64_t state)
{
    // Spawn a tile in a random position.
    // Assumes there's at least one empty position.

    std::uniform_int_distribution<> temp(0, CELLS-1);

    int pos = temp(gen);
    while (state & (0xFULL << (4 * pos)))
        pos = temp(gen);

    return state | (new_tile() << (4 * pos));
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

void draw_row(uint16_t row)
{
    for (int i=0; i<4; i++)
    {
        printf("%x", row & 0xF);
        row >>= 4;
    }
    printf("\n");
}

uint16_t flip_row(uint16_t row)
{
    return (row >> 12) | ((row >> 4) & 0x00F0) | ((row << 4) & 0x0F00) | (row << 12);
}

void init_L()
{
    // Generate lookup tables for moving left.

    for (int i=0; i<MAX_VALS; i++)
    {
        uint16_t lower = i & 0xFF;
        uint16_t upper = i >> 8;

        if ((lower > 0 && lower < 0xFF) && ((lower & 0xF) == (lower >> 4)))
            lower = (lower & 0xF) + 1;

        if ((upper > 0 && upper < 0xFF) && ((upper & 0xF) == (upper >> 4)))
            upper = (upper & 0xF) + 1;

        move_L[i] = (upper << 8) | lower;
    }
}

void init_R()
{
    for (int i=0; i<MAX_VALS; i++)
    {
        uint16_t temp = ((i & 0xFF) << 8) | (i >> 8);
        move_R[i] = flip_row(move_L[temp]);
    }
}

void init_moves()
{
    init_L();
    init_R();
}

int main()
{
    init_moves();

    uint16_t hi = 0xaa01;

    draw_row(hi);
    draw_row(move_L[hi]);
    draw_row(move_R[hi]);

    return 0;


    init_L();

    uint64_t state = spawn_tile(spawn_tile(0));
    draw_board(state);

    return 0;
}

