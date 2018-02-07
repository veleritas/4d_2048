#include <iostream>
#include <random>
using namespace std;

typedef uint64_t board_t;
typedef uint16_t row_t;

static const int CELLS = 16;
static const int MAX_VALS = 65536; // 2^16

static const uint16_t ROW_MASK = 0xFFFF;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(0, 9); // [0, 9]

// Store move lookup tables

row_t move_L[MAX_VALS];
row_t move_R[MAX_VALS];

uint64_t new_tile()
{
    return dist(gen) < 9? 1 : 2;
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

void draw_board(board_t state)
{
    // Given a game state value, draw the board with ASCII art.

    puts("+--+--+");
    for (int i=0; i<CELLS; i++)
    {
        if (i % 2 == 0)
            printf("|");

        printf("%x", state & 0xF);
        state >>= 4;

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

row_t flip_row(row_t row)
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

        if ((lower & 0xF) == 0)
            lower >>= 4; // Move tile to empty spot
        else if ((lower < 0xFF) && ((lower & 0xF) == (lower >> 4)))
            lower = (lower & 0xF) + 1; // Combine tiles

        if ((upper & 0xF) == 0)
            upper >>= 4;
        else if ((upper < 0xFF) && ((upper & 0xF) == (upper >> 4)))
            upper = (upper & 0xF) + 1;

        move_L[i] = (upper << 8) | lower;
    }
}

void init_R()
{
    for (int i=0; i<MAX_VALS; i++)
        move_R[i] = flip_row(move_L[flip_row(i)]);
}

void init_moves()
{
    init_L();
    init_R();
}

static inline board_t move_board_L(board_t state)
{
    // Move the entire board left.
    board_t res = 0;

    res |= board_t(move_L[(state >> 0) & ROW_MASK]) << 0;
    res |= board_t(move_L[(state >> 16) & ROW_MASK]) << 16;
    res |= board_t(move_L[(state >> 32) & ROW_MASK]) << 32;
    res |= board_t(move_L[(state >> 48) & ROW_MASK]) << 48;

    return res;
}

static inline board_t move_board_R(board_t state)
{
    // Move the entire board to the right.
    board_t res = 0;

    res |= board_t(move_R[(state >> 0) & ROW_MASK]) << 0;
    res |= board_t(move_R[(state >> 16) & ROW_MASK]) << 16;
    res |= board_t(move_R[(state >> 32) & ROW_MASK]) << 32;
    res |= board_t(move_R[(state >> 48) & ROW_MASK]) << 48;

    return res;
}

board_t rand_start()
{
    // Create a randomly filled board for debugging purposes.

    std::uniform_int_distribution<> dist(0, MAX_VALS-1);

    board_t res = 0;
    for (int i=0; i<4; i++)
        res |= board_t(dist(gen)) << (16 * i);

    return res;
}

int main()
{
    init_moves();

    board_t state = rand_start();
    draw_board(state);

    state = move_board_R(state);
    draw_board(state);

    return 0;

/*
    init_moves();

    board_t state = spawn_tile(spawn_tile(0));
    draw_board(state);

    cout << endl;

    state = move_board_L(state);
    draw_board(state);

*/

    return 0;
}

