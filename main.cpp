#include "move.h"

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
        if (move_board(state) != state)
            return true;

    return false;
}

int main()
{
    init_moves();

    board_t state = spawn_tile(spawn_tile(0));
    draw_board(state);

    cout << endl;

    state = move_board_L(state);
    draw_board(state);

    return 0;
}

