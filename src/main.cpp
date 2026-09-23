#include "board.h"
#include "position.h"

int main()
{
    Position position = make_start_position();

    print_board(position.board);
}