#!/usr/bin/env bash
# Match a git ref ("base") against current working tree ("new") with
# cutechess-cli, using SPRT to decide when enough games have been played.
#
#   tools/match.sh [BASE_REF]          # default BASE_REF=HEAD
#
# Env overrides: TC (10+0.1), CONC (4), HASH (64, MB), ELO0 (0), ELO1 (10),
# ALPHA (0.05), BETA (0.05), OPENINGS (tools/openings/8_moves_v3.pgn),
# CUTECHESS (~/cutechess/build/cutechess-cli), OUT (bin/match-<timestamp>).
set -euo pipefail
cd "$(dirname "$0")/.."

BASE=${1:-HEAD}
TC=${TC:-10+0.1}
CONC=${CONC:-5}
HASH=${HASH:-64}
ELO0=${ELO0:-0}
ELO1=${ELO1:-10}
ALPHA=${ALPHA:-0.05}
BETA=${BETA:-0.05}
OPENINGS=${OPENINGS:-tools/openings/8moves_v3.pgn}
CUTECHESS=${CUTECHESS:-$HOME/cutechess/build/cutechess-cli}
OUT=$(realpath -m "${OUT:-bin/match-$(date +%Y%m%d%H%M%S)}")

mkdir -p "$OUT/base-src"
git archive "$BASE" | tar -x -C "$OUT/base-src"
(cd "$OUT/base-src" && cmake -B build-release -DCMAKE_BUILD_TYPE=Release && cmake --build build-release --target chess_engine)
cp "$OUT/base-src/build-release/chess_engine" "$OUT/base"

cmake -B build-release -DCMAKE_BUILD_TYPE=Release && cmake --build build-release --target chess_engine
cp build-release/chess_engine "$OUT/new"

exec "$CUTECHESS" \
    -engine name=new cmd="$OUT/new" proto=uci option.Hash="$HASH" \
    -engine name=base cmd="$OUT/base" proto=uci option.Hash="$HASH" \
    -each tc="$TC" \
    -rounds 999999 -games 2 -repeat \
    -concurrency "$CONC" \
    -openings file="$OPENINGS" format=pgn order=random \
    -sprt elo0="$ELO0" elo1="$ELO1" alpha="$ALPHA" beta="$BETA" \
    -pgnout "$OUT/match.pgn" \
    -recover \
    -ratinginterval 10

