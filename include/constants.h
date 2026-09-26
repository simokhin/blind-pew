#pragma once

constexpr int INFINITE = 32000;
constexpr int MATE = 31000;
constexpr const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
constexpr const char* ENGINE_NAME = "BlindPew";
constexpr const char* ENGINE_AUTHOR = "Nikita Simokhin";
constexpr int MATE_THRESHOLD = MATE - 1000;
constexpr int MAX_PLY = 64;
constexpr int MAX_HISTORY = 16384;
