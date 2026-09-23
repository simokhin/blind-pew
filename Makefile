.PHONY: build run clean

build:
	cmake -B build
	cmake --build build

run: build
	./build/chess_engine

clean:
	rm -rf build
