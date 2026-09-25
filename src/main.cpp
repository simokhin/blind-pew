#include "tt.h"
#include "uci.h"
#include "zobrist.h"

int main() {
    init_zobrist_keys();

    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();

    resize_transposition_table(16);

    uci_loop();
}
