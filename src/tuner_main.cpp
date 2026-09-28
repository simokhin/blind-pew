#include <zobrist.h>

#include <fstream>
#include <iostream>

#include "bitboard.h"
#include "dataset.h"
#include "evaluate.h"
#include "search.h"
#include "tuner.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return 1;
    }

    // Инициализируем всё, что нужно для генерации ходов
    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
    init_rook_magics();
    init_bishop_magics();

    // 1. Заполняем реестр параметрами для тюнинга
    register_eval_tunable();

    // 2. Загружаем датасет
    auto dataset = load_epd_dataset(argv[1]);

    // 3. Подбираем K
    double k = fit_k(dataset);

    // 4. Запускаем тюнер
    run_tuner(dataset, k);

    // 5. Сохранение тюненных параметров в файл
    std::ofstream out("tuned_params.txt");
    print_tuned_params(out);
}
