#include "move.h"

#include <fstream>
#include <unordered_map>
#include <chrono>
#include <cassert>
#include <utility>

const int MAX_DEPTH = 4;
const double CPROB_MINIMUM = 0.005;

const double SCORE_EMPTY = 1000;
const double SCORE_INTRA_MERGE = 50;
const double SCORE_INTER_MERGE = 50;

const double SCORE_SQR_MIDDLE = 100;

double row_heur_score[MAX_VALS];
double sqr_heur_score[MAX_VALS];

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

void init_row_heuristics()
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

row_t subsqr(int idx, board_t state)
{
    /*
     * Get the idx th 2x2 subsquare as a 16 bit number.
     * Example:
     *
     * Given the board
     * +--+--+
     * |f1|38|
     * |0c|7b|
     * +--+--+
     * |00|b2|
     * |5c|f1|
     * +--+--+
     *
     * the 2nd subsqr is:
     *
     * +--+--+
     * |00|00|
     * |5c|00|
     * +--+--+
     * |00|00|
     * |00|00|
     * +--+--+
     *
     * which is converted to 0xC500
    */

    board_t subsqr = state >> SQR_SHIFTS[idx];
    return ((subsqr >> 8) & 0xFF00) | (subsqr & 0xFF);
}

void swap(int &a, int &b)
{
    int temp = b;
    b = a;
    a = temp;
}

bool is_middle(int i, int a, int b)
{
    // Checks that i is the middle value of {i, a, b}
    if (a > b)
        swap(a, b);

    // do we allow zeroes as the smaller one?
    return (a < i) && (b > i);
}

void init_sqr_heuristics()
{
    // Generate heuristic scores for the 2x2 subsquares.
    for (int i=0; i<MAX_VALS; i++)
    {
        // Reward one smaller, one bigger for adjacent tiles
        int middles = is_middle(i & 0xF, (i >> 4) & 0xF, (i >> 8) & 0xF)
            + is_middle((i >> 4) & 0xF, i & 0xF, i >> 12)
            + is_middle((i >> 8) & 0xF, i & 0xF, i >> 12)
            + is_middle(i >> 12, (i >> 4) & 0xF, (i >> 8) & 0xF);

        sqr_heur_score[i] = middles * SCORE_SQR_MIDDLE;
    }
}

void init_heuristics()
{
    init_row_heuristics();
    init_sqr_heuristics();
}

void test()
{
    board_t a = rand_start();

    puts("original");
    draw_board(a);

    cout << endl;

    // shift amounts aren't nice (unfortunately)
    for (int i=0; i<4; i++)
    {
        draw_board((a >> SQR_SHIFTS[i]) & SQR_MASK);
        draw_row(subsqr(i, a));

        cout << sqr_heur_score[subsqr(i, a)] << endl;
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

double board_sqr_heur_score(board_t state)
{
    return sqr_heur_score[subsqr(0, state)]
        + sqr_heur_score[subsqr(1, state)]
        + sqr_heur_score[subsqr(2, state)]
        + sqr_heur_score[subsqr(3, state)];
}

double board_value(board_t state)
{
    return board_heur_score(state)
        + board_heur_score(transpose(state))
        + board_sqr_heur_score(state);
}

//------------------------------------------------------------------------------

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

            eval += 0.1 * eval_move(state | (spawn_val << 1), depth+1, cprob * 0.1);
            eval += 0.9 * eval_move(state | spawn_val, depth+1, cprob * 0.9);
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

//------------------------------------------------------------------------------

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

pair<board_t, int> play_game()
{
    // Play a single game and return the end position of the game.

    board_t state = spawn_tile(spawn_tile(0));
    draw_board(state);

    int moves = 0;
    while (!game_over(state))
    {
        auto start = std::chrono::system_clock::now();
        int move = find_best_move(state);
        auto stop = std::chrono::system_clock::now();

        std::chrono::duration<double> search_time = stop - start;

        moves++;

        printf("Moving: %c\n", MOVE_NAME[move]);
        printf("Move #%d\n", moves);
        printf("Search time: %6fs\n", search_time.count());

        state = move_board(move, state);
        state = spawn_tile(state);

        puts("Result:");
        draw_board(state);
    }

    return make_pair(state, moves);
}

int main(int argc, char* argv[])
{
    init_moves();
    init_heuristics();

    /*
     * Play a single game if no parameters are provided.
     * Otherwise play a variable number of games and save results to a file.
     * Command line argument order: num_games, file_name
     */

    if (argc == 1)
    {
        play_game();
        return 0;
    }

    assert(argc == 3);

    int num_games = atoi(argv[1]);
    assert(num_games > 0);
    ofstream fout(argv[2]);

    while (num_games--)
    {
        pair<board_t, int> res = play_game();
        fout << res.first << " " << res.second << endl;
    }

    return 0;
}

