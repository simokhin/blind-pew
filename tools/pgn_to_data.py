import sys

import chess
import chess.pgn

MAX_SCORE = 2000

input_path = sys.argv[1]
output_path = sys.argv[2]

with open(input_path) as f, open(output_path, "w") as out:
    results = {"1-0": 1.0, "1/2-1/2": 0.5, "0-1": 0.0}

    while True:
        game = chess.pgn.read_game(f)  # читаем партию
        if game is None:
            break

        board = game.board()
        result = results[game.headers["Result"]]

        for node in game.mainline():  # для каждой позиции
            token = node.comment.split()[0]
            score_text, depth_text = token.split("/")

            is_quiet = (
                not board.is_check()
                and not board.is_capture(node.move)
                and node.move.promotion is None
            )

            if score_text != "" and "M" not in score_text and is_quiet:
                score_cp = round(float(score_text) * 100)
                if board.turn == chess.BLACK:
                    score_cp = -score_cp
                if abs(score_cp) <= MAX_SCORE:
                    out.write(f"{board.fen()} | {score_cp} | {result}\n")
            board.push(node.move)
