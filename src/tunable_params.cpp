#include "tunable_params.h"

std::unordered_map<std::string, TunableParam> tunable_params;

// Добавляет параметр в хэш таблицу под определенным именем (key)
void register_tunable(const std::string& name, int* value, int min, int max) {
    TunableParam tunable_param = {
        .value = value,
        .min = min,
        .max = max,
    };

    tunable_params[name] = tunable_param;
}

// Регистрирует каждый элемент массива параметров и даёт каждому элементу уникальное имя
void register_tunable_array(const std::string& base_name, int* array, int size, int min, int max) {
    for (int i = 0; i < size; i++) {
        std::string name = base_name + std::to_string(i);
        register_tunable(name, &array[i], min, max);
    }
}
