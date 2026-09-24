.PHONY: build run clean snapshot

build:
	cmake -B build
	cmake --build build

run: build
	./build/chess_engine

clean:
	rm -rf build build-release

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
