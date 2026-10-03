cmake_minimum_required(VERSION 3.20)
project(chess_engine_cpp CXX)

# LTO (межмодульная оптимизация): включаем, если компилятор её поддерживает.
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "LTO/IPO not supported, building without it: ${ipo_error}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# compile_commands.json нужен clangd/IDE для подсказок и навигации
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Если тип сборки не задан явно, собираем Debug.
# Release задаётся в Makefile (-DCMAKE_BUILD_TYPE=Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# -mpopcnt и -mavx2 нужны для popcount и NNUE.
add_compile_options(-Wall -Wextra -Wpedantic -mpopcnt -mavx2)

# --- Ядро движка ---
# Весь код движка, кроме main.cpp, собирается один раз в статическую библиотеку.
# Исполняемые файлы ниже линкуются с ней, а не перекомпилируют исходники.
add_library(engine_core STATIC
    src/board.cpp src/movegen.cpp src/move.cpp src/position.cpp src/perft.cpp
    src/evaluate.cpp src/search.cpp src/fen.cpp src/uci.cpp src/bench.cpp
    src/zobrist.cpp src/tt.cpp src/bitboard.cpp src/nnue.cpp)
# PUBLIC: include/ нужен и самой библиотеке, и всем, кто с ней линкуется
target_include_directories(engine_core PUBLIC include)

# Сеть NNUE: путь к файлу вшивается в код как макрос NNUE_NET_PATH.
# PRIVATE: макрос нужен только при компиляции библиотеки.
set(NNUE_FILE ${CMAKE_SOURCE_DIR}/nets/default.nnue)
target_compile_definitions(engine_core PRIVATE NNUE_NET_PATH="${NNUE_FILE}")
# Пересобирать nnue.cpp при замене файла сети
set_source_files_properties(src/nnue.cpp PROPERTIES OBJECT_DEPENDS ${NNUE_FILE})

# --- Исполняемые файлы ---
# Сам движок (UCI)
add_executable(chess_engine src/main.cpp)
target_link_libraries(chess_engine PRIVATE engine_core)

# Утилита для парсинга .json файла позиций Lichess Cloud Database
# в формате для тренинга сети через Bullet; использует nlohmann/json
add_executable(eval_converter tools/eval_converter.cpp)
target_link_libraries(eval_converter PRIVATE engine_core)
target_include_directories(eval_converter PRIVATE tools/third_party)

# --- Тесты ---
add_executable(perft_test tests/perft_test.cpp)
target_link_libraries(perft_test PRIVATE engine_core)
enable_testing()
add_test(NAME perft_startpos COMMAND perft_test)
