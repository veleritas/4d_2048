#include <iostream>
#include <random>
using namespace std;

typedef uint16_t row_t;
typedef uint64_t board_t;

static const int CELLS = 16;
static const int MAX_VALS = 65536; // 2^16

static const int NUM_MOVES = 8;

static const row_t ROW_MASK = 0xFFFF;
static const board_t COL_MASK = 0x000F000F000F000FULL;

static const char MOVE_NAME[] = "LTRBAWDS";

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(0, 9); // [0, 9] 0 to 9 inclusive

board_t new_tile()
{
    return dist(gen) < 9? 1 : 2;
}

board_t spawn_tile(board_t state)
{
    // Spawn a tile in a random position.
    // Assumes there's at least one empty position.

    std::uniform_int_distribution<> temp(0, CELLS-1);

    int pos = temp(gen);
    while ((state >> (4 * pos)) & 0xF)
        pos = temp(gen);

    return state | (new_tile() << (4 * pos));
}

void draw_row(row_t row)
{
    for (int i=0; i<4; i++)
    {
        printf("%x", row & 0xF);
        row >>= 4;
    }
    printf("\n");
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

row_t flip_row(row_t row)
{
    return (row >> 12) | ((row >> 4) & 0x00F0) | ((row << 4) & 0x0F00) | (row << 12);
}

board_t row_to_col(row_t row)
{
    // Convert a row into a column (flipped along diagonal).
    board_t tmp = row;
    return (tmp | (tmp << 12) | (tmp << 24) | (tmp << 36)) & COL_MASK;
}

int count_empty(board_t x)
{
    // Count the number of empty positions (= zero nibbles) in a board.
    // Precondition: the board cannot be fully empty.

    x |= (x >> 2) & 0x3333333333333333ULL;
    x |= (x >> 1);
    x = ~x & 0x1111111111111111ULL;
    // At this point each nibble is:
    //  0 if the original nibble was non-zero
    //  1 if the original nibble was zero
    // Next sum them all
    x += x >> 32;
    x += x >> 16;
    x += x >>  8;
    x += x >>  4; // this can overflow to the next nibble if there were 16 empty positions
    return x & 0xf;
}

// Transpose rows/columns in a board:
//   0123       048c
//   4567  -->  159d
//   89ab       26ae
//   cdef       37bf
board_t transpose(board_t x)
{
    board_t a1 = x & 0xF0F00F0FF0F00F0FULL;
    board_t a2 = x & 0x0000F0F00000F0F0ULL;
    board_t a3 = x & 0x0F0F00000F0F0000ULL;
    board_t a = a1 | (a2 << 12) | (a3 >> 12);
    board_t b1 = a & 0xFF00FF0000FF00FFULL;
    board_t b2 = a & 0x00FF00FF00000000ULL;
    board_t b3 = a & 0x00000000FF00FF00ULL;
    return b1 | (b2 >> 24) | (b3 << 24);
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
