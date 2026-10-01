#include "move.h"

#include "board.h"

// Конструктор хода
Move::Move(int from, int to, MoveFlag flag, PromotionPiece promotion) {
    data = static_cast<uint16_t>(to) | (static_cast<uint16_t>(from) << 6) |
           (static_cast<uint16_t>(promotion) << 12) | (static_cast<uint16_t>(flag) << 14);
}

// Получить клекту, с которой сделан ход
int Move::from() const {
    return (data >> 6) & 0x3F;  // 0x3F = 0b111111, маска на 6 бит
}

// Получить клетку, на которую сделан ход
int Move::to() const { return data & 0x3F; }

// Если ход является превращением, получить фигуру, в которую превращаются
PromotionPiece Move::promotion() const {
    int promotion_piece = (data >> 12) & 0x3;
    return static_cast<PromotionPiece>(promotion_piece);
}

bool Move::operator==(const Move& other) const { return data == other.data; }

uint16_t Move::raw() const { return Move::data; }

Move Move::from_raw(uint16_t raw) {
    Move m;
    m.data = raw;
    return m;
}

// Получить флаг, характеризующий ход
MoveFlag Move::flag() const {
    int flag = (data >> 14) & 0x3;
    return static_cast<MoveFlag>(flag);
}

std::string move_to_uci(const Move& m) {
    std::string move_string = algebraic_from_square(m.from()) + algebraic_from_square(m.to());

    if (m.flag() == MoveFlag::Promotion) {
        switch (m.promotion()) {
            case PromotionPiece::Queen:
                move_string += "q";
                break;
            case PromotionPiece::Rook:
                move_string += "r";
                break;
            case PromotionPiece::Knight:
                move_string += "n";
                break;
            case PromotionPiece::Bishop:
                move_string += "b";
                break;
            default:
                break;
        }
    }

    return move_string;
}
