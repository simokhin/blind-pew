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

- Bitboards
- Simple tapered evaluation
  - material
  - PST
- Simple search
  - negamax with alpha-beta pruning
  - iterative deepening
  - quiescence
  - MVV-LVA
  - transposition table
  - null move pruning
  - killer moves and history heuristic
  - principal variation search (PVS)
  - late move reduction (LMR)
  - static exchange evaluation (SEE)
- Basic UCI support
  - uci / isready / quit
  - position (startpos, fen, moves)
  - go (depth, movetime, wtime/btime/winc/binc)
  - setoption (Hash)
  - principal variation reporting per depth
- Simple benchmark

## Next

- Better evaluation
- Better move ordering
