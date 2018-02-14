#include "move.h"

#include <unistd.h>
#include <cassert>

const double SCORE_EMPTY = 1000;

double row_score[MAX_VALS];
double row_heur_score[MAX_VALS];

board_t move_board(int direction, board_t state)
{
    // Move a board in a given direction.
    // LTRB AWDS
    // 0123 4567
    // left top right bottom 4d: left up right down

    switch (direction)
    {
        case 0: // left (left arrow)
            return move_board_L(state);
        case 1: // top (up arrow)
            return move_board_T(state);
        case 2: // right (right arrow)
            return move_board_R(state);
        case 3: // bottom (down arrow)
            return move_board_B(state);

        case 4: // 4D left (A key)
            return move_board_A(state);
        case 5: // 4D up (W key)
            return move_board_W(state);
        case 6: // 4D right (D key)
            return move_board_D(state);
        case 7: // 4D down (S key)
            return move_board_S(state);
    }
}

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

double board_value(board_t state)
{
    double value = 0;
    value += row_score[(state >> 0) & ROW_MASK];
    value += row_score[(state >> 16) & ROW_MASK];
    value += row_score[(state >> 32) & ROW_MASK];
    value += row_score[(state >> 48) & ROW_MASK];

    value += row_heur_score[(state >> 0) & ROW_MASK];
    value += row_heur_score[(state >> 16) & ROW_MASK];
    value += row_heur_score[(state >> 32) & ROW_MASK];
    value += row_heur_score[(state >> 48) & ROW_MASK];

    return value;
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
            double move_val = board_value(res);
            if (move_val > best_val)
                best_dir = i, best_val = move_val;
        }
    }

    assert(best_dir != -1);
    return best_dir;
}

int main()
{
    init_moves();

    init_heuristics();

    board_t state = spawn_tile(spawn_tile(0));
    draw_board(state);

    std::uniform_int_distribution<> rmove(0, 7);

    while (!game_over(state))
    {
        int move = find_best_move(state);

        printf("Moving: %c\n", MOVE_NAME[move]);
        state = move_board(move, state);
        state = spawn_tile(state);

        printf("After move:\n");
        draw_board(state);

        // usleep() in microseconds, must multiply by 1000 to get milliseconds
        usleep(200 * 1000);

//        getchar();
    }

    return 0;
}

