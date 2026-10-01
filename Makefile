.PHONY: build build-windows run debug clean snapshot test

build:
	cmake -B build-release -DCMAKE_BUILD_TYPE=Release
	cmake --build build-release --target chess_engine

build-windows:
	cmake -B build-windows -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ -DCMAKE_EXE_LINKER_FLAGS=-static
	cmake --build build-windows --target chess_engine

run: build
	./build-release/chess_engine

debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build

clean:
	rm -rf build build-release build-windows

snapshot:
	@if [ -z "$(NAME)" ]; then \
		echo "Usage: make snapshot NAME=<label>"; \
		exit 1; \
	fi
	cmake -B build-release -DCMAKE_BUILD_TYPE=Release
	cmake --build build-release
	@mkdir -p bin
	@last=$$(ls bin 2>/dev/null | grep -oE 'v[0-9]+_' | grep -oE '[0-9]+' | sort -n | tail -1); \
	last=$${last:-0}; \
	n=$$(( last + 1 )); \
	cp build-release/chess_engine bin/chess_engine_v$${n}_$(NAME); \
	echo "Saved bin/chess_engine_v$${n}_$(NAME)"

test: build
	cmake --build build-release --target perft_test
	ctest --test-dir build-release --output-on-failure
