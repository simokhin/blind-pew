cmake_minimum_required(VERSION 3.20)
project(chess_engine_cpp CXX)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "LTO/IPO not supported, building without it: ${ipo_error}")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -mpopcnt -mavx2)

add_executable(chess_engine src/main.cpp src/board.cpp src/movegen.cpp src/move.cpp src/position.cpp src/perft.cpp src/evaluate.cpp src/search.cpp src/fen.cpp src/uci.cpp src/bench.cpp src/zobrist.cpp src/tt.cpp src/bitboard.cpp src/nnue.cpp)
target_include_directories(chess_engine PRIVATE include)
set(NNUE_FILE ${CMAKE_SOURCE_DIR}/nets/default.nnue)
target_compile_definitions(chess_engine PRIVATE NNUE_NET_PATH="${NNUE_FILE}")
set_source_files_properties(src/nnue.cpp PROPERTIES OBJECT_DEPENDS ${NNUE_FILE})

add_executable(eval_converter tools/eval_converter.cpp src/board.cpp src/position.cpp src/fen.cpp src/movegen.cpp src/move.cpp src/bitboard.cpp src/zobrist.cpp src/nnue.cpp)
target_include_directories(eval_converter PRIVATE include)
target_include_directories(eval_converter PRIVATE tools/third_party)
target_compile_definitions(eval_converter PRIVATE NNUE_NET_PATH="${NNUE_FILE}")
