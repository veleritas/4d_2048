#include "move.h"

#include <unistd.h>

#include <fstream>
#include <unordered_map>

const int MAX_DEPTH = 8;
const double CPROB_MINIMUM = 0.001;

const double SCORE_EMPTY = 1000;
const double SCORE_INTRA_MERGE = 50;
const double SCORE_INTER_MERGE = 50;

double row_heur_score[MAX_VALS];

unordered_map<board_t, double> cache;

int visited;
int mdepth;

// Count the number of empty positions (= zero nibbles) in a board.
// Precondition: the board cannot be fully empty.
int count_empty(board_t x)
{
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

bool game_over(board_t state)
{
    // Determines whether the game is over.

    for (int i=0; i<NUM_MOVES; i++)
        if (move_board(i, state) != state)
            return false;

    return true;
}

bool intra_same(row_t sub)
{
    /*
     * Check if a subrow (2x1 cells) have the same value.
     * Example:
     *   22 00
     *   00 33
     *
     * Does not count:
     *   01 40
     *   00 00
     *   13 57
     */

    return ((sub & 0xF) > 0) && ((sub & 0xF) == (sub >> 4));
}

bool inter_same(row_t row)
{
    /*
     * Check if cells are the same across subrows.
     * Example:
     *   20 20
     *   14 24
     *   11 11
     *
     * Does not count:
     *   13 31
     *   45 78
     *
     * Assumes input has been preprocessed with: & 0xF0F
     */
    return ((row & 0xF) > 0) && ((row & 0xF) == (row >> 8));
}

void init_heuristics()
{
    // Pre-calculate heuristic scores for all board positions.

    for (int i=0; i<MAX_VALS; i++)
    {
        // Reward within small cell merges:

        int intra_merges = intra_same(i & 0xFF) * (i & 0xF)
            + intra_same(i >> 8) * ((i >> 8) & 0xF);

        // Reward inter subrow merges
        int inter_merges = inter_same(i & 0xF0F) * (i & 0xF)
            + inter_same((i >> 4) & 0xF0F) * ((i & 0xF0) >> 4);

        // Reward empty cells
        int num_empty = 0;
        for (int j=0; j<4; j++)
            if (((i >> (4 * j)) & 0xF) == 0)
                num_empty++;

        row_heur_score[i] = num_empty * SCORE_EMPTY
            + intra_merges * SCORE_INTRA_MERGE
            + inter_merges * SCORE_INTER_MERGE;
    }
}

double board_heur_score(board_t state)
{
    // Return the heuristic score of all the rows.
    return row_heur_score[(state >> 0) & ROW_MASK]
        + row_heur_score[(state >> 16) & ROW_MASK]
        + row_heur_score[(state >> 32) & ROW_MASK]
        + row_heur_score[(state >> 48) & ROW_MASK];
}

double board_value(board_t state)
{
    return board_heur_score(state)
        + board_heur_score(transpose(state));
}

// eval = expected value
double eval_move(board_t state, int depth, double cprob);

double eval_spawn(board_t state, int depth, double cprob)
{
    if (cprob < CPROB_MINIMUM || depth >= MAX_DEPTH)
    {
        mdepth = max(mdepth, depth);
        return board_value(state);
    }

    auto lookup = cache.find(state);
    if (lookup != cache.end())
        return lookup->second;

    visited++;

    int empty = count_empty(state);

    cprob /= empty;

    double eval = 0;

    board_t temp = state;
    board_t spawn_val = 1;

    while (spawn_val) // ends after shifting left 16 times
    {
        if ((temp & 0xF) == 0)
        {
            // Try spawning, and see what the score will be.

            eval += 0.9 * eval_move(state | spawn_val, depth+1, cprob * 0.9);
            eval += 0.1 * eval_move(state | (spawn_val << 1), depth+1, cprob * 0.1);
        }

        temp >>= 4;
        spawn_val <<= 4;
    }

    eval = eval / empty;

    cache[state] = eval;
    return eval;
}

double eval_move(board_t state, int depth, double cprob)
{
    /*
     * Given a board state, determine the best possible heuristic score
     * which can be achieved. Picks a single move from the possible moves.
     */
    double best = -1;
    for (int i=0; i<NUM_MOVES; i++)
    {
        board_t res = move_board(i, state);
        if (res != state)
            best = max(best, eval_spawn(res, depth, cprob));
    }

    if (best < 0) // No valid moves left (game ends).
        return board_value(state);

    return best;
}

int find_best_move(board_t state)
{
    // Given a board state, return the direction of the best move to make.

    visited = 0;
    mdepth = 0;
    cache.clear();

    int best_dir = -1;
    double best_val = -1;
    for (int i=0; i<NUM_MOVES; i++)
    {
        board_t res = move_board(i, state);
        if (res != state) // is a valid move
        {
            double hscore = eval_spawn(res, 0, 1);
            if (hscore > best_val)
                best_dir = i, best_val = hscore;
        }
    }

    printf("Visited: %d\n", visited);
    printf("Max depth: %d\n", mdepth);

    return best_dir;
}

board_t play_game(bool draw_game)
{
    // Play a single game and return the end position of the game.
    // Draw the game state to the screen if directed to do so.

    board_t state = spawn_tile(spawn_tile(0));

    if (draw_game)
        draw_board(state);

    while (!game_over(state))
    {
        int move = find_best_move(state);

        if (draw_game)
            printf("Moving: %c\n", MOVE_NAME[move]);

        state = move_board(move, state);
        state = spawn_tile(state);

        if (draw_game)
        {
            printf("After move:\n");
            draw_board(state);
        }
    }

    return state;
}

int main()
{
    init_moves();
    init_heuristics();

    ofstream fout("hi.txt");

    int NUM_GAMES = 1;
    for (int i=0; i<NUM_GAMES; i++)
    {
        board_t res = play_game(true);
        fout << res << endl;
    }

    return 0;
}

