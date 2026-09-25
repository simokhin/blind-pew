#include "uci.h"
#include "zobrist.h"

int main() {
    init_zobrist_keys();
    uci_loop();
}