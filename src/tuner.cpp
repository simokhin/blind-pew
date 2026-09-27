#include "tuner.h"

#include <chrono>
#include <cmath>

#include "search.h"

// Оценка позиции в сантипешках
int compute_qscore(Position& position) {
    SearchState state;

    // Ставим дэлдайн, чтобы поиск не прерывался из-за срабатывания функций, ответственных за
    // тайм-контроль
    state.hard_deadline = std::chrono::steady_clock::now() + std::chrono::hours(1);

    int score = quiescence(position, -INFINITE, INFINITE, state, 0);

    if (position.side_to_move == Color::White) {
        return score;
    } else {
        return -score;
    }
}

// Функция, сжимающая любое число в диапозон (0, 1)
// и вычисляющая вероятность победы.
double sigmoid(int q, double k) { return 1.0 / (1.0 + std::pow(10.0, -k * q / 400.0)); };

// Считает, насколько наш прогноз отличается от реального результата партии
double compute_error(std::vector<DatasetPosition>& dataset, double k) {
    double error_sum = 0.0;

    for (DatasetPosition& dp : dataset) {
        int qscore = compute_qscore(dp.position);

        double sigmoid_score = sigmoid(qscore, k);

        // Насколько наш прогноз отличается от реального результата партии
        double diff = sigmoid_score - dp.result;

        // Возводим в квадрат и прибавляем к счетчику
        error_sum += diff * diff;
    }

    return error_sum / static_cast<double>(dataset.size());
};

// Функция подбора K
double fit_k(std::vector<DatasetPosition>& dataset) {
    double best_k = 0.0;
    double best_error = 999.0;

    for (int i = 1; i <= 30; i++) {
        double k = i * 0.1;

        double error = compute_error(dataset, k);

        if (error < best_error) {
            best_error = error;
            best_k = k;
        }
    }

    return best_k;
};

bool try_delta(TunableParam& param, int delta, std::vector<DatasetPosition>& dataset, double k,
               double& best_error) {
    int old_value = *param.value;

    int new_value = old_value + delta;

    if (new_value < param.min || new_value > param.max) {
        return false;
    }

    // Применяем новое значение к реальной переменной
    *param.value = new_value;

    double new_error = compute_error(dataset, k);

    if (new_error < best_error) {
        // Если новое значение показал себя лучше, обновляем переменную
        best_error = new_error;
        return true;
    } else {
        // Возвращаем старое значение
        *param.value = old_value;
        return false;
    }
}

void run_tuner(std::vector<DatasetPosition>& dataset, double k) {
    // Считаем стартовую ошибку
    double best_error = compute_error(dataset, k);

    bool improved = true;

    while (improved) {
        improved = false;

        for (auto& [name, param] : tunable_params) {
            if (try_delta(param, 1, dataset, k, best_error)) {  // Пробуем добавить к параметру 1
                improved = true;
            } else if (try_delta(param, -1, dataset, k,
                                 best_error)) {  // Пробуем вычесть из параметра 1
                improved = true;
            }
        }
    }
}

void print_param(std::ostream& out, const std::string& name, int value) {
    out << name << " = " << value << ";\n";
}

void print_param_array(std::ostream& out, const std::string& name, int* array, int size) {
    out << name << " = {";
    for (int i = 0; i < size; i++) {
        out << array[i] << ", ";
    }
    out << "};\n";
}
