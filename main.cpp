#include "move.h"

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

