#pragma once
#include <cstdint>

enum class MoveFlag
{
    Normal,
    Promotion,
    EnPassant,
    Castling,
};

enum class PromotionPiece
{
    Knight,
    Bishop,
    Rook,
    Queen,
};

class Move
{
public:
    Move(int from, int to, MoveFlag flag = MoveFlag::Normal, PromotionPiece promotion = PromotionPiece::Knight);

    int from() const;
    int to() const;
    MoveFlag flag() const;
    PromotionPiece promotion() const;

private:
    uint16_t data;
};