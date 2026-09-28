#pragma once
#include <ostream>

#include "bitboard.h"
#include "position.h"

int evaluate(const Position& position);

// Регистрирует все параметры оценки для тюнинга
void register_eval_tunable();

// Записывает тюненные параметры в файл
void print_tuned_params(std::ostream& out);
