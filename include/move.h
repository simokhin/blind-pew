#pragma once
#include <cstdint>
#include <string>

enum class MoveFlag {
    Normal,
    Promotion,
    EnPassant,
    Castling,
};

enum class PromotionPiece {
    Knight,
    Bishop,
    Rook,
    Queen,
};

class Move {
   public:
    Move(int from = 0, int to = 0, MoveFlag flag = MoveFlag::Normal,
         PromotionPiece promotion = PromotionPiece::Knight);

    int from() const;
    int to() const;
    MoveFlag flag() const;
    PromotionPiece promotion() const;
    bool operator==(const Move& other) const;
    uint16_t raw() const;
    static Move from_raw(uint16_t raw);

   private:
    uint16_t data;
};

std::string move_to_uci(const Move& m);
