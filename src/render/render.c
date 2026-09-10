#include "render.h"

void render_frame(Buffer *buf, int divc, Div *divs[], uint32_t bg) {
    for (int i = 0; i < divc; i++)
        div_tree_update(divs[i], buf->width, buf->height);

    for (uint32_t y = 0; y < buf->height; y++)
        for (uint32_t x = 0; x < buf->width; x++)
            buf->bits[y * buf->stride + x] = bg;

    for (int i = 0; i < divc; i++)
        div_draw(divs[i], buf);
}