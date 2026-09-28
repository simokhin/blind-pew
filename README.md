# BlindPew

<p align="center">
  <img src="assets/blind_pew.png" width="300" alt="BlindPew logo">
</p>

A chess engine written in C++, built as a learning project for the language and for chess engine programming.

## Usage

### Build and run

The `Makefile` has `build`, `run` and `clean` targets. `make run` builds and optimized (Release) binary and starts the engine.

The engine speaks UCI over stdin/stdout - connect it to a UCI-compatible GUI, or type commands directly:

```bash
uci
isready
position startpos
go movetime 1000
```

## Implemented

- Bitboards
- Simple tapered evaluation
  - material
  - PST
  - mobility
  - bishop pair, rooks on open/semi-open files, passed pawns bonus
  - doubled/isolated pawns penalty
  - king safety (pawn shield)
  - parameters tuned with the Texel method
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
- Texel-method evaluation tuner (`chess_tuner`, multithreaded)

## Credits

- [Chess Programming Wikie](https://www.chesspogramming.org) - reference for most of algorithms used here
- [Texel's Tuning Method](https://www.chessporgramming.org/Texel%27s_Tuning_Method) by Peter Österlund, author of the [Texel](https://github.com/peterosterlund2/texel) engine - basis of the evaluation tuner
- [cutechess-cli](https://github.com/cutechess/cutechess) - engine match testing
