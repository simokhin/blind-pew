#include "move.h"

// Конструктор хода
Move::Move(int from, int to, MoveFlag flag, PromotionPiece promotion)
{
    data = static_cast<uint16_t>(to) | (static_cast<uint16_t>(from) << 6) | (static_cast<uint16_t>(promotion) << 12) | (static_cast<uint16_t>(flag) << 14);
}

// Получить клекту, с которой сделан ход
int Move::from() const
{
    return (data >> 6) & 0x3F; // 0x3F = 0b111111, маска на 6 бит
}

// Получить клетку, на которую сделан ход
int Move::to() const
{
    return data & 0x3F;
}

// Если ход является превращением, получить фигуру, в которую превращаются
PromotionPiece Move::promotion() const
{
    int promotion_piece = (data >> 12) & 0x3;
    return static_cast<PromotionPiece>(promotion_piece);
}

// Получить флаг, характеризующий ход
MoveFlag Move::flag() const
{
    int flag = (data >> 14) & 0x3;
    return static_cast<MoveFlag>(flag);
}
