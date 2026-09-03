#include "render.h"
#include "../div/div.h"

static ANativeWindow* g_window = NULL;

void render_init(ANativeWindow* window) {
    g_window = window;
    ANativeWindow_setBuffersGeometry(g_window, 0, 0, WINDOW_FORMAT_RGBA_8888);
}

void render_shutdown(void) { g_window = NULL; }

void render_frame(Div *root) {
    if (!g_window) return;
    
    ANativeWindow_Buffer buffer;

    if (ANativeWindow_lock(g_window, &buffer, NULL) != 0) return;

    div_tree_update(root, buffer.width, buffer.height);
    div_draw(root, &buffer);

    ANativeWindow_unlockAndPost(g_window);
}