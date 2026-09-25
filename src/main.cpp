#include "tt.h"
#include "uci.h"
#include "zobrist.h"

int main() {
    init_zobrist_keys();
    resize_transposition_table(16);
    uci_loop();
}