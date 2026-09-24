.PHONY: build run clean snapshot

build:
	cmake -B build
	cmake --build build

run: build
	./build/chess_engine

clean:
	rm -rf build build-release build-windows

# Builds optimized (Release) binaries for both Linux and Windows and saves named,
# versioned copies in bin/ under one shared version number, for match testing
# between engine versions. Usage: make snapshot NAME=alphabeta
snapshot:
	@if [ -z "$(NAME)" ]; then \
		echo "Usage: make snapshot NAME=<label>"; \
		exit 1; \
	fi
	cmake -B build-release -DCMAKE_BUILD_TYPE=Release
	cmake --build build-release
	cmake -B build-windows -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/windows-toolchain.cmake
	cmake --build build-windows
	@mkdir -p bin
	@last=$$(ls bin 2>/dev/null | grep -oE 'v[0-9]+_' | grep -oE '[0-9]+' | sort -n | tail -1); \
	last=$${last:-0}; \
	n=$$(( last + 1 )); \
	cp build-release/chess_engine bin/chess_engine_v$${n}_$(NAME); \
	cp build-windows/chess_engine.exe bin/chess_engine_v$${n}_$(NAME).exe; \
	echo "Saved bin/chess_engine_v$${n}_$(NAME) and bin/chess_engine_v$${n}_$(NAME).exe"
