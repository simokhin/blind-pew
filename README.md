# Chess Engine (C++)

A chess engine written in C++, built as a learning project for the language and for chess engine programming.

## Usage

### Build and run

There's a `Makefile` with `build`/`run`/`clean` targets. `make run` builds and starts the engine.

The engine speaks UCI over stdin/stdout - connect it to a UCI-compatible GUI, or type commands directly:

```bash
uci
isready
position startpos
go movetime 1000
```

### Benchmark

```bash
bench [depth]
```

Runs a fixed set of test positions to the given depth (default 4), reports total nodes/time/NPS.

### Versioned release binaries

```bash
make snapshot NAME=<label>
```

Builds optimized Linux and Windows binaries and saves them to `bin/`, versioned automatically.

## Implemented

- Mailbox 8x8 board representation
- Move generation
- Simple tapered evaluation 
  - material
  - PST
- Simple search
  - negamax with alpha-beta pruning
  - quiescence
  - MVV-LVA
  - transposition table
  - iterative deepening
- Basic UCI support
  - uci / isready / quit
  - position (startpos, fen, moves)
  - go (depth, movetime, wtime/btime/winc/binc)
- Simple benchmark

## Next

- Better evaluation
- Better move ordering