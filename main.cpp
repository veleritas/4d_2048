#include "move.h"

#include <unistd.h>
#include <cassert>

#include <fstream>

const double SCORE_EMPTY = 1000;
const int MAX_DEPTH = 3;

double row_score[MAX_VALS];
double row_heur_score[MAX_VALS];

bool game_over(board_t state)
{
    // Determines whether the game is over.

    for (int i=0; i<NUM_MOVES; i++)
        if (move_board(i, state) != state)
            return false;

    return true;
}

void init_heuristics()
{
    // Pre-calculate heuristic scores for all board positions.

    for (int i=0; i<MAX_VALS; i++)
    {
        double score = 0;
        int num_empty = 0;

        for (int j=0; j<4; j++)
        {
            int val = (i >> (4 * j)) & 0xF;
            if (val == 0) // empty square
                num_empty++;
            else if (val >= 2)
                score += (val - 1) * (1 << val);
        }

        row_score[i] = score;
        row_heur_score[i] = num_empty * SCORE_EMPTY;
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
    double value = 0;
    value += row_score[(state >> 0) & ROW_MASK];
    value += row_score[(state >> 16) & ROW_MASK];
    value += row_score[(state >> 32) & ROW_MASK];
    value += row_score[(state >> 48) & ROW_MASK];

    value += board_heur_score(state);
    value += board_heur_score(transpose(state));

    return value;
}

// eval = expected value
double eval_move(board_t state, int depth);

double eval_spawn(board_t state, int depth)
{
    int empty = 0;
    for (int i=0; i<CELLS; i++)
        empty += (((state >> (4 * i)) & 0xF) == 0);

    double eval = 0;
    for (int i=0; i<CELLS; i++)
        if (((state >> (4 * i)) & 0xF) == 0) // empty spot
        {
            // Try spawning, and see what the score will be.

            eval += 0.9 * eval_move(state | (board_t(1) << (4 * i)), depth+1);
            eval += 0.1 * eval_move(state | (board_t(2) << (4 * i)), depth+1);
        }

    return eval / empty;
}

double eval_move(board_t state, int depth)
{
    /*
     * Given a board state, determine the best possible heuristic score
     * which can be achieved. Picks a single move from the possible moves.
     */
    if (depth == MAX_DEPTH)
        return board_value(state);

    double best = -1;
    for (int i=0; i<NUM_MOVES; i++)
    {
        board_t res = move_board(i, state);
        if (res != state)
        {
            double hscore = eval_spawn(res, depth);
            if (hscore > best)
                best = hscore;
        }
    }

    if (best < 0) // No valid moves left (game ends).
        return board_value(state);

    return best;
}

int find_best_move(board_t state)
{
    // Given a board state, find the best move to make.

    int best_dir = -1;
    double best_val = -1;
    for (int i=0; i<NUM_MOVES; i++)
    {
        board_t res = move_board(i, state);
        if (res != state) // is a valid move
        {
            double hscore = eval_move(res, 0);
            if (hscore > best_val)
                best_dir = i, best_val = hscore;
        }
    }

    assert(best_dir != -1);
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

