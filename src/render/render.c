#include "render.h"

static ANativeWindow* g_window = NULL;

void render_init(ANativeWindow* window) {
    g_window = window;
    ANativeWindow_setBuffersGeometry(g_window, 0, 0, WINDOW_FORMAT_RGBA_8888);
}

void render_shutdown(void) { g_window = NULL; }

void render_frame(int divc, Div *divs[], uint32_t bg) {
    if (!g_window) return;
    
    ANativeWindow_Buffer buffer;

    if (ANativeWindow_lock(g_window, &buffer, NULL) != 0) return;

    uint32_t *pixels = buffer.bits;

    for (int y = 0; y < buffer.height; y++)
        for (int x = 0; x < buffer.width; x++)
            pixels[y * buffer.stride + x] = bg;

    for (int i = 0; i < divc; i++) {
        div_tree_update(divs[i], buffer.width, buffer.height);
        div_draw(divs[i], &(Buffer){ buffer.bits, buffer.width, buffer.height, buffer.stride });
    }

    ANativeWindow_unlockAndPost(g_window);
}