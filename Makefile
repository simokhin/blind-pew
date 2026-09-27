.PHONY: build run clean snapshot

build:
	cmake -B build
	cmake --build build

run: build
	./build/chess_engine

clean:
	rm -rf build build-release

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
