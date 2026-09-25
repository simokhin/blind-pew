#pragma once
#include "position.h"

// Массив ценности фигур
const std::array<int, 13> values = {0, 100, 320, 330, 500, 900, 0, 100, 320, 330, 500, 900, 0};

int evaluate(const Position& position);
int pst_bonus(Piece piece, int square);