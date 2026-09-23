#pragma once

#include <vector>

#include "board.h"
#include "move.h"
#include "position.h"

struct Offset {
    int dr;
    int df;
};

std::vector<Move> generate_leaper_moves(const Board& board, int square,
                                        const std::vector<Offset>& offsets);
std::vector<Move> generate_knight_moves(const Board& board, int square);
std::vector<Move> generate_king_moves(const Board& board, int square);
std::vector<Move> generate_slider_moves(const Board& board, int square,
                                        const std::vector<Offset>& directions);
std::vector<Move> generate_rook_moves(const Board& board, int square);
std::vector<Move> generate_bishop_moves(const Board& board, int square);
std::vector<Move> generate_queen_moves(const Board& board, int square);
std::vector<Move> generate_pawn_moves(const Position& position, int square);
std::vector<Move> generate_castling_moves(const Position& position);

bool is_square_attacked(const Position& position, int square, Color by_color);

std::vector<Move> generate_pseudo_legal_moves(const Position& position);

constexpr int MAX_MOVES = 256;

class MoveList {
   public:
    void add(const Move& move);
    int size() const;
    const Move& operator[](int index) const;

   private:
    std::array<Move, MAX_MOVES> moves;
    int count = 0;
};