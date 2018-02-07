#include <iostream>
#include <random>
using namespace std;

typedef uint16_t row_t;
typedef uint64_t board_t;

static const int CELLS = 16;
static const int MAX_VALS = 65536; // 2^16

static const row_t ROW_MASK = 0xFFFF;
static const board_t COL_MASK = 0x000F000F000F000FULL;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(0, 9); // [0, 9]

// Store move lookup tables
row_t move_L[MAX_VALS]; // left arrow
row_t move_R[MAX_VALS]; // right arrow
board_t move_T[MAX_VALS]; // up arrow (top)
board_t move_B[MAX_VALS]; // down arrow (bottom)

row_t move_A[MAX_VALS]; // A key
row_t move_D[MAX_VALS]; // D key
board_t move_W[MAX_VALS]; // W key
board_t move_S[MAX_VALS]; // S key

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

void init_LR()
{
    // Generate lookup tables for moving left and right.

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

    for (int i=0; i<MAX_VALS; i++)
        move_R[i] = flip_row(move_L[flip_row(i)]);
}

void init_AD()
{
    // Generate lookup tables for moving 4D left and right (A, D keys).

    for (int i=0; i<MAX_VALS; i++)
    {
        row_t even = i & 0x0F0F;
        row_t odd = (i & 0xF0F0) >> 4;

        if ((even & 0xF) == 0)
            even >>= 8;
        else if ((even < 0x0F0F) && ((even & 0xF) == (even >> 8)))
            even = (even & 0xF) + 1;

        if ((odd & 0xF) == 0)
            odd >>= 8;
        else if ((odd < 0x0F0F) && ((odd & 0xF) == (odd >> 8)))
            odd = (odd & 0xF) + 1;

        move_A[i] = (odd << 4) | even;
    }

    for (int i=0; i<MAX_VALS; i++)
        move_D[i] = flip_row(move_A[flip_row(i)]);
}

board_t row_to_col(row_t row)
{
    // Convert a row into a column (flipped along diagonal).
    board_t tmp = row;
    return (tmp | (tmp << 12) | (tmp << 24) | (tmp << 36)) & COL_MASK;
}

void init_TB()
{
    // Generate lookup tables for moving up and down (arrow keys).
    for (int i=0; i<MAX_VALS; i++)
        move_T[i] = row_to_col(move_L[i]);

    for (int i=0; i<MAX_VALS; i++)
        move_B[i] = row_to_col(move_R[i]);
}

void init_WS()
{
    // Generate lookup tables for moving 4D up and down (W and S keys).
    for (int i=0; i<MAX_VALS; i++)
        move_W[i] = row_to_col(move_A[i]);

    for (int i=0; i<MAX_VALS; i++)
        move_S[i] = row_to_col(move_D[i]);
}

void init_moves()
{
    init_LR();
    init_TB();
    init_AD();
    init_WS();
}

board_t move_board_L(board_t state)
{
    // Move the entire board left.
    board_t res = 0;

    res |= board_t(move_L[(state >> 0) & ROW_MASK]) << 0;
    res |= board_t(move_L[(state >> 16) & ROW_MASK]) << 16;
    res |= board_t(move_L[(state >> 32) & ROW_MASK]) << 32;
    res |= board_t(move_L[(state >> 48) & ROW_MASK]) << 48;

    return res;
}

board_t move_board_R(board_t state)
{
    // Move the entire board to the right.
    board_t res = 0;

    res |= board_t(move_R[(state >> 0) & ROW_MASK]) << 0;
    res |= board_t(move_R[(state >> 16) & ROW_MASK]) << 16;
    res |= board_t(move_R[(state >> 32) & ROW_MASK]) << 32;
    res |= board_t(move_R[(state >> 48) & ROW_MASK]) << 48;

    return res;
}

board_t move_board_T(board_t state)
{
    // Up arrow (top)
    board_t temp = transpose(state);
    board_t res = 0;

    res |= move_T[(temp >> 0) & ROW_MASK] << 0;
    res |= move_T[(temp >> 16) & ROW_MASK] << 4;
    res |= move_T[(temp >> 32) & ROW_MASK] << 8;
    res |= move_T[(temp >> 48) & ROW_MASK] << 12;

    return res;
}

board_t move_board_B(board_t state)
{
    // Down arrow (bottom)
    board_t temp = transpose(state);
    board_t res = 0;

    res |= move_B[(temp >> 0) & ROW_MASK] << 0;
    res |= move_B[(temp >> 16) & ROW_MASK] << 4;
    res |= move_B[(temp >> 32) & ROW_MASK] << 8;
    res |= move_B[(temp >> 48) & ROW_MASK] << 12;

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

    row_t hi = 0x2222;
    draw_row(hi);

    draw_row(move_A[hi]);
    draw_row(move_D[hi]);

    cout << endl;

    draw_row(move_L[hi]);
    draw_row(move_R[hi]);


    return 0;


/*
    board_t state = rand_start();
    draw_board(state);

    state = move_board_D(state);
    draw_board(state);

    return 0;
*/

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

