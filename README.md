# BlindPew

<p align="center">
  <img src="assets/blind_pew.png" width="300" alt="BlindPew logo">
</p>

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

Builds optimized Linux binary and saves it to `bin/`, versioned automatically.

### Match testing

```bash
tools/match.sh [BASE_REF]
```

Runs a SPRT-terminated cutechess-cli match between the current working tree and `BASE_REF` (default `HEAD`), stopping once there's enough evidence to accept or reject the improvement hypothesis. Requires `cutechess-cli` and an opening book; see the script header for environment variable overrides (time control, SPRT bounds, concurrency, hash size).

## Implemented

- Bitboards
- Simple tapered evaluation
  - material
  - PST
  - mobility
  - bishop pair, rooks on open/semi-open files, passed pawns bonus
  - doubled/isolated pawns penalty
  - king safety (pawn shield)
- Simple search
  - negamax with alpha-beta pruning
  - iterative deepening
  - quiescence
  - MVV-LVA
  - transposition table
  - null move pruning
  - killer moves and history heuristic
  - principal variation search (PVS)
  - late move reduction (LMR, graduated by depth/move index)
  - static exchange evaluation (SEE)
- Basic UCI support
  - uci / isready / quit / stop / ucinewgame
  - position (startpos, fen, moves)
  - go (depth, movetime, wtime/btime/winc/binc, infinite)
  - setoption (Hash)
  - principal variation reporting per depth
- Simple benchmark

## Next

- Improve evaluation
- Improve search
