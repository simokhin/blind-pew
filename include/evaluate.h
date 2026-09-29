#pragma once
#include <ostream>

#include "bitboard.h"
#include "position.h"

int evaluate(const Position& position);

// Регистрирует все параметры оценки для тюнинга
void register_eval_tunable();

// Записывает тюненные параметры в файл
void print_tuned_params(std::ostream& out);

int material_pst_value_mg(Piece piece, int square);
int material_pst_value_eg(Piece piece, int square);

void compute_material_pst(Position& position);
