#include "board.h"

int square_from_algebraic(const std::string& s) {
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return square_of(rank, file);
}

std::string algebraic_from_square(int square) {
    char file = file_of(square) + 'a';
    char rank = rank_of(square) + '1';

    std::string move;
    move += file;
    move += rank;

    return move;
}
