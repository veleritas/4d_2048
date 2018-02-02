#include <iostream>
#include <random>
using namespace std;

const int CELLS = 16;

int state[CELLS];

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dist(0, 9); // [0, 9]

int new_tile()
{
    return dist(gen) < 9? 1 : 2;
}

void init_board()
{
    std::uniform_int_distribution<> temp(0, CELLS-1);

    int pos[2] = {0};
    while (pos[0] == pos[1])
        pos[0] = temp(gen), pos[1] = temp(gen);

    state[pos[0]] = new_tile();
    state[pos[1]] = new_tile();
}

void draw_board()
{
    puts("+--+--+");
    for (int i=0; i<CELLS; i++)
    {
        if (i % 2 == 0)
            printf("|");

        printf("%x", state[i]);

        if (i % 4 == 3)
            puts("|");

        if (i % 8 == 7)
            puts("+--+--+");
    }

    printf("\n");
}


int main()
{
    init_board();
    draw_board();

    return 0;
}

