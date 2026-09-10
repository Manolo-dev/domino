#include "render.h"

static ANativeWindow* g_window = NULL;

void render_init(ANativeWindow* window) {
    g_window = window;
    ANativeWindow_setBuffersGeometry(g_window, 0, 0, WINDOW_FORMAT_RGBA_8888);
}

void render_shutdown(void) { g_window = NULL; }

typedef struct {
    Div* div;
    Buffer* buffer;
} DivRenderArgs;

void* render_div(void* divRenderArgs) {
    DivRenderArgs* args = (DivRenderArgs*)divRenderArgs;
    div_tree_update(args->div, args->buffer->width, args->buffer->height);
    div_draw(args->div, args->buffer);
    return NULL;
}

void render_frame(int divc, Div* divs[], uint32_t bg) {
    if (!g_window) return;
    
    ANativeWindow_Buffer buffer;

    if (ANativeWindow_lock(g_window, &buffer, NULL) != 0) return;

    uint32_t* pixels = buffer.bits;

    for (int y = 0; y < buffer.height; y++)
        for (int x = 0; x < buffer.width; x++)
            pixels[y * buffer.stride + x] = bg;

    for (int i = 0; i < divc; i++) {
        pthread_t thread;
        DivRenderArgs args = {divs[i], &(Buffer){ buffer.bits, buffer.width, buffer.height, buffer.stride }};
        pthread_create(&thread, NULL, render_div, &args);
        pthread_join(thread, NULL);
    }

    ANativeWindow_unlockAndPost(g_window);
}