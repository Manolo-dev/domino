#pragma once
#define MAX_TILES 28

typedef struct {
    int left, right;
    float x, y;
    int placed;
} Tile;

typedef struct {
    Tile tiles[MAX_TILES];
    int tile_count;
} Board;

void board_init(Board* board);
