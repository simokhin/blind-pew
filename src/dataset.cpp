#include "dataset.h"

#include <format>
#include <fstream>
#include <sstream>

#include "fen.h"

DatasetPosition parse_epd_line(const std::string& line) {
    std::istringstream stream(line);

    std::string piece_placement;
    std::string side_to_move_str;
    std::string castling_rights_str;
    std::string en_passant_str;

    // Сохраняем значения из .epd
    stream >> piece_placement;
    stream >> side_to_move_str;
    stream >> castling_rights_str;
    stream >> en_passant_str;

    // Собираем FEN-строку
    std::string fen = std::format("{} {} {} {} 0 1", piece_placement, side_to_move_str,
                                  castling_rights_str, en_passant_str);

    Position position = parse_fen(fen);

    // Вытаскиваем результат партии
    size_t first_quote = line.find('"');
    size_t second_quote = line.find('"', first_quote + 1);

    std::string result_str = line.substr(first_quote + 1, second_quote - first_quote - 1);

    double result = 0.0;
    if (result_str == "1-0") {
        result = 1.0;
    } else if (result_str == "0-1") {
        result = 0.0;
    } else if (result_str == "1/2-1/2") {
        result = 0.5;
    }

    return DatasetPosition{position, result};
}

std::vector<DatasetPosition> load_epd_dataset(const std::string& path) {
    std::ifstream epd(path);
    std::vector<DatasetPosition> positions;

    // Читаем поток из .epd файла, пока он не завершится (позиции не закончатся)
    std::string line;
    while (std::getline(epd, line)) {
        DatasetPosition parsed_position = parse_epd_line(line);
        positions.push_back(parsed_position);
    }

    return positions;
}
