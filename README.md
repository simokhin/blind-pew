# BlindPew

<p align="center">
  <img src="assets/blind_pew.png" width="300" alt="BlindPew logo">
</p>

A chess engine written in C++, built as a learning project for the language and for chess engine programming.

## Usage

### Build and run

The `Makefile` has `build`, `run` and `clean` targets. `make run` builds an optimized (Release) binary and starts the engine.

The engine speaks UCI over stdin/stdout - connect it to a UCI-compatible GUI, or type commands directly:

```bash
uci
isready
position startpos
go movetime 1000
```

## Implemented

- Bitboards
- NNUE evaluation
  - (768 → 128)x2 → 1, SCReLU, quantised weights
  - incrementally updated accumulators
  - trained with Bullet
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
  - futility pruning (frontier and reverse)
  - aspiration windows
- Basic UCI support
  - uci / isready / quit / stop / ucinewgame
  - position (startpos, fen, moves)
  - go (depth, movetime, wtime/btime/winc/binc, infinite, nodes)
  - setoption (Hash)
  - principal variation reporting per depth
- Simple benchmark `bench <depth>`

## Credits

- [Chess Programming Wiki](https://chessprogramming.org) - reference for most of algorithms used here
- [cutechess-cli](https://github.com/cutechess/cutechess) - engine match testing
- [Bullet](https://github.com/jw1912/bullet) - network training
