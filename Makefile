.PHONY: build run clean snapshot snapshot-windows

build:
	cmake -B build
	cmake --build build

run: build
	./build/chess_engine

clean:
	rm -rf build build-release build-windows

# Builds an optimized (Release) binary and saves a named, versioned copy in bin/,
# for match testing between engine versions. Usage: make snapshot NAME=alphabeta
snapshot:
	@if [ -z "$(NAME)" ]; then \
		echo "Usage: make snapshot NAME=<label>"; \
		exit 1; \
	fi
	cmake -B build-release -DCMAKE_BUILD_TYPE=Release
	cmake --build build-release
	@mkdir -p bin
	@n=$$(( $$(ls bin 2>/dev/null | wc -l) + 1 )); \
	cp build-release/chess_engine bin/chess_engine_v$${n}_$(NAME); \
	echo "Saved bin/chess_engine_v$${n}_$(NAME)"

# Same as snapshot, but cross-compiles a standalone .exe for Windows via MinGW-w64.
# Usage: make snapshot-windows NAME=alphabeta
snapshot-windows:
	@if [ -z "$(NAME)" ]; then \
		echo "Usage: make snapshot-windows NAME=<label>"; \
		exit 1; \
	fi
	cmake -B build-windows -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/windows-toolchain.cmake
	cmake --build build-windows
	@mkdir -p bin
	@n=$$(( $$(ls bin 2>/dev/null | wc -l) + 1 )); \
	cp build-windows/chess_engine.exe bin/chess_engine_v$${n}_$(NAME).exe; \
	echo "Saved bin/chess_engine_v$${n}_$(NAME).exe"
