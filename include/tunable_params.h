#pragma once
#include <ostream>
#include <string>
#include <unordered_map>

// Структура для тюнингуемого параметра
struct TunableParam {
    int* value;
    int min;
    int max;
};

// Хэш таблица с названием и структурой тюнингуемого параметра
extern std::unordered_map<std::string, TunableParam> tunable_params;

void register_tunable(const std::string&, int* value, int min, int max);

void register_tunable_array(const std::string& base_name, int* array, int size, int min, int max);

void print_param(std::ostream& out, const std::string& name, int value);

void print_param_array(std::ostream& out, const std::string& name, int* array, int size);
