#pragma once
#include <cstdint>
#include <random>

#include "board.h"

struct Position;

/// Клетки нуменуются от A1 = 0 до H8 = 63.
/// Бит `n` битборда соотвествует клетке `n`.
using Bitboard = uint64_t;

/// Все клетки вертикалей A и H. Используются для отсечения переходов через край доски.
constexpr Bitboard FILE_A = 0x0101010101010101ULL;
constexpr Bitboard FILE_H = 0x8080808080808080ULL;

/// Возвращает битборд с одним установленным битом на клетке `square` (0..63)
constexpr Bitboard square_bb(int square) { return 1ULL << square; }

/// Возвращает индекс младшего установленного бита и сбрасывает его в `bb`.
/// Предусловие: `bb != 0` (для нуля `__builtin_ctzll` даёт неопределённое поведение).
constexpr int pop_lsb(Bitboard& bb) {
    int bit_number = __builtin_ctzll(bb);
    bb &= bb - 1;
    return bit_number;
}

/// Возвращает битборд только с младшим установленным битом `bb`.
/// Для `bb == 0` возвращает 0.
constexpr Bitboard lsb_bb(Bitboard bb) { return bb & (-bb); }

/// Возвращает количество установленных битов в `bb`.
constexpr int popcount(Bitboard bb) { return __builtin_popcountll(bb); }

/// Строит таблицу ходов коня для всех клеток. Результат доступен как `knight_attacks`.
constexpr std::array<Bitboard, 64> make_knight_attacks() {
    std::array<Bitboard, 64> result{};

    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (const Offset& o : knight_offsets) {
            int new_rank = rank + o.dr;
            int new_file = file + o.df;

            if (is_valid_square(new_rank, new_file)) {
                result[square] |= square_bb(square_of(new_rank, new_file));
            }
        }
    }
    return result;
}

/// Строит таблицу ходов короля для всех клеток. Результат доступен как `knight_attacks`.
constexpr std::array<Bitboard, 64> make_king_attacks() {
    std::array<Bitboard, 64> result{};

    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (const Offset& o : king_offsets) {
            int new_rank = rank + o.dr;
            int new_file = file + o.df;

            if (is_valid_square(new_rank, new_file)) {
                result[square] |= square_bb(square_of(new_rank, new_file));
            }
        }
    }

    return result;
}

/// Строит таблицу клеток, которые атакуют пешки, индексированную как `[цвет][клетка]`.
/// Результат доступен как `pawn_attack`.
constexpr std::array<std::array<Bitboard, 64>, 2> make_pawn_attacks() {
    std::array<std::array<Bitboard, 64>, 2> result{};

    for (int square = 0; square < 64; square++) {
        int rank = rank_of(square);
        int file = file_of(square);

        for (Color color : {Color::White, Color::Black}) {
            int direction = color == Color::White ? 1 : -1;
            for (int df : {-1, 1}) {
                int new_rank = rank + direction;
                int new_file = file + df;
                if (is_valid_square(new_rank, new_file)) {
                    result[static_cast<int>(color)][square] |=
                        square_bb(square_of(new_rank, new_file));
                }
            }
        }
    }

    return result;
}

/// Возвращает клетки, занятость которых влияет на атаки ладьи с `square`: лучи во все стороны без
/// последней клетки каждого луча (фигура на краю доски не может ничего заслонить). Нужна для
/// индексации таблицы атак по магическим числам.
constexpr Bitboard rook_mask(int square) {
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

/// Возвращает клетки, занятость которых влияет на атаки слона с `square`: лучи во все стороны без
/// последней клетки каждого луча (фигура на краю доски не может ничего заслонить). Нужна для
/// индексации таблицы атак по магическим числам.
constexpr Bitboard bishop_mask(int square) {
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

/// Строит `rook_masks` для всех клеток.
constexpr std::array<Bitboard, 64> make_rook_masks() {
    std::array<Bitboard, 64> result{};

    for (int square = 0; square < 64; square++) {
        result[square] = rook_mask(square);
    }

    return result;
}

/// Строит `bishop masks` для всех клеток.
constexpr std::array<Bitboard, 64> make_bishop_masks() {
    std::array<Bitboard, 64> result{};

    for (int square = 0; square < 64; square++) {
        result[square] = bishop_mask(square);
    }

    return result;
}

/// Клетки, которые атакует конь с данной клетки: `knight_attacks[square]`.
inline constexpr std::array<Bitboard, 64> knight_attacks = make_knight_attacks();
static_assert(knight_attacks[0] == (square_bb(10) | square_bb(17)),
              "knight_attacks[A1] must be C2 and B3");

/// Клетки, которые атакует король с данной клетки: `king_attacks[square]`.
inline constexpr std::array<Bitboard, 64> king_attacks = make_king_attacks();
static_assert(king_attacks[0] == (square_bb(1) | square_bb(8) | square_bb(9)),
              "king_attacks[A1] must be A2, B1 and B2");

/// Клетки, которые атакует пешка цвета `color` с клетки `square`: `pawn_attacks[color][square]`.
/// Это только взятия без ходов вперёд.
inline constexpr std::array<std::array<Bitboard, 64>, 2> pawn_attacks = make_pawn_attacks();
static_assert(pawn_attacks[static_cast<int>(Color::White)][8] == square_bb(17),
              "white pawn on A2 must attack only B3");
static_assert(pawn_attacks[static_cast<int>(Color::Black)][55] == square_bb(46),
              "black pawn on H7 must attack only G6");

/// Маски значимой занятости ладьи по клеткам: `rook_masks[square]`.
inline constexpr std::array<Bitboard, 64> rook_masks = make_rook_masks();
static_assert(std::popcount(rook_masks[0]) == 12, "rook mask on A1 must have 12 squares");

/// Маски значимой занятости слона по клеткам: `rook_masks[square]`.
inline constexpr std::array<Bitboard, 64> bishop_masks = make_bishop_masks();
static_assert(std::popcount(bishop_masks[0]) == 6, "bishop mask on A1 must have 6 squares");

Bitboard random_sparse_u64(std::mt19937_64& rng);
Bitboard find_rook_magic(int square, std::mt19937_64& rng);
Bitboard find_bishop_magic(int square, std::mt19937_64& rng);
bool is_magic_valid(int square, Bitboard magic, Bitboard mask, int bits,
                    Bitboard (*attacks_fn)(int, Bitboard));

/// Заполняет таблицу атак ладьи для `rook_attacks_from`. Вызывается один раз при запуске, до
/// первого использования `rook_attacks_from`.
void init_rook_magics();

/// Заполняет таблицу атак слона для `bishop_attacks_from`. Вызывается один раз при запуске, до
/// первого использования `bishop_attacks_from`.
void init_bishop_magics();

/// Возвращает битборд всех фигур обоих цветов, которые атакуют клетку `square`.
/// Учитываются только фигуры из `occupancy`: передавая занятость с убранными фигурами, можно
/// получить атакующих после серии размена (рентгеновские атаки для SEE).
Bitboard attackers_to(const Position& position, int square, Bitboard occupancy);

/// Выбирает из `attackers` наименее ценную фигуру цвета `side` (порядок: пешка, конь, слон, ладья,
/// ферзь, король). Возвращает битборд с одной этой фигурой и записывает её тип в `out_type`. Если
/// фигур цвета `side` в `attackers` нет, возвращает 0, а `out_type` не меняется.
Bitboard least_valuable_attacker(const Position& position, Bitboard attackers, Color side,
                                 PieceType& out_type);

/// Возвращает битборд всех клеток, которые атакуют пешки `pawns` цвета `color`.
/// Считает сразу все пешки битовыми сдвигами; переходы через край доски отсекаются.
Bitboard pawn_attacks_bulk(Bitboard pawns, Color color);

/// Возвращает клетки, которые атакует ладья на `square` при занятости `occupancy`.
/// Первая занятая клетка на каждом луче включается в результат независимо от цвета фигуры.
/// Предусловие: до вызова выполнен `init_rook_magics()`.
Bitboard rook_attacks_from(int square, Bitboard occupancy);

/// Возвращает клетки, которые атакует слон на `square` при занятости `occupancy`.
/// Первая занятая клетка на каждом луче включается в результат независимо от цвета фигуры.
/// Предусловие: до вызова выполнен `init_bishop_magics()`.
Bitboard bishop_attacks_from(int square, Bitboard occupancy);
