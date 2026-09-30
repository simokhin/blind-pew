#pragma once

#include <array>
#include <cstdint>

#include "board.h"

struct Position;

constexpr int INPUT_SIZE = 768;
constexpr int HIDDEN_SIZE = 128;
constexpr int QA = 255;
constexpr int QB = 64;
constexpr int SCALE = 182;
constexpr int NUM_OUTPUT_BUCKETS = 8;

struct alignas(64) Accumulator {
    std::array<int16_t, HIDDEN_SIZE> hidden_values;
};

struct Network {
    std::array<Accumulator, INPUT_SIZE> input_weights;
    Accumulator input_bias;
    std::array<std::array<int16_t, 2 * HIDDEN_SIZE>, NUM_OUTPUT_BUCKETS> output_weights;
    std::array<int16_t, NUM_OUTPUT_BUCKETS> output_bias;
};

extern Network nnue_network;

bool load_embedded_network();

int feature_index(Color color, Piece piece, int square);

void add_feature(Accumulator& acc, int index);
void remove_feature(Accumulator& acc, int index);

void refresh_accumulator(const Position& position, Color perspective, Accumulator& acc);

int nnue_evaluate(const Accumulator& us, const Accumulator& them, int bucket);

void accumulators_add(std::array<Accumulator, 2>& accs, Piece piece, int square);
void accumulators_remove(std::array<Accumulator, 2>& accs, Piece piece, int square);

bool load_network(const std::string& path);
