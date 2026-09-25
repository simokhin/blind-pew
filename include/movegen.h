#pragma once

#include <vector>

#include "board.h"
#include "move.h"
#include "position.h"

struct Offset {
    int dr;
    int df;
};

constexpr int MAX_MOVES = 256;

class MoveList {
   public:
    void add(const Move& move);
    int size() const;
    const Move& operator[](int index) const;
    const Move* begin() const;
    const Move* end() const;
    Move* begin();
    Move* end();

   private:
    std::array<Move, MAX_MOVES> moves;
    int count = 0;
};

MoveList generate_leaper_moves(const Board& board, int square, const std::vector<Offset>& offsets);
MoveList generate_knight_moves(const Board& board, int square);
MoveList generate_king_moves(const Board& board, int square);
MoveList generate_slider_moves(const Board& board, int square,
                               const std::vector<Offset>& directions);
MoveList generate_rook_moves(const Board& board, int square);
MoveList generate_bishop_moves(const Board& board, int square);
MoveList generate_queen_moves(const Board& board, int square);
MoveList generate_pawn_moves(const Position& position, int square);
MoveList generate_castling_moves(const Position& position);

bool is_square_attacked(const Position& position, int square, Color by_color);

MoveList generate_pseudo_legal_moves(const Position& position);
MoveList generate_legal_moves(const Position& position);
MoveList generate_capture_moves(const Position& position);