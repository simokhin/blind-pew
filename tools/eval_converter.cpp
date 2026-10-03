#include <bitboard.h>
#include <board.h>
#include <fen.h>
#include <movegen.h>

#include <cstdlib>
#include <format>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

int main() {
    // Отключаем синхронизацию потоков C++ со старым вводом-выводом C
    std::ios::sync_with_stdio(false);

    std::string line;
    long count = 0;

    long skipped_mate = 0;
    long skipped_depth = 0;
    long skipped_cp = 0;
    long skipped_check = 0;
    long kept = 0;
    long skipped_capture = 0;
    long skipped_promotions = 0;
    long skipped_en_passant = 0;

    init_rook_magics();
    init_bishop_magics();

    while (std::getline(std::cin, line)) {
        count++;

        if (count % 10000000 == 0) {
            std::cerr << "Строк обработано: " << count << "\nПозиций оставлено: " << kept << "\n";
        }

        json j = json::parse(line);
        std::string fen = j["fen"].get<std::string>();

        int best_depth = -1;
        json best_pv;

        for (const auto& e : j["evals"]) {
            int depth = e["depth"].get<int>();
            if (depth > best_depth) {
                best_depth = depth;
                best_pv = e["pvs"][0];
            }
        }

        // Пропускаем позиции с матом
        if (!best_pv.contains("cp")) {
            skipped_mate++;
            continue;
        }

        // Пропускаем ходы, найденные на глубине ниже 20
        if (best_depth < 20) {
            skipped_depth++;
            continue;
        }

        int cp = best_pv["cp"].get<int>();

        // Пропускаем ходы с оценекой выше 3000
        if (std::abs(cp) > 3000) {
            skipped_cp++;
            continue;
        }

        Position position = parse_fen(fen + " 0 1");

        // Пропускаем шахи
        if (is_square_attacked_bb(position, king_square_of(position, position.side_to_move),
                                  opposite_color(position.side_to_move))) {
            skipped_check++;
            continue;
        }

        std::string moves = best_pv["line"].get<std::string>();
        std::string mv = moves.substr(0, moves.find(' '));

        int to = square_from_algebraic(mv.substr(2, 2));

        // Пропускаем взятия
        if (position.board[to] != Piece::None &&
            color_of(position.board[to]) != position.side_to_move) {
            skipped_capture++;
            continue;
        }

        // Пропускаем превращения
        if (mv.size() == 5) {
            skipped_promotions++;
            continue;
        }

        // Пропуска en-passant
        int from = square_from_algebraic(mv.substr(0, 2));
        if ((piece_type_of(position.board[from]) == PieceType::Pawn) &&
            (to == position.en_passant_target)) {
            skipped_en_passant++;
            continue;
        }

        std::cout << fen + " 0 1" << " | " << cp << " | 0.5\n";

        kept++;
    }

    std::cerr << std::format(
        "count: {}\nskipped_mate: {}\nskipped_depth: {}\nskipped_cp: {}\nskiped_check: "
        "{}\nskipped_caputre: {}\nskipped_promotions: {}\nskipped_en_passant: {}\nkept: "
        "{}\n",
        count, skipped_mate, skipped_depth, skipped_cp, skipped_check, skipped_capture,
        skipped_promotions, skipped_en_passant, kept);

    return 0;
}
