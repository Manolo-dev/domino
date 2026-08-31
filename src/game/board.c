#include "board.h"
#include <string.h>

void board_init(Board* board) {
    memset(board, 0, sizeof(Board));
    int idx = 0;
    for (int i = 0; i <= 6; i++) {
        for (int j = i; j <= 6; j++) {
            board->tiles[idx].left = i;
            board->tiles[idx].right = j;
            board->tiles[idx].placed = 0;
            idx++;
        }
    }
    board->tile_count = idx;
}
