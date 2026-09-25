#include "bitboard.h"

#include "board.h"

// Строит битборд, в котором установлен один бит на позиции square
Bitboard square_bb(int square) {
    return 1ULL << square;  // 1ULL - unsigned long long
}

Bitboard knight_attacks[64];
Bitboard king_attacks[64];
Bitboard pawn_attacks[2][64];

void init_knight_attacks() {
    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (const Offset& o : knight_offsets) {
            int new_rank = rank + o.dr;
            int new_file = file + o.df;

            if (is_valid_square(new_rank, new_file)) {
                knight_attacks[square] |= square_bb(square_of(new_rank, new_file));
            }
        }
    }
}

void init_king_attacks() {
    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (const Offset& o : king_offsets) {
            int new_rank = rank + o.dr;
            int new_file = file + o.df;

            if (is_valid_square(new_rank, new_file)) {
                king_attacks[square] |= square_bb(square_of(new_rank, new_file));
            }
        }
    }
}

void init_pawn_attacks() {
    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (Color color : {Color::White, Color::Black}) {
            int direction = color == Color::White ? 1 : -1;
            for (int df : {-1, 1}) {
                int new_rank = rank + direction;
                int new_file = file + df;
                if (is_valid_square(new_rank, new_file)) {
                    pawn_attacks[static_cast<int>(color)][square] |=
                        square_bb(square_of(new_rank, new_file));
                }
            }
        }
    }
}

Bitboard rook_mask(int square) {
    Bitboard mask = 0;

    int rank = rank_of(square);
    int file = file_of(square);

    for (const Offset& d : rook_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                int next_rank = new_rank + d.dr;
                int next_file = new_file + d.df;

                if (!is_valid_square(next_rank, next_file)) {
                    break;
                }

                mask |= square_bb(square_of(new_rank, new_file));
                new_rank += d.dr;
                new_file += d.df;
            } else {
                break;
            }
        }
    }

    return mask;
}

Bitboard bishop_mask(int square) {
    Bitboard mask = 0;

    int rank = rank_of(square);
    int file = file_of(square);

    for (const Offset& d : bishop_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                int next_rank = new_rank + d.dr;
                int next_file = new_file + d.df;

                if (!is_valid_square(next_rank, next_file)) {
                    break;
                }

                mask |= square_bb(square_of(new_rank, new_file));
                new_rank += d.dr;
                new_file += d.df;
            } else {
                break;
            }
        }
    }

    return mask;
}

Bitboard rook_attacks_otf(int square, Bitboard occupancy) {
    Bitboard attacks = 0;

    int rank = rank_of(square);
    int file = file_of(square);

    for (const Offset& d : rook_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                if (occupancy & square_bb(square_of(new_rank, new_file))) {
                    attacks |= square_bb(square_of(new_rank, new_file));
                    break;
                }

                attacks |= square_bb(square_of(new_rank, new_file));
                new_rank += d.dr;
                new_file += d.df;
            } else {
                break;
            }
        }
    }

    return attacks;
}

Bitboard bishop_attacks_otf(int square, Bitboard occupancy) {
    Bitboard attacks = 0;

    int rank = rank_of(square);
    int file = file_of(square);

    for (const Offset& d : bishop_directions) {
        int new_rank = rank + d.dr;
        int new_file = file + d.df;

        while (true) {
            if (is_valid_square(new_rank, new_file)) {
                if (occupancy & square_bb(square_of(new_rank, new_file))) {
                    attacks |= square_bb(square_of(new_rank, new_file));
                    break;
                }

                attacks |= square_bb(square_of(new_rank, new_file));
                new_rank += d.dr;
                new_file += d.df;
            } else {
                break;
            }
        }
    }

    return attacks;
}
