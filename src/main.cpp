#include "nnue.h"
#include "search.h"
#include "tt.h"
#include "uci.h"
#include "zobrist.h"

int main() {
    init_zobrist_keys();

    init_knight_attacks();
    init_king_attacks();
    init_pawn_attacks();
    init_rook_magics();
    init_bishop_magics();

    init_lmr_table();

    resize_transposition_table(16);

    if (!load_embedded_network()) {
        return 1;
    }

    uci_loop();
}
