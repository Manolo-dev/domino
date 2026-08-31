#include "domino.h"
#include "board.h"

static Board g_board;

void domino_init(void) {
    board_init(&g_board);
}

void domino_update(InputState* input) {
    if (input->touch_down) {
        // logique de sélection / placement de tuile à venir
    }
}
