#include "bitboard.h"

// Строит битборд, в котором установлен один бит на позиции square
Bitboard square_bb(int square) {
    return 1ULL << square;  // 1ULL - unsigned long long
}
