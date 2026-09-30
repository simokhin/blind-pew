#include "nnue.h"

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

#include "position.h"

asm(".section .rodata\n"
    ".balign 64\n"
    ".global net_start\n"
    "net_start:\n"
    ".incbin \"" NNUE_NET_PATH
    "\"\n"
    ".global net_end\n"
    "net_end:\n"
    ".text\n");

extern "C" const unsigned char net_start[];
extern "C" const unsigned char net_end[];

Network nnue_network;

static int screlu(int16_t value) {
    int new_value = std::clamp(static_cast<int>(value), 0, QA);

    new_value *= new_value;

    return new_value;
}

int feature_index(Color color, Piece piece, int square) {
    int index = color_of(piece) == color ? 0 : 384;

    index += static_cast<int>(piece_type_of(piece)) * 64;

    if (color == Color::Black) {
        index += square ^ 56;
    } else {
        index += square;
    }

    return index;
}

void add_feature(Accumulator& acc, int index) {
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        acc.hidden_values[i] += nnue_network.input_weights[index].hidden_values[i];
    }
}

void remove_feature(Accumulator& acc, int index) {
    for (int i = 0; i < HIDDEN_SIZE; i++) {
        acc.hidden_values[i] -= nnue_network.input_weights[index].hidden_values[i];
    }
}

void refresh_accumulator(const Position& position, Color perspective, Accumulator& acc) {
    acc = nnue_network.input_bias;

    for (int square = 0; square < 64; square++) {
        Piece piece = position.board[square];
        if (piece == Piece::None) {
            continue;
        } else {
            int index = feature_index(perspective, piece, square);
            add_feature(acc, index);
        }
    }
}

int nnue_evaluate(const Accumulator& us, const Accumulator& them) {
    int output = 0;

    for (int i = 0; i < HIDDEN_SIZE; i++) {
        output += screlu(us.hidden_values[i]) * nnue_network.output_weights[i];
    }

    for (int i = 0; i < HIDDEN_SIZE; i++) {
        output += screlu(them.hidden_values[i]) * nnue_network.output_weights[HIDDEN_SIZE + i];
    }

    // Череда манипуляций с output, чтобы первести его в сантипешки
    output /= QA;
    output += nnue_network.output_bias;
    output *= SCALE;
    output /= QA * QB;

    return output;
}

void accumulators_add(std::array<Accumulator, 2>& accs, Piece piece, int square) {
    add_feature(accs[0], feature_index(Color::White, piece, square));
    add_feature(accs[1], feature_index(Color::Black, piece, square));
}

void accumulators_remove(std::array<Accumulator, 2>& accs, Piece piece, int square) {
    remove_feature(accs[0], feature_index(Color::White, piece, square));
    remove_feature(accs[1], feature_index(Color::Black, piece, square));
}

bool load_network(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::cerr << "NNUE: file not found: " << path << "\n";
        return false;
    }
    if (std::filesystem::file_size(path) != sizeof(Network)) {
        std::cerr << std::format("NNUE: wrong file size for {}: expected {} bytes, got {}.\n", path,
                                 sizeof(Network), std::filesystem::file_size(path));
        return false;
    }

    std::ifstream file(path, std::ios::binary);

    if (!file) {
        std::cerr << "NNUE: cannot open " << path << "\n";
        return false;
    }

    file.read(reinterpret_cast<char*>(&nnue_network), sizeof(Network));

    if (file.gcount() != sizeof(Network)) {
        std::cerr << std::format("NNUE: short read from {}: got {} of {} bytes.\n", path,
                                 file.gcount(), sizeof(Network));
        return false;
    }

    std::cerr << "NNUE: loaded " << path << "\n";

    return true;
}

bool load_embedded_network() {
    if ((net_end - net_start) != sizeof(Network)) {
        std::cerr << std::format(
            "NNUE: embedded network has wrong size: expected {} bytes (HIDDEN_SIZE={}), got {}\n",
            sizeof(Network), HIDDEN_SIZE, net_end - net_start);
        return false;
    } else {
        std::memcpy(&nnue_network, net_start, sizeof(Network));
        std::cerr << "NNUE: embedded network loaded\n";
        return true;
    }
}
